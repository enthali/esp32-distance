# Validate Code Quality

**Purpose:** Analyze code quality, consistency with coding standards, and architectural patterns across the ESP32 project.

**Output:** `temp/code-quality-analysis.md`

---

## Scope

### Source Code

- `main/**/*.c` - C source files
- `main/**/*.h` - Header files
- `main/components/*/` - Component implementations
- `main/main.c` - Application entry point

### Configuration

- `main/components/*/CMakeLists.txt` - Component build configs
- `main/Kconfig.projbuild` - Configuration options

### Documentation in Code

- Doxygen comments
- Inline documentation
- REQ_* traceability comments

---

## Code Quality Checks

### 1. Naming Conventions

**Check against:** `.github/prompt-snippets/esp32-coding-standards.md`

**Functions:**
- ✅ `snake_case` format
- ✅ Component prefix (e.g., `config_manager_init()`)
- ❌ CamelCase or mixedCase

**Variables:**
- ✅ `snake_case` format
- ✅ Descriptive names
- ❌ Single letter variables (except loop counters)

**Constants/Macros:**
- ✅ `UPPER_SNAKE_CASE` format
- ✅ Clear naming
- ❌ Magic numbers without defines

**Structs/Enums:**
- ✅ `snake_case_t` format
- ✅ Type suffixes

**Report:**
- Files with inconsistent naming
- Functions without component prefixes
- Magic numbers that should be constants
- Unclear variable names

### 2. Component Architecture

**Check:**
- Component folder structure follows ESP-IDF conventions
- Header files in `include/` subdirectory
- Public APIs in headers, private in .c files
- CMakeLists.txt properly configured

**Expected structure:**
```text
components/my_component/
├── CMakeLists.txt
├── include/
│   └── my_component.h
├── my_component.c
└── README.md
```

**Report:**
- Components with incorrect structure
- Missing CMakeLists.txt or README.md
- Public functions not declared in headers
- Missing include directories

### 3. Documentation Quality

**Check:**
- Every function has Doxygen comment
- File-level Doxygen header present
- Complex algorithms have inline comments
- Hardware dependencies documented

**Required Doxygen elements:**
- `@brief` - Function purpose
- `@param` - Parameter descriptions
- `@return` - Return value description
- `@note` - Important notes (if applicable)

**Report:**
- Functions without Doxygen comments
- Files without header comments
- Missing parameter/return documentation
- Undocumented hardware dependencies (GPIO, timers, etc.)

### 4. Error Handling Patterns

**Check:**
- ESP-IDF error codes used correctly
- Return values checked
- `ESP_ERROR_CHECK()` used for critical operations
- Error messages include `esp_err_to_name()`

**Good patterns:**
```c
esp_err_t ret = esp_wifi_init(&config);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "WiFi init failed: %s", esp_err_to_name(ret));
    return ret;
}
```

**Bad patterns:**
```c
esp_wifi_init(&config);  // No error check
if (ret) { ... }  // Wrong check (should be != ESP_OK)
printf("Error\n");  // Not using ESP_LOG
```

**Report:**
- Functions ignoring return values
- Incorrect error checking patterns
- Missing error logging
- Use of printf instead of ESP_LOG

### 5. Logging Standards

**Check:**
- Static `TAG` defined at file level
- Appropriate log levels used
- Consistent log format
- No printf/fprintf in production code

**Log level usage:**
- `ESP_LOGI` - Normal operations, initialization
- `ESP_LOGW` - Warnings, recoverable errors
- `ESP_LOGE` - Critical failures
- `ESP_LOGD` - Debug information

**Report:**
- Files without TAG definition
- Incorrect log level usage
- printf/fprintf usage (should be ESP_LOG)
- Inconsistent log formatting

### 6. Memory Management

**Check:**
- Dynamic allocation checked for NULL
- Allocated memory freed properly
- No memory leaks in error paths
- Appropriate use of stack vs. heap

**Good patterns:**
```c
char* data = malloc(size);
if (data == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory");
    return ESP_ERR_NO_MEM;
}
// Use data...
free(data);
```

