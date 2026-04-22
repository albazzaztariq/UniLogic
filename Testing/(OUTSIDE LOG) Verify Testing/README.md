# Verify Testing Tools

Tools for monitoring and verifying XPile transpilation work. Claude uses these for all build operations. The user monitors via the dashboard.

## Files

| File | Purpose |
|------|---------|
| `xpile_wrap.py` | Build wrapper. Claude runs ALL compiles through this. Logs every attempt, copies files. |
| `xpile_dashboard.py` | Live terminal dashboard. Shows build entries, auto-compares files, alerts on discrepancies. |
| `xpile_fuzz.py` | Differential fuzzer. Runs two executables with same inputs, reports any output difference. |
| `xpile-dashboard.bat` | Windows shortcut. Run `xpile-dashboard` from this folder. |

## Usage

### Start the dashboard (run in its own terminal):
```
xpile-dashboard
```

### Claude uses the wrapper for ALL builds:
```
python xpile_wrap.py <source_file> <output_file> <target>
```
Examples:
```
python xpile_wrap.py test.c test.ul ul          # C -> UL
python xpile_wrap.py test.ul test_rt.c c        # UL -> C
python xpile_wrap.py test.c test.exe native      # C -> binary
```

### Fuzz test two executables:
```
python xpile_fuzz.py original.exe roundtrip.exe file.ul --rounds 100
```

## What the dashboard shows

Every build attempt appears as:
```
3:45 PM | SUCCESS | c2ul | C -> UniLogic | redis_zipint
```

When two files with the same extension appear in the same test folder, they're automatically compared:
- **MATCH**: green entry, no prompt
- **DISCREPANCY**: red alert, waits for OK/D (details)

## Where files go

- Test data: `LANG\UniLogic\CURRENT\XPile\VerifyTest\<library_name>\`
- Dashboard log: `VerifyTest\dashboard.jsonl`
- Each build copies: compiler script, source file, output file (no binaries)
- Failed builds still copy files so you can see what broke

## All files are READ-ONLY

Claude cannot modify these tools. Only the user can unlock them with `attrib -R <file>`.
