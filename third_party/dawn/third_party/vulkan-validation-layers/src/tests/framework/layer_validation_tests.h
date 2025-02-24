/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <vulkan/vulkan.h>

#include "../layers/vk_lunarg_device_profile_api_layer.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/log.h>
#include <android_native_app_glue.h>
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include "wayland-client.h"
#endif

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/utility/vk_struct_helper.hpp>

// Remove Windows macro that prevents usage of its name in any scope of the program.
// For example, BitstreamBuffer::MemoryBarrier() won't compile on ARM64.
#if defined(VK_USE_PLATFORM_WIN32_KHR) && defined(MemoryBarrier)
#undef MemoryBarrier
#endif

#include "binding.h"
#include "containers/custom_containers.h"
#include "generated/vk_extension_helper.h"
#include "render.h"

#include <cmath>
#include <functional>
#include <limits>
#include <string>
#include <vector>

// MSVC and GCC define __SANITIZE_ADDRESS__ when compiling with address sanitization
// However, clang doesn't. Instead you have to use __has_feature to check.
#if defined(__clang__)
#if __has_feature(address_sanitizer)
#define VVL_ENABLE_ASAN 1
#endif
#elif defined(__SANITIZE_ADDRESS__)
#define VVL_ENABLE_ASAN 1
#endif

// GCC defines __SANITIZE_THREAD__ when compiling with address sanitization
// However, clang doesn't. Instead you have to use __has_feature to check.
#if defined(__clang__)
#if __has_feature(thread_sanitizer)
#define VVL_ENABLE_TSAN 1
#endif
#elif defined(__SANITIZE_THREAD__)
#define VVL_ENABLE_TSAN 1
#endif

#if defined(VVL_ENABLE_ASAN)
#if __has_include(<sanitizer/lsan_interface.h>)
#include <sanitizer/lsan_interface.h>
#else
#error The lsan_interface.h header was not found!
#endif
#endif

#define OBJECT_LAYER_NAME "VK_LAYER_KHRONOS_validation"

// This is only for tests where you have a good reason to have more than the default (10) duplicate message limit.
// It is highly suggested you first try to breakup your test up into smaller tests if you are trying to use this.
static VkBool32 kVkFalse = VK_FALSE;
static const VkLayerSettingEXT kDisableMessageLimitSetting = {OBJECT_LAYER_NAME, "enable_message_limit",
                                                              VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkFalse};
[[maybe_unused]] static VkLayerSettingsCreateInfoEXT kDisableMessageLimit = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
                                                                             nullptr, 1, &kDisableMessageLimitSetting};

// Static arrays helper
template <class ElementT, size_t array_size>
uint32_t size32(ElementT (&)[array_size]) {
    return static_cast<uint32_t>(array_size);
}

template <class Container>
uint32_t size32(const Container &c) {
    return static_cast<uint32_t>(c.size());
}

// Format search helper
VkFormat FindSupportedDepthOnlyFormat(VkPhysicalDevice phy);
VkFormat FindSupportedStencilOnlyFormat(VkPhysicalDevice phy);
VkFormat FindSupportedDepthStencilFormat(VkPhysicalDevice phy);

// Returns true if *any* requested features are available.
// Assumption is that the framework can successfully create an image as
// long as at least one of the feature bits is present (excepting VTX_BUF).
bool FormatIsSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                       VkFormatFeatureFlags features = ~VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);

// Returns true if format and *all* requested features are available.
bool FormatFeaturesAreSupported(VkPhysicalDevice phy, VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features);

// Returns true if format and *all* requested features are available.
bool ImageFormatIsSupported(const VkInstance inst, const VkPhysicalDevice phy, const VkImageCreateInfo info,
                            const VkFormatFeatureFlags features);

// Returns true if format and *all* requested features are available.
bool BufferFormatAndFeaturesSupported(VkPhysicalDevice phy, VkFormat format, VkFormatFeatureFlags features);

// Simple sane SamplerCreateInfo boilerplate
VkSamplerCreateInfo SafeSaneSamplerCreateInfo();

// Dependent "false" type for the static assert, as GCC will evaluate
// non-dependent static_asserts even for non-instantiated templates
template <typename T>
struct AlwaysFalse : std::false_type {};

// Helpers to get nearest greater or smaller value (of float) -- useful for testing the boundary cases of Vulkan limits
template <typename T>
T NearestGreater(const T from) {
    using Lim = std::numeric_limits<T>;
    const auto positive_direction = Lim::has_infinity ? Lim::infinity() : Lim::max();

    return std::nextafter(from, positive_direction);
}

