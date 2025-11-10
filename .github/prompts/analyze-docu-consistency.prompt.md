# Maintain Project Consistency

**Purpose:** Analyze and report inconsistencies across project infrastructure, documentation, and configuration files.

**Output:** `temp/docu-consistency-analysis.md`

---

## Scope

### Infrastructure Files

- `.github/workflows/*.yml` - CI/CD pipelines
- `.github/actions/*/action.yml` - Custom GitHub Actions
- `.github/copilot-instructions.md` - Copilot context
- `.github/prompt-snippets/*.md` - Development prompts
- `.devcontainer/Dockerfile` - Container setup
- `.devcontainer/devcontainer.json` - Dev container config
- `.vscode/tasks.json` - VS Code tasks
- `.vscode/launch.json` - Debug configurations

### Documentation Files

- `README.md` (root) - Main project documentation
- `docs/**/*.rst` - Sphinx documentation
- `tools/README.md` - Tool documentation
- `main/components/*/README.md` - Component documentation

### Configuration Files

- `CMakeLists.txt` - Build configuration
- `sdkconfig` - ESP-IDF configuration (key settings only)
- `main/components/*/CMakeLists.txt` - Component configs

---

## Consistency Checks

### 1. Tool Versions & Dependencies

**Check:**
- ESP-IDF version across: Dockerfile, workflows, README, copilot-instructions
- Python version: Dockerfile, workflows, requirements
- Node.js version: Dockerfile, workflows
- Tool versions: markdownlint, sphinx, pre-commit

**Report inconsistencies:**
- Which files have mismatched versions?
- What is the "correct" version (use Dockerfile as source of truth)?

### 2. Workflow vs. Devcontainer Alignment

**Check:**
- Do workflows use same base image/tools as devcontainer?
- Are pre-commit hooks identical in `.pre-commit-config.yaml` and CI?
- Do both environments install the same documentation tools?

**Report inconsistencies:**
- Environment differences that could cause "works locally, fails in CI"
- Missing tools in one environment vs. the other

### 3. VS Code Tasks vs. Actual Scripts

**Check:**
- Do tasks in `.vscode/tasks.json` reference existing scripts?
- Are script paths correct after reorganization?
- Are task descriptions accurate?

**Report inconsistencies:**
- Tasks pointing to non-existent scripts
- Outdated task labels or descriptions
- Missing tasks for new scripts in `tools/`

### 4. Documentation vs. Reality

**Check:**
- Does `README.md` mention features that don't exist?
- Does `README.md` list components that have been removed?
- **Is `README.md` an "elevator pitch" (< 100 lines) or does it duplicate Sphinx docs?**
- **Does `README.md` link prominently to full documentation instead of replicating content?**
- Is `tools/README.md` up-to-date with reorganized structure?
- Do component READMEs match actual component functionality?
- Are build instructions in `README.md` consistent with `build-instructions.md`?
- **Do all major tools in `tools/` have corresponding guides in `docs/90_guides/`?**
- **Are tool guides linked from the main guides index?**

**Report inconsistencies:**
- Features mentioned but not implemented
- Outdated examples or commands
- Missing documentation for new features/tools
- **README.md contains detailed guides that should live in Sphinx docs**
- **README.md lacks prominent link to full documentation**
- **Tools without documentation guides (e.g., `tools/qemu/`, `tools/web-flasher/`, `tools/docu/`)**
- **Tool guides not included in `docs/90_guides/index.rst` toctree**

### 5. Prompt Snippets Internal Consistency

**Check:**
- Do snippets contradict each other?
- Are branch naming conventions consistent?
- Are commit scopes aligned?
- Do code examples reference current project structure?
- Are ESP-IDF version references consistent?

**Report inconsistencies:**
- Conflicting information between snippets
- Outdated examples or references
- Duplicate information that should be centralized

### 6. GitHub Actions References

**Check:**
- Do custom actions reference existing scripts?
- Are action descriptions accurate?
- Do workflows reference correct action paths?
- Are action inputs/outputs documented and used correctly?

**Report inconsistencies:**
- Actions referencing moved/deleted scripts
- Workflows using outdated action parameters
- Missing or incorrect action documentation

### 7. Copilot Instructions Accuracy

**Check:**
- Does `.github/copilot-instructions.md` reflect current:
  - Project structure
  - Component organization
  - Tool locations
  - Development workflow
  - Known limitations
- Are template features accurately described?
- Are known issues (like HTTPS/debugger) documented?

**Report inconsistencies:**
- Outdated project structure descriptions
- Missing information about recent changes
- Incorrect tool/script references

---

## Analysis Process

1. **Read all files in scope** - Gather current state
2. **Compare versions** - Check tool/dependency versions
3. **Validate references** - Ensure paths/scripts exist
4. **Check semantics** - Verify descriptions match reality
5. **Cross-reference** - Find contradictions between documents
6. **Prioritize findings** - Categorize by severity

---

## Output Format

Write analysis to `temp/docu-consistency-analysis.md`:

```markdown
# Project Consistency Analysis

**Date:** YYYY-MM-DD
**Branch:** <current-branch>

---

## ✅ Consistent Areas

List areas that are properly synchronized:
- Tool versions aligned across Dockerfile and workflows
- VS Code tasks reference correct scripts
- etc.

---

## ⚠️ Inconsistencies Found

### Critical (Breaks functionality or CI)

**1. [Title]**
- **Files:** `path/to/file1`, `path/to/file2`
- **Issue:** Detailed description of the inconsistency
- **Impact:** How this affects users/developers/CI
- **Recommendation:** Specific action to fix

### Medium (Causes confusion or incorrect documentation)

**2. [Title]**
- **Files:** `path/to/file`
- **Issue:** Description
- **Impact:** Impact description
- **Recommendation:** Action to fix

### Low (Minor inconsistencies, typos, or improvements)

**3. [Title]**
- **Files:** `path/to/file`
- **Issue:** Description
- **Impact:** Impact description
- **Recommendation:** Action to fix

---

## 📋 Summary

- **Total inconsistencies:** X
- **Critical:** X
- **Medium:** X
- **Low:** X

---

## 🔧 Recommended Actions

Prioritized list of fixes:

1. [ ] **Critical:** Update workflow Node version to v18
2. [ ] **Critical:** Fix VS Code task pointing to old script path
3. [ ] **Medium:** Update README to reflect reorganized tools structure
4. [ ] **Low:** Fix typo in component README
```

## Notes

- **Source of truth priorities:**
  1. Dockerfile (for tool versions)
  2. Actual code/scripts (for functionality)
  3. Root README (for user-facing features)
  
- **README.md philosophy:**
  - **Keep it minimal** - "Elevator pitch" style (target < 100 lines)
  - **Link, don't duplicate** - Point to Sphinx docs for details
  - **Quick start only** - Basic usage, not comprehensive guides
  - **No detailed instructions** - QEMU, debugging, customization belong in docs
  
- **Exclusions:**
  - Code quality (covered by pre-commit)
  - Requirements traceability (separate prompt)
  - Generated files (build/, docs/_build/)

- **Focus on:**
  - User-facing documentation accuracy
  - Developer environment consistency
  - CI/CD reliability
