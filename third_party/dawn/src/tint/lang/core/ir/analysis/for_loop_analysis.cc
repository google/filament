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

#include <optional>
#include <utility>

#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/containers/hashset.h"

namespace tint::core::ir::analysis {

namespace {

std::optional<Hashset<const core::ir::Instruction*, 32>> SinkChain(
    const core::ir::Instruction* root) {
    Hashset<const core::ir::Instruction*, 32> sink_chain{root};
    const Instruction* inst = root->prev;
    while (inst) {
        const auto& results = inst->Results();
        if (results.Length() != 1u) {
            // We do not support more than one result for simplicity.
            return std::nullopt;
        }

        // Check that the instruction can be inlined as an expression (i.e. it either loads or
        // operates on values).
        if (!inst->IsAnyOf<Access, Binary, BuiltinCall, Load, LoadVectorElement, Swizzle,
                           Unary>()) {
            return std::nullopt;
        }

        for (const auto& each_usage : inst->Result()->UsagesUnsorted()) {
            // All usages must sink into the root instruction.
            if (!each_usage->instruction || !sink_chain.Contains(each_usage->instruction)) {
                return std::nullopt;
            }
        }
        sink_chain.Add(inst);
        inst = inst->prev;
    }
    return sink_chain;
}

const core::ir::Store* GetContinuingSimpleLoopUpdate(const Loop* loop) {
    // Check whether the continuing block can be embedded in a for-loop update statement.
    // We can do this if the following conditions hold:
    //  - the last instruction of the continuing block is a next_iteration with no operands
    //  - the preceding instruction is a store
    //  - all other instructions in the block sink into that store
    //  - none of these instructions will be emitted as statements (e.g. var declarations)
    //  - none of the operands to these instructions are defined in the loop body

    if (!loop->Continuing() || loop->Continuing()->IsEmpty()) {
        return nullptr;
    }
    auto* next_iteration = loop->Continuing()->Back()->As<const core::ir::NextIteration>();
    if (!next_iteration || !next_iteration->Operands().IsEmpty()) {
        return nullptr;
    }

    auto* store = As<core::ir::Store>(next_iteration->prev);
    if (!store) {
        return nullptr;
    }

    auto sink_chain = SinkChain(store);
    if (!sink_chain) {
        return nullptr;
    }

    // If an operand was defined in the loop body, then sinking may violate scoping rules.
    for (auto inst : *sink_chain) {
        for (auto* operand : inst->Operands()) {
            if (auto* result = operand->As<InstructionResult>()) {
                if (result->Instruction()->Block() == loop->Body()) {
                    return nullptr;
                }
            }
        }
    }

    return store;
}

}  // namespace

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
        }
    }
    if (!if_to_remove) {
        return;
    }

    // Check that all instructions that precede the condition feed into it.
    auto sink_chain = SinkChain(if_to_remove);
    if (!sink_chain) {
        return;
    }

    body_removed_instructions = std::move(sink_chain.value());

    // Valid condition found. Success criteria for condition hoisting.
    for_condition = if_to_remove->Condition();

    // Now check whether the continuing block can be embedded in a for-loop update statement.
    continuing_update_store = GetContinuingSimpleLoopUpdate(loop);
}

ForLoopAnalysis::ForLoopAnalysis(const Loop& loop) {
    AttemptForLoopDeduction(&loop);
}
ForLoopAnalysis::~ForLoopAnalysis() = default;

}  // namespace tint::core::ir::analysis
