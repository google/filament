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

#include "src/tint/lang/glsl/writer/raise/raise.h"

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/array_length_from_uniform.h"
#include "src/tint/lang/core/ir/transform/bgra8unorm_polyfill.h"
#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/decompose_access.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/direct_variable_access.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/prepare_immediate_data.h"
#include "src/tint/lang/core/ir/transform/preserve_padding.h"
#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"
#include "src/tint/lang/core/ir/transform/remove_continue_in_switch.h"
#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"
#include "src/tint/lang/core/ir/transform/remove_uniform_vector_component_loads.h"
#include "src/tint/lang/core/ir/transform/rename_conflicts.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/signed_integer_polyfill.h"
#include "src/tint/lang/core/ir/transform/single_entry_point.h"
#include "src/tint/lang/core/ir/transform/std140.h"
#include "src/tint/lang/core/ir/transform/substitute_overrides.h"
#include "src/tint/lang/core/ir/transform/value_to_let.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/glsl/writer/common/option_helpers.h"
#include "src/tint/lang/glsl/writer/raise/binary_polyfill.h"
#include "src/tint/lang/glsl/writer/raise/bitcast_polyfill.h"
#include "src/tint/lang/glsl/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/glsl/writer/raise/offset_first_index.h"
#include "src/tint/lang/glsl/writer/raise/shader_io.h"
#include "src/tint/lang/glsl/writer/raise/texture_builtins_from_uniform.h"
#include "src/tint/lang/glsl/writer/raise/texture_polyfill.h"

namespace tint::glsl::writer {

Result<SuccessType> Raise(core::ir::Module& module, const Options& options) {
    TINT_CHECK_RESULT(core::ir::transform::SingleEntryPoint(module, options.entry_point_name));

    TINT_CHECK_RESULT(
        core::ir::transform::SubstituteOverrides(module, options.substitute_overrides_config));

    // Must come before TextureBuiltinsFromUniform as it may add `textureNumLevels` calls.
    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config{};
        config.use_integer_range_analysis = !options.disable_integer_range_analysis;
        TINT_CHECK_RESULT(core::ir::transform::Robustness(module, config));

        TINT_CHECK_RESULT(core::ir::transform::PreventInfiniteLoops(module));
    }

