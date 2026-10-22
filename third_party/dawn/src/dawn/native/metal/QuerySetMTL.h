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

#ifndef SRC_DAWN_NATIVE_METAL_QUERYSETMTL_H_
#define SRC_DAWN_NATIVE_METAL_QUERYSETMTL_H_

#import <Metal/Metal.h>

#include <vector>

#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/NSRef.h"
#include "src/dawn/native/QuerySet.h"

namespace dawn::native::metal {

class Device;

// CounterSampleBufferAllocator is designed to work around Apple Metal's strict driver limit of
// 32 MTLCounterSampleBuffer allocations per process. Instead of creating a new counter sample
// buffer for every timestamp QuerySet, this allocator pools large, shared MTLCounterSampleBuffer
// objects and sub-allocates contiguous slices within them. When all sub-allocated ranges in a
// pooled buffer are deallocated, the underlying MTLCounterSampleBuffer is released.
// TODO(crbug.com/537774848): Share this pool across multiple wgpu::Device instances to fully
// respect the per-process allocation limits.
class CounterSampleBufferAllocator {
  public:
    struct Allocation {
        id<MTLCounterSampleBuffer> buffer = nil;
        uint32_t baseIndex = 0;
    };

    explicit CounterSampleBufferAllocator(Device* device);
    ~CounterSampleBufferAllocator();

    ResultOrError<Allocation> Allocate(uint32_t count);
    void Deallocate(const Allocation& allocation, uint32_t count);

  private:
    struct PoolBuffer {
        NSPRef<id<MTLCounterSampleBuffer>> buffer;
        std::vector<bool> occupied;
        uint32_t occupiedCount = 0;
    };

    raw_ptr<Device> mDevice;
    std::vector<PoolBuffer> mPool;
};

class QuerySet final : public QuerySetBase {
  public:
    static ResultOrError<Ref<QuerySet>> Create(Device* device,
                                               const QuerySetDescriptor* descriptor);

    QuerySet(DeviceBase* device, const QuerySetDescriptor* descriptor);

    id<MTLBuffer> GetVisibilityBuffer() const;
    id<MTLCounterSampleBuffer> GetCounterSampleBuffer() const;
    uint32_t GetSampleIndex(QueryIndex queryIndex) const;

  private:
    using QuerySetBase::QuerySetBase;
    MaybeError Initialize();

    ~QuerySet() override;

    // Dawn API
    void DestroyImpl(DestroyReason reason) override;

    NSPRef<id<MTLBuffer>> mVisibilityBuffer;

    CounterSampleBufferAllocator::Allocation mCounterSampleBufferAllocation;
    id<MTLCounterSampleBuffer> mCounterSampleBuffer = nullptr;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_QUERYSETMTL_H_
