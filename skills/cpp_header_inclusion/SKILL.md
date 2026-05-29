---
name: cpp-header-inclusion
description: >
  Enforce the strict topological header inclusion layering and reordering rules in Filament.
  Use this skill when adding or modifying `#include` directives in C++ source or header files.
---

# Dynamic C++ Header Inclusion & Reordering

Filament enforces a strict, acyclic include layering hierarchy to optimize compile times, prevent header pollution, and identify missing dependencies.

## 1. Logical Layer Hierarchy

Includes must be grouped and separated by **exactly one blank line** in the following order (top to bottom):

1.  **Layer 0 (Corresponding Header):** `#include "details/Foo.h"` (isolated at the very top).
2.  **Layer 0.5 (Windows Cleanup):** `#include <utils/unwindows.h>` (always second, only if needed).
3.  **Layer 1 (Private/Local Headers):** `#include "PrivateStuff.h"` (using double quotes `" "` strictly for private local headers located under the local target's `src/` folder).
4.  **Layer 2 (Internal Proprietary Libraries):** Public and private includes for other internal libraries (e.g. `<filaflat/...>`, `<backend/...>`, `<utils/...>`, `<math/...>`). All proprietary library includes **must** use `< >` syntax (including `private/` sub-folders). The layering priority order of these libraries is dynamically computed based on `CMakeLists.txt` target linkages.
5.  **Layer 3 (Third-Party Libraries):** `<tsl/...>`, `<absl/...>`, `<jni.h>`.
6.  **Layer 4 (C++ Standard Library):** `<algorithm>`, `<vector>`, `<cstddef>` (including standard `<c...>` wrappers).
7.  **Layer 5 (POSIX System Headers):** `<unistd.h>`, `<sys/stat.h>` (C system headers below C++ std, but above C std).
8.  **Layer 6 (C Standard Library):** Legacy C system headers ending in `.h` (e.g., `<stdint.h>`, `<stddef.h>`).

---

## 2. Sorting & Spacing Rules for Private Includes (Layer 1)

*   **Flat-First:** Flat/top-level includes (without a subdirectory prefix, e.g., `"Allocators.h"`) are sorted **above** nested subdirectory includes (e.g., `"components/Foo.h"`).
*   **Visual Grouping:** Private includes must be grouped by their first subdirectory path prefix, separated by **exactly one blank line**.

---

## 3. Running the Automated Include Formatter

AI agents **must never manually sort includes** or guess target dependencies. Always run the repository's dynamic topological include formatter after modifying C++ files:

```bash
./tools/reorganize_headers.py <file_or_directory>
```

*Note: The tool dynamically parses `CMakeLists.txt` files to topologically sort proprietary libraries and includes preprocessor safety checks to avoid breaking platform-conditional include guards.*
