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

#ifndef SRC_TINT_UTILS_RTTI_SWITCH_H_
#define SRC_TINT_UTILS_RTTI_SWITCH_H_

#include <tuple>
#include <utility>

#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/memory/aligned_storage.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/rtti/ignore.h"

namespace tint {

/// Default can be used as the default case for a Switch(), when all previous cases failed to match.
///
/// Example:
/// ```
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ },
///     [&](Default) { /* If not TypeA or TypeB */ });
/// ```
struct Default {};

/// SwitchMustMatchCase is a flag that can be passed as the last argument to Switch() which will
/// trigger an ICE if none of the cases matched. Cannot be used with Default.
/// See TINT_ICE_ON_NO_MATCH
struct SwitchMustMatchCase {
    /// The source file that holds the TINT_ICE_ON_NO_MATCH
    const char* file = "<unknown>";
    /// The source line that holds the TINT_ICE_ON_NO_MATCH
    unsigned int line = 0;
};

/// SwitchMustMatchCase is a flag that can be passed as the last argument to Switch() which will
/// trigger an ICE if none of the cases matched. Cannot be used with Default.
///
/// Example:
/// ```
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ },
///     TINT_ICE_ON_NO_MATCH);
/// ```
#define TINT_ICE_ON_NO_MATCH      \
    ::tint::SwitchMustMatchCase { \
        __FILE__, __LINE__        \
    }

}  // namespace tint

namespace tint::detail {

/// Evaluates to the Switch case type being matched by the switch case function `FN`.
/// @note does not handle the Default case
/// @see Switch().
template <typename FN>
using SwitchCaseType =
    std::remove_pointer_t<tint::traits::ParameterType<std::remove_reference_t<FN>, 0>>;

/// Searches the list of Switch cases for a Default case, returning the index of the Default case.
/// If the a Default case is not found in the tuple, then -1 is returned.
template <typename TUPLE, std::size_t START_IDX = 0>
constexpr int IndexOfDefaultCase() {
    if constexpr (START_IDX < std::tuple_size_v<TUPLE>) {
        using T = std::decay_t<std::tuple_element_t<START_IDX, TUPLE>>;
        if constexpr (std::is_same_v<T, SwitchMustMatchCase>) {
            return -1;
        } else if constexpr (std::is_same_v<tint::traits::ParameterType<T, 0>, Default>) {
            return static_cast<int>(START_IDX);
        } else {
            return IndexOfDefaultCase<TUPLE, START_IDX + 1>();
        }
    } else {
        return -1;
    }
}

/// Searches the list of Switch cases for a SwitchMustMatchCase flag, returning the index of the
/// SwitchMustMatchCase case. If the a SwitchMustMatchCase case is not found in the tuple, then -1
/// is returned.
template <typename TUPLE, std::size_t START_IDX = 0>
constexpr int IndexOfSwitchMustMatchCase() {
    if constexpr (START_IDX < std::tuple_size_v<TUPLE>) {
        using T = std::decay_t<std::tuple_element_t<START_IDX, TUPLE>>;
        return std::is_same_v<T, SwitchMustMatchCase>
                   ? static_cast<int>(START_IDX)
                   : IndexOfSwitchMustMatchCase<TUPLE, START_IDX + 1>();
    } else {
        return -1;
    }
}
/// Resolves to T if T is not nullptr_t, otherwise resolves to Ignore.
template <typename T>
using NullptrToIgnore = std::conditional_t<std::is_same_v<T, std::nullptr_t>, ::tint::Ignore, T>;

/// Resolves to `const TYPE` if any of `CASE_RETURN_TYPES` are const or pointer-to-const, otherwise
/// resolves to TYPE.
template <typename TYPE, typename... CASE_RETURN_TYPES>
using PropagateReturnConst = std::conditional_t<
    // Are any of the pointer-stripped types const?
    (std::is_const_v<std::remove_pointer_t<CASE_RETURN_TYPES>> || ...),
    const TYPE,  // Yes: Apply const to TYPE
    TYPE>;       // No:  Passthrough

/// SwitchReturnTypeImpl is the implementation of SwitchReturnType
template <bool IS_CASTABLE, typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl;

/// SwitchReturnTypeImpl specialization for non-castable case types and an explicitly specified
/// return type.
template <typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ false, REQUESTED_TYPE, CASE_RETURN_TYPES...> {
    /// Resolves to `REQUESTED_TYPE`
    using type = REQUESTED_TYPE;
};

/// SwitchReturnTypeImpl specialization for non-castable case types and an inferred return type.
template <typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ false, ::tint::detail::Infer, CASE_RETURN_TYPES...> {
    /// Resolves to the common type for all the cases return types.
    using type = std::common_type_t<CASE_RETURN_TYPES...>;
};

/// SwitchReturnTypeImpl specialization for castable case types and an explicitly specified return
/// type.
template <typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ true, REQUESTED_TYPE, CASE_RETURN_TYPES...> {
  public:
    /// Resolves to `const REQUESTED_TYPE*` or `REQUESTED_TYPE*`
    using type = PropagateReturnConst<std::remove_pointer_t<REQUESTED_TYPE>, CASE_RETURN_TYPES...>*;
};

/// SwitchReturnTypeImpl specialization for castable case types and an inferred return type.
template <typename... CASE_RETURN_TYPES>
struct SwitchReturnTypeImpl</*IS_CASTABLE*/ true, ::tint::detail::Infer, CASE_RETURN_TYPES...> {
  private:
    using InferredType =
        CastableCommonBase<NullptrToIgnore<std::remove_pointer_t<CASE_RETURN_TYPES>>...>;

