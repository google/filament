/*
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
 */

#pragma once

#include "error_monitor.h"
#include "test_framework.h"
#include "feature_requirements.h"
#include "binding.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/log.h>
#include <android_native_app_glue.h>
#endif

#include <vector>

using vkt::MakeVkHandles;

static constexpr uint64_t kWaitTimeout{10000000000};  // 10 seconds in ns
static constexpr VkDeviceSize kZeroDeviceSize{0};

// Common (and simple) enough to make global
static constexpr VkMemoryPropertyFlags kHostVisibleMemProps =
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

struct SurfaceContext {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HWND m_win32Window{};
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    Display *m_surface_dpy{};
    Window m_surface_window{};
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_connection_t *m_surface_xcb_conn{};
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    void *caMetalLayer{};
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    static bool CanResize() { return true; }
    void Resize(uint32_t width, uint32_t height);
#else
    static bool CanResize() { return false; }
    void Resize(uint32_t width, uint32_t height) {}
#endif
    void Destroy();
    ~SurfaceContext() { Destroy(); }
};

struct SurfaceInformation {
    VkSurfaceCapabilitiesKHR surface_capabilities{};
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> surface_present_modes;
    VkPresentModeKHR surface_non_shared_present_mode{};
    VkCompositeAlphaFlagBitsKHR surface_composite_alpha{};
};

class VkRenderFramework : public VkTestFramework {
  public:
    VkInstance instance() const { return instance_; }
    VkDevice device() const { return m_device->handle(); }
    vkt::Device *DeviceObj() const { return m_device; }
    VkPhysicalDevice Gpu() const;
    VkRenderPass RenderPass() const { return m_renderPass; }
    VkFramebuffer Framebuffer() const { return m_framebuffer->handle(); }

    vkt::Queue *DefaultQueue() const { return m_default_queue; }
    vkt::Queue *SecondQueue() const { return m_second_queue; }

    ErrorMonitor &Monitor();
    const VkPhysicalDeviceProperties &PhysicalDeviceProps() const;

    bool InstanceLayerSupported(const char *layer_name, uint32_t spec_version = 0, uint32_t impl_version = 0);
    bool InstanceExtensionSupported(const char *extension_name, uint32_t spec_version = 0);

    VkInstanceCreateInfo GetInstanceCreateInfo() const;
    void InitFramework(void *instance_pnext = NULL);
    void ShutdownFramework();

