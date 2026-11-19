# Checks the Windows build prerequisites (Visual Studio C++ tools, Python 3.12),
# offers to install what is missing with winget or the VS Installer, builds
# the C++ libraries with vcpkg and creates the GCS venv.
# Usage: .\install-deps.ps1 [-WhatIf]
#   If scripts are blocked (default Windows policy), run:
#   powershell -ExecutionPolicy Bypass -File .\install-deps.ps1 [-WhatIf]
#   -WhatIf  run the checks, print the install commands that would run if
#            everything were missing, and run nothing.
param(
    [switch]$WhatIf
)
$ErrorActionPreference = 'Stop'
$RootDir = $PSScriptRoot
$VenvDir = Join-Path $RootDir 'GCS\.venv-windows'
$VcpkgInstallRoot = Join-Path $RootDir 'vcpkg_installed'
$Triplet = 'x64-windows-static-md'
$MaxRepoPathLength = 40
$RebootRequiredExitCode = 3010
$VsSetup = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\setup.exe'

Import-Module (Join-Path $RootDir 'scripts\VsDevShell.psm1') -Force

function Get-VsInstallArguments($installPath, $components) {
    $adds = ($components | ForEach-Object { "--add $_" }) -join ' '
    return "modify --installPath `"$installPath`" $adds --includeRecommended --quiet --wait"
}

# setup.exe needs admin rights and is a GUI program, so it is started elevated
# with Start-Process -Wait rather than called with &.
function Get-VsInstallCommand($installPath, $components) {
    $arguments = (Get-VsInstallArguments $installPath $components).Replace('"', '`"')
    return "Start-Process -FilePath `"$VsSetup`" -ArgumentList `"$arguments`" -Verb RunAs -Wait -PassThru"
}

