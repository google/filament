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

#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"
#include "dawn/native/vulkan/VulkanFunctions.h"
#include "dawn/native/vulkan/VulkanInfo.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreServiceImplementation.h"

#if DAWN_PLATFORM_IS(FUCHSIA)
#include "dawn/native/vulkan/external_semaphore/SemaphoreServiceImplementationZirconHandle.h"
#endif  // DAWN_PLATFORM_IS(FUCHSIA)

// Android, ChromeOS and Linux
#if DAWN_PLATFORM_IS(LINUX)
#include "dawn/native/vulkan/external_semaphore/SemaphoreServiceImplementationFD.h"
#endif  // DAWN_PLATFORM_IS(LINUX)

namespace dawn::native::vulkan::external_semaphore {
// static
bool Service::CheckSupport(const VulkanDeviceInfo& deviceInfo,
                           VkPhysicalDevice physicalDevice,
                           const VulkanFunctions& fn) {
#if DAWN_PLATFORM_IS(FUCHSIA)
    return CheckZirconHandleSupport(deviceInfo, physicalDevice, fn);
#elif DAWN_PLATFORM_IS(LINUX)  // Android, ChromeOS and Linux
    return CheckFDSupport(deviceInfo, physicalDevice, fn);
#else
    return false;
#endif
}

Service::Service(Device* device, VkExternalSemaphoreHandleTypeFlagBits handleType) {
#if DAWN_PLATFORM_IS(FUCHSIA)  // Fuchsia
    DAWN_ASSERT(handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA);
    mServiceImpl = CreateZirconHandleService(device);
#elif DAWN_PLATFORM_IS(LINUX) || DAWN_PLATFORM_IS(CHROMEOS)  // Android, ChromeOS and Linux
    mServiceImpl = CreateFDService(device, handleType);
#endif
}

Service::~Service() = default;

bool Service::Supported() {
    if (!mServiceImpl) {
        return false;
    }

    return mServiceImpl->Supported();
}

void Service::CloseHandle(ExternalSemaphoreHandle handle) {
    DAWN_ASSERT(mServiceImpl);
    mServiceImpl->CloseHandle(handle);
}

ResultOrError<VkSemaphore> Service::ImportSemaphore(ExternalSemaphoreHandle handle) {
    DAWN_ASSERT(mServiceImpl);
    return mServiceImpl->ImportSemaphore(handle);
}

ResultOrError<VkSemaphore> Service::CreateExportableSemaphore() {
    DAWN_ASSERT(mServiceImpl);
    return mServiceImpl->CreateExportableSemaphore();
}

ResultOrError<ExternalSemaphoreHandle> Service::ExportSemaphore(VkSemaphore semaphore) {
    DAWN_ASSERT(mServiceImpl);
    return mServiceImpl->ExportSemaphore(semaphore);
}

ExternalSemaphoreHandle Service::DuplicateHandle(ExternalSemaphoreHandle handle) {
    DAWN_ASSERT(mServiceImpl);
    return mServiceImpl->DuplicateHandle(handle);
}

}  // namespace dawn::native::vulkan::external_semaphore
