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

#ifndef TNT_FILAMAT_MATERIAL_TEXT_CHUNK_H
#define TNT_FILAMAT_MATERIAL_TEXT_CHUNK_H

#include <vector>

#include "Chunk.h"
#include "LineDictionary.h"
#include "ShaderEntry.h"

namespace filamat {

class MaterialTextChunk final : public Chunk {
public:
    MaterialTextChunk(const std::vector<TextEntry>&& entries, const LineDictionary& dictionary,
            ChunkType type) : Chunk(type), mEntries(entries), mDictionary(dictionary) {
    }
    ~MaterialTextChunk() override = default;

private:
    void flatten(Flattener& f) override;

    void writeEntryAttributes(size_t entryIndex, Flattener& f) const noexcept;

    // Structure to keep track of duplicates.
    struct ShaderMapping {
        bool isDup = false;
        size_t dupOfIndex = 0;
    };
    std::vector<ShaderMapping> mDuplicateMap;

    const std::vector<TextEntry> mEntries;
    const LineDictionary& mDictionary;
};

} // namespace filamat
#endif // TNT_FILAMAT_TEXT_CHUNK_H
