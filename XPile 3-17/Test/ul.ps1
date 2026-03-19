$python = $null
$cmd = Get-Command python3 -ErrorAction SilentlyContinue
if ($cmd) { $python = $cmd.Source }
if (-not $python) {
    $cmd = Get-Command python -ErrorAction SilentlyContinue
    if ($cmd) { $python = $cmd.Source }
}
if (-not $python) {
    $candidates = @(
        "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe",
        "$env:LOCALAPPDATA\Programs\Python\Python311\python.exe",
        "$env:LOCALAPPDATA\Programs\Python\Python310\python.exe"
    )
    $python = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}
if (-not $python) {
    Write-Error "Python not found. Install Python 3.10+ and try again."
    exit 1
}
& $python "$PSScriptRoot\Main.py" @args
