// Copyright 2026 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/SharedFenceWGPU.h"

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(
    Device* device,
    const UnpackedPtr<SharedFenceDescriptor>& descriptor) {
    WGPUSharedFenceDescriptor innerDesc = WGPU_SHARED_FENCE_DESCRIPTOR_INIT;
    innerDesc.label = ToOutputStringView(descriptor->label);

    // TODO(crbug.com/483147423): Handle all possible chained structures in SharedFenceDescriptor.
    // For now we only handle Metal.
    WGPUSharedFenceMTLSharedEventDescriptor mtlEventDesc =
        WGPU_SHARED_FENCE_MTL_SHARED_EVENT_DESCRIPTOR_INIT;
    if (auto* mtlEventChain = descriptor.Get<SharedFenceMTLSharedEventDescriptor>()) {
        DAWN_INVALID_IF(mtlEventChain->sharedEvent == nullptr, "MTLSharedEvent is missing.");
        mtlEventDesc.sharedEvent = mtlEventChain->sharedEvent;
        innerDesc.nextInChain = &mtlEventDesc.chain;
    } else if (descriptor.Get<SharedFenceDXGISharedHandleDescriptor>()) {
        return DAWN_UNIMPLEMENTED_ERROR(
            "SharedFence in WebGPU backend has not been implemented for all platforms.");
    } else if (descriptor.Get<SharedFenceEGLSyncDescriptor>()) {
        return DAWN_UNIMPLEMENTED_ERROR(
            "SharedFence in WebGPU backend has not been implemented for all platforms.");
    } else if (descriptor.Get<SharedFenceSyncFDDescriptor>()) {
        return DAWN_UNIMPLEMENTED_ERROR(
            "SharedFence in WebGPU backend has not been implemented for all platforms.");
    } else if (descriptor.Get<SharedFenceVkSemaphoreOpaqueFDDescriptor>()) {
        return DAWN_UNIMPLEMENTED_ERROR(
            "SharedFence in WebGPU backend has not been implemented for all platforms.");
    } else if (descriptor.Get<SharedFenceVkSemaphoreZirconHandleDescriptor>()) {
        return DAWN_UNIMPLEMENTED_ERROR(
            "SharedFence in WebGPU backend has not been implemented for all platforms.");
    } else {
        DAWN_UNREACHABLE();
    }

    const DawnProcTable& wgpu = device->wgpu.get();

    WGPUSharedFence innerHandle =
        wgpu.deviceImportSharedFence(device->GetInnerHandle(), &innerDesc);
    // APIImportSharedFence always returns a shared fence. Either a valid object or an error one.
    DAWN_ASSERT(innerHandle);

    Ref<SharedFence> fence = AcquireRef(new SharedFence(device, descriptor->label, innerHandle));
    if (auto* mtlEventChain = descriptor.Get<SharedFenceMTLSharedEventDescriptor>()) {
        fence->mSharedResourceHandle = mtlEventChain->sharedEvent;
    }
    return fence;
}

// static
Ref<SharedFence> SharedFence::CreateFromHandle(Device* device,
                                               StringView label,
                                               WGPUSharedFence innerHandle) {
    return AcquireRef(new SharedFence(device, label, innerHandle));
}

SharedFence::SharedFence(Device* device, StringView label, WGPUSharedFence innerHandle)
    : SharedFenceBase(device, label), ObjectWGPU(device->wgpu->sharedFenceRelease) {
    mInnerHandle = innerHandle;
}

MaybeError SharedFence::ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const {
    WGPUSharedFenceExportInfo innerInfo = WGPU_SHARED_FENCE_EXPORT_INFO_INIT;

    // TODO(crbug.com/483147423): Handle all possible chained structures in SharedFenceExportInfo.
    // For now we only handle Metal.
    DAWN_TRY(info.ValidateSubset<SharedFenceMTLSharedEventExportInfo>());
    WGPUSharedFenceMTLSharedEventExportInfo mtlExportInfo =
        WGPU_SHARED_FENCE_MTL_SHARED_EVENT_EXPORT_INFO_INIT;
    if (info.Get<SharedFenceMTLSharedEventExportInfo>()) {
        innerInfo.nextInChain = &mtlExportInfo.chain;
    }

    const DawnProcTable& wgpu = ToBackend(GetDevice())->wgpu.get();
    wgpu.sharedFenceExportInfo(mInnerHandle, &innerInfo);

    info->type = static_cast<wgpu::SharedFenceType>(innerInfo.type);

    if (auto* mtlExportInfoChain = info.Get<SharedFenceMTLSharedEventExportInfo>()) {
        mtlExportInfoChain->sharedEvent = mtlExportInfo.sharedEvent;
    }

    return {};
}

void SharedFence::SetLabelImpl() {
    const DawnProcTable& wgpu = ToBackend(GetDevice())->wgpu.get();
    wgpu.sharedFenceSetLabel(mInnerHandle, ToOutputStringView(GetLabel()));
}

}  // namespace dawn::native::webgpu
