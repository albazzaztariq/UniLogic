"""
tsan_runner.py — compile with ThreadSanitizer and run.

TSan catches data races: two threads accessing the same memory concurrently
where at least one is a write and there is no synchronisation between them.

IMPORTANT: TSan cannot be combined with ASan in the same binary.
           Run this as a separate pass for multi-threaded code.
"""

import os
import shutil
import subprocess
import tempfile
from ..core.attestation import ToolResult

_TSAN_FLAGS = ['-fsanitize=thread', '-O1', '-Wall']


def _gcc_version() -> str:
    try:
        r = subprocess.run(['gcc', '--version'], capture_output=True,
                           text=True, timeout=10)
        return r.stdout.splitlines()[0].strip() if r.stdout else 'unknown'
    except Exception:
        return 'unknown'


def run_tsan(c_file: str, run_timeout: int = 30) -> ToolResult:
    if not shutil.which('gcc'):
        return ToolResult(
            tool='tsan', version='gcc-not-found', passed=False,
            stdout='', stderr='gcc not found on PATH',
        )

    with tempfile.TemporaryDirectory() as tmpdir:
        exe = os.path.join(tmpdir, 'tsan_test')
        compile_cmd = ['gcc', *_TSAN_FLAGS, '-o', exe, c_file]

        try:
            cr = subprocess.run(compile_cmd, capture_output=True, text=True,
                                encoding='utf-8', errors='replace', timeout=60)
        except subprocess.TimeoutExpired:
            return ToolResult(
                tool='tsan', version=_gcc_version(), passed=False,
                stdout='', stderr='Compilation timed out',
            )

        if cr.returncode != 0:
            return ToolResult(
                tool='tsan', version=_gcc_version(), passed=False,
                stdout=cr.stdout, stderr=cr.stderr,
            )

        try:
            rr = subprocess.run([exe], capture_output=True, text=True,
                                encoding='utf-8', errors='replace',
                                timeout=run_timeout)
        except subprocess.TimeoutExpired:
            return ToolResult(
                tool='tsan', version=_gcc_version(), passed=False,
                stdout='', stderr=f'Binary timed out after {run_timeout}s',
            )

        race_detected = 'DATA RACE' in rr.stderr or 'ThreadSanitizer' in rr.stderr
        passed = rr.returncode == 0 and not race_detected

        return ToolResult(
            tool    = 'tsan',
            version = _gcc_version(),
            passed  = passed,
            stdout  = rr.stdout,
            stderr  = cr.stderr + rr.stderr,
        )
