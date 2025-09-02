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

#include <utility>

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
        // Find module-scope variables that need to be replaced.
        if (!ir.root_block->IsEmpty()) {
            Vector<Instruction*, 4> to_remove;
            for (auto inst : *ir.root_block) {
                auto* var = inst->As<Var>();
                if (!var) {
                    continue;
                }
                auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                if (!ptr) {
                    continue;
                }
                auto* storage_texture = ptr->StoreType()->As<core::type::StorageTexture>();
                if (storage_texture &&
                    storage_texture->TexelFormat() == core::TexelFormat::kBgra8Unorm) {
                    ReplaceVar(var, storage_texture);
                    to_remove.Push(var);
                    continue;
                }

                auto* texel_buffer = ptr->StoreType()->As<core::type::TexelBuffer>();
                if (texel_buffer && texel_buffer->TexelFormat() == core::TexelFormat::kBgra8Unorm) {
                    ReplaceVar(var, texel_buffer);
                    to_remove.Push(var);
                }
            }
            for (auto* remove : to_remove) {
                remove->Destroy();
            }
        }

        // Find function parameters that need to be replaced.
        for (auto& func : ir.functions) {
            for (uint32_t index = 0; index < func->Params().Length(); index++) {
                auto* param = func->Params()[index];
                auto* storage_texture = param->Type()->As<core::type::StorageTexture>();
                if (storage_texture &&
                    storage_texture->TexelFormat() == core::TexelFormat::kBgra8Unorm) {
                    ReplaceParameter(func, param, index, storage_texture);
                    continue;
                }

                auto* texel_buffer = param->Type()->As<core::type::TexelBuffer>();
                if (texel_buffer && texel_buffer->TexelFormat() == core::TexelFormat::kBgra8Unorm) {
                    ReplaceParameter(func, param, index, texel_buffer);
                }
            }
        }
    }

    /// Replace a variable declaration with one that uses rgba8unorm instead of bgra8unorm.
    /// @param old_var the variable declaration to replace
    /// @param bgra8 the bgra8unorm texture type
    void ReplaceVar(Var* old_var, const core::type::StorageTexture* bgra8) {
        // Redeclare the variable with a rgba8unorm texel format.
        auto* rgba8 =
            ty.storage_texture(bgra8->Dim(), core::TexelFormat::kRgba8Unorm, bgra8->Access());
        auto* new_var = b.Var(ty.ptr(handle, rgba8));
        auto bp = old_var->BindingPoint();
        new_var->SetBindingPoint(bp->group, bp->binding);
        new_var->InsertBefore(old_var);
        if (auto name = ir.NameOf(old_var)) {
            ir.SetName(new_var, name.NameView());
        }

        // Replace all uses of the old variable with the new one.
        ReplaceUses(old_var->Result(), new_var->Result());
    }

    /// Replace a variable declaration with one that uses rgba8unorm instead of bgra8unorm.
    /// @param old_var the variable declaration to replace
    /// @param bgra8 the bgra8unorm texel buffer type
    void ReplaceVar(Var* old_var, const core::type::TexelBuffer* bgra8) {
        // Redeclare the variable with a rgba8unorm texel format.
        auto* rgba8 = ty.texel_buffer(core::TexelFormat::kRgba8Unorm, bgra8->Access());
        auto* new_var = b.Var(ty.ptr(handle, rgba8));
        auto bp = old_var->BindingPoint();
        new_var->SetBindingPoint(bp->group, bp->binding);
        new_var->InsertBefore(old_var);
        if (auto name = ir.NameOf(old_var)) {
            ir.SetName(new_var, name.NameView());
        }

        // Replace all uses of the old variable with the new one.
        ReplaceUses(old_var->Result(), new_var->Result());
    }

    /// Replace a function parameter with one that uses rgba8unorm instead of bgra8unorm.
    /// @param func the function
    /// @param old_param the function parameter to replace
    /// @param index the index of the function parameter
    /// @param bgra8 the bgra8unorm texture type
    void ReplaceParameter(Function* func,
                          FunctionParam* old_param,
                          uint32_t index,
                          const core::type::StorageTexture* bgra8) {
        // Redeclare the parameter with a rgba8unorm texel format.
        auto* rgba8 =
            ty.storage_texture(bgra8->Dim(), core::TexelFormat::kRgba8Unorm, bgra8->Access());
        auto* new_param = b.FunctionParam(rgba8);
        if (auto name = ir.NameOf(old_param)) {
            ir.SetName(new_param, name.NameView());
        }

        Vector<FunctionParam*, 4> new_params = func->Params();
        new_params[index] = new_param;
        func->SetParams(std::move(new_params));

        // Replace all uses of the old parameter with the new one.
        ReplaceUses(old_param, new_param);
    }

    /// Replace a function parameter with one that uses rgba8unorm instead of bgra8unorm.
    /// @param func the function
    /// @param old_param the function parameter to replace
    /// @param index the index of the function parameter
    /// @param bgra8 the bgra8unorm texel buffer type
    void ReplaceParameter(Function* func,
                          FunctionParam* old_param,
                          uint32_t index,
                          const core::type::TexelBuffer* bgra8) {
        // Redeclare the parameter with a rgba8unorm texel format.
        auto* rgba8 = ty.texel_buffer(core::TexelFormat::kRgba8Unorm, bgra8->Access());
        auto* new_param = b.FunctionParam(rgba8);
        if (auto name = ir.NameOf(old_param)) {
            ir.SetName(new_param, name.NameView());
        }

        Vector<FunctionParam*, 4> new_params = func->Params();
        new_params[index] = new_param;
        func->SetParams(std::move(new_params));

        // Replace all uses of the old parameter with the new one.
        ReplaceUses(old_param, new_param);
    }

    /// Recursively replace the uses of @p value with @p new_value.
    /// @param old_value the value whose usages should be replaced
    /// @param new_value the value to use instead
    void ReplaceUses(Value* old_value, Value* new_value) {
        old_value->ForEachUseUnsorted([&](Usage use) {
            tint::Switch(
                use.instruction,
                [&](Load* load) {
                    // Replace load instructions with new ones that have the updated type.
                    auto* new_load = b.Load(new_value);
                    new_load->InsertBefore(load);
                    ReplaceUses(load->Result(), new_load->Result());
                    load->Destroy();
                },
                [&](CoreBuiltinCall* call) {
                    // Replace arguments to builtin functions and add swizzles if necessary.
                    call->SetOperand(use.operand_index, new_value);
                    if (call->Func() == core::BuiltinFn::kTextureStore) {
                        // Swizzle the value argument of a `textureStore()` builtin.
                        uint32_t index = 2u;
                        if (auto* tex = old_value->Type()->As<core::type::StorageTexture>()) {
                            index = core::type::IsTextureArray(tex->Dim()) ? 3u : 2u;
                        }
                        auto* value = call->Args()[index];
                        auto* swizzle = b.Swizzle(value->Type(), value, Vector{2u, 1u, 0u, 3u});
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
                [&](UserCall* call) {
                    // Just replace arguments to user functions and then stop.
                    call->SetOperand(use.operand_index, new_value);
                },
                TINT_ICE_ON_NO_MATCH);
        });
    }
};

}  // namespace

Result<SuccessType> Bgra8UnormPolyfill(Module& ir) {
    auto result =
        ValidateAndDumpIfNeeded(ir, "core.Bgra8UnormPolyfill", kBgra8UnormPolyfillCapabilities);
    if (result != Success) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
