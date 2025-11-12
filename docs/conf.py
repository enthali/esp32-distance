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

needs_types = [
    dict(
        directive="req",
        title="Requirement",
        prefix="REQ_",
        color="#BFD8D2",
        style="node"
    ),
    dict(
        directive="spec",
        title="Design Specification",
        prefix="DSN_",
        color="#FEDCD2",
        style="node"
    ),
    dict(
        directive="impl",
        title="Implementation",
        prefix="IMPL_",
        color="#DF744A",
        style="node"
    ),
    dict(
        directive="test",
        title="Test Case",
        prefix="TEST_",
        color="#DCB239",
        style="node"
    ),
    # API Documentation Types
    dict(
        directive="apicomponent",
        title="API Component",
        prefix="API_COMP_",
        color="#A8DADC",
        style="node"
    ),
    dict(
        directive="apifunction",
        title="API Function",
        prefix="API_FUNC_",
        color="#457B9D",
        style="node"
    ),
    dict(
        directive="apistruct",
        title="API Data Structure",
        prefix="API_STRUCT_",
        color="#1D3557",
        style="node"
    ),
]

# Extra options for needs (status is built-in, don't redefine it)
needs_extra_options = [
    "priority",
    "rationale",
    "acceptance_criteria",
    # API-specific options
    "api_signature",   # Function signature (e.g., "esp_err_t config_init(void)")
    "returns",         # Return value description
    "parameters",      # Parameter descriptions
    "component",       # Parent component name
    "header_file",     # Source header file path
]

# Status options
needs_statuses = [
    dict(name="draft", description="Draft - Work in progress"),
    dict(name="open", description="Open - Identified but not yet started"),
    dict(name="approved", description="Approved - Ready for implementation"),
    dict(name="implemented", description="Implemented - Code exists"),
    dict(name="verified", description="Verified - Tested and validated"),
    dict(name="deprecated", description="Deprecated - No longer used"),
]

# Priority options
needs_priority = [
    dict(name="mandatory", description="Must have - Critical requirement"),
    dict(name="high", description="Should have - Important requirement"),
    dict(name="medium", description="Could have - Nice to have"),
    dict(name="low", description="Won't have this time - Future consideration"),
]

# Enable automatic ID generation
needs_id_required = True

# Configure needs file output
needs_build_json = True
needs_build_json_per_id = True

# Use Graphviz for needflow diagrams instead of PlantUML
needs_flow_engine = "graphviz"

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
