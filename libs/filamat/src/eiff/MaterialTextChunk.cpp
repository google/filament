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

void MaterialTextChunk::writeEntryAttributes(size_t entryIndex, Flattener& f) const noexcept {
    const TextEntry& entry = mEntries[entryIndex];
    f.writeUint8(entry.shaderModel);
    f.writeUint8(entry.variant);
    f.writeUint8(entry.stage);
}

const char* MaterialTextChunk::getShaderText(size_t entryIndex) const noexcept {
    return mEntries[entryIndex].shader.c_str();
}

void compressShader(const char *s, Flattener &f, const LineDictionary& dictionary) {
    f.writeUint32(static_cast<uint32_t>(strlen(s) + 1));
    f.writeValuePlaceholder();

    size_t numLines = 0;

    size_t cur = 0;

    while (s[cur] != '\0') {
        size_t pos = cur;
        size_t len = 0;

        while (s[cur] != '\n') {
            cur++;
            len++;
        }

        std::string newLine(s + pos, len);

        size_t index = dictionary.getIndex(newLine);
        if (index > UINT16_MAX) {
            slog.e << "Dictionary returned line index > UINT16_MAX" << io::endl;
            continue;
        }

        f.writeUint16(static_cast<uint16_t>(index));
        numLines += 1;

        cur++;
    }
    f.writePlaceHoldValue(numLines);
}

void MaterialTextChunk::flatten(Flattener& f) {
    f.resetOffsets();

    // Avoid detecting duplicate twice (once for dry run and once for actual flattening).
    if (mDuplicateMap.empty()) {
        mDuplicateMap.resize(mEntries.size());

        // Detect duplicate;
        std::unordered_map<std::string, size_t> stringToIndex;
        for (size_t i = 0; i < mEntries.size(); i++) {
            if (stringToIndex.find(getShaderText(i)) == stringToIndex.end()) { // New
                stringToIndex[getShaderText(i)] = i;
                mDuplicateMap[i].isDup = false;
            } else { // Dup
                mDuplicateMap[i].isDup = true;
                mDuplicateMap[i].dupOfIndex = stringToIndex[mEntries[i].shader];
            }
        }
    }

    // All offsets expressed later will start at the current flattener cursor position
    f.markOffsetBase();

    // Write how many shaders we have
    f.writeUint64(mEntries.size());

    // Write all indexes.
    for (size_t i = 0; i < mEntries.size(); i++) {
        writeEntryAttributes(i, f);

        // Try to reuse a shader if this is a dup.
        if (mDuplicateMap[i].isDup) {
            f.writeOffsetplaceholder(mDuplicateMap[i].dupOfIndex);
        } else {
            f.writeOffsetplaceholder(i);
        }
    }

    // Write all strings
    for (size_t i = 0; i < mEntries.size(); i++) {
        if (mDuplicateMap[i].isDup)
            continue;
        f.writeOffsets(i);
        compressShader(getShaderText(i), f, mDictionary);
    }
}

} // namespace filamat