    // PrepareImmediateData must come before any transform that needs internal immediate data.
    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    if (options.first_instance_offset) {
        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            options.first_instance_offset.value(), module.symbols.New("tint_first_instance"),
            module.Types().u32()));
    }
    if (options.first_vertex_offset) {
        TINT_CHECK_RESULT(immediate_data_config.AddInternalImmediateData(
            options.first_vertex_offset.value(), module.symbols.New("tint_first_vertex"),
            module.Types().u32()));
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

    // Note, this must come after Robustness as it may add `arrayLength`.
    // This also needs to come before binding remapper as Dawn inserts _pre-remapping_ binding
    // information. So, in order to move this later we'd need to update Dawn to send the
    // _post-remapping_ data.
    if (options.use_array_length_from_uniform) {
        TINT_CHECK_RESULT(core::ir::transform::ArrayLengthFromUniform(
            module, options.array_length_from_uniform.ubo_binding,
            options.array_length_from_uniform.bindpoint_to_size_index));
    }

    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    PopulateBindingInfo(options, remapper_data, multiplanar_map);
    TINT_CHECK_RESULT(core::ir::transform::BindingRemapper(module, remapper_data));
    // Capability::kAllowDuplicateBindings needed after BindingRemapper
    {
        core::ir::transform::BinaryPolyfillConfig binary_polyfills{};
        binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        binary_polyfills.bitshift_modulo = true;  // crbug.com/tint/1543
        TINT_CHECK_RESULT(core::ir::transform::BinaryPolyfill(module, binary_polyfills));
    }

    {
        core::ir::transform::BuiltinPolyfillConfig core_polyfills{};
        core_polyfills.clamp_int = true;
        core_polyfills.count_leading_zeros = true;
        core_polyfills.count_trailing_zeros = true;
        core_polyfills.extract_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
        core_polyfills.first_leading_bit = true;
        core_polyfills.first_trailing_bit = true;
        core_polyfills.insert_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
        core_polyfills.saturate = true;
        core_polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        core_polyfills.dot_4x8_packed = true;
        core_polyfills.pack_unpack_4x8 = true;
        core_polyfills.pack_4xu8_clamp = true;
        core_polyfills.abs_signed_int = true;
        TINT_CHECK_RESULT(core::ir::transform::BuiltinPolyfill(module, core_polyfills));
    }

    TINT_CHECK_RESULT(core::ir::transform::Bgra8UnormPolyfill(module));

    {
        core::ir::transform::ConversionPolyfillConfig conversion_polyfills;
        conversion_polyfills.ftoi = true;
        TINT_CHECK_RESULT(core::ir::transform::ConversionPolyfill(module, conversion_polyfills));
    }

    TINT_CHECK_RESULT(core::ir::transform::MultiplanarExternalTexture(module, multiplanar_map));

    // `PreservePadding` must run before `DirectVariableAccess`.
    TINT_CHECK_RESULT(core::ir::transform::PreservePadding(module));

    {
        // This must come after `MultiplanarExternalTexture` as it will insert functions with
        // texture parameters, and also after `PreservePadding` which inserts functions with storage
        // buffer parameters.
        core::ir::transform::DirectVariableAccessOptions dva_config{};
        dva_config.transform_handle = core::ir::transform::HandleTransformLevel::kFull;
        TINT_CHECK_RESULT(core::ir::transform::DirectVariableAccess(module, dva_config));
    }

    if (!options.use_uniform_buffers) {
        // DecomposeAccess must come before BlockDecoratedStructs, which will wrap the
        // uniform variable in a structure. It must come after DirectVariableAccess which removes
        // uniform buffers passed as function parameters.
        core::ir::transform::DecomposeAccessOptions decompose_config{.uniform = true};
        TINT_CHECK_RESULT(core::ir::transform::DecomposeAccess(module, decompose_config));
    } else {
        TINT_CHECK_RESULT(core::ir::transform::Std140(module));
    }
    TINT_CHECK_RESULT(core::ir::transform::BlockDecoratedStructs(module));

    // RemoveUniformVectorComponentLoads is used to work around a Qualcomm driver bug.
    // See crbug.com/452350626.
    TINT_CHECK_RESULT(core::ir::transform::RemoveUniformVectorComponentLoads(module));

    // Note, this must come after remapping as it uses post-remapping indices for its options.
    // Note, this must come after DirectVariableAccess as it doesn't handle tracing through function
    // calls.
    TINT_CHECK_RESULT(
        raise::TextureBuiltinsFromUniform(module, options.texture_builtins_from_uniform));

    if (!options.disable_workgroup_init) {
        TINT_CHECK_RESULT(core::ir::transform::ZeroInitWorkgroupMemory(module));
    }

    // DemoteToHelper must come before any transform that introduces non-core instructions.
    TINT_CHECK_RESULT(core::ir::transform::DemoteToHelper(module));

    TINT_CHECK_RESULT(raise::BinaryPolyfill(module));
    // Must come after zero-init as it will add builtins
    TINT_CHECK_RESULT(raise::BuiltinPolyfill(module));

    {
        // Must come after DirectVariableAccess
        raise::TexturePolyfillConfig tex_config;
        tex_config.placeholder_sampler_bind_point = options.placeholder_sampler_bind_point;
        TINT_CHECK_RESULT(raise::TexturePolyfill(module, tex_config));
    }

    // must come before 'BitcastPolyfill' as this adds bitcasts
    core::ir::transform::SignedIntegerPolyfillConfig signed_integer_cfg{
        .signed_negation = true, .signed_arithmetic = true, .signed_shiftleft = true};
    TINT_CHECK_RESULT(core::ir::transform::SignedIntegerPolyfill(module, signed_integer_cfg));

    TINT_CHECK_RESULT(core::ir::transform::VectorizeScalarMatrixConstructors(module));
    TINT_CHECK_RESULT(core::ir::transform::RemoveContinueInSwitch(module));

    TINT_CHECK_RESULT(raise::ShaderIO(
        module, raise::ShaderIOConfig{immediate_data_layout, options.depth_range_offsets,
                                      options.bgra_swizzle_locations}));

    // Must come after ShaderIO as it operates on module-scope `in` variables.
    TINT_CHECK_RESULT(raise::OffsetFirstIndex(
        module, raise::OffsetFirstIndexConfig{immediate_data_layout, options.first_vertex_offset,
                                              options.first_instance_offset}));

    TINT_CHECK_RESULT(core::ir::transform::DecomposeAccess(
        module, {.immediate = true, .minimum_array_size = options.minimum_immediate_size}));

    // Must come after DecomposeImmediateAccess and BuiltinPolyfill as those can add bitcasts.
    TINT_CHECK_RESULT(raise::BitcastPolyfill(module));

    // These transforms need to be run last as various transforms introduce terminator arguments,
    // naming conflicts, and expressions that need to be explicitly not inlined.
    TINT_CHECK_RESULT(core::ir::transform::RemoveTerminatorArgs(module));
    TINT_CHECK_RESULT(core::ir::transform::RenameConflicts(module));

    {
        core::ir::transform::ValueToLetConfig cfg;
        cfg.replace_pointer_lets = true;
        TINT_CHECK_RESULT(core::ir::transform::ValueToLet(module, cfg));
    }

    return Success;
}

}  // namespace tint::glsl::writer
