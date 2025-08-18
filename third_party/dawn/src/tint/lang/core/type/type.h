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

#ifndef SRC_TINT_LANG_CORE_TYPE_TYPE_H_
#define SRC_TINT_LANG_CORE_TYPE_TYPE_H_

#include <functional>
#include <string>

#include "src/tint/lang/core/type/clone_context.h"
#include "src/tint/lang/core/type/unique_node.h"
#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint {
class ProgramBuilder;
class SymbolTable;
}  // namespace tint
namespace tint::core::type {
class Type;
}  // namespace tint::core::type

namespace tint::core::type {

/// Flag is an enumerator of type flag bits, used by Flags.
enum Flag {
    /// Type is constructable.
    /// @see https://gpuweb.github.io/gpuweb/wgsl/#constructible-types
    kConstructable,
    /// Type has a creation-fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    kCreationFixedFootprint,
    /// Type has a fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    kFixedFootprint,
    /// Type is host-shareable.
    /// @see https://www.w3.org/TR/WGSL/#host-shareable
    kHostShareable,
};

/// An alias to tint::EnumSet<Flag>
using Flags = tint::EnumSet<Flag>;

/// TypeAndCount holds a type and count
struct TypeAndCount {
    /// The type
    const Type* type = nullptr;
    /// The count
    uint32_t count = 0;
};

/// Equality operator.
/// @param lhs the LHS TypeAndCount
/// @param rhs the RHS TypeAndCount
/// @returns true if the two TypeAndCounts have the same type and count
inline bool operator==(TypeAndCount lhs, TypeAndCount rhs) {
    return lhs.type == rhs.type && lhs.count == rhs.count;
}

/// Base class for a type in the system
class Type : public Castable<Type, UniqueNode> {
  public:
    /// Destructor
    ~Type() override;

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    virtual std::string FriendlyName() const = 0;

    /// @returns the inner most pointee type if this is a pointer, `this`
    /// otherwise
    const Type* UnwrapPtr() const;

    /// @returns the inner type if this is a reference, `this` otherwise
    const Type* UnwrapRef() const;

    /// @returns the inner type if this is a pointer or a reference, `this` otherwise
    const Type* UnwrapPtrOrRef() const;

    /// @returns the size in bytes of the type. This may include tail padding.
    /// @note opaque types will return a size of 0.
    virtual uint32_t Size() const;

    /// @returns the alignment in bytes of the type. This may include tail
    /// padding.
    /// @note opaque types will return a size of 0.
    virtual uint32_t Align() const;

    /// @param ctx the clone context
    /// @returns a clone of this type created in the provided context
    virtual Type* Clone(CloneContext& ctx) const = 0;

    /// @returns the flags on the type
    core::type::Flags Flags() { return flags_; }

    /// @returns true if type is constructable
    /// https://gpuweb.github.io/gpuweb/wgsl/#constructible-types
    inline bool IsConstructible() const { return flags_.Contains(Flag::kConstructable); }

    /// @returns true has a creation-fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    inline bool HasCreationFixedFootprint() const {
        return flags_.Contains(Flag::kCreationFixedFootprint);
    }

    /// @returns true has a fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    inline bool HasFixedFootprint() const { return flags_.Contains(Flag::kFixedFootprint); }

    /// @returns true if type is host-shareable
    /// https://www.w3.org/TR/WGSL/#host-shareable
    inline bool IsHostShareable() const { return flags_.Contains(Flag::kHostShareable); }

    /// @returns true if the type is a scalar
    bool IsScalar() const;
    /// @returns true if this type is a float scalar
    bool IsFloatScalar() const;
    /// @returns true if this type is a float matrix
    bool IsFloatMatrix() const;
    /// @returns true if this type is a float vector
    bool IsFloatVector() const;
    /// @returns true if this type is a float scalar or vector
    bool IsFloatScalarOrVector() const;
    /// @returns true if this type is an integer scalar
    bool IsIntegerScalar() const;
    /// @returns true if this type is a integer vector
    bool IsIntegerVector() const;
    /// @returns true if this type is an integer scalar or vector
    bool IsIntegerScalarOrVector() const;
    /// @returns true if this type is a signed integer scalar
    bool IsSignedIntegerScalar() const;
    /// @returns true if this type is a signed integer vector
    bool IsSignedIntegerVector() const;
    /// @returns true if this type is a signed scalar or vector
    bool IsSignedIntegerScalarOrVector() const;
    /// @returns true if this type is an unsigned integer scalar
    bool IsUnsignedIntegerScalar() const;
    /// @returns true if this type is an unsigned vector
    bool IsUnsignedIntegerVector() const;
    /// @returns true if this type is an unsigned scalar or vector
    bool IsUnsignedIntegerScalarOrVector() const;
    /// @returns true if this type is an abstract integer scalar or vector
    bool IsAbstractScalarOrVector() const;
    /// @returns true if this type is boolean scalar or vector
    bool IsBoolScalarOrVector() const;
    /// @returns true if this type is boolean vector
    bool IsBoolVector() const;
    /// @returns true if this type is a numeric scale or vector
    bool IsNumericScalarOrVector() const;
    /// @returns true if this type is a handle type
    virtual bool IsHandle() const;
    /// @returns true if this type is an abstract type. It could be a numeric directly or an
    /// abstract container which holds an abstract numeric
    bool IsAbstract() const;

