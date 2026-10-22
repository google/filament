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

#include "src/tint/lang/core/ir/transform/resource_table.h"

#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/resource_type.h"
#include "src/tint/lang/core/type/sampled_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// Th resource_table transform handles the removal of the `getResource` and `hasResource` calls and
/// replaces them with accesses into backend specific arrays. The backend specific parts are handled
/// by the `ResourceTableHelper` which is attached to the transform.
///
/// There are a number of cases this transform handles, and it makes assumptions based on the
/// information provided to it from the API. There is a bi-directional relationship in the
/// validation for the combined texture/sampler calls with the API.
///
/// We have to handle several cases in here:
///
/// 1. `hasResource` -- this is just changed to a validation check that the type in the WGSL
///    template (e.g. `hasResource<texture_2d<f32>>(n)`) matches the ResourceType that Dawn has
///    stored into the metadata table for the slot `n`.
///
/// 2. `getResource` with no usages -- this just gets removed, we don't use the value returned so we
///    don't actually need to do anything.
///
/// 3. `getResource<texture_*>` used without an associated sampler -- In this case we just need to
///    check that the type in `getResource<texture_*>` matches the type in slot `n` of the metadata
///    table. If the ResourceType does not match then we will use the default resource provided by
///    Dawn for the `texture_*` type.
///
/// 4. `getResource<texture_*>` used with a `getResource<sampler>` -- In this case we need to
///    validate that the `getResource<texture_*>(t)` is correct against metadata slot `t`, and the
///    `getResource<sampler>(s)` is correct against the metadata slot `s`. Additionally we need to
///    validate if the `sampler` is `filtering` then the `texture` is a `filterable` texture. If
///    the texture/sampler pair are incompatible (an unfilterable texture paired with a filtering
///    sampler) then we will return a `0` value result.
///
///    ```
///    if (sampler.kind == Filtering && IsUnFilterable(texture.kind)) {
///      value = vec4(0)
///    } else {
///      value = textureSample(t, s)
///    }
///    ```
///
///    Note, this means there is no substitution of default texture or sampler.
///
/// 5. `getResource<texture_*>` used with a bindful sampler -- In this case we need to check that
///    the type in `getResource<texture_*>(t)` matches the type in the slot `t` of the metadata
///    table. The API will provide information to Tint on the ResourceKind of the sampler, which is
///    known at pipeline creation time. We will then validate that the texture/sampler pair are
///    compatible (same as #4). If the are not compatible we will return a 0 value.
///
///    Note, this means there is no substitution of default texture or sampler.
///
/// 6. `getResource<sampler>` used with a bindful texture -- In this case we will validate that the
///    ResourceType of the element at slot `n` is a sampler. We then get the `ResourceType` for the
///    bindful texture as reported by the API side. At this point we do the same validation as #5.

struct State {
    /// The configuration.
    const std::optional<ResourceTableConfig>& config;

    /// The IR module.
    core::ir::Module& ir;

    /// The helper
    ResourceTableHelper* helper;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The storage buffer used to hold API metadata
    core::ir::Var* storage_buffer = nullptr;

    /// Map from a tint Type to the `var` which holds the resource table of that type
    Hashmap<const core::type::Type*, core::ir::Var*, 4> var_for_type{};

    /// Maps resource_index value to the default offset for that type
    std::unordered_map<ResourceType, uint32_t> resource_type_to_default_idx{};

    /// Information about an item that we need to get from the resource table.
    struct Info {
        const core::type::Type* binding_type;
        core::ir::Value* slot_idx;
        size_t operand_idx;
    };
    /// A list of instructions which need to be replaced with the, possibly, two indices which are
    /// using a resource_table element.
    struct CallArgs {
        std::optional<Info> texture;
        std::optional<Info> sampler;
    };
    Hashmap<ir::CoreBuiltinCall*, CallArgs, 8> sampled_call_replacements{};

