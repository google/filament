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

#ifndef SRC_TINT_LANG_CORE_IR_CONTINUE_H_
#define SRC_TINT_LANG_CORE_IR_CONTINUE_H_

#include <string>

#include "src/tint/lang/core/ir/terminator.h"
#include "src/tint/utils/containers/const_propagating_ptr.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class Loop;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A continue instruction.
class Continue final : public Castable<Continue, Terminator> {
  public:
    /// The base offset in Operands() for the args
    static constexpr size_t kArgsOperandOffset = 0;

    /// Constructor (no operands, no loop)
    /// @param id the instruction id
    explicit Continue(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param loop the loop owning the continue block
    /// @param args the arguments for the MultiInBlock
    Continue(Id id, ir::Loop* loop, VectorRef<Value*> args = tint::Empty);
    ~Continue() override;

    /// @copydoc Instruction::Clone()
    Continue* Clone(CloneContext& ctx) override;

    /// @returns the loop owning the continue block
    ir::Loop* Loop() { return loop_; }

    /// @returns the loop owning the continue block
    const ir::Loop* Loop() const { return loop_; }

    /// @param loop the new loop owning the continue block
    void SetLoop(ir::Loop* loop);

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "continue"; }

  private:
    ConstPropagatingPtr<ir::Loop> loop_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CONTINUE_H_
