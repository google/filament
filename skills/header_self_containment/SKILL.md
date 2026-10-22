---
name: header-self-containment
description: >
  Enforce strict header self-containment so that every header file compiles independently.
  Use this skill when creating or modifying C++ header files.
---

# Strict Header Self-Containment

Every header file (`.h`) in Filament **must be fully self-contained** and compile independently.

*   **Rule:** Never assume a header's type dependencies (like `FrameGraphHandle`) are pre-declared by preceding includes in a `.cpp` file.
*   **Action:** Always explicitly `#include` the declaring header (e.g., `fg/FrameGraphId.h`) inside the header itself.
