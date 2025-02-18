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

#include "src/tint/lang/core/ir/block.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Block);

namespace tint::core::ir {

Block::Block() : Base() {}

Block::~Block() = default;

Block* Block::Clone(CloneContext&) {
    TINT_UNREACHABLE() << "blocks must be cloned with CloneInto";
}

void Block::CloneInto(CloneContext& ctx, Block* out) {
    // Note, the `parent_` is not cloned here. Doing so can end up in infinite loops as we try to
    // clone a control instruction and the blocks inside of it. The `parent_` pointer should be set
    // by the control instructions constructor.

    for (auto* inst_in : *this) {
        auto* inst_out = inst_in->Clone(ctx);
        auto results_out = inst_out->Results();
        auto results_in = inst_in->Results();
        TINT_ASSERT(results_out.Length() == results_in.Length());

        size_t len = results_out.Length();
        for (size_t i = 0; i < len; ++i) {
            ctx.Replace(results_in[i], results_out[i]);
        }
        out->Append(inst_out);
    }
}

Instruction* Block::Prepend(Instruction* inst) {
    TINT_ASSERT(inst);
    TINT_ASSERT(inst->Block() == nullptr);

    inst->SetBlock(this);
    instructions_.count += 1;

    if (instructions_.first == nullptr) {
        instructions_.first = inst;
        instructions_.last = inst;
    } else {
        inst->next = instructions_.first;
        instructions_.first->prev = inst;
        instructions_.first = inst;
    }

    return inst;
}

Instruction* Block::Append(Instruction* inst) {
    TINT_ASSERT(inst);
    TINT_ASSERT(inst->Block() == nullptr);

    inst->SetBlock(this);
    instructions_.count += 1;

    if (instructions_.first == nullptr) {
        instructions_.first = inst;
        instructions_.last = inst;
    } else {
        inst->prev = instructions_.last;
        instructions_.last->next = inst;
        instructions_.last = inst;
    }

    return inst;
}

void Block::InsertBefore(Instruction* before, Instruction* inst) {
    TINT_ASSERT(before);
    TINT_ASSERT(inst);
    TINT_ASSERT(before->Block() == this);
    TINT_ASSERT(inst->Block() == nullptr);

    inst->SetBlock(this);
    instructions_.count += 1;

    inst->next = before;
    inst->prev = before->prev;
    before->prev = inst;

    if (inst->prev) {
        inst->prev->next = inst;
    }

    if (before == instructions_.first) {
        instructions_.first = inst;
    }
}

void Block::InsertAfter(Instruction* after, Instruction* inst) {
    TINT_ASSERT(after);
    TINT_ASSERT(inst);
    TINT_ASSERT(after->Block() == this);
    TINT_ASSERT(inst->Block() == nullptr);

    inst->SetBlock(this);
    instructions_.count += 1;

    inst->prev = after;
    inst->next = after->next;
    after->next = inst;

    if (inst->next) {
        inst->next->prev = inst;
    }
    if (after == instructions_.last) {
        instructions_.last = inst;
    }
}

void Block::Replace(Instruction* target, Instruction* inst) {
    TINT_ASSERT(target);
    TINT_ASSERT(inst);
    TINT_ASSERT(target->Block() == this);
    TINT_ASSERT(inst->Block() == nullptr);

    inst->SetBlock(this);
    target->SetBlock(nullptr);

    inst->next = target->next;
    inst->prev = target->prev;

    target->next = nullptr;
    target->prev = nullptr;

    if (inst->next) {
        inst->next->prev = inst;
    }
    if (inst->prev) {
        inst->prev->next = inst;
    }

    if (target == instructions_.first) {
        instructions_.first = inst;
    }
    if (target == instructions_.last) {
        instructions_.last = inst;
    }
}

void Block::Remove(Instruction* inst) {
    TINT_ASSERT(inst);
    TINT_ASSERT(inst->Block() == this);

    inst->SetBlock(nullptr);
    instructions_.count -= 1;

    if (inst->prev) {
        inst->prev->next = inst->next;
    }
    if (inst->next) {
        inst->next->prev = inst->prev;
    }
    if (inst == instructions_.first) {
        instructions_.first = inst->next;
    }
    if (inst == instructions_.last) {
        instructions_.last = inst->prev;
    }

    inst->prev = nullptr;
    inst->next = nullptr;
}

void Block::Destroy() {
    while (instructions_.first) {
        instructions_.first->Destroy();
    }
}

}  // namespace tint::core::ir
