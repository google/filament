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
#include "TextChunk.h"
#include "LineDictionary.h"
#include "ShaderEntry.h"

namespace filamat {

class MaterialTextChunk final : public TextChunk<TextEntry> {
public:
    MaterialTextChunk(const std::vector<TextEntry> &entries, LineDictionary &dictionary,
            ChunkType chunkType);
    ~MaterialTextChunk() = default;
protected:
    virtual const char* getShaderText(size_t entryIndex) const override;
    virtual void writeEntryAttributes(size_t entryIndex, Flattener& f) override;
};

} // namespace filamat
#endif // TNT_FILAMAT_TEXT_CHUNK_H
