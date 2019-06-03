/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMAT_MATERIAL_SPIRV_CHUNK_H
#define TNT_FILAMAT_MATERIAL_SPIRV_CHUNK_H

#include "Chunk.h"
#include "ShaderEntry.h"

#include <vector>

namespace filamat {

class MaterialSpirvChunk final : public Chunk {
public:
    explicit MaterialSpirvChunk(const std::vector<SpirvEntry>&& entries);
    ~MaterialSpirvChunk() = default;

private:
    void flatten(Flattener& f) override;

    const std::vector<SpirvEntry> mEntries;
};

} // namespace filamat

#endif // TNT_FILAMAT_MATERIAL_SPIRV_CHUNK_H
