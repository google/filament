// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_TYPE_ARRAY_H_
#define SRC_TINT_LANG_CORE_TYPE_ARRAY_H_

#include <stdint.h>
#include <optional>
#include <string>
#include <variant>

#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/macros/compiler.h"

namespace tint::core::type {

/// Array holds the type information for Array nodes.
class Array : public Castable<Array, Type> {
  public:
    /// An error message string stating that the array count was expected to be a constant
    /// expression. Used by multiple writers and transforms.
    static const char* const kErrExpectedConstantCount;

    /// Constructor
    /// @param element the array element type
    /// @param count the number of elements in the array.
    /// @param align the byte alignment of the array
    /// @param size the byte size of the array. The size will be 0 if the array element count is
    ///        pipeline overridable.
    /// @param stride the number of bytes from the start of one element of the
    ///        array to the start of the next element
    /// @param implicit_stride the number of bytes from the start of one element
    /// of the array to the start of the next element, if there was no `@stride`
    /// attribute applied.
    Array(Type const* element,
          const ArrayCount* count,
          uint32_t align,
          uint32_t size,
          uint32_t stride,
          uint32_t implicit_stride);

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @return the array element type
    Type const* ElemType() const { return element_; }

    /// @returns the number of elements in the array.
    const ArrayCount* Count() const { return count_; }

    /// @returns the array count if the count is a const-expression, otherwise returns nullopt.
    inline std::optional<uint32_t> ConstantCount() const {
        if (auto* count = count_->As<ConstantArrayCount>()) {
            return count->value;
        }
        return std::nullopt;
    }

    /// @returns the byte alignment of the array
    /// @note this may differ from the alignment of a structure member of this
    /// array type, if the member is annotated with the `@align(n)` attribute.
    uint32_t Align() const override;

    /// @returns the byte size of the array
    /// @note this may differ from the size of a structure member of this array
    /// type, if the member is annotated with the `@size(n)` attribute.
    uint32_t Size() const override;

    /// @returns the number of bytes from the start of one element of the
    /// array to the start of the next element
    uint32_t Stride() const { return stride_; }

    /// @returns the number of bytes from the start of one element of the
    /// array to the start of the next element, if there was no `@stride`
    /// attribute applied
    uint32_t ImplicitStride() const { return implicit_stride_; }

    /// @returns true if the value returned by Stride() matches the element's
    /// natural stride
    bool IsStrideImplicit() const { return stride_ == implicit_stride_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @copydoc Type::Elements
    TypeAndCount Elements(const Type* type_if_invalid = nullptr,
                          uint32_t count_if_invalid = 0) const override;

    /// @copydoc Type::Element
    const Type* Element(uint32_t index) const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    Array* Clone(CloneContext& ctx) const override;

  protected:
    /// Constructor for subclasses
    /// @param hash the immutable hash for the node
    /// @param element the array element type
    /// @param count the number of elements in the array.
    /// @param align the byte alignment of the array
    /// @param size the byte size of the array.
    /// @param stride the number of bytes from the start of one element of the array to the start of
    /// the next element
    Array(size_t hash,
          Type const* element,
          const ArrayCount* count,
          uint32_t align,
          uint32_t size,
          uint32_t stride);

    Type const* const element_;
    const ArrayCount* count_;
    const uint32_t align_;
    const uint32_t size_;
    const uint32_t stride_;
    const uint32_t implicit_stride_;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_ARRAY_H_
