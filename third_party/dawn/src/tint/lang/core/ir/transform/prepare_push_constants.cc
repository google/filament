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

#include "src/tint/lang/core/ir/transform/prepare_push_constants.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The transform config.
    const PreparePushConstantsConfig& config;

    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    PushConstantLayout Run() {
        if (config.internal_constants.empty()) {
            return PushConstantLayout{};
        }

        PushConstantLayout layout;
        Var* user_defined_constants = nullptr;
        Vector<core::type::StructMember*, 4> members;

        // Check for user-defined push constants.
        for (auto inst : *ir.root_block) {
            auto* var = inst->As<Var>();
            if (!var) {
                continue;
            }
            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            if (ptr->AddressSpace() != core::AddressSpace::kPushConstant) {
                continue;
            }

            if (user_defined_constants) {
                TINT_ICE() << "multiple user-defined push constant variables";
            }
            user_defined_constants = var;

            // Assume that user-defined constants start at offset 0 until Dawn tells us otherwise.
            members.Push(ty.Get<core::type::StructMember>(ir.symbols.New("user_constant"),
                                                          ptr->StoreType(),
                                                          /* index */ 0u,
                                                          /* offset */ 0u,
                                                          /* align */ ptr->StoreType()->Align(),
                                                          /* size */ ptr->StoreType()->Size(),
                                                          /* attributes */ IOAttributes{}));
        }

        // Create the structure and push constant variable.
        for (auto& internal : config.internal_constants) {
            if (!members.IsEmpty()) {
                TINT_ASSERT(internal.first >= members.Back()->Offset() + members.Back()->Size());
            }

            auto index = static_cast<uint32_t>(members.Length());
            layout.offset_to_index.Add(internal.first, index);
            members.Push(ty.Get<core::type::StructMember>(internal.second.name,
                                                          internal.second.type,
                                                          /* index */ index,
                                                          /* offset */ internal.first,
                                                          /* align */ internal.second.type->Align(),
                                                          /* size */ internal.second.type->Size(),
                                                          /* attributes */ IOAttributes{}));
        }
        auto* push_constant_struct =
            ty.Struct(ir.symbols.New("tint_push_constant_struct"), std::move(members));
        push_constant_struct->SetStructFlag(type::kBlock);
        layout.var =
            b.Var("tint_push_constants", core::AddressSpace::kPushConstant, push_constant_struct);
        ir.root_block->Append(layout.var);

        // Update uses of the user defined push constant variable.
        if (user_defined_constants) {
            user_defined_constants->Result()->ReplaceAllUsesWith([&](Usage use) {
                auto* access = b.Access(user_defined_constants->Result()->Type(), layout.var, 0_u);
                access->InsertBefore(use.instruction);
                return access->Result();
            });
            user_defined_constants->Destroy();
        }

        return layout;
    }
};

}  // namespace

Result<PushConstantLayout> PreparePushConstants(Module& ir,
                                                const PreparePushConstantsConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "core.PreparePushConstants");
    if (result != Success) {
        return result.Failure();
    }

    return State{config, ir}.Run();
}

}  // namespace tint::core::ir::transform
