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
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "render.h"

#include <cassert>
#include <cstring>

#include <vulkan/utility/vk_format_utils.h>

#include "generated/vk_extension_helper.h"
#include "layer_validation_tests.h"
#include "vk_layer_config.h"
#include "shader_helper.h"

#if defined(VK_USE_PLATFORM_METAL_EXT)
#include "apple_wsi.h"
#endif

template <typename C, typename F>
typename C::iterator RemoveIf(C &container, F &&fn) {
    return container.erase(std::remove_if(container.begin(), container.end(), std::forward<F>(fn)), container.end());
}

VkRenderFramework::VkRenderFramework()
    : instance_(nullptr),
      m_device(nullptr),
      m_renderPass(VK_NULL_HANDLE),
      m_vertex_buffer(nullptr),
      m_width(256),   // default window width
      m_height(256),  // default window height
      m_render_target_fmt(VK_FORMAT_R8G8B8A8_UNORM),
      m_depth_stencil_fmt(VK_FORMAT_UNDEFINED),
      m_depthStencil(nullptr),
      m_framebuffer(nullptr) {
    m_renderPassBeginInfo = vku::InitStructHelper();

    // clear the back buffer to dark grey
    m_clear_color.float32[0] = 0.25f;
    m_clear_color.float32[1] = 0.25f;
    m_clear_color.float32[2] = 0.25f;
    m_clear_color.float32[3] = 0.0f;
}

VkRenderFramework::~VkRenderFramework() {
    ShutdownFramework();
    m_errorMonitor->Finish();
}

VkPhysicalDevice VkRenderFramework::Gpu() const {
    EXPECT_NE((VkInstance)0, instance_);  // Invalid to request gpu before instance exists
    return gpu_;
}

const VkPhysicalDeviceProperties &VkRenderFramework::PhysicalDeviceProps() const {
    EXPECT_NE((VkPhysicalDevice)0, gpu_);  // Invalid to request physical device properties before gpu
    return physDevProps_;
}

// Return true if layer name is found and spec+implementation values are >= requested values
bool VkRenderFramework::InstanceLayerSupported(const char *const layer_name, const uint32_t spec_version,
                                               const uint32_t impl_version) {
    if (available_layers_.empty()) {
        available_layers_ = vkt::GetGlobalLayers();
    }

    for (const auto &layer : available_layers_) {
        if (0 == strncmp(layer_name, layer.layerName, VK_MAX_EXTENSION_NAME_SIZE)) {
            return layer.specVersion >= spec_version && layer.implementationVersion >= impl_version;
        }
    }
    return false;
}

