#!/usr/bin/env python3
"""profile_reader.py — UniLogic profile log analyzer

Reads ul_profile.log (NDJSON) produced by --profile compilation,
outputs per-function timing stats and memory allocation summary.

Usage:
    python profile_reader.py [ul_profile.log]

Outputs:
    - Per-function stats table (call count, total/min/max/avg time)
    - Memory allocation summary (total allocs, total bytes, peak, leaks)
    - Writes ul_profile_summary.json
"""

import sys
import json
import os


def read_log(path):
    entries = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if line:
                entries.append(json.loads(line))
    return entries


def analyze(entries):
    # Per-function timing
    func_stats = {}  # name -> {calls, total_ns, min_ns, max_ns}
    for e in entries:
        if e.get("event") == "exit" and "elapsed_ns" in e:
            name = e["func"]
            elapsed = e["elapsed_ns"]
            if name not in func_stats:
                func_stats[name] = {
                    "calls": 0, "total_ns": 0,
                    "min_ns": float("inf"), "max_ns": 0,
                }
            s = func_stats[name]
            s["calls"] += 1
            s["total_ns"] += elapsed
            s["min_ns"] = min(s["min_ns"], elapsed)
            s["max_ns"] = max(s["max_ns"], elapsed)

    # Compute averages
    for name, s in func_stats.items():
        s["avg_ns"] = s["total_ns"] / s["calls"] if s["calls"] > 0 else 0
        if s["min_ns"] == float("inf"):
            s["min_ns"] = 0

    # Memory allocation tracking
    alloc_ptrs = {}  # ptr -> size
    total_allocs = 0
    total_bytes = 0
    peak_live = 0
    live_bytes = 0
    leak_candidates = []

    # DR behavior tracking
    gc_collects = 0
    refcount_ops = 0
    arena_resets = 0

    for e in entries:
        ev = e.get("event")
        if ev == "alloc":
            ptr = e.get("ptr", "")
            size = e.get("size", 0)
            alloc_ptrs[ptr] = size
            total_allocs += 1
            total_bytes += size
            live_bytes += size
            if live_bytes > peak_live:
                peak_live = live_bytes
        elif ev == "free":
            ptr = e.get("ptr", "")
            if ptr in alloc_ptrs:
                live_bytes -= alloc_ptrs.pop(ptr)
        elif ev == "gc_collect":
            gc_collects += 1
        elif ev == "refcount":
            refcount_ops += 1
        elif ev == "arena_reset":
            arena_resets += 1
        elif ev == "summary":
            # Use summary event data if present (C target)
            total_allocs = max(total_allocs, e.get("total_allocs", 0))
            total_bytes = max(total_bytes, e.get("total_bytes", 0))
            peak_live = max(peak_live, e.get("peak_live_bytes", 0))

    # Remaining allocations are leak candidates
    for ptr, size in alloc_ptrs.items():
        leak_candidates.append({"ptr": ptr, "size": size})

    dr_summary = {
        "gc_collects": gc_collects,
        "refcount_ops": refcount_ops,
        "arena_resets": arena_resets,
    }

    mem_summary = {
        "total_allocs": total_allocs,
        "total_bytes": total_bytes,
        "peak_live_bytes": peak_live,
        "leak_candidates": len(leak_candidates),
        "leaks": leak_candidates,
    }

    return func_stats, mem_summary, dr_summary


def fmt_time(ns):
    if ns >= 1_000_000_000:
        return f"{ns / 1e9:.3f}s"
    if ns >= 1_000_000:
        return f"{ns / 1e6:.3f}ms"
    if ns >= 1_000:
        return f"{ns / 1e3:.1f}us"
    return f"{ns:.0f}ns"


def print_report(func_stats, mem_summary, dr_summary):
    print("=" * 72)
    print("  UniLogic Profile Report")
    print("=" * 72)
    print()

    if func_stats:
        print("  Per-Function Timing")
        print("  " + "-" * 68)
        header = f"  {'Function':<24} {'Calls':>6} {'Total':>10} {'Min':>10} {'Max':>10} {'Avg':>10}"
        print(header)
        print("  " + "-" * 68)
        for name in sorted(func_stats, key=lambda n: func_stats[n]["total_ns"], reverse=True):
            s = func_stats[name]
            print(f"  {name:<24} {s['calls']:>6} {fmt_time(s['total_ns']):>10} "
                  f"{fmt_time(s['min_ns']):>10} {fmt_time(s['max_ns']):>10} "
                  f"{fmt_time(s['avg_ns']):>10}")
        print()

    print("  Memory Allocation Summary")
    print("  " + "-" * 68)
    m = mem_summary
    print(f"  Total allocations:  {m['total_allocs']}")
    print(f"  Total bytes:        {m['total_bytes']}")
    print(f"  Peak live bytes:    {m['peak_live_bytes']}")
    print(f"  Leak candidates:    {m['leak_candidates']}")
    if m["leaks"]:
        print()
        for lk in m["leaks"]:
            print(f"    leaked: {lk['ptr']}  ({lk['size']} bytes)")
    print()

    dr = dr_summary
    if dr["gc_collects"] or dr["refcount_ops"] or dr["arena_resets"]:
        print("  DR Behavior Monitor")
        print("  " + "-" * 68)
        print(f"  GC collections:     {dr['gc_collects']}")
        print(f"  Refcount operations: {dr['refcount_ops']}")
        print(f"  Arena resets:        {dr['arena_resets']}")
        print()

    print("=" * 72)


def write_summary(func_stats, mem_summary, dr_summary, out_path):
    summary = {
        "functions": {},
        "memory": {
            "total_allocs": mem_summary["total_allocs"],
            "total_bytes": mem_summary["total_bytes"],
            "peak_live_bytes": mem_summary["peak_live_bytes"],
            "leak_candidates": mem_summary["leak_candidates"],
        },
        "dr": dr_summary,
    }
    for name, s in func_stats.items():
        summary["functions"][name] = {
            "calls": s["calls"],
            "total_ns": s["total_ns"],
            "min_ns": s["min_ns"],
            "max_ns": s["max_ns"],
            "avg_ns": s["avg_ns"],
        }
    with open(out_path, "w") as f:
        json.dump(summary, f, indent=2)
    print(f"  Summary written to {out_path}")


def main():
    log_path = sys.argv[1] if len(sys.argv) > 1 else "ul_profile.log"
    if not os.path.isfile(log_path):
        print(f"error: {log_path} not found", file=sys.stderr)
        sys.exit(1)

    entries = read_log(log_path)
    func_stats, mem_summary, dr_summary = analyze(entries)
    print_report(func_stats, mem_summary, dr_summary)

    out_path = os.path.splitext(log_path)[0] + "_summary.json"
    write_summary(func_stats, mem_summary, dr_summary, out_path)


if __name__ == "__main__":
    main()
