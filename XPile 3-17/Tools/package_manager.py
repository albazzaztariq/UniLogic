#!/usr/bin/env python3
"""package_manager.py — UniLogic package manager commands.

Commands:
    install [package]  — install a package (or all from ul.toml)
    remove  package    — remove an installed package
    list               — list installed packages
    new     name       — create a new project
    publish            — prepare package for publishing
"""

import os
import sys
import json
import hashlib
import zipfile
import shutil
import tempfile
import subprocess

# Add Tools/ to path for manifest
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from manifest import Manifest, MANIFEST_NAME

REGISTRY_URL = "https://raw.githubusercontent.com/albazzaztariq/UniLogic/main/registry/packages.json"
PACKAGES_DIR = "ul_packages"


def _fetch_url(url):
    """Fetch a URL and return bytes. Works with urllib (no pip deps)."""
    from urllib.request import urlopen, Request
    from urllib.error import URLError, HTTPError
    try:
        req = Request(url, headers={"User-Agent": "UniLogic-PackageManager/1.0"})
        with urlopen(req, timeout=30) as resp:
            return resp.read()
    except HTTPError as e:
        raise RuntimeError(f"HTTP {e.code} fetching {url}") from e
    except URLError as e:
        raise RuntimeError(f"Network error fetching {url}: {e.reason}") from e


def _fetch_registry():
    """Download and parse the package registry."""
    try:
        data = _fetch_url(REGISTRY_URL)
        return json.loads(data)
    except Exception as e:
        print(f"error: failed to fetch registry — {e}", file=sys.stderr)
        sys.exit(1)


def _sha256(data):
    return hashlib.sha256(data).hexdigest()


def _latest_version(versions):
    """Return the latest version string from a dict of version → info."""
    # Simple: sort by version string parts
    def ver_key(v):
        return tuple(int(x) for x in v.split(".") if x.isdigit())
    return max(versions.keys(), key=ver_key)


# ── install ──────────────────────────────────────────────────────────────────

def cmd_install(args):
    if args:
        pkg_name = args[0]
        version = args[1] if len(args) > 1 else None
        _install_one(pkg_name, version)
    else:
        # Install all from ul.toml
        try:
            m = Manifest.load()
        except FileNotFoundError:
            print(f"error: {MANIFEST_NAME} not found. Run 'new' to create a project first.")
            sys.exit(1)
        if not m.dependencies:
            print("No dependencies in ul.toml.")
            return
        for name, ver in m.dependencies.items():
            _install_one(name, ver)


def _install_one(name, version=None):
    registry = _fetch_registry()
    packages = registry.get("packages", {})

    if name not in packages:
        print(f"error: package '{name}' not found in registry.")
        print(f"Available packages: {', '.join(packages.keys()) or 'none'}")
        sys.exit(1)

    versions = packages[name].get("versions", {})
    if not versions:
        print(f"error: package '{name}' has no versions.")
        sys.exit(1)

    if version and version not in versions:
        print(f"error: version '{version}' not found for '{name}'. Available: {', '.join(versions.keys())}")
        sys.exit(1)

    ver = version or _latest_version(versions)
    info = versions[ver]
    url = info["url"]
    expected_sha = info.get("sha256", "")

    # Download
    print(f"Downloading {name}@{ver}...")
    try:
        data = _fetch_url(url)
    except RuntimeError as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(1)

    # Verify checksum if provided
    if expected_sha:
        actual_sha = _sha256(data)
        if actual_sha != expected_sha:
            print(f"error: checksum mismatch for {name}@{ver}")
            print(f"  expected: {expected_sha}")
            print(f"  got:      {actual_sha}")
            sys.exit(1)

    # Extract
    dest = os.path.join(PACKAGES_DIR, name)
    os.makedirs(dest, exist_ok=True)

    with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as tmp:
        tmp.write(data)
        tmp_path = tmp.name

    try:
        with zipfile.ZipFile(tmp_path) as zf:
            # GitHub archives have a top-level dir — strip it
            members = zf.namelist()
            prefix = ""
            if members and "/" in members[0]:
                prefix = members[0].split("/")[0] + "/"

            for member in members:
                if member.endswith("/"):
                    continue
                rel = member[len(prefix):] if member.startswith(prefix) else member
                if not rel:
                    continue
                out_path = os.path.join(dest, rel)
                os.makedirs(os.path.dirname(out_path), exist_ok=True)
                with zf.open(member) as src, open(out_path, "wb") as dst:
                    dst.write(src.read())
    except zipfile.BadZipFile:
        print(f"error: downloaded file for {name}@{ver} is not a valid zip.")
        sys.exit(1)
    finally:
        os.unlink(tmp_path)

    # Update ul.toml if it exists
    try:
        m = Manifest.load()
        m.add_dependency(name, ver)
    except FileNotFoundError:
        pass

    print(f"Installed {name}@{ver}")


