/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMAT_INCLUDES_H
#define TNT_FILAMAT_INCLUDES_H

#include <utils/CString.h>

#include <filamat/IncludeCallback.h>

#include <vector>

namespace filamat {

// Recursively handle all the includes inside of source.
// Returns true if all includes were handled successfully, false otherwise.
// callback may be null, in which case any #include directives found will result in a failure.
bool resolveIncludes(const utils::CString& rootName, utils::CString& source,
        IncludeCallback callback, size_t depth = 0);

struct FoundInclude {
    utils::CString name;
    size_t startPosition;
    size_t length;
};

std::vector<FoundInclude> parseForIncludes(const utils::CString& source);

} // namespace filamat

#endif
