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

#ifndef SRC_TINT_LANG_CORE_IR_SWITCH_H_
#define SRC_TINT_LANG_CORE_IR_SWITCH_H_

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/utils/containers/const_propagating_ptr.h"

// Forward declarations
namespace tint::core::ir {
class Constant;
class MultiInBlock;
}  // namespace tint::core::ir

namespace tint::core::ir {
/// Switch instruction.
///
/// ```
///                           in
///                            ┃
///     ╌╌╌╌╌╌╌╌┲━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━┱╌╌╌╌╌╌╌╌
///             ▼              ▼              ▼
///        ┌────────┐     ┌────────┐     ┌────────┐
///        │ Case A │     │ Case B │     │ Case C │
///        └────────┘     └────────┘     └────────┘
///  ExitSwitch ┃   ExitSwitch ┃   ExitSwitch ┃
///             ┃              ┃              ┃
///     ╌╌╌╌╌╌╌╌┺━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━┹╌╌╌╌╌╌╌╌
///                            ┃
///                            ▼
///                           out
/// ```
class Switch final : public Castable<Switch, ControlInstruction> {
  public:
    /// The offset in Operands() for the condition
    static constexpr size_t kConditionOperandOffset = 0;

    /// A case selector
    struct CaseSelector {
        /// @returns true if this is a default selector
        bool IsDefault() const { return val == nullptr; }

        /// The selector value, or nullptr if this is the default selector
        ConstPropagatingPtr<Constant> val;
    };

    /// A case label in the struct
    struct Case {
        /// The case selector for this node
        Vector<CaseSelector, 4> selectors;

        /// The case block.
        ConstPropagatingPtr<ir::Block> block;
    };

    /// Constructor (no results, no operands, no cases)
    /// @param id the instruction id
    explicit Switch(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param cond the condition
    Switch(Id id, Value* cond);

    ~Switch() override;

    /// @copydoc Instruction::Clone()
    Switch* Clone(CloneContext& ctx) override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(ir::Block*)>& cb) override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(const ir::Block*)>& cb) const override;

    /// @returns the switch cases
    Vector<Case, 4>& Cases() { return cases_; }

    /// @returns the switch cases
    VectorRef<Case> Cases() const { return cases_; }

    /// @returns the condition
    Value* Condition() { return Operand(kConditionOperandOffset); }

    /// @returns the condition
    const Value* Condition() const { return Operand(kConditionOperandOffset); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "switch"; }

    /// @returns the default block for the switch, or nullptr if non-exists
    ir::Block* DefaultBlock() {
        for (auto& c : cases_) {
            for (auto& s : c.selectors) {
                if (s.IsDefault()) {
                    return c.block.Get();
                }
            }
        }
        return nullptr;
    }

  private:
    Vector<Case, 4> cases_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_SWITCH_H_
