// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See layer_chassis_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
 * Copyright (c) 2023-2024 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

// NOLINTBEGIN

// This file contains methods for class vvl::base::Instance and it is designed to ONLY be
// included into validation_object.h.

virtual bool PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                           VkInstance* pInstance, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                         VkInstance* pInstance, const RecordObject& record_obj) {}
virtual void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                          VkInstance* pInstance, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                            const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                     VkPhysicalDevice* pPhysicalDevices, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                   VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {}
virtual void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                                    VkPhysicalDevice* pPhysicalDevices, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                      const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                              VkFormatProperties* pFormatProperties,
                                                              const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                            VkFormatProperties* pFormatProperties, const RecordObject& record_obj) {
}
virtual void PostCallRecordGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                             VkFormatProperties* pFormatProperties,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                   VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
                                                                   VkImageCreateFlags flags,
                                                                   VkImageFormatProperties* pImageFormatProperties,
                                                                   const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                                 VkImageTiling tiling, VkImageUsageFlags usage,
                                                                 VkImageCreateFlags flags,
                                                                 VkImageFormatProperties* pImageFormatProperties,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                  VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
                                                                  VkImageCreateFlags flags,
                                                                  VkImageFormatProperties* pImageFormatProperties,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                        const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                   const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                 uint32_t* pQueueFamilyPropertyCount,
                                                                 VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                              VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                                                              const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                            VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetInstanceProcAddr(VkInstance instance, const char* pName, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) {}
virtual void PostCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj) {
}
virtual void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                 VkExtensionProperties* pProperties,
                                                                 const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                               VkExtensionProperties* pProperties, const RecordObject& record_obj) {
}
virtual void PostCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                                VkExtensionProperties* pProperties,
                                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                               uint32_t* pPropertyCount, VkExtensionProperties* pProperties,
                                                               const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                             uint32_t* pPropertyCount, VkExtensionProperties* pProperties,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                              uint32_t* pPropertyCount, VkExtensionProperties* pProperties,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties,
                                                             const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                           VkLayerProperties* pProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                         VkLayerProperties* pProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                          VkLayerProperties* pProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                         VkImageType type, VkSampleCountFlagBits samples,
                                                                         VkImageUsageFlags usage, VkImageTiling tiling,
                                                                         uint32_t* pPropertyCount,
                                                                         VkSparseImageFormatProperties* pProperties,
                                                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage,
    VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSparseImageFormatProperties(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage,
    VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                          VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                          const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                        VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                         VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                       const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                               VkFormatProperties2* pFormatProperties,
                                                               const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                             VkFormatProperties2* pFormatProperties,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                              VkFormatProperties2* pFormatProperties,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                    VkImageFormatProperties2* pImageFormatProperties,
                                                                    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                  VkImageFormatProperties2* pImageFormatProperties,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                   VkImageFormatProperties2* pImageFormatProperties,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                    uint32_t* pQueueFamilyPropertyCount,
                                                                    VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                               VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                               const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                             VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                              VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                          const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                          uint32_t* pPropertyCount,
                                                                          VkSparseImageFormatProperties2* pProperties,
                                                                          const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                        uint32_t* pPropertyCount,
                                                                        VkSparseImageFormatProperties2* pProperties,
                                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                         const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                         uint32_t* pPropertyCount,
                                                                         VkSparseImageFormatProperties2* pProperties,
                                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                      VkExternalBufferProperties* pExternalBufferProperties,
                                                                      const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                    VkExternalBufferProperties* pExternalBufferProperties,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                     VkExternalBufferProperties* pExternalBufferProperties,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                     VkExternalFenceProperties* pExternalFenceProperties,
                                                                     const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                   VkExternalFenceProperties* pExternalFenceProperties,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                    VkExternalFenceProperties* pExternalFenceProperties,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                            VkPhysicalDeviceToolProperties* pToolProperties,
                                                            const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                          VkPhysicalDeviceToolProperties* pToolProperties,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                           VkPhysicalDeviceToolProperties* pToolProperties,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                              const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                             const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               VkSurfaceKHR surface, VkBool32* pSupported,
                                                               const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                             VkSurfaceKHR surface, VkBool32* pSupported,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                              VkSurfaceKHR surface, VkBool32* pSupported,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                               const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                             uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                              uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t* pRectCount, VkRect2D* pRects,
                                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                uint32_t* pRectCount, VkRect2D* pRects,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                 uint32_t* pRectCount, VkRect2D* pRects,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkDisplayPropertiesKHR* pProperties,
                                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                VkDisplayPropertiesKHR* pProperties,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                 VkDisplayPropertiesKHR* pProperties,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                       VkDisplayPlanePropertiesKHR* pProperties,
                                                                       const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                     VkDisplayPlanePropertiesKHR* pProperties,
                                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                      VkDisplayPlanePropertiesKHR* pProperties,
                                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                                const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                              uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                               uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                        uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties,
                                                        const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                      uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties,
                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                       uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties,
                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                 const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                 const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                               const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                const VkDisplayModeCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                           uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                           const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                         uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode,
                                                          uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_XLIB_KHR
virtual bool PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                        Display* dpy, VisualID visualID,
                                                                        const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                      Display* dpy, VisualID visualID,
                                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       Display* dpy, VisualID visualID,
                                                                       const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
virtual bool PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       xcb_connection_t* connection, xcb_visualid_t visual_id,
                                                                       const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                     xcb_connection_t* connection, xcb_visualid_t visual_id,
                                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                      xcb_connection_t* connection, xcb_visualid_t visual_id,
                                                                      const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
virtual bool PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, struct wl_display* display,
                                                                           const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                         struct wl_display* display,
                                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, struct wl_display* display,
                                                                          const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
virtual bool PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {}
virtual void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                        const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                  const VkVideoProfileInfoKHR* pVideoProfile,
                                                                  VkVideoCapabilitiesKHR* pCapabilities,
                                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                const VkVideoProfileInfoKHR* pVideoProfile,
                                                                VkVideoCapabilitiesKHR* pCapabilities,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                 const VkVideoProfileInfoKHR* pVideoProfile,
                                                                 VkVideoCapabilitiesKHR* pCapabilities,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                                      uint32_t* pVideoFormatPropertyCount,
                                                                      VkVideoFormatPropertiesKHR* pVideoFormatProperties,
                                                                      const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                                    uint32_t* pVideoFormatPropertyCount,
                                                                    VkVideoFormatPropertiesKHR* pVideoFormatProperties,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                                     uint32_t* pVideoFormatPropertyCount,
                                                                     VkVideoFormatPropertiesKHR* pVideoFormatProperties,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                          const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                         const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                            VkPhysicalDeviceProperties2* pProperties,
                                                            const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice,
                                                           VkPhysicalDeviceProperties2* pProperties,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                  VkFormatProperties2* pFormatProperties,
                                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                VkFormatProperties2* pFormatProperties,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                 VkFormatProperties2* pFormatProperties,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                       VkImageFormatProperties2* pImageFormatProperties,
                                                                       const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                     VkImageFormatProperties2* pImageFormatProperties,
                                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                                      VkImageFormatProperties2* pImageFormatProperties,
                                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                       uint32_t* pQueueFamilyPropertyCount,
                                                                       VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                       const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                     uint32_t* pQueueFamilyPropertyCount,
                                                                     VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      uint32_t* pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                                const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                 VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount,
    VkSparseImageFormatProperties2* pProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                             VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                             const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                           VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                            VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                            const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
    VkExternalBufferProperties* pExternalBufferProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                        VkExternalFenceProperties* pExternalFenceProperties,
                                                                        const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                      VkExternalFenceProperties* pExternalFenceProperties,
                                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                       VkExternalFenceProperties* pExternalFenceProperties,
                                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const RecordObject& record_obj) {}