function Get-VsWingetCommand {
    $adds = (Get-VsBuildToolsComponents | ForEach-Object { "--add $_" }) -join ' '
    return "winget install --id Microsoft.VisualStudio.2022.BuildTools --override `"--quiet --wait $adds --includeRecommended`" --accept-source-agreements --accept-package-agreements"
}

$PythonCommand = 'winget install --id Python.Python.3.12 --accept-source-agreements --accept-package-agreements'
$VcpkgCommand = "vcpkg install --triplet $Triplet --x-install-root=$VcpkgInstallRoot"

function Test-Python {
    if (-not (Get-Command py -ErrorAction SilentlyContinue)) { return $false }
    py -3.12 --version | Out-Null
    return ($LASTEXITCODE -eq 0)
}

# Runs the checks and prints their result; returns the pending install steps
# as objects with Description and Command.
function Get-MissingSteps {
    $steps = @()
    $instances = @(Get-VsInstances)
    $complete = Find-SdcdVsInstance
    if ($complete) {
        Write-Host "OK  Visual Studio $($complete.catalog.productLineVersion) ($($complete.installationVersion)) at $($complete.installationPath)"
        Write-Host '    C++ x64 tools, Windows SDK, CMake tools and vcpkg are installed.'
    } elseif ($instances.Count -gt 0) {
        $inst = $instances[0]
        $missing = Get-VsMissingComponents $inst
        Write-Host "MISSING  Visual Studio $($inst.catalog.productLineVersion) at $($inst.installationPath) lacks:"
        $missing | ForEach-Object { Write-Host "    $_" }
        $steps += [pscustomobject]@{
            Description = 'Add the missing Visual Studio components'
            Command     = Get-VsInstallCommand $inst.installationPath $missing
            SetupArgs   = Get-VsInstallArguments $inst.installationPath $missing
        }
    } else {
        Write-Host 'MISSING  Visual Studio 2022 or 2026 (Build Tools or the IDE)'
        $steps += [pscustomobject]@{
            Description = 'Install Visual Studio 2022 Build Tools with the C++ components'
            Command     = Get-VsWingetCommand
        }
    }
    if (Test-Python) {
        Write-Host "OK  $(py -3.12 --version) (py -3.12)"
    } else {
        Write-Host 'MISSING  Python 3.12 (py -3.12)'
        $steps += [pscustomobject]@{
            Description = 'Install Python 3.12'
            Command     = $PythonCommand
        }
    }
    return $steps
}

function Show-Steps($steps) {
    foreach ($step in $steps) {
        Write-Host "  $($step.Description):"
        Write-Host "    $($step.Command)"
    }
}

if ($RootDir.Length -gt $MaxRepoPathLength) {
    Write-Warning ("This folder's path is $($RootDir.Length) characters long. Long paths can break the C++ library build (MSVC and vcpkg path limits). Move the repo to a short path such as C:\src\SDCD.")
}

if ($WhatIf) {
    Write-Host '-WhatIf: running the checks only.'
    $null = Get-MissingSteps
    Write-Host ''
    Write-Host 'Commands that would run if every check had failed (nothing is run):'
    $all = Get-VsBuildToolsComponents
    Show-Steps @(
        [pscustomobject]@{ Description = 'No Visual Studio instance'; Command = Get-VsWingetCommand },
        [pscustomobject]@{ Description = 'Incomplete Visual Studio Build Tools instance (an IDE instance gets Workload.NativeDesktop instead of Workload.VCTools)'; Command = Get-VsInstallCommand '<Visual Studio install path>' $all },
        [pscustomobject]@{ Description = 'No Python 3.12'; Command = $PythonCommand },
        [pscustomobject]@{ Description = 'Always, in the VS developer shell (about 7 minutes the first time on the development machine; slower machines take longer)'; Command = $VcpkgCommand },
        [pscustomobject]@{ Description = 'Always, GCS venv'; Command = "py -3.12 -m venv $VenvDir; $VenvDir\Scripts\pip.exe install -r $RootDir\GCS\requirements-dev.txt" }
    )
    exit 0
}

$steps = @(Get-MissingSteps)
if ($steps.Count -gt 0) {
    Write-Host ''
    Write-Host 'Missing pieces and the commands that install them (they need admin rights, expect a UAC prompt):'
    Show-Steps $steps
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Write-Host ''
        Write-Host 'winget was not found. Run the commands above yourself (or install the pieces by hand), then re-run this script.'
        exit 1
    }
    $answer = Read-Host 'Install the missing pieces with winget? [y/N]'
    if ($answer -notmatch '^[yY]') {
        Write-Host 'Nothing installed. Run the commands above yourself, then re-run this script.'
        exit 1
    }
    foreach ($step in $steps) {
        Write-Host "Running: $($step.Command)"
        if ($step.SetupArgs) {
            $proc = Start-Process -FilePath $VsSetup -ArgumentList $step.SetupArgs -Verb RunAs -Wait -PassThru
            $code = $proc.ExitCode
            if ($code -eq $RebootRequiredExitCode) {
                Write-Warning 'The Visual Studio Installer asks for a reboot. Reboot, then re-run this script if a later step fails.'
                $code = 0
            }
        } else {
            Invoke-Expression $step.Command
            $code = $LASTEXITCODE
        }
        if ($code -ne 0) {
            Write-Error "'$($step.Description)' failed (exit code $code)."
            exit 1
        }
    }
    # The installers change PATH for new processes only; reload it here.
    $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User')
    $steps = @(Get-MissingSteps)
    if ($steps.Count -gt 0) {
        Write-Error 'Some pieces are still missing after the install. Open a new terminal and re-run this script.'
        exit 1
    }
}

Write-Host ''
Write-Host 'Building C++ libraries with vcpkg (about 7 minutes the first time on the development machine, slower machines take longer; seconds when cached)...'
Enter-SdcdVsDevShell | Out-Null
Set-Location $RootDir
if (-not $env:VCPKG_ROOT) {
    Write-Error 'VCPKG_ROOT is not set by the VS developer shell. Add the Vcpkg component to Visual Studio.'
    exit 1
}
& (Join-Path $env:VCPKG_ROOT 'vcpkg.exe') install --triplet $Triplet "--x-install-root=$VcpkgInstallRoot"
if ($LASTEXITCODE -ne 0) { Write-Error "vcpkg install failed (exit code $LASTEXITCODE)."; exit 1 }

# The GCS's Python packages go into their own venv (re-running updates it).
Write-Host 'Creating the GCS venv...'
py -3.12 -m venv $VenvDir
if ($LASTEXITCODE -ne 0) { Write-Error "venv creation failed (exit code $LASTEXITCODE)."; exit 1 }
& (Join-Path $VenvDir 'Scripts\pip.exe') install --quiet -r (Join-Path $RootDir 'GCS\requirements-dev.txt')
if ($LASTEXITCODE -ne 0) { Write-Error "pip install failed (exit code $LASTEXITCODE)."; exit 1 }

Write-Host 'Dependencies installed.'
Write-Host "  GCS venv: $VenvDir ($(& (Join-Path $VenvDir 'Scripts\python.exe') --version))"
Write-Host "  cmake:  $((cmake --version | Select-Object -First 1))"
Write-Host "  vcpkg libraries: $VcpkgInstallRoot"
