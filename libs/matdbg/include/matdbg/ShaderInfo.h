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

#ifndef MATDBG_SHADERINFO_H
#define MATDBG_SHADERINFO_H

#include <backend/DriverEnums.h>

#include <filaflat/ChunkContainer.h>
#include <filaflat/MaterialChunk.h>

#include <private/filament/Variant.h>

namespace filament {
namespace matdbg {

struct ShaderInfo {
    backend::ShaderModel shaderModel;
    Variant variant;
    backend::ShaderStage pipelineStage;
    uint32_t offset;
};

size_t getShaderCount(const filaflat::ChunkContainer& container, filamat::ChunkType type);
bool getMetalShaderInfo(const filaflat::ChunkContainer& container, ShaderInfo* info);
bool getGlShaderInfo(const filaflat::ChunkContainer& container, ShaderInfo* info);
bool getVkShaderInfo(const filaflat::ChunkContainer& container, ShaderInfo* info);

} // namespace matdbg
} // namespace filament

#endif  // MATDBG_SHADERINFO_H
