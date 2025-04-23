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

#include "src/tint/lang/spirv/writer/writer.h"

#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/spirv/validate/validate.h"
#include "src/tint/lang/spirv/writer/helpers/generate_bindings.h"

namespace tint::spirv::writer {
namespace {

Result<SuccessType> IRFuzzer(core::ir::Module& module, const fuzz::ir::Context&, Options options) {
    auto check = CanGenerate(module, options);
    if (check != Success) {
        return Failure{check.Failure().reason};
    }

    options.bindings = GenerateBindings(module);
    auto output = Generate(module, options);
    if (output != Success) {
        return Failure{output.Failure().reason};
    }

    auto& spirv = output->spirv;
    if (auto res = validate::Validate(Slice(spirv.data(), spirv.size()), SPV_ENV_VULKAN_1_1);
        res != Success) {
        TINT_ICE() << "output of SPIR-V writer failed to validate with SPIR-V Tools\n"
                   << res.Failure() << "\n\n"
                   << "IR:\n"
                   << core::ir::Disassembler(module).Plain();
    }

    return Success;
}

}  // namespace
}  // namespace tint::spirv::writer

TINT_IR_MODULE_FUZZER(tint::spirv::writer::IRFuzzer, tint::core::ir::Capabilities{});
