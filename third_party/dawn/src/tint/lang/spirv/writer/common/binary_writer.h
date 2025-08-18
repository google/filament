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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_BINARY_WRITER_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_BINARY_WRITER_H_

#include <vector>

#include "src/tint/lang/spirv/writer/common/module.h"

namespace tint::spirv::writer {

/// Writer to convert from module to SPIR-V binary.
class BinaryWriter {
  public:
    /// Constructor
    BinaryWriter();
    ~BinaryWriter();

    /// Writes the SPIR-V header.
    /// @param bound the bound to output
    /// @param version the generator version number
    /// @param spirv_version the SPIR-V binary version (default SPIR-V 1.3).
    void WriteHeader(uint32_t bound, uint32_t version = 0, uint32_t spirv_version = 0x10300u);

    /// Writes the given module data into a binary. Note, this does not emit the SPIR-V header. You
    /// **must** call WriteHeader() before WriteModule() if you want the SPIR-V to be emitted.
    /// @param module the module to assemble from
    void WriteModule(const Module& module);

    /// Writes the given instruction into the binary.
    /// @param inst the instruction to assemble
    void WriteInstruction(const Instruction& inst);

    /// @returns the assembled SPIR-V
    const std::vector<uint32_t>& Result() const { return out_; }

    /// @returns the assembled SPIR-V
    std::vector<uint32_t>& Result() { return out_; }

  private:
    void ProcessInstruction(const Instruction& inst);
    void ProcessOp(const Operand& op);

    std::vector<uint32_t> out_;
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_BINARY_WRITER_H_
