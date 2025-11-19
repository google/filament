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

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

class BufferDescriptor;
class WebGPUQueueManager;

/**
  * A base class for WebGPU buffer objects, providing common functionality for creating and
  * updating GPU buffers.
  */
class WebGPUBufferBase /* intended to be extended */ {
public:
    /**
     * IMPORTANT NOTE: when reusing a buffers with command encoders in the caller logic,
     * e.g. the WebGPU driver/backend, make sure to flush (submit) pending commands (draws, etc.) to
     * the GPU prior to calling this blit, because texture updates may otherwise (unintentionally)
     * happen after draw commands encoded in the encoder. Submitting any commands up to this point
     * ensures the calls happen in the expected sequence.
     */
    void updateGPUBuffer(BufferDescriptor&&, uint32_t byteOffset, wgpu::Device const& device,
            WebGPUQueueManager* const webGPUQueueManager);

    [[nodiscard]] wgpu::Buffer const& getBuffer() const { return mBuffer; }

protected:
    WebGPUBufferBase(wgpu::Device const&, wgpu::BufferUsage, uint32_t size, char const* label);

private:
    const wgpu::Buffer mBuffer;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUBUFFERBASE_H
