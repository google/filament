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

#include "dawn/native/vulkan/SharedFenceVk.h"

#include <utility>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/vulkan/DeviceVk.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(
    Device* device,
    StringView label,
    const SharedFenceVkSemaphoreZirconHandleDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->handle == 0, "Zircon handle (%d) was invalid.", descriptor->handle);

    SystemHandle handle;
    DAWN_TRY_ASSIGN(handle, SystemHandle::Duplicate(descriptor->handle));
    auto fence = AcquireRef(new SharedFence(device, label, std::move(handle)));
    fence->mType = wgpu::SharedFenceType::VkSemaphoreZirconHandle;
    return fence;
}

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(Device* device,
                                                    StringView label,
                                                    const SharedFenceSyncFDDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->handle < 0, "File descriptor (%d) was invalid.",
                    descriptor->handle);
    SystemHandle handle;
    DAWN_TRY_ASSIGN(handle, SystemHandle::Duplicate(descriptor->handle));
    auto fence = AcquireRef(new SharedFence(device, label, std::move(handle)));
    fence->mType = wgpu::SharedFenceType::SyncFD;
    return fence;
}

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(
    Device* device,
    StringView label,
    const SharedFenceVkSemaphoreOpaqueFDDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->handle < 0, "File descriptor (%d) was invalid.",
                    descriptor->handle);
    SystemHandle handle;
    DAWN_TRY_ASSIGN(handle, SystemHandle::Duplicate(descriptor->handle));
    auto fence = AcquireRef(new SharedFence(device, label, std::move(handle)));
    fence->mType = wgpu::SharedFenceType::VkSemaphoreOpaqueFD;
    return fence;
}

SharedFence::SharedFence(Device* device, StringView label, SystemHandle handle)
    : SharedFenceBase(device, label), mHandle(std::move(handle)) {}

void SharedFence::DestroyImpl() {
    mHandle.Close();
}

const SystemHandle& SharedFence::GetHandle() const {
    return mHandle;
}

MaybeError SharedFence::ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const {
    info->type = mType;

#if DAWN_PLATFORM_IS(FUCHSIA)
    DAWN_TRY(info.ValidateSubset<SharedFenceVkSemaphoreZirconHandleExportInfo>());
    auto* exportInfo = info.Get<SharedFenceVkSemaphoreZirconHandleExportInfo>();
    if (exportInfo != nullptr) {
        exportInfo->handle = mHandle.Get();
    }
#elif DAWN_PLATFORM_IS(LINUX)
    switch (mType) {
        case wgpu::SharedFenceType::SyncFD:
            DAWN_TRY(info.ValidateSubset<SharedFenceSyncFDExportInfo>());
            {
                auto* exportInfo = info.Get<SharedFenceSyncFDExportInfo>();
                if (exportInfo != nullptr) {
                    exportInfo->handle = mHandle.Get();
                }
            }
            break;
        case wgpu::SharedFenceType::VkSemaphoreOpaqueFD:
            DAWN_TRY(info.ValidateSubset<SharedFenceVkSemaphoreOpaqueFDExportInfo>());
            {
                auto* exportInfo = info.Get<SharedFenceVkSemaphoreOpaqueFDExportInfo>();
                if (exportInfo != nullptr) {
                    exportInfo->handle = mHandle.Get();
                }
            }
            break;
        default:
            DAWN_UNREACHABLE();
    }
#else
    DAWN_UNREACHABLE();
#endif
    return {};
}

}  // namespace dawn::native::vulkan
