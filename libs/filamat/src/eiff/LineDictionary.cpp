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
#include <algorithm>

namespace filamat {

namespace {
bool isWordChar(char const c) { return std::isalnum(c) || c == '_'; }


// Function to trim specified characters from both ends of a string_view
std::string_view trim(std::string_view sv, std::string_view chars_to_trim = " \t\r\n") {
    // Find the first character that is not in our set of trim characters
    const auto start = sv.find_first_not_of(chars_to_trim);

    // If no such character exists, the string is empty or all trim characters
    if (start == std::string_view::npos) {
        return ""; // Return an empty view
    }

    // Find the last character that is not in our set of trim characters
    const auto end = sv.find_last_not_of(chars_to_trim);

    // Calculate the length of the new view
    const auto len = end - start + 1;

    // Return a new view representing the trimmed string
    return sv.substr(start, len);
}

} // anonymous namespace

LineDictionary::LineDictionary() = default;

LineDictionary::~LineDictionary() noexcept {
    //printStatistics(utils::slog.d);
}

std::string const& LineDictionary::getString(index_t const index) const noexcept {
    return *mStrings[index];
}

std::vector<LineDictionary::index_t> LineDictionary::getIndices(
        std::string_view const& line) const noexcept {
    std::vector<index_t> result;
    std::vector<std::string_view> const sublines = splitString(line);
    for (std::string_view const& subline : sublines) {
        if (auto iter = mLineIndices.find(subline); iter != mLineIndices.end()) {
            result.push_back(iter->second.index);
        } else {
            return {};
        }
    }
    return result;
}

void LineDictionary::addText(std::string_view const text) noexcept {
    utils::slog.e <<"begin addText l(" << text.length() << ")=" <<
            trim(text.substr(0, std::min(15, (int)text.length())));

    size_t cur = 0;
    size_t const len = text.length();
    const char* s = text.data();
    size_t trueCount = 0;
    while (cur < len) {
        trueCount++;
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
        if (trueCount > 1000000 && trueCount > 0) {
            utils::slog.e <<"print addText at=" << trim(text.substr(0, std::min(15, (int)text.length()))) <<
                    " count=" << trueCount << " cur=" << cur << utils::io::endl;
            trueCount = 0;
        }
    }
    utils::slog.e <<" ----- end addText" << utils::io::endl;
}

void LineDictionary::addLine(std::string_view const line) noexcept {
    utils::slog.e <<"********** begin addline l(" << line.length() << ")=" <<
            trim(line.substr(0, std::min(15, (int)line.length())));
    
    auto const lines = splitString(line);
    for (std::string_view const& subline : lines) {
        utils::slog.e <<"addline indices=" << mLineIndices.size() << utils::io::endl;    
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
    utils::slog.e <<"*********** end addline" << utils::io::endl;    
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

    utils::slog.e <<"begin findPattern l(" << line.length() << ")=" <<
            trim(line.substr(0, std::min(15, (int)line.length())));

    if (line.length() == 2) {
        utils::slog.e <<"begin findPattern l[0]=" << (int) line[0] << " l[1]=" << (int) line[1] <<
                
                utils::io::endl;

    }

    size_t trueCount = 1;
    const size_t line_len = line.length();
    for (size_t i = offset; i < line_len; ++i, ++trueCount) {
        // A pattern must be a whole word (or at the start of the string).
        if (i > 0 && isWordChar(line[i - 1])) {
            continue;
        }

        for (const auto& prefix : kPatterns) {
            trueCount++;
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
        if (trueCount % 10000 == 0 && trueCount > 0) {
            utils::slog.e <<"print findPattern at=" << trim(line) <<
                    " count=" << trueCount << utils::io::endl;
        }
    }
    utils::slog.e <<" ----- end findPattern" << utils::io::endl;
    return { std::string_view::npos, 0 }; // No pattern found
}

std::vector<std::string_view> LineDictionary::splitString(std::string_view const line) {
    utils::slog.e <<"begin splitString l=" << trim(line.substr(0,
                    std::min(15, (int)line.length())));

    std::vector<std::string_view> result;
    size_t current_pos = 0;

    if (line.empty()) {
        result.push_back({});
        return result;
    }

    size_t trueCount = 0;
    while (current_pos < line.length()) {
        trueCount++;
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

        if (trueCount % 10000 == 0 && trueCount > 0) {
            utils::slog.e <<"print splitString at=" << trim(line) <<
                    " count=" << trueCount << utils::io::endl;
        }
    }
    utils::slog.e <<"------- end splitString" << utils::io::endl;

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
