---
name: preprocessor-guard-hygiene
description: >
  Enforce preprocessor guard rules and macro-dependent inclusion hygiene in C++.
  Use this skill when modifying conditional imports or preprocessor directives.
---

# Preprocessor Guard Hygiene

*   **Simple Guarded Includes:** Headers wrapped inside single-line preprocessor guards (like `#if __EXCEPTIONS` or `#if defined(__ARM_NEON)`) must remain wrapped and are sorted alphabetically within their correct layer.
*   **Macro-Dependent Templating:** Never modify or split includes that are tightly coupled with macro configurations (such as `#define UTILS_PRIVATE_IMPLEMENTATION_NON_COPYABLE` in `ostream.cpp`). The `reorganize_headers.py` safety check will automatically detect and skip these; respect these skips.
