// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/diagnostic/source.h"

#include <algorithm>
#include <string_view>
#include <utility>

#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/unicode.h"

namespace tint {
namespace {

bool ParseLineBreak(std::string_view str, size_t i, bool* is_line_break, size_t* line_break_size) {
    // See https://www.w3.org/TR/WGSL/#blankspace

    auto [cp, n] = tint::utf8::Decode(str.substr(i));

    if (n == 0) {
        return false;
    }

    static const auto kLF = tint::CodePoint(0x000A);    // line feed
    static const auto kVTab = tint::CodePoint(0x000B);  // vertical tab
    static const auto kFF = tint::CodePoint(0x000C);    // form feed
    static const auto kNL = tint::CodePoint(0x0085);    // next line
    static const auto kCR = tint::CodePoint(0x000D);    // carriage return
    static const auto kLS = tint::CodePoint(0x2028);    // line separator
    static const auto kPS = tint::CodePoint(0x2029);    // parargraph separator

    if (cp == kLF || cp == kVTab || cp == kFF || cp == kNL || cp == kPS || cp == kLS) {
        *is_line_break = true;
        *line_break_size = n;
        return true;
    }

    // Handle CRLF as one line break, and CR alone as one line break
    if (cp == kCR) {
        *is_line_break = true;
        *line_break_size = n;

        if (auto next_i = i + n; next_i < str.size()) {
            auto [next_cp, next_n] = tint::utf8::Decode(str.substr(next_i));

            if (next_n == 0) {
                return false;
            }

            if (next_cp == kLF) {
                // CRLF as one break
                *line_break_size = n + next_n;
            }
        }

        return true;
    }

    *is_line_break = false;
    return true;
}

std::vector<Source::FileContent::LineRange> SplitLines(std::string_view str) {
    std::vector<Source::FileContent::LineRange> lines;

    size_t lineStart = 0;
    for (size_t i = 0; i < str.size();) {
        bool is_line_break{};
        size_t line_break_size{};
        // We don't handle decode errors from ParseLineBreak. Instead, we rely on
        // the Lexer to do so.
        ParseLineBreak(str, i, &is_line_break, &line_break_size);
        if (is_line_break) {
            lines.push_back({.start = lineStart, .length = i - lineStart});
            i += line_break_size;
            lineStart = i;
        } else {
            ++i;
        }
    }
    if (lineStart < str.size()) {
        lines.push_back({.start = lineStart, .length = str.size() - lineStart});
    }

    return lines;
}

}  // namespace

Source::FileContent::FileContent(std::string_view body)
    : data(body), line_ranges(SplitLines(data)) {}

Source::FileContent::FileContent(const FileContent& rhs)
    : data(rhs.data), line_ranges(rhs.line_ranges) {}

Source::FileContent::~FileContent() = default;

std::string_view Source::FileContent::GetLine(size_t n) const {
    TINT_ASSERT(n < line_ranges.size());
    return std::string_view(data).substr(line_ranges[n].start, line_ranges[n].length);
}

size_t Source::FileContent::GetLineCount() const {
    return line_ranges.size();
}

Source::File::~File() = default;

std::string ToString(const Source& source) {
    StringStream out;

    auto rng = source.range;

    if (source.file) {
        out << source.file->path << ":";
    }
    if (rng.begin.line) {
        out << rng.begin.line << ":";
        if (rng.begin.column) {
            out << rng.begin.column;
        }

        if (source.file) {
            out << "\n\n";

            auto repeat = [&](char c, size_t n) {
                while (n--) {
                    out << c;
                }
            };

            for (size_t line = rng.begin.line; line <= rng.end.line; line++) {
                if (line <= source.file->content.GetLineCount()) {
                    auto len = source.file->content.GetLine(line - 1).size();

                    out << source.file->content.GetLine(line - 1) << "\n";

                    if (line == rng.begin.line && line == rng.end.line) {
                        // Single line
                        repeat(' ', rng.begin.column - 1);
                        repeat('^', std::max<size_t>(rng.end.column - rng.begin.column, 1));
                    } else if (line == rng.begin.line) {
                        // Start of multi-line
                        repeat(' ', rng.begin.column - 1);
                        repeat('^', len - (rng.begin.column - 1));
                    } else if (line == rng.end.line) {
                        // End of multi-line
                        repeat('^', rng.end.column - 1);
                    } else {
                        // Middle of multi-line
                        repeat('^', len);
                    }

                    out << "\n";
                }
            }
        }
    }
    return out.str();
}

size_t Source::Range::Length(const FileContent& content) const {
    TINT_ASSERT(begin <= end);
    TINT_ASSERT(begin.column > 0);
    TINT_ASSERT(begin.line > 0);
    TINT_ASSERT(end.line <= 1 + content.GetLineCount());
    TINT_ASSERT(end.column <= 1 + content.GetLine(end.line - 1).size());

    if (end.line == begin.line) {
        return end.column - begin.column;
    }

    size_t len = (content.GetLine(begin.line - 1).size() + 1 - begin.column) +  // first line
                 (end.column - 1) +                                             // last line
                 end.line - begin.line;                                         // newlines

    for (size_t line = begin.line + 1; line < end.line; line++) {
        len += content.GetLine(line - 1).size();  // whole-lines
    }
    return len;
}

}  // namespace tint
