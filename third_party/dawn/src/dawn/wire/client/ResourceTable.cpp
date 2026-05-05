// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/wire/client/ResourceTable.h"

#include <limits>
#include <utility>

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"
#include "dawn/wire/client/LimitsAndFeatures.h"
#include "dawn/wire/client/Queue.h"

namespace dawn::wire::client {

// static
WGPUResourceTable ResourceTable::Create(Device* device,
                                        const WGPUResourceTableDescriptor* descriptor) {
    if (descriptor->size > kMaxResourceTableSize) {
        return nullptr;
    }

    Client* wireClient = device->GetClient();

    DeviceCreateResourceTableCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = descriptor;

    Ref<ResourceTable> table = wireClient->Make<ResourceTable>(device, descriptor);
    cmd.result = table->GetWireHandle(wireClient);

    wireClient->SerializeCommand(cmd);

    return ReturnToAPI(std::move(table));
}

ResourceTable::ResourceTable(const ObjectBaseParams& params,
                             Device* device,
                             const WGPUResourceTableDescriptor* descriptor)
    : ObjectBase(params), mDevice(device) {
    const LimitsAndFeatures& limitsAndFeatures = device->GetLimitsAndFeatures();

    uint32_t sizeLimit = 0;
    if (limitsAndFeatures.HasFeature(WGPUFeatureName_ChromiumExperimentalSamplingResourceTable)) {
        sizeLimit = kMaxResourceTableSize;
    }

    if (descriptor->size <= sizeLimit) {
        // Fill with 0s for each slot, which means that the slot is immediately available.
        mSize = descriptor->size;
        mSlotAvailableAfterSubmit.resize(mSize);
    } else {
        mDestroyed = true;
    }
}

ResourceTable::~ResourceTable() = default;

ObjectType ResourceTable::GetObjectType() const {
    return ObjectType::ResourceTable;
}

void ResourceTable::APIDestroy() {
    mDestroyed = true;
    mSlotAvailableAfterSubmit.clear();

    // Forward the command to the server.
    ResourceTableDestroyCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);
}

WGPUStatus ResourceTable::APIUpdate(uint32_t slot, const WGPUBindingResource* resource) {
    if (mDestroyed || slot >= mSlotAvailableAfterSubmit.size() ||
        mSlotAvailableAfterSubmit[slot] > mDevice->GetQueue()->GetCompletedSubmitIndex()) {
        return WGPUStatus_Error;
    }

    constexpr uint64_t kSlotInUseOnGPU = std::numeric_limits<uint64_t>::max();
    mSlotAvailableAfterSubmit[slot] = kSlotInUseOnGPU;

    // Forward the command to the server.
    ResourceTableUpdateCmd cmd;
    cmd.self = ToAPI(this);
    cmd.slot = slot;
    cmd.resource = resource;
    GetClient()->SerializeCommand(cmd);

    return WGPUStatus_Success;
}

uint32_t ResourceTable::APIInsertBinding(const WGPUBindingResource* resource) {
    if (mDestroyed) {
        return WGPU_INVALID_BINDING;
    }

    // TODO(https://crbug.com/435317394): This is O(n) in the number of slots. We could make it
    // O(logN) with a heap of the free slots that's maintained over time.
    uint64_t completedSubmit = mDevice->GetQueue()->GetCompletedSubmitIndex();
    DAWN_ASSERT(mSlotAvailableAfterSubmit.size() == mSize);
    for (uint32_t slot = 0; slot < mSize; slot++) {
        if (mSlotAvailableAfterSubmit[slot] > completedSubmit) {
            continue;
        }

        WGPUStatus updateStatus = APIUpdate(slot, resource);
        DAWN_ASSERT(updateStatus == WGPUStatus_Success);
        return slot;
    }

    // No slot found, return the invalid binding.
    return WGPU_INVALID_BINDING;
}

WGPUStatus ResourceTable::APIRemoveBinding(uint32_t slot) {
    if (mDestroyed || slot >= mSlotAvailableAfterSubmit.size()) {
        return WGPUStatus_Error;
    }

    mSlotAvailableAfterSubmit[slot] = mDevice->GetQueue()->GetLastSubmitIndex();

    // Forward the command to the server.
    ResourceTableRemoveBindingCmd cmd;
    cmd.self = ToAPI(this);
    cmd.slot = slot;
    GetClient()->SerializeCommand(cmd);

    return WGPUStatus_Success;
}

uint32_t ResourceTable::APIGetSize() const {
    return mSize;
}

}  // namespace dawn::wire::client
