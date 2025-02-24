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

#include "src/tint/lang/glsl/writer/raise/texture_builtins_from_uniform.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::glsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The configuration
    const TextureBuiltinsFromUniformOptions cfg;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The global var for texture uniform information
    core::ir::Var* texture_uniform_data_ = nullptr;

    /// Map from binding point to index into uniform structure
    Hashmap<BindingPoint, uint32_t, 2> binding_point_to_uniform_idx_{};

    /// Maps from a function parameter to the replacement function parameter for texture uniform
    /// data
    Hashmap<core::ir::FunctionParam*, core::ir::FunctionParam*, 2> texture_param_replacement_{};

    /// Process the module.
    void Process() {
        Vector<core::ir::CoreBuiltinCall*, 4> call_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kTextureNumLevels:
                    case core::BuiltinFn::kTextureNumSamples:
                        call_worklist.Push(call);
                        break;
                    default:
                        break;
                }
                continue;
            }
        }

        // Replace the builtin calls that we found
        for (auto* call : call_worklist) {
            switch (call->Func()) {
                case core::BuiltinFn::kTextureNumLevels:
                case core::BuiltinFn::kTextureNumSamples:
                    TextureFromUniform(call);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
        }
    }

    void MakeTextureUniformStructure() {
        if (texture_uniform_data_ != nullptr) {
            return;
        }

        auto count = cfg.ubo_bindingpoint_ordering.size();

        Vector<core::type::Manager::StructMemberDesc, 2> members;
        members.Reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            members.Push({ir.symbols.New("tint_builtin_value_" + std::to_string(i)), ty.u32()});
            binding_point_to_uniform_idx_.Add(cfg.ubo_bindingpoint_ordering[i], i);
        }

        auto* strct =
            ir.Types().Struct(ir.symbols.New("TintTextureUniformData"), std::move(members));

        b.Append(ir.root_block, [&] {
            texture_uniform_data_ = b.Var(ty.ptr(uniform, strct, read));
            texture_uniform_data_->SetBindingPoint(cfg.ubo_binding.group, cfg.ubo_binding.binding);
        });
    }

    // Note, assumes is called inside an insert block
    core::ir::Access* GetUniformValue(const BindingPoint& binding) {
        TINT_ASSERT(binding_point_to_uniform_idx_.Contains(binding));

        uint32_t idx = *binding_point_to_uniform_idx_.Get(binding);
        return b.Access(ty.ptr<uniform, u32, read>(), texture_uniform_data_, u32(idx));
    }

    core::ir::Value* FindSource(core::ir::Value* tex) {
        core::ir::Value* res = nullptr;
        while (res == nullptr) {
            tint::Switch(
                tex,
                [&](core::ir::InstructionResult* r) {
                    tint::Switch(
                        r->Instruction(),                               //
                        [&](core::ir::Var* v) { res = v->Result(0); },  //
                        [&](core::ir::Load* l) { tex = l->From(); },    //
                        TINT_ICE_ON_NO_MATCH);
                },
                [&](core::ir::FunctionParam* fp) { res = fp; },

                TINT_ICE_ON_NO_MATCH);
        }
        return res;
    }

    core::ir::Value* GetAccessFromUniform(core::ir::Value* arg) {
        auto* src = FindSource(arg);
        return tint::Switch(
            src,
            [&](core::ir::InstructionResult* r) -> core::ir::Value* {
                if (auto* v = r->Instruction()->As<core::ir::Var>()) {
                    auto* access = GetUniformValue(v->BindingPoint().value());
                    return b.Load(access)->Result(0);

                } else {
                    TINT_UNREACHABLE() << "invalid instruction type";
                }
            },
            [&](core::ir::FunctionParam* fp) { return AppendFunctionParam(fp); },
            TINT_ICE_ON_NO_MATCH);
    }

    core::ir::Value* AppendFunctionParam(core::ir::FunctionParam* fp) {
        return texture_param_replacement_.GetOrAdd(fp, [&] {
            auto* new_param = b.FunctionParam("tint_tex_value", ty.u32());
            fp->Function()->AppendParam(new_param);
            texture_param_replacement_.Add(fp, new_param);

            AddUniformArgToCallSites(fp->Function(), fp->Index());
            return new_param;
        });
    }

    void AddUniformArgToCallSites(core::ir::Function* func, uint32_t tex_idx) {
        for (auto usage : func->UsagesUnsorted()) {
            auto* call = usage->instruction->As<core::ir::UserCall>();
            if (!call) {
                continue;
            }

            b.InsertBefore(call, [&] {
                auto* val = GetAccessFromUniform(call->Args()[tex_idx]);
                call->AppendArg(val);
            });
        }
    }

    void TextureFromUniform(core::ir::BuiltinCall* call) {
        MakeTextureUniformStructure();

        auto* src = call->Args()[0];
        b.InsertBefore(call, [&] {
            auto* val = GetAccessFromUniform(src);
            call->Result(0)->ReplaceAllUsesWith(val);
        });

        call->Destroy();

        // Clean up the load and texture declaration if destroying the call leaves them
        // orphaned.
        if (auto* ld = src->As<core::ir::InstructionResult>()) {
            if (!ld->IsUsed()) {
                if (auto* load = ld->Instruction()->As<core::ir::Load>()) {
                    auto* tex = load->From()->As<core::ir::InstructionResult>();
                    load->Destroy();

                    if (!tex->IsUsed()) {
                        tex->Instruction()->Destroy();
                    }
                }
            }
        }
    }
};

}  // namespace

Result<SuccessType> TextureBuiltinsFromUniform(core::ir::Module& ir,
                                               const TextureBuiltinsFromUniformOptions& cfg) {
    auto result = ValidateAndDumpIfNeeded(ir, "glsl.TextureBuiltinsFromUniform");
    if (result != Success) {
        return result.Failure();
    }

    State{ir, cfg}.Process();

    return Success;
}

}  // namespace tint::glsl::writer::raise
