/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_WEBGPUPUSHCONSTANTDESCRIPTION_H
#define TNT_FILAMENT_BACKEND_WEBGPUPUSHCONSTANTDESCRIPTION_H

#include <backend/Program.h>
#include <backend/DriverEnums.h>

#include <utils/FixedCapacityVector.h>

#include <webgpu/webgpu_cpp.h>

namespace filament::backend {

class WebGPUPushConstantDescription {
public:
    explicit WebGPUPushConstantDescription(Program const& program);

    void setPushConstant(const wgpu::RenderPassEncoder& renderPassEncoder, ShaderStage stage,
            uint8_t index, PushConstantVariant value);

private:
    static constexpr uint32_t ENTRY_SIZE = sizeof(uint32_t);

    struct ConstantDescription {
        uint32_t offset = 0;
        utils::FixedCapacityVector<ConstantType> types;
    };

    ConstantDescription mDescriptions[Program::SHADER_TYPE_COUNT];
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUPUSHCONSTANTDESCRIPTION_H
