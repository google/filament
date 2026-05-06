// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/utils/text/string_stream.h"

namespace tint {

StringStream::StringStream() {
    Reset();
}

StringStream::StringStream(const StringStream& other) {
    Reset();
    sstream_ << other.str();
}

StringStream::~StringStream() = default;

StringStream& StringStream::operator=(const StringStream& other) {
    Reset();
    return *this << other.str();
}

void StringStream::Reset() {
    sstream_.str("");
    sstream_.clear();
    sstream_.flags(sstream_.flags() | std::ios_base::showpoint | std::ios_base::fixed);
    sstream_.imbue(std::locale::classic());
    sstream_.precision(9);
}

StringStream& operator<<(StringStream& out, CodePoint code_point) {
    if (code_point < 0x7f) {
        // See https://en.cppreference.com/w/cpp/language/escape
        switch (code_point) {
            case '\a':
                return out << R"('\a')";
            case '\b':
                return out << R"('\b')";
            case '\f':
                return out << R"('\f')";
            case '\n':
                return out << R"('\n')";
            case '\r':
                return out << R"('\r')";
            case '\t':
                return out << R"('\t')";
            case '\v':
                return out << R"('\v')";
        }
        return out << "'" << static_cast<char>(code_point) << "'";
    }
    return out << "'U+" << std::hex << code_point.value << "'";
}

}  // namespace tint
