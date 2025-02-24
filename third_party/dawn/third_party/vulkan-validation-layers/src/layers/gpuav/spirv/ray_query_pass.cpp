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

#include "ray_query_pass.h"
#include "module.h"
#include <spirv/unified1/spirv.hpp>
#include <iostream>

#include "generated/instrumentation_ray_query_comp.h"

namespace gpuav {
namespace spirv {

// By appending the LinkInfo, it will attempt at linking stage to add the function.
uint32_t RayQueryPass::GetLinkFunctionId() {
    static LinkInfo link_info = {instrumentation_ray_query_comp, instrumentation_ray_query_comp_size, 0, "inst_ray_query"};

    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

uint32_t RayQueryPass::CreateFunctionCall(BasicBlock& block, InstructionIt* inst_it, const InjectionData& injection_data) {
    const uint32_t function_result = module_.TakeNextId();
    const uint32_t function_def = GetLinkFunctionId();
    const uint32_t bool_type = module_.type_manager_.GetTypeBool().Id();

    const uint32_t ray_flags_id = target_instruction_->Operand(2);
    const uint32_t ray_origin_id = target_instruction_->Operand(4);
    const uint32_t ray_tmin_id = target_instruction_->Operand(5);
    const uint32_t ray_direction_id = target_instruction_->Operand(6);
    const uint32_t ray_tmax_id = target_instruction_->Operand(7);

    block.CreateInstruction(spv::OpFunctionCall,
                            {bool_type, function_result, function_def, injection_data.inst_position_id,
                             injection_data.stage_info_id, ray_flags_id, ray_origin_id, ray_tmin_id, ray_direction_id, ray_tmax_id},
                            inst_it);

    return function_result;
}

void RayQueryPass::Reset() { target_instruction_ = nullptr; }

bool RayQueryPass::RequiresInstrumentation(const Function& function, const Instruction& inst) {
    (void)function;
    const uint32_t opcode = inst.Opcode();
    if (opcode != spv::OpRayQueryInitializeKHR) {
        return false;
    }
    target_instruction_ = &inst;
    return true;
}

void RayQueryPass::PrintDebugInfo() const {
    std::cout << "RayQueryPass instrumentation count: " << instrumentations_count_ << '\n';
}

}  // namespace spirv
}  // namespace gpuav