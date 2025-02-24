/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "state_tracker/state_tracker.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/device_state.h"
#include "state_tracker/descriptor_sets.h"

class BestPractices;

namespace bp_state {
class Image : public vvl::Image {
  public:
    Image(const vvl::Device& dev_data, VkImage handle, const VkImageCreateInfo* create_info, VkFormatFeatureFlags2 features)
        : vvl::Image(dev_data, handle, create_info, features) {
        SetupUsages();
    }

    Image(const vvl::Device& dev_data, VkImage handle, const VkImageCreateInfo* create_info, VkSwapchainKHR swapchain,
          uint32_t swapchain_index, VkFormatFeatureFlags2 features)
        : vvl::Image(dev_data, handle, create_info, swapchain, swapchain_index, features) {
        SetupUsages();
    }

    struct Usage {
        IMAGE_SUBRESOURCE_USAGE_BP type;
        uint32_t queue_family_index;
    };

    Usage UpdateUsage(uint32_t array_layer, uint32_t mip_level, IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t queue_family) {
        auto last_usage = usages_[array_layer][mip_level];
        usages_[array_layer][mip_level].type = usage;
        usages_[array_layer][mip_level].queue_family_index = queue_family;
        return last_usage;
    }

    Usage GetUsage(uint32_t array_layer, uint32_t mip_level) const { return usages_[array_layer][mip_level]; }

    IMAGE_SUBRESOURCE_USAGE_BP GetUsageType(uint32_t array_layer, uint32_t mip_level) const {
        return GetUsage(array_layer, mip_level).type;
    }

    uint32_t GetLastQueueFamily(uint32_t array_layer, uint32_t mip_level) const {
        return GetUsage(array_layer, mip_level).queue_family_index;
    }

  private:
    void SetupUsages() {
        usages_.resize(create_info.arrayLayers);
        for (auto& mip_vec : usages_) {
            mip_vec.resize(create_info.mipLevels, {IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED, VK_QUEUE_FAMILY_IGNORED});
        }
    }
    // A 2d vector for all the array layers and mip levels.
    // This does not split usages per aspect.
    // Aspects are generally read and written together,
    // and tracking them independently could be misleading.
    // second/uint32_t is last queue family usage
    std::vector<std::vector<Usage>> usages_;
};

class PhysicalDevice : public vvl::PhysicalDevice {
  public:
    PhysicalDevice(VkPhysicalDevice phys_dev) : vvl::PhysicalDevice(phys_dev) {}

    // Track the call state and array sizes for various query functions
    CALL_STATE vkGetPhysicalDeviceQueueFamilyPropertiesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceQueueFamilyProperties2State = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceQueueFamilyProperties2KHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceLayerPropertiesState = UNCALLED;      // Currently unused
    CALL_STATE vkGetPhysicalDeviceExtensionPropertiesState = UNCALLED;  // Currently unused
    CALL_STATE vkGetPhysicalDeviceFeaturesState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceCapabilitiesKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfacePresentModesKHRState = UNCALLED;
    CALL_STATE vkGetPhysicalDeviceSurfaceFormatsKHRState = UNCALLED;
    uint32_t surface_formats_count = 0;
    CALL_STATE vkGetPhysicalDeviceDisplayPlanePropertiesKHRState = UNCALLED;
};

class Swapchain : public vvl::Swapchain {
  public:
    Swapchain(vvl::Device& dev_data, const VkSwapchainCreateInfoKHR* create_info, VkSwapchainKHR handle)
        : vvl::Swapchain(dev_data, create_info, handle) {}

    CALL_STATE vkGetSwapchainImagesKHRState = UNCALLED;
};

class DeviceMemory : public vvl::DeviceMemory {
  public:
    DeviceMemory(VkDeviceMemory handle, const VkMemoryAllocateInfo* allocate_info, uint64_t fake_address,
                 const VkMemoryType& memory_type, const VkMemoryHeap& memory_heap,
                 std::optional<vvl::DedicatedBinding>&& dedicated_binding, uint32_t physical_device_count)
        : vvl::DeviceMemory(handle, allocate_info, fake_address, memory_type, memory_heap, std::move(dedicated_binding),
                            physical_device_count) {}

    std::optional<float> dynamic_priority;  // VK_EXT_pageable_device_local_memory priority
};

struct AttachmentInfo {
    uint32_t framebufferAttachment;
    VkImageAspectFlags aspects;

