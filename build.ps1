param(
    [ValidateSet("Release", "Debug", "RelWithDebInfo")]
    [string]$Config = "Release"
)

# Configure, build, and test the anim library (library + test suite).
# To build and run the examples, use examples\run_example.ps1 instead.
$ErrorActionPreference = "Stop"

# Run from the repository root regardless of where the script was invoked from.
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

Push-Location $RepoRoot
try {
    # CMAKE_BUILD_TYPE covers single-config generators (Ninja/Make);
    # --config covers multi-config generators (Visual Studio).
    cmake -B build -S . -DCMAKE_BUILD_TYPE="$Config" -DANIM_BUILD_TESTS=ON -DANIM_BUILD_EXAMPLES=OFF
    cmake --build build --config $Config

    # -C is required for multi-config generators; without it ctest finds no tests.
    ctest --test-dir build -C $Config --output-on-failure
} finally {
    Pop-Location
}
