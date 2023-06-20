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

#include <filaflat/ChunkContainer.h>

#include <filaflat/Unflattener.h>

namespace filaflat {

ChunkContainer::~ChunkContainer() noexcept = default;

bool ChunkContainer::hasChunk(Type type) const noexcept {
    auto const& chunks = mChunks;
    auto pos = chunks.find(type);
    return pos != chunks.end();
}

bool ChunkContainer::hasChunk(Type type, ChunkDesc* pChunkDesc) const noexcept {
    assert_invariant(pChunkDesc);
    auto const& chunks = mChunks;
    auto pos = chunks.find(type);
    if (UTILS_LIKELY(pos != chunks.end())) {
        *pChunkDesc = pos.value();
        return true;
    }
    return false;
}

bool ChunkContainer::parseChunk(Unflattener& unflattener) {
    uint64_t type;
    if (!unflattener.read(&type)) {
        return false;
    }

    uint32_t size;
    if (!unflattener.read(&size)) {
        return false;
    }

    // If size goes beyond the boundaries of the package, this is an invalid chunk. Discard it.
    // All remaining chunks cannot be accessed and will not be mapped.
    auto cursor = unflattener.getCursor();
    if (!(cursor + size >= (uint8_t *)mData &&
          cursor + size <= (uint8_t *)mData + mSize)) {
        return false;
    }

    mChunks[Type(type)] = { cursor, size };
    unflattener.setCursor(cursor + size);
    return true;
}

bool ChunkContainer::parse() noexcept {
    Unflattener unflattener((uint8_t *)mData, (uint8_t *)mData + mSize);
    do {
        if (!parseChunk(unflattener)) {
            return false;
        }
    } while (unflattener.hasData());
    return true;
}

} // namespace filaflat
