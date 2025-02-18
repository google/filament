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

#include "dawn/native/metal/QuerySetMTL.h"

#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/UtilsMetal.h"

namespace dawn::native::metal {

namespace {

ResultOrError<id<MTLCounterSampleBuffer>> CreateCounterSampleBuffer(Device* device,
                                                                    NSString* label,
                                                                    MTLCommonCounterSet counterSet,
                                                                    uint32_t count) {
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

    descriptor.sampleCount = static_cast<NSUInteger>(std::max(count, uint32_t(1u)));
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
            NSUInteger bufferSize =
                static_cast<NSUInteger>(std::max(GetQueryCount() * sizeof(uint64_t), size_t(4u)));
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
            NSRef<NSString> label = MakeDebugName(
                GetDevice(), "Dawn_QuerySet_TimestampCounterSampleBuffer", GetLabel());
            DAWN_TRY_ASSIGN(
                mCounterSampleBuffer,
                CreateCounterSampleBuffer(device, label.Get(), MTLCommonCounterSetTimestamp,
                                          GetQueryCount()));
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
    return mCounterSampleBuffer;
}

QuerySet::~QuerySet() = default;

void QuerySet::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the query set is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the query set.
    // - It may be called when the last ref to the query set is dropped andit
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the query set since there are no other live refs.
    QuerySetBase::DestroyImpl();

    mVisibilityBuffer = nullptr;

    [mCounterSampleBuffer release];
    mCounterSampleBuffer = nullptr;
}

}  // namespace dawn::native::metal
