#!/bin/bash
# Build the anim documentation: Doxygen (XML) -> Sphinx (HTML).
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

echo "Running Doxygen..."
doxygen Doxyfile

if [ ! -d "doxygen/xml" ]; then
    echo "Error: Doxygen did not produce XML output." >&2
    exit 1
fi

echo "Building Sphinx site..."
sphinx-build -b html sphinx build/html

echo "Documentation built. Open: $SCRIPT_DIR/build/html/index.html"
