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

#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>
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
using namespace utils;

namespace filament {
namespace matdbg {

ShaderExtractor::ShaderExtractor(backend::ShaderLanguage target, const void* data, size_t size)
        : mChunkContainer(data, size), mMaterialChunk(mChunkContainer) {
    switch (target) {
        case backend::ShaderLanguage::ESSL1:
            mMaterialTag = ChunkType::MaterialEssl1;
            mDictionaryTag = ChunkType::DictionaryText;
            break;
        case backend::ShaderLanguage::ESSL3:
            mMaterialTag = ChunkType::MaterialGlsl;
            mDictionaryTag = ChunkType::DictionaryText;
            break;
        case backend::ShaderLanguage::MSL:
            mMaterialTag = ChunkType::MaterialMetal;
            mDictionaryTag = ChunkType::DictionaryText;
            break;
        case backend::ShaderLanguage::METAL_LIBRARY:
            mMaterialTag = ChunkType::MaterialMetalLibrary;
            mDictionaryTag = ChunkType::DictionaryMetalLibrary;
            break;
        case backend::ShaderLanguage::WGSL:
            mMaterialTag = ChunkType::MaterialWgsl;
            mDictionaryTag = ChunkType::DictionaryText;
            break;
        case backend::ShaderLanguage::SPIRV:
            mMaterialTag = ChunkType::MaterialSpirv;
            mDictionaryTag = ChunkType::DictionarySpirv;
            break;
    }
}

bool ShaderExtractor::parse() noexcept {
    if (mChunkContainer.parse()) {
        return mMaterialChunk.initialize(mMaterialTag);
    }
    return false;
}

bool ShaderExtractor::getDictionary(BlobDictionary& dictionary) noexcept {
    return DictionaryReader::unflatten(mChunkContainer, mDictionaryTag, dictionary);
}

bool ShaderExtractor::getShader(ShaderModel shaderModel,
        Variant variant, ShaderStage stage, ShaderContent& shader) noexcept {

    ChunkContainer const& cc = mChunkContainer;
    if (!cc.hasChunk(mMaterialTag) || !cc.hasChunk(mDictionaryTag)) {
        return false;
    }

    BlobDictionary blobDictionary;
    if (!DictionaryReader::unflatten(cc, mDictionaryTag, blobDictionary)) {
        return false;
    }

    return mMaterialChunk.getShader(shader, blobDictionary, shaderModel, variant, stage);
}

CString ShaderExtractor::spirvToGLSL(ShaderModel shaderModel, const uint32_t* data,
        size_t wordCount) {
    using namespace spirv_cross;

    CompilerGLSL::Options emitOptions;
    if (shaderModel == ShaderModel::MOBILE) {
        emitOptions.es = true;
        emitOptions.version = 310;
        emitOptions.fragment.default_float_precision = CompilerGLSL::Options::Precision::Mediump;
        emitOptions.fragment.default_int_precision = CompilerGLSL::Options::Precision::Mediump;
    } else if (shaderModel == ShaderModel::DESKTOP) {
        emitOptions.es = false;
        emitOptions.version = 450;
    }
    emitOptions.vulkan_semantics = true;

    std::vector<uint32_t> spirv(data, data + wordCount);
    CompilerGLSL glslCompiler(std::move(spirv));
    glslCompiler.set_common_options(emitOptions);

    return CString(glslCompiler.compile().c_str());
}

// If desired feel free to locally replace this with the glslang disassembler (spv::Disassemble)
// but please do not submit. We prefer to use the syntax that the standalone "spirv-dis" tool
// uses, which lets us easily generate test cases for the spirv-cross project.
CString ShaderExtractor::spirvToText(const uint32_t* begin, size_t wordCount) {
    spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_3);

    if (spvValidateBinary(context, begin, wordCount, nullptr) != SPV_SUCCESS) {
        spvContextDestroy(context);
        return CString("Validation failure.");
    }

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
