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

namespace filamat {

DictionarySpirvChunk::DictionarySpirvChunk(BlobDictionary& dictionary) :
        Chunk(ChunkType::DictionarySpirv), mDictionary(dictionary){
}

void DictionarySpirvChunk::flatten(Flattener& f) {
    // TODO: for now the compression option is always 0, fix this.
    f.writeUint32(0);
    f.writeUint32(mDictionary.getBlobCount());
    for (size_t i = 0 ; i < mDictionary.getBlobCount() ; i++) {
        const std::string& blob = mDictionary.getBlob(i);
        f.writeBlob(blob.data(), blob.size());
    }
}

} // namespace filaflat
