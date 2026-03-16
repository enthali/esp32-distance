# Change Document: doc-consistency

**Status**: approved
**Branch**: feature/doc-consistency
**Created**: 2026-03-16
**Author**: Developer

---

## Summary

Two stakeholder roles (Developer, Workshop participant) are defined in the project but have no User Stories. This change addresses the Developer role (G-1 from mece-us-findings.rst). It adds:
1. A Developer user story for **firmware extensibility** (adding sensors/components without rewriting core)
2. A Developer user story for **documentation traceability** (supporting docs like README, guides, overview pages linked to the specs they describe)

The documentation traceability story drives a Level-1 decision on which spec documents should exist, and Level-2 decisions on their content and requirement mapping.

Source finding: `temp/mece-us-findings.rst` G-1

---

## Level 0: User Stories

**Status**: ✅ completed

### New User Stories
- US_DEV_1: Modular Firmware Extensibility (`docs/10_userstories/us_developer.rst`)
- US_DEV_2: Traceable Supporting Documentation (`docs/10_userstories/us_developer.rst`)

### Scope Decisions
- `introduction.rst` — out of scope (narrative)
- `qemu-network-internals.rst` — out of scope (netif-tunnel branch)
- `pre-commit-hooks.rst` — **deleted** (dead file)
- `docs/05_workshop/` — out of scope (G-2, separate change)

---

## Level 1: Requirements

**Status**: ✅ completed

### New Requirements (`docs/11_requirements/req_developer_docs.rst`)
- REQ_DEV_DOC_1: Supporting Documentation Traceability (governing principle)
- REQ_DEV_README_1: README Content Scope
- REQ_DEV_OV_INDEX_1: Overview Index Content
- REQ_DEV_OV_HW_1: Hardware Specifications Content
- REQ_DEV_OV_QS_1: Quick Start Content
- REQ_DEV_GUIDE_QEMU_1: QEMU Emulator Guide Content
- REQ_DEV_GUIDE_MODES_1: Switching Dev Modes Guide Content
- REQ_DEV_GUIDE_DEBUG_1: Debugging Guide Content
- REQ_DEV_GUIDE_FLASH_1: Web Flasher Guide Content
- REQ_DEV_ENV_1: Supported Development Environments
- REQ_DEV_LIMITS_1: Known Limitations Documentation

---

## Level 2: Design

**Status**: ✅ completed

### New Spec Files

**`docs/12_design/spec_developer_docs.rst`** (16 SPECs):
- SPEC_DEV_README_INTRO, SPEC_DEV_README_META
- SPEC_DEV_OV_INDEX_WHAT, SPEC_DEV_OV_INDEX_WHO, SPEC_DEV_OV_INDEX_NAV
- SPEC_DEV_OV_HW_COMPONENTS, SPEC_DEV_OV_HW_PINS
- SPEC_DEV_OV_QS_QEMU, SPEC_DEV_OV_QS_HW
- SPEC_DEV_GUIDE_QEMU_START, SPEC_DEV_GUIDE_QEMU_WEB, SPEC_DEV_GUIDE_QEMU_STOP
- SPEC_DEV_GUIDE_MODES_HOW, SPEC_DEV_GUIDE_MODES_DIFF
- SPEC_DEV_GUIDE_DEBUG_START, SPEC_DEV_GUIDE_DEBUG_FEATURES
- SPEC_DEV_GUIDE_FLASH_OVERVIEW, SPEC_DEV_GUIDE_FLASH_STEPS

**`docs/12_design/spec_dev_environment.rst`** (2 SPECs):
- SPEC_DEV_ENV_OPTIONS, SPEC_DEV_ENV_USAGE

**`docs/12_design/spec_known_limitations.rst`** (1 SPEC):
- SPEC_DEV_LIMITS_STRUCTURE

---

## Final Consistency Check

**Status**: ✅ passed

### Traceability Verification

