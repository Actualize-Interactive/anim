# Configuration file for the Sphinx documentation builder.
#
# The API reference is generated from the Doxygen XML (produced by running
# doxygen on docs/Doxyfile) via Breathe + Exhale. See docs/build_docs.sh.

import pathlib
import re

project = 'anim'
copyright = '2025-2026, Actualize Interactive Inc.'
author = 'Actualize Interactive Inc.'


def _version_from_cmake():
    """Read the single source of truth for the version: project() in CMakeLists.txt."""
    cmakelists = pathlib.Path(__file__).resolve().parents[2] / 'CMakeLists.txt'
    match = re.search(r'project\s*\(\s*anim\s+VERSION\s+([0-9]+(?:\.[0-9]+)*)',
                      cmakelists.read_text(encoding='utf-8'))
    if not match:
        raise RuntimeError(f'Could not parse the project version from {cmakelists}')
    return match.group(1)


release = _version_from_cmake()
version = release

extensions = [
    'breathe',
    'exhale',
]

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

# -- Breathe -----------------------------------------------------------------
breathe_projects = {'anim': '../doxygen/xml'}
breathe_default_project = 'anim'

# -- Exhale ------------------------------------------------------------------
# Builds the full API tree under api/ from the Doxygen XML. Doxygen is run
# separately by the build script, so Exhale does not invoke it itself.
exhale_args = {
    'containmentFolder': './api',
    'rootFileName': 'library_root.rst',
    'rootFileTitle': 'API Reference',
    'doxygenStripFromPath': '../../include',
    'createTreeView': True,
    'exhaleExecutesDoxygen': False,
}

primary_domain = 'cpp'
highlight_language = 'cpp'
