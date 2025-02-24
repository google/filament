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

#include <stdint.h>
#include "inject_conditional_function_pass.h"

namespace gpuav {
namespace spirv {

// Create a pass to instrument physical buffer address checking
// This pass instruments all physical buffer address references to check that
// all referenced bytes fall in a valid buffer.
class BufferDeviceAddressPass : public InjectConditionalFunctionPass {
  public:
    BufferDeviceAddressPass(Module& module) : InjectConditionalFunctionPass(module) {}
    const char* Name() const final { return "BufferDeviceAddressPass"; }
    void PrintDebugInfo() const final;

  private:
    bool RequiresInstrumentation(const Function& function, const Instruction& inst) final;
    uint32_t CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data) final;
    void Reset() final;

    uint32_t link_function_id = 0;
    uint32_t GetLinkFunctionId();

    uint32_t alignment_literal_ = 0;
    uint32_t type_length_ = 0;
};

}  // namespace spirv
}  // namespace gpuav