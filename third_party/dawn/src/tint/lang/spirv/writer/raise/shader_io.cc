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

#include "src/tint/lang/spirv/writer/raise/shader_io.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <utility>

#include "spirv/unified1/spirv.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/shader_io.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/builtin_fn.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/image.h"
#include "src/tint/lang/spirv/type/literal.h"
#include "src/tint/utils/ice/ice.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

/// PIMPL state for the parts of the shader IO transform specific to SPIR-V.
/// For SPIR-V, we declare a global variable for each input and output. The wrapper entry point then
/// loads from and stores to these variables. We also modify the type of the SampleMask builtin to
/// be an array, as required by Vulkan.
struct StateImpl : core::ir::transform::ShaderIOBackendState {
    /// The input variables.
    Vector<core::ir::Var*, 4> input_vars;
    /// The output variables.
    Vector<core::ir::Var*, 4> output_vars;

    Vector<uint32_t, 4> input_indices;

    /// The configuration options.
    const ShaderIOConfig& config;

    // Final output value of 'position' builtin from the vertex shader.
    core::ir::Value* vert_out_position = nullptr;

    // IO index for vertex position emulation interpolant
    std::optional<uint32_t> center_pos_vert_idx = std::nullopt;

    // IO index for fragment position emulation interpolant
    std::optional<uint32_t> center_pos_frag_idx = std::nullopt;

    // IO index for sample_index
    std::optional<uint32_t> sample_index_idx = std::nullopt;

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

    /// Add a new interpolant that will be used to emulate the position builtin as if it always is
    /// pixel centered.
    /// @param entries the entries to emit
    /// @param addrspace the address to use for the global variables
    uint32_t AddCenterPosInterpolant(Vector<core::type::Manager::StructMemberDesc, 4>& entries,
                                     core::AddressSpace addrspace) {
        // Verbose way of finding the smallest free location (id). This of course needs to be the
        // same id value for both vertex and fragment.
        std::set<uint32_t> existing_locations;
        for (auto io : entries) {
            if (io.attributes.location.has_value()) {
                existing_locations.insert(io.attributes.location.value());
            }
        }
        uint32_t free_location = 0u;
        // We only need to search through existing_locations.size + 1 because either we will simply
        // add an index to the end or there will be a hole in the range of locations
        for (uint32_t i = 0u; i < (existing_locations.size() + 1); i++) {
            if (existing_locations.find(i) == existing_locations.end()) {
                free_location = i;
                break;
            }
        }

        auto io_attrib = core::IOAttributes{
            .location = free_location,
            .interpolation = core::Interpolation{.type = core::InterpolationType::kLinear,
                                                 .sampling = core::InterpolationSampling::kCenter}};

        if (addrspace == core::AddressSpace::kOut) {
            return AddOutput(ir.symbols.New("center_pos"), ty.vec4f(), io_attrib);
        }
        return AddInput(ir.symbols.New("center_pos"), ty.vec4f(), io_attrib);
    }