// Return true if extension name is found and spec value is >= requested spec value
// WARNING: for simplicity, does not cover layers' extensions
bool VkRenderFramework::InstanceExtensionSupported(const char *const extension_name, const uint32_t spec_version) {
    // WARNING: assume debug and validation feature extensions are always supported, which are usually provided by layers
    if (0 == strncmp(extension_name, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;
    if (0 == strncmp(extension_name, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;
    if (0 == strncmp(extension_name, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE)) return true;

    if (available_extensions_.empty()) {
        available_extensions_ = vkt::GetGlobalExtensions();
    }

    const auto IsTheQueriedExtension = [extension_name, spec_version](const VkExtensionProperties &extension) {
        return strncmp(extension_name, extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0 &&
               extension.specVersion >= spec_version;
    };

    return std::any_of(available_extensions_.begin(), available_extensions_.end(), IsTheQueriedExtension);
}

// Return true if extension name is found and spec value is >= requested spec value
bool VkRenderFramework::DeviceExtensionSupported(const char *extension_name, const uint32_t spec_version) const {
    if (!instance_ || !gpu_) {
        EXPECT_NE((VkInstance)0, instance_);  // Complain, not cool without an instance
        EXPECT_NE((VkPhysicalDevice)0, gpu_);
        return false;
    }

    const vkt::PhysicalDevice device_obj(gpu_);

    const auto enabled_layers = instance_layers_;  // assumes instance_layers_ contains enabled layers

    auto extensions = device_obj.Extensions();
    for (const auto &layer : enabled_layers) {
        const auto layer_extensions = device_obj.Extensions(layer);
        extensions.insert(extensions.end(), layer_extensions.begin(), layer_extensions.end());
    }

    const auto IsTheQueriedExtension = [extension_name, spec_version](const VkExtensionProperties &extension) {
        return strncmp(extension_name, extension.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0 &&
               extension.specVersion >= spec_version;
    };

    return std::any_of(extensions.begin(), extensions.end(), IsTheQueriedExtension);
}

VkInstanceCreateInfo VkRenderFramework::GetInstanceCreateInfo() const {
    VkInstanceCreateInfo info = vku::InitStructHelper();
    info.pNext = m_errorMonitor->GetDebugCreateInfo();
#if defined(VK_USE_PLATFORM_METAL_EXT)
    info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    info.pApplicationInfo = &app_info_;
    info.enabledLayerCount = size32(instance_layers_);
    info.ppEnabledLayerNames = instance_layers_.data();
    info.enabledExtensionCount = size32(m_instance_extension_names);
    info.ppEnabledExtensionNames = m_instance_extension_names.data();
    return info;
}

void VkRenderFramework::InitFramework(void *instance_pnext) {
    ASSERT_EQ((VkInstance)0, instance_);

    const auto ExtensionIncludedInTargetVersion = [this](const char *extension) {
        if (IsPromotedInstanceExtension(extension)) {
            // Replicate the core entry points into the extension entry points
            vk::InitExtensionFromCore(extension);
            return true;
        }
        return false;
    };
    const auto LayerNotSupportedWithReporting = [this](const char *layer) {
        if (InstanceLayerSupported(layer))
            return false;
        else {
            ADD_FAILURE() << "InitFramework(): Requested layer \"" << layer << "\" is not supported. It will be disabled.";
            return true;
        }
    };
    const auto ExtensionNotSupportedWithReporting = [this](const char *extension) {
        if (InstanceExtensionSupported(extension))
            return false;
        else {
            ADD_FAILURE() << "InitFramework(): Requested extension \"" << extension << "\" is not supported. It will be disabled.";
            return true;
        }
    };

    // Beginning with the 1.3.216 Vulkan SDK, the VK_KHR_PORTABILITY_subset extension is mandatory.
#ifdef VK_USE_PLATFORM_METAL_EXT
    AddRequiredExtensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#else
    // Note by default VK_KHRONOS_PROFILES_EMULATE_PORTABILITY is true.
    if (auto str = GetEnvironment("VK_KHRONOS_PROFILES_EMULATE_PORTABILITY"); !str.empty() && str != "false") {
        AddRequiredExtensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        AddRequiredExtensions(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
#endif

    vk::ResetAllExtensions();

    // Remove promoted extensions from both the instance and required extension lists
    if (!allow_promoted_extensions_) {
        RemoveIf(m_required_extensions, ExtensionIncludedInTargetVersion);
        RemoveIf(m_optional_extensions, ExtensionIncludedInTargetVersion);
        RemoveIf(m_instance_extension_names, ExtensionIncludedInTargetVersion);
    }

    RemoveIf(instance_layers_, LayerNotSupportedWithReporting);
    RemoveIf(m_instance_extension_names, ExtensionNotSupportedWithReporting);

    auto ici = GetInstanceCreateInfo();

    // concatenate pNexts
    void *last_pnext = nullptr;
    if (instance_pnext) {
        last_pnext = instance_pnext;
        while (reinterpret_cast<const VkBaseOutStructure *>(last_pnext)->pNext)
            last_pnext = reinterpret_cast<VkBaseOutStructure *>(last_pnext)->pNext;

        void *&link = reinterpret_cast<void *&>(reinterpret_cast<VkBaseOutStructure *>(last_pnext)->pNext);
        link = const_cast<void *>(ici.pNext);
        ici.pNext = instance_pnext;
    }

    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &instance_));
    if (instance_pnext) reinterpret_cast<VkBaseOutStructure *>(last_pnext)->pNext = nullptr;  // reset back borrowed pNext chain

    for (const char *instance_ext_name : m_instance_extension_names) {
        vk::InitInstanceExtension(instance_, instance_ext_name);
    }

    // Choose a physical device
    uint32_t gpu_count = 0;
    const VkResult err = vk::EnumeratePhysicalDevices(instance_, &gpu_count, nullptr);
    ASSERT_TRUE(err == VK_SUCCESS || err == VK_INCOMPLETE) << string_VkResult(err);
    ASSERT_GT(gpu_count, (uint32_t)0) << "No GPU (i.e. VkPhysicalDevice) available";

    std::vector<VkPhysicalDevice> phys_devices(gpu_count);
    vk::EnumeratePhysicalDevices(instance_, &gpu_count, phys_devices.data());

    const int phys_device_index = VkTestFramework::m_phys_device_index;
    if ((phys_device_index >= 0) && (phys_device_index < static_cast<int>(gpu_count))) {
        gpu_ = phys_devices[phys_device_index];
        vk::GetPhysicalDeviceProperties(gpu_, &physDevProps_);
        m_gpu_index = phys_device_index;
    } else {
        // Specify a "physical device priority" with larger values meaning higher priority.
        std::array<int, VK_PHYSICAL_DEVICE_TYPE_CPU + 1> device_type_rank;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU] = 4;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU] = 3;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU] = 2;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_CPU] = 1;
        device_type_rank[VK_PHYSICAL_DEVICE_TYPE_OTHER] = 0;

        // Initialize physical device and properties with first device found
        gpu_ = phys_devices[0];
        m_gpu_index = 0;
        vk::GetPhysicalDeviceProperties(gpu_, &physDevProps_);

        // See if there are any higher priority devices found
        for (size_t i = 1; i < phys_devices.size(); ++i) {
            VkPhysicalDeviceProperties tmp_props;
            vk::GetPhysicalDeviceProperties(phys_devices[i], &tmp_props);
            if (device_type_rank[tmp_props.deviceType] > device_type_rank[physDevProps_.deviceType]) {
                physDevProps_ = tmp_props;
                gpu_ = phys_devices[i];
                m_gpu_index = i;
            }
        }
    }

    m_errorMonitor->CreateCallback(instance_);

    static bool driver_printed = false;
    static bool print_driver_info = GetEnvironment("VK_LAYER_TESTS_PRINT_DRIVER") != "";

    if (print_driver_info && !driver_printed) {
        bool phys_dev_props_2_enabled = m_target_api_version.Minor() >= 1;
        if (!phys_dev_props_2_enabled) {
            for (const char *ext : vvl::make_span(ici.ppEnabledExtensionNames, ici.enabledExtensionCount)) {
                if (strcmp(ext, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
                    phys_dev_props_2_enabled = true;
                    break;
                }
            }
        }

        if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) && phys_dev_props_2_enabled) {
            VkPhysicalDeviceDriverProperties driver_properties = vku::InitStructHelper();
            VkPhysicalDeviceProperties2 physical_device_properties2 = vku::InitStructHelper(&driver_properties);
            vk::GetPhysicalDeviceProperties2(gpu_, &physical_device_properties2);
            printf("Driver Name = %s\n", driver_properties.driverName);
            printf("Driver Info = %s\n", driver_properties.driverInfo);

            driver_printed = true;
        } else {
            printf(
                "Could not print driver info - VK_KHR_get_physical_device_properties2 is either not supported or not enabled.\n");
        }
    }

    const APIVersion used_version = std::min(m_instance_api_version, APIVersion(physDevProps_.apiVersion));
    if (used_version < m_target_api_version) {
        GTEST_SKIP() << "At least Vulkan version 1." << m_target_api_version.Minor() << " is required";
    }

    for (const auto &ext : m_required_extensions) {
        AddRequestedDeviceExtensions(ext);
    }

    if (!std::all_of(m_required_extensions.begin(), m_required_extensions.end(),
                     [&](const char *ext) -> bool { return IsExtensionsEnabled(ext); })) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // If the user requested wsi extension(s), only 1 needs to be enabled.
    if (!m_wsi_extensions.empty()) {
        if (!std::any_of(m_wsi_extensions.begin(), m_wsi_extensions.end(),
                         [&](const char *ext) -> bool { return CanEnableInstanceExtension(ext); })) {
            GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
        }
    }

    for (const auto &ext : m_optional_extensions) {
        AddRequestedDeviceExtensions(ext);
    }
}

void VkRenderFramework::AddRequiredExtensions(const char *ext_name) {
    m_required_extensions.push_back(ext_name);
    AddRequestedInstanceExtensions(ext_name);
}

void VkRenderFramework::AddOptionalExtensions(const char *ext_name) {
    m_optional_extensions.push_back(ext_name);
    AddRequestedInstanceExtensions(ext_name);
}

void VkRenderFramework::AddWsiExtensions(const char *ext_name) {
    m_wsi_extensions.push_back(ext_name);
    AddRequestedInstanceExtensions(ext_name);
}

bool VkRenderFramework::IsExtensionsEnabled(const char *ext_name) const {
    return (CanEnableDeviceExtension(ext_name) || CanEnableInstanceExtension(ext_name));
}

std::string VkRenderFramework::RequiredExtensionsNotSupported() const {
    std::stringstream ss;
    bool first = true;
    for (const auto &ext : m_required_extensions) {
        if (!CanEnableDeviceExtension(ext) && !CanEnableInstanceExtension(ext)) {
            if (first) {
                first = false;
            } else {
                ss << ", ";
            }
            ss << ext;
        }
    }
    if (!m_wsi_extensions.empty() && ss.str().empty()) {
        ss << "Unable to find at least 1 supported WSI extension";
    }
    return ss.str();
}

void VkRenderFramework::AddRequiredFeature(vkt::Feature feature) {
    required_features_.AddRequiredFeature(m_target_api_version, feature);
    features_to_enable_.AddRequiredFeature(m_target_api_version, feature);
}

void VkRenderFramework::AddOptionalFeature(vkt::Feature feature) {
    features_to_enable_.AddOptionalFeature(m_target_api_version, feature);
}

bool VkRenderFramework::AddRequestedInstanceExtensions(const char *ext_name) {
    if (CanEnableInstanceExtension(ext_name)) {
        return true;
    }

    const auto &instance_exts_map = InstanceExtensions::GetInfoMap();
    bool is_instance_ext = false;
    vvl::Extension extension = GetExtension(ext_name);
    if (instance_exts_map.find(extension) != instance_exts_map.cend()) {
        if (!InstanceExtensionSupported(ext_name)) {
            return false;
        } else {
            is_instance_ext = true;
        }
    }

    // Different tables need to be used for extension dependency lookup depending on whether `ext_name` refers to a device or
    // instance extension
    if (is_instance_ext) {
        const auto &info = InstanceExtensions::GetInfo(extension);
        for (const auto &req : info.requirements) {
            if (0 == strncmp(req.name, "VK_VERSION", 10)) {
                continue;
            }
            if (!AddRequestedInstanceExtensions(req.name)) {
                return false;
            }
        }
        m_instance_extension_names.push_back(ext_name);
    } else {
        const auto &info = DeviceExtensions::GetInfo(extension);
        for (const auto &req : info.requirements) {
            if (!AddRequestedInstanceExtensions(req.name)) {
                return false;
            }
        }
    }

    return true;
}

bool VkRenderFramework::IsPromotedInstanceExtension(const char *inst_ext_name) const {
    if (!m_target_api_version.Valid()) return false;

    const auto promotion_info_map = GetInstancePromotionInfoMap();
    for (const auto &version_it : promotion_info_map) {
        if (m_target_api_version >= version_it.first) {
            const auto promoted_exts = version_it.second.second;
            vvl::Extension extension = GetExtension(inst_ext_name);
            if (promoted_exts.find(extension) != promoted_exts.end()) {
                return true;
            }
        }
    }

    return false;
}

bool VkRenderFramework::CanEnableInstanceExtension(const std::string &inst_ext_name) const {
    return (!allow_promoted_extensions_ && IsPromotedInstanceExtension(inst_ext_name.c_str())) ||
           std::any_of(m_instance_extension_names.cbegin(), m_instance_extension_names.cend(),
                       [&inst_ext_name](const char *ext) { return inst_ext_name == ext; });
}

bool VkRenderFramework::AddRequestedDeviceExtensions(const char *dev_ext_name) {
    // Check if the extension has already been added
    if (CanEnableDeviceExtension(dev_ext_name)) {
        return true;
    }

    if (0 == strncmp(dev_ext_name, "VK_VERSION", 10)) {
        return true;
    }

    // If this is an instance extension, just return true under the assumption instance extensions do not depend on any device
    // extensions.
    const auto &instance_exts_map = InstanceExtensions::GetInfoMap();
    vvl::Extension extension = GetExtension(dev_ext_name);
    if (instance_exts_map.find(extension) != instance_exts_map.cend()) {
        return true;
    }

    if (!DeviceExtensionSupported(Gpu(), nullptr, dev_ext_name)) {
        return false;
    }
    m_device_extension_names.push_back(dev_ext_name);

    const auto &info = DeviceExtensions::GetInfo(extension);
    for (const auto &req : info.requirements) {
        if (!AddRequestedDeviceExtensions(req.name)) {
            return false;
        }
    }
    return true;
}

bool VkRenderFramework::IsPromotedDeviceExtension(const char *dev_ext_name) const {
    auto device_version = std::min(m_target_api_version, APIVersion(PhysicalDeviceProps().apiVersion));
    if (!device_version.Valid()) return false;

    const auto promotion_info_map = GetDevicePromotionInfoMap();
    for (const auto &version_it : promotion_info_map) {
        if (device_version >= version_it.first) {
            const auto promoted_exts = version_it.second.second;
            vvl::Extension extension = GetExtension(dev_ext_name);
            if (promoted_exts.find(extension) != promoted_exts.end()) {
                return true;
            }
        }
    }

    return false;
}

bool VkRenderFramework::CanEnableDeviceExtension(const std::string &dev_ext_name) const {
    return (!allow_promoted_extensions_ && IsPromotedDeviceExtension(dev_ext_name.c_str())) ||
           std::any_of(m_device_extension_names.cbegin(), m_device_extension_names.cend(),
                       [&dev_ext_name](const char *ext) { return dev_ext_name == ext; });
}

void VkRenderFramework::ShutdownFramework() {
    // Nothing to shut down without a VkInstance
    if (!instance_) return;

    if (m_device && m_device->handle() != VK_NULL_HANDLE) {
        m_device->Wait();
    }

    m_command_buffer.destroy();
    m_command_pool.destroy();

    if (m_second_queue) {
        m_second_command_buffer.destroy();
        m_second_command_pool.destroy();
    }

    delete m_vertex_buffer;
    m_vertex_buffer = nullptr;

    delete m_framebuffer;
    m_framebuffer = nullptr;
    if (m_renderPass) vk::DestroyRenderPass(device(), m_renderPass, NULL);
    m_renderPass = VK_NULL_HANDLE;

    m_render_target_views.clear();
    m_renderTargets.clear();

    delete m_depthStencil;
    m_depthStencil = nullptr;

    DestroySwapchain();

    // reset the driver
    delete m_device;
    m_device = nullptr;

    m_errorMonitor->DestroyCallback(instance_);

    m_surface.Destroy();
    m_surface_context.Destroy();

    vk::DestroyInstance(instance_, nullptr);
    instance_ = NULL;  // In case we want to re-initialize
    vk::ResetAllExtensions();
}

ErrorMonitor &VkRenderFramework::Monitor() { return monitor_; }

void VkRenderFramework::GetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures *features) {
    vk::GetPhysicalDeviceFeatures(Gpu(), features);
}

