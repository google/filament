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

#ifndef TNT_FILAMAT_DIC_TEXT_CHUNK_H
#define TNT_FILAMAT_DIC_TEXT_CHUNK_H

#include <stdint.h>
#include <vector>

#include "Chunk.h"
#include "Flattener.h"
#include "LineDictionary.h"

namespace filamat {

class DictionaryTextChunk final : public Chunk {
public:
    DictionaryTextChunk(LineDictionary&& dictionary, ChunkType chunkType);
    ~DictionaryTextChunk() = default;

    const LineDictionary& getDictionary() const noexcept { return mDictionary; }

private:
    void flatten(Flattener& f) override;

    const LineDictionary mDictionary;
};

} // namespace filamat

#endif // TNT_FILAMAT_DIC_TEXT_CHUNK_H