    AttachmentInfo(uint32_t framebufferAttachment_, VkImageAspectFlags aspects_)
        : framebufferAttachment(framebufferAttachment_), aspects(aspects_) {}
};

// used to track state regarding render pass heuristic checks
struct RenderPassState {
    bool depthAttachment = false;
    bool colorAttachment = false;
    bool depthOnly = false;
    bool depthEqualComparison = false;
    uint32_t numDrawCallsDepthOnly = 0;
    uint32_t numDrawCallsDepthEqualCompare = 0;

    // For secondaries, we need to keep this around for execute commands.
    struct ClearInfo {
        uint32_t framebufferAttachment;
        uint32_t colorAttachment;
        VkImageAspectFlags aspects;
        std::vector<VkClearRect> rects;
    };

    std::vector<ClearInfo> earlyClearAttachments;
    std::vector<AttachmentInfo> touchesAttachments;
    std::vector<AttachmentInfo> nextDrawTouchesAttachments;
    bool drawTouchAttachments = false;
};

struct CommandBufferStateNV {
    struct TessGeometryMesh {
        enum class State {
            Unknown,
            Disabled,
            Enabled,
        };

        uint32_t num_switches = 0;
        State state = State::Unknown;
        bool threshold_signaled = false;
    };
    struct ZcullResourceState {
        ZcullDirection direction = ZcullDirection::Unknown;
        uint64_t num_less_draws = 0;
        uint64_t num_greater_draws = 0;
    };
    struct ZcullTree {
        std::vector<ZcullResourceState> states;
        uint32_t mip_levels = 0;
        uint32_t array_layers = 0;

        const ZcullResourceState& GetState(uint32_t layer, uint32_t level) const { return states[layer * mip_levels + level]; }

        ZcullResourceState& GetState(uint32_t layer, uint32_t level) { return states[layer * mip_levels + level]; }
    };
    struct ZcullScope {
        VkImage image = VK_NULL_HANDLE;
        VkImageSubresourceRange range{};
        ZcullTree* tree = nullptr;
    };

    TessGeometryMesh tess_geometry_mesh;

    vvl::unordered_map<VkImage, ZcullTree> zcull_per_image;
    ZcullScope zcull_scope;
    ZcullDirection zcull_direction = ZcullDirection::Unknown;

    VkCompareOp depth_compare_op = VK_COMPARE_OP_NEVER;
    bool depth_test_enable = false;
};

class CommandBuffer : public vvl::CommandBuffer {
  public:
    CommandBuffer(BestPractices& bp, VkCommandBuffer handle, const VkCommandBufferAllocateInfo* allocate_info,
                  const vvl::CommandPool* pool);

    RenderPassState render_pass_state;
    CommandBufferStateNV nv;
    uint64_t num_submits = 0;
    bool uses_vertex_buffer = false;
    uint32_t small_indexed_draw_call_count = 0;

    // This function used to not be empty. It has been left empty because
    // the logic to decide to call this function is not simple, so adding this
    // function back could tedious.
    void UnbindResources() {}

    struct SignalingInfo {
        // True, if the event's first state change within a command buffer is a signal (SetEvent)
        // rather than an unsignal (ResetEvent). It is used to do validation on the boundary
        // between two command buffers.
        const bool first_state_change_is_signal = false;

        // Tracks how the event signaling state changes as the command buffer recording progresses.
        // When recording is finished, this is the event state "at the end of the command buffer".
        bool signaled = false;

        SignalingInfo(bool signal) : first_state_change_is_signal(signal), signaled(signal) {}
    };
    vvl::unordered_map<VkEvent, SignalingInfo> event_signaling_state;
};

class DescriptorPool : public vvl::DescriptorPool {
  public:
    DescriptorPool(vvl::Device& dev, const VkDescriptorPool handle, const VkDescriptorPoolCreateInfo* create_info)
        : vvl::DescriptorPool(dev, handle, create_info) {}

    uint32_t freed_count{0};
};

class Pipeline : public vvl::Pipeline {
  public:
    Pipeline(const vvl::Device& state_data, const VkGraphicsPipelineCreateInfo* create_info,
             std::shared_ptr<const vvl::PipelineCache>&& pipe_cache, std::shared_ptr<const vvl::RenderPass>&& rpstate,
             std::shared_ptr<const vvl::PipelineLayout>&& layout);

    const std::vector<AttachmentInfo> access_framebuffer_attachments;
};
}  // namespace bp_state
