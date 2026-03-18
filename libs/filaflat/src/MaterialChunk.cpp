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

#include <private/filament/LineDictionaryUtils.h>


#include "private/filament/Variant.h"

#include <filaflat/ChunkContainer.h>

#include <filament/MaterialChunkType.h>

#include <utils/Invocable.h>
#include <utils/debug.h>
#include <utils/Log.h>

#include <charconv>
#include <string_view>
#include <vector>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace filaflat {

static inline uint32_t makeKey(
        MaterialChunk::ShaderModel shaderModel,
        MaterialChunk::Variant const variant,
        MaterialChunk::ShaderStage stage) noexcept {
    static_assert(sizeof(variant.key) * 8 <= 8);
    return (uint32_t(shaderModel) << 16) | (uint32_t(stage) << 8) | variant.key;
}

void MaterialChunk::decodeKey(uint32_t const key,
        ShaderModel* outModel,
        Variant* outVariant,
        ShaderStage* outStage) {
    outVariant->key = key & 0xff;
    *outModel = ShaderModel((key >> 16) & 0xff);
    *outStage = ShaderStage((key >> 8) & 0xff);
}

MaterialChunk::MaterialChunk(ChunkContainer const& container)
        : mContainer(container) {
}

MaterialChunk::~MaterialChunk() noexcept = default;

bool MaterialChunk::initialize(filamat::ChunkType const materialTag) {

    if (mBase != nullptr) {
        // initialize() should be called only once.
        return true;
    }

    auto [start, end] = mContainer.getChunkRange(materialTag);
    if (start == end) {
        return false;
    }

    Unflattener unflattener(start, end);

    mMaterialTag = materialTag;
    mBase = unflattener.getCursor();

    bool const isTextChunk = (
            mMaterialTag == filamat::ChunkType::MaterialGlsl ||
            mMaterialTag == filamat::ChunkType::MaterialEssl1 ||
            mMaterialTag == filamat::ChunkType::MaterialWgsl ||
            mMaterialTag == filamat::ChunkType::MaterialMetal);

    if (isTextChunk) {
        if (!unflattener.read(&mSharedStrings)) return false;
        if (!unflattener.read(&mVertexStrings)) return false;
        if (!unflattener.read(&mFragmentStrings)) return false;
        if (!unflattener.read(&mComputeStrings)) return false;
    }

    mUnflattener = unflattener;

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

        uint32_t const key = makeKey(ShaderModel(model), variant, ShaderStage(stage));
        mOffsets[key] = offsetValue;
    }
    return true;
}

static bool readDictionaryId(Unflattener& base, Unflattener& ext, uint32_t& outId, size_t& outBytesRead) noexcept {
    uint8_t b8;
    if (!base.read(&b8)) {
        return false;
    }
    
    if (b8 < filament::LineDictionaryUtils::DICTIONARY_1_BYTE_ID_CAPACITY) {
        outId = b8;
        outBytesRead = 1;
        return true;
    }
    if (b8 == filament::LineDictionaryUtils::DICTIONARY_NUMERIC_ID) {
        outId = filament::LineDictionaryUtils::DICTIONARY_NUMERIC_FLAG;
        outBytesRead = 1;
        return true;
    }
    if (b8 < filament::LineDictionaryUtils::DICTIONARY_3_BYTE_ID) {
        uint8_t e;
        if (!ext.read(&e)) return false;
        outId = filament::LineDictionaryUtils::unpack2ByteDictionaryId(b8, e);
        outBytesRead = 2;
        return true;
    }
    uint8_t e0, e1;
    if (!ext.read(&e0) || !ext.read(&e1)) return false;
    outId = filament::LineDictionaryUtils::unpack3ByteDictionaryId(e0, e1);
    outBytesRead = 3;
    return true;
}

