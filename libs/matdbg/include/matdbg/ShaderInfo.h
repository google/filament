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

namespace filament {
namespace matdbg {

struct ShaderInfo {
    backend::ShaderModel shaderModel;
    uint8_t variant;
    backend::ShaderType pipelineStage;
    uint32_t offset;
};

size_t getShaderCount(filaflat::ChunkContainer container, filamat::ChunkType type);
bool getMetalShaderInfo(filaflat::ChunkContainer container, ShaderInfo* info);
bool getGlShaderInfo(filaflat::ChunkContainer container, ShaderInfo* info);
bool getVkShaderInfo(filaflat::ChunkContainer container, ShaderInfo* info);

} // namespace matdbg
} // namespace filament

#endif  // MATDBG_SHADERINFO_H
