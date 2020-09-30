/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef VIEWER_AUTOMATION_H
#define VIEWER_AUTOMATION_H

#include <viewer/Settings.h>

namespace filament {
namespace viewer {

// Immutable list of Settings objects generated from a JSON spec.
//
// Each top-level item in the JSON spec is an object with "name", "base" and "permute".
// The "base" object specifies a single set of changes to apply to default settings.
// The optional "permute" object specifies a cross product of changes to apply to the base.
// See the unit test for an example.
class AutomationList {
public:

    // Parses a JSON spec, then generates a list of Settings objects.
    // Returns null on failure (see utils log for warnings and errors).
    // Clients should release memory using "delete".
    static AutomationList* generate(const char* jsonSpec, size_t size);

    // Generates a list of Settings objects using an embedded JSON spec.
    static AutomationList* generateDefaultTestCases();

    // Returns the number of generated Settings objects.
    size_t size() const;

    // Gets a generated Settings object and copies it out.
    // Returns false if the given index is out of bounds.
    bool get(size_t index, Settings* out) const;

    // Returns the name of the JSON group for a given Settings object.
    char const* getName(size_t index) const;

    // Frees all Settings objects and name strings.
    ~AutomationList();

private:
    struct Impl;
    AutomationList(Impl*);
    Impl* mImpl;
};

} // namespace viewer
} // namespace filament

#endif // VIEWER_AUTOMATION_H
