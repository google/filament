/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_METALBUFFER_H
#define TNT_FILAMENT_DRIVER_METALBUFFER_H

#include "MetalContext.h"
#include "MetalBufferPool.h"

#include <Metal/Metal.h>

namespace filament {
namespace backend {
namespace metal {

class MetalBuffer {
public:

    MetalBuffer(MetalContext& context, size_t size, bool forceGpuBuffer = false);
    ~MetalBuffer();

    MetalBuffer(const MetalBuffer& rhs) = delete;
    MetalBuffer& operator=(const MetalBuffer& rhs) = delete;

    size_t getSize() const noexcept { return mBufferSize; }

    /**
     * Update the buffer with data inside src. Potentially allocates a new buffer allocation to hold
     * the bytes which will be released when the current frame is finished.
     */
    void copyIntoBuffer(void* src, size_t size);

    /**
     * Denotes that this buffer is used for a draw call ensuring that its allocation remains valid
     * until the end of the current frame.
     *
     * @return The MTLBuffer representing the current state of the buffer to bind, or nil if there
     * is no device allocation.
     */
    id<MTLBuffer> getGpuBufferForDraw(id<MTLCommandBuffer> cmdBuffer) noexcept;

    void* getCpuBuffer() const noexcept { return mCpuBuffer; }

    enum Stage {
        VERTEX = 1,
        FRAGMENT = 2
    };

    /**
     * Bind multiple buffers to pipeline stages.
     *
     * bindBuffers binds an array of buffers to the given stage(s) of a MTLRenderCommandEncoder's
     * pipeline.
     */
    static void bindBuffers(id<MTLCommandBuffer> cmdBuffer, id<MTLRenderCommandEncoder> encoder,
            size_t bufferStart, uint8_t stages, MetalBuffer* const* buffers, size_t const* offsets,
            size_t count);

private:

    size_t mBufferSize = 0;
    const MetalBufferPoolEntry* mBufferPoolEntry = nullptr;
    void* mCpuBuffer = nullptr;
    MetalContext& mContext;

};

} // namespace metal
} // namespace backend
} // namespace filament

#endif
