---
description: Generate GitHub Issue for Coding Agent from feature description
---

# Generate Feature Issue for Coding Agent

**Use `mcp_github_create_issue` tool - DO NOT output text manually!**

## Feature Request

${input:featureRequest:Describe the feature you want to implement}

---

## Analysis & Issue Creation

1. **Search the codebase**:
   - Find related requirements in `docs/11_requirements/`
   - Find related design specs in `docs/12_design/`
   - Identify affected components

2. **Determine documentation evolution** ⚠️ **CRITICAL**:
   - What to **update** (modify existing REQ-*/SPEC-*)?
   - What to **supersede** (mark old REQ-*/SPEC-* as superseded)?
   - What **new** documentation needed?
   - **NEVER create conflicts** - all documentation must be consistent

3. **Create issue** using `mcp_github_create_issue`:
   - **title**: `[Feature]: CLEAR_TITLE`
   - **labels**: `["feature", "COMPONENT", "coding-agent"]`
   - **body**: Use structure below ↓

---

## Issue Body Structure

```markdown
## 🎯 Objective

<Clear statement: what needs to be done and why>

## 📋 Context

**Existing Requirements** (from docs/11_requirements/):
- To Update: <REQ-* IDs>
- To Supersede: <REQ-* IDs>
- To Add: <New REQ-* IDs>

**Existing Design Docs** (from docs/12_design/):
- To Update: <SPEC-* IDs>
- To Supersede: <SPEC-* IDs>
- To Add: <New SPEC-* IDs>

**Affected Components**: <files/modules>
**Current Behavior**: <from code analysis>
**Desired Behavior**: <from feature request>

## 🔧 Technical Details

<ESP32-specific constraints, dependencies>

---

## Implementation Workflow

### Requirements & Design
1. **Update requirements** in `docs/11_requirements/req_<area>.rst`
   - Follow Sphinx-Needs format (see `docs/conf.py`)
   - Status: `draft` (new), `approved` (updated), `superseded` (old)
   - **NO conflicting requirements**
2. **Update design docs** in `docs/12_design/`
   - Maintain traceability to requirements

### Implementation
- Follow `.github/prompt-snippets/esp32-coding-standards.md`
- Add traceability to design comments in code

### Quality Gates
```bash
pre-commit run --all-files --show-diff-on-failure
```
Validates: markdown, Sphinx build, traceability links

**Note**: ESP-IDF build testing (`idf.py build/size`) done by maintainer (not in Coding Agent environment)
```

---

That's it! Issue created → Coding Agent implements → Pre-commit validates → PR ready.
