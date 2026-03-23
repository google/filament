/*
 * Copyright (C) 2025 The Android Open Source Project
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

/*
 * SHADER DICTIONARY ENCODING ARCHITECTURE
 * ---------------------------------------
 * The LineDictionary compresses raw shader source text into an optimized
 * dictionary-based token stream. It specifically targets the redundancy found in
 * generated materials, such as monolithic ubershaders (e.g. GLTFIO), where
 * recurring tokens and numbered suffix identifiers (like 'param_1', 'param_2')
 * dominate the text length.
 *
 * Tokenization and Splitting Algorithm:
 * To maximize dictionary reusability without polluting the global token pool with
 * thousands of unique permutations, the parser automatically splits pure numerical
 * digit strings out of the text (e.g. "param_112" -> "param_" + "112").
 *
 * Triple-Stream Topology (Base + String Extension + Numeric Literal):
 * Achieving the lowest uncompressed RAM footprint requires
 * aggressively splitting strings based on a broad set of keywords. However, ZLib
 * (used in Android `.aar` packaging) inherently struggles with fragmented token
 * layouts. When strings are highly fragmented, ZLib loses predictive "sliding
 * window" sequence continuity. Generating unique String Dictionary ID hashes for 
 * every dynamic numerical variable prevents cross-shader Zlib ZIP deduplication.
 *
 * To balance both targets, the String splitting algorithm pushes extension 
 * and numeric data out of the core base representation. Zstandard compresses 
 * the remaining high-entropy base bytes cleanly.
 *
 *   [Base Stream]  (High entropy identifiers, densely packed 1-byte variables)
 *   ┌────┬────┬────┬──────┬──────┬────┬─────┐
 *   │ 43 │ 12 │ 08 │ ESC1 │ 0xFF │ 15 │ NUM │ => (43, 12, 08, ESC1+01, 0xFF+031A, 15, NUM:152)
 *   └────┴────┴────┴──────┴──────┴────┴─────┘
 *                    │      │           │    ESC1  = [240-253] (1-byte String Ext)
 *                    ▼      ▼           │    NUM   = [254]     (Numeric Escape)
 *   [Ext Stream]   ┌────┐ ┌────┬────┐   │    0xFF  = [255]     (2-byte String Ext)
 *                  │ 01 │ │ 03 │ 1A │   ▼
 *                  └────┘ └────┴────┘
 *   [Num Stream]                      ┌─────────┐
 *                                     │ 152 [N] │ (Integers 15-bit LEB128 layout)
 *                                     └─────────┘  N < 128: 1 byte, N >= 128: 2 bytes
 *
 * By isolating integers into a static array, LZ77 can match identical values
 * like `var_1024` and `other_var_1024` efficiently across the entire archive.
 *
 * Stage-Partitioned Bucket Encoding (1-Byte Collapse):
 * ----------------------------------------------------
 * To hyper-optimize the `0 to 239` limits of the 1-byte base representation, the global
 * string dictionary is partitioned into 4 distinct stage buckets:
 *
 *   [ Dictionary Array Mappings ]
 *   ┌──────────────────┐ ◀─── 0: (S) Shared Strings (multi-stage)
 *   │      Shared      │
 *   ├──────────────────┤ ◀─── S: (V) Vertex Strings
 *   │      Vertex      │
 *   ├──────────────────┤ ◀─── S+V: (F) Fragment Strings
 *   │     Fragment     │
 *   ├──────────────────┤ ◀─── S+V+F: (C) Compute Strings
 *   │     Compute      │
 *   └──────────────────┘
 *
 * During Material encoding, any string isolated to a single pipeline
 * collapses its offset value by bypassing preceding buckets:
 *
 *   [ Fragment Shader Encoding Example ]:
 *      Local_Index = (Global_Index >= S) ? (Global_Index - V) : Global_Index
 *
 * This guarantees the majority of isolated strings drop back into
 * the dense 1-byte target limits without duplicating memory!
 */

#include "LineDictionary.h"

#include <private/filament/LineDictionaryUtils.h>

#include <utils/ostream.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <charconv>
#include <utility>
#include <vector>

