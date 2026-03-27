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

#ifndef TNT_FILAMAT_LINEDICTIONARY_H
#define TNT_FILAMAT_LINEDICTIONARY_H

#include <backend/DriverEnums.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <backend/DriverEnums.h>

namespace utils::io {
class ostream;
}

namespace filamat {

using ShaderStage = filament::backend::ShaderStage;

/**
 * LineDictionary parses and deduplicates a text shader into a minimal set of optimal string tokens.
 *
 * Encoding Schema (Triple-Stream Interleaved Token Stream):
 * ----------------------------------------------------------------------
 * Once the dictionary determines the global frequency of tokens, MaterialTextChunk encodes the shader
 * text into a binary stream according to the following layout, designed to maximize
 * LZ77 sliding window deduplication across archives:
 *
 *   Tokens with frequency-sorted dictionary ID < 240:
 *      [ 1 byte ]: The exact dictionary ID representing the token (0 to 239).
 *
 *   Tokens with frequency-sorted dictionary ID < 3824:
 *      [ 2 bytes ]: `<240 to 253> <uint8_t(LSB)>`.
 *      The first byte dedicates 14 statically bound permutation slots to provide 4 bits
 *      (nibble) of spatial MSB data.
 *
 *   Numeric Literals (Bypassing Dictionary):
 *      [ 2-3 bytes ]: `<254> <LEB128_value>`.
 *      Integers up to 32767 are stored as inline LEB128 values rather than dictionary IDs.
 *      This ensures uniform byte sequences across all shaders for optimal LZ77 compression.
 *
 *   Tokens with frequency-sorted dictionary ID >= 3824:
 *      [ 3 bytes ]: `<255> <uint16_t(ID - 3824)>`.
 *      The `255` byte acts as an escape for long-tail string dictionary indexing.
 *
 *
 * Stage-Partitioned Sliding Overlaps:
 * ----------------------------------------------------------------------
 * To hyper-optimize the `0 to 239` subset mappings native to 1-byte encoding, the LineDictionary
 * partitions strings by their respective occurrence boundaries into four sorted buckets:
 *   [0]: Shared (Strings required by MULTIPLE shader stages).
 *   [1]: Vertex (Strings required exclusively by vertex shaders).
 *   [2]: Fragment (Strings required exclusively by fragment shaders).
 *   [3]: Compute (Strings required exclusively by compute shaders).
 *
 * During Binary Encoder/Decoder passes (`MaterialTextChunk` & `MaterialChunk`), indices mapped outside
 * the [0] Shared partition are shifted back towards 0 using simple offsets.
 * For example, a Fragment string at global dictionary index 500 might be referenced as
 * index `500 - [Vertex String Count]` during Fragment mapping block evaluations, pushing it
 * down into a 1-byte schema bounds without risking literal memory duplication.
 *
 * The matching decoder loop reverses this inside `MaterialChunk::getTextShader()`.
 */
class LineDictionary {
public:
    using index_t = uint32_t;

    LineDictionary();
    ~LineDictionary() noexcept;

    // Due to the presence of unique_ptr, disallow copy construction but allow move construction.
    LineDictionary(LineDictionary const&) = delete;
    LineDictionary(LineDictionary&&) = default;

    // Adds text to the dictionary, parsing it into lines.
    void addText(filament::backend::ShaderStage stage, std::string_view text) noexcept;

    // Sorts the dictionary entries by frequency and reassigns their indices
    void resolve() noexcept;

    // Returns the total number of unique lines stored in the dictionary.
    size_t getDictionaryLineCount() const {
        return mStrings.size();
    }

    uint32_t getStageStringCount(size_t stageIndex) const noexcept {
        return mStageStringCounts[stageIndex];
    }

    // Checks if the dictionary is empty.
    bool isEmpty() const noexcept {
        return mStrings.empty();
    }

    // Retrieves a string by its index.
    std::string const& getString(index_t index) const noexcept;



    // Gets the indices and numeric stream for a given registered text block.
    std::pair<std::vector<index_t>, std::vector<index_t>> tokenize(std::string_view text) const noexcept;

    // Prints statistics about the dictionary to the given output stream.
    void printStatistics(utils::io::ostream& stream) const noexcept;

    // conveniences...
    size_t size() const {
        return getDictionaryLineCount();
    }

    bool empty() const noexcept {
        return isEmpty();
    }

    std::string const& operator[](index_t const index) const noexcept {
        return getString(index);
    }

private:
    // Adds a single line to the dictionary.
    void addLine(ShaderStage stage, std::string_view line, std::vector<index_t>& ids, std::vector<index_t>& numerics) noexcept;

    // Trims leading whitespace from a string view.
    static std::string_view ltrim(std::string_view s);

    // Splits a string view into a vector of string views based on delimiters.
    static std::vector<std::string_view> splitString(std::string_view line);

    // Finds a pattern within a string view starting from an offset.
    static std::pair<size_t, size_t> findPattern(std::string_view line, size_t offset);

    struct LineInfo {
        index_t index;
        uint32_t count[3] = {0, 0, 0};
    };

    // Recalculates exact occurrences for each token across stages and discards unused tokens.
    std::vector<std::pair<std::string_view, LineInfo>> calculateAndFilterFrequencies() noexcept;

    // Sorts the surviving tokens into buckets and maps them into their final stage-partitioned offsets.
    void finalizeDictionaryBuckets(std::vector<std::pair<std::string_view, LineInfo>>& info) noexcept;

    struct TokenizedShader {
        ShaderStage stage;
        std::string_view text;
        std::vector<uint32_t> tokens;
        std::vector<uint32_t> numericTokens;
    };

    std::unordered_map<std::string_view, LineInfo> mLineIndices;
    std::vector<std::unique_ptr<std::string>> mStrings;
    uint32_t mStageStringCounts[4] = {0, 0, 0, 0}; // Shared, Vertex, Fragment, Compute
    std::vector<TokenizedShader> mTokenizedShaders;
    std::unordered_map<std::string_view, std::pair<std::vector<uint32_t>, std::vector<uint32_t>>> mFinalTokenizedShadersMap;

};

} // namespace filamat
#endif // TNT_FILAMAT_LINEDICTIONARY_H