    // Functions to modify the VkRenderFramework surface & swapchain variables
    void InitSurface();
    void InitSwapchainInfo();
    void InitSwapchain(VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                       VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    void DestroySwapchain();
    // Functions to create surfaces and swapchains that *aren't* member variables of VkRenderFramework
    VkResult CreateSurface(SurfaceContext &surface_context, vkt::Surface &surface, VkInstance custom_instance = VK_NULL_HANDLE);
    SurfaceInformation GetSwapchainInfo(const VkSurfaceKHR surface);
    vkt::Swapchain CreateSwapchain(VkSurfaceKHR surface, VkImageUsageFlags imageUsage, VkSurfaceTransformFlagBitsKHR preTransform,
                                   VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);

    // Swapchain capabilities declaration to be used with RETURN_IF_SKIP
    void SupportMultiSwapchain();
    void SupportSurfaceResize();

    void InitRenderTarget();
    void InitRenderTarget(uint32_t targets);
    void InitRenderTarget(const VkImageView *dsBinding);
    void InitRenderTarget(uint32_t targets, const VkImageView *dsBinding);
    void InitDynamicRenderTarget(VkFormat format = VK_FORMAT_UNDEFINED);
    VkImageView GetDynamicRenderTarget() const;
    VkRect2D GetRenderTargetArea() const;
    void DestroyRenderTarget();

    // Used for VK_EXT_shader_object
    void SetDefaultDynamicStatesExclude(const std::vector<VkDynamicState> &exclude = {}, bool tessellation = false,
                                        VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
    void SetDefaultDynamicStatesAll(VkCommandBuffer cmdBuffer);

    static bool IgnoreDisableChecks();
    bool IsPlatformMockICD();
    void GetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures *features);
    void GetPhysicalDeviceProperties(VkPhysicalDeviceProperties *props);
    VkFormat GetRenderTargetFormat();
    // default to CommandPool Reset flag to allow recording multiple command buffers simpler
    void InitState(VkPhysicalDeviceFeatures *features = nullptr, void *create_device_pnext = nullptr,
                   const VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    void InitStateWithRequirements(vkt::FeatureRequirements &feature_requirements);
    bool DeviceExtensionSupported(const char *extension_name, uint32_t spec_version = 0) const;
    bool DeviceExtensionSupported(VkPhysicalDevice, const char *, const char *name,
                                  uint32_t spec_version = 0) const {  // deprecated
        return DeviceExtensionSupported(name, spec_version);
    }

    // Tracks ext_name to be enabled at device creation time and attempts to enable any required instance extensions.
    // Does not return anything as will be checked when creating the framework
    // `ext_name` can refer to a device or instance extension.
    void AddRequiredExtensions(const char *ext_name);
    // Ensures at least 1 WSI instance extension is enabled
    void AddWsiExtensions(const char *ext_name);
    // Same as AddRequiredExtensions but won't skip test if not found
    void AddOptionalExtensions(const char *ext_name);
    // After instance and physical device creation (e.g., after InitFramework), returns if extension was enabled
    bool IsExtensionsEnabled(const char *ext_name) const;
    // if requested extensions are not supported, helper function to get string to print out
    std::string RequiredExtensionsNotSupported() const;

    // By default, requested extensions that are promoted to the effective API version (and thus are redundant)
    // are not enabled, but this can be overridden for individual test cases that explicitly test such use cases.
    void AllowPromotedExtensions() { allow_promoted_extensions_ = true; }

    // Add a feature required for the test to be executed. The currently targeted API version is used to add the correct struct, so
    // be sure to call SetTargetApiVersion before.
    // Only features added with this or the AddOptionalFeature method will be enabled.
    void AddRequiredFeature(vkt::Feature feature);
    // Add an optional feature for the test to be executed. The currently targeted API version is used to add the correct struct, so
    // be sure to call SetTargetApiVersion before
    // Only features added with this or the AddRequiredFeature method will be enabled.
    void AddOptionalFeature(vkt::Feature feature);

    std::vector<uint32_t> GLSLToSPV(VkShaderStageFlagBits stage, const char *code, const spv_target_env env = SPV_ENV_VULKAN_1_0);

    void SetDesiredFailureMsg(const VkFlags msg_flags, const std::string &msg) {
        m_errorMonitor->SetDesiredFailureMsg(msg_flags, msg);
    };
    void SetDesiredFailureMsg(const VkFlags msg_flags, const char *const msg_string) {
        m_errorMonitor->SetDesiredFailureMsg(msg_flags, msg_string);
    };
    void VerifyFound() { m_errorMonitor->VerifyFound(); }

  protected:
    APIVersion m_instance_api_version = 0;
    APIVersion m_target_api_version = 0;
    APIVersion m_attempted_api_version = 0;

    VkRenderFramework();
    virtual ~VkRenderFramework() = 0;

    std::vector<VkLayerProperties> available_layers_;          // allow caching of available layers
    std::vector<VkExtensionProperties> available_extensions_;  // allow caching of available instance extensions

    ErrorMonitor monitor_ = ErrorMonitor(m_print_vu);
    ErrorMonitor *m_errorMonitor = &monitor_;  // TODO: Removing this properly is it's own PR. It's a big change.

    bool allow_promoted_extensions_ = false;

    VkApplicationInfo app_info_;
    std::vector<const char *> instance_layers_;
    std::vector<const char *> m_instance_extension_names;
    VkInstance instance_;
    VkPhysicalDevice gpu_ = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physDevProps_;
    // This set of required features is used for the features query.
    // If any required feature is not available, test will fail
    vkt::FeatureRequirements required_features_;
    // This is the set of features that will be enabled.
    // The same features added to required_features_ are added here.
    // But when querying features, required_features_ will be filled with all
    // available features. Hence, if used to create a device, the required_features_ set
    // would *also* enable available features, when we just want to enable required features.
    vkt::FeatureRequirements features_to_enable_;
    bool all_queue_count_ = false;

    uint32_t m_gpu_index;
    vkt::Device *m_device;
    vkt::CommandPool m_command_pool;
    vkt::CommandBuffer m_command_buffer;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    vkt::Buffer *m_vertex_buffer;

    // WSI items
    SurfaceContext m_surface_context{};
    vkt::Surface m_surface{};
    vkt::Swapchain m_swapchain;
    VkSurfaceCapabilitiesKHR m_surface_capabilities{};
    std::vector<VkSurfaceFormatKHR> m_surface_formats;
    std::vector<VkPresentModeKHR> m_surface_present_modes;
    VkPresentModeKHR m_surface_non_shared_present_mode{};
    VkCompositeAlphaFlagBitsKHR m_surface_composite_alpha{};

    std::vector<VkClearValue> m_renderPassClearValues;
    VkRenderPassBeginInfo m_renderPassBeginInfo;
    std::vector<std::unique_ptr<vkt::Image>> m_renderTargets;
    uint32_t m_width, m_height;
    VkFormat m_render_target_fmt;
    VkFormat m_depth_stencil_fmt;
    VkClearColorValue m_clear_color;
    vkt::Image *m_depthStencil;
    // first graphics queue, used must often, don't overwrite, use Device class
    vkt::Queue *m_default_queue = nullptr;
    VkQueueFlags m_default_queue_caps = 0;

    // A queue different from the default one (can be null).
    // The queue with the most capabilities is selected (graphics > compute > transfer).
    // Supports transfer capabilities; check m_second_queue_caps for compute/graphics support.
    vkt::Queue *m_second_queue = nullptr;
    VkQueueFlags m_second_queue_caps = 0;

    // A queue different from the default or the second one (can be null).
    // The queue with the most capabilities is selected (graphics > compute > transfer).
    // Supports transfer capabilities; check m_third_queue_caps for compute/graphics support.
    vkt::Queue *m_third_queue = nullptr;
    VkQueueFlags m_third_queue_caps = 0;

    vkt::CommandPool m_second_command_pool;  // associated with a queue family of the second command queue
    vkt::CommandBuffer m_second_command_buffer;

    // Requested extensions to enable at device creation time
    std::vector<const char *> m_required_extensions;
    // Optional extensions to try and enable at device creation time
    std::vector<const char *> m_optional_extensions;
    // wsi extensions to try and enable
    std::vector<const char *> m_wsi_extensions;
    // Device extensions to enable
    std::vector<const char *> m_device_extension_names;

  private:
    // TODO - move to own helper logic
    vkt::Framebuffer *m_framebuffer;
    std::vector<vkt::ImageView> m_render_target_views;   // color attachments but not depth
    std::vector<VkImageView> m_framebuffer_attachments;  // all attachments, can be consumed directly by the API

    // Add ext_name, the names of all instance extensions required by ext_name, and return true if ext_name is supported. If the
    // extension is not supported, no extension names are added for instance creation. `ext_name` can refer to a device or instance
    // extension.
    bool AddRequestedInstanceExtensions(const char *ext_name);
    // Returns true if the instance extension is promoted to core in the target API version requested using SetTargetApiVersion().
    bool IsPromotedInstanceExtension(const char *inst_ext_name) const;
    // Returns true if the instance extension inst_ext_name is enabled. This call is only valid _after_ previous
    // `AddRequired*Extensions` calls and InitFramework has been called. `inst_ext_name` must be an instance extension name; false
    // is returned for all device extension names.
    // This function also returns true if the instance extension is implicitly supported in the target API version
    // requested using SetTargetApiVersion().
    bool CanEnableInstanceExtension(const std::string &inst_ext_name) const;
    // Add dev_ext_name, then names of _device_ extensions required by dev_ext_name, and return true if dev_ext_name is supported.
    // If the extension is not supported, no extension names are added for device creation. This function has no effect if
    // dev_ext_name refers to an instance extension.
    bool AddRequestedDeviceExtensions(const char *dev_ext_name);
    // Returns true if the device extension is promoted to core in the API version supported by the device.
    bool IsPromotedDeviceExtension(const char *dev_ext_name) const;
    // Returns true if the device extension is enabled. This call is only valid _after_ previous `AddRequired*Extensions` calls and
    // InitFramework has been called.
    // `dev_ext_name` must be an instance extension name; false is returned for all instance extension names.
    // This function also returns true if the device extension is implicitly supported by the API version supported
    // by the device, as queriable using DeviceValidationVersion().
    bool CanEnableDeviceExtension(const std::string &dev_ext_name) const;
};
