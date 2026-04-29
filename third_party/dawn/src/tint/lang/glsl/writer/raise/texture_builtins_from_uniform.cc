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

#include <algorithm>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/binding_array.h"

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
    struct OffsetAndCount {
        uint32_t offset;
        uint32_t count;
    };
    Hashmap<BindingPoint, OffsetAndCount, 2> binding_point_to_uniform_info_{};

    /// Process the module.
    Result<SuccessType> Process() {
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
        if (call_worklist.IsEmpty()) {
            return Success;
        }

        AddTextureBuiltinUniform();

        // Replace the builtin calls that we found
        for (auto* call : call_worklist) {
            switch (call->Func()) {
                case core::BuiltinFn::kTextureNumLevels:
                case core::BuiltinFn::kTextureNumSamples:
                    TINT_CHECK_RESULT(TextureFromUniform(call));
                    break;
                default:
                    TINT_IR_UNREACHABLE(ir);
            }
        }

        return Success;
    }

    void AddTextureBuiltinUniform() {
        uint32_t required_size = 0;
        for (auto builtin : cfg.ubo_contents) {
            required_size = std::max(required_size, builtin.offset + builtin.count);
            binding_point_to_uniform_info_.Add(builtin.binding, OffsetAndCount{
                                                                    .offset = builtin.offset,
                                                                    .count = builtin.count,
                                                                });
        }

        // The uniform buffer will contain packed u32s. The uniform buffer packing rules make
        // array<u32> have a stride of 16, so instead we use an array<vec4> that will be indexed
        // twice.
        auto vec4_count = (required_size + 3) / 4;

        // Wrap the array<vec4> in a struct as GLSL cannot have uniforms that are arrays directly.
        Vector<core::type::Manager::StructMemberDesc, 1> members;
        members.Push({ir.symbols.New("metadata"), ty.array(ty.vec4u(), vec4_count)});
        auto* strct =
            ir.Types().Struct(ir.symbols.New("TintTextureUniformData"), std::move(members));

        b.Append(ir.root_block, [&] {
            texture_uniform_data_ = b.Var(ty.ptr(uniform, strct, read));
            texture_uniform_data_->SetBindingPoint(0, cfg.ubo_binding.binding);
        });
    }

    // Get the module root var a texture value comes from.
    struct TextureVariablePath {
        /// The root module variable.
        core::ir::Var* var = nullptr;
        /// The index in the binding_array, nullptr if not a binding_array.
        core::ir::Value* index = nullptr;
    };
    TextureVariablePath PathForTexture(core::ir::Value* val) const {
        // Because DirectVariableAccess for handles ran prior to this transform, we know that the
        // chain to get to the variable is a mix of loads and accesses (but don't have guarantees on
        // their order). There is at most one load and one access so the recursion is bounded.
        return Switch(
            val->As<core::ir::InstructionResult>()->Instruction(),
            [&](core::ir::Var* var) -> TextureVariablePath { return {var}; },
            [&](core::ir::Load* load) -> TextureVariablePath {
                return PathForTexture(load->From());
            },
            [&](core::ir::Access* access) -> TextureVariablePath {
                auto* binding_array = access->Object();
                TINT_IR_ASSERT(ir, access->Indices().size() == 1);
                auto* index = access->Indices()[0];

                TextureVariablePath path = PathForTexture(binding_array);
                TINT_IR_ASSERT(ir, path.index == nullptr);
                path.index = index;
                return path;
            },
            TINT_ICE_ON_NO_MATCH);
    }

    // Note, assumes is called inside an insert block
    Result<core::ir::Value*> GetAccessFromUniform(core::ir::Value* arg) {
        auto path = PathForTexture(arg);

        BindingPoint binding = {
            .group = 0,
            .binding = path.var->BindingPoint()->binding,
        };

        auto metadata = binding_point_to_uniform_info_.Get(binding);
        if (!metadata) {
            return Failure("texture missing from texture_builtins_from_uniform list");
        }
        if (auto* ary = path.var->Result()->Type()->UnwrapPtr()->As<core::type::BindingArray>()) {
            auto count = ary->Count()->As<core::type::ConstantArrayCount>()->value;
            if (count > metadata->count) {
                return Failure(
                    "binding_array of textures doesn't have enough data in "
                    "texture_builtins_from_uniform list");
            }
        }

        // Returns the u32 at `metadata_offset` + `path.index` (if present) in
        // `texture_uniform_data`. Because it is a uniform buffer it contains an array of uvec4
        // instead of an array of u32 so we need more complicated indexing logic.
        core::ir::Value* offset = b.Constant(u32(metadata->offset));
        if (path.index != nullptr) {
            offset = b.Add(offset, b.InsertConvertIfNeeded(ty.u32(), path.index))->Result();
        }
        auto* index_in_array = b.Divide(offset, u32(4));
        auto* index_in_vector = b.Modulo(offset, u32(4));

        auto* vec4_ptr =
            b.Access(ty.ptr<uniform>(ty.vec4u()), texture_uniform_data_, u32(0), index_in_array);
        auto* vec4_value = b.Load(vec4_ptr);
        auto* u32_value = b.Access(ty.u32(), vec4_value, index_in_vector);
        return u32_value->Result();
    }

    Result<SuccessType> TextureFromUniform(core::ir::BuiltinCall* call) {
        auto* src = call->Args()[0];
        Result<core::ir::Value*> val;
        b.InsertBefore(call, [&] {  //
            val = GetAccessFromUniform(src);
        });
        TINT_CHECK_RESULT(val);
        call->Result()->ReplaceAllUsesWith(val.Get());

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

        return Success;
    }
};

}  // namespace

Result<SuccessType> TextureBuiltinsFromUniform(core::ir::Module& ir,
                                               const TextureBuiltinsFromUniformOptions& cfg) {
    AssertValid(ir, kTextureBuiltinFromUniformCapabilities,
                "before glsl.TextureBuiltinsFromUniform");

    return State{ir, cfg}.Process();
}

}  // namespace tint::glsl::writer::raise
