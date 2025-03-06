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

#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/binary/decode.h"
#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::core::ir::binary {
namespace {

Result<SuccessType> IRBinaryRoundtripFuzzer(core::ir::Module& module, const fuzz::ir::Context&) {
    auto encoded = EncodeToBinary(module);
    if (encoded != Success) {
        // Failing to encode, not ICE'ing, indicates that an internal limit to the IR binary
        // encoding/decoding logic was hit. Due to differences between the AST and IR
        // implementations, there exist corner cases where these internal limits are hit for IR,
        // but not AST.
        return Failure{"Failed to encode module to binary"};
    }

    auto decoded = Decode(encoded->Slice());
    if (decoded != Success) {
        TINT_ICE() << "Decode() failed\n" << decoded.Failure();
    }

    auto in = Disassembler(module).Plain();
    auto out = Disassembler(decoded.Get()).Plain();
    if (in != out) {
        TINT_ICE() << "Roundtrip produced different disassembly\n"
                   << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"
                   << "-=                     In                      =-\n"
                   << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"
                   << in << "\n"
                   << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"
                   << "-=                     Out                     =-\n"
                   << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n"
                   << out << "\n"
                   << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n";
    }
    return Success;
}

}  // namespace
}  // namespace tint::core::ir::binary

TINT_IR_MODULE_FUZZER(tint::core::ir::binary::IRBinaryRoundtripFuzzer,
                      tint::core::ir::Capabilities{});
