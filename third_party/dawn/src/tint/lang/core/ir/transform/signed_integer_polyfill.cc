// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/signed_integer_polyfill.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir::transform {
namespace {

/// PIMPL state for the transform.
struct State {
    core::ir::Module& ir;
    SignedIntegerPolyfillConfig cfg;
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        Vector<core::ir::Unary*, 4> signed_int_negate_worklist;
        Vector<core::ir::CoreBinary*, 4> signed_integer_arithmetic_worklist;
        Vector<core::ir::CoreBinary*, 4> signed_integer_leftshift_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* unary = inst->As<core::ir::Unary>()) {
                auto op = unary->Op();
                auto* type = unary->Val()->Type();
                if (cfg.signed_negation && op == core::UnaryOp::kNegation &&
                    type->IsSignedIntegerScalarOrVector()) {
                    signed_int_negate_worklist.Push(unary);
                }
            } else if (auto* binary = inst->As<core::ir::CoreBinary>()) {
                auto op = binary->Op();
                auto* lhs_type = binary->LHS()->Type();
                if (cfg.signed_arithmetic &&
                    (op == core::BinaryOp::kAdd || op == core::BinaryOp::kMultiply ||
                     op == core::BinaryOp::kSubtract) &&
                    lhs_type->IsSignedIntegerScalarOrVector()) {
                    signed_integer_arithmetic_worklist.Push(binary);
                } else if (cfg.signed_shiftleft && op == core::BinaryOp::kShiftLeft &&
                           lhs_type->IsSignedIntegerScalarOrVector()) {
                    signed_integer_leftshift_worklist.Push(binary);
                }
            }
        }

        // Replace the instructions that we found.
        for (auto* signed_int_negate : signed_int_negate_worklist) {
            SignedIntegerNegation(signed_int_negate);
        }
        for (auto* signed_arith : signed_integer_arithmetic_worklist) {
            SignedIntegerArithmetic(signed_arith);
        }
        for (auto* signed_shift_left : signed_integer_leftshift_worklist) {
            SignedIntegerShiftLeft(signed_shift_left);
        }
    }

    /// Replace a signed integer negation to avoid undefined behavior.
    /// @param unary the unary instruction
    void SignedIntegerNegation(core::ir::Unary* unary) {
        // Replace `-x` with `bitcast<int>((~bitcast<uint>(x)) + 1)`.
        auto* signed_type = unary->Result()->Type();
        auto* unsigned_type = ty.MatchWidth(ty.u32(), signed_type);
        b.InsertBefore(unary, [&] {
            auto* unsigned_value = b.Bitcast(unsigned_type, unary->Val());
            auto* complement = b.Complement(unsigned_type, unsigned_value);
            auto* plus_one = b.Add(unsigned_type, complement, b.MatchWidth(u32(1), unsigned_type));
            auto* result = b.Bitcast(signed_type, plus_one);
            unary->Result()->ReplaceAllUsesWith(result->Result());
        });
        unary->Destroy();
    }

    /// Replace a signed integer arithmetic instruction.
    /// @param binary the signed integer arithmetic instruction
    void SignedIntegerArithmetic(core::ir::CoreBinary* binary) {
        // MSL (HLSL/SPIR-V) does not define the behavior of signed integer overflow, so bitcast the
        // operands to unsigned integers, perform the operation, and then bitcast the result back to
        // a signed integer.
        auto* signed_result_ty = binary->Result()->Type();
        auto* unsigned_result_ty = ty.MatchWidth(ty.u32(), signed_result_ty);
        auto* unsigned_lhs_ty = ty.MatchWidth(ty.u32(), binary->LHS()->Type());
        auto* unsigned_rhs_ty = ty.MatchWidth(ty.u32(), binary->RHS()->Type());
        b.InsertBefore(binary, [&] {
            auto* uint_lhs = b.Bitcast(unsigned_lhs_ty, binary->LHS());
            auto* uint_rhs = b.Bitcast(unsigned_rhs_ty, binary->RHS());
            auto* uint_binary = b.Binary(binary->Op(), unsigned_result_ty, uint_lhs, uint_rhs);
            auto* bitcast = b.Bitcast(signed_result_ty, uint_binary);
            binary->Result()->ReplaceAllUsesWith(bitcast->Result());
        });
        binary->Destroy();
    }

    /// Replace a signed integer shift left instruction.
    /// @param binary the signed integer shift left instruction
    void SignedIntegerShiftLeft(core::ir::CoreBinary* binary) {
        // Left-shifting a negative integer is undefined behavior in C++14 and therefore potentially
        // in MSL (HLSL/SPRI-V) too, so we bitcast to an unsigned integer, perform the shift, and
        // bitcast the result back to a signed integer.
        auto* signed_ty = binary->Result()->Type();
        auto* unsigned_ty = ty.MatchWidth(ty.u32(), signed_ty);
        b.InsertBefore(binary, [&] {
            auto* unsigned_lhs = b.Bitcast(unsigned_ty, binary->LHS());
            auto* unsigned_binary =
                b.Binary(binary->Op(), unsigned_ty, unsigned_lhs, binary->RHS());
            auto* bitcast = b.Bitcast(signed_ty, unsigned_binary);
            binary->Result()->ReplaceAllUsesWith(bitcast->Result());
        });
        binary->Destroy();
    }
};

}  // namespace

Result<SuccessType> SignedIntegerPolyfill(core::ir::Module& ir,
                                          const SignedIntegerPolyfillConfig& cfg) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "ir.SignedIntegerPolyfill",
                                core::ir::Capabilities{
                                    core::ir::Capability::kAllowPointersAndHandlesInStructures,
                                    core::ir::Capability::kAllowDuplicateBindings,
                                    core::ir::Capability::kAllow8BitIntegers,
                                    core::ir::Capability::kAllow64BitIntegers,
                                    core::ir::Capability::kAllowVectorElementPointer,
                                    core::ir::Capability::kAllowHandleVarsWithoutBindings,
                                    core::ir::Capability::kAllowClipDistancesOnF32,
                                    core::ir::Capability::kAllowPrivateVarsInFunctions,
                                    core::ir::Capability::kAllowAnyLetType,
                                    core::ir::Capability::kAllowWorkspacePointerInputToEntryPoint,
                                    core::ir::Capability::kAllowModuleScopeLets,
                                    core::ir::Capability::kAllowAnyInputAttachmentIndexType,
                                    core::ir::Capability::kAllowNonCoreTypes,
                                });
    if (result != Success) {
        return result.Failure();
    }

    State{ir, cfg}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
