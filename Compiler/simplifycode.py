# ulc/normalize.py — UniLogic AST normalization pass
# Sits between the parser and semantic checker in the pipeline.
# Rewrites the AST to simplify complex expressions into flat sequences
# of simple statements. This makes later analysis and codegen easier.
#
# Normalization levels:
#   0: no normalization
#   1: compound assignments + increment/decrement
#   2: level 1 + nested call flattening + chained cast expansion
#   3: level 2 + boolean expression splitting + array initializer expansion
#
# Per-file override:  @norm 2  (first-class syntax)
# Legacy fallback:    // @norm 2  (comment form, still accepted)

import re
from ast_nodes import *


def detect_norm_level(program, source=None):
    """Check for a @norm directive in the AST. Falls back to scanning
    source text for a legacy // @norm N comment. Returns int or None."""
    # First-class: look for NormDirective in the AST
    if program is not None:
        for decl in program.decls:
            if isinstance(decl, NormDirective):
                return decl.level

    # Legacy fallback: scan source comments (first 20 lines)
    if source is not None:
        for line in source.splitlines()[:20]:
            m = re.match(r'^\s*//\s*@norm\s+(\d+)', line)
            if m:
                return int(m.group(1))

    return None


class Normalizer:
    def __init__(self, level=2):
        self._tmp_counter = 0
        self.level = level

    def _fresh(self, line=0, col=0):
        """Generate a fresh temporary variable name."""
        name = f"_t{self._tmp_counter}"
        self._tmp_counter += 1
        return name

    # ── Entry point ──────────────────────────────────────────────────────

    def normalize(self, program):
        if self.level == 0:
            return program
        new_decls = []
        for decl in program.decls:
            if isinstance(decl, FunctionDecl):
                decl.body = self.norm_body(decl.body)
                for nf in getattr(decl, 'nested_functions', []):
                    nf.body = self.norm_body(nf.body)
            new_decls.append(decl)
        program.decls = new_decls
        return program

    # ── Body / statement list ────────────────────────────────────────────

    def norm_body(self, stmts):
        out = []
        for stmt in stmts:
            out.extend(self.norm_stmt(stmt))
        return out

    def norm_stmt(self, node):
        """Normalize a statement. Returns a list of statements (may expand to multiple)."""
        t = type(node)

        if t == NormDirective:
            # Inline @norm N changes the active normalization level
            self.level = node.level
            return []  # directive itself is consumed, not emitted

        if t == VarDecl:
            if node.init is not None:
                pre, node.init = self.flatten_expr(node.init, node.line, node.col)
                # Level 3: expand complex array initializers element by element
                if (self.level >= 3 and isinstance(node.init, ArrayLiteral)
                        and node.type_.is_array and len(node.init.elements) > 0):
                    return pre + self.expand_array_init(node)
                return pre + [node]
            return [node]

        if t == Assign:
            # Level 1+: compound assignments → explicit form
            if self.level >= 1 and node.op != "=":
                return self.expand_compound_assign(node)
            pre, node.value = self.flatten_expr(node.value, node.line, node.col)
            return pre + [node]

        if t == If:
            pre, node.condition = self.flatten_expr(node.condition, node.line, node.col)
            # Level 3: split multi-condition booleans in if conditions
            if self.level >= 3:
                extra, node.condition = self.split_boolean(node.condition)
                pre.extend(extra)
            node.then_body = self.norm_body(node.then_body)
            node.else_body = self.norm_body(node.else_body)
            return pre + [node]

        if t == While:
            # Don't hoist condition expressions out of the loop — they re-evaluate each iteration
            node.body = self.norm_body(node.body)
            return [node]

        if t == For:
            pre, node.iterable = self.flatten_expr(node.iterable, node.line, node.col)
            node.body = self.norm_body(node.body)
            return pre + [node]

        if t == Return:
            if node.value is not None:
                pre, node.value = self.flatten_expr(node.value, node.line, node.col)
                return pre + [node]
            return [node]

        if t == Print:
            pre, node.value = self.flatten_expr(node.value, node.line, node.col)
            return pre + [node]

        if t == ExprStmt:
            # Level 1+: standalone post-increment/decrement
            if self.level >= 1:
                if isinstance(node.expr, PostIncrement):
                    return self.expand_post_inc_dec(node.expr, standalone=True)
                if isinstance(node.expr, PostDecrement):
                    return self.expand_post_inc_dec(node.expr, standalone=True)
            pre, node.expr = self.flatten_expr(node.expr, node.line, node.col)
            return pre + [node]

        # Escape, Continue — pass through
        return [node]

    # ── Expression flattening ────────────────────────────────────────────
    # Returns (pre_stmts, simplified_expr)
    # pre_stmts are VarDecl statements that must execute before the expression.
    # simplified_expr has nested calls / chained casts extracted out.

    def flatten_expr(self, node, line=0, col=0):
        """Flatten complex sub-expressions into temporaries.
        Returns (list_of_preceding_stmts, simplified_expr)."""
        t = type(node)

        if t in (IntLiteral, FloatLiteral, StringLiteral, BoolLiteral,
                 EmptyLiteral, Identifier, ArrayComprehension):
            return [], node

        if t == BinaryOp:
            pre_l, node.left = self.flatten_expr(node.left, node.line, node.col)
            pre_r, node.right = self.flatten_expr(node.right, node.line, node.col)
            return pre_l + pre_r, node

        if t == UnaryOp:
            pre, node.operand = self.flatten_expr(node.operand, node.line, node.col)
            return pre, node

        if t == Call:
            # Level 2+: flatten nested calls
            if self.level >= 2:
                return self.flatten_call(node)
            return [], node

        if t == Cast:
            # Level 2+: flatten chained casts
            if self.level >= 2:
                return self.flatten_cast(node)
            return [], node

        if t == Index:
            pre_t, node.target = self.flatten_expr(node.target, node.line, node.col)
            pre_i, node.index = self.flatten_expr(node.index, node.line, node.col)
            return pre_t + pre_i, node

        if t == ArrayLiteral:
            all_pre = []
            new_elems = []
            for elem in node.elements:
                pre, flat = self.flatten_expr(elem, node.line, node.col)
                all_pre.extend(pre)
                new_elems.append(flat)
            node.elements = new_elems
            return all_pre, node

        if t == PostIncrement or t == PostDecrement:
            # Level 1+: expand post-inc/dec in expressions
            if self.level >= 1:
                return self.expand_post_inc_dec(node, standalone=False)
            return [], node

        return [], node

    # ── Case 1: Nested function calls → named temporaries ────────────────
    # Level 2+

    def flatten_call(self, node):
        """foo(bar(x), baz(y)) → _t0 = bar(x); _t1 = baz(y); foo(_t0, _t1)"""
        all_pre = []
        new_args = []
        for arg in node.args:
            pre, flat_arg = self.flatten_expr(arg, node.line, node.col)
            all_pre.extend(pre)
            # If the flattened arg is itself a call, extract it to a temp
            if isinstance(flat_arg, Call):
                tmp_name = self._fresh(node.line, node.col)
                tmp_decl = VarDecl(
                    TypeName("auto"), tmp_name, flat_arg,
                    fixed=False, line=node.line, col=node.col
                )
                all_pre.append(tmp_decl)
                new_args.append(Identifier(tmp_name, node.line, node.col))
            else:
                new_args.append(flat_arg)
        node.args = new_args
        return all_pre, node

    # ── Case 2: Compound assignment → explicit form ──────────────────────
    # Level 1+

    def expand_compound_assign(self, node):
        """x += expr → x = x + expr"""
        op_map = {
            "+=": "+", "-=": "-", "*=": "*", "/=": "/", "%=": "%",
            "left=": "<<", "right=": ">>",
        }
        bin_op = op_map.get(node.op, "+")

        # Flatten the RHS first
        pre, flat_val = self.flatten_expr(node.value, node.line, node.col)

        new_value = BinaryOp(
            bin_op,
            node.target,     # read the target
            flat_val,
            node.line, node.col
        )
        return pre + [Assign(node.target, "=", new_value, node.line, node.col)]

    # ── Case 3: Chained casts → separate steps ──────────────────────────
    # Level 2+

    def flatten_cast(self, node):
        """change(change(x)->int)->float → _t0 = change(x)->int; change(_t0)->float"""
        # Flatten the inner expression first
        pre, flat_inner = self.flatten_expr(node.expr, node.line, node.col)

        # If the inner expression is itself a cast, extract to temp
        if isinstance(flat_inner, Cast):
            tmp_name = self._fresh(node.line, node.col)
            tmp_decl = VarDecl(
                TypeName("auto"), tmp_name, flat_inner,
                fixed=False, line=node.line, col=node.col
            )
            pre.append(tmp_decl)
            node.expr = Identifier(tmp_name, node.line, node.col)
        # Also detect change(Cast)->type pattern: the inner is Call("change", [Cast(...)])
        elif (isinstance(flat_inner, Call) and flat_inner.name == "change"
              and len(flat_inner.args) == 1 and isinstance(flat_inner.args[0], Cast)):
            inner_cast = flat_inner.args[0]
            tmp_name = self._fresh(node.line, node.col)
            tmp_decl = VarDecl(
                TypeName("auto"), tmp_name, inner_cast,
                fixed=False, line=node.line, col=node.col
            )
            pre.append(tmp_decl)
            # Replace the inner cast arg with the temp identifier
            flat_inner.args[0] = Identifier(tmp_name, node.line, node.col)
            node.expr = flat_inner
        else:
            node.expr = flat_inner

        return pre, node

    # ── Case 4: Post-increment/decrement ─────────────────────────────────
    # Level 1+

    def expand_post_inc_dec(self, node, standalone=False):
        """
        Standalone: x++ → x = x + 1
        In expression: y = x++ → _t0 = x; x = x + 1; (use _t0)
        """
        is_inc = isinstance(node, PostIncrement)
        op = "+" if is_inc else "-"
        operand = node.operand
        line, col = node.line, node.col

        if standalone:
            # Just x = x + 1 (or x = x - 1)
            new_val = BinaryOp(op, operand, IntLiteral(1, line, col), line, col)
            return [Assign(operand, "=", new_val, line, col)]
        else:
            # In an expression context: save old value, then increment
            tmp_name = self._fresh(line, col)
            # auto _t = x
            save = VarDecl(
                TypeName("auto"), tmp_name, operand,
                fixed=False, line=line, col=col
            )
            # x = x + 1
            inc = Assign(
                operand, "=",
                BinaryOp(op, operand, IntLiteral(1, line, col), line, col),
                line, col
            )
            return [save, inc], Identifier(tmp_name, line, col)

    # ── Case 5: Split multi-condition boolean expressions ─────────────────
    # Level 3

    def split_boolean(self, node):
        """
        a and b and c → _t0 = a and b; _t0 and c
        a or b or c   → _t0 = a or b; _t0 or c
        Only splits when there are 2+ chained and/or ops (3+ operands).
        """
        if not isinstance(node, BinaryOp) or node.op not in ("and", "or"):
            return [], node

        # Check if this is a chain: left side is same op
        if not (isinstance(node.left, BinaryOp) and node.left.op == node.op):
            return [], node

        # Extract left sub-expression into a temp
        pre, flat_left = self.split_boolean(node.left)
        tmp_name = self._fresh(node.line, node.col)
        tmp_decl = VarDecl(
            TypeName("auto"), tmp_name, flat_left,
            fixed=False, line=node.line, col=node.col
        )
        pre.append(tmp_decl)
        node.left = Identifier(tmp_name, node.line, node.col)
        return pre, node

    # ── Case 6: Expand complex array initializers element by element ─────
    # Level 3

    def expand_array_init(self, node):
        """
        array int arr = [expr1, expr2, expr3]
        →
        array int arr = [0, 0, 0]   (or uninitialized)
        arr[0] = expr1
        arr[1] = expr2
        arr[2] = expr3

        Only triggers when at least one element is non-literal (call, binop, etc.).
        """
        elements = node.init.elements
        has_complex = any(
            not isinstance(e, (IntLiteral, FloatLiteral, StringLiteral,
                               BoolLiteral, EmptyLiteral))
            for e in elements
        )
        if not has_complex:
            return [node]

        line, col = node.line, node.col
        count = len(elements)

        # Declare the array with zeroed-out literal placeholders
        zero = IntLiteral(0, line, col)
        placeholder = ArrayLiteral([zero] * count, line, col)
        decl = VarDecl(node.type_, node.name, placeholder, node.fixed, line, col)
        stmts = [decl]

        # Assign each element individually
        for i, elem in enumerate(elements):
            target = Index(
                Identifier(node.name, line, col),
                IntLiteral(i, line, col),
                line, col
            )
            stmts.append(Assign(target, "=", elem, line, col))

        return stmts


def normalize(program, level=2, source=None):
    """Normalize the AST at the given level. Checks for a per-file
    @norm N directive in the AST, with legacy // @norm N comment fallback."""
    file_level = detect_norm_level(program, source)
    if file_level is not None:
        level = file_level
    if level == 0:
        return program
    return Normalizer(level).normalize(program)
