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

#include "src/tint/lang/spirv/writer/raise/handle_matrix_arithmetic.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/utils/ice/ice.h"

using namespace tint::core::fluent_types;  // NOLINT

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
        // Find and replace the instructions that need to be modified.
        for (auto* inst : ir.Instructions()) {
            if (auto* binary = inst->As<core::ir::CoreBinary>()) {
                if (binary->LHS()->Type()->Is<core::type::Matrix>() ||
                    binary->RHS()->Type()->Is<core::type::Matrix>()) {
                    ReplaceBinary(binary);
                }
            } else if (auto* convert = inst->As<core::ir::Convert>()) {
                if (convert->Result(0)->Type()->Is<core::type::Matrix>()) {
                    ReplaceConvert(convert);
                }
            }
        }
    }

    /// Replace a binary matrix arithmetic instruction.
    /// @param binary the instruction to replace
    void ReplaceBinary(core::ir::Binary* binary) {
        auto* lhs = binary->LHS();
        auto* rhs = binary->RHS();
        auto* lhs_ty = lhs->Type();
        auto* rhs_ty = rhs->Type();
        auto* ty = binary->Result(0)->Type();

        b.InsertBefore(binary, [&] {
            // Helper to replace the instruction with a column-wise operation.
            auto column_wise = [&](auto op) {
                auto* mat = ty->As<core::type::Matrix>();
                Vector<core::ir::Value*, 4> args;
                for (uint32_t col = 0; col < mat->Columns(); col++) {
                    auto* lhs_col = b.Access(mat->ColumnType(), lhs, u32(col));
                    auto* rhs_col = b.Access(mat->ColumnType(), rhs, u32(col));
                    auto* add = b.Binary(op, mat->ColumnType(), lhs_col, rhs_col);
                    args.Push(add->Result(0));
                }
                b.ConstructWithResult(binary->DetachResult(), std::move(args));
            };

            switch (binary->Op()) {
                case core::BinaryOp::kAdd:
                    column_wise(core::BinaryOp::kAdd);
                    break;
                case core::BinaryOp::kSubtract:
                    column_wise(core::BinaryOp::kSubtract);
                    break;
                case core::BinaryOp::kMultiply:
                    // Select the SPIR-V intrinsic that corresponds to the operation being
                    // performed.
                    if (lhs_ty->Is<core::type::Matrix>()) {
                        if (rhs_ty->Is<core::type::Scalar>()) {
                            b.CallWithResult<spirv::ir::BuiltinCall>(
                                binary->DetachResult(), spirv::BuiltinFn::kMatrixTimesScalar, lhs,
                                rhs);
                        } else if (rhs_ty->Is<core::type::Vector>()) {
                            b.CallWithResult<spirv::ir::BuiltinCall>(
                                binary->DetachResult(), spirv::BuiltinFn::kMatrixTimesVector, lhs,
                                rhs);
                        } else if (rhs_ty->Is<core::type::Matrix>()) {
                            b.CallWithResult<spirv::ir::BuiltinCall>(
                                binary->DetachResult(), spirv::BuiltinFn::kMatrixTimesMatrix, lhs,
                                rhs);
                        }
                    } else {
                        if (lhs_ty->Is<core::type::Scalar>()) {
                            b.CallWithResult<spirv::ir::BuiltinCall>(
                                binary->DetachResult(), spirv::BuiltinFn::kMatrixTimesScalar, rhs,
                                lhs);
                        } else if (lhs_ty->Is<core::type::Vector>()) {
                            b.CallWithResult<spirv::ir::BuiltinCall>(
                                binary->DetachResult(), spirv::BuiltinFn::kVectorTimesMatrix, lhs,
                                rhs);
                        }
                    }
                    break;

                default:
                    TINT_UNREACHABLE() << "unhandled matrix arithmetic instruction";
            }
        });

        binary->Destroy();
    }

    /// Replace a matrix convert instruction.
    /// @param convert the instruction to replace
    void ReplaceConvert(core::ir::Convert* convert) {
        auto* arg = convert->Args()[core::ir::Convert::kValueOperandOffset];
        auto* in_mat = arg->Type()->As<core::type::Matrix>();
        auto* out_mat = convert->Result(0)->Type()->As<core::type::Matrix>();

        b.InsertBefore(convert, [&] {
            // Extract and convert each column separately.
            Vector<core::ir::Value*, 4> args;
            for (uint32_t c = 0; c < out_mat->Columns(); c++) {
                auto* col = b.Access(in_mat->ColumnType(), arg, u32(c));
                auto* new_col = b.Convert(out_mat->ColumnType(), col);
                args.Push(new_col->Result(0));
            }

            // Reconstruct the result matrix from the converted columns.
            b.ConstructWithResult(convert->DetachResult(), std::move(args));
        });

        convert->Destroy();
    }
};

}  // namespace

Result<SuccessType> HandleMatrixArithmetic(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.HandleMatrixArithmetic");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
