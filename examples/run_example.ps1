param(
    [ValidateSet("Release", "Debug", "RelWithDebInfo")]
    [string]$Config = "Release"
)

# Build the project and run the curve_visualization example.
$ErrorActionPreference = "Stop"

# Repo root is the parent of this script's directory (examples/).
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir

Push-Location $RepoRoot
try {
    cmake -B build -S . -DANIM_BUILD_TESTS=ON -DANIM_BUILD_EXAMPLES=ON
    cmake --build build --config $Config
    & ".\build\examples\$Config\curve_visualization.exe"
} finally {
    Pop-Location
}
