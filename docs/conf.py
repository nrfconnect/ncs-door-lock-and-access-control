# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

from pathlib import Path

# -- Project information -----------------------------------------------------

project = 'nRF Door Lock and Access Control Add-on'
copyright = '2026, Nordic Semiconductor'
author = 'Nordic Semiconductor'
release = 'Latest'
version = 'Latest'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx_tabs.tabs',
    'sphinx_copybutton',
    'sphinx_togglebutton',
    'sphinx.ext.autosummary'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['.venv']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_ncs_theme'

html_theme_options = {
    'docsets': {},
    "ncs_url": "https://nrfconnectdocs.nordicsemi.com/ncs/latest/nrf/",
    "ncs_label": "nRF Connect SDK Docs",
    "addons_url": "https://nrfconnect.github.io/ncs-app-index/",
    "bare_metal_url": "",
    "logo_url": "https://docs.nordicsemi.com",
}

html_show_sphinx = False
html_extra_path = ['versions.json']

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static'] (currently not in use)

# Read all your shortcut files and concatenate them
rst_epilog = ""

shortcuts_files = [
    "links.txt",
    "shortcuts.txt",
]

for f in shortcuts_files:
    filepath = Path(__file__).parent / f
    if filepath.exists():
        rst_epilog += filepath.read_text() + "\n"

# -- Options for sphinx_ncs_theme -------------------------------------------
html_theme_options = {
    "docset": "aliro",
    "docsets": {},
}
