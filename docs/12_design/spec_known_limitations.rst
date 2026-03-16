Known Limitations Structure Specification
==========================================

This document governs the structure and maintenance process for
``docs/90_guides/known-issues.rst``. It defines how limitations are
recorded and traced back to their origin — not the limitations themselves.

**Document Version**: 1.0
**Last Updated**: 2026-03-16

.. spec:: Known Limitations File Structure and Process
   :id: SPEC_DEV_LIMITS_STRUCTURE
   :links: REQ_DEV_LIMITS_1
   :status: approved
   :tags: developer, limitations, known-issues

   **Description:** ``docs/90_guides/known-issues.rst`` SHALL be structured
   with defined categories, a per-entry format, and a maintenance process
   so that limitations are traceable and discoverable.

   **Required categories:**

   - QEMU / Emulation
   - Hardware / Peripherals
   - Networking
   - Build System

   **Per-entry format:**

   Each entry SHALL contain:

   - **Short title** — one line describing the limitation
   - **Origin reference** — link to the relevant spec or requirement
     (e.g. ``:need:`REQ_SYS_SIM_1```)
   - **Description** — what the limitation is and when it is encountered
   - **Workaround** — how to work around it, or "None" if no workaround exists

   **Maintenance process:**

   - Entries are ADDED when a limitation is first encountered and confirmed
   - Entries are REMOVED when the limitation is resolved, with a note
     referencing the commit or PR that fixed it
   - The file is reviewed as part of change impact analysis whenever a
     linked spec changes

   **What this spec does NOT govern:**

   The actual content of individual known-issue entries is not specified here.
   Only the structure and process are governed. Issue content is maintained
   by developers as limitations are discovered and resolved.
