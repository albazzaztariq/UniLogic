# make_release.ps1
# Run from repo root: powershell scripts/make_release.ps1

$ErrorActionPreference = "Stop"
$version = "0.1.0"

Write-Host "Building UniLogic v$version release..." -ForegroundColor Cyan

# Create release directory
New-Item -ItemType Directory -Force "release" | Out-Null

# Step 1: Run regression tests
Write-Host "Running regression tests..."
& python Main.py test
if ($LASTEXITCODE -ne 0) {
    Write-Error "Regression tests failed. Fix before releasing."
    exit 1
}
Write-Host "Tests passed." -ForegroundColor Green

# Step 2: Build VS Code extension
Write-Host "Building VS Code extension..."
Push-Location "LSP/vscode-ul"
& npm install
& npx vsce package --out "../../release/unilogic-$version.vsix"
Pop-Location
Write-Host "Extension built." -ForegroundColor Green

# Step 3: Build Qt Project Manager
Write-Host "Building Project Manager..."
$buildDir = "C:\Dev\GUI_Qt_build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Force $buildDir | Out-Null
    & cmake -S "GUI_Qt" -B $buildDir -G "MinGW Makefiles" `
        -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/mingw_64" `
        -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" `
        -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe" `
        -DCMAKE_BUILD_TYPE=Release
}
& cmake --build $buildDir --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Error "Qt build failed."
    exit 1
}
Write-Host "Project Manager built." -ForegroundColor Green

# Step 4: Deploy Qt
Write-Host "Deploying Qt dependencies..."
New-Item -ItemType Directory -Force "release/ProjectManager" | Out-Null
Copy-Item "$buildDir/UniLogicProjectManager.exe" "release/ProjectManager/"
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;$env:PATH"
& "C:\Qt\6.8.3\mingw_64\bin\windeployqt6.exe" "release/ProjectManager/UniLogicProjectManager.exe" `
    --no-translations --no-opengl-sw --no-system-d3d-compiler

# Trim unused plugins
Remove-Item "release/ProjectManager/Qt6Svg.dll" -ErrorAction SilentlyContinue
Remove-Item "release/ProjectManager/tls" -Recurse -ErrorAction SilentlyContinue
Remove-Item "release/ProjectManager/generic" -Recurse -ErrorAction SilentlyContinue
Remove-Item "release/ProjectManager/iconengines" -Recurse -ErrorAction SilentlyContinue

Compress-Archive -Path "release/ProjectManager/*" `
    -DestinationPath "release/UniLogic-ProjectManager-$version.zip" -Force
Write-Host "Project Manager packaged." -ForegroundColor Green

# Step 5: Build installer
Write-Host "Building installer..."

# Ensure vsix is available for installer bundling
# The .iss file references ../../LSP/vscode-ul/unilogic-0.1.0.vsix relative to installer/
$vsixSrc = "release/unilogic-$version.vsix"
$vsixDst = "LSP/vscode-ul/unilogic-$version.vsix"
if ((Test-Path $vsixSrc) -and -not (Test-Path $vsixDst)) {
    Copy-Item $vsixSrc $vsixDst
    Write-Host "Copied vsix to LSP/vscode-ul/ for installer bundling." -ForegroundColor Yellow
}

$iscc = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe"
if (Test-Path $iscc) {
    & $iscc "GUI_Qt/installer/unilogic_setup.iss"
    Copy-Item "GUI_Qt/installer/output/UniLogic_Setup_$version.exe" "release/"
    Write-Host "Installer built." -ForegroundColor Green
} else {
    Write-Warning "Inno Setup not found. Skipping installer build."
}

# Step 6: Copy ul wrapper
Copy-Item "ul.bat" "release/" -ErrorAction SilentlyContinue
Copy-Item "ul.sh" "release/" -ErrorAction SilentlyContinue

# Done
Write-Host ""
Write-Host "Release artifacts ready in release/:" -ForegroundColor Cyan
Get-ChildItem "release/" -File | Format-Table Name, @{L="Size (KB)";E={[math]::Round($_.Length/1KB)}}
