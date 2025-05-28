# Store the current location to restore it later
$originalLocation = Get-Location

try {
    
    # Create build directory
    $null = New-Item -Path .\build -ItemType Directory -Force
    Set-Location -Path .\build

    # Configure using CMake
    # Explicitly enable tests and examples for CI/standalone builds
    cmake .. -DANIM_BUILD_TESTS=ON -DANIM_BUILD_EXAMPLES=ON

    # Build
    cmake --build . # --verbose 

    # Run tests
    ctest # --verbose

    # Run example
    # Note: The example executable is expected to be in the build directory
    # .\examples\Debug\curve_visualization.exe

    Pop-Location
} finally {
    # Ensure we always return to the original directory, even if errors occur
    Set-Location -Path $originalLocation
}