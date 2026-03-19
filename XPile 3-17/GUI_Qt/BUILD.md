# Building UniLogic Project Manager

Native Qt6 C++ application. Builds on Windows with MinGW or MSVC.

## Prerequisites

| Tool | Version | Install |
|------|---------|---------|
| CMake | 3.20+ | `choco install cmake` or [cmake.org](https://cmake.org/download/) |
| Qt6 | 6.8+ | See below |
| C++ Compiler | MinGW 13+ or MSVC 2022 | Bundled with Qt or Visual Studio |

### Installing Qt6 via aqt (recommended)

```bash
pip install aqtinstall

# Qt6 libraries + matching MinGW compiler
aqt install-qt   windows desktop 6.8.3 win64_mingw --outputdir C:/Qt
aqt install-tool windows desktop tools_mingw1310    --outputdir C:/Qt
```

This puts:
- Qt6 libraries at `C:/Qt/6.8.3/mingw_64/`
- MinGW 13.1 compiler at `C:/Qt/Tools/mingw1310_64/`

---

## Quick Build (Dynamic Linking)

~350 KB exe + ~22 MB Qt DLLs. Best for development.

**Important:** The build directory path must have no spaces (MinGW `windres`
limitation). Use an out-of-tree build:

```bash
# Create a space-free build directory
mkdir C:\Dev\GUI_Qt_build
cd C:\Dev\GUI_Qt_build

# Configure — point at source and Qt prefix
cmake "C:/path/to/GUI_Qt" -G "MinGW Makefiles" ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/mingw_64" ^
  -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" ^
  -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe" ^
  -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

### Deploy DLLs

```bash
mkdir deploy
copy UniLogicProjectManager.exe deploy\

C:\Qt\6.8.3\mingw_64\bin\windeployqt6.exe deploy\UniLogicProjectManager.exe ^
  --no-translations --no-opengl-sw --no-system-d3d-compiler

# Remove unused plugins (no networking/SVG in this app)
del deploy\Qt6Network.dll deploy\Qt6Svg.dll
rmdir /s /q deploy\networkinformation deploy\tls deploy\generic deploy\iconengines
del deploy\imageformats\qgif.dll deploy\imageformats\qjpeg.dll deploy\imageformats\qsvg.dll
```

**Expected output:** ~22 MB `deploy/` folder. Zip for distribution or feed to the
Inno Setup installer script.

---

## Static Build (Single .exe, No DLLs)

Produces a self-contained exe with no runtime dependencies. This is what gets
distributed via the installer.

### Step 1 — Install vcpkg

```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

### Step 2 — Install Qt6 static libraries

Compiles Qt from source with static linking. Takes 30-60 minutes on first run;
cached after that.

```powershell
# MSVC static (recommended — smallest binary)
.\vcpkg install qtbase:x64-windows-static

# Or MinGW static
.\vcpkg install qtbase:x64-mingw-static
```

### Step 3 — Build

```powershell
mkdir C:\Dev\GUI_Qt_static
cd C:\Dev\GUI_Qt_static

# MSVC — run from "x64 Native Tools Command Prompt"
cmake "C:/path/to/GUI_Qt" -G Ninja ^
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
  -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
  -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release
```

The `CMakeLists.txt` automatically handles:
- Static CRT (`/MT`) for MSVC
- Static libgcc/libstdc++ for GCC
- Release symbol stripping

### Step 4 — Verify

```powershell
# Should run standalone
.\UniLogicProjectManager.exe

# Check size — expect 6-9 MB
dir UniLogicProjectManager.exe

# Verify only Windows system DLL dependencies
dumpbin /dependents UniLogicProjectManager.exe
# Should show only: KERNEL32.dll, USER32.dll, GDI32.dll, SHELL32.dll, etc.
```

### Expected sizes

| Build type | Exe size | Total distribution |
|-----------|----------|-------------------|
| Dynamic (MinGW) | ~350 KB | ~22 MB (exe + DLLs) |
| Static (MSVC) | ~6-9 MB | ~6-9 MB (single file) |
| Static (MinGW) | ~8-12 MB | ~8-12 MB (single file) |

---

## Building the Installer

After building (static or dynamic), create a Windows installer with Inno Setup:

1. Install [Inno Setup 6+](https://jrsoftware.org/isinfo.php)
2. If using dynamic build, run `windeployqt` to populate `deploy/`
3. Open `installer/unilogic_setup.iss` in Inno Setup Compiler
4. Click Build — produces `installer/output/UniLogic_Setup_0.1.0.exe`

The installer:
- Installs to `Program Files\UniLogic\`
- Creates Start Menu and optional desktop shortcuts
- Registers in Add/Remove Programs
- Optionally installs the VS Code extension (if `.vsix` present)
- Optionally adds the compiler directory to PATH
- Offers to launch Project Manager on finish

---

## Troubleshooting

### `windres` fails with "not a directory"
The **build directory** path contains spaces. Use an out-of-tree build in a
space-free path: `cmake "path/to/GUI_Qt" -B C:\Dev\build`

### `__imp___argc` undefined reference
CRT mismatch: Qt was built with MSVCRT but your compiler uses UCRT.
Use the Qt-bundled MinGW: `C:/Qt/Tools/mingw1310_64/bin/g++.exe`

### Qt6 not found by CMake
Set `-DCMAKE_PREFIX_PATH` to your Qt6 installation, e.g.:
`-DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/mingw_64"`

### App icon not showing
Ensure `unilogic.ico` is in the `GUI_Qt/` directory alongside `app.rc`.
The icon is embedded in the exe at build time.

### Compiler path not set
The Build tab reads the compiler path from Settings (gear icon in header bar).
Stored in the Windows registry at `HKCU\Software\UniLogic\ProjectManager`.
On first launch the Settings dialog opens automatically.
