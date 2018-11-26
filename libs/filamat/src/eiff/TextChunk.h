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

#ifndef TNT_FILAMAT_TEXT_CHUNK_H
#define TNT_FILAMAT_TEXT_CHUNK_H

#include <vector>

#include "Chunk.h"
#include "Flattener.h"
#include "LineDictionary.h"

#include <utils/Panic.h>

using namespace utils;

namespace filamat {

template <class T>
class TextChunk : public Chunk {
public:
    virtual ~TextChunk() = default;
protected:
    TextChunk(ChunkType type, const std::vector<T>& entries, const LineDictionary& dictionary) :
            Chunk(type), mDictionary(dictionary), mEntries(entries) {

    }
    virtual void writeEntryAttributes(size_t entryIndex, Flattener& f) = 0;
    virtual const char* getShaderText(size_t entryIndex) const = 0;

    void compressShader(const char *s, Flattener &f, const LineDictionary& dictionary) const {
        f.writeUint32(static_cast<uint32_t>(strlen(s) + 1));
        f.writeValuePlaceholder();

        size_t numLines = 0;

        size_t cur = 0;
        size_t pos = 0;
        size_t len = 0;

        while (s[cur] != '\0') {
            pos = cur;
            len = 0;
            while (s[cur] != '\n') {
                cur++;
                len++;
            }
            std::string newLine(s, pos, len);
            size_t index = dictionary.getIndex(newLine);
            if (index > UINT16_MAX) {
                slog.e << "Dictionary returned line index beyond what is representable on 16 bits ("
                        << UINT16_MAX << ")." << io::endl;
                exit(0);
            }
            f.writeUint16(static_cast<uint16_t>(index));
            numLines += 1;
            cur++;
        }
        f.writePlaceHoldValue(numLines);
    };

    virtual void flatten(Flattener& f) override {
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

    const std::vector<T>& mEntries;
    const LineDictionary& mDictionary;

    // Structure to keep track of duplicates.
    struct ShaderAttribute{
        bool isDup = false;
        size_t dupOfIndex = 0;
    };
    std::vector<ShaderAttribute> mDuplicateMap;
};

} // namespace filamat

#endif // TNT_FILAMAT_TEXT_CHUNK_H
