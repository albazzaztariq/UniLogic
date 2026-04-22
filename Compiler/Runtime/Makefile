# Makefile — UniLogic Memory Mode Tests
#
# Builds one test binary per memory mode.
# Each binary is compiled from the same test_memory.c with a different -D flag.
#
# Usage:
#   make all              — build all four test binaries
#   make test             — build all and run all
#   make test_manual      — build and run manual mode test only
#   make test_arena       — build and run arena mode test only
#   make test_refcount    — build and run refcount mode test only
#   make test_gc          — build and run GC mode test only
#   make clean            — remove all build artifacts
#
# Prerequisites:
#   - mimalloc v3 installed or bundled.
#     Set MIMALLOC_DIR to the root of the mimalloc install if not in default paths.
#     Expected layout:
#       $(MIMALLOC_DIR)/include/mimalloc.h
#       $(MIMALLOC_DIR)/lib/mimalloc.lib  (Windows) or libmimalloc.a (Linux/macOS)
#   - C11-capable compiler (clang or gcc recommended).
#   - On Linux/macOS: pthreads available (-lpthread).
#   - On Windows: link with -lkernel32 (provides CreateThread/WaitForSingleObject).

CC       := clang
CFLAGS   := -std=c11 -Wall -Wextra -Wpedantic -O2 -g
INCLUDES := -I.

# --------------------------------------------------------------------------
# mimalloc configuration
# --------------------------------------------------------------------------
# Override MIMALLOC_DIR on the command line:
#   make MIMALLOC_DIR=/usr/local all
# Default: assume mimalloc headers and static lib are on the default search path.
MIMALLOC_DIR ?=

ifdef MIMALLOC_DIR
  INCLUDES  += -I$(MIMALLOC_DIR)/include
  MIMALLOC_LIB := $(MIMALLOC_DIR)/lib/libmimalloc.a
else
  # Assume -lmimalloc is available via the system linker path.
  MIMALLOC_LIB := -lmimalloc
endif

# --------------------------------------------------------------------------
# Platform-specific flags
# --------------------------------------------------------------------------
UNAME := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME), Windows)
  EXT      := .exe
  LDFLAGS  := $(MIMALLOC_LIB) -lkernel32
else
  EXT      :=
  LDFLAGS  := $(MIMALLOC_LIB) -lpthread
endif

# --------------------------------------------------------------------------
# Source files
# --------------------------------------------------------------------------
RUNTIME_SRC_MANUAL   := ul_manual.c
RUNTIME_SRC_ARENA    := ul_arena.c
RUNTIME_SRC_REFCOUNT := ul_refcount.c
RUNTIME_SRC_GC       := ul_gc.c
TEST_SRC             := test_memory.c

# --------------------------------------------------------------------------
# Output binaries
# --------------------------------------------------------------------------
BIN_MANUAL   := test_memory_manual$(EXT)
BIN_ARENA    := test_memory_arena$(EXT)
BIN_REFCOUNT := test_memory_refcount$(EXT)
BIN_GC       := test_memory_gc$(EXT)

ALL_BINS := $(BIN_MANUAL) $(BIN_ARENA) $(BIN_REFCOUNT) $(BIN_GC)

# --------------------------------------------------------------------------
# Targets
# --------------------------------------------------------------------------

.PHONY: all test test_manual test_arena test_refcount test_gc clean

all: $(ALL_BINS)

# ---- Manual ----
$(BIN_MANUAL): $(TEST_SRC) $(RUNTIME_SRC_MANUAL) ul_manual.h ul_memory.h
	$(CC) $(CFLAGS) $(INCLUDES) -DUL_MEMORY_MANUAL \
	    $(TEST_SRC) $(RUNTIME_SRC_MANUAL) \
	    -o $@ $(LDFLAGS)

test_manual: $(BIN_MANUAL)
	@echo "--- Running manual mode test ---"
	./$(BIN_MANUAL)

# ---- Arena ----
$(BIN_ARENA): $(TEST_SRC) $(RUNTIME_SRC_ARENA) ul_arena.h ul_memory.h
	$(CC) $(CFLAGS) $(INCLUDES) -DUL_MEMORY_ARENA \
	    $(TEST_SRC) $(RUNTIME_SRC_ARENA) \
	    -o $@ $(LDFLAGS)

test_arena: $(BIN_ARENA)
	@echo "--- Running arena mode test ---"
	./$(BIN_ARENA)

# ---- Refcount ----
$(BIN_REFCOUNT): $(TEST_SRC) $(RUNTIME_SRC_REFCOUNT) ul_refcount.h ul_memory.h
	$(CC) $(CFLAGS) $(INCLUDES) -DUL_MEMORY_REFCOUNT \
	    $(TEST_SRC) $(RUNTIME_SRC_REFCOUNT) \
	    -o $@ $(LDFLAGS)

test_refcount: $(BIN_REFCOUNT)
	@echo "--- Running refcount mode test ---"
	./$(BIN_REFCOUNT)

# ---- GC ----
$(BIN_GC): $(TEST_SRC) $(RUNTIME_SRC_GC) ul_gc.h ul_memory.h
	$(CC) $(CFLAGS) $(INCLUDES) -DUL_MEMORY_GC \
	    $(TEST_SRC) $(RUNTIME_SRC_GC) \
	    -o $@ $(LDFLAGS)

test_gc: $(BIN_GC)
	@echo "--- Running GC mode test ---"
	./$(BIN_GC)

# ---- Run all ----
test: all
	@echo "=============================="
	@echo " UniLogic Memory Mode Tests"
	@echo "=============================="
	@$(MAKE) test_manual
	@$(MAKE) test_arena
	@$(MAKE) test_refcount
	@$(MAKE) test_gc
	@echo "=============================="
	@echo " All tests complete."
	@echo "=============================="

# ---- Clean ----
clean:
	rm -f $(ALL_BINS) *.o
