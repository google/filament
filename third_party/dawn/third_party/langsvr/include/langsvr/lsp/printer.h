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

#ifndef LANGSVR_LSP_PRINTER_H_
#define LANGSVR_LSP_PRINTER_H_

#include <ostream>
#include <type_traits>

#include "langsvr/json/builder.h"
#include "langsvr/lsp/lsp.h"
#include "langsvr/result.h"
#include "langsvr/traits.h"

namespace langsvr::lsp {

/// ostream operator << for LSP types, using a temporary JSON encoder.
template <typename T,
          typename = std::enable_if_t<!HasOperatorShiftLeft<std::ostream&, T>>,
          typename = decltype(Encode(std::declval<T>(), std::declval<json::Builder&>()))>
std::ostream& operator<<(std::ostream& stream, const T& object) {
    auto builder = json::Builder::Create();
    if (auto res = Encode(object, *builder); res == Success) {
        return stream << res.Get()->Json();
    } else {
        return stream << res.Failure();
    }
}

}  // namespace langsvr::lsp

#endif  // LANGSVR_LSP_PRINTER_H_
