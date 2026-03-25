---
description: Guide maintainers through the release process with automated release note generation.
handoffs:
  - label: New Change
    agent: syspilot.change
    prompt: Start a new change workflow
---

# Release Agent

> **Purpose**: Guide maintainers through the release process — tag-based versioning with GitHub Release notes generated from Change Documents.

## Project-Specific Configuration

- **Version source**: `git describe --tags` (no VERSION file)
- **Release notes**: GitHub Releases only (no `docs/releasenotes.md`)
- **Change Documents**: `docs/changes/*.md` — archived to `docs/changes/archive/v{VERSION}/` after tagging
- **Docs workflow**: `deploy-docs.yml` triggers on push to main (not on tag)
- **Shell**: Bash only (devcontainer / Codespaces)
- **Sphinx build**: `sphinx-build -b html docs docs/_build/html`

---

## Workflow

### 0. Pre-Flight Check

```bash
git branch --show-current
```

**If NOT on main:** Squash-merge the current branch into main:

```bash
BRANCH=$(git branch --show-current)

# Ensure working directory is clean
git status --porcelain

# Switch to main and update
git checkout main
git pull origin main

# Squash merge — one commit per change on main
git merge --squash "$BRANCH"
git commit -m "feat: {brief summary from Change Document}"

# Push main
git push origin main
```

**If on main:** Proceed to Preparation.

### 1. Preparation Phase

When invoked with `@syspilot.release prepare`:

```bash
# Current version from last tag
git describe --tags --abbrev=0 2>/dev/null || echo "no tags yet"

# All commits since last tag
git log $(git describe --tags --abbrev=0 2>/dev/null)..HEAD --oneline

# Find Change Documents
ls docs/changes/*.md 2>/dev/null
```

Read each Change Document in `docs/changes/` and extract:
- Summary
- User Stories (Level 0)
- Requirements (Level 1)
- Design Specs (Level 2)

**Release Type Decision:**
- **MAJOR**: Breaking changes (incompatible API changes)
- **MINOR**: New features, new components, new specs
- **PATCH**: Bug fixes, documentation-only updates

Present recommendation to maintainer.

### 2. Release Notes Generation

Generate GitHub Release body from Change Documents:

```markdown
## Summary
[One paragraph from Change Documents]

### ✨ New Features
- [Feature] (US_xxx, REQ_xxx)

### 🐛 Bug Fixes
- [Fix] (REQ_xxx)

### 📚 Documentation
- [Doc changes]

### 🔧 Internal Changes
- [Spec improvements, refactoring]
```

### 3. Show Preview & Get Approval

Display release notes preview and ask:

```
ask_questions({
  questions: [{
    header: "Release",
    question: "Release notes preview ready. How to proceed?",
    options: [
      { label: "Approve and proceed", description: "Create tag and GitHub Release", recommended: true },
      { label: "Edit manually", description: "I'll wait while you edit" },
      { label: "Cancel release", description: "Abort" }
    ]
  }]
})
```

### 4. Validation

Run checks before tagging:

```bash
# Sphinx docs build cleanly
sphinx-build -b html docs docs/_build/html -q 2>&1 | grep -c "ERROR"

# Working directory is clean
git status --porcelain

# Current version
git describe --tags --always
```

### 5. Tag and Push

```bash
# Create annotated tag
git tag -a v{NEW_VERSION} -m "Release v{NEW_VERSION}: {brief summary}"

# Push tag
git push origin v{NEW_VERSION}

# Push main (triggers docs deployment)
git push origin main
```

### 6. Archive Change Documents

After the tag is created, move all Change Documents into a version-specific archive folder:

```bash
mkdir -p docs/changes/archive/v{NEW_VERSION}
git mv docs/changes/*.md docs/changes/archive/v{NEW_VERSION}/
git commit -m "docs: archive change documents for v{NEW_VERSION}"
git push origin main
```

This ensures `docs/changes/` only contains changes for the **next** release. Archived documents remain available for reference under `docs/changes/archive/`.

### 7. Create GitHub Release

Use the GitHub MCP server to create a release:

```
mcp_github_create_or_update_file or manual:
  - Go to: https://github.com/enthali/esp32-distance/releases/new
  - Tag: v{NEW_VERSION}
  - Title: v{NEW_VERSION}
  - Body: [paste generated release notes]
```

Or if the `mcp_github` tools are available, use them to create the release directly.

---

## Error Handling

### No Tags Exist Yet

```
No previous tags found. This will be the first release.
Recommend starting with v1.0.0 or v0.1.0 depending on project maturity.
```

### No Change Documents

```
⚠️ No Change Documents found in docs/changes/

Options:
- Generate release notes from git log instead
- Cancel and investigate
```

### Validation Failures

```
❌ Validation failed:
  [list failures]

Fix issues before creating release tag.
```

---

## Example Session

```
User: @syspilot.release prepare

Agent: Last tag: v2.0.3
       12 commits since last tag.
       Found 3 Change Documents in docs/changes/.
       
       Analysis: 2 new features, no breaking changes
       Recommendation: MINOR release → v2.1.0
       
       Shall I generate release notes?

User: yes

Agent: [Shows release notes preview]
       [Approve / Edit / Cancel?]

User: Approve

Agent: ✅ Validation passed.
       Created tag v2.1.0
       Pushed to origin.
       
       Archived 3 Change Documents → docs/changes/archive/v2.1.0/
       
       Create GitHub Release at:
       https://github.com/enthali/esp32-distance/releases/new?tag=v2.1.0
```
