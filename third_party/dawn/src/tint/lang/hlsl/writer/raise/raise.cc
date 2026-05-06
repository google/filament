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

#include "src/tint/lang/hlsl/writer/raise/raise.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/array_length_from_immediate.h"
#include "src/tint/lang/core/ir/transform/array_length_from_uniform.h"
#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/builtin_scalarize.h"
#include "src/tint/lang/core/ir/transform/change_immediate_to_uniform.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/decompose_access.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/direct_variable_access.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"
#include "src/tint/lang/core/ir/transform/remove_continue_in_switch.h"
#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"
#include "src/tint/lang/core/ir/transform/rename_conflicts.h"
#include "src/tint/lang/core/ir/transform/resource_table.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/signed_integer_polyfill.h"
#include "src/tint/lang/core/ir/transform/single_entry_point.h"
#include "src/tint/lang/core/ir/transform/substitute_overrides.h"
#include "src/tint/lang/core/ir/transform/value_to_let.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/hlsl/writer/common/option_helpers.h"
#include "src/tint/lang/hlsl/writer/common/options.h"
#include "src/tint/lang/hlsl/writer/raise/array_offset_from_immediate.h"
#include "src/tint/lang/hlsl/writer/raise/array_offset_from_uniform.h"
#include "src/tint/lang/hlsl/writer/raise/binary_polyfill.h"
#include "src/tint/lang/hlsl/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/hlsl/writer/raise/decompose_storage_access.h"
#include "src/tint/lang/hlsl/writer/raise/extract_ternary_values.h"
#include "src/tint/lang/hlsl/writer/raise/localize_struct_array_assignment.h"
#include "src/tint/lang/hlsl/writer/raise/pixel_local.h"
#include "src/tint/lang/hlsl/writer/raise/promote_initializers.h"
#include "src/tint/lang/hlsl/writer/raise/replace_default_only_switch.h"
#include "src/tint/lang/hlsl/writer/raise/replace_non_indexable_mat_vec_stores.h"
#include "src/tint/lang/hlsl/writer/raise/resource_table_helper.h"
#include "src/tint/lang/hlsl/writer/raise/shader_io.h"

