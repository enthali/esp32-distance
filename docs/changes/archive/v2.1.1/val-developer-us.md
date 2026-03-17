# Verification Report: developer-us

**Date**: 2026-03-17
**Change Proposal**: [docs/changes/developer-us.md](developer-us.md)
**Status**: ⚠️ PARTIAL

---

## Summary

| Category | Total | Verified | Issues |
|----------|-------|----------|--------|
| User Stories | 2 | 2 | 1 |
| Requirements | 11 | 11 | 0 |
| Designs | 19 | 19 | 0 |
| Doc files implemented | 10 | 10 | 3 |
| Traceability | — | — | 2 |

---

## User Stories Coverage

| ID | Title | REQ links | Code | Status |
|----|-------|-----------|------|--------|
| US_DEV_1 | Modular Firmware Extensibility | REQ_SYS_ARCH_1, REQ_TEMP_1 | ✅ existing arch | ✅ |
| US_DEV_2 | Traceable Supporting Documentation | ❌ no `:links:` | REQ_DEV_DOC_1…REQ_DEV_LIMITS_1 drive it | ⚠️ |

---

## Requirements Coverage

All requirements in `req_developer_docs.rst` link to US_DEV_2 and are included
in the requirements index toctree.

| REQ ID | Description | SPEC exists | Doc file exists | Status |
|--------|-------------|-------------|-----------------|--------|
| REQ_DEV_DOC_1 | Supporting Documentation Traceability | SPEC_DEV_* (many) | — principle | ✅ |
| REQ_DEV_README_1 | README Content Scope | SPEC_DEV_README_INTRO, META | README.md | ✅ |
| REQ_DEV_OV_INDEX_1 | Overview Index Content | SPEC_DEV_OV_INDEX_* | overview/index.rst | ⚠️ |
| REQ_DEV_OV_HW_1 | Hardware Specifications Content | SPEC_DEV_OV_HW_* | overview/hardware.rst | ✅ |
| REQ_DEV_OV_QS_1 | Quick Start Content | SPEC_DEV_OV_QS_* | overview/quick-start.rst | ✅ |
| REQ_DEV_GUIDE_QEMU_1 | QEMU Guide Content | SPEC_DEV_GUIDE_QEMU_* | guides/qemu-emulator.rst | ✅ |
| REQ_DEV_GUIDE_MODES_1 | Switching Dev Modes Guide | SPEC_DEV_GUIDE_MODES_* | guides/switching-dev-modes.rst | ✅ |
| REQ_DEV_GUIDE_DEBUG_1 | Debugging Guide Content | SPEC_DEV_GUIDE_DEBUG_* | guides/debugging.rst | ✅ |
| REQ_DEV_GUIDE_FLASH_1 | Web Flasher Guide Content | SPEC_DEV_GUIDE_FLASH_* | guides/web-flasher.rst | ✅ |
| REQ_DEV_ENV_1 | Supported Dev Environments | SPEC_DEV_ENV_OPTIONS, USAGE | guides/devcontainer.rst | ⚠️ |
| REQ_DEV_LIMITS_1 | Known Limitations Documentation | SPEC_DEV_LIMITS_STRUCTURE | guides/known-issues.rst | ❌ |

---

## Acceptance Criteria Verification

### REQ_DEV_README_1 — README

- [x] AC-1: Short project description (≤ 3 sentences) ✅
- [x] AC-2: Link to GitHub Pages ✅ (prominently in the body)
- [x] AC-3: License statement ✅ (MIT badge + section)
- [x] AC-4: Contributing pointer ✅
- [x] AC-5: No detailed hardware specs or build instructions ✅

### REQ_DEV_OV_INDEX_1 — Overview Index

- [x] AC-1: Describes what project does in plain language ✅
- [ ] AC-2: Lists the three required audiences (garage user, maker, developer) ⚠️
  - Current text shows: Makers & Tinkerers / Students & Learners / Workshop Participants / Embedded Developers
  - Required per SPEC_DEV_OV_INDEX_WHO: Garage user / Maker-builder / Developer
  - **"Garage user" is absent; "Developer" label is missing from the list**
- [x] AC-3: Navigation section with links to doc sections ✅

### REQ_DEV_OV_HW_1 — Hardware Page

- [x] AC-1: Component list with specs ✅
- [x] AC-2: GPIO pin assignment table ✅
- [x] AC-3: Reflects design-spec components (HC-SR04, WS2812) ✅
- [x] AC-4: (no hardware spec changed in this PR) ✅

### REQ_DEV_OV_QS_1 — Quick Start

- [x] AC-1: QEMU emulation path covered ✅
- [x] AC-2: Hardware flashing path covered ✅
- [x] AC-3: (REQ_SYS_SIM_1 did not change) ✅

### REQ_DEV_GUIDE_QEMU_1 — QEMU Guide

- [x] AC-1: Starting QEMU (script + VS Code task) ✅
- [x] AC-2: Accessing web interface from host ✅
- [x] AC-3: Stopping QEMU ✅

