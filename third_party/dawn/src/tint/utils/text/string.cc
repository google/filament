// Copyright 2022 The Dawn & Tint Authors
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

#include <algorithm>

#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint {

size_t Distance(std::string_view str_a, std::string_view str_b) {
    const auto len_a = str_a.size();
    const auto len_b = str_b.size();

    Vector<size_t, 64> mat;
    mat.Resize((len_a + 1) * (len_b + 1));

    auto at = [&](size_t a, size_t b) -> size_t& { return mat[a + b * (len_a + 1)]; };

    at(0, 0) = 0;
    for (size_t a = 1; a <= len_a; a++) {
        at(a, 0) = a;
    }
    for (size_t b = 1; b <= len_b; b++) {
        at(0, b) = b;
    }
    for (size_t b = 1; b <= len_b; b++) {
        for (size_t a = 1; a <= len_a; a++) {
            bool eq = str_a[a - 1] == str_b[b - 1];
            at(a, b) = std::min({
                at(a - 1, b) + 1,
                at(a, b - 1) + 1,
                at(a - 1, b - 1) + (eq ? 0 : 1),
            });
        }
    }
    return at(len_a, len_b);
}

void SuggestAlternatives(std::string_view got,
                         Slice<const std::string_view> strings,
                         StyledText& ss,
                         const SuggestAlternativeOptions& options /* = {} */) {
    // If the string typed was within kSuggestionDistance of one of the possible enum values,
    // suggest that. Don't bother with suggestions if the string was extremely long.
    auto default_style = ss.Style();
    constexpr size_t kSuggestionDistance = 5;
    constexpr size_t kSuggestionMaxLength = 64;
    if (!got.empty() && got.size() < kSuggestionMaxLength) {
        size_t candidate_dist = kSuggestionDistance;
        std::string_view candidate;
        for (auto str : strings) {
            auto dist = tint::Distance(str, got);
            if (dist < candidate_dist) {
                candidate = str;
                candidate_dist = dist;
            }
        }
        if (!candidate.empty()) {
            ss << "Did you mean " << options.alternatives_style << options.prefix << candidate
               << default_style << "?";
            if (options.list_possible_values) {
                ss << "\n";
            }
        }
    }

    if (options.list_possible_values) {
        // List all the possible enumerator values
        ss << "Possible values: ";
        for (auto str : strings) {
            if (str != strings[0]) {
                ss << ", ";
            }
            ss << options.alternatives_style << options.prefix << str << default_style;
        }
    }
}

}  // namespace tint
