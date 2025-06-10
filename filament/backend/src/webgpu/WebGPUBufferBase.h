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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUBUFFERBASE_H
#define TNT_FILAMENT_BACKEND_WEBGPUBUFFERBASE_H

#include "WebGPUConstants.h"

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>

namespace filament::backend {

class BufferDescriptor;

class WebGPUBufferBase /* intended to be extended */ {
public:
    void updateGPUBuffer(BufferDescriptor const&, uint32_t byteOffset, wgpu::Queue const&);

    [[nodiscard]] wgpu::Buffer const& getBuffer() const { return mBuffer; }

protected:
    WebGPUBufferBase(wgpu::Device const&, wgpu::BufferUsage, uint32_t size, char const* label);

private:
    const wgpu::Buffer mBuffer;
    // WEBGPU_BUFFER_SIZE_MODULUS (e.g. 4) bytes to hold any extra chunk we need.
    std::array<uint8_t, WEBGPU_BUFFER_SIZE_MODULUS> mRemainderChunk{};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUBUFFERBASE_H
