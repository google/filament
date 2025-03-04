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

#include "src/tint/lang/spirv/writer/raise/expand_implicit_splats.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/builtin_fn.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// Process the module.
    void Process() {
        // Find the instructions that use implicit splats and modify or replace them.
        for (auto* inst : ir.Instructions()) {
            if (auto* construct = inst->As<core::ir::Construct>()) {
                // A vector constructor with a single scalar argument needs to be modified to
                // replicate the argument N times.
                auto* vec = construct->Result(0)->Type()->As<core::type::Vector>();
                if (vec &&  //
                    construct->Args().Length() == 1 &&
                    construct->Args()[0]->Type()->Is<core::type::Scalar>()) {
                    for (uint32_t i = 1; i < vec->Width(); i++) {
                        construct->AppendArg(construct->Args()[0]);
                    }
                }
            } else if (auto* binary = inst->As<core::ir::CoreBinary>()) {
                // A binary instruction that mixes vector and scalar operands needs to have the
                // scalar operand replaced with an explicit vector constructor.
                if (binary->Result(0)->Type()->Is<core::type::Vector>()) {
                    if (binary->LHS()->Type()->Is<core::type::Scalar>() ||
                        binary->RHS()->Type()->Is<core::type::Scalar>()) {
                        ExpandBinary(binary);
                    }
                }
            } else if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
                // A mix builtin call that mixes vector and scalar operands needs to have the scalar
                // operand replaced with an explicit vector constructor.
                if (builtin->Func() == core::BuiltinFn::kMix) {
                    if (builtin->Result(0)->Type()->Is<core::type::Vector>()) {
                        if (builtin->Args()[2]->Type()->Is<core::type::Scalar>()) {
                            ExpandOperand(builtin,
                                          core::ir::CoreBuiltinCall::kArgsOperandOffset + 2);
                        }
                    }
                }
            }
        }
    }

    /// Helper to expand a scalar operand of an instruction by replacing it with an explicitly
    /// constructed vector that matches the result type.
    void ExpandOperand(core::ir::Instruction* inst, size_t operand_idx) {
        auto* vec = inst->Result(0)->Type()->As<core::type::Vector>();

        Vector<core::ir::Value*, 4> args;
        args.Resize(vec->Width(), inst->Operands()[operand_idx]);

        auto* construct = b.Construct(vec, std::move(args));
        construct->InsertBefore(inst);
        inst->SetOperand(operand_idx, construct->Result(0));
    }

    /// Replace scalar operands to binary instructions that produce vectors.
    /// @param binary the binary instruction to modify
    void ExpandBinary(core::ir::Binary* binary) {
        auto* result_ty = binary->Result(0)->Type();
        if (result_ty->IsFloatVector() && binary->Op() == core::BinaryOp::kMultiply) {
            // Use OpVectorTimesScalar for floating point multiply.
            auto* vts = b.CallWithResult<spirv::ir::BuiltinCall>(
                binary->DetachResult(), spirv::BuiltinFn::kVectorTimesScalar);
            if (binary->LHS()->Type()->Is<core::type::Scalar>()) {
                vts->AppendArg(binary->RHS());
                vts->AppendArg(binary->LHS());
            } else {
                vts->AppendArg(binary->LHS());
                vts->AppendArg(binary->RHS());
            }
            if (auto name = ir.NameOf(binary)) {
                ir.SetName(vts->Result(0), name);
            }
            binary->ReplaceWith(vts);
            binary->Destroy();
        } else {
            // Expand the scalar argument into an explicitly constructed vector.
            if (binary->LHS()->Type()->Is<core::type::Scalar>()) {
                ExpandOperand(binary, core::ir::CoreBinary::kLhsOperandOffset);
            } else if (binary->RHS()->Type()->Is<core::type::Scalar>()) {
                ExpandOperand(binary, core::ir::CoreBinary::kRhsOperandOffset);
            }
        }
    }
};

}  // namespace

Result<SuccessType> ExpandImplicitSplats(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.ExpandImplicitSplats");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
