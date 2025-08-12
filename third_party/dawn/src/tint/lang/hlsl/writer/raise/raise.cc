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

#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/add_empty_entry_point.h"
#include "src/tint/lang/core/ir/transform/array_length_from_uniform.h"
#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/builtin_scalarize.h"
#include "src/tint/lang/core/ir/transform/change_immediate_to_uniform.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/direct_variable_access.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"
#include "src/tint/lang/core/ir/transform/remove_continue_in_switch.h"
#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"
#include "src/tint/lang/core/ir/transform/rename_conflicts.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/signed_integer_polyfill.h"
#include "src/tint/lang/core/ir/transform/value_to_let.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/hlsl/writer/common/option_helpers.h"
#include "src/tint/lang/hlsl/writer/common/options.h"
#include "src/tint/lang/hlsl/writer/raise/binary_polyfill.h"
#include "src/tint/lang/hlsl/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/hlsl/writer/raise/decompose_storage_access.h"
#include "src/tint/lang/hlsl/writer/raise/decompose_uniform_access.h"
#include "src/tint/lang/hlsl/writer/raise/localize_struct_array_assignment.h"
#include "src/tint/lang/hlsl/writer/raise/pixel_local.h"
#include "src/tint/lang/hlsl/writer/raise/promote_initializers.h"
#include "src/tint/lang/hlsl/writer/raise/replace_default_only_switch.h"
#include "src/tint/lang/hlsl/writer/raise/replace_non_indexable_mat_vec_stores.h"
#include "src/tint/lang/hlsl/writer/raise/shader_io.h"

namespace tint::hlsl::writer {

Result<SuccessType> Raise(core::ir::Module& module, const Options& options) {
#define RUN_TRANSFORM(name, ...)         \
    do {                                 \
        auto result = name(__VA_ARGS__); \
        if (result != Success) {         \
            return result.Failure();     \
        }                                \
    } while (false)

    // PrepareImmediateData must come before any transform that needs internal push constants.
    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    if (options.first_index_offset) {
        immediate_data_config.AddInternalImmediateData(
            options.first_index_offset.value(), module.symbols.New("tint_first_index_offset"),
            module.Types().u32());
    }

    if (options.first_instance_offset) {
        immediate_data_config.AddInternalImmediateData(
            options.first_instance_offset.value(), module.symbols.New("tint_first_instance_offset"),
            module.Types().u32());
    }

    if (options.num_workgroups_start_offset) {
        immediate_data_config.AddInternalImmediateData(
            options.num_workgroups_start_offset.value(),
            module.symbols.New("tint_num_workgroups_start_offset"),
            module.Types().vec3(module.Types().u32()));
    }
    auto immediate_data_layout =
        core::ir::transform::PrepareImmediateData(module, immediate_data_config);
    if (immediate_data_layout != Success) {
        return immediate_data_layout.Failure();
    }

    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    ArrayLengthFromUniformOptions array_length_from_uniform_options{};
    PopulateBindingRelatedOptions(options, remapper_data, multiplanar_map,
                                  array_length_from_uniform_options);

    RUN_TRANSFORM(core::ir::transform::BindingRemapper, module, remapper_data);
    RUN_TRANSFORM(core::ir::transform::MultiplanarExternalTexture, module, multiplanar_map);

    // `LocalizeStructArrayAssignment` and `ReplaceNonIndexableMatVecStores` may insert additional
    // expressions before the assignments to arrays or vectors so it is better to add them before
    // `Robustness`.
    if (options.compiler == Options::Compiler::kFXC) {
        RUN_TRANSFORM(raise::LocalizeStructArrayAssignment, module);
        RUN_TRANSFORM(raise::ReplaceNonIndexableMatVecStores, module);
    }

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config{};
        config.bindings_ignored = std::unordered_set<BindingPoint>(
            options.bindings.ignored_by_robustness_transform.cbegin(),
            options.bindings.ignored_by_robustness_transform.cend());

        // Direct3D guarantees to return zero for any resource that is accessed out of bounds, and
        // according to the description of the assembly store_uav_typed, out of bounds addressing
        // means nothing gets written to memory.
        config.clamp_texture = false;

        config.use_integer_range_analysis = options.enable_integer_range_analysis;

        RUN_TRANSFORM(core::ir::transform::Robustness, module, config);

        RUN_TRANSFORM(core::ir::transform::PreventInfiniteLoops, module);
    }

    {
        core::ir::transform::BinaryPolyfillConfig binary_polyfills{};
        binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        binary_polyfills.bitshift_modulo = true;
        RUN_TRANSFORM(core::ir::transform::BinaryPolyfill, module, binary_polyfills);
    }

    {
        core::ir::transform::BuiltinPolyfillConfig core_polyfills{};
        core_polyfills.clamp_int = true;
        core_polyfills.dot_4x8_packed = options.polyfill_dot_4x8_packed;

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
        core_polyfills.pack_unpack_4x8 = options.polyfill_pack_unpack_4x8;
        core_polyfills.radians = true;
        core_polyfills.reflect_vec2_f32 = options.polyfill_reflect_vec2_f32;
        core_polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        core_polyfills.abs_signed_int = true;
        RUN_TRANSFORM(core::ir::transform::BuiltinPolyfill, module, core_polyfills);
    }

    {
        core::ir::transform::ConversionPolyfillConfig conversion_polyfills{};
        conversion_polyfills.ftoi = true;
        RUN_TRANSFORM(core::ir::transform::ConversionPolyfill, module, conversion_polyfills);
    }

