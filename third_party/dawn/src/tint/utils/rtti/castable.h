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

#ifndef SRC_TINT_UTILS_RTTI_CASTABLE_H_
#define SRC_TINT_UTILS_RTTI_CASTABLE_H_

#include <stdint.h>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/math/crc32.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/ignore.h"
#include "src/tint/utils/rtti/traits.h"

#if defined(__clang__)
/// Temporarily disable certain warnings when using Castable API
#define TINT_CASTABLE_PUSH_DISABLE_WARNINGS()                                 \
    _Pragma("clang diagnostic push")                                     /**/ \
        _Pragma("clang diagnostic ignored \"-Wundefined-var-template\"") /**/ \
        TINT_REQUIRE_SEMICOLON

/// Restore disabled warnings
#define TINT_CASTABLE_POP_DISABLE_WARNINGS() \
    _Pragma("clang diagnostic pop") /**/     \
        TINT_REQUIRE_SEMICOLON
#else
#define TINT_CASTABLE_PUSH_DISABLE_WARNINGS() TINT_REQUIRE_SEMICOLON
#define TINT_CASTABLE_POP_DISABLE_WARNINGS() TINT_REQUIRE_SEMICOLON
#endif

TINT_CASTABLE_PUSH_DISABLE_WARNINGS();

// Forward declarations
namespace tint {
class CastableBase;
}  // namespace tint

namespace tint::detail {
template <typename T>
struct TypeInfoOf;
}  // namespace tint::detail

