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

#ifndef SRC_TINT_UTILS_RTTI_TRAITS_H_
#define SRC_TINT_UTILS_RTTI_TRAITS_H_

#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

// Predeclarations
namespace tint {
class StringStream;
}

namespace tint::traits {

/// NthTypeOf returns the `N`th type in `Types`
template <int N, typename... Types>
using NthTypeOf = typename std::tuple_element<N, std::tuple<Types...>>::type;

/// Signature describes the signature of a function.
template <typename RETURN, typename... PARAMETERS>
struct Signature {
    /// The return type of the function signature
    using ret = RETURN;
    /// The parameters of the function signature held in a std::tuple
    using parameters = std::tuple<PARAMETERS...>;
    /// The type of the Nth parameter of function signature
    template <std::size_t N>
    using parameter = NthTypeOf<N, PARAMETERS...>;
    /// The total number of parameters
    static constexpr std::size_t parameter_count = sizeof...(PARAMETERS);
};

/// SignatureOf is a traits helper that infers the signature of the function,
/// method, static method, lambda, or function-like object `F`.
template <typename F>
struct SignatureOf {
    /// The signature of the function-like object `F`
    using type = typename SignatureOf<decltype(&F::operator())>::type;
};

/// SignatureOf specialization for a regular function or static method.
template <typename R, typename... ARGS>
struct SignatureOf<R (*)(ARGS...)> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay_t<R>, typename std::decay_t<ARGS>...>;
};

/// SignatureOf specialization for a non-static method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...)> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay_t<R>, typename std::decay_t<ARGS>...>;
};

/// SignatureOf specialization for a non-static, const method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...) const> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay_t<R>, typename std::decay_t<ARGS>...>;
};

/// SignatureOfT is an alias to `typename SignatureOf<F>::type`.
template <typename F>
using SignatureOfT = typename SignatureOf<std::decay_t<F>>::type;

/// ParameterType is an alias to `typename SignatureOf<F>::type::parameter<N>`.
template <typename F, std::size_t N>
using ParameterType = typename SignatureOfT<std::decay_t<F>>::template parameter<N>;

/// LastParameterType returns the type of the last parameter of `F`. `F` must have at least one
/// parameter.
template <typename F>
using LastParameterType = ParameterType<F, SignatureOfT<std::decay_t<F>>::parameter_count - 1>;

/// ReturnType is an alias to `typename SignatureOf<F>::type::ret`.
template <typename F>
using ReturnType = typename SignatureOfT<std::decay_t<F>>::ret;

/// @returns the std::index_sequence with all the indices shifted by OFFSET.
template <std::size_t OFFSET, std::size_t... INDICES>
constexpr auto Shift(std::index_sequence<INDICES...>) {
    return std::integer_sequence<std::size_t, OFFSET + INDICES...>{};
}

/// @returns a std::integer_sequence with the integers `[OFFSET..OFFSET+COUNT)`
template <std::size_t OFFSET, std::size_t COUNT>
constexpr auto Range() {
    return Shift<OFFSET>(std::make_index_sequence<COUNT>{});
}

namespace detail {

/// @returns a nullptr of the tuple type `TUPLE` swizzled by `INDICES`.
/// @note: This function is intended to be used in a `decltype()` expression,
/// and returns a pointer-to-tuple as the tuple may hold non-constructable
/// types.
template <typename TUPLE, std::size_t... INDICES>
constexpr auto* SwizzlePtrTy(std::index_sequence<INDICES...>) {
    using Swizzled = std::tuple<std::tuple_element_t<INDICES, TUPLE>...>;
    return static_cast<Swizzled*>(nullptr);
}

/// Base template for IsTypeIn
template <typename T, typename TypeList>
struct IsTypeIn;

/// Specialization for IsTypeIn
template <typename T, template <typename...> typename TypeContainer, typename... Ts>
struct IsTypeIn<T, TypeContainer<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};

}  // namespace detail

/// Resolves to the slice of the tuple `t` with the tuple elements
/// `[OFFSET..OFFSET+COUNT)`
template <std::size_t OFFSET, std::size_t COUNT, typename TUPLE>
using SliceTuple =
    std::remove_pointer_t<decltype(traits::detail::SwizzlePtrTy<TUPLE>(Range<OFFSET, COUNT>()))>;

/// Evaluates to the decayed pointer element type, or the decayed type T if T is not a pointer.
template <typename T>
using PtrElTy = std::decay_t<std::remove_pointer_t<std::decay_t<T>>>;

////////////////////////////////////////////////////////////////////////////////
/// Concepts
////////////////////////////////////////////////////////////////////////////////

/// Returns true iff decayed T and decayed U are the same.
template <typename T, typename U>
concept IsType = std::is_same_v<std::decay_t<T>, std::decay_t<U>>;

/// Evaluates to true if T is one of the types in the TypeContainer's template arguments.
/// Works for std::variant, std::tuple, std::pair, or any typename template where all parameters are
/// types.
template <typename T, typename TypeContainer>
concept IsTypeIn = traits::detail::IsTypeIn<T, TypeContainer>::value;

/// IsTypeOrDerived<T, BASE> is true iff `T` is of type `BASE`, or derives from `BASE`.
template <typename T, typename BASE>
concept IsTypeOrDerived =
    std::is_base_of_v<BASE, std::decay_t<T>> || std::is_same_v<BASE, std::decay_t<T>>;

template <typename T>
concept IsOStream = std::is_same_v<T, std::ostream> || std::is_same_v<T, std::stringstream> ||
                    std::is_same_v<T, tint::StringStream>;

/// Evaluates to true if `T` decayed is a `std::string`, `std::string_view`, `const char*` or
/// `char*`
template <typename T>
concept IsStringLike =
    std::is_same_v<std::decay_t<T>, std::string> ||
    std::is_same_v<std::decay_t<T>, std::string_view> ||
    std::is_same_v<std::decay_t<T>, const char*> || std::is_same_v<std::decay_t<T>, char*>;

/// Is true if operator<<(LHS, RHS) exists
template <typename LHS, typename RHS>
concept HasOperatorShiftLeft = requires(LHS os, RHS a) { os << a; };

}  // namespace tint::traits

#endif  // SRC_TINT_UTILS_RTTI_TRAITS_H_
