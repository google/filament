// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/collapse_subgroup_min_max.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::core::ir::transform {

namespace {

bool IsCollapsibleSubgroupOp(core::BuiltinFn func) {
    return (func == core::BuiltinFn::kSubgroupMin) || (func == core::BuiltinFn::kSubgroupMax);
}

// Helper to check if a value is the result of a collapsible subgroup operation,
// possibly through "let" instructions.
bool IsCollapsibleSubgroupValue(core::ir::Value* value) {
    while (auto* res = value->As<core::ir::InstructionResult>()) {
        auto* inst = res->Instruction();
        if (auto* let = inst->As<core::ir::Let>()) {
            value = let->Value();
            continue;
        }
        if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
            return IsCollapsibleSubgroupOp(call->Func());
        }
        break;
    }
    return false;
}

void Process(Module& ir) {
    // Add all nested subgroupMin/Max calls that can be collapsed to a worklist.
    Vector<core::ir::CoreBuiltinCall*, 16> worklist;
    for (auto* inst : ir.Instructions()) {
        if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
            if (IsCollapsibleSubgroupOp(call->Func())) {
                if (IsCollapsibleSubgroupValue(call->Args()[0])) {
                    worklist.Push(call);
                }
            }
        }
    }

    // Replace outer call with inner result.
    for (auto* call : worklist) {
        call->Result(0)->ReplaceAllUsesWith(call->Args()[0]);
        call->Destroy();
    }
}

}  // namespace

Result<SuccessType> CollapseSubgroupMinMax(Module& ir) {
    Process(ir);
    return Success;
}

}  // namespace tint::core::ir::transform
