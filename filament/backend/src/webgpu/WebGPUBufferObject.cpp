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

#include "WebGPUBufferObject.h"

#include "WebGPUBufferBase.h"

#include "DriverBase.h"
#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

namespace {

[[nodiscard]] constexpr wgpu::BufferUsage getBufferObjectUsage(
        const BufferObjectBinding bindingType) noexcept {
    switch (bindingType) {
        case BufferObjectBinding::VERTEX:         return wgpu::BufferUsage::Vertex;
        case BufferObjectBinding::UNIFORM:        return wgpu::BufferUsage::Uniform;
        case BufferObjectBinding::SHADER_STORAGE: return wgpu::BufferUsage::Storage;
    }
}

} // namespace

// The usage flags are determined by the binding type, and always include CopyDst to allow for
// updating the buffer.
WebGPUBufferObject::WebGPUBufferObject(wgpu::Device const& device,
        const BufferObjectBinding bindingType, const uint32_t byteCount)
    : HwBufferObject{ byteCount },
      WebGPUBufferBase{ device, wgpu::BufferUsage::CopyDst | getBufferObjectUsage(bindingType),
          byteCount, "buffer_object" } {}

} // namespace filament::backend
