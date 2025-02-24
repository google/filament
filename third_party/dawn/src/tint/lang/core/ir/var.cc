// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/var.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Var);

namespace tint::core::ir {

Var::Var(Id id) : Base(id) {}

Var::Var(Id id, InstructionResult* result) : Base(id) {
    if (result && result->Type()) {
        TINT_ASSERT(result->Type()->Is<core::type::MemoryView>());
    }

    // Default to no initializer.
    AddOperand(Var::kInitializerOperandOffset, nullptr);
    AddResult(result);
}

Var::~Var() = default;

Var* Var::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result(0));
    auto* new_var = ctx.ir.CreateInstruction<Var>(new_result);

    new_var->binding_point_ = binding_point_;
    new_var->attributes_ = attributes_;

    if (auto* init = Initializer()) {
        new_var->SetInitializer(ctx.Clone(init));
    }

    auto name = ctx.ir.NameOf(this);
    if (name.IsValid()) {
        ctx.ir.SetName(new_var, name.Name());
    }
    return new_var;
}

void Var::SetInitializer(Value* initializer) {
    SetOperand(Var::kInitializerOperandOffset, initializer);
}

void Var::DestroyIfOnlyAssigned() {
    auto* result = Result(0);
    if (result->UsagesUnsorted().All(
            [](const Usage& u) { return u.instruction->Is<ir::Store>(); })) {
        while (result->IsUsed()) {
            auto& usage = *result->UsagesUnsorted().begin();
            usage->instruction->Destroy();
        }
        Destroy();
    }
}

}  // namespace tint::core::ir
