// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See best_practices_generator.py for modifications

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
void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                  VkInstance* pInstance, const RecordObject& record_obj) override;

void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices,
                                            const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                          VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                          VkImageFormatProperties* pImageFormatProperties,
                                                          const RecordObject& record_obj) override;

void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                const RecordObject& record_obj) override;

void PostCallRecordEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount,
                                                        VkExtensionProperties* pProperties,
                                                        const RecordObject& record_obj) override;

void PostCallRecordEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                      uint32_t* pPropertyCount, VkExtensionProperties* pProperties,
                                                      const RecordObject& record_obj) override;

void PostCallRecordEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties,
                                                    const RecordObject& record_obj) override;

void PostCallRecordEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                  VkLayerProperties* pProperties, const RecordObject& record_obj) override;

void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                 VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                 const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                           VkImageFormatProperties2* pImageFormatProperties,
                                                           const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                   VkPhysicalDeviceToolProperties* pToolProperties,
                                                   const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                      VkSurfaceKHR surface, VkBool32* pSupported,
                                                      const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                           const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                      uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                      const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                           const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                         uint32_t* pRectCount, VkRect2D* pRects,
                                                         const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                         VkDisplayPropertiesKHR* pProperties,
                                                         const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                              VkDisplayPlanePropertiesKHR* pProperties,
                                                              const RecordObject& record_obj) override;

void PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                       uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                       const RecordObject& record_obj) override;

void PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                               VkDisplayModePropertiesKHR* pProperties, const RecordObject& record_obj) override;

void PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                        const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                        VkDisplayModeKHR* pMode, const RecordObject& record_obj) override;

void PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                  VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                  const RecordObject& record_obj) override;

void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_XLIB_KHR
void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                       const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                         const VkVideoProfileInfoKHR* pVideoProfile,
                                                         VkVideoCapabilitiesKHR* pCapabilities,
                                                         const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                             const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                             uint32_t* pVideoFormatPropertyCount,
                                                             VkVideoFormatPropertiesKHR* pVideoFormatProperties,
                                                             const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                              const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                              VkImageFormatProperties2* pImageFormatProperties,
                                                              const RecordObject& record_obj) override;

void PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                    const RecordObject& record_obj) override;

void PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
    VkPerformanceCounterDescriptionKHR* pCounterDescriptions, const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                            VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                            const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                       const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                       uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                       const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                          VkDisplayProperties2KHR* pProperties,
                                                          const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                               VkDisplayPlaneProperties2KHR* pProperties,
                                                               const RecordObject& record_obj) override;

void PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                VkDisplayModeProperties2KHR* pProperties, const RecordObject& record_obj) override;

void PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                                   VkDisplayPlaneCapabilities2KHR* pCapabilities,
                                                   const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
                                                            VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates,
                                                            const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties, const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkCooperativeMatrixPropertiesKHR* pProperties,
                                                                   const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                VkTimeDomainKHR* pTimeDomains,
                                                                const RecordObject& record_obj) override;

void PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback,
                                                const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_GGP
void PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_GGP
void PostCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
    VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType,
    VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_VI_NN
void PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_VI_NN
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                         const RecordObject& record_obj) override;

void PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput,
                                            VkDisplayKHR* pDisplay, const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                            const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_IOS_MVK
void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                       const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_MACOS_MVK
void PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger,
                                                const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                                VkTimeDomainKHR* pTimeDomains,
                                                                const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_FUCHSIA
void PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_METAL_EXT
void PostCallRecordGetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                      VkPhysicalDeviceToolProperties* pToolProperties,
                                                      const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkCooperativeMatrixPropertiesNV* pProperties,
                                                                  const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(
    VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations,
    const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                            const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                            uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                            const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;

void PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                        const RecordObject& record_obj) override;

void PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR* display,
                                    const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                         const RecordObject& record_obj) override;

void PostCallRecordGetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay,
                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
void PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_SCREEN_QNX
void PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice,
                                                              const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo,
                                                              uint32_t* pFormatCount,
                                                              VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties,
                                                              const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkCooperativeVectorPropertiesNV* pProperties,
                                                                  const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties,
    const RecordObject& record_obj) override;

// NOLINTEND
