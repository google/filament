// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/cmd/bench/bench.h"
#include "src/tint/lang/glsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/glsl/writer/writer.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/reader/reader.h"

namespace tint::glsl::writer {
namespace {

void GenerateGLSL(benchmark::State& state, std::string input_name) {
    auto res = bench::GetWgslProgram(input_name);
    TINT_ASSERT(res == Success) << res.Failure().reason;

    std::vector<std::string> names;
    // Get the list of entry point names.
    for (auto* func : res->program.AST().Functions()) {
        if (func->IsEntryPoint()) {
            names.push_back(func->name->symbol.Name());
        }
    }

    for (auto _ : state) {
        for (auto& name : names) {
            // Convert the AST program to an IR module.
            auto ir = tint::wgsl::reader::ProgramToLoweredIR(res->program);
            TINT_ASSERT(ir == Success) << ir.Failure().reason;

            tint::glsl::writer::Options gen_options = {};
            gen_options.entry_point_name = name;
            {
                auto data = tint::glsl::writer::GenerateBindings(ir.Get(), name);
                gen_options.bindings = std::move(data.bindings);
                gen_options.texture_builtins_from_uniform =
                    std::move(data.texture_builtins_from_uniform);
            }

            // Generate GLSL.
            auto gen_res = Generate(ir.Get(), gen_options);
            TINT_ASSERT(gen_res == Success) << gen_res.Failure().reason;
        }
    }
}

TINT_BENCHMARK_PROGRAMS(GenerateGLSL);

}  // namespace
}  // namespace tint::glsl::writer
