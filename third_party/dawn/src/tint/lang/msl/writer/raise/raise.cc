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

#include "src/tint/lang/msl/writer/raise/raise.h"

#include <algorithm>
#include <utility>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/array_length_from_immediate.h"
#include "src/tint/lang/core/ir/transform/array_length_from_uniform.h"
#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/builtin_scalarize.h"
#include "src/tint/lang/core/ir/transform/change_immediate_to_uniform.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"
#include "src/tint/lang/core/ir/transform/preserve_padding.h"
#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"
#include "src/tint/lang/core/ir/transform/remove_continue_in_switch.h"
#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"
#include "src/tint/lang/core/ir/transform/rename_conflicts.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/signed_integer_polyfill.h"
#include "src/tint/lang/core/ir/transform/single_entry_point.h"
#include "src/tint/lang/core/ir/transform/substitute_overrides.h"
#include "src/tint/lang/core/ir/transform/value_to_let.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/vertex_pulling.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/msl/writer/common/option_helpers.h"
#include "src/tint/lang/msl/writer/raise/argument_buffers.h"
#include "src/tint/lang/msl/writer/raise/binary_polyfill.h"
#include "src/tint/lang/msl/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/msl/writer/raise/convert_print_to_log.h"
#include "src/tint/lang/msl/writer/raise/fix_type_layout.h"
#include "src/tint/lang/msl/writer/raise/module_constant.h"
#include "src/tint/lang/msl/writer/raise/module_scope_vars.h"
#include "src/tint/lang/msl/writer/raise/shader_io.h"
#include "src/tint/lang/msl/writer/raise/simd_ballot.h"
#include "src/tint/lang/msl/writer/raise/validate_subgroup_matrix.h"

