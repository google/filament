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

#include "CommonWriter.h"

#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>
#include <filaflat/Unflattener.h>

#include <filament/MaterialChunkType.h>
#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

namespace filament::matdbg {

using namespace backend;
using namespace filaflat;
using namespace filamat;
using namespace utils;

size_t getShaderCount(const ChunkContainer& container, ChunkType type) {
    if (!container.hasChunk(type)) {
        return 0;
    }

    auto [start, end] = container.getChunkRange(type);
    Unflattener unflattener(start, end);

    uint64_t shaderCount = 0;
    if (!unflattener.read(&shaderCount) || shaderCount == 0) {
        return 0;
    }
    return shaderCount;
}

bool getShaderInfo(const ChunkContainer& container, ShaderInfo* info, ChunkType chunkType) {
    if (!container.hasChunk(chunkType)) {
        return true;
    }

    MaterialDomain domain;
    if (!read(container, ChunkType::MaterialDomain, reinterpret_cast<uint8_t*>(&domain))) {
        return false;
    }

    auto [start, end] = container.getChunkRange(chunkType);
    Unflattener unflattener(start, end);

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

        auto stage = ShaderStage(pipelineStageValue);
        if (domain == MaterialDomain::SURFACE) {
            variant = stage == ShaderStage::VERTEX ?
                    Variant::filterVariantVertex(variant) :
                    Variant::filterVariantFragment(variant);
        }
        *info++ = {
            .shaderModel = ShaderModel(shaderModelValue),
            .variant = variant,
            .pipelineStage = ShaderStage(pipelineStageValue),
            .offset = offsetValue
        };
    }
    return true;
}

} // namespace filament
