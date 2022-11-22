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

#ifndef MATDBG_SHADERREPLACER_H
#define MATDBG_SHADERREPLACER_H

#include <filaflat/ChunkContainer.h>

#include <backend/DriverEnums.h>
#include "private/filament/Variant.h"

namespace filament {
namespace matdbg {

// ShaderReplacer is a utility class for replacing shader source within a material package. It works
// in a manner similar to ShaderExtractor.
class ShaderReplacer {
public:
    ShaderReplacer(backend::Backend backend, const void* data, size_t size);
    ~ShaderReplacer();
    bool replaceShaderSource(backend::ShaderModel shaderModel, Variant variant,
            backend::ShaderStage stage, const char* sourceString, size_t stringLength);
    const uint8_t* getEditedPackage() const;
    size_t getEditedSize() const;
private:
    bool replaceSpirv(backend::ShaderModel shaderModel, Variant variant,
            backend::ShaderStage stage, const char* source, size_t sourceLength);
    const backend::Backend mBackend;
    filaflat::ChunkContainer mOriginalPackage;
    filaflat::ChunkContainer* mEditedPackage = nullptr;
    filamat::ChunkType mMaterialTag = filamat::ChunkType::Unknown;
    filamat::ChunkType mDictionaryTag = filamat::ChunkType::Unknown;
};

} // namespace matdbg
} // namespace filament

#endif  // MATDBG_SHADERREPLACER_H