### REQ_DEV_GUIDE_MODES_1 — Switching Modes Guide

- [x] AC-1: menuconfig / sdkconfig approach documented ✅
- [x] AC-2: What changes between modes documented ✅
- [x] AC-3: (no QEMU/build split change in this PR) ✅

### REQ_DEV_GUIDE_DEBUG_1 — Debugging Guide

- [x] AC-1: GDB attach via VS Code launch config ✅
- [x] AC-2: Breakpoints, step-through, variable inspection ✅
- [x] AC-3: (dev container / debug config unchanged) ✅

### REQ_DEV_GUIDE_FLASH_1 — Web Flasher Guide

- [x] AC-1: Browser requirements (Web Serial API) ✅
- [x] AC-2: Full flash procedure (build → start server → flash) ✅
- [x] AC-3: Hardware prerequisites ✅
- [x] AC-4: (web flasher tool unchanged) ✅

### REQ_DEV_ENV_1 — Supported Dev Environments

- [x] AC-1: Devcontainer listed as supported ✅
- [x] AC-2: GitHub Codespaces listed as supported ✅
- [ ] AC-3: Native Windows NOT explicitly listed as unsupported ❌
  - The file contains a **"Windows USB Device Setup"** section (line 91 in
    `devcontainer.rst`), which implies Windows *is* supported as a host OS.
  - The spec (SPEC_DEV_ENV_OPTIONS) requires an explicit
    *"Windows not supported"* notice with rationale (ESP-IDF toolchain
    complexity → container required).
  - There is no such notice in the current file.
- [x] AC-4: Container setup steps documented ✅

### REQ_DEV_LIMITS_1 — Known Limitations

- [x] AC-1: Issues are organised into categories ⚠️
  - Categories present use emoji titles (🔒 HTTPS, 📱 Captive Portal, 🦄 Other)
  - Required categories per SPEC_DEV_LIMITS_STRUCTURE: **QEMU / Emulation,
    Hardware / Peripherals, Networking, Build System** — none present
- [ ] AC-2: Each entry does NOT reference relevant spec or requirement ❌
  - Entries use humorous prose with no `:need:` references
- [ ] AC-3: File structure and process governed by design spec — spec exists
  (SPEC_DEV_LIMITS_STRUCTURE), but the actual file does not follow it ❌

---

## Issues Found

### ⚠️ Issue 1: US_DEV_2 missing `:links:` attribute

- **Severity**: Medium
- **Category**: Traceability
- **Description**: `us_developer.rst` — `US_DEV_2` has no `:links:` field.
  Sphinx-Needs cannot generate forward links from the story to the
  requirements it drives.
- **Expected**: `:links: REQ_DEV_DOC_1` (at minimum; or all driven reqs)
- **Actual**: No `:links:` attribute present on `US_DEV_2`
- **Recommendation**: Add `:links: REQ_DEV_DOC_1` to US_DEV_2

---

### ⚠️ Issue 2: Overview index audience list does not match spec

- **Severity**: Medium
- **Category**: Requirements / Doc content
- **File**: `docs/01_overview/index.rst`, "Who Is This For?" section
- **Description**: SPEC_DEV_OV_INDEX_WHO requires the three project-defined
  stakeholder roles (Garage user / Maker-builder / Developer) to be
  enumerated explicitly. The current text lists different labels
  (Makers & Tinkerers, Students & Learners, Workshop Participants,
  Embedded Developers). The primary stakeholder "Garage user" is absent.
- **Expected**: Three audience entries matching the stakeholder model
- **Actual**: Four entries with different naming; no "Garage user"
- **Recommendation**: Align with the stakeholder terminology from
  `docs/10_userstories/index.rst`

---

### ❌ Issue 3: `devcontainer.rst` missing "Native Windows not supported" notice

- **Severity**: High
- **Category**: Requirements compliance (REQ_DEV_ENV_1 AC-3)
- **File**: `docs/90_guides/devcontainer.rst`
- **Description**: The spec (SPEC_DEV_ENV_OPTIONS) requires an explicit
  statement that **native Windows is not supported**, with the rationale
  that ESP-IDF toolchain complexity requires a container environment.
  Instead, the file has a "Windows USB Device Setup" section that implies
  Windows *is* a supported host — which is accurate for running the
  *container* on Windows, but the required unsupported-Windows NOTICE
  is absent.
- **Expected**: Explicit section or note: "Native Windows (without
  container): not supported — rationale: ESP-IDF toolchain conflicts with
  Windows PATH and shell assumptions"
- **Actual**: No such notice; Windows is listed positively in
  "Benefits: ✅ Consistent environments across Windows, macOS, Linux"
- **Recommendation**: Add a note in the Development Options section
  clarifying: Codespaces and local devcontainer *on Windows with Docker*
  are supported; **native Windows development (no container) is not
  supported**.

---

### ❌ Issue 4: `known-issues.rst` does not follow SPEC_DEV_LIMITS_STRUCTURE

