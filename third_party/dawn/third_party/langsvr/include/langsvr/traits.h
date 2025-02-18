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

#ifndef LANGSVR_TRAITS_H_
#define LANGSVR_TRAITS_H_

#include <string>
#include <tuple>

/// Forward declaration
namespace langsvr {
template <typename SUCCESS_TYPE, typename FAILURE_TYPE>
struct Result;
}

namespace langsvr::detail {

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
    using type = Signature<typename std::decay<R>::type, typename std::decay<ARGS>::type...>;
};

/// SignatureOf specialization for a non-static method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...)> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay<R>::type, typename std::decay<ARGS>::type...>;
};

/// SignatureOf specialization for a non-static, const method.
template <typename R, typename C, typename... ARGS>
struct SignatureOf<R (C::*)(ARGS...) const> {
    /// The signature of the function-like object `F`
    using type = Signature<typename std::decay<R>::type, typename std::decay<ARGS>::type...>;
};

template <typename T, typename... TYPES>
struct TypeIndexHelper;

template <typename T>
struct TypeIndexHelper<T> {
    static const size_t value = 0;
};

template <typename T, typename FIRST, typename... OTHERS>
struct TypeIndexHelper<T, FIRST, OTHERS...> {
    static const size_t value =
        std::is_same_v<T, FIRST> ? 0 : 1 + TypeIndexHelper<T, OTHERS...>::value;
};

template <typename T, typename... TYPES>
struct TypeIndex {
    static const size_t value = TypeIndexHelper<T, TYPES...>::value;
    static_assert(value < sizeof...(TYPES), "type not found");
};

template <typename LHS, typename RHS, typename = void>
struct HasOperatorShiftLeft : std::false_type {};
template <typename LHS, typename RHS>
struct HasOperatorShiftLeft<LHS,
                            RHS,
                            std::void_t<decltype((std::declval<LHS>() << std::declval<RHS>()))>>
    : std::true_type {};

template <typename T, typename = void>
struct IsResult : std::false_type {};
template <typename SUCCESS, typename FAILURE>
struct IsResult<Result<SUCCESS, FAILURE>> : std::true_type {};

}  // namespace langsvr::detail

namespace langsvr {

/// SignatureOf provides information about the function-like F.
template <typename F>
using SignatureOf = typename detail::SignatureOf<std::decay_t<F>>::type;

/// ParameterType resolves to the type of the Nth parameter of the function-like F
template <typename F, std::size_t N>
using ParameterType = typename SignatureOf<std::decay_t<F>>::template parameter<N>;

/// ReturnType resolves to the return type of the function-like F
template <typename F>
using ReturnType = typename SignatureOf<std::decay_t<F>>::ret;

/// TypeIndex resolves to zero-based index of T in TYPES
template <typename T, typename... TYPES>
static constexpr size_t TypeIndex = detail::TypeIndex<T, TYPES...>::value;

/// TypeIsIn resolves to true iff T is in TYPES
template <typename T, typename... TYPES>
static constexpr bool TypeIsIn = (std::is_same_v<T, TYPES> || ...);

/// Is true if operator<<(LHS, RHS) exists
template <typename LHS, typename RHS>
static constexpr bool HasOperatorShiftLeft = detail::HasOperatorShiftLeft<LHS, RHS>::value;

/// Evaluates to true if `T` decayed is a `std::string`, `std::string_view`, `const char*` or
/// `char*`
template <typename T>
static constexpr bool IsStringLike =
    std::is_same_v<std::decay_t<T>, std::string> ||
    std::is_same_v<std::decay_t<T>, std::string_view> ||
    std::is_same_v<std::decay_t<T>, const char*> || std::is_same_v<std::decay_t<T>, char*>;

/// Evaluates to true if `T` is a Result<...>
template <typename T>
static constexpr bool IsResult = detail::IsResult<T>::value;

}  // namespace langsvr

#endif  // LANGSVR_TRAITS_H_
