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

#include "src/tint/utils/diagnostic/formatter.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/text_style.h"

namespace tint::diag {
namespace {

const char* ToString(Severity severity) {
    switch (severity) {
        case Severity::Note:
            return "note";
        case Severity::Warning:
            return "warning";
        case Severity::Error:
            return "error";
    }
    return "";
}

std::string ToString(const Source::Location& location) {
    StringStream ss;
    if (location.line > 0) {
        ss << location.line;
        if (location.column > 0) {
            ss << ":" << location.column;
        }
    }
    return ss.str();
}

}  // namespace

Formatter::Formatter() {}
Formatter::Formatter(const Style& style) : style_(style) {}

StyledText Formatter::Format(const List& list) const {
    StyledText text;

    bool first = true;
    for (auto diag : list) {
        if (!first) {
            text << "\n";
        }
        Format(diag, text);
        first = false;
    }

    if (style_.print_newline_at_end) {
        text << "\n";
    }

    return text;
}

void Formatter::Format(const Diagnostic& diag, StyledText& text) const {
    auto const& src = diag.source;
    auto const& rng = src.range;

    text << style::Plain;
    TINT_DEFER(text << style::Plain);

    struct TextAndStyle {
        std::string text;
        TextStyle style = {};
    };
    Vector<TextAndStyle, 6> prefix;

    if (style_.print_file && src.file != nullptr) {
        if (rng.begin.line > 0) {
            prefix.Push(TextAndStyle{src.file->path + ":" + ToString(rng.begin)});
        } else {
            prefix.Push(TextAndStyle{src.file->path});
        }
    } else if (rng.begin.line > 0) {
        prefix.Push(TextAndStyle{ToString(rng.begin)});
    }

    if (style_.print_severity) {
        TextStyle style;
        switch (diag.severity) {
            case Severity::Note:
                break;
            case Severity::Warning:
                style = style::Warning + style::Bold;
                break;
            case Severity::Error:
                style = style::Error + style::Bold;
                break;
        }
        prefix.Push(TextAndStyle{ToString(diag.severity), style});
    }

    for (size_t i = 0; i < prefix.Length(); i++) {
        if (i > 0) {
            text << " ";
        }
        text << prefix[i].style << prefix[i].text;
    }

    if (!prefix.IsEmpty()) {
        text << style::Plain(": ");
    }
    text << style::Bold << diag.message;

    if (style_.print_line && src.file && rng.begin.line > 0) {
        text << style::Plain("\n");

        for (size_t line_num = rng.begin.line;
             (line_num <= rng.end.line) && (line_num <= src.file->content.lines.size());
             line_num++) {
            auto& line = src.file->content.lines[line_num - 1];
            auto line_len = line.size();

            bool is_ascii = true;
            for (auto c : line) {
                if (c == '\t') {
                    text.Repeat(' ', style_.tab_width);
                } else {
                    text << c;
                }
                if (c & 0x80) {
                    is_ascii = false;
                }
            }

            text << style::Plain("\n");

            // If the line contains non-ascii characters, then we cannot assume that
            // a single utf8 code unit represents a single glyph, so don't attempt to
            // draw squiggles.
            if (!is_ascii) {
                continue;
            }

            text << style::Squiggle;

            // Count the number of glyphs in the line span.
            // start and end use 1-based indexing.
            auto num_glyphs = [&](size_t start, size_t end) {
                size_t count = 0;
                start = (start > 0) ? (start - 1) : 0;
                end = (end > 0) ? (end - 1) : 0;
                for (size_t i = start; (i < end) && (i < line_len); i++) {
                    count += (line[i] == '\t') ? style_.tab_width : 1;
                }
                return count;
            };

            if (line_num == rng.begin.line && line_num == rng.end.line) {
                // Single line
                text.Repeat(' ', num_glyphs(1, rng.begin.column));
                text.Repeat('^', std::max<size_t>(num_glyphs(rng.begin.column, rng.end.column), 1));
            } else if (line_num == rng.begin.line) {
                // Start of multi-line
                text.Repeat(' ', num_glyphs(1, rng.begin.column));
                text.Repeat('^', num_glyphs(rng.begin.column, line_len + 1));
            } else if (line_num == rng.end.line) {
                // End of multi-line
                text.Repeat('^', num_glyphs(1, rng.end.column));
            } else {
                // Middle of multi-line
                text.Repeat('^', num_glyphs(1, line_len + 1));
            }
            text << style::Plain("\n");
        }
    }
}

Formatter::~Formatter() = default;

}  // namespace tint::diag
