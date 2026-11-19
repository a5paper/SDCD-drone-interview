# Configures and builds the project. CMake generates the protobuf sources.
# Usage: .\compile.ps1 [Debug|Release] [-Clean]
#   If scripts are blocked (default Windows policy), run:
#   powershell -ExecutionPolicy Bypass -File .\compile.ps1
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',
    [switch]$Clean
)
$ErrorActionPreference = 'Stop'
$RootDir = $PSScriptRoot
$BuildDir = Join-Path $RootDir 'build-windows'

Import-Module (Join-Path $RootDir 'scripts\VsDevShell.psm1') -Force
Enter-SdcdVsDevShell | Out-Null
Set-Location $RootDir

if ($Clean) {
    Write-Host 'Cleaning build directory...'
    if (Test-Path $BuildDir) { Remove-Item -Recurse -Force $BuildDir }
}

Write-Host "Configuring ($Config)..."
cmake --preset windows
if ($LASTEXITCODE -ne 0) { Write-Error "cmake configure failed (exit code $LASTEXITCODE)."; exit 1 }

Write-Host 'Building...'
cmake --build --preset "windows-$($Config.ToLower())"
if ($LASTEXITCODE -ne 0) { Write-Error "cmake build failed (exit code $LASTEXITCODE)."; exit 1 }

Write-Host "Build complete: $BuildDir\DroneSimulator\$Config\DroneSimulator.exe"
