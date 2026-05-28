# Build the anim documentation: Doxygen (XML) -> Sphinx (HTML).
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

Write-Host "Running Doxygen..."
doxygen Doxyfile

if (-not (Test-Path "doxygen/xml")) {
    Write-Error "Doxygen did not produce XML output."
    exit 1
}

Write-Host "Building Sphinx site..."
sphinx-build -b html sphinx build/html

Write-Host "Documentation built. Open: $ScriptDir\build\html\index.html"
