// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See stateless_validation_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#pragma once
bool PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                    const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount,
                                             VkPhysicalDevice* pPhysicalDevices, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                      VkFormatProperties* pFormatProperties,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                           VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                           VkImageFormatProperties* pImageFormatProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                           VkQueueFamilyProperties* pQueueFamilyProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                                                      const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                                 VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                                                                 VkImageTiling tiling, uint32_t* pPropertyCount,
                                                                 VkSparseImageFormatProperties* pProperties,
                                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                  VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format,
                                                       VkFormatProperties2* pFormatProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                            VkImageFormatProperties2* pImageFormatProperties,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                            VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice,
                                                       VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                  const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                  uint32_t* pPropertyCount,
                                                                  VkSparseImageFormatProperties2* pProperties,
                                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                              VkExternalBufferProperties* pExternalBufferProperties,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                             VkExternalFenceProperties* pExternalFenceProperties,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                    VkPhysicalDeviceToolProperties* pToolProperties,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                      const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                       VkSurfaceKHR surface, VkBool32* pSupported,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                       uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pRectCount, VkRect2D* pRects,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                          VkDisplayPropertiesKHR* pProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                               VkDisplayPlanePropertiesKHR* pProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                        uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                VkDisplayModePropertiesKHR* pProperties,
                                                const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                         const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                         VkDisplayModeKHR* pMode, const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                   VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                Display* dpy, VisualID visualID,
                                                                const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
bool PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                               xcb_connection_t* connection, xcb_visualid_t visual_id,
                                                               const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
bool PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                   struct wl_display* display,
                                                                   const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
bool PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                 const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                          const VkVideoProfileInfoKHR* pVideoProfile,
                                                          VkVideoCapabilitiesKHR* pCapabilities,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                              uint32_t* pVideoFormatPropertyCount,
                                                              VkVideoFormatPropertiesKHR* pVideoFormatProperties,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                                          VkFormatProperties2* pFormatProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                               const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                               VkImageFormatProperties2* pImageFormatProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceMemoryProperties2* pMemoryProperties,
                                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                     const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                                     uint32_t* pPropertyCount,
                                                                     VkSparseImageFormatProperties2* pProperties,
                                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                     const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                                 VkExternalBufferProperties* pExternalBufferProperties,
                                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                                VkExternalFenceProperties* pExternalFenceProperties,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
    VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses,
    const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                             VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                        const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                        uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                        const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                           VkDisplayProperties2KHR* pProperties,
                                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                VkDisplayPlaneProperties2KHR* pProperties,
                                                                const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                 VkDisplayModeProperties2KHR* pProperties,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                    VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
                                                             VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates,
                                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                    VkCooperativeMatrixPropertiesKHR* pProperties,
                                                                    const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                 VkTimeDomainKHR* pTimeDomains,
                                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                          uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                          const char* pMessage, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_GGP
bool PreCallValidateCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_GGP
bool PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_VI_NN
bool PreCallValidateCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_VI_NN
bool PreCallValidateReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                      const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                             VkDisplayKHR* pDisplay, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
bool PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                             VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                             const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_IOS_MVK
bool PreCallValidateCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
bool PreCallValidateCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_MACOS_MVK
bool PreCallValidateCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger,
                                                 const ErrorObject& error_obj) const override;
bool PreCallValidateDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  const ErrorObject& error_obj) const override;
bool PreCallValidateSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                                              VkMultisamplePropertiesEXT* pMultisampleProperties,
                                                              const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                 VkTimeDomainKHR* pTimeDomains,
                                                                 const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_FUCHSIA
bool PreCallValidateCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                  const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_METAL_EXT
bool PreCallValidateGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                       VkPhysicalDeviceToolProperties* pToolProperties,
                                                       const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkCooperativeMatrixPropertiesNV* pProperties,
                                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                             uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                             const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                         const ErrorObject& error_obj) const override;
bool PreCallValidateGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR* display,
                                     const ErrorObject& error_obj) const override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PreCallValidateAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                          const ErrorObject& error_obj) const override;
bool PreCallValidateGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay,
                                      const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
bool PreCallValidateCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                    IDirectFB* dfb, const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                  struct _screen_window* window,
                                                                  const ErrorObject& error_obj) const override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
bool PreCallValidateGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice,
                                                               const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo,
                                                               uint32_t* pFormatCount,
                                                               VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties,
                                                               const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkCooperativeVectorPropertiesNV* pProperties,
                                                                   const ErrorObject& error_obj) const override;
bool PreCallValidateGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties,
    const ErrorObject& error_obj) const override;

// NOLINTEND
