"""
framac_runner.py — run Frama-C/WP on annotated C source and parse results.

Frama-C/WP attempts to prove ACSL annotations using an SMT solver (Alt-Ergo,
Z3, or CVC5).  It returns a structured report of which proof obligations were
discharged ("Valid"), which timed out, and which are unknown.

Install Frama-C:
  Ubuntu/Debian: sudo apt install frama-c
  macOS:         brew install frama-c
  Or via opam:   opam install frama-c

Install an SMT solver (at least one):
  sudo apt install why3 alt-ergo
  pip install z3-solver   (then: why3 config)
"""

import os
import re
import shutil
import subprocess
import tempfile
from dataclasses import dataclass, field


@dataclass
class FramaCResult:
    passed:        bool
    total_goals:   int  = 0
    valid_goals:   int  = 0
    failed_count:  int  = 0
    failures_text: str  = ""   # raw WP output for the failed obligations
    stdout:        str  = ""
    stderr:        str  = ""
    version:       str  = ""


def _framac_version() -> str:
    try:
        r = subprocess.run(['frama-c', '--version'], capture_output=True,
                           text=True, timeout=10)
        return r.stdout.strip().splitlines()[0] if r.stdout else 'unknown'
    except Exception:
        return 'unknown'


def run_framac(source: str | None = None,
               c_file: str | None = None,
               timeout: int = 180) -> FramaCResult:
    """
    Run Frama-C/WP on either a source string or a .c file path.

    If `source` is given, it is written to a temp file first.
    Returns a FramaCResult with pass/fail and structured failure output.
    """
    if not shutil.which('frama-c'):
        return FramaCResult(
            passed=False,
            failures_text='frama-c not found on PATH',
            version='not-installed',
        )

    with tempfile.TemporaryDirectory() as tmpdir:
        if source is not None:
            c_file_path = os.path.join(tmpdir, 'input.c')
            with open(c_file_path, 'w', encoding='utf-8') as f:
                f.write(source)
        else:
            c_file_path = c_file

        cmd = [
            'frama-c',
            '-wp',                      # activate WP plugin
            '-wp-rte',                  # add runtime error proof obligations
            '-wp-timeout', '30',        # per-goal timeout in seconds
            '-wp-prover', 'alt-ergo,z3',# try both provers
            '-wp-log', 'r:wp_report',   # write machine-readable report
            c_file_path,
        ]

        try:
            r = subprocess.run(cmd, capture_output=True, text=True,
                               encoding='utf-8', errors='replace',
                               timeout=timeout, cwd=tmpdir)
        except subprocess.TimeoutExpired:
            return FramaCResult(
                passed=False,
                failures_text=f'Frama-C timed out after {timeout}s',
                version=_framac_version(),
            )

        return _parse_output(r.stdout, r.stderr)


_RE_TOTAL   = re.compile(r'(\d+)\s+goal', re.IGNORECASE)
_RE_VALID   = re.compile(r'(\d+)\s+(?:goal\s+)?(?:valid|proved)', re.IGNORECASE)
_RE_FAILED  = re.compile(r'(\d+)\s+(?:goal\s+)?(?:failed|unknown|timeout)', re.IGNORECASE)


def _parse_output(stdout: str, stderr: str) -> FramaCResult:
    combined = stdout + stderr

    total_m  = _RE_TOTAL.search(combined)
    valid_m  = _RE_VALID.search(combined)
    failed_m = _RE_FAILED.search(combined)

    total   = int(total_m.group(1))  if total_m  else 0
    valid   = int(valid_m.group(1))  if valid_m  else 0
    failed  = int(failed_m.group(1)) if failed_m else 0

    # Extract just the failure lines for the feedback loop
    failure_lines = [
        line for line in combined.splitlines()
        if any(kw in line for kw in ('Unknown', 'Timeout', 'Failed', 'invalid'))
    ]
    failures_text = '\n'.join(failure_lines)

    passed = failed == 0 and valid > 0

    return FramaCResult(
        passed        = passed,
        total_goals   = total,
        valid_goals   = valid,
        failed_count  = failed,
        failures_text = failures_text,
        stdout        = stdout,
        stderr        = stderr,
        version       = _framac_version(),
    )
