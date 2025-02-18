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

#include "src/tint/lang/core/ir/evaluator.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir {
namespace eval {

Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Instruction* inst) {
    return Eval(b, inst->Result(0));
}

Result<core::ir::Constant*> Eval(core::ir::Builder& b, core::ir::Value* val) {
    ir::Evaluator e(b);
    return e.Evaluate(val);
}

}  // namespace eval

Evaluator::Evaluator(ir::Builder& builder)
    : b_(builder), const_eval_(b_.ir.constant_values, diagnostics_) {}

Evaluator::~Evaluator() = default;

Result<core::ir::Constant*> Evaluator::Evaluate(core::ir::Value* src) {
    auto res = EvalValue(src);
    if (res != Success) {
        return Failure(diagnostics_);
    }
    if (!res.Get()) {
        return nullptr;
    }

    return b_.Constant(res.Get());
}

diag::Diagnostic& Evaluator::AddError(Source src) {
    diag::Diagnostic diag;
    diag.source = src;
    return diagnostics_.Add(diag);
}

Source Evaluator::SourceOf(core::ir::Instruction* val) {
    return b_.ir.SourceOf(val);
}

Evaluator::EvalResult Evaluator::EvalValue(core::ir::Value* val) {
    return tint::Switch(
        val,  //
        [&](core::ir::Constant* c) { return c->Value(); },
        [&](core::ir::InstructionResult* r) {
            return tint::Switch(
                r->Instruction(),  //
                [&](core::ir::Bitcast* bc) { return EvalBitcast(bc); },
                [&](core::ir::Access* a) { return EvalAccess(a); },
                [&](core::ir::Construct* c) { return EvalConstruct(c); },
                [&](core::ir::Convert* c) { return EvalConvert(c); },
                [&](core::ir::CoreBinary* cb) { return EvalBinary(cb); },
                [&](core::ir::CoreBuiltinCall* c) { return EvalCoreBuiltinCall(c); },
                [&](core::ir::CoreUnary* u) { return EvalUnary(u); },
                [&](core::ir::Swizzle* s) { return EvalSwizzle(s); },  //
                [&](Default) {
                    // Treat any unknown instruction as a termination point for trying to eval.
                    return nullptr;
                });
        },
        TINT_ICE_ON_NO_MATCH);
}

