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

#include "src/tint/lang/hlsl/writer/raise/shader_io.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/shader_io.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {

namespace {

/// PIMPL state for the parts of the shader IO transform specific to HLSL.
/// For HLSL, move all inputs to a struct passed as an entry point parameter, and wrap outputs in
/// a structure returned by the entry point.
struct StateImpl : core::ir::transform::ShaderIOBackendState {
    /// The config
    const ShaderIOConfig& config;

    /// The input parameter
    core::ir::FunctionParam* input_param = nullptr;

    Vector<uint32_t, 4> input_indices;
    Vector<uint32_t, 4> output_indices;

    /// The output struct type.
    core::type::Struct* output_struct = nullptr;

    /// The output values to return from the entry point.
    Vector<core::ir::Value*, 4> output_values;

    // Indices of inputs that require special handling
    std::optional<uint32_t> subgroup_invocation_id_index;
    std::optional<uint32_t> subgroup_size_index;
    std::optional<uint32_t> num_workgroups_index;
    std::optional<uint32_t> first_clip_distance_index;
    std::optional<uint32_t> second_clip_distance_index;
    Hashset<uint32_t, 4> truncated_indices;

    // If set, points to a var of type struct with fields for offsets to apply to vertex_index and
    // instance_index
    core::ir::Var* tint_first_index_offset = nullptr;

    /// Constructor
    StateImpl(core::ir::Module& mod, core::ir::Function* f, const ShaderIOConfig& c)
        : ShaderIOBackendState(mod, f), config(c) {}

    /// Destructor
    ~StateImpl() override {}

    /// FXC is sensitive to field order in structures, this is used by StructMemberComparator to
    /// ensure that FXC is happy with the order of emitted fields.
    uint32_t BuiltinOrder(core::BuiltinValue builtin) {
        switch (builtin) {
            case core::BuiltinValue::kPosition:
                return 1;
            case core::BuiltinValue::kVertexIndex:
                return 2;
            case core::BuiltinValue::kInstanceIndex:
                return 3;
            case core::BuiltinValue::kFrontFacing:
                return 4;
            case core::BuiltinValue::kFragDepth:
                return 5;
            case core::BuiltinValue::kLocalInvocationId:
                return 6;
            case core::BuiltinValue::kLocalInvocationIndex:
                return 7;
            case core::BuiltinValue::kGlobalInvocationId:
                return 8;
            case core::BuiltinValue::kWorkgroupId:
                return 9;
            case core::BuiltinValue::kNumWorkgroups:
                return 10;
            case core::BuiltinValue::kSampleIndex:
                return 11;
            case core::BuiltinValue::kSampleMask:
                return 12;
            case core::BuiltinValue::kPointSize:
                return 13;
            case core::BuiltinValue::kClipDistances:
                return 14;
            case core::BuiltinValue::kPrimitiveId:
                return 15;
            case core::BuiltinValue::kBarycentricCoord:
                return 16;
            case core::BuiltinValue::kSubgroupInvocationId:
            case core::BuiltinValue::kSubgroupSize:
                // These are sorted, but don't actually end up as members. Value doesn't really
                // matter, so just make it larger than the rest.
                return std::numeric_limits<uint32_t>::max();
            default:
                break;
        }
        TINT_UNREACHABLE() << "Unhandled builtin value: " << ToString(builtin);
    }

    struct MemberInfo {
        core::type::Manager::StructMemberDesc member;
        uint32_t idx;
    };

