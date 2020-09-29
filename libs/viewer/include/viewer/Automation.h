/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by mIcable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VIEWER_AUTOMATION_H
#define VIEWER_AUTOMATION_H

#include <string>
#include <vector>

#include <viewer/Settings.h>

namespace filament {
namespace viewer {

// Named list of settings permutations.
struct AutomationSpec {
    std::string name;
    std::vector<Settings> cases;
};

// Consumes a JSON string and produces a list of automation specs using a counting algorithm.
//
// Each top-level item in the JSON is an object with "name", "base" and "permute".
// The "base" object specifies a single set of changes to apply to default settings.
// The optional "permute" object specifies a cross product of changes to apply to the base.
// See the unit test for an example.
//
// - Returns true if successful.
// - This function writes warnings and error messages into the utils log.
bool generate(const char* jsonChunk, size_t size, std::vector<AutomationSpec>* out);

} // namespace viewer
} // namespace filament

#endif // VIEWER_AUTOMATION_H
