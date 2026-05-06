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

#include "src/tint/lang/spirv/writer/raise/raise.h"

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/bgra8unorm_polyfill.h"
#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/builtin_scalarize.h"
#include "src/tint/lang/core/ir/transform/combine_access_instructions.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/decompose_access.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/direct_variable_access.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"
#include "src/tint/lang/core/ir/transform/preserve_padding.h"
#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"
#include "src/tint/lang/core/ir/transform/remove_uniform_vector_component_loads.h"
#include "src/tint/lang/core/ir/transform/resource_table.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/signed_integer_polyfill.h"
#include "src/tint/lang/core/ir/transform/single_entry_point.h"
#include "src/tint/lang/core/ir/transform/std140.h"
#include "src/tint/lang/core/ir/transform/substitute_overrides.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/spirv/writer/common/option_helpers.h"
#include "src/tint/lang/spirv/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/spirv/writer/raise/case_switch_to_if_else.h"
#include "src/tint/lang/spirv/writer/raise/expand_implicit_splats.h"
#include "src/tint/lang/spirv/writer/raise/fork_explicit_layout_types.h"
#include "src/tint/lang/spirv/writer/raise/handle_matrix_arithmetic.h"
#include "src/tint/lang/spirv/writer/raise/keep_binding_array_as_pointer.h"
#include "src/tint/lang/spirv/writer/raise/merge_return.h"
#include "src/tint/lang/spirv/writer/raise/pass_matrix_by_pointer.h"
#include "src/tint/lang/spirv/writer/raise/remove_unreachable_in_loop_continuing.h"
#include "src/tint/lang/spirv/writer/raise/resource_table_helper.h"
#include "src/tint/lang/spirv/writer/raise/shader_io.h"
#include "src/tint/lang/spirv/writer/raise/unary_polyfill.h"
#include "src/tint/lang/spirv/writer/raise/var_for_dynamic_index.h"

namespace tint::spirv::writer {

Result<SuccessType> Raise(core::ir::Module& module, const Options& options) {
    TINT_CHECK_RESULT(core::ir::transform::SingleEntryPoint(module, options.entry_point_name));

    TINT_CHECK_RESULT(
        core::ir::transform::SubstituteOverrides(module, options.substitute_overrides_config));

    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    PopulateRemapperAndMultiplanarOptions(options, remapper_data, multiplanar_map);

    TINT_CHECK_RESULT(core::ir::transform::BindingRemapper(module, remapper_data));

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config;
        if (options.extensions.disable_image_robustness) {
            config.clamp_texture = false;
        }
        config.disable_runtime_sized_array_index_clamping =
            options.extensions.disable_runtime_sized_array_index_clamping;
        config.use_integer_range_analysis = !options.disable_integer_range_analysis;
        TINT_CHECK_RESULT(core::ir::transform::Robustness(module, config));

        TINT_CHECK_RESULT(core::ir::transform::PreventInfiniteLoops(module));
    }

    spirv::writer::raise::ResourceTableHelper helper;
    TINT_CHECK_RESULT(core::ir::transform::ResourceTable(module, options.resource_table, &helper));

