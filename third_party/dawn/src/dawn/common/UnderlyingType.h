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

#ifndef SRC_DAWN_COMMON_UNDERLYINGTYPE_H_
#define SRC_DAWN_COMMON_UNDERLYINGTYPE_H_

#include <concepts>
#include <type_traits>

namespace dawn {

// UnderlyingType is similar to std::underlying_type_t. It is a passthrough for already
// integer types which simplifies getting the underlying primitive type for an arbitrary
// template parameter. It includes a specialization for detail::TypedIntegerImpl which yields
// the wrapped integer type.
namespace detail {
template <typename T, typename Enable = void>
struct UnderlyingTypeImpl;

template <typename I>
    requires std::integral<I>
struct UnderlyingTypeImpl<I> {
    using type = I;
};

template <typename E>
    requires std::is_enum_v<E>
struct UnderlyingTypeImpl<E> {
    using type = std::underlying_type_t<E>;
};

// Forward declare the TypedInteger impl.
template <typename Tag, typename T>
class TypedIntegerImpl;

template <typename Tag, typename I>
struct UnderlyingTypeImpl<TypedIntegerImpl<Tag, I>> {
    using type = typename UnderlyingTypeImpl<I>::type;
};
}  // namespace detail

template <typename T>
using UnderlyingType = typename detail::UnderlyingTypeImpl<T>::type;

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_UNDERLYINGTYPE_H_
