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

#ifndef TNT_FILAMAT_SHADER_ENTRY_H
#define TNT_FILAMAT_SHADER_ENTRY_H

#include <string>
#include <vector>

namespace filamat {

// TextEntry stores a shader in ASCII text format, like GLSL.
struct TextEntry {
    uint8_t shaderModel;
    uint8_t variant;
    uint8_t stage;
    std::string shader;
};

struct SpirvEntry {
    uint8_t shaderModel;
    uint8_t variant;
    uint8_t stage;
    size_t dictionaryIndex;

#ifndef FILAMAT_LITE
    // temporarily holds this entry's spirv until added to the dictionary
    std::vector<uint32_t> spirv;
#endif
};

}  // namespace filamat

#endif // TNT_FILAMAT_SHADER_ENTRY_H
