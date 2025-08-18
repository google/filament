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

#include "src/tint/lang/msl/writer/raise/binary_polyfill.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

namespace tint::msl::writer::raise {
namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find the binary operators that need replacing.
        Vector<core::ir::CoreBinary*, 4> fmod_worklist;
        Vector<core::ir::CoreBinary*, 4> logical_bool_worklist;

        for (auto* inst : ir.Instructions()) {
            if (auto* binary = inst->As<core::ir::CoreBinary>()) {
                auto op = binary->Op();
                auto* lhs_type = binary->LHS()->Type();
                if (op == core::BinaryOp::kModulo && lhs_type->IsFloatScalarOrVector()) {
                    fmod_worklist.Push(binary);
                } else if ((op == core::BinaryOp::kAnd || op == core::BinaryOp::kOr) &&
                           lhs_type->IsBoolScalarOrVector()) {
                    logical_bool_worklist.Push(binary);
                }
            }
        }

        // Replace the instructions that we found.
        for (auto* fmod : fmod_worklist) {
            FMod(fmod);
        }
        for (auto* logical_bool : logical_bool_worklist) {
            LogicalBool(logical_bool);
        }
    }

    /// Replace a floating point modulo binary instruction with the equivalent MSL intrinsic.
    /// @param binary the float point modulo binary instruction
    void FMod(core::ir::CoreBinary* binary) {
        auto* call = b.CallWithResult<msl::ir::BuiltinCall>(
            binary->DetachResult(), msl::BuiltinFn::kFmod, binary->Operands());
        call->InsertBefore(binary);
        binary->Destroy();
    }

    /// Replace a logical boolean binary instruction.
    /// @param binary the logical boolean binary instruction
    void LogicalBool(core::ir::CoreBinary* binary) {
        // MSL does not have boolean overloads for `&` and `|`, so it promotes the operands to
        // integers. Make this explicit in the IR and then convert the result of the binary
        // instruction back to a boolean.
        auto* result_ty = binary->Result()->Type();
        auto* int_ty = ty.MatchWidth(ty.u32(), result_ty);
        b.InsertBefore(binary, [&] {
            auto* int_lhs = b.Convert(int_ty, binary->LHS());
            auto* int_rhs = b.Convert(int_ty, binary->RHS());
            auto* int_binary = b.Binary(binary->Op(), int_ty, int_lhs, int_rhs);
            b.ConvertWithResult(binary->DetachResult(), int_binary);
        });
        binary->Destroy();
    }
};

}  // namespace

Result<SuccessType> BinaryPolyfill(core::ir::Module& ir) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "msl.BinaryPolyfill",
                                core::ir::Capabilities{
                                    core::ir::Capability::kAllow8BitIntegers,
                                    core::ir::Capability::kAllowPointersAndHandlesInStructures,
                                    core::ir::Capability::kAllowPrivateVarsInFunctions,
                                    core::ir::Capability::kAllowAnyLetType,
                                    core::ir::Capability::kAllowNonCoreTypes,
                                    core::ir::Capability::kAllowWorkspacePointerInputToEntryPoint,
                                });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
