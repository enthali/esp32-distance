Developer Documentation Requirements
=====================================

This document specifies requirements for the Developer stakeholder role, covering
firmware extensibility and traceable supporting documentation.

**Document Version**: 1.0
**Last Updated**: 2026-03-16

Overview
--------

These requirements derive from :need:`US_DEV_1` and :need:`US_DEV_2`. They govern:

- The modular component structure that allows firmware extension
- Traceable links between each supporting documentation file and the
  design specifications it describes

Governing Principle
-------------------

.. req:: Supporting Documentation Traceability
   :id: REQ_DEV_DOC_1
   :links: US_DEV_2
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, traceability

   **Description:** The project SHALL maintain traceable links between
   supporting documentation files and the design specifications they describe,
   so that a spec change automatically brings the related documentation into
   the change impact scope.

   **Acceptance Criteria:**

   - AC-1: Each in-scope file SHALL have a dedicated design spec
   - AC-2: Each file-level spec SHALL define what content the file must cover
     (not how it is written)
   - AC-3: A change to a linked spec SHALL list the corresponding doc file
     in its change impact analysis


Overview Documentation Requirements
-------------------------------------

.. req:: README Content Scope
   :id: REQ_DEV_README_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, readme

   **Description:** ``README.md`` SHALL be intentionally minimal — a single
   entry point that orients the reader and points to the full documentation.
   It SHALL NOT duplicate content that lives in the Sphinx docs.

   **Acceptance Criteria:**

   - AC-1: README SHALL contain a short project description (≤3 sentences)
   - AC-2: README SHALL link to the GitHub Pages documentation site
   - AC-3: README SHALL contain a license statement
   - AC-4: README SHALL contain a contributing pointer
   - AC-5: README SHALL NOT contain detailed hardware specs, pin tables,
     or step-by-step build instructions (those live in the Sphinx docs)


.. req:: Overview Index Content
   :id: REQ_DEV_OV_INDEX_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, overview

   **Description:** ``docs/01_overview/index.rst`` SHALL serve as the project
   portal page, providing orientation and navigation for all audiences.

   **Acceptance Criteria:**

   - AC-1: SHALL describe what the project does in plain language
   - AC-2: SHALL list the target audiences (garage user, maker, developer)
   - AC-3: SHALL provide a section navigation map for the documentation
   - AC-4: Overlap with README intro is acceptable; detail beyond README
     is expected here


.. req:: Hardware Specifications Content
   :id: REQ_DEV_OV_HW_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, hardware

   **Description:** ``docs/01_overview/hardware.rst`` SHALL document all
   hardware components and wiring required to build the device.

   **Acceptance Criteria:**

   - AC-1: SHALL list required components with specifications
   - AC-2: SHALL provide GPIO pin assignment table
   - AC-3: SHALL reflect the component specs in ``docs/12_design/``
     (distance sensor, LED controller)
   - AC-4: SHALL be updated when hardware-related design specs change


.. req:: Quick Start Content
   :id: REQ_DEV_OV_QS_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, quickstart

   **Description:** ``docs/01_overview/quick-start.rst`` SHALL provide a
   working first-run experience for both QEMU and hardware paths.

   **Acceptance Criteria:**

   - AC-1: SHALL cover the QEMU emulation path (no hardware required)
   - AC-2: SHALL cover the hardware flashing path
   - AC-3: SHALL be updated when ``REQ_SYS_SIM_1`` or the build process changes


Guide Documentation Requirements
----------------------------------

.. req:: QEMU Emulator Guide Content
   :id: REQ_DEV_GUIDE_QEMU_1
   :links: US_DEV_2, REQ_DEV_DOC_1, REQ_SYS_SIM_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, qemu

   **Description:** ``docs/90_guides/qemu-emulator.rst`` SHALL document
   how to start, use, and stop the QEMU emulator for this project.

   **Acceptance Criteria:**

   - AC-1: SHALL cover starting QEMU (script and VS Code task)
   - AC-2: SHALL cover accessing the web interface from the host
   - AC-3: SHALL cover stopping QEMU
   - AC-4: SHALL be updated when the QEMU setup (``REQ_SYS_SIM_1``) changes


.. req:: Switching Dev Modes Guide Content
   :id: REQ_DEV_GUIDE_MODES_1
   :links: US_DEV_2, REQ_DEV_DOC_1, REQ_SYS_SIM_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, devmodes

   **Description:** ``docs/90_guides/switching-dev-modes.rst`` SHALL explain
   how and when to switch between QEMU emulation and real hardware targets.

   **Acceptance Criteria:**

   - AC-1: SHALL document the menuconfig / sdkconfig approach
   - AC-2: SHALL document what changes between modes
   - AC-3: SHALL be updated when the QEMU/hardware build split changes


.. req:: Debugging Guide Content
   :id: REQ_DEV_GUIDE_DEBUG_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, debugging

   **Description:** ``docs/90_guides/debugging.rst`` SHALL document how to
   debug the firmware in QEMU using VS Code and GDB.

   **Acceptance Criteria:**

   - AC-1: SHALL cover GDB attach via VS Code launch config
   - AC-2: SHALL cover breakpoints, step-through, variable inspection
   - AC-3: SHALL be updated when the dev container or debug config changes


.. req:: Web Flasher Guide Content
   :id: REQ_DEV_GUIDE_FLASH_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, documentation, flasher

   **Description:** ``docs/90_guides/web-flasher.rst`` SHALL document how
   to flash firmware to hardware using the browser-based web flasher.

   **Acceptance Criteria:**

   - AC-1: SHALL list browser requirements (Web Serial API)
   - AC-2: SHALL cover the full flash procedure (build → start server → flash)
   - AC-3: SHALL list hardware prerequisites
   - AC-4: SHALL be updated when the web flasher tool changes


Development Environment Requirements
--------------------------------------

.. req:: Supported Development Environments
   :id: REQ_DEV_ENV_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, devcontainer, codespaces

   **Description:** ``docs/90_guides/devcontainer.rst`` SHALL document which
   development environments are officially supported and which are not.

   **Acceptance Criteria:**

   - AC-1: Devcontainer (VS Code + Docker) SHALL be listed as supported
   - AC-2: GitHub Codespaces SHALL be listed as supported
   - AC-3: Native Windows SHALL be explicitly listed as not supported,
     with rationale (ESP-IDF toolchain complexity requires container)
   - AC-4: Container setup steps SHALL be documented


.. req:: Known Limitations Documentation
   :id: REQ_DEV_LIMITS_1
   :links: US_DEV_2, REQ_DEV_DOC_1
   :status: implemented
   :priority: mandatory
   :tags: developer, limitations, known-issues

   **Description:** ``docs/90_guides/known-issues.rst`` SHALL provide a
   maintained record of known limitations with traceable origins.

   **Acceptance Criteria:**

   - AC-1: Known issues SHALL be organised into categories
   - AC-2: Each entry SHALL reference the relevant spec or component
   - AC-3: The file's structure and maintenance process SHALL be governed
     by a design spec (not the issues themselves)