# ── remove ───────────────────────────────────────────────────────────────────

def cmd_remove(args):
    if not args:
        print("usage: remove <packagename>")
        sys.exit(1)

    name = args[0]
    pkg_dir = os.path.join(PACKAGES_DIR, name)

    if os.path.isdir(pkg_dir):
        shutil.rmtree(pkg_dir)
        print(f"Removed {pkg_dir}/")
    else:
        print(f"Package directory {pkg_dir}/ not found.")

    try:
        m = Manifest.load()
        if m.remove_dependency(name):
            print(f"Removed '{name}' from {MANIFEST_NAME}")
        else:
            print(f"'{name}' not in {MANIFEST_NAME} dependencies")
    except FileNotFoundError:
        pass


# ── list ─────────────────────────────────────────────────────────────────────

def cmd_list(args):
    try:
        m = Manifest.load()
    except FileNotFoundError:
        print(f"No {MANIFEST_NAME} found.")
        sys.exit(1)

    if not m.dependencies:
        print("No dependencies.")
        return

    print(f"Dependencies ({len(m.dependencies)}):")
    for name, ver in sorted(m.dependencies.items()):
        installed = os.path.isdir(os.path.join(PACKAGES_DIR, name))
        status = "installed" if installed else "not installed"
        print(f"  {name} = \"{ver}\"  ({status})")


# ── new ──────────────────────────────────────────────────────────────────────

def cmd_new(args):
    if not args:
        print("usage: new <projectname>")
        sys.exit(1)

    name = args[0]
    if os.path.exists(name):
        print(f"error: directory '{name}' already exists.")
        sys.exit(1)

    os.makedirs(name)
    os.makedirs(os.path.join(name, PACKAGES_DIR))

    Manifest.create(name, name)

    main_ul = os.path.join(name, "main.ul")
    with open(main_ul, "w") as f:
        f.write(f'function main()\n  print "Hello from {name}"\nend function\n')

    print(f"Created project '{name}'.")
    print(f"  cd {name}")
    print(f"  python Main.py main.ul --run")


# ── publish ──────────────────────────────────────────────────────────────────

def cmd_publish(args):
    try:
        m = Manifest.load()
    except FileNotFoundError:
        print(f"error: {MANIFEST_NAME} not found. Run 'new' first.")
        sys.exit(1)

    name = m.project["name"]
    version = m.project["version"]

    # Collect .ul files
    ul_files = []
    for root, dirs, files in os.walk("."):
        dirs[:] = [d for d in dirs if d != PACKAGES_DIR and not d.startswith(".")]
        for f in files:
            if f.endswith(".ul") or f == MANIFEST_NAME:
                ul_files.append(os.path.join(root, f))

    # Create zip
    zip_name = f"{name}-{version}.zip"
    with zipfile.ZipFile(zip_name, "w", zipfile.ZIP_DEFLATED) as zf:
        for f in ul_files:
            zf.write(f)

    sha = _sha256(open(zip_name, "rb").read())

    print(f"Packaged {len(ul_files)} files into {zip_name}")
    print(f"SHA256: {sha}")
    print()
    print("To publish, open a PR to https://github.com/albazzaztariq/UniLogic")
    print("adding your package to registry/packages.json:")
    print()
    print(json.dumps({
        name: {
            "versions": {
                version: {
                    "url": f"https://github.com/albazzaztariq/UniLogic/releases/download/{name}-{version}/{zip_name}",
                    "sha256": sha,
                    "description": m.project.get("description", ""),
                    "entry": m.project.get("entry", "main.ul"),
                }
            }
        }
    }, indent=2))