    RUN_TRANSFORM(core::ir::transform::AddEmptyEntryPoint, module);

    if (options.compiler == Options::Compiler::kFXC) {
        RUN_TRANSFORM(raise::ReplaceDefaultOnlySwitch, module);
    }

    // ArrayLengthFromUniform must run after Robustness, which introduces arrayLength calls.
    {
        auto result = core::ir::transform::ArrayLengthFromUniform(
            module,
            BindingPoint{array_length_from_uniform_options.ubo_binding.group,
                         array_length_from_uniform_options.ubo_binding.binding},
            array_length_from_uniform_options.bindpoint_to_size_index);
        if (result != Success) {
            return result.Failure();
        }
    }

    if (!options.disable_workgroup_init) {
        // Must run before ShaderIO as it may introduce a builtin parameter (local_invocation_index)
        RUN_TRANSFORM(core::ir::transform::ZeroInitWorkgroupMemory, module);
    }

    const bool pixel_local_enabled = !options.pixel_local.attachments.empty();

    // ShaderIO must be run before DecomposeUniformAccess because it might
    // introduce a uniform buffer for kNumWorkgroups.
    {
        raise::ShaderIOConfig config = {
            .immediate_data_layout = immediate_data_layout.Get(),
            .num_workgroups_binding = options.root_constant_binding_point,
            .first_index_offset_binding = options.root_constant_binding_point,
            .add_input_position_member = pixel_local_enabled,
            .truncate_interstage_variables = options.truncate_interstage_variables,
            .interstage_locations = std::move(options.interstage_locations),
            .first_index_offset = options.first_index_offset,
            .first_instance_offset = options.first_instance_offset,
            .num_workgroups_start_offset = options.num_workgroups_start_offset,
        };

        RUN_TRANSFORM(raise::ShaderIO, module, config);
    }

    // DemoteToHelper must come before any transform that introduces non-core instructions.
    // Run after ShaderIO to ensure the discards are added to the entry point it introduces.
    // TODO(crbug.com/42250787): This is only necessary when FXC is being used.
    if (options.compiler == tint::hlsl::writer::Options::Compiler::kFXC) {
        RUN_TRANSFORM(core::ir::transform::DemoteToHelper, module);
    }
    RUN_TRANSFORM(core::ir::transform::DirectVariableAccess, module,
                  core::ir::transform::DirectVariableAccessOptions{});

    // DecomposeStorageAccess must come after Robustness and DirectVariableAccess
    RUN_TRANSFORM(raise::DecomposeStorageAccess, module);

    // ChangeImmediateToUniformConfig must come before DecomposeUniformAccess(to write correct
    // uniform access instructions) and after DirectVariableAccess(to handle immediate pointers
    // being passed as function parameters).
    {
        core::ir::transform::ChangeImmediateToUniformConfig config = {
            .immediate_binding_point = options.immediate_binding_point,
        };
        RUN_TRANSFORM(core::ir::transform::ChangeImmediateToUniform, module, config);
    }
    // Comes after DecomposeStorageAccess and ChangeImmediateToUniform.
    RUN_TRANSFORM(raise::DecomposeUniformAccess, module);

    // PixelLocal must run after DirectVariableAccess to avoid chasing pointer parameters.
    if (pixel_local_enabled) {
        raise::PixelLocalConfig config;
        config.options = options.pixel_local;
        RUN_TRANSFORM(raise::PixelLocal, module, config);
    }

    RUN_TRANSFORM(raise::BinaryPolyfill, module);

    // TODO(crbug.com/429211395): Resolve unsigned/signed casting issues with DXC.
    constexpr bool kEnableSignedIntegerPolyfill = false;
    if (kEnableSignedIntegerPolyfill) {
        core::ir::transform::SignedIntegerPolyfillConfig signed_integer_cfg{
            .signed_negation = true, .signed_arithmetic = true, .signed_shiftleft = true};
        RUN_TRANSFORM(core::ir::transform::SignedIntegerPolyfill, module, signed_integer_cfg);
    }

    // BuiltinPolyfill must come after BinaryPolyfill and DecomposeStorageAccess as they add
    // builtins
    RUN_TRANSFORM(raise::BuiltinPolyfill, module);
    RUN_TRANSFORM(core::ir::transform::VectorizeScalarMatrixConstructors, module);
    RUN_TRANSFORM(core::ir::transform::RemoveContinueInSwitch, module);

    core::ir::transform::BuiltinScalarizeConfig scalarize_config{
        .scalarize_clamp = options.scalarize_max_min_clamp,
        .scalarize_max = options.scalarize_max_min_clamp,
        .scalarize_min = options.scalarize_max_min_clamp};
    RUN_TRANSFORM(core::ir::transform::BuiltinScalarize, module, scalarize_config);

    // These transforms need to be run last as various transforms introduce terminator arguments,
    // naming conflicts, and expressions that need to be explicitly not inlined.
    RUN_TRANSFORM(core::ir::transform::RemoveTerminatorArgs, module);
    RUN_TRANSFORM(core::ir::transform::RenameConflicts, module);
    {
        core::ir::transform::ValueToLetConfig cfg;
        cfg.replace_pointer_lets = true;
        RUN_TRANSFORM(core::ir::transform::ValueToLet, module, cfg);
    }

    // Anything which runs after this needs to handle `Capabilities::kAllowModuleScopedLets`
    RUN_TRANSFORM(raise::PromoteInitializers, module);

    return Success;
}

}  // namespace tint::hlsl::writer
