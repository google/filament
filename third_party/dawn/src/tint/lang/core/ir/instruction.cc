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

#include "src/tint/lang/core/ir/instruction.h"

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Instruction);

namespace tint::core::ir {

Instruction::Instruction(Id id) : id_(id) {}

Instruction::~Instruction() = default;

void Instruction::Destroy() {
    TINT_ASSERT(Alive());
    if (Block()) {
        Remove();
    }
    for (auto* result : Results()) {
        result->SetInstruction(nullptr);
        result->Destroy();
    }
    flags_.Add(Flag::kDead);
}

void Instruction::InsertBefore(Instruction* before) {
    TINT_ASSERT(before);
    TINT_ASSERT(before->Block() != nullptr);
    before->Block()->InsertBefore(before, this);
}

void Instruction::InsertAfter(Instruction* after) {
    TINT_ASSERT(after);
    TINT_ASSERT(after->Block() != nullptr);
    after->Block()->InsertAfter(after, this);
}

void Instruction::ReplaceWith(Instruction* replacement) {
    TINT_ASSERT(replacement);
    TINT_ASSERT(Block() != nullptr);
    Block()->Replace(this, replacement);
}

void Instruction::Remove() {
    TINT_ASSERT(Block() != nullptr);
    Block()->Remove(this);
}

InstructionResult* Instruction::DetachResult() {
    TINT_ASSERT(Results().Length() == 1u);
    auto* result = Results()[0];
    SetResults({});
    return result;
}

}  // namespace tint::core::ir
