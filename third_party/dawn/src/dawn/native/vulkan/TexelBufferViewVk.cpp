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

#include "dawn/native/vulkan/TexelBufferViewVk.h"

#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<TexelBufferView>> TexelBufferView::Create(
    BufferBase* buffer,
    const UnpackedPtr<TexelBufferViewDescriptor>& descriptor) {
    Ref<TexelBufferView> view = AcquireRef(new TexelBufferView(buffer, descriptor));
    DAWN_TRY(view->Initialize(descriptor));
    return view;
}

TexelBufferView::~TexelBufferView() = default;

MaybeError TexelBufferView::Initialize(const UnpackedPtr<TexelBufferViewDescriptor>& descriptor) {
    Buffer* buffer = ToBackend(GetBuffer());
    VkBuffer handle = buffer->GetHandle();
    if (buffer->IsDestroyed() || handle == VK_NULL_HANDLE) {
        return {};
    }

    Device* device = ToBackend(GetDevice());

    VkBufferViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    createInfo.buffer = handle;
    createInfo.format = VulkanImageFormat(device, GetFormat());
    createInfo.offset = GetOffset();
    createInfo.range = GetSize();

    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateBufferView(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreateBufferView"));

    SetLabelImpl();
    return {};
}

void TexelBufferView::DestroyImpl(DestroyReason reason) {
    Device* device = ToBackend(GetDevice());
    if (mHandle != VK_NULL_HANDLE) {
        device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        mHandle = VK_NULL_HANDLE;
    }
    TexelBufferViewBase::DestroyImpl(reason);
}

VkBufferView TexelBufferView::GetHandle() const {
    return mHandle;
}

void TexelBufferView::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_TexelBufferView", GetLabel());
}

}  // namespace dawn::native::vulkan
