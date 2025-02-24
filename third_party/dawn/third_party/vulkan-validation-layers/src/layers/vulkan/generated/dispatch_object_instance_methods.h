// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_object_generator.py for modifications

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

// This file contains methods for class vvl::dispatch::Instance  and it is designed to ONLY be
// included into dispatch_object.h.

#pragma once

VkResult CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
void DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);
VkResult EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
void GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
void GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties);
VkResult GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                VkImageFormatProperties* pImageFormatProperties);
void GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
void GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                            VkQueueFamilyProperties* pQueueFamilyProperties);
void GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* pName);
VkResult CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                      const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
VkResult EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
VkResult EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount,
                                            VkExtensionProperties* pProperties);
VkResult EnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties);
VkResult EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties);
void GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                  VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
                                                  uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties);
VkResult EnumerateInstanceVersion(uint32_t* pApiVersion);
VkResult EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                       VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
void GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
void GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties);
VkResult GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                 const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                 VkImageFormatProperties2* pImageFormatProperties);
void GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                             VkQueueFamilyProperties2* pQueueFamilyProperties);
void GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
void GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                   const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                   uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties);
void GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice,
                                               const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                               VkExternalBufferProperties* pExternalBufferProperties);
void GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice,
                                              const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                              VkExternalFenceProperties* pExternalFenceProperties);
void GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice,
                                                  const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
                                                  VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
VkResult GetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                         VkPhysicalDeviceToolProperties* pToolProperties);
void DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                            VkBool32* pSupported);
VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                 VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount,
                                            VkSurfaceFormatKHR* pSurfaceFormats);
VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount,
                                                 VkPresentModeKHR* pPresentModes);
VkResult GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount,
                                               VkRect2D* pRects);
VkResult GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                               VkDisplayPropertiesKHR* pProperties);
VkResult GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                    VkDisplayPlanePropertiesKHR* pProperties);
VkResult GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount,
                                             VkDisplayKHR* pDisplays);
VkResult GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                     VkDisplayModePropertiesKHR* pProperties);
VkResult CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo,
                              const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode);
VkResult GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                        VkDisplayPlaneCapabilitiesKHR* pCapabilities);
VkResult CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#ifdef VK_USE_PLATFORM_XLIB_KHR
VkResult CreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VkBool32 GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy,
                                                     VisualID visualID);
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
VkResult CreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VkBool32 GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                    xcb_connection_t* connection, xcb_visualid_t visual_id);
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
VkResult CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VkBool32 GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                        struct wl_display* display);
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
VkResult CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VkBool32 GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
#endif  // VK_USE_PLATFORM_WIN32_KHR
VkResult GetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice, const VkVideoProfileInfoKHR* pVideoProfile,
                                               VkVideoCapabilitiesKHR* pCapabilities);
VkResult GetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                   const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo,
                                                   uint32_t* pVideoFormatPropertyCount,
                                                   VkVideoFormatPropertiesKHR* pVideoFormatProperties);
void GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
void GetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
void GetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format,
                                           VkFormatProperties2* pFormatProperties);
VkResult GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo,
                                                    VkImageFormatProperties2* pImageFormatProperties);
void GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                VkQueueFamilyProperties2* pQueueFamilyProperties);
void GetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
void GetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                      const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo,
                                                      uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties);
VkResult EnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                          VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
void GetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                  const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo,
                                                  VkExternalBufferProperties* pExternalBufferProperties);
void GetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                     const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo,
                                                     VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
void GetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                 const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo,
                                                 VkExternalFenceProperties* pExternalFenceProperties);
VkResult EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                       uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters,
                                                                       VkPerformanceCounterDescriptionKHR* pCounterDescriptions);
void GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice,
                                                           const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo,
                                                           uint32_t* pNumPasses);
VkResult GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                  const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                  VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
VkResult GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                             uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats);
VkResult GetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                VkDisplayProperties2KHR* pProperties);
VkResult GetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                     VkDisplayPlaneProperties2KHR* pProperties);
VkResult GetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                      VkDisplayModeProperties2KHR* pProperties);
VkResult GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo,
                                         VkDisplayPlaneCapabilities2KHR* pCapabilities);
VkResult GetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount,
                                                  VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates);
VkResult GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR* pQualityLevelProperties);
VkResult GetPhysicalDeviceCooperativeMatrixPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                         VkCooperativeMatrixPropertiesKHR* pProperties);
VkResult GetPhysicalDeviceCalibrateableTimeDomainsKHR(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                      VkTimeDomainKHR* pTimeDomains);
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
void DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
                           size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage);
#ifdef VK_USE_PLATFORM_GGP
VkResult CreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_GGP
VkResult GetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                          VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                          VkExternalMemoryHandleTypeFlagsNV externalHandleType,
                                                          VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties);
#ifdef VK_USE_PLATFORM_VI_NN
VkResult CreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                           VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_VI_NN
VkResult ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
VkResult AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display);
VkResult GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay);
#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
VkResult GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                  VkSurfaceCapabilities2EXT* pSurfaceCapabilities);
#ifdef VK_USE_PLATFORM_IOS_MVK
VkResult CreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
VkResult CreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_MACOS_MVK
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks* pAllocator);
void SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);
void GetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples,
                                               VkMultisamplePropertiesEXT* pMultisampleProperties);
VkResult GetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount,
                                                      VkTimeDomainKHR* pTimeDomains);
#ifdef VK_USE_PLATFORM_FUCHSIA
VkResult CreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
VkResult CreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#endif  // VK_USE_PLATFORM_METAL_EXT
VkResult GetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                            VkPhysicalDeviceToolProperties* pToolProperties);
VkResult GetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                        VkCooperativeMatrixPropertiesNV* pProperties);
VkResult GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice,
                                                                         uint32_t* pCombinationCount,
                                                                         VkFramebufferMixedSamplesCombinationNV* pCombinations);
#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult GetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                  const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount,
                                                  VkPresentModeKHR* pPresentModes);
#endif  // VK_USE_PLATFORM_WIN32_KHR
VkResult CreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VkResult AcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display);
VkResult GetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR* display);
#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult AcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display);
VkResult GetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay);
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
VkResult CreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                  const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VkBool32 GetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                         IDirectFB* dfb);
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
VkResult CreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VkBool32 GetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                       struct _screen_window* window);
#endif  // VK_USE_PLATFORM_SCREEN_QNX
VkResult GetPhysicalDeviceOpticalFlowImageFormatsNV(VkPhysicalDevice physicalDevice,
                                                    const VkOpticalFlowImageFormatInfoNV* pOpticalFlowImageFormatInfo,
                                                    uint32_t* pFormatCount,
                                                    VkOpticalFlowImageFormatPropertiesNV* pImageFormatProperties);
VkResult GetPhysicalDeviceCooperativeVectorPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                        VkCooperativeVectorPropertiesNV* pProperties);
VkResult GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
    VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixFlexibleDimensionsPropertiesNV* pProperties);

// NOLINTEND
