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

#include <filament/MaterialChunkType.h>

#include "Chunk.h"
#include "SimpleFieldChunk.h"

#include <memory>
#include <vector>

namespace filamat {

class Flattener;

class ChunkContainer {
public:
    ChunkContainer() = default;
    ~ChunkContainer() = default;

    template <typename T,
             std::enable_if_t<std::is_base_of<Chunk, T>::value, int> = 0,
             typename... Args>
    const T& push(Args&&... args) {
        T* chunk = new T(std::forward<Args>(args)...);
        mChildren.emplace_back(chunk);
        return *chunk;
    }

    // Helper method to add a SimpleFieldChunk to this ChunkContainer.
    template <typename T, typename... Args>
    const SimpleFieldChunk<T>& emplace(Args&&... args) {
        return push<SimpleFieldChunk<T>>(std::forward<Args>(args)...);
    }

    size_t getSize() const;
    size_t flatten(Flattener& f) const;

private:
    using ChunkPtr = std::unique_ptr<Chunk>;
    std::vector<ChunkPtr> mChildren;
};

} // namespace filamat
#endif
