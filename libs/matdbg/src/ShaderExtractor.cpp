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

#include <matdbg/ShaderExtractor.h>

#include <filaflat/BlobDictionary.h>
#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>
#include <filaflat/ShaderBuilder.h>
#include <filaflat/Unflattener.h>

#include <filament/MaterialChunkType.h>
#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

using namespace filament;
using namespace backend;
using namespace filaflat;
using namespace filamat;
using namespace std;
using namespace utils;

namespace filament {
namespace matdbg {

ShaderExtractor::ShaderExtractor(Backend backend, const void* data, size_t size)
        : mChunkContainer(data, size), mMaterialChunk(mChunkContainer) {
    switch (backend) {
        case Backend::OPENGL:
            mMaterialTag = ChunkType::MaterialGlsl;
            mDictionaryTag = ChunkType::DictionaryGlsl;
            break;
        case Backend::METAL:
            mMaterialTag = ChunkType::MaterialMetal;
            mDictionaryTag = ChunkType::DictionaryMetal;
            break;
        case Backend::VULKAN:
            mMaterialTag = ChunkType::MaterialSpirv;
            mDictionaryTag = ChunkType::DictionarySpirv;
            break;
        default:
            break;
    }
}

bool ShaderExtractor::parse() noexcept {
    if (mChunkContainer.parse()) {
        return mMaterialChunk.readIndex(mMaterialTag);
    }
    return false;
}

bool ShaderExtractor::getDictionary(BlobDictionary& dictionary) noexcept {
    return DictionaryReader::unflatten(mChunkContainer, mDictionaryTag, dictionary);
}

bool ShaderExtractor::getShader(ShaderModel shaderModel,
        uint8_t variant, ShaderType stage, ShaderBuilder& shader) noexcept {

    ChunkContainer const& cc = mChunkContainer;
    if (!cc.hasChunk(mMaterialTag) || !cc.hasChunk(mDictionaryTag)) {
        return false;
    }

    BlobDictionary blobDictionary;
    if (!DictionaryReader::unflatten(cc, mDictionaryTag, blobDictionary)) {
        return false;
    }

    return mMaterialChunk.getShader(shader, blobDictionary, (uint8_t)shaderModel, variant, stage);
}

CString ShaderExtractor::spirvToGLSL(const uint32_t* data, size_t wordCount) {
    using namespace spirv_cross;

    CompilerGLSL::Options emitOptions;
    emitOptions.es = true;
    emitOptions.vulkan_semantics = true;

    vector<uint32_t> spirv(data, data + wordCount);
    CompilerGLSL glslCompiler(move(spirv));
    glslCompiler.set_common_options(emitOptions);

    return CString(glslCompiler.compile().c_str());
}

// If desired feel free to locally replace this with the glslang disassembler (spv::Disassemble)
// but please do not submit. We prefer to use the syntax that the standalone "spirv-dis" tool
// uses, which lets us easily generate test cases for the spirv-cross project.
CString ShaderExtractor::spirvToText(const uint32_t* begin, size_t wordCount) {
    auto context = spvContextCreate(SPV_ENV_UNIVERSAL_1_1);
    spv_text text = nullptr;
    const uint32_t options = SPV_BINARY_TO_TEXT_OPTION_INDENT |
            SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;
    spvBinaryToText(context, begin, wordCount, options, &text, nullptr);
    CString result(text->str);
    spvTextDestroy(text);
    spvContextDestroy(context);
    return result;
}

} // namespace matdbg
} // namespace filament
