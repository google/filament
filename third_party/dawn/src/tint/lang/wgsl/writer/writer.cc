// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/writer/writer.h"

#include <memory>

#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"

namespace tint::wgsl::writer {

Result<Output> Generate(const Program& program) {
    Output output;

    // Generate the WGSL code.
    auto impl = std::make_unique<ASTPrinter>(program);
    if (!impl->Generate()) {
        return Failure{impl->Diagnostics().Str()};
    }
    output.wgsl = impl->Result();

    return output;
}

Result<Output> WgslFromIR(core::ir::Module& module, const ProgramOptions& options) {
    auto res = ProgramFromIR(module, options);
    if (res != Success) {
        return res.Failure();
    }
    return Generate(res.Move());
}

Result<Program> ProgramFromIR(core::ir::Module& module, const ProgramOptions& options) {
    // core-dialect -> WGSL-dialect
    if (auto res = Raise(module); res != Success) {
        return res.Failure();
    }

    auto program = IRToProgram(module, options);
    if (!program.IsValid()) {
        return Failure{program.Diagnostics().Str()};
    }

    return program;
}

}  // namespace tint::wgsl::writer
