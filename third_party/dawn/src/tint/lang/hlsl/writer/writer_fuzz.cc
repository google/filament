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
#include "src/tint/lang/hlsl/validate/validate.h"
#include "src/tint/lang/hlsl/writer/printer/printer.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#include "src/tint/utils/command/command.h"

namespace tint::hlsl::writer {
namespace {

// Fuzzed options used to init tint::hlsl::writer::Options
struct FuzzedOptions {
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: These options should not be reordered or removed as it will change the operation of //
    // pre-existing fuzzer cases. Always append new options to the end of the list.              //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    bool strip_all_names;
    bool disable_robustness;
    bool enable_integer_range_analysis;
    bool disable_workgroup_init;
    bool truncate_interstage_variables;
    bool disable_polyfill_integer_div_mod;
    bool polyfill_reflect_vec2_f32;
    bool polyfill_dot_4x8_packed;
    bool polyfill_pack_unpack_4x8;
    Options::Compiler compiler;
    bool scalarize_max_min_clamp;
    bool polyfill_subgroup_broadcast_f16;
    std::bitset<kMaxInterStageLocations> interstage_locations;
    std::optional<uint32_t> first_index_offset;
    std::optional<uint32_t> first_instance_offset;
    std::optional<uint32_t> num_workgroups_start_offset;
    std::vector<BindingPoint> ignored_by_robustness_transform;
    SubstituteOverridesConfig substitute_overrides_config;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(FuzzedOptions,
                 strip_all_names,
                 disable_robustness,
                 enable_integer_range_analysis,
                 disable_workgroup_init,
                 truncate_interstage_variables,
                 disable_polyfill_integer_div_mod,
                 polyfill_reflect_vec2_f32,
                 polyfill_dot_4x8_packed,
                 polyfill_pack_unpack_4x8,
                 compiler,
                 scalarize_max_min_clamp,
                 polyfill_subgroup_broadcast_f16,
                 interstage_locations,
                 first_index_offset,
                 first_instance_offset,
                 num_workgroups_start_offset,
                 ignored_by_robustness_transform,
                 substitute_overrides_config);
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
    options.truncate_interstage_variables = fuzzed_options.truncate_interstage_variables;
    options.disable_polyfill_integer_div_mod = fuzzed_options.disable_polyfill_integer_div_mod;
    options.workarounds.scalarize_max_min_clamp = fuzzed_options.scalarize_max_min_clamp;
    options.workarounds.polyfill_reflect_vec2_f32 = fuzzed_options.polyfill_reflect_vec2_f32;
    options.workarounds.polyfill_subgroup_broadcast_f16 =
        fuzzed_options.polyfill_subgroup_broadcast_f16;
    options.extensions.polyfill_dot_4x8_packed = fuzzed_options.polyfill_dot_4x8_packed;
    options.extensions.polyfill_pack_unpack_4x8 = fuzzed_options.polyfill_pack_unpack_4x8;
    options.compiler = fuzzed_options.compiler;
    options.interstage_locations = fuzzed_options.interstage_locations;
    options.first_index_offset = fuzzed_options.first_index_offset;
    options.first_instance_offset = fuzzed_options.first_instance_offset;
    options.num_workgroups_start_offset = fuzzed_options.num_workgroups_start_offset;
    options.ignored_by_robustness_transform = fuzzed_options.ignored_by_robustness_transform;
    options.substitute_overrides_config = fuzzed_options.substitute_overrides_config;

    options.bindings = GenerateBindings(module, ep_name, false, false);
    options.immediate_binding_point = BindingPoint(0, 30);

    // Add array_length_from_uniform entries for all storage buffers with runtime sized arrays.
    std::unordered_set<tint::BindingPoint> storage_bindings;
    for (auto* inst : *module.root_block) {
        auto* var = inst->As<core::ir::Var>();
        if (!var->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
            if (auto bp = var->BindingPoint()) {
                if (storage_bindings.insert(bp.value()).second) {
                    options.array_length_from_uniform.bindpoint_to_size_index.emplace(
                        bp.value(), static_cast<uint32_t>(storage_bindings.size() - 1));
                }
            }
        }
    }
    options.array_length_from_uniform.buffer_sizes_offset = 0x800;

    TINT_CHECK_RESULT_UNWRAP(output, Generate(module, options));
    if (context.options.dump) {
        std::cout << "Dumping generated HLSL:\n" << output.hlsl << "\n";
    }

    // Run DXC against generated HLSL in order to fuzz it. Note that we ignore whether it succeeds
    // as DXC may fail on valid HLSL emitted by Tint. For example: post optimization infinite loops
    // will fail to compile, but these are beyond Tint's analysis capabilities.
    const char* dxc_path = validate::kDxcDLLName;
    if (!context.options.dxc.empty()) {
        dxc_path = context.options.dxc.c_str();
    }
    auto dxc = tint::Command::LookPath(dxc_path);
    if (dxc.Found()) {
        uint32_t hlsl_shader_model = 66;
        bool require_16bit_types = true;
        [[maybe_unused]] auto validate_res = validate::ValidateUsingDXC(
            dxc.Path(), output.hlsl, output.entry_point_name, output.pipeline_stage,
            require_16bit_types, hlsl_shader_model);
    }

    return Success;
}

}  // namespace
}  // namespace tint::hlsl::writer

TINT_IR_MODULE_FUZZER(tint::hlsl::writer::IRFuzzer,
                      tint::core::ir::Capabilities{},
                      tint::hlsl::writer::kPrinterCapabilities);