    /// Comparison function used to reorder struct members such that all members with
    /// color attributes appear first (ordered by color slot), then location attributes (ordered by
    /// location slot), then blend_src attributes (ordered by blend_src slot), followed by those
    /// with builtin attributes (ordered by BuiltinOrder).
    /// @param x a struct member
    /// @param y another struct member
    /// @returns true if a comes before b
    bool StructMemberComparator(const MemberInfo& x, const MemberInfo& y) {
        if (x.member.attributes.color.has_value() && y.member.attributes.color.has_value() &&
            x.member.attributes.color != y.member.attributes.color) {
            // Both have color attributes: smallest goes first.
            return x.member.attributes.color < y.member.attributes.color;
        } else if (x.member.attributes.color.has_value() != y.member.attributes.color.has_value()) {
            // The member with the color goes first
            return x.member.attributes.color.has_value();
        }

        if (x.member.attributes.location.has_value() && y.member.attributes.location.has_value() &&
            x.member.attributes.location != y.member.attributes.location) {
            // Both have location attributes: smallest goes first.
            return x.member.attributes.location < y.member.attributes.location;
        } else if (x.member.attributes.location.has_value() !=
                   y.member.attributes.location.has_value()) {
            // The member with the location goes first
            return x.member.attributes.location.has_value();
        }

        if (x.member.attributes.blend_src.has_value() &&
            y.member.attributes.blend_src.has_value() &&
            x.member.attributes.blend_src != y.member.attributes.blend_src) {
            // Both have blend_src attributes: smallest goes first.
            return x.member.attributes.blend_src < y.member.attributes.blend_src;
        } else if (x.member.attributes.blend_src.has_value() !=
                   y.member.attributes.blend_src.has_value()) {
            // The member with the blend_src goes first
            return x.member.attributes.blend_src.has_value();
        }

        auto x_blt = x.member.attributes.builtin;
        auto y_blt = y.member.attributes.builtin;
        if (x_blt.has_value() && y_blt.has_value()) {
            // Both are builtins: order matters for FXC.
            auto order_a = BuiltinOrder(*x_blt);
            auto order_b = BuiltinOrder(*y_blt);
            if (order_a != order_b) {
                return order_a < order_b;
            }
        } else if (x_blt.has_value() != y_blt.has_value()) {
            // The member with the builtin goes first
            return x_blt.has_value();
        }

        // Control flow reaches here if x is the same as y.
        // Sort algorithms sometimes do that.
        return false;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    Vector<core::ir::FunctionParam*, 4> FinalizeInputs() override {
        if (config.add_input_position_member) {
            const bool has_position_member = inputs.Any([](auto& struct_mem_desc) {
                return struct_mem_desc.attributes.builtin == core::BuiltinValue::kPosition;
            });
            if (!has_position_member) {
                core::IOAttributes attrs;
                attrs.builtin = core::BuiltinValue::kPosition;
                AddInput(ir.symbols.New("pos"), ty.vec4<f32>(), attrs);
            }
        }

        Vector<MemberInfo, 4> input_data;
        bool has_vertex_or_instance_index = false;
        for (uint32_t i = 0; i < inputs.Length(); ++i) {
            // Save the index of certain builtins for GetIndex. Although struct members will not be
            // added for these inputs, we still add entries to input_data so that other inputs with
            // struct members can index input_indices properly in GetIndex.
            if (auto builtin = inputs[i].attributes.builtin) {
                if (*builtin == core::BuiltinValue::kSubgroupInvocationId) {
                    subgroup_invocation_id_index = i;
                } else if (*builtin == core::BuiltinValue::kSubgroupSize) {
                    subgroup_size_index = i;
                } else if (*builtin == core::BuiltinValue::kNumWorkgroups) {
                    num_workgroups_index = i;
                } else if (*builtin == core::BuiltinValue::kVertexIndex) {
                    has_vertex_or_instance_index = true;
                } else if (*builtin == core::BuiltinValue::kInstanceIndex) {
                    has_vertex_or_instance_index = true;
                }
            }

            input_data.Push(MemberInfo{inputs[i], i});
        }

        input_indices.Resize(input_data.Length());

        if (config.first_index_offset_binding.has_value() && has_vertex_or_instance_index) {
            // Create a FirstIndexOffset uniform buffer. GetInput will use this to offset the
            // vertex/instance index.
            TINT_ASSERT(func->IsVertex());
            tint::Vector<tint::core::type::Manager::StructMemberDesc, 2> members;
            auto* str = ty.Struct(ir.symbols.New("tint_first_index_offset_struct"),
                                  {
                                      {ir.symbols.New("vertex_index"), ty.u32(), {}},
                                      {ir.symbols.New("instance_index"), ty.u32(), {}},
                                  });
            tint_first_index_offset = b.Var("tint_first_index_offset", uniform, str);
            tint_first_index_offset->SetBindingPoint(config.first_index_offset_binding->group,
                                                     config.first_index_offset_binding->binding);
            ir.root_block->Append(tint_first_index_offset);
        }

        // Sort the struct members to satisfy HLSL interfacing matching rules.
        // We use stable_sort so that two members with the same attributes maintain their relative
        // ordering (e.g. kClipDistance).
        std::stable_sort(input_data.begin(), input_data.end(),
                         [&](auto& x, auto& y) { return StructMemberComparator(x, y); });

        Vector<core::type::Manager::StructMemberDesc, 4> input_struct_members;
        for (auto& input : input_data) {
            // Don't add members for certain builtins
            if (input.idx == subgroup_invocation_id_index ||  //
                input.idx == subgroup_size_index ||           //
                input.idx == num_workgroups_index) {
                // Invalid value, should not be indexed
                input_indices[input.idx] = std::numeric_limits<uint32_t>::max();
                continue;
            }
            input_indices[input.idx] = static_cast<uint32_t>(input_struct_members.Length());
            input_struct_members.Push(input.member);
        }

        if (!input_struct_members.IsEmpty()) {
            auto* input_struct = ty.Struct(ir.symbols.New(ir.NameOf(func).Name() + "_inputs"),
                                           std::move(input_struct_members));
            switch (func->Stage()) {
                case core::ir::Function::PipelineStage::kFragment:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kFragmentInput);
                    break;
                case core::ir::Function::PipelineStage::kVertex:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kVertexInput);
                    break;
                case core::ir::Function::PipelineStage::kCompute:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kComputeInput);
                    break;
                case core::ir::Function::PipelineStage::kUndefined:
                    TINT_UNREACHABLE();
            }
            input_param = b.FunctionParam("inputs", input_struct);
            return {input_param};
        }

