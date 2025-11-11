Pre-commit Hooks
================

This project uses `pre-commit <https://pre-commit.com/>`_ hooks to ensure code quality and documentation integrity before commits reach the repository.

Overview
--------

Pre-commit hooks run automatically on ``git commit`` and validate changes before they are committed. This ensures:

* Documentation is always valid and buildable
* Markdown syntax follows consistent style guidelines  
* RestructuredText is valid for Sphinx
* Links and references are correct
* Code quality standards are met
* Both human developers and AI agents (like GitHub Copilot) follow the same quality gates

Philosophy: Quality as a Process
---------------------------------

Pre-commit hooks are **automation assistants**, not quality gatekeepers. They:

* **Catch trivial errors** so humans can focus on design and logic
* **Enforce consistency** automatically (formatting, style)
* **Provide fast feedback** during development, not just at "the gate"

**Quality should be everywhere:**

* Write tests alongside code
* Review design before implementation
* Use IDE linters while editing (not just at commit time)
* Think about maintainability from the start

Pre-commit hooks are the *last automated safety check*, not the *first time* you think about quality.

Current Hooks
-------------

Markdown Linting
~~~~~~~~~~~~~~~~

**Hook ID:** ``markdownlint``

Validates Markdown syntax and style consistency across temporary/analysis documents.

**Configuration** (``.pre-commit-config.yaml``):

.. code-block:: yaml

   - id: markdownlint
     name: Markdown lint
     entry: markdownlint --fix
     language: system
     types: [markdown]
     files: ^(temp/.*\.md)$

**What it checks:**

* Consistent heading styles and hierarchy
* Blank lines before lists and code blocks
* Trailing whitespace and multiple blank lines
* Proper code fence formatting
* Link reference definitions
* Consistent list marker styles

**When it runs:**

* On any ``.md`` file in ``temp/`` directory (analysis docs)
* Auto-fixes issues when possible (e.g., adds missing blank lines)

**Configuration** (``.markdownlint.json``):

.. code-block:: json

   {
     "default": true,
     "MD013": false,
     "MD033": false,
     "MD041": false,
     "MD046": {
       "style": "fenced"
     }
   }

**Key rules:**

