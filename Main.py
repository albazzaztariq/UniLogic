#!/usr/bin/env python3
# ulc/ulc.py — UniLogic compiler driver
# Usage: python ulc.py input.ul [-t c|python|js|llvm|wasm] [-o output] [--run]

import sys
import os
import shutil
import subprocess

__version__ = "0.1.0"

_base = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(_base, "Compiler", "Lexer"))
sys.path.insert(0, os.path.join(_base, "Compiler", "AST Gen"))
sys.path.insert(0, os.path.join(_base, "Compiler", "SynTree Def"))
sys.path.insert(0, os.path.join(_base, "Compiler", "Semantic"))
sys.path.insert(0, os.path.join(_base, "Compiler"))
sys.path.insert(0, os.path.join(_base, "Transcoder", "Decoder"))
sys.path.insert(0, os.path.join(_base, "Transcoder", "Encoder"))
sys.path.insert(0, os.path.join(_base, "Testing", "Tools"))


from src2token import tokenize
from token2ast import parse
from semcheck  import check
from ast_nodes import ForeignImport, FunctionDecl

TARGETS = {
    "1": ("c",      ".c"),
    "2": ("python", ".py"),
    "3": ("js",     ".js"),
    "4": ("llvm",   ".ll"),
    "5": ("wasm",   ".wasm"),
    "6": ("run",    None),
    "7": ("bytecode", ".ulb"),
    "8": ("native", ".exe"),
}

TARGET_ALIASES = {
    "c": "c", "python": "python", "py": "python",
    "js": "js", "javascript": "js",
    "llvm": "llvm", "ll": "llvm", "ir": "llvm",
    "wasm": "wasm", "wasm32": "wasm",
    "run": "run",
    "bytecode": "bytecode", "bc": "bytecode", "ulb": "bytecode",
    "native": "native", "exe": "native",
}

def resolve_local_imports(program, source_dir):
    """Resolve import "file.ul" declarations.

    For each ForeignImport where lib ends in '.ul', read and parse
    that file, find the matching FunctionDecl, and replace the
    ForeignImport with the resolved FunctionDecl (marked is_imported=True).

    Search order:
      1. Same directory as the source file
      2. ul_packages/<package>/ directory (for installed packages)
      3. Compiler root directory (for Stdlib/ imports)
    """
    new_decls = []
    # Track existing FFI names to avoid duplicates when pulling transitive deps
    existing_ffi = set()
    for decl in program.decls:
        if isinstance(decl, ForeignImport) and not decl.lib.endswith(".ul"):
            existing_ffi.add(decl.name)

    for decl in program.decls:
        if isinstance(decl, ForeignImport) and decl.lib.endswith(".ul"):
            ul_path = os.path.join(source_dir, decl.lib)
            # Also check ul_packages/ for package imports like "mathlib/utils.ul"
            if not os.path.isfile(ul_path):
                pkg_path = os.path.join(source_dir, "ul_packages", decl.lib)
                if os.path.isfile(pkg_path):
                    ul_path = pkg_path
            # Check compiler root (for Stdlib/ imports)
            if not os.path.isfile(ul_path):
                root_path = os.path.join(_base, decl.lib)
                if os.path.isfile(root_path):
                    ul_path = root_path
            if not os.path.isfile(ul_path):
                raise FileNotFoundError(
                    f"line {decl.line}: import \"{decl.lib}\" — file not found: {ul_path}"
                )
            lib_src = open(ul_path, encoding='utf-8').read()
            lib_tokens = tokenize(lib_src, decl.lib)
            lib_program = parse(lib_tokens, decl.lib)

            # Find the matching function by name
            found = None
            for lib_decl in lib_program.decls:
                if isinstance(lib_decl, FunctionDecl) and lib_decl.name == decl.name:
                    found = lib_decl
                    break
            if found is None:
                raise NameError(
                    f"line {decl.line}: import \"{decl.lib}\" — "
                    f"function '{decl.name}' not found in {decl.lib}"
                )

            # Pull in FFI declarations from the library file (transitive deps)
            for lib_decl in lib_program.decls:
                if isinstance(lib_decl, ForeignImport) and not lib_decl.lib.endswith(".ul"):
                    if lib_decl.name not in existing_ffi:
                        new_decls.append(lib_decl)
                        existing_ffi.add(lib_decl.name)

            found.is_imported = True
            new_decls.append(found)
        else:
            new_decls.append(decl)
    program.decls = new_decls


