/* Copyright (c) 2022-2023 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>
#include "state_tracker/shader_instruction.h"
#include "generated/spirv_grammar_helper.h"

namespace spirv {

Instruction::Instruction(std::vector<uint32_t>::const_iterator it) {
    words_.emplace_back(*it++);
    words_.reserve(Length());
    for (uint32_t i = 1; i < Length(); i++) {
        words_.emplace_back(*it++);
    }
    SetResultTypeIndex();
    UpdateDebugInfo();
}

Instruction::Instruction(const uint32_t* it) {
    words_.emplace_back(*it);
    it++;
    words_.reserve(Length());
    for (uint32_t i = 1; i < Length(); i++) {
        words_.emplace_back(*it);
        it++;
    }
    SetResultTypeIndex();
    UpdateDebugInfo();
}

void Instruction::SetResultTypeIndex() {
    const bool has_result = OpcodeHasResult(Opcode());
    if (OpcodeHasType(Opcode())) {
        type_id_index_ = 1;
        if (has_result) {
            result_id_index_ = 2;
        }
    } else if (has_result) {
        result_id_index_ = 1;
    }
}

void Instruction::UpdateDebugInfo() {
#ifndef NDEBUG
    d_opcode_ = std::string(string_SpvOpcode(Opcode()));
    d_length_ = Length();
    d_result_id_ = ResultId();
    d_type_id_ = TypeId();
    for (uint32_t i = 0; i < d_length_ && i < 12; i++) {
        d_words_[i] = words_[i];
    }
#endif
}

std::string Instruction::Describe() const {
    std::ostringstream ss;
    const uint32_t opcode = Opcode();
    const uint32_t length = Length();
    const bool has_result = ResultId() != 0;
    const bool has_type = TypeId() != 0;
    uint32_t operand_offset = 1;  // where to start printing operands
    // common disassembled for SPIR-V is
    // %result = Opcode %result_type %operands
    if (has_result) {
        operand_offset++;
        ss << "%" << (has_type ? Word(2) : Word(1)) << " = ";
    }

    ss << string_SpvOpcode(opcode);

    if (has_type) {
        operand_offset++;
        ss << " %" << Word(1);
    }

    // Exception for some opcode
    if (opcode == spv::OpEntryPoint) {
        ss << " " << string_SpvExecutionModel(Word(1)) << " %" << Word(2) << " [Unknown]";
    } else {
        const OperandInfo& info = GetOperandInfo(opcode);
        const uint32_t operands = static_cast<uint32_t>(info.types.size());
        const uint32_t remaining_words = length - operand_offset;
        for (uint32_t i = 0; i < remaining_words; i++) {
            OperandKind kind = (i < operands) ? info.types[i] : info.types.back();
            if (kind == OperandKind::LiteralString) {
                ss << " [string]";
                break;
            }
            ss << ((kind == OperandKind::Id) ? " %" : " ") << Word(operand_offset + i);
        }
    }

    return ss.str();
}

// While simple, function name provides a more human readable description why Word(3) is used.
//
// The current various uses for constant values (OpAccessChain, OpTypeArray, LocalSize, etc) all have spec langauge making sure they
// are scalar ints. It is also not valid for any of these use cases to have a negative value. While it is valid SPIR-V to use 64-bit
// int, found writting test there is no way to create something valid that also calls this function. So until a use-case is found,
// we can safely assume returning a uint32_t is ok.
uint32_t Instruction::GetConstantValue() const {
    // This should be a OpConstant (not a OpSpecConstant), if this asserts then 2 things are happening
    // 1. This function is being used where we don't actually know it is a constant and is a bug in the validation layers
    // 2. The CreateFoldSpecConstantOpAndCompositePass didn't fully fold everything and is a bug in spirv-opt
    assert(Opcode() == spv::OpConstant);
    return Word(3);
}

// The idea of this function is to not have to constantly lookup which operand for the width
// inst.Word(2) -> inst.GetBitWidth()
uint32_t Instruction::GetBitWidth() const {
    const uint32_t opcode = Opcode();
    uint32_t bit_width = 0;
    switch (opcode) {
        case spv::Op::OpTypeFloat:
        case spv::Op::OpTypeInt:
            bit_width = Word(2);
            break;
        case spv::Op::OpTypeBool:
            // The Spec states:
            // "Boolean values considered as 32-bit integer values for the purpose of this calculation"
            // when getting the size for the limits
            bit_width = 32;
            break;
        default:
            // Most likely the caller is not checking for this being a matrix/array/struct/etc
            // This class only knows a single instruction's information
            assert(false);
            break;
    }
    return bit_width;
}

spv::BuiltIn Instruction::GetBuiltIn() const {
    if (Opcode() == spv::OpDecorate) {
        return static_cast<spv::BuiltIn>(Word(3));
    } else if (Opcode() == spv::OpMemberDecorate) {
        return static_cast<spv::BuiltIn>(Word(4));
    } else {
        assert(false);  // non valid Opcode
        return spv::BuiltInMax;
    }
}

bool Instruction::IsArray() const { return (Opcode() == spv::OpTypeArray || Opcode() == spv::OpTypeRuntimeArray); }

spv::Dim Instruction::FindImageDim() const { return (Opcode() == spv::OpTypeImage) ? (spv::Dim(Word(3))) : spv::DimMax; }

bool Instruction::IsImageArray() const { return (Opcode() == spv::OpTypeImage) && (Word(5) != 0); }

bool Instruction::IsImageMultisampled() const {
    // spirv-val makes sure that the MS operand is only non-zero when possible to be Multisampled
    return (Opcode() == spv::OpTypeImage) && (Word(6) != 0);
}

spv::StorageClass Instruction::StorageClass() const {
    spv::StorageClass storage_class = spv::StorageClassMax;
    switch (Opcode()) {
        case spv::OpTypePointer:
            storage_class = static_cast<spv::StorageClass>(Word(2));
            break;
        case spv::OpTypeForwardPointer:
            storage_class = static_cast<spv::StorageClass>(Word(2));
            break;
        case spv::OpVariable:
            storage_class = static_cast<spv::StorageClass>(Word(3));
            break;

        default:
            break;
    }
    return storage_class;
}

}  // namespace spirv
