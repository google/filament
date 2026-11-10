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

#include <filaflat/DictionaryReader.h>

#include "SmolvUtils.h"

#include <filaflat/ChunkContainer.h>
#include <filaflat/Unflattener.h>

#if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
#include <utils/Log.h>
#include <smolv.h>
#endif

#include <assert.h>

using namespace filamat;

namespace filaflat {

namespace {

// Each blob is stored as an 8-byte length header followed by its bytes, so a chunk can hold at
// most one blob per 8 bytes remaining. Counts beyond that should be rejected before reserve(),
// which allocates the whole array up front.
bool isBlobCountPlausible(uint32_t const blobCount,
        uint8_t const* const cursor, uint8_t const* const end) noexcept {
    if (cursor > end) {
        return false;
    }
    return blobCount <= size_t(end - cursor) / sizeof(uint64_t);
}

} // anonymous namespace

bool DictionaryReader::unflatten(ChunkContainer const& container,
        ChunkContainer::Type dictionaryTag,
        BlobDictionary& dictionary) {

    auto [start, end] = container.getChunkRange(dictionaryTag);
    Unflattener unflattener(start, end);

    if (dictionaryTag == ChunkType::DictionarySpirv) {
        uint32_t compressionScheme;
        if (!unflattener.read(&compressionScheme)) {
            return false;
        }
        // For now, 1 is the only acceptable compression scheme.
        assert(compressionScheme == 1);

        uint32_t blobCount;
        if (!unflattener.read(&blobCount)) {
            return false;
        }
        if (!isBlobCountPlausible(blobCount, unflattener.getCursor(), end)) {
            return false;
        }

        dictionary.reserve(blobCount);
        for (uint32_t i = 0; i < blobCount; i++) {
            unflattener.skipAlignmentPadding();

            const char* compressed;
            size_t compressedSize;
            if (!unflattener.read(&compressed, &compressedSize)) {
                return false;
            }

            assert_invariant((intptr_t(compressed) % 8) == 0);

#if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
            if (getBoundedSmolvDecodedSize(compressed, compressedSize) == 0 ||
                    !smolv::Validate(compressed, compressedSize)) {
                return false;
            }
            dictionary.emplace_back(reinterpret_cast<uint8_t const*>(compressed), compressedSize);
#else
            return false;
#endif

        }
        return true;
    } else if (dictionaryTag == ChunkType::DictionaryMetalLibrary) {
        uint32_t blobCount;
        if (!unflattener.read(&blobCount)) {
            return false;
        }
        if (!isBlobCountPlausible(blobCount, unflattener.getCursor(), end)) {
            return false;
        }

        dictionary.reserve(blobCount);
        for (uint32_t i = 0; i < blobCount; i++) {
            unflattener.skipAlignmentPadding();

            const char* data;
            size_t dataSize;
            if (!unflattener.read(&data, &dataSize)) {
                return false;
            }
            dictionary.emplace_back(reinterpret_cast<uint8_t const*>(data), dataSize);
        }
        return true;
    } else if (dictionaryTag == ChunkType::DictionaryText) {
        uint32_t stringCount = 0;
        if (!unflattener.read(&stringCount)) {
            return false;
        }
        // Reject unreasonably large string counts to prevent OOM from
        // attacker-controlled material data.
        static constexpr uint32_t MAX_DICTIONARY_STRING_COUNT = 65536;
        if (stringCount > MAX_DICTIONARY_STRING_COUNT) {
            return false;
        }

        dictionary.reserve(stringCount);
        for (uint32_t i = 0; i < stringCount; i++) {
            const char* str;
            if (!unflattener.read(&str)) {
                return false;
            }
            // BlobDictionary holds views into the chunk buffer and does not care if the data holds
            // text; Unflattener::read(const char**) advances the cursor to one past the trailing
            // null terminator, which is included in the slice size.
            uint8_t const* const strStart = reinterpret_cast<uint8_t const*>(str);
            dictionary.emplace_back(strStart, size_t(unflattener.getCursor() - strStart));
        }
        return true;
    }

    return false;
}

} // namespace filaflat
