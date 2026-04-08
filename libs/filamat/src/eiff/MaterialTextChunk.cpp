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

#include <private/filament/LineDictionaryUtils.h>

#include <utils/Log.h>
#include <utils/ostream.h>

#include <algorithm>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include <exception>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace filamat {

void MaterialTextChunk::writeEntryAttributes(size_t const entryIndex, Flattener& f) const noexcept {
    const TextEntry& entry = mEntries[entryIndex];
    f.writeUint8(uint8_t(entry.shaderModel));
    f.writeUint8(entry.variant.key);
    f.writeUint8(uint8_t(entry.stage));
}

void compressShader(const std::string& src, ShaderStage const stage, Flattener& f, const LineDictionary& dictionary) {
    if (dictionary.getDictionaryLineCount() > 65536) {
        slog.e << "Dictionary is too large!" << io::endl;
        std::terminate();
    }

    f.writeUint32(static_cast<uint32_t>(src.size() + 1));
    f.writeValuePlaceholder(); // Num Lines
    
    // F.writeValue resolves backwards matching the LIFO queue of Placeholders!
    f.writeValuePlaceholder(); // Ext Stream Size
    f.writeValuePlaceholder(); // Base Stream Size
    f.writeValuePlaceholder(); // Numeric Stream Size

    size_t numLines = 0;
    std::vector<uint8_t> base_stream;
    std::vector<uint8_t> ext_stream;

    uint32_t const S = dictionary.getStageStringCount(0);
    uint32_t const V = dictionary.getStageStringCount(1);
    uint32_t const F = dictionary.getStageStringCount(2);

    auto const [indices, numerics] = dictionary.tokenize(src);
    if (indices.empty() && !src.empty()) {
        slog.e << "Shader completely failed to tokenize!" << io::endl;
        slog.e << "Shader size: " << src.size() << " | src substring: " << src.substr(0, std::min(size_t(50), src.size())) << io::endl;
        slog.e << "Indices map size: " << dictionary.size() << io::endl;
        std::terminate();
    }

    numLines = indices.size();
    for (auto const index : indices) {
        if (index == filament::LineDictionaryUtils::DICTIONARY_NUMERIC_FLAG) {
            base_stream.push_back(filament::LineDictionaryUtils::DICTIONARY_NUMERIC_ID);
            continue;
        }



        uint32_t local_index = index;
        if (stage == ShaderStage::FRAGMENT && index >= S) {
            local_index -= V;
        } else if (stage == ShaderStage::COMPUTE && index >= S) {
            local_index -= V + F;
        }

        if (local_index < filament::LineDictionaryUtils::DICTIONARY_1_BYTE_ID_CAPACITY) {
            base_stream.push_back(static_cast<uint8_t>(local_index));
        } else if (local_index < filament::LineDictionaryUtils::DICTIONARY_2_BYTE_ID_MAX) {
            auto const [prefix, ext] = filament::LineDictionaryUtils::pack2ByteDictionaryId(local_index);
            base_stream.push_back(prefix);
            ext_stream.push_back(ext);
        } else {
            base_stream.push_back(filament::LineDictionaryUtils::DICTIONARY_3_BYTE_ID);
            auto const [extb0, extb1] = filament::LineDictionaryUtils::pack3ByteDictionaryId(local_index);
            ext_stream.push_back(extb0);
            ext_stream.push_back(extb1);
        }
    }

    std::vector<uint8_t> numeric_stream;

    for (auto const value : numerics) {
        if (value < 128) {
            numeric_stream.push_back(static_cast<uint8_t>(value));
        } else {
            numeric_stream.push_back(static_cast<uint8_t>((value & 0x7F) | 0x80));
            numeric_stream.push_back(static_cast<uint8_t>((value >> 7) & 0xFF));
        }
    }

    f.writeRaw(reinterpret_cast<const char*>(base_stream.data()), base_stream.size());
    f.writeRaw(reinterpret_cast<const char*>(ext_stream.data()), ext_stream.size());
    f.writeRaw(reinterpret_cast<const char*>(numeric_stream.data()), numeric_stream.size());

    f.writeValue(static_cast<uint32_t>(numeric_stream.size()));
    f.writeValue(static_cast<uint32_t>(base_stream.size()));
    f.writeValue(static_cast<uint32_t>(ext_stream.size()));
    f.writeValue(static_cast<uint32_t>(numLines));
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

    // Write stage partition counts for decoding restitution
    f.writeUint16(static_cast<uint16_t>(mDictionary.getStageStringCount(0)));
    f.writeUint16(static_cast<uint16_t>(mDictionary.getStageStringCount(1)));
    f.writeUint16(static_cast<uint16_t>(mDictionary.getStageStringCount(2)));
    f.writeUint16(static_cast<uint16_t>(mDictionary.getStageStringCount(3)));

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
        compressShader(mEntries.at(i).shader, mEntries.at(i).stage, f, mDictionary);
    }
}

} // namespace filamat