    /// Process the module.
    Result<SuccessType> Process() {
        // Find calls to hasResource() and getResource().
        Vector<core::ir::CoreBuiltinCall*, 8> has_resource_calls;
        Vector<core::ir::CoreBuiltinCall*, 8> get_resource_calls;
        for (auto* inst : ir.Instructions()) {
            auto* call = inst->As<core::ir::CoreBuiltinCall>();
            if (call == nullptr) {
                continue;
            }
            if (call->Func() == core::BuiltinFn::kHasResource) {
                has_resource_calls.Push(call);
            } else if (call->Func() == core::BuiltinFn::kGetResource) {
                get_resource_calls.Push(call);
            }
        }
        if (has_resource_calls.IsEmpty() && get_resource_calls.IsEmpty()) {
            return Success;
        }
        if (!config.has_value()) {
            return Failure{"hasResource and getResource require a resource table"};
        }

        InjectRootBlockEntries();
        CalculateDefaultBindingIndices();

        ReplaceHasResourceCalls(has_resource_calls);
        ReplaceGetResourceCalls(get_resource_calls);

        return Success;
    }

    std::pair<const type::Type*, ir::Value*> GetResourceTableCallInfo(
        core::ir::CoreBuiltinCall* call) {
        TINT_IR_ASSERT(
            ir, std::holds_alternative<const core::type::Type*>(call->ExplicitTemplateParams()[0]));
        const type::Type* binding_ty =
            std::get<const core::type::Type*>(call->ExplicitTemplateParams()[0]);
        ir::Value* idx = b.InsertConvertIfNeeded(ty.u32(), call->Args()[0]);
        return {binding_ty, idx};
    }

    void ReplaceHasResourceCalls(const Vector<core::ir::CoreBuiltinCall*, 8>& calls) {
        for (auto* call : calls) {
            b.InsertBefore(call, [&] {
                auto [binding_type, idx] = GetResourceTableCallInfo(call);
                GenHasResource(call->DetachResult(), binding_type, idx);
            });
            call->Destroy();
        }
    }

    void ReplaceGetResourceCalls(const Vector<core::ir::CoreBuiltinCall*, 8>& calls) {
        for (auto* call : calls) {
            b.InsertBefore(call, [&] {
                auto [binding_type, idx] = GetResourceTableCallInfo(call);
                for (auto& usage : call->Result()->UsagesSorted()) {
                    ReplaceGetResourceCallUsage(usage, binding_type, idx);
                }
            });
        }

        // We should have gathered the potential texture/sampler pairs needed for the call so we can
        // now replace all the sampled calls.
        for (auto& entry : sampled_call_replacements) {
            GenSampledGetResource(entry.key, entry.value);
        }

        // Destroy all known getResource calls, they should have been replaced above
        for (auto* call : calls) {
            TINT_IR_ASSERT(ir, call->Result()->UsagesUnsorted().IsEmpty());
            call->Destroy();
        }
    }

    void ReplaceGetResourceCallUsage(const Usage& usage,
                                     const type::Type* binding_type,
                                     ir::Value* idx) {
        auto* call = usage.instruction->As<ir::CoreBuiltinCall>();
        TINT_IR_ASSERT(ir, call);

        switch (call->Func()) {
            case core::BuiltinFn::kTextureStore:
            case core::BuiltinFn::kTextureLoad:
            case core::BuiltinFn::kTextureDimensions:
            case core::BuiltinFn::kTextureNumLayers:
            case core::BuiltinFn::kTextureNumLevels:
            case core::BuiltinFn::kTextureNumSamples: {
                // Each of these usages just require the texture so they can be replaced
                // immediately, there is no extra validation which needs to happen.

                auto* has_result = b.InstructionResult(ty.bool_());
                GenHasResource(has_result, binding_type, idx);

                ir::InstructionResult* res =
                    GenValidateAndGetFromResourceTable(has_result, binding_type, idx);
                call->SetOperand(usage.operand_index, res);
                break;
            }
            case core::BuiltinFn::kTextureGather:
            case core::BuiltinFn::kTextureGatherCompare:
            case core::BuiltinFn::kTextureSample:
            case core::BuiltinFn::kTextureSampleBias:
            case core::BuiltinFn::kTextureSampleCompare:
            case core::BuiltinFn::kTextureSampleCompareLevel:
            case core::BuiltinFn::kTextureSampleGrad:
            case core::BuiltinFn::kTextureSampleLevel:
            case core::BuiltinFn::kTextureSampleBaseClampToEdge: {
                // These calls are all combined with a sampler, this means we can't do the checks at
                // this point as the sampler may also be a getResource call. So, store the usage
                // away and continue.
                Info call_info{
                    .binding_type = binding_type,
                    .slot_idx = idx,
                    .operand_idx = usage.operand_index,
                };

                auto& info = sampled_call_replacements.GetOrAddZeroEntry(call);
                if (binding_type->Is<core::type::Texture>()) {
                    info.value.texture = call_info;
                } else {
                    info.value.sampler = call_info;
                }
                break;
            }
            default: {
                TINT_IR_UNREACHABLE(ir) << "unknown builtin function: " << call->Func();
            }
        }
    }

