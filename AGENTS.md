# Filament AI Coding & Agent Guidelines

This document defines machine-readable instructions and technical guardrails for AI coding assistants, agents, and LLMs working on the Filament repository. All AI-generated code, refactorings, and cleanups must strictly adhere to the standardized skills located under the `skills/` directory.

---

## Core AI Skills & Guidelines

AI agents **must** consult and strictly follow the corresponding skill files for each respective development and validation phase:

### 1. C++ Code & Header Integrity
*   [skills/cpp_header_inclusion/SKILL.md](skills/cpp_header_inclusion/SKILL.md): Dynamic C++ Header Inclusion layering, logical layering hierarchy, and reordering rules.
*   [skills/header_self_containment/SKILL.md](skills/header_self_containment/SKILL.md): Strict Header Self-Containment requirement to ensure every header compiles independently.
*   [skills/preprocessor_guard_hygiene/SKILL.md](skills/preprocessor_guard_hygiene/SKILL.md): Preprocessor guard alphabetizing and macro-dependent template hygiene.
*   [skills/cpp_static_thread_safety/SKILL.md](skills/cpp_static_thread_safety/SKILL.md): Thread capability annotations (UTILS_GUARDED_BY, UTILS_REQUIRES), LockGuard invariants, and lambda CV bypass rules.

### 2. Workspace Operations, Verification, and Builds
*   [skills/verification_protocols/SKILL.md](skills/verification_protocols/SKILL.md): Mandatory verification pipeline (include formatting, desktop compilation, core test runs) before concluding a C++ task.
*   [skills/filament_build_clean/SKILL.md](skills/filament_build_clean/SKILL.md): Clean commands, debug desktop compilation, and release desktop compilation protocols.
*   [skills/filament_desktop_testing/SKILL.md](skills/filament_desktop_testing/SKILL.md): Instructions for executing and filtering tests and benchmarks on desktop.
*   [skills/filament_android_development/SKILL.md](skills/filament_android_development/SKILL.md): Android development, compilation with Perfetto, binary deployment, shell invocation, and formatted benchmarking.
