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

#include <string>

#include "src/tint/cmd/bench/bench.h"
#include "src/tint/lang/hlsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#include "src/tint/lang/wgsl/reader/reader.h"

namespace tint::hlsl::writer {
namespace {

void GenerateHLSL(benchmark::State& state, std::string input_name) {
    auto res = bench::GetWgslProgram(input_name);
    if (res != Success) {
        state.SkipWithError(res.Failure().reason);
        return;
    }
    for (auto _ : state) {
        // Convert the AST program to an IR module.
        auto ir = tint::wgsl::reader::ProgramToLoweredIR(res->program);
        if (ir != Success) {
            state.SkipWithError(ir.Failure().reason);
            return;
        }

        Options gen_options;
        gen_options.bindings = GenerateBindings(res->program);
        auto gen_res = Generate(ir.Get(), gen_options);
        if (gen_res != Success) {
            state.SkipWithError(gen_res.Failure().reason);
        }
    }
}

void GenerateHLSL_AST(benchmark::State& state, std::string input_name) {
    auto res = bench::GetWgslProgram(input_name);
    if (res != Success) {
        state.SkipWithError(res.Failure().reason);
        return;
    }
    for (auto _ : state) {
        Options gen_options;
        gen_options.bindings = GenerateBindings(res->program);
        auto gen_res = Generate(res->program, gen_options);
        if (gen_res != Success) {
            state.SkipWithError(gen_res.Failure().reason);
        }
    }
}

TINT_BENCHMARK_PROGRAMS(GenerateHLSL);
TINT_BENCHMARK_PROGRAMS(GenerateHLSL_AST);

}  // namespace
}  // namespace tint::hlsl::writer
