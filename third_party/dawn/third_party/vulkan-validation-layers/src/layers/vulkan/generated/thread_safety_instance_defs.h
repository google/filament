// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See thread_safety_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
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
Counter<VkInstance> c_VkInstance;
Counter<VkPhysicalDevice> c_VkPhysicalDevice;
Counter<VkDevice> c_VkDevice;
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES
Counter<VkSurfaceKHR> c_VkSurfaceKHR;
Counter<VkDisplayKHR> c_VkDisplayKHR;
Counter<VkDisplayModeKHR> c_VkDisplayModeKHR;
Counter<VkDebugReportCallbackEXT> c_VkDebugReportCallbackEXT;
Counter<VkDebugUtilsMessengerEXT> c_VkDebugUtilsMessengerEXT;
#else
Counter<uint64_t> c_uint64_t;
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES

WRAPPER(VkInstance)
WRAPPER(VkPhysicalDevice)
WRAPPER(VkDevice)
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES
WRAPPER(VkSurfaceKHR)
WRAPPER(VkDisplayKHR)
WRAPPER(VkDisplayModeKHR)
WRAPPER(VkDebugReportCallbackEXT)
WRAPPER(VkDebugUtilsMessengerEXT)
#else
WRAPPER(uint64_t)
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES

void InitCounters() {
    c_VkInstance.Init(kVulkanObjectTypeInstance, this);
    c_VkPhysicalDevice.Init(kVulkanObjectTypePhysicalDevice, this);
    c_VkDevice.Init(kVulkanObjectTypeDevice, this);
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES
    c_VkSurfaceKHR.Init(kVulkanObjectTypeSurfaceKHR, this);
    c_VkDisplayKHR.Init(kVulkanObjectTypeDisplayKHR, this);
    c_VkDisplayModeKHR.Init(kVulkanObjectTypeDisplayModeKHR, this);
    c_VkDebugReportCallbackEXT.Init(kVulkanObjectTypeDebugReportCallbackEXT, this);
    c_VkDebugUtilsMessengerEXT.Init(kVulkanObjectTypeDebugUtilsMessengerEXT, this);
#else
    c_uint64_t.Init(kVulkanObjectTypeUnknown, this);
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES
}
void PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                 VkInstance* pInstance, const RecordObject& record_obj) override;

void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                  VkInstance* pInstance, const RecordObject& record_obj) override;

void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                  const RecordObject& record_obj) override;

void PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject& record_obj) override;

void PreCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices,
                                           const RecordObject& record_obj) override;

void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices,
                                            const RecordObject& record_obj) override;

void PreCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) override;

void PostCallRecordGetInstanceProcAddr(VkInstance instance, const char* pName, const RecordObject& record_obj) override;

void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj) override;

void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                const RecordObject& record_obj) override;

void PreCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                const RecordObject& record_obj) override;

void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                 VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                 const RecordObject& record_obj) override;

void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;

void PostCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                     const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                     VkSurfaceKHR surface, VkBool32* pSupported,
                                                     const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                      VkSurfaceKHR surface, VkBool32* pSupported,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                          const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                           const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                     uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                     const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                      uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                      const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                          const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                           const RecordObject& record_obj) override;

void PreCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount,
                                                        VkRect2D* pRects, const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                         uint32_t* pRectCount, VkRect2D* pRects,
                                                         const RecordObject& record_obj) override;

void PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount,
                                                      VkDisplayKHR* pDisplays, const RecordObject& record_obj) override;

void PostCallRecordGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                       uint32_t* pDisplayCount, VkDisplayKHR* pDisplays,
                                                       const RecordObject& record_obj) override;

void PreCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                              VkDisplayModePropertiesKHR* pProperties, const RecordObject& record_obj) override;

void PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                               VkDisplayModePropertiesKHR* pProperties, const RecordObject& record_obj) override;

void PreCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                       const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                       VkDisplayModeKHR* pMode, const RecordObject& record_obj) override;

void PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                        const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                        VkDisplayModeKHR* pMode, const RecordObject& record_obj) override;

void PreCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                 VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                 const RecordObject& record_obj) override;

void PostCallRecordGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex,
                                                  VkDisplayPlaneCapabilitiesKHR* pCapabilities,
                                                  const RecordObject& record_obj) override;

void PreCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) override;

void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_XLIB_KHR
void PreCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                       const RecordObject& record_obj) override;

void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
void PreCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                      const RecordObject& record_obj) override;

void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                       const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void PreCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const RecordObject& record_obj) override;

void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
void PreCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const RecordObject& record_obj) override;

void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const RecordObject& record_obj) override;

void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                   VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                   const RecordObject& record_obj) override;

void PostCallRecordEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount,
                                                    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties,
                                                    const RecordObject& record_obj) override;

void PreCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                               VkDisplayModeProperties2KHR* pProperties, const RecordObject& record_obj) override;

void PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount,
                                                VkDisplayModeProperties2KHR* pProperties, const RecordObject& record_obj) override;

void PreCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback,
                                               const RecordObject& record_obj) override;

void PostCallRecordCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback,
                                                const RecordObject& record_obj) override;

void PreCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PostCallRecordDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                        uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                        const char* pMessage, const RecordObject& record_obj) override;

void PostCallRecordDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                         uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix,
                                         const char* pMessage, const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_GGP
void PreCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                   const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                   const RecordObject& record_obj) override;

void PostCallRecordCreateStreamDescriptorSurfaceGGP(VkInstance instance, const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_GGP
#ifdef VK_USE_PLATFORM_VI_NN
void PreCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                    const RecordObject& record_obj) override;

void PostCallRecordCreateViSurfaceNN(VkInstance instance, const VkViSurfaceCreateInfoNN* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                     const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_VI_NN
void PreCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const RecordObject& record_obj) override;

void PostCallRecordReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                     const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
void PreCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                        const RecordObject& record_obj) override;

void PostCallRecordAcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT
void PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                           VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                           const RecordObject& record_obj) override;

void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                            VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                            const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_IOS_MVK
void PreCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                      const RecordObject& record_obj) override;

void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                       const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
void PreCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const RecordObject& record_obj) override;

void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_MACOS_MVK
void PreCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger,
                                               const RecordObject& record_obj) override;

void PostCallRecordCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger,
                                                const RecordObject& record_obj) override;

void PreCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PostCallRecordDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;

void PreCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             const RecordObject& record_obj) override;

void PostCallRecordSubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                              VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                              const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_FUCHSIA
void PreCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) override;

void PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                 const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_METAL_EXT
void PreCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                        const RecordObject& record_obj) override;

void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_METAL_EXT
void PreCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;

void PreCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                       const RecordObject& record_obj) override;

void PostCallRecordAcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display,
                                        const RecordObject& record_obj) override;

#ifdef VK_USE_PLATFORM_WIN32_KHR
void PreCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                        const RecordObject& record_obj) override;

void PostCallRecordAcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                         const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT
void PreCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;

void PostCallRecordCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_DIRECTFB_EXT
#ifdef VK_USE_PLATFORM_SCREEN_QNX
void PreCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                         const RecordObject& record_obj) override;

void PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                          const RecordObject& record_obj) override;

#endif  // VK_USE_PLATFORM_SCREEN_QNX

// NOLINTEND
