# ulc/parser.py — UniLogic parser
# Takes the token list from lexer.tokenize() and builds an AST.
# Uses recursive descent. One method per grammar rule.

from ast_nodes import *


class Parser:
    def __init__(self, tokens, filename="<source>"):
        self.tokens     = tokens
        self.pos        = 0
        self.filename   = filename
        self.user_types = {}   # name -> TypeDecl (registered during parsing)

    # ── Token navigation ──────────────────────────────────────────────────────

    def peek(self):
        return self.tokens[self.pos]

    def advance(self):
        tok = self.tokens[self.pos]
        if tok[0] != "EOF":
            self.pos += 1
        return tok

    def check(self, kind, value=None):
        tok = self.peek()
        if tok[0] != kind:
            return False
        if value is not None and tok[1] != value:
            return False
        return True

    def match(self, kind, value=None):
        if self.check(kind, value):
            return self.advance()
        return None

    def expect(self, kind, value=None):
        tok = self.match(kind, value)
        if tok is None:
            got = self.peek()
            want = f"{kind}" + (f" '{value}'" if value else "")
            raise SyntaxError(
                f"{self.filename}:{got[2]}:{got[3]} — expected {want}, got {got[0]} '{got[1]}'"
            )
        return tok

    def at_end(self):
        return self.peek()[0] == "EOF"

    # ── Entry point ───────────────────────────────────────────────────────────

    def parse(self):
        decls = []
        while not self.at_end():
            if self.check("AT"):
                decls.append(self.parse_at_directive())
            elif self.check("KW", "import"):
                decls.append(self.parse_foreign_import())
            elif self.check("KW", "type"):
                decls.append(self.parse_type_decl())
            elif self.check("KW", "object"):
                decls.append(self.parse_object_decl())
            elif self.check("KW", "const") or self.check("KW", "constant"):
                decls.append(self.parse_const_decl())
            elif self.check("KW", "fixed"):
                # Peek ahead: fixed IDENT (not type keyword) = enum
                # fixed int x = 5 = const var (handled in function)
                save = self.pos
                self.advance()  # skip 'fixed'
                nxt = self.peek()
                self.pos = save  # restore
                if nxt[0] == "IDENT" and not self.is_type_keyword(nxt[1]):
                    decls.append(self.parse_enum_decl())
                else:
                    decls.append(self.parse_function())
            elif (self.check("KW") and self.is_type_keyword(self.peek()[1])):
                # Module-level variable declaration: int x = 5
                decls.append(self.parse_var_decl())
            else:
                decls.append(self.parse_function())
        return Program(decls)

    # ── @ directives ─────────────────────────────────────────────────────
    # @dr key = value
    # @norm N

    def parse_at_directive(self):
        tok = self.expect("AT")
        line, col = tok[2], tok[3]
        name_tok = self.expect("IDENT")
        name = name_tok[1]

        if name == "dr":
            key = self.expect("IDENT")[1]
            self.expect("OP", "=")
            # Accept both IDENT and KW as value (e.g. arena, gc are keywords)
            vtok = self.advance()
            if vtok[0] not in ("IDENT", "KW"):
                raise SyntaxError(
                    f"{self.filename}:{vtok[2]}:{vtok[3]} — expected identifier or keyword for @dr value, got {vtok[0]} '{vtok[1]}'")
            value = vtok[1]
            return DrDirective(key, value, line, col)

        if name == "norm":
            level_tok = self.expect("INT")
            return NormDirective(int(level_tok[1]), line, col)

        if name == "deprecated":
            # @deprecated("message") — consume string, attach to next function
            msg = ""
            if self.check("LPAREN"):
                self.advance()
                msg = self.expect("STRING")[1]
                self.expect("RPAREN")
            # The next declaration must be a function — parse it and tag
            func = self.parse_function()
            func._deprecated_msg = msg
            return func

        if name == "asm":
            # @asm(arch) ... end asm — inline assembly block (parsed as statement)
            self.expect("LPAREN")
            arch = self.expect("IDENT")[1]
            self.expect("RPAREN")
            # Collect raw lines until "end asm"
            body_lines = []
            while not self.at_end():
                if self.check("KW", "end"):
                    next_tok = self.tokens[self.pos + 1] if self.pos + 1 < len(self.tokens) else None
                    if next_tok and next_tok[0] == "IDENT" and next_tok[1] == "asm":
                        self.advance()  # consume 'end'
                        self.advance()  # consume 'asm'
                        break
                tok = self.advance()
                body_lines.append(tok[1])
            return AsmBlock(arch, " ".join(body_lines), line, col)

        raise SyntaxError(
            f"{self.filename}:{line}:{col} — unknown directive @{name}"
        )

    # ── Foreign import ──────────────────────────────────────────────────────────
    # import "libname" function name(type param, ...) returns type

    def parse_foreign_import(self):
        tok = self.expect("KW", "import")
        line, col = tok[2], tok[3]
        lib_tok = self.expect("STRING")
        lib = lib_tok[1]
        self.expect("KW", "function")
        name = self.expect("IDENT")[1]
        self.expect("LPAREN")
        params = []
        variadic = False
        if not self.check("RPAREN"):
            # Check for "..." (variadic with no named params)
            if self.check("DOT"):
                self.advance()  # .
                self.expect("DOT")
                self.expect("DOT")
                variadic = True
            else:
                params.append(self.parse_param())
                while self.match("COMMA"):
                    if self.check("DOT"):
                        self.advance()  # .
                        self.expect("DOT")
                        self.expect("DOT")
                        variadic = True
                        break
                    params.append(self.parse_param())
        self.expect("RPAREN")
        return_type = None
        if self.match("KW", "returns"):
            return_type = self.parse_type()
        return ForeignImport(lib, name, params, return_type, variadic, line, col)

    # ── Type declaration ───────────────────────────────────────────────────────
    # type Name
    #   type field
    #   ...
    # end type

    def parse_type_decl(self):
        tok = self.expect("KW", "type")
        line, col = tok[2], tok[3]
        name = self.expect("IDENT")[1]
        parent = None
        if self.match("KW", "inherits"):
            parent = self.expect("IDENT")[1]
        fields = []
        while not self.at_end():
            if self.check("KW", "end"):
                break
            field_type = self.parse_type()
            field_tok = self.expect("IDENT")
            fields.append(Param(field_type, field_tok[1], field_tok[2], field_tok[3]))
        self.expect("KW", "end")
        self.expect("KW", "type")
        decl = TypeDecl(name, fields, parent, line, col)
        self.user_types[name] = decl
        return decl

    # ── Object declaration ────────────────────────────────────────────────────
    # object Name [inherits Parent]
    #   type field
    #   function method(...) returns type ... end function
    # end object

    def parse_object_decl(self):
        tok = self.expect("KW", "object")
        line, col = tok[2], tok[3]
        name = self.expect("IDENT")[1]
        parent = None
        if self.match("KW", "inherits"):
            parent = self.expect("IDENT")[1]
        fields = []
        methods = []
        while not self.at_end():
            if self.check("KW", "end"):
                break
            if self.check("KW", "function"):
                methods.append(self.parse_function())
            else:
                # Field declaration: type name
                field_type = self.parse_type()
                field_tok = self.expect("IDENT")
                fields.append(Param(field_type, field_tok[1], field_tok[2], field_tok[3]))
        self.expect("KW", "end")
        self.expect("KW", "object")
        decl = ObjectDecl(name, fields, methods, parent, line, col)
        self.user_types[name] = decl
        return decl

    # ── Const declaration ────────────────────────────────────────────────────
    # const type name = literal

    def parse_const_decl(self):
        # Accept both 'const' and 'constant' keywords
        tok = self.match("KW", "const") or self.expect("KW", "constant")
        line, col = tok[2], tok[3]
        type_ = self.parse_type()
        name = self.expect("IDENT")[1]
        self.expect("OP", "=")
        value = self.parse_expr()
        return ConstDecl(type_, name, value, line, col)

    def parse_enum_decl(self):
        tok = self.expect("KW", "fixed")
        line, col = tok[2], tok[3]
        name = self.expect("IDENT")[1]
        members = []
        while not self.at_end():
            if self.check("KW", "end"):
                break
            member_name = self.expect("IDENT")[1]
            self.expect("OP", "=")
            member_value = self.parse_primary()
            members.append((member_name, member_value))
        self.expect("KW", "end")
        self.expect("KW", "fixed")
        return EnumDecl(name, members, line, col)

    # ── Function declaration ──────────────────────────────────────────────────
    # function name(type param, ...) returns type
    #   body
    # end function

    def parse_function(self):
        tok = self.expect("KW", "function")
        line, col = tok[2], tok[3]
        name = self.expect("IDENT")[1]
        self.expect("LPAREN")
        params = []
        variadic = False
        if not self.check("RPAREN"):
            # Check for ... (variadic marker)
            if self.check("DOT"):
                self.advance(); self.expect("DOT"); self.expect("DOT")
                variadic = True
            else:
                params.append(self.parse_param())
                while self.match("COMMA"):
                    if self.check("DOT"):
                        self.advance(); self.expect("DOT"); self.expect("DOT")
                        variadic = True
                        break
                    params.append(self.parse_param())
        self.expect("RPAREN")
        return_type = None
        is_generator = False
        returns_result = False
        if self.match("KW", "yields"):
            return_type = self.parse_type()
            is_generator = True
        elif self.match("KW", "returns"):
            # Check for result type: returns ok|error
            if self.check("KW", "ok"):
                ok_tok = self.advance()
                self.expect("PIPE")
                self.expect("KW", "error")
                returns_result = True
                return_type = None
            elif self.check("LPAREN"):
                # Tuple return: returns (type, type)
                self.advance()
                types = [self.parse_type()]
                while self.match("COMMA"):
                    types.append(self.parse_type())
                self.expect("RPAREN")
                return_type = TupleReturn(types, tok[2], tok[3])
            else:
                return_type = self.parse_type()
                if self.check("PIPE"):
                    self.advance()
                    self.expect("KW", "error")
                    returns_result = True
        body = self.parse_body()
        if not self.match("KW", "end"):
            got = self.peek()
            raise SyntaxError(
                f"{self.filename}:{got[2]}:{got[3]} — missing 'end function' for "
                f"function '{name}' (declared at line {line}). "
                f"Every function must be closed with 'end function'."
            )
        if not self.match("KW", "function"):
            got = self.peek()
            raise SyntaxError(
                f"{self.filename}:{got[2]}:{got[3]} — expected 'function' after 'end' "
                f"to close function '{name}' (declared at line {line})"
            )
        nested = [s for s in body if isinstance(s, FunctionDecl)]
        body = [s for s in body if not isinstance(s, FunctionDecl)]
        decl = FunctionDecl(name, params, return_type, body, line, col, is_generator, returns_result)
        decl.variadic = variadic
        decl.nested_functions = nested
        return decl

    def parse_param(self):
        type_ = self.parse_type()
        name  = self.expect("IDENT")
        # Check for default value: type name = literal
        default = None
        if self.check("OP", "="):
            self.advance()
            default = self.parse_primary()
        return Param(type_, name[1], default, name[2], name[3])

    # ── Type ──────────────────────────────────────────────────────────────────
    # type  ::=  typename  |  '<' typename '>'

    def parse_type(self):
        tok = self.peek()
        pointer = False
        if self.match("OP", "<"):
            pointer = True
        # Accept built-in type keywords OR user-defined type names (IDENT)
        if self.check("KW"):
            type_tok = self.advance()
        elif self.check("IDENT") and self.peek()[1] in self.user_types:
            type_tok = self.advance()
        else:
            type_tok = self.expect("KW")  # will fail with good error message
        if pointer:
            self.expect("OP", ">")
        # 'array int' → is_array=True, name="int"
        if type_tok[1] == "array":
            elem_tok = self.expect("KW")
            return TypeName(elem_tok[1], pointer, type_tok[2], type_tok[3], is_array=True)
        # 'list int' → is_list=True, name="int"
        if type_tok[1] == "list":
            # Check for 2D: list list int
            if self.check("KW", "list"):
                self.advance()
                inner_tok = self.expect("KW")
                return TypeName(inner_tok[1], pointer, type_tok[2], type_tok[3], is_list=True)
            elem_tok = self.expect("KW")
            return TypeName(elem_tok[1], pointer, type_tok[2], type_tok[3], is_list=True)
        return TypeName(type_tok[1], pointer, type_tok[2], type_tok[3])

    # ── Body ──────────────────────────────────────────────────────────────────
    # A sequence of statements until 'end' or 'else'

    def parse_body(self, extra_stops=None):
        stops = {"end", "else"}
        if extra_stops:
            stops |= extra_stops
        stmts = []
        while not self.at_end():
            tok = self.peek()
            # stop at block-closing keywords
            if tok[0] == "KW" and tok[1] in stops:
                break
            stmts.append(self.parse_statement())
        return stmts

    # ── Statements ────────────────────────────────────────────────────────────

    def parse_statement(self):
        tok = self.peek()

        # Inline @norm directive as statement
        if tok[0] == "AT":
            return self.parse_at_directive()

        if tok[0] == "KW":
            if tok[1] == "if":       return self.parse_if()
            if tok[1] == "while":    return self.parse_while()
            if tok[1] == "do":       return self.parse_do_while()
            if tok[1] == "for":      return self.parse_for()
            if tok[1] == "parallel":
                self.advance()
                return self.parse_for(parallel=True)
            if tok[1] == "match":    return self.parse_match()
            if tok[1] == "return":   return self.parse_return()
            if tok[1] == "yield":    return self.parse_yield()
            if tok[1] == "print":    return self.parse_print()
            if tok[1] == "escape":   self.advance(); return Escape(tok[2], tok[3])
            if tok[1] == "continue": self.advance(); return Continue(tok[2], tok[3])
            if tok[1] == "portal":
                self.advance()
                name = self.expect("IDENT")[1]
                return PortalDecl(name, tok[2], tok[3])
            if tok[1] == "goto":
                self.advance()
                name = self.expect("IDENT")[1]
                return GotoStmt(name, tok[2], tok[3])
            if tok[1] == "try":      return self.parse_try()
            if tok[1] == "throw":    return self.parse_throw()
            if tok[1] == "with":     return self.parse_with()
            if tok[1] == "killswitch":
                self.advance()
                cond = self.parse_expr()
                return KillswitchStmt(cond, tok[2], tok[3])
            if tok[1] == "fixed":
                # Peek ahead: fixed IDENT (not type keyword) = enum
                save = self.pos
                self.advance()
                nxt = self.peek()
                self.pos = save
                if nxt[0] == "IDENT" and not self.is_type_keyword(nxt[1]):
                    return self.parse_enum_decl()
                return self.parse_var_decl()
            # spawn func(args) — spawn as statement prefix
            if tok[1] == "spawn":
                self.advance()
                expr = self.parse_expr()  # parses func(args) as a Call
                return ExprStmt(Call("spawn", [expr], tok[2], tok[3]), tok[2], tok[3])
            # wait — standalone or wait()
            if tok[1] == "wait":
                self.advance()
                if self.check("LPAREN"):
                    self.advance()
                    self.expect("RPAREN")
                return ExprStmt(Call("wait", [], tok[2], tok[3]), tok[2], tok[3])
            # lock/unlock ident
            if tok[1] in ("lock", "unlock"):
                self.advance()
                expr = self.parse_expr()
                return ExprStmt(Call(tok[1], [expr], tok[2], tok[3]), tok[2], tok[3])
            # nested function declaration
            if tok[1] == "function":
                return self.parse_function()
            # type keyword at statement level = var declaration
            if self.is_type_keyword(tok[1]):
                return self.parse_var_decl()

        # user-defined type name at statement level = var declaration
        if tok[0] == "IDENT" and tok[1] in self.user_types:
            return self.parse_var_decl()

        # tuple destructure: (type name, type name) = expr
        if tok[0] == "LPAREN":
            return self._try_tuple_destructure_or_expr()

        # assignment or expression statement
        return self.parse_assign_or_expr()

    def is_type_keyword(self, word):
        return word in {
            "int", "float", "double", "string", "bool", "none", "complex",
            "int8", "int16", "int32", "int64",
            "uint8", "uint16", "uint32", "uint64",
            "auto", "array", "list", "map", "arena", "file",
        }

    def _try_tuple_destructure_or_expr(self):
        """Try to parse (type name, type name) = expr, fallback to expression stmt."""
        save = self.pos
        tok = self.peek()
        line, col = tok[2], tok[3]
        try:
            self.advance()  # skip (
            # Try to parse type name
            first_type = self.parse_type()
            first_name = self.expect("IDENT")[1]
            if self.check("COMMA"):
                # It's a tuple destructure
                targets = [(first_type, first_name)]
                while self.match("COMMA"):
                    t = self.parse_type()
                    n = self.expect("IDENT")[1]
                    targets.append((t, n))
                self.expect("RPAREN")
                self.expect("OP", "=")
                value = self.parse_expr()
                return TupleDestructure(targets, value, line, col)
        except (SyntaxError, Exception):
            pass
        # Fallback: restore position and parse as expression statement
        self.pos = save
        return self.parse_assign_or_expr()

    # ── Variable declaration ──────────────────────────────────────────────────
    # [fixed] type name [= expr]

    def parse_var_decl(self):
        tok = self.peek()
        line, col = tok[2], tok[3]
        fixed = bool(self.match("KW", "fixed"))
        type_ = self.parse_type()
        name  = self.expect("IDENT")[1]
        init  = None
        if self.match("OP", "="):
            init = self.parse_expr()
        return VarDecl(type_, name, init, fixed, line, col)

    # ── If ────────────────────────────────────────────────────────────────────
    # if condition
    #   body
    # [else
    #   body]
    # end if

    def parse_if(self):
        tok = self.expect("KW", "if")
        line, col = tok[2], tok[3]
        condition = self.parse_expr()
        then_body = self.parse_body()
        else_body = []
        if self.match("KW", "else"):
            else_body = self.parse_body()
        self.expect("KW", "end")
        self.expect("KW", "if")
        return If(condition, then_body, else_body, line, col)

    # ── Try/catch/finally ──────────────────────────────────────────────────────
    # try
    #   body
    # catch ExceptionType [as name]
    #   body
    # [finally
    #   body]
    # end try

    def parse_try(self):
        tok = self.expect("KW", "try")
        line, col = tok[2], tok[3]
        body = self.parse_body(extra_stops={"catch", "finally"})
        handlers = []
        while self.check("KW", "catch"):
            catch_tok = self.advance()
            exception_type = None
            binding_name = None
            if self.check("KW", "as"):
                # catch as name — catch-all with binding
                self.advance()  # consume 'as'
                binding_name = self.expect("IDENT")[1]
            elif self.check("IDENT"):
                exception_type = self.advance()[1]
                # Check for 'as name'
                if self.check("KW", "as"):
                    self.advance()  # consume 'as'
                    binding_name = self.expect("IDENT")[1]
            handler_body = self.parse_body(extra_stops={"catch", "finally"})
            handlers.append(CatchClause(exception_type, binding_name,
                                        handler_body, catch_tok[2], catch_tok[3]))
        finally_body = []
        if self.match("KW", "finally"):
            finally_body = self.parse_body()
        self.expect("KW", "end")
        self.expect("KW", "try")
        return TryStmt(body, handlers, finally_body, line, col)

    # ── Throw ────────────────────────────────────────────────────────────────
    # throw ExceptionType expr

    def parse_throw(self):
        tok = self.expect("KW", "throw")
        exception_type = self.expect("IDENT")[1]
        message = self.parse_expr()
        return ThrowStmt(exception_type, message, tok[2], tok[3])

    # ── With ──────────────────────────────────────────────────────────────────
    # with expr as binding
    #   body
    # end with

    def parse_with(self):
        tok = self.expect("KW", "with")
        line, col = tok[2], tok[3]
        expr = self.parse_expr()
        self.expect("KW", "as")
        binding = self.expect("IDENT")[1]
        body = self.parse_body()
        self.expect("KW", "end")
        self.expect("KW", "with")
        return WithStmt(expr, binding, body, line, col)

    # ── While ─────────────────────────────────────────────────────────────────
    # while condition
    #   body
    # end while

    def parse_while(self):
        tok = self.expect("KW", "while")
        line, col = tok[2], tok[3]
        condition = self.parse_expr()
        body = self.parse_body()
        self.expect("KW", "end")
        self.expect("KW", "while")
        return While(condition, body, line, col)

    def parse_do_while(self):
        tok = self.expect("KW", "do")
        line, col = tok[2], tok[3]
        body = self.parse_body(extra_stops={"while"})
        self.expect("KW", "while")
        condition = self.parse_expr()
        self.expect("KW", "end")
        self.expect("KW", "do")
        return DoWhile(body, condition, line, col)

    # ── For ───────────────────────────────────────────────────────────────────
    # for each var in iterable
    #   body
    # end for

    def parse_for(self, parallel=False):
        tok = self.expect("KW", "for")
        line, col = tok[2], tok[3]
        # for each var in iterable → For loop
        if self.check("KW", "each"):
            self.advance()
            var = self.expect("IDENT")[1]
            self.expect("KW", "in")
            iterable = self.parse_expr()
            body = self.parse_body()
            self.expect("KW", "end")
            self.expect("KW", "for")
            return For(var, iterable, body, line, col, parallel=parallel)
        # C-style for: for init while cond do update  OR  for init, cond, update
        init = self.parse_statement()
        if self.check("KW", "while"):
            # for int i = 0 while i < 10 do i++
            self.advance()
            condition = self.parse_expr()
            self.expect("KW", "do")
            update = self.parse_statement()
            body = self.parse_body()
            self.expect("KW", "end")
            self.expect("KW", "for")
            return CFor(init, condition, update, body, line, col)
        elif self.check("COMMA"):
            # for i=0, i<10, i++
            self.advance()
            condition = self.parse_expr()
            self.expect("COMMA")
            update = self.parse_statement()
            body = self.parse_body()
            self.expect("KW", "end")
            self.expect("KW", "for")
            return CFor(init, condition, update, body, line, col)
        else:
            # Fallback — shouldn't happen with well-formed code
            body = self.parse_body()
            self.expect("KW", "end")
            self.expect("KW", "for")
            return CFor(init, BoolLiteral(True), None, body, line, col)

    # ── Match ─────────────────────────────────────────────────────────────────
    # match expr
    #   value
    #     body
    #   default
    #     body
    # end match

    def parse_match(self):
        tok = self.expect("KW", "match")
        line, col = tok[2], tok[3]
        subject = self.parse_postfix()  # restricted: no binary ops in subject
        cases = []
        while not self.at_end():
            if self.check("KW", "end"):
                break
            if self.match("KW", "default"):
                body = self._parse_case_body()
                cases.append(MatchCase(None, body, tok[2], tok[3]))
            else:
                case_tok = self.peek()
                value = self.parse_expr()
                body = self._parse_case_body()
                cases.append(MatchCase(value, body, case_tok[2], case_tok[3]))
        self.expect("KW", "end")
        self.expect("KW", "match")
        return Match(subject, cases, line, col)

    def _parse_case_body(self):
        """Parse statements for a match case, stopping at next case value, default, or end."""
        stmts = []
        while not self.at_end():
            tok = self.peek()
            # End of match block
            if tok[0] == "KW" and tok[1] in ("end", "default"):
                break
            # Next case: a bare literal or identifier that isn't a statement keyword
            if self._is_case_start(tok):
                break
            stmts.append(self.parse_statement())
        return stmts

    def _is_case_start(self, tok):
        """Check if a token looks like the start of a new match case value."""
        # Literal tokens that can only be case values, not statement keywords
        if tok[0] in ("INT", "HEX", "FLOAT", "STRING"):
            return True
        # Boolean/empty literals used as case values
        if tok[0] == "KW" and tok[1] in ("true", "false", "empty"):
            return True
        # Negative literal: OP '-' followed by INT or FLOAT
        if tok[0] == "OP" and tok[1] == "-":
            next_pos = self.pos + 1
            if next_pos < len(self.tokens):
                nt = self.tokens[next_pos]
                if nt[0] in ("INT", "FLOAT"):
                    return True
        return False

    # ── Return ────────────────────────────────────────────────────────────────

    def parse_return(self):
        tok = self.expect("KW", "return")
        # return ok expr
        if self.check("KW", "ok"):
            self.advance()
            value = self.parse_expr()
            return Return(OkResult(value, tok[2], tok[3]), tok[2], tok[3])
        # return error expr
        if self.check("KW", "error"):
            self.advance()
            value = self.parse_expr()
            return Return(ErrorResult(value, tok[2], tok[3]), tok[2], tok[3])
        value = None
        # if next token could start an expression, parse it
        if not self.at_end() and not self.check("KW", "end"):
            # Check for tuple return: return (expr1, expr2, ...)
            if self.check("LPAREN"):
                save = self.pos
                self.advance()
                first = self.parse_expr()
                if self.check("COMMA"):
                    # It's a tuple return
                    elements = [first]
                    while self.match("COMMA"):
                        elements.append(self.parse_expr())
                    self.expect("RPAREN")
                    value = TupleLiteral(elements, tok[2], tok[3])
                else:
                    # Just a parenthesized expression
                    self.expect("RPAREN")
                    value = first
            else:
                value = self.parse_expr()
        return Return(value, tok[2], tok[3])

    # ── Yield ────────────────────────────────────────────────────────────────

    def parse_yield(self):
        tok = self.expect("KW", "yield")
        value = self.parse_expr()
        return Yield(value, tok[2], tok[3])

    # ── Print ─────────────────────────────────────────────────────────────────

    def parse_print(self):
        tok = self.expect("KW", "print")
        value = self.parse_expr()
        return Print(value, tok[2], tok[3])

    # ── Assignment or expression statement ───────────────────────────────────

    def parse_assign_or_expr(self):
        expr = self.parse_expr()
        tok  = self.peek()
        ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "%=", "left=", "right="}
        if tok[0] == "OP" and tok[1] in ASSIGN_OPS:
            op = self.advance()[1]
            value = self.parse_expr()
            return Assign(expr, op, value, tok[2], tok[3])
        return ExprStmt(expr, tok[2], tok[3])

    # ── Expressions (precedence climbing) ────────────────────────────────────
    # Precedence low → high:
    #   pipe |>
    #   or (logical)
    #   xor (logical)
    #   and (logical)
    #   not (logical unary)
    #   bit_or (bitwise)
    #   bit_xor (bitwise)
    #   bit_and (bitwise)
    #   == != < > <= >=
    #   bit_left bit_right (shifts)
    #   + -
    #   * / %
    #   unary - address deref bit_not
    #   postfix: call, index, cast

    def parse_expr(self):      return self.parse_pipe()

    def parse_pipe(self):
        left = self.parse_or()
        while self.check("OP", "|>"):
            op = self.advance()
            left = BinaryOp("|>", left, self.parse_or(), op[2], op[3])
        return left

    def parse_or(self):
        left = self.parse_and()
        while self.check("KW", "or"):
            op = self.advance()
            left = BinaryOp("or", left, self.parse_and(), op[2], op[3])
        return left

    def parse_and(self):
        left = self.parse_not()
        while self.check("KW", "and"):
            op = self.advance()
            left = BinaryOp("and", left, self.parse_not(), op[2], op[3])
        return left

    def parse_not(self):
        if self.check("KW", "not"):
            op = self.advance()
            return UnaryOp("not", self.parse_not(), op[2], op[3])
        return self.parse_bit_or()

    def parse_bit_or(self):
        left = self.parse_bit_xor()
        while self.check("KW", "bit_or"):
            op = self.advance()
            left = BinaryOp("bit_or", left, self.parse_bit_xor(), op[2], op[3])
        return left

    def parse_bit_xor(self):
        left = self.parse_bit_and()
        while self.check("KW", "bit_xor"):
            op = self.advance()
            left = BinaryOp("bit_xor", left, self.parse_bit_and(), op[2], op[3])
        return left

    def parse_bit_and(self):
        left = self.parse_comparison()
        while self.check("KW", "bit_and"):
            op = self.advance()
            left = BinaryOp("bit_and", left, self.parse_comparison(), op[2], op[3])
        return left

    def parse_comparison(self):
        left = self.parse_shift()
        CMP = {"==", "!=", "<", ">", "<=", ">="}
        while True:
            if self.peek()[0] == "OP" and self.peek()[1] in CMP:
                op = self.advance()
                left = BinaryOp(op[1], left, self.parse_shift(), op[2], op[3])
            elif self.check("KW", "in"):
                op = self.advance()
                left = BinaryOp("in", left, self.parse_shift(), op[2], op[3])
            else:
                break
        return left

    def parse_shift(self):
        left = self.parse_addition()
        while self.check("KW", "bit_left") or self.check("KW", "bit_right"):
            op = self.advance()
            left = BinaryOp(op[1], left, self.parse_addition(), op[2], op[3])
        return left

    def parse_addition(self):
        left = self.parse_multiplication()
        while self.peek()[0] == "OP" and self.peek()[1] in ("+", "-"):
            op = self.advance()
            left = BinaryOp(op[1], left, self.parse_multiplication(), op[2], op[3])
        return left

    def parse_multiplication(self):
        left = self.parse_unary()
        while self.peek()[0] == "OP" and self.peek()[1] in ("*", "/", "%"):
            op = self.advance()
            left = BinaryOp(op[1], left, self.parse_unary(), op[2], op[3])
        return left

    def parse_unary(self):
        tok = self.peek()
        if tok[0] == "OP" and tok[1] == "-":
            self.advance()
            return UnaryOp("-", self.parse_unary(), tok[2], tok[3])
        if tok[0] == "KW" and tok[1] == "address":
            self.advance()
            return UnaryOp("address", self.parse_unary(), tok[2], tok[3])
        if tok[0] == "KW" and tok[1] == "deref":
            self.advance()
            return UnaryOp("deref", self.parse_unary(), tok[2], tok[3])
        if tok[0] == "KW" and tok[1] == "bit_not":
            self.advance()
            return UnaryOp("bit_not", self.parse_unary(), tok[2], tok[3])
        return self.parse_postfix()

    def parse_postfix(self):
        expr = self.parse_primary()
        while True:
            # function call
            if self.check("LPAREN"):
                tok = self.advance()
                name = expr.name if isinstance(expr, Identifier) else "__expr__"
                # cast(expr, type) — second arg is a type, not an expression
                if name == "cast":
                    cast_expr = self.parse_expr()
                    self.expect("COMMA")
                    cast_type = self.parse_type()
                    self.expect("RPAREN")
                    expr = Cast(cast_expr, cast_type, tok[2], tok[3])
                else:
                    args = []
                    if not self.check("RPAREN"):
                        args.append(self.parse_expr())
                        while self.match("COMMA"):
                            args.append(self.parse_expr())
                    self.expect("RPAREN")
                    expr = Call(name, args, tok[2], tok[3])
            # array index or slice: expr[i] or expr[i:j]
            elif self.check("LBRACKET"):
                tok = self.advance()
                start = self.parse_expr()
                if self.check("COLON"):
                    self.advance()  # consume ':'
                    end = self.parse_expr()
                    self.expect("RBRACKET")
                    expr = SliceExpr(expr, start, end, tok[2], tok[3])
                else:
                    self.expect("RBRACKET")
                    expr = Index(expr, start, tok[2], tok[3])
            # cast: change(x)->type
            elif self.check("OP", "->"):
                tok = self.advance()
                target = self.parse_type()
                expr = Cast(expr, target, tok[2], tok[3])
            # field access or method call: expr.field or expr.method(args)
            elif self.check("DOT"):
                tok = self.advance()
                # Accept both IDENT and KW as field/method names (e.g. sort, reverse, clear, remove)
                if self.check("IDENT"):
                    field_tok = self.advance()
                elif self.check("KW"):
                    field_tok = self.advance()
                else:
                    field_tok = self.expect("IDENT")  # will raise error
                if self.check("LPAREN"):
                    # Method call: expr.method(args)
                    self.advance()
                    args = []
                    if not self.check("RPAREN"):
                        args.append(self.parse_expr())
                        while self.match("COMMA"):
                            args.append(self.parse_expr())
                    self.expect("RPAREN")
                    expr = MethodCall(expr, field_tok[1], args, tok[2], tok[3])
                else:
                    expr = FieldAccess(expr, field_tok[1], tok[2], tok[3])
            # post-increment: x++
            elif self.check("OP", "++"):
                tok = self.advance()
                expr = PostIncrement(expr, tok[2], tok[3])
            # post-decrement: x--
            elif self.check("OP", "--"):
                tok = self.advance()
                expr = PostDecrement(expr, tok[2], tok[3])
            # result propagation: expr?
            elif self.check("OP", "?"):
                tok = self.advance()
                expr = ResultPropagation(expr, tok[2], tok[3])
            else:
                break
        return expr

    def parse_primary(self):
        tok = self.peek()

        if tok[0] == "INT":
            self.advance()
            return IntLiteral(int(tok[1]), tok[2], tok[3])

        if tok[0] == "HEX":
            self.advance()
            return IntLiteral(int(tok[1], 16), tok[2], tok[3])

        if tok[0] == "FLOAT":
            self.advance()
            return FloatLiteral(float(tok[1]), tok[2], tok[3])

        if tok[0] == "STRING":
            self.advance()
            # Lexer already strips quotes and handles escapes
            return StringLiteral(tok[1], tok[2], tok[3])

        if tok[0] == "KW" and tok[1] == "true":
            self.advance(); return BoolLiteral(True, tok[2], tok[3])
        if tok[0] == "KW" and tok[1] == "false":
            self.advance(); return BoolLiteral(False, tok[2], tok[3])
        if tok[0] == "KW" and tok[1] == "empty":
            self.advance(); return EmptyLiteral(tok[2], tok[3])

        if tok[0] == "IDENT":
            # Check for struct literal: TypeName { field = val, ... }
            if tok[1] in self.user_types and self.pos + 1 < len(self.tokens) and self.tokens[self.pos + 1][0] == "LBRACE":
                self.advance()  # consume IDENT
                self.advance()  # consume {
                fields = []
                while not self.check("RBRACE"):
                    fname = self.expect("IDENT")[1]
                    self.expect("OP", "=")
                    fval = self.parse_expr()
                    fields.append((fname, fval))
                    self.match("COMMA")
                self.expect("RBRACE")
                return StructLiteral(tok[1], fields, tok[2], tok[3])
            self.advance()
            return Identifier(tok[1], tok[2], tok[3])

        # Builtin keywords that are callable as functions
        _CALLABLE_KW = {"change", "cast", "memtake", "memgive", "memcopy", "memset",
                        "memmove", "size", "absval", "exit", "range", "typeof",
                        "spawn", "wait", "lock", "unlock",
                        "char_code", "sort",
                        "random", "random_int", "random_seed", "userinput"}
        if tok[0] == "KW" and tok[1] in _CALLABLE_KW:
            self.advance()
            return Identifier(tok[1], tok[2], tok[3])

        if tok[0] == "LBRACKET":
            self.advance()
            if self.check("RBRACKET"):
                self.advance()
                return ArrayLiteral([], tok[2], tok[3])
            # Parse first expression
            first = self.parse_expr()
            # Check for comprehension: [expr for var in iterable]
            if self.check("KW", "for"):
                self.advance()  # consume 'for'
                var = self.expect("IDENT")[1]
                self.expect("KW", "in")
                iterable = self.parse_expr()
                self.expect("RBRACKET")
                return ArrayComprehension(first, var, iterable, tok[2], tok[3])
            # Regular array literal
            elements = [first]
            while self.match("COMMA"):
                elements.append(self.parse_expr())
            self.expect("RBRACKET")
            return ArrayLiteral(elements, tok[2], tok[3])

        if tok[0] == "LPAREN":
            self.advance()
            expr = self.parse_expr()
            self.expect("RPAREN")
            return expr

        raise SyntaxError(
            f"{self.filename}:{tok[2]}:{tok[3]} — unexpected token {tok[0]} '{tok[1]}'"
        )


def parse(tokens, filename="<source>"):
    return Parser(tokens, filename).parse()
