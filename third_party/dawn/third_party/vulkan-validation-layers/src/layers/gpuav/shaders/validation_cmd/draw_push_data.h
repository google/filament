// Copyright (c) 2021-2025 The Khronos Group Inc.
// Copyright (c) 2021-2025 Valve Corporation
// Copyright (c) 2021-2025 LunarG, Inc.
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
// Values used between the GLSL shaders and the GPU-AV logic

// NOTE: This header is included by the instrumentation shaders and glslang doesn't support #pragma once
#ifndef GPU_SHADERS_DRAW_PUSH_DATA_H
#define GPU_SHADERS_DRAW_PUSH_DATA_H

#ifdef __cplusplus
namespace gpuav {
namespace glsl {
using uint = unsigned int;
#else
#if defined(GL_ARB_gpu_shader_int64)
#extension GL_ARB_gpu_shader_int64 : require
#else
#error No extension available for 64-bit integers.
#endif
#endif

// Bindings for all pre draw types
const uint kPreDrawBinding_IndirectBuffer = 0;
const uint kPreDrawBinding_CountBuffer = 1;
const uint kPreDrawBinding_IndexBuffer = 2;

const uint kIndexedIndirectDrawFlags_DrawCountFromBuffer = uint(1) << 0;
struct DrawIndexedIndirectIndexBufferPushData {
    uint flags;
    uint draw_cmds_stride_dwords;
    uint bound_index_buffer_indices_count;  // Number of indices in the index buffer, taking index type in account. NOT a byte size.
    uint cpu_draw_count;
    uint draw_indexed_indirect_cmds_buffer_dwords_offset;
    uint count_buffer_dwords_offset;
};

const uint kDrawMeshFlags_DrawCountFromBuffer = uint(1) << 0;
struct DrawMeshPushData {
    uint flags;
    uint draw_cmds_stride_dwords;
    uint cpu_draw_count;
    uint max_workgroup_count_x;
    uint max_workgroup_count_y;
    uint max_workgroup_count_z;
    uint max_workgroup_total_count;
    uint draw_buffer_dwords_offset;
    uint count_buffer_dwords_offset;
};

const uint kFirstInstanceFlags_DrawCountFromBuffer = uint(1) << 0;
struct FirstInstancePushData {
    uint flags;
    uint draw_cmds_stride_dwords;
    uint cpu_draw_count;
    uint first_instance_member_pos;
    uint draw_buffer_dwords_offset;
    uint count_buffer_dwords_offset;
};

struct CountBufferPushData {
    uint draw_cmds_byte_stride;
    uint64_t draw_buffer_offset;
    uint64_t draw_buffer_size;
    uint draw_cmd_byte_size;
    uint device_limit_max_draw_indirect_count;
    uint count_buffer_dwords_offset;
};

#ifdef __cplusplus
}  // namespace glsl
}  // namespace gpuav
#endif
#endif
