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

#include "src/tint/lang/msl/writer/raise/shader_io.h"

#include <memory>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/shader_io.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {

namespace {

/// PIMPL state for the parts of the shader IO transform specific to MSL.
/// For MSL, we take builtin inputs as entry point parameters, move non-builtin inputs to a struct
/// passed as an entry point parameter, and wrap outputs in a structure returned by the entry point.
struct StateImpl : core::ir::transform::ShaderIOBackendState {
    /// The configuration options.
    const ShaderIOConfig& config;

    /// The input parameters of the entry point.
    Vector<core::ir::FunctionParam*, 4> input_params;

    /// The list of input indices which map to parameter and optional struct member accesses.
    struct InputIndex {
        const uint32_t param_index;
        const uint32_t member_index;
    };
    Vector<InputIndex, 4> input_indices;

    /// The output struct type.
    core::type::Struct* output_struct = nullptr;

    /// The output values to return from the entry point.
    Vector<core::ir::Value*, 4> output_values;

    /// The index of the fixed sample mask builtin, if it was added.
    std::optional<uint32_t> fixed_sample_mask_index;

    std::optional<uint32_t> global_invocation_index_index;
    std::optional<uint32_t> global_invocation_id_index;
    std::optional<uint32_t> workgroup_index_index;
    std::optional<uint32_t> workgroup_id_index;
    std::optional<uint32_t> num_workgroups_index;

    /// Constructor
    StateImpl(core::ir::Module& mod, core::ir::Function* f, const ShaderIOConfig& cfg)
        : ShaderIOBackendState(mod, f), config(cfg) {
        if (auto wgsize = func->WorkgroupSizeAsConst()) {
            workgroup_size = wgsize;
        }
    }

    /// Destructor
    ~StateImpl() override {}

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    Vector<core::ir::FunctionParam*, 4> FinalizeInputs() override {
        // The following builtin values are polyfilled using other builtin values:
        // * workgroup_index - workgroup_id and num_workgroups
        // * global_invocation_index - global_invocation_id, num_workgroups (and workgroup size)
        const bool has_global_invocation_index =
            HasBuiltinInput(core::BuiltinValue::kGlobalInvocationIndex);
        const bool has_workgroup_index = HasBuiltinInput(core::BuiltinValue::kWorkgroupIndex);
        const bool needs_workgroup_id = has_workgroup_index;
        if (needs_workgroup_id) {
            RequireBuiltinInput(core::BuiltinValue::kWorkgroupId, ty.vec3u(), "workgroup_id");
        }
        const bool needs_num_workgroups = has_workgroup_index || has_global_invocation_index;
        if (needs_num_workgroups) {
            RequireBuiltinInput(core::BuiltinValue::kNumWorkgroups, ty.vec3u(), "num_workgroups");
        }
        const bool needs_global_invocation_id = has_global_invocation_index;
        if (needs_global_invocation_id) {
            RequireBuiltinInput(core::BuiltinValue::kGlobalInvocationId, ty.vec3u(),
                                "global_invocation_id");
        }

        Vector<core::type::Manager::StructMemberDesc, 4> input_struct_members;
        core::ir::FunctionParam* input_struct_param = nullptr;
        uint32_t input_struct_param_index = 0xffffffff;

        for (auto& input : inputs) {
            if (input.attributes.builtin) {
                auto builtin_value = input.attributes.builtin.value();
                auto index = static_cast<uint32_t>(input_indices.Length());
                switch (builtin_value) {
                    // Record an index for polyfilled inputs.
                    case core::BuiltinValue::kGlobalInvocationIndex:
                        input_indices.Push(InputIndex{index, 0u});
                        global_invocation_index_index = index;
                        continue;
                    case core::BuiltinValue::kWorkgroupIndex:
                        input_indices.Push(InputIndex{index, 0u});
                        workgroup_index_index = index;
                        continue;
                    // Save the indices of the builtins below for use in polyfills.
                    case core::BuiltinValue::kGlobalInvocationId:
                        global_invocation_id_index = index;
                        break;
                    case core::BuiltinValue::kWorkgroupId:
                        workgroup_id_index = index;
                        break;
                    case core::BuiltinValue::kNumWorkgroups:
                        num_workgroups_index = index;
                        break;
                    default:
                        break;
                }
                auto* param = b.FunctionParam(input.type);
                if (input.name) {
                    ir.SetName(param, input.name);
                }
                param->SetInvariant(input.attributes.invariant);
                param->SetBuiltin(builtin_value);
                input_indices.Push(InputIndex{static_cast<uint32_t>(input_params.Length()), 0u});
                input_params.Push(param);
            } else {
                if (!input_struct_param) {
                    input_struct_param = b.FunctionParam("inputs", nullptr);
                    input_struct_param_index = static_cast<uint32_t>(input_params.Length());
                    input_params.Push(input_struct_param);
                }
                input_indices.Push(
                    InputIndex{input_struct_param_index,
                               static_cast<uint32_t>(input_struct_members.Length())});
                input_struct_members.Push(input);
            }
        }

        if (!input_struct_members.IsEmpty()) {
            auto* input_struct =
                ty.Struct(ir.symbols.New(ir.NameOf(func).Name() + "_inputs"), input_struct_members);
            switch (func->Stage()) {
                case core::ir::Function::PipelineStage::kFragment:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kFragmentInput);
                    break;
                case core::ir::Function::PipelineStage::kVertex:
                    input_struct->AddUsage(core::type::PipelineStageUsage::kVertexInput);
                    break;
                case core::ir::Function::PipelineStage::kCompute:
                case core::ir::Function::PipelineStage::kUndefined:
                    TINT_IR_UNREACHABLE(ir);
            }
            input_struct_param->SetType(input_struct);
        }

