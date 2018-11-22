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

#include "MaterialTextChunk.h"

namespace filamat {

MaterialTextChunk::MaterialTextChunk(const std::vector<TextEntry> &entries,
        LineDictionary &dictionary, ChunkType chunkType) :
    TextChunk(chunkType, entries, dictionary) {
}

void MaterialTextChunk::writeEntryAttributes(size_t entryIndex, Flattener& f) {
    const TextEntry& entry = mEntries[entryIndex];
    f.writeUint8(entry.shaderModel);
    f.writeUint8(entry.variant);
    f.writeUint8(entry.stage);
}

const char* MaterialTextChunk::getShaderText(size_t entryIndex) const {
    return mEntries[entryIndex].shader;
}

} // namespace filamat
