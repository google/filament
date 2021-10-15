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

#include <backend/DriverEnums.h>

#include <Metal/Metal.h>

namespace filament {
namespace backend {
namespace metal {

class MetalBuffer {
public:

    MetalBuffer(MetalContext& context, BufferUsage usage, size_t size, bool forceGpuBuffer = false);
    ~MetalBuffer();

    MetalBuffer(const MetalBuffer& rhs) = delete;
    MetalBuffer& operator=(const MetalBuffer& rhs) = delete;

    size_t getSize() const noexcept { return mBufferSize; }

    /**
     * Wrap an external Metal buffer. Stores a strong reference to it.
     */
    void wrapExternalBuffer(id<MTLBuffer> buffer);

    /**
     * Release the external Metal buffer, if wrapping any.
     */
    bool releaseExternalBuffer();

    /**
     * Update the buffer with data inside src. Potentially allocates a new buffer allocation to hold
     * the bytes which will be released when the current frame is finished.
     */
    void copyIntoBuffer(void* src, size_t size, size_t byteOffset = 0);

    /**
     * Denotes that this buffer is used for a draw call ensuring that its allocation remains valid
     * until the end of the current frame.
     *
     * @return The MTLBuffer representing the current state of the buffer to bind, or nil if there
     * is no device allocation.
     *
     * For STREAM buffers, getGpuBufferStreamOffset() should be called to retrieve the correct
     * buffer offset.
     *
     */
    id<MTLBuffer> getGpuBufferForDraw(id<MTLCommandBuffer> cmdBuffer) noexcept;

    /**
     * Returns the offset into the buffer returned by getGpuBufferForDraw. This is always 0 for
     * non-STREAM buffers.
     */
    size_t getGpuBufferStreamOffset() noexcept { return mCurrentStreamStart; }

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

    BufferUsage mUsage;
    size_t mBufferSize = 0;
    id<MTLBuffer> mExternalBuffer = nil;
    size_t mCurrentStreamStart = 0;
    size_t mCurrentStreamEnd = 0;
    const MetalBufferPoolEntry* mBufferPoolEntry = nullptr;
    void* mCpuBuffer = nullptr;
    MetalContext& mContext;

    void copyIntoStreamBuffer(void* src, size_t size);
};

} // namespace metal
} // namespace backend
} // namespace filament

#endif
