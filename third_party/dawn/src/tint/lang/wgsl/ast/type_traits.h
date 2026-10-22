// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TYPE_TRAITS_H_
#define SRC_TINT_LANG_WGSL_AST_TYPE_TRAITS_H_

#include <type_traits>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/utils/diagnostic/source.h"
#include "src/tint/utils/symbol/symbol.h"

namespace tint::ast {

/// Evaluates to true if T is a Infer, AInt or AFloat.
template <typename T>
static constexpr const bool IsInferOrAbstract =
    std::is_same_v<std::decay_t<T>, core::fluent_types::Infer> || core::IsAbstract<std::decay_t<T>>;

// Forward declare metafunction that evaluates to true iff T can be wrapped in a statement.
template <typename T, typename = void>
struct CanWrapInStatement;

/// Evaluates to true if T is a Source
template <typename T>
static constexpr const bool IsSource = std::is_same_v<T, Source>;

/// Evaluates to true if T is a Number or bool.
template <typename T>
static constexpr const bool IsScalar =
    std::is_integral_v<core::UnwrapNumber<T>> || std::is_floating_point_v<core::UnwrapNumber<T>> ||
    std::is_same_v<T, bool>;

/// Evaluates to true if T can be converted to an identifier.
template <typename T>
static constexpr const bool IsIdentifierLike = std::is_same_v<T, Symbol> ||  // Symbol
                                               std::is_enum_v<T> ||          // Enum
                                               traits::IsStringLike<T>;      // String

/// A helper used to disable overloads if the first type in `TYPES` is a Source. Used to avoid
/// ambiguities in overloads that take a Source as the first parameter and those that
/// perfectly-forward the first argument.
template <typename... TYPES>
using DisableIfSource =
    std::enable_if_t<!IsSource<std::decay_t<traits::NthTypeOf<0, TYPES..., void>>>>;

/// A helper used to disable overloads if the first type in `TYPES` is a scalar type. Used to
/// avoid ambiguities in overloads that take a scalar as the first parameter and those that
/// perfectly-forward the first argument.
template <typename... TYPES>
using DisableIfScalar =
    std::enable_if_t<!IsScalar<std::decay_t<traits::NthTypeOf<0, TYPES..., void>>>>;

/// A helper used to enable overloads if the first type in `TYPES` is a scalar type. Used to
/// avoid ambiguities in overloads that take a scalar as the first parameter and those that
/// perfectly-forward the first argument.
template <typename... TYPES>
using EnableIfScalar =
    std::enable_if_t<IsScalar<std::decay_t<traits::NthTypeOf<0, TYPES..., void>>>>;

/// A helper used to disable overloads if the first type in `TYPES` is a Vector or
/// VectorRef.
template <typename... TYPES>
using DisableIfVectorLike =
    std::enable_if_t<!IsVectorLike<std::decay_t<traits::NthTypeOf<0, TYPES..., void>>>>;

/// A helper used to enable overloads if the first type in `TYPES` is identifier-like.
template <typename... TYPES>
using EnableIfIdentifierLike =
    std::enable_if_t<IsIdentifierLike<std::decay_t<traits::NthTypeOf<0, TYPES..., void>>>>;

/// A helper used to disable overloads if the first type in `TYPES` is Infer or an abstract
/// numeric.
template <typename... TYPES>
using DisableIfInferOrAbstract =
    std::enable_if_t<!IsInferOrAbstract<std::decay_t<traits::NthTypeOf<0, TYPES..., void>>>>;

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_TYPE_TRAITS_H_
