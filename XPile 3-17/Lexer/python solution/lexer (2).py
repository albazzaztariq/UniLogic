# ulc/lexer.py — UniLogic lexer
# Reads source text, produces a list of tokens.
# Each token is a tuple: (kind, value, line, col)

import re

KEYWORDS = {
    "function", "end", "returns", "return",
    "if", "else", "while", "do", "for", "each", "in",
    "match", "iterate", "default", "escape", "continue",
    "type", "inherits", "new", "fixed", "constant",
    "import", "from", "export", "print", "prompt",
    "parallel", "killswitch", "teleport", "portal",
    "nocache", "yield", "inline", "pack",
    "true", "false", "empty",
    "and", "or", "not", "equals",
    "both1", "both0", "either1", "delta", "bitflip", "negate", "left", "right",
    "address", "deref", "memmove", "memcopy", "memset", "memtake", "memgive",
    "size", "change", "absval",
    "int", "integer", "float", "double", "string", "bool", "none", "complex",
    "int8", "int16", "int32", "int64",
    "uint8", "uint16", "uint32", "uint64",
    "array", "list", "map", "arena", "file",
    "ok", "error", "some",
}

# Patterns are tried in order — first match wins.
# Longer/more specific patterns must come before shorter ones (e.g. ++ before +).
TOKEN_PATTERNS = [
    # Skip
    ("COMMENT",   r"//[^\n]*"),
    ("SKIP",      r"[ \t\r\n]+"),

    # Literals
    ("HEX",       r"0[xX][0-9a-fA-F]+"),
    ("FLOAT",     r"\d+\.\d+(?:[eE][+-]?\d+)?"),
    ("INT",       r"\d+"),
    ("STRING",    r'"[^"\\]*(?:\\.[^"\\]*)*"'),

    # Compound operators — longer first
    ("OP",        r"\+\+|--|->|==|!=|<=|>=|\+=|-=|\*=|/=|%=|left=|right="),

    # Single-char operators
    ("OP",        r"[+\-*/%=<>!?]"),

    # Delimiters
    ("LPAREN",    r"\("),
    ("RPAREN",    r"\)"),
    ("LBRACKET",  r"\["),
    ("RBRACKET",  r"\]"),
    ("COMMA",     r","),
    ("DOT",       r"\."),
    ("COLON",     r":"),
    ("AT",        r"@"),
    ("HASH",      r"#"),

    # Identifiers and keywords (after operators so 'left=' is caught above)
    ("IDENT",     r"[a-zA-Z_][a-zA-Z0-9_]*"),

    # Anything else is an error
    ("UNKNOWN",   r"."),
]

MASTER_RE = re.compile(
    "|".join(f"(?P<T{i}_{name}>{pattern})"
             for i, (name, pattern) in enumerate(TOKEN_PATTERNS))
)


def tokenize(src, filename="<source>"):
    tokens = []
    line = 1
    line_start = 0

    for m in MASTER_RE.finditer(src):
        # lastgroup is like "T3_FLOAT" — strip the index prefix to get the kind
        kind = m.lastgroup.split("_", 1)[1]
        value = m.group()
        col = m.start() - line_start + 1

        # track line/col for tokens that come after this one
        newlines = value.count("\n")
        if newlines:
            line += newlines
            line_start = m.start() + value.rfind("\n") + 1

        if kind in ("COMMENT", "SKIP"):
            continue

        if kind == "UNKNOWN":
            raise SyntaxError(f"{filename}:{line}:{col} — unexpected character {repr(value)}")

        if kind == "IDENT":
            if value == "integer":
                tokens.append(("KW", "int", line, col))
            elif value in KEYWORDS:
                tokens.append(("KW", value, line, col))
            else:
                tokens.append(("IDENT", value, line, col))
        else:
            tokens.append((kind, value, line, col))

    tokens.append(("EOF", "", line, 0))
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
