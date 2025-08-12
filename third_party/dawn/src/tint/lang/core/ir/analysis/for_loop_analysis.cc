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

#include "src/tint/lang/core/ir/analysis/for_loop_analysis.h"

#include <utility>

#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::analysis {

void ForLoopAnalysis::AttemptForLoopDeduction(const Loop* loop) {
    // Find the first 'if' which should be the condition for the for-loop.
    // Order here is critical since instructions in blocks are forward dependencies.
    const core::ir::If* if_to_remove = nullptr;
    for (auto* inst : *loop->Body()) {
        if (auto* if_ = inst->As<const core::ir::If>()) {
            if (if_->Results().IsEmpty() && if_->True()->Length() == 1 &&
                if_->False()->Length() == 1 && tint::Is<core::ir::ExitIf>(if_->True()->Front()) &&
                tint::Is<core::ir::ExitLoop>(if_->False()->Front())) {
                // Matched the loop condition as it was converted from the 'for' originally.
                if_to_remove = if_;
                break;
            } else {
                // Conservatively fail to avoid the possibility of reordering instructions (when
                // moving into conditional).
                return;
            }
        } else if (inst->Is<const Binary>() || inst->Is<const BuiltinCall>() ||
                   inst->Is<const Access>() || inst->Is<const Load>() ||
                   inst->Is<const Swizzle>() || inst->Is<const Unary>() ||
                   inst->Is<const LoadVectorElement>()) {
            // Allowed instructions since either load or operate on values (side effect free).
            continue;
        } else {
            // Conservatively fail for all other functions that could potentially store/mutate
            // memory.
            return;
        }
    }
    if (!if_to_remove) {
        return;
    }

    Hashset<const core::ir::Instruction*, 32> sink_chain;
    // Add the 'if' instruction to set to avoid adding it to the body.
    sink_chain.Add(if_to_remove);
    const Instruction* inst = if_to_remove->prev;
    while (inst) {
        const auto& results = inst->Results();
        if (results.Length() != 1u) {
            // We do not support more than one result for simplicity.
            return;
        }

        for (const auto& each_usage : inst->Result()->UsagesUnsorted()) {
            // All usages must sink into the condition of the if
            if (!each_usage->instruction || !sink_chain.Contains(each_usage->instruction)) {
                return;
            }
        }
        sink_chain.Add(inst);
        inst = inst->prev;
    }

    body_removed_instructions = std::move(sink_chain);
    // Valid condition found. Success criteria for condition hoisting.
    for_condition = if_to_remove->Condition();
}

ForLoopAnalysis::ForLoopAnalysis(const Loop& loop) {
    AttemptForLoopDeduction(&loop);
}
ForLoopAnalysis::~ForLoopAnalysis() = default;

}  // namespace tint::core::ir::analysis
