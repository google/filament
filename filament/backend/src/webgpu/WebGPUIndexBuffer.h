/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUINDEXBUFFER_H
#define TNT_FILAMENT_BACKEND_WEBGPUINDEXBUFFER_H

#include "WebGPUBufferBase.h"

#include "DriverBase.h"

#include <cstdint>

namespace wgpu {
class Device;
enum class IndexFormat : uint32_t;
} // namespace wgpu

namespace filament::backend {

class WebGPUIndexBuffer final : public HwIndexBuffer, public WebGPUBufferBase {
public:
    WebGPUIndexBuffer(wgpu::Device const&, uint8_t elementSize, uint32_t indexCount);

    [[nodiscard]] wgpu::IndexFormat getIndexFormat() const { return mIndexFormat; }

private:
    const wgpu::IndexFormat mIndexFormat;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUINDEXBUFFER_H
