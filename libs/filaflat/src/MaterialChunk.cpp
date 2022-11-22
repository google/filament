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

#include <utils/Log.h>

namespace filaflat {

static inline uint32_t makeKey(uint8_t shaderModel, filament::Variant variant, uint8_t stage) noexcept {
    static_assert(sizeof(variant.key) * 8 <= 8);
    return (shaderModel << 16) | (stage << 8) | variant.key;
}

void MaterialChunk::decodeKey(uint32_t key, uint8_t* model, filament::Variant::type_t* variant,
        uint8_t* stage) {
    *variant = key & 0xff;
    *stage = (key >> 8) & 0xff;
    *model = (key >> 16) & 0xff;
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
        uint8_t shaderModelValue;
        filament::Variant variant;
        uint8_t pipelineStageValue;
        uint32_t offsetValue;

        if (!unflattener.read(&shaderModelValue)) {
            return false;
        }

        if (!unflattener.read(&variant)) {
            return false;
        }

        if (!unflattener.read(&pipelineStageValue)) {
            return false;
        }

        if (!unflattener.read(&offsetValue)) {
            return false;
        }

        uint32_t key = makeKey(shaderModelValue, variant, pipelineStageValue);
        mOffsets[key] = offsetValue;
    }
    return true;
}

bool MaterialChunk::getTextShader(Unflattener unflattener, BlobDictionary const& dictionary,
        ShaderContent& shaderContent, uint8_t shaderModel, filament::Variant variant, uint8_t ps) {
    if (mBase == nullptr) {
        return false;
    }

    // Jump and read
    uint32_t key = makeKey(shaderModel, variant, ps);
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

bool MaterialChunk::getSpirvShader(BlobDictionary const& dictionary,
        ShaderContent& shaderContent, uint8_t shaderModel, filament::Variant variant, uint8_t stage) {

    if (mBase == nullptr) {
        return false;
    }

    uint32_t key = makeKey(shaderModel, variant, stage);
    auto pos = mOffsets.find(key);
    if (pos == mOffsets.end()) {
        return false;
    }

    shaderContent = dictionary[pos->second];
    return true;
}

bool MaterialChunk::getShader(ShaderContent& shaderContent,
        BlobDictionary const& dictionary, uint8_t shaderModel, filament::Variant variant, uint8_t stage) {
    switch (mMaterialTag) {
        case filamat::ChunkType::MaterialGlsl:
        case filamat::ChunkType::MaterialMetal:
            return getTextShader(mUnflattener, dictionary, shaderContent, shaderModel, variant, stage);
        case filamat::ChunkType::MaterialSpirv:
            return getSpirvShader(dictionary, shaderContent, shaderModel, variant, stage);
        default:
            return false;
    }
}

} // namespace filaflat