**Report:**
- Unchecked malloc/calloc calls
- Missing free() calls
- Memory leaks in error paths
- Excessive stack allocation (>1KB)

### 7. FreeRTOS Task Patterns

**Check:**
- Task priorities defined as constants
- Stack sizes reasonable and documented
- Task names descriptive
- Proper task creation error handling

**Expected patterns:**
```c
#define APP_TASK_PRIORITY 5
#define APP_TASK_STACK_SIZE 2048

xTaskCreate(app_task, "app_task", 
            APP_TASK_STACK_SIZE, NULL, 
            APP_TASK_PRIORITY, NULL);
```

**Report:**
- Magic numbers for priorities/stack sizes
- Unclear task names
- Unchecked xTaskCreate calls
- Inappropriate stack sizes

### 8. GPIO and Hardware Interface

**Check:**
- Pin assignments as named constants
- GPIO configuration properly initialized
- Pin conflicts documented
- Hardware dependencies in file header

**Good patterns:**
```c
#define STATUS_LED_PIN GPIO_NUM_2

gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << STATUS_LED_PIN),
    .mode = GPIO_MODE_OUTPUT,
};
ESP_ERROR_CHECK(gpio_config(&io_conf));
```

**Report:**
- Hardcoded pin numbers
- Missing GPIO configuration
- Undocumented hardware dependencies
- Potential pin conflicts

### 9. Header File Quality