virtual void PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses,
    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses,
    const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses,
    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                     VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                     const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                   VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                    VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                                const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                              uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                              const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                               uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                               const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkDisplayProperties2KHR* pProperties,
                                                                   const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                 VkDisplayProperties2KHR* pProperties,
                                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkDisplayProperties2KHR* pProperties,
                                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                        VkDisplayPlaneProperties2KHR* pProperties,
                                                                        const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                      VkDisplayPlaneProperties2KHR* pProperties,
                                                                      const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                       VkDisplayPlaneProperties2KHR* pProperties,
                                                                       const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                         uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties,
                                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                       uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                        uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties,
                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                            const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                            VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                            const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                          const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                          VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                           const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                           VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
                                                                     uint32_t* pFragmentShadingRateCount,
                                                                     VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates,
                                                                     const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
                                                                   uint32_t* pFragmentShadingRateCount,
                                                                   VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
                                                                    uint32_t* pFragmentShadingRateCount,
                                                                    VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates,
                                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t* pPropertyCount,
                                                                            VkCooperativeMatrixPropertiesKHR* pProperties,
                                                                            const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkCooperativeMatrixPropertiesKHR* pProperties,
                                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pPropertyCount,
                                                                           VkCooperativeMatrixPropertiesKHR* pProperties,
                                                                           const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice,
                                                                         uint32_t* pTimeDomainCount, VkTimeDomainKHR* pTimeDomains,
                                                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                       VkTimeDomainKHR* pTimeDomains,
                                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                        VkTimeDomainKHR* pTimeDomains,
                                                                        const RecordObject& record_obj) {}
