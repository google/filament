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
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/single_entry_point.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/glsl/writer/writer.h"

namespace tint::glsl::writer {
namespace {

Options GenerateOptions(core::ir::Module& module) {
    Options options;
    options.version = Version(Version::Standard::kES, 3, 1);
    options.disable_robustness = false;
    options.disable_workgroup_init = false;
    options.disable_polyfill_integer_div_mod = false;
    options.bindings = GenerateBindings(module);

    // Leave some room for user-declared push constants.
    uint32_t next_push_constant_offset = 0x800;
    auto builtin_push_constant = [&next_push_constant_offset] {
        auto offset = next_push_constant_offset;
        next_push_constant_offset += 4;
        return offset;
    };

    // Set offsets for push constants used for certain builtins.
    for (auto& func : module.functions) {
        if (!func->IsEntryPoint()) {
            continue;
        }

        // vertex_index and instance_index use push constants for offsets if used.
        for (auto* param : func->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    if (member->Attributes().builtin == core::BuiltinValue::kVertexIndex) {
                        options.first_vertex_offset = builtin_push_constant();
                    } else if (member->Attributes().builtin == core::BuiltinValue::kInstanceIndex) {
                        options.first_vertex_offset = builtin_push_constant();
                    }
                }
            } else {
                if (param->Builtin() == core::BuiltinValue::kVertexIndex) {
                    options.first_vertex_offset = builtin_push_constant();
                } else if (param->Builtin() == core::BuiltinValue::kInstanceIndex) {
                    options.first_vertex_offset = builtin_push_constant();
                }
            }
        }

        // frag_depth uses push constants for min and max clamp values if used.
        if (auto* str = func->ReturnType()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                if (member->Attributes().builtin == core::BuiltinValue::kFragDepth) {
                    options.depth_range_offsets = {
                        builtin_push_constant(),
                        builtin_push_constant(),
                    };
                }
            }
        } else {
            if (func->ReturnBuiltin() == core::BuiltinValue::kFragDepth) {
                options.depth_range_offsets = {
                    builtin_push_constant(),
                    builtin_push_constant(),
                };
            }
        }
    }

    return options;
}

Result<SuccessType> IRFuzzer(core::ir::Module& module, const fuzz::ir::Context& context) {
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
    if (entry_point) {
        auto name = module.NameOf(entry_point).NameView();
        TINT_ASSERT(core::ir::transform::SingleEntryPoint(module, name) == Success);
    }

    // TODO(377391551): Enable fuzzing of options.
    auto options = GenerateOptions(module);

    auto check = CanGenerate(module, options);
    if (check != Success) {
        return Failure{check.Failure().reason};
    }

    auto output = Generate(module, options, "");

    if (output == Success && context.options.dump) {
        std::cout << "Dumping generated GLSL:\n" << output->glsl << "\n";
    }

    return Success;
}

}  // namespace
}  // namespace tint::glsl::writer

TINT_IR_MODULE_FUZZER(tint::glsl::writer::IRFuzzer,
                      tint::core::ir::Capabilities{},
                      tint::core::ir::Capabilities{
                          tint::core::ir::Capability::kAllowHandleVarsWithoutBindings});