def pick_target(flag=None):
    if flag:
        key = TARGET_ALIASES.get(flag.lower())
        if not key:
            print(f"unknown target '{flag}'. choices: c, python, js, llvm, wasm, run")
            sys.exit(1)
        return key
    print("select target:")
    print("  1) C")
    print("  2) Python")
    print("  3) JavaScript")
    print("  4) LLVM IR")
    print("  5) WebAssembly")
    print("  6) Run (interpreter)")
    print("  7) Bytecode (.ulb)")
    print("  8) Native binary")
    choice = input("> ").strip()
    entry = TARGETS.get(choice)
    if not entry:
        print(f"invalid choice '{choice}'")
        sys.exit(1)
    return entry[0]

def load_codegen(target):
    if target == "c":
        from ul2c import generate
        return generate, ".c"
    if target == "llvm":
        from ul2llvm import generate
        return generate, ".ll"
    if target == "wasm":
        from ul2llvm import generate
        def gen_wasm(program, profile=False):
            return generate(program, target_triple="wasm32-unknown-unknown", profile=profile)
        return gen_wasm, ".ll"
    if target == "python":
        from ul2py import generate
        return generate, ".py"
    if target == "js":
        from ul2js import generate
        return generate, ".js"
    if target == "bytecode" or target == "ubi":
        from ul2ubi import generate
        return generate, ".ubi"

def compile_file(path, target, out_path=None, norm_level=0, profile=False, quiet=False):
    src      = open(path, encoding='utf-8').read()
    filename = os.path.basename(path)

    tokens   = tokenize(src, filename)
    program  = parse(tokens, filename)

    # Resolve local .ul file imports before semantic check
    resolve_local_imports(program, os.path.dirname(os.path.abspath(path)))

    if norm_level > 0:
        from simplifycode import normalize as norm
        program = norm(program, level=norm_level, source=src)
    else:
        # Even at level 0, check for per-file override
        from simplifycode import detect_norm_level, normalize as norm
        file_level = detect_norm_level(program, src)
        if file_level and file_level > 0:
            program = norm(program, level=file_level, source=src)

    check(program, filename)

    # Native binary: generate C, then compile to executable
    if target == "native":
        return _compile_native(program, path, filename, out_path, profile)

    generate, ext = load_codegen(target)
    output = generate(program, profile=profile)

    # For WASM: write the .ll IR, then attempt llc + wasm-ld
    if target == "wasm":
        return _compile_wasm(output, path, filename, out_path)

    if out_path is None:
        out_path = os.path.splitext(path)[0] + ext

    if isinstance(output, bytes):
        with open(out_path, "wb") as f:
            f.write(output)
    else:
        with open(out_path, "w") as f:
            f.write(output)

    if not quiet:
        print(f"compiled {filename} -> {out_path}  [{target}]")
    return output


def run_file(path, norm_level=0):
    """Parse and execute a .ul file directly via the tree-walking interpreter."""
    from repl import run_program

    src      = open(path, encoding='utf-8').read()
    filename = os.path.basename(path)

    tokens   = tokenize(src, filename)
    program  = parse(tokens, filename)

    # Resolve local .ul file imports before semantic check
    resolve_local_imports(program, os.path.dirname(os.path.abspath(path)))

    if norm_level > 0:
        from simplifycode import normalize as norm
        program = norm(program, level=norm_level, source=src)
    else:
        from simplifycode import detect_norm_level, normalize as norm
        file_level = detect_norm_level(program, src)
        if file_level and file_level > 0:
            program = norm(program, level=file_level, source=src)

    check(program, filename)

    return run_program(program)


