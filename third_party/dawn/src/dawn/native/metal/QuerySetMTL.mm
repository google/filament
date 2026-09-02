// Copyright 2020 The Dawn & Tint Authors
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

#include "src/dawn/native/metal/QuerySetMTL.h"

#include "src/dawn/common/Math.h"
#include "src/dawn/native/metal/DeviceMTL.h"
#include "src/dawn/native/metal/UtilsMetal.h"
#include "src/utils/platform.h"

namespace dawn::native::metal {

namespace {

ResultOrError<id<MTLCounterSampleBuffer>> CreateCounterSampleBuffer(Device* device,
                                                                    NSString* label,
                                                                    MTLCommonCounterSet counterSet,
                                                                    QueryIndex count) {
    NSRef<MTLCounterSampleBufferDescriptor> descriptorRef =
        AcquireNSRef([MTLCounterSampleBufferDescriptor new]);
    MTLCounterSampleBufferDescriptor* descriptor = descriptorRef.Get();
    descriptor.label = label;

    // To determine which counters are available from a device, we need to iterate through
    // the counterSets property of a MTLDevice. Then configure which counters will be
    // sampled by creating a MTLCounterSampleBufferDescriptor and setting its counterSet
    // property to the matched one of the available set.
    for (id<MTLCounterSet> set in device->GetMTLDevice().counterSets) {
        if ([set.name isEqualToString:counterSet]) {
            descriptor.counterSet = set;
            break;
        }
    }
    DAWN_ASSERT(descriptor.counterSet != nullptr);

    descriptor.sampleCount = NSUInteger{std::max(count, QueryIndex(1u))};
    descriptor.storageMode = MTLStorageModePrivate;
    if (device->IsToggleEnabled(Toggle::MetalUseSharedModeForCounterSampleBuffer)) {
        descriptor.storageMode = MTLStorageModeShared;
    }

    NSError* error = nullptr;
    id<MTLCounterSampleBuffer> counterSampleBuffer =
        [device->GetMTLDevice() newCounterSampleBufferWithDescriptor:descriptor error:&error];
    if (error != nullptr) {
        return DAWN_OUT_OF_MEMORY_ERROR(std::string("Error creating query set: ") +
                                        [error.localizedDescription UTF8String]);
    }

    return counterSampleBuffer;
}
}  // namespace

CounterSampleBufferAllocator::CounterSampleBufferAllocator(Device* device) : mDevice(device) {}

CounterSampleBufferAllocator::~CounterSampleBufferAllocator() {
    mPool.clear();
}

ResultOrError<CounterSampleBufferAllocator::Allocation> CounterSampleBufferAllocator::Allocate(
    uint32_t count) {
    // Try to find an existing pool buffer with a contiguous block of 'count' free elements.
    // poolBuffer.occupied acts as a bitmap tracker for the allocated query slots.
    for (auto& poolBuffer : mPool) {
        uint32_t run = 0;
        for (uint32_t i = 0; i < poolBuffer.occupied.size(); ++i) {
            if (!poolBuffer.occupied[i]) {
                run++;
                if (run == count) {
                    uint32_t firstIndex = i - count + 1;
                    for (uint32_t j = 0; j < count; ++j) {
                        poolBuffer.occupied[firstIndex + j] = true;
                    }
                    poolBuffer.occupiedCount += count;
                    return Allocation{poolBuffer.buffer.Get(), firstIndex};
                }
            } else {
                run = 0;
            }
        }
    }

    // Allocate a new pool buffer.
    uint32_t poolBufferSize = std::max(count, 4096u);
    NSRef<NSString> label =
        MakeDebugName(mDevice.get(), "Dawn_QuerySet_TimestampCounterSampleBuffer", "Pooled");

    id<MTLCounterSampleBuffer> newBuffer = nil;
    DAWN_TRY_ASSIGN(newBuffer, CreateCounterSampleBuffer(mDevice.get(), label.Get(),
                                                         MTLCommonCounterSetTimestamp,
                                                         QueryIndex(poolBufferSize)));

    // Allocate from the new buffer.
    PoolBuffer poolBuffer;
    poolBuffer.buffer = AcquireNSPRef(newBuffer);
    poolBuffer.occupied.resize(poolBufferSize, false);
    poolBuffer.occupiedCount = count;

    for (uint32_t j = 0; j < count; ++j) {
        poolBuffer.occupied[j] = true;
    }

    mPool.push_back(std::move(poolBuffer));
    return Allocation{newBuffer, 0};
}

void CounterSampleBufferAllocator::Deallocate(const Allocation& allocation, uint32_t count) {
    for (auto it = mPool.begin(); it != mPool.end(); ++it) {
        if (it->buffer.Get() == allocation.buffer) {
            for (uint32_t i = 0; i < count; ++i) {
                it->occupied[allocation.baseIndex + i] = false;
            }

            it->occupiedCount -= count;
            if (it->occupiedCount == 0) {
                mPool.erase(it);
            }
            return;
        }
    }
}

// static
ResultOrError<Ref<QuerySet>> QuerySet::Create(Device* device,
                                              const QuerySetDescriptor* descriptor) {
    Ref<QuerySet> queryset = AcquireRef(new QuerySet(device, descriptor));
    DAWN_TRY(queryset->Initialize());
    return queryset;
}

QuerySet::QuerySet(DeviceBase* dev, const QuerySetDescriptor* desc) : QuerySetBase(dev, desc) {}

MaybeError QuerySet::Initialize() {
    Device* device = ToBackend(GetDevice());

    switch (GetQueryType()) {
        case wgpu::QueryType::Occlusion: {
            // Create buffer for writing 64-bit results.
            NSUInteger bufferSize = std::max(ToQueryStorageSize(GetQueryCount()), 4u);
            mVisibilityBuffer = AcquireNSPRef([device->GetMTLDevice()
                newBufferWithLength:bufferSize
                            options:MTLResourceStorageModePrivate]);
            SetDebugName(GetDevice(), mVisibilityBuffer.Get(), "Dawn_QuerySet_VisibilityBuffer",
                         GetLabel());

            if (mVisibilityBuffer == nil) {
                return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate query set.");
            }
            break;
        }
        case wgpu::QueryType::Timestamp: {
            if (GetLabel().empty()) {
                auto* allocator = device->GetCounterSampleBufferAllocator();
                DAWN_TRY_ASSIGN(mCounterSampleBufferAllocation,
                                allocator->Allocate(static_cast<uint32_t>(GetQueryCount())));
            } else {
                NSRef<NSString> label = MakeDebugName(
                    GetDevice(), "Dawn_QuerySet_TimestampCounterSampleBuffer", GetLabel());
                DAWN_TRY_ASSIGN(
                    mCounterSampleBuffer,
                    CreateCounterSampleBuffer(device, label.Get(), MTLCommonCounterSetTimestamp,
                                              GetQueryCount()));
                mCounterSampleBufferAllocation = {mCounterSampleBuffer, 0};
            }
        } break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    return {};
}

id<MTLBuffer> QuerySet::GetVisibilityBuffer() const {
    return mVisibilityBuffer.Get();
}

id<MTLCounterSampleBuffer> QuerySet::GetCounterSampleBuffer() const {
    return mCounterSampleBufferAllocation.buffer;
}

uint32_t QuerySet::GetSampleIndex(QueryIndex queryIndex) const {
    DAWN_ASSERT(GetQueryType() == wgpu::QueryType::Timestamp);
    return static_cast<uint32_t>(queryIndex) + mCounterSampleBufferAllocation.baseIndex;
}

QuerySet::~QuerySet() = default;

void QuerySet::DestroyImpl(DestroyReason reason) {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the query set is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the query set.
    // - It may be called when the last ref to the query set is dropped andit
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the query set since there are no other live refs.
    QuerySetBase::DestroyImpl(reason);

    mVisibilityBuffer = nullptr;

    if (mCounterSampleBufferAllocation.buffer != nil) {
        if (mCounterSampleBuffer == nullptr) {
            Device* device = ToBackend(GetDevice());
            auto* allocator = device->GetCounterSampleBufferAllocator();
            allocator->Deallocate(mCounterSampleBufferAllocation,
                                  static_cast<uint32_t>(GetQueryCount()));
        } else {
            [mCounterSampleBuffer release];
            mCounterSampleBuffer = nullptr;
        }
        mCounterSampleBufferAllocation = {};
    }
}

}  // namespace dawn::native::metal
