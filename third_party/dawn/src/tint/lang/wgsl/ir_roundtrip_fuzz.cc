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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader && tint_build_wgsl_writer)

#include <iostream>

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/transform/renamer.h"
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl {

bool CanRun(core::ir::Module& ir) {
    // IRToProgram cannot handle constants whose types are builtin structures, since it would have
    // to reverse-engineer the builtin function call that would produce the right output.
    for (auto* c : ir.constant_values) {
        if (auto* str = c->Type()->As<core::type::Struct>()) {
            // TODO(350778507): Consider using a struct flag for builtin structures instead.
            if (tint::HasPrefix(str->Name().NameView(), "__")) {
                return false;
            }
        }
    }
    return true;
}

void IRRoundtripFuzzer(const tint::Program& program, const fuzz::wgsl::Context& context) {
    if (program.AST().Enables().Any(tint::wgsl::reader::IsUnsupportedByIR)) {
        return;
    }
    auto transformed = tint::wgsl::ApplySubstituteOverrides(program);
    auto& src = transformed ? transformed.value() : program;
    if (!src.IsValid()) {
        return;
    }
    auto ir = tint::wgsl::reader::ProgramToLoweredIR(src);
    if (ir != Success) {
        return;
    }
    if (auto val = core::ir::Validate(ir.Get()); val != Success) {
        TINT_ICE() << val.Failure();
    }

    if (!CanRun(ir.Get())) {
        return;
    }

    if (auto res = tint::wgsl::writer::Raise(ir.Get()); res != Success) {
        TINT_ICE() << res.Failure();
    }

    writer::ProgramOptions program_options;
    program_options.allowed_features = AllowedFeatures::Everything();
    auto dst = tint::wgsl::writer::IRToProgram(ir.Get(), program_options);
    if (!dst.IsValid()) {
        std::cerr << "IR:\n" << core::ir::Disassembler(ir.Get()).Plain() << "\n";
        if (auto result = tint::wgsl::writer::Generate(dst, {}); result == Success) {
            std::cerr << "WGSL:\n" << result->wgsl << "\n\n";
        }
        TINT_ICE() << dst.Diagnostics();
    }

    return;
}

}  // namespace tint::wgsl

TINT_WGSL_PROGRAM_FUZZER(tint::wgsl::IRRoundtripFuzzer);
