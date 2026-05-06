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

#include "src/tint/lang/core/ir/transform/bgra8unorm_polyfill.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find module-scope variables that may need to be updated.
        for (auto inst : *ir.root_block) {
            if (auto* var = inst->As<Var>()) {
                ChangeTypeIfNeeded(var->Result());
            }
        }

        // Find function parameters that may need to be updated.
        for (auto& func : ir.functions) {
            for (auto* param : func->Params()) {
                ChangeTypeIfNeeded(param);
            }
        }
    }

    /// Update a value's type to use rgba8unorm instead of bgra8unorm if needed.
    /// @param value the value that may have a bgra8unorm type
    void ChangeTypeIfNeeded(Value* value) {
        // Check if the type has a bgra8unorm texel format and create the rgba8unorm equivalent.
        auto* rgba8unorm_type = tint::Switch(
            value->Type()->UnwrapPtr(),
            [&](const core::type::StorageTexture* st) -> const core::type::Type* {
                if (st->TexelFormat() == TexelFormat::kBgra8Unorm) {
                    return ty.storage_texture(st->Dim(), TexelFormat::kRgba8Unorm, st->Access());
                }
                return nullptr;
            },
            [&](const core::type::TexelBuffer* tb) -> const core::type::Type* {
                if (tb->TexelFormat() == TexelFormat::kBgra8Unorm) {
                    return ty.texel_buffer(TexelFormat::kRgba8Unorm, tb->Access());
                }
                return nullptr;
            });
        if (!rgba8unorm_type) {
            // No change needed.
            return;
        }

        // If the original type was a pointer, make the new type a pointer too.
        if (value->Type()->Is<core::type::Pointer>()) {
            rgba8unorm_type = ty.ptr(handle, rgba8unorm_type);
        }

        // Change the value to use the new rgba8unorm type.
        value->SetType(rgba8unorm_type);

        // Update all uses of the value.
        UpdateUses(value);
    }

    /// Recursively update the uses of @p value.
    /// @param value the value to update
    void UpdateUses(Value* value) {
        value->ForEachUseUnsorted([&](Usage use) {
            tint::Switch(
                use.instruction,
                [&](Load* load) {
                    // Update the result type of the load instruction and then update its uses too.
                    load->Result()->SetType(value->Type()->UnwrapPtr());
                    UpdateUses(load->Result());
                },
                [&](CoreBuiltinCall* call) {
                    if (call->Func() == core::BuiltinFn::kTextureStore) {
                        // Swizzle the value argument of a `textureStore()` builtin.
                        uint32_t index = 2u;
                        if (auto* tex = value->Type()->As<core::type::StorageTexture>()) {
                            index = core::type::IsTextureArray(tex->Dim()) ? 3u : 2u;
                        }
                        auto* texel = call->Args()[index];
                        auto* swizzle = b.Swizzle(texel->Type(), texel, Vector{2u, 1u, 0u, 3u});
                        swizzle->InsertBefore(call);
                        call->SetOperand(index, swizzle->Result());
                    } else if (call->Func() == core::BuiltinFn::kTextureLoad) {
                        // Swizzle the result of a `textureLoad()` builtin.
                        auto* swizzle =
                            b.Swizzle(call->Result()->Type(), nullptr, Vector{2u, 1u, 0u, 3u});
                        call->Result()->ReplaceAllUsesWith(swizzle->Result());
                        swizzle->InsertAfter(call);
                        swizzle->SetOperand(Swizzle::kObjectOperandOffset, call->Result());
                    }
                },
                [&](UserCall*) {
                    // Nothing to do since the parameter will already have been updated.
                },
                TINT_ICE_ON_NO_MATCH);
        });
    }
};

}  // namespace

Result<SuccessType> Bgra8UnormPolyfill(Module& ir) {
    core::ir::AssertValid(ir, kBgra8UnormPolyfillCapabilities, "before core.Bgra8UnormPolyfill");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
