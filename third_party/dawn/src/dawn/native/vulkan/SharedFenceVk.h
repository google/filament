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

#ifndef SRC_DAWN_NATIVE_VULKAN_SHAREDTEXTUREFENCEVk_H_
#define SRC_DAWN_NATIVE_VULKAN_SHAREDTEXTUREFENCEVk_H_

#include "dawn/common/Platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/SystemHandle.h"

namespace dawn::native::vulkan {

class Device;

class SharedFence final : public SharedFenceBase {
  public:
    static ResultOrError<Ref<SharedFence>> Create(
        Device* device,
        StringView label,
        const SharedFenceVkSemaphoreOpaqueFDDescriptor* descriptor);

    static ResultOrError<Ref<SharedFence>> Create(Device* device,
                                                  StringView label,
                                                  const SharedFenceSyncFDDescriptor* descriptor);

    static ResultOrError<Ref<SharedFence>> Create(
        Device* device,
        StringView label,
        const SharedFenceVkSemaphoreZirconHandleDescriptor* descriptor);

    const SystemHandle& GetHandle() const;

  private:
    SharedFence(Device* device, StringView label, SystemHandle handle);
    void DestroyImpl() override;

    MaybeError ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const override;

    wgpu::SharedFenceType mType;
    SystemHandle mHandle;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_SHAREDTEXTUREFENCEVk_H_
