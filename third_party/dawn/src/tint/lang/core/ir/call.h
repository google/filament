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

#ifndef SRC_TINT_LANG_CORE_IR_CALL_H_
#define SRC_TINT_LANG_CORE_IR_CALL_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A Call instruction in the IR.
class Call : public Castable<Call, OperandInstruction<4, 1>> {
  public:
    ~Call() override;

    /// @returns the offset of the arguments in Operands()
    virtual size_t ArgsOperandOffset() const { return 0; }

    /// Sets the explicit template params for the call
    void SetExplicitTemplateParams(VectorRef<const core::type::Type*> params) {
        explicit_template_params_ = params;
    }
    /// Retrieves the explicit template params for the call
    tint::VectorRef<const core::type::Type*> ExplicitTemplateParams() const {
        return explicit_template_params_;
    }

    /// @returns the call arguments
    tint::Slice<Value* const> Args() { return operands_.Slice().Offset(ArgsOperandOffset()); }

    /// @returns the call arguments
    tint::Slice<const Value* const> Args() const {
        return operands_.Slice().Offset(ArgsOperandOffset());
    }

    /// Sets the argument at `idx` of `arg`. `idx` must be within bounds of the current argument
    /// set.
    void SetArg(size_t idx, ir::Value* arg) { SetOperand(ArgsOperandOffset() + idx, arg); }

    /// Append a new argument to the argument list for this call instruction.
    /// @param arg the argument value to append
    void AppendArg(ir::Value* arg) { AddOperand(operands_.Length(), arg); }

    /// @returns the side effects for this instruction
    Accesses GetSideEffects() const override { return Accesses{Access::kLoad, Access::kStore}; }

  protected:
    /// Constructor
    explicit Call(Id id);

    Vector<const core::type::Type*, 1> explicit_template_params_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CALL_H_
