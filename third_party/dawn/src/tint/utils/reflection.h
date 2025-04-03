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

#ifndef SRC_TINT_UTILS_REFLECTION_H_
#define SRC_TINT_UTILS_REFLECTION_H_

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/foreach.h"
#include "src/tint/utils/memory/aligned_storage.h"
#include "src/tint/utils/result.h"

/// Forward declarations
namespace tint {
class CastableBase;
}

namespace tint::reflection::detail {

/// Helper for detecting whether the type T contains a nested Reflection class.
template <typename T, typename ENABLE = void>
struct HasReflection : std::false_type {};

/// Specialization for types that have a nested Reflection class.
template <typename T>
struct HasReflection<T, std::void_t<typename T::Reflection>> : std::true_type {};

/// Helper for inferring the base class size of T.
template <typename T, typename ENABLE = void>
struct BaseClassSize {
    /// Zero as T::Base does not exist
    static constexpr size_t value = 0;
};

/// Specialization for types that contain a 'Base' type alias.
template <typename T>
struct BaseClassSize<T, std::void_t<typename T::Base>> {
    /// The size of T::Base, or zero if T::Base is not a base of T
    static constexpr size_t value =
        std::is_base_of_v<typename T::Base, T> ? sizeof(typename T::Base) : 0;
};

/// ReflectedFieldInfo describes a single reflected field. Used by CheckAllFieldsReflected()
struct ReflectedFieldInfo {
    /// The field name
    const std::string_view name;
    /// The field size in bytes
    const size_t size;
    /// The field alignment in bytes
    const size_t align;
    /// The field offset in bytes
    const size_t offset;
};

/// @returns Success if the sequential fields of @p fields have the expected offsets, and aligned
/// sum match the class size @p class_size.
Result<SuccessType> CheckAllFieldsReflected(VectorRef<ReflectedFieldInfo> fields,
                                            std::string_view class_name,
                                            size_t class_size,
                                            size_t class_align,
                                            bool class_is_castable);

/// @returns Success if the TINT_REFLECT() reflected fields of @tparam CLASS match the declaration
/// order, do not have any gaps, and fully account for the entire size of the class.
template <typename CLASS>
Result<SuccessType> CheckAllFieldsReflected() {
    static_assert(!std::has_virtual_destructor_v<CLASS> || std::is_base_of_v<CastableBase, CLASS>,
                  "TINT_ASSERT_ALL_FIELDS_REFLECTED() cannot be used on virtual classes, except "
                  "for types using the tint::Castable framework");
    using R = typename CLASS::Reflection;
    using Fields = typename R::Fields;
    Vector<ReflectedFieldInfo, std::tuple_size_v<Fields>> fields;
    AlignedStorage<CLASS> obj;
    R::ForeachField(obj.Get(), [&](auto&& field, std::string_view name) {
        using T = std::decay_t<decltype(field)>;
        size_t offset = static_cast<size_t>(reinterpret_cast<const std::byte*>(&field) -
                                            reinterpret_cast<const std::byte*>(&obj));
        fields.Push({name, sizeof(T), alignof(T), offset});
    });
    return CheckAllFieldsReflected(fields, R::Name, sizeof(CLASS), alignof(CLASS),
                                   std::is_base_of_v<CastableBase, CLASS>);
}

}  // namespace tint::reflection::detail

