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

#ifndef SRC_TINT_LANG_WGSL_IR_BUILTIN_CALL_H_
#define SRC_TINT_LANG_WGSL_IR_BUILTIN_CALL_H_

#include <string>

#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/intrinsic/dialect.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::wgsl::ir {

/// A WGSL builtin call instruction in the IR.
class BuiltinCall : public Castable<BuiltinCall, core::ir::BuiltinCall> {
  public:
    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param fn the builtin function
    /// @param args the conversion arguments
    BuiltinCall(Id id,
                core::ir::InstructionResult* result,
                BuiltinFn fn,
                VectorRef<core::ir::Value*> args = tint::Empty);

    ~BuiltinCall() override;

    /// @copydoc core::ir::Instruction::Clone()
    BuiltinCall* Clone(core::ir::CloneContext& ctx) override;

    /// @returns the builtin function
    BuiltinFn Func() const { return fn_; }

    /// @returns the identifier for the function
    size_t FuncId() const override { return static_cast<size_t>(fn_); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return std::string("wgsl.") + str(fn_); }

    /// @returns the table data to validate this builtin
    const core::intrinsic::TableData& TableData() const override {
        return intrinsic::Dialect::kData;
    }

  private:
    BuiltinFn fn_;
};

}  // namespace tint::wgsl::ir

#endif  // SRC_TINT_LANG_WGSL_IR_BUILTIN_CALL_H_