namespace tint {

/// True if all template types that are not Ignore derive from CastableBase
template <typename... TYPES>
static constexpr bool IsCastable =
    ((tint::traits::IsTypeOrDerived<TYPES, CastableBase> || std::is_same_v<TYPES, Ignore>)&&...) &&
    !(std::is_same_v<TYPES, Ignore> && ...);

/// Helper macro to instantiate the TypeInfo<T> template for `CLASS`.
#define TINT_INSTANTIATE_TYPEINFO(CLASS)                        \
    TINT_CASTABLE_PUSH_DISABLE_WARNINGS();                      \
    template <>                                                 \
    const tint::TypeInfo tint::detail::TypeInfoOf<CLASS>::info{ \
        &tint::detail::TypeInfoOf<CLASS::TrueBase>::info,       \
        #CLASS,                                                 \
        tint::TypeCode::Of<CLASS>(),                            \
        tint::TypeCodeSet::OfHierarchy<CLASS>(),                \
    };                                                          \
    TINT_CASTABLE_POP_DISABLE_WARNINGS();                       \
    static_assert(std::is_same_v<CLASS, CLASS::Base::Class>,    \
                  #CLASS " does not derive from Castable<" #CLASS "[, BASE]>")

/// Bit flags that can be passed to the template parameter `FLAGS` of Is() and As().
enum CastFlags {
    /// Disables the static_assert() inside Is(), that compile-time-verifies that the cast is
    /// possible. This flag may be useful for highly-generic template
    /// code that needs to compile for template permutations that generate
    /// impossible casts.
    kDontErrorOnImpossibleCast = 1,
};

/// TypeCode is a bit pattern used by Tint's RTTI system to determine whether two types are related
/// by inheritance.
/// Each TypeCode has exactly two bits set.
struct TypeCode {
    /// @returns a compile-time TypeCode for the type `T`.
    template <typename T>
    static constexpr inline TypeCode Of() {
        static_assert(IsCastable<T>, "T is not Castable");
        static_assert(std::is_same_v<T, std::remove_cv_t<T>>,
                      "Strip const / volatile decorations before calling Of");
        /// Use the compiler's "pretty" function name, which includes the template type, to obtain a
        /// unique hash value.
#ifdef _MSC_VER
        constexpr Bits crc = tint::CRC32(__FUNCSIG__);
#else
        constexpr Bits crc = tint::CRC32(__PRETTY_FUNCTION__);
#endif
        constexpr Bits bit_a = (crc & 63);
        constexpr Bits bit_b = ((crc >> 6) & 63);
        constexpr Bits bit_c = (bit_a == bit_b) ? ((bit_a + 1) & 63) : bit_b;
        return {(static_cast<Bits>(1) << bit_a) | (static_cast<Bits>(1) << bit_c)};
    }

    /// The unsigned integer type that holds the bits of the TypeCode
    using Bits = uint64_t;

    /// The bits pattern of the TypeCode
    const Bits bits;
};

/// TypeCodeSet is a set of TypeCodes, and internally uses a single integer to represent its
/// contents. TypeCodeSet acts as a bloom-filter, exposing methods to query whether the set _may_
/// contain one or more TypeCodes. If these methods return `false` then the set definitely does
/// contain the TypeCode(s), however returning `true` means the *set has a possibility* of
/// containing the TypeCodes(s).
/// @see https://en.wikipedia.org/wiki/Bloom_filter
struct TypeCodeSet {
    /// @returns the TypeCodeSet that contains the TypeCode of `T` and all its ancestor types.
    template <typename T>
    static constexpr inline TypeCodeSet OfHierarchy() {
        if constexpr (std::is_same_v<T, CastableBase>) {
            return {TypeCode::Of<T>().bits};
        } else {
            return {TypeCode::Of<T>().bits | TypeCodeSet::OfHierarchy<typename T::TrueBase>().bits};
        }
    }

    /// @returns the TypeCodeSet of all the types in the tuple `TUPLE`.
    template <typename TUPLE>
    static constexpr inline TypeCodeSet OfTuple() {
        constexpr auto kCount = std::tuple_size_v<TUPLE>;
        if constexpr (kCount == 0) {
            return {0};
        } else if constexpr (kCount == 1) {
            return {TypeCode::Of<std::remove_cv_t<std::tuple_element_t<0, TUPLE>>>().bits};
        } else {
            constexpr auto kMid = kCount / 2;
            return {OfTuple<tint::traits::SliceTuple<0, kMid, TUPLE>>().bits |
                    OfTuple<tint::traits::SliceTuple<kMid, kCount - kMid, TUPLE>>().bits};
        }
    }

    /// @returns true if this TypeCodeSet may contain the TypeCode @p type_code.
    inline bool MayContain(TypeCode type_code) const {
        return (bits & type_code.bits) == type_code.bits;
    }

    /// @returns true if TypeCodeSet may contain any of the TypeCodes in @p type_codes.
    inline bool MayContainAnyOf(TypeCodeSet type_codes) const {
        // TypeCode always has exactly two bits set, so if the intersection of this TypeCodeSet and
        // type_codes contains fewer than two bits set, we know there's no possibility of the same
        // type being part of both sets.
        Bits mask = bits & type_codes.bits;
        bool has_at_least_two_bits = (mask & (mask - 1)) != 0;
        return has_at_least_two_bits;
    }

    /// The unsigned integer type that holds the bits of the TypeCode
    using Bits = typename TypeCode::Bits;

    /// The bits pattern of the TypeCode
    const Bits bits;
};

/// TypeInfo holds type information for a Castable type.
struct TypeInfo {
    /// The base class of this type
    const TypeInfo* base;
    /// The type name
    const char* name;
    /// The type's TypeCode
    const TypeCode type_code;
    /// The set of this type's TypeCode and all ancestor's TypeCodes
    const TypeCodeSet full_type_code;

    /// @returns true if `type` derives from the class `TO`
    /// @param object the object type to test from, which must be, or derive from type `FROM`.
    /// @see CastFlags
    template <typename TO, typename FROM, int FLAGS = 0>
    static inline bool Is(const tint::TypeInfo* object) {
        constexpr const bool downcast = std::is_base_of<FROM, TO>::value;
        constexpr const bool upcast = std::is_base_of<TO, FROM>::value;
        constexpr const bool nocast = std::is_same<FROM, TO>::value;
        constexpr const bool assert_is_castable = (FLAGS & kDontErrorOnImpossibleCast) == 0;

        static_assert(upcast || downcast || nocast || !assert_is_castable, "impossible cast");

        return upcast || nocast || object->Is<TO>();
    }

    /// @returns true if this type derives from the class `T`
    template <typename T>
    inline bool Is() const {
        auto* type = &Of<std::remove_cv_t<T>>();

        if constexpr (std::is_final_v<T>) {
            // T is final, so nothing can derive from T.
            // We do not need to check ancestors, only whether this type is equal to the type T.
            return type == this;
        } else {
            return Is(type);
        }
    }

    /// @param type the test type info
    /// @returns true if the class with this TypeInfo is of, or derives from the
    /// class with the given TypeInfo.
    inline bool Is(const tint::TypeInfo* type) const {
        if (!full_type_code.MayContain(type->type_code)) {
            return false;
        }

        // Walk the base types, starting with this TypeInfo, to see if any of the pointers match
        // `type`.
        for (auto* ti = this; ti != nullptr; ti = ti->base) {
            if (ti == type) {
                return true;
            }
        }
        return false;
    }

    /// @returns the static TypeInfo for the type T
    template <typename T>
    static const TypeInfo& Of() {
        return tint::detail::TypeInfoOf<std::remove_cv_t<T>>::info;
    }

    /// @returns true if this TypeInfo is of, or derives from any of the types in `TUPLE`.
    template <typename TUPLE>
    inline bool IsAnyOfTuple() const {
        constexpr auto kCount = std::tuple_size_v<TUPLE>;
        if constexpr (kCount == 0) {
            return false;
        } else if constexpr (kCount == 1) {
            return Is(&Of<std::tuple_element_t<0, TUPLE>>());
        } else {
            if (full_type_code.MayContainAnyOf(TypeCodeSet::OfTuple<TUPLE>())) {
                // Possibly one of the types in `TUPLE`.
                // Split the search in two, and scan each block.
                static constexpr auto kMid = kCount / 2;
                return IsAnyOfTuple<tint::traits::SliceTuple<0, kMid, TUPLE>>() ||
                       IsAnyOfTuple<tint::traits::SliceTuple<kMid, kCount - kMid, TUPLE>>();
            }
            return false;
        }
    }

    /// @returns true if this TypeInfo is of, or derives from any of the types in `TYPES`.
    template <typename... TYPES>
    inline bool IsAnyOf() const {
        return IsAnyOfTuple<std::tuple<TYPES...>>();
    }
};

namespace detail {

/// TypeInfoOf contains a single TypeInfo field for the type T.
/// TINT_INSTANTIATE_TYPEINFO() must be defined in a .cpp file for each type `T`.
template <typename T>
struct TypeInfoOf {
    /// The unique TypeInfo for the type T.
    static const TypeInfo info;
};

/// A placeholder structure used for template parameters that need a default type, but can always be
/// automatically inferred.
struct Infer;

}  // namespace detail

/// @returns true if `obj` is a valid pointer, and is of, or derives from the class `TO`
/// @param obj the object to test from
/// @see CastFlags
template <typename TO, int FLAGS = 0, typename FROM = tint::detail::Infer>
inline bool Is(FROM* obj) {
    if (obj == nullptr) {
        return false;
    }
    return TypeInfo::Is<TO, FROM, FLAGS>(&obj->TypeInfo());
}

/// @returns true if `obj` is a valid pointer, and is of, or derives from the type `TYPE`, and
/// pred(const TYPE*) returns true
/// @param obj the object to test from
/// @param pred predicate function with signature `bool(const TYPE*)` called iff object is of, or
/// derives from the class `TYPE`.
/// @see CastFlags
template <typename TYPE,
          int FLAGS = 0,
          typename OBJ = tint::detail::Infer,
          typename Pred = tint::detail::Infer>
inline bool Is(OBJ* obj, Pred&& pred) {
    return Is<TYPE, FLAGS, OBJ>(obj) && pred(static_cast<std::add_const_t<TYPE>*>(obj));
}

/// @returns true if `obj` is a valid pointer, and is of, or derives from any of the types in
/// `TYPES`.
/// @param obj the object to query.
template <typename... TYPES, typename OBJ>
inline bool IsAnyOf(OBJ* obj) {
    if (!obj) {
        return false;
    }
    return obj->TypeInfo().template IsAnyOf<TYPES...>();
}

/// @returns obj dynamically cast to the type `TO` or `nullptr` if this object does not derive from
/// `TO`.
/// @param obj the object to cast from
/// @see CastFlags
template <typename TO, int FLAGS = 0, typename FROM = tint::detail::Infer>
inline TO* As(FROM* obj) {
    auto* as_castable = static_cast<CastableBase*>(obj);
    return Is<TO, FLAGS>(obj) ? static_cast<TO*>(as_castable) : nullptr;
}

/// @returns obj dynamically cast to the type `TO` or `nullptr` if this object does not derive from
/// `TO`.
/// @param obj the object to cast from
/// @see CastFlags
template <typename TO, int FLAGS = 0, typename FROM = tint::detail::Infer>
inline const TO* As(const FROM* obj) {
    auto* as_castable = static_cast<const CastableBase*>(obj);
    return Is<TO, FLAGS>(obj) ? static_cast<const TO*>(as_castable) : nullptr;
}

/// CastableBase is the base class for all Castable objects.
/// It is not encouraged to directly derive from CastableBase without using the Castable helper
/// template.
/// @see Castable
class CastableBase {
  public:
    /// Copy constructor
    CastableBase(const CastableBase&);

    /// Destructor
    virtual ~CastableBase();

    /// Copy assignment
    /// @param other the CastableBase to copy
    /// @returns the new CastableBase
    CastableBase& operator=(const CastableBase& other) = default;

    /// @returns the TypeInfo of the object
    inline const tint::TypeInfo& TypeInfo() const { return *type_info_; }

    /// @returns true if this object is of, or derives from the class `TO`
    template <typename TO>
    inline bool Is() const {
        return tint::Is<TO>(this);
    }

    /// @returns true if this object is of, or derives from the class `TO` and pred(const TO*)
    /// returns true
    /// @param pred predicate function with signature `bool(const TO*)` called iff object is of, or
    /// derives from the class `TO`.
    template <typename TO, int FLAGS = 0, typename Pred = tint::detail::Infer>
    inline bool Is(Pred&& pred) const {
        return tint::Is<TO, FLAGS>(this, std::forward<Pred>(pred));
    }

    /// @returns true if this object is of, or derives from any of the `TO` classes.
    template <typename... TO>
    inline bool IsAnyOf() const {
        return tint::IsAnyOf<TO...>(this);
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if this object does not
    /// derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline TO* As() {
        return tint::As<TO, FLAGS>(this);
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if this object does not
    /// derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline const TO* As() const {
        return tint::As<const TO, FLAGS>(this);
    }

  protected:
    CastableBase() = default;

    /// The type information for the object
    const tint::TypeInfo* type_info_ = nullptr;
};

/// Castable is a helper to derive `CLASS` from `BASE`, automatically implementing the Is() and As()
/// methods, along with a #Base type alias.
///
/// Example usage:
///
/// ```
/// class Animal : public Castable<Animal> {};
///
/// class Sheep : public Castable<Sheep, Animal> {};
///
/// Sheep* cast_to_sheep(Animal* animal) {
///    // You can query whether a Castable is of the given type with Is<T>():
///    printf("animal is a sheep? %s", animal->Is<Sheep>() ? "yes" : "no");
///
///    // You can always just try the cast with As<T>().
///    // If the object is not of the correct type, As<T>() will return nullptr:
///    return animal->As<Sheep>();
/// }
/// ```
template <typename CLASS, typename BASE = CastableBase>
class Castable : public BASE {
  public:
    /// A type alias to this Castable. Commonly used in derived type constructors to forward
    /// constructor arguments to BASE.
    using Base = Castable;

    /// A type alias for `BASE`.
    using TrueBase = BASE;

    /// A type alias for `CLASS`.
    using Class = CLASS;

    /// Constructor
    /// @param arguments the arguments to forward to the base class.
    template <typename... ARGS>
    inline explicit Castable(ARGS&&... arguments) : TrueBase(std::forward<ARGS>(arguments)...) {
        this->type_info_ = &TypeInfo::Of<CLASS>();
    }

    /// @returns true if this object is of, or derives from the class `TO`
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline bool Is() const {
        return tint::Is<TO, FLAGS>(static_cast<const CLASS*>(this));
    }

    /// @returns true if this object is of, or derives from the class `TO` and
    /// pred(const TO*) returns true
    /// @param pred predicate function with signature `bool(const TO*)` called iff
    /// object is of, or derives from the class `TO`.
    template <int FLAGS = 0, typename Pred = tint::detail::Infer>
    inline bool Is(Pred&& pred) const {
        using TO = typename std::remove_pointer<tint::traits::ParameterType<Pred, 0>>::type;
        return tint::Is<TO, FLAGS>(static_cast<const CLASS*>(this), std::forward<Pred>(pred));
    }

    /// @returns true if this object is of, or derives from any of the `TO`
    /// classes.
    template <typename... TO>
    inline bool IsAnyOf() const {
        return tint::IsAnyOf<TO...>(static_cast<const CLASS*>(this));
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if
    /// this object does not derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline TO* As() {
        return tint::TypeInfo::Is<TO, CLASS, FLAGS>(&this->TypeInfo())
                   ? static_cast<TO*>(static_cast<CastableBase*>(this))
                   : nullptr;
    }

    /// @returns this object dynamically cast to the type `TO` or `nullptr` if
    /// this object does not derive from `TO`.
    /// @see CastFlags
    template <typename TO, int FLAGS = 0>
    inline const TO* As() const {
        return tint::TypeInfo::Is<const TO, CLASS, FLAGS>(&this->TypeInfo())
                   ? static_cast<const TO*>(static_cast<const CastableBase*>(this))
                   : nullptr;
    }
};

namespace detail {
/// <code>typename CastableCommonBaseImpl<TYPES>::type</code> resolves to the common base class for
/// all of TYPES.
template <typename... TYPES>
struct CastableCommonBaseImpl {};

/// Alias to typename CastableCommonBaseImpl<TYPES>::type
template <typename... TYPES>
using CastableCommonBase = typename CastableCommonBaseImpl<TYPES...>::type;

/// CastableCommonBaseImpl template specialization for a single type
template <typename T>
struct CastableCommonBaseImpl<T> {
    /// Common base class of a single type is itself
    using type = T;
};

/// CastableCommonBaseImpl A <-> CastableBase specialization
template <typename A>
struct CastableCommonBaseImpl<A, CastableBase> {
    /// Common base class for A and CastableBase is CastableBase
    using type = CastableBase;
};

/// CastableCommonBaseImpl T <-> Ignore specialization
template <typename T>
struct CastableCommonBaseImpl<T, Ignore> {
    /// Resolves to T as the other type is ignored
    using type = T;
};

/// CastableCommonBaseImpl Ignore <-> T specialization
template <typename T>
struct CastableCommonBaseImpl<Ignore, T> {
    /// Resolves to T as the other type is ignored
    using type = T;
};

/// CastableCommonBaseImpl A <-> B specialization
template <typename A, typename B>
struct CastableCommonBaseImpl<A, B> {
    /// The common base class for A, B and OTHERS
    using type = std::conditional_t<tint::traits::IsTypeOrDerived<A, B>,
                                    B,  // A derives from B
                                    CastableCommonBase<A, typename B::TrueBase>>;
};

/// CastableCommonBaseImpl 3+ types specialization
template <typename A, typename B, typename... OTHERS>
struct CastableCommonBaseImpl<A, B, OTHERS...> {
    /// The common base class for A, B and OTHERS
    using type = CastableCommonBase<CastableCommonBase<A, B>, OTHERS...>;
};

}  // namespace detail

/// Resolves to the common most derived type that each of the types in `TYPES` derives from.
template <typename... TYPES>
using CastableCommonBase = tint::detail::CastableCommonBase<TYPES...>;

}  // namespace tint

namespace tint {

using tint::As;
using tint::Is;

}  // namespace tint

TINT_CASTABLE_POP_DISABLE_WARNINGS();

#endif  // SRC_TINT_UTILS_RTTI_CASTABLE_H_
