/* Copyright (c) 2024-2025 LunarG, Inc.
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
#pragma once

#include "pass.h"

namespace gpuav {
namespace spirv {

// A type of common pass that will inject a function call and link it up later,
// We will have wrap the checks to be safe from bad values crashing things
// For OpStore we will just ignore the store if it is invalid, example:
// Before:
//     bda.data[index] = value;
// After:
//    if (isValid(bda.data, index)) {
//         bda.data[index] = value;
//    }
//
// For OpLoad we replace the value with Zero (via Phi node) if it is invalid, example
// Before:
//     int X = bda.data[index];
//     int Y = bda.data[X];
// After:
//    if (isValid(bda.data, index)) {
//         int X = bda.data[index];
//    } else {
//         int X = 0;
//    }
//    if (isValid(bda.data, X)) {
//         int Y = bda.data[X];
//    } else {
//         int Y = 0;
//    }
class InjectConditionalFunctionPass : public Pass {
  public:
    bool Instrument() final;

  protected:
    InjectConditionalFunctionPass(Module& module);

    BasicBlockIt InjectFunction(Function* function, BasicBlockIt block_it, InstructionIt inst_it,
                                const InjectionData& injection_data);

    // Each pass decides if the instruction should needs to have its function check injected
    virtual bool RequiresInstrumentation(const Function& function, const Instruction& inst) = 0;
    // A callback from the function injection logic.
    // Each pass creates a OpFunctionCall and returns its result id.
    // If |inst_it| is not null, it will update it to instruction post OpFunctionCall
    virtual uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data) = 0;
};

}  // namespace spirv
}  // namespace gpuav