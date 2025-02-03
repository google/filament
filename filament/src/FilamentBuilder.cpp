/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <filament/FilamentAPI.h>

#include <algorithm>

namespace filament {

void builderMakeName(utils::CString& outName, const char* name, size_t const len) noexcept {
    if (!name) {
        return;
    }
    size_t const length = std::min(len, size_t { 128u });
    outName = utils::CString(name, length);
}

} // namespace filament
