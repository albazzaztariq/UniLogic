@echo off
:: ul.bat — UniLogic compiler wrapper for Windows
:: Usage: ul run hello.ul, ul build, ul test, ul --version

setlocal enabledelayedexpansion

:: ── Find Main.py ────────────────────────────────────────────────────────

set "SCRIPT_DIR=%~dp0"

:: Check compiler/ subdirectory (installed by UniLogic Setup)
if exist "%SCRIPT_DIR%compiler\Main.py" (
    set "MAIN_PY=%SCRIPT_DIR%compiler\Main.py"
    goto :findpy
)

:: Check same directory (development / repo checkout)
if exist "%SCRIPT_DIR%Main.py" (
    set "MAIN_PY=%SCRIPT_DIR%Main.py"
    goto :findpy
)

:: Check UNILOGIC_HOME environment variable
if defined UNILOGIC_HOME (
    if exist "%UNILOGIC_HOME%\Main.py" (
        set "MAIN_PY=%UNILOGIC_HOME%\Main.py"
        goto :findpy
    )
)

:: Check registry (set by Project Manager or installer)
for /f "tokens=2*" %%a in ('reg query "HKCU\Software\UniLogic\ProjectManager\build" /v "compilerDir" 2^>nul') do (
    if exist "%%b\Main.py" (
        set "MAIN_PY=%%b\Main.py"
        goto :findpy
    )
)

echo Error: Cannot find the UniLogic compiler (Main.py).
echo.
echo To fix this, either:
echo   1. Run the UniLogic installer (it bundles the compiler)
echo   2. Set UNILOGIC_HOME to the directory containing Main.py
echo   3. Configure the compiler path in UniLogic Project Manager
exit /b 1

:: ── Find Python ─────────────────────────────────────────────────────────

:findpy
where python >nul 2>nul && (
    set "PY=python"
    goto :run
)

where py >nul 2>nul && (
    set "PY=py -3"
    goto :run
)

for %%V in (313 312 311 310) do (
    if exist "%LOCALAPPDATA%\Programs\Python\Python%%V\python.exe" (
        set "PY=%LOCALAPPDATA%\Programs\Python\Python%%V\python.exe"
        goto :run
    )
)

echo Error: Python not found.
echo Install Python 3.10+ from https://python.org/downloads/
echo Make sure to check "Add Python to PATH" during installation.
exit /b 1

:run
%PY% "%MAIN_PY%" %*
exit /b %errorlevel%
