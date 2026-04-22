#!/usr/bin/env python3
"""xpile_dashboard.py — Terminal dashboard for monitoring build attempts + file comparison.

Usage:
    python xpile_dashboard.py              # live dashboard (auto-refreshes, auto-compares)
    python xpile_dashboard.py --compare    # interactive file comparison mode
    python xpile_dashboard.py --compare fileA fileB   # compare two specific files

Dashboard shows every build attempt with timestamp, status, files involved.
When new files arrive with matching extensions, queues a comparison and waits for OK.
"""

import sys
import os
import json
import time
import difflib
import threading

XPILE_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VERIFY_DIR = os.path.join(XPILE_ROOT, "VerifyTest")

# Allow --path override
if "--path" in sys.argv:
    _pi = sys.argv.index("--path")
    if _pi + 1 < len(sys.argv):
        VERIFY_DIR = sys.argv[_pi + 1]
        sys.argv.pop(_pi)
        sys.argv.pop(_pi)

DASHBOARD_FILE = os.path.join(VERIFY_DIR, "dashboard.jsonl")

# Map extensions to language names
EXT_LANG = {
    ".c": "C",
    ".ul": "UniLogic",
    ".py": "Python",
    ".js": "JavaScript",
    ".java": "Java",
    ".exe": "Native Binary",
    ".o": "Object File",
}


def min_edit_distance(s1, s2):
    """Count minimum character edits (insertions, deletions, substitutions).
    Levenshtein distance — 'aaaa' vs ' aaaa' = 1, not 4."""
    m, n = len(s1), len(s2)
    if m > 50000 or n > 50000:
        lines1 = s1.splitlines(keepends=True)
        lines2 = s2.splitlines(keepends=True)
        diff = list(difflib.unified_diff(lines1, lines2, n=0))
        added = sum(1 for l in diff if l.startswith("+") and not l.startswith("+++"))
        removed = sum(1 for l in diff if l.startswith("-") and not l.startswith("---"))
        return max(added, removed)
    if m > 10000 or n > 10000:
        lines1 = s1.splitlines()
        lines2 = s2.splitlines()
        s1, s2 = lines1, lines2
        m, n = len(s1), len(s2)
    prev = list(range(n + 1))
    for i in range(1, m + 1):
        curr = [i] + [0] * n
        for j in range(1, n + 1):
            if s1[i - 1] == s2[j - 1]:
                curr[j] = prev[j - 1]
            else:
                curr[j] = 1 + min(prev[j], curr[j - 1], prev[j - 1])
        prev = curr
    return prev[n]


def compare_files(path1, path2, silent=False):
    """Compare two files and report discrepancy. Returns (identical, edit_distance)."""
    if not os.path.isfile(path1):
        if not silent:
            print(f"  File not found: {path1}")
        return False, -1
    if not os.path.isfile(path2):
        if not silent:
            print(f"  File not found: {path2}")
        return False, -1

    with open(path1, encoding="utf-8", errors="replace") as f:
        content1 = f.read()
    with open(path2, encoding="utf-8", errors="replace") as f:
        content2 = f.read()

    if content1 == content2:
        if not silent:
            print(f"  IDENTICAL — {len(content1)} chars, {content1.count(chr(10))} lines")
        return True, 0

    dist = min_edit_distance(content1, content2)
    total = max(len(content1), len(content2))
    pct = (dist / total * 100) if total > 0 else 0

    if not silent:
        print(f"  DIFFERENT — {dist} edit(s), {pct:.1f}% different")
        print(f"  File 1: {len(content1)} chars, {content1.count(chr(10))} lines")
        print(f"  File 2: {len(content2)} chars, {content2.count(chr(10))} lines")

        lines1 = content1.splitlines()
        lines2 = content2.splitlines()
        diff_count = 0
        for i, (a, b) in enumerate(zip(lines1, lines2)):
            if a != b:
                diff_count += 1
                if diff_count <= 5:
                    print(f"  Line {i+1}:")
                    print(f"    < {a[:100]}")
                    print(f"    > {b[:100]}")
        if len(lines1) != len(lines2):
            print(f"  Line count differs: {len(lines1)} vs {len(lines2)}")

    return False, dist


def get_lang(filename):
    """Get language name from file extension."""
    ext = os.path.splitext(filename)[1].lower()
    return EXT_LANG.get(ext, ext)


