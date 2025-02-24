// Copyright (c) 2024 The Khronos Group Inc.
// Copyright (c) 2024 Valve Corporation
// Copyright (c) 2024 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// NOTE: This file doesn't contain any entrypoints and should be compiled with the --no-link option for glslang

#version 450
#extension GL_GOOGLE_include_directive : enable

#include "gpuav_error_header.h"
#include "gpuav_shaders_constants.h"
#include "common_descriptor_sets.h"

layout(set = kInstDefaultDescriptorSet, binding = kBindingInstVertexAttributeFetchLimits, std430) 
readonly buffer VertexAttributeFetchLimits {
    uint has_max_vbb_vertex_input_rate;
    uint vertex_attribute_fetch_limit_vertex_input_rate;

    uint has_max_vbb_instance_input_rate;
    uint vertex_attribute_fetch_limit_instance_input_rate;
};

void inst_vertex_attribute_fetch_oob(const uvec4 stage_info)
{
    const uint vertex_index = stage_info[1];
    const uint instance_index = stage_info[2];
    
    bool valid_vertex_attribute_fetch_vertex_input_rate = true;
    bool valid_vertex_attribute_fetch_instance_input_rate = true;

    if (has_max_vbb_vertex_input_rate == 1u) {
        valid_vertex_attribute_fetch_vertex_input_rate = vertex_index < vertex_attribute_fetch_limit_vertex_input_rate;
    }
    if (has_max_vbb_instance_input_rate == 1u) {
        valid_vertex_attribute_fetch_instance_input_rate = instance_index < vertex_attribute_fetch_limit_instance_input_rate;
    }

    if (!valid_vertex_attribute_fetch_vertex_input_rate || !valid_vertex_attribute_fetch_instance_input_rate) {
        const uint cmd_id = inst_cmd_resource_index_buffer.index[0];
        const uint cmd_errors_count = atomicAdd(inst_cmd_errors_count_buffer.errors_count[cmd_id], 1);
        const bool max_cmd_errors_count_reached = cmd_errors_count >= kMaxErrorsPerCmd;

        if (max_cmd_errors_count_reached) return;

        uint write_pos = atomicAdd(inst_errors_buffer.written_count, kErrorRecordSize);
        const bool errors_buffer_not_filled = (write_pos + kErrorRecordSize) <= uint(inst_errors_buffer.data.length());

        if (errors_buffer_not_filled) {
            inst_errors_buffer.data[write_pos + kHeaderErrorRecordSizeOffset] = kErrorRecordSize;
            inst_errors_buffer.data[write_pos + kHeaderShaderIdOffset] = kLinkShaderId;
            inst_errors_buffer.data[write_pos + kHeaderInstructionIdOffset] = 0;// Irrelevant
            inst_errors_buffer.data[write_pos + kHeaderStageIdOffset] = stage_info[0];
            inst_errors_buffer.data[write_pos + kHeaderStageInfoOffset_0] = stage_info[1];
            inst_errors_buffer.data[write_pos + kHeaderStageInfoOffset_1] = stage_info[2];
            inst_errors_buffer.data[write_pos + kHeaderStageInfoOffset_2] = stage_info[3];

            inst_errors_buffer.data[write_pos + kHeaderErrorGroupOffset] = kErrorGroupInstIndexedDraw;
            inst_errors_buffer.data[write_pos + kHeaderErrorSubCodeOffset] = 
                !valid_vertex_attribute_fetch_vertex_input_rate ? 
                kErrorSubCode_IndexedDraw_OOBVertexIndex :
                kErrorSubCode_IndexedDraw_OOBInstanceIndex;

            inst_errors_buffer.data[write_pos + kHeaderActionIdOffset] = inst_action_index_buffer.index[0];
            inst_errors_buffer.data[write_pos + kHeaderCommandResourceIdOffset] = inst_cmd_resource_index_buffer.index[0];
        }   
    }
}