template <typename T>
T NearestSmaller(const T from) {
    using Lim = std::numeric_limits<T>;
    const auto negative_direction = Lim::has_infinity ? -Lim::infinity() : Lim::lowest();

    return std::nextafter(from, negative_direction);
}

// Defining VVL_TESTS_USE_CUSTOM_TEST_FRAMEWORK allows downstream users
// to inject custom test framework changes. This includes the ability
// to override the the base class of the VkLayerTest class so that
// appropriate test framework customizations can be injected into the
// class hierarchy at the closest possible place to the base class used
// by all validation layer tests. Downstream users can provide their
// own version of custom_test_framework.h to define the appropriate
// custom base class to use through the VkLayerTestBase type identifier.
#ifdef VVL_TESTS_USE_CUSTOM_TEST_FRAMEWORK
#include "framework/custom_test_framework.h"
#else
using VkLayerTestBase = VkRenderFramework;
#endif

// VkLayerTest is the main GTest test class
// It is the root for all other test class variations
class VkLayerTest : public VkLayerTestBase {
  public:
    const char *kValidationLayerName = "VK_LAYER_KHRONOS_validation";
    const char *kSynchronization2LayerName = "VK_LAYER_KHRONOS_synchronization2";

    void Init(VkPhysicalDeviceFeatures *features = nullptr, VkPhysicalDeviceFeatures2 *features2 = nullptr,
              void *instance_pnext = nullptr);
    void AddSurfaceExtension();

    template <typename Features>
    VkPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2(Features &feature_query) {
        VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&feature_query);
        return GetPhysicalDeviceFeatures2(features2);
    }

    template <typename Properties>
    VkPhysicalDeviceProperties2 GetPhysicalDeviceProperties2(Properties &props_query) {
        VkPhysicalDeviceProperties2 props2 = vku::InitStructHelper(&props_query);
        return GetPhysicalDeviceProperties2(props2);
    }

    template <typename Proc, bool assert_proc = true>
    [[nodiscard]] const Proc GetInstanceProcAddr(const char *proc_name) const noexcept {
        static_assert(std::is_pointer_v<Proc>);

        auto proc = reinterpret_cast<Proc>(vk::GetInstanceProcAddr(instance(), proc_name));
        if constexpr (assert_proc) {
            assert(proc);
        }
        return proc;
    }

    template <typename Proc, bool assert_proc = true>
    [[nodiscard]] const Proc GetDeviceProcAddr(const char *proc_name) noexcept {
        static_assert(std::is_pointer_v<Proc>);

        auto proc = reinterpret_cast<Proc>(vk::GetDeviceProcAddr(device(), proc_name));
        if constexpr (assert_proc) {
            assert(proc);
        }
        return proc;
    }
    APIVersion DeviceValidationVersion() const;

  protected:
    void SetTargetApiVersion(APIVersion target_api_version);
    bool LoadDeviceProfileLayer(
        PFN_vkSetPhysicalDeviceFormatPropertiesEXT &fpvkSetPhysicalDeviceFormatPropertiesEXT,
        PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT &fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT);
    bool LoadDeviceProfileLayer(
        PFN_vkSetPhysicalDeviceFormatProperties2EXT &fpvkSetPhysicalDeviceFormatProperties2EXT,
        PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT &fpvkGetOriginalPhysicalDeviceFormatProperties2EXT);
    bool LoadDeviceProfileLayer(PFN_vkSetPhysicalDeviceLimitsEXT &fpvkSetPhysicalDeviceLimitsEXT,
                                PFN_vkGetOriginalPhysicalDeviceLimitsEXT &fpvkGetOriginalPhysicalDeviceLimitsEXT);
    bool LoadDeviceProfileLayer(PFN_vkSetPhysicalDeviceFeaturesEXT &fpvkSetPhysicalDeviceFeaturesEXT,
                                PFN_vkGetOriginalPhysicalDeviceFeaturesEXT &fpvkGetOriginalPhysicalDeviceFeaturesEXT);
    bool LoadDeviceProfileLayer(PFN_VkSetPhysicalDeviceProperties2EXT &fpvkSetPhysicalDeviceProperties2EXT);

    VkLayerTest();
};

template <>
VkPhysicalDeviceFeatures2 VkLayerTest::GetPhysicalDeviceFeatures2(VkPhysicalDeviceFeatures2 &feature_query);

template <>
VkPhysicalDeviceProperties2 VkLayerTest::GetPhysicalDeviceProperties2(VkPhysicalDeviceProperties2 &props2);

class VkBestPracticesLayerTest : public VkLayerTest {
  public:
    void InitBestPracticesFramework(const char *ValidationChecksToEnable = "");
    void InitBestPractices(const char *ValidationChecksToEnable = "");

