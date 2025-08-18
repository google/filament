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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_FUNCTION_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_FUNCTION_H_

#include <functional>
#include <unordered_map>
#include <vector>

#include "src/tint/lang/spirv/writer/common/instruction.h"

namespace tint::spirv::writer {

/// A SPIR-V function
class Function {
  public:
    using Block = InstructionList;

    /// Constructor for testing purposes
    /// This creates a bad declaration, so won't generate correct SPIR-V
    Function();

    /// Constructor
    /// @param declaration the function declaration
    /// @param label_op the operand for function's entry block label
    /// @param params the function parameters
    Function(const Instruction& declaration,
             const Operand& label_op,
             const InstructionList& params);
    /// Copy constructor
    /// @param other the function to copy
    Function(const Function& other);
    /// Copy assignment operator
    /// @param other the function to copy
    /// @returns the new Function
    Function& operator=(const Function& other);
    /// Destructor
    ~Function();

    /// Iterates over the function call the cb on each instruction
    /// @param cb the callback to call
    void Iterate(std::function<void(const Instruction&)> cb) const;

    /// @returns the declaration
    const Instruction& Declaration() const { return declaration_; }

    /// @returns the label ID for the function entry block
    uint32_t LabelId() const { return std::get<uint32_t>(label_op_); }

    /// Adds an instruction to the instruction list
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void PushInst(spv::Op op, const OperandList& operands) {
        blocks_[current_block_idx_].push_back(Instruction{op, operands});
    }
    /// Adds a new block to the block list
    /// @returns the index of the new block
    size_t AppendBlock(uint32_t spv_id) {
        blocks_.push_back({});

        auto blk_id = blocks_.size() - 1;
        block_id_to_block_[spv_id] = static_cast<uint32_t>(blk_id);
        return blk_id;
    }
    /// Sets the block to insert into
    /// @param idx the index to set
    void SetCurrentBlockIndex(size_t idx) { current_block_idx_ = idx; }

    /// Adds a variable to the variable list
    /// @param operands the operands for the variable
    void PushVar(const OperandList& operands) {
        vars_.push_back(Instruction{spv::Op::OpVariable, operands});
    }
    /// @returns the variable list
    const InstructionList& Variables() const { return vars_; }

    /// @returns the word length of the function
    uint32_t WordLength() const {
        // 1 for the Label and 1 for the FunctionEnd
        uint32_t size = 2 + declaration_.WordLength();

        for (const auto& param : params_) {
            size += param.WordLength();
        }
        for (const auto& var : vars_) {
            size += var.WordLength();
        }
        for (const auto& blk : blocks_) {
            for (const auto& inst : blk) {
                size += inst.WordLength();
            }
        }
        return size;
    }

    /// @returns true if the function has a valid declaration
    explicit operator bool() const { return declaration_.Opcode() == spv::Op::OpFunction; }

  private:
    Instruction declaration_;
    Operand label_op_;
    InstructionList params_;
    InstructionList vars_;
    std::vector<Block> blocks_;
    size_t current_block_idx_ = 0;

    std::unordered_map<uint32_t, uint32_t> block_id_to_block_;
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_FUNCTION_H_
