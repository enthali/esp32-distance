# Verify Requirements Traceability

**Purpose:** Validate bidirectional traceability between requirements, design specifications, and code implementation.

**Output:** `temp/traceability-analysis.md`

---

## Scope

### Requirements Documentation

- `docs/11_requirements/*.rst` - All requirement specifications
- Sphinx-Needs requirement directives with IDs (REQ_*)

### Design Documentation

- `docs/12_design/*.rst` or `*.md` - Design specifications
- Links to requirements (`:links:` attribute)

### Code Implementation

- `main/**/*.c` and `main/**/*.h` - Source code
- Code comments with requirement IDs (e.g., `// REQ_CFG_1`)
- Doxygen comments referencing requirements

### Test Files

- `main/**/*test*.c` - Unit tests (if present)
- Test comments referencing requirements

---

## Traceability Checks

### 1. Requirements → Design

**Check:**
- Does every requirement have at least one design document referencing it?
- Are requirement links in design docs valid (ID exists)?
- Are high-priority requirements covered by design?

**Report:**
- Requirements without design documentation
- Broken requirement links in design docs
- Mandatory requirements lacking design coverage

### 2. Requirements → Code

**Check:**
- Does every requirement have code implementing it?
- Are requirement IDs in code comments valid?
- Do code comments reference correct requirement types?

