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

#ifndef MATDBG_SHADEREXTRACTOR_H
#define MATDBG_SHADEREXTRACTOR_H

#include <filaflat/ChunkContainer.h>
#include <filaflat/MaterialChunk.h>

#include <backend/DriverEnums.h>

#include <utils/CString.h>

namespace filament {
namespace matdbg {

// ShaderExtractor is a utility class for extracting shader source from a material package. It works
// in a manner similar to ShaderReplacer.
class ShaderExtractor {
public:
    ShaderExtractor(backend::ShaderLanguage target, const void* data, size_t size);
    bool parse() noexcept;
    bool getShader(backend::ShaderModel shaderModel,
            Variant variant, backend::ShaderStage stage, filaflat::ShaderContent& shader) noexcept;
    bool getDictionary(filaflat::BlobDictionary& dictionary) noexcept;

    static utils::CString spirvToGLSL(backend::ShaderModel shaderModel, const uint32_t* data,
            size_t wordCount);
    static utils::CString spirvToText(const uint32_t* data, size_t wordCount);

private:
    filaflat::ChunkContainer mChunkContainer;
    filaflat::MaterialChunk mMaterialChunk;
    filamat::ChunkType mMaterialTag = filamat::ChunkType::Unknown;
    filamat::ChunkType mDictionaryTag = filamat::ChunkType::Unknown;
};

} // namespace matdbg
} // namespace filament

#endif  // MATDBG_SHADEREXTRACTOR_H
