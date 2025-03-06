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

#include "src/tint/lang/hlsl/writer/raise/pixel_local.h"

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/exit.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/lang/hlsl/type/rasterizer_ordered_texture_2d.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/result/result.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    // Config options
    const PixelLocalOptions& options;

    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    // A Rasterizer Order View (ROV)
    struct ROV {
        core::ir::Var* var;
        core::type::Type* subtype;
    };
    // Create ROV root variables, one per member of `pixel_local_struct`
    Vector<ROV, 4> CreateROVs(const core::type::Struct* pixel_local_struct) {
        Vector<ROV, 4> rovs;
        // Create ROVs for each member of the struct
        for (auto* mem : pixel_local_struct->Members()) {
            auto iter = options.attachments.find(mem->Index());
            if (iter == options.attachments.end()) {
                TINT_ICE() << "missing options for member at index " << mem->Index();
            }
            core::TexelFormat texel_format;
            switch (iter->second.format) {
                case PixelLocalAttachment::TexelFormat::kR32Sint:
                    texel_format = core::TexelFormat::kR32Sint;
                    break;
                case PixelLocalAttachment::TexelFormat::kR32Uint:
                    texel_format = core::TexelFormat::kR32Uint;
                    break;
                case PixelLocalAttachment::TexelFormat::kR32Float:
                    texel_format = core::TexelFormat::kR32Float;
                    break;
                default:
                    TINT_ICE() << "missing texel format for pixel local storage attachment";
            }
            auto* subtype = hlsl::type::RasterizerOrderedTexture2D::SubtypeFor(texel_format, ty);

            auto* rov_ty = ty.Get<hlsl::type::RasterizerOrderedTexture2D>(texel_format, subtype);
            auto* rov = b.Var("pixel_local_" + mem->Name().Name(), ty.ptr<handle>(rov_ty));
            rov->SetBindingPoint(options.group_index, iter->second.index);

            ir.root_block->Append(rov);
            rovs.Emplace(rov, subtype);
        }
        return rovs;
    }

    void ProcessFragmentEntryPoint(core::ir::Function* entry_point,
                                   core::ir::Var* pixel_local_var,
                                   const core::type::Struct* pixel_local_struct,
                                   const Vector<ROV, 4>& rovs) {
        TINT_ASSERT(entry_point->Params().Length() == 1);  // Guaranteed by ShaderIO
        core::ir::FunctionParam* entry_point_param = entry_point->Params()[0];

        auto* param_struct = entry_point_param->Type()->As<core::type::Struct>();
        TINT_ASSERT(param_struct);  // Guaranteed by ShaderIO

        // Find the position builtin in the struct
        const core::type::StructMember* position_member = nullptr;
        for (auto* mem : param_struct->Members()) {
            if (mem->Attributes().builtin == core::BuiltinValue::kPosition) {
                position_member = mem;
                break;
            }
        }
        TINT_ASSERT(position_member);

        // Get the entry point's single return instruction. We expect only one as this transform
        // should run after ShaderIO.
        core::ir::Return* entry_point_ret = nullptr;
        if (entry_point->UsagesUnsorted().Count() == 1) {
            entry_point_ret =
                entry_point->UsagesUnsorted().begin()->instruction->As<core::ir::Return>();
        }
        if (!entry_point_ret) {
            TINT_ICE() << "expected entry point with a single return";
        }

        // Change the address space of the var from 'pixel_local' to 'private'
        pixel_local_var->Result(0)->SetType(ty.ptr<private_>(pixel_local_struct));
        // As well as the usages
        for (auto& usage : pixel_local_var->Result(0)->UsagesUnsorted()) {
            if (auto* ptr = usage->instruction->Result(0)->Type()->As<core::type::Pointer>()) {
                usage->instruction->Result(0)->SetType(ty.ptr<private_>(ptr->StoreType()));
            }
        }

        // Insert coord decl used to index ROVs at the entry point start
        core::ir::Instruction* coord = nullptr;
        b.InsertBefore(entry_point->Block()->Front(), [&] {
            coord = b.Access(ty.vec4<f32>(), entry_point_param, u32(position_member->Index()));
            coord = b.Swizzle(ty.vec2<f32>(), coord, {0, 1});
            coord = b.Convert<vec2<u32>>(coord);  // Input type to .Load
        });

        // Insert copy from ROVs to the struct right after the coord decl
        b.InsertAfter(coord, [&] {
            for (auto* mem : pixel_local_struct->Members()) {
                auto& rov = rovs[mem->Index()];
                auto* mem_ty = mem->Type();
                TINT_ASSERT(mem_ty->Is<core::type::Scalar>());
                core::ir::Instruction* from = b.Load(rov.var);
                // Load returns a vec4, so we need to swizzle the first element
                from = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                    ty.vec4(rov.subtype), tint::hlsl::BuiltinFn::kLoad, from, coord);
                from = b.Swizzle(rov.subtype, from, {0});
                if (mem_ty != rov.subtype) {
                    // ROV and struct member types don't match
                    from = b.Convert(mem_ty, from);
                }
                auto* to = b.Access(ty.ptr<private_>(mem_ty), pixel_local_var, u32(mem->Index()));
                b.Store(to, from);
            }
        });

        // Insert a copy from the struct back to ROVs at the return point
        b.InsertBefore(entry_point_ret, [&] {
            for (auto* mem : pixel_local_struct->Members()) {
                auto& rov = rovs[mem->Index()];
                auto* mem_ty = mem->Type();
                TINT_ASSERT(mem_ty->Is<core::type::Scalar>());
                core::ir::Instruction* from =
                    b.Access(ty.ptr<private_>(mem_ty), pixel_local_var, u32(mem->Index()));
                from = b.Load(from);
                if (mem_ty != rov.subtype) {
                    // ROV and struct member types don't match
                    from = b.Convert(rov.subtype, from);
                }
                // Store requires a vec4
                from = b.Construct(ty.vec4(rov.subtype), from);
                core::ir::Instruction* to = b.Load(rov.var);
                b.Call<hlsl::ir::BuiltinCall>(  //
                    ty.void_(), hlsl::BuiltinFn::kTextureStore, to, coord, from);
            }
        });
    }

    /// Process the module.
    void Process() {
        if (options.attachments.size() == 0) {
            return;
        }

        // Inline pointers
        for (auto* inst : ir.Instructions()) {
            if (auto* l = inst->As<core::ir::Let>()) {
                if (l->Result(0)->Type()->Is<core::type::Pointer>()) {
                    l->Result(0)->ReplaceAllUsesWith(l->Value());
                    l->Destroy();
                }
            }
        }

        // Find the pixel_local module var, if any
        core::ir::Var* pixel_local_var = nullptr;
        const core::type::Struct* pixel_local_struct = nullptr;
        for (auto* inst : *ir.root_block) {
            if (auto* var = inst->As<core::ir::Var>()) {
                auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
                if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
                    pixel_local_var = var;
                    pixel_local_struct = ptr->StoreType()->As<core::type::Struct>();
                    break;
                }
            }
        }
        if (!pixel_local_var || !pixel_local_var->Result(0)->IsUsed()) {
            return;
        }
        if (!pixel_local_struct) {
            TINT_ICE() << "pixel_local var must be of struct type";
        }
        if (pixel_local_struct->Members().Length() != options.attachments.size()) {
            TINT_ICE() << "missing options for each member of the pixel_local struct";
        }

        auto rovs = CreateROVs(pixel_local_struct);

        for (auto f : ir.functions) {
            if (f->IsFragment()) {
                ProcessFragmentEntryPoint(f, pixel_local_var, pixel_local_struct, rovs);
            }
        }
    }
};

}  // namespace

Result<SuccessType> PixelLocal(core::ir::Module& ir, const PixelLocalConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "hlsl.PixelLocal",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowClipDistancesOnF32,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{config.options, ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
