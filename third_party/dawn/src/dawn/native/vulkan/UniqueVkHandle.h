// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_UNIQUEVKHANDLE_H_
#define SRC_DAWN_NATIVE_VULKAN_UNIQUEVKHANDLE_H_

#include <utility>

#include "dawn/common/NonCopyable.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"

namespace dawn::native::vulkan {

// unique_ptr<> like RAII helper for Vulkan objects.
template <typename Handle>
class UniqueVkHandle : public NonCopyable {
  public:
    UniqueVkHandle() : mDevice(nullptr), mHandle(VK_NULL_HANDLE) {}
    UniqueVkHandle(Device* device, Handle handle) : mDevice(device), mHandle(handle) {}
    ~UniqueVkHandle() {
        if (mHandle != VK_NULL_HANDLE) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
        }
    }

    UniqueVkHandle(UniqueVkHandle<Handle>&& other)
        : mDevice(std::move(other.mDevice)), mHandle(other.mHandle) {
        other.mHandle = VK_NULL_HANDLE;
    }
    UniqueVkHandle<Handle>& operator=(UniqueVkHandle<Handle>&& other) {
        if (&other != this) {
            if (mHandle != VK_NULL_HANDLE) {
                mDevice->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            }
            mDevice = std::move(other.mDevice);
            mHandle = other.mHandle;
            other.mHandle = VK_NULL_HANDLE;
        }

        return *this;
    }

    Handle Get() const { return mHandle; }
    Handle Acquire() {
        Handle handle = mHandle;
        mDevice = nullptr;
        mHandle = VK_NULL_HANDLE;
        return handle;
    }

  private:
    Ref<Device> mDevice;
    Handle mHandle = VK_NULL_HANDLE;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_UNIQUEVKHANDLE_H_
