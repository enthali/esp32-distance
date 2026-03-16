# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

# Read version from VERSION file in project root
version_file = os.path.join(os.path.dirname(__file__), '..', 'VERSION')
if os.path.exists(version_file):
    with open(version_file, 'r') as f:
        release = f.read().strip()
else:
    release = '1.0.0'

version = release

project = 'ESP32 Distance Sensor'
copyright = '2025, ESP32 Distance Sensor Team'
author = 'ESP32 Distance Sensor Team'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.todo',
    'sphinx.ext.viewcode',
    'sphinx.ext.graphviz',
    'sphinx_needs',
    'sphinxcontrib.plantuml',
    'myst_parser',
]

templates_path = ['_templates']
exclude_patterns = [
    '_build',
    'Thumbs.db',
    '.DS_Store',
    '01_general',  # Empty placeholder folders
    '02_development',
]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_static_path = ['_static']
html_title = 'ESP32 Distance Sensor'

# Custom CSS for dark mode fixes
html_css_files = [
    'custom.css',
]

# -- Sphinx-Needs Configuration ----------------------------------------------
# https://sphinx-needs.readthedocs.io/
# Ontology (types, statuses, extra_options) is defined in ubproject.toml.
# ubcode MCP server reads this file for structured needs queries.

needs_from_toml = "ubproject.toml"

# Configure needflow to use SVG instead of PNG (better quality, smaller size)
needs_flow_configs = {
    'needflow': {
        'engine': 'dot',
        'format': 'svg',
    }
}

# -- MyST Parser Configuration -----------------------------------------------
# https://myst-parser.readthedocs.io/

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "html_image",
]

# -- Intersphinx Configuration -----------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html

intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
}

# -- Todo Extension Configuration --------------------------------------------

todo_include_todos = True
