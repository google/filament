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

#include "VulkanSpirvUtils.h"

#include <utils/Log.h>

#include <bluevk/BlueVK.h>
#include <spirv/unified1/spirv.hpp>

#include <unordered_map>

namespace filament::backend {

using namespace bluevk;

namespace {

// This function transforms an OpSpecConstant instruction into just a OpConstant instruction.
// Additionally, it will adjust the value of the constant as given by the program.
void getTransformedConstantInst(SpecConstantValue const& value, uint32_t* inst) {

    // The first word is the size of the instruction and the instruction type.
    // The second word is the type of the instruction.
    // The third word is the "result id" (i.e. the variable to store into).
    // The fourth word (only for floats and ints) is the literal representing the value.
    // We only want to change the first and fourth words.
    constexpr size_t const OP = 0;
    constexpr size_t const VALUE = 3;

    if (std::holds_alternative<bool>(value)) {
        inst[OP] = (3 << 16)
                   | (std::get<bool>(value) ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse);
    } else if (std::holds_alternative<float>(value)) {
        float const fval = std::get<float>(value);
        inst[OP] = (4 << 16) | spv::Op::OpConstant;
        inst[VALUE] = *reinterpret_cast<uint32_t const*>(&fval);
    } else {
        int const ival = std::get<int>(value);
        inst[OP] = (4 << 16) | spv::Op::OpConstant;
        inst[VALUE] = *reinterpret_cast<uint32_t const*>(&ival);
    }
}

} // anonymous namespace

void workaroundSpecConstant(Program::ShaderBlob& blob,
        utils::FixedCapacityVector<Program::SpecializationConstant> const& specConstants) {
    using WordMap = std::unordered_map<uint32_t, uint32_t>;
    using SpecValueMap = std::unordered_map<uint32_t, SpecConstantValue>;

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
    uint32_t* outputData = (uint32_t*) blob.data();

    for (uint32_t cursor = 5, cursorEnd = blob.size() / 4; cursor < cursorEnd;) {
        uint32_t const firstWord = outputData[cursor];
        uint32_t const wordCount = firstWord >> 16;
        uint32_t const op = firstWord & 0x0000FFFF;

        switch(op) {
            case spv::Op::OpDecorate: {
                if (outputData[cursor + 2] == spv::Decoration::DecorationSpecId) {
                    uint32_t const targetVar = outputData[cursor + 1];
                    uint32_t const specId = outputData[cursor + 3];
                    varToIdMap[targetVar] = specId;
                    // Now we write noops over the decoration since its no longer needed.
                    for (size_t i = cursor, n = cursor + wordCount; i < n; ++i) {
                        outputData[i] = (1 << 16) | spv::Op::OpNop;
                    }
                }
                break;
            }
            case spv::Op::OpSpecConstant:
            case spv::Op::OpSpecConstantTrue:
            case spv::Op::OpSpecConstantFalse: {
                uint32_t const targetVar = outputData[cursor + 2];

                UTILS_UNUSED_IN_RELEASE WordMap::const_iterator idItr = varToIdMap.find(targetVar);
                assert_invariant(idItr != varToIdMap.end() &&
                        "Cannot find variable previously decorated with SpecId");

                assert_invariant(idToValue.find(idItr->second) != idToValue.end() &&
                        "Spec constant value not provided");

                auto const& val = idToValue[varToIdMap[targetVar]];
                getTransformedConstantInst(val, &outputData[cursor]);
                break;
            }
            default:
                break;
        }
        cursor += wordCount;
    }
}

} // namespace filament::backend
