# Runs the simulator's GoogleTest suite and the GCS's pytest suite.
# Usage: .\test.ps1 [Debug|Release]   (run .\compile.ps1 with the same config first)
#   If scripts are blocked (default Windows policy), run:
#   powershell -ExecutionPolicy Bypass -File .\test.ps1 [Debug|Release]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug'
)
$ErrorActionPreference = 'Stop'
$RootDir = $PSScriptRoot

Import-Module (Join-Path $RootDir 'scripts\VsDevShell.psm1') -Force
Enter-SdcdVsDevShell | Out-Null
Set-Location $RootDir

$failed = $false

Write-Host 'C++ tests...'
ctest --preset "windows-$($Config.ToLower())"
if ($LASTEXITCODE -ne 0) { $failed = $true }

Write-Host 'GCS tests...'
$pytest = Join-Path $RootDir 'GCS\.venv-windows\Scripts\pytest.exe'
if (-not (Test-Path $pytest)) {
    Write-Error 'GCS venv not found. Run .\install-deps.ps1 first.'
    exit 1
}
& $pytest (Join-Path $RootDir 'GCS\tests')
if ($LASTEXITCODE -ne 0) { $failed = $true }

if ($failed) { Write-Error 'Some tests failed.'; exit 1 }
