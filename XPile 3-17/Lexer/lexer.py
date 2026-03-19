# ulc/lexer.py — UniLogic lexer
# Reads source text, produces a list of tokens.
# Each token is a tuple: (kind, value, line, col)
# kind is a string like 'KW', 'IDENT', 'INT', 'OP', etc.

# Every reserved word in UL. If a scanned word is in this set it becomes
# a 'KW' token instead of an 'IDENT' token.
KEYWORDS = {
    "function", "end", "returns", "return",
    "if", "else", "while", "do", "for", "each", "in",
    "match", "iterate", "default", "escape", "continue",
    "type", "inherits", "new", "fixed", "constant",
    "import", "from", "export", "print", "prompt",
    "parallel", "killswitch", "teleport", "portal", "goto",
    "try", "catch", "finally", "throw", "with", "as",
    "nocache", "yield", "yields", "inline", "pack",
    "true", "false", "empty",
    "and", "or", "not", "equals",
    "both1", "both0", "either1", "bitflip", "negate", "left", "right",
    "band", "bor", "bnot", "lshift", "rshift",
    "bit_and", "bit_or", "bit_xor", "bit_not", "bit_left", "bit_right",
    "char_code", "sort",
    "address", "deref", "memmove", "memcopy", "memset", "memtake", "memgive",
    "size", "change", "absval",
    "int", "integer", "float", "double", "string", "bool", "none", "complex",
    "int8", "int16", "int32", "int64",
    "uint8", "uint16", "uint32", "uint64",
    "array", "list", "map", "arena", "file",
    "ok", "error", "some",
    "object", "const", "cast", "exit", "typeof", "spawn", "wait",
    "lock", "unlock",
    "random", "random_int", "random_seed", "userinput",
}

# Keyword aliases — old names normalize to their canonical replacements.
# When the lexer scans one of these words it emits a KW token with the
# canonical (value) name.  Keeps old code working while migrating names.
_KW_ALIASES = {
    'integer':   'int',
    'teleport':  'goto',
    'rand':      'random',
    'rand_int':  'random_int',
    'rand_seed': 'random_seed',
    'args':      'userinput',
    # Bitwise verbose names → canonical bit_* forms
    'both1':     'bit_and',
    'band':      'bit_and',
    'either1':   'bit_or',
    'bor':       'bit_or',
    'xor':       'bit_xor',
    'diff':      'bit_xor',
    'delta':     'bit_xor',
    'bitflip':   'bit_not',
    'bnot':      'bit_not',
    'negate':    'bit_not',
    'left':      'bit_left',
    'lshift':    'bit_left',
    'right':     'bit_right',
    'rshift':    'bit_right',
}

