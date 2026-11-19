# Runs the whole system: the simulator in the background, the GCS in front.
# Closing the GCS (or Ctrl+C) stops the simulator. Builds first if needed.
# Usage: .\run.ps1 [Debug|Release] [-ConfigPath PATH]   (default Release)
#   If scripts are blocked (default Windows policy), run:
#   powershell -ExecutionPolicy Bypass -File .\run.ps1 [Debug|Release]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Release',
    [string]$ConfigPath = ''
)
$ErrorActionPreference = 'Stop'
$RootDir = $PSScriptRoot
$BuildDir = Join-Path $RootDir 'build-windows'
$Simulator = Join-Path $BuildDir "DroneSimulator\$Config\DroneSimulator.exe"
$GcsPython = Join-Path $RootDir 'GCS\.venv-windows\Scripts\python.exe'
$SimulatorLog = Join-Path $BuildDir 'simulator.log'
$SimulatorErrorLog = Join-Path $BuildDir 'simulator-error.log'
$StartupCheckMs = 1000

if (-not (Test-Path $GcsPython)) {
    Write-Error 'GCS venv not found. Run .\install-deps.ps1 first.'
}

$configArgs = @()
if ($ConfigPath -ne '') {
    $configArgs = @('--config', (Resolve-Path $ConfigPath).Path)
}

# Multi-config build: each configuration has its own exe, so build when it
# is missing.
if (-not (Test-Path $Simulator)) {
    & powershell -NoProfile -ExecutionPolicy Bypass `
        -File (Join-Path $RootDir 'compile.ps1') $Config
    if ($LASTEXITCODE -ne 0) {
        Write-Error "compile.ps1 failed (exit code $LASTEXITCODE)."
    }
}

$startArgs = @{
    FilePath               = $Simulator
    RedirectStandardOutput = $SimulatorLog
    RedirectStandardError  = $SimulatorErrorLog
    WindowStyle            = 'Hidden'
    PassThru               = $true
}
if ($configArgs.Count -gt 0) { $startArgs.ArgumentList = $configArgs }
$simulatorProcess = Start-Process @startArgs
Write-Host "Simulator running (PID $($simulatorProcess.Id), log: $SimulatorLog)"

try {
    # A simulator that cannot open its sockets exits at once: report it.
    Start-Sleep -Milliseconds $StartupCheckMs
    if ($simulatorProcess.HasExited) {
        Write-Host "Error: the simulator stopped. Last lines of its logs:"
        Get-Content $SimulatorLog, $SimulatorErrorLog -Tail 5
        exit 1
    }
    & $GcsPython (Join-Path $RootDir 'GCS\main.py') @configArgs
} finally {
    if (-not $simulatorProcess.HasExited) {
        Stop-Process -Id $simulatorProcess.Id -Force
    }
}
