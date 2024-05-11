/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <filaflat/MaterialChunk.h>
#include <filaflat/ChunkContainer.h>

#include <backend/DriverEnums.h>

#include <utils/Log.h>

namespace filaflat {

static inline uint32_t makeKey(
        MaterialChunk::ShaderModel shaderModel,
        MaterialChunk::Variant variant,
        MaterialChunk::ShaderStage stage) noexcept {
    static_assert(sizeof(variant.key) * 8 <= 8);
    return (uint32_t(shaderModel) << 16) | (uint32_t(stage) << 8) | variant.key;
}

void MaterialChunk::decodeKey(uint32_t key,
        MaterialChunk::ShaderModel* model,
        MaterialChunk::Variant* variant,
        MaterialChunk::ShaderStage* stage) {
    variant->key = key & 0xff;
    *model = MaterialChunk::ShaderModel((key >> 16) & 0xff);
    *stage = MaterialChunk::ShaderStage((key >> 8) & 0xff);
}

MaterialChunk::MaterialChunk(ChunkContainer const& container)
        : mContainer(container) {
}

MaterialChunk::~MaterialChunk() noexcept = default;

bool MaterialChunk::initialize(filamat::ChunkType materialTag) {

    if (mBase != nullptr) {
        // initialize() should be called only once.
        return true;
    }

    auto [start, end] = mContainer.getChunkRange(materialTag);
    Unflattener unflattener(start, end);

    mUnflattener = unflattener;
    mMaterialTag = materialTag;
    mBase = unflattener.getCursor();

    // Read how many shaders we have in the chunk.
    uint64_t numShaders;
    if (!unflattener.read(&numShaders) || numShaders == 0) {
        return false;
    }

    // Read all index entries.
    for (uint64_t i = 0 ; i < numShaders; i++) {
        uint8_t model;
        Variant variant;
        uint8_t stage;
        uint32_t offsetValue;

        if (!unflattener.read(&model)) {
            return false;
        }

        if (!unflattener.read(&variant)) {
            return false;
        }

        if (!unflattener.read(&stage)) {
            return false;
        }

        if (!unflattener.read(&offsetValue)) {
            return false;
        }

        uint32_t key = makeKey(ShaderModel(model), variant, ShaderStage(stage));
        mOffsets[key] = offsetValue;
    }
    return true;
}

bool MaterialChunk::getTextShader(Unflattener unflattener,
        BlobDictionary const& dictionary, ShaderContent& shaderContent,
        ShaderModel shaderModel, Variant variant, ShaderStage shaderStage) {
    if (mBase == nullptr) {
        return false;
    }

    // Jump and read
    uint32_t key = makeKey(shaderModel, variant, shaderStage);
    auto pos = mOffsets.find(key);
    if (pos == mOffsets.end()) {
        return false;
    }

    size_t offset = pos->second;
    if (offset == 0) {
        // This shader was not found.
        return false;
    }
    unflattener.setCursor(mBase + offset);

    // Read how big the shader is.
    uint32_t shaderSize = 0;
    if (!unflattener.read(&shaderSize)){
        return false;
    }

    // Read how many lines there are.
    uint32_t lineCount = 0;
    if (!unflattener.read(&lineCount)){
        return false;
    }

    shaderContent.reserve(shaderSize);
    shaderContent.resize(shaderSize);
    size_t cursor = 0;

    // Read all lines.
    for(int32_t i = 0 ; i < lineCount; i++) {
        uint16_t lineIndex;
        if (!unflattener.read(&lineIndex)) {
            return false;
        }
        const auto& content = dictionary[lineIndex];

        // Replace null with newline.
        memcpy(&shaderContent[cursor], content.data(), content.size() - 1);
        cursor += content.size() - 1;
        shaderContent[cursor++] = '\n';
    }

    // Write the terminating null character.
    shaderContent[cursor++] = 0;
    assert_invariant(cursor == shaderSize);

    return true;
}

bool MaterialChunk::getBinaryShader(BlobDictionary const& dictionary,
        ShaderContent& shaderContent, ShaderModel shaderModel, filament::Variant variant, ShaderStage shaderStage) {

    if (mBase == nullptr) {
        return false;
    }

    uint32_t key = makeKey(shaderModel, variant, shaderStage);
    auto pos = mOffsets.find(key);
    if (pos == mOffsets.end()) {
        return false;
    }

    shaderContent = dictionary[pos->second];
    return true;
}

bool MaterialChunk::hasShader(ShaderModel model, Variant variant, ShaderStage stage) const noexcept {
    if (mBase == nullptr) {
        return false;
    }
    auto pos = mOffsets.find(makeKey(model, variant, stage));
    return pos != mOffsets.end();
}

bool MaterialChunk::getShader(ShaderContent& shaderContent, BlobDictionary const& dictionary,
        ShaderModel shaderModel, filament::Variant variant, ShaderStage stage) {
    switch (mMaterialTag) {
        case filamat::ChunkType::MaterialGlsl:
        case filamat::ChunkType::MaterialEssl1:
        case filamat::ChunkType::MaterialMetal:
            return getTextShader(mUnflattener, dictionary, shaderContent, shaderModel, variant, stage);
        case filamat::ChunkType::MaterialSpirv:
        case filamat::ChunkType::MaterialMetalLibrary:
            return getBinaryShader(dictionary, shaderContent, shaderModel, variant, stage);
        default:
            return false;
    }
}

uint32_t MaterialChunk::getShaderCount() const noexcept {
    Unflattener unflattener{ mUnflattener }; // make a copy
    uint64_t numShaders;
    unflattener.read(&numShaders);
    return uint32_t(numShaders);
}

void MaterialChunk::visitShaders(
        utils::Invocable<void(ShaderModel, Variant, ShaderStage)>&& visitor) const {

    Unflattener unflattener{ mUnflattener }; // make a copy

    // read() calls below cannot fail by construction, because we've already run through them
    // in the constructor.

    // Read how many shaders we have in the chunk.
    uint64_t numShaders;
    unflattener.read(&numShaders);

    // Read all index entries.
    for (uint64_t i = 0; i < numShaders; i++) {
        uint8_t shaderModelValue;
        filament::Variant variant;
        uint8_t pipelineStageValue;
        uint32_t offsetValue;

        unflattener.read(&shaderModelValue);
        unflattener.read(&variant);
        unflattener.read(&pipelineStageValue);
        unflattener.read(&offsetValue);

        visitor(ShaderModel(shaderModelValue), variant, ShaderStage(pipelineStageValue));
    }
}

} // namespace filaflat