def tokenize(src, filename="<source>"):
    tokens = []  # output list — we append to this throughout
    pos  = 0     # current position in src string
    line = 1     # current line number (for error messages)
    col  = 1     # current column number (for error messages)

    def peek(offset=0):
        # Look at the character at pos+offset without consuming it.
        # Returns None if we're past the end of the file.
        i = pos + offset
        return src[i] if i < len(src) else None

    def advance():
        # Consume and return the current character.
        # Updates line/col tracking so error messages stay accurate.
        nonlocal pos, line, col
        ch = src[pos]
        pos += 1
        if ch == '\n':
            line += 1
            col = 1
        else:
            col += 1
        return ch

    def add(kind, value):
        # Append a completed token to the output list.
        tokens.append((kind, value, line, col))

    def error(msg):
        # Raise a SyntaxError with the current file/line/col in the message.
        raise SyntaxError(f"{filename}:{line}:{col} — {msg}")

    while pos < len(src):

        # ── Skip whitespace ───────────────────────────────────────────────────
        # Consume spaces, tabs, carriage returns, and newlines silently.
        while pos < len(src) and src[pos] in ' \t\r\n':
            advance()

        if pos >= len(src):
            break

        # ── Skip line comments ────────────────────────────────────────────────
        # // starts a comment that runs to end of line. Consume everything
        # after // on this line then loop back to the top.
        if peek() == '/' and peek(1) == '/':
            while pos < len(src) and peek() != '\n':
                advance()
            continue

        ch = peek()  # the character we're about to scan

        # ── String literal ────────────────────────────────────────────────────
        # Starts with ". Read until the closing ", handling escape sequences.
        # Produces a STRING token whose value is the unescaped string content.
        if ch == '"':
            advance()  # consume opening "
            buf = []
            while pos < len(src) and peek() != '"':
                if peek() == '\\':
                    # escape sequence — consume the backslash and the next char
                    advance()
                    esc = advance()
                    # map the escape character to its actual value
                    buf.append({'n':'\n', 't':'\t', '"':'"', '\\':'\\'}.get(esc, esc))
                elif peek() == '\n':
                    error("unterminated string")
                else:
                    buf.append(advance())
            if pos >= len(src):
                error("unterminated string")
            advance()  # consume closing "
            add('STRING', ''.join(buf))

        # ── Hex literal ───────────────────────────────────────────────────────
        # Starts with 0x or 0X. Read hex digits until a non-hex character.
        # Produces a HEX token, e.g. '0xDEAD0000'.
        elif ch == '0' and peek(1) in ('x', 'X'):
            buf = [advance(), advance()]  # consume '0' and 'x'
            # keep consuming as long as the next char is a valid hex digit
            while peek() and peek() in '0123456789abcdefABCDEF':
                buf.append(advance())
            add('HEX', ''.join(buf))

        # ── Number ────────────────────────────────────────────────────────────
        # Starts with a digit. Read digits; if we see a decimal point followed
        # by more digits it's a float, otherwise it's an integer.
        # Floats may have an optional exponent like 1e-5 or 3.14E+2.
        elif ch.isdigit():
            buf = []
            while peek() and peek().isdigit():
                buf.append(advance())
            if peek() == '.' and peek(1) and peek(1).isdigit():
                # decimal point followed by digit — it's a float
                buf.append(advance())  # consume '.'
                while peek() and peek().isdigit():
                    buf.append(advance())
                # optional exponent part: e or E, optional sign, digits
                if peek() in ('e', 'E'):
                    buf.append(advance())
                    if peek() in ('+', '-'):
                        buf.append(advance())
                    while peek() and peek().isdigit():
                        buf.append(advance())
                add('FLOAT', ''.join(buf))
            else:
                add('INT', ''.join(buf))

        # ── Identifier or keyword ─────────────────────────────────────────────
        # Starts with a letter or underscore. Read letters, digits, underscores.
        # If the resulting word is in KEYWORDS it becomes a 'KW' token.
        # Otherwise it's an 'IDENT' (variable name, function name, type name).
        # Special case: 'left=' and 'right=' are bitwise compound assignments —
        # we check for the = immediately after the word.
        elif ch.isalpha() or ch == '_':
            buf = []
            while peek() and (peek().isalnum() or peek() == '_'):
                buf.append(advance())
            word = ''.join(buf)
            if word == 'left' and peek() == '=':
                advance()  # consume '='
                add('OP', 'left=')
            elif word == 'right' and peek() == '=':
                advance()  # consume '='
                add('OP', 'right=')
            elif word in _KW_ALIASES:
                add('KW', _KW_ALIASES[word])
            elif word in KEYWORDS:
                add('KW', word)
            else:
                add('IDENT', word)

        # ── @ mod prefix ──────────────────────────────────────────────────────
        # @ always precedes a modifier name like @async, @nullable, @callembed.
        # We emit AT then the parser reads the following IDENT as the mod name.
        elif ch == '@':
            advance()
            add('AT', '@')

        # ── # macro prefix ────────────────────────────────────────────────────
        # # precedes function/end function for preprocessor-level macro functions.
        elif ch == '#':
            advance()
            add('HASH', '#')

        # ── Operators ─────────────────────────────────────────────────────────
        # Multi-character operators are checked first (++, +=, ->, etc.)
        # then single-character fallback.

        elif ch == '+':
            advance()
            if peek() == '+': advance(); add('OP', '++')    # increment
            elif peek() == '=': advance(); add('OP', '+=')  # compound add
            else: add('OP', '+')

        elif ch == '-':
            advance()
            if peek() == '-': advance(); add('OP', '--')    # decrement
            elif peek() == '=': advance(); add('OP', '-=')  # compound subtract
            elif peek() == '>': advance(); add('OP', '->')  # cast arrow: change(x)->int
            else: add('OP', '-')

        elif ch == '*':
            advance()
            if peek() == '=': advance(); add('OP', '*=')
            else: add('OP', '*')

        elif ch == '/':
            advance()
            if peek() == '=': advance(); add('OP', '/=')
            else: add('OP', '/')

        elif ch == '%':
            advance()
            if peek() == '=': advance(); add('OP', '%=')
            else: add('OP', '%')

        elif ch == '=':
            advance()
            if peek() == '=': advance(); add('OP', '==')   # equality check
            else: add('OP', '=')                           # assignment

        elif ch == '!':
            advance()
            if peek() == '=': advance(); add('OP', '!=')   # not-equal
            else: error("unexpected '!' — did you mean 'not'?")

        elif ch == '<':
            advance()
            if peek() == '=': advance(); add('OP', '<=')   # less-or-equal
            else: add('OP', '<')  # less-than OR pointer open bracket — parser decides

        elif ch == '>':
            advance()
            if peek() == '=': advance(); add('OP', '>=')   # greater-or-equal
            else: add('OP', '>')  # greater-than OR pointer close bracket — parser decides

        # ── Delimiters ────────────────────────────────────────────────────────
        elif ch == '(': advance(); add('LPAREN', '(')
        elif ch == ')': advance(); add('RPAREN', ')')
        elif ch == '[': advance(); add('LBRACKET', '[')
        elif ch == ']': advance(); add('RBRACKET', ']')
        elif ch == '{': advance(); add('LBRACE', '{')
        elif ch == '}': advance(); add('RBRACE', '}')
        elif ch == ',': advance(); add('COMMA', ',')
        elif ch == '.': advance(); add('DOT', '.')
        elif ch == ':': advance(); add('COLON', ':')
        elif ch == '?': advance(); add('OP', '?')   # Result propagation operator
        elif ch == '|':
            advance()
            if pos < len(src) and src[pos] == '>':
                advance(); add('OP', '|>')  # pipe operator
            else:
                add('PIPE', '|')  # ok|error type union

        else:
            error(f"unexpected character {repr(ch)}")

    # ── End of file ───────────────────────────────────────────────────────────
    # Always append an EOF token so the parser has a clean sentinel to stop on.
    tokens.append(('EOF', '', line, col))
    return tokens


if __name__ == "__main__":
    import sys
    src = open(sys.argv[1]).read() if len(sys.argv) > 1 else """
function add(int a, int b) returns int
  return a + b
end function

function main() returns int
  int x = add(3, 7)
  print "result: " + change(x)->string
  return 0
end function
"""
    for tok in tokenize(src):
        print(tok)
