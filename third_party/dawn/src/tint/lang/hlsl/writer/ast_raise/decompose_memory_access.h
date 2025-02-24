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

#ifndef SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_DECOMPOSE_MEMORY_ACCESS_H_
#define SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_DECOMPOSE_MEMORY_ACCESS_H_

#include <string>

#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::hlsl::writer {

/// DecomposeMemoryAccess is a transform used to replace storage and uniform buffer accesses with a
/// combination of load, store or atomic functions on primitive types.
class DecomposeMemoryAccess final
    : public Castable<DecomposeMemoryAccess, ast::transform::Transform> {
  public:
    /// Intrinsic is an InternalAttribute that's used to decorate a stub function so that the HLSL
    /// transforms this into calls to
    /// `[RW]ByteAddressBuffer.Load[N]()` or `[RW]ByteAddressBuffer.Store[N]()`,
    /// with a possible cast.
    class Intrinsic final : public Castable<Intrinsic, ast::InternalAttribute> {
      public:
        /// Intrinsic op
        enum class Op {
            kLoad,
            kStore,
            kAtomicLoad,
            kAtomicStore,
            kAtomicAdd,
            kAtomicSub,
            kAtomicMax,
            kAtomicMin,
            kAtomicAnd,
            kAtomicOr,
            kAtomicXor,
            kAtomicExchange,
            kAtomicCompareExchangeWeak,
        };

        /// Intrinsic data type
        enum class DataType {
            kU32,
            kF32,
            kI32,
            kF16,
            kVec2U32,
            kVec2F32,
            kVec2I32,
            kVec2F16,
            kVec3U32,
            kVec3F32,
            kVec3I32,
            kVec3F16,
            kVec4U32,
            kVec4F32,
            kVec4I32,
            kVec4F16,
        };

        /// Constructor
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        /// @param o the op of the intrinsic
        /// @param type the data type of the intrinsic
        /// @param address_space the address space of the buffer
        /// @param buffer the storage or uniform buffer identifier
        Intrinsic(GenerationID pid,
                  ast::NodeID nid,
                  Op o,
                  DataType type,
                  core::AddressSpace address_space,
                  const ast::IdentifierExpression* buffer);
        /// Destructor
        ~Intrinsic() override;

        /// @return a short description of the internal attribute which will be
        /// displayed as `@internal(<name>)`
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const Intrinsic* Clone(ast::CloneContext& ctx) const override;

        /// @return true if op is atomic
        bool IsAtomic() const;

        /// @return the buffer that this intrinsic operates on
        const ast::IdentifierExpression* Buffer() const;

        /// The op of the intrinsic
        const Op op;

        /// The type of the intrinsic
        const DataType type;

        /// The address space of the buffer this intrinsic operates on
        const core::AddressSpace address_space;
    };

    /// Constructor
    DecomposeMemoryAccess();
    /// Destructor
    ~DecomposeMemoryAccess() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_AST_RAISE_DECOMPOSE_MEMORY_ACCESS_H_
