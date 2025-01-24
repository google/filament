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

```
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

```
extern int gGlobalWarming;

class FooBar {
public:
    void methodName();
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

```
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

- **always** include a class' header **first** in the `.cpp` file
- other headers are sorted in reverse order of their layering, that is, lower layer headers last
- within a layer, headers are sorted alphabetically
- strive for implementing one class per file
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


*Sorting the headers is important to help catching missing `#include` directives.*

```
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

#include <foo/Bar.h>

#include "PrivateStuff.h"

#include <foo/Alloc.h>
#include <foo/Bar.h>

#include <utils/compiler.h>

#include <algorithm>
#include <iostream>

#include <assert.h>
#include <string.h>
```

### Strings

- Never use `std::string` in the Filament core renderer. Prefer `utils::CString` or `std::string_view`.
- When using `std::string` in tools, always include the `std::` qualifier to disambiguate it
  from other string types.

### Misc

- Use `auto` only when the type appears on the same line or with iterators and lambdas.
```
auto foo = new Foo();
for (auto& i : collection) { }
```
