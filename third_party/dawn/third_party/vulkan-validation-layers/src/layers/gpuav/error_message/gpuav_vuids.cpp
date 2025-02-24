/* Copyright (c) 2021-2024 The Khronos Group Inc.
 * Copyright (c) 2021-2024 Valve Corporation
 * Copyright (c) 2021-2024 LunarG, Inc.
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
#include "gpuav/error_message/gpuav_vuids.h"

namespace gpuav {

// clang-format off
struct GpuVuidsCmdDraw : GpuVuid {
    GpuVuidsCmdDraw() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDraw-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDraw-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDraw-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDraw-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDraw-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDraw-None-10068";
    }
};

struct GpuVuidsCmdDrawMultiEXT : GpuVuid {
    GpuVuidsCmdDrawMultiEXT() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMultiEXT-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMultiEXT-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMultiEXT-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMultiEXT-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMultiEXT-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMultiEXT-None-10068";
    }
};

struct GpuVuidsCmdDrawIndexed : GpuVuid {
    GpuVuidsCmdDrawIndexed() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawIndexed-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawIndexed-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawIndexed-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawIndexed-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawIndexed-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawIndexed-None-10068";
    }
};

struct GpuVuidsCmdDrawMultiIndexedEXT : GpuVuid {
    GpuVuidsCmdDrawMultiIndexedEXT() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMultiIndexedEXT-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMultiIndexedEXT-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMultiIndexedEXT-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMultiIndexedEXT-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMultiIndexedEXT-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMultiIndexedEXT-None-10068";
    }
};

struct GpuVuidsCmdDrawIndirect : GpuVuid {
    GpuVuidsCmdDrawIndirect() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawIndirect-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawIndirect-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawIndirect-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawIndirect-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawIndirect-None-08114";
        first_instance_not_zero = "VUID-VkDrawIndirectCommand-firstInstance-00501";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawIndirect-None-10068";
    }
};

struct GpuVuidsCmdDrawIndexedIndirect : GpuVuid {
    GpuVuidsCmdDrawIndexedIndirect() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawIndexedIndirect-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawIndexedIndirect-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawIndexedIndirect-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawIndexedIndirect-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawIndexedIndirect-None-08114";
        first_instance_not_zero = "VUID-VkDrawIndexedIndirectCommand-firstInstance-00554";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawIndexedIndirect-None-10068";
    }
};

struct GpuVuidsCmdDispatch : GpuVuid {
    GpuVuidsCmdDispatch() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDispatch-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDispatch-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDispatch-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDispatch-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDispatch-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDispatch-None-10068";
    }
};

struct GpuVuidsCmdDispatchIndirect : GpuVuid {
    GpuVuidsCmdDispatchIndirect() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDispatchIndirect-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDispatchIndirect-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDispatchIndirect-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDispatchIndirect-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDispatchIndirect-None-08114";
        group_exceeds_device_limit_x = "VUID-VkDispatchIndirectCommand-x-00417";
        group_exceeds_device_limit_y = "VUID-VkDispatchIndirectCommand-y-00418";
        group_exceeds_device_limit_z = "VUID-VkDispatchIndirectCommand-z-00419";
        descriptor_index_oob_10068 = "VUID-vkCmdDispatchIndirect-None-10068";

    }
};

struct GpuVuidsCmdDrawIndirectCount : GpuVuid {
    GpuVuidsCmdDrawIndirectCount() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawIndirectCount-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawIndirectCount-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawIndirectCount-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawIndirectCount-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawIndirectCount-None-08114";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndirectCount-countBuffer-02717";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawIndirectCount-None-10068";
    }
};

struct GpuVuidsCmdDrawIndexedIndirectCount : GpuVuid {
    GpuVuidsCmdDrawIndexedIndirectCount() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawIndexedIndirectCount-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawIndexedIndirectCount-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawIndexedIndirectCount-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawIndexedIndirectCount-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawIndexedIndirectCount-None-08114";
        count_exceeds_device_limit = "VUID-vkCmdDrawIndexedIndirectCount-countBuffer-02717";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawIndexedIndirectCount-None-10068";
    }
};

struct GpuVuidsCmdTraceRaysNV : GpuVuid {
    GpuVuidsCmdTraceRaysNV() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdTraceRaysNV-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdTraceRaysNV-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdTraceRaysNV-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdTraceRaysNV-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdTraceRaysNV-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdTraceRaysNV-None-10068";
    }
};

struct GpuVuidsCmdTraceRaysKHR : GpuVuid {
    GpuVuidsCmdTraceRaysKHR() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdTraceRaysKHR-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdTraceRaysKHR-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdTraceRaysKHR-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdTraceRaysKHR-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdTraceRaysKHR-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdTraceRaysKHR-None-10068";
    }
};

struct GpuVuidsCmdTraceRaysIndirectKHR : GpuVuid {
    GpuVuidsCmdTraceRaysIndirectKHR() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdTraceRaysIndirectKHR-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdTraceRaysIndirectKHR-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdTraceRaysIndirectKHR-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdTraceRaysIndirectKHR-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdTraceRaysIndirectKHR-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdTraceRaysIndirectKHR-None-10068";
    }
};

struct GpuVuidsCmdTraceRaysIndirect2KHR : GpuVuid {
    GpuVuidsCmdTraceRaysIndirect2KHR() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdTraceRaysIndirect2KHR-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdTraceRaysIndirect2KHR-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdTraceRaysIndirect2KHR-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdTraceRaysIndirect2KHR-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdTraceRaysIndirect2KHR-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdTraceRaysIndirect2KHR-None-10068";
    }
};  

struct GpuVuidsCmdDrawMeshTasksNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksNV() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMeshTasksNV-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMeshTasksNV-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMeshTasksNV-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMeshTasksNV-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMeshTasksNV-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMeshTasksNV-None-10068";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectNV() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMeshTasksIndirectNV-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMeshTasksIndirectNV-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMeshTasksIndirectNV-None-10068";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectCountNV : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectCountNV() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMeshTasksIndirectCountNV-None-10068";
        count_exceeds_device_limit = "VUID-vkCmdDrawMeshTasksIndirectCountNV-countBuffer-02717";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawMeshTasksEXT : GpuVuid {
    GpuVuidsCmdDrawMeshTasksEXT() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMeshTasksEXT-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMeshTasksEXT-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMeshTasksEXT-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMeshTasksEXT-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMeshTasksEXT-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMeshTasksEXT-None-10068";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectEXT : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectEXT() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMeshTasksIndirectEXT-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMeshTasksIndirectEXT-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMeshTasksIndirectEXT-None-10068";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawMeshTasksIndirectCountEXT : GpuVuid {
    GpuVuidsCmdDrawMeshTasksIndirectCountEXT() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-None-10068";
        count_exceeds_device_limit = "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02717";
        task_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07322";
        task_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07323";
        task_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07324";
        task_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07325";
        mesh_group_count_exceeds_max_x = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07326";
        mesh_group_count_exceeds_max_y = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07327";
        mesh_group_count_exceeds_max_z = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07328";
        mesh_group_count_exceeds_max_total = "VUID-VkDrawMeshTasksIndirectCommandEXT-TaskEXT-07329";
    }
};

struct GpuVuidsCmdDrawIndirectByteCountEXT : GpuVuid {
    GpuVuidsCmdDrawIndirectByteCountEXT() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDrawIndirectByteCountEXT-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDrawIndirectByteCountEXT-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDrawIndirectByteCountEXT-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDrawIndirectByteCountEXT-None-10068";
    }
};

struct GpuVuidsCmdDispatchBase : GpuVuid {
    GpuVuidsCmdDispatchBase() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdDispatchBase-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdDispatchBase-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdDispatchBase-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdDispatchBase-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdDispatchBase-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdDispatchBase-None-10068";
    }
};

struct GpuVuidsCmdExecuteGeneratedCommandsEXT : GpuVuid {
    GpuVuidsCmdExecuteGeneratedCommandsEXT() : GpuVuid() {
        uniform_access_oob_06935 = "VUID-vkCmdExecuteGeneratedCommandsEXT-uniformBuffers-06935";
        storage_access_oob_06936 = "VUID-vkCmdExecuteGeneratedCommandsEXT-storageBuffers-06936";
        uniform_access_oob_08612 = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08612";
        storage_access_oob_08613 = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08613";
        invalid_descriptor_08114 = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-08114";
        descriptor_index_oob_10068 = "VUID-vkCmdExecuteGeneratedCommandsEXT-None-10068";
    }
};

using Func = vvl::Func;

static const std::map<Func, GpuVuid> &GetGpuVuidsMap() {
// This LUT is created to allow a static listing of each VUID that is covered by drawdispatch commands
static const std::map<Func, GpuVuid> gpu_vuid = {
    {Func::vkCmdDraw, GpuVuidsCmdDraw()},
    {Func::vkCmdDrawMultiEXT, GpuVuidsCmdDrawMultiEXT()},
    {Func::vkCmdDrawIndexed, GpuVuidsCmdDrawIndexed()},
    {Func::vkCmdDrawMultiIndexedEXT, GpuVuidsCmdDrawMultiIndexedEXT()},
    {Func::vkCmdDrawIndirect, GpuVuidsCmdDrawIndirect()},
    {Func::vkCmdDrawIndexedIndirect, GpuVuidsCmdDrawIndexedIndirect()},
    {Func::vkCmdDispatch, GpuVuidsCmdDispatch()},
    {Func::vkCmdDispatchIndirect, GpuVuidsCmdDispatchIndirect()},
    {Func::vkCmdDrawIndirectCount, GpuVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndirectCountKHR, GpuVuidsCmdDrawIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCount, GpuVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdDrawIndexedIndirectCountKHR, GpuVuidsCmdDrawIndexedIndirectCount()},
    {Func::vkCmdTraceRaysNV, GpuVuidsCmdTraceRaysNV()},
    {Func::vkCmdTraceRaysKHR, GpuVuidsCmdTraceRaysKHR()},
    {Func::vkCmdTraceRaysIndirectKHR, GpuVuidsCmdTraceRaysIndirectKHR()},
    {Func::vkCmdTraceRaysIndirect2KHR, GpuVuidsCmdTraceRaysIndirect2KHR()},
    {Func::vkCmdDrawMeshTasksNV, GpuVuidsCmdDrawMeshTasksNV()},
    {Func::vkCmdDrawMeshTasksIndirectNV, GpuVuidsCmdDrawMeshTasksIndirectNV()},
    {Func::vkCmdDrawMeshTasksIndirectCountNV, GpuVuidsCmdDrawMeshTasksIndirectCountNV()},
    {Func::vkCmdDrawMeshTasksEXT, GpuVuidsCmdDrawMeshTasksEXT()},
    {Func::vkCmdDrawMeshTasksIndirectEXT, GpuVuidsCmdDrawMeshTasksIndirectEXT()},
    {Func::vkCmdDrawMeshTasksIndirectCountEXT, GpuVuidsCmdDrawMeshTasksIndirectCountEXT()},
    {Func::vkCmdDrawIndirectByteCountEXT, GpuVuidsCmdDrawIndirectByteCountEXT()},
    {Func::vkCmdDispatchBase, GpuVuidsCmdDispatchBase()},
    {Func::vkCmdDispatchBaseKHR, GpuVuidsCmdDispatchBase()},
    {Func::vkCmdExecuteGeneratedCommandsEXT, GpuVuidsCmdExecuteGeneratedCommandsEXT()},
    // Used if invalid function is used
    {Func::Empty, GpuVuid()}
};
return gpu_vuid;
}

const GpuVuid &GetGpuVuid(Func command) {
    const auto &gpu_vuids_map = GetGpuVuidsMap();
    if (gpu_vuids_map.find(command) != gpu_vuids_map.cend()) {
        return gpu_vuids_map.at(command);
    }
    else {
        return gpu_vuids_map.at(Func::Empty);
    }
}
// clang-format on
}  // namespace gpuav
