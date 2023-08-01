/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "SpirvFixup.h"

namespace filamat {

bool fixupClipDistance(std::string& spirvDisassembly) {
    size_t p = spirvDisassembly.find("OpDecorate %filament_gl_ClipDistance Location");
    if (p == std::string::npos) {
        return false;
    }
    size_t lineEnd = spirvDisassembly.find('\n', p);
    if (lineEnd == std::string::npos) {
        lineEnd = spirvDisassembly.size();
    }
    spirvDisassembly.replace(p, lineEnd - p,
            "OpDecorate %filament_gl_ClipDistance BuiltIn ClipDistance");
    return true;
}

} // namespace filamat
