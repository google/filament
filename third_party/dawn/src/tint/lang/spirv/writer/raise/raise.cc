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

#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/add_empty_entry_point.h"
#include "src/tint/lang/core/ir/transform/bgra8unorm_polyfill.h"
#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/combine_access_instructions.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/direct_variable_access.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/prepare_push_constants.h"
#include "src/tint/lang/core/ir/transform/preserve_padding.h"
#include "src/tint/lang/core/ir/transform/prevent_infinite_loops.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/std140.h"
#include "src/tint/lang/core/ir/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/spirv/writer/common/option_helpers.h"
#include "src/tint/lang/spirv/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/spirv/writer/raise/expand_implicit_splats.h"
#include "src/tint/lang/spirv/writer/raise/handle_matrix_arithmetic.h"
#include "src/tint/lang/spirv/writer/raise/merge_return.h"
#include "src/tint/lang/spirv/writer/raise/pass_matrix_by_pointer.h"
#include "src/tint/lang/spirv/writer/raise/remove_unreachable_in_loop_continuing.h"
#include "src/tint/lang/spirv/writer/raise/shader_io.h"
#include "src/tint/lang/spirv/writer/raise/var_for_dynamic_index.h"

namespace tint::spirv::writer {

Result<SuccessType> Raise(core::ir::Module& module, const Options& options) {
#define RUN_TRANSFORM(name, ...)         \
    do {                                 \
        auto result = name(__VA_ARGS__); \
        if (result != Success) {         \
            return result;               \
        }                                \
    } while (false)

    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    PopulateRemapperAndMultiplanarOptions(options, remapper_data, multiplanar_map);

    RUN_TRANSFORM(core::ir::transform::BindingRemapper, module, remapper_data);

    if (!options.disable_robustness) {
        RUN_TRANSFORM(core::ir::transform::PreventInfiniteLoops, module);
    }

    // PreparePushConstants must come before any transform that needs internal push constants.
    core::ir::transform::PreparePushConstantsConfig push_constant_config;
    if (options.depth_range_offsets) {
        push_constant_config.AddInternalConstant(options.depth_range_offsets.value().min,
                                                 module.symbols.New("tint_frag_depth_min"),
                                                 module.Types().f32());
        push_constant_config.AddInternalConstant(options.depth_range_offsets.value().max,
                                                 module.symbols.New("tint_frag_depth_max"),
                                                 module.Types().f32());
    }
    auto push_constant_layout =
        core::ir::transform::PreparePushConstants(module, push_constant_config);
    if (push_constant_layout != Success) {
        return push_constant_layout.Failure();
    }

    core::ir::transform::BinaryPolyfillConfig binary_polyfills;
    binary_polyfills.bitshift_modulo = true;
    binary_polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
    RUN_TRANSFORM(core::ir::transform::BinaryPolyfill, module, binary_polyfills);

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
    core_polyfills.dot_4x8_packed = options.polyfill_dot_4x8_packed;
    core_polyfills.pack_unpack_4x8 = true;
    core_polyfills.pack_4xu8_clamp = true;
    core_polyfills.pack_unpack_4x8_norm = options.polyfill_pack_unpack_4x8_norm;
    RUN_TRANSFORM(core::ir::transform::BuiltinPolyfill, module, core_polyfills);

    core::ir::transform::ConversionPolyfillConfig conversion_polyfills;
    conversion_polyfills.ftoi = true;
    RUN_TRANSFORM(core::ir::transform::ConversionPolyfill, module, conversion_polyfills);

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config;
        if (options.disable_image_robustness) {
            config.clamp_texture = false;
        }
        config.disable_runtime_sized_array_index_clamping =
            options.disable_runtime_sized_array_index_clamping;
        RUN_TRANSFORM(core::ir::transform::Robustness, module, config);
    }

    RUN_TRANSFORM(core::ir::transform::MultiplanarExternalTexture, module, multiplanar_map);

    if (!options.disable_workgroup_init &&
        !options.use_zero_initialize_workgroup_memory_extension) {
        RUN_TRANSFORM(core::ir::transform::ZeroInitWorkgroupMemory, module);
    }

    // PreservePadding must come before DirectVariableAccess.
    RUN_TRANSFORM(core::ir::transform::PreservePadding, module);

    core::ir::transform::DirectVariableAccessOptions dva_options;
    dva_options.transform_function = true;
    dva_options.transform_private = true;
    RUN_TRANSFORM(core::ir::transform::DirectVariableAccess, module, dva_options);

    if (options.pass_matrix_by_pointer) {
        // PassMatrixByPointer must come after PreservePadding+DirectVariableAccess.
        RUN_TRANSFORM(raise::PassMatrixByPointer, module);
    }

    RUN_TRANSFORM(core::ir::transform::AddEmptyEntryPoint, module);
    RUN_TRANSFORM(core::ir::transform::Bgra8UnormPolyfill, module);
    RUN_TRANSFORM(core::ir::transform::BlockDecoratedStructs, module);
    RUN_TRANSFORM(core::ir::transform::VectorizeScalarMatrixConstructors, module);

    // CombineAccessInstructions must come after DirectVariableAccess and BlockDecoratedStructs.
    // We run this transform as some Qualcomm drivers struggle with partial access chains that
    // produce pointers to matrices.
    RUN_TRANSFORM(core::ir::transform::CombineAccessInstructions, module);

    if (!options.use_demote_to_helper_invocation_extensions) {
        // DemoteToHelper must come before any transform that introduces non-core instructions.
        RUN_TRANSFORM(core::ir::transform::DemoteToHelper, module);
    }

    RUN_TRANSFORM(raise::BuiltinPolyfill, module, options.use_vulkan_memory_model);
    RUN_TRANSFORM(raise::ExpandImplicitSplats, module);
    RUN_TRANSFORM(raise::HandleMatrixArithmetic, module);
    RUN_TRANSFORM(raise::MergeReturn, module);
    RUN_TRANSFORM(raise::RemoveUnreachableInLoopContinuing, module);
    RUN_TRANSFORM(
        raise::ShaderIO, module,
        raise::ShaderIOConfig{push_constant_layout.Get(), options.clamp_frag_depth,
                              options.emit_vertex_point_size, !options.use_storage_input_output_16,
                              options.depth_range_offsets});
    RUN_TRANSFORM(core::ir::transform::Std140, module);
    RUN_TRANSFORM(raise::VarForDynamicIndex, module);

    return Success;
}

}  // namespace tint::spirv::writer
