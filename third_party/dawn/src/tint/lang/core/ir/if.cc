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

#include "src/tint/lang/core/ir/if.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::If);

namespace tint::core::ir {

If::If(Id id) : Base(id) {}

If::If(Id id, Value* cond, ir::Block* t, ir::Block* f) : Base(id), true_(t), false_(f) {
    TINT_ASSERT(true_);
    TINT_ASSERT(false_);

    AddOperand(If::kConditionOperandOffset, cond);

    if (true_) {
        true_->SetParent(this);
    }
    if (false_) {
        false_->SetParent(this);
    }
}

If::~If() = default;

void If::ForeachBlock(const std::function<void(ir::Block*)>& cb) {
    if (true_) {
        cb(true_);
    }
    if (false_) {
        cb(false_);
    }
}

void If::ForeachBlock(const std::function<void(const ir::Block*)>& cb) const {
    if (true_) {
        cb(true_);
    }
    if (false_) {
        cb(false_);
    }
}

If* If::Clone(CloneContext& ctx) {
    auto* cond = ctx.Remap(Condition());
    auto* new_true = ctx.ir.blocks.Create<ir::Block>();
    auto* new_false = ctx.ir.blocks.Create<ir::Block>();

    auto* new_if = ctx.ir.CreateInstruction<If>(cond, new_true, new_false);
    ctx.Replace(this, new_if);

    true_->CloneInto(ctx, new_true);
    false_->CloneInto(ctx, new_false);

    new_if->SetResults(ctx.Clone(results_));

    return new_if;
}

void If::SetTrue(ir::Block* block) {
    if (true_ && true_->Parent() == this) {
        true_->SetParent(nullptr);
    }
    true_ = block;
    if (block) {
        block->SetParent(this);
    }
}

void If::SetFalse(ir::Block* block) {
    if (false_ && false_->Parent() == this) {
        false_->SetParent(nullptr);
    }
    false_ = block;
    if (block) {
        block->SetParent(this);
    }
}

}  // namespace tint::core::ir
