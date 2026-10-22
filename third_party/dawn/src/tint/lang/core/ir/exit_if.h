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

#ifndef SRC_TINT_LANG_CORE_IR_EXIT_IF_H_
#define SRC_TINT_LANG_CORE_IR_EXIT_IF_H_

#include <string>

#include "src/tint/lang/core/ir/exit.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class If;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A exit if instruction.
class ExitIf final : public Castable<ExitIf, Exit> {
  public:
    /// The base offset in Operands() for the args
    static constexpr size_t kArgsOperandOffset = 0;

    /// Constructor (no operands, no if)
    /// @param id the instruction id
    explicit ExitIf(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param i the if being exited
    /// @param args the target MultiInBlock arguments
    ExitIf(Id id, ir::If* i, VectorRef<Value*> args = tint::Empty);

    ~ExitIf() override;

    /// @copydoc Instruction::Clone()
    ExitIf* Clone(CloneContext& ctx) override;

    /// Re-associates the exit with the given if instruction
    /// @param i the new If to exit from
    void SetIf(ir::If* i);

    /// @returns the if being exited
    ir::If* If();

    /// @returns the if being exited
    const ir::If* If() const;

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "exit_if"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_EXIT_IF_H_
