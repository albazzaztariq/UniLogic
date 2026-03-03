"""
report.py — human-readable and GitHub-friendly report generation.
"""

import json
import datetime
from dataclasses import asdict
from .attestation import Attestation


_PASS = "PASS"
_FAIL = "FAIL"
_TICK = "[OK]"
_CROSS = "[FAIL]"


def _fmt_time(epoch: int) -> str:
    return datetime.datetime.utcfromtimestamp(epoch).strftime('%Y-%m-%d %H:%M:%S UTC')


def generate_summary(att: Attestation) -> str:
    """Single-line summary for CLI output."""
    status = _PASS if att.overall_pass else _FAIL
    return (
        f"MemResistant [{status}]  "
        f"source={att.source_hash[:12]}...  "
        f"time={_fmt_time(att.timestamp)}"
    )


def generate_report(att: Attestation) -> str:
    """Full human-readable text report."""
    lines = [
        "=" * 64,
        "  MemResistant Attestation Report",
        "=" * 64,
        f"  Source file : {att.source_path}",
        f"  Source hash : {att.source_hash}",
        f"  Timestamp   : {_fmt_time(att.timestamp)}",
        f"  Fingerprint : {att.fingerprint}",
        f"  Overall     : {_PASS if att.overall_pass else _FAIL}",
        "",
        "  Tool Results",
        "  " + "-" * 44,
    ]
    for r in att.tool_results:
        icon = _TICK if r['passed'] else _CROSS
        lines.append(f"  {icon}  {r['tool']:<10} v{r['version']}")
        if not r['passed'] and r['stderr'].strip():
            for line in r['stderr'].strip().splitlines()[:10]:
                lines.append(f"        {line}")
    lines.append("=" * 64)
    return "\n".join(lines)


def generate_github_markdown(att: Attestation) -> str:
    """GitHub-flavoured markdown suitable for posting as a file or issue comment."""
    status_badge = "![PASS](https://img.shields.io/badge/MemResistant-PASS-brightgreen)" \
                   if att.overall_pass else \
                   "![FAIL](https://img.shields.io/badge/MemResistant-FAIL-red)"

    rows = []
    for r in att.tool_results:
        icon = "✅" if r['passed'] else "❌"
        rows.append(f"| {r['tool']} | {r['version']} | {icon} |")

    tool_table = "\n".join([
        "| Tool | Version | Result |",
        "|------|---------|--------|",
        *rows,
    ])

    return f"""\
# MemResistant Attestation {status_badge}

| Field | Value |
|-------|-------|
| Source hash | `{att.source_hash}` |
| Attestation fingerprint | `{att.fingerprint}` |
| Timestamp | {_fmt_time(att.timestamp)} |
| Overall | {'**PASS**' if att.overall_pass else '**FAIL**'} |

## Tool Results

{tool_table}

<details>
<summary>Full attestation JSON</summary>

```json
{json.dumps(asdict(att), indent=2)}
```

</details>

> Anyone can independently re-run the listed tool versions on the source
> file matching the source hash and verify the outputs match.
"""
