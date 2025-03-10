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

#ifndef SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_CALCULATE_ARRAY_LENGTH_H_
#define SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_CALCULATE_ARRAY_LENGTH_H_

#include <string>

#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::hlsl::writer {

/// CalculateArrayLength is a transform used to replace calls to arrayLength()
/// with a value calculated from the size of the storage buffer.
///
/// @note Depends on the following transforms to have been run first:
/// * SimplifyPointers
class CalculateArrayLength final
    : public Castable<CalculateArrayLength, ast::transform::Transform> {
  public:
    /// BufferSizeIntrinsic is an InternalAttribute that's applied to intrinsic
    /// functions used to obtain the runtime size of a storage buffer.
    class BufferSizeIntrinsic final : public Castable<BufferSizeIntrinsic, ast::InternalAttribute> {
      public:
        /// Constructor
        /// @param generation_id the identifier of the program that owns this node
        /// @param nid the unique node identifier
        BufferSizeIntrinsic(GenerationID generation_id, ast::NodeID nid);
        /// Destructor
        ~BufferSizeIntrinsic() override;

        /// @return "buffer_size"
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const BufferSizeIntrinsic* Clone(ast::CloneContext& ctx) const override;
    };

    /// Constructor
    CalculateArrayLength();
    /// Destructor
    ~CalculateArrayLength() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;
};

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_CALCULATE_ARRAY_LENGTH_H_
