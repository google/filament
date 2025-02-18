// Copyright 2024 The langsvr Authors
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

#ifndef LANGSVR_LSP_COMPARATORS_H_
#define LANGSVR_LSP_COMPARATORS_H_

#include "langsvr/lsp/lsp.h"

namespace langsvr::lsp {

/// @returns -1 if @p a is before @p b, 1 if @p b is after @p b, 0 if @p a and @p b are equal.
inline int Compare(Position a, Position b) {
    if (a.line < b.line) {
        return -1;
    }
    if (a.line > b.line) {
        return 1;
    }
    if (a.character < b.character) {
        return -1;
    }
    if (a.character > b.character) {
        return 1;
    }
    return 0;
}

/// @returns true if @p a is before @p b
inline bool operator<(Position a, Position b) {
    return Compare(a, b) < 0;
}

/// @returns true if @p a is before or equal to @p b
inline bool operator<=(Position a, Position b) {
    return Compare(a, b) <= 0;
}

/// @returns true if @p a is after @p b
inline bool operator>(Position a, Position b) {
    return Compare(a, b) > 0;
}

/// @returns true if @p a is after or equal to @p b
inline bool operator>=(Position a, Position b) {
    return Compare(a, b) >= 0;
}

/// @returns true if the range `[r.start, r.end)` contains the position @p p
inline bool ContainsExclusive(Range r, Position p) {
    return p >= r.start && p < r.end;
}

/// @returns true if the range `[r.start, r.end]` contains the position @p p
inline bool ContainsInclusive(Range r, Position p) {
    return p >= r.start && p <= r.end;
}

}  // namespace langsvr::lsp

#endif  // LANGSVR_LSP_COMPARATORS_H_
