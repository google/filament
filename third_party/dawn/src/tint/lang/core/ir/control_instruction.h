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

#ifndef SRC_TINT_LANG_CORE_IR_CONTROL_INSTRUCTION_H_
#define SRC_TINT_LANG_CORE_IR_CONTROL_INSTRUCTION_H_

#include <utility>

#include "src/tint/lang/core/ir/operand_instruction.h"

// Forward declarations
namespace tint::core::ir {
class Block;
class Exit;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// Base class of instructions that perform control flow to two or more blocks, owned by the
/// ControlInstruction.
class ControlInstruction : public Castable<ControlInstruction, OperandInstruction<1, 1>> {
  public:
    /// Constructor
    /// @param id the instruction id
    explicit ControlInstruction(Id id);

    /// Destructor
    ~ControlInstruction() override;

    /// Calls @p cb for each block owned by this control instruction
    /// @param cb the function to call once for each block
    virtual void ForeachBlock(const std::function<void(ir::Block*)>& cb) = 0;

    /// Calls @p cb for each block owned by this control instruction
    /// @param cb the function to call once for each block
    virtual void ForeachBlock(const std::function<void(const ir::Block*)>& cb) const = 0;

    /// @return All the exits for the flow control instruction
    const Hashset<Exit*, 2>& Exits() const { return exits_; }

    /// Adds the exit to the flow control instruction
    /// @param exit the exit instruction
    void AddExit(Exit* exit);

    /// Removes the exit to the flow control instruction
    /// @param exit the exit instruction
    void RemoveExit(Exit* exit);

    /// @copydoc Instruction::Destroy
    void Destroy() override;

  protected:
    /// The flow control exits
    Hashset<Exit*, 2> exits_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CONTROL_INSTRUCTION_H_
