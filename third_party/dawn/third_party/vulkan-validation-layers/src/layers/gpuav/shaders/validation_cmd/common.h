// Copyright (c) 2023-2024 The Khronos Group Inc.
// Copyright (c) 2023-2024 Valve Corporation
// Copyright (c) 2023-2024 LunarG, Inc.
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

#include "gpuav_error_header.h"
#include "gpuav_shaders_constants.h"

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagErrorBuffer) buffer ErrorBuffer {
    uint flags;
    uint errors_count;
    uint errors_buffer[];
};

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagActionIndex) readonly buffer ActionIndexBuffer {
    uint action_index[];
};

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagCmdResourceIndex) readonly buffer ResourceIndexBuffer {
    uint resource_index[];
};

layout(set = kDiagCommonDescriptorSet, binding = kBindingDiagCmdErrorsCount) buffer CmdErrorsCountBuffer {
    uint cmd_errors_count[];
};

bool MaxCmdErrorsCountReached() {
    const uint cmd_id = resource_index[0];
    const uint cmd_errors_count = atomicAdd(cmd_errors_count[cmd_id], 1);
    return cmd_errors_count >= kMaxErrorsPerCmd;
}

void GpuavLogError4(uint error_group, uint error_sub_code, uint param_0, uint param_1, uint param_2, uint param_3) {
    if (MaxCmdErrorsCountReached()) return;

    uint vo_idx = atomicAdd(errors_count, kErrorRecordSize);
    const bool errors_buffer_filled = (vo_idx + kErrorRecordSize) > errors_buffer.length();
    if (errors_buffer_filled) return;

    errors_buffer[vo_idx + kHeaderErrorRecordSizeOffset] = kErrorRecordSize;
    errors_buffer[vo_idx + kHeaderActionIdOffset] = action_index[0];
    errors_buffer[vo_idx + kHeaderCommandResourceIdOffset] = resource_index[0];
    errors_buffer[vo_idx + kHeaderErrorGroupOffset] = error_group;
    errors_buffer[vo_idx + kHeaderErrorSubCodeOffset] = error_sub_code;

    errors_buffer[vo_idx + kPreActionParamOffset_0] = param_0;
    errors_buffer[vo_idx + kPreActionParamOffset_1] = param_1;
    errors_buffer[vo_idx + kPreActionParamOffset_2] = param_2;
    errors_buffer[vo_idx + kPreActionParamOffset_3] = param_3;
}

void GpuavLogError2(uint error_group, uint error_sub_code, uint param_0, uint param_1) {
    GpuavLogError4(error_group, error_sub_code, param_0, param_1, 0, 0);
}