namespace tint {

/// Is true if the class T has reflected its fields with TINT_REFLECT()
template <typename T>
static constexpr bool HasReflection = tint::reflection::detail::HasReflection<T>::value;

/// Calls @p callback with each field of @p object
/// @param object the object
/// @param callback a function that is called for each field of @p object.
/// @tparam CB a function with one of the signatures:
///         `void(auto& FIELD)`
///         `void(auto& FIELD, std::string_view NAME)`
template <typename OBJECT, typename CB>
void ForeachField(OBJECT& object, CB&& callback) {
    using T = std::decay_t<OBJECT>;
    static_assert(HasReflection<T>, "object missing TINT_REFLECT() declaration");
    constexpr bool callback_field = std::is_invocable_v<std::decay_t<CB>, int&>;
    constexpr bool callback_field_name =
        std::is_invocable_v<std::decay_t<CB>, int&, std::string_view>;
    static_assert(callback_field || callback_field_name,
                  "callback must have the signature of:\n"
                  "   'void(auto& field)'\n"
                  "or 'void(auto& field, std::string_view name)'");
    if constexpr (callback_field) {
        T::Reflection::ForeachField(object,
                                    [&](auto& field, std::string_view) { callback(field); });
    } else {
        T::Reflection::ForeachField(object, std::forward<CB>(callback));
    }
}

/// Calls @p callback with each field of @p object
/// @param object the object
/// @param callback a function that is called for each field of @p object.
/// @tparam CB a function with one of the signatures:
///         `void(const auto& FIELD)`
///         `void(const auto& FIELD, std::string_view NAME)`
template <typename OBJECT, typename CB>
void ForeachField(const OBJECT& object, CB&& callback) {
    using T = std::decay_t<OBJECT>;
    static_assert(HasReflection<T>, "object missing TINT_REFLECT() declaration");
    constexpr bool callback_field = std::is_invocable_v<std::decay_t<CB>, const int&>;
    constexpr bool callback_field_name =
        std::is_invocable_v<std::decay_t<CB>, const int&, std::string_view>;
    static_assert(callback_field || callback_field_name,
                  "callback must have the signature of:\n"
                  "   'void(auto& field)'\n"
                  "or 'void(auto& field, std::string_view name)'");
    if constexpr (callback_field) {
        T::Reflection::ForeachField(object,
                                    [&](const auto& field, std::string_view) { callback(field); });
    } else {
        T::Reflection::ForeachField(object, std::forward<CB>(callback));
    }
}

/// Macro used by TINT_FOREACH() in TINT_REFLECT() to generate the T::Reflection::Fields tuple.
#define TINT_REFLECT_FIELD_TYPE(FIELD) decltype(Class::FIELD),

/// Macro used by TINT_FOREACH() in TINT_REFLECT() to call the callback function with each field in
/// the variadic.
#define TINT_REFLECT_CALLBACK_FIELD(FIELD) callback(object.FIELD, #FIELD);

// TINT_REFLECT(CLASS, ...) reflects each of the fields arguments of CLASS so that the types can be
// used with tint::ForeachField().
#define TINT_REFLECT(CLASS, ...)                                                              \
    struct Reflection {                                                                       \
        using Class = CLASS;                                                                  \
        using Fields = std::tuple<TINT_FOREACH(TINT_REFLECT_FIELD_TYPE, __VA_ARGS__) void>;   \
        [[maybe_unused]] static constexpr std::string_view Name = #CLASS;                     \
        template <typename OBJECT, typename CB>                                               \
        [[maybe_unused]] static constexpr void ForeachField(OBJECT&& object, CB&& callback) { \
            TINT_FOREACH(TINT_REFLECT_CALLBACK_FIELD, __VA_ARGS__)                            \
        }                                                                                     \
    }

/// TINT_ASSERT_ALL_FIELDS_REFLECTED(...) performs a compile-time assertion that all the fields of
/// CLASS have been reflected with TINT_REFLECT().
/// @note The order in which the fields are passed to TINT_REFLECT must match the declaration order
/// in the class.
#define TINT_ASSERT_ALL_FIELDS_REFLECTED(CLASS) \
    ASSERT_EQ(::tint::reflection::detail::CheckAllFieldsReflected<CLASS>(), ::tint::Success)

/// A template that can be specialized to reflect the valid range of an enum
/// Use TINT_REFLECT_ENUM_RANGE to specialize this class
template <typename T>
struct EnumRange;

/// Declares a specialization of EnumRange for the enum ENUM with the lowest enum value MIN and
/// largest enum value MAX. Must only be used in the `tint` namespace.
#define TINT_REFLECT_ENUM_RANGE(ENUM, MIN, MAX) \
    template <>                                 \
    struct EnumRange<ENUM> {                    \
        static constexpr ENUM kMin = ENUM::MIN; \
        static constexpr ENUM kMax = ENUM::MAX; \
    }

}  // namespace tint

#endif  // SRC_TINT_UTILS_REFLECTION_H_
