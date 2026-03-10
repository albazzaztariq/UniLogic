"""
cbmc_runner.py — run CBMC on a C file and return a ToolResult.

CBMC (C Bounded Model Checker) statically walks execution paths up to a
bounded loop depth and checks for:
  - array out-of-bounds
  - integer overflow
  - division by zero
  - null pointer dereference
  - memory leaks

It requires no instrumented binary — it works directly on the .c source.
Install: sudo apt install cbmc  |  brew install cbmc
"""

import shutil
import subprocess
from ..core.attestation import ToolResult

_FLAGS = [
    '--bounds-check',
    '--overflow-check',
    '--div-by-zero-check',
    '--pointer-check',
    '--memory-leak-check',
    '--signed-overflow-check',
]


def _cbmc_version() -> str:
    try:
        r = subprocess.run(['cbmc', '--version'], capture_output=True,
                           text=True, timeout=10)
        return r.stdout.strip() or r.stderr.strip() or 'unknown'
    except Exception:
        return 'unknown'


def run_cbmc(c_file: str, extra_flags: list[str] | None = None) -> ToolResult:
    """
    Run CBMC on c_file.  Returns a ToolResult with passed=True only if
    CBMC exits 0 (VERIFICATION SUCCESSFUL).

    CBMC exit codes:
      0 = verified
      6 = verification failed (counterexample found)
      10 = parse/compilation error
    """
    if not shutil.which('cbmc'):
        return ToolResult(
            tool='cbmc', version='not-installed', passed=False,
            stdout='', stderr='cbmc not found on PATH',
        )

    flags = _FLAGS + (extra_flags or [])
    cmd = ['cbmc', *flags, c_file]

    try:
        r = subprocess.run(cmd, capture_output=True, text=True,
                           encoding='utf-8', errors='replace', timeout=120)
    except subprocess.TimeoutExpired:
        return ToolResult(
            tool='cbmc', version=_cbmc_version(), passed=False,
            stdout='', stderr='CBMC timed out after 120s',
        )

    passed = r.returncode == 0
    return ToolResult(
        tool    = 'cbmc',
        version = _cbmc_version(),
        passed  = passed,
        stdout  = r.stdout,
        stderr  = r.stderr,
    )
