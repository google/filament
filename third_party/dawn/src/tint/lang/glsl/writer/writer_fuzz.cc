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

#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/single_entry_point.h"
#include "src/tint/lang/glsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/glsl/writer/printer/printer.h"
#include "src/tint/lang/glsl/writer/writer.h"

namespace tint::glsl::writer {
namespace {

// Fuzzed options used to init tint::glsl::writer::Options
struct FuzzedOptions {
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: These options should not be reordered or removed as it will change the operation of //
    // pre-existing fuzzer cases. Always append new options to the end of the list.              //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    bool strip_all_names;
    bool disable_robustness;
    bool enable_integer_range_analysis;
    bool disable_workgroup_init;
    bool disable_polyfill_integer_div_mod;
    bool use_array_length_from_uniform;
    std::unordered_set<uint32_t> bgra_swizzle_locations;
    SubstituteOverridesConfig substitute_overrides_config;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(FuzzedOptions,
                 strip_all_names,
                 disable_robustness,
                 enable_integer_range_analysis,
                 disable_workgroup_init,
                 disable_polyfill_integer_div_mod,
                 use_array_length_from_uniform,
                 bgra_swizzle_locations,
                 substitute_overrides_config);
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
    options.strip_all_names = fuzzed_options.strip_all_names;
    options.disable_robustness = fuzzed_options.disable_robustness;
    options.disable_integer_range_analysis = !fuzzed_options.enable_integer_range_analysis;
    options.disable_workgroup_init = fuzzed_options.disable_workgroup_init;
    options.disable_polyfill_integer_div_mod = fuzzed_options.disable_polyfill_integer_div_mod;
    options.use_array_length_from_uniform = fuzzed_options.use_array_length_from_uniform;
    options.entry_point_name = ep_name;
    options.bgra_swizzle_locations = fuzzed_options.bgra_swizzle_locations;
    options.substitute_overrides_config = fuzzed_options.substitute_overrides_config;

    options.version = Version(Version::Standard::kES, 3, 1);

    auto data = GenerateBindings(module, ep_name);
    options.bindings = std::move(data.bindings);
    options.texture_builtins_from_uniform = std::move(data.texture_builtins_from_uniform);

    // Leave some room for user-declared immediate data.
    uint32_t next_immediate_offset = 0x800;
    auto builtin_immediate = [&next_immediate_offset] {
        auto offset = next_immediate_offset;
        next_immediate_offset += 4;
        return offset;
    };

    // Set offsets for immediate data used for certain builtins.
    for (auto& func : module.functions) {
        if (!func->IsEntryPoint()) {
            continue;
        }

        // vertex_index and instance_index use immediate data for offsets if used.
        for (auto* param : func->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    if (member->Attributes().builtin == core::BuiltinValue::kVertexIndex) {
                        options.first_vertex_offset = builtin_immediate();
                    } else if (member->Attributes().builtin == core::BuiltinValue::kInstanceIndex) {
                        options.first_vertex_offset = builtin_immediate();
                    }
                }
            } else {
                if (param->Builtin() == core::BuiltinValue::kVertexIndex) {
                    options.first_vertex_offset = builtin_immediate();
                } else if (param->Builtin() == core::BuiltinValue::kInstanceIndex) {
                    options.first_vertex_offset = builtin_immediate();
                }
            }
        }

        // frag_depth uses immediate data for min and max clamp values if used.
        if (auto* str = func->ReturnType()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                if (member->Attributes().builtin == core::BuiltinValue::kFragDepth) {
                    options.depth_range_offsets = {
                        builtin_immediate(),
                        builtin_immediate(),
                    };
                }
            }
        } else {
            if (func->ReturnBuiltin() == core::BuiltinValue::kFragDepth) {
                options.depth_range_offsets = {
                    builtin_immediate(),
                    builtin_immediate(),
                };
            }
        }
    }

    TINT_CHECK_RESULT_UNWRAP(output, Generate(module, options));
    if (context.options.dump) {
        std::cout << "Dumping generated GLSL:\n" << output.glsl << "\n";
    }

    return Success;
}

}  // namespace
}  // namespace tint::glsl::writer

TINT_IR_MODULE_FUZZER(tint::glsl::writer::IRFuzzer,
                      tint::core::ir::Capabilities{},
                      tint::glsl::writer::kPrinterCapabilities);
