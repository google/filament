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
#include <spirv/unified1/spirv.hpp>
#include "function_basic_block.h"

namespace gpuav {
namespace spirv {

class Module;
struct Variable;
struct BasicBlock;
struct Type;

// Info we know is the same regardless what pass is consuming the CreateFunctionCall()
struct InjectionData {
    uint32_t stage_info_id;
    uint32_t inst_position_id;
};

// Common helpers for all passes
// The pass takes the Module object and modifies it as needed
class Pass {
  public:
    // Needed to know where an error/warning comes from
    virtual const char* Name() const = 0;
    // Return true if code was instrumented/modified in anyway
    virtual bool Instrument() = 0;
    // Requiring because this becomes important/helpful while debugging
    virtual void PrintDebugInfo() const = 0;
    // Wrapper that each pass can use to start
    bool Run();

    // Finds (and creates if needed) decoration and returns the OpVariable it points to
    const Variable& GetBuiltinVariable(uint32_t built_in);

    // Returns the ID for OpCompositeConstruct it creates
    uint32_t GetStageInfo(Function& function, BasicBlockIt target_block_it, InstructionIt& target_inst_it);

    const Instruction* GetDecoration(uint32_t id, spv::Decoration decoration);
    const Instruction* GetMemberDecoration(uint32_t id, uint32_t member_index, spv::Decoration decoration);

    uint32_t FindTypeByteSize(uint32_t type_id, uint32_t matrix_stride = 0, bool col_major = false, bool in_matrix = false);
    uint32_t GetLastByte(const Type& descriptor_type, std::vector<const Instruction*>& access_chain_insts, BasicBlock& block,
                         InstructionIt* inst_it);
    // Generate SPIR-V needed to help convert things to be uniformly uint32_t
    // If no inst_it is passed in, any new instructions will be added to end of the Block
    uint32_t ConvertTo32(uint32_t id, BasicBlock& block, InstructionIt* inst_it);
    uint32_t CastToUint32(uint32_t id, BasicBlock& block, InstructionIt* inst_it);

  protected:
    Pass(Module& module) : module_(module) {}
    Module& module_;

    // clear values between instrumented instructions
    virtual void Reset() = 0;

    // As various things are modifiying the instruction streams, we need to get back to where we were.
    // (normally set in the RequiresInstrumentation call)
    const Instruction* target_instruction_ = nullptr;
    InstructionIt FindTargetInstruction(BasicBlock& block) const;

    uint32_t instrumentations_count_ = 0;
};

}  // namespace spirv
}  // namespace gpuav