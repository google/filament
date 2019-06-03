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

#include "DictionarySpirvChunk.h"

#include <smolv.h>

namespace filamat {

DictionarySpirvChunk::DictionarySpirvChunk(BlobDictionary&& dictionary) :
        Chunk(ChunkType::DictionarySpirv), mDictionary(dictionary){
}

void DictionarySpirvChunk::flatten(Flattener& f) {

    // For now, 1 is the only acceptable compression scheme.
    f.writeUint32(1);

    f.writeUint32(mDictionary.getBlobCount());
    for (size_t i = 0 ; i < mDictionary.getBlobCount() ; i++) {
        const std::string& spirv = mDictionary.getBlob(i);

        smolv::ByteArray compressed;
        const uint32_t flags = smolv::kEncodeFlagStripDebugInfo;
        if (!smolv::Encode(spirv.data(), spirv.size(), compressed, flags)) {
            utils::slog.e << "Error with SPIRV compression" << utils::io::endl;
        }

        f.writeBlob((const char*) compressed.data(), compressed.size());
    }
}

} // namespace filamat
