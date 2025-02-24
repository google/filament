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

#ifndef SRC_TINT_UTILS_TEXT_BASE64_H_
#define SRC_TINT_UTILS_TEXT_BASE64_H_

#include <cstdint>
#include <optional>

#include "src/tint/utils/containers/vector.h"

namespace tint {

/// Decodes a byte from a base64 encoded character
/// @param c the character to decode
/// @return the decoded value, or std::nullopt if the character is padding ('=') or an invalid
/// character.
inline std::optional<uint8_t> DecodeBase64(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<uint8_t>(c - 'A');
    }
    if (c >= 'a' && c <= 'z') {
        return static_cast<uint8_t>(26 + c - 'a');
    }
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(52 + c - '0');
    }
    if (c == '+') {
        return 62;
    }
    if (c == '/') {
        return 63;
    }
    return std::nullopt;
}
/// DecodeBase64FromComments parses all the comments from the WGSL source string as a base64 byte
/// stream. Non-base64 characters are skipped
/// @param wgsl the WGSL source
/// @return the base64 decoded bytes
Vector<std::byte, 0> DecodeBase64FromComments(std::string_view wgsl);

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_BASE64_H_
