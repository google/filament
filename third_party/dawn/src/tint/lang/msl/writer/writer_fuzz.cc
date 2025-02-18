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
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/msl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/msl/writer/writer.h"

namespace tint::msl::writer {
namespace {

Result<SuccessType> IRFuzzer(core::ir::Module& module,
                             const fuzz::ir::Context& context,
                             Options options) {
    options.bindings = GenerateBindings(module);
    options.array_length_from_uniform.ubo_binding = 30;

    // Add array_length_from_uniform entries for all storage buffers with runtime sized arrays.
    std::unordered_set<tint::BindingPoint> storage_bindings;
    for (auto* inst : *module.root_block) {
        auto* var = inst->As<core::ir::Var>();
        if (!var->Result(0)->Type()->UnwrapPtr()->HasFixedFootprint()) {
            if (auto bp = var->BindingPoint()) {
                if (storage_bindings.insert(bp.value()).second) {
                    options.array_length_from_uniform.bindpoint_to_size_index.emplace(
                        bp.value(), static_cast<uint32_t>(storage_bindings.size() - 1));
                }
            }
        }
    }

    auto check = CanGenerate(module, options);
    if (check != Success) {
        return check.Failure();
    }

    auto output = Generate(module, options);

    if (output == Success && context.options.dump) {
        std::cout << "Dumping generated MSL:\n" << output->msl << "\n";
    }

    return Success;
}

}  // namespace
}  // namespace tint::msl::writer

TINT_IR_MODULE_FUZZER(tint::msl::writer::IRFuzzer, tint::core::ir::Capabilities{});
