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

#include "WebGPUIndexBuffer.h"

#include "WebGPUBufferBase.h"

#include "DriverBase.h"

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

WebGPUIndexBuffer::WebGPUIndexBuffer(wgpu::Device const& device, const uint8_t elementSize,
        const uint32_t indexCount)
    : HwIndexBuffer{ elementSize, indexCount },
      WebGPUBufferBase{ device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
          elementSize * indexCount, "index_buffer" },
      mIndexFormat{ elementSize == 2 ? wgpu::IndexFormat::Uint16 : wgpu::IndexFormat::Uint32 } {}

} // namespace filament::backend
