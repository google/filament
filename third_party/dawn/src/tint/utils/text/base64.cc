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

#include "src/tint/utils/text/base64.h"

#include <array>
#include <string>

#include "src/tint/utils/result.h"

namespace tint {

Vector<std::byte, 0> DecodeBase64FromComments(std::string_view wgsl) {
    Vector<std::byte, 0> out;
    size_t block_nesting = 0;
    bool line_comment = false;
    for (size_t i = 0, n = wgsl.length(); i < n; i++) {
        char curr = wgsl[i];
        if (curr == '\n') {
            line_comment = false;
            continue;
        }

        char next = (i + 1) < n ? wgsl[i + 1] : 0;
        if (curr == '/' && next == '*') {
            block_nesting++;
            i++;  // skip '*'
            continue;
        }
        if (block_nesting > 0 && curr == '*' && next == '/') {
            block_nesting--;
            i++;  // skip '/'
            continue;
        }
        if (block_nesting == 0 && curr == '/' && next == '/') {
            line_comment = true;
            i++;  // skip '/'
            continue;
        }

        if (block_nesting > 0 || line_comment) {
            if (auto v = DecodeBase64(curr)) {
                out.Push(std::byte{*v});
            }
        }
    }
    return out;
}

Result<std::string> EncodeBase64(std::span<const std::byte> data) {
    constexpr std::array kMap{
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
    };
    std::string out;
    out.reserve(data.size());
    for (std::byte b : data) {
        const auto val = static_cast<uint8_t>(b);
        if (val >= kMap.size()) {
            return Failure{"byte value " + std::to_string(val) + " is out of range [0, 63]"};
        }
        out.push_back(kMap[val]);
    }
    return out;
}

}  // namespace tint
