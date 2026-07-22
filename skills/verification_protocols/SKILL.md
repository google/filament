---
name: verification-protocols
description: >
  Standard verification pipeline to execute after modifying C++ source or header files.
  Use this skill to format includes, build the engine, and run core tests.
---

# Verification Protocols

Before completing any task that modifies C++ source or header files, AI agents **must** execute the following verification pipeline:

1.  **Format Includes:** Run `./tools/reorganize_headers/run.py <target>`.
2.  **Compile Core Engine:** Run `./build.sh -ip desktop debug` to ensure the entire desktop target compiles cleanly with no warnings or errors.
3.  **Run Tests:** Run the test suite binary located in the build output directory:
    ```bash
    ./out/cmake-debug/libs/utils/test_utils
    ```
4.  **Language Bindings Check:** If any public APIs under `filament/include/filament/` were modified, verify that matching Java and JavaScript/TypeScript bindings are implemented and up-to-date according to [bindings-synchronization](../bindings_synchronization/SKILL.md).
