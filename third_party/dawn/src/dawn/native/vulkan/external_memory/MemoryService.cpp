// Copyright 2022 The Dawn & Tint Authors
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

#include <utility>

#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/external_memory/MemoryService.h"

#if DAWN_PLATFORM_IS(LINUX_DESKTOP) || DAWN_PLATFORM_IS(CHROMEOS)
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementationDmaBuf.h"
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementationOpaqueFD.h"
#endif  // DAWN_PLATFORM_IS(LINUX_DESKTOP) || DAWN_PLATFORM_IS(CHROMEOS)

#if DAWN_PLATFORM_IS(ANDROID)
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.h"
#endif  // DAWN_PLATFORM_IS(ANDROID)

#if DAWN_PLATFORM_IS(FUCHSIA)
#include "dawn/native/vulkan/external_memory/MemoryServiceImplementationZirconHandle.h"
#endif  // DAWN_PLATFORM_IS(FUCHSIA)

namespace dawn::native::vulkan::external_memory {
// static
bool Service::CheckSupport(const VulkanDeviceInfo& deviceInfo) {
#if DAWN_PLATFORM_IS(ANDROID)
    return CheckAHardwareBufferSupport(deviceInfo);
#elif DAWN_PLATFORM_IS(FUCHSIA)
    return CheckZirconHandleSupport(deviceInfo);
#elif DAWN_PLATFORM_IS(LINUX_DESKTOP) || DAWN_PLATFORM_IS(CHROMEOS)
    return CheckOpaqueFDSupport(deviceInfo) || CheckDmaBufSupport(deviceInfo);
#else
    return false;
#endif
}

Service::Service(Device* device) {
#if DAWN_PLATFORM_IS(FUCHSIA)
    if (CheckZirconHandleSupport(device->GetDeviceInfo())) {
        mServiceImpls[ExternalImageType::OpaqueFD] = CreateZirconHandleService(device);
    }
#endif  // DAWN_PLATFORM_IS(FUCHSIA)

#if DAWN_PLATFORM_IS(ANDROID)
    if (CheckAHardwareBufferSupport(device->GetDeviceInfo())) {
        mServiceImpls[ExternalImageType::AHardwareBuffer] = CreateAHardwareBufferService(device);
    }
#endif  // DAWN_PLATFORM_IS(ANDROID)

#if DAWN_PLATFORM_IS(LINUX_DESKTOP) || DAWN_PLATFORM_IS(CHROMEOS)
    if (CheckOpaqueFDSupport(device->GetDeviceInfo())) {
        mServiceImpls[ExternalImageType::OpaqueFD] = CreateOpaqueFDService(device);
    }

    if (CheckDmaBufSupport(device->GetDeviceInfo())) {
        mServiceImpls[ExternalImageType::DmaBuf] = CreateDmaBufService(device);
    }
#endif  // DAWN_PLATFORM_IS(LINUX_DESKTOP) || DAWN_PLATFORM_IS(CHROMEOS)
}

Service::~Service() = default;

bool Service::SupportsImportMemory(ExternalImageType externalImageType,
                                   VkFormat format,
                                   VkImageType type,
                                   VkImageTiling tiling,
                                   VkImageUsageFlags usage,
                                   VkImageCreateFlags flags) {
    ServiceImplementation* serviceImpl = GetServiceImplementation(externalImageType);
    DAWN_ASSERT(serviceImpl);

    return serviceImpl->SupportsImportMemory(format, type, tiling, usage, flags);
}

bool Service::SupportsCreateImage(const ExternalImageDescriptor* descriptor,
                                  VkFormat format,
                                  VkImageUsageFlags usage,
                                  bool* supportsDisjoint) {
    ServiceImplementation* serviceImpl = GetServiceImplementation(descriptor->GetType());
    DAWN_ASSERT(serviceImpl);

    return serviceImpl->SupportsCreateImage(descriptor, format, usage, supportsDisjoint);
}

ResultOrError<MemoryImportParams> Service::GetMemoryImportParams(
    const ExternalImageDescriptor* descriptor,
    VkImage image) {
    ServiceImplementation* serviceImpl = GetServiceImplementation(descriptor->GetType());
    DAWN_ASSERT(serviceImpl);

    return serviceImpl->GetMemoryImportParams(descriptor, image);
}

uint32_t Service::GetQueueFamilyIndex(ExternalImageType externalImageType) {
    ServiceImplementation* serviceImpl = GetServiceImplementation(externalImageType);
    DAWN_ASSERT(serviceImpl);

    return serviceImpl->GetQueueFamilyIndex();
}

ResultOrError<VkDeviceMemory> Service::ImportMemory(ExternalImageType externalImageType,
                                                    ExternalMemoryHandle handle,
                                                    const MemoryImportParams& importParams,
                                                    VkImage image) {
    ServiceImplementation* serviceImpl = GetServiceImplementation(externalImageType);
    DAWN_ASSERT(serviceImpl);

    return serviceImpl->ImportMemory(handle, importParams, image);
}

ResultOrError<VkImage> Service::CreateImage(const ExternalImageDescriptor* descriptor,
                                            const VkImageCreateInfo& baseCreateInfo) {
    ServiceImplementation* serviceImpl = GetServiceImplementation(descriptor->GetType());
    DAWN_ASSERT(serviceImpl);

    return serviceImpl->CreateImage(descriptor, baseCreateInfo);
}

ServiceImplementation* Service::GetServiceImplementation(ExternalImageType externalImageType) {
    if (!mServiceImpls[externalImageType]) {
        return nullptr;
    }

    return mServiceImpls[externalImageType].get();
}

}  // namespace dawn::native::vulkan::external_memory