def _compile_native(program, path, filename, out_path, profile):
    """Compile to a native executable binary.

    Strategy:
      1. Try LLVM path: generate IR → llc → gcc linker
      2. Fall back to C path: generate C → gcc
      3. If neither toolchain is available, error out.
    """
    base = os.path.splitext(path)[0]
    exe_ext = ".exe" if sys.platform == "win32" else ""
    exe_path = out_path if out_path else base + exe_ext

    llc = shutil.which("llc")
    gcc = shutil.which("gcc")

    if llc and gcc:
        # LLVM path: .ul → .ll → .o → executable
        from ul2llvm import generate as gen_llvm
        ir = gen_llvm(program, profile=profile)
        ll_path = base + ".ll"
        obj_path = base + ".o"
        with open(ll_path, "w") as f:
            f.write(ir)

        r = subprocess.run([llc, "-filetype=obj", ll_path, "-o", obj_path],
                           capture_output=True, text=True)
        if r.returncode != 0:
            print(f"llc failed: {r.stderr.strip()}")
            sys.exit(1)

        r = subprocess.run([gcc, obj_path, "-o", exe_path, "-lm"],
                           capture_output=True, text=True)
        if r.returncode != 0:
            print(f"linker failed: {r.stderr.strip()}")
            sys.exit(1)

        for tmp in [ll_path, obj_path]:
            try: os.remove(tmp)
            except OSError: pass

        print(f"compiled {filename} -> {exe_path}  [native/llvm]")
        return ir

    if gcc:
        # C fallback: .ul → .c → executable
        from ul2c import generate as gen_c
        c_code = gen_c(program, profile=profile)
        c_path = base + "._tmp.c"
        with open(c_path, "w") as f:
            f.write(c_code)

        r = subprocess.run([gcc, "-O2", c_path, "-o", exe_path, "-lm"],
                           capture_output=True, text=True)
        if r.returncode != 0:
            print(f"gcc failed: {r.stderr.strip()}")
            sys.exit(1)

        try: os.remove(c_path)
        except OSError: pass

        print(f"compiled {filename} -> {exe_path}  [native/gcc]")
        return c_code

    print("Native compilation requires LLVM tools (llc) or GCC.")
    print("Install LLVM and ensure llc is in PATH, or install GCC.")
    sys.exit(1)


def _compile_wasm(ir_output, path, filename, out_path):
    """Compile LLVM IR → .wasm binary.
    Strategy:
      1. Try clang --target=wasm32-wasi (wasi-sdk)
      2. Fall back to llc + wasm-ld (LLVM tools)
      3. If neither available, emit .ll IR with instructions
    """
    base = os.path.splitext(path)[0]
    ll_path = (out_path.rsplit(".", 1)[0] if out_path else base) + ".ll"
    wasm_path = out_path if out_path else base + ".wasm"
    obj_path = ll_path.rsplit(".", 1)[0] + ".o"

    # Always write the .ll IR
    with open(ll_path, "w") as f:
        f.write(ir_output)

    clang = shutil.which("clang")
    llc = shutil.which("llc")
    wasm_ld = shutil.which("wasm-ld") or shutil.which("lld")

    # Path 1: clang with wasm32-wasi target (wasi-sdk)
    if clang:
        r = subprocess.run(
            [clang, "--target=wasm32-wasi", "-nostdlib",
             "-Wl,--no-entry", "-Wl,--export-all",
             "-o", wasm_path, ll_path],
            capture_output=True, text=True)
        if r.returncode == 0:
            try: os.remove(ll_path)
            except OSError: pass
            print(f"compiled {filename} -> {wasm_path}  [wasm/clang]")
            return ir_output
        # clang failed — try llc path

    # Path 2: llc + wasm-ld
    if llc and wasm_ld:
        r = subprocess.run(
            [llc, "-march=wasm32", "-filetype=obj", ll_path, "-o", obj_path],
            capture_output=True, text=True)
        if r.returncode != 0:
            print(f"llc failed: {r.stderr.strip()}")
            print(f"compiled {filename} -> {ll_path}  [wasm/llvm-ir]")
            return ir_output

        r = subprocess.run(
            [wasm_ld, "--no-entry", "--export-all", obj_path, "-o", wasm_path],
            capture_output=True, text=True)
        if r.returncode != 0:
            print(f"wasm-ld failed: {r.stderr.strip()}")
            print(f"compiled {filename} -> {ll_path}  [wasm/llvm-ir]")
            return ir_output

        for tmp in [obj_path, ll_path]:
            try: os.remove(tmp)
            except OSError: pass

        print(f"compiled {filename} -> {wasm_path}  [wasm]")
        return ir_output

    # No toolchain — emit IR with instructions
    print(f"compiled {filename} -> {ll_path}  [wasm/llvm-ir]")
    print("WASM compilation requires clang with WASM target and wasm-ld.")
    print("Install wasi-sdk from https://github.com/WebAssembly/wasi-sdk/releases")
    return ir_output


