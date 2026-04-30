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

#ifndef SRC_TINT_LANG_CORE_IR_LOOP_H_
#define SRC_TINT_LANG_CORE_IR_LOOP_H_

#include <string>

#include "src/tint/lang/core/ir/control_instruction.h"

// Forward declarations
namespace tint::core::ir {
class MultiInBlock;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// Loop instruction.
///
/// ```
///                     in
///                      ┃
///                      ┣━━━━━━━━━━━┓
///                      ▼           ┃
///             ┌─────────────────┐  ┃
///             │   Initializer   │  ┃
///             │    (optional)   │  ┃
///             └─────────────────┘  ┃
///        NextIteration ┃           ┃
///                      ┃◀━━━━━━━━━━┫
///                      ▼           ┃
///             ┌─────────────────┐  ┃
///          ┏━━│       Body      │  ┃
///          ┃  └─────────────────┘  ┃
///          ┃  Continue ┃           ┃ NextIteration
///          ┃           ▼           ┃
///          ┃  ┌─────────────────┐  ┃ BreakIf(false)
/// ExitLoop ┃  │   Continuing    │━━┛
///          ┃  │  (optional)     │
///          ┃  └─────────────────┘
///          ┃           ┃
///          ┃           ┃ BreakIf(true)
///          ┗━━━━━━━━━━▶┃
///                      ▼
///                     out
///
/// ```
class Loop final : public Castable<Loop, ControlInstruction> {
  public:
    /// Constructor (no results, no operands, no blocks)
    /// @param id the instruction id
    explicit Loop(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param i the initializer block
    /// @param b the body block
    /// @param c the continuing block
    Loop(Id id, ir::Block* i, ir::MultiInBlock* b, ir::MultiInBlock* c);
    ~Loop() override;

    /// @copydoc Instruction::Clone()
    Loop* Clone(CloneContext& ctx) override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(ir::Block*)>& cb) override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(const ir::Block*)>& cb) const override;

    /// @returns the switch initializer block
    ir::Block* Initializer() { return initializer_; }

    /// @returns the switch initializer block
    const ir::Block* Initializer() const { return initializer_; }

    /// @returns true if the loop uses an initializer block. If true, then the Loop first branches
    /// to the initializer block, otherwise it first branches to the body block.
    bool HasInitializer() const;

    /// @param block the new switch initializer block
    void SetInitializer(ir::Block* block);

    /// @returns the switch start block
    ir::MultiInBlock* Body() { return body_; }

    /// @returns the switch start block
    const ir::MultiInBlock* Body() const { return body_; }

    /// @param block the new switch body block
    void SetBody(ir::MultiInBlock* block);

    /// @returns the switch continuing block
    ir::MultiInBlock* Continuing() { return continuing_; }

    /// @returns the switch continuing block
    const ir::MultiInBlock* Continuing() const { return continuing_; }

    /// @returns true if the loop uses an continuing block. If true, then the Loop first branches
    /// to the continuing block, otherwise it first branches to the body block.
    bool HasContinuing() const;

    /// @param block the new switch continuing block
    void SetContinuing(ir::MultiInBlock* block);

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "loop"; }

  private:
    ir::Block* initializer_ = nullptr;
    ir::MultiInBlock* body_ = nullptr;
    ir::MultiInBlock* continuing_ = nullptr;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_LOOP_H_
