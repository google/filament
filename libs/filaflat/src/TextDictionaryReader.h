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

#ifndef TNT_FILAFLAT_DIC_GLSL_CHUNK_H
#define TNT_FILAFLAT_DIC_GLSL_CHUNK_H

#include <filaflat/ChunkContainer.h>
#include <filaflat/FilaflatDefs.h>
#include <filaflat/Unflattener.h>

#include "BlobDictionary.h"

#include <vector>
#include <stdint.h>

namespace filaflat {

struct TextDictionaryReader {
    bool unflatten(Unflattener& unflattener, BlobDictionary& dictionary);

    static bool unflatten(ChunkContainer const& container, BlobDictionary& blobDictionary,
            filamat::ChunkType chunkType) {
        Unflattener dictionaryUnflattener(container, chunkType);
        TextDictionaryReader dictionary;
        return dictionary.unflatten(dictionaryUnflattener, blobDictionary);
    }
};

} // namespace filaflat
#endif // TNT_FILAFLAT_DIC_GLSL_CHUNK_H