- **Severity**: High
- **Category**: Design compliance (SPEC_DEV_LIMITS_STRUCTURE, REQ_DEV_LIMITS_1)
- **File**: `docs/90_guides/known-issues.rst`
- **Description**: The file uses free-form, humorous prose with emoji
  headings and no structured per-entry format. It does not comply with the
  design spec that governs its structure.
  - **Missing required categories**: QEMU / Emulation, Hardware /
    Peripherals, Networking, Build System
  - **Missing per-entry format**: no Origin reference (`:need:` link to
    spec/req), no structured Description + Workaround fields
  - **No spec link**: entries do not reference the origin spec or requirement
- **Expected**: Structured format per SPEC_DEV_LIMITS_STRUCTURE — defined
  categories, short title, origin reference, description, workaround per entry
- **Actual**: Two emoji-titled humorous sections with prose content and no
  traceability links
- **Recommendation**: Restructure `known-issues.rst` to comply with
  SPEC_DEV_LIMITS_STRUCTURE. Map existing issues to the correct categories
  and add `:need:` origin references (e.g. HTTPS → SPEC_WEB_SSL or
  REQ_SYS_NET_1; Captive Portal → REQ_WEB_1).

---

## Traceability Matrix

| Requirement | Design | Doc-file verified | Status |
|-------------|--------|-------------------|--------|
| US_DEV_1 → REQ_SYS_ARCH_1 | SPEC_ARCH_LAYERS_1 | existing arch code | ✅ |
| US_DEV_2 (no `:links:`) | — | — | ⚠️ |
| REQ_DEV_DOC_1 | multiple SPEC_DEV_* | principles applied | ✅ |
| REQ_DEV_README_1 | SPEC_DEV_README_INTRO, META | README.md | ✅ |
| REQ_DEV_OV_INDEX_1 | SPEC_DEV_OV_INDEX_* | overview/index.rst (partial) | ⚠️ |
| REQ_DEV_OV_HW_1 | SPEC_DEV_OV_HW_* | overview/hardware.rst | ✅ |
| REQ_DEV_OV_QS_1 | SPEC_DEV_OV_QS_* | overview/quick-start.rst | ✅ |
| REQ_DEV_GUIDE_QEMU_1 | SPEC_DEV_GUIDE_QEMU_* | guides/qemu-emulator.rst | ✅ |
| REQ_DEV_GUIDE_MODES_1 | SPEC_DEV_GUIDE_MODES_* | guides/switching-dev-modes.rst | ✅ |
| REQ_DEV_GUIDE_DEBUG_1 | SPEC_DEV_GUIDE_DEBUG_* | guides/debugging.rst | ✅ |
| REQ_DEV_GUIDE_FLASH_1 | SPEC_DEV_GUIDE_FLASH_* | guides/web-flasher.rst | ✅ |
| REQ_DEV_ENV_1 | SPEC_DEV_ENV_OPTIONS | guides/devcontainer.rst (AC-3 missing) | ⚠️ |
| REQ_DEV_LIMITS_1 | SPEC_DEV_LIMITS_STRUCTURE | guides/known-issues.rst (non-compliant) | ❌ |

---

## Recommendations

1. **Fix US_DEV_2 traceability** — add `:links: REQ_DEV_DOC_1` to `US_DEV_2`
   in `us_developer.rst` (low effort, high traceability value).

2. **Fix overview index audience list** — update "Who Is This For?" in
   `docs/01_overview/index.rst` to use the three canonical stakeholder labels
   (Garage user, Maker/builder, Developer) consistent with
   `docs/10_userstories/index.rst`.

3. **Add Windows-not-supported notice to devcontainer guide** — add a short
   note in the "Development Options" section:
   *"Native Windows (no container) is not supported — ESP-IDF requires
   a Linux-based container environment."*

4. **Restructure known-issues.rst** — this is the most significant gap.
   The file must be rewritten to follow SPEC_DEV_LIMITS_STRUCTURE: define
   the four required categories, move existing issues into correct categories,
   add `:need:` origin references, and apply the structured per-entry format
   (title / origin / description / workaround).

5. **Update spec statuses after fixes** — once items 1–4 are resolved,
   update all affected REQ_DEV_* and SPEC_DEV_* from `:status: approved`
   to `:status: implemented`.

---

## Conclusion

The core deliverables of `developer-us.md` are in place: both user stories
are documented, all 11 requirements and 19 design specs are created and
indexed. Eight of the ten referenced documentation files are fully compliant.

Two files fall short of the spec:

- `known-issues.rst` — structural non-compliance with SPEC_DEV_LIMITS_STRUCTURE
  (High, blocks full pass)
- `devcontainer.rst` — missing the required "native Windows not supported" notice
  (High, blocks full pass)

Additionally, `US_DEV_2` has no `:links:` field (Medium) and the overview index
audience labels are misaligned (Medium).

**Verdict: ⚠️ PARTIAL — two High issues must be resolved before marking the
change as implemented.**
