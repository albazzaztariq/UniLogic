# Makefile — UniLogic XPile C AST
#
# Builds ast_arena.c + ast_strings.c + ast_debug.c + test_ast.c
# and runs the test.
#
# Usage:
#   make          — build and run tests
#   make build    — build only
#   make test     — run existing binary
#   make clean    — remove build artifacts
#
# Requires: gcc (or clang — set CC=clang)

CC      ?= gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter \
           -O2 -g
# Treat warnings as errors so size-assertion failures are hard failures
CFLAGS  += -Werror=implicit-function-declaration

# ast_debug.c has a large switch that triggers a GCC/MinGW ICE at -O2.
# Compile it at -O0; it is a debug-only file and performance is irrelevant.
CFLAGS_DEBUG := -std=c11 -Wall -Wextra -Wpedantic -Wno-unused-parameter \
                -O0 -g -Werror=implicit-function-declaration

# Source files
SRCS    := ast_arena.c ast_strings.c test_ast.c
OBJS    := $(SRCS:.c=.o) ast_debug.o
TARGET  := test_ast

.PHONY: all build test clean

all: build test

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ast_debug.c compiled separately with -O0
ast_debug.o: ast_debug.c ast_nodes.h ast_arena.h ast_strings.h
	$(CC) $(CFLAGS_DEBUG) -c -o $@ $<

# Dependencies (headers used by each source)
ast_arena.o:   ast_arena.c   ast_nodes.h ast_arena.h ast_strings.h
ast_strings.o: ast_strings.c ast_nodes.h ast_arena.h ast_strings.h
test_ast.o:    test_ast.c    ast_nodes.h ast_arena.h ast_strings.h

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
