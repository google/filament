# Filament Code style and Formatting

Filament largely uses Android's code style, which is significantly different from the
Google code style and is derived from the Java code style, but not quite.

The guiding principles of the filament code style and code formatting can be resumed as:
- no nonsense
- use your own judgement
- break the rules **if it makes sense** e.g.: it improves readability substantially
- use the formatting of the file you're in, even if it breaks the rules
- no nonsense

## Formatting

- 4 spaces indent
- 8 spaces continuation indent
- 100 columns
- `{` at the end of the line
- spaces around operators and after `;`
- class access modifiers are not indented
- last line of `.cpp` or `.h` file must be an empty line
- there should be no trailing white spaces on any line

```c++
for (int i = 0; i < max; i++) {
}

class Foo {
public:
protected:
private:
};

```

## Naming Conventions

### Files

- headers use the `.h` extension
- implementation files use the `.cpp` extension
- included files use the `.inc` extension
- class files bear the name of the class they implement
- **no spaces** in file names
- file names must be treated as case **insensitive**, i.e. it is not allowed to have several files
  with the same name but a different case
- `#include` must use **fully qualified** names
- use `#include < >` for all public (exported) headers
- use `#include " "` for private headers
- all *public* include files must reside under the `include` folder
- all *source* files must reside under the `src` folder
- tests reside under the `test` folder
- public headers of a `foo` library must live in a folder named `foo`

```
libfoo.so

include/foo/FooBar.h
src/FooBar.cpp
src/data.inc

#include <foo/FooBar.h>
#include "FooBarPrivate.h"
```

### Code

- Everything is camel case except constants
- `constants` are uppercase and don't have a prefix
- `global` variables prefixed with `g`
- `static` variables prefixed with `s`
- `private` and `protected` class attributes prefixed with `m`
- `static` class attributes prefixed with `s`
- `public` class attributes *are not* prefixed
- class attributes and methods are lower camelcase

```c++
extern int gGlobalWarming;

class FooBar {
public:
    FooBar(int attributeName, int sizeInBytes)
            : mAttributeName(attributeName),
              sizeInBytes(sizeInBytes) {}

    void reallyLongMethodNameWithLotsOfArguments(bool argument1,
            int someSecondArgument, int bestArgument) {
        std::pair<bool, int> pair = {
            argument1,
            argument2,
        };
        // etc
    }

    int sizeInBytes;
private:
    int mAttributeName;
    static int sGlobalAttribute;
    static constexpr int FOO_COUNT = 10;
    enum {
        ONE, TWO, THREE
    };
};
```

## Code Style

### Files

- always include the copyright notice at the top of every file
- make sure the date is correct

```c++
/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

### Headers

- **always** include a class' header **first** in the `.cpp` file.
- other headers are sorted in order of their layering, from **most dependent to least dependent** (most dependent at the top, least dependent/lower layers at the bottom).
- within a layer, headers are sorted alphabetically (with the exception of private local headers).
- strive for implementing one class per file.

#### Header Layering Hierarchy
Headers must be grouped and separated by **exactly one blank line** in the following order (top to bottom):

1.  **0. Corresponding Header:** `#include "details/Bar.h"` (the implementation's own header).
2.  **0.5. Windows Cleanup Header:** `#include <utils/unwindows.h>` (always second to undefine Windows macros, only if needed).
3.  **1. Private / Local Headers:** `#include "PrivateStuff.h"` (using `" "` syntax **only** for truly private headers located under the local module's `src/` directory or its subdirectories).
4.  **2. Internal Proprietary Libraries:** `#include <filament/Box.h>`, `#include <private/filament/EngineEnums.h>`, `#include <filaflat/MaterialChunk.h>`, `#include <backend/Handle.h>` (all public and private includes for other proprietary libraries **must** use `< >` brackets syntax. The layering priority order of these libraries is dynamically computed based on the `CMakeLists.txt` target linkage graph from most dependent to least dependent).
5.  **3. Third-Party Libraries:** `#include <tsl/robin_map.h>`, `#include <jni.h>` (known third-party and system includes).
6.  **4. C++ Standard Library:** `#include <algorithm>` (standard C++ templates and `<c...>` wrappers like `<cstdint>`, `<cstddef>`).
7.  **5. POSIX System Headers:** `#include <unistd.h>` (C POSIX system headers and `<sys/...>` headers, placed below C++ standard but above legacy C standard).
8.  **6. C Standard Library:** `#include <stddef.h>` (legacy system headers ending in `.h`).

#### Private / Local Includes Formatting (Layer 1)
To maximize readability for local quoted includes:
*   **Flat-First Sorting:** Flat/top-level files (without a directory prefix, e.g., `"Allocators.h"`) are sorted **first**. Files nested inside subdirectories (e.g., `"components/Foo.h"`) are sorted **second**.
*   **Sub-Directory Spacing:** includes are grouped by their sub-directory path prefix, with **exactly one blank line** separating different sub-directory groups.

*Sorting the headers is important to help catching missing `#include` directives.*

```c++
/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Bar.cpp

// 0. Corresponding header
#include <foo/Bar.h>

// 0.5. Windows Cleanup
#include <utils/unwindows.h>

// 1. Private / local headers (grouped by subdirectory prefix, flat first)
#include "Allocators.h"

#include "components/LightManager.h"
#include "components/RenderableManager.h"

#include "details/Engine.h"
#include "details/Skybox.h"

// 2. Internal library headers (sorted by topological dependencies: filaflat above backend)
#include <private/filament/EngineEnums.h>
#include <filament/Box.h>

#include <filaflat/MaterialChunk.h>

#include <backend/Handle.h>

#include <private/utils/Tracing.h>
#include <utils/Allocator.h>

#include <math/vec3.h>

// 3. Third-party
#include <tsl/robin_map.h>

// 4. C++ Standard Library
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

// 5. POSIX / System headers
#include <unistd.h>
#include <sys/stat.h>

// 6. C Standard Library
#include <stddef.h>
#include <stdint.h>
```

#### Automatic Header Formatting Tool
An official script is available in the repository to automatically format and sort C++ include blocks in accordance with these rules:

```bash
./tools/reorganize_headers.py <file_or_directory>
```

To see what changes would be made without modifying the files, use the `--dry-run` option:
```bash
./tools/reorganize_headers.py --dry-run <file_or_directory>
```

*Note: The tool has safety checks built-in and will automatically skip any files with inline preprocessor conditionals (`#if`, `#ifdef`, etc.) or nested code statements inside their include block to prevent compilation regressions.*

- `STL` limited in **filament** public headers to:
    - `array`
    - `initializer_list`
    - `iterator`
    - `limits`
    - `optional`
    - `type_traits`
    - `utility`
    - `variant`

For **libfilament** the rule of thumb is that STL headers that don't generate code are allowed (e.g. `type_traits`),
conversely containers and algorithms are not allowed. There are exceptions such as `array`. See above for the full list.
- The following `STL` headers are banned entirely, from public and private headers as well as implementation files:
  - `iostream`

### Strings

- Never use `std::string` in the Filament core renderer. Prefer `utils::CString` or `std::string_view`.
- When using `std::string` in tools, always include the `std::` qualifier to disambiguate it
  from other string types.

### Misc

- Use `auto` only when the type appears on the same line or with iterators and lambdas.
```c++
auto foo = new Foo();
for (auto& i : collection) { }
```
