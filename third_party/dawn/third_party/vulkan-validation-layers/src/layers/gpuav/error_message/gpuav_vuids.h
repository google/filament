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

#include "gpuav/core/gpuav.h"

#pragma once

namespace gpuav {

struct GpuVuid {
    const char* uniform_access_oob_06935 = kVUIDUndefined;
    const char* storage_access_oob_06936 = kVUIDUndefined;
    const char* uniform_access_oob_08612 = kVUIDUndefined;
    const char* storage_access_oob_08613 = kVUIDUndefined;
    const char* invalid_descriptor_08114 = kVUIDUndefined;
    const char* descriptor_index_oob_10068 = kVUIDUndefined;
    const char* count_exceeds_device_limit = kVUIDUndefined;
    const char* first_instance_not_zero = kVUIDUndefined;
    const char* group_exceeds_device_limit_x = kVUIDUndefined;
    const char* group_exceeds_device_limit_y = kVUIDUndefined;
    const char* group_exceeds_device_limit_z = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_x = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_y = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_z = kVUIDUndefined;
    const char* mesh_group_count_exceeds_max_total = kVUIDUndefined;
    const char* task_group_count_exceeds_max_x = kVUIDUndefined;
    const char* task_group_count_exceeds_max_y = kVUIDUndefined;
    const char* task_group_count_exceeds_max_z = kVUIDUndefined;
    const char* task_group_count_exceeds_max_total = kVUIDUndefined;
};

// Getter function to provide kVUIDUndefined in case an invalid function is passed in

const GpuVuid& GetGpuVuid(vvl::Func command);
}  // namespace gpuav
