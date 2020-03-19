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

#include <filaflat/BlobDictionary.h>
#include <filaflat/Unflattener.h>

#if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
#include <utils/Log.h>
#include <smolv.h>
#endif

#include <assert.h>

using namespace filamat;

namespace filaflat {

bool DictionaryReader::unflatten(ChunkContainer const& container,
        ChunkContainer::Type dictionaryTag,
        BlobDictionary& dictionary) {

    Unflattener unflattener(
            container.getChunkStart(dictionaryTag),
            container.getChunkEnd(dictionaryTag));

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

        dictionary.reserve(blobCount);
        for (uint32_t i = 0; i < blobCount; i++) {
            const char* compressed;
            size_t compressedSize;
            if (!unflattener.read(&compressed, &compressedSize)) {
                return false;
            }

#if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
            size_t spirvSize = smolv::GetDecodedBufferSize(compressed, compressedSize);
            if (spirvSize == 0) {
                return false;
            }
            BlobDictionary::Blob spirv(spirvSize);
            if (!smolv::Decode(compressed, compressedSize, spirv.data(), spirvSize)) {
                return false;
            }
            dictionary.addBlob(std::move(spirv));
#else
            return false;
#endif

        }
        return true;
    } else if (dictionaryTag == ChunkType::DictionaryText) {
        uint32_t stringCount = 0;
        if (!unflattener.read(&stringCount)) {
            return false;
        }

        dictionary.reserve(stringCount);
        for (uint32_t i = 0; i < stringCount; i++) {
            const char* str;
            if (!unflattener.read(&str)) {
                return false;
            }
            // BlobDictionary hold binary chunks and does not care if the data holds text, it is
            // therefore crucial to include the trailing null.
            dictionary.addBlob(str, strlen(str) + 1);
        }
        return true;
    }

    return false;
}

} // namespace filaflat