  protected:
    VkValidationFeatureEnableEXT enables_[1] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
    VkValidationFeaturesEXT features_ = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1, enables_, 0, nullptr};
};

class GpuAVTest : public virtual VkLayerTest {
  public:
    void InitGpuAvFramework(void *p_next = nullptr);

    VkValidationFeaturesEXT GetGpuAvValidationFeatures();
};

class GpuAVBufferDeviceAddressTest : public GpuAVTest {
  public:
    void InitGpuVUBufferDeviceAddress(void *p_next = nullptr);
};

class GpuAVDescriptorIndexingTest : public GpuAVTest {
  public:
    void InitGpuVUDescriptorIndexing();
};

class GpuAVDescriptorClassGeneralBuffer : public GpuAVTest {
  public:
    void ComputeStorageBufferTest(const char *shader, bool is_glsl, VkDeviceSize buffer_size, const char *expected_error = nullptr);
};

class GpuAVRayQueryTest : public GpuAVTest {
  public:
    void InitGpuAVRayQuery();
};

class GpuAVImageLayout : public GpuAVTest {
  public:
    void InitGpuAVImageLayout();
};

class DebugPrintfTests : public VkLayerTest {
  public:
    void InitDebugPrintfFramework(void *p_next = nullptr, bool reserve_slot = false);
};

struct SyncValSettings;
class VkSyncValTest : public VkLayerTest {
  public:
    void InitSyncValFramework(const SyncValSettings *p_sync_settings = nullptr);
    void InitSyncVal(const SyncValSettings *p_sync_settings = nullptr);
    void InitTimelineSemaphore();
    void InitRayTracing();
};

class AndroidExternalResolveTest : public VkLayerTest {
  public:
    void InitBasicAndroidExternalResolve();
    bool nullColorAttachmentWithExternalFormatResolve;
};

class DescriptorBufferTest : public VkLayerTest {
  public:
    void InitBasicDescriptorBuffer(void *instance_pnext = nullptr);
};

class DescriptorIndexingTest : public VkLayerTest {
  public:
    void ComputePipelineShaderTest(const char *shader, std::vector<VkDescriptorSetLayoutBinding> &bindings);
};

class DynamicRenderingTest : public VkLayerTest {
  public:
    void InitBasicDynamicRendering();
    void InitBasicDynamicRenderingLocalRead();
};

class DynamicStateTest : public VkLayerTest {
  public:
    void InitBasicExtendedDynamicState();  // enables VK_EXT_extended_dynamic_state
};

class ExternalMemorySyncTest : public VkLayerTest {
  protected:
#ifdef VK_USE_PLATFORM_WIN32_KHR
    using ExternalHandle = HANDLE;
#else
    using ExternalHandle = int;
#endif
};

class DeviceGeneratedCommandsTest : public VkLayerTest {
  public:
    void InitBasicDeviceGeneratedCommands();

    void SetPreProcessBuffer(VkGeneratedCommandsInfoEXT &generated_commands_info);
    std::unique_ptr<vkt::Buffer> pre_process_buffer_ = std::make_unique<vkt::Buffer>();
};

class GraphicsLibraryTest : public VkLayerTest {
  public:
    void InitBasicGraphicsLibrary();
};

class HostImageCopyTest : public VkLayerTest {
  public:
    void InitHostImageCopyTest();
    bool CopyLayoutSupported(const std::vector<VkImageLayout> &copy_src_layouts, const std::vector<VkImageLayout> &copy_dst_layouts,
                             VkImageLayout layout);
    std::vector<VkImageLayout> copy_src_layouts;
    std::vector<VkImageLayout> copy_dst_layouts;

    // Every test will use these, set the default most will use
    uint32_t width = 32;
    uint32_t height = 32;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageCreateInfo image_ci;
};

class ImageTest : public VkLayerTest {
  public:
    VkImageCreateInfo DefaultImageInfo();
};

class ImageDrmTest : public VkLayerTest {
  public:
    void InitBasicImageDrm();
    std::vector<uint64_t> GetFormatModifier(VkFormat format, VkFormatFeatureFlags2 features, uint32_t plane_count = 1);
};

class QueryTest : public VkLayerTest {
  public:
    bool HasZeroTimestampValidBits();
};

class RayTracingTest : public virtual VkLayerTest {
  public:
    void InitFrameworkForRayTracingTest(VkValidationFeaturesEXT *enabled_features = nullptr);

    void NvInitFrameworkForRayTracingTest(VkPhysicalDeviceFeatures2KHR *features2 = nullptr,
                                          VkValidationFeaturesEXT *enabled_features = nullptr);
};