namespace filamat {

/*
 * Note on String Splitting & Dictionary Optimization
 * --------------------------------------------------
 * The array below defines heuristics to split generated variables from their trailing digits
 * (e.g. `hp_copy_1` -> `hp_copy_`, `1`). We mined the full compiled corpus of
 * gltfio/filament materials to find the most common variable prefixes ending in digits that
 * break otherwise identical lines in spirv-cross output. The optimal top results are:
 *
 * "SPIRV_CROSS_CONSTANT_ID_", "VARIABLE_CUSTOM", "spvDescriptorSet", "HAS_ATTRIBUTE_UV",
 * "mesh_custom", "dynReserved", "material_", "vertex_uv", "hp_copy_", "mp_copy_", "normal_",
 * "normal", "pixel_", "param_", "mesh_uv", "Arr_", "uv_", "n_", "x_", "i_", "uv", "f",
 * "s", "i", "r", "p", "u", "a", "n", "m", "x", "_"
 *
 * While hardcoding all 32 discovered prefixes reduces uncompressed binaries by ~33 KB across the library,
 * the aggressive fragmentation of tiny, highly localized variables (like `s1` or `f2`)
 * destructs contiguous literal sequences. This causes Zlib/Deflate (LZ77) compressed `.aar` and `.bin`
 * archive sizes to balloon by ~2-3 KB.
 *
 * Therefore, we restrict this list to an optimal subset, maximizing ZIP synergy.
 * Patterns must be ordered from longest to shortest to ensure correct prefix matching.
 */
static constexpr std::string_view kSplittingPatterns[] = {"hp_copy_", "mp_copy_", "_"};

using namespace ::filament::backend;

namespace {

