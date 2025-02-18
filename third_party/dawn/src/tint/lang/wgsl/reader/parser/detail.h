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

#ifndef SRC_TINT_LANG_WGSL_READER_PARSER_DETAIL_H_
#define SRC_TINT_LANG_WGSL_READER_PARSER_DETAIL_H_

#include <memory>

namespace tint::wgsl::reader::detail {

/// OperatorArrow is a traits helper for ParserImpl::Expect<T>::operator->() and
/// ParserImpl::Maybe<T>::operator->() so that pointer types are automatically
/// dereferenced. This simplifies usage by allowing
///  `result.value->field`
/// to be written as:
///  `result->field`
/// As well as reducing the amount of code, using the operator->() asserts that
/// the Expect<T> or Maybe<T> is not in an error state before dereferencing.
template <typename T>
struct OperatorArrow {
    /// type resolves to the return type for the operator->()
    using type = T*;
    /// @param val the value held by `ParserImpl::Expect<T>` or
    /// `ParserImpl::Maybe<T>`.
    /// @return a pointer to `val`
    static inline T* ptr(T& val) { return &val; }
};

/// OperatorArrow template specialization for std::unique_ptr<>.
template <typename T>
struct OperatorArrow<std::unique_ptr<T>> {
    /// type resolves to the return type for the operator->()
    using type = T*;
    /// @param val the value held by `ParserImpl::Expect<T>` or
    /// `ParserImpl::Maybe<T>`.
    /// @return the raw pointer held by `val`.
    static inline T* ptr(std::unique_ptr<T>& val) { return val.get(); }
};

/// OperatorArrow template specialization for T*.
template <typename T>
struct OperatorArrow<T*> {
    /// type resolves to the return type for the operator->()
    using type = T*;
    /// @param val the value held by `ParserImpl::Expect<T>` or
    /// `ParserImpl::Maybe<T>`.
    /// @return `val`.
    static inline T* ptr(T* val) { return val; }
};

}  // namespace tint::wgsl::reader::detail

#endif  // SRC_TINT_LANG_WGSL_READER_PARSER_DETAIL_H_
