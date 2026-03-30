Developer User Stories
=======================

This document contains user stories for the Developer stakeholder role — engineers
extending, maintaining, or contributing to the firmware and documentation.

.. story:: Modular Firmware Extensibility
   :id: US_DEV_1
   :status: implemented
   :links: REQ_SYS_ARCH_1
   :tags: developer, firmware, extensibility

   **Role:** Developer

   **Story:**
   As a developer, I want a modular component structure so that I can add a
   new sensor or display type without modifying the core application logic.

   **Value:**
   The component-based architecture allows extending the firmware by adding
   or replacing components. Existing components serve as reference
   implementations. The developer is not required to understand the full
   system to add a new peripheral.


.. story:: Hardware-Free Development via QEMU Emulation
   :id: US_DEV_3
   :status: approved
   :links: REQ_SYS_SIM_1, REQ_SYS_SIM_2
   :tags: developer, qemu, emulation

   **Role:** Developer

   **Story:**
   As a developer, I want to be able to develop and test the firmware without
   physical hardware using QEMU emulation, so that I can iterate quickly and
   run automated tests in CI without needing an ESP32 board.

   **Value:**
   QEMU emulation removes the dependency on physical hardware during
   development. Developers can build, flash, and test in a fully emulated
   environment — including network features via UART tunnel. This enables
   faster iteration, reproducible CI pipelines, and onboarding of new
   contributors who do not have hardware.


.. story:: Traceable Supporting Documentation
   :id: US_DEV_2
   :status: implemented
   :links: REQ_DEV_DOC_1
   :tags: developer, documentation, traceability

   **Role:** Developer

   **Story:**
   As a developer, I want README, overview pages and guides to be linked to
   the specs they describe, so that when a spec changes, supporting docs are
   automatically in the change impact scope — including which development
   environments are supported and what known limitations exist.

   **Value:**
   Without traceability, supporting documentation silently goes stale when
   specs change. A developer setting up for the first time needs to know
   which environments are supported (devcontainer / Codespaces — not
   Windows) and where to find known limitations, without discovering them
   by trial and error.