**Report:**
- Requirements without implementation
- Code referencing non-existent requirements
- Mismatched requirement types (e.g., code claims REQ_CFG_1 but it's REQ_WEB_1)

### 3. Code → Requirements (Reverse Traceability)

**Check:**
- Does code with REQ_* references point to valid requirements?
- Is orphaned code (no REQ_* reference) documented as such?
- Are all major components traceable to requirements?

**Report:**
- Code with invalid requirement references
- Major features without requirement traceability
- Components missing requirement documentation

### 4. Design → Code

**Check:**
- Are design specifications implemented in code?
- Do implementations follow design decisions?
- Are design documents up-to-date with code changes?

**Report:**
- Designs without implementation
- Code diverging from design specifications
- Outdated design documents

### 5. Requirements Status Validation

**Check:**
- Are approved requirements implemented?
- Are draft requirements accidentally implemented?
- Are deprecated requirements still in code?

**Report:**
- Approved requirements without code
- Code implementing non-approved requirements
- Dead code from deprecated requirements

### 6. Acceptance Criteria Coverage

**Check:**
- Do requirements have testable acceptance criteria?
- Are acceptance criteria referenced in tests?
- Can each AC be verified?

**Report:**
- Requirements without acceptance criteria
- Untested acceptance criteria
- Vague or untestable criteria

---

## Analysis Process

1. **Parse Requirements** - Extract all REQ_* IDs from Sphinx-Needs docs
2. **Parse Design** - Extract requirement links from design docs
3. **Parse Code** - Find all REQ_* references in source files
4. **Build Traceability Matrix** - Map requirements → design → code
5. **Identify Gaps** - Find missing links in any direction
6. **Validate Status** - Check requirement status vs. implementation
7. **Prioritize Findings** - Categorize by requirement priority and impact

---

## Output Format

Write analysis to `temp/traceability-analysis.md`:

```markdown
# Requirements Traceability Analysis

**Date:** YYYY-MM-DD
**Branch:** <current-branch>

---

## 📊 Traceability Matrix Summary

| Requirement Area | Total Reqs | With Design | With Code | Complete* |
|------------------|-----------|-------------|-----------|-----------|
| System (REQ_SYS) | X | X | X | X% |
| Config (REQ_CFG) | X | X | X | X% |
| Web (REQ_WEB) | X | X | X | X% |
| Network (REQ_NETIF) | X | X | X | X% |
| **Total** | **X** | **X** | **X** | **X%** |

*Complete = Has design AND implementation

---

## ✅ Well-Traced Requirements

List requirements with complete traceability:
- REQ_SYS_1: System initialization → design/system-architecture.rst → main/main.c
- REQ_CFG_3: Runtime config structure → design/config-manager.rst → config_manager.c

---

## ⚠️ Traceability Gaps

### Critical (Mandatory requirements without implementation)

**1. [REQ_ID] - [Requirement Title]**
- **Priority:** mandatory/high
- **Status:** approved
- **Gap:** Missing implementation
- **Files Checked:** (list relevant source files)
- **Impact:** Core functionality not implemented
- **Recommendation:** Implement or update requirement status

### Medium (Requirements with partial traceability)

**2. [REQ_ID] - [Requirement Title]**
- **Gap:** Has design but no code implementation
- **Design Doc:** `docs/12_design/xyz.rst`
- **Impact:** Designed but not built
- **Recommendation:** Implement or mark as future work

**3. [REQ_ID] - [Requirement Title]**
- **Gap:** Has code but no design documentation
- **Code Location:** `main/components/xyz/xyz.c`
- **Impact:** Implementation without design rationale
- **Recommendation:** Add design documentation

### Low (Minor traceability issues)

**4. [REQ_ID] - [Requirement Title]**
- **Gap:** Acceptance criteria not testable
- **Issue:** Vague criteria or no test reference
- **Recommendation:** Clarify acceptance criteria

---

## 🔍 Orphaned Code

Code implementing features without requirement traceability:

**1. Component: xyz**
- **Files:** `main/components/xyz/*.c`
- **Issue:** No REQ_* references found
- **Impact:** Untraceable feature, unclear purpose
- **Recommendation:** Add requirements or document as internal implementation detail

---

## 🔗 Invalid References

### Code Referencing Non-Existent Requirements

**1. main/components/config_manager/config_manager.c:123**
- **Reference:** `// Implements REQ_CFG_99`
- **Issue:** REQ_CFG_99 not found in requirements docs
- **Recommendation:** Fix requirement ID or remove invalid reference

### Design Docs with Broken Links

**2. docs/12_design/web-server.rst**
- **Reference:** `:links: REQ_WEB_15`
- **Issue:** REQ_WEB_15 not found in requirements
- **Recommendation:** Update link to correct requirement ID

---

## 📋 Requirements Status Report

### Approved Requirements (should be implemented)

- **Total approved:** X
- **Implemented:** X (X%)
- **Not implemented:** X (X%)

**Not implemented list:**
1. REQ_SYS_5 - [Title]
2. REQ_CFG_8 - [Title]

### Draft Requirements (should NOT be implemented yet)

- **Total draft:** X
- **Accidentally implemented:** X

**Accidentally implemented list:**
1. REQ_WEB_12 - [Title] - Found in `web_server.c`

### Deprecated Requirements (should be removed from code)

- **Total deprecated:** X
- **Still in code:** X

**Still in code list:**
1. REQ_OLD_3 - [Title] - Found in `legacy_component.c`

---

## 📈 Recommendations

### Immediate Actions (Critical)

1. [ ] Implement mandatory requirements without code
2. [ ] Fix invalid requirement references in code
3. [ ] Remove code implementing deprecated requirements

### Short-term (Medium Priority)

1. [ ] Add design documentation for implemented features
2. [ ] Implement features with existing design docs
3. [ ] Clarify vague acceptance criteria

### Long-term (Maintenance)

1. [ ] Add requirement references to orphaned code
2. [ ] Improve acceptance criteria testability
3. [ ] Establish pre-commit hook to validate REQ_* references

---

## 🎯 Traceability Health Score

**Overall Score:** X/100

- Requirements coverage: X%
- Design coverage: X%
- Implementation coverage: X%
- Bidirectional traceability: X%

**Target:** 80%+ for mandatory requirements
```

---

## Usage

Run this prompt when:
- Before releases (validate all approved requirements are implemented)
- After adding new requirements
- During design reviews
- Periodically (quarterly) for maintenance

Recommended workflow:
1. Run prompt: "Please verify requirements traceability following verify-requirements-traceability.prompt.md"
2. Review generated `temp/traceability-analysis.md`
3. Create issues for critical gaps
4. Update requirements status or code as needed
5. Commit analysis file for reference

---

## Notes

- **Requirement ID Format:** `REQ_<AREA>_<NUMBER>`
  - `REQ_SYS_*` - System requirements
  - `REQ_CFG_*` - Configuration manager
  - `REQ_WEB_*` - Web server
  - `REQ_NETIF_*` - Network tunnel
  
- **Expected Traceability:**
  - Mandatory requirements: MUST have design + code
  - High priority: SHOULD have design + code
  - Medium/Low: MAY have code without design for simple features

- **Exclusions:**
  - Internal implementation details (no requirement needed)
  - Third-party components (ESP-IDF, QEMU)
  - Build system internals

- **Focus on:**
  - User-facing features
  - System architecture decisions
  - Configuration and management features
