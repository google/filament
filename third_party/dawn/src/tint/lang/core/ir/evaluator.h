// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_EVALUATOR_H_
#define SRC_TINT_LANG_CORE_IR_EVALUATOR_H_

#include "src/tint/lang/core/constant/eval.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_unary.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/value.h"

namespace tint::core::ir {

/// An evaluator to take a given `ir::Value` and return the result of evaluating the expression.
class Evaluator {
  public:
    /// Constructor
    /// @param builder the ir builder
    explicit Evaluator(ir::Builder& builder);
    /// Destructor
    ~Evaluator();

    /// Evaluate the given `src` expression.
    /// @param src the source expression
    /// @returns the generated constant or a failure result.
    diag::Result<core::ir::Constant*> Evaluate(core::ir::Value* src);

  private:
    using EvalResult = Result<const core::constant::Value*>;

    diag::Diagnostic& AddError(Source src);
    Source SourceOf(core::ir::Instruction* val);

    EvalResult EvalBitcast(core::ir::Bitcast* bc);
    EvalResult EvalValue(core::ir::Value* val);
    EvalResult EvalAccess(core::ir::Access* a);
    EvalResult EvalConvert(core::ir::Convert* c);
    EvalResult EvalConstruct(core::ir::Construct* c);
    EvalResult EvalSwizzle(core::ir::Swizzle* s);
    EvalResult EvalOverride(core::ir::Override* o);
    EvalResult EvalUnary(core::ir::CoreUnary* u);
    EvalResult EvalBinary(core::ir::CoreBinary* cb);
    EvalResult EvalCoreBuiltinCall(core::ir::CoreBuiltinCall* c);
    EvalResult EvalConstExprIf(core::ir::ConstExprIf* c);

    ir::Builder& b_;
    diag::List diagnostics_;
    core::constant::Eval const_eval_;
};

namespace eval {

/// Evaluate the given `inst` with the provided `b`.
/// @param b the builder
/// @param inst the instruction
/// @returns the evaluated constant for `inst` or a `Failure` otherwise.
diag::Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Instruction* inst);

/// Evaluate the given `val` with the provided `b`.
/// @param b the builder
/// @param val the value
/// @returns the evaluated constant for `val` or a `Failure` otherwise.
diag::Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Value* val);

}  // namespace eval

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_EVALUATOR_H_
