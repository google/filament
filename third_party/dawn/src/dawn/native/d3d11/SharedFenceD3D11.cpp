// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d11/SharedFenceD3D11.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/PlatformFunctions.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/QueueD3D11.h"

namespace dawn::native::d3d11 {

namespace {
bool IsSameHandle(DeviceBase* device, HANDLE handle, HANDLE other) {
    if (handle == other) {
        return true;
    }

    auto deviceD3D11 = ToBackend(device);
    return deviceD3D11->GetFunctions()->compareObjectHandles(handle, other);
}
}  // namespace

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(
    Device* device,
    StringView label,
    const SharedFenceDXGISharedHandleDescriptor* descriptor) {
    DAWN_ASSERT(!device->IsToggleEnabled(Toggle::D3D11DisableFence));

    DAWN_INVALID_IF(descriptor->handle == nullptr, "shared HANDLE is missing.");

    const auto& queueFence = ToBackend(device->GetQueue())->GetSharedFence();
    if (queueFence &&
        IsSameHandle(device, queueFence->GetSystemHandle().Get(), descriptor->handle)) {
        return queueFence;
    }

    SystemHandle ownedHandle;
    DAWN_TRY_ASSIGN(ownedHandle, SystemHandle::Duplicate(descriptor->handle));

    Ref<SharedFence> fence = AcquireRef(new SharedFence(device, label, std::move(ownedHandle)));
    DAWN_TRY(CheckHRESULT(device->GetD3D11Device5()->OpenSharedFence(descriptor->handle,
                                                                     IID_PPV_ARGS(&fence->mFence)),
                          "D3D11 fence open shared handle"));

    return fence;
}

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(Device* device,
                                                    StringView label,
                                                    ComPtr<ID3D11Fence> d3d11Fence) {
    SystemHandle ownedHandle;
    DAWN_TRY(CheckHRESULT(
        d3d11Fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, ownedHandle.GetMut()),
        "D3D11: creating fence shared handle"));
    DAWN_ASSERT(ownedHandle.IsValid());
    Ref<SharedFence> fence = AcquireRef(new SharedFence(device, label, std::move(ownedHandle)));
    fence->mFence = std::move(d3d11Fence);
    return fence;
}

void SharedFence::DestroyImpl() {
    mFence = nullptr;
}

ID3D11Fence* SharedFence::GetD3DFence() const {
    return mFence.Get();
}

}  // namespace dawn::native::d3d11
