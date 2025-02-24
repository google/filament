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

#ifndef SRC_TINT_LANG_CORE_FLUENT_TYPES_H_
#define SRC_TINT_LANG_CORE_FLUENT_TYPES_H_

#include <stdint.h>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/number.h"

namespace tint::core::fluent_types {

using f16 = tint::core::f16;
using f32 = tint::core::f32;
using i32 = tint::core::i32;
using i8 = tint::core::i8;
using u32 = tint::core::u32;
using u64 = tint::core::u64;
using u8 = tint::core::u8;
using AFloat = tint::core::AFloat;
using AInt = tint::core::AInt;

// A sentinel type used by some template arguments to signal that the a type should be inferred.
struct Infer {};

/// A 'fluent' type helper used to construct an ast::Array or type::Array.
/// @tparam T the array element type
/// @tparam N the array length. 0 represents a runtime-sized array.
/// @see https://www.w3.org/TR/WGSL/#array-types
template <typename T, uint32_t N = 0>
struct array {
    /// the array element type
    using type = T;
    /// the array length. 0 represents a runtime-sized array.
    static constexpr uint32_t length = N;
};

/// A 'fluent' type helper used to construct an ast::Atomic or type::Atomic.
/// @tparam T the atomic element type
/// @see https://www.w3.org/TR/WGSL/#atomic-types
template <typename T>
struct atomic {
    /// the atomic element type
    using type = T;
};

/// A 'fluent' type helper used to construct an ast::Vector or type::Vector.
/// @tparam N the vector width
/// @tparam T the vector element type
template <uint32_t N, typename T>
struct vec {
    /// the vector width
    static constexpr uint32_t width = N;
    /// the vector element type
    using type = T;
};

/// A 'fluent' type helper used to construct an ast::Matrix or type::Matrix.
/// @tparam C the number of columns of the matrix
/// @tparam R the number of rows of the matrix
/// @tparam T the matrix element type
/// @see https://www.w3.org/TR/WGSL/#matrix-types
template <uint32_t C, uint32_t R, typename T>
struct mat {
    /// the number of columns of the matrix
    static constexpr uint32_t columns = C;
    /// the number of rows of the matrix
    static constexpr uint32_t rows = R;
    /// the matrix element type
    using type = T;
    /// the column vector type
    using column = vec<R, T>;
};

/// A 'fluent' type helper used to construct an ast::Pointer or type::Pointer.
/// @tparam ADDRESS the pointer address space
/// @tparam T the pointer storage type
/// @tparam ACCESS the pointer access control
template <core::AddressSpace ADDRESS, typename T, core::Access ACCESS = core::Access::kUndefined>
struct ptr {
    /// the pointer address space
    static constexpr core::AddressSpace address = ADDRESS;
    /// the pointer storage type
    using type = T;
    /// the pointer access control
    static constexpr core::Access access = ACCESS;
};

////////////////////////////////////////////////////////////////////////////////
// Aliases
//
// Shorthand aliases for the types declared above
////////////////////////////////////////////////////////////////////////////////

//! @cond Doxygen_Suppress
template <typename T>
using mat2x2 = mat<2, 2, T>;

template <typename T>
using mat2x3 = mat<2, 3, T>;

template <typename T>
using mat2x4 = mat<2, 4, T>;

template <typename T>
using mat3x2 = mat<3, 2, T>;

template <typename T>
using mat3x3 = mat<3, 3, T>;

template <typename T>
using mat3x4 = mat<3, 4, T>;

template <typename T>
using mat4x2 = mat<4, 2, T>;

template <typename T>
using mat4x3 = mat<4, 3, T>;

template <typename T>
using mat4x4 = mat<4, 4, T>;

template <typename T>
using vec2 = vec<2, T>;

template <typename T>
using vec3 = vec<3, T>;

template <typename T>
using vec4 = vec<4, T>;

//! @endcond

////////////////////////////////////////////////////////////////////////////////
// Address space aliases
////////////////////////////////////////////////////////////////////////////////
static constexpr core::AddressSpace function = core::AddressSpace::kFunction;
static constexpr core::AddressSpace handle = core::AddressSpace::kHandle;
static constexpr core::AddressSpace private_ = core::AddressSpace::kPrivate;
static constexpr core::AddressSpace push_constant = core::AddressSpace::kPushConstant;
static constexpr core::AddressSpace storage = core::AddressSpace::kStorage;
static constexpr core::AddressSpace uniform = core::AddressSpace::kUniform;
static constexpr core::AddressSpace workgroup = core::AddressSpace::kWorkgroup;

////////////////////////////////////////////////////////////////////////////////
// Access control aliases
////////////////////////////////////////////////////////////////////////////////
static constexpr core::Access read = core::Access::kRead;
static constexpr core::Access read_write = core::Access::kReadWrite;
static constexpr core::Access write = core::Access::kWrite;

////////////////////////////////////////////////////////////////////////////////
// Traits
////////////////////////////////////////////////////////////////////////////////
namespace detail {

//! @cond Doxygen_Suppress
template <typename T>
struct IsArray {
    static constexpr bool value = false;
};

template <typename T, uint32_t N>
struct IsArray<array<T, N>> {
    static constexpr bool value = true;
};

template <typename T>
struct IsAtomic {
    static constexpr bool value = false;
};

template <typename T>
struct IsAtomic<atomic<T>> {
    static constexpr bool value = true;
};

template <typename T>
struct IsMatrix {
    static constexpr bool value = false;
};

template <uint32_t C, uint32_t R, typename T>
struct IsMatrix<mat<C, R, T>> {
    static constexpr bool value = true;
};

template <typename T>
struct IsVector {
    static constexpr bool value = false;
};

template <uint32_t N, typename T>
struct IsVector<vec<N, T>> {
    static constexpr bool value = true;
};

template <typename T>
struct IsPointer {
    static constexpr bool value = false;
};

template <core::AddressSpace ADDRESS, typename T, core::Access ACCESS>
struct IsPointer<ptr<ADDRESS, T, ACCESS>> {
    static constexpr bool value = true;
};
//! @endcond

}  // namespace detail

/// Evaluates to true if `T` is a array
template <typename T>
static constexpr bool IsArray = fluent_types::detail::IsArray<T>::value;

/// Evaluates to true if `T` is a atomic
template <typename T>
static constexpr bool IsAtomic = fluent_types::detail::IsAtomic<T>::value;

/// Evaluates to true if `T` is a mat
template <typename T>
static constexpr bool IsMatrix = fluent_types::detail::IsMatrix<T>::value;

/// Evaluates to true if `T` is a vec
template <typename T>
static constexpr bool IsVector = fluent_types::detail::IsVector<T>::value;

/// Evaluates to true if `T` is a ptr
template <typename T>
static constexpr bool IsPointer = fluent_types::detail::IsPointer<T>::value;

}  // namespace tint::core::fluent_types

#endif  // SRC_TINT_LANG_CORE_FLUENT_TYPES_H_