bool MaterialChunk::getTextShader(Unflattener unflattener,
        BlobDictionary const& dictionary, ShaderContent& shaderContent,
        ShaderModel const shaderModel, Variant const variant, ShaderStage const shaderStage) const {
    if (mBase == nullptr) {
        return false;
    }

    // Jump and read
    uint32_t const key = makeKey(shaderModel, variant, shaderStage);
    auto const pos = mOffsets.find(key);
    if (pos == mOffsets.end()) {
        return false;
    }

    size_t const offset = pos->second;
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

    uint32_t extLength = 0;
    if (!unflattener.read(&extLength)) {
        return false;
    }

    uint32_t baseLength = 0;
    if (!unflattener.read(&baseLength)) {
        return false;
    }

    uint32_t numericLength = 0;
    if (!unflattener.read(&numericLength)) { return false; }

    shaderContent.reserve(shaderSize);
    shaderContent.resize(shaderSize);
    size_t cursor = 0;

    Unflattener extUnflattener(unflattener);
    extUnflattener.setCursor(unflattener.getCursor() + baseLength);
    
    Unflattener numericUnflattener(unflattener);
    numericUnflattener.setCursor(unflattener.getCursor() + baseLength + extLength);

    auto readNumericLiteral = [](Unflattener& stream) -> uint32_t {
        uint8_t e0;
        if (!stream.read(&e0)) {
            return 0;
        }
        if (e0 < 128) {
            return e0;
        }
        uint8_t e1;
        if (!stream.read(&e1)) {
            return 0;
        }
        return (e0 & 0x7F) | (e1 << 7);
    };

    for (size_t i = 0; i < lineCount; ++i) {
        uint32_t lineIndex = 0;
        size_t bytesRead = 0;
        if (!readDictionaryId(unflattener, extUnflattener, lineIndex, bytesRead)) {
            return false;
        }

        if (lineIndex == filament::LineDictionaryUtils::DICTIONARY_NUMERIC_FLAG) {
            uint32_t const numericLiteral = readNumericLiteral(numericUnflattener);
            char buf[16];
            auto const [ptr, ec] = std::to_chars(buf, buf + 16, numericLiteral);
            size_t const len = ptr - buf;
            memcpy(&shaderContent[cursor], buf, len);
            cursor += len;
            continue;
        }

        uint32_t globalIndex = lineIndex;
        if (shaderStage == ShaderStage::FRAGMENT && lineIndex >= mSharedStrings) {
            globalIndex += mVertexStrings;
        } else if (shaderStage == ShaderStage::COMPUTE && lineIndex >= mSharedStrings) {
            globalIndex += mVertexStrings + mFragmentStrings;
        }

        const auto& content = dictionary[globalIndex];
        // remove the terminating null character.
        size_t const length = content.size() - 1;
        memcpy(&shaderContent[cursor], content.data(), length);
        cursor += length;
    }

    // Explicitly leapfrog the native stream reader past the isolated Extension stream
    // to preserve unflatten sync consistency natively across chunks.
    unflattener.setCursor(numericUnflattener.getCursor());

    // Write the terminating null character.
    shaderContent[cursor++] = 0;
    assert_invariant(cursor == shaderSize);

    return true;
}

bool MaterialChunk::getBinaryShader(BlobDictionary const& dictionary,
        ShaderContent& shaderContent, ShaderModel const shaderModel,
        filament::Variant const variant, ShaderStage const shaderStage) const {

    if (mBase == nullptr) {
        return false;
    }

    uint32_t const key = makeKey(shaderModel, variant, shaderStage);
    auto const pos = mOffsets.find(key);
    if (pos == mOffsets.end()) {
        return false;
    }

    if (UTILS_UNLIKELY(pos->second >= dictionary.size())) {
        return false;
    }

    shaderContent = dictionary[pos->second];
    return true;
}

bool MaterialChunk::hasShader(ShaderModel const model, Variant const variant, ShaderStage const stage) const noexcept {
    if (mBase == nullptr) {
        return false;
    }
    auto const pos = mOffsets.find(makeKey(model, variant, stage));
    return pos != mOffsets.end();
}

bool MaterialChunk::getShader(ShaderContent& shaderContent, BlobDictionary const& dictionary,
        ShaderModel const shaderModel, filament::Variant const variant, ShaderStage const stage) const {
    switch (mMaterialTag) {
        case filamat::ChunkType::MaterialGlsl:
        case filamat::ChunkType::MaterialEssl1:
        case filamat::ChunkType::MaterialWgsl:
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

size_t MaterialChunk::getDictionaryOccurrences(std::vector<uint32_t>& outOccurrences) const {
    size_t totalIndicesSize = 0;

    if (mBase == nullptr || (
        mMaterialTag != filamat::ChunkType::MaterialGlsl &&
        mMaterialTag != filamat::ChunkType::MaterialEssl1 &&
        mMaterialTag != filamat::ChunkType::MaterialWgsl &&
        mMaterialTag != filamat::ChunkType::MaterialMetal)) {
        return 0;
    }

    for (auto const& chunk : mOffsets) {
        ShaderModel model;
        Variant variant;
        ShaderStage stage;
        decodeKey(chunk.first, &model, &variant, &stage);

        Unflattener unflattener(mBase + chunk.second, mContainer.getChunkRange(mMaterialTag).second);

        uint32_t shaderSize = 0;
        if (!unflattener.read(&shaderSize)) {
            continue;
        }

        uint32_t lineCount = 0;
        if (!unflattener.read(&lineCount)) {
            continue;
        }

        uint32_t extLength = 0;
        if (!unflattener.read(&extLength)) {
            continue;
        }

        uint32_t baseLength = 0;
        if (!unflattener.read(&baseLength)) {
            continue;
        }

        Unflattener extUnflattener(unflattener);
        extUnflattener.setCursor(unflattener.getCursor() + baseLength);

        for (size_t i = 0; i < lineCount; ++i) {
            uint32_t lineIndex = 0;
            size_t bytesRead = 0;
            if (!readDictionaryId(unflattener, extUnflattener, lineIndex, bytesRead)) {
                break;
            }
            totalIndicesSize += bytesRead;

            if (lineIndex == filament::LineDictionaryUtils::DICTIONARY_NUMERIC_FLAG) {
                continue;
            }

            uint32_t globalIndex = lineIndex;
            if (stage == ShaderStage::FRAGMENT && lineIndex >= mSharedStrings) {
                globalIndex += mVertexStrings;
            } else if (stage == ShaderStage::COMPUTE && lineIndex >= mSharedStrings) {
                globalIndex += mVertexStrings + mFragmentStrings;
            }

            if (globalIndex < outOccurrences.size()) {
                outOccurrences[globalIndex]++;
            }
        }
    }
    return totalIndicesSize;
}

} // namespace filaflat
