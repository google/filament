/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "vertex_attribute_fetch_oob.h"
#include "module.h"
#include "generated/instrumentation_vertex_attribute_fetch_oob_vert.h"

#include <iostream>

#include "profiling/profiling.h"

namespace gpuav {
namespace spirv {

uint32_t VertexAttributeFetchOob::GetLinkFunctionId() {
    static LinkInfo link_info = {instrumentation_vertex_attribute_fetch_oob_vert,
                                 instrumentation_vertex_attribute_fetch_oob_vert_size, 0, "inst_vertex_attribute_fetch_oob"};

    if (link_function_id == 0) {
        link_function_id = module_.TakeNextId();
        link_info.function_id = link_function_id;
        module_.link_info_.push_back(link_info);
    }
    return link_function_id;
}

VertexAttributeFetchOob::VertexAttributeFetchOob(Module& module) : Pass(module) {}

bool VertexAttributeFetchOob::Instrument() {
    for (const auto& entry_point_inst : module_.entry_points_) {
        const uint32_t execution_model = entry_point_inst->Word(1);
        if (execution_model != spv::ExecutionModelVertex) continue;

        const uint32_t vertex_shader_entry_point_id = entry_point_inst->Word(2);
        for (const auto& function : module_.functions_) {
            if (function->instrumentation_added_) continue;
            const uint32_t function_id = function->GetDef().Word(2);
            if (vertex_shader_entry_point_id != function_id) continue;

            BasicBlock& first_block = *function->blocks_[0];
            InstructionIt first_injectable_instruction = first_block.GetFirstInjectableInstrution();
            target_instruction_ = first_injectable_instruction->get();

            const uint32_t stage_info_id = GetStageInfo(*function, function->blocks_.begin(), first_injectable_instruction);

            InstructionIt stage_info_inst_it;
            for (auto inst_it = first_block.instructions_.begin(); inst_it != first_block.instructions_.end(); ++inst_it) {
                if ((*inst_it)->ResultId() == stage_info_id) {
                    stage_info_inst_it = inst_it;
                    break;
                }
            }
            ++stage_info_inst_it;

            std::vector<uint32_t> index_validation_inst_words;
            const uint32_t void_type = module_.type_manager_.GetTypeVoid().Id();
            const uint32_t function_result = module_.TakeNextId();
            const uint32_t function_def = GetLinkFunctionId();

            first_block.CreateInstruction(spv::OpFunctionCall, {void_type, function_result, function_def, stage_info_id},
                                          &stage_info_inst_it);

            instrumentation_performed = true;
            return true;
        }
    }
    return false;
}

void VertexAttributeFetchOob::PrintDebugInfo() const {
    std::cout << "VertexAttributeFetchOob instrumentation performed: " << std::boolalpha << instrumentation_performed << '\n';
    VVL_TracyPlot(__FUNCTION__, int64_t(instrumentation_performed));
}

}  // namespace spirv

}  // namespace gpuav
