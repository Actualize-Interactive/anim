# Build the anim documentation: Doxygen (XML) -> Sphinx (HTML).
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# The version lives only in CMakeLists.txt; Doxyfile reads it as $(ANIM_VERSION)
# and conf.py parses the same file, so there is one source of truth.
$cmakeLists = Join-Path (Split-Path -Parent $ScriptDir) "CMakeLists.txt"
$versionMatch = [regex]::Match((Get-Content -Raw $cmakeLists), 'project\s*\(\s*anim\s+VERSION\s+([0-9]+(?:\.[0-9]+)*)')
if (-not $versionMatch.Success) {
    Write-Error "Could not parse the project version from $cmakeLists"
    exit 1
}
$env:ANIM_VERSION = $versionMatch.Groups[1].Value
Write-Host "Building docs for anim $($env:ANIM_VERSION)"

Write-Host "Running Doxygen..."
doxygen Doxyfile

if (-not (Test-Path "doxygen/xml")) {
    Write-Error "Doxygen did not produce XML output."
    exit 1
}

Write-Host "Building Sphinx site..."
sphinx-build -b html sphinx build/html

Write-Host "Documentation built. Open: $ScriptDir\build\html\index.html"
