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

#include <zircon/syscalls.h>
#include <utility>

#include "dawn/native/SystemHandle.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreServiceImplementation.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreServiceImplementationZirconHandle.h"

namespace dawn::native::vulkan::external_semaphore {

class ServiceImplementationZirconHandle : public ServiceImplementation {
  public:
    explicit ServiceImplementationZirconHandle(Device* device)
        : ServiceImplementation(device),
          mSupported(CheckSupport(device->GetDeviceInfo(),
                                  ToBackend(device->GetPhysicalDevice())->GetVkPhysicalDevice(),
                                  device->fn)) {}

    ~ServiceImplementationZirconHandle() override = default;

    static bool CheckSupport(const VulkanDeviceInfo& deviceInfo,
                             VkPhysicalDevice vkPhysicalDevice,
                             const VulkanFunctions& fn) {
        if (!deviceInfo.HasExt(DeviceExt::ExternalSemaphoreZirconHandle)) {
            return false;
        }

        VkPhysicalDeviceExternalSemaphoreInfoKHR semaphoreInfo;
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR;
        semaphoreInfo.pNext = nullptr;
        semaphoreInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

        VkExternalSemaphorePropertiesKHR semaphoreProperties;
        semaphoreProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR;
        semaphoreProperties.pNext = nullptr;

        fn.GetPhysicalDeviceExternalSemaphoreProperties(vkPhysicalDevice, &semaphoreInfo,
                                                        &semaphoreProperties);

        VkFlags requiredFlags = VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR |
                                VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR;

        return IsSubset(requiredFlags, semaphoreProperties.externalSemaphoreFeatures);
    }

    // True if the device reports it supports this feature
    bool Supported() override { return mSupported; }

    // Given an external handle, import it into a VkSemaphore
    ResultOrError<VkSemaphore> ImportSemaphore(ExternalSemaphoreHandle handle) override {
        DAWN_INVALID_IF(handle == ZX_HANDLE_INVALID,
                        "Importing a semaphore with an invalid handle.");

        VkSemaphore semaphore = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        DAWN_TRY(CheckVkSuccess(
            mDevice->fn.CreateSemaphore(mDevice->GetVkDevice(), &info, nullptr, &*semaphore),
            "vkCreateSemaphore"));

        VkImportSemaphoreZirconHandleInfoFUCHSIA importSemaphoreHandleInfo;
        importSemaphoreHandleInfo.sType =
            VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA;
        importSemaphoreHandleInfo.pNext = nullptr;
        importSemaphoreHandleInfo.semaphore = semaphore;
        // A temporary import means that after we wait on this semaphore, the semaphore payload
        // will be restored to its prior permanent state - which is signaled.
        // Note that this is different from how Vulkan binary semaphores usually work - where
        // waiting on them resets them to unsignaled.
        // For Zircon events we use temporary because it enables concurrent waiting.
        // Multiple waiters can wait on the same semaphore and all be unblocked because after
        // one waiter is woken, the state resets back to signaled for the next waiter to be woken.
        importSemaphoreHandleInfo.flags = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT;
        importSemaphoreHandleInfo.handleType =
            VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;
        SystemHandle handleCopy;
        DAWN_TRY_ASSIGN(handleCopy, SystemHandle::Duplicate(handle));
        importSemaphoreHandleInfo.zirconHandle = handleCopy.Get();

        MaybeError status = CheckVkSuccess(mDevice->fn.ImportSemaphoreZirconHandleFUCHSIA(
                                               mDevice->GetVkDevice(), &importSemaphoreHandleInfo),
                                           "vkImportSemaphoreZirconHandleFUCHSIA");

        if (status.IsError()) {
            mDevice->fn.DestroySemaphore(mDevice->GetVkDevice(), semaphore, nullptr);
            DAWN_TRY(std::move(status));
        }

        handleCopy.Detach();  // Ownership transfered to the semaphore.
        return semaphore;
    }

    // Create a VkSemaphore that is exportable into an external handle later
    ResultOrError<VkSemaphore> CreateExportableSemaphore() override {
        VkExportSemaphoreCreateInfoKHR exportSemaphoreInfo;
        exportSemaphoreInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR;
        exportSemaphoreInfo.pNext = nullptr;
        exportSemaphoreInfo.handleTypes =
            VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

        VkSemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = &exportSemaphoreInfo;
        semaphoreCreateInfo.flags = 0;

        VkSemaphore signalSemaphore;
        DAWN_TRY(
            CheckVkSuccess(mDevice->fn.CreateSemaphore(mDevice->GetVkDevice(), &semaphoreCreateInfo,
                                                       nullptr, &*signalSemaphore),
                           "vkCreateSemaphore"));
        return signalSemaphore;
    }

    // Export a VkSemaphore into an external handle
    ResultOrError<ExternalSemaphoreHandle> ExportSemaphore(VkSemaphore semaphore) override {
        VkSemaphoreGetZirconHandleInfoFUCHSIA semaphoreGetHandleInfo;
        semaphoreGetHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA;
        semaphoreGetHandleInfo.pNext = nullptr;
        semaphoreGetHandleInfo.semaphore = semaphore;
        semaphoreGetHandleInfo.handleType =
            VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_ZIRCON_EVENT_BIT_FUCHSIA;

        zx_handle_t handle = ZX_HANDLE_INVALID;
        DAWN_TRY(CheckVkSuccess(mDevice->fn.GetSemaphoreZirconHandleFUCHSIA(
                                    mDevice->GetVkDevice(), &semaphoreGetHandleInfo, &handle),
                                "VkSemaphoreGetZirconHandleInfoFUCHSIA"));

        DAWN_ASSERT(handle != ZX_HANDLE_INVALID);
        return handle;
    }

    // Duplicate a new external handle from the given one.
    ExternalSemaphoreHandle DuplicateHandle(ExternalSemaphoreHandle handle) override {
        zx_handle_t out_handle = ZX_HANDLE_INVALID;
        zx_status_t status = zx_handle_duplicate(handle, ZX_RIGHT_SAME_RIGHTS, &out_handle);
        DAWN_ASSERT(status == ZX_OK);
        return out_handle;
    }

    void CloseHandle(ExternalSemaphoreHandle handle) override {
        zx_status_t status = zx_handle_close(handle);
        DAWN_ASSERT(status == ZX_OK);
    }

  private:
    bool mSupported = false;
};

std::unique_ptr<ServiceImplementation> CreateZirconHandleService(Device* device) {
    return std::make_unique<ServiceImplementationZirconHandle>(device);
}
bool CheckZirconHandleSupport(const VulkanDeviceInfo& deviceInfo,
                              VkPhysicalDevice vkPhysicalDevice,
                              const VulkanFunctions& fn) {
    return ServiceImplementationZirconHandle::CheckSupport(deviceInfo, vkPhysicalDevice, fn);
}

}  // namespace dawn::native::vulkan::external_semaphore
