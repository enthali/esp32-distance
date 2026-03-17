# Change Document: developer-us

**Status**: in-progress
**Branch**: consistency-checks
**Created**: 2026-03-16
**Author**: GitHub Copilot / syspilot.change

---

## Summary

Add a User Story for the *Developer* stakeholder role, which is defined in
`docs/10_userstories/index.rst` but has no corresponding story file.

The Developer role covers two orthogonal concerns:
1. **Firmware extensibility** — adding sensors, display types, or board variants
   without rewriting core logic.
2. **Documentation traceability** — README, guides, and overview pages drift
   silently when specs change; linking them to specs makes them part of change
   impact analysis.

This change adds `us_developer.rst`, derives `REQ_DEV_*` requirements, and
creates a companion design spec.

---

## Level 0: User Stories

**Status**: 🔄 in progress

### Impacted User Stories (existing)

| ID | Title | Impact | Notes |
|----|-------|--------|-------|
| US_DISPLAY_1 | Visual Distance Feedback | none | Garage user, unrelated |
| US_SETUP_1 | Device Setup via Web Interface | none | Maker/builder, unrelated |
| US_RELIABLE_1 | Reliable operation | none | Garage user, unrelated |

No existing US is impacted — this is a pure addition.

### New User Stories

**US_DEV_1** — Modular Firmware Architecture

```rst
.. story:: Modular Firmware Architecture for Extensibility
   :id: US_DEV_1
   :status: draft
   :links: (to be filled after REQ level)
   :tags: developer, firmware, architecture

   **Role:** Developer

   **Story:**
   As a developer, I want the firmware to be structured in independent,
   replaceable components (sensor, display, config, network), so that I can
   add a new sensor type or display variant without modifying unrelated code.

   **Value:**
   Component isolation means a developer can swap HC-SR04 for a LIDAR, or
   add a second display type, by creating a new component and wiring it into
   main.c — without touching sensor logic, config manager, or the web server.
```

**US_DEV_2** — Documentation Traceability for Supporting Docs

```rst
.. story:: Traceable Supporting Documentation
   :id: US_DEV_2
   :status: draft
   :links: (to be filled after REQ level)
   :tags: developer, documentation, traceability

   **Role:** Developer

   **Story:**
   As a developer, I want the README, overview pages, and guides to be
   linked to the specs they describe, so that when a spec changes the
   supporting documentation is automatically included in the change impact
   scope.

   **Value:**
   Today README and guides go stale silently — a spec change triggers a
   code review but not a doc review. With traceability links, Sphinx-Needs
   can flag outdated docs and a developer reviewing a change sees exactly
   which supporting docs need updating.
```

### Horizontal Check (MECE)

- ✅ US_DEV_1 (firmware extensibility) does not overlap with US_DISPLAY_1 or
  US_SETUP_1 — those describe *what the device does*, DEV_1 describes *how
  a developer extends it*
- ✅ US_DEV_2 (doc traceability) does not overlap with any existing story —
  it addresses the meta-layer of documentation maintenance
- ✅ US_DEV_1 and US_DEV_2 are mutually exclusive: firmware architecture vs.
  documentation governance — no overlap
- ✅ Together they cover all sub-areas identified in the MECE findings (G-1)
- ⚠️ US_DEV_2 has no code component — requirements will be process/doc
  requirements, not firmware requirements. This is intentional and valid.

### Decisions

- D-1: Split into two stories (architecture + traceability) rather than one
  monolithic "developer" story — they drive different requirement areas
- D-2: US_DEV_2 is in-scope even though it produces no firmware code; it
  produces traceability links in RST files
- D-3: Workshop participant story (G-2) is explicitly out-of-scope and low
  priority — deferred

---

## Level 1: Requirements

**Status**: ⏳ not started

---

## Level 2: Design

**Status**: ⏳ not started

---

## Open Questions

- Q-1: Should REQ_DEV_* live in a new file `req_developer.rst` or be appended
  to `req_system.rst`? → Proposed: new file (consistent with all other areas)
- Q-2: For US_DEV_2 doc traceability: should the spec define *which* docs must
  be linked, or only the *principle* that supporting docs shall be linked?
  → Proposed: list specific docs (README, overview, guides index)
