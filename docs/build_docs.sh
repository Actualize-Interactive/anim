#!/bin/bash
# Build the anim documentation: Doxygen (XML) -> Sphinx (HTML).
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# The version lives only in CMakeLists.txt; Doxyfile reads it as $(ANIM_VERSION)
# and conf.py parses the same file, so there is one source of truth.
ANIM_VERSION="$(grep -oE 'project[[:space:]]*\([[:space:]]*anim[[:space:]]+VERSION[[:space:]]+[0-9.]+' ../CMakeLists.txt | grep -oE '[0-9.]+$')"
if [ -z "$ANIM_VERSION" ]; then
    echo "Error: could not parse the project version from ../CMakeLists.txt" >&2
    exit 1
fi
export ANIM_VERSION
echo "Building docs for anim $ANIM_VERSION"

echo "Running Doxygen..."
doxygen Doxyfile

if [ ! -d "doxygen/xml" ]; then
    echo "Error: Doxygen did not produce XML output." >&2
    exit 1
fi

echo "Building Sphinx site..."
sphinx-build -b html sphinx build/html

echo "Documentation built. Open: $SCRIPT_DIR/build/html/index.html"