namespace tint::msl::writer {

Result<RaiseResult> Raise(core::ir::Module& module, const Options& options) {
    TINT_CHECK_RESULT(core::ir::transform::SingleEntryPoint(module, options.entry_point_name));

    TINT_CHECK_RESULT(
        core::ir::transform::SubstituteOverrides(module, options.substitute_overrides_config));

    TINT_CHECK_RESULT(raise::ValidateSubgroupMatrix(module));

    RaiseResult raise_result;

    // VertexPulling must come before BindingRemapper and Robustness.
    if (options.vertex_pulling_config) {
        TINT_CHECK_RESULT(
            core::ir::transform::VertexPulling(module, *options.vertex_pulling_config));
    }

    // Populate binding-related options before prepare immediate data transform
    // to ensure buffer_sizes is set correctly.
    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    ArrayLengthOptions array_length_from_constants{};
    PopulateBindingRelatedOptions(options, remapper_data, multiplanar_map,
                                  array_length_from_constants);

    // The number of vec4s used to store buffer sizes that will be set into the immediate block.
    uint32_t buffer_sizes_array_elements_num = 0;

    // PrepareImmediateData must come before any transform that needs internal immediates.
    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    if (array_length_from_constants.buffer_sizes_offset) {
        // Find the largest index declared in the map, in order to determine the number of
        // elements needed in the array of buffer sizes. The buffer sizes will be packed into
        // vec4s to satisfy the 16-byte alignment requirement for array elements in uniform
        // buffers.
        uint32_t max_index = 0;
        for (auto& entry : array_length_from_constants.bindpoint_to_size_index) {
            max_index = std::max(max_index, entry.second);
        }
        buffer_sizes_array_elements_num = (max_index / 4) + 1;

        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            array_length_from_constants.buffer_sizes_offset.value(),
            module.symbols.New("tint_storage_buffer_sizes"),
            module.Types().array(module.Types().vec4<core::u32>(),
                                 buffer_sizes_array_elements_num)));
    }
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
    TINT_CHECK_RESULT(core::ir::transform::BindingRemapper(module, remapper_data));

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config{};
        config.use_integer_range_analysis = !options.disable_integer_range_analysis;
        TINT_CHECK_RESULT(core::ir::transform::Robustness(module, config));

        TINT_CHECK_RESULT(core::ir::transform::PreventInfiniteLoops(module));
    }

    {
        core::ir::transform::BinaryPolyfillConfig binary_polyfills{};
        binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        binary_polyfills.bitshift_modulo = true;  // crbug.com/tint/1543
        TINT_CHECK_RESULT(core::ir::transform::BinaryPolyfill(module, binary_polyfills));
    }

    {
        core::ir::transform::BuiltinPolyfillConfig core_polyfills{
            .clamp_int = true,
            .clamp_float = options.workarounds.polyfill_clamp_float,
            .abs_signed_int = true,
            .degrees = true,
            .extract_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck,
            .first_leading_bit = true,
            .first_trailing_bit = true,
            .fwidth_fine = true,
            .insert_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck,
            .radians = true,
            .texture_sample_base_clamp_to_edge_2d_f32 = true,
            .dot_4x8_packed = true,
            .pack_unpack_4x8 = true,
            .pack_4xu8_clamp = true,
            .subgroup_broadcast_f16 = options.workarounds.polyfill_subgroup_broadcast_f16,
        };
        TINT_CHECK_RESULT(core::ir::transform::BuiltinPolyfill(module, core_polyfills));
    }

    {
        core::ir::transform::ConversionPolyfillConfig conversion_polyfills;
        conversion_polyfills.ftoi = true;
        TINT_CHECK_RESULT(core::ir::transform::ConversionPolyfill(module, conversion_polyfills));
    }

    TINT_CHECK_RESULT(core::ir::transform::MultiplanarExternalTexture(module, multiplanar_map));

    // TODO(crbug.com/366291600): Replace ArrayLengthFromUniform with ArrayLengthFromImmediates
    if (array_length_from_constants.ubo_binding) {
        TINT_CHECK_RESULT_UNWRAP(
            array_length_from_uniform_result,
            core::ir::transform::ArrayLengthFromUniform(
                module, BindingPoint{0u, array_length_from_constants.ubo_binding.value()},
                array_length_from_constants.bindpoint_to_size_index));
        raise_result.needs_storage_buffer_sizes =
            array_length_from_uniform_result.needs_storage_buffer_sizes;
    }

    if (array_length_from_constants.buffer_sizes_offset) {
        TINT_IR_ASSERT(module, !array_length_from_constants.ubo_binding);
        TINT_CHECK_RESULT_UNWRAP(array_length_from_immediate_result,
                                 core::ir::transform::ArrayLengthFromImmediates(
                                     module, immediate_data_layout,
                                     array_length_from_constants.buffer_sizes_offset.value(),
                                     buffer_sizes_array_elements_num,
                                     array_length_from_constants.bindpoint_to_size_index));
        raise_result.needs_storage_buffer_sizes =
            array_length_from_immediate_result.needs_storage_buffer_sizes;
    }

    if (!options.disable_workgroup_init) {
        TINT_CHECK_RESULT(core::ir::transform::ZeroInitWorkgroupMemory(module));
    }

    TINT_CHECK_RESULT(core::ir::transform::PreservePadding(module));
    TINT_CHECK_RESULT(core::ir::transform::VectorizeScalarMatrixConstructors(module));
    TINT_CHECK_RESULT(core::ir::transform::RemoveContinueInSwitch(module));

    // DemoteToHelper must come before any transform that introduces non-core instructions.
    if (!options.extensions.disable_demote_to_helper) {
        TINT_CHECK_RESULT(core::ir::transform::DemoteToHelper(module));
    }

    // ConvertPrintToLog must come before ShaderIO as it may introduce entry point builtins.
    TINT_CHECK_RESULT(raise::ConvertPrintToLog(module));

    TINT_CHECK_RESULT(raise::ShaderIO(
        module, raise::ShaderIOConfig{immediate_data_layout, options.emit_vertex_point_size,
                                      options.fixed_sample_mask, options.depth_range_offsets}));

    raise::FixTypeLayoutOptions fix_type_layout_options{
        .replace_bool_with_u32 = options.workarounds.replace_workgroup_bool_with_u32,
    };
    TINT_CHECK_RESULT(raise::FixTypeLayout(module, fix_type_layout_options));

    TINT_CHECK_RESULT(raise::SimdBallot(module));

    // ArgumentBuffers must come before ModuleScopeVars
    if (options.use_argument_buffers) {
        raise::ArgumentBuffersConfig cfg{
            .group_to_argument_buffer_info = std::move(options.group_to_argument_buffer_info),
        };

        if (options.immediate_binding_point) {
            cfg.skip_bindings.insert(options.immediate_binding_point.value());
        }

        if (array_length_from_constants.ubo_binding) {
            cfg.skip_bindings.insert(
                BindingPoint{0u, array_length_from_constants.ubo_binding.value()});
        }

        if (options.vertex_pulling_config) {
            auto group = options.vertex_pulling_config->pulling_group;
            for (uint32_t i = 0; i < options.vertex_pulling_config->vertex_state.size(); ++i) {
                BindingPoint bp{group, i};
                auto iter = remapper_data.find(bp);
                if (iter != remapper_data.end()) {
                    bp = iter->second;
                }
                cfg.skip_bindings.insert(bp);
            }
        }
        TINT_CHECK_RESULT(raise::ArgumentBuffers(module, cfg));
    }

    // ChangeImmediateToUniform must come before ModuleScopeVars
    {
        core::ir::transform::ChangeImmediateToUniformConfig config = {
            .immediate_binding_point = options.immediate_binding_point,
        };
        TINT_CHECK_RESULT(core::ir::transform::ChangeImmediateToUniform(module, config));
    }

    TINT_CHECK_RESULT(raise::ModuleScopeVars(module));

    TINT_CHECK_RESULT(raise::BinaryPolyfill(module));
    TINT_CHECK_RESULT(raise::BuiltinPolyfill(
        module, {
                    .polyfill_unpack_2x16_snorm = options.workarounds.polyfill_unpack_2x16_snorm,
                    .polyfill_unpack_2x16_unorm = options.workarounds.polyfill_unpack_2x16_unorm,
                    .polyfill_tanh_f16 = options.workarounds.polyfill_tanh_f16,
                }));
    // After 'BuiltinPolyfill' as that transform can introduce signed dot products.
    core::ir::transform::SignedIntegerPolyfillConfig signed_integer_cfg{
        .signed_negation = true, .signed_arithmetic = true, .signed_shiftleft = true};
    TINT_CHECK_RESULT(core::ir::transform::SignedIntegerPolyfill(module, signed_integer_cfg));

    core::ir::transform::BuiltinScalarizeConfig scalarize_config{
        .scalarize_clamp = options.workarounds.scalarize_max_min_clamp,
        .scalarize_max = options.workarounds.scalarize_max_min_clamp,
        .scalarize_min = options.workarounds.scalarize_max_min_clamp,
    };
    TINT_CHECK_RESULT(core::ir::transform::BuiltinScalarize(module, scalarize_config));

    raise::ModuleConstantConfig module_const_config{
        options.workarounds.disable_module_constant_f16};
    TINT_CHECK_RESULT(raise::ModuleConstant(module, module_const_config));

    // These transforms need to be run last as various transforms introduce terminator arguments,
    // naming conflicts, and expressions that need to be explicitly not inlined.
    TINT_CHECK_RESULT(core::ir::transform::RemoveTerminatorArgs(module));
    TINT_CHECK_RESULT(core::ir::transform::RenameConflicts(module));
    {
        core::ir::transform::ValueToLetConfig cfg;
        TINT_CHECK_RESULT(core::ir::transform::ValueToLet(module, cfg));
    }

    return raise_result;
}

}  // namespace tint::msl::writer