* ``MD013: false`` - No line length limit (avoids breaking URLs)
* ``MD033: false`` - Allow inline HTML (useful for tables)
* ``MD041: false`` - Don't require H1 as first line
* ``MD046: fenced`` - Enforce fenced code blocks (\`\`\`) instead of indented

**Output example:**

.. code-block:: text

   Markdown lint........................................................Failed
   - hook id: markdownlint
   - files were modified by this hook

   temp/analysis.md:38:1 MD032/blanks-around-lists 
   Lists should be surrounded by blank lines

   Fixed automatically - please review and stage changes

Sphinx Documentation Validation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Hook ID:** ``sphinx-build``

Validates all Sphinx documentation before allowing commits.

**Configuration** (``.pre-commit-config.yaml``):

.. code-block:: yaml

   - id: sphinx-build
     name: Sphinx build (strict)
     entry: sphinx-build -W -b html docs docs/_build/html
     language: system
     files: ^(docs/.*\.rst|docs/conf\.py)$

**What it checks:**

* Broken cross-references (``:doc:``, ``:ref:``)
* Invalid RestructuredText syntax
* Missing files referenced in toctrees
* Sphinx configuration errors
* Sphinx-Needs requirement traceability issues

**When it runs:**

* Only when files in ``docs/`` directory change
* Only when ``docs/conf.py`` configuration changes
* Skipped for code-only commits (performance optimization)

**Output example:**

.. code-block:: text

   Sphinx build (strict)................................................Failed
   - hook id: sphinx-build
   - duration: 2.5s
   - exit code: 2

   WARNING: document isn't included in any toctree
   ERROR: Unknown directive type "req"

   Build failed due to warnings (strict mode)!

Installation
------------

Automatic (Devcontainer)
~~~~~~~~~~~~~~~~~~~~~~~~

**Pre-commit hooks are pre-installed** in GitHub Codespaces and dev containers.

No manual installation required!

Manual Installation (Local)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If working outside the devcontainer:

.. code-block:: bash

   # Install pre-commit
   pip install pre-commit

   # Install hooks
   pre-commit install

   # Verify installation
   pre-commit --version

Usage
-----

Running Hooks
~~~~~~~~~~~~~

**Automatic (on commit):**

.. code-block:: bash

   git add .
   git commit -m "Your message"
   # Hooks run automatically

**Manual (test before commit):**

.. code-block:: bash

   # Run on staged files only
   pre-commit run

   # Run on all files
   pre-commit run --all-files

   # Run specific hook
   pre-commit run markdownlint
   pre-commit run sphinx-build

**Show differences:**

.. code-block:: bash

   # Show what hooks would fix
   pre-commit run --all-files --show-diff-on-failure

Typical Workflow
~~~~~~~~~~~~~~~~

1. **Make changes** to documentation or code
2. **Stage changes:** ``git add docs/``
3. **Commit:** ``git commit -m "docs: Update requirements"``
4. **Hooks run automatically:**
   
   * If **pass** ✅ - Commit succeeds
   * If **fail** ❌ - Commit blocked, review errors
   * If **auto-fixed** 🔧 - Review changes, stage, commit again

5. **If auto-fixed:**

   .. code-block:: bash

      git add .  # Stage auto-fixes
      git commit -m "docs: Update requirements"  # Retry commit

Handling Failures
~~~~~~~~~~~~~~~~~

**Markdown lint failures:**

.. code-block:: bash

   # Auto-fix formatting issues
   markdownlint --fix temp/*.md

   # Review changes
   git diff

   # Stage and retry
   git add temp/
   git commit -m "..."

**Sphinx build failures:**

.. code-block:: bash

   # Build locally to see full error
   sphinx-build -W -b html docs docs/_build/html

   # Fix issues (broken links, invalid RST syntax)
   # Retry commit

**Skip hooks (emergency only):**

.. code-block:: bash

   git commit --no-verify -m "..."

.. warning::

   **Don't skip hooks regularly!** This defeats the purpose of automated quality checks. 
   Use ``--no-verify`` only in emergencies (e.g., critical hotfix).

Common Issues
-------------

Issue: "markdownlint: command not found"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Solution:**

.. code-block:: bash

   npm install -g markdownlint-cli

Issue: "sphinx-build: command not found"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Solution:**

.. code-block:: bash

   pip install sphinx sphinx-needs

Issue: Hooks run slowly
~~~~~~~~~~~~~~~~~~~~~~~

**Optimization:**

* Hooks only run on changed files (not entire repo)
* Sphinx hook only runs for ``.rst`` file changes
* Use ``pre-commit run`` to test specific hooks

Issue: False positives
~~~~~~~~~~~~~~~~~~~~~~

**Markdown linting:**

Disable specific rules in ``.markdownlint.json``:

.. code-block:: json

   {
     "MD013": false,  // Disable line length check
     "MD033": false   // Allow inline HTML
   }

**Sphinx warnings:**

Some warnings can be suppressed in ``docs/conf.py`` if genuinely false positives.

Benefits
--------

✅ **Consistent quality** - All commits meet minimum standards

✅ **Fast feedback** - Catch errors before pushing to remote

✅ **Automated fixes** - Many issues fixed automatically

✅ **Documentation integrity** - Broken links caught immediately

✅ **CI/CD confidence** - Local checks match CI pipeline

✅ **Learning tool** - See quality issues and understand corrections

Best Practices
--------------

For Developers
~~~~~~~~~~~~~~

* **Run hooks before committing** large changes
* **Review auto-fixes** - don't blindly accept
* **Fix root causes** - not just symptoms
* **Update rules** - if they don't make sense, discuss and adjust

For Documentation Writers
~~~~~~~~~~~~~~~~~~~~~~~~~

* **Test Sphinx build locally** before committing
* **Validate cross-references** - use ``make linkcheck``
* **Follow RST syntax** - reference Sphinx documentation
* **Keep temp/ docs clean** - run markdownlint on analysis docs

For AI Coding Agents
~~~~~~~~~~~~~~~~~~~~

* **Understand hook output** - parse error messages correctly
* **Auto-fix when possible** - stage fixes and retry
* **Report persistent failures** - don't infinitely retry
* **Learn from patterns** - avoid repeating same mistakes

Configuration Files
-------------------

``.pre-commit-config.yaml``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Main configuration file defining all hooks:

.. code-block:: yaml

   repos:
     - repo: local
       hooks:
         - id: markdownlint
           name: Markdown lint
           entry: markdownlint --fix
           language: system
           types: [markdown]
           files: ^(temp/.*\.md)$

         - id: sphinx-build
           name: Sphinx build (strict)
           entry: sphinx-build -W -b html docs docs/_build/html
           language: system
           files: ^(docs/.*\.rst|docs/conf\.py)$

``.markdownlint.json``
~~~~~~~~~~~~~~~~~~~~~~

Markdownlint-specific rules:

.. code-block:: json

   {
     "default": true,
     "MD013": false,
     "MD033": false,
     "MD041": false,
     "MD046": {"style": "fenced"}
   }

``docs/conf.py``
~~~~~~~~~~~~~~~~

Sphinx configuration with Sphinx-Needs for requirements engineering.

Continuous Integration
----------------------

Pre-commit hooks run in CI/CD pipeline to ensure quality gates:

**GitHub Actions** (``.github/workflows/docs.yml``):

.. code-block:: yaml

   - name: Run pre-commit
     run: pre-commit run --all-files

This ensures:

* Pull requests pass same checks as local development
* No quality regressions merge to main
* Automated documentation deployment is always valid

Related Documentation
---------------------

* :doc:`index` - All development guides
* :doc:`devcontainer` - Development environment setup
* :doc:`debugging` - Debugging workflows
* `Pre-commit Documentation <https://pre-commit.com/>`_ - Official docs

---

**Version:** 1.0  
**Last Updated:** 2025-01-27  
**Applies to:** GitHub Copilot, human developers, CI/CD pipelines