        return tint::Empty;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    const core::type::Type* FinalizeOutputs() override {
        if (outputs.IsEmpty()) {
            return ty.void_();
        }

        // If a clip_distances output is found, replace it with either one or two new outputs,
        // depending on the array size. The new outputs maintain the ClipDistance attribute, which
        // is translated to SV_ClipDistanceN in the printer.
        for (uint32_t i = 0; i < outputs.Length(); ++i) {
            if (outputs[i].attributes.builtin == core::BuiltinValue::kClipDistances) {
                auto* const type = outputs[i].type;
                auto const name = outputs[i].name;
                auto const attributes = outputs[i].attributes;
                // Compute new member element counts
                auto* arr = type->As<core::type::Array>();
                uint32_t arr_count = *arr->ConstantCount();
                TINT_ASSERT(arr_count >= 1 && arr_count <= 8);
                uint32_t count0, count1;
                if (arr_count >= 4) {
                    count0 = 4;
                    count1 = arr_count - 4;
                } else {
                    count0 = arr_count;
                    count1 = 0;
                }
                // Replace current output for the first one
                auto* ty0 = ty.MatchWidth(ty.f32(), count0);
                auto name0 = ir.symbols.New(name.Name() + std::to_string(0));
                outputs[i] = {name0, ty0, attributes};
                first_clip_distance_index = i;
                // And add a new output for the second, if any
                if (count1 > 0) {
                    auto* ty1 = ty.MatchWidth(ty.f32(), count1);
                    auto name1 = ir.symbols.New(name.Name() + std::to_string(1));
                    second_clip_distance_index = AddOutput(name1, ty1, attributes);
                }
                break;
            }
        }

        Vector<MemberInfo, 4> output_data;
        for (uint32_t i = 0; i < outputs.Length(); ++i) {
            output_data.Push(MemberInfo{outputs[i], i});
        }

        // Sort the struct members to satisfy HLSL interfacing matching rules.
        // We use stable_sort so that two members with the same attributes maintain their relative
        // ordering (e.g. kClipDistance).
        std::stable_sort(output_data.begin(), output_data.end(),
                         [&](auto& x, auto& y) { return StructMemberComparator(x, y); });

        output_indices.Resize(outputs.Length());
        output_values.Resize(outputs.Length(), nullptr);

        Vector<core::type::Manager::StructMemberDesc, 4> output_struct_members;
        for (size_t i = 0; i < output_data.Length(); ++i) {
            output_indices[output_data[i].idx] = static_cast<uint32_t>(i);

            // If we need to truncate this member, don't add it to the output struct
            if (config.truncate_interstage_variables) {
                if (auto loc = output_data[i].member.attributes.location) {
                    if (!config.interstage_locations.test(*loc)) {
                        truncated_indices.Add(output_data[i].idx);
                        continue;
                    }
                }
            }

            output_struct_members.Push(output_data[i].member);
        }
        if (output_struct_members.IsEmpty()) {
            // All members were truncated
            return ty.void_();
        }

        output_struct =
            ty.Struct(ir.symbols.New(ir.NameOf(func).Name() + "_outputs"), output_struct_members);
        switch (func->Stage()) {
            case core::ir::Function::PipelineStage::kFragment:
                output_struct->AddUsage(core::type::PipelineStageUsage::kFragmentOutput);
                break;
            case core::ir::Function::PipelineStage::kVertex:
                output_struct->AddUsage(core::type::PipelineStageUsage::kVertexOutput);
                break;
            case core::ir::Function::PipelineStage::kCompute:
                output_struct->AddUsage(core::type::PipelineStageUsage::kComputeOutput);
                break;
            case core::ir::Function::PipelineStage::kUndefined:
                TINT_UNREACHABLE();
        }
        return output_struct;
    }

