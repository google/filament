/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vulkan/utils/Spirv.h"

#include <utils/compiler.h>   // UTILS_UNUSED_IN_RELEASE, UTILS_FALLTHROUGH
#include <utils/debug.h>      // assert_invariant
#include <utils/FixedCapacityVector.h>

#include <spirv/unified1/spirv.hpp>

#include <cstring>
#include <unordered_map>
#include <vector>

namespace filament::backend::fvkutils {

namespace {

// This function transforms an OpSpecConstant instruction into just a OpConstant instruction.
// Additionally, it will adjust the value of the constant as given by the program.
void getTransformedConstantInst(SpecConstantValue* value, uint32_t* inst) {

    // The first word is the size of the instruction and the instruction type.
    // The second word is the type of the instruction.
    // The third word is the "result id" (i.e. the variable to store into).
    // The fourth word (only for floats and ints) is the literal representing the value.
    // We only want to change the first and fourth words.
    constexpr size_t const OP = 0;
    constexpr size_t const VALUE = 3;

    if (!value) {
        // If a specialization wasn't provided, just switch the opcode.
        uint32_t const op = inst[OP] & 0x0000FFFF;
        if (op == spv::Op::OpSpecConstantFalse) {
            inst[OP] = (3 << 16) | spv::Op::OpConstantFalse;
        } else if (op == spv::Op::OpSpecConstantTrue) {
            inst[OP] = (3 << 16) | spv::Op::OpConstantTrue;
        } else if (op == spv::Op::OpSpecConstant) {
            inst[OP] = (4 << 16) | spv::Op::OpConstant;
        }
    } else if (std::holds_alternative<bool>(*value)) {
        inst[OP] = (3 << 16)
                   | (std::get<bool>(*value) ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse);
    } else if (std::holds_alternative<float>(*value)) {
        float const fval = std::get<float>(*value);
        inst[OP] = (4 << 16) | spv::Op::OpConstant;
        inst[VALUE] = *reinterpret_cast<uint32_t const*>(&fval);
    } else {
        int const ival = std::get<int>(*value);
        inst[OP] = (4 << 16) | spv::Op::OpConstant;
        inst[VALUE] = *reinterpret_cast<uint32_t const*>(&ival);
    }
}

} // anonymous namespace

void workaroundSpecConstant(Program::ShaderBlob const& blob,
        utils::FixedCapacityVector<Program::SpecializationConstant> const& specConstants,
        std::vector<uint32_t>& output) {
    using WordMap = std::unordered_map<uint32_t, uint32_t>;
    using SpecValueMap = std::unordered_map<uint32_t, SpecConstantValue>;
    constexpr size_t HEADER_SIZE = 5;

    WordMap varToIdMap;
    SpecValueMap idToValue;

    for (auto spec: specConstants) {
        idToValue[spec.id] = spec.value;
    }

    // The follow loop will iterate through all the instructions and look for instructions of the
    // form:
    //
    //     OpDecorate %1 SpecId 0
    //     %1 = OpSpecConstantFalse %bool
    //
    // We want to track the mapping between the variable id %1 and the specId 0. And when seeing a
    // %1 as the result, we want to change that instruction to just an OpConsant where the constant
    // value is adjusted by the spec values provided in the program.
    // Additionally, we will turn the SpecId decorations to no-ops.
    size_t const dataSize = blob.size() / 4;

    output.resize(dataSize);
    uint32_t const* data = (uint32_t*) blob.data();
    uint32_t* outputData = output.data();

    memcpy(&outputData[0], &data[0], HEADER_SIZE * 4);
    size_t outputCursor = HEADER_SIZE;

    for (uint32_t cursor = HEADER_SIZE, cursorEnd = dataSize; cursor < cursorEnd;) {
        uint32_t const firstWord = data[cursor];
        uint32_t const wordCount = firstWord >> 16;
        uint32_t const op = firstWord & 0x0000FFFF;

        switch(op) {
            case spv::Op::OpSpecConstant:
            case spv::Op::OpSpecConstantTrue:
            case spv::Op::OpSpecConstantFalse: {
                uint32_t const targetVar = data[cursor + 2];

                UTILS_UNUSED_IN_RELEASE WordMap::const_iterator idItr = varToIdMap.find(targetVar);
                assert_invariant(idItr != varToIdMap.end() &&
                        "Cannot find variable previously decorated with SpecId");

                auto valItr = idToValue.find(idItr->second);

                SpecConstantValue* val = (valItr != idToValue.end()) ? &valItr->second : nullptr;
                std::memcpy(&outputData[outputCursor], &data[cursor], wordCount * 4);
                getTransformedConstantInst(val, &outputData[outputCursor]);
                outputCursor += wordCount;
                break;
            }
            case spv::Op::OpDecorate: {
                if (data[cursor + 2] == spv::Decoration::DecorationSpecId) {
                    uint32_t const targetVar = data[cursor + 1];
                    uint32_t const specId = data[cursor + 3];
                    varToIdMap[targetVar] = specId;

                    // Note these decorations do not need to be written to the output.
                    break;
                }
                // else fallthrough and copy like all other instructions
                UTILS_FALLTHROUGH;
            }
            default:
                std::memcpy(&outputData[outputCursor], &data[cursor], wordCount * 4);
                outputCursor += wordCount;
                break;
        }
        cursor += wordCount;
    }
    output.resize(outputCursor);
}

} // namespace filament::backend::fvkutils
