// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/switch.h"

#include <utility>

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Switch);

namespace tint::core::ir {

Switch::Switch(Id id) : Base(id) {}

Switch::Switch(Id id, Value* cond) : Base(id) {
    TINT_ASSERT(cond);

    AddOperand(Switch::kConditionOperandOffset, cond);
}

Switch::~Switch() = default;

void Switch::ForeachBlock(const std::function<void(ir::Block*)>& cb) {
    for (auto& c : cases_) {
        cb(c.block);
    }
}

void Switch::ForeachBlock(const std::function<void(const ir::Block*)>& cb) const {
    for (auto& c : cases_) {
        cb(c.block);
    }
}

Switch* Switch::Clone(CloneContext& ctx) {
    auto* cond = ctx.Remap(Condition());
    auto* new_switch = ctx.ir.CreateInstruction<Switch>(cond);
    ctx.Replace(this, new_switch);

    new_switch->cases_.Reserve(cases_.Length());
    for (auto& cse : cases_) {
        Switch::Case new_case{};
        new_case.block = ctx.ir.blocks.Create<ir::Block>();
        new_case.block->SetParent(new_switch);
        cse.block->CloneInto(ctx, new_case.block);

        new_case.selectors.Reserve(cse.selectors.Length());
        for (auto& sel : cse.selectors) {
            auto* new_val = sel.val ? ctx.Clone(sel.val) : nullptr;
            new_case.selectors.Push(Switch::CaseSelector{new_val});
        }
        new_switch->cases_.Push(std::move(new_case));
    }

    new_switch->SetResults(ctx.Clone(results_));

    return new_switch;
}

}  // namespace tint::core::ir