    /// Handles kNumWorkgroups builtin by emitting a UBO to hold the num_workgroups value,
    /// along with the load of the value. Returns the loaded value.
    core::ir::Value* GetInputForNumWorkgroups(core::ir::Builder& builder) {
        // Create uniform var that will receive the number of workgroups
        core::ir::Var* num_wg_var = nullptr;
        builder.Append(ir.root_block, [&] {
            num_wg_var = builder.Var("tint_num_workgroups", ty.ptr(uniform, ty.vec3<u32>()));
        });
        if (config.num_workgroups_binding.has_value()) {
            // If config.num_workgroups_binding holds a value, use it.
            auto bp = *config.num_workgroups_binding;
            num_wg_var->SetBindingPoint(bp.group, bp.binding);
        } else {
            // Otherwise, use the binding 0 of the largest used group plus 1, or group 0 if no
            // resources are bound.
            uint32_t group = 0;
            for (auto* inst : *ir.root_block.Get()) {
                if (auto* var = inst->As<core::ir::Var>()) {
                    if (const auto& bp = var->BindingPoint()) {
                        if (bp->group >= group) {
                            group = bp->group + 1;
                        }
                    }
                }
            }
            num_wg_var->SetBindingPoint(group, 0);
        }
        auto* load = builder.Load(num_wg_var);
        return load->Result();
    }

    /// @copydoc ShaderIO::BackendState::GetInput
    core::ir::Value* GetInput(core::ir::Builder& builder, uint32_t idx) override {
        if (subgroup_invocation_id_index == idx) {
            return builder
                .Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kWaveGetLaneIndex)
                ->Result();
        }
        if (subgroup_size_index == idx) {
            return builder
                .Call<hlsl::ir::BuiltinCall>(ty.u32(), hlsl::BuiltinFn::kWaveGetLaneCount)
                ->Result();
        }
        if (num_workgroups_index == idx) {
            if (config.num_workgroups_start_offset.has_value()) {
                auto* immediate_data = config.immediate_data_layout.var;
                auto num_workgroup_idx = u32(config.immediate_data_layout.IndexOf(
                    config.num_workgroups_start_offset.value()));
                auto* load = builder.Load(
                    builder.Access<ptr<immediate, vec3<u32>>>(immediate_data, num_workgroup_idx));
                return load->Result();
            }
            return GetInputForNumWorkgroups(builder);
        }

        auto index = input_indices[idx];

        core::ir::Value* v = builder.Access(inputs[idx].type, input_param, u32(index))->Result();

