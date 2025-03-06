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

#include "dawn/native/d3d/SharedFenceD3D.h"

#include <utility>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/d3d/DeviceD3D.h"

namespace dawn::native::d3d {

SharedFence::SharedFence(Device* device, StringView label, SystemHandle ownedHandle)
    : SharedFenceBase(device, label), mHandle(std::move(ownedHandle)) {}

HANDLE SharedFence::GetFenceHandle() const {
    return mHandle.Get();
}

MaybeError SharedFence::ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const {
    info->type = wgpu::SharedFenceType::DXGISharedHandle;

    DAWN_TRY(info.ValidateSubset<SharedFenceDXGISharedHandleExportInfo>());
    auto* exportInfo = info.Get<SharedFenceDXGISharedHandleExportInfo>();
    if (exportInfo != nullptr) {
        exportInfo->handle = mHandle.Get();
    }
    return {};
}

}  // namespace dawn::native::d3d