// static
bool VkRenderFramework::IgnoreDisableChecks() {
    static const bool skip_disable_checks = GetEnvironment("VK_LAYER_TESTS_IGNORE_DISABLE_CHECKS") != "";
    return skip_disable_checks;
}

// Will also return true if using VVL Test ICD
// Using "Mock" for aliasing legacy name
static const std::string mock_icd_device_name = "Vulkan Mock Device";
bool VkRenderFramework::IsPlatformMockICD() {
    if (VkRenderFramework::IgnoreDisableChecks()) {
        return false;
    } else {
        return 0 == mock_icd_device_name.compare(PhysicalDeviceProps().deviceName);
    }
}

void VkRenderFramework::GetPhysicalDeviceProperties(VkPhysicalDeviceProperties *props) { *props = physDevProps_; }

VkFormat VkRenderFramework::GetRenderTargetFormat() {
    VkFormatProperties format_props = {};
    vk::GetPhysicalDeviceFormatProperties(gpu_, VK_FORMAT_B8G8R8A8_UNORM, &format_props);
    if (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ||
        format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        return VK_FORMAT_B8G8R8A8_UNORM;
    }
    vk::GetPhysicalDeviceFormatProperties(gpu_, VK_FORMAT_R8G8B8A8_UNORM, &format_props);
    if (format_props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ||
        format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
    // According to VulkanCapsViewer rgba8/bgra8 support with optimal tiling + color_attachment is 99.45% across all platforms
    assert(false);
    return VK_FORMAT_UNDEFINED;
}

void VkRenderFramework::InitState(VkPhysicalDeviceFeatures *features, void *create_device_pnext,
                                  const VkCommandPoolCreateFlags flags) {
    const auto ExtensionIncludedInDeviceApiVersion = [&](const char *extension) {
        if (IsPromotedDeviceExtension(extension)) {
            // Replicate the core entry points into the extension entry points
            vk::InitExtensionFromCore(extension);

            // Handle special cases which did not have a feature flag in the extension
            // but do have one in their core promoted form
            static const std::unordered_map<std::string, std::vector<vkt::Feature>> vk12_ext_features = {
                {VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, {vkt::Feature::drawIndirectCount}},
                {VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME, {vkt::Feature::samplerFilterMinmax}},
                {VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
                 {vkt::Feature::shaderOutputViewportIndex, vkt::Feature::shaderOutputLayer}},
                {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, {vkt::Feature::descriptorIndexing}},
                {VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME, {vkt::Feature::scalarBlockLayout}}};
            auto it = vk12_ext_features.find(extension);
            if (it != vk12_ext_features.end()) {
                for (const auto feature : it->second) {
                    AddRequiredFeature(feature);
                }
            }

            return true;
        }
        return false;
    };
    const auto ExtensionNotSupportedWithReporting = [this](const char *extension) {
        if (DeviceExtensionSupported(extension))
            return false;
        else {
            ADD_FAILURE() << "InitState(): Requested device extension \"" << extension
                          << "\" is not supported. It will be disabled.";
            return true;
        }
    };

    // Remove promoted extensions from both the instance and required extension lists
    if (!allow_promoted_extensions_) {
        RemoveIf(m_required_extensions, ExtensionIncludedInDeviceApiVersion);
        RemoveIf(m_optional_extensions, ExtensionIncludedInDeviceApiVersion);
        RemoveIf(m_device_extension_names, ExtensionIncludedInDeviceApiVersion);
    }

    RemoveIf(m_device_extension_names, ExtensionNotSupportedWithReporting);

    // Apply required features after we are done with handling promoted extensions
    if (!features) {
        if (required_features_.HasFeatures2()) {
            if (vk::GetPhysicalDeviceFeatures2KHR) {
                vk::GetPhysicalDeviceFeatures2KHR(Gpu(), required_features_.GetFeatures2());
            } else {
                vk::GetPhysicalDeviceFeatures2(Gpu(), required_features_.GetFeatures2());
            }
        } else {
            GetPhysicalDeviceFeatures(required_features_.GetFeatures());
        }

        if (const char *f = required_features_.AnyRequiredFeatureDisabled()) {
            GTEST_SKIP() << "Required feature " << f << " is not available on device, skipping test";
        }

        features_to_enable_.EnforceRequiredFeatures();

        if (features_to_enable_.HasFeatures2()) {
            if (create_device_pnext) {
                // Chain to the end of the list
                VkBaseOutStructure *p = reinterpret_cast<VkBaseOutStructure *>(create_device_pnext);
                while (p->pNext != nullptr) {
                    p = p->pNext;
                }
                p->pNext = reinterpret_cast<VkBaseOutStructure *>(features_to_enable_.GetFeatures2());
            } else {
                create_device_pnext = features_to_enable_.GetFeatures2();
            }
        } else {
            features = features_to_enable_.GetFeatures();
        }
    }

    m_device = new vkt::Device(gpu_, m_device_extension_names, features, create_device_pnext, all_queue_count_);

    for (const char *device_ext_name : m_device_extension_names) {
        vk::InitDeviceExtension(instance_, *m_device, device_ext_name);
    }

    std::vector<vkt::Queue *> queues;
    vvl::Append(queues, m_device->QueuesWithGraphicsCapability());
    for (vkt::Queue *queue_with_compute_caps : m_device->QueuesWithComputeCapability()) {
        if (!vvl::Contains(queues, queue_with_compute_caps)) {
            queues.emplace_back(queue_with_compute_caps);
        }
    }
    for (vkt::Queue *queue_with_transfer_caps : m_device->QueuesWithTransferCapability()) {
        if (!vvl::Contains(queues, queue_with_transfer_caps)) {
            queues.emplace_back(queue_with_transfer_caps);
        }
    }
    m_default_queue = queues[0];
    m_default_queue_caps = m_device->Physical().queue_properties_[m_default_queue->family_index].queueFlags;
    if (queues.size() > 1) {
        m_second_queue = queues[1];
        m_second_queue_caps = m_device->Physical().queue_properties_[m_second_queue->family_index].queueFlags;
    }
    if (queues.size() > 2) {
        m_third_queue = queues[2];
        m_third_queue_caps = m_device->Physical().queue_properties_[m_third_queue->family_index].queueFlags;
    }

    m_depthStencil = new vkt::Image();

    m_render_target_fmt = GetRenderTargetFormat();

    m_command_pool.Init(*m_device, m_device->graphics_queue_node_index_, flags);
    m_command_buffer.Init(*m_device, m_command_pool);

    if (m_second_queue) {
        m_second_command_pool.Init(*m_device, m_second_queue->family_index, flags);
        m_second_command_buffer.Init(*m_device, m_second_command_pool);
    }
}

void VkRenderFramework::InitSurface() {
    // NOTE: Currently InitSurface can leak the WIN32 handle if called multiple times without first calling Destroy() on
    // m_surface_context.
    // This is intentional. Each swapchain/surface combo needs a unique HWND.
    VkResult result = CreateSurface(m_surface_context, m_surface);
    if (result != VK_SUCCESS) {
        GTEST_SKIP() << "Failed to create surface.";
    }
    ASSERT_TRUE(m_surface.Handle() != VK_NULL_HANDLE);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SurfaceContext::Resize(uint32_t width, uint32_t height) {
    ::SetWindowPos(m_win32Window, NULL, 0, 0, (int)width, (int)height, SWP_NOMOVE);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult VkRenderFramework::CreateSurface(SurfaceContext &surface_context, vkt::Surface &surface, VkInstance custom_instance) {
    const VkInstance surface_instance = (custom_instance != VK_NULL_HANDLE) ? custom_instance : instance();
    (void)surface_instance;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (IsExtensionsEnabled(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        HINSTANCE window_instance = GetModuleHandle(nullptr);
        const char class_name[] = "test";
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = window_instance;
        wc.lpszClassName = class_name;
        RegisterClass(&wc);
        HWND window = CreateWindowEx(0, class_name, nullptr, 0, 0, 0, (int)m_width, (int)m_height, NULL, NULL, window_instance, NULL);
        surface_context.m_win32Window = window;
        ShowWindow(window, SW_HIDE);

        VkWin32SurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.hinstance = window_instance;
        surface_create_info.hwnd = window;
        return surface.Init(surface_instance, surface_create_info);
    }
#endif

#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (IsExtensionsEnabled(VK_EXT_METAL_SURFACE_EXTENSION_NAME)) {
        const VkMetalSurfaceCreateInfoEXT surface_create_info = vkt::CreateMetalSurfaceInfoEXT();
        assert(surface_create_info.pLayer != nullptr);
        return surface.Init(surface_instance, surface_create_info);
    }
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (IsExtensionsEnabled(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        VkAndroidSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
        surface_create_info.window = VkTestFramework::window;
        return surface.Init(surface_instance, surface_create_info);
    }
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (IsExtensionsEnabled(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        surface_context.m_surface_dpy = XOpenDisplay(nullptr);
        if (surface_context.m_surface_dpy) {
            int s = DefaultScreen(surface_context.m_surface_dpy);
            surface_context.m_surface_window = XCreateSimpleWindow(
                surface_context.m_surface_dpy, RootWindow(surface_context.m_surface_dpy, s), 0, 0, (int)m_width, (int)m_height, 1,
                BlackPixel(surface_context.m_surface_dpy, s), WhitePixel(surface_context.m_surface_dpy, s));
            VkXlibSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
            surface_create_info.dpy = surface_context.m_surface_dpy;
            surface_create_info.window = surface_context.m_surface_window;
            return surface.Init(surface_instance, surface_create_info);
        }
    }
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (IsExtensionsEnabled(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        surface_context.m_surface_xcb_conn = xcb_connect(nullptr, nullptr);
        int err = xcb_connection_has_error(surface_context.m_surface_xcb_conn);
        if (surface_context.m_surface_xcb_conn && !err) {
            xcb_window_t window = xcb_generate_id(surface_context.m_surface_xcb_conn);
            VkXcbSurfaceCreateInfoKHR surface_create_info = vku::InitStructHelper();
            surface_create_info.connection = surface_context.m_surface_xcb_conn;
            surface_create_info.window = window;
            return surface.Init(surface_instance, surface_create_info);
        }
    }
#endif

    return VK_ERROR_UNKNOWN;
}

#if defined(VK_USE_PLATFORM_XLIB_KHR)
int IgnoreXErrors(Display *, XErrorEvent *) { return 0; }
#endif

void SurfaceContext::Destroy() {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (m_win32Window != nullptr) {
        DestroyWindow(m_win32Window);
        m_win32Window = nullptr;
    }
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (m_surface_dpy != nullptr) {
        // Ignore BadDrawable errors we seem to get during shutdown.
        // The default error handler will exit() and end the test suite.
        XSetErrorHandler(IgnoreXErrors);
        XDestroyWindow(m_surface_dpy, m_surface_window);
        m_surface_window = None;
        XCloseDisplay(m_surface_dpy);
        m_surface_dpy = nullptr;
        XSetErrorHandler(nullptr);
    }
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (m_surface_xcb_conn != nullptr) {
        xcb_disconnect(m_surface_xcb_conn);
        m_surface_xcb_conn = nullptr;
    }
#endif
}

// Queries the info needed to create a swapchain and assigns it to the member variables of VkRenderFramework
void VkRenderFramework::InitSwapchainInfo() {
    auto info = GetSwapchainInfo(m_surface.Handle());
    m_surface_capabilities = info.surface_capabilities;
    m_surface_formats = info.surface_formats;
    m_surface_present_modes = info.surface_present_modes;
    m_surface_non_shared_present_mode = info.surface_non_shared_present_mode;
    m_surface_composite_alpha = info.surface_composite_alpha;
}

// Makes query to get information about swapchain needed to create a valid swapchain object each test creating a swapchain will
// need
SurfaceInformation VkRenderFramework::GetSwapchainInfo(const VkSurfaceKHR surface) {
    const VkPhysicalDevice physicalDevice = Gpu();

    assert(surface != VK_NULL_HANDLE);

    SurfaceInformation info{};

    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &info.surface_capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, nullptr);
    if (format_count != 0) {
        info.surface_formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, info.surface_formats.data());
    }

    uint32_t present_mode_count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        info.surface_present_modes.resize(present_mode_count);
        vk::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &present_mode_count,
                                                    info.surface_present_modes.data());

        // Shared Present mode has different requirements most tests won't actually want
        // Implementation required to support a non-shared present mode
        for (size_t i = 0; i < info.surface_present_modes.size(); i++) {
            const VkPresentModeKHR present_mode = info.surface_present_modes[i];
            if ((present_mode != VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) &&
                (present_mode != VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR)) {
                info.surface_non_shared_present_mode = present_mode;
                break;
            }
        }
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    info.surface_composite_alpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    info.surface_composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif

    return info;
}

void VkRenderFramework::InitSwapchain(VkImageUsageFlags imageUsage, VkSurfaceTransformFlagBitsKHR preTransform) {
    RETURN_IF_SKIP(InitSurface());
    m_swapchain = CreateSwapchain(m_surface.Handle(), imageUsage, preTransform);
    ASSERT_TRUE(m_swapchain.initialized());
}

vkt::Swapchain VkRenderFramework::CreateSwapchain(VkSurfaceKHR surface, VkImageUsageFlags imageUsage,
                                                  VkSurfaceTransformFlagBitsKHR preTransform, VkSwapchainKHR oldSwapchain) {
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(Gpu(), m_device->graphics_queue_node_index_, surface, &supported);
    if (!supported) {
        // Graphics queue does not support present
        return vkt::Swapchain{};
    }

    SurfaceInformation info = GetSwapchainInfo(surface);

    // If this is being called from InitSwapchain, we need to also initialize all the VkRenderFramework
    // data associated with the swapchain since many tests use those variables. We can do this by checking
    // if the surface parameters address is the same as VkRenderFramework::m_surface
    if (surface == m_surface.Handle()) {
        InitSwapchainInfo();
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = info.surface_formats[0].format;
    swapchain_create_info.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.compositeAlpha = info.surface_composite_alpha;
    swapchain_create_info.presentMode = info.surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = oldSwapchain;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
    return swapchain;
}

void VkRenderFramework::DestroySwapchain() {
    if (m_device && m_device->handle() != VK_NULL_HANDLE) {
        m_device->Wait();
        if (m_swapchain.initialized()) {
            m_swapchain.destroy();
        }
    }
}

void VkRenderFramework::SupportMultiSwapchain() {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    GTEST_SKIP() << "Android currently doesn't support multiple swapchain on all devices";
#endif  // VK_USE_PLATFORM_ANDROID_KHR
}

void VkRenderFramework::SupportSurfaceResize() {
    if (!SurfaceContext::CanResize()) {
        GTEST_SKIP() << "VVL test framework does not support surface resizing on the current platform";
    }
}

void VkRenderFramework::InitRenderTarget() { InitRenderTarget(1); }

void VkRenderFramework::InitRenderTarget(uint32_t targets) { InitRenderTarget(targets, NULL); }

void VkRenderFramework::InitRenderTarget(const VkImageView *dsBinding) { InitRenderTarget(1, dsBinding); }

void VkRenderFramework::InitRenderTarget(uint32_t targets, const VkImageView *dsBinding) {
    std::vector<VkAttachmentReference> color_references;
    std::vector<VkAttachmentDescription> attachment_descriptions;

    attachment_descriptions.reserve(targets + 1);  // +1 for dsBinding
    color_references.reserve(targets);
    m_framebuffer_attachments.reserve(targets + 1);  // +1 for dsBinding

    VkAttachmentDescription att = {};
    att.format = m_render_target_fmt;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    att.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ref = {};
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_renderPassClearValues.clear();
    VkClearValue clear = {};
    clear.color = m_clear_color;

    for (uint32_t i = 0; i < targets; i++) {
        attachment_descriptions.push_back(att);

        ref.attachment = i;
        color_references.push_back(ref);

        m_renderPassClearValues.push_back(clear);

        VkFormatProperties props;
        vk::GetPhysicalDeviceFormatProperties(m_device->Physical().handle(), m_render_target_fmt, &props);

        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        if (props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_LINEAR;
        } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        } else {
            FAIL() << "Neither Linear nor Optimal allowed for render target";
        }

        auto image_ci = vkt::Image::ImageCreateInfo2D(
            m_width, m_height, 1, 1, m_render_target_fmt,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, tiling);
        std::unique_ptr<vkt::Image> img(new vkt::Image(*m_device, image_ci, vkt::set_layout));

        m_render_target_views.push_back(img->CreateView());
        m_framebuffer_attachments.push_back(m_render_target_views.back().handle());
        m_renderTargets.push_back(std::move(img));
    }

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = targets;
    subpass.pColorAttachments = color_references.data();
    subpass.pResolveAttachments = NULL;

    VkAttachmentReference ds_reference;
    if (dsBinding) {
        att.format = m_depth_stencil_fmt;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment_descriptions.push_back(att);

        clear.depthStencil.depth = 1.0;
        clear.depthStencil.stencil = VK_FORMAT_UNDEFINED;
        m_renderPassClearValues.push_back(clear);

        m_framebuffer_attachments.push_back(*dsBinding);

        ds_reference.attachment = targets;
        ds_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpass.pDepthStencilAttachment = &ds_reference;
    } else {
        subpass.pDepthStencilAttachment = NULL;
    }

    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo rp_info = vku::InitStructHelper();
    rp_info.attachmentCount = attachment_descriptions.size();
    rp_info.pAttachments = attachment_descriptions.data();
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 0;
    rp_info.pDependencies = nullptr;

    vk::CreateRenderPass(device(), &rp_info, NULL, &m_renderPass);

    m_framebuffer = new vkt::Framebuffer(*m_device, m_renderPass, m_framebuffer_attachments.size(),
                                         m_framebuffer_attachments.data(), m_width, m_height);

    m_renderPassBeginInfo.renderPass = m_renderPass;
    m_renderPassBeginInfo.framebuffer = m_framebuffer->handle();
    m_renderPassBeginInfo.renderArea.extent.width = m_width;
    m_renderPassBeginInfo.renderArea.extent.height = m_height;
    m_renderPassBeginInfo.clearValueCount = m_renderPassClearValues.size();
    m_renderPassBeginInfo.pClearValues = m_renderPassClearValues.data();
}

void VkRenderFramework::InitDynamicRenderTarget(VkFormat format) {
    if (format != VK_FORMAT_UNDEFINED) {
        m_render_target_fmt = format;
    }

    m_renderPassClearValues.clear();
    VkClearValue clear = {};
    clear.color = m_clear_color;

    if ((m_device->FormatFeaturesOptimal(m_render_target_fmt) & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0) {
        FAIL() << "Optimal tiling not allowed for render target";
    }

    auto image_ci = vkt::Image::ImageCreateInfo2D(
        m_width, m_height, 1, 1, m_render_target_fmt,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    std::unique_ptr<vkt::Image> img(new vkt::Image(*m_device, image_ci, vkt::set_layout));

    m_render_target_views.push_back(img->CreateView());
    m_framebuffer_attachments.push_back(m_render_target_views.back().handle());
    m_renderTargets.push_back(std::move(img));
}

VkImageView VkRenderFramework::GetDynamicRenderTarget() const {
    assert(m_framebuffer_attachments.size() == 1);
    return m_framebuffer_attachments[0];
}

VkRect2D VkRenderFramework::GetRenderTargetArea() const { return {{0, 0}, {m_width, m_height}}; }

void VkRenderFramework::DestroyRenderTarget() {
    vk::DestroyRenderPass(device(), m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;
    delete m_framebuffer;
    m_framebuffer = nullptr;
}

void VkRenderFramework::SetDefaultDynamicStatesExclude(const std::vector<VkDynamicState> &exclude, bool tessellation,
                                                       VkCommandBuffer commandBuffer) {
    const auto excluded = [&exclude](VkDynamicState state) {
        return std::find(exclude.begin(), exclude.end(), state) != exclude.end();
    };
    if (!m_vertex_buffer) {
        m_vertex_buffer = new vkt::Buffer(*m_device, 32u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    VkCommandBuffer cmdBuffer = commandBuffer ? commandBuffer : m_command_buffer.handle();
    VkViewport viewport = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {m_width, m_height}};
    if (!excluded(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)) vk::CmdSetViewportWithCountEXT(cmdBuffer, 1u, &viewport);
    if (!excluded(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT)) vk::CmdSetScissorWithCountEXT(cmdBuffer, 1u, &scissor);
    if (!excluded(VK_DYNAMIC_STATE_LINE_WIDTH)) vk::CmdSetLineWidth(cmdBuffer, 1.0f);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BIAS)) vk::CmdSetDepthBias(cmdBuffer, 1.0f, 0.0f, 1.0f);
    float blendConstants[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    if (!excluded(VK_DYNAMIC_STATE_BLEND_CONSTANTS)) vk::CmdSetBlendConstants(cmdBuffer, blendConstants);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BOUNDS)) vk::CmdSetDepthBounds(cmdBuffer, 0.0f, 1.0f);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK))
        vk::CmdSetStencilCompareMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK))
        vk::CmdSetStencilWriteMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_REFERENCE))
        vk::CmdSetStencilReference(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    VkDeviceSize offset = 0u;
    VkDeviceSize size = sizeof(float);
    if (!excluded(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE))
        vk::CmdBindVertexBuffers2EXT(cmdBuffer, 0, 1, &m_vertex_buffer->handle(), &offset, &size, &size);
    if (!excluded(VK_DYNAMIC_STATE_CULL_MODE)) vk::CmdSetCullModeEXT(cmdBuffer, VK_CULL_MODE_NONE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE)) vk::CmdSetDepthBoundsTestEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP)) vk::CmdSetDepthCompareOpEXT(cmdBuffer, VK_COMPARE_OP_NEVER);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE)) vk::CmdSetDepthTestEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE)) vk::CmdSetDepthWriteEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_FRONT_FACE)) vk::CmdSetFrontFaceEXT(cmdBuffer, VK_FRONT_FACE_CLOCKWISE);
    if (!excluded(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY))
        vk::CmdSetPrimitiveTopologyEXT(cmdBuffer,
                                       tessellation ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_OP))
        vk::CmdSetStencilOpEXT(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
                               VK_STENCIL_OP_KEEP, VK_COMPARE_OP_NEVER);
    if (!excluded(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE)) vk::CmdSetStencilTestEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) vk::CmdSetDepthBiasEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)) vk::CmdSetPrimitiveRestartEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) vk::CmdSetRasterizerDiscardEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) vk::CmdSetVertexInputEXT(cmdBuffer, 0u, nullptr, 0u, nullptr);
    if (!excluded(VK_DYNAMIC_STATE_LOGIC_OP_EXT)) vk::CmdSetLogicOpEXT(cmdBuffer, VK_LOGIC_OP_COPY);
    if (!excluded(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)) vk::CmdSetPatchControlPointsEXT(cmdBuffer, 4u);
    if (!excluded(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT))
        vk::CmdSetTessellationDomainOriginEXT(cmdBuffer, VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
    if (!excluded(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) vk::CmdSetDepthClampEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_POLYGON_MODE_EXT)) vk::CmdSetPolygonModeEXT(cmdBuffer, VK_POLYGON_MODE_FILL);
    if (!excluded(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) vk::CmdSetRasterizationSamplesEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 0xFFFFFFFF;
    if (!excluded(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT)) vk::CmdSetSampleMaskEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    if (!excluded(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT)) vk::CmdSetAlphaToCoverageEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT)) vk::CmdSetAlphaToOneEnableEXT(cmdBuffer, VK_FALSE);
    if (!excluded(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) vk::CmdSetLogicOpEnableEXT(cmdBuffer, VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    if (!excluded(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT)) vk::CmdSetColorBlendEnableEXT(cmdBuffer, 0u, 1u, &colorBlendEnable);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
    };
    if (!excluded(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT))
        vk::CmdSetColorBlendEquationEXT(cmdBuffer, 0u, 1u, &colorBlendEquation);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (!excluded(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) vk::CmdSetColorWriteMaskEXT(cmdBuffer, 0u, 1u, &colorWriteMask);
}

void VkRenderFramework::SetDefaultDynamicStatesAll(VkCommandBuffer cmdBuffer) {
    uint32_t width = 32;
    uint32_t height = 32;
    VkViewport viewport = {0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {width, height}};
    vk::CmdSetViewportWithCountEXT(cmdBuffer, 1u, &viewport);
    vk::CmdSetScissorWithCountEXT(cmdBuffer, 1u, &scissor);
    vk::CmdSetLineWidth(cmdBuffer, 1.0f);
    vk::CmdSetDepthBias(cmdBuffer, 1.0f, 0.0f, 1.0f);
    float blendConstants[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    vk::CmdSetBlendConstants(cmdBuffer, blendConstants);
    vk::CmdSetDepthBounds(cmdBuffer, 0.0f, 1.0f);
    vk::CmdSetStencilCompareMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetStencilWriteMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetStencilReference(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetCullModeEXT(cmdBuffer, VK_CULL_MODE_NONE);
    vk::CmdSetDepthBoundsTestEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetDepthCompareOpEXT(cmdBuffer, VK_COMPARE_OP_NEVER);
    vk::CmdSetDepthTestEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetDepthWriteEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetFrontFaceEXT(cmdBuffer, VK_FRONT_FACE_CLOCKWISE);
    vk::CmdSetPrimitiveTopologyEXT(cmdBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    vk::CmdSetStencilOpEXT(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
                           VK_COMPARE_OP_NEVER);
    vk::CmdSetStencilTestEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetDepthBiasEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetPrimitiveRestartEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetRasterizerDiscardEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetVertexInputEXT(cmdBuffer, 0u, nullptr, 0u, nullptr);
    vk::CmdSetLogicOpEXT(cmdBuffer, VK_LOGIC_OP_COPY);
    vk::CmdSetPatchControlPointsEXT(cmdBuffer, 4u);
    vk::CmdSetTessellationDomainOriginEXT(cmdBuffer, VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
    vk::CmdSetDepthClampEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetPolygonModeEXT(cmdBuffer, VK_POLYGON_MODE_FILL);
    vk::CmdSetRasterizationSamplesEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 0xFFFFFFFF;
    vk::CmdSetSampleMaskEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vk::CmdSetAlphaToCoverageEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetAlphaToOneEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetLogicOpEnableEXT(cmdBuffer, VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(cmdBuffer, 0u, 1u, &colorBlendEnable);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(cmdBuffer, 0u, 1u, &colorBlendEquation);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(cmdBuffer, 0u, 1u, &colorWriteMask);
}

std::vector<uint32_t> VkRenderFramework::GLSLToSPV(VkShaderStageFlagBits stage, const char *code, const spv_target_env env) {
    std::vector<uint32_t> spv;
    GLSLtoSPV(m_device->Physical().limits_, stage, code, spv, env);
    return spv;
}
