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

#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

void Run(Module& ir) {
    Builder builder{ir};
    type::Manager& ty{ir.Types()};

    if (ir.root_block->IsEmpty()) {
        return;
    }

    // Loop over module-scope declarations, looking for storage or uniform buffers.
    Vector<Var*, 8> buffer_variables;
    for (auto inst : *ir.root_block) {
        auto* var = inst->As<Var>();
        if (!var) {
            continue;
        }
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        if (!ptr || !core::IsHostShareable(ptr->AddressSpace())) {
            continue;
        }
        buffer_variables.Push(var);
    }

    // Now process the buffer variables.
    for (auto* var : buffer_variables) {
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        auto* store_ty = ptr->StoreType();

        if (auto* str = store_ty->As<core::type::Struct>()) {
            if (str->StructFlags().Contains(type::kBlock)) {
                // The struct already has a block attribute, so we don't need to do anything here.
                continue;
            }
            if (!str->HasFixedFootprint()) {
                // We know the original struct will only ever be used as the store type of a buffer,
                // so just mark it as a block-decorated struct.
                // TODO(crbug.com/tint/745): Remove the const_cast.
                const_cast<type::Struct*>(str)->SetStructFlag(type::kBlock);
                continue;
            }
        }

        // The original struct might be used in other places, so create a new block-decorated
        // struct that wraps the original struct.
        // Use a consistent name for the inner struct member to satisfy GLSL's interface matching
        // rules. Derive the struct name from the variable name to preserve any prefixes provided by
        // Dawn, which are needed to avoid clashing between vertex and fragment shaders in GLSL.
        auto inner_name = ir.symbols.Register("inner");
        Symbol wrapper_name;
        if (auto var_name = ir.NameOf(var)) {
            wrapper_name = ir.symbols.New(var_name.Name() + "_block");
        } else {
            wrapper_name = ir.symbols.New();
        }
        auto* block_struct = ty.Struct(wrapper_name, {{inner_name, store_ty}});
        block_struct->SetStructFlag(core::type::StructFlag::kBlock);

        // Replace the old variable declaration with one that uses the block-decorated struct type.
        auto* new_var = builder.Var(ty.ptr(ptr->AddressSpace(), block_struct, ptr->Access()));
        if (var->BindingPoint()) {
            new_var->SetBindingPoint(var->BindingPoint()->group, var->BindingPoint()->binding);
        }
        var->ReplaceWith(new_var);

        // Replace uses of the old variable.
        // The structure has been wrapped, so replace all uses of the old variable with a member
        // accessor on the new variable.
        var->Result(0)->ReplaceAllUsesWith([&](Usage use) -> Value* {
            auto* access = builder.Access(var->Result(0)->Type(), new_var, 0_u);
            access->InsertBefore(use.instruction);
            return access->Result(0);
        });

        var->Destroy();
    }
}

}  // namespace

Result<SuccessType> BlockDecoratedStructs(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.BlockDecoratedStructs");
    if (result != Success) {
        return result;
    }

    Run(ir);

    return Success;
}

}  // namespace tint::core::ir::transform
