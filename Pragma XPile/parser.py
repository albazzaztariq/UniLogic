"""
Pragma Parser
Recursive-descent parser for .run source files.

Method:  parse(source: str) -> Program
"""

from typing import List, Optional, Any
from lexer import Lexer, Token, TT
from ast_nodes import *


class ParseError(Exception):
    def __init__(self, msg: str, line: int):
        super().__init__(f"Line {line}: {msg}")


class Parser:
    """
    Inputs:  tokens: List[Token]  — output of Lexer.tokenize()
    Outputs: Program              — root AST node
    """

    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    # ── Token helpers ──────────────────────────────────────────────────────────

    def peek(self, offset: int = 0) -> Token:
        i = self.pos + offset
        return self.tokens[i] if i < len(self.tokens) else self.tokens[-1]

    def advance(self) -> Token:
        tok = self.tokens[self.pos]
        if self.pos < len(self.tokens) - 1:
            self.pos += 1
        return tok

    def check(self, *types: TT) -> bool:
        return self.peek().type in types

    def match(self, *types: TT) -> Optional[Token]:
        if self.check(*types):
            return self.advance()
        return None

    def expect(self, tt: TT, msg: str = "") -> Token:
        if self.check(tt):
            return self.advance()
        tok = self.peek()
        raise ParseError(
            msg or f"Expected {tt.name}, got {tok.type.name} ({repr(tok.value)})",
            tok.line,
        )

    def skip_newlines(self):
        while self.check(TT.NEWLINE):
            self.advance()

    # ── Top level ──────────────────────────────────────────────────────────────

    def parse(self) -> Program:
        prog = Program()
        self.skip_newlines()
        while not self.check(TT.EOF):
            if self.check(TT.GLOBAL):
                prog.globals.append(self._global_block())
            elif self.check(TT.MAIN):
                prog.mains.append(self._main_block())
            else:
                tok = self.peek()
                raise ParseError(
                    f"Expected GLOBAL or MAIN, got {tok.type.name} ({repr(tok.value)})",
                    tok.line,
                )
            self.skip_newlines()
        return prog

    def _global_block(self) -> GlobalBlock:
        self.expect(TT.GLOBAL)
        self.skip_newlines()
        body = self._body(TT.END_GLOBAL)
        self.expect(TT.END_GLOBAL)
        self.skip_newlines()
        return GlobalBlock(body=body)

    def _main_block(self) -> MainBlock:
        self.expect(TT.MAIN)
        self.skip_newlines()
        body = self._body(TT.END_MAIN)
        self.expect(TT.END_MAIN)
        self.skip_newlines()
        return MainBlock(body=body)

    def _body(self, *end_tokens: TT) -> List[Any]:
        stmts = []
        while not self.check(*end_tokens) and not self.check(TT.EOF):
            if self.check(TT.NEWLINE):
                self.advance()
                continue
            stmts.append(self._statement())
        return stmts

    # ── Statement dispatch ────────────────────────────────────────────────────

    def _statement(self) -> Any:
        tok = self.peek()

        if tok.type == TT.IMPORT:      return self._import_decl()
        if tok.type == TT.CONSTANT:    return self._constant_decl()
        if tok.type == TT.TYPE:        return self._type_var_decl()
        if tok.type == TT.FUNCTION:    return self._func_decl()
        if tok.type == TT.OBJECT:      return self._obj_decl()
        if tok.type == TT.INTERFACE:   return self._interface_decl()
        if tok.type == TT.MANDATE:     return self._mandate_decl()
        if tok.type == TT.RETURN:      return self._return_stmt()
        if tok.type == TT.PRINT:       return self._print_stmt()
        if tok.type == TT.IF:          return self._if_stmt()
        if tok.type == TT.FOR:         return self._for_stmt()
        if tok.type == TT.FOR_EACH:    return self._foreach_stmt()
        if tok.type == TT.WHILE:       return self._while_stmt()
        if tok.type == TT.TRY:         return self._try_stmt()
        if tok.type == TT.CONDITIONS:  return self._conditions_stmt()
        if tok.type == TT.ESCAPE:      self.advance(); self.match(TT.NEWLINE); return EscapeStmt()
        if tok.type == TT.SKIP:        self.advance(); self.match(TT.NEWLINE); return SkipStmt()
        if tok.type == TT.NEW:
            s = self._new_expr(); self.match(TT.NEWLINE); return s
        if tok.type == TT.RAW:         return self._raw_block()
        if tok.type == TT.POINTER:     return self._pointer_decl()

        if tok.type == TT.IDENT:
            if tok.value == 'array':
                self.advance(); return self._array_decl()
            if tok.value == 'List':
                self.advance(); return self._list_decl()
            if tok.value == 'Map':
                self.advance(); return self._map_decl()
            # User-defined type variable:  TypeName varName [= expr]
            if self.peek(1).type == TT.IDENT:
                type_name = self.advance().value
                return self._var_decl(type_name)
            return self._expr_stmt()

        raise ParseError(
            f"Unexpected token: {tok.type.name} ({repr(tok.value)})",
            tok.line,
        )

    # ── Declaration parsers ───────────────────────────────────────────────────

    def _import_decl(self) -> ImportDecl:
        self.expect(TT.IMPORT)
        name = self.expect(TT.IDENT, "Expected module name after 'import'")
        self.match(TT.NEWLINE)
        return ImportDecl(module=name.value)

    def _constant_decl(self) -> ConstantDecl:
        self.expect(TT.CONSTANT)
        type_tok = self.expect(TT.TYPE, "Expected type after 'constant'")
        name = self.expect(TT.IDENT, "Expected constant name")
        self.expect(TT.ASSIGN, "Expected '=' in constant declaration")
        init = self._expr()
        self.match(TT.NEWLINE)
        return ConstantDecl(type_name=type_tok.value, name=name.value, init=init)

    def _type_var_decl(self) -> VarDecl:
        type_tok = self.advance()
        return self._var_decl(type_tok.value)

    def _var_decl(self, type_name: str) -> VarDecl:
        name = self.expect(TT.IDENT, f"Expected variable name after type '{type_name}'")
        init = None
        if self.match(TT.ASSIGN):
            init = self._expr()
        self.match(TT.NEWLINE)
        return VarDecl(type_name=type_name, name=name.value, init=init)

    def _array_decl(self) -> ArrayDecl:
        self.expect(TT.LBRACKET, "Expected '[' in array[Type]")
        elem = self.expect(TT.TYPE, "Expected element type")
        self.expect(TT.RBRACKET, "Expected ']' in array[Type]")
        name = self.expect(TT.IDENT, "Expected array name")
        init: List[Any] = []
        if self.match(TT.ASSIGN):
            init = self._bracket_list()
        self.match(TT.NEWLINE)
        return ArrayDecl(elem_type=elem.value, name=name.value, init=init)

    def _list_decl(self) -> ListDecl:
        self.expect(TT.LBRACKET, "Expected '[' in List[Type]")
        elem = self.expect(TT.TYPE, "Expected element type")
        self.expect(TT.RBRACKET, "Expected ']' in List[Type]")
        name = self.expect(TT.IDENT, "Expected list name")
        init: List[Any] = []
        if self.match(TT.ASSIGN):
            init = self._bracket_list()
        self.match(TT.NEWLINE)
        return ListDecl(elem_type=elem.value, name=name.value, init=init)

    def _map_decl(self):
        # Map[KeyType, ValType] name = {}  — placeholder, returns VarDecl for now
        self.expect(TT.LBRACKET)
        key = self.expect(TT.TYPE, "Expected key type").value
        self.expect(TT.COMMA)
        val = self.expect(TT.TYPE, "Expected value type").value
        self.expect(TT.RBRACKET)
        name = self.expect(TT.IDENT, "Expected map name").value
        self.match(TT.ASSIGN)
        self.match(TT.NEWLINE)
        return VarDecl(type_name=f'Map[{key},{val}]', name=name, init=None)

    def _func_decl(self) -> FunctionDecl:
        self.expect(TT.FUNCTION)
        name = self.expect(TT.IDENT, "Expected function name")
        params: List[Param] = []
        if self.match(TT.LPAREN):
            params = self._param_list()
            self.expect(TT.RPAREN, "Expected ')' after parameters")
        ret = 'void'
        if self.match(TT.RETURNS):
            ret_tok = self.expect(TT.TYPE, "Expected return type after 'returns'")
            ret = ret_tok.value
        self.skip_newlines()
        body = self._body(TT.END_FUNCTION)
        self.expect(TT.END_FUNCTION)
        self.match(TT.NEWLINE)
        return FunctionDecl(name=name.value, return_type=ret, params=params, body=body)

    def _param_list(self) -> List[Param]:
        params: List[Param] = []
        if self.check(TT.RPAREN):
            return params
        params.append(self._param())
        while self.match(TT.COMMA):
            params.append(self._param())
        return params

    def _param(self) -> Param:
        type_tok = self.expect(TT.TYPE, "Expected parameter type")
        name_tok = self.expect(TT.IDENT, "Expected parameter name")
        return Param(type_name=type_tok.value, name=name_tok.value)

    def _obj_decl(self) -> ObjectDecl:
        self.expect(TT.OBJECT)
        name = self.expect(TT.IDENT, "Expected object name")
        parent = None
        ifaces: List[str] = []
        # inherits ParentName
        if self.match(TT.INHERITS):
            parent = self.expect(TT.IDENT, "Expected parent object name after 'inherits'").value
        # interfaces A, B, C
        if self.match(TT.INTERFACES):
            ifaces.append(self.expect(TT.IDENT, "Expected interface name").value)
            while self.match(TT.COMMA):
                ifaces.append(self.expect(TT.IDENT, "Expected interface name").value)
        self.skip_newlines()
        fields: List[ObjectField] = []
        methods: List[FunctionDecl] = []
        while not self.check(TT.END_OBJECT, TT.EOF):
            if self.check(TT.NEWLINE):
                self.advance(); continue
            if self.check(TT.FUNCTION):
                methods.append(self._func_decl())
            elif self.check(TT.TYPE):
                fields.append(self._obj_field())
            else:
                tok = self.peek()
                raise ParseError(
                    f"Expected field or method in object, got {tok.type.name}",
                    tok.line,
                )
        self.expect(TT.END_OBJECT)
        self.match(TT.NEWLINE)
        return ObjectDecl(name=name.value, parent=parent, ifaces=ifaces,
                          fields=fields, methods=methods)

    def _obj_field(self) -> ObjectField:
        type_tok = self.advance()
        name_tok = self.expect(TT.IDENT, "Expected field name")
        default = None
        if self.match(TT.ASSIGN):
            default = self._expr()
        self.match(TT.NEWLINE)
        return ObjectField(type_name=type_tok.value, name=name_tok.value, default=default)

    def _interface_decl(self) -> InterfaceDecl:
        self.expect(TT.INTERFACE)
        name = self.expect(TT.IDENT, "Expected interface name")
        self.skip_newlines()
        methods: List[FunctionDecl] = []
        while not self.check(TT.END_INTERFACE, TT.EOF):
            if self.check(TT.NEWLINE):
                self.advance(); continue
            if self.check(TT.FUNCTION):
                methods.append(self._func_sig())   # signature only, no body
            else:
                tok = self.peek()
                raise ParseError(f"Expected method signature in interface, got {tok.type.name}", tok.line)
        self.expect(TT.END_INTERFACE)
        self.match(TT.NEWLINE)
        return InterfaceDecl(name=name.value, methods=methods)

    def _func_sig(self) -> FunctionDecl:
        """Parse a function signature with no body (for interface declarations)."""
        self.expect(TT.FUNCTION)
        name = self.expect(TT.IDENT, "Expected function name")
        params: List[Param] = []
        if self.match(TT.LPAREN):
            params = self._param_list()
            self.expect(TT.RPAREN)
        ret = 'void'
        if self.match(TT.RETURNS):
            ret_tok = self.expect(TT.TYPE, "Expected return type")
            ret = ret_tok.value
        self.match(TT.NEWLINE)
        return FunctionDecl(name=name.value, return_type=ret, params=params, body=[])

    def _mandate_decl(self) -> MandateDecl:
        """mandate Priority low = 1, medium = 2, high = 3 end mandate
           OR multiline version"""
        self.expect(TT.MANDATE)
        name = self.expect(TT.IDENT, "Expected mandate (enum) name")
        members: List[MandateMember] = []
        # Skip optional newline after name
        if self.check(TT.NEWLINE):
            self.skip_newlines()
            # Multiline: parse members until END_MANDATE
            while not self.check(TT.END_MANDATE, TT.EOF):
                if self.check(TT.NEWLINE):
                    self.advance(); continue
                members.append(self._mandate_member())
                self.match(TT.COMMA)   # optional trailing comma
        else:
            # Inline: parse comma-separated members on same line
            members.append(self._mandate_member())
            while self.match(TT.COMMA):
                if self.check(TT.END_MANDATE, TT.NEWLINE, TT.EOF):
                    break
                members.append(self._mandate_member())
        self.expect(TT.END_MANDATE)
        self.match(TT.NEWLINE)
        return MandateDecl(name=name.value, members=members)

    def _mandate_member(self) -> MandateMember:
        name = self.expect(TT.IDENT, "Expected mandate member name")
        value = None
        if self.match(TT.ASSIGN):
            val_tok = self.expect(TT.INT_LIT, "Expected integer value in mandate member")
            value = int(val_tok.value, 0)
        return MandateMember(name=name.value, value=value)

    # ── Statement parsers ─────────────────────────────────────────────────────

    def _return_stmt(self) -> ReturnStmt:
        self.expect(TT.RETURN)
        value = None
        if not self.check(TT.NEWLINE, TT.EOF):
            value = self._expr()
        self.match(TT.NEWLINE)
        return ReturnStmt(value=value)

    def _print_stmt(self) -> PrintStmt:
        self.expect(TT.PRINT)
        value = self._expr()
        self.match(TT.NEWLINE)
        return PrintStmt(value=value)

    def _if_stmt(self) -> IfStmt:
        self.expect(TT.IF)
        cond = self._expr()
        self.skip_newlines()
        body = self._body(TT.ELSE_IF, TT.ELSE, TT.END_IF)
        elifs: List[ElifClause] = []
        else_body = None
        while self.check(TT.ELSE_IF):
            self.advance()
            elif_cond = self._expr()
            self.skip_newlines()
            elif_body = self._body(TT.ELSE_IF, TT.ELSE, TT.END_IF)
            elifs.append(ElifClause(condition=elif_cond, body=elif_body))
        if self.check(TT.ELSE):
            self.advance()
            self.skip_newlines()
            else_body = self._body(TT.END_ELSE, TT.END_IF)
            self.match(TT.END_ELSE)
            self.skip_newlines()
        self.expect(TT.END_IF)
        self.match(TT.NEWLINE)
        return IfStmt(condition=cond, body=body, elif_clauses=elifs, else_body=else_body)

    def _for_stmt(self) -> ForStmt:
        """for int i = 0 while i < 10 do i++"""
        self.expect(TT.FOR)
        type_tok = self.expect(TT.TYPE, "Expected type in for-loop init")
        name_tok = self.expect(TT.IDENT, "Expected variable name")
        self.expect(TT.ASSIGN, "Expected '='")
        start_val = self._expr()
        init = VarDecl(type_name=type_tok.value, name=name_tok.value, init=start_val)
        self.expect(TT.WHILE, "Expected 'while'")
        cond = self._expr()
        self.expect(TT.DO, "Expected 'do'")
        step = self._step_expr()
        self.skip_newlines()
        body = self._body(TT.END_FOR)
        self.expect(TT.END_FOR)
        self.match(TT.NEWLINE)
        return ForStmt(init=init, condition=cond, step=step, body=body)

    def _step_expr(self) -> Any:
        name_tok = self.expect(TT.IDENT, "Expected variable in step")
        ident = Identifier(name=name_tok.value)
        if self.match(TT.INCREMENT):
            return PostfixOp(op='++', operand=ident)
        if self.match(TT.DECREMENT):
            return PostfixOp(op='--', operand=ident)
        for tt, op in [(TT.PLUS_ASSIGN, '+='), (TT.MINUS_ASSIGN, '-='),
                       (TT.STAR_ASSIGN,  '*='), (TT.SLASH_ASSIGN, '/=')]:
            if self.match(tt):
                return AssignStmt(target=ident, op=op, value=self._expr())
        raise ParseError("Expected ++, --, or compound-assign in step", name_tok.line)

    def _foreach_stmt(self) -> ForEachStmt:
        self.expect(TT.FOR_EACH)
        var = self.expect(TT.IDENT, "Expected variable in 'for each'")
        self.expect(TT.IN, "Expected 'in'")
        iterable = self._expr()
        self.skip_newlines()
        body = self._body(TT.END_FOR)
        self.expect(TT.END_FOR)
        self.match(TT.NEWLINE)
        return ForEachStmt(var=var.value, iterable=iterable, body=body)

    def _while_stmt(self) -> WhileStmt:
        """while condition do ... end do"""
        self.expect(TT.WHILE)
        cond = self._expr()
        self.expect(TT.DO, "Expected 'do' after while condition")
        self.skip_newlines()
        body = self._body(TT.END_DO)
        self.expect(TT.END_DO)
        self.match(TT.NEWLINE)
        return WhileStmt(condition=cond, body=body)

    def _try_stmt(self) -> TryStmt:
        self.expect(TT.TRY)
        self.skip_newlines()
        body = self._body(TT.CATCH, TT.END_TRY)
        catches: List[CatchClause] = []
        while self.check(TT.CATCH):
            catches.append(self._catch_clause())
        self.expect(TT.END_TRY)
        self.match(TT.NEWLINE)
        return TryStmt(body=body, catches=catches)

    def _catch_clause(self) -> CatchClause:
        self.expect(TT.CATCH)
        self.expect(TT.LPAREN)
        exceptions = [self.expect(TT.IDENT, "Expected exception type").value]
        while self.match(TT.OR):
            exceptions.append(self.expect(TT.IDENT, "Expected exception type").value)
        var = self.expect(TT.IDENT, "Expected exception variable")
        self.expect(TT.RPAREN)
        self.skip_newlines()
        body = self._body(TT.END_CATCH, TT.CATCH, TT.END_TRY)
        self.expect(TT.END_CATCH)
        self.match(TT.NEWLINE)
        return CatchClause(exceptions=exceptions, var=var.value, body=body)

    def _conditions_stmt(self) -> ConditionsStmt:
        """conditions x
             1: expr
             5
               statements
               escape
             default: expr
           end conditions"""
        self.expect(TT.CONDITIONS)
        expr = self._expr()
        self.skip_newlines()
        cases: List[ConditionsCase] = []
        while not self.check(TT.END_CONDITIONS, TT.EOF):
            if self.check(TT.NEWLINE):
                self.advance(); continue
            cases.append(self._conditions_case())
        self.expect(TT.END_CONDITIONS)
        self.match(TT.NEWLINE)
        return ConditionsStmt(expr=expr, cases=cases)

    def _is_case_start(self) -> bool:
        """True if current position starts a new case header."""
        if self.check(TT.DEFAULT, TT.END_CONDITIONS, TT.EOF):
            return True
        if self.check(TT.INT_LIT, TT.STRING_LIT, TT.FLOAT_LIT, TT.BOOL_LIT):
            # Next token is colon or newline → it's a case header, not an expression
            nxt = self.peek(1).type
            return nxt in (TT.COLON, TT.NEWLINE, TT.EOF)
        return False

    def _conditions_case(self) -> ConditionsCase:
        is_default = False
        value = None
        if self.match(TT.DEFAULT):
            is_default = True
        else:
            value = self._primary_expr()   # simple literal

        if self.match(TT.COLON):
            # Single-statement case:  1: print "one"  or  1: escape
            body_stmt = self._statement()
            return ConditionsCase(value=value, is_default=is_default, body=[body_stmt])
        else:
            # Multi-statement case
            self.skip_newlines()
            body: List[Any] = []
            while not self._is_case_start() and not self.check(TT.EOF):
                if self.check(TT.NEWLINE):
                    self.advance(); continue
                body.append(self._statement())
            return ConditionsCase(value=value, is_default=is_default, body=body)

    def _new_expr(self) -> NewStmt:
        self.expect(TT.NEW)
        type_name = self.expect(TT.IDENT, "Expected type name after 'new'")
        args: List[Any] = []
        if self.match(TT.LBRACKET):
            args = self._arg_list()
            self.expect(TT.RBRACKET)
        return NewStmt(type_name=type_name.value, args=args)

    def _raw_block(self) -> RawBlock:
        self.expect(TT.RAW)
        self.skip_newlines()
        body: List[Any] = []
        while not self.check(TT.END_RAW, TT.EOF):
            if self.check(TT.NEWLINE):
                self.advance(); continue
            body.append(self._raw_statement())
        self.expect(TT.END_RAW)
        self.match(TT.NEWLINE)
        return RawBlock(body=body)

    def _raw_statement(self) -> Any:
        tok = self.peek()
        if tok.type == TT.POINTER:  return self._pointer_decl()
        if tok.type == TT.MEM:      return self._mem_access_stmt()
        if tok.type == TT.DEREF:    return self._deref_stmt()
        return self._statement()

    def _pointer_decl(self) -> PointerDecl:
        self.expect(TT.POINTER)
        type_tok = self.expect(TT.TYPE, "Expected type after 'pointer'")
        name = self.expect(TT.IDENT, "Expected pointer name")
        self.expect(TT.ASSIGN)
        target = self._address_of()
        self.match(TT.NEWLINE)
        return PointerDecl(type_name=type_tok.value, name=name.value, target=target)

    def _address_of(self) -> AddressOf:
        self.expect(TT.ADDRESS)
        self.expect(TT.LANGLE)
        target = self.expect(TT.IDENT, "Expected variable in ADDRESS<>")
        self.expect(TT.RANGLE)
        return AddressOf(target=target.value)

    def _mem_access_stmt(self) -> MemAccess:
        self.expect(TT.MEM)
        self.expect(TT.LANGLE)
        addr = self._expr()
        self.expect(TT.RANGLE)
        self.match(TT.NEWLINE)
        return MemAccess(address=addr)

    def _deref_stmt(self) -> DerefExpr:
        self.expect(TT.DEREF)
        self.expect(TT.LANGLE)
        ptr = self._expr()
        self.expect(TT.RANGLE)
        self.match(TT.NEWLINE)
        return DerefExpr(pointer=ptr)

    def _expr_stmt(self) -> Any:
        expr = self._postfix_expr()
        for tt, op in [
            (TT.ASSIGN,        '='),
            (TT.PLUS_ASSIGN,  '+='),
            (TT.MINUS_ASSIGN, '-='),
            (TT.STAR_ASSIGN,  '*='),
            (TT.SLASH_ASSIGN, '/='),
        ]:
            if self.match(tt):
                value = self._expr()
                self.match(TT.NEWLINE)
                return AssignStmt(target=expr, op=op, value=value)
        self.match(TT.NEWLINE)
        return expr

    # ── Expression hierarchy ──────────────────────────────────────────────────
    # Precedence (low → high):
    #   or | and | equality | delta(xor) | comparison | shift(left/right)
    #   | additive | multiplicative | unary(!/- /flip) | postfix | call | primary

    def _expr(self) -> Any:
        return self._or_expr()

    def _or_expr(self) -> Any:
        left = self._and_expr()
        while self.check(TT.OR):
            op = self.advance().value
            left = BinOp(op=op, left=left, right=self._and_expr())
        return left

    def _and_expr(self) -> Any:
        left = self._equality_expr()
        while self.check(TT.AND):
            op = self.advance().value
            left = BinOp(op=op, left=left, right=self._equality_expr())
        return left

    def _equality_expr(self) -> Any:
        left = self._delta_expr()
        while self.check(TT.EQ, TT.NEQ):
            op = self.advance().value
            left = BinOp(op=op, left=left, right=self._delta_expr())
        return left

    def _delta_expr(self) -> Any:
        """XOR:  a delta b"""
        left = self._comparison_expr()
        while self.check(TT.DELTA):
            self.advance()
            left = BinOp(op='delta', left=left, right=self._comparison_expr())
        return left

    def _comparison_expr(self) -> Any:
        left = self._shift_expr()
        while self.check(TT.LANGLE, TT.RANGLE, TT.LTE, TT.GTE):
            op = self.advance().value
            left = BinOp(op=op, left=left, right=self._shift_expr())
        return left

    def _shift_expr(self) -> Any:
        """a left n  /  a right n"""
        left = self._additive_expr()
        while self.check(TT.LEFT, TT.RIGHT):
            op = self.advance().value   # 'left' or 'right'
            left = BinOp(op=op, left=left, right=self._additive_expr())
        return left

    def _additive_expr(self) -> Any:
        left = self._multiplicative_expr()
        while self.check(TT.PLUS, TT.MINUS):
            op = self.advance().value
            left = BinOp(op=op, left=left, right=self._multiplicative_expr())
        return left

    def _multiplicative_expr(self) -> Any:
        left = self._unary_expr()
        while self.check(TT.STAR, TT.SLASH, TT.MODULO):
            op = self.advance().value
            left = BinOp(op=op, left=left, right=self._unary_expr())
        return left

    def _unary_expr(self) -> Any:
        if self.check(TT.NOT):
            op = self.advance().value
            return UnaryOp(op=op, operand=self._unary_expr())
        if self.check(TT.MINUS):
            op = self.advance().value
            return UnaryOp(op=op, operand=self._unary_expr())
        if self.check(TT.FLIP):
            self.advance()
            return UnaryOp(op='flip', operand=self._unary_expr())
        return self._postfix_expr()

    def _postfix_expr(self) -> Any:
        expr = self._call_expr()
        if self.match(TT.INCREMENT):
            return PostfixOp(op='++', operand=expr)
        if self.match(TT.DECREMENT):
            return PostfixOp(op='--', operand=expr)
        return expr

    def _call_expr(self) -> Any:
        expr = self._primary_expr()
        while True:
            if self.check(TT.DOT):
                self.advance()
                field = self.expect(TT.IDENT, "Expected field name after '.'")
                expr = FieldAccess(obj=expr, field=field.value)
            elif self.check(TT.LPAREN):
                self.advance()
                args = self._arg_list()
                self.expect(TT.RPAREN)
                expr = CallExpr(func=expr, args=args)
            elif self.check(TT.LBRACKET):
                # arr[i] — subscript / index expression
                self.advance()
                index = self._expr()
                self.expect(TT.RBRACKET, "Expected ']' after index expression")
                expr = IndexExpr(array=expr, index=index)
            else:
                break
        return expr

    def _arg_list(self) -> List[Any]:
        args: List[Any] = []
        if self.check(TT.RPAREN, TT.RBRACKET):
            return args
        args.append(self._expr())
        while self.match(TT.COMMA):
            args.append(self._expr())
        return args

    def _bracket_list(self) -> List[Any]:
        self.expect(TT.LBRACKET)
        items: List[Any] = []
        if not self.check(TT.RBRACKET):
            items.append(self._expr())
            while self.match(TT.COMMA):
                items.append(self._expr())
        self.expect(TT.RBRACKET)
        return items

    def _primary_expr(self) -> Any:
        tok = self.peek()

        if tok.type == TT.INT_LIT:
            self.advance(); return IntLit(value=int(tok.value, 0))
        if tok.type == TT.FLOAT_LIT:
            self.advance(); return FloatLit(value=float(tok.value))
        if tok.type == TT.STRING_LIT:
            self.advance(); return StringLit(value=tok.value)
        if tok.type == TT.BOOL_LIT:
            self.advance(); return BoolLit(value=tok.value == 'true')
        if tok.type == TT.NULL:
            self.advance(); return NullLit()
        if tok.type == TT.EMPTY:
            self.advance(); return EmptyLit()
        if tok.type == TT.SUPER:
            self.advance(); return SuperRef()
        if tok.type == TT.IDENT:
            self.advance(); return Identifier(name=tok.value)
        if tok.type == TT.LPAREN:
            self.advance()
            expr = self._expr()
            self.expect(TT.RPAREN)
            return expr
        if tok.type == TT.NEW:
            return self._new_expr()

        # change(x)->type  — cast expression
        if tok.type == TT.CHANGE:
            self.advance()
            self.expect(TT.LPAREN)
            expr = self._expr()
            self.expect(TT.RPAREN)
            self.expect(TT.ARROW, "Expected '->' after change(...)")
            # Accept built-in type or user-defined type name
            if self.check(TT.TYPE):
                target = self.advance().value
            else:
                target = self.expect(TT.IDENT, "Expected type name after '->'").value
            return CastExpr(expr=expr, target_type=target)

        # Memory expressions
        if tok.type == TT.SIZE:
            self.advance()
            self.expect(TT.LANGLE)
            type_tok = self.expect(TT.TYPE, "Expected type in SIZE<>")
            self.expect(TT.RANGLE)
            return SizeOf(type_name=type_tok.value)
        if tok.type == TT.MEM:
            self.advance()
            self.expect(TT.LANGLE)
            addr = self._expr()
            self.expect(TT.RANGLE)
            return MemAccess(address=addr)
        if tok.type == TT.DEREF:
            self.advance()
            self.expect(TT.LANGLE)
            ptr = self._expr()
            self.expect(TT.RANGLE)
            return DerefExpr(pointer=ptr)

        raise ParseError(
            f"Unexpected token in expression: {tok.type.name} ({repr(tok.value)})",
            tok.line,
        )


# ── Public API ─────────────────────────────────────────────────────────────────

def parse(source: str) -> Program:
    """Lex and parse a Pragma .run source string into the root Program AST."""
    tokens = Lexer(source).tokenize()
    return Parser(tokens).parse()