    // Get the type id from the metadata buffer
    core::ir::InstructionResult* GetTypeId(core::ir::Value* idx) {
        auto* metadata_access = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 1_u, idx);
        return b.Load(metadata_access)->Result();
    }

    bool IsSampler(const core::type::Type* binding_type) const {
        auto res_type = core::type::TypeToResourceType(binding_type);
        return tint::IsSampler(res_type);
    }

    // Note, assumes it's called inside a builder append block.
    void GenHasResource(core::ir::InstructionResult* result,
                        const core::type::Type* binding_type,
                        core::ir::Value* idx) {
        // Get the table's API size, which is stored at index 0 in the metadata buffer
        auto* length = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 0_u);
        ir.SetName(length, "tint_storage_metadata_length");

        auto* len_check = b.LessThan(idx, b.Load(length));
        auto* has_check = b.If(len_check);
        has_check->SetResult(result);

        b.Append(has_check->False(), [&] { b.ExitIf(has_check, b.Constant(false)); });
        b.Append(has_check->True(), [&] {
            auto* metadata_val = GetTypeId(idx);

            ir::Value* type_id = nullptr;
            if (config->get_sampler_index_from_metadata && IsSampler(binding_type)) {
                // Type id is in lower 16 bits
                type_id = b.And(metadata_val, u32(0xFFFF))->Result();
            } else {
                type_id = metadata_val;
            }

            core::ir::Value* eq = nullptr;
            std::vector<ResourceType> conv = core::type::ConvertsFrom(binding_type);
            if (!conv.empty()) {
                auto* conv_ty = ty.vec(ty.u32(), static_cast<uint32_t>(conv.size()));
                auto* lhs = b.Construct(conv_ty, type_id);

                Vector<Value*, 4> vals;
                for (auto& r : conv) {
                    vals.Push(b.Value(u32(r)));
                }

                auto* rhs = b.Construct(conv_ty, vals);
                auto* cmp = b.Equal(lhs, rhs);
                eq = b.Call(ty.bool_(), core::BuiltinFn::kAny, cmp)->Result();
            } else {
                ResourceType resource_ty = core::type::TypeToResourceType(binding_type);
                eq = b.Equal(type_id, u32(resource_ty))->Result();
            }
            b.ExitIf(has_check, eq);
        });
    }

    // Note, assumes it's called inside a builder append block.
    ir::InstructionResult* GenValidateAndGetFromResourceTable(core::ir::Value* has_result,
                                                              const core::type::Type* binding_type,
                                                              core::ir::Value* idx) {
        auto* get_check = b.If(has_result);
        auto* res = b.InstructionResult(ty.u32());
        get_check->SetResult(res);
        ir.SetName(get_check, "item_idx");

        // Table lookup succeeded, use the input index
        b.Append(get_check->True(), [&] { b.ExitIf(get_check, idx); });

        // Table lookup failed, so get default resource located at the end of the table,
        // at API size + resource type index.
        //
        // For textures which can be filterable, the default will be the filterable variant.
        // Otherwise it will be unfilterable.
        b.Append(get_check->False(), [&] {
            ir::Value* r =
                GetDefaultIndexForResourceType(core::type::DefaultResourceTypeFor(binding_type));
            b.ExitIf(get_check, r);
        });

        return GenGetResource(res, binding_type)->Result();
    }

    // We have a bind-ful texture/sampler, get the ResourceKind the API reported
    ir::Value* GetBindfulKind(ir::CoreBuiltinCall* call, size_t idx) {
        auto* operand_inst_result = call->Operands()[idx]->As<core::ir::InstructionResult>();
        TINT_IR_ASSERT(ir, operand_inst_result);

        core::ir::Var* var = RootVarFor(operand_inst_result);
        TINT_IR_ASSERT(ir, var);

        auto iter = config->binding_to_resource_type.find(var->BindingPoint().value());
        TINT_IR_ASSERT(ir, iter != config->binding_to_resource_type.end());

        return b.Constant(u32(iter->second));
    }

    // Returns the root Var for `value` by walking up the chain of instructions, or nullptr if none
    // is found.
    Var* RootVarFor(Value* value) {
        Var* result = nullptr;
        while (value) {
            TINT_IR_ASSERT(ir, value->Alive());

            auto* res = value->As<core::ir::InstructionResult>();
            TINT_IR_ASSERT(ir, res);

            // value was emitted by an instruction
            ir::Instruction* inst = res->Instruction();
            value = tint::Switch(
                inst,
                [&](Load* l) {
                    auto* from = l->From()->As<core::ir::InstructionResult>();
                    TINT_IR_ASSERT(ir, from);

                    result = from->Instruction()->As<core::ir::Var>();  // Final var to return
                    TINT_IR_ASSERT(ir, result);

                    return nullptr;  // Returns from switch.
                },
                [&](Var* var) {
                    result = var;    // Final var to return
                    return nullptr;  // Returns from switch.
                },
                TINT_ICE_ON_NO_MATCH);
        }
        return result;
    }

    // We have a bind-less texture/sampler, we need to verify the ResourceKind matches what we need
    // and substitute in a default if needed.
    ir::InstructionResult* GetBindlessKind(ir::CoreBuiltinCall* call,
                                           const Info& info,
                                           bool is_sampler_comparison_call,
                                           std::string_view name) {
        ir::InstructionResult* resource_kind = nullptr;
        b.InsertBefore(call, [&] {
            // Determine if the slot kind matches the type in WGSL
            auto* has_result = b.InstructionResult(ty.bool_());
            ir.SetName(has_result, "has_resource");
            GenHasResource(has_result, info.binding_type, info.slot_idx);

            // If this isn't a comparison sampler then we need to get the default resource type. If
            // this is a comparison sampler, then we'll skip the texture/sampler combination checks
            // and we just need to know the resource table slot is valid.
            if (!is_sampler_comparison_call) {
                // Get the ResourceKind for the resource
                core::ir::If* if_ = b.If(has_result);
                resource_kind = b.InstructionResult(ty.u32());
                ir.SetName(resource_kind, "resource_kind");
                if_->SetResult(resource_kind);

                b.Append(if_->True(), [&] {  //
                    b.ExitIf(if_, GetTypeId(info.slot_idx));
                });
                b.Append(if_->False(), [&] {
                    ResourceType res_type = core::type::DefaultResourceTypeFor(info.binding_type);
                    b.ExitIf(if_, u32(res_type));
                });
                ir.SetName(resource_kind, name);
            }

            // Get the resource from the table and update the argument
            ir::InstructionResult* res =
                GenValidateAndGetFromResourceTable(has_result, info.binding_type, info.slot_idx);

            call->SetOperand(info.operand_idx, res);
        });
        return resource_kind;
    }

    ir::Value* GetKind(ir::CoreBuiltinCall* call,
                       const std::optional<Info>& opt,
                       bool is_sampler_comparison_call,
                       size_t idx,
                       std::string_view name) {
        core::ir::Value* kind = nullptr;
        if (opt.has_value()) {
            kind = GetBindlessKind(call, opt.value(), is_sampler_comparison_call, name);
        } else if (!is_sampler_comparison_call) {
            kind = GetBindfulKind(call, idx);
        }
        return kind;
    }

    // Returns a `true` constant if the texture is filterable
    ir::Value* ConstructTextureFilterableCheck(const type::Type* tex_ty, ir::Value* texture_kind) {
        const auto* samp_ty = tex_ty->As<type::SampledTexture>();
        // Only sampled texture types can be filterable
        if (!samp_ty) {
            return b.Constant(false);
        }
        // Only floating point sampled textures can be filterable
        if (samp_ty->Type()->IsAnyOf<type::I32, type::U32>()) {
            return b.Constant(false);
        }

        // The default resource type for all f32 sampled textures is `filterable`
        ResourceType res_type = core::type::DefaultResourceTypeFor(tex_ty);
        if (texture_kind->Is<ir::Constant>()) {
            auto val = texture_kind->As<ir::Constant>()->Value()->ValueAs<uint32_t>();
            return b.Constant(val == uint32_t(res_type));
        }
        return b.Equal(texture_kind, u32(res_type))->Result();
    }

    // Generates the code to access the item at `idx` from the resource table. Does not do any
    // validation of the types, just gets the resource. For samplers, will handle the
    // `get_sampler_index_from_metadata` flag.
    ir::Instruction* GenGetResource(ir::Value* idx, const type::Type* binding_type) {
        if (config->get_sampler_index_from_metadata && IsSampler(binding_type)) {
            // Get the sampler index from the metadata entry (high 16 bits)
            // TODO(crbug.com/503755700): Optimize to avoid loading twice from
            // storage_buffer[idx].
            auto* metadata_val = GetTypeId(idx);
            auto* sampler_index = b.ShiftRight(metadata_val, u32(16));
            idx = sampler_index->Result();
        }

        auto var = var_for_type.Get(binding_type);
        TINT_IR_ASSERT(ir, var);

        const core::type::Pointer* ptr_ty = ty.ptr(handle, binding_type, read);
        auto* access = b.Access(ptr_ty, (*var)->Result(), idx);
        return b.Load(access);
    }

    ir::Value* GetDefaultIndexForResourceType(ResourceType resource_type) {
        auto idx_iter = resource_type_to_default_idx.find(resource_type);
        TINT_IR_ASSERT(ir, idx_iter != resource_type_to_default_idx.end());

        // Get the table's API size, which is stored at index 0 in the metadata buffer
        auto* len_access = b.Access(ty.ptr<storage, u32, read>(), storage_buffer, 0_u);
        ir::Value* num_elements = b.Load(len_access)->Result();

        return b.Add(u32(idx_iter->second), num_elements)->Result();
    }

    void GenSampledGetResource(ir::CoreBuiltinCall* call, const CallArgs& args) {
        // we can get the texture, or the sampler, or both.
        TINT_IR_ASSERT(ir, args.texture.has_value() || args.sampler.has_value());

        bool is_sampler_comparison_call =
            call->Func() == core::BuiltinFn::kTextureSampleCompare ||
            call->Func() == core::BuiltinFn::kTextureSampleCompareLevel ||
            call->Func() == core::BuiltinFn::kTextureGatherCompare;

        // Pre-calculate the operand indices. We know we have at least one of these, so if the
        // needed optional isn't set we can always use the other optional to calculate the index.
        size_t texture_operand_idx =
            args.texture.has_value() ? args.texture->operand_idx : args.sampler->operand_idx - 1;
        size_t sampler_operand_idx =
            args.sampler.has_value() ? args.sampler->operand_idx : args.texture->operand_idx + 1;

        core::ir::Value* texture_kind = GetKind(call, args.texture, is_sampler_comparison_call,
                                                texture_operand_idx, "texture_kind");
        core::ir::Value* sampler_kind = GetKind(call, args.sampler, is_sampler_comparison_call,
                                                sampler_operand_idx, "sampler_kind");

        // Only need to validate the texture/sampler if we aren't don't a comparison sampler. The
        // validation is done in the `GetKind` calls.
        if (is_sampler_comparison_call) {
            return;
        }

        // If we know the sampler type at compile time, we can skip all the if checks below if
        // we know it isn't filtering.
        if (sampler_kind->Is<core::ir::Constant>() &&
            sampler_kind->As<core::ir::Constant>()->Value()->ValueAs<uint32_t>() ==
                uint32_t(ResourceType::kSampler_non_filtering)) {
            return;
        }

        ir::Value* tex = call->Args()[texture_operand_idx];

        // Validate that the sampler/texture pair work together
        b.InsertBefore(call, [&] {
            core::ir::Value* samp_res = nullptr;
            if (!sampler_kind->Is<core::ir::Constant>()) {
                // The sampler is bind-less so we need to lookup the kind

                ir::Value* tex_res = ConstructTextureFilterableCheck(tex->Type(), texture_kind);
                // If the texture filterable check returns constant true we can just return true as
                // both branches of the if will be the same
                if (tex_res->Is<ir::Constant>() &&
                    tex_res->As<ir::Constant>()->Value()->ValueAs<bool>() == true) {
                    samp_res = tex_res;
                } else {
                    core::ir::Instruction* sampler_compare =
                        b.Equal(sampler_kind, u32(ResourceType::kSampler_filtering));

                    // Returns `true` if  the texture/sampler combination is valid.
                    core::ir::If* samp_if = b.If(sampler_compare);
                    samp_res = b.InstructionResult(ty.bool_());
                    ir.SetName(samp_res, "texture_sampler_combo_valid");
                    samp_if->SetResult(samp_res->As<ir::InstructionResult>());

                    // If the sampler is filtering
                    b.Append(samp_if->True(), [&] { b.ExitIf(samp_if, tex_res); });

                    // Sampler != filtering, so texture_sampler_combo_valid is true
                    b.Append(samp_if->False(), [&] { b.ExitIf(samp_if, true); });
                }
            } else {
                // Sampler is bind-ful which means the sampler is `filtering` due to the early
                // return above.

                samp_res = ConstructTextureFilterableCheck(tex->Type(), texture_kind);
            }

            const core::type::Type* result_ty = call->Result()->Type();

            // If the samp_res is statically known we can just pick the right branch
            if (auto* res = samp_res->As<ir::Constant>()) {
                if (res->Value()->ValueAs<bool>() == false) {
                    call->Result()->ReplaceAllUsesWith(b.Zero(result_ty));
                    call->Destroy();
                }
                // In the case where it's true, we just leave the call alone
            } else {
                // Branch over if we can use the sampler or not, the `if` returns the result of the
                // `call` we're attempting to make
                core::ir::If* check = b.If(samp_res);
                check->SetResult(call->DetachResult());

                // Sampler and texture matched, just call
                b.Append(check->True(), [&] {
                    core::ir::Call* c = b.Call(result_ty, call->Func());
                    for (ir::Value* arg : call->Args()) {
                        c->AppendArg(arg);
                    }
                    b.ExitIf(check, c);
                });

                // Sampler and texture mismatch, return the 0 value
                b.Append(check->False(), [&] { b.ExitIf(check, b.Zero(result_ty)); });
                call->Destroy();
            }
        });
    }

    void InjectRootBlockEntries() {
        b.Append(ir.root_block, [&] {
            // Any needed vars to hold the resource table bindings (e.g. in SPIR-V this turns in a
            // var per type.)
            var_for_type = helper->GenerateVars(b, config->resource_table_binding,
                                                config->default_binding_type_order);

            // Create the metadata table for resource type information
            auto* str = ty.Struct(ir.symbols.New("tint_resource_table_metadata_struct"),
                                  Vector<core::type::Manager::StructMemberDesc, 2>{
                                      {ir.symbols.New("array_length"), ty.u32()},
                                      {ir.symbols.New("bindings"), ty.array<u32>()},
                                  });
            auto* sb_ty = ty.ptr(storage, str, read);

            storage_buffer = b.Var("tint_resource_table_metadata", sb_ty);
            storage_buffer->SetBindingPoint(config->storage_buffer_binding.group,
                                            config->storage_buffer_binding.binding);
        });
        TINT_IR_ASSERT(ir, storage_buffer != nullptr);
    }

    void CalculateDefaultBindingIndices() {
        for (size_t i = 0; i < config->default_binding_type_order.size(); ++i) {
            auto res_ty = static_cast<ResourceType>(config->default_binding_type_order[i]);
            resource_type_to_default_idx.insert({res_ty, static_cast<uint32_t>(i)});
        }
    }
};

}  // namespace

ResourceTableHelper::~ResourceTableHelper() = default;

Result<SuccessType> ResourceTable(core::ir::Module& ir,
                                  const std::optional<ResourceTableConfig>& config,
                                  ResourceTableHelper* helper) {
    AssertValid(ir, "before core.ResourceTable");

    return State{config, ir, helper}.Process();
}

}  // namespace tint::core::ir::transform
