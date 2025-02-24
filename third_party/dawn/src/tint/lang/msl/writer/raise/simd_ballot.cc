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

#include "src/tint/lang/msl/writer/raise/simd_ballot.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The subgroupBallot polyfill function.
    core::ir::Function* subgroup_ballot_polyfill = nullptr;

    /// The subgroup_size_mask module-scope variable.
    core::ir::Var* subgroup_size_mask = nullptr;

    /// Process the module.
    void Process() {
        // Find calls to `subgroupBallot`.
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                if (call->Func() == core::BuiltinFn::kSubgroupBallot) {
                    Replace(call);
                }
            }
        }

        // Set the subgroup size mask value from all entry points that use it.
        core::ir::ReferencedModuleVars<core::ir::Module> refs(ir,
                                                              [&](const core::ir::Var* var) {  //
                                                                  return var == subgroup_size_mask;
                                                              });
        for (auto func : ir.functions) {
            if (func->IsEntryPoint()) {
                if (refs.TransitiveReferences(func).Contains(subgroup_size_mask)) {
                    SetSubgroupSizeMaskForEntryPoint(func);
                }
            }
        }
    }

    /// Replace a call to subgroupBallot with a call to a polyfill function.
    void Replace(core::ir::CoreBuiltinCall* call) {
        b.InsertBefore(call, [&] {
            b.CallWithResult(call->DetachResult(), SubgroupBallotPolyfill(), call->Args()[0]);
        });
        call->Destroy();
    }

    /// Get (or create, on first call) the polyfill function.
    core::ir::Function* SubgroupBallotPolyfill() {
        if (subgroup_ballot_polyfill) {
            // The polyfill has already been created.
            return subgroup_ballot_polyfill;
        }

        // Declare the subgroup_size_mask variable that we need.
        b.Append(ir.root_block, [&] {
            subgroup_size_mask = b.Var<private_, vec2<u32>>("tint_subgroup_size_mask");
        });

        // Create the polyfill function, which looks like this:
        //   fn tint_subgroup_ballot(pred: bool) -> vec4u {
        //     let simd_vote: vec2u = msl.simd_ballot(pred);
        //     return vec4u(simd_vote & tint_subgroup_size_mask, 0, 0);
        //   }
        auto* pred = b.FunctionParam("pred", ty.bool_());
        subgroup_ballot_polyfill = b.Function("tint_subgroup_ballot", ty.vec4<u32>());
        subgroup_ballot_polyfill->SetParams({pred});
        b.Append(subgroup_ballot_polyfill->Block(), [&] {
            auto* simd_vote =
                b.Call<msl::ir::BuiltinCall>(ty.vec2<u32>(), msl::BuiltinFn::kSimdBallot, pred);
            auto* masked = b.And<vec2<u32>>(simd_vote, b.Load(subgroup_size_mask));
            auto* result = b.Construct(ty.vec4<u32>(), masked, u32(0), u32(0));
            b.Return(subgroup_ballot_polyfill, result);
        });

        return subgroup_ballot_polyfill;
    }

    /// Set the subgroup_size_mask variable from an entry point.
    void SetSubgroupSizeMaskForEntryPoint(core::ir::Function* ep) {
        // Check if there is a user provided subgroup_size builtin.
        core::ir::FunctionParam* subgroup_size = nullptr;
        for (auto* param : ep->Params()) {
            if (param->Attributes().builtin == core::BuiltinValue::kSubgroupSize) {
                subgroup_size = param;
                break;
            }
        }
        if (!subgroup_size) {
            // No user defined subgroup_size builtin was found, so create our own.
            subgroup_size = b.FunctionParam("tint_subgroup_size", ty.u32());
            subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);
            ep->AppendParam(subgroup_size);
        }

        // Set the subgroup_size_mask based on the subgroup_size:
        //   let size_gt_32 = (subgroup_size > 32u);
        //   let high = select(4294967295u >> (32u - subgroup_size), 4294967295u, size_gt_32);
        //   let low  = select(0u, (4294967295u >> (64u - subgroup_size)), size_gt_32);
        //   tint_subgroup_size_mask[0u] = high;
        //   tint_subgroup_size_mask[1u] = low;
        b.InsertBefore(ep->Block()->Front(), [&] {
            auto* gt32 = b.GreaterThan<bool>(subgroup_size, u32(32));
            auto* high_mask =
                b.ShiftRight<u32>(u32::Highest(), b.Subtract<u32>(u32(32), subgroup_size));
            auto* high = b.Call<u32>(core::BuiltinFn::kSelect, high_mask, u32::Highest(), gt32);
            auto* low_mask =
                b.ShiftRight<u32>(u32::Highest(), b.Subtract<u32>(u32(64), subgroup_size));
            auto* low = b.Call<u32>(core::BuiltinFn::kSelect, u32(0), low_mask, gt32);
            b.StoreVectorElement(subgroup_size_mask, u32(0), high);
            b.StoreVectorElement(subgroup_size_mask, u32(1), low);
        });
    }
};

}  // namespace

Result<SuccessType> SimdBallot(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "msl.SimdBallot");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