        if (inputs[idx].attributes.builtin == core::BuiltinValue::kPosition) {
            // If this is an input position builtin we need to invert the 'w' component of the
            // vector.
            auto* w = builder.Access(ty.f32(), v, 3_u);
            auto* div = builder.Divide(ty.f32(), 1.0_f, w);
            auto* swizzle = builder.Swizzle(ty.vec3<f32>(), v, {0, 1, 2});
            v = builder.Construct(ty.vec4<f32>(), swizzle, div)->Result();
        } else if (config.first_index_offset_binding.has_value() &&
                   inputs[idx].attributes.builtin == core::BuiltinValue::kVertexIndex) {
            // Apply vertex_index offset
            TINT_ASSERT(tint_first_index_offset);
            auto* vertex_index_offset =
                builder.Access(ty.ptr<uniform, u32>(), tint_first_index_offset, 0_u);
            v = builder.Add<u32>(v, builder.Load(vertex_index_offset))->Result();
        } else if (config.first_index_offset_binding.has_value() &&
                   inputs[idx].attributes.builtin == core::BuiltinValue::kInstanceIndex) {
            // Apply instance_index offset
            TINT_ASSERT(tint_first_index_offset);
            auto* instance_index_offset =
                builder.Access(ty.ptr<uniform, u32>(), tint_first_index_offset, 1_u);
            v = builder.Add<u32>(v, builder.Load(instance_index_offset))->Result();
        } else if (config.first_index_offset.has_value() &&
                   inputs[idx].attributes.builtin == core::BuiltinValue::kVertexIndex) {
            auto* immediate_data = config.immediate_data_layout.var;
            auto first_index_offset_idx =
                u32(config.immediate_data_layout.IndexOf(config.first_index_offset.value()));
            auto first_index_offset =
                builder.Access<ptr<immediate, u32>>(immediate_data, first_index_offset_idx);
            v = builder.Add<u32>(v, builder.Load(first_index_offset))->Result();
        } else if (config.first_instance_offset.has_value() &&
                   inputs[idx].attributes.builtin == core::BuiltinValue::kInstanceIndex) {
            auto* immediate_data = config.immediate_data_layout.var;
            auto first_instance_offset_idx =
                u32(config.immediate_data_layout.IndexOf(config.first_instance_offset.value()));
            auto first_instance_offset =
                builder.Access<ptr<immediate, u32>>(immediate_data, first_instance_offset_idx);
            v = builder.Add<u32>(v, builder.Load(first_instance_offset))->Result();
        }

        return v;
    }

    core::ir::Value* BuildClipDistanceInitValue(core::ir::Builder& builder,
                                                uint32_t output_index,
                                                core::ir::Value* src_array,
                                                uint32_t src_array_first_index) {
        // Copy from the array `src_array`
        auto* src_array_ty = src_array->Type()->As<core::type::Array>();
        TINT_ASSERT(src_array_ty);

        core::ir::Value* dst_value;
        if (auto* dst_vec_ty = outputs[output_index].type->As<core::type::Vector>()) {
            // Create a vector and copy array elements to it
            Vector<core::ir::Value*, 4> init;
            for (size_t i = 0; i < dst_vec_ty->Elements().count; ++i) {
                init.Push(builder.Access<f32>(src_array, u32(src_array_first_index + i))->Result());
            }
            dst_value = builder.Construct(dst_vec_ty, std::move(init))->Result();
        } else {
            TINT_ASSERT(outputs[output_index].type->As<core::type::Scalar>());
            dst_value = builder.Access<f32>(src_array, u32(src_array_first_index))->Result();
        }
        return dst_value;
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(core::ir::Builder& builder, uint32_t idx, core::ir::Value* value) override {
        if (truncated_indices.Contains(idx)) {
            // Leave this output value as nullptr
            TINT_ASSERT(!output_values[output_indices[idx]]);
            return;
        }

        // If setting a ClipDistance output, build the initial value for one or both output values.
        if (idx == first_clip_distance_index) {
            auto index = output_indices[idx];
            output_values[index] = BuildClipDistanceInitValue(builder, idx, value, 0);

            if (second_clip_distance_index) {
                index = output_indices[*second_clip_distance_index];
                output_values[index] =
                    BuildClipDistanceInitValue(builder, *second_clip_distance_index, value, 4);
            }
            return;
        }

        auto index = output_indices[idx];
        output_values[index] = value;
    }

    /// @copydoc ShaderIO::BackendState::MakeReturnValue
    core::ir::Value* MakeReturnValue(core::ir::Builder& builder) override {
        if (!output_struct) {
            return nullptr;
        }

        if (truncated_indices.Count()) {
            // Remove all truncated values, which are nullptr in output_values
            output_values.EraseIf([](auto* v) { return v == nullptr; });
        }

        TINT_ASSERT(output_values.Length() == output_struct->Members().Length());
        return builder.Construct(output_struct, std::move(output_values))->Result();
    }
};
}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir, const ShaderIOConfig& config) {
    auto result = ValidateAndDumpIfNeeded(
        ir, "hlsl.ShaderIO", core::ir::Capabilities{core::ir::Capability::kAllowDuplicateBindings});
    if (result != Success) {
        return result;
    }

    core::ir::transform::RunShaderIOBase(ir, [&](core::ir::Module& mod, core::ir::Function* func) {
        return std::make_unique<StateImpl>(mod, func, config);
    });

    return Success;
}

}  // namespace tint::hlsl::writer::raise