    /// Declare a global variable for each IO entry listed in @p entries.
    /// @param vars the list of variables
    /// @param entries the entries to emit
    /// @param addrspace the address to use for the global variables
    /// @param access the access mode to use for the global variables
    /// @param name_suffix the suffix to add to struct and variable names
    void MakeVars(Vector<core::ir::Var*, 4>& vars,
                  Vector<core::type::Manager::StructMemberDesc, 4>& entries,
                  core::AddressSpace addrspace,
                  core::Access access,
                  const char* name_suffix) {
        if (func->IsVertex() && addrspace == core::AddressSpace::kOut &&
            config.polyfill_pixel_center) {
            center_pos_vert_idx = AddCenterPosInterpolant(entries, addrspace);

        } else if (func->IsFragment() && addrspace == core::AddressSpace::kIn &&
                   config.polyfill_pixel_center) {
            center_pos_frag_idx = AddCenterPosInterpolant(entries, addrspace);
        }

        for (auto io : entries) {
            StringStream name;
            name << ir.NameOf(func).Name();

            if (io.attributes.builtin) {
                // SampleMask must be an array for Vulkan.
                if (io.attributes.builtin == core::BuiltinValue::kSampleMask) {
                    io.type = ty.array<u32, 1>();
                }
                name << "_" << io.attributes.builtin.value();

                // Vulkan requires that fragment integer builtin inputs be Flat decorated.
                if (func->IsFragment() && addrspace == core::AddressSpace::kIn &&
                    io.type->IsIntegerScalarOrVector()) {
                    io.attributes.interpolation =
                        core::Interpolation{core::InterpolationType::kFlat};
                }

                uint32_t index = static_cast<uint32_t>(input_indices.Length());
                switch (io.attributes.builtin.value()) {
                    // Record an index for polyfilled inputs.
                    case core::BuiltinValue::kGlobalInvocationIndex:
                        global_invocation_index_index = index;
                        input_indices.Push(index);
                        continue;
                    // Save the indices of the builtins below for use in polyfills.
                    case core::BuiltinValue::kWorkgroupIndex:
                        workgroup_index_index = index;
                        input_indices.Push(index);
                        continue;
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
            }
            if (io.attributes.location) {
                name << "_loc" << io.attributes.location.value();
                if (io.attributes.blend_src.has_value()) {
                    name << "_idx" << io.attributes.blend_src.value();
                }
            }
            name << name_suffix;

            // Replace f16 types with f32 types if necessary.
            auto* store_type = io.type;
            if (config.polyfill_f16_io) {
                if (store_type->DeepestElement()->Is<core::type::F16>()) {
                    store_type = ty.MatchWidth(ty.f32(), io.type);
                }
            }

            auto new_addrspace = addrspace;

            // Color becomes an InputAttachment in the handle space
            if (io.attributes.color) {
                auto sample_ty = store_type->DeepestElement();
                store_type = ty.Get<spirv::type::Image>(
                    sample_ty, type::Dim::kSubpassData, type::Depth::kNotDepth,
                    type::Arrayed::kNonArrayed,
                    config.multisampled_framebuffer_fetch ? type::Multisampled::kMultisampled
                                                          : type::Multisampled::kSingleSampled,
                    type::Sampled::kReadWriteOpCompatible, core::TexelFormat::kUndefined, access);

                new_addrspace = core::AddressSpace::kHandle;

                // Attach the provided binding point
                auto iter = config.colour_index_to_binding_point.find(io.attributes.color.value());
                TINT_IR_ASSERT(ir, iter != config.colour_index_to_binding_point.end());

                TINT_IR_ASSERT(ir, !io.attributes.binding_point.has_value());
                io.attributes.binding_point = iter->second;

                io.attributes.input_attachment_index = io.attributes.color;
                io.attributes.color = std::nullopt;
            }

            // Create an IO variable and add it to the root block.
            auto* ptr = ty.ptr(new_addrspace, store_type, access);
            auto* var = b.Var(name.str(), ptr);
            var->SetAttributes(io.attributes);

            ir.root_block->Append(var);
            input_indices.Push(static_cast<uint32_t>(vars.Length()));
            vars.Push(var);
        }
    }

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    Vector<core::ir::FunctionParam*, 4> FinalizeInputs() override {
        if (config.multisampled_framebuffer_fetch) {
            sample_index_idx =
                RequireBuiltinInput(core::BuiltinValue::kSampleIndex, ty.u32(), "sample_idx");
        }

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

        MakeVars(input_vars, inputs, core::AddressSpace::kIn, core::Access::kRead, "_Input");
        return tint::Empty;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    const core::type::Type* FinalizeOutputs() override {
        MakeVars(output_vars, outputs, core::AddressSpace::kOut, core::Access::kWrite, "_Output");
        return ty.void_();
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
        // Load the input from the global variable declared earlier.
        auto* ptr = ty.ptr(core::AddressSpace::kIn, inputs[idx].type, core::Access::kRead);
        auto input_index = input_indices[idx];
        auto* from = input_vars[input_index]->Result();

        // SampleMask becomes an array for SPIR-V, so load from the first element.
        if (inputs[idx].attributes.builtin == core::BuiltinValue::kSampleMask) {
            from = builder.Access(ptr, input_vars[input_index], 0_u)->Result();
        }

        core::ir::Value* value = builder.Load(from)->Result();

        if (inputs[idx].attributes.color.has_value()) {
            // coords for input_attachment are always (0, 0)
            auto* coords = builder.Composite(ty.vec2i(), 0_i, 0_i);

            // Start building the argument list for the builtin.
            // The first two operands are always the texture and then the coordinates.
            Vector<core::ir::Value*, 8> builtin_args;
            builtin_args.Push(value);
            builtin_args.Push(coords);

            if (config.multisampled_framebuffer_fetch) {
                builtin_args.Push(
                    builder.Constant(builder.ir.constant_values.Get<core::constant::Scalar<u32>>(
                        ty.Get<type::Literal>(), u32(SpvImageOperandsSampleMask))));
                builtin_args.Push(builder.Load(input_vars[sample_index_idx.value()])->Result());
            }

            // Call the builtin.
            value = builder
                        .Call<spirv::ir::BuiltinCall>(ty.vec4(inputs[idx].type->DeepestElement()),
                                                      spirv::BuiltinFn::kImageRead,
                                                      std::move(builtin_args))
                        ->Result();

            auto* orig_ty = inputs[idx].type;
            if (orig_ty->IsAnyOf<core::type::I32, core::type::U32, core::type::F32>()) {
                value = builder.Swizzle(orig_ty, value, {0})->Result();
            } else {
                auto* vec = orig_ty->As<core::type::Vector>();
                TINT_IR_ASSERT(ir, vec);

                if (vec->Width() != 4) {
                    Vector<uint32_t, 3> indices;
                    for (uint32_t i = 0; i < vec->Width(); ++i) {
                        indices.Push(i);
                    }
                    value = builder.Swizzle(orig_ty, value, indices)->Result();
                }
            }
        }

        // Convert f32 values to f16 values if needed.
        if (config.polyfill_f16_io && inputs[idx].type->DeepestElement()->Is<core::type::F16>()) {
            value = builder.Convert(inputs[idx].type, value)->Result();
        }

        if (inputs[idx].attributes.builtin == core::BuiltinValue::kPosition &&
            center_pos_frag_idx.has_value()) {
            // This fix is idempotent in that if it was not needed it will still apply correctly.
            auto* vec_xy = builder.Swizzle(ty.vec2f(), value, {0, 1});
            auto* floor_xy = builder.Call(ty.vec2f(), core::BuiltinFn::kFloor, vec_xy);
            auto p5_const = builder.Constant(0.5_f);
            auto* plus_p5 = builder.Add(floor_xy, builder.Splat(ty.vec2f(), p5_const));

            auto center_idx = input_indices[center_pos_frag_idx.value()];
            auto* xyzw_from_user_center = builder.Load(input_vars[center_idx]);

            auto* user_center_z = builder.Swizzle(ty.f32(), xyzw_from_user_center, {2});
            auto* user_center_w = builder.Swizzle(ty.f32(), xyzw_from_user_center, {3});

            auto* viewport_user_center_z =
                ViewportMappedFragDepth(builder, user_center_z->Result());
            value = builder.Construct(ty.vec4f(), plus_p5, viewport_user_center_z, user_center_w)
                        ->Result();
        }

        return value;
    }

    /// Propagate outputs from the inner function call to their final destination.
    /// @param builder the IR builder for new instructions
    /// @param inner_result the return value from calling the original entry point function
    void SetBackendOutputs(core::ir::Builder& builder, core::ir::Value* inner_result) override {
        if (center_pos_vert_idx.has_value()) {
            SetOutput(builder, center_pos_vert_idx.value(), inner_result);
        }
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(core::ir::Builder& builder, uint32_t idx, core::ir::Value* value) override {
        // Store the output to the global variable declared earlier.
        auto& output = outputs[idx];
        auto* ptr = ty.ptr(core::AddressSpace::kOut, output.type, core::Access::kWrite);
        auto* to = output_vars[idx]->Result();

        // SampleMask becomes an array for SPIR-V, so store to the first element.
        if (output.attributes.builtin == core::BuiltinValue::kSampleMask) {
            to = builder.Access(ptr, to, 0_u)->Result();
        }

        if (output.attributes.builtin == core::BuiltinValue::kPosition) {
            vert_out_position = value;
        }

        if (center_pos_vert_idx.has_value() && center_pos_vert_idx == idx) {
            // Special center position polyfilled from within vertex shader.
            TINT_IR_ASSERT(ir, vert_out_position);
            auto one_div_w =
                builder.Divide(1_f, builder.Swizzle(ty.f32(), vert_out_position, {3u}));
            auto z_div_w =
                builder.Multiply(one_div_w, builder.Swizzle(ty.f32(), vert_out_position, {2u}));
            value =
                builder
                    .Construct(ty.vec4f(), builder.Swizzle(ty.vec2f(), vert_out_position, {0, 1}),
                               z_div_w, one_div_w)
                    ->Result();
        }

        // Clamp frag_depth values if necessary.
        if (output.attributes.builtin == core::BuiltinValue::kFragDepth) {
            value = ClampFragDepth(builder, value);
        }

        // Convert f16 values to f32 values if needed.
        if (config.polyfill_f16_io && value->Type()->DeepestElement()->Is<core::type::F16>()) {
            value = builder.Convert(to->Type()->UnwrapPtr(), value)->Result();
        }

        builder.Store(to, value);
    }

    /// Remap viewport z if necessary.
    /// @param builder the builder to use for new instructions
    /// @param frag_depth the incoming frag_depth value
    /// @returns the clamped value
    core::ir::Value* ViewportMappedFragDepth(core::ir::Builder& builder,
                                             core::ir::Value* frag_depth) {
        if (!config.depth_range_offsets) {
            return frag_depth;
        }

        auto* immediate_data = config.immediate_data_layout.var;
        auto min_idx = u32(config.immediate_data_layout.IndexOf(config.depth_range_offsets->min));
        auto max_idx = u32(config.immediate_data_layout.IndexOf(config.depth_range_offsets->max));
        auto* min = builder.Load(builder.Access<ptr<immediate, f32>>(immediate_data, min_idx));
        auto* max = builder.Load(builder.Access<ptr<immediate, f32>>(immediate_data, max_idx));
        // Viewport remapping depth normalization equation.
        // https://www.w3.org/TR/webgpu/#coordinate-systems#:~:text=Viewport%20coordinates
        auto* max_minus_min = builder.Subtract(max, min);
        auto* rhs = builder.Multiply(max_minus_min, frag_depth);
        return builder.Add(min, rhs)->Result();
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

    /// @copydoc ShaderIO::BackendState::NeedsVertexPointSize
    bool NeedsVertexPointSize() const override { return config.emit_vertex_point_size; }
};
}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir, const ShaderIOConfig& config) {
    AssertValid(ir, kShaderIOCapabilities, "before spirv.ShaderIO");

    core::ir::transform::RunShaderIOBase(ir, [&](core::ir::Module& mod, core::ir::Function* func) {
        return std::make_unique<StateImpl>(mod, func, config);
    });

    return Success;
}

}  // namespace tint::spirv::writer::raise
