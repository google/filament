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
#include "Flattener.h"
#include "LineDictionary.h"
#include "ShaderEntry.h"

#include <exception>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <cstdint>
#include <cassert>
#include <cstddef>
#include <string_view>
#include <unordered_map>
#include <string>

namespace filamat {

void MaterialTextChunk::writeEntryAttributes(size_t const entryIndex, Flattener& f) const noexcept {
    const TextEntry& entry = mEntries[entryIndex];
    f.writeUint8(uint8_t(entry.shaderModel));
    f.writeUint8(entry.variant.key);
    f.writeUint8(uint8_t(entry.stage));
}

void compressShader(std::string_view const src, Flattener &f, const LineDictionary& dictionary) {
    if (dictionary.getDictionaryLineCount() > 65536) {
        slog.e << "Dictionary is too large!" << io::endl;
        std::terminate();
    }

    f.writeUint32(static_cast<uint32_t>(src.size() + 1));
    f.writeValuePlaceholder();

    size_t numLines = 0;

    size_t cur = 0;
    size_t const len = src.length();
    const char* s = src.data();
    while (cur < len) {
        // Start of the current line
        size_t const pos = cur;
        // Find the end of the current line or end of text
        while (cur < len && s[cur] != '\n') {
            cur++;
        }
        // If we found a newline, advance past it for the next iteration, ensuring '\n' is included
        if (cur < len) {
            cur++;
        }
        std::string_view const newLine{ s + pos, cur - pos };

        auto const indices = dictionary.getIndices(newLine);
        if (indices.empty()) {
            slog.e << "Line not found in dictionary!" << io::endl;
            std::terminate();
        }

        numLines += indices.size();
        for (auto const index : indices) {
            f.writeUint16(static_cast<uint16_t>(index));
        }
    }
    f.writeValue(numLines);
}

void MaterialTextChunk::flatten(Flattener& f) {
    f.resetOffsets();

    // Avoid detecting duplicate twice (once for dry run and once for actual flattening).
    if (mDuplicateMap.empty()) {
        mDuplicateMap.resize(mEntries.size());

        // Detect duplicate;
        std::unordered_map<std::string_view, size_t> stringToIndex;
        for (size_t i = 0; i < mEntries.size(); i++) {
            const std::string& text = mEntries[i].shader;
            if (auto iter = stringToIndex.find(text); iter != stringToIndex.end()) {
                mDuplicateMap[i] = { true, iter->second };
            } else {
                stringToIndex.emplace(text, i);
                mDuplicateMap[i].isDup = false;
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
        const ShaderMapping& mapping = mDuplicateMap[i];
        f.writeOffsetPlaceholder(mapping.isDup ? mapping.dupOfIndex : i);
    }

    // Write all strings
    for (size_t i = 0; i < mEntries.size(); i++) {
        if (mDuplicateMap[i].isDup) {
            continue;
        }
        f.writeOffsets(i);
        compressShader(mEntries.at(i).shader, f, mDictionary);
    }
}

} // namespace filamat