# ── run ──────────────────────────────────────────────────────────────────────

def cmd_run(args):
    """Run the project entry file via the interpreter."""
    try:
        m = Manifest.load()
    except FileNotFoundError:
        print(f"error: No {MANIFEST_NAME} found. Run 'new <name>' to create a project.")
        sys.exit(1)

    entry = m.project.get("entry", "main.ul")
    if not os.path.isfile(entry):
        print(f"error: entry file '{entry}' not found.")
        sys.exit(1)

    # Find Main.py
    main_py = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Main.py")
    if not os.path.isfile(main_py):
        print("error: Main.py not found.")
        sys.exit(1)

    result = subprocess.run([sys.executable, main_py, entry, "--run"] + args,
                            cwd=os.getcwd())
    sys.exit(result.returncode)


# ── build ────────────────────────────────────────────────────────────────────

def cmd_build(args):
    """Build the project entry file to the configured target."""
    try:
        m = Manifest.load()
    except FileNotFoundError:
        print(f"error: No {MANIFEST_NAME} found. Run 'new <name>' to create a project.")
        sys.exit(1)

    entry = m.project.get("entry", "main.ul")
    if not os.path.isfile(entry):
        print(f"error: entry file '{entry}' not found.")
        sys.exit(1)

    target = m.build.get("target", "c")
    # Map build target names to compiler flags
    target_map = {"native": "native", "c": "c", "python": "python", "js": "js",
                  "llvm": "llvm", "wasm": "wasm", "bytecode": "bytecode"}
    flag = target_map.get(target, "c")

    os.makedirs("build", exist_ok=True)

    # Determine output extension
    ext_map = {"c": ".c", "python": ".py", "js": ".js", "llvm": ".ll",
               "wasm": ".wasm", "bytecode": ".ulb", "native": (".exe" if sys.platform == "win32" else "")}
    ext = ext_map.get(flag, ".c")
    out_name = os.path.splitext(os.path.basename(entry))[0] + ext
    out_path = os.path.join("build", out_name)

    main_py = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Main.py")
    if not os.path.isfile(main_py):
        print("error: Main.py not found.")
        sys.exit(1)

    result = subprocess.run([sys.executable, main_py, entry, "-t", flag, "-o", out_path] + args,
                            cwd=os.getcwd())
    sys.exit(result.returncode)


# ── test ─────────────────────────────────────────────────────────────────────

