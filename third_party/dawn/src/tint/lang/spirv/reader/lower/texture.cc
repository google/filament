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

#include "src/tint/lang/spirv/reader/lower/texture.h"

#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/spirv/builtin_fn.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/image.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

enum class ImageOperandsMask : uint32_t {
    kBias = 0x00000001,
    kLod = 0x00000002,
    kGrad = 0x00000004,
    kConstOffset = 0x00000008,
    kSample = 0x00000040,
};

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Map of all OpSampledImages seen
    Hashmap<core::ir::Value*, core::ir::Instruction*, 4> sampled_images_{};

    /// Any `ir::UserCall` instructions which have texture params which need to be updated.
    Hashset<core::ir::UserCall*, 2> user_calls_to_convert_{};

    /// The `ir::Values`s which have had their types changed, they then need to have their
    /// usages updated to match. This maps to the root FunctionParam or Var for each texture.
    Vector<core::ir::Value*, 8> values_to_fix_usages_{};

    Vector<core::ir::Let*, 8> lets_to_inline_{};

    /// Function to texture replacements, this is done by hashcode since the
    /// function pointer is combined with the parameters which are converted to
    /// textures.
    Hashmap<size_t, core::ir::Function*, 4> func_hash_to_func_{};

    /// Set of textures used in dref calls which need to be depth textures.
    Hashset<core::ir::Value*, 4> textures_to_convert_to_depth_{};
    /// Set of samplers used in dref calls which need to be comparison samplers
    Hashset<core::ir::Value*, 4> samplers_to_convert_to_comparison_{};

    /// Process the module.
    void Process() {
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            TINT_ASSERT(ptr);

            auto* type = ptr->UnwrapPtr();
            if (!type->IsAnyOf<spirv::type::Image, core::type::Sampler>()) {
                continue;
            }

            auto* new_ty = TypeFor(type);
            if (type->Is<spirv::type::Image>()) {
                var->Result()->SetType(ty.ptr(ptr->AddressSpace(), new_ty, ptr->Access()));
                values_to_fix_usages_.Push(var->Result());
            }

            var->Result()->ForEachUseUnsorted([&](const core::ir::Usage& usage) {
                tint::Switch(
                    usage.instruction,  //
                    [&](core::ir::Load* l) {
                        if (type->Is<spirv::type::Image>()) {
                            l->Result()->SetType(new_ty);
                        }
                    },
                    [&](core::ir::Let* let) {
                        if (type->Is<core::type::Sampler>()) {
                            lets_to_inline_.Push(let);
                        }
                    });
            });
        }

        Vector<spirv::ir::BuiltinCall*, 4> depth_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<spirv::ir::BuiltinCall>()) {
                switch (builtin->Func()) {
                    case spirv::BuiltinFn::kSampledImage:
                        SampledImage(builtin);
                        break;
                    case spirv::BuiltinFn::kImageDrefGather:
                    case spirv::BuiltinFn::kImageSampleDrefImplicitLod:
                    case spirv::BuiltinFn::kImageSampleDrefExplicitLod:
                    case spirv::BuiltinFn::kImageSampleProjDrefImplicitLod:
                    case spirv::BuiltinFn::kImageSampleProjDrefExplicitLod:
                        depth_worklist.Push(builtin);
                        break;
                    default:
                        break;
                }
            }
        }

        // TODO(dsinclair): Propagate OpTypeSampledImage through function params by replacing with
        // the texture/sampler

        // Run the depth functions first so we can convert all the types to depth that are needed.
        // This then allows things like textureSample calls below to have the correct return type
        // and be able to convert the results if needed.
        for (auto* builtin : depth_worklist) {
            switch (builtin->Func()) {
                case spirv::BuiltinFn::kImageSampleDrefImplicitLod:
                case spirv::BuiltinFn::kImageSampleDrefExplicitLod:
                case spirv::BuiltinFn::kImageSampleProjDrefImplicitLod:
                case spirv::BuiltinFn::kImageSampleProjDrefExplicitLod:
                    ImageSampleDref(builtin);
                    break;
                case spirv::BuiltinFn::kImageDrefGather:
                    ImageGatherDref(builtin);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
        }

        for (auto tex : textures_to_convert_to_depth_) {
            ConvertVarToDepth(FindRootVarFor(tex));
        }
        for (auto tex : samplers_to_convert_to_comparison_) {
            ConvertVarToComparison(FindRootVarFor(tex));
        }
        UpdateValues();

        Vector<spirv::ir::BuiltinCall*, 4> builtin_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<spirv::ir::BuiltinCall>()) {
                switch (builtin->Func()) {
                    case spirv::BuiltinFn::kSampledImage:
                        // Note, we _also_ do this here even though it was done above. The one above
                        // registers for the depth functions, but, we may have forked functions in
                        // the `UpdateValues` when it does the `ConvertUserCalls`. This would then
                        // generate new `SampledImage` objects which need to be registered. In the
                        // worse case, we just write the same data twice.
                        SampledImage(builtin);
                        break;
                    case spirv::BuiltinFn::kImage:
                    case spirv::BuiltinFn::kImageRead:
                    case spirv::BuiltinFn::kImageFetch:
                    case spirv::BuiltinFn::kImageGather:
                    case spirv::BuiltinFn::kImageQueryLevels:
                    case spirv::BuiltinFn::kImageQuerySamples:
                    case spirv::BuiltinFn::kImageQuerySize:
                    case spirv::BuiltinFn::kImageQuerySizeLod:
                    case spirv::BuiltinFn::kImageSampleExplicitLod:
                    case spirv::BuiltinFn::kImageSampleImplicitLod:
                    case spirv::BuiltinFn::kImageSampleProjImplicitLod:
                    case spirv::BuiltinFn::kImageSampleProjExplicitLod:
                    case spirv::BuiltinFn::kImageWrite:
                        builtin_worklist.Push(builtin);
                        break;
                    default:
                        TINT_UNREACHABLE() << "unknown spirv builtin: " << builtin->Func();
                }
            }
        }

        for (auto* builtin : builtin_worklist) {
            switch (builtin->Func()) {
                case spirv::BuiltinFn::kImage:
                    Image(builtin);
                    break;
                case spirv::BuiltinFn::kImageRead:
                    ImageFetch(builtin);
                    break;
                case spirv::BuiltinFn::kImageFetch:
                    ImageFetch(builtin);
                    break;
                case spirv::BuiltinFn::kImageGather:
                    ImageGather(builtin);
                    break;
                case spirv::BuiltinFn::kImageQueryLevels:
                    ImageQuery(builtin, core::BuiltinFn::kTextureNumLevels);
                    break;
                case spirv::BuiltinFn::kImageQuerySamples:
                    ImageQuery(builtin, core::BuiltinFn::kTextureNumSamples);
                    break;
                case spirv::BuiltinFn::kImageQuerySize:
                case spirv::BuiltinFn::kImageQuerySizeLod:
                    ImageQuerySize(builtin);
                    break;
                case spirv::BuiltinFn::kImageSampleExplicitLod:
                case spirv::BuiltinFn::kImageSampleImplicitLod:
                case spirv::BuiltinFn::kImageSampleProjImplicitLod:
                case spirv::BuiltinFn::kImageSampleProjExplicitLod:
                    ImageSample(builtin);
                    break;
                case spirv::BuiltinFn::kImageWrite:
                    ImageWrite(builtin);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
        }

        // Destroy all the OpSampledImage instructions.
        for (auto res : sampled_images_) {
            // If the sampled image was in a user function which was forked and the original
            // destroyed then it will no longer be alive.
            if (res.value->Alive()) {
                res.value->Destroy();
            }
        }
    }

    void UpdateValues() {
        // The double loop happens because when we convert user calls, that will
        // add more values to convert, but those values can find user calls to
        // convert, so we have to work until we stabilize
        while (!values_to_fix_usages_.IsEmpty() || !lets_to_inline_.IsEmpty() ||
               !user_calls_to_convert_.IsEmpty()) {
            while (!values_to_fix_usages_.IsEmpty()) {
                auto* val = values_to_fix_usages_.Pop();
                ConvertUsagesToTexture(val);
            }

            while (!lets_to_inline_.IsEmpty()) {
                auto* let = lets_to_inline_.Pop();
                TINT_ASSERT(let->Value());
                let->Result()->ReplaceAllUsesWith(let->Value());
                // We may have done this value already, but push it back on as any of the let usages
                // will now point to it and they need to be updated.
                values_to_fix_usages_.Push(let->Value());
                let->Destroy();
            }

            auto user_calls = user_calls_to_convert_.Vector();
            // Sort for deterministic output
            user_calls.Sort();
            for (auto& call : user_calls) {
                ConvertUserCall(call);
            }
            user_calls_to_convert_.Clear();
        }

        TINT_ASSERT(lets_to_inline_.IsEmpty());
        TINT_ASSERT(values_to_fix_usages_.IsEmpty());
        TINT_ASSERT(user_calls_to_convert_.IsEmpty());
    }

    // Given a value, walk back up and find the root `var`.
    core::ir::Var* FindRootVarFor(core::ir::Value* val) {
        auto* inst_res = val->As<core::ir::InstructionResult>();
        TINT_ASSERT(inst_res);
        return tint::Switch(
            inst_res->Instruction(),  //
            [&](core::ir::Let* l) { return FindRootVarFor(l->Value()); },
            [&](core::ir::Load* l) { return FindRootVarFor(l->From()); },
            [&](core::ir::Var* v) { return v; },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void ConvertVarToDepth(core::ir::Var* var) {
        auto* orig_ptr_ty = var->Result()->Type()->As<core::type::Pointer>();
        TINT_ASSERT(orig_ptr_ty);

        auto* orig_tex_ty = orig_ptr_ty->UnwrapPtr()->As<core::type::Texture>();
        TINT_ASSERT(orig_tex_ty);

        const core::type::Type* depth_ty = nullptr;
        if (orig_tex_ty->Is<core::type::MultisampledTexture>()) {
            depth_ty = ty.depth_multisampled_texture(orig_tex_ty->Dim());
        } else {
            depth_ty = ty.depth_texture(orig_tex_ty->Dim());
        }

        auto* depth_ptr = ty.ptr(orig_ptr_ty->AddressSpace(), depth_ty, orig_ptr_ty->Access());
        var->Result()->SetType(depth_ptr);

        values_to_fix_usages_.Push(var->Result());
    }

    void ConvertVarToComparison(core::ir::Var* var) {
        auto* orig_ptr_ty = var->Result()->Type()->As<core::type::Pointer>();
        TINT_ASSERT(orig_ptr_ty);

        auto* sampler = ty.comparison_sampler();
        auto* sampler_ptr = ty.ptr(orig_ptr_ty->AddressSpace(), sampler, orig_ptr_ty->Access());
        var->Result()->SetType(sampler_ptr);

        values_to_fix_usages_.Push(var->Result());
    }

    // Stores information for operands which need to be updated with the new load result.
    struct ReplacementValue {
        // The instruction to update
        core::ir::Instruction* instruction;
        // The operand index to insert into
        size_t idx;
        // The new value to insert into the instruction operands at `idx`
        core::ir::Value* value;
    };

    void ConvertUsagesToTexture(core::ir::Value* val) {
        val->ForEachUseUnsorted([&](const core::ir::Usage& usage) {
            auto* inst = usage.instruction;

            tint::Switch(  //
                inst,      //
                [&](core::ir::Let* l) { lets_to_inline_.Push(l); },
                [&](core::ir::Load* l) {
                    auto* res = l->Result();
                    res->SetType(val->Type()->UnwrapPtr());
                    values_to_fix_usages_.Push(res);
                },  //
                [&](core::ir::UserCall* uc) { user_calls_to_convert_.Add(uc); },
                [&](core::ir::BuiltinCall*) {},  //
                TINT_ICE_ON_NO_MATCH);
        });
    }

    // The user calls need to check all of the parameters which were converted
    // to textures and create a forked function call for that combination of
    // parameters.
    void ConvertUserCall(core::ir::UserCall* uc) {
        auto* target = uc->Target();
        auto& params = target->Params();
        const auto& args = uc->Args();

        Vector<size_t, 2> to_convert;
        for (size_t i = 0; i < args.Length(); ++i) {
            if (params[i]->Type() != args[i]->Type()) {
                to_convert.Push(i);
            }
        }
        // Everything is already converted we're done.
        if (to_convert.IsEmpty()) {
            return;
        }

        // Hash based on the original function pointer and the specific
        // parameters we're converting.
        auto hash = Hash(target);
        hash = HashCombine(hash, to_convert);

        auto* new_fn = func_hash_to_func_.GetOrAdd(hash, [&] {
            core::ir::CloneContext ctx{ir};
            auto* fn = uc->Target()->Clone(ctx);
            ir.functions.Push(fn);

            for (auto idx : to_convert) {
                auto* p = fn->Params()[idx];
                p->SetType(args[idx]->Type());
                values_to_fix_usages_.Push(p);
            }
            return fn;
        });
        uc->SetTarget(new_fn);

        if (target->IsEntryPoint()) {
            return;
        }

        // Check if any of the usages are calls, if they aren't we can destroy the function.
        for (auto& usage : target->UsagesUnsorted()) {
            if (usage->instruction->Is<core::ir::Call>()) {
                return;
            }
        }
        ir.Destroy(target);
    }

    // Record the sampled image so we can extract the texture/sampler information as we process the
    // builtins. It will be destroyed after all builtins are done.
    void SampledImage(spirv::ir::BuiltinCall* call) { sampled_images_.Add(call->Result(), call); }

    std::pair<core::ir::Value*, core::ir::Value*> GetTextureSampler(core::ir::Value* sampled) {
        auto res = sampled_images_.Get(sampled);
        TINT_ASSERT(res);

        core::ir::Instruction* inst = *res;
        TINT_ASSERT(inst->Operands().Length() == 2);

        return {inst->Operands()[0], inst->Operands()[1]};
    }

    uint32_t CoordsRequiredForDim(core::type::TextureDimension dim, bool is_proj) {
        uint32_t ret = 0;
        if (is_proj) {
            ++ret;
        }

        switch (dim) {
            case core::type::TextureDimension::k1d:
                ret += 1;
                break;
            case core::type::TextureDimension::k2d:
                ret += 2;
                break;
            case core::type::TextureDimension::k2dArray:
            case core::type::TextureDimension::k3d:
            case core::type::TextureDimension::kCube:
                ret += 3;
                break;
            case core::type::TextureDimension::kCubeArray:
                ret += 4;
                break;
            default:
                TINT_UNREACHABLE();
        }

        return ret;
    }

    uint32_t Length(const core::type::Type* type) {
        if (type->IsScalar()) {
            return 1;
        }
        if (auto* vec = type->As<core::type::Vector>()) {
            return vec->Width();
        }
        TINT_UNREACHABLE();
    }

    void ProcessCoords(const core::type::Type* type,
                       bool is_proj,
                       core::ir::Value* coords,
                       Vector<core::ir::Value*, 5>& new_args) {
        auto* tex_ty = type->As<core::type::Texture>();
        TINT_ASSERT(tex_ty);

        auto coords_received = Length(coords->Type());

        auto mk_coords = [&](uint32_t count) -> core::ir::Value* {
            if (count == coords_received) {
                return coords;
            }

            Vector<uint32_t, 3> swizzle_idx = {0};
            for (uint32_t i = 1; i < count; ++i) {
                swizzle_idx.Push(i);
            }

            auto* new_ty = ty.MatchWidth(coords->Type()->DeepestElement(), count);
            return b.Swizzle(new_ty, coords, swizzle_idx)->Result();
        };

        auto coords_needed = CoordsRequiredForDim(tex_ty->Dim(), is_proj);
        TINT_ASSERT(coords_needed <= coords_received);

        if (!is_proj && !IsTextureArray(tex_ty->Dim())) {
            new_args.Push(mk_coords(coords_needed));
            return;
        }

        auto* coords_ty = coords->Type()->As<core::type::Vector>();
        TINT_ASSERT(coords_ty);

        // The array / projection index, is on the end
        uint32_t new_coords_width = coords_needed - 1;
        auto* new_coords_ty = ty.MatchWidth(coords_ty->Type(), new_coords_width);

        auto* swizzle = mk_coords(new_coords_width);
        core::ir::Value* last =
            b.Swizzle(coords_ty->Type(), coords, Vector{new_coords_width})->Result();

        if (is_proj) {
            // New coords
            // Divide the coordinates by the last value to simulate the
            // projection behaviour.
            new_args.Push(b.Divide(new_coords_ty, swizzle, last)->Result());
        } else {
            TINT_ASSERT(new_coords_ty->Is<core::type::Vector>());

            // New coords
            new_args.Push(swizzle);
            // Array index
            if (!last->Type()->Is<core::type::I32>()) {
                last = b.Convert(ty.i32(), last)->Result();
            }
            new_args.Push(last);
        }
    }

    void ProcessOffset(core::ir::Value* offset, Vector<core::ir::Value*, 5>& new_args) {
        if (offset->Type()->IsUnsignedIntegerVector()) {
            offset = b.Convert(ty.MatchWidth(ty.i32(), offset->Type()), offset)->Result();
        }
        new_args.Push(offset);
    }

    uint32_t GetOperandMask(core::ir::Value* val) {
        auto* op = val->As<core::ir::Constant>();
        TINT_ASSERT(op);
        return op->Value()->ValueAs<uint32_t>();
    }

    bool HasBias(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kBias)) != 0;
    }
    bool HasGrad(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kGrad)) != 0;
    }
    bool HasLod(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kLod)) != 0;
    }
    bool HasConstOffset(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kConstOffset)) != 0;
    }
    bool HasSample(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kSample)) != 0;
    }

    void Image(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();
        core::ir::Value* tex = nullptr;
        [[maybe_unused]] core::ir::Value* sampler = nullptr;
        std::tie(tex, sampler) = GetTextureSampler(args[0]);

        call->Result()->ReplaceAllUsesWith(tex);
        call->Destroy();
    }

    void ImageFetch(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];
            auto* coords = args[1];

            auto* tex_ty = tex->Type();
            uint32_t operand_mask = GetOperandMask(args[2]);

            Vector<core::ir::Value*, 5> new_args = {tex};
            ProcessCoords(tex->Type(), false, coords, new_args);

            uint32_t idx = 3;
            if (HasLod(operand_mask)) {
                core::ir::Value* lod = args[idx++];

                if (!lod->Type()->Is<core::type::I32>()) {
                    lod = b.Convert(ty.i32(), lod)->Result();
                }
                new_args.Push(lod);
            } else if (!tex_ty->IsAnyOf<core::type::DepthMultisampledTexture,
                                        core::type::MultisampledTexture,
                                        core::type::StorageTexture>()) {
                // textureLoad requires an explicit level-of-detail parameter for non-multisampled
                // and non-storage texture types.
                new_args.Push(b.Zero(ty.i32()));
            }
            if (HasSample(operand_mask)) {
                core::ir::Value* sample = args[idx++];

                if (!sample->Type()->Is<core::type::I32>()) {
                    sample = b.Convert(ty.i32(), sample)->Result();
                }
                new_args.Push(sample);
            }

            // Depth textures have a single value return in WGSL, but a vec4 in SPIR-V.
            auto* call_ty = call->Result()->Type();
            if (tex_ty->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>()) {
                call_ty = call_ty->DeepestElement();
            }
            auto* res = b.Call(call_ty, core::BuiltinFn::kTextureLoad, new_args)->Result();

            // Restore the vec4 result by padding with 0's.
            if (call_ty != call->Result()->Type()) {
                auto* vec = call->Result()->Type()->As<core::type::Vector>();
                TINT_ASSERT(vec && vec->Width() == 4);

                auto* z = b.Zero(call_ty);
                res = b.Construct(call->Result()->Type(), res, z, z, z)->Result();
            }
            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    void ImageGatherDref(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        b.InsertBefore(call, [&] {
            core::ir::Value* tex = nullptr;
            core::ir::Value* sampler = nullptr;
            std::tie(tex, sampler) = GetTextureSampler(args[0]);

            textures_to_convert_to_depth_.Add(tex);
            samplers_to_convert_to_comparison_.Add(sampler);

            auto* coords = args[1];
            auto* dref = args[2];

            uint32_t operand_mask = GetOperandMask(args[3]);

            Vector<core::ir::Value*, 5> new_args;

            new_args.Push(tex);
            new_args.Push(sampler);

            ProcessCoords(tex->Type(), false, coords, new_args);
            new_args.Push(dref);

            if (HasConstOffset(operand_mask)) {
                ProcessOffset(args[4], new_args);
            }

            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kTextureGatherCompare,
                             new_args);
        });
        call->Destroy();
    }

    void ImageGather(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        b.InsertBefore(call, [&] {
            core::ir::Value* tex = nullptr;
            core::ir::Value* sampler = nullptr;
            std::tie(tex, sampler) = GetTextureSampler(args[0]);

            auto* coords = args[1];
            auto* component = args[2];

            uint32_t operand_mask = GetOperandMask(args[3]);

            Vector<core::ir::Value*, 5> new_args;
            if (!tex->Type()->Is<core::type::DepthTexture>()) {
                new_args.Push(component);
            }
            new_args.Push(tex);
            new_args.Push(sampler);

            ProcessCoords(tex->Type(), false, coords, new_args);

            if (HasConstOffset(operand_mask)) {
                ProcessOffset(args[4], new_args);
            }

            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kTextureGather, new_args);
        });
        call->Destroy();
    }

    void ImageSampleDref(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        auto* sampled_image = args[0];

        core::ir::Value* tex = nullptr;
        core::ir::Value* sampler = nullptr;
        std::tie(tex, sampler) = GetTextureSampler(sampled_image);

        textures_to_convert_to_depth_.Add(tex);
        samplers_to_convert_to_comparison_.Add(sampler);

        auto* tex_ty = tex->Type();

        auto* coords = args[1];
        auto* depth = args[2];
        uint32_t operand_mask = GetOperandMask(args[3]);

        bool is_proj = call->Func() == spirv::BuiltinFn::kImageSampleProjDrefImplicitLod ||
                       call->Func() == spirv::BuiltinFn::kImageSampleProjDrefExplicitLod;

        uint32_t idx = 4;
        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 5> new_args;
            new_args.Push(tex);
            new_args.Push(sampler);

            ProcessCoords(tex_ty, is_proj, coords, new_args);
            new_args.Push(depth);

            auto fn = core::BuiltinFn::kTextureSampleCompare;
            if (HasLod(operand_mask)) {
                fn = core::BuiltinFn::kTextureSampleCompareLevel;
                idx++;  // Skip over the index

                // Metal only supports Lod = 0 for comparison sampling without derivatives. So, WGSL
                // doesn't take a level value, drop the LOD param.
            }
            if (HasConstOffset(operand_mask)) {
                ProcessOffset(args[idx++], new_args);
            }

            b.CallWithResult(call->DetachResult(), fn, new_args);
        });

        call->Destroy();
    }

    void ImageSample(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        auto* sampled_image = args[0];

        core::ir::Value* tex = nullptr;
        core::ir::Value* sampler = nullptr;
        std::tie(tex, sampler) = GetTextureSampler(sampled_image);

        auto* tex_ty = tex->Type();

        auto* coords = args[1];
        uint32_t operand_mask = GetOperandMask(args[2]);

        bool is_proj = call->Func() == spirv::BuiltinFn::kImageSampleProjImplicitLod ||
                       call->Func() == spirv::BuiltinFn::kImageSampleProjExplicitLod;

        uint32_t idx = 3;
        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 5> new_args;
            new_args.Push(tex);
            new_args.Push(sampler);

            ProcessCoords(tex_ty, is_proj, coords, new_args);

            core::BuiltinFn fn = core::BuiltinFn::kTextureSample;
            if (HasBias(operand_mask)) {
                fn = core::BuiltinFn::kTextureSampleBias;
                new_args.Push(args[idx++]);
            }
            if (HasLod(operand_mask)) {
                fn = core::BuiltinFn::kTextureSampleLevel;

                core::ir::Value* lod = args[idx++];

                // Depth texture LOD in WGSL is i32/u32 but f32 in SPIR-V.
                // Convert to i32
                if (tex_ty->Is<core::type::DepthTexture>()) {
                    lod = b.Convert(ty.i32(), lod)->Result();
                }
                new_args.Push(lod);
            }
            if (HasGrad(operand_mask)) {
                fn = core::BuiltinFn::kTextureSampleGrad;
                new_args.Push(args[idx++]);  // ddx
                new_args.Push(args[idx++]);  // ddy
            }
            if (HasConstOffset(operand_mask)) {
                ProcessOffset(args[idx++], new_args);
            }

            // Depth textures have a single value return in WGSL, but a vec4 in SPIR-V.
            auto* call_ty = call->Result()->Type();
            if (tex_ty->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>()) {
                call_ty = call_ty->DeepestElement();
            }
            auto* res = b.Call(call_ty, fn, new_args)->Result();

            // Restore the vec4 result by padding with 0's.
            if (call_ty != call->Result()->Type()) {
                auto* vec = call->Result()->Type()->As<core::type::Vector>();
                TINT_ASSERT(vec && vec->Width() == 4);

                auto* z = b.Zero(call_ty);
                res = b.Construct(call->Result()->Type(), res, z, z, z)->Result();
            }

            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    void ImageWrite(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];
            auto* coords = args[1];
            auto* texel = args[2];

            Vector<core::ir::Value*, 5> new_args;
            new_args.Push(tex);

            ProcessCoords(tex->Type(), false, coords, new_args);

            new_args.Push(texel);

            b.Call(call->Result()->Type(), core::BuiltinFn::kTextureStore, new_args);
        });
        call->Destroy();
    }

    void ImageQuery(spirv::ir::BuiltinCall* call, core::BuiltinFn fn) {
        auto* image = call->Args()[0];

        b.InsertBefore(call, [&] {
            auto* type = call->Result()->Type();

            // WGSL requires a `u32` result component where SPIR-V allows `i32` or `u32`
            core::ir::Value* res =
                b.Call(ty.MatchWidth(ty.u32(), type), fn, Vector{image})->Result();
            if (type->IsSignedIntegerScalarOrVector()) {
                res = b.Convert(type, res)->Result();
            }

            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    void ImageQuerySize(spirv::ir::BuiltinCall* call) {
        auto* image = call->Args()[0];

        auto* tex_ty = image->Type()->As<core::type::Texture>();
        TINT_ASSERT(tex_ty);

        b.InsertBefore(call, [&] {
            auto* type = call->Result()->Type();

            // WGSL requires a `u32` result component where SPIR-V allows `i32` or `u32`
            auto* wgsl_type = ty.MatchWidth(ty.u32(), type);

            // A SPIR-V OpImageQuery will return the array `element` entry with
            // the image query. In WGSL, these are two calls, the
            // `textureDimensions` will get the width,height,depth but we also
            // have to call `textureNumLayers` to get the elements and then
            // re-inject that back into the result.
            if (core::type::IsTextureArray(tex_ty->Dim())) {
                wgsl_type = ty.vec(ty.u32(), wgsl_type->As<core::type::Vector>()->Width() - 1);
            }

            Vector<core::ir::Value*, 2> args = {image};
            if (call->Func() == spirv::BuiltinFn::kImageQuerySizeLod) {
                args.Push(call->Args()[1]);
            }

            core::ir::Value* res =
                b.Call(wgsl_type, core::BuiltinFn::kTextureDimensions, args)->Result();

            if (core::type::IsTextureArray(tex_ty->Dim())) {
                core::ir::Value* layers =
                    b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, image)->Result();
                res = b.Construct(ty.MatchWidth(ty.u32(), type), res, layers)->Result();
            }

            if (type->IsSignedIntegerScalarOrVector()) {
                res = b.Convert(type, res)->Result();
            }

            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    const core::type::Type* TypeFor(const core::type::Type* src_ty) {
        if (auto* img = src_ty->As<spirv::type::Image>()) {
            return TypeForImage(img);
        }
        return src_ty;
    }

    core::type::TextureDimension ConvertDim(spirv::type::Dim dim, spirv::type::Arrayed arrayed) {
        switch (dim) {
            case spirv::type::Dim::kD1:
                return core::type::TextureDimension::k1d;
            case spirv::type::Dim::kD2:
                return arrayed == spirv::type::Arrayed::kArrayed
                           ? core::type::TextureDimension::k2dArray
                           : core::type::TextureDimension::k2d;
            case spirv::type::Dim::kD3:
                return core::type::TextureDimension::k3d;
            case spirv::type::Dim::kCube:
                return arrayed == spirv::type::Arrayed::kArrayed
                           ? core::type::TextureDimension::kCubeArray
                           : core::type::TextureDimension::kCube;
            default:
                TINT_UNREACHABLE();
        }
    }

    const core::type::Type* TypeForImage(const spirv::type::Image* img) {
        if (img->GetDim() == spirv::type::Dim::kSubpassData) {
            return ty.input_attachment(img->GetSampledType());
        }

        if (img->GetSampled() == spirv::type::Sampled::kReadWriteOpCompatible) {
            return ty.storage_texture(ConvertDim(img->GetDim(), img->GetArrayed()),
                                      img->GetTexelFormat(), img->GetAccess());
        }

        if (img->GetDepth() == spirv::type::Depth::kDepth) {
            if (img->GetMultisampled() == spirv::type::Multisampled::kMultisampled) {
                return ty.depth_multisampled_texture(ConvertDim(img->GetDim(), img->GetArrayed()));
            }
            return ty.depth_texture(ConvertDim(img->GetDim(), img->GetArrayed()));
        }

        if (img->GetMultisampled() == spirv::type::Multisampled::kMultisampled) {
            return ty.multisampled_texture(ConvertDim(img->GetDim(), img->GetArrayed()),
                                           img->GetSampledType());
        }

        return ty.sampled_texture(ConvertDim(img->GetDim(), img->GetArrayed()),
                                  img->GetSampledType());
    }
};

}  // namespace

Result<SuccessType> Texture(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.Texture",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowMultipleEntryPoints,
                                              core::ir::Capability::kAllowOverrides,
                                              core::ir::Capability::kAllowNonCoreTypes,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
