// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/writer/printer/printer.h"
#include "src/tint/lang/glsl/writer/raise/raise.h"

namespace tint::glsl::writer {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // Check for unsupported types.
    for (auto* ty : ir.Types()) {
        if (ty->Is<core::type::SubgroupMatrix>()) {
            return Failure("subgroup matrices are not supported by the GLSL backend");
        }
    }

    // Make sure that every texture variable is in the texture_builtins_from_uniform binding list,
    // otherwise TextureBuiltinsFromUniform will fail.
    // Also make sure there is at most one user-declared push_constant, and make a note of its size.
    uint32_t user_push_constant_size = 0;
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<core::ir::Var>();

        if (!var) {
            continue;
        }
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();

        // The pixel_local extension is not supported by the GLSL backend.
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return Failure("pixel_local address space is not supported by the GLSL backend");
        }

        if (ptr->StoreType()->Is<core::type::Texture>()) {
            bool found = false;
            auto binding_point = var->BindingPoint();
            for (auto& bp :
                 options.bindings.texture_builtins_from_uniform.ubo_bindingpoint_ordering) {
                if (bp == binding_point) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return Failure("texture missing from texture_builtins_from_uniform list");
            }

            // Check texel formats for read-write storage textures when targeting ES.
            if (options.version.IsES()) {
                if (auto* st = ptr->StoreType()->As<core::type::StorageTexture>()) {
                    if (st->Access() == core::Access::kReadWrite) {
                        switch (st->TexelFormat()) {
                            case core::TexelFormat::kR32Float:
                            case core::TexelFormat::kR32Sint:
                            case core::TexelFormat::kR32Uint:
                                break;
                            default:
                                return Failure("unsupported read-write storage texture format");
                        }
                    }
                }
            }
        }

        if (ptr->AddressSpace() == core::AddressSpace::kPushConstant) {
            if (user_push_constant_size > 0) {
                // We've already seen a user-declared push constant.
                return Failure("multiple user-declared push constants");
            }
            user_push_constant_size = tint::RoundUp(4u, ptr->StoreType()->Size());
        }
    }

    // Check for calls to unsupported builtin functions.
    for (auto* inst : ir.Instructions()) {
        auto* call = inst->As<core::ir::CoreBuiltinCall>();
        if (!call) {
            continue;
        }

        if (core::IsSubgroup(call->Func())) {
            return Failure("subgroups are not supported by the GLSL backend");
        }
        if (call->Func() == core::BuiltinFn::kInputAttachmentLoad) {
            return Failure("input attachments are not supported by the GLSL backend");
        }
    }

    // Check for unsupported shader IO builtins.
    for (auto& func : ir.functions) {
        if (!func->IsEntryPoint()) {
            continue;
        }

        // subgroup builtins are not supported.
        for (auto* param : func->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    if (member->Attributes().builtin == core::BuiltinValue::kSubgroupInvocationId ||
                        member->Attributes().builtin == core::BuiltinValue::kSubgroupSize) {
                        return Failure("subgroups are not supported by the GLSL backend");
                    }
                }
            } else {
                if (param->Builtin() == core::BuiltinValue::kSubgroupInvocationId ||
                    param->Builtin() == core::BuiltinValue::kSubgroupSize) {
                    return Failure("subgroups are not supported by the GLSL backend");
                }
            }
        }

        // clip_distance is not supported.
        if (auto* str = func->ReturnType()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                if (member->Attributes().builtin == core::BuiltinValue::kClipDistances) {
                    return Failure("clip_distances is not supported by the GLSL backend");
                }
            }
        }
    }

    static constexpr uint32_t kMaxOffset = 0x1000;
    Hashset<uint32_t, 4> push_constant_word_offsets;
    auto check_push_constant_offset = [&](uint32_t offset) {
        // Excessive values can cause OOM / timeouts when padding structures in the printer.
        if (offset > kMaxOffset) {
            return false;
        }
        // Offset must be 4-byte aligned.
        if (offset & 0x3) {
            return false;
        }
        // Offset must not have already been used.
        if (!push_constant_word_offsets.Add(offset >> 2)) {
            return false;
        }
        // Offset must be after the user-defined push constants.
        if (offset < user_push_constant_size) {
            return false;
        }
        return true;
    };

    if (options.first_instance_offset &&
        !check_push_constant_offset(*options.first_instance_offset)) {
        return Failure("invalid offset for first_instance_offset push constant");
    }

    if (options.first_vertex_offset && !check_push_constant_offset(*options.first_vertex_offset)) {
        return Failure("invalid offset for first_vertex_offset push constant");
    }

    if (options.depth_range_offsets) {
        if (!check_push_constant_offset(options.depth_range_offsets->max) ||
            !check_push_constant_offset(options.depth_range_offsets->min)) {
            return Failure("invalid offsets for depth range push constants");
        }
    }

    return Success;
}

Result<Output> Generate(core::ir::Module& ir, const Options& options, const std::string&) {
    // Raise from core-dialect to GLSL-dialect.
    if (auto res = Raise(ir, options); res != Success) {
        return res.Failure();
    }

    return Print(ir, options);
}

}  // namespace tint::glsl::writer