| User Story | Requirements | Design Specs | Complete? |
|------------|-------------|--------------|-----------|
| US_DEV_1 | REQ_SYS_ARCH_1 (existing) | SPEC_ARCH_LAYERS_1 (existing) | ✅ |
| US_DEV_2 | REQ_DEV_DOC_1 | *(governing principle — no direct SPEC)* | ✅ |
| US_DEV_2 | REQ_DEV_README_1 | SPEC_DEV_README_INTRO, SPEC_DEV_README_META | ✅ |
| US_DEV_2 | REQ_DEV_OV_INDEX_1 | SPEC_DEV_OV_INDEX_WHAT, _WHO, _NAV | ✅ |
| US_DEV_2 | REQ_DEV_OV_HW_1 | SPEC_DEV_OV_HW_COMPONENTS, _PINS | ✅ |
| US_DEV_2 | REQ_DEV_OV_QS_1 | SPEC_DEV_OV_QS_QEMU, _HW | ✅ |
| US_DEV_2 | REQ_DEV_GUIDE_QEMU_1 | SPEC_DEV_GUIDE_QEMU_START, _WEB, _STOP | ✅ |
| US_DEV_2 | REQ_DEV_GUIDE_MODES_1 | SPEC_DEV_GUIDE_MODES_HOW, _DIFF | ✅ |
| US_DEV_2 | REQ_DEV_GUIDE_DEBUG_1 | SPEC_DEV_GUIDE_DEBUG_START, _FEATURES | ✅ |
| US_DEV_2 | REQ_DEV_GUIDE_FLASH_1 | SPEC_DEV_GUIDE_FLASH_OVERVIEW, _STEPS | ✅ |
| US_DEV_2 | REQ_DEV_ENV_1 | SPEC_DEV_ENV_OPTIONS, _USAGE | ✅ |
| US_DEV_2 | REQ_DEV_LIMITS_1 | SPEC_DEV_LIMITS_STRUCTURE | ✅ |

### Existing SPEC Overlap Check

| Existing ID | Scope | New SPEC | Relationship | Conflict? |
|-------------|-------|----------|-------------|-----------|
| SPEC_ARCH_CODESPACES_1 | Codespaces architecture, workflow diagram | SPEC_DEV_ENV_OPTIONS | Arch = HOW it works; Dev = WHAT the doc must say | ✅ no conflict — SPEC_DEV_ENV_OPTIONS links to both REQ_DEV_ENV_1 and SPEC_ARCH_CODESPACES_1 |
| SPEC_ARCH_QEMU_1 | QEMU hardware abstraction, network stack | SPEC_DEV_GUIDE_QEMU_START/WEB/STOP | Arch = HOW it works; Guide = WHAT the user doc must cover | ✅ no conflict — guide SPECs link to REQ_SYS_SIM_1 |
| SPEC_NETIF_UART_* | Wire protocol, lwIP driver (parallel branch) | SPEC_DEV_GUIDE_MODES_DIFF links REQ_NETIF_TUNNEL_5 | Arch = wire protocol; Guide = user-facing switching steps | ✅ no conflict |

### Scope Decisions Confirmed

| Item | Decision |
|------|----------|
| `docs/01_overview/introduction.rst` | Out of scope — narrative text, not spec-driven |
| `docs/90_guides/qemu-network-internals.rst` | Out of scope — handled in netif-tunnel branch |
| `docs/90_guides/pre-commit-hooks.rst` | **DELETE** — dead file, no longer used |
| `docs/05_workshop/` | Out of scope — separate change (G-2) |
| `docs/90_guides/devcontainer.rst` | In scope as REQ_DEV_ENV_1 |
| `docs/90_guides/known-issues.rst` | In scope as REQ_DEV_LIMITS_1 |

### Sign-off

- ✅ All levels completed — no ⚠️ DEPRECATED markers
- ✅ All conflicts resolved
- ✅ Traceability verified — all new REQs link to US, all new SPECs link to REQs
- ✅ No ID collisions with existing 130+ IDs
- ✅ Existing SPECs (SPEC_ARCH_CODESPACES_1, SPEC_ARCH_QEMU_1) complemented, not duplicated
- ✅ README scope deliberately minimal — bloat prevention built into SPEC
- ✅ Ready for implementation

---

*Generated by syspilot Change Agent*
