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

#include "LineDictionary.h"

#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace filamat {

LineDictionary::LineDictionary() = default;

LineDictionary::~LineDictionary() noexcept {
    //printStatistics(utils::slog.d);
}

std::string const& LineDictionary::getString(index_t const index) const noexcept {
    return *mStrings[index];
}

size_t LineDictionary::getDictionaryLineCount() const {
    return mStrings.size();
}
std::vector<LineDictionary::index_t> LineDictionary::getIndices(
        std::string_view const& line) const noexcept {
    std::vector<index_t> result;
    std::vector<std::string_view> const sublines = splitString(line);
    for (std::string_view const& subline : sublines) {
        if (auto iter = mLineIndices.find(subline); iter != mLineIndices.end()) {
            result.push_back(iter->second.index);
        }
    }
    return result;
}

void LineDictionary::addText(std::string_view const text) noexcept {
    size_t cur = 0;
    size_t const len = text.length();
    const char* s = text.data();
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
        addLine({ s + pos, cur - pos });
    }
}

void LineDictionary::addLine(std::string_view const line) noexcept {
    auto const lines = splitString(line);
    for (std::string_view const& subline : lines) {
        // Never add a line twice.
        auto pos = mLineIndices.find(subline);
        if (pos != mLineIndices.end()) {
            pos->second.count++;
            continue;
        }
        mStrings.emplace_back(std::make_unique<std::string>(subline));
        mLineIndices.emplace(*mStrings.back(),
                LineInfo{
                    .index = index_t(mStrings.size() - 1),
                    .count = 1 });
    }
}

std::string_view LineDictionary::ltrim(std::string_view s) {
    s.remove_prefix(std::distance(s.begin(), std::find_if(s.begin(), s.end(),
            [](unsigned char const c) { return !std::isspace(c); })));
    return { s.data(), s.size() };
}

std::pair<size_t, size_t> LineDictionary::findPattern(
        std::string_view const line, size_t const offset) {
    // Patterns are ordered from longest to shortest to ensure correct prefix matching.
    static constexpr std::string_view kPatterns[] = { "hp_copy_", "mp_copy_", "_" };

    const size_t line_len = line.length();
    for (size_t i = offset; i < line_len; ++i) {
        // A pattern must be a whole word (or at the start of the string).
        if (i > 0 && std::isalnum(line[i - 1])) {
            continue;
        }

        for (const auto& prefix : kPatterns) {
            if (line.size() - i >= prefix.size() && line.substr(i, prefix.size()) == prefix) {
                // A known prefix has been matched. Now, check for a sequence of digits.
                size_t const startOfDigits = i + prefix.size();
                if (startOfDigits < line_len && std::isdigit(line[startOfDigits])) {
                    size_t j = startOfDigits;
                    while (j < line_len && (j < startOfDigits + 6) && std::isdigit(line[j])) {
                        j++;
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

        // Add the match itself.
        result.push_back(line.substr(match_pos, match_len));

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
        if (lhs.second.count != rhs.second.count) {
            return lhs.second.count > rhs.second.count;
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
        compressed_size += pair.first.length();
        total_size += pair.first.length() * pair.second.count;
        total_lines += pair.second.count;
        indices_size += sizeof(uint16_t) * pair.second.count;
        if (pair.second.index <= 127) {
            indices_size_if_varlen += sizeof(uint8_t) * pair.second.count;
        } else {
            indices_size_if_varlen += sizeof(uint16_t) * pair.second.count;
        }
        if (i <= 128) {
            indices_size_if_varlen_sorted += sizeof(uint8_t) * pair.second.count;
        } else {
            indices_size_if_varlen_sorted += sizeof(uint16_t) * pair.second.count;
        }
        i++;
        stream << "  " << pair.second.count << ": " << pair.first << io::endl;
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

    // some data we gathered

    // Total size: 751161, compressed size: 59818
    // Saved size: 691343
    // Unique lines: 3659
    // Total lines: 61686
    // Compression ratio: 12.557440904075696
    // Average line length (total): 12.177171481373406
    // Average line length (compressed): 16.34818256354195


    // Total size: 751161, compressed size: 263215
    // Saved size: 487946
    // Unique lines: 4672
    // Total lines: 23258
    // Compression ratio: 2.8537925270216364
    // Average line length (total): 32.296887092613296
    // Average line length (compressed): 56.338827054794521
}

} // namespace filamat