def find_pairs(folder):
    """Find files with the same extension in a folder that can be compared."""
    files = {}
    for f in sorted(os.listdir(folder)):
        ext = os.path.splitext(f)[1]
        if ext not in files:
            files[ext] = []
        files[ext].append(f)

    pairs = []
    for ext, flist in files.items():
        if len(flist) >= 2:
            for i in range(len(flist)):
                for j in range(i + 1, len(flist)):
                    pairs.append((flist[i], flist[j]))
    return pairs


def get_latest_pair_by_ext(folder, ext):
    """Get the two most recently modified files with the given extension."""
    files = []
    for f in os.listdir(folder):
        if os.path.splitext(f)[1] == ext:
            full = os.path.join(folder, f)
            files.append((os.path.getmtime(full), f))
    files.sort(reverse=True)  # newest first
    if len(files) >= 2:
        return files[0][1], files[1][1]
    return None, None


def scan_folders_for_new_pairs(known_files):
    """Scan all VerifyTest subfolders for new same-extension file pairs.
    Returns list of (folder, file1, file2, ext) that are new."""
    new_pairs = []
    if not os.path.isdir(VERIFY_DIR):
        return new_pairs

    for d in os.listdir(VERIFY_DIR):
        folder = os.path.join(VERIFY_DIR, d)
        if not os.path.isdir(folder):
            continue

        # Group files by extension
        by_ext = {}
        for f in os.listdir(folder):
            full = os.path.join(folder, f)
            if not os.path.isfile(full):
                continue
            ext = os.path.splitext(f)[1]
            if ext not in by_ext:
                by_ext[ext] = []
            by_ext[ext].append((os.path.getmtime(full), f))

        # For each extension with 2+ files, get the latest pair
        for ext, flist in by_ext.items():
            if len(flist) < 2:
                continue
            flist.sort(reverse=True)
            f1 = flist[0][1]
            f2 = flist[1][1]
            key = (d, f1, f2)
            if key not in known_files:
                new_pairs.append((folder, d, f1, f2, ext))
                known_files.add(key)

    return new_pairs


def interactive_compare():
    """Interactive file comparison mode."""
    if not os.path.isdir(VERIFY_DIR):
        print("No VerifyTest directory found.")
        return

    folders = [d for d in sorted(os.listdir(VERIFY_DIR))
               if os.path.isdir(os.path.join(VERIFY_DIR, d))]

    if not folders:
        print("No test folders found in VerifyTest/")
        return

    while True:
        print("\n=== File Comparison ===")
        print("Available test folders:")
        for i, f in enumerate(folders):
            file_count = len(os.listdir(os.path.join(VERIFY_DIR, f)))
            print(f"  [{i+1}] {f} ({file_count} files)")
        print(f"  [q] Quit")

        choice = input("\nSelect folder: ").strip()
        if choice.lower() == "q":
            break
        try:
            idx = int(choice) - 1
            if idx < 0 or idx >= len(folders):
                continue
        except ValueError:
            continue

        folder = os.path.join(VERIFY_DIR, folders[idx])
        pairs = find_pairs(folder)

        if not pairs:
            print("No file pairs found to compare.")
            continue

        print(f"\nFile pairs in {folders[idx]}:")
        for i, (a, b) in enumerate(pairs):
            lang_a = get_lang(a)
            lang_b = get_lang(b)
            print(f"  [{i+1}] {a} ({lang_a})  vs  {b} ({lang_b})")
        print(f"  [a] Compare ALL pairs")
        print(f"  [b] Back")

        pchoice = input("\nSelect pair: ").strip()
        if pchoice.lower() == "b":
            continue
        if pchoice.lower() == "a":
            for a, b in pairs:
                lang_a = get_lang(a)
                lang_b = get_lang(b)
                print(f"\n--- {a} ({lang_a}) vs {b} ({lang_b}) ---")
                compare_files(os.path.join(folder, a), os.path.join(folder, b))
            continue
        try:
            pidx = int(pchoice) - 1
            if pidx < 0 or pidx >= len(pairs):
                continue
        except ValueError:
            continue

        a, b = pairs[pidx]
        lang_a = get_lang(a)
        lang_b = get_lang(b)
        print(f"\n--- {a} ({lang_a}) vs {b} ({lang_b}) ---")
        compare_files(os.path.join(folder, a), os.path.join(folder, b))