    bool isWordChar(char const c) {
        // Note: isalnum is locale-dependent, which can be problematic.
        // For our purpose, we define word characters as ASCII alphanumeric characters plus underscore.
        // This is safe for UTF-8 strings, as any byte of a multi-byte character will not be
        // in these ranges.
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    }
} // anonymous namespace

LineDictionary::LineDictionary() = default;

LineDictionary::~LineDictionary() noexcept {
    //printStatistics(utils::slog.d);
}

std::string const& LineDictionary::getString(index_t const index) const noexcept {
    return *mStrings[index];
}

std::pair<std::vector<LineDictionary::index_t>, std::vector<LineDictionary::index_t>> LineDictionary::tokenize(
        std::string_view const text) const noexcept {
    auto const it = mFinalTokenizedShadersMap.find(text);
    if (it != mFinalTokenizedShadersMap.end()) {
        return it->second;
    }
    return {};
}

void LineDictionary::addText(ShaderStage stage, std::string_view const text) noexcept {
    mTokenizedShaders.push_back({stage, text, {}, {}});
    
    size_t pos = 0;
    while (pos < text.length()) {
        size_t const start = pos;
        while (pos < text.length() && text[pos] != '\n') pos++;
        size_t const len = (pos < text.length()) ? (pos - start + 1) : (text.length() - start);
        addLine(stage, text.substr(start, len), mTokenizedShaders.back().tokens, mTokenizedShaders.back().numericTokens);
        if (pos < text.length()) pos++;
    }
}


void LineDictionary::addLine(ShaderStage stage, std::string_view const line, std::vector<index_t>& ids, std::vector<index_t>& numerics) noexcept {
    auto const lines = splitString(line);
    for (std::string_view const& subline : lines) {
        bool isNumeric = !subline.empty() && std::all_of(subline.begin(), subline.end(), ::isdigit);
        if (isNumeric) {
            if (subline.data() > line.data() && *(subline.data() - 1) == '_') {
                // keep true
            } else {
                isNumeric = false;
            }
        }
        
        uint32_t numValue = 0;
        bool parsedNumeric = false;
        if (isNumeric) {
            auto const result = std::from_chars(subline.data(), subline.data() + subline.size(), numValue);
            if (result.ec == std::errc() && numValue <= 32767) {
                parsedNumeric = true;
            }
        }

        if (parsedNumeric) {
            ids.push_back(filament::LineDictionaryUtils::DICTIONARY_NUMERIC_FLAG);
            numerics.push_back(numValue);
            continue;
        }

        index_t id;
        auto pos = mLineIndices.find(subline);
        if (pos != mLineIndices.end()) {
            pos->second.count[size_t(stage)]++;
            id = pos->second.index;
        } else {
            mStrings.emplace_back(std::make_unique<std::string>(subline));
            LineInfo info = { .index = index_t(mStrings.size() - 1) };
            info.count[size_t(stage)] = 1;
            mLineIndices.emplace(*mStrings.back(), info);
            id = info.index;
        }

        ids.push_back(id);
    }
}

std::string_view LineDictionary::ltrim(std::string_view s) {
    s.remove_prefix(std::distance(s.begin(), std::find_if(s.begin(), s.end(),
            [](unsigned char const c) { return !std::isspace(c); })));
    return { s.data(), s.size() };
}

std::pair<size_t, size_t> LineDictionary::findPattern(
        std::string_view const line, size_t const offset) {
    const size_t line_len = line.length();
    for (size_t i = offset; i < line_len; ++i) {
        // A pattern must be a whole word (or at the start of the string).
        if (i > 0 && isWordChar(line[i - 1])) {
            continue;
        }

        for (const auto& prefix : kSplittingPatterns) {
            if (line.size() - i >= prefix.size() && line.substr(i, prefix.size()) == prefix) {
                // A known prefix has been matched. Now, check for a sequence of digits.
                size_t const startOfDigits = i + prefix.size();
                if (startOfDigits < line_len && std::isdigit(line[startOfDigits])) {
                    size_t j = startOfDigits;
                    while (j < line_len && (j < startOfDigits + 6) && std::isdigit(line[j])) {
                        j++;
                    }
                    // We have a potential match (prefix + digits).
                    // Check if the character after the match is also a word character.
                    if (j < line_len && isWordChar(line[j])) {
                        // It is, so this is not a valid boundary. Continue searching.
                        break; // breaks from the inner loop (kPatterns)
                    }
                    // We have a full pattern match (prefix + digits).
                    return { i, j - i };
                }
                // If a prefix is matched but not followed by digits, it's not a valid pattern.
                // We break to the outer loop to continue searching from the next character,
                // because we've already checked the longest possible prefix at this position.
                break;
            }
        }
    }
    return { std::string_view::npos, 0 }; // No pattern found
}

std::vector<std::string_view> LineDictionary::splitString(std::string_view const line) {
    std::vector<std::string_view> result;
    size_t current_pos = 0;

    if (line.empty()) {
        result.push_back({});
        return result;
    }

    while (current_pos < line.length()) {
        auto const [match_pos, match_len] = findPattern(line, current_pos);

        if (match_pos == std::string_view::npos) {
            // No more patterns found, add the rest of the string.
            result.push_back(line.substr(current_pos));
            break;
        }

        // Add the part before the match.
        if (match_pos > current_pos) {
            result.push_back(line.substr(current_pos, match_pos - current_pos));
        }

        // Add the match itself, but split prefix from numbers.
        for (const auto& prefix : kSplittingPatterns) {
            if (match_len >= prefix.size() && line.substr(match_pos, prefix.size()) == prefix) {
                std::string_view const prefixWithoutUnderscore = line.substr(match_pos, prefix.size() - 1);
                if (!prefixWithoutUnderscore.empty()) {
                    result.push_back(prefixWithoutUnderscore);
                }
                result.push_back(line.substr(match_pos + prefix.size(), match_len - prefix.size()));
                break;
            }
        }

        // Move cursor past the match.
        current_pos = match_pos + match_len;
    }

    return result;
}



void LineDictionary::printStatistics(utils::io::ostream& stream) const noexcept {
    std::vector<std::pair<std::string_view, LineInfo>> info;
    for (auto const& pair : mLineIndices) {
        info.push_back(pair);
    }

    // Sort by count, then by index.
    std::sort(info.begin(), info.end(),
            [](auto const& lhs, auto const& rhs) {
        uint32_t const lhsTotal = lhs.second.count[0] + lhs.second.count[1] + lhs.second.count[2];
        uint32_t const rhsTotal = rhs.second.count[0] + rhs.second.count[1] + rhs.second.count[2];
        if (lhsTotal != rhsTotal) {
            return lhsTotal > rhsTotal;
        }
        return lhs.second.index < rhs.second.index;
    });

    size_t total_size = 0;
    size_t compressed_size = 0;
    size_t total_lines = 0;
    size_t indices_size = 0;
    size_t indices_size_if_varlen = 0;
    size_t indices_size_if_varlen_sorted = 0;
    size_t i = 0;
    using namespace utils;
    // Print the dictionary.
    stream << "Line dictionary:" << io::endl;
    for (auto const& pair : info) {
        uint32_t const totalCount = pair.second.count[0] + pair.second.count[1] + pair.second.count[2];
        compressed_size += pair.first.length();
        total_size += pair.first.length() * totalCount;
        total_lines += totalCount;
        indices_size += sizeof(uint16_t) * totalCount;
        if (pair.second.index <= 127) {
            indices_size_if_varlen += sizeof(uint8_t) * totalCount;
        } else {
            indices_size_if_varlen += sizeof(uint16_t) * totalCount;
        }
        if (i <= 128) {
            indices_size_if_varlen_sorted += sizeof(uint8_t) * totalCount;
        } else {
            indices_size_if_varlen_sorted += sizeof(uint16_t) * totalCount;
        }
        i++;
        stream << "  " << totalCount << ": " << pair.first << io::endl;
    }
    stream << "Total size: " << total_size << ", compressed size: " << compressed_size << io::endl;
    stream << "Saved size: " << total_size - compressed_size << io::endl;
    stream << "Unique lines: " << mLineIndices.size() << io::endl;
    stream << "Total lines: " << total_lines << io::endl;
    stream << "Compression ratio: " << double(total_size) / compressed_size << io::endl;
    stream << "Average line length (total): " << double(total_size) / total_lines << io::endl;
    stream << "Average line length (compressed): " << double(compressed_size) / mLineIndices.size() << io::endl;
    stream << "Indices size: " << indices_size << io::endl;
    stream << "Indices size (if varlen): " << indices_size_if_varlen << io::endl;
    stream << "Indices size (if varlen, sorted): " << indices_size_if_varlen_sorted << io::endl;
}

// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// calculateAndFilterFrequencies()
// Analysis to determine the exact isolated occurrences of each surviving String token.
//
// Input/Dependencies:
//    - `mTokenizedShaders`: Read to tally exact occurrences across stages.
//    - `mLineIndices`:      LineInfo counts are zeroed and then overwritten.
//
// Output:
//    - Returns `std::vector<LineInfo>` containing ONLY the strings with `total > 0` hits.
//      Returns this vector rather than mutating state directly to prepare for downstream sorting.
// ------------------------------------------------------------------------------------------------
std::vector<std::pair<std::string_view, LineDictionary::LineInfo>> LineDictionary::calculateAndFilterFrequencies() noexcept {
    // Rebuild exact occurrences across the final optimized token arrays
    for (auto& entry : mLineIndices) {
        entry.second.count[0] = 0;
        entry.second.count[1] = 0;
        entry.second.count[2] = 0;
    }

    for (const auto& shader : mTokenizedShaders) {
        ShaderStage const st = shader.stage;
        size_t st_idx = 0;
        if (st == ShaderStage::VERTEX) st_idx = 0;
        else if (st == ShaderStage::FRAGMENT) st_idx = 1;
        else if (st == ShaderStage::COMPUTE) st_idx = 2;

        for (uint32_t const id : shader.tokens) {
            if (id & filament::LineDictionaryUtils::DICTIONARY_NUMERIC_FLAG) continue;
            const auto& str = *mStrings[id];
            auto it = mLineIndices.find(str);
            if (it != mLineIndices.end()) {
                it->second.count[st_idx]++;
            }
        }
    }

    std::vector<std::pair<std::string_view, LineInfo>> info;
    info.reserve(mLineIndices.size());
    for (auto const& pair : mLineIndices) {
        uint32_t const total = pair.second.count[0] + pair.second.count[1] + pair.second.count[2];
        if (total > 0) {
            info.push_back(pair);
        }
    }
    return info;
}

// ------------------------------------------------------------------------------------------------
// finalizeDictionaryBuckets(info)
// Maps the final sorted tokens into their stage-partitioned subsets (Shared, Vertex, Fragment, Compute),
// resolving the absolute 1-byte schema boundaries for downstream chunk encoding.
//
// Input/Dependencies:
//    - `info`:           The filtered and occurrence-populated vector from `calculateAndFilterFrequencies`.
//    - `mStrings`:       Will be stripped of abandoned subsumed tokens and densely repacked.
//    - `mLineIndices`:   Updates the index mappings for runtime lookup scaling.
//
// Output/Modifications:
//    - `mStageStringCounts`:         Records the exact boundary limits per pipeline stage constraint.
//    - `mFinalTokenizedShadersMap`:  Populates the resolved binary integer sequence mapping for the encoder.
// ------------------------------------------------------------------------------------------------
void LineDictionary::finalizeDictionaryBuckets(std::vector<std::pair<std::string_view, LineInfo>>& info) noexcept {
    // Step A: Define the closure that categorizes Strings into 4 distinct groups:
    // [0] Shared: Appears in > 1 Pipeline Stage
    // [1] Vertex, [2] Fragment, [3] Compute (Isolated locally to that explicit pipeline)
    auto getBucketIndex = [](const uint32_t count[3]) -> uint8_t {
        uint32_t const v = count[size_t(ShaderStage::VERTEX)];
        uint32_t const f = count[size_t(ShaderStage::FRAGMENT)];
        uint32_t const c = count[size_t(ShaderStage::COMPUTE)];
        int const sharedClasses = (v > 0) + (f > 0) + (c > 0);
        if (sharedClasses > 1) return 0;
        if (v > 0) return 1;
        if (f > 0) return 2;
        if (c > 0) return 3;
        return 0; // fallback
    };

    // Step B: Sort the surviving vocabulary tokens.
    // 1st Priority: Stage Bucket (Shared strings go first, then Vertex, Fragment, Compute)
    //               This ensures we can offset isolated indices.
    // 2nd Priority: Occurrence count globally. The most common tokens must map to 
    //               0-239 to compress into a single variable-length byte.
    // 3rd Priority: Original parse token allocation order to enforce stable deterministic builds.
    std::sort(info.begin(), info.end(),
            [&getBucketIndex](auto const& lhs, auto const& rhs) {
        uint8_t const lhsBucket = getBucketIndex(lhs.second.count);
        uint8_t const rhsBucket = getBucketIndex(rhs.second.count);
        if (lhsBucket != rhsBucket) {
            return lhsBucket < rhsBucket;
        }

        uint32_t const lhsTotal = lhs.second.count[0] + lhs.second.count[1] + lhs.second.count[2];
        uint32_t const rhsTotal = rhs.second.count[0] + rhs.second.count[1] + rhs.second.count[2];

        if (lhsTotal != rhsTotal) {
            return lhsTotal > rhsTotal;
        }
        return lhs.second.index < rhs.second.index;
    });

    for (int i = 0; i < 4; i++) {
        mStageStringCounts[i] = 0;
    }

    // Step C: Repackage the physical mStrings layout!
    // We map every old string ID layout sequentially into the new partitioned layout mappings.
    // Discarded (0-occurrence) substrings are bypassed here and cleared off RAM matrix.
    std::vector<uint32_t> initialToFinal(mStrings.size(), 0xFFFFFFFF);
    std::vector<std::unique_ptr<std::string>> newStrings;
    newStrings.reserve(info.size());
    
    for (index_t i = 0; i < info.size(); i++) {
        uint8_t const bucket = getBucketIndex(info[i].second.count);
        mStageStringCounts[bucket]++; // Log boundary bounds for `MaterialTextChunk` metadata block

        auto& entry = mLineIndices[info[i].first];
        initialToFinal[entry.index] = newStrings.size(); // Map Old ID -> New ID
        newStrings.push_back(std::move(mStrings[entry.index]));
        entry.index = newStrings.size() - 1;
    }
    mStrings = std::move(newStrings);

    // Step D: Encode the final uncompressed Sequence index Array.
    // Loop through our shader text blocks one last time and map the optimized tokens tightly.
    for (auto& shader : mTokenizedShaders) {
        std::vector<uint32_t> final_sequence;
        final_sequence.reserve(shader.tokens.size());
        
        for (uint32_t const init_id : shader.tokens) {
            if (init_id & filament::LineDictionaryUtils::DICTIONARY_NUMERIC_FLAG) {
                final_sequence.push_back(init_id);
            } else {
                uint32_t const final_id = initialToFinal[init_id];
                if (final_id != 0xFFFFFFFF) {
                    final_sequence.push_back(final_id);
                }
            }
        }
        mFinalTokenizedShadersMap.emplace(shader.text, std::make_pair(std::move(final_sequence), std::move(shader.numericTokens)));
    }
}

void LineDictionary::resolve() noexcept {
    auto info = calculateAndFilterFrequencies();
    finalizeDictionaryBuckets(info);
}

} // namespace filamat