  public:
    /// `const T*` or `T*`, where T is the common base type for all the castable case types.
    using type = PropagateReturnConst<InferredType, CASE_RETURN_TYPES...>*;
};

/// Resolves to the return type for a Switch() with the requested return type `REQUESTED_TYPE` and
/// case statement return types. If `REQUESTED_TYPE` is Infer then the return type will be inferred
/// from the case return types.
template <typename REQUESTED_TYPE, typename... CASE_RETURN_TYPES>
using SwitchReturnType = typename SwitchReturnTypeImpl<
    ::tint::IsCastable<NullptrToIgnore<std::remove_pointer_t<CASE_RETURN_TYPES>>...>,
    REQUESTED_TYPE,
    CASE_RETURN_TYPES...>::type;

/// SwitchCaseReturnTypeImpl is the implementation of SwitchCaseReturnType
template <typename CASE, bool is_flag>
struct SwitchCaseReturnTypeImpl;

/// SwitchCaseReturnTypeImpl specialization for non-flags.
template <typename CASE>
struct SwitchCaseReturnTypeImpl<CASE, /* is_flag */ false> {
    /// The case function's return type.
    using type = ::tint::traits::ReturnType<CASE>;
};

/// SwitchCaseReturnTypeImpl specialization for flags.
template <typename CASE>
struct SwitchCaseReturnTypeImpl<CASE, /* is_flag */ true> {
    /// These are not functions, they have no return type.
    using type = ::tint::Ignore;
};

/// Resolves to the return type for a Switch() case.
/// If CASE is a flag like SwitchMustMatchCase, then resolves to ::tint::Ignore
template <typename CASE>
using SwitchCaseReturnType = typename SwitchCaseReturnTypeImpl<
    CASE,
    std::is_same_v<std::decay_t<CASE>, SwitchMustMatchCase>>::type;

/// Raises an ICE error that a Switch() was passed a nullptr object and there was no default case
[[noreturn]] void ICENoSwitchPassedNullptr(const char* file, unsigned int line);

/// Raises an ICE error that a Switch() with a TINT_ICE_ON_NO_MATCH matched no cases.
/// @param file the file holding the Switch()
/// @param line the line of the TINT_ICE_ON_NO_MATCH
/// @type_name the type name of the object passed to Switch()
[[noreturn]] void ICENoSwitchCasesMatched(const char* file,
                                          unsigned int line,
                                          const char* type_name);

}  // namespace tint::detail