        return input_params;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    const core::type::Type* FinalizeOutputs() override {
        // Add a fixed sample mask builtin for fragment shaders if needed.
        if (config.fixed_sample_mask != UINT32_MAX && func->IsFragment()) {
            AddFixedSampleMaskOutput();
        }

        if (outputs.IsEmpty()) {
            return ty.void_();
        }
        output_struct = ty.Struct(ir.symbols.New(ir.NameOf(func).Name() + "_outputs"), outputs);
        switch (func->Stage()) {
            case core::ir::Function::PipelineStage::kFragment:
                output_struct->AddUsage(core::type::PipelineStageUsage::kFragmentOutput);
                break;
            case core::ir::Function::PipelineStage::kVertex:
                output_struct->AddUsage(core::type::PipelineStageUsage::kVertexOutput);
                break;
            case core::ir::Function::PipelineStage::kCompute:
            case core::ir::Function::PipelineStage::kUndefined:
                TINT_IR_UNREACHABLE(ir);
        }
        output_values.Resize(outputs.Length());
        return output_struct;
    }

    /// @copydoc ShaderIO::BackendState::GetInput
    core::ir::Value* GetInput(core::ir::Builder& builder, uint32_t idx) override {
        if (idx == global_invocation_index_index) {
            return PolyfillGlobalInvocationIndex(builder, global_invocation_id_index.value(),
                                                 num_workgroups_index.value());
        }
        if (idx == workgroup_index_index) {
            return PolyfillWorkgroupIndex(builder, workgroup_id_index.value(),
                                          num_workgroups_index.value());
        }

        auto index = input_indices[idx];
        auto* param = input_params[index.param_index];
        if (param->Type()->Is<core::type::Struct>()) {
            return builder.Access(inputs[idx].type, param, u32(index.member_index))->Result();
        } else {
            return param;
        }
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(core::ir::Builder& builder, uint32_t idx, core::ir::Value* value) override {
        auto& output = outputs[idx];

        // If this a sample mask builtin, combine with the fixed sample mask if provided.
        if (config.fixed_sample_mask != UINT32_MAX &&
            output.attributes.builtin == core::BuiltinValue::kSampleMask) {
            value = builder.And(value, u32(config.fixed_sample_mask))->Result();
        }

        // Clamp frag_depth values if necessary.
        if (output.attributes.builtin == core::BuiltinValue::kFragDepth) {
            value = ClampFragDepth(builder, value);
        }

        output_values[idx] = value;
    }

    /// @copydoc ShaderIO::BackendState::MakeReturnValue
    core::ir::Value* MakeReturnValue(core::ir::Builder& builder) override {
        if (!output_struct) {
            return nullptr;
        }

        // If we created a fixed sample mask builtin, set the value from the provided mask.
        if (fixed_sample_mask_index) {
            output_values[fixed_sample_mask_index.value()] =
                builder.Constant(u32(config.fixed_sample_mask));
        }

        // Copy all of the outputs into a local variable and then return that.
        // We need to do this because the clip distances array has to be assigned one element at a
        // time and cannot be inlined as part of a single struct constructor.
        auto* result = builder.Var("tint_wrapper_result", ty.ptr<function>(output_struct));
        for (uint32_t i = 0; i < output_values.Length(); i++) {
            if (outputs[i].attributes.builtin == core::BuiltinValue::kClipDistances) {
                // Copy each clip distance to the result array.
                auto* arr = outputs[i].type->As<core::type::Array>();
                TINT_IR_ASSERT(ir, arr && arr->ConstantCount());
                for (uint32_t d = 0; d < arr->ConstantCount(); d++) {
                    auto* to = builder.Access<ptr<function, f32>>(result, u32(i), u32(d));
                    auto* from = builder.Access<f32>(output_values[i], u32(d));
                    builder.Store(to, from);
                }
            } else {
                // Copy the output directly to the corresponding member of the result structure.
                builder.Store(
                    builder.Access(ty.ptr<function>(output_values[i]->Type()), result, u32(i)),
                    output_values[i]);
            }
        }
        return builder.Load(result)->Result();
    }

    /// @copydoc ShaderIO::BackendState::NeedsVertexPointSize
    bool NeedsVertexPointSize() const override { return config.emit_vertex_point_size; }

    /// Add a fixed sample mask builtin output, unless a user-declared one already exists.
    void AddFixedSampleMaskOutput() {
        // Check if a user-defined sample mask builtin is present.
        for (auto& output : outputs) {
            if (output.attributes.builtin == core::BuiltinValue::kSampleMask) {
                return;
            }
        }

        // Create a new builtin sample mask output.
        fixed_sample_mask_index = AddOutput(ir.symbols.New("tint_sample_mask"), ty.u32(),
                                            core::IOAttributes{
                                                .builtin = core::BuiltinValue::kSampleMask,
                                            });
    }

    /// Clamp a frag_depth builtin value if necessary.
    /// @param builder the builder to use for new instructions
    /// @param frag_depth the incoming frag_depth value
    /// @returns the clamped value
    core::ir::Value* ClampFragDepth(core::ir::Builder& builder, core::ir::Value* frag_depth) {
        if (!config.depth_range_offsets) {
            return frag_depth;
        }

        auto* immediate_data = config.immediate_data_layout.var;
        auto min_idx = u32(config.immediate_data_layout.IndexOf(config.depth_range_offsets->min));
        auto max_idx = u32(config.immediate_data_layout.IndexOf(config.depth_range_offsets->max));
        auto* min = builder.Load(builder.Access<ptr<immediate, f32>>(immediate_data, min_idx));
        auto* max = builder.Load(builder.Access<ptr<immediate, f32>>(immediate_data, max_idx));
        return builder.Clamp(frag_depth, min, max)->Result();
    }
};

}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir, const ShaderIOConfig& config) {
    AssertValid(ir,
                tint::core::ir::Capabilities{
                    core::ir::Capability::kAllow8BitIntegers,
                    core::ir::Capability::kAllowDuplicateBindings,
                    core::ir::Capability::kAllowNonCoreTypes,
                },
                "before msl.ShaderIO");

    core::ir::transform::RunShaderIOBase(ir, [&](core::ir::Module& mod, core::ir::Function* func) {
        return std::make_unique<StateImpl>(mod, func, config);
    });

    return Success;
}

}  // namespace tint::msl::writer::raise
