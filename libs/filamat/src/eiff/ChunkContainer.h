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

#ifndef TNT_FILAMAT_CHUNK_CONTAINER_H
#define TNT_FILAMAT_CHUNK_CONTAINER_H

#include <vector>

#include "Chunk.h"
#include "Flattener.h"

#include <filament/MaterialChunkType.h>

namespace filamat {

class ChunkContainer {
public:
    ChunkContainer() = default;
    ~ChunkContainer() = default;
    // Copy the POINTER to the chunk and doesn't make a copy. The Chunk* is used later when flattening
    // The Chunk object must be valid until flatten is called (so for the life duration of ChunkContainer).
    void addChild(Chunk* chunk);
    size_t getSize() const;
    size_t flatten(Flattener& f) const;
private:
    std::vector<Chunk*> mChildren;
};

} // namespace filamat
#endif