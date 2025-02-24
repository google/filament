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

#include "src/tint/lang/spirv/reader/reader.h"

#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/spirv/reader/ast_parser/parse.h"
#include "src/tint/lang/spirv/reader/lower/lower.h"
#include "src/tint/lang/spirv/reader/parser/parser.h"

namespace tint::spirv::reader {

Result<core::ir::Module> ReadIR(const std::vector<uint32_t>& input) {
    // Parse the input SPIR-V to the SPIR-V dialect of the IR.
    auto mod = Parse(Slice(input.data(), input.size()));
    if (mod != Success) {
        return mod.Failure();
    }

    // Lower the module to the core dialect of the IR.
    if (auto res = Lower(mod.Get()); res != Success) {
        return std::move(res.Failure());
    }

    return mod;
}

Program Read(const std::vector<uint32_t>& input, const Options& options) {
    return ast_parser::Parse(input, options);
}

}  // namespace tint::spirv::reader
