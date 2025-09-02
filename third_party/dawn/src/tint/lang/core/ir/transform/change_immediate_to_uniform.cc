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

#include "src/tint/lang/core/ir/transform/change_immediate_to_uniform.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::core::ir::transform {
namespace {

/// PIMPL state for the transform.
///
struct State {
    /// The transform config.
    const ChangeImmediateToUniformConfig& config;
    /// The IR module.
    core::ir::Module& ir;
    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        core::ir::Var* immediate = nullptr;

        // candidate group number if immediate_binding_point is not available.
        uint32_t candidate_group = 0;

        for (auto* inst : *ir.root_block) {
            // Allow this to run before or after PromoteInitializers by handling non-var root_block
            // entries
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            // use the largest used group plus 1, or group 0 if no
            // resources are bound as candidate group number.
            if (const auto& bp_visited = var->BindingPoint()) {
                candidate_group = std::max(candidate_group, bp_visited->group + 1);
            }

            auto* var_ty = var->Result()->Type()->As<core::type::Pointer>();

            // In HLSL backend, DecomposeStorageAccess transform may have converted the var pointers
            // into ByteAddressBuffer objects. Since they've been changed, then they're Storage
            // buffers and we don't care about them here.
            if (!var_ty) {
                continue;
            }

            // Only care about immediate space variables.
            if (var_ty->AddressSpace() != core::AddressSpace::kImmediate) {
                continue;
            }

            if (immediate) {
                TINT_ICE() << "multiple immediate variables";
            }

            immediate = var;
        }

        if (!immediate) {
            return;
        }

        BindingPoint bp = {};
        if (config.immediate_binding_point.has_value()) {
            bp = *config.immediate_binding_point;
        } else {
            // Otherwise, use the binding 0 of candidate group.
            bp = {candidate_group, 0};
        }
        immediate->SetBindingPoint(bp.group, bp.binding);

        ReplaceImmediateAddressSpace(immediate);
    }

    /// Replace an output pointer address space to make it `uniform`.
    /// @param value the output variable
    void ReplaceImmediateAddressSpace(core::ir::Instruction* value) {
        Vector<core::ir::Instruction*, 8> to_replace;
        to_replace.Push(value);

        // Update all uses of the module-scope variable.
        while (!to_replace.IsEmpty()) {
            auto* inst = to_replace.Pop();
            auto* new_ptr_type =
                ty.ptr(core::AddressSpace::kUniform, inst->Result()->Type()->UnwrapPtr());
            inst->Result()->SetType(new_ptr_type);

            for (auto usage : inst->Result()->UsagesUnsorted()) {
                if (!usage->instruction->Is<core::ir::Let>() &&
                    !usage->instruction->Is<core::ir::Access>()) {
                    continue;
                }
                to_replace.Push(usage->instruction);
            }
        }
    }
};

}  // namespace

Result<SuccessType> ChangeImmediateToUniform(core::ir::Module& ir,
                                             const ChangeImmediateToUniformConfig& config) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "core.ChangeImmediateToUniform",
                                core::ir::Capabilities{
                                    core::ir::Capability::kAllow8BitIntegers,
                                    core::ir::Capability::kAllowClipDistancesOnF32,
                                    core::ir::Capability::kAllowDuplicateBindings,
                                    core::ir::Capability::kAllowNonCoreTypes,
                                    core::ir::Capability::kAllowPointersAndHandlesInStructures,
                                });
    if (result != Success) {
        return result.Failure();
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
