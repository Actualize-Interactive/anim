# Configuration file for the Sphinx documentation builder.
import os
import sys
sys.path.insert(0, os.path.abspath('.'))

project = 'anim'
html_theme = 'sphinx_rtd_theme'
extensions = [
    'breathe',
    'exhale',
]

breathe_projects = {
    "anim": "../doxygen/xml"
}
breathe_default_project = "anim"

exhale_args = {
    "containmentFolder":     "api",
    "rootFileName":          "library_root.rst",
    "rootFileTitle":         "API Reference",
    "doxygenStripFromPath":  "..",
    "createTreeView":        True,
}

html_static_path = ['_static']
