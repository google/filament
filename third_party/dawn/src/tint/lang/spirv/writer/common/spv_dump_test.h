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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_SPV_DUMP_TEST_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_SPV_DUMP_TEST_H_

#include <string>
#include <vector>

#include "src/tint/lang/spirv/writer/common/module.h"

namespace tint::spirv::writer {

/// Disassembles SPIR-V binary data into its textual form.
/// @param data the SPIR-V binary data
/// @param options the additional SPIR-V disassembler options to use
/// @returns the disassembled SPIR-V string
std::string Disassemble(const std::vector<uint32_t>& data, uint32_t options = 0u);

/// Dumps the given module to a SPIR-V disassembly string
/// @param module the module to convert
/// @returns the module as a SPIR-V disassembly string
std::string DumpModule(Module& module);

/// Dumps the given instruction to a SPIR-V disassembly string
/// @param inst the instruction to dump
/// @returns the instruction as a SPIR-V disassembly string
std::string DumpInstruction(const Instruction& inst);

/// Dumps the given instructions to a SPIR-V disassembly string
/// @param insts the instructions to dump
/// @returns the instruction as a SPIR-V disassembly string
std::string DumpInstructions(const InstructionList& insts);

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_SPV_DUMP_TEST_H_