    // PrepareImmediateData must come before any transform that needs internal immediate data.
    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    if (options.depth_range_offsets) {
        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            options.depth_range_offsets.value().min, module.symbols.New("tint_frag_depth_min"),
            module.Types().f32()));
        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            options.depth_range_offsets.value().max, module.symbols.New("tint_frag_depth_max"),
            module.Types().f32()));
    }
    TINT_CHECK_RESULT_UNWRAP(immediate_data_layout, core::ir::transform::PrepareImmediateData(
                                                        module, immediate_data_config));

    core::ir::transform::BinaryPolyfillConfig binary_polyfills;
    binary_polyfills.bitshift_modulo = true;
    binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
    TINT_CHECK_RESULT(core::ir::transform::BinaryPolyfill(module, binary_polyfills));

    core::ir::transform::BuiltinPolyfillConfig core_polyfills;
    core_polyfills.clamp_int = true;
    core_polyfills.count_leading_zeros = true;
    core_polyfills.count_trailing_zeros = true;
    core_polyfills.extract_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
    core_polyfills.first_leading_bit = true;
    core_polyfills.first_trailing_bit = true;
    core_polyfills.insert_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
    core_polyfills.saturate = true;
    core_polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
    core_polyfills.dot_4x8_packed = options.extensions.dot_4x8_packed;
    core_polyfills.pack_unpack_4x8 = true;
    core_polyfills.pack_4xu8_clamp = true;
    core_polyfills.pack_unpack_4x8_norm = options.workarounds.polyfill_pack_unpack_4x8_norm;
    core_polyfills.abs_signed_int = true;
    core_polyfills.length_scalar_f32 = options.workarounds.polyfill_length_scalar_f32;
    core_polyfills.distance_scalar_f32 = options.workarounds.polyfill_distance_scalar_f32;
    core_polyfills.subgroup_broadcast_f16 = options.workarounds.polyfill_subgroup_broadcast_f16;
    core_polyfills.saturate_as_min_max = options.workarounds.polyfill_saturate_as_min_max_f16;
    TINT_CHECK_RESULT(core::ir::transform::BuiltinPolyfill(module, core_polyfills));

    core::ir::transform::ConversionPolyfillConfig conversion_polyfills;
    conversion_polyfills.ftoi = true;
    TINT_CHECK_RESULT(core::ir::transform::ConversionPolyfill(module, conversion_polyfills));

    if (!options.disable_workgroup_init &&
        !options.extensions.use_zero_initialize_workgroup_memory) {
        TINT_CHECK_RESULT(core::ir::transform::ZeroInitWorkgroupMemory(module));
    }

    // PreservePadding must come before DirectVariableAccess.
    TINT_CHECK_RESULT(core::ir::transform::PreservePadding(module));

    core::ir::transform::DirectVariableAccessOptions dva_options;
    dva_options.transform_function = true;
    dva_options.transform_private = true;
    dva_options.transform_handle = options.workarounds.dva_transform_handle
                                       ? core::ir::transform::HandleTransformLevel::kFull
                                       : core::ir::transform::HandleTransformLevel::kExternal;
    TINT_CHECK_RESULT(core::ir::transform::DirectVariableAccess(module, dva_options));

    // Must come after DirectVariableAccess as we need all ExternalTextures to have their functions
    // flattened.
    TINT_CHECK_RESULT(core::ir::transform::MultiplanarExternalTexture(module, multiplanar_map));

    // Fixup loads of binding_arrays of handles that may have been introduced by
    // DirectVariableAccess (DVA). Vulkan drivers that need DVA of handle expect binding_arrays to
    // stay as pointer and many mishandle by-value binding_arrays.
    if (options.workarounds.dva_transform_handle) {
        TINT_CHECK_RESULT(raise::KeepBindingArrayAsPointer(module));
    }

    if (options.workarounds.pass_matrix_by_pointer) {
        // PassMatrixByPointer must come after PreservePadding+DirectVariableAccess.
        TINT_CHECK_RESULT(raise::PassMatrixByPointer(module));
    }

    TINT_CHECK_RESULT(core::ir::transform::Bgra8UnormPolyfill(module));

    if (!options.extensions.use_uniform_buffers) {
        // DecomposeAccess must come before BlockDecoratedStructs, which will wrap the
        // uniform variable in a structure.
        core::ir::transform::DecomposeAccessOptions decompose_config{.uniform = true};
        TINT_CHECK_RESULT(core::ir::transform::DecomposeAccess(module, decompose_config));
    } else {
        TINT_CHECK_RESULT(core::ir::transform::Std140(module));
    }
    TINT_CHECK_RESULT(core::ir::transform::BlockDecoratedStructs(module));

    TINT_CHECK_RESULT(core::ir::transform::VectorizeScalarMatrixConstructors(module));

    // CombineAccessInstructions must come after DirectVariableAccess and BlockDecoratedStructs.
    // We run this transform as some Qualcomm drivers struggle with partial access chains that
    // produce pointers to matrices.
    TINT_CHECK_RESULT(core::ir::transform::CombineAccessInstructions(module));

    // RemoveUniformVectorComponentLoads is used to work around a Qualcomm driver bug.
    // See crbug.com/452350626.
    TINT_CHECK_RESULT(core::ir::transform::RemoveUniformVectorComponentLoads(module));

    if (!options.extensions.use_demote_to_helper_invocation) {
        // DemoteToHelper must come before any transform that introduces non-core instructions.
        TINT_CHECK_RESULT(core::ir::transform::DemoteToHelper(module));
    }

    raise::PolyfillConfig config = {
        .use_vulkan_memory_model = options.extensions.use_vulkan_memory_model,
        .version = options.spirv_version,
        .subgroup_shuffle_clamped = options.workarounds.subgroup_shuffle_clamped,
        .texture_sample_compare_depth_cube_array =
            options.workarounds.texture_sample_compare_depth_cube_array,
        .cooperative_matrix_stride_is_matrix_elements =
            options.workarounds.cooperative_matrix_stride_is_matrix_elements,
    };
    TINT_CHECK_RESULT(raise::BuiltinPolyfill(module, config));
    TINT_CHECK_RESULT(raise::ExpandImplicitSplats(module));

    core::ir::transform::BuiltinScalarizeConfig scalarize_config{
        .scalarize_clamp = options.workarounds.scalarize_max_min_clamp,
        .scalarize_max = options.workarounds.scalarize_max_min_clamp,
        .scalarize_min = options.workarounds.scalarize_max_min_clamp};
    TINT_CHECK_RESULT(core::ir::transform::BuiltinScalarize(module, scalarize_config));

    core::ir::transform::SignedIntegerPolyfillConfig signed_integer_cfg{
        .signed_negation = true, .signed_arithmetic = true, .signed_shiftleft = true};
    TINT_CHECK_RESULT(core::ir::transform::SignedIntegerPolyfill(module, signed_integer_cfg));

    // AMD mesa front end optimizer bug for unary negation and abs.
    // Fixed in 25.3 - See crbug.com/448294721
    raise::UnaryPolyfillConfig unary_polyfill_cfg = {
        .polyfill_f32_negation = options.workarounds.polyfill_unary_f32_negation,
        .polyfill_f32_abs = options.workarounds.polyfill_f32_abs};

    TINT_CHECK_RESULT(raise::UnaryPolyfill(module, unary_polyfill_cfg));

    // kAllowAnyInputAttachmentIndexType required after ExpandImplicitSplats
    TINT_CHECK_RESULT(raise::HandleMatrixArithmetic(module));
    TINT_CHECK_RESULT(raise::MergeReturn(module));
    if (options.workarounds.polyfill_case_switch) {
        TINT_CHECK_RESULT(raise::CaseSwitchToIfElse(module));
    }
    TINT_CHECK_RESULT(raise::RemoveUnreachableInLoopContinuing(module));
    TINT_CHECK_RESULT(raise::ShaderIO(
        module, raise::ShaderIOConfig{
                    .immediate_data_layout = immediate_data_layout,
                    .colour_index_to_binding_point = options.colour_index_to_binding_point,
                    .emit_vertex_point_size = options.emit_vertex_point_size,
                    .polyfill_f16_io = !options.extensions.use_storage_input_output_16,
                    .polyfill_pixel_center = options.polyfill_pixel_center,
                    .multisampled_framebuffer_fetch = options.multisampled_framebuffer_fetch,
                    .depth_range_offsets = options.depth_range_offsets,
                }));

    // ForkExplicitLayoutTypes must come after DecomposeAccess, since it rewrites
    // host-shareable array types to use the explicitly laid array type defined by the SPIR-V
    // dialect.
    TINT_CHECK_RESULT(raise::ForkExplicitLayoutTypes(module, options.spirv_version));

    TINT_CHECK_RESULT(raise::VarForDynamicIndex(module));

    return Success;
}

}  // namespace tint::spirv::writer
