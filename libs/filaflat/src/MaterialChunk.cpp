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

#include <utils/Log.h>

using namespace filament::driver;

namespace filaflat {


static inline uint32_t makeKey(ShaderModel shaderModel, uint8_t variant, ShaderType type) noexcept {
    static_assert(sizeof(ShaderModel) == 1, "ShaderModel must not exceed 8 bits");
    static_assert(sizeof(ShaderType) == 1, "ShaderType must not exceed 8 bits");
    return (uint8_t(shaderModel) << 16) | (uint8_t(type) << 8) | variant;
}

bool MaterialChunk::readIndex(Unflattener& unflattener) {
    mBase = unflattener.getCursor();

    // Read how many shaders we have in the chunk.
    uint64_t numShaders;
    if (!unflattener.read(&numShaders) || numShaders == 0) {
        return false;
    }

    // Read all index entries.
    for (uint64_t i = 0 ; i < numShaders; i++) {
        uint8_t shaderModelValue;
        uint8_t variantValue;
        uint8_t pipelineStageValue;
        uint32_t offsetValue;

        if (!unflattener.read(&shaderModelValue)) {
            return false;
        }

        if (!unflattener.read(&variantValue)) {
            return false;
        }

        if (!unflattener.read(&pipelineStageValue)) {
            return false;
        }

        if (!unflattener.read(&offsetValue)) {
            return false;
        }

        switch (ShaderModel(shaderModelValue)) {
            case ShaderModel::GL_ES_30:
            case ShaderModel::GL_CORE_41:
                // skip unsupported shader models (needed for older materials that support GL4.5)
                break;
            default:
                continue;
        }

        pipelineStageValue = std::min(pipelineStageValue, uint8_t(filament::driver::PIPELINE_STAGE_COUNT));

        uint32_t key = makeKey(ShaderModel(shaderModelValue), variantValue, ShaderType(pipelineStageValue));
        mOffsets[key] = offsetValue;
    }
    return true;
}

bool MaterialChunk::getTextShader(Unflattener unflattener, BlobDictionary& dictionary,
        ShaderBuilder& shader, ShaderModel shaderModel, uint8_t variant, ShaderType ps) {

    shader.reset();
    if (mBase == nullptr ) {
        if (!readIndex(unflattener)) {
            return false;
        }
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

    // Add an extra char for the null terminator.
    shader.announce(shaderSize + 1);

    // Read how many lines there are.
    uint32_t numLines = 0;
    if (!unflattener.read(&numLines)){
        return false;
    }

    // Read all lines.
    for(int32_t i = 0 ; i < numLines; i++) {
        uint16_t lineIndex;
        if (!unflattener.read(&lineIndex)) {
            return false;
        }
        const char* string = dictionary.getString(lineIndex);
        shader.appendPart(string, strlen(string));
        shader.appendPart("\n", 1);
    }

    // Write the terminating null character.
    shader.appendPart("", 1);

    return true;
}


bool MaterialChunk::getSpirvShader(Unflattener unflattener, BlobDictionary& dictionary,
        ShaderBuilder& builder, ShaderModel shaderModel, uint8_t variant, ShaderType stage) {
    if (mBase == nullptr ) {
        if (!readIndex(unflattener)) {
            return false;
        }
    }
    uint32_t key = makeKey(shaderModel, variant, stage);
    auto pos = mOffsets.find(key);
    if (pos == mOffsets.end()) {
        return false;
    }

    size_t index = pos->second;
    size_t shaderSize;
    const char* shaderContent = dictionary.getBlob(index, &shaderSize);
    builder.reset();
    builder.announce(shaderSize);
    builder.appendPart(shaderContent, shaderSize);
    return true;
}

}
