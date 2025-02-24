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

#include "dawn/native/d3d12/SharedFenceD3D12.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"

namespace dawn::native::d3d12 {

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(
    Device* device,
    StringView label,
    const SharedFenceDXGISharedHandleDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->handle == nullptr, "shared HANDLE is missing.");

    SystemHandle ownedHandle;
    DAWN_TRY_ASSIGN(ownedHandle, SystemHandle::Duplicate(descriptor->handle));

    Ref<SharedFence> fence = AcquireRef(new SharedFence(device, label, std::move(ownedHandle)));
    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->OpenSharedHandle(descriptor->handle,
                                                                     IID_PPV_ARGS(&fence->mFence)),
                          "D3D12 fence open shared handle"));

    return fence;
}

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(Device* device,
                                                    StringView label,
                                                    ComPtr<ID3D12Fence> d3d12Fence) {
    SystemHandle ownedHandle;
    DAWN_TRY(
        CheckHRESULT(device->GetD3D12Device()->CreateSharedHandle(
                         d3d12Fence.Get(), nullptr, GENERIC_ALL, nullptr, ownedHandle.GetMut()),
                     "D3D12 create fence handle"));
    DAWN_ASSERT(ownedHandle.IsValid());
    Ref<SharedFence> fence = AcquireRef(new SharedFence(device, label, std::move(ownedHandle)));
    fence->mFence = std::move(d3d12Fence);
    return fence;
}

void SharedFence::DestroyImpl() {
    ToBackend(GetDevice())->ReferenceUntilUnused(std::move(mFence));
    mFence = nullptr;
}

ID3D12Fence* SharedFence::GetD3DFence() const {
    return mFence.Get();
}

}  // namespace dawn::native::d3d12