namespace tint::hlsl::writer {

Result<SuccessType> Raise(core::ir::Module& module, const Options& options) {
    TINT_CHECK_RESULT(core::ir::transform::SingleEntryPoint(module, options.entry_point_name));

    TINT_CHECK_RESULT(
        core::ir::transform::SubstituteOverrides(module, options.substitute_overrides_config));

    // PopulateBindingRelatedOptions must come before PrepareImmediateData so that
    // buffer_sizes_offset is available when configuring immediate data.
    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    ArrayLengthFromUniformOptions array_length_from_uniform_options{};
    ArrayOffsetFromUniformOptions array_offset_from_uniform_options{};
    PopulateBindingRelatedOptions(options, remapper_data, multiplanar_map,
                                  array_length_from_uniform_options,
                                  array_offset_from_uniform_options);

    // The number of vec4s used to store buffer sizes that will be set into the immediate block.
    uint32_t buffer_sizes_array_elements_num = 0;
    // The number of vec4s used to store buffer offsets that will be set into the immediate block.
    uint32_t buffer_offsets_array_elements_num = 0;

    // PrepareImmediateData must come before any transform that needs internal push constants.
    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    if (options.first_index_offset) {
        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            options.first_index_offset.value(), module.symbols.New("tint_first_index_offset"),
            module.Types().u32()));
    }

    if (options.first_instance_offset) {
        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            options.first_instance_offset.value(), module.symbols.New("tint_first_instance_offset"),
            module.Types().u32()));
    }

    if (options.num_workgroups_start_offset) {
        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            options.num_workgroups_start_offset.value(),
            module.symbols.New("tint_num_workgroups_start_offset"), module.Types().vec3u()));
    }

    if (array_length_from_uniform_options.buffer_sizes_offset) {
        // Find the largest index declared in the map, in order to determine the number of
        // elements needed in the array of buffer sizes. The buffer sizes will be packed into
        // vec4s to satisfy the 16-byte alignment requirement for array elements in constant
        // buffers.
        uint32_t max_index = 0;
        for (const auto& entry : array_length_from_uniform_options.bindpoint_to_size_index) {
            max_index = std::max(max_index, entry.second);
        }
        buffer_sizes_array_elements_num = (max_index / 4) + 1;

        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            array_length_from_uniform_options.buffer_sizes_offset.value(),
            module.symbols.New("buffer_sizes"),
            module.Types().array(module.Types().vec4<core::u32>(),
                                 buffer_sizes_array_elements_num)));
    }

    if (array_offset_from_uniform_options.buffer_offsets_offset) {
        // Find the largest index declared in the map, in order to determine the number of
        // elements needed in the array of buffer offsets. The buffer offsets will be packed into
        // vec4s to satisfy the 16-byte alignment requirement for array elements in constant
        // buffers.
        uint32_t max_index = 0;
        for (const auto& entry : array_offset_from_uniform_options.bindpoint_to_offset_index) {
            max_index = std::max(max_index, entry.second);
        }
        buffer_offsets_array_elements_num = (max_index / 4) + 1;

        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            array_offset_from_uniform_options.buffer_offsets_offset.value(),
            module.symbols.New("buffer_offsets"),
            module.Types().array(module.Types().vec4<core::u32>(),
                                 buffer_offsets_array_elements_num)));
    }

    TINT_CHECK_RESULT_UNWRAP(immediate_data_layout, core::ir::transform::PrepareImmediateData(
                                                        module, immediate_data_config));

    TINT_CHECK_RESULT(core::ir::transform::BindingRemapper(module, remapper_data));
    TINT_CHECK_RESULT(core::ir::transform::MultiplanarExternalTexture(module, multiplanar_map));

    // `LocalizeStructArrayAssignment` and `ReplaceNonIndexableMatVecStores` may insert additional
    // expressions before the assignments to arrays or vectors so it is better to add them before
    // `Robustness`.
    if (options.compiler == Options::Compiler::kFXC) {
        TINT_CHECK_RESULT(raise::LocalizeStructArrayAssignment(module));
        TINT_CHECK_RESULT(raise::ReplaceNonIndexableMatVecStores(module));
    }

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config{};
        config.bindings_ignored =
            std::unordered_set<BindingPoint>(options.ignored_by_robustness_transform.cbegin(),
                                             options.ignored_by_robustness_transform.cend());

        // Direct3D guarantees to return zero for any resource that is accessed out of bounds, and
        // according to the description of the assembly store_uav_typed, out of bounds addressing
        // means nothing gets written to memory.
        config.clamp_texture = false;

        config.use_integer_range_analysis = !options.disable_integer_range_analysis;

        TINT_CHECK_RESULT(core::ir::transform::Robustness(module, config));

        TINT_CHECK_RESULT(core::ir::transform::PreventInfiniteLoops(module));
    }

    if (options.resource_table.has_value()) {
        hlsl::writer::raise::ResourceTableHelper helper;
        TINT_CHECK_RESULT(
            core::ir::transform::ResourceTable(module, options.resource_table.value(), &helper));
    }

    {
        core::ir::transform::BinaryPolyfillConfig binary_polyfills{};
        binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        binary_polyfills.bitshift_modulo = true;
        TINT_CHECK_RESULT(core::ir::transform::BinaryPolyfill(module, binary_polyfills));
    }

    {
        core::ir::transform::BuiltinPolyfillConfig core_polyfills{};
        core_polyfills.clamp_int = true;
        core_polyfills.dot_4x8_packed = options.extensions.polyfill_dot_4x8_packed;

        // TODO(crbug.com/tint/1449): Some of these can map to HLSL's `firstbitlow`
        // and `firstbithigh`.
        core_polyfills.count_leading_zeros = true;
        core_polyfills.count_trailing_zeros = true;
        core_polyfills.degrees = true;
        core_polyfills.extract_bits = core::ir::transform::BuiltinPolyfillLevel::kFull;
        core_polyfills.first_leading_bit = true;
        core_polyfills.first_trailing_bit = true;
        core_polyfills.fwidth_fine = true;
        core_polyfills.insert_bits = core::ir::transform::BuiltinPolyfillLevel::kFull;

        // Currently Pack4xU8Clamp() must be polyfilled because on latest DXC pack_clamp_u8()
        // receives an int32_t4 as its input.
        // See https://github.com/microsoft/DirectXShaderCompiler/issues/5091 for more details.
        core_polyfills.pack_4xu8_clamp = true;
        core_polyfills.pack_unpack_4x8 = options.extensions.polyfill_pack_unpack_4x8;
        core_polyfills.radians = true;
        core_polyfills.reflect_vec2_f32 = options.workarounds.polyfill_reflect_vec2_f32;
        core_polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        core_polyfills.abs_signed_int = true;
        core_polyfills.subgroup_broadcast_f16 = options.workarounds.polyfill_subgroup_broadcast_f16;
        TINT_CHECK_RESULT(core::ir::transform::BuiltinPolyfill(module, core_polyfills));
    }

    {
        core::ir::transform::ConversionPolyfillConfig conversion_polyfills{};
        conversion_polyfills.ftoi = true;
        TINT_CHECK_RESULT(core::ir::transform::ConversionPolyfill(module, conversion_polyfills));
    }

    if (options.compiler == Options::Compiler::kFXC) {
        TINT_CHECK_RESULT(raise::ReplaceDefaultOnlySwitch(module));
    }

    // ArrayLength must run after Robustness, which introduces arrayLength calls.
    // TODO(crbug.com/366291600): Replace ArrayLengthFromUniform with ArrayLengthFromImmediates
    if (array_length_from_uniform_options.buffer_sizes_offset) {
        // Use ArrayLengthFromImmediates when buffer_sizes_offset is provided.
        TINT_ASSERT(!array_length_from_uniform_options.ubo_binding.group &&
                    !array_length_from_uniform_options.ubo_binding.binding);

        TINT_CHECK_RESULT(core::ir::transform::ArrayLengthFromImmediates(
            module, immediate_data_layout,
            array_length_from_uniform_options.buffer_sizes_offset.value(),
            buffer_sizes_array_elements_num,
            array_length_from_uniform_options.bindpoint_to_size_index));
    } else {
        // Always fall back to ArrayLengthFromUniform when buffer_sizes_offset is not provided.
        // This preserves the behavior from before ArrayLengthFromImmediates was introduced,
        // ensuring that arrayLength() calls are properly handled even without explicit options.
        TINT_CHECK_RESULT(core::ir::transform::ArrayLengthFromUniform(
            module,
            BindingPoint{array_length_from_uniform_options.ubo_binding.group,
                         array_length_from_uniform_options.ubo_binding.binding},
            array_length_from_uniform_options.bindpoint_to_size_index));
    }

    if (!options.disable_workgroup_init) {
        // Must run before ShaderIO as it may introduce a builtin parameter (local_invocation_index)
        TINT_CHECK_RESULT(core::ir::transform::ZeroInitWorkgroupMemory(module));
    }

    const bool pixel_local_enabled = !options.pixel_local.attachments.empty();

    // ShaderIO must be run before DecomposeAccess because it might
    // introduce a uniform buffer for kNumWorkgroups.
    {
        raise::ShaderIOConfig config = {
            .immediate_data_layout = immediate_data_layout,
            .num_workgroups_binding = options.root_constant_binding_point,
            .first_index_offset_binding = options.root_constant_binding_point,
            .add_input_position_member = pixel_local_enabled,
            .truncate_interstage_variables = options.truncate_interstage_variables,
            .interstage_locations = std::move(options.interstage_locations),
            .first_index_offset = options.first_index_offset,
            .first_instance_offset = options.first_instance_offset,
            .num_workgroups_start_offset = options.num_workgroups_start_offset,
        };

        TINT_CHECK_RESULT(raise::ShaderIO(module, config));
    }

    // DemoteToHelper must come before any transform that introduces non-core instructions.
    // Run after ShaderIO to ensure the discards are added to the entry point it introduces.
    // TODO(crbug.com/42250787): This is only necessary when FXC is being used.
    if (options.compiler == tint::hlsl::writer::Options::Compiler::kFXC) {
        TINT_CHECK_RESULT(core::ir::transform::DemoteToHelper(module));
    }
    TINT_CHECK_RESULT(core::ir::transform::DirectVariableAccess(
        module, core::ir::transform::DirectVariableAccessOptions{}));

    // DecomposeStorageAccess must come after Robustness and DirectVariableAccess
    TINT_CHECK_RESULT(raise::DecomposeStorageAccess(module));

    // ArrayOffsetFrom* transforms must come after both DirectVariableAccess and
    // DecomposeStorageAccess, and BEFORE ChangeImmediateToUniform.
    // TODO(crbug.com/366291600): Replace ArrayOffsetFromUniform with ArrayOffsetFromImmediates
    if (array_offset_from_uniform_options.buffer_offsets_offset) {
        // Use ArrayOffsetFromImmediates when buffer_offsets_offset is provided.
        TINT_ASSERT(!array_offset_from_uniform_options.ubo_binding.group &&
                    !array_offset_from_uniform_options.ubo_binding.binding);

        TINT_CHECK_RESULT(raise::ArrayOffsetFromImmediates(
            module, immediate_data_layout,
            array_offset_from_uniform_options.buffer_offsets_offset.value(),
            buffer_offsets_array_elements_num,
            array_offset_from_uniform_options.bindpoint_to_offset_index));
    } else if (array_offset_from_uniform_options.ubo_binding.group ||
               array_offset_from_uniform_options.ubo_binding.binding) {
        // Fall back to ArrayOffsetFromUniform when UBO binding is provided.
        // ArrayOffsetFromUniform operates on uniform buffers and must come after
        // ChangeImmediateToUniform (see below after ChangeImmediateToUniform transform).
    }

    // ChangeImmediateToUniformConfig must come before DecomposeAccess (to write correct
    // uniform access instructions).
    {
        core::ir::transform::ChangeImmediateToUniformConfig config = {
            .immediate_binding_point = options.immediate_binding_point,
        };
        TINT_CHECK_RESULT(core::ir::transform::ChangeImmediateToUniform(module, config));
    }

    // ArrayOffsetFromUniform must come after ChangeImmediateToUniform, DirectVariableAccess, and
    // DecomposeStorageAccess.
    if (array_offset_from_uniform_options.ubo_binding.group ||
        array_offset_from_uniform_options.ubo_binding.binding) {
        TINT_CHECK_RESULT(raise::ArrayOffsetFromUniform(
            module,
            BindingPoint{array_offset_from_uniform_options.ubo_binding.group,
                         array_offset_from_uniform_options.ubo_binding.binding},
            array_offset_from_uniform_options.bindpoint_to_offset_index));
    }

    // DecomposeAccess must come after DecomposeStorageAccess, ChangeImmediateToUniform, and
    // ArrayOffsetFrom* transforms
    core::ir::transform::DecomposeAccessOptions decompose_config{.uniform = true};
    TINT_CHECK_RESULT(core::ir::transform::DecomposeAccess(module, decompose_config));

    // PixelLocal must run after DirectVariableAccess to avoid chasing pointer parameters.
    if (pixel_local_enabled) {
        raise::PixelLocalConfig config;
        config.options = options.pixel_local;
        TINT_CHECK_RESULT(raise::PixelLocal(module, config));
    }

    TINT_CHECK_RESULT(raise::BinaryPolyfill(module));

    // Avoid potential UB (aka signed overflow) by performing unsigned integer arithmetic.
    core::ir::transform::SignedIntegerPolyfillConfig signed_integer_cfg{
        .signed_negation = true, .signed_arithmetic = true, .signed_shiftleft = true};
    TINT_CHECK_RESULT(core::ir::transform::SignedIntegerPolyfill(module, signed_integer_cfg));

    // BuiltinPolyfill must come after BinaryPolyfill and DecomposeStorageAccess as they add
    // builtins
    TINT_CHECK_RESULT(raise::BuiltinPolyfill(module));
    TINT_CHECK_RESULT(core::ir::transform::VectorizeScalarMatrixConstructors(module));
    TINT_CHECK_RESULT(core::ir::transform::RemoveContinueInSwitch(module));

    // ExtractTernaryValues must come after BuiltinPolyfill because that's what introduces the
    // ternary builtins.
    TINT_CHECK_RESULT(raise::ExtractTernaryValues(module));

    core::ir::transform::BuiltinScalarizeConfig scalarize_config{
        .scalarize_clamp = options.workarounds.scalarize_max_min_clamp,
        .scalarize_max = options.workarounds.scalarize_max_min_clamp,
        .scalarize_min = options.workarounds.scalarize_max_min_clamp};
    TINT_CHECK_RESULT(core::ir::transform::BuiltinScalarize(module, scalarize_config));

    // These transforms need to be run last as various transforms introduce terminator arguments,
    // naming conflicts, and expressions that need to be explicitly not inlined.
    TINT_CHECK_RESULT(core::ir::transform::RemoveTerminatorArgs(module));
    TINT_CHECK_RESULT(core::ir::transform::RenameConflicts(module));
    {
        core::ir::transform::ValueToLetConfig cfg;
        cfg.replace_pointer_lets = true;
        TINT_CHECK_RESULT(core::ir::transform::ValueToLet(module, cfg));
    }

    // Anything which runs after this needs to handle `Capabilities::kAllowModuleScopedLets`
    TINT_CHECK_RESULT(raise::PromoteInitializers(module));

    return Success;
}

}  // namespace tint::hlsl::writer
