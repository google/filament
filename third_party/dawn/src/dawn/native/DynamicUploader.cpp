// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/DynamicUploader.h"

#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "dawn/native/Queue.h"

namespace dawn::native {

DynamicUploader::DynamicUploader(DeviceBase* device) : mDevice(device) {}

ResultOrError<UploadReservation> DynamicUploader::Reserve(uint64_t allocationSize,
                                                          uint64_t offsetAlignment) {
    // Disable further sub-allocation should the request be too large.
    if (allocationSize > kRingBufferSize) {
        BufferDescriptor bufferDesc = {};
        bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        bufferDesc.size = Align(allocationSize, 4);
        bufferDesc.mappedAtCreation = true;
        bufferDesc.label = "Dawn_DynamicUploaderStaging";

        IgnoreLazyClearCountScope scope(mDevice);
        Ref<BufferBase> stagingBuffer;
        DAWN_TRY_ASSIGN(stagingBuffer, mDevice->CreateBuffer(&bufferDesc));

        UploadReservation reservation;
        reservation.mappedPointer = static_cast<uint8_t*>(stagingBuffer->GetMappedPointer());
        reservation.offsetInBuffer = 0;
        reservation.buffer = std::move(stagingBuffer);
        return reservation;
    }

    // Request is small, we sub-allocate transiently in one of our ring buffers. The reservation
    // will only be valid for the pending serial.
    ExecutionSerial serial = mDevice->GetQueue()->GetPendingCommandSerial();

    if (mRingBuffers.empty()) {
        mRingBuffers.emplace_back(std::unique_ptr<RingBuffer>(
            new RingBuffer{nullptr, RingBufferAllocator(kRingBufferSize)}));
    }

    // Note: Validation ensures size is already aligned.
    // First-fit: find next buffer large enough to satisfy the allocation request.
    uint64_t startOffset = RingBufferAllocator::kInvalidOffset;
    RingBuffer* targetRingBuffer = mRingBuffers.back().get();
    for (auto& ringBuffer : mRingBuffers) {
        RingBufferAllocator& ringBufferAllocator = ringBuffer->mAllocator;
        // Prevent overflow.
        DAWN_ASSERT(ringBufferAllocator.GetSize() >= ringBufferAllocator.GetUsedSize());
        startOffset = ringBufferAllocator.Allocate(allocationSize, serial, offsetAlignment);
        if (startOffset != RingBufferAllocator::kInvalidOffset) {
            targetRingBuffer = ringBuffer.get();
            break;
        }
    }

    // Upon failure, append a newly created ring buffer to fulfill the
    // request.
    if (startOffset == RingBufferAllocator::kInvalidOffset) {
        mRingBuffers.emplace_back(std::unique_ptr<RingBuffer>(
            new RingBuffer{nullptr, RingBufferAllocator(kRingBufferSize)}));

        targetRingBuffer = mRingBuffers.back().get();
        startOffset = targetRingBuffer->mAllocator.Allocate(allocationSize, serial);
    }

    DAWN_ASSERT(startOffset != RingBufferAllocator::kInvalidOffset);

    // Allocate the staging buffer backing the ringbuffer.
    // Note: the first ringbuffer will be lazily created.
    if (targetRingBuffer->mStagingBuffer == nullptr) {
        BufferDescriptor bufferDesc = {};
        bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        bufferDesc.size = Align(targetRingBuffer->mAllocator.GetSize(), 4);
        bufferDesc.mappedAtCreation = true;
        bufferDesc.label = "Dawn_DynamicUploaderStaging";

        IgnoreLazyClearCountScope scope(mDevice);
        Ref<BufferBase> stagingBuffer;
        DAWN_TRY_ASSIGN(stagingBuffer, mDevice->CreateBuffer(&bufferDesc));
        targetRingBuffer->mStagingBuffer = std::move(stagingBuffer);
    }

    DAWN_ASSERT(targetRingBuffer->mStagingBuffer != nullptr);

    UploadReservation reservation;
    reservation.buffer = targetRingBuffer->mStagingBuffer;
    reservation.mappedPointer =
        static_cast<uint8_t*>(reservation.buffer->GetMappedPointer()) + startOffset;
    reservation.offsetInBuffer = startOffset;

    return reservation;
}

void DynamicUploader::Deallocate(ExecutionSerial lastCompletedSerial, bool freeAll) {
    // Reclaim memory within the ring buffers by ticking (or removing requests no longer
    // in-flight).
    size_t i = 0;
    while (i < mRingBuffers.size()) {
        mRingBuffers[i]->mAllocator.Deallocate(lastCompletedSerial);

        // Never erase the last buffer as to prevent re-creating smaller buffers
        // again unless explicitly asked to do so. The last buffer is the largest.
        const bool shouldFree = (i < mRingBuffers.size() - 1) || freeAll;
        if (mRingBuffers[i]->mAllocator.Empty() && shouldFree) {
            mRingBuffers.erase(mRingBuffers.begin() + i);
        } else {
            i++;
        }
    }
}

bool DynamicUploader::ShouldFlush() const {
    uint64_t kTotalAllocatedSizeThreshold = 64 * 1024 * 1024;
    // We use total allocated size instead of pending-upload size to prevent Dawn from allocating
    // too much GPU memory so that the risk of OOM can be minimized.
    return GetTotalAllocatedSize() > kTotalAllocatedSizeThreshold;
}

uint64_t DynamicUploader::GetTotalAllocatedSize() const {
    uint64_t size = 0;
    for (const auto& buffer : mRingBuffers) {
        if (buffer->mStagingBuffer != nullptr) {
            size += buffer->mStagingBuffer->GetSize();
        }
    }
    return size;
}

}  // namespace dawn::native
