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

#include "src/tint/lang/wgsl/reader/reader.h"

#include <limits>
#include <utility>

#include "src/tint/lang/wgsl/reader/lower/lower.h"
#include "src/tint/lang/wgsl/reader/parser/parser.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::wgsl::reader {

Program Parse(const Source::File* file, const Options& options) {
    if (DAWN_UNLIKELY(file->content.data.size() >
                      static_cast<size_t>(std::numeric_limits<uint32_t>::max()))) {
        ProgramBuilder b;
        b.Diagnostics().AddError(tint::Source{}) << "WGSL source must be 0xffffffff bytes or fewer";
        return Program(std::move(b));
    }
    Parser parser(file);
    parser.Parse();
    return resolver::Resolve(parser.builder(), options.allowed_features);
}

Result<core::ir::Module> WgslToIR(const Source::File* file, const Options& options) {
    Program program = Parse(file, options);
    return ProgramToLoweredIR(program);
}

tint::Result<core::ir::Module> ProgramToLoweredIR(const Program& program) {
    auto ir = ProgramToIR(program);
    if (ir != Success) {
        return ir.Failure();
    }

    // Lower from WGSL-dialect to core-dialect
    auto res = Lower(ir.Get());
    if (res != Success) {
        return res.Failure();
    }
    return ir;
}

bool IsUnsupportedByIR(const ast::Enable* enable) {
    for (auto ext : enable->extensions) {
        switch (ext->name) {
            case tint::wgsl::Extension::kChromiumExperimentalFramebufferFetch:
            case tint::wgsl::Extension::kChromiumInternalRelaxedUniformLayout:
                return true;
            default:
                break;
        }
    }
    return false;
}

}  // namespace tint::wgsl::reader
