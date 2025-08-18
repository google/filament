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
#include "src/tint/lang/core/ir/transform/value_to_let.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/vertex_pulling.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/msl/writer/common/option_helpers.h"
#include "src/tint/lang/msl/writer/raise/argument_buffers.h"
#include "src/tint/lang/msl/writer/raise/binary_polyfill.h"
#include "src/tint/lang/msl/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/msl/writer/raise/convert_print_to_log.h"
#include "src/tint/lang/msl/writer/raise/module_constant.h"
#include "src/tint/lang/msl/writer/raise/module_scope_vars.h"
#include "src/tint/lang/msl/writer/raise/packed_vec3.h"
#include "src/tint/lang/msl/writer/raise/shader_io.h"
#include "src/tint/lang/msl/writer/raise/simd_ballot.h"

namespace tint::msl::writer {

Result<RaiseResult> Raise(core::ir::Module& module, const Options& options) {
#define RUN_TRANSFORM(name, ...)         \
    do {                                 \
        auto result = name(__VA_ARGS__); \
        if (result != Success) {         \
            return result.Failure();     \
        }                                \
    } while (false)

    RaiseResult raise_result;

    // VertexPulling must come before BindingRemapper and Robustness.
    if (options.vertex_pulling_config) {
        RUN_TRANSFORM(core::ir::transform::VertexPulling, module, *options.vertex_pulling_config);
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
    if (options.array_length_from_constants.buffer_sizes_offset) {
        // Find the largest index declared in the map, in order to determine the number of
        // elements needed in the array of buffer sizes. The buffer sizes will be packed into
        // vec4s to satisfy the 16-byte alignment requirement for array elements in uniform
        // buffers.
        uint32_t max_index = 0;
        for (auto& entry : array_length_from_constants.bindpoint_to_size_index) {
            max_index = std::max(max_index, entry.second);
        }
        buffer_sizes_array_elements_num = (max_index / 4) + 1;

        immediate_data_config.AddInternalImmediateData(
            options.array_length_from_constants.buffer_sizes_offset.value(),
            module.symbols.New("tint_storage_buffer_sizes"),
            module.Types().array(module.Types().vec4<core::u32>(),
                                 buffer_sizes_array_elements_num));
    }
    auto immediate_data_layout =
        core::ir::transform::PrepareImmediateData(module, immediate_data_config);
    if (immediate_data_layout != Success) {
        return immediate_data_layout.Failure();
    }
    RUN_TRANSFORM(core::ir::transform::BindingRemapper, module, remapper_data);

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config{};
        config.use_integer_range_analysis = options.enable_integer_range_analysis;
        RUN_TRANSFORM(core::ir::transform::Robustness, module, config);

        RUN_TRANSFORM(core::ir::transform::PreventInfiniteLoops, module);
    }

    {
        core::ir::transform::BinaryPolyfillConfig binary_polyfills{};
        binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        binary_polyfills.bitshift_modulo = true;  // crbug.com/tint/1543
        RUN_TRANSFORM(core::ir::transform::BinaryPolyfill, module, binary_polyfills);
    }

    {
        core::ir::transform::BuiltinPolyfillConfig core_polyfills{};
        core_polyfills.clamp_int = true;
        core_polyfills.degrees = true;
        core_polyfills.dot_4x8_packed = true;
        core_polyfills.extract_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
        core_polyfills.first_leading_bit = true;
        core_polyfills.first_trailing_bit = true;
        core_polyfills.fwidth_fine = true;
        core_polyfills.insert_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
        core_polyfills.pack_unpack_4x8 = true;
        core_polyfills.pack_4xu8_clamp = true;
        core_polyfills.radians = true;
        core_polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        core_polyfills.abs_signed_int = true;
        RUN_TRANSFORM(core::ir::transform::BuiltinPolyfill, module, core_polyfills);
    }

    {
        core::ir::transform::ConversionPolyfillConfig conversion_polyfills;
        conversion_polyfills.ftoi = true;
        RUN_TRANSFORM(core::ir::transform::ConversionPolyfill, module, conversion_polyfills);
    }

    RUN_TRANSFORM(core::ir::transform::MultiplanarExternalTexture, module, multiplanar_map);

    // TODO(crbug.com/366291600): Replace ArrayLengthFromUniform with ArrayLengthFromImmediates
    if (options.array_length_from_constants.ubo_binding) {
        auto array_length_from_uniform_result = core::ir::transform::ArrayLengthFromUniform(
            module, BindingPoint{0u, array_length_from_constants.ubo_binding.value()},
            array_length_from_constants.bindpoint_to_size_index);
        if (array_length_from_uniform_result != Success) {
            return array_length_from_uniform_result.Failure();
        }
        raise_result.needs_storage_buffer_sizes =
            array_length_from_uniform_result->needs_storage_buffer_sizes;
    }

    if (options.array_length_from_constants.buffer_sizes_offset) {
        TINT_ASSERT(!options.array_length_from_constants.ubo_binding);
        auto array_length_from_immediate_result = core::ir::transform::ArrayLengthFromImmediates(
            module, immediate_data_layout.Get(),
            array_length_from_constants.buffer_sizes_offset.value(),
            buffer_sizes_array_elements_num, array_length_from_constants.bindpoint_to_size_index);
        if (array_length_from_immediate_result != Success) {
            return array_length_from_immediate_result.Failure();
        }
        raise_result.needs_storage_buffer_sizes =
            array_length_from_immediate_result->needs_storage_buffer_sizes;
    }

    if (!options.disable_workgroup_init) {
        RUN_TRANSFORM(core::ir::transform::ZeroInitWorkgroupMemory, module);
    }

    RUN_TRANSFORM(core::ir::transform::PreservePadding, module);
    RUN_TRANSFORM(core::ir::transform::VectorizeScalarMatrixConstructors, module);
    RUN_TRANSFORM(core::ir::transform::RemoveContinueInSwitch, module);

    // DemoteToHelper must come before any transform that introduces non-core instructions.
    if (!options.disable_demote_to_helper) {
        RUN_TRANSFORM(core::ir::transform::DemoteToHelper, module);
    }

    // ConvertPrintToLog must come before ShaderIO as it may introduce entry point builtins.
    RUN_TRANSFORM(raise::ConvertPrintToLog, module);

    RUN_TRANSFORM(raise::ShaderIO, module,
                  raise::ShaderIOConfig{options.emit_vertex_point_size, options.fixed_sample_mask});
    RUN_TRANSFORM(raise::PackedVec3, module);
    RUN_TRANSFORM(raise::SimdBallot, module);

    // ArgumentBuffers must come before ModuleScopeVars
    if (options.use_argument_buffers) {
        raise::ArgumentBuffersConfig cfg{
            .group_to_argument_buffer_info = std::move(options.group_to_argument_buffer_info),
        };

        if (options.immediate_binding_point) {
            cfg.skip_bindings.insert(options.immediate_binding_point.value());
        }

        if (options.array_length_from_constants.ubo_binding) {
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
        RUN_TRANSFORM(raise::ArgumentBuffers, module, cfg);
    }

    // ChangeImmediateToUniform must come before ModuleScopeVars
    {
        core::ir::transform::ChangeImmediateToUniformConfig config = {
            .immediate_binding_point = options.immediate_binding_point,
        };
        RUN_TRANSFORM(core::ir::transform::ChangeImmediateToUniform, module, config);
    }

    RUN_TRANSFORM(raise::ModuleScopeVars, module);

    RUN_TRANSFORM(raise::BinaryPolyfill, module);
    RUN_TRANSFORM(raise::BuiltinPolyfill, module);
    // After 'BuiltinPolyfill' as that transform can introduce signed dot products.
    core::ir::transform::SignedIntegerPolyfillConfig signed_integer_cfg{
        .signed_negation = true, .signed_arithmetic = true, .signed_shiftleft = true};
    RUN_TRANSFORM(core::ir::transform::SignedIntegerPolyfill, module, signed_integer_cfg);

    core::ir::transform::BuiltinScalarizeConfig scalarize_config{
        .scalarize_clamp = options.scalarize_max_min_clamp,
        .scalarize_max = options.scalarize_max_min_clamp,
        .scalarize_min = options.scalarize_max_min_clamp,
    };
    RUN_TRANSFORM(core::ir::transform::BuiltinScalarize, module, scalarize_config);

    raise::ModuleConstantConfig module_const_config{options.disable_module_constant_f16};
    RUN_TRANSFORM(raise::ModuleConstant, module, module_const_config);

    // These transforms need to be run last as various transforms introduce terminator arguments,
    // naming conflicts, and expressions that need to be explicitly not inlined.
    RUN_TRANSFORM(core::ir::transform::RemoveTerminatorArgs, module);
    RUN_TRANSFORM(core::ir::transform::RenameConflicts, module);
    {
        core::ir::transform::ValueToLetConfig cfg;
        RUN_TRANSFORM(core::ir::transform::ValueToLet, module, cfg);
    }

    return raise_result;
}

}  // namespace tint::msl::writer