**Check:**
- Include guards present (`#ifndef` / `#define` / `#endif`)
- C++ compatibility (`extern "C"` blocks)
- Minimal includes (only what's needed)
- Clean public API separation

**Expected pattern:**
```c
#ifndef MY_COMPONENT_H
#define MY_COMPONENT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declarations...

#ifdef __cplusplus
}
#endif

#endif // MY_COMPONENT_H
```

**Report:**
- Missing include guards
- Missing C++ compatibility
- Excessive includes
- Private functions in public headers

### 10. Code Complexity

**Check:**
- Function length (prefer <50 lines)
- Cyclomatic complexity (prefer <10)
- Deep nesting (avoid >3 levels)
- Duplicated code patterns

**Report:**
- Overly long functions
- Functions with high complexity
- Deeply nested if/for/while blocks
- Code duplication opportunities

### 11. ESP-IDF Best Practices

**Check:**
- `IRAM_ATTR` only on ISRs
- WiFi event handlers properly registered
- `const` used for read-only data
- Kconfig used for configurable parameters

**Good patterns:**
```c
IRAM_ATTR void gpio_isr_handler(void* arg) {
    // Minimal ISR code
}

#ifdef CONFIG_APP_MAX_RETRIES
#define MAX_RETRIES CONFIG_APP_MAX_RETRIES
#else
#define MAX_RETRIES 3
#endif
```

**Report:**
- Misuse of IRAM_ATTR
- Missing const qualifiers
- Hardcoded values that should be Kconfig
- Incorrect WiFi event handling

### 12. Component Dependencies

**Check CMakeLists.txt:**
- REQUIRES vs. PRIV_REQUIRES correctly used
- Dependencies minimal and justified
- Include directories properly specified
- Source files correctly listed

**Expected pattern:**
```cmake
idf_component_register(
    SRCS "config_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_http_server
    PRIV_REQUIRES esp_timer
)
```

**Report:**
- Over-dependencies (too many REQUIRES)
- Public dependencies in PRIV_REQUIRES
- Missing dependencies
- Incorrect include directory specs

---

## Analysis Process

1. **Scan all source files** - Read .c and .h files in scope
2. **Check naming conventions** - Validate against coding standards
3. **Analyze structure** - Verify component organization
4. **Parse documentation** - Check Doxygen completeness
5. **Validate patterns** - Check error handling, logging, memory
6. **Assess complexity** - Measure function length and nesting
7. **Review dependencies** - Check CMakeLists.txt configs
8. **Prioritize findings** - Categorize by severity and impact

---

## Output Format

Write analysis to `temp/code-quality-analysis.md`:

```markdown
# Code Quality Analysis

**Date:** YYYY-MM-DD
**Branch:** <current-branch>
**Files Analyzed:** X source files, Y header files

---

## 📊 Quality Metrics Summary

| Category | Pass | Warn | Fail | Score |
|----------|------|------|------|-------|
| Naming Conventions | X | X | X | X% |
| Component Architecture | X | X | X | X% |
| Documentation | X | X | X | X% |
| Error Handling | X | X | X | X% |
| Logging Standards | X | X | X | X% |
| Memory Management | X | X | X | X% |
| Header Files | X | X | X | X% |
| Code Complexity | X | X | X | X% |
| ESP-IDF Best Practices | X | X | X | X% |
| **Overall** | **X** | **X** | **X** | **X%** |

---

## ✅ Well-Written Components

Components following all standards:
- `config_manager` - Excellent documentation, clean error handling
- `web_server` - Proper component structure, good logging

---

## ⚠️ Code Quality Issues

### Critical (Must Fix - Breaks standards or could cause bugs)

**1. Missing Error Handling**
- **File:** `main/components/xyz/xyz.c:45`
- **Issue:** `malloc()` result not checked for NULL
- **Code:**
  ```c
  char* buffer = malloc(1024);
  strcpy(buffer, data);  // Could crash if malloc failed
  ```
- **Impact:** Potential crash/undefined behavior
- **Fix:** Add NULL check and return ESP_ERR_NO_MEM

**2. Undocumented Public API**
- **File:** `main/components/abc/include/abc.h`
- **Issue:** Function `abc_init()` has no Doxygen comment
- **Impact:** API unclear to users
- **Fix:** Add complete Doxygen documentation

### High (Should Fix - Standard violations)

**3. Incorrect Naming Convention**
- **File:** `main/components/xyz/xyz.c`
- **Issue:** Function `XyzStart()` uses CamelCase instead of snake_case
- **Expected:** `xyz_start()`
- **Fix:** Rename to follow snake_case convention

**4. Missing Include Guards**
- **File:** `main/components/abc/include/abc_internal.h`
- **Issue:** No include guards present
- **Impact:** Could cause multiple definition errors
- **Fix:** Add `#ifndef ABC_INTERNAL_H` guards

### Medium (Should Fix - Best practice violations)

**5. Magic Numbers**
- **File:** `main/components/xyz/xyz.c:123`
- **Issue:** Hardcoded `5000` (timeout value)
- **Code:** `vTaskDelay(5000 / portTICK_PERIOD_MS);`
- **Fix:** Define as `#define TIMEOUT_MS 5000`

**6. printf Usage**
- **File:** `main/components/abc/abc.c:67`
- **Issue:** Using `printf()` instead of ESP_LOG
- **Fix:** Replace with `ESP_LOGI(TAG, ...)`

### Low (Nice to Fix - Minor improvements)

**7. Function Complexity**
- **File:** `main/components/xyz/xyz.c`
- **Function:** `xyz_process()`
- **Lines:** 85 lines
- **Recommendation:** Consider splitting into smaller functions

**8. Code Duplication**
- **Files:** `config_manager.c` and `web_server.c`
- **Issue:** Similar JSON parsing code in both files
- **Recommendation:** Extract common utility function

---

## 📁 Component-by-Component Analysis

### config_manager
- **Structure:** ✅ Correct
- **Documentation:** ✅ Complete
- **Error Handling:** ✅ Good
- **Logging:** ✅ Consistent
- **Issues:** None
- **Score:** 95%

### web_server
- **Structure:** ✅ Correct
- **Documentation:** ⚠️ Missing some param docs
- **Error Handling:** ✅ Good
- **Logging:** ✅ Consistent
- **Issues:** 2 minor (missing Doxygen details)
- **Score:** 88%

### cert_handler
- **Structure:** ❌ Missing README.md
- **Documentation:** ⚠️ Incomplete
- **Error Handling:** ✅ Good
- **Logging:** ✅ Consistent
- **Issues:** 3 medium (structure, docs)
- **Score:** 72%

### netif_uart_tunnel
- **Structure:** ✅ Correct
- **Documentation:** ✅ Complete
- **Error Handling:** ⚠️ Some unchecked returns
- **Logging:** ✅ Consistent
- **Issues:** 1 high (error handling)
- **Score:** 82%

---

## 🔍 Detailed Findings by Category

### Naming Conventions
- ✅ **Pass:** 95% of functions follow snake_case
- ⚠️ **Warn:** 3 functions use CamelCase (list below)
- ❌ **Fail:** 2 magic numbers found

**Functions to rename:**
1. `main/components/xyz/xyz.c:45` - `XyzStart()` → `xyz_start()`
2. `main/components/abc/abc.c:123` - `AbcProcess()` → `abc_process()`

### Documentation Quality
- ✅ **Pass:** 80% of functions have Doxygen
- ⚠️ **Warn:** 15% have incomplete Doxygen
- ❌ **Fail:** 5% have no Doxygen

**Missing documentation:**
1. `main/components/xyz/xyz.c:78` - `xyz_cleanup()`
2. `main/components/abc/abc.c:156` - `abc_validate()`

### Error Handling
- ✅ **Pass:** 90% of ESP calls checked
- ⚠️ **Warn:** 8% use printf instead of ESP_LOG
- ❌ **Fail:** 2% don't check malloc returns

**Critical issues:**
1. `main/components/xyz/xyz.c:45` - Unchecked malloc
2. `main/components/abc/abc.c:89` - Ignored return value

### Memory Management
- ✅ **Pass:** No obvious leaks detected
- ⚠️ **Warn:** 2 functions with complex cleanup paths
- ❌ **Fail:** 1 potential leak in error path

**Potential leak:**
1. `main/components/xyz/xyz.c:123` - Buffer not freed on early return

---

## 📈 Recommendations

### Immediate Actions (Critical/High)

1. [ ] Fix unchecked malloc in `xyz.c:45`
2. [ ] Add missing include guards to `abc_internal.h`
3. [ ] Document public APIs in `abc.h`
4. [ ] Fix memory leak in `xyz.c:123`
5. [ ] Rename CamelCase functions to snake_case

### Short-term (Medium Priority)

1. [ ] Replace printf with ESP_LOG macros
2. [ ] Define constants for magic numbers
3. [ ] Add missing component READMEs
4. [ ] Complete incomplete Doxygen comments

### Long-term (Improvements)

1. [ ] Refactor long functions (>50 lines)
2. [ ] Extract duplicated code into utilities
3. [ ] Add static analysis to pre-commit hooks
4. [ ] Create coding standards checklist

---

## 🎯 Quality Score

**Overall Score:** X/100

- **Excellent (90-100):** config_manager
- **Good (80-89):** web_server, netif_uart_tunnel
- **Needs Improvement (70-79):** cert_handler
- **Poor (<70):** None

**Target:** All components 80%+ before release

---

## 📋 Action Items by Component

### config_manager
- No actions needed - exemplary code quality

### web_server
- [ ] Complete parameter documentation in 2 functions
- [ ] Define timeout constant instead of magic number

### cert_handler
- [ ] Add component README.md
- [ ] Complete Doxygen documentation
- [ ] Fix component structure

### netif_uart_tunnel
- [ ] Add error checking for 3 function calls
- [ ] Add cleanup validation in error paths

---

## 🔧 Tools Suggestions

Consider adding to development workflow:
- **cppcheck** - Static analysis for C/C++
- **clang-format** - Automatic code formatting
- **lizard** - Cyclomatic complexity analyzer
- **pre-commit hook** - Enforce standards before commit
```

## Notes

- **Baseline Standards:** `.github/prompt-snippets/esp32-coding-standards.md`
- **Focus Areas:**
  - Template maintainability (others will fork this)
  - ESP-IDF best practices
  - Beginner-friendly code (learning resource)
  - Production readiness
  
- **Exclusions:**
  - Generated code (build artifacts)
  - Third-party code (ESP-IDF components)
  - Example code marked as "example only"
  
- **Quality Targets:**
  - New components: 90%+ score
  - Existing components: 80%+ score
  - Critical components (config, web): 95%+ score

- **Integration:**
  - Can be automated via GitHub Actions
  - Results can feed into PR review process
  - Metrics can track quality over time
