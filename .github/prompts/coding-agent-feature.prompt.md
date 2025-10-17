---
description: Generate GitHub Issue for Coding Agent from feature description
---

Generate a complete GitHub Issue for the ESP32 Distance Sensor project that can be assigned to the GitHub Copilot Coding Agent.

## Feature Request

${input:featureRequest:Describe the feature you want to implement}

---

## Your Task

Analyze the feature request above and:

1. **Search the codebase** to identify affected components (files/modules)
2. **Search docs/requirements/** for related REQ-* requirements
3. **Determine priority** based on feature type:
   - Core functionality (sensor, LED, WiFi) = High
   - User experience improvements = Medium  
   - Nice-to-have features = Low
4. **Extract clear objective** - what needs to be done and why
5. **Infer current behavior** from codebase context
6. **Generate complete GitHub Issue** with all mandatory workflow steps

---

## Generate Issue

Create a GitHub Issue with this exact structure:

### Issue Title

```text
[Feature]: <Generate clear, concise title from feature request> #github-pull-request_copilot-coding-agent
```

### Issue Body

```markdown
## 🎯 Objective

<Rewrite feature request as clear objective - what needs to be done and why>

## 📋 Context

- **Related Requirements**: <List REQ-* IDs found in docs/requirements/, or state "Create REQ-<AREA>-N">
- **Affected Components**: <List files/modules identified from codebase search>
- **Current Behavior**: <Describe current behavior based on code analysis>
- **Desired Behavior**: <Extract from feature request>

## 🔧 Technical Details

<List technical constraints, dependencies, ESP32-specific considerations from your analysis>

---

## MANDATORY WORKFLOW - Follow in Order

### Step 1: Requirements Analysis & Update

1. Read existing requirements in \`docs/requirements/\`
2. Create or update requirements following OpenFastTrack:
   - Format: \`REQ-<AREA>-<NUMBER>\` (e.g., \`REQ-LED-5\`)
   - Include: ID, Title, Description, Rationale, Dependencies, Acceptance Criteria
   - Maintain bidirectional traceability
3. Update \`docs/requirements/<area>-requirements.md\`

### Step 2: Design Documentation

1. Review affected design documents in \`docs/design/\`
2. Document design decisions in code comments with traceability:
   \`\`\`c
   /**
    * @brief Function description
    * 
    * DESIGN TRACEABILITY:
    * - DSN-<AREA>-<ID>: Design decision reference
    * 
    * REQUIREMENTS TRACEABILITY:
    * - REQ-<AREA>-<ID>: Requirement reference
    */
   \`\`\`
3. Update \`docs/design/<component>-design.md\` if architectural changes

### Step 3: Implementation

Follow ESP32 coding standards from \`.github/prompt-snippets/esp32-coding-standards.md\`:

- Use snake_case for functions/variables
- Include proper error handling with ESP_ERROR_CHECK
- Add ESP_LOG statements for debugging
- Document functions with Doxygen comments
- Add traceability references in code

### Step 4: Quality Gates ⚠️ **CRITICAL - RUN BEFORE COMMIT**

**YOU MUST RUN THESE COMMANDS BEFORE COMMITTING:**

\`\`\`bash
# 1. Install tools (available in GitHub Actions environment)
pip install pre-commit mkdocs mkdocs-material markdownlint-cli

# 2. For documentation changes - auto-fix errors
markdownlint --fix docs/**/*.md *.md

# 3. Verify documentation builds
mkdocs build --strict

# 4. Run pre-commit hooks
pre-commit run --all-files --show-diff-on-failure

# 5. ONLY commit if ALL checks pass!
\`\`\`

**Why this is critical:**
- CI runs AFTER you commit (too late to fix)
- You will be offline when CI reports errors
- Human maintainer must manually fix your mistakes

### Step 5: Testing

1. Build test: \`idf.py build\`
2. Memory check: \`idf.py size\`
3. Functional testing: Verify feature works as expected
4. Document test results in commit message

### Step 6: Commit with Traceability

Follow commit format from \`.github/prompt-snippets/commit-message.md\`:

\`\`\`
<type>(<scope>): <subject>

<body>

DESIGN TRACEABILITY: DSN-<AREA>-<ID>
REQUIREMENTS: REQ-<AREA>-<ID>
Closes #<issue-number>
\`\`\`

**Types**: feat, fix, docs, refactor, test, chore, perf, security
**Scopes**: sensor, led, wifi, web, build, memory, https, component, requirements, design

---

## ✅ Success Criteria

Before submitting PR verify:

- [ ] Requirements documented and traceable
- [ ] Design documentation updated
- [ ] Code follows ESP32 coding standards
- [ ] All quality gates passed (markdownlint, mkdocs, pre-commit)
- [ ] Build succeeds (\`idf.py build\`)
- [ ] Memory usage acceptable (\`idf.py size\`)
- [ ] Commit messages follow project format
- [ ] Traceability maintained (REQ → DSN → Implementation)
```

---

## Output Instructions

After generating the issue:

1. **Present the complete issue** (title + body) in a code block for easy copying
2. **Provide direct link** to create issue: `https://github.com/enthali/esp32-distance/issues/new`
3. **Explain your analysis**:
   - Which requirements you found (or why new ones are needed)
   - Which components you identified as affected
   - Priority level and reasoning
   - Any technical considerations discovered

The user will copy-paste the generated issue to GitHub where it will be automatically assigned to the Coding Agent via the hashtag.

- ✅ Traceability maintained (REQ → DSN → Implementation)
