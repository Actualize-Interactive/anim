# Configuration file for the Sphinx documentation builder.
#
# The API reference is generated from the Doxygen XML (produced by running
# doxygen on docs/Doxyfile) via Breathe + Exhale. See docs/build_docs.sh.

project = 'anim'
copyright = '2025, Actualize Interactive Inc.'
author = 'Actualize Interactive Inc.'
release = '0.1.2'

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