virtual bool PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDebugReportCallbackEXT* pCallback, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback,
                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkDebugReportCallbackEXT* pCallback, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                  VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                  int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                 VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                                 int32_t messageCode, const char* pLayerPrefix, const char* pMessage,
                                                 const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_GGP
virtual bool PreCallValidateCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                             const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                             const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                           const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                           const RecordObject& record_obj) {}
virtual void PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                            const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                            const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_GGP
virtual bool PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_VI_NN
virtual bool PreCallValidateCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                              const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_VI_NN
virtual bool PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                              const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const RecordObject& record_obj) {
}
virtual void PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                             const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
virtual bool PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                     VkDisplayKHR* pDisplay, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                   VkDisplayKHR* pDisplay, const RecordObject& record_obj) {}
virtual void PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                                    VkDisplayKHR* pDisplay, const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
virtual bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                     const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                   VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                    VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                    const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_IOS_MVK
virtual bool PreCallValidateCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                              const RecordObject& record_obj) {}
virtual void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
virtual bool PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_MACOS_MVK
virtual bool PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDebugUtilsMessengerEXT* pMessenger, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDebugUtilsMessengerEXT* pMessenger, const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        VkDebugUtilsMessengerEXT* pMessenger, const RecordObject& record_obj) {}
virtual bool PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                          const VkAllocationCallbacks* pAllocator,
                                                          const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                        const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual void PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                         const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) {}
virtual bool PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                       const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                     const RecordObject& record_obj) {}
virtual void PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                      const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice,
                                                                      VkSampleCountFlagBits samples,
                                                                      VkMultisamplePropertiesEXT* pMultisampleProperties,
                                                                      const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                                    VkMultisamplePropertiesEXT* pMultisampleProperties,
                                                                    const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                                     VkMultisamplePropertiesEXT* pMultisampleProperties,
                                                                     const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice,
                                                                         uint32_t* pTimeDomainCount, VkTimeDomainKHR* pTimeDomains,
                                                                         const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                       VkTimeDomainKHR* pTimeDomains,
                                                                       const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                        VkTimeDomainKHR* pTimeDomains,
                                                                        const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_FUCHSIA
virtual bool PreCallValidateCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                          const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                          const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                         const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                         const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
virtual bool PreCallValidateCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_METAL_EXT
virtual bool PreCallValidateGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                               VkPhysicalDeviceToolProperties* pToolProperties,
                                                               const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                             VkPhysicalDeviceToolProperties* pToolProperties,
                                                             const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                              VkPhysicalDeviceToolProperties* pToolProperties,
                                                              const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pPropertyCount,
                                                                           VkCooperativeMatrixPropertiesNV* pProperties,
                                                                           const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                         VkCooperativeMatrixPropertiesNV* pProperties,
                                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkCooperativeMatrixPropertiesNV* pProperties,
                                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                     uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                     const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                   uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                    const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                    uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                                    const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                 const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                               const RecordObject& record_obj) {}
virtual void PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                                const RecordObject& record_obj) {}
virtual bool PreCallValidateGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                             VkDisplayKHR* display, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                           VkDisplayKHR* display, const RecordObject& record_obj) {}
virtual void PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId,
                                            VkDisplayKHR* display, const RecordObject& record_obj) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
virtual bool PreCallValidateAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                  const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                const RecordObject& record_obj) {}
virtual void PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                 const RecordObject& record_obj) {}
virtual bool PreCallValidateGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay,
                                              const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay,
                                            const RecordObject& record_obj) {}
virtual void PostCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay,
                                             const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
virtual bool PreCallValidateCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) {}
virtual void PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
                                                                            uint32_t queueFamilyIndex, IDirectFB* dfb,
                                                                            const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, IDirectFB* dfb,
                                                                          const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice,
                                                                           uint32_t queueFamilyIndex, IDirectFB* dfb,
                                                                           const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
virtual bool PreCallValidateCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) {}
virtual void PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, struct _screen_window* window,
                                                                          const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                        struct _screen_window* window,
                                                                        const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                         struct _screen_window* window,
                                                                         const RecordObject& record_obj) {}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
virtual bool PreCallValidateGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties, const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties, const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice physicalDevice, const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo, uint32_t* pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties, const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pPropertyCount,
                                                                           VkCooperativeVectorPropertiesNV* pProperties,
                                                                           const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                         VkCooperativeVectorPropertiesNV* pProperties,
                                                                         const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                          VkCooperativeVectorPropertiesNV* pProperties,
                                                                          const RecordObject& record_obj) {}
virtual bool PreCallValidateGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties,
    const ErrorObject& error_obj) const {
    return false;
}
virtual void PreCallRecordGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties,
    const RecordObject& record_obj) {}
virtual void PostCallRecordGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties,
    const RecordObject& record_obj) {}

// NOLINTEND
