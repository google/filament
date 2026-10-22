// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_TYPE_EXPLICIT_LAYOUT_ARRAY_H_
#define SRC_TINT_LANG_SPIRV_TYPE_EXPLICIT_LAYOUT_ARRAY_H_

#include <cstdint>
#include <string>

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::spirv::type {

/// ExplicitLayoutArray holds the type information for ExplicitLayoutArray nodes.
class ExplicitLayoutArray : public Castable<ExplicitLayoutArray, core::type::Array> {
  public:
    /// Constructor
    /// @param element the array element type
    /// @param count the number of elements in the array.
    /// @param size the byte size of the array.
    /// @param stride the number of bytes from the start of one element of the array to the start of
    /// the next element
    ExplicitLayoutArray(Type const* element,
                        const core::type::ArrayCount* count,
                        uint32_t size,
                        uint32_t stride);

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns true if the stride is implicit
    bool IsStrideImplicit() const { return ImplicitStride() == stride_; }

    /// @returns the stride
    uint32_t Stride() const { return stride_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    ExplicitLayoutArray* Clone(core::type::CloneContext& ctx) const override;

  private:
    // The explicit stride
    uint32_t stride_;
};

}  // namespace tint::spirv::type

#endif  // SRC_TINT_LANG_SPIRV_TYPE_EXPLICIT_LAYOUT_ARRAY_H_