if __name__ == "__main__":
    if "--version" in sys.argv or (len(sys.argv) == 2 and sys.argv[1] == "version"):
        print(f"UniLogic {__version__}")
        sys.exit(0)

    if len(sys.argv) < 2:
        print("usage: ulc.py <input.ul> [-t c|python|js|llvm|wasm|run] [-o output] [--run]")
        print("       ulc.py new <name>     — create a new project")
        print("       ulc.py init [name]    — initialize project in current directory")
        print("       ulc.py run            — run project entry file")
        print("       ulc.py build          — build project to configured target")
        print("       ulc.py test           — run all tests in tests/")
        print("       ulc.py install [pkg]  — install dependencies")
        print("       ulc.py remove <pkg>   — remove a package")
        print("       ulc.py list           — list installed packages")
        print("       ulc.py publish        — prepare for publishing")
        sys.exit(1)

    # Package manager commands
    pm_commands = {"install", "remove", "list", "new", "publish", "run", "build", "test", "init"}
    if sys.argv[1] in pm_commands:
        from package_manager import run as pm_run
        pm_run(sys.argv[1], sys.argv[2:])
        sys.exit(0)

    # Python → UL transpilation
    if "--from" in sys.argv:
        idx = sys.argv.index("--from")
        from_lang = sys.argv[idx + 1]
        if from_lang == "python":
            from py2ul import transpile
            input_path = sys.argv[1]
            out_path = None
            if "-o" in sys.argv:
                out_path = sys.argv[sys.argv.index("-o") + 1]
            else:
                out_path = os.path.splitext(input_path)[0] + ".ul"
            source = open(input_path, encoding='utf-8').read()
            ul_source, warnings, line_count = transpile(source, input_path)
            with open(out_path, 'w', encoding='utf-8') as f:
                f.write(ul_source)
            print(f"Translated: {line_count} lines")
            if warnings:
                print(f"Warnings: {len(warnings)} constructs could not be translated")
            print(f"Output: {out_path}")
            sys.exit(0)
        else:
            print(f"Unknown source language: {from_lang}. Supported: python")
            sys.exit(1)

    # Validate flags — reject unknown ones before treating argv[1] as a file
    known_flags = {
        "--version", "--profile", "--dry-run", "--run", "-r", "--vm",
        "--normalize", "--norm-level", "--from", "-t", "-o",
        "--help", "-h",
    }
    # Flags that consume the next argument
    flags_with_args = {"--norm-level", "--from", "-t", "-o"}
    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg.startswith("-"):
            if arg not in known_flags:
                print(f"error: unknown flag '{arg}'")
                print("usage: ulc.py <input.ul> [-t c|python|js|llvm|wasm|run] [-o output] [--run]")
                print("       ulc.py --help for more options")
                sys.exit(1)
            if arg in flags_with_args:
                i += 1  # skip the value
        i += 1

    if "--help" in sys.argv or "-h" in sys.argv:
        print("usage: ulc.py <input.ul> [-t c|python|js|llvm|wasm|run] [-o output] [--run]")
        print()
        print("flags:")
        print("  -t <target>     target language (c, python, js, llvm, wasm, run)")
        print("  -o <output>     output file path")
        print("  -r, --run       compile and run immediately")
        print("  --vm            run in bytecode VM")
        print("  --profile       show timing information")
        print("  --dry-run       normalize and print reconstructed source")
        print("  --normalize     normalize AST (alias for --norm-level 2)")
        print("  --norm-level N  normalization level (0-3)")
        print("  --from python   transpile from Python to UL")
        print("  --version       show version")
        sys.exit(0)

    input_path  = sys.argv[1]
    output_path = None
    target_flag = None
    norm_level  = 0
    profile     = "--profile" in sys.argv
    dry_run     = "--dry-run" in sys.argv
    run_mode    = "--run" in sys.argv or "-r" in sys.argv
    vm_mode     = "--vm" in sys.argv

    # --normalize is an alias for --norm-level 2
    if "--normalize" in sys.argv:
        norm_level = 2
    if "--norm-level" in sys.argv:
        idx = sys.argv.index("--norm-level")
        norm_level = int(sys.argv[idx + 1])

    if "-t" in sys.argv:
        idx = sys.argv.index("-t")
        target_flag = sys.argv[idx + 1]

    if "-o" in sys.argv:
        idx = sys.argv.index("-o")
        output_path = sys.argv[idx + 1]

    # Support positional output: xpile input.ul output.py -t python
    # If argv[2] exists, doesn't start with '-', and output_path is still None
    if output_path is None and len(sys.argv) > 2:
        candidate = sys.argv[2]
        if not candidate.startswith("-"):
            output_path = candidate

    # --dry-run: normalize and print reconstructed source, then exit
    if dry_run:
        src = open(input_path, encoding='utf-8').read()
        tokens = tokenize(src, os.path.basename(input_path))
        program = parse(tokens, os.path.basename(input_path))
        resolve_local_imports(program, os.path.dirname(os.path.abspath(input_path)))
        if norm_level > 0:
            from simplifycode import normalize as norm
            program = norm(program, level=norm_level, source=src)
        else:
            from simplifycode import detect_norm_level, normalize as norm
            file_level = detect_norm_level(program, src)
            if file_level and file_level > 0:
                program = norm(program, level=file_level, source=src)
        from ast2src import unparse
        print(unparse(program), end="")
        sys.exit(0)

    # --vm flag: compile to bytecode, then run through ulvm
    if vm_mode:
        import tempfile
        # Compile to bytecode .ulb
        ulb_path = os.path.splitext(input_path)[0] + ".ulb"
        try:
            compile_file(input_path, "bytecode", ulb_path, norm_level, profile, quiet=True)
        except Exception as e:
            print(f"error: bytecode compilation failed: {e}", file=sys.stderr)
            sys.exit(1)
        # Find ulvm executable
        ulvm = None
        _interp = os.path.normpath(os.path.join(_base, "..", "Interpreter", "UBI"))
        for candidate in [
            os.path.join(_interp, "ulvm.exe"),
            os.path.join(_interp, "ulvm"),
            os.path.join(_base, "VM", "ulvm.exe"),
            os.path.join(_base, "VM", "ulvm"),
            os.path.join(_base, "ulvm.exe"),
            os.path.join(_base, "ulvm"),
        ]:
            if os.path.isfile(candidate):
                ulvm = candidate
                break
        if ulvm is None:
            print("error: ulvm executable not found (searched Interpreter/UBI/ and XPile/VM/)", file=sys.stderr)
            sys.exit(1)
        # Run through VM
        result = subprocess.run([ulvm, ulb_path])
        sys.exit(result.returncode)

    # --run flag or -t run both trigger interpreter mode
    if run_mode or target_flag == "run":
        try:
            exit_code = run_file(input_path, norm_level)
            sys.exit(exit_code)
        except (SyntaxError, Exception) as e:
            print(f"error: {e}", file=sys.stderr)
            sys.exit(1)

    target = pick_target(target_flag)

    # Interactive menu selected "run"
    if target == "run":
        try:
            exit_code = run_file(input_path, norm_level)
            sys.exit(exit_code)
        except (SyntaxError, Exception) as e:
            print(f"error: {e}", file=sys.stderr)
            sys.exit(1)

    try:
        compile_file(input_path, target, output_path, norm_level, profile)
    except NotImplementedError as e:
        print(f"error: {e}")
        sys.exit(1)
    except (SyntaxError, Exception) as e:
        print(f"error: {e}")
        sys.exit(1)