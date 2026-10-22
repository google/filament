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

#ifndef SRC_TINT_LANG_CORE_TYPE_VECTOR_H_
#define SRC_TINT_LANG_CORE_TYPE_VECTOR_H_

#include <string>

#include "src/tint/lang/core/type/type.h"

namespace tint::core::type {

/// A vector type.
class Vector : public Castable<Vector, type::Type> {
  public:
    /// Constructor
    /// @param subtype the vector element type
    /// @param size the number of elements in the vector
    /// @param packed the optional 'packed' modifier
    Vector(Type const* subtype, uint32_t size, bool packed = false);

    /// Destructor
    ~Vector() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the type of the vector elements
    const type::Type* Type() const { return subtype_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @returns the number of elements in the vector
    uint32_t Width() const { return width_; }

    /// @returns the size in bytes of the type. This may include tail padding.
    uint32_t Size() const override;

    /// @returns the alignment in bytes of the type. This may include tail padding.
    uint32_t Align() const override;

    /// @returns `true` if this vector is packed, false otherwise
    bool Packed() const { return packed_; }

    /// @param width the width of the vector
    /// @returns the size in bytes of a vector of the given width.
    static uint32_t SizeOf(uint32_t width);

    /// @param width the width of the vector
    /// @returns the alignment in bytes of a vector of the given width.
    static uint32_t AlignOf(uint32_t width);

    /// @copydoc Type::Elements
    TypeAndCount Elements(const type::Type* type_if_invalid = nullptr,
                          uint32_t count_if_invalid = 0) const override;

    /// @copydoc Type::Element
    const type::Type* Element(uint32_t index) const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    Vector* Clone(CloneContext& ctx) const override;

  private:
    type::Type const* const subtype_;
    const uint32_t width_;
    const bool packed_;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_VECTOR_H_
