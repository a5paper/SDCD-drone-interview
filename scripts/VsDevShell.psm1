# Shared helpers for the Windows scripts: find Visual Studio and enter its
# developer shell (puts cl, cmake, ninja and vcpkg on PATH, sets VCPKG_ROOT).

$script:VsVersionRange = '[17.0,19.0)'  # VS 2022 (17.x) and VS 2026 (18.x)

# Component IDs that exist in both Build Tools and the IDE (Community,
# Professional, Enterprise). Workload IDs differ: the C++ workload is
# Workload.VCTools in Build Tools and Workload.NativeDesktop in the IDE.
$script:RequiredComponents = @(
    'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
    'Microsoft.VisualStudio.Component.VC.CMake.Project',
    'Microsoft.VisualStudio.Component.Vcpkg'
)
$script:CppWorkloadPattern = '^Microsoft\.VisualStudio\.Workload\.(VCTools|NativeDesktop)$'
$script:BuildToolsProductId = 'Microsoft.VisualStudio.Product.BuildTools'
$script:BuildToolsCppWorkload = 'Microsoft.VisualStudio.Workload.VCTools'
$script:IdeCppWorkload = 'Microsoft.VisualStudio.Workload.NativeDesktop'
# The Windows SDK is satisfied by any 10 or 11 SDK.
$script:SdkPattern = '^Microsoft\.VisualStudio\.Component\.Windows(10|11)SDK'
$script:SdkDefaultComponent = 'Microsoft.VisualStudio.Component.Windows11SDK.26100'

function Get-VsWherePath {
    $path = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $path) { return $path }
    return $null
}

# Returns every VS instance in the version range, newest first, each with
# its installed package IDs. Returns an empty array when there is none.
function Get-VsInstances {
    $vswhere = Get-VsWherePath
    if (-not $vswhere) { return @() }
    $json = & $vswhere -products * -version $script:VsVersionRange -format json -include packages
    if ($LASTEXITCODE -ne 0) { throw "vswhere failed with exit code $LASTEXITCODE." }
    $text = ($json -join "`n").Trim()
    if (-not $text) { return @() }
    # Windows PowerShell 5.1's ConvertFrom-Json emits a JSON array as one
    # object, so "[]" would become one empty instance. Piping the variable
    # enumerates it: "[]" gives no instances.
    $parsed = $text | ConvertFrom-Json
    return @($parsed | Sort-Object { [version]$_.installationVersion } -Descending)
}

# The IDs to pass to the VS Installer's "modify --add" for this instance;
# empty when it has everything. Reads the instance's package list: vswhere
# -requires takes exact IDs only, so it cannot express "any Windows SDK".
# A missing C++ workload is only added when components are missing too,
# so an instance with the components but no workload counts as complete.
function Get-VsMissingComponents($instance) {
    $ids = @($instance.packages | ForEach-Object { $_.id })
    $missing = @($script:RequiredComponents | Where-Object { $ids -notcontains $_ })
    if (-not ($ids | Where-Object { $_ -match $script:SdkPattern })) {
        $missing += $script:SdkDefaultComponent
    }
    if ($missing.Count -gt 0 -and -not ($ids | Where-Object { $_ -match $script:CppWorkloadPattern })) {
        $missing = @(Get-VsCppWorkload $instance) + $missing
    }
    return $missing
}

# The C++ workload ID that is valid for this instance's product.
function Get-VsCppWorkload($instance) {
    if ($instance.productId -eq $script:BuildToolsProductId) { return $script:BuildToolsCppWorkload }
    return $script:IdeCppWorkload
}

# Every ID a fresh Build Tools install needs (used for winget).
function Get-VsBuildToolsComponents {
    return @($script:BuildToolsCppWorkload) + $script:RequiredComponents + $script:SdkDefaultComponent
}

# The newest instance that has every required component, or $null.
function Find-SdcdVsInstance {
    return Get-VsInstances | Where-Object { (Get-VsMissingComponents $_).Count -eq 0 } | Select-Object -First 1
}

# Enters the developer shell of the instance Find-SdcdVsInstance accepts,
# the same one install-deps.ps1 checked.
function Enter-SdcdVsDevShell {
    $vswhere = Get-VsWherePath
    if (-not $vswhere) {
        throw 'Visual Studio not found (vswhere.exe is missing). Run .\install-deps.ps1 first.'
    }
    $instance = Find-SdcdVsInstance
    if (-not $instance) {
        throw 'Visual Studio 2022 or 2026 with the C++ tools, CMake tools, vcpkg and a Windows SDK not found. Run .\install-deps.ps1 first.'
    }
    $installPath = $instance.installationPath
    # The dev shell scripts call vswhere by name; without it on PATH they print
    # "'vswhere.exe' is not recognized".
    $env:Path = (Split-Path $vswhere) + ';' + $env:Path
    Import-Module (Join-Path $installPath 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
    Enter-VsDevShell -VsInstallPath $installPath -SkipAutomaticLocation `
        -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
    return $installPath
}

Export-ModuleMember -Function Get-VsWherePath, Get-VsInstances, Get-VsMissingComponents, `
    Get-VsBuildToolsComponents, Find-SdcdVsInstance, Enter-SdcdVsDevShell
