# Analyze API Documentation Quality

## Quick Usage

```text
Analyze API documentation for: [HEADER_FILE_PATH]
Output analysis to: temp/[module_name]_doc_analysis.md
```

## Purpose

Analyze a C/C++ header file to assess documentation quality and readiness for API doc generation. Generate a detailed analysis report with findings, metrics, and actionable recommendations.

## Analysis Checklist

Evaluate these aspects of the header file:

### File-Level Documentation
- [ ] File purpose and overview
- [ ] Architecture/design notes
- [ ] Key features or usage patterns
- [ ] Author and version information
- [ ] Requirement traceability (if applicable)

### Function Documentation
- [ ] Every public function has documentation
- [ ] Purpose clearly described
- [ ] All parameters documented
- [ ] Return values documented with possible error codes
- [ ] Thread-safety notes (if applicable)
- [ ] Usage constraints or preconditions

### Data Types Documentation
- [ ] Structs/classes have purpose descriptions
- [ ] All fields/members documented
- [ ] Enums have value descriptions
- [ ] Typedefs explain abstractions

### Quality Indicators
- [ ] Consistent documentation style
- [ ] Code examples provided
- [ ] Cross-references between related items
- [ ] Requirement/design traceability

## Report Structure

Generate a Markdown report with these sections:

### 1. Executive Summary
- Header file path and analysis date
- Overall status (PASS/NEEDS IMPROVEMENT/FAIL)
- Overall score (0-100)
- Top 3 key findings (strengths and issues)

### 2. File-Level Documentation
- Status assessment
- Present elements (what's documented well)
- Missing/weak elements (what needs improvement)
- Traceability status (if applicable)

### 3. Function Documentation Analysis
- Statistics: total functions, coverage percentages
- Compliance matrix table with columns:
  - Function name
  - Purpose (✅/⚠️/❌)
  - Parameters (✅/⚠️/❌)
  - Returns (✅/⚠️/❌)
  - Thread-safety (✅/⚠️/❌)
  - Traceability (✅/⚠️/❌)
  - Overall status
- Detailed findings grouped by quality level

### 4. Data Types Documentation
- Statistics for structs/classes/enums
- Analysis of each major data type
- Documentation completeness assessment

### 5. Code Examples & Usage Patterns
- Status (Present/Limited/Missing)
- What examples exist
- Recommendations for additional examples

### 6. Documentation Quality Metrics
- Table with metrics, scores, targets, and status
- Key metrics:
  - File-level completeness
  - Function coverage
  - Parameter documentation
  - Return value documentation
  - Thread-safety notes
  - Requirements traceability
  - Code examples
- Overall score calculation

### 7. Prioritized Recommendations
- **High Priority**: Critical issues blocking API doc generation
- **Medium Priority**: Quality improvements recommended
- **Low Priority**: Nice-to-have enhancements

### 8. Readiness Assessment
- Can generate docs now? (YES/WITH ISSUES/NOT READY)
- Specific blockers (if any)
- Warnings about incomplete documentation
- Overall recommendation

### 9. Next Steps
- Checklist of actions to take
- Re-validation steps
- Doc generation readiness

## Scoring Guidelines

**Score Calculation (Total: 100 points):**
- File-level completeness: 20 points
- Function documentation: 40 points
- Data types documentation: 15 points
- Requirements traceability: 15 points
- Code examples: 10 points

**Status Thresholds:**
- **90-100**: ✅ EXCELLENT - Ready for doc generation
- **75-89**: ⚠️ GOOD - Minor improvements recommended
- **60-74**: ⚠️ NEEDS WORK - Several issues to address
- **Below 60**: ❌ POOR - Major documentation gaps

**Minimum for PASS:**
- File-level overview present (90%+)
- 90%+ functions have purpose descriptions
- 90%+ parameters documented
- 80%+ return values documented
- 50%+ functions have traceability (if applicable)

## Output Location

Save report to: `temp/[module_name]_doc_analysis.md`

Example: `temp/config_manager_doc_analysis.md`

## Notes

- Analysis checks **source code documentation**, not generated docs
- Use before running doc generation to ensure quality input
- Re-run after improvements to verify fixes
- Report is for human review and decision-making
