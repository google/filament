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

#include <iostream>

#include "src/tint/api/helpers/generate_bindings.h"
#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/msl/writer/printer/printer.h"
#include "src/tint/lang/msl/writer/writer.h"

namespace tint::msl::writer {
namespace {

// Fuzzed options used to init tint::msl::writer::Options
struct FuzzedOptions {
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: These options should not be reordered or removed as it will change the operation of //
    // pre-existing fuzzer cases. Always append new options to the end of the list.              //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    bool strip_all_names;
    bool disable_robustness;
    bool enable_integer_range_analysis;
    bool disable_workgroup_init;
    bool emit_vertex_point_size;
    bool disable_polyfill_integer_div_mod;
    bool scalarize_max_min_clamp;
    bool disable_module_constant_f16;
    bool polyfill_subgroup_broadcast_f16;
    bool polyfill_clamp_float;
    bool polyfill_unpack_2x16_snorm;
    bool polyfill_unpack_2x16_unorm;
    uint32_t fixed_sample_mask;
    std::unordered_map<uint32_t, uint32_t> pixel_local_attachments;
    std::optional<VertexPullingConfig> vertex_pulling_config;
    std::unordered_map<uint32_t, ArgumentBufferInfo> group_to_argument_buffer_info;
    SubstituteOverridesConfig substitute_overrides_config;
    bool polyfill_tanh_f16;
    bool replace_workgroup_bool_with_u32;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(FuzzedOptions,
                 strip_all_names,
                 disable_robustness,
                 enable_integer_range_analysis,
                 disable_workgroup_init,
                 emit_vertex_point_size,
                 disable_polyfill_integer_div_mod,
                 scalarize_max_min_clamp,
                 disable_module_constant_f16,
                 polyfill_subgroup_broadcast_f16,
                 polyfill_clamp_float,
                 polyfill_unpack_2x16_snorm,
                 polyfill_unpack_2x16_unorm,
                 fixed_sample_mask,
                 pixel_local_attachments,
                 vertex_pulling_config,
                 group_to_argument_buffer_info,
                 substitute_overrides_config,
                 polyfill_tanh_f16,
                 replace_workgroup_bool_with_u32);
    TINT_REFLECT_HASH_CODE(FuzzedOptions);
};

Result<SuccessType> IRFuzzer(core::ir::Module& module,
                             const fuzz::ir::Context& context,
                             FuzzedOptions fuzzed_options) {
    // TODO(375388101): We cannot run the backend for every entry point in the module unless we
    // clone the whole module each time, so for now we just generate the first entry point.

    // Strip the module down to a single entry point.
    core::ir::Function* entry_point = nullptr;
    for (auto& func : module.functions) {
        if (func->IsEntryPoint()) {
            entry_point = func;
            break;
        }
    }
    std::string ep_name;
    if (entry_point) {
        ep_name = module.NameOf(entry_point).NameView();
    }
    if (ep_name.empty()) {
        // No entry point, just return success
        return Success;
    }

    // We fuzz options that Dawn will vary depending on the platform and provided toggles.
    // Options that are entirely controlled by Dawn (e.g. binding points) are not fuzzed.
    Options options;
    options.entry_point_name = ep_name;
    options.strip_all_names = fuzzed_options.strip_all_names;
    options.disable_robustness = fuzzed_options.disable_robustness;
    options.disable_integer_range_analysis = !fuzzed_options.enable_integer_range_analysis;
    options.disable_workgroup_init = fuzzed_options.disable_workgroup_init;
    options.emit_vertex_point_size = fuzzed_options.emit_vertex_point_size;
    options.disable_polyfill_integer_div_mod = fuzzed_options.disable_polyfill_integer_div_mod;
    options.use_argument_buffers = true;
    options.workarounds.scalarize_max_min_clamp = fuzzed_options.scalarize_max_min_clamp;
    options.workarounds.disable_module_constant_f16 = fuzzed_options.disable_module_constant_f16;
    options.workarounds.polyfill_subgroup_broadcast_f16 =
        fuzzed_options.polyfill_subgroup_broadcast_f16;
    options.workarounds.polyfill_clamp_float = fuzzed_options.polyfill_clamp_float;
    options.workarounds.polyfill_unpack_2x16_snorm = fuzzed_options.polyfill_unpack_2x16_snorm;
    options.workarounds.polyfill_unpack_2x16_unorm = fuzzed_options.polyfill_unpack_2x16_unorm;
    options.workarounds.polyfill_tanh_f16 = fuzzed_options.polyfill_tanh_f16;
    options.workarounds.replace_workgroup_bool_with_u32 =
        fuzzed_options.replace_workgroup_bool_with_u32;
    options.fixed_sample_mask = fuzzed_options.fixed_sample_mask;
    options.pixel_local_attachments = fuzzed_options.pixel_local_attachments;
    options.vertex_pulling_config = fuzzed_options.vertex_pulling_config;
    options.group_to_argument_buffer_info = fuzzed_options.group_to_argument_buffer_info;
    options.substitute_overrides_config = fuzzed_options.substitute_overrides_config;

    options.bindings = GenerateBindings(module, ep_name, false, false);
    options.immediate_binding_point = BindingPoint(0, 30);

    // Add array_length_from_constants entries for all storage buffers with runtime sized arrays.
    std::unordered_set<tint::BindingPoint> storage_bindings;
    for (auto* inst : *module.root_block) {
        auto* var = inst->As<core::ir::Var>();
        if (!var->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
            if (auto bp = var->BindingPoint()) {
                if (storage_bindings.insert(bp.value()).second) {
                    options.array_length_from_constants.bindpoint_to_size_index.emplace(
                        bp.value(), static_cast<uint32_t>(storage_bindings.size() - 1));
                }
            }
        }
    }
    options.array_length_from_constants.buffer_sizes_offset = 0x800;

    TINT_CHECK_RESULT_UNWRAP(output, Generate(module, options));
    if (context.options.dump) {
        std::cout << "Dumping generated MSL:\n" << output.msl << "\n";
    }

    return Success;
}

}  // namespace
}  // namespace tint::msl::writer

TINT_IR_MODULE_FUZZER(tint::msl::writer::IRFuzzer,
                      tint::core::ir::Capabilities{},
                      tint::msl::writer::kPrinterCapabilities);
