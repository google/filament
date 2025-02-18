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

#include "src/tint/lang/wgsl/ls/helpers_test.h"

#include <utility>

namespace tint::wgsl::ls {

namespace lsp = langsvr::lsp;

ParsedMarkers ParseMarkers(std::string_view str) {
    std::stringstream clean;
    lsp::Position current_position;
    std::vector<langsvr::lsp::Position> positions;
    std::vector<langsvr::lsp::Range> ranges;
    std::optional<langsvr::lsp::Range> current_range;
    while (!str.empty()) {
        auto [codepoint, len] =
            utf8::Decode(reinterpret_cast<const uint8_t*>(str.data()), str.length());
        if (codepoint == 0 || len == 0) {
            break;
        }

        switch (codepoint) {
            case '\n':
                current_position.line++;
                current_position.character = 0;
                clean << "\n";
                break;
            case U"「"[0]:
                // Range start. Replace with ' '
                current_position.character++;
                current_range = lsp::Range{};
                current_range->start = current_position;
                clean << ' ';
                break;
            case U"」"[0]:
                // Range end. Replace with ' '
                if (current_range) {
                    current_range->end = current_position;
                    ranges.push_back(*current_range);
                    current_range.reset();
                }
                clean << ' ';
                current_position.character++;
                break;
            case U"⧘"[0]:
                // Position. Consume
                positions.push_back(current_position);
                break;
            default:
                clean << str.substr(0, len);
                current_position.character++;
                break;
        }
        str = str.substr(len);
    }
    return ParsedMarkers{std::move(ranges), std::move(positions), clean.str()};
}

}  // namespace tint::wgsl::ls
