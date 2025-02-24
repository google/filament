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
#include "pass.h"
#include "interface.h"

namespace gpuav {
namespace spirv {

struct Type;

// Create a pass to instrument NonSemantic.DebugPrintf (GL_EXT_debug_printf) instructions
class DebugPrintfPass : public Pass {
  public:
    DebugPrintfPass(Module& module, std::vector<InternalOnlyDebugPrintf>& debug_printf, uint32_t binding_slot = 0)
        : Pass(module), intenral_only_debug_printf_(debug_printf), binding_slot_(binding_slot) {}
    const char* Name() const final { return "DebugPrintfPass"; }

    bool Instrument() final;
    void PrintDebugInfo() const final;

  private:
    bool RequiresInstrumentation(const Instruction& inst);
    void CreateFunctionCall(BasicBlockIt block_it, InstructionIt* inst_it);
    void CreateFunctionParams(uint32_t argument_id, const Type& argument_type, std::vector<uint32_t>& params, BasicBlock& block,
                              InstructionIt* inst_it);
    void CreateDescriptorSet();
    void CreateBufferWriteFunction(uint32_t argument_count, uint32_t function_id);
    void Reset() final;

    bool Validate(const Function& current_function);

    // for debugging instrumented shaders
    std::vector<InternalOnlyDebugPrintf>& intenral_only_debug_printf_;

    const uint32_t binding_slot_;
    uint32_t ext_import_id_ = 0;

    // <number of arguments in the function call, function id>
    vvl::unordered_map<uint32_t, uint32_t> function_id_map_;
    uint32_t GetLinkFunctionId(uint32_t argument_count);

    uint32_t output_buffer_variable_id_ = 0;

    // Used to detect where 64-bit floats are
    uint32_t double_bitmask_ = 0;
    // Used to detect where signed ints are 8 or 16 bits
    uint32_t signed_8_bitmask_ = 0;
    uint32_t signed_16_bitmask_ = 0;
    // Count number of parameters the CPU will need to print out
    // This expands vectors and accounts for 64-bit parameters
    uint32_t expanded_parameter_count_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