def cmd_test(args):
    """Run all .ul files in tests/ directory, report pass/fail for all targets."""
    test_dir = "tests"
    if not os.path.isdir(test_dir):
        print(f"No '{test_dir}/' directory found. Create it and add test .ul files.")
        sys.exit(1)

    test_files = sorted(f for f in os.listdir(test_dir) if f.endswith(".ul"))
    if not test_files:
        print(f"No .ul files found in {test_dir}/.")
        sys.exit(1)

    main_py = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Main.py")
    if not os.path.isfile(main_py):
        print("error: Main.py not found.")
        sys.exit(1)

    # Define targets: (label, cli_args)
    targets = [
        ("Interp", ["--run"]),
        ("C",      ["-t", "c"]),
        ("Python", ["-t", "python"]),
        ("JS",     ["-t", "js"]),
        ("Bytecode", ["-t", "bytecode", "--vm"]),
    ]

    # Run all tests for all targets
    results = {}  # (test_file, target_label) → bool
    for tf in test_files:
        test_path = os.path.join(test_dir, tf)
        for label, cli_args in targets:
            result = subprocess.run(
                [sys.executable, main_py, test_path] + cli_args,
                capture_output=True, text=True, cwd=os.getcwd())
            results[(tf, label)] = (result.returncode == 0)

    # Print table
    labels = [t[0] for t in targets]
    name_width = max(len(tf) for tf in test_files)
    col_width = max(max(len(l) for l in labels), 4)

    header = f"{'Test':<{name_width}}  " + "  ".join(f"{l:^{col_width}}" for l in labels)
    print(header)
    print("-" * len(header))

    target_passed = {l: 0 for l in labels}
    for tf in test_files:
        row = f"{tf:<{name_width}}  "
        for label in labels:
            ok = results[(tf, label)]
            mark = "PASS" if ok else "FAIL"
            row += f"{mark:^{col_width}}  "
            if ok:
                target_passed[label] += 1
        print(row)

    total = len(test_files)
    print("-" * len(header))
    summary = f"{'Total':<{name_width}}  "
    for label in labels:
        p = target_passed[label]
        summary += f"{p}/{total}".center(col_width) + "  "
    print(summary)

    # Exit non-zero if any target has failures
    any_fail = any(not v for v in results.values())
    sys.exit(1 if any_fail else 0)


# ── init ─────────────────────────────────────────────────────────────────────

def cmd_init(args):
    """Initialize a UL project in the current directory."""
    if os.path.isfile(MANIFEST_NAME):
        print(f"{MANIFEST_NAME} already exists in this directory.")
        sys.exit(1)

    name = args[0] if args else os.path.basename(os.getcwd())

    os.makedirs(PACKAGES_DIR, exist_ok=True)
    Manifest.create(".", name)

    # Create main.ul if it doesn't exist
    entry = "main.ul"
    if not os.path.isfile(entry):
        with open(entry, "w") as f:
            f.write(f'function main()\n  print "Hello from {name}"\nend function\n')
        print(f"Created {entry}")

    print(f"Initialized project '{name}' in {os.getcwd()}")


# ── compiler sync check ─────────────────────────────────────────────────────

def _check_compiler_sync():
    """Warn if bundled compiler files are stale."""
    tools_dir = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(tools_dir)
    bundle_dir = os.path.join(root, "LSP", "vscode-ul", "compiler")
    if not os.path.isdir(bundle_dir):
        return

    files = [
        ("Lexer", "lexer.py"), ("Parser", "parser.py"), ("AST", "ast_nodes.py"),
        ("Semantic", "semcheck.py"), ("Codegen", "codegen_c.py"), ("Compiler", "normalize.py"),
    ]
    stale = False
    for subdir, filename in files:
        src = os.path.join(root, subdir, filename)
        dst = os.path.join(bundle_dir, filename)
        if os.path.isfile(src) and os.path.isfile(dst):
            if os.path.getmtime(src) > os.path.getmtime(dst):
                stale = True
                break
    if stale:
        sync_script = os.path.join(root, "LSP", "sync_compiler.py")
        print(f"Note: Compiler files have been updated. Run 'python {sync_script}' to sync the VS Code extension.")


# ── dispatch ─────────────────────────────────────────────────────────────────

COMMANDS = {
    "install": cmd_install,
    "remove":  cmd_remove,
    "list":    cmd_list,
    "new":     cmd_new,
    "publish": cmd_publish,
    "run":     cmd_run,
    "build":   cmd_build,
    "test":    cmd_test,
    "init":    cmd_init,
}


def run(cmd, args):
    if cmd not in COMMANDS:
        return False
    _check_compiler_sync()
    COMMANDS[cmd](args)
    return True
    return True
