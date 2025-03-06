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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_INSTRUCTION_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_INSTRUCTION_H_

#include <vector>

#include "spirv/unified1/spirv.hpp11"
#include "src/tint/lang/spirv/writer/common/operand.h"

namespace tint::spirv::writer {

/// A single SPIR-V instruction
class Instruction {
  public:
    /// Constructor
    /// @param op the op to generate
    /// @param operands the operand values for the instruction
    Instruction(spv::Op op, OperandList operands);
    /// Copy Constructor
    Instruction(const Instruction&);
    /// Copy assignment operator
    /// @param other the instruction to copy
    /// @returns the new Instruction
    Instruction& operator=(const Instruction& other);
    /// Destructor
    ~Instruction();

    /// @returns the instructions op
    spv::Op Opcode() const { return op_; }

    /// @returns the instructions operands
    const OperandList& Operands() const { return operands_; }

    /// @returns the number of uint32_t's needed to hold the instruction
    uint32_t WordLength() const;

  private:
    spv::Op op_ = spv::Op::OpNop;
    OperandList operands_;
};

/// A list of instructions
using InstructionList = std::vector<Instruction>;

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_INSTRUCTION_H_
