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

#include <matdbg/ShaderInfo.h>

#include <filaflat/BlobDictionary.h>
#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>
#include <filaflat/ShaderBuilder.h>
#include <filaflat/Unflattener.h>

#include <filament/MaterialChunkType.h>
#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

namespace filament {
namespace matdbg {

using namespace filament;
using namespace backend;
using namespace filaflat;
using namespace filamat;
using namespace std;
using namespace utils;

size_t getShaderCount(ChunkContainer container, filamat::ChunkType type) {
    if (!container.hasChunk(type)) {
        return 0;
    }

    Unflattener unflattener(container.getChunkStart(type), container.getChunkEnd(type));

    uint64_t shaderCount = 0;
    if (!unflattener.read(&shaderCount) || shaderCount == 0) {
        return 0;
    }
    return shaderCount;
}

bool getMetalShaderInfo(ChunkContainer container, ShaderInfo* info) {
    if (!container.hasChunk(filamat::ChunkType::MaterialMetal)) {
        return true;
    }

    Unflattener unflattener(
            container.getChunkStart(filamat::ChunkType::MaterialMetal),
            container.getChunkEnd(filamat::ChunkType::MaterialMetal));

    uint64_t shaderCount = 0;
    if (!unflattener.read(&shaderCount) || shaderCount == 0) {
        return false;
    }

    for (uint64_t i = 0; i < shaderCount; i++) {
        uint8_t shaderModelValue;
        Variant variant;
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

        *info++ = {
                .shaderModel = ShaderModel(shaderModelValue),
                .variant = variant,
                .pipelineStage = ShaderType(pipelineStageValue),
                .offset = offsetValue
        };
    }

    return true;
}

bool getGlShaderInfo(ChunkContainer container, ShaderInfo* info) {
    if (!container.hasChunk(filamat::ChunkType::MaterialGlsl)) {
        return true;
    }

    Unflattener unflattener(
            container.getChunkStart(filamat::ChunkType::MaterialGlsl),
            container.getChunkEnd(filamat::ChunkType::MaterialGlsl));

    uint64_t shaderCount;
    if (!unflattener.read(&shaderCount) || shaderCount == 0) {
        return false;
    }

    for (uint64_t i = 0; i < shaderCount; i++) {
        uint8_t shaderModelValue;
        Variant variant;
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

        *info++ = {
            .shaderModel = ShaderModel(shaderModelValue),
            .variant = variant,
            .pipelineStage = ShaderType(pipelineStageValue),
            .offset = offsetValue
        };
    }
    return true;
}

bool getVkShaderInfo(ChunkContainer container, ShaderInfo* info) {
    if (!container.hasChunk(filamat::ChunkType::MaterialSpirv)) {
        return true;
    }

    Unflattener unflattener(
            container.getChunkStart(filamat::ChunkType::MaterialSpirv),
            container.getChunkEnd(filamat::ChunkType::MaterialSpirv));

    uint64_t shaderCount;
    if (!unflattener.read(&shaderCount) || shaderCount == 0) {
        return false;
    }

    for (uint64_t i = 0; i < shaderCount; i++) {
        uint8_t shaderModelValue;
        Variant variant;
        uint8_t pipelineStageValue;
        uint32_t dictionaryIndex;

        if (!unflattener.read(&shaderModelValue)) {
            return false;
        }

        if (!unflattener.read(&variant)) {
            return false;
        }

        if (!unflattener.read(&pipelineStageValue)) {
            return false;
        }

        if (!unflattener.read(&dictionaryIndex)) {
            return false;
        }

        *info++ = {
            .shaderModel = ShaderModel(shaderModelValue),
            .variant = variant,
            .pipelineStage = ShaderType(pipelineStageValue),
            .offset = dictionaryIndex
        };
    }
    return true;
}

} // namespace matdbg
} // namespace filament
