# Build script for Windows PowerShell

# Create build directory
$null = New-Item -Path .\build -ItemType Directory -Force
Set-Location -Path .\build

# Configure using CMake
cmake ..

# Build
cmake --build . # --verbose 

# Run tests
ctest # --verbose
