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

#ifndef TNT_FILAFLAT_CHUNK_CONTAINER_H
#define TNT_FILAFLAT_CHUNK_CONTAINER_H


#include <utils/compiler.h>

#include <filament/MaterialChunkType.h>

#include <tsl/robin_map.h>

namespace filaflat {

class Unflattener;

// Allows to build a map of chunks in a Package and get direct individual access based on chunk ID.
class UTILS_PUBLIC ChunkContainer {
public:
    using Type = filamat::ChunkType;

    ChunkContainer(void const* data, size_t size) : mData(data), mSize(size) {}

    ~ChunkContainer() = default;

    // Must be called before trying to access any of the chunk. Fails and return false ONLY if
    // an incomplete chunk is found or if a chunk with bogus size is found.
    bool parse() noexcept;

    typedef struct {
        const uint8_t* start;
        size_t size;
    } ChunkDesc;

    typedef struct {
        Type type;
        ChunkDesc desc;
    } Chunk;

    size_t getChunkCount() const noexcept {
        return mChunks.size();
    }

    Chunk getChunk(size_t index) const noexcept {
        auto it = mChunks.begin();
        std::advance(it, index);
        return { it->first, it->second };
    }

    const uint8_t* getChunkStart(Type type) const noexcept {
        return mChunks.at(type).start;
    }

    const uint8_t* getChunkEnd(Type type) const noexcept {
        return mChunks.at(type).start + mChunks.at(type).size;
    }

    bool hasChunk(Type type) const noexcept {
        return mChunks.find(type) != mChunks.end();
    }

    void const* getData() const { return mData; }

    size_t getSize() const { return mSize; }

private:
    bool parseChunk(Unflattener& unflattener);

    void const* mData;
    size_t mSize;
    tsl::robin_map<Type, ChunkContainer::ChunkDesc> mChunks;
};

} // namespace filaflat
#endif
