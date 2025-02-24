/* Copyright (c) 2024 LunarG, Inc.
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

#include "instruction.h"
#include "generated/spirv_grammar_helper.h"

namespace gpuav {
namespace spirv {

void Instruction::UpdateDebugInfo() {
#ifndef NDEBUG
    d_opcode_ = spv::Op(Opcode());
    d_length_ = Length();
    d_result_id_ = ResultId();
    d_type_id_ = TypeId();
    // the words might not all be filled in yet
    for (uint32_t i = 0; i < words_.size() && i < 12; i++) {
        d_words_[i] = words_[i];
    }
#endif
}

void Instruction::SetResultTypeIndex() {
    const bool has_result = OpcodeHasResult(Opcode());
    if (OpcodeHasType(Opcode())) {
        type_id_index_ = 1;
        operand_index_++;
        if (has_result) {
            result_id_index_ = 2;
            operand_index_++;
        }
    } else if (has_result) {
        result_id_index_ = 1;
        operand_index_++;
    }
}

Instruction::Instruction(spirv_iterator it, uint32_t position)
    : position_index_(position), operand_info_(GetOperandInfo(*it & 0x0ffffu)) {
    words_.emplace_back(*it++);
    words_.reserve(Length());
    for (uint32_t i = 1; i < Length(); i++) {
        words_.emplace_back(*it++);
    }

    SetResultTypeIndex();
    UpdateDebugInfo();
}

Instruction::Instruction(uint32_t length, spv::Op opcode) : operand_info_(GetOperandInfo(opcode)) {
    words_.reserve(length);
    uint32_t first_word = (length << 16) | opcode;
    words_.emplace_back(first_word);

    SetResultTypeIndex();
}

void Instruction::Fill(const std::vector<uint32_t>& words) {
    for (uint32_t word : words) {
        words_.emplace_back(word);
    }
    UpdateDebugInfo();
}

void Instruction::AppendWord(uint32_t word) {
    words_.emplace_back(word);
    const uint32_t new_length = Length() + 1;
    uint32_t first_word = (new_length << 16) | Opcode();
    words_[0] = first_word;
    UpdateDebugInfo();
}

void Instruction::ToBinary(std::vector<uint32_t>& out) {
    for (auto word : words_) {
        out.push_back(word);
    }
}

void Instruction::ReplaceResultId(uint32_t new_result_id) {
    words_[result_id_index_] = new_result_id;
    UpdateDebugInfo();
}

void Instruction::ReplaceOperandId(uint32_t old_word, uint32_t new_word) {
    const uint32_t length = Length();
    uint32_t type_index = 0;
    // Use length as some operands can be optional at the end
    for (uint32_t word_index = operand_index_; word_index < length; word_index++, type_index++) {
        if (words_[word_index] != old_word) {
            continue;
        }

        OperandKind kind = OperandKind::Invalid;
        if (type_index < operand_info_.types.size()) {
            kind = operand_info_.types[type_index];
        } else {
            // If the last operands are a wildcard use the last kind for the remaining words
            kind = operand_info_.types.back();
            if (kind == OperandKind::BitEnum) {
                // ImageOperands may be found, their optional parameters will always have an Id
                const uint32_t image_operand_position = OpcodeImageOperandsPosition(Opcode());
                if (image_operand_position != 0 && word_index > image_operand_position) {
                    kind = OperandKind::Id;
                }
            }
        }

        // insructions like OpPhi will be Composite which are just groups of Ids
        // We are not trying to replace/mess with with Control Flow, so all OperandKind::Label are ignored on purpose
        if (kind == OperandKind::Id || kind == OperandKind::Composite) {
            words_[word_index] = new_word;
            UpdateDebugInfo();
        }
    }
}

bool Instruction::IsArray() const {
    const uint32_t opcode = Opcode();
    return opcode == spv::OpTypeArray || opcode == spv::OpTypeRuntimeArray;
}

bool Instruction::IsAccessChain() const {
    const uint32_t opcode = Opcode();
    return opcode == spv::OpAccessChain || opcode == spv::OpPtrAccessChain || opcode == spv::OpInBoundsAccessChain ||
           opcode == spv::OpInBoundsPtrAccessChain;
}

// The main challenge with linking to functions from 2 modules is the IDs overlap.
// TODO - Use the new generated operand to find the IDs.
void Instruction::ReplaceLinkedId(vvl::unordered_map<uint32_t, uint32_t>& id_swap_map) {
    auto swap = [this, &id_swap_map](uint32_t index) {
        uint32_t old_id = words_[index];
        uint32_t new_id = id_swap_map[old_id];
        assert(new_id != 0);
        words_[index] = new_id;
    };

    auto swap_to_end = [this, swap](uint32_t start_index) {
        for (uint32_t i = start_index; i < Length(); i++) {
            swap(i);
        }
    };

    // Swap all Reference IDs (ignores Result ID)
    switch (Opcode()) {
        case spv::OpCompositeExtract:
        case spv::OpLoad:
        case spv::OpArrayLength:
        case spv::OpBitcast:
        case spv::OpUConvert:
        case spv::OpLogicalNot:
        case spv::OpIsNan:
        case spv::OpIsInf:
        case spv::OpIsFinite:
        case spv::OpConvertFToU:
        case spv::OpConvertFToS:
        case spv::OpConvertSToF:
        case spv::OpConvertUToF:
            swap(1);
            swap(3);
            break;
        case spv::OpFAdd:
        case spv::OpIAdd:
        case spv::OpISub:
        case spv::OpFSub:
        case spv::OpIMul:
        case spv::OpFMul:
        case spv::OpUDiv:
        case spv::OpSDiv:
        case spv::OpFDiv:
        case spv::OpUMod:
        case spv::OpSRem:
        case spv::OpSMod:
        case spv::OpFRem:
        case spv::OpFMod:
        case spv::OpIEqual:
        case spv::OpINotEqual:
        case spv::OpUGreaterThan:
        case spv::OpSGreaterThan:
        case spv::OpUGreaterThanEqual:
        case spv::OpSGreaterThanEqual:
        case spv::OpULessThan:
        case spv::OpSLessThan:
        case spv::OpULessThanEqual:
        case spv::OpSLessThanEqual:
        case spv::OpFOrdEqual:
        case spv::OpFUnordEqual:
        case spv::OpFOrdNotEqual:
        case spv::OpFUnordNotEqual:
        case spv::OpFOrdLessThan:
        case spv::OpFUnordLessThan:
        case spv::OpFOrdGreaterThan:
        case spv::OpFUnordGreaterThan:
        case spv::OpFOrdLessThanEqual:
        case spv::OpFUnordLessThanEqual:
        case spv::OpFOrdGreaterThanEqual:
        case spv::OpFUnordGreaterThanEqual:
        case spv::OpLogicalEqual:
        case spv::OpLogicalNotEqual:
        case spv::OpLogicalOr:
        case spv::OpLogicalAnd:
        case spv::OpShiftRightLogical:
        case spv::OpShiftRightArithmetic:
        case spv::OpShiftLeftLogical:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpBitwiseAnd:
            swap(1);
            swap(3);
            swap(4);
            break;
        case spv::OpStore:
        case spv::OpLoopMerge:
            swap(1);
            swap(2);
            break;
        case spv::OpReturnValue:
        case spv::OpFunctionParameter:
        case spv::OpVariable:  // never use optional initializer
        case spv::OpConstantTrue:
        case spv::OpSpecConstantTrue:
        case spv::OpConstantFalse:
        case spv::OpSpecConstantFalse:
        case spv::OpConstant:
        case spv::OpSpecConstant:
        case spv::OpConstantNull:
        case spv::OpSelectionMerge:
        case spv::OpBranch:
        case spv::OpDecorate:
        case spv::OpMemberDecorate:
            swap(1);
            break;
        case spv::OpTypePointer:
            swap(3);
            break;
        case spv::OpAtomicStore:
        case spv::OpBranchConditional:
            swap_to_end(1);
            break;
        case spv::OpAtomicLoad:
        case spv::OpAtomicExchange:
        case spv::OpAtomicCompareExchange:
        case spv::OpAtomicCompareExchangeWeak:
        case spv::OpAtomicIIncrement:
        case spv::OpAtomicIDecrement:
        case spv::OpAtomicIAdd:
        case spv::OpAtomicISub:
        case spv::OpAtomicSMin:
        case spv::OpAtomicUMin:
        case spv::OpAtomicSMax:
        case spv::OpAtomicUMax:
        case spv::OpAtomicAnd:
        case spv::OpAtomicOr:
        case spv::OpAtomicXor:
        case spv::OpPhi:
        case spv::OpAccessChain:
        case spv::OpConstantComposite:
        case spv::OpSpecConstantComposite:
        case spv::OpSelect:
        case spv::OpCompositeConstruct:
            swap(1);
            swap_to_end(3);
            break;
        case spv::OpTypeStruct:
        case spv::OpTypeFunction:
            swap_to_end(2);
            break;
        case spv::OpExtInst:
            swap(3);
            swap_to_end(5);
            break;
        case spv::OpReturn:
        case spv::OpLabel:
        case spv::OpFunctionEnd:
        case spv::OpExtInstImport:
        case spv::OpString:
            break;  // Instructions aware of, but nothing to swap
        default:
            assert(false && "Need to add support for new instruction");
    }

    UpdateDebugInfo();
}

}  // namespace spirv
}  // namespace gpuav