Evaluator::EvalResult Evaluator::EvalBitcast(core::ir::Bitcast* bc) {
    auto val = EvalValue(bc->Val());
    if (val != Success) {
        return val;
    }
    // Check if the value could be evaluated
    if (!val.Get()) {
        return nullptr;
    }

    auto r = const_eval_.bitcast(bc->Result(0)->Type(), Vector{val.Get()}, SourceOf(bc));
    if (r != Success) {
        return Failure();
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalAccess(core::ir::Access* a) {
    auto obj_res = EvalValue(a->Object());
    if (obj_res != Success) {
        return obj_res;
    }
    // Check if the object could be evaluated
    if (!obj_res.Get()) {
        return nullptr;
    }
    auto* obj = obj_res.Get();

    for (auto* idx : a->Indices()) {
        auto val = EvalValue(idx);
        if (val != Success) {
            return val;
        }
        // Check if the value could be evaluated
        if (!val.Get()) {
            return nullptr;
        }
        TINT_ASSERT(val.Get()->Is<core::constant::Value>());

        auto res = const_eval_.Index(obj, a->Result(0)->Type(), val.Get(), SourceOf(a));
        if (res != Success) {
            return Failure();
        }
        obj = res.Get();
    }

    return obj;
}

Evaluator::EvalResult Evaluator::EvalConstruct(core::ir::Construct* c) {
    auto table = core::intrinsic::Table<core::intrinsic::Dialect>(b_.ir.Types(), b_.ir.symbols);

    auto result_ty = c->Result(0)->Type();

    Vector<const core::type::Type*, 4> arg_types;
    arg_types.Reserve(c->Args().Length());
    Vector<const core::constant::Value*, 4> arg_values;
    arg_values.Reserve(c->Args().Length());

    for (auto* arg : c->Args()) {
        arg_types.Push(arg->Type());

        auto val = EvalValue(arg);
        if (val != Success) {
            return val;
        }
        // Check if the value could be evaluated
        if (!val.Get()) {
            return nullptr;
        }
        arg_values.Push(val.Get());
    }

    auto mat_vec = [&](const core::type::Type* type,
                       core::intrinsic::CtorConv intrinsic) -> constant::Eval::Result {
        auto op =
            table.Lookup(intrinsic, Vector{type}, arg_types, core::EvaluationStage::kOverride);
        if (op != Success) {
            AddError(SourceOf(c)) << "unable to find intrinsic for construct: " << op.Failure();
            return constant::Eval::Error();
        }
        if (!op->const_eval_fn) {
            AddError(SourceOf(c)) << "unhandled type constructor";
            return constant::Eval::Error();
        }
        auto r = (const_eval_.*op->const_eval_fn)(result_ty, arg_values, SourceOf(c));
        if (r != Success) {
            return constant::Eval::Error();
        }
        return r.Get();
    };

    // Dispatch to the appropriate const eval function.
    auto r = tint::Switch(
        result_ty,  //
        [&](const core::type::Array*) {
            return const_eval_.ArrayOrStructCtor(result_ty, arg_values);
        },
        [&](const core::type::Struct*) {
            return const_eval_.ArrayOrStructCtor(result_ty, arg_values);
        },
        [&](const core::type::Vector* vec) {
            return mat_vec(vec->Type(), core::intrinsic::VectorCtorConv(vec->Width()));
        },
        [&](const core::type::Matrix* mat) {
            return mat_vec(mat->Type(),
                           core::intrinsic::MatrixCtorConv(mat->Columns(), mat->Rows()));
        },
        [&](Default) {
            if (!result_ty->Is<core::type::Scalar>()) {
                AddError(SourceOf(c)) << "unhandled type constructor";
                return core::constant::Eval::Result(nullptr);
            }
            // For scalars, this must be an identity constructor.
            if (arg_values[0]->Type() != result_ty) {
                AddError(SourceOf(c)) << "invalid type constructor";
                return core::constant::Eval::Result(nullptr);
            }
            return const_eval_.Identity(result_ty, arg_values, SourceOf(c));
        });

    if (r != Success) {
        return Failure();
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalConvert(core::ir::Convert* c) {
    auto val = EvalValue(c->Args()[0]);
    if (val != Success) {
        return val;
    }
    // Check if the value could be evaluated
    if (!val.Get()) {
        return nullptr;
    }
    auto r = const_eval_.Convert(c->Result(0)->Type(), val.Get(), SourceOf(c));
    if (r != Success) {
        return Failure();
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalSwizzle(core::ir::Swizzle* s) {
    auto val = EvalValue(s->Object());
    if (val != Success) {
        return val;
    }
    // Check if the value could be evaluated
    if (!val.Get()) {
        return nullptr;
    }

    auto r = const_eval_.Swizzle(s->Result(0)->Type(), val.Get(), s->Indices());
    if (r != Success) {
        return Failure();
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalUnary(core::ir::CoreUnary* u) {
    intrinsic::Context context{u->TableData(), b_.ir.Types(), b_.ir.symbols};

    auto overload = core::intrinsic::LookupUnary(context, u->Op(), u->Val()->Type(),
                                                 core::EvaluationStage::kOverride);
    if (overload != Success) {
        AddError(SourceOf(u)) << overload.Failure().Plain();
        return Failure();
    }

    auto const_eval_fn = overload->const_eval_fn;
    if (!const_eval_fn) {
        AddError(SourceOf(u)) << "invalid unary expression";
        return Failure();
    }

    auto val = EvalValue(u->Val());
    if (val != Success) {
        return Failure();
    }
    // Check if the value could be evaluated
    if (!val.Get()) {
        return nullptr;
    }

    auto r = (const_eval_.*const_eval_fn)(u->Result(0)->Type(), Vector{val.Get()}, SourceOf(u));
    if (r != Success) {
        return Failure();
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalBinary(core::ir::CoreBinary* cb) {
    intrinsic::Context context{cb->TableData(), b_.ir.Types(), b_.ir.symbols};

    auto overload =
        core::intrinsic::LookupBinary(context, cb->Op(), cb->LHS()->Type(), cb->RHS()->Type(),
                                      core::EvaluationStage::kOverride, /* is_compound */ false);
    if (overload != Success) {
        AddError(SourceOf(cb)) << overload.Failure().Plain();
        return Failure();
    }

    auto const_eval_fn = overload->const_eval_fn;
    if (!const_eval_fn) {
        AddError(SourceOf(cb)) << "invalid binary expression";
        return Failure();
    }

    auto lhs = EvalValue(cb->LHS());
    if (lhs != Success) {
        return lhs;
    }
    // Check LHS could be evaluated
    if (!lhs.Get()) {
        return nullptr;
    }

    auto rhs = EvalValue(cb->RHS());
    if (rhs != Success) {
        return rhs;
    }
    // Check RHS could be evaluated
    if (!rhs.Get()) {
        return nullptr;
    }

    auto r = (const_eval_.*const_eval_fn)(cb->Result(0)->Type(), Vector{lhs.Get(), rhs.Get()},
                                          SourceOf(cb));
    if (r != Success) {
        return Failure();
    }
    return r.Get();
}

Evaluator::EvalResult Evaluator::EvalCoreBuiltinCall(core::ir::CoreBuiltinCall* c) {
    intrinsic::Context context{c->TableData(), b_.ir.Types(), b_.ir.symbols};

    Vector<const core::type::Type*, 0> arg_types;
    arg_types.Reserve(c->Args().Length());
    Vector<const core::constant::Value*, 0> args;
    args.Reserve(c->Args().Length());
    for (auto* arg : c->Args()) {
        arg_types.Push(arg->Type());

        auto val = EvalValue(arg);
        if (val != Success) {
            return val;
        }
        // Check if the value could be evaluated
        if (!val.Get()) {
            return nullptr;
        }
        args.Push(val.Get());
    }

    auto overload = core::intrinsic::LookupFn(context, c->FriendlyName().c_str(), c->FuncId(),
                                              Empty, arg_types, core::EvaluationStage::kOverride);
    if (overload != Success) {
        AddError(SourceOf(c)) << overload.Failure();
        return Failure();
    }

    // If there is no `@const` override, we don't fail the eval, we return a nullptr. This is
    // because we can call eval for things like `dpdx` which is not overridable but that's not an
    // eval failure, we just don't eval.
    auto const_eval_fn = overload->const_eval_fn;
    if (!const_eval_fn) {
        return nullptr;
    }

    auto r = (const_eval_.*const_eval_fn)(c->Result(0)->Type(), args, SourceOf(c));
    if (r != Success) {
        return Failure();
    }
    return r.Get();
}

}  // namespace tint::core::ir
