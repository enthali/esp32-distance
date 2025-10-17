---
description: Analyze requirements document for MECE compliance (Mutually Exclusive, Collectively Exhaustive)
---

You are analyzing requirements documentation for the ESP32 Distance Sensor project.

Analyze the requirements in `${input:requirementsFile:${fileBasenameNoExtension}}` for MECE compliance:

## Check Mutual Exclusivity

For each requirement verify:

- No overlap with other requirements in scope
- No duplicate functionality across requirements
- Clear boundaries between related requirements

Report any violations: Which requirements overlap? What is the overlapping scope? How to fix?

## Check Collective Exhaustiveness

For the requirement area verify:

- All aspects of the feature/component are covered
- No gaps in functionality or behavior
- Edge cases are addressed

Report any gaps: What is missing? Which scenarios not covered? Suggest new requirements.

## Check Requirement Structure

Each requirement must have:

- Unique ID (REQ-AREA-NUMBER)
- Clear title
- Description
- Rationale
- Dependencies (if applicable)
- Testable acceptance criteria

## Output Format

Provide analysis as markdown with:

1. **Mutual Exclusivity Issues**: List overlapping requirements with recommendations
2. **Collective Exhaustiveness Gaps**: List missing requirements with suggestions
3. **Structural Issues**: List incomplete requirements
4. **Summary**: Counts and overall PASS/FAIL assessment

Create output file: `docs/requirements/analysis/${input:requirementsFile}-mece-analysis.md`
