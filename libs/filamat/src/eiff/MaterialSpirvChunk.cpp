/*
 * Copyright (C) 2018 The Android Open Source Project
 *DictionaryGlsl
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

#include "MaterialSpirvChunk.h"

namespace filamat {

MaterialSpirvChunk::MaterialSpirvChunk(const std::vector<SpirvEntry>&& entries) :
        Chunk(ChunkType::MaterialSpirv), mEntries(entries) {}

void MaterialSpirvChunk::flatten(Flattener &f) {
    f.writeUint64(mEntries.size());
    for (const SpirvEntry& entry : mEntries) {
        f.writeUint8(entry.shaderModel);
        f.writeUint8(entry.variantKey);
        f.writeUint8(entry.stage);
        f.writeUint32(entry.dictionaryIndex);
    }
}

}  // namespace filamat
