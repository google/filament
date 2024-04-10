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

#include "MaterialBinaryChunk.h"

namespace filamat {

MaterialBinaryChunk::MaterialBinaryChunk(
        const std::vector<SpirvEntry>&& entries, ChunkType chunkType)
    : Chunk(chunkType), mEntries(entries) {}

void MaterialBinaryChunk::flatten(Flattener &f) {
    f.writeUint64(mEntries.size());
    for (const SpirvEntry& entry : mEntries) {
        f.writeUint8(uint8_t(entry.shaderModel));
        f.writeUint8(entry.variant.key);
        f.writeUint8(uint8_t(entry.stage));
        f.writeUint32(entry.dictionaryIndex);
    }
}

}  // namespace filamat
