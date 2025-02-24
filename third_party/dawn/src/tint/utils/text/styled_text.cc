// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/utils/text/styled_text.h"
#include <string_view>
#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/text_style.h"

namespace tint {

StyledText::StyledText() = default;

StyledText::StyledText(const StyledText&) = default;

StyledText::StyledText(const std::string& text) {
    stream_ << text;
}

StyledText::StyledText(std::string_view text) {
    stream_ << text;
}

StyledText::StyledText(StyledText&&) = default;

StyledText& StyledText::operator=(const StyledText& other) = default;

StyledText& StyledText::operator=(std::string_view text) {
    Clear();
    return *this << text;
}

void StyledText::Clear() {
    *this = StyledText{};
}

StyledText& StyledText::SetStyle(TextStyle style) {
    if (spans_.Back().style == style) {
        return *this;
    }
    if (spans_.Back().length == 0) {
        spans_.Pop();
    }
    if (spans_.IsEmpty() || spans_.Back().style != style) {
        spans_.Push(Span{style});
    }
    return *this;
}

std::string StyledText::Plain() const {
    StringStream ss;
    bool is_code_no_quote = false;
    Walk([&](std::string_view text, TextStyle style) {
        if (is_code_no_quote != (style.IsCode() && !style.IsNoQuote())) {
            ss << "'";
        }
        is_code_no_quote = style.IsCode() && !style.IsNoQuote();

        ss << text;
    });
    if (is_code_no_quote) {
        ss << "'";
    }
    return ss.str();
}

void StyledText::Append(const StyledText& other) {
    other.Walk([&](std::string_view text, TextStyle style) { *this << style << text; });
}

StyledText& StyledText::Repeat(char c, size_t n) {
    stream_.repeat(c, n);
    spans_.Back().length += n;
    return *this;
}

}  // namespace tint