class GpuAVRayTracingTest : public GpuAVTest, public RayTracingTest {};

class ShaderObjectTest : public virtual VkLayerTest {
  public:
    void InitBasicShaderObject();
    void InitBasicMeshShaderObject(APIVersion target_api_version);

    // Many tests just need a basic vert/frag shader
    vkt::Shader m_vert_shader;
    vkt::Shader m_frag_shader;
    void CreateMinimalShaders();
};

class SyncObjectTest : public VkLayerTest {
  protected:
#ifdef VK_USE_PLATFORM_WIN32_KHR
    using ExternalHandle = HANDLE;
#else
    using ExternalHandle = int;
#endif
};

class WsiTest : public VkLayerTest {
  public:
    // most tests need images in VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout
    void SetImageLayoutPresentSrc(VkImage image);
    VkImageMemoryBarrier TransitionToPresent(VkImage swapchain_image, VkImageLayout old_layout, VkAccessFlags src_access_mask);

  protected:
    // Find physical device group that contains physical device selected by the test framework
    std::optional<VkPhysicalDeviceGroupProperties> FindPhysicalDeviceGroup();

  protected:
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    struct WaylandContext {
        wl_display *display = nullptr;
        wl_registry *registry = nullptr;
        wl_surface *surface = nullptr;
        wl_compositor *compositor = nullptr;
    };
    void InitWaylandContext(WaylandContext& context);
    void ReleaseWaylandContext(WaylandContext& context);
#endif
};

class CooperativeMatrixTest : public VkLayerTest {
  public:
    void InitCooperativeMatrixKHR();
    bool HasValidProperty(VkScopeKHR scope, uint32_t m, uint32_t n, uint32_t k, VkComponentTypeKHR type);
    std::vector<VkCooperativeMatrixPropertiesKHR> coop_matrix_props;
    std::vector<VkCooperativeMatrixFlexibleDimensionsPropertiesNV> coop_matrix_flex_props;
};

class ParentTest : public VkLayerTest {
  public:
    ~ParentTest();
    vkt::Device *m_second_device = nullptr;
};

template <typename T>
bool IsValidVkStruct(const T &s) {
    return vku::GetSType<T>() == s.sType;
}

struct DebugUtilsLabelCheckData {
    std::function<void(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *)> callback;
    size_t count;
};

bool operator==(const VkDebugUtilsLabelEXT &rhs, const VkDebugUtilsLabelEXT &lhs);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

void TestRenderPassCreate(ErrorMonitor *error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo &create_info,
                          bool rp2_supported, const char *rp1_vuid, const char *rp2_vuid);
void PositiveTestRenderPassCreate(ErrorMonitor *error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo &create_info,
                                  bool rp2_supported);
void PositiveTestRenderPass2KHRCreate(const vkt::Device &device, const VkRenderPassCreateInfo2KHR &create_info);
void TestRenderPass2KHRCreate(ErrorMonitor &error_monitor, const vkt::Device &device, const VkRenderPassCreateInfo2KHR &create_info,
                              const std::initializer_list<const char *> &vuids);
void TestRenderPassBegin(ErrorMonitor *error_monitor, const VkDevice device, const VkCommandBuffer command_buffer,
                         const VkRenderPassBeginInfo *begin_info, bool rp2Supported, const char *rp1_vuid, const char *rp2_vuid);

VkResult GPDIFPHelper(VkPhysicalDevice dev, const VkImageCreateInfo *ci, VkImageFormatProperties *limits = nullptr);

VkFormat FindFormatWithoutFeatures(VkPhysicalDevice gpu, VkImageTiling tiling,
                                   VkFormatFeatureFlags undesired_features = vvl::kU32Max);

VkFormat FindFormatWithoutFeatures2(VkPhysicalDevice gpu, VkImageTiling tiling, VkFormatFeatureFlags2 undesired_features);

void CreateSamplerTest(VkLayerTest &test, const VkSamplerCreateInfo *pCreateInfo, const std::string &code = "");

void CreateBufferTest(VkLayerTest &test, const VkBufferCreateInfo *pCreateInfo, const std::string &code = "");

void CreateImageTest(VkLayerTest &test, const VkImageCreateInfo *pCreateInfo, const std::string &code = "");

void CreateBufferViewTest(VkLayerTest &test, const VkBufferViewCreateInfo *pCreateInfo, const std::vector<std::string> &codes);

void CreateImageViewTest(VkLayerTest &test, const VkImageViewCreateInfo *pCreateInfo, const std::string &code = "");

void PrintAndroid(const char *c);
