"""
asan_runner.py — compile with AddressSanitizer + UndefinedBehaviourSanitizer
and run the resulting binary.

ASan catches at runtime:
  - heap/stack/global buffer overflow
  - use-after-free, use-after-return
  - double-free, invalid-free

UBSan catches at runtime:
  - signed integer overflow
  - null pointer dereference
  - misaligned access
  - invalid enum values

NOTE: These only fire on executed paths.  Pair with CBMC for static coverage.
NOTE: Cannot be combined with TSan in the same binary.
"""

import os
import shutil
import subprocess
import tempfile
from ..core.attestation import ToolResult

_SANITIZE_FLAGS = [
    '-fsanitize=address,undefined',
    '-fno-omit-frame-pointer',
    '-O1',   # O1 keeps stack frames meaningful; O0 is too slow
    '-Wall',
]


def _gcc_version() -> str:
    try:
        r = subprocess.run(['gcc', '--version'], capture_output=True,
                           text=True, timeout=10)
        return r.stdout.splitlines()[0].strip() if r.stdout else 'unknown'
    except Exception:
        return 'unknown'


def run_asan(c_file: str, run_timeout: int = 30) -> ToolResult:
    """
    Compile c_file with ASan+UBSan, run it, and return a ToolResult.

    passed=True means: compiled cleanly AND ran without any sanitiser report.

    If the binary has no main() or requires arguments, the run will likely
    fail with a non-zero exit — that is expected and reported honestly.
    """
    if not shutil.which('gcc'):
        return ToolResult(
            tool='asan+ubsan', version='gcc-not-found', passed=False,
            stdout='', stderr='gcc not found on PATH',
        )

    with tempfile.TemporaryDirectory() as tmpdir:
        exe = os.path.join(tmpdir, 'asan_test')
        compile_cmd = ['gcc', *_SANITIZE_FLAGS, '-o', exe, c_file]

        # --- Compile ---
        try:
            cr = subprocess.run(compile_cmd, capture_output=True, text=True,
                                encoding='utf-8', errors='replace', timeout=60)
        except subprocess.TimeoutExpired:
            return ToolResult(
                tool='asan+ubsan', version=_gcc_version(), passed=False,
                stdout='', stderr='Compilation timed out',
            )

        if cr.returncode != 0:
            return ToolResult(
                tool='asan+ubsan', version=_gcc_version(), passed=False,
                stdout=cr.stdout, stderr=cr.stderr,
            )

        # --- Run ---
        try:
            rr = subprocess.run([exe], capture_output=True, text=True,
                                encoding='utf-8', errors='replace',
                                timeout=run_timeout)
        except subprocess.TimeoutExpired:
            return ToolResult(
                tool='asan+ubsan', version=_gcc_version(), passed=False,
                stdout='', stderr=f'Binary timed out after {run_timeout}s',
            )

        # ASan/UBSan write reports to stderr.  Any "ERROR:" line = failure.
        asan_triggered = 'ERROR:' in rr.stderr or 'runtime error:' in rr.stderr
        passed = rr.returncode == 0 and not asan_triggered

        return ToolResult(
            tool    = 'asan+ubsan',
            version = _gcc_version(),
            passed  = passed,
            stdout  = rr.stdout,
            stderr  = cr.stderr + rr.stderr,
        )