    /// kNoConversion is returned from ConversionRank() when the implicit conversion is not
    /// permitted.
    static constexpr uint32_t kNoConversion = 0xffffffffu;

    /// ConversionRank returns the implicit conversion rank when attempting to convert `from` to
    /// `to`. Lower ranks are preferred over higher ranks.
    /// @param from the source type
    /// @param to the destination type
    /// @returns the rank value for converting from type `from` to type `to`, or #kNoConversion if
    /// the implicit conversion is not allowed.
    /// @see https://www.w3.org/TR/WGSL/#conversion-rank
    static uint32_t ConversionRank(const Type* from, const Type* to);

    /// @param type_if_invalid the type to return if this type has no child elements.
    /// @param count_if_invalid the count to return if this type has no child elements, or the
    /// number is unbounded.
    /// @returns The child element type and the number of child elements held by this type.
    /// If this type has no child element types, then @p invalid is returned.
    /// If this type can hold a mix of different elements types (like a Struct), then
    /// `[type_if_invalid, N]` is returned, where `N` is the number of elements.
    /// If this type is unbounded in size (e.g. runtime sized arrays), then the returned count will
    /// equal `count_if_invalid`.
    ///
    /// Examples:
    ///  * Elements() of `array<vec3<f32>, 5>` returns `[vec3<f32>, 5]`.
    ///  * Elements() of `array<f32>` returns `[f32, count_if_invalid]`.
    ///  * Elements() of `struct S { a : f32, b : i32 }` returns `[count_if_invalid, 2]`.
    ///  * Elements() of `struct S { a : i32, b : i32 }` also returns `[count_if_invalid, 2]`.
    virtual TypeAndCount Elements(const Type* type_if_invalid = nullptr,
                                  uint32_t count_if_invalid = 0) const;

    /// @param index the i'th element index to return
    /// @returns The child element with the given index, or nullptr if the element does not exist.
    ///
    /// Examples:
    ///  * Element(1) of `mat3x2<f32>` returns `vec2<f32>`.
    ///  * Element(1) of `array<vec3<f32>, 5>` returns `vec3<f32>`.
    ///  * Element(0) of `struct S { a : f32, b : i32 }` returns `f32`.
    ///  * Element(0) of `f32` returns `nullptr`.
    ///  * Element(3) of `vec3<f32>` returns `nullptr`.
    ///  * Element(3) of `struct S { a : f32, b : i32 }` returns `nullptr`.
    virtual const Type* Element(uint32_t index) const;

    /// @returns the most deeply nested element of the type. For non-composite types,
    /// DeepestElement() will return this type. Examples:
    ///  * Element() of `f32` returns `f32`.
    ///  * Element() of `vec3<f32>` returns `f32`.
    ///  * Element() of `mat3x2<f32>` returns `f32`.
    ///  * Element() of `array<vec3<f32>, 5>` returns `f32`.
    ///  * Element() of `struct S { a : f32, b : i32 }` returns `S`.
    const Type* DeepestElement() const;

    /// @param types the list of types
    /// @returns the lowest-ranking type that all types in `types` can be implicitly converted to,
    ///          or nullptr if there is no consistent common type across all types in `types`.
    /// @see https://www.w3.org/TR/WGSL/#conversion-rank
    static const Type* Common(VectorRef<const Type*> types);

  protected:
    /// Constructor
    /// @param hash the immutable hash for the node
    /// @param flags the flags of this type
    Type(size_t hash, core::type::Flags flags);

    /// The flags of this type.
    const core::type::Flags flags_;
};

}  // namespace tint::core::type

namespace std {

/// std::hash specialization for tint::core::type::Type
template <>
struct hash<tint::core::type::Type> {
    /// @param type the type to obtain a hash from
    /// @returns the hash of the type
    size_t operator()(const tint::core::type::Type& type) const { return type.unique_hash; }
};

/// std::equal_to specialization for tint::core::type::Type
template <>
struct equal_to<tint::core::type::Type> {
    /// @param a the first type to compare
    /// @param b the second type to compare
    /// @returns true if the two types are equal
    bool operator()(const tint::core::type::Type& a, const tint::core::type::Type& b) const {
        return a.Equals(b);
    }
};

}  // namespace std

#endif  // SRC_TINT_LANG_CORE_TYPE_TYPE_H_