def live_dashboard():
    """Live terminal dashboard — shows build attempts and auto-queues comparisons."""
    print("=== XPile Build Dashboard ===")
    print("Watching for build attempts... (Ctrl+C to exit)\n")

    last_count = 0
    known_pairs = set()
    compare_queue = []

    # Seed known_pairs with existing files so we don't re-compare old stuff
    if os.path.isdir(VERIFY_DIR):
        scan_folders_for_new_pairs(known_pairs)

    try:
        while True:
            # Check for new dashboard entries
            if os.path.isfile(DASHBOARD_FILE):
                with open(DASHBOARD_FILE, encoding="utf-8") as f:
                    lines = f.readlines()

                if len(lines) > last_count:
                    for line in lines[last_count:]:
                        try:
                            entry = json.loads(line.strip())
                            ts = entry.get("timestamp", "?")[:19]
                            lib = entry.get("library", "?")
                            origin = entry.get("origin", "?")
                            target = entry.get("target_file", "?")
                            lang = entry.get("target_lang", "?")
                            compiler = entry.get("compiler", "?")
                            status = entry.get("status", "?")
                            error = entry.get("error", "")
                            output = entry.get("output", "")

                            if status == "SUCCESS":
                                status_str = f"\033[92mSUCCESS\033[0m"
                            else:
                                status_str = f"\033[91mFAILURE\033[0m"

                            # 12h HH:MM format
                            try:
                                from datetime import datetime as _dt
                                _t = _dt.fromisoformat(ts)
                                ts_short = _t.strftime("%I:%M %p").lstrip("0")
                            except Exception:
                                ts_short = ts[11:16]

                            origin_ext = os.path.splitext(origin)[1].lower()
                            target_ext = os.path.splitext(target)[1].lower()
                            src_lang = EXT_LANG.get(origin_ext, origin_ext)
                            dst_lang = EXT_LANG.get(target_ext, target_ext)

                            print(f"{ts_short} | {status_str} | {compiler} | {src_lang} -> {dst_lang} | {lib}")
                            if error:
                                print(f"  {error[:200]}")
                            elif status == "FAILURE" and output:
                                print(f"  {output[:200]}")
                        except json.JSONDecodeError:
                            pass

                    last_count = len(lines)

                    # After new entries, scan for new file pairs to compare
                    new_pairs = scan_folders_for_new_pairs(known_pairs)
                    for folder, lib_name, f1, f2, ext in new_pairs:
                        lang1 = get_lang(f1)
                        lang2 = get_lang(f2)
                        # Quick check — get discrepancy count
                        path1 = os.path.join(folder, f1)
                        path2 = os.path.join(folder, f2)
                        identical, dist = compare_files(path1, path2, silent=True)
                        compare_queue.append((lib_name, f1, f2, lang1, lang2, identical, dist, folder))

            # Process compare queue
            while compare_queue:
                lib_name, f1, f2, lang1, lang2, identical, dist, folder = compare_queue[0]

                if identical:
                    # Identical — just log it, no prompt
                    print(f"  \033[92mMATCH\033[0m {lib_name}: {f1} ({lang1}) == {f2} ({lang2})")
                    compare_queue.pop(0)
                else:
                    # DIFFERENT — alert and wait for user
                    print()
                    print("\033[91m" + "=" * 60 + "\033[0m")
                    print(f"  \033[91mDISCREPANCY DETECTED\033[0m")
                    print(f"  Library:  {lib_name}")
                    print(f"  File 1:   {f1} ({lang1})")
                    print(f"  File 2:   {f2} ({lang2})")
                    print(f"  Edits:    {dist}")
                    print("\033[91m" + "=" * 60 + "\033[0m")

                    resp = input("  [OK] Acknowledge  [D] Show details  > ").strip().upper()
                    if resp == "D":
                        path1 = os.path.join(folder, f1)
                        path2 = os.path.join(folder, f2)
                        compare_files(path1, path2)
                        input("  Press Enter to continue...")

                    compare_queue.pop(0)
                    print()

            time.sleep(1)

    except KeyboardInterrupt:
        print("\nDashboard stopped.")


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == "--compare":
        if len(sys.argv) == 4:
            print(f"Comparing: {sys.argv[2]} vs {sys.argv[3]}")
            compare_files(sys.argv[2], sys.argv[3])
        else:
            interactive_compare()
    else:
        live_dashboard()


if __name__ == "__main__":
    main()