namespace tint {

/// Switch is used to dispatch one of the provided callback case handler functions based on the type
/// of `object` and the parameter type of the case handlers. Switch will sequentially check the type
/// of `object` against each of the switch case handler functions, and will invoke the first case
/// handler function which has a parameter type that matches the object type. When a case handler is
/// matched, it will be called with the single argument of `object` cast to the case handler's
/// parameter type. Switch will invoke at most one case handler. Each of the case functions must
/// have the signature `R(T*)` or `R(const T*)`, where `T` is the type matched by that case and `R`
/// is the return type, consistent across all case handlers.
///
/// An optional default case function with the signature `R(Default)` can be used as the last case.
/// This default case will be called if all previous cases failed to match.
///
/// The last argument may be SwitchMustMatchCase, in which case the Switch will trigger an ICE if
/// none of the cases matched. SwitchMustMatchCase cannot be used with a default case.
///
/// If `object` is nullptr and a default case is provided, then the default case will be called. If
/// `object` is nullptr and no default case is provided, then no cases will be called.
///
/// Example:
/// ```
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ });
///
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ },
///     [&](Default) { /* Called if object is not TypeA or TypeB */ });
///
/// Switch(object,
///     [&](TypeA*) { /* ... */ },
///     [&](TypeB*) { /* ... */ },
///     SwitchMustMatchCase); /* ICE if object is not TypeA or TypeB */
/// ```
///
/// @param object the object who's type is used to
/// @param args the switch cases followed by an optional TINT_ICE_ON_NO_MATCH
/// @return the value returned by the called case. If no cases matched, then the zero value for the
/// consistent case type.
template <typename RETURN_TYPE = ::tint::detail::Infer, typename T = CastableBase, typename... ARGS>
inline auto Switch(T* object, ARGS&&... args) {
    TINT_BEGIN_DISABLE_WARNING(UNUSED_VALUE);

    using ArgsTuple = std::tuple<ARGS...>;
    static constexpr int kMustMatchCaseIndex =
        ::tint::detail::IndexOfSwitchMustMatchCase<ArgsTuple>();
    static constexpr bool kHasMustMatchCase = kMustMatchCaseIndex >= 0;
    static constexpr int kDefaultIndex = ::tint::detail::IndexOfDefaultCase<ArgsTuple>();
    static constexpr bool kHasDefaultCase = kDefaultIndex >= 0;
    using ReturnType =
        ::tint::detail::SwitchReturnType<RETURN_TYPE,
                                         ::tint::detail::SwitchCaseReturnType<ARGS>...>;
    static constexpr bool kHasReturnType = !std::is_same_v<ReturnType, void>;

    // Static assertions
    static constexpr bool kDefaultIsOK =
        kDefaultIndex == -1 || kDefaultIndex == static_cast<int>(sizeof...(ARGS) - 1);
    static constexpr bool kMustMatchCaseIsOK =
        kMustMatchCaseIndex == -1 || kMustMatchCaseIndex == static_cast<int>(sizeof...(ARGS) - 1);
    static constexpr bool kReturnIsOK =
        kHasDefaultCase || !kHasReturnType || std::is_constructible_v<ReturnType>;
    static_assert(kDefaultIsOK, "Default case must be last in Switch()");
    static_assert(kMustMatchCaseIsOK, "SwitchMustMatchCase must be last argument in Switch()");
    static_assert(!kHasDefaultCase || !kHasMustMatchCase,
                  "SwitchMustMatchCase cannot be used with a Default case");
    static_assert(kReturnIsOK,
                  "Switch() requires either a Default case or a return type that is either void or "
                  "default-constructable");

    if (!object) {  // Object is nullptr, so no cases can match
        if constexpr (kHasMustMatchCase) {
            const SwitchMustMatchCase& info = (args, ...);
            ::tint::detail::ICENoSwitchPassedNullptr(info.file, info.line);
            if constexpr (kHasReturnType) {
                return ReturnType{};
            } else {
                return;
            }
        } else if constexpr (kHasDefaultCase) {
            // Evaluate default case.
            const auto& default_case = (args, ...);
            return static_cast<ReturnType>(default_case(Default{}));
        } else {
            // No default case, no case can match.
            if constexpr (kHasReturnType) {
                return ReturnType{};
            } else {
                return;
            }
        }
    }

    AlignedStorage<std::conditional_t<kHasReturnType, ReturnType, uint8_t>> return_storage;
    auto* result = &return_storage.Get();

    const ::tint::TypeInfo& type_info = object->TypeInfo();

    // Examines the parameter type of the case function.
    // If the parameter is a pointer type that `object` is of, or derives from, then that case
    // function is called with `object` cast to that type, and `try_case` returns true.
    // If the parameter is of type `Default`, then that case function is called and `try_case`
    // returns true.
    // Otherwise `try_case` returns false.
    // If the case function is called and it returns a value, then this is copy constructed to the
    // `result` pointer.
    auto try_case = [&](auto&& case_fn) {
        using CaseFunc = std::decay_t<decltype(case_fn)>;
        bool success = false;
        if constexpr (std::is_same_v<CaseFunc, SwitchMustMatchCase>) {
            ::tint::detail::ICENoSwitchCasesMatched(case_fn.file, case_fn.line, type_info.name);
        } else {
            using CaseType = ::tint::detail::SwitchCaseType<CaseFunc>;
            if constexpr (std::is_same_v<CaseType, Default>) {
                if constexpr (kHasReturnType) {
                    new (result) ReturnType(static_cast<ReturnType>(case_fn(Default{})));
                } else {
                    case_fn(Default{});
                }
                success = true;
            } else {
                if (type_info.Is<CaseType>()) {
                    auto* v = static_cast<CaseType*>(object);
                    if constexpr (kHasReturnType) {
                        new (result) ReturnType(static_cast<ReturnType>(case_fn(v)));
                    } else {
                        case_fn(v);
                    }
                    success = true;
                }
            }
        }
        return success;
    };

    // Use a logical-or fold expression to try each of the cases in turn, until one matches the
    // object type or a Default is reached. `handled` is true if a case function was called.
    bool handled = ((try_case(std::forward<ARGS>(args)) || ...));

    if constexpr (kHasReturnType) {
        if constexpr (kHasDefaultCase) {
            // Default case means there must be a returned value.
            // No need to check handled, no requirement for a zero-initializer of ReturnType.
            TINT_DEFER(result->~ReturnType());
            return *result;
        } else {
            if (handled) {
                TINT_DEFER(result->~ReturnType());
                return *result;
            }
            return ReturnType{};
        }
    }

    TINT_END_DISABLE_WARNING(UNUSED_VALUE);
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_RTTI_SWITCH_H_
