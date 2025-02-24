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

#include "binding.h"

#include <string.h>  // memset(), memcmp()
#include <cassert>

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/utility/vk_struct_helper.hpp>

#define NON_DISPATCHABLE_HANDLE_INIT(create_func, dev, ...)                                                \
    do {                                                                                                   \
        assert(!initialized());                                                                            \
        handle_type handle;                                                                                \
        auto result = create_func(dev.handle(), __VA_ARGS__, nullptr, &handle);                            \
        ASSERT_TRUE((result == VK_SUCCESS) || (result == VK_ERROR_VALIDATION_FAILED_EXT) ||                \
                    (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) || (result == VK_ERROR_OUT_OF_HOST_MEMORY)); \
        if (result == VK_SUCCESS) {                                                                        \
            NonDispHandle::init(dev.handle(), handle);                                                     \
        }                                                                                                  \
    } while (0)

#define NON_DISPATCHABLE_HANDLE_DTOR(cls, destroy_func)        \
    void cls::destroy() noexcept {                             \
        if (!initialized()) {                                  \
            return;                                            \
        }                                                      \
        destroy_func(device(), handle(), NULL);                \
        handle_ = VK_NULL_HANDLE;                              \
        internal::NonDispHandle<decltype(handle_)>::destroy(); \
    }                                                          \
    cls::~cls() noexcept { destroy(); }

namespace vkt {

void Instance::Init(const VkInstanceCreateInfo &info) { ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&info, NULL, &handle_)); }

void Instance::Destroy() noexcept {
    if (handle_ != VK_NULL_HANDLE) {
        vk::DestroyInstance(handle_, nullptr);
        handle_ = VK_NULL_HANDLE;
    }
}

VkPhysicalDeviceProperties PhysicalDevice::Properties() const {
    VkPhysicalDeviceProperties info;
    vk::GetPhysicalDeviceProperties(handle(), &info);
    return info;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::QueueProperties() const {
    uint32_t count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties(handle(), &count, nullptr);
    std::vector<VkQueueFamilyProperties> info(count);
    vk::GetPhysicalDeviceQueueFamilyProperties(handle(), &count, info.data());
    return info;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::MemoryProperties() const {
    VkPhysicalDeviceMemoryProperties info;
    vk::GetPhysicalDeviceMemoryProperties(handle(), &info);
    return info;
}

VkPhysicalDeviceFeatures PhysicalDevice::Features() const {
    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(handle(), &features);
    return features;
}

/*
 * Return list of Global layers available
 */
std::vector<VkLayerProperties> GetGlobalLayers() {
    VkResult err;
    uint32_t layer_count = 32;
    std::vector<VkLayerProperties> layers(layer_count);
    for (;;) {
        err = vk::EnumerateInstanceLayerProperties(&layer_count, layers.data());
        if (err == VK_SUCCESS) {
            layers.resize(layer_count);
            return layers;
        } else if (err == VK_INCOMPLETE) {
            layer_count *= 2;  // wasn't enough space, increase it
            layers.resize(layer_count);
        } else {
            return {};
        }
    }
}

/*
 * Return list of Global extensions provided by the ICD / Loader
 */
std::vector<VkExtensionProperties> GetGlobalExtensions() { return GetGlobalExtensions(nullptr); }

/*
 * Return list of Global extensions provided by the specified layer
 * If pLayerName is NULL, will return extensions implemented by the loader /
 * ICDs
 */
std::vector<VkExtensionProperties> GetGlobalExtensions(const char *pLayerName) {
    VkResult err;
    uint32_t extension_count = 32;
    std::vector<VkExtensionProperties> extensions(extension_count);
    for (;;) {
        err = vk::EnumerateInstanceExtensionProperties(pLayerName, &extension_count, extensions.data());
        if (err == VK_SUCCESS) {
            extensions.resize(extension_count);
            return extensions;
        } else if (err == VK_INCOMPLETE) {
            extension_count *= 2;  // wasn't enough space, increase it
            extensions.resize(extension_count);
        } else {
            return {};
        }
    }
}

/*
 * Return list of PhysicalDevice extensions provided by the specified layer
 * If pLayerName is NULL, will return extensions for ICD / loader.
 */
std::vector<VkExtensionProperties> PhysicalDevice::Extensions(const char *pLayerName) const {
    VkResult err;
    uint32_t extension_count = 512;
    std::vector<VkExtensionProperties> extensions(extension_count);
    for (;;) {
        err = vk::EnumerateDeviceExtensionProperties(handle(), pLayerName, &extension_count, extensions.data());
        if (err == VK_SUCCESS) {
            extensions.resize(extension_count);
            return extensions;
        } else if (err == VK_INCOMPLETE) {
            extension_count *= 2;  // wasn't enough space, increase it
            extensions.resize(extension_count);
        } else {
            return {};
        }
    }
}

bool PhysicalDevice::SetMemoryType(const uint32_t type_bits, VkMemoryAllocateInfo *info, const VkFlags properties,
                                   const VkFlags forbid) const {
    uint32_t type_mask = type_bits;
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < memory_properties_.memoryTypeCount; i++) {
        if ((type_mask & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memory_properties_.memoryTypes[i].propertyFlags & properties) == properties &&
                (memory_properties_.memoryTypes[i].propertyFlags & forbid) == 0 &&
                (memory_properties_.memoryHeaps[memory_properties_.memoryTypes[i].heapIndex].size >= info->allocationSize)) {
                info->memoryTypeIndex = i;
                return true;
            }
        }
        type_mask >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

/*
 * Return list of PhysicalDevice layers
 */
std::vector<VkLayerProperties> PhysicalDevice::Layers() const {
    VkResult err;
    uint32_t layer_count = 32;
    std::vector<VkLayerProperties> layers(layer_count);
    for (;;) {
        err = vk::EnumerateDeviceLayerProperties(handle(), &layer_count, layers.data());
        if (err == VK_SUCCESS) {
            layers.resize(layer_count);
            return layers;
        } else if (err == VK_INCOMPLETE) {
            layer_count *= 2;  // wasn't enough space, increase it
            layers.resize(layer_count);
        } else {
            return {};
        }
    }
}

QueueCreateInfoArray::QueueCreateInfoArray(const std::vector<VkQueueFamilyProperties> &queue_props, bool all_queue_count)
    : queue_info_(), queue_priorities_() {
    queue_info_.reserve(queue_props.size());

    for (uint32_t i = 0; i < (uint32_t)queue_props.size(); ++i) {
        if (queue_props[i].queueCount > 0) {
            VkDeviceQueueCreateInfo qi = vku::InitStructHelper();
            qi.queueFamilyIndex = i;
            // It is very slow on some drivers (ex Windows NVIDIA) to create all 16/32 queues supported
            qi.queueCount = all_queue_count ? queue_props[i].queueCount : 1;
            queue_priorities_.emplace_back(qi.queueCount, 0.0f);
            qi.pQueuePriorities = queue_priorities_[i].data();
            queue_info_.push_back(qi);
        }
    }
}

void Device::destroy() noexcept {
    if (!initialized()) return;
    vk::DestroyDevice(handle(), NULL);
    handle_ = VK_NULL_HANDLE;
}

Device::~Device() noexcept { destroy(); }

void Device::init(std::vector<const char *> &extensions, VkPhysicalDeviceFeatures *features, void *create_device_pnext,
                  bool all_queue_count) {
    // request all queues
    QueueCreateInfoArray queue_info(physical_device_.queue_properties_, all_queue_count);
    for (uint32_t i = 0; i < (uint32_t)physical_device_.queue_properties_.size(); i++) {
        if (physical_device_.queue_properties_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_node_index_ = i;
            break;
        }
    }
    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.Data();
    for (uint32_t j = 0; j < queue_info.Size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }

    enabled_extensions_ = extensions;

    VkDeviceCreateInfo dev_info = vku::InitStructHelper(create_device_pnext);
    dev_info.queueCreateInfoCount = static_cast<uint32_t>(create_queue_infos.size());
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    dev_info.ppEnabledExtensionNames = extensions.data();

    VkPhysicalDeviceFeatures all_features;
    // Let VkPhysicalDeviceFeatures2 take priority over VkPhysicalDeviceFeatures,
    // since it supports extensions

    if (!(vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(dev_info.pNext))) {
        if (features) {
            dev_info.pEnabledFeatures = features;
        } else {
            // request all supportable features enabled
            all_features = Physical().Features();
            dev_info.pEnabledFeatures = &all_features;
        }
    }

    init(dev_info);
}

void Device::init(const VkDeviceCreateInfo &info) {
    VkDevice dev;

    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(physical_device_.handle(), &info, NULL, &dev));
    Handle::init(dev);

    InitQueues(info);
}

void Device::InitQueues(const VkDeviceCreateInfo &info) {
    uint32_t queue_node_count = physical_device_.queue_properties_.size();

    queue_families_.resize(queue_node_count);
    for (uint32_t i = 0; i < info.queueCreateInfoCount; i++) {
        const auto &queue_create_info = info.pQueueCreateInfos[i];
        auto queue_family_i = queue_create_info.queueFamilyIndex;
        const auto &queue_family_prop = physical_device_.queue_properties_[queue_family_i];

        QueueFamilyQueues &queue_storage = queue_families_[queue_family_i];
        queue_storage.reserve(queue_create_info.queueCount);
        for (uint32_t queue_i = 0; queue_i < queue_create_info.queueCount; ++queue_i) {
            VkQueue queue = VK_NULL_HANDLE;
            vk::GetDeviceQueue(handle(), queue_family_i, queue_i, &queue);

            // Store single copy of the queue object that will self destruct
            queue_storage.emplace_back(new Queue(queue, queue_family_i));

            if (queue_family_prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queues_[GRAPHICS].push_back(queue_storage.back().get());
            }

            if (queue_family_prop.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                queues_[COMPUTE].push_back(queue_storage.back().get());
            }

            if (queue_family_prop.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                queues_[TRANSFER].push_back(queue_storage.back().get());
            }

            if (queue_family_prop.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
                queues_[SPARSE].push_back(queue_storage.back().get());
            }

            if (queue_family_prop.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
                queues_[VIDEO_DECODE].push_back(queue_storage.back().get());
            }

            if (queue_family_prop.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
                queues_[VIDEO_ENCODE].push_back(queue_storage.back().get());
            }
        }
    }

    ASSERT_TRUE(!queues_[GRAPHICS].empty() || !queues_[COMPUTE].empty() || !queues_[TRANSFER].empty() || !queues_[SPARSE].empty() ||
                !queues_[VIDEO_DECODE].empty() || !queues_[VIDEO_ENCODE].empty());
}

const Device::QueueFamilyQueues &Device::QueuesFromFamily(uint32_t queue_family) const {
    assert(queue_family < queue_families_.size());
    return queue_families_[queue_family];
}

std::optional<uint32_t> Device::QueueFamily(VkQueueFlags with, VkQueueFlags without) const {
    for (uint32_t i = 0; i < physical_device_.queue_properties_.size(); i++) {
        if (physical_device_.queue_properties_[i].queueCount > 0) {
            const auto flags = physical_device_.queue_properties_[i].queueFlags;
            const bool matches = (flags & with) == with;
            if (matches && ((flags & without) == 0)) {
                return i;
            }
        }
    }
    return {};
}

std::optional<uint32_t> Device::QueueFamilyWithoutCapabilities(VkQueueFlags without) const {
    for (uint32_t i = 0; i < physical_device_.queue_properties_.size(); i++) {
        if (physical_device_.queue_properties_[i].queueCount > 0) {
            const auto flags = physical_device_.queue_properties_[i].queueFlags;
            if ((flags & without) == 0) {
                return i;
            }
        }
    }
    return {};
}

Queue *Device::QueueWithoutCapabilities(VkQueueFlags without) const {
    auto family_index = QueueFamilyWithoutCapabilities(without);
    return family_index.has_value() ? QueuesFromFamily(*family_index)[0].get() : nullptr;
}

std::optional<uint32_t> Device::ComputeOnlyQueueFamily() const { return QueueFamily(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT); }

Queue *Device::ComputeOnlyQueue() const {
    auto family_index = ComputeOnlyQueueFamily();
    return family_index.has_value() ? QueuesFromFamily(*family_index)[0].get() : nullptr;
}

std::optional<uint32_t> Device::TransferOnlyQueueFamily() const {
    return QueueFamily(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
}

Queue *Device::TransferOnlyQueue() const {
    auto family_index = TransferOnlyQueueFamily();
    return family_index.has_value() ? QueuesFromFamily(*family_index)[0].get() : nullptr;
}

std::optional<uint32_t> Device::NonGraphicsQueueFamily() const {
    if (auto compute_qfi = ComputeOnlyQueueFamily(); compute_qfi.has_value()) {
        return compute_qfi;
    }
    return TransferOnlyQueueFamily();
}

Queue *Device::NonGraphicsQueue() const {
    auto family_index = NonGraphicsQueueFamily();
    return family_index.has_value() ? QueuesFromFamily(*family_index)[0].get() : nullptr;
}

bool Device::IsEnabledExtension(const char *extension) const {
    const auto is_x = [&extension](const char *enabled_extension) { return strcmp(extension, enabled_extension) == 0; };
    return std::any_of(enabled_extensions_.begin(), enabled_extensions_.end(), is_x);
}

VkFormatFeatureFlags2 Device::FormatFeaturesLinear(VkFormat format) const {
    if (IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
        vk::GetPhysicalDeviceFormatProperties2(Physical().handle(), format, &fmt_props_2);
        return fmt_props_3.linearTilingFeatures;
    } else {
        VkFormatProperties format_properties;
        vk::GetPhysicalDeviceFormatProperties(Physical().handle(), format, &format_properties);
        return format_properties.linearTilingFeatures;
    }
}

VkFormatFeatureFlags2 Device::FormatFeaturesOptimal(VkFormat format) const {
    if (IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
        vk::GetPhysicalDeviceFormatProperties2(Physical().handle(), format, &fmt_props_2);
        return fmt_props_3.optimalTilingFeatures;
    } else {
        VkFormatProperties format_properties;
        vk::GetPhysicalDeviceFormatProperties(Physical().handle(), format, &format_properties);
        return format_properties.optimalTilingFeatures;
    }
}

VkFormatFeatureFlags2 Device::FormatFeaturesBuffer(VkFormat format) const {
    if (IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
        vk::GetPhysicalDeviceFormatProperties2(Physical().handle(), format, &fmt_props_2);
        return fmt_props_3.bufferFeatures;
    } else {
        VkFormatProperties format_properties;
        vk::GetPhysicalDeviceFormatProperties(Physical().handle(), format, &format_properties);
        return format_properties.bufferFeatures;
    }
}

void Device::Wait() const { ASSERT_EQ(VK_SUCCESS, vk::DeviceWaitIdle(handle())); }

VkResult Device::Wait(const std::vector<const Fence *> &fences, bool wait_all, uint64_t timeout) {
    const std::vector<VkFence> fence_handles = MakeVkHandles<VkFence>(fences);
    VkResult err =
        vk::WaitForFences(handle(), static_cast<uint32_t>(fence_handles.size()), fence_handles.data(), wait_all, timeout);
    EXPECT_TRUE(err == VK_SUCCESS || err == VK_TIMEOUT);

    return err;
}

VkResult Queue::Submit(const CommandBuffer &cmd, const Fence &fence) {
    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBuffers = &cmd.handle();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Submit(const vvl::span<CommandBuffer *> &cmds, const Fence &fence) {
    const std::vector<VkCommandBuffer> cmd_handles = MakeVkHandles<VkCommandBuffer>(cmds);
    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = static_cast<uint32_t>(cmd_handles.size());
    submit_info.pCommandBuffers = cmd_handles.data();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit_info, fence.handle());
    return result;
}

VkResult Queue::Submit(const CommandBuffer &cmd, const vkt::Wait &wait, const Fence &fence) {
    assert(wait.stage_mask < VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);
    const auto wait_stage_mask = static_cast<VkPipelineStageFlags>(wait.stage_mask);

    VkSubmitInfo submit = vku::InitStructHelper();
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &wait.semaphore.handle();
    submit.pWaitDstStageMask = &wait_stage_mask;
    submit.commandBufferCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBuffers = &cmd.handle();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit, fence);
    return result;
}

VkResult Queue::Submit(const CommandBuffer &cmd, const Signal &signal, const Fence &fence) {
    assert(signal.stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkSubmitInfo submit = vku::InitStructHelper();
    submit.commandBufferCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBuffers = &cmd.handle();
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &signal.semaphore.handle();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Submit(const CommandBuffer &cmd, const vkt::Wait &wait, const Signal &signal, const Fence &fence) {
    assert(wait.stage_mask < VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);
    assert(signal.stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    const auto wait_stage_mask = static_cast<VkPipelineStageFlags>(wait.stage_mask);

    VkSubmitInfo submit = vku::InitStructHelper();
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &wait.semaphore.handle();
    submit.pWaitDstStageMask = &wait_stage_mask;
    submit.commandBufferCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBuffers = &cmd.handle();
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &signal.semaphore.handle();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Submit(const CommandBuffer &cmd, const TimelineWait &wait, const Fence &fence) {
    assert(wait.stage_mask < VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);
    const auto wait_stage_mask = static_cast<VkPipelineStageFlags>(wait.stage_mask);

    VkTimelineSemaphoreSubmitInfo timeline_info = vku::InitStructHelper();
    timeline_info.waitSemaphoreValueCount = 1;
    timeline_info.pWaitSemaphoreValues = &wait.value;

    VkSubmitInfo submit = vku::InitStructHelper(&timeline_info);
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &wait.semaphore.handle();
    submit.pWaitDstStageMask = &wait_stage_mask;
    submit.commandBufferCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBuffers = &cmd.handle();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Submit(const CommandBuffer &cmd, const TimelineSignal &signal, const Fence &fence) {
    assert(signal.stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkTimelineSemaphoreSubmitInfo timeline_info = vku::InitStructHelper();
    timeline_info.signalSemaphoreValueCount = 1;
    timeline_info.pSignalSemaphoreValues = &signal.value;

    VkSubmitInfo submit = vku::InitStructHelper(&timeline_info);
    submit.commandBufferCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBuffers = &cmd.handle();
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &signal.semaphore.handle();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Submit(const CommandBuffer &cmd, const TimelineWait &wait, const TimelineSignal &signal, const Fence &fence) {
    assert(wait.stage_mask < VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);
    assert(signal.stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    const auto wait_stage_mask = static_cast<VkPipelineStageFlags>(wait.stage_mask);

    VkTimelineSemaphoreSubmitInfo timeline_info = vku::InitStructHelper();
    timeline_info.waitSemaphoreValueCount = 1;
    timeline_info.pWaitSemaphoreValues = &wait.value;
    timeline_info.signalSemaphoreValueCount = 1;
    timeline_info.pSignalSemaphoreValues = &signal.value;

    VkSubmitInfo submit = vku::InitStructHelper(&timeline_info);
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &wait.semaphore.handle();
    submit.pWaitDstStageMask = &wait_stage_mask;
    submit.commandBufferCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBuffers = &cmd.handle();
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &signal.semaphore.handle();
    VkResult result = vk::QueueSubmit(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const Fence &fence, bool use_khr) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const vvl::span<const CommandBuffer> &cmds, const Fence &fence, bool use_khr) {
    std::vector<VkCommandBufferSubmitInfo> cmd_submit_infos;
    cmd_submit_infos.reserve(cmds.size());
    for (size_t i = 0; i < cmds.size(); i++) {
        VkCommandBufferSubmitInfo cmd_submit_info = vku::InitStructHelper();
        cmd_submit_info.commandBuffer = cmds[i].handle();
        cmd_submit_infos.push_back(cmd_submit_info);
    }
    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.commandBufferInfoCount = static_cast<uint32_t>(cmd_submit_infos.size());
    submit.pCommandBufferInfos = cmd_submit_infos.data();

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const vkt::Wait &wait, const Fence &fence, bool use_khr) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = wait.semaphore.handle();
    wait_info.stageMask = wait.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const Signal &signal, const Fence &fence, bool use_khr) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = signal.semaphore.handle();
    signal_info.stageMask = signal.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const vkt::Wait &wait, const Signal &signal, const Fence &fence, bool use_khr) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = wait.semaphore.handle();
    wait_info.stageMask = wait.stage_mask;

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = signal.semaphore.handle();
    signal_info.stageMask = signal.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const TimelineWait &wait, const Fence &fence, bool use_khr) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = wait.semaphore.handle();
    wait_info.value = wait.value;
    wait_info.stageMask = wait.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const TimelineSignal &signal, const Fence &fence, bool use_khr) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = signal.semaphore.handle();
    signal_info.value = signal.value;
    signal_info.stageMask = signal.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const TimelineWait &wait, const TimelineSignal &signal, const Fence &fence,
                        bool use_khr) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = wait.semaphore.handle();
    wait_info.value = wait.value;
    wait_info.stageMask = wait.stage_mask;

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = signal.semaphore.handle();
    signal_info.value = signal.value;
    signal_info.stageMask = signal.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;

    VkResult result;
    if (use_khr) {
        result = vk::QueueSubmit2KHR(handle(), 1, &submit, fence.handle());
    } else {
        result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    }
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const vkt::Wait &wait, const TimelineSignal &signal, const Fence &fence) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = wait.semaphore.handle();
    wait_info.stageMask = wait.stage_mask;

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = signal.semaphore.handle();
    signal_info.value = signal.value;
    signal_info.stageMask = signal.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;

    VkResult result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Submit2(const CommandBuffer &cmd, const TimelineWait &wait, const Signal &signal, const Fence &fence) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = cmd.handle();

    VkSemaphoreSubmitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphore = wait.semaphore.handle();
    wait_info.value = wait.value;
    wait_info.stageMask = wait.stage_mask;

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = signal.semaphore.handle();
    signal_info.stageMask = signal.stage_mask;

    VkSubmitInfo2 submit = vku::InitStructHelper();
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &wait_info;
    submit.commandBufferInfoCount = cmd.initialized() ? 1 : 0;
    submit.pCommandBufferInfos = &cb_info;
    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signal_info;

    VkResult result = vk::QueueSubmit2(handle(), 1, &submit, fence.handle());
    return result;
}

VkResult Queue::Present(const Swapchain &swapchain, uint32_t image_index, const Semaphore &wait_semaphore,
                        void *present_info_pnext) {
    VkPresentInfoKHR present = vku::InitStructHelper(present_info_pnext);
    if (wait_semaphore.initialized()) {
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &wait_semaphore.handle();
    }
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain.handle();
    present.pImageIndices = &image_index;
    VkResult result = vk::QueuePresentKHR(handle(), &present);
    return result;
}

VkResult Queue::Wait() {
    VkResult result = vk::QueueWaitIdle(handle());
    EXPECT_EQ(VK_SUCCESS, result);
    return result;
}

void DeviceMemory::destroy() noexcept {
    if (!initialized()) {
        return;
    }

    vk::FreeMemory(device(), handle(), NULL);
    handle_ = VK_NULL_HANDLE;
}

DeviceMemory::~DeviceMemory() noexcept { destroy(); }

void DeviceMemory::init(const Device &dev, const VkMemoryAllocateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::AllocateMemory, dev, &info);
    memory_allocate_info_ = info;
}

VkResult DeviceMemory::TryInit(const Device &dev, const VkMemoryAllocateInfo &info) {
    assert(!initialized());
    VkDeviceMemory handle = VK_NULL_HANDLE;
    auto result = vk::AllocateMemory(dev.handle(), &info, nullptr, &handle);
    if (result == VK_SUCCESS) {
        NonDispHandle::init(dev.handle(), handle);
    }
    return result;
}

const void *DeviceMemory::Map(VkFlags flags) const {
    void *data;
    VkResult result = vk::MapMemory(device(), handle(), 0, VK_WHOLE_SIZE, flags, &data);
    EXPECT_EQ(VK_SUCCESS, result);
    if (result != VK_SUCCESS) data = NULL;
    return data;
}

void *DeviceMemory::Map(VkFlags flags) {
    void *data;
    VkResult result = vk::MapMemory(device(), handle(), 0, VK_WHOLE_SIZE, flags, &data);
    EXPECT_EQ(VK_SUCCESS, result);
    if (result != VK_SUCCESS) data = NULL;

    return data;
}

void DeviceMemory::Unmap() const { vk::UnmapMemory(device(), handle()); }

VkMemoryAllocateInfo DeviceMemory::GetResourceAllocInfo(const Device &dev, const VkMemoryRequirements &reqs,
                                                        VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper(alloc_info_pnext);
    alloc_info.allocationSize = reqs.size;
    EXPECT_TRUE(dev.Physical().SetMemoryType(reqs.memoryTypeBits, &alloc_info, mem_props));
    return alloc_info;
}

NON_DISPATCHABLE_HANDLE_DTOR(Fence, vk::DestroyFence)

Fence &Fence::operator=(Fence &&rhs) noexcept {
    destroy();
    NonDispHandle<VkFence>::operator=(std::move(rhs));
    return *this;
}

void Fence::Init(const Device &dev, const VkFenceCreateInfo &info) { NON_DISPATCHABLE_HANDLE_INIT(vk::CreateFence, dev, &info); }

void Fence::Init(const Device &dev, const VkFenceCreateFlags flags) {
    VkFenceCreateInfo fence_ci = vku::InitStructHelper();
    fence_ci.flags = flags;
    Init(dev, fence_ci);
}

VkResult Fence::Wait(uint64_t timeout) const { return vk::WaitForFences(device(), 1, &handle(), VK_TRUE, timeout); }

VkResult Fence::Reset() const { return vk::ResetFences(device(), 1, &handle()); }

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult Fence::ExportHandle(HANDLE &win32_handle, VkExternalFenceHandleTypeFlagBits handle_type) {
    VkFenceGetWin32HandleInfoKHR ghi = vku::InitStructHelper();
    ghi.fence = handle();
    ghi.handleType = handle_type;
    return vk::GetFenceWin32HandleKHR(device(), &ghi, &win32_handle);
}

VkResult Fence::ImportHandle(HANDLE win32_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    VkImportFenceWin32HandleInfoKHR ifi = vku::InitStructHelper();
    ifi.fence = handle();
    ifi.handleType = handle_type;
    ifi.handle = win32_handle;
    ifi.flags = flags;
    return vk::ImportFenceWin32HandleKHR(device(), &ifi);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult Fence::ExportHandle(int &fd_handle, VkExternalFenceHandleTypeFlagBits handle_type) {
    VkFenceGetFdInfoKHR gfi = vku::InitStructHelper();
    gfi.fence = handle();
    gfi.handleType = handle_type;
    return vk::GetFenceFdKHR(device(), &gfi, &fd_handle);
}

VkResult Fence::ImportHandle(int fd_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    VkImportFenceFdInfoKHR ifi = vku::InitStructHelper();
    ifi.fence = handle();
    ifi.handleType = handle_type;
    ifi.fd = fd_handle;
    ifi.flags = flags;
    return vk::ImportFenceFdKHR(device(), &ifi);
}

NON_DISPATCHABLE_HANDLE_DTOR(Semaphore, vk::DestroySemaphore)

Semaphore::Semaphore(const Device &dev, VkSemaphoreType type, uint64_t initial_value) {
    if (type == VK_SEMAPHORE_TYPE_BINARY) {
        init(dev, vku::InitStruct<VkSemaphoreCreateInfo>());
    } else {
        VkSemaphoreTypeCreateInfo semaphore_type_ci = vku::InitStructHelper();
        semaphore_type_ci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphore_type_ci.initialValue = initial_value;
        VkSemaphoreCreateInfo semaphore_ci = vku::InitStructHelper(&semaphore_type_ci);
        init(dev, semaphore_ci);
    }
}

Semaphore &Semaphore::operator=(Semaphore &&rhs) noexcept {
    destroy();
    NonDispHandle<VkSemaphore>::operator=(std::move(rhs));
    return *this;
}

void Semaphore::init(const Device &dev, const VkSemaphoreCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSemaphore, dev, &info);
}

VkResult Semaphore::Wait(uint64_t value, uint64_t timeout) {
    VkSemaphoreWaitInfo wait_info = vku::InitStructHelper();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &handle();
    wait_info.pValues = &value;
    VkResult result = vk::WaitSemaphores(device(), &wait_info, timeout);
    return result;
}

VkResult Semaphore::WaitKHR(uint64_t value, uint64_t timeout) {
    VkSemaphoreWaitInfoKHR wait_info = vku::InitStructHelper();
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &handle();
    wait_info.pValues = &value;
    VkResult result = vk::WaitSemaphoresKHR(device(), &wait_info, timeout);
    return result;
}

VkResult Semaphore::Signal(uint64_t value) {
    VkSemaphoreSignalInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = handle();
    signal_info.value = value;
    VkResult result = vk::SignalSemaphore(device(), &signal_info);
    return result;
}

VkResult Semaphore::SignalKHR(uint64_t value) {
    VkSemaphoreSignalInfoKHR signal_info = vku::InitStructHelper();
    signal_info.semaphore = handle();
    signal_info.value = value;
    VkResult result = vk::SignalSemaphoreKHR(device(), &signal_info);
    return result;
}

uint64_t Semaphore::GetCounterValue(bool use_khr) const {
    uint64_t counter = 0;
    if (use_khr) {
        vk::GetSemaphoreCounterValueKHR(device(), handle(), &counter);
    } else {
        vk::GetSemaphoreCounterValue(device(), handle(), &counter);
    }
    return counter;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
VkResult Semaphore::ExportHandle(HANDLE &win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    win32_handle = nullptr;
    VkSemaphoreGetWin32HandleInfoKHR ghi = vku::InitStructHelper();
    ghi.semaphore = handle();
    ghi.handleType = handle_type;
    return vk::GetSemaphoreWin32HandleKHR(device(), &ghi, &win32_handle);
}

VkResult Semaphore::ImportHandle(HANDLE win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                 VkSemaphoreImportFlags flags) {
    VkImportSemaphoreWin32HandleInfoKHR ihi = vku::InitStructHelper();
    ihi.semaphore = handle();
    ihi.handleType = handle_type;
    ihi.handle = win32_handle;
    ihi.flags = flags;
    return vk::ImportSemaphoreWin32HandleKHR(device(), &ihi);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

VkResult Semaphore::ExportHandle(int &fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    fd_handle = -1;
    VkSemaphoreGetFdInfoKHR ghi = vku::InitStructHelper();
    ghi.semaphore = handle();
    ghi.handleType = handle_type;
    return vk::GetSemaphoreFdKHR(device(), &ghi, &fd_handle);
}

VkResult Semaphore::ImportHandle(int fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags) {
    // Import opaque handle exported above
    VkImportSemaphoreFdInfoKHR ihi = vku::InitStructHelper();
    ihi.semaphore = handle();
    ihi.handleType = handle_type;
    ihi.fd = fd_handle;
    ihi.flags = flags;
    return vk::ImportSemaphoreFdKHR(device(), &ihi);
}

NON_DISPATCHABLE_HANDLE_DTOR(Event, vk::DestroyEvent)

void Event::init(const Device &dev, const VkEventCreateInfo &info) { NON_DISPATCHABLE_HANDLE_INIT(vk::CreateEvent, dev, &info); }

void Event::Set() { ASSERT_EQ(VK_SUCCESS, vk::SetEvent(device(), handle())); }

void Event::CmdSet(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask) {
    vk::CmdSetEvent(cmd.handle(), handle(), stage_mask);
}

void Event::CmdReset(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask) {
    vk::CmdResetEvent(cmd.handle(), handle(), stage_mask);
}

void Event::CmdWait(const CommandBuffer &cmd, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                    const std::vector<VkMemoryBarrier> &memory_barriers, const std::vector<VkBufferMemoryBarrier> &buffer_barriers,
                    const std::vector<VkImageMemoryBarrier> &image_barriers) {
    VkEvent event_handle = handle();
    vk::CmdWaitEvents(cmd.handle(), 1, &event_handle, src_stage_mask, dst_stage_mask, static_cast<uint32_t>(memory_barriers.size()),
                      memory_barriers.data(), static_cast<uint32_t>(buffer_barriers.size()), buffer_barriers.data(),
                      static_cast<uint32_t>(image_barriers.size()), image_barriers.data());
}

void Event::Reset() { ASSERT_EQ(VK_SUCCESS, vk::ResetEvent(device(), handle())); }

NON_DISPATCHABLE_HANDLE_DTOR(QueryPool, vk::DestroyQueryPool)

void QueryPool::init(const Device &dev, const VkQueryPoolCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateQueryPool, dev, &info);
}

VkResult QueryPool::Results(uint32_t first, uint32_t count, size_t size, void *data, size_t stride) {
    VkResult err = vk::GetQueryPoolResults(device(), handle(), first, count, size, data, stride, 0);
    EXPECT_TRUE(err == VK_SUCCESS || err == VK_NOT_READY);

    return err;
}

NON_DISPATCHABLE_HANDLE_DTOR(Buffer, vk::DestroyBuffer)

Buffer::Buffer(Buffer &&rhs) noexcept : NonDispHandle(std::move(rhs)) {
    create_info_ = std::move(rhs.create_info_);
    internal_mem_ = std::move(rhs.internal_mem_);
}

Buffer &Buffer::operator=(Buffer &&rhs) noexcept {
    if (&rhs == this) {
        return *this;
    }
    destroy();
    internal_mem_.destroy();
    NonDispHandle::operator=(std::move(rhs));
    create_info_ = std::move(rhs.create_info_);
    internal_mem_ = std::move(rhs.internal_mem_);
    return *this;
}

void Buffer::init(const Device &dev, const VkBufferCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    InitNoMemory(dev, info);

    auto alloc_info = DeviceMemory::GetResourceAllocInfo(dev, MemoryRequirements(), mem_props, alloc_info_pnext);
    internal_mem_.init(dev, alloc_info);

    BindMemory(internal_mem_, 0);
}

void Buffer::InitHostVisibleWithData(const Device &dev, VkBufferUsageFlags usage, const void *data, size_t data_size,
                                     const vvl::span<uint32_t> &queue_families) {
    const auto create_info = CreateInfo(static_cast<VkDeviceSize>(data_size), usage, queue_families);
    InitNoMemory(dev, create_info);

    // According to the specification there is always a host visible coherent memory type.
    // It can always be bound to a buffer created without SPARSE_BIDNING/PROTECTED flags.
    const VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    const VkMemoryRequirements memory_requirements = MemoryRequirements();
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = memory_requirements.size;
    dev.Physical().SetMemoryType(memory_requirements.memoryTypeBits, &alloc_info, memory_properties);
    internal_mem_.init(dev, alloc_info);
    BindMemory(internal_mem_, 0);

    void *ptr = internal_mem_.Map();
    std::memcpy(ptr, data, data_size);
    internal_mem_.Unmap();
}

void Buffer::InitNoMemory(const Device &dev, const VkBufferCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateBuffer, dev, &info);
    create_info_ = info;
}

VkMemoryRequirements Buffer::MemoryRequirements() const {
    VkMemoryRequirements reqs;

    vk::GetBufferMemoryRequirements(device(), handle(), &reqs);

    return reqs;
}

void Buffer::AllocateAndBindMemory(const Device &dev, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    assert(!internal_mem_.initialized());
    internal_mem_.init(dev, DeviceMemory::GetResourceAllocInfo(dev, MemoryRequirements(), mem_props, alloc_info_pnext));
    BindMemory(internal_mem_, 0);
}

void Buffer::BindMemory(const DeviceMemory &mem, VkDeviceSize mem_offset) {
    const auto result = vk::BindBufferMemory(device(), handle(), mem.handle(), mem_offset);
    // Allow successful calls and the calls that cause validation errors (but not actual Vulkan errors).
    // In the case of a validation error, it's part of the test logic how to handle it.
    ASSERT_TRUE(result == VK_SUCCESS || result == VK_ERROR_VALIDATION_FAILED_EXT);
}

VkDeviceAddress Buffer::Address() const {
    VkBufferDeviceAddressInfo bdai = vku::InitStructHelper();
    bdai.buffer = handle();
    if (vk::GetBufferDeviceAddressKHR) {
        return vk::GetBufferDeviceAddressKHR(device(), &bdai);
    } else {
        return vk::GetBufferDeviceAddress(device(), &bdai);
    }
}

NON_DISPATCHABLE_HANDLE_DTOR(BufferView, vk::DestroyBufferView)

void BufferView::init(const Device &dev, const VkBufferViewCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateBufferView, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Image, vk::DestroyImage)

Image::Image(const Device &dev, const VkImageCreateInfo &info) : device_(&dev) { init(dev, info, 0); }

Image::Image(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext)
    : device_(&dev) {
    init(dev, info, mem_props, alloc_info_pnext);
}

Image::Image(const Device &dev, uint32_t const width, uint32_t const height, uint32_t const mip_levels, VkFormat const format,
             VkFlags const usage)
    : device_(&dev) {
    Init(dev, width, height, mip_levels, format, usage);
}

Image::Image(const Device &dev, const VkImageCreateInfo &info, NoMemT) : device_(&dev) { InitNoMemory(dev, info); }

// If you find yourself wanting to expand this or making another overload, then you might really just want to call
//     SetLayout(VK_IMAGE_LAYOUT_GENERAL);
// after you init the image manually
Image::Image(const Device &dev, const VkImageCreateInfo &info, SetLayoutT) : device_(&dev) {
    init(*device_, info, 0);

    VkImageLayout newLayout;
    const auto usage = info.usage;
    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    } else if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
        newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    VkImageAspectFlags image_aspect = AspectMask(info.format);
    SetLayout(image_aspect, newLayout);
}

Image::Image(Image &&rhs) noexcept : NonDispHandle(std::move(rhs)) {
    device_ = std::move(rhs.device_);
    rhs.device_ = nullptr;

    create_info_ = std::move(rhs.create_info_);
    rhs.create_info_ = vku::InitStructHelper();

    internal_mem_ = std::move(rhs.internal_mem_);

    image_layout_ = std::move(rhs.image_layout_);
    rhs.image_layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
}

Image &Image::operator=(Image &&rhs) noexcept {
    if (&rhs == this) {
        return *this;
    }
    destroy();
    internal_mem_.destroy();
    NonDispHandle::operator=(std::move(rhs));

    device_ = std::move(rhs.device_);
    rhs.device_ = nullptr;

    create_info_ = std::move(rhs.create_info_);
    rhs.create_info_ = vku::InitStructHelper();

    internal_mem_ = std::move(rhs.internal_mem_);

    image_layout_ = std::move(rhs.image_layout_);
    rhs.image_layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
    return *this;
}

void Image::init(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    InitNoMemory(dev, info);

    if (initialized()) {
        auto alloc_info = DeviceMemory::GetResourceAllocInfo(dev, MemoryRequirements(), mem_props, alloc_info_pnext);
        internal_mem_.init(dev, alloc_info);
        BindMemory(internal_mem_, 0);
    }
}

void Image::Init(const Device &dev, uint32_t const width, uint32_t const height, uint32_t const mip_levels, VkFormat const format,
                 VkFlags const usage) {
    const VkImageCreateInfo info = ImageCreateInfo2D(width, height, mip_levels, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    init(dev, info, 0);
}

// Currently all init call here, so can set things for all path
void Image::InitNoMemory(const Device &dev, const VkImageCreateInfo &info) {
    if (!device_) {
        device_ = &dev;
    }
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateImage, dev, &info);
    create_info_ = info;
    image_layout_ = info.initialLayout;
}

bool Image::IsCompatible(const Device &dev, const VkImageUsageFlags usages, const VkFormatFeatureFlags2 features) {
    VkFormatFeatureFlags2 all_feature_flags =
        VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT |
        VK_FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT | VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT |
        VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT |
        VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT | VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT |
        VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_FORMAT_FEATURE_2_BLIT_SRC_BIT | VK_FORMAT_FEATURE_2_BLIT_DST_BIT | VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    if (dev.IsEnabledExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT;
    }

    if (dev.IsEnabledExtension(VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT;
    }

    if (dev.IsEnabledExtension(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_MINMAX_BIT;
    }

    if (dev.IsEnabledExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT |
                             VK_FORMAT_FEATURE_2_DISJOINT_BIT | VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT;
    }

    if (dev.IsEnabledExtension(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        all_feature_flags |= VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT |
                             VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT |
                             VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT;
    }

    if ((features & all_feature_flags) == 0) return false;  // whole format unsupported

    if ((usages & VK_IMAGE_USAGE_SAMPLED_BIT) && !(features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_STORAGE_BIT) && !(features & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT)) return false;
    if ((usages & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) && !(features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT))
        return false;

    return true;
}

VkImageCreateInfo Image::ImageCreateInfo2D(uint32_t const width, uint32_t const height, uint32_t const mip_levels,
                                           uint32_t const layers, VkFormat const format, VkFlags const usage,
                                           VkImageTiling const requested_tiling, const vvl::span<uint32_t> &queue_families) {
    VkImageCreateInfo imageCreateInfo = CreateInfo();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.mipLevels = mip_levels;
    imageCreateInfo.arrayLayers = layers;
    imageCreateInfo.tiling = requested_tiling;  // This will be touched up below...
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Automatically set sharing mode etc. based on queue family information
    if (queue_families.size() > 1) {
        imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queue_families.size());
        imageCreateInfo.pQueueFamilyIndices = queue_families.data();
    }
    imageCreateInfo.usage = usage;
    return imageCreateInfo;
}

VkMemoryRequirements Image::MemoryRequirements() const {
    VkMemoryRequirements reqs;

    vk::GetImageMemoryRequirements(device(), handle(), &reqs);

    return reqs;
}

void Image::AllocateAndBindMemory(const Device &dev, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext) {
    assert(!internal_mem_.initialized());
    internal_mem_.init(dev, DeviceMemory::GetResourceAllocInfo(dev, MemoryRequirements(), mem_props, alloc_info_pnext));
    BindMemory(internal_mem_, 0);
}

void Image::BindMemory(const DeviceMemory &mem, VkDeviceSize mem_offset) {
    const auto result = vk::BindImageMemory(device(), handle(), mem.handle(), mem_offset);
    // Allow successful calls and the calls that cause validation errors (but not actual Vulkan errors).
    // In the case of a validation error, it's part of the test logic how to handle it.
    ASSERT_TRUE(result == VK_SUCCESS || result == VK_ERROR_VALIDATION_FAILED_EXT);
}

VkImageAspectFlags Image::AspectMask(VkFormat format) {
    VkImageAspectFlags image_aspect;
    if (vkuFormatIsDepthAndStencil(format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (vkuFormatIsDepthOnly(format)) {
        image_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (vkuFormatIsStencilOnly(format)) {
        image_aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {  // color
        image_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    return image_aspect;
}

void Image::ImageMemoryBarrier(CommandBuffer &cmd_buf, VkImageAspectFlags aspect, VkFlags output_mask, VkFlags input_mask,
                               VkImageLayout image_layout, VkPipelineStageFlags src_stages, VkPipelineStageFlags dest_stages) {
    // clang-format on
    const VkImageSubresourceRange subresourceRange =
        SubresourceRange(aspect, 0, create_info_.mipLevels, 0, create_info_.arrayLayers);
    VkImageMemoryBarrier barrier;
    barrier = ImageMemoryBarrier(output_mask, input_mask, image_layout_, image_layout, subresourceRange);

    VkImageMemoryBarrier *pmemory_barrier = &barrier;

    // write barrier to the command buffer
    vk::CmdPipelineBarrier(cmd_buf.handle(), src_stages, dest_stages, VK_DEPENDENCY_BY_REGION_BIT, 0, NULL, 0, NULL, 1,
                           pmemory_barrier);
}

void Image::SetLayout(CommandBuffer &cmd_buf, VkImageAspectFlags aspect, VkImageLayout image_layout) {
    VkFlags src_mask, dst_mask;
    const VkFlags all_cache_outputs = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    const VkFlags all_cache_inputs = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
                                     VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                     VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;

    const VkFlags shader_read_inputs = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;

    if (image_layout == image_layout_) {
        return;
    }

    // Attempt to narrow the src_mask, by what the image could have validly been used for in it's current layout
    switch (image_layout_) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            src_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            src_mask = shader_read_inputs;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            src_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            src_mask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_UNDEFINED:
            src_mask = 0;
            break;
        default:
            src_mask = all_cache_outputs;  // Only need to worry about writes, as the stage mask will protect reads
    }

    // Narrow the dst mask by the valid accesss for the new layout
    switch (image_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // NOTE: not sure why shader read is here...
            dst_mask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            dst_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            dst_mask = shader_read_inputs;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            dst_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            dst_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        default:
            // Must wait all read and write operations for the completion of the layout tranisition
            dst_mask = all_cache_inputs | all_cache_outputs;
            break;
    }

    ImageMemoryBarrier(cmd_buf, aspect, src_mask, dst_mask, image_layout);
    image_layout_ = image_layout;
}

void Image::SetLayout(VkImageAspectFlags aspect, VkImageLayout image_layout) {
    if (image_layout == image_layout_) {
        return;
    }

    CommandPool pool(*device_, device_->graphics_queue_node_index_);
    CommandBuffer cmd_buf(*device_, pool);

    /* Build command buffer to set image layout in the driver */
    cmd_buf.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    SetLayout(cmd_buf, aspect, image_layout);
    cmd_buf.End();

    auto graphics_queue = device_->QueuesWithGraphicsCapability()[0];
    graphics_queue->Submit(cmd_buf);
    graphics_queue->Wait();
}

VkImageViewCreateInfo Image::BasicViewCreatInfo(VkImageAspectFlags aspect_mask) const {
    VkImageViewCreateInfo ci = vku::InitStructHelper();
    ci.image = handle();
    ci.format = Format();
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.components.r = VK_COMPONENT_SWIZZLE_R;
    ci.components.g = VK_COMPONENT_SWIZZLE_G;
    ci.components.b = VK_COMPONENT_SWIZZLE_B;
    ci.components.a = VK_COMPONENT_SWIZZLE_A;
    ci.subresourceRange = {aspect_mask, 0, 1, 0, 1};
    return ci;
}

ImageView Image::CreateView(VkImageAspectFlags aspect, void *pNext) const {
    VkImageViewCreateInfo ci = BasicViewCreatInfo(aspect);
    ci.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    ci.pNext = pNext;
    return ImageView(*device_, ci);
}

ImageView Image::CreateView(VkImageViewType type, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer,
                            uint32_t layerCount, VkImageAspectFlags aspect) const {
    VkImageViewCreateInfo ci = BasicViewCreatInfo();
    ci.viewType = type;
    ci.subresourceRange = {aspect, baseMipLevel, levelCount, baseArrayLayer, layerCount};
    return ImageView(*device_, ci);
}

NON_DISPATCHABLE_HANDLE_DTOR(ImageView, vk::DestroyImageView)

void ImageView::init(const Device &dev, const VkImageViewCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateImageView, dev, &info);
}

void AccelerationStructureNV::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV =
        (PFN_vkDestroyAccelerationStructureNV)vk::GetDeviceProcAddr(device(), "vkDestroyAccelerationStructureNV");
    assert(vkDestroyAccelerationStructureNV != nullptr);

    vkDestroyAccelerationStructureNV(device(), handle(), nullptr);
    handle_ = VK_NULL_HANDLE;
}
AccelerationStructureNV::~AccelerationStructureNV() noexcept { destroy(); }

VkMemoryRequirements2 AccelerationStructureNV::MemoryRequirements() const {
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vk::GetDeviceProcAddr(device(),
                                                                                  "vkGetAccelerationStructureMemoryRequirementsNV");
    assert(vkGetAccelerationStructureMemoryRequirementsNV != nullptr);
    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper();
    VkAccelerationStructureMemoryRequirementsInfoNV memory_requirements_info = vku::InitStructHelper();
    memory_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
    memory_requirements_info.accelerationStructure = handle();
    vkGetAccelerationStructureMemoryRequirementsNV(device(), &memory_requirements_info, &memory_requirements);
    return memory_requirements;
}

VkMemoryRequirements2 AccelerationStructureNV::BuildScratchMemoryRequirements() const {
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV =
        (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vk::GetDeviceProcAddr(device(),
                                                                                  "vkGetAccelerationStructureMemoryRequirementsNV");
    assert(vkGetAccelerationStructureMemoryRequirementsNV != nullptr);

    VkAccelerationStructureMemoryRequirementsInfoNV memory_requirements_info = vku::InitStructHelper();
    memory_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    memory_requirements_info.accelerationStructure = handle();

    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper();
    vkGetAccelerationStructureMemoryRequirementsNV(device(), &memory_requirements_info, &memory_requirements);
    return memory_requirements;
}

void AccelerationStructureNV::init(const Device &dev, const VkAccelerationStructureCreateInfoNV &info, bool init_memory) {
    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV =
        (PFN_vkCreateAccelerationStructureNV)vk::GetDeviceProcAddr(dev.handle(), "vkCreateAccelerationStructureNV");
    assert(vkCreateAccelerationStructureNV != nullptr);

    NON_DISPATCHABLE_HANDLE_INIT(vkCreateAccelerationStructureNV, dev, &info);

    info_ = info.info;

    if (init_memory) {
        memory_.init(dev, DeviceMemory::GetResourceAllocInfo(dev, MemoryRequirements().memoryRequirements,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV =
            (PFN_vkBindAccelerationStructureMemoryNV)vk::GetDeviceProcAddr(dev.handle(), "vkBindAccelerationStructureMemoryNV");
        assert(vkBindAccelerationStructureMemoryNV != nullptr);

        VkBindAccelerationStructureMemoryInfoNV bind_info = vku::InitStructHelper();
        bind_info.accelerationStructure = handle();
        bind_info.memory = memory_.handle();
        ASSERT_EQ(VK_SUCCESS, vkBindAccelerationStructureMemoryNV(dev.handle(), 1, &bind_info));

        PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV =
            (PFN_vkGetAccelerationStructureHandleNV)vk::GetDeviceProcAddr(dev.handle(), "vkGetAccelerationStructureHandleNV");
        assert(vkGetAccelerationStructureHandleNV != nullptr);
        ASSERT_EQ(VK_SUCCESS, vkGetAccelerationStructureHandleNV(dev.handle(), handle(), sizeof(uint64_t), &opaque_handle_));
    }
}

Buffer AccelerationStructureNV::CreateScratchBuffer(const Device &device, VkBufferCreateInfo *pCreateInfo /*= nullptr*/,
                                                    bool buffer_device_address /*= false*/) const {
    VkMemoryRequirements scratch_buffer_memory_requirements = BuildScratchMemoryRequirements().memoryRequirements;
    VkBufferCreateInfo create_info = {};
    create_info.size = scratch_buffer_memory_requirements.size;
    if (pCreateInfo) {
        create_info.sType = pCreateInfo->sType;
        create_info.usage = pCreateInfo->usage;
    } else {
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
        if (buffer_device_address) create_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    void *pNext = nullptr;
    if (buffer_device_address) {
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        pNext = &alloc_flags;
    }

    return Buffer(device, create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pNext);
}

NON_DISPATCHABLE_HANDLE_DTOR(ShaderModule, vk::DestroyShaderModule)

void ShaderModule::init(const Device &dev, const VkShaderModuleCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateShaderModule, dev, &info);
}

VkResult ShaderModule::InitTry(const Device &dev, const VkShaderModuleCreateInfo &info) {
    VkShaderModule mod;

    VkResult err = vk::CreateShaderModule(dev.handle(), &info, NULL, &mod);
    if (err == VK_SUCCESS) NonDispHandle::init(dev.handle(), mod);

    return err;
}

NON_DISPATCHABLE_HANDLE_DTOR(Shader, vk::DestroyShaderEXT)

void Shader::init(const Device &dev, const VkShaderCreateInfoEXT &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateShadersEXT, dev, 1u, &info);
}

VkResult Shader::InitTry(const Device &dev, const VkShaderCreateInfoEXT &info) {
    VkShaderEXT mod;

    VkResult err = vk::CreateShadersEXT(dev.handle(), 1u, &info, NULL, &mod);
    if (err == VK_SUCCESS) NonDispHandle::init(dev.handle(), mod);

    return err;
}

Shader::Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint32_t> &spv,
               const VkDescriptorSetLayout *descriptorSetLayout, const VkPushConstantRange *pushConstRange) {
    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = stage;
    createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    createInfo.codeSize = spv.size() * sizeof(spv[0]);
    createInfo.pCode = spv.data();
    createInfo.pName = "main";
    if (descriptorSetLayout) {
        createInfo.setLayoutCount = 1u;
        createInfo.pSetLayouts = descriptorSetLayout;
    }
    if (pushConstRange) {
        createInfo.pushConstantRangeCount = 1u;
        createInfo.pPushConstantRanges = pushConstRange;
    }
    init(dev, createInfo);
}

Shader::Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint8_t> &binary,
               const VkDescriptorSetLayout *descriptorSetLayout, const VkPushConstantRange *pushConstRange) {
    VkShaderCreateInfoEXT createInfo = vku::InitStructHelper();
    createInfo.stage = stage;
    createInfo.codeType = VK_SHADER_CODE_TYPE_BINARY_EXT;
    createInfo.codeSize = binary.size();
    createInfo.pCode = binary.data();
    createInfo.pName = "main";
    if (descriptorSetLayout) {
        createInfo.setLayoutCount = 1u;
        createInfo.pSetLayouts = descriptorSetLayout;
    }
    if (pushConstRange) {
        createInfo.pushConstantRangeCount = 1u;
        createInfo.pPushConstantRanges = pushConstRange;
    }
    init(dev, createInfo);
}

NON_DISPATCHABLE_HANDLE_DTOR(PipelineCache, vk::DestroyPipelineCache)

void PipelineCache::init(const Device &dev, const VkPipelineCacheCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreatePipelineCache, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Pipeline, vk::DestroyPipeline)

void Pipeline::init(const Device &dev, const VkGraphicsPipelineCreateInfo &info) {
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    if (err == VK_SUCCESS) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateGraphicsPipelines, dev, cache, 1, &info);
        vk::DestroyPipelineCache(dev.handle(), cache, NULL);
    }
}

VkResult Pipeline::InitTry(const Device &dev, const VkGraphicsPipelineCreateInfo &info) {
    VkPipeline pipe;
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    EXPECT_EQ(VK_SUCCESS, err);
    if (err == VK_SUCCESS) {
        err = vk::CreateGraphicsPipelines(dev.handle(), cache, 1, &info, NULL, &pipe);
        if (err == VK_SUCCESS) {
            NonDispHandle::init(dev.handle(), pipe);
        }
        vk::DestroyPipelineCache(dev.handle(), cache, NULL);
    }

    return err;
}

void Pipeline::init(const Device &dev, const VkComputePipelineCreateInfo &info) {
    VkPipelineCache cache;
    VkPipelineCacheCreateInfo ci = vku::InitStructHelper();
    VkResult err = vk::CreatePipelineCache(dev.handle(), &ci, NULL, &cache);
    if (err == VK_SUCCESS) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateComputePipelines, dev, cache, 1, &info);
        vk::DestroyPipelineCache(dev.handle(), cache, NULL);
    }
}

void Pipeline::init(const Device &dev, const VkRayTracingPipelineCreateInfoKHR &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateRayTracingPipelinesKHR, dev, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &info);
}

void Pipeline::InitDeferred(const Device &dev, const VkRayTracingPipelineCreateInfoKHR &info, VkDeferredOperationKHR deferred_op) {
    // ALL parameters need to survive until deferred operation is completed
    // In our case, it means both info and handle need to be kept alive
    // => cannot use NON_DISPATCHABLE_HANDLE_INIT because used handle is a temporary
    const VkResult result =
        vk::CreateRayTracingPipelinesKHR(dev.handle(), deferred_op, VK_NULL_HANDLE, 1, &info, nullptr, &handle());
    ASSERT_TRUE(result == VK_OPERATION_DEFERRED_KHR || result == VK_OPERATION_NOT_DEFERRED_KHR || result == VK_SUCCESS);
    NonDispHandle::SetDevice(dev.handle());
}

NON_DISPATCHABLE_HANDLE_DTOR(PipelineLayout, vk::DestroyPipelineLayout)

void PipelineLayout::init(const Device &dev, VkPipelineLayoutCreateInfo &info,
                          const std::vector<const DescriptorSetLayout *> &layouts) {
    const std::vector<VkDescriptorSetLayout> layout_handles = MakeVkHandles<VkDescriptorSetLayout>(layouts);
    info.setLayoutCount = static_cast<uint32_t>(layout_handles.size());
    info.pSetLayouts = layout_handles.data();

    init(dev, info);
}

void PipelineLayout::init(const Device &dev, VkPipelineLayoutCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreatePipelineLayout, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Sampler, vk::DestroySampler)

void Sampler::init(const Device &dev, const VkSamplerCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSampler, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(DescriptorSetLayout, vk::DestroyDescriptorSetLayout)

void DescriptorSetLayout::init(const Device &dev, const VkDescriptorSetLayoutCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateDescriptorSetLayout, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(DescriptorPool, vk::DestroyDescriptorPool)

void DescriptorPool::init(const Device &dev, const VkDescriptorPoolCreateInfo &info) {
    dynamic_usage_ = (info.flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) != 0;
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateDescriptorPool, dev, &info);
}

void DescriptorPool::Reset() { ASSERT_EQ(VK_SUCCESS, vk::ResetDescriptorPool(device(), handle(), 0)); }

std::vector<DescriptorSet *> DescriptorPool::AllocateSets(const Device &dev,
                                                          const std::vector<const DescriptorSetLayout *> &layouts) {
    const std::vector<VkDescriptorSetLayout> layout_handles = MakeVkHandles<VkDescriptorSetLayout>(layouts);

    std::vector<VkDescriptorSet> set_handles;
    set_handles.resize(layout_handles.size());

    VkDescriptorSetAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.descriptorSetCount = static_cast<uint32_t>(layout_handles.size());
    alloc_info.descriptorPool = handle();
    alloc_info.pSetLayouts = layout_handles.data();
    VkResult err = vk::AllocateDescriptorSets(device(), &alloc_info, set_handles.data());
    EXPECT_EQ(VK_SUCCESS, err);

    std::vector<DescriptorSet *> sets;
    for (std::vector<VkDescriptorSet>::const_iterator it = set_handles.begin(); it != set_handles.end(); it++) {
        // do descriptor sets need memories bound?
        DescriptorSet *descriptorSet = new DescriptorSet(dev, this, *it);
        sets.push_back(descriptorSet);
    }
    return sets;
}

std::vector<DescriptorSet *> DescriptorPool::AllocateSets(const Device &dev, const DescriptorSetLayout &layout, uint32_t count) {
    return AllocateSets(dev, std::vector<const DescriptorSetLayout *>(count, &layout));
}

DescriptorSet *DescriptorPool::AllocateSets(const Device &dev, const DescriptorSetLayout &layout) {
    std::vector<DescriptorSet *> set = AllocateSets(dev, layout, 1);
    return (set.empty()) ? NULL : set[0];
}
void DescriptorSet::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    // Only call vk::Free* on sets allocated from pool with usage *_DYNAMIC
    if (containing_pool_->GetDynamicUsage()) {
        VkDescriptorSet sets[1] = {handle()};
        ASSERT_EQ(VK_SUCCESS, vk::FreeDescriptorSets(device(), containing_pool_->handle(), 1, sets));
    }
    handle_ = VK_NULL_HANDLE;
}
DescriptorSet::~DescriptorSet() noexcept { destroy(); }

void DescriptorUpdateTemplate::Init(const Device &dev, const VkDescriptorUpdateTemplateCreateInfo &info) {
    if (vk::CreateDescriptorUpdateTemplateKHR) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateDescriptorUpdateTemplateKHR, dev, &info);
    } else {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateDescriptorUpdateTemplate, dev, &info);
    }
}

void DescriptorUpdateTemplate::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    if (vk::DestroyDescriptorUpdateTemplateKHR) {
        vk::DestroyDescriptorUpdateTemplateKHR(device(), handle(), nullptr);
    } else {
        vk::DestroyDescriptorUpdateTemplate(device(), handle(), nullptr);
    }
    handle_ = VK_NULL_HANDLE;
    internal::NonDispHandle<decltype(handle_)>::destroy();
}

DescriptorUpdateTemplate::~DescriptorUpdateTemplate() noexcept { destroy(); }

NON_DISPATCHABLE_HANDLE_DTOR(CommandPool, vk::DestroyCommandPool)

void CommandPool::Init(const Device &dev, const VkCommandPoolCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateCommandPool, dev, &info);
}

void CommandPool::Init(const Device &dev, uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo pool_ci = vku::InitStructHelper();
    pool_ci.flags = flags;
    pool_ci.queueFamilyIndex = queue_family_index;
    Init(dev, pool_ci);
}

CommandPool::CommandPool(const Device &dev, uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
    Init(dev, queue_family_index, flags);
}

void CommandBuffer::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    VkCommandBuffer cmds[] = {handle()};
    vk::FreeCommandBuffers(dev_handle_, cmd_pool_, 1, cmds);
    handle_ = VK_NULL_HANDLE;
}
CommandBuffer::~CommandBuffer() noexcept { destroy(); }

void CommandBuffer::init(const Device &dev, const VkCommandBufferAllocateInfo &info) {
    VkCommandBuffer cmd;

    // Make sure commandPool is set
    assert(info.commandPool);

    ASSERT_EQ(VK_SUCCESS, vk::AllocateCommandBuffers(dev.handle(), &info, &cmd));
    Handle::init(cmd);
    dev_handle_ = dev.handle();
    cmd_pool_ = info.commandPool;
}

void CommandBuffer::Init(const Device &dev, const CommandPool &pool, VkCommandBufferLevel level) {
    auto create_info = CommandBuffer::CreateInfo(pool.handle());
    create_info.level = level;
    init(dev, create_info);
}

void CommandBuffer::Begin(const VkCommandBufferBeginInfo *info) { ASSERT_EQ(VK_SUCCESS, vk::BeginCommandBuffer(handle(), info)); }

void CommandBuffer::Begin(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    info.flags = flags;
    info.pInheritanceInfo = &hinfo;
    hinfo.renderPass = VK_NULL_HANDLE;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    Begin(&info);
}

void CommandBuffer::End() {
    VkResult result = vk::EndCommandBuffer(handle());
    if (result == VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR) {
        GTEST_MESSAGE_AT_(__FILE__, __LINE__, "", ::testing::TestPartResult::kSkip)
            << "This test cannot be executed on an implementation that reports VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR in "
               "vkEndCommandBuffer()";
        throw testing::AssertionException(testing::TestPartResult(testing::TestPartResult::kSkip, __FILE__, __LINE__, ""));
    } else {
        ASSERT_EQ(VK_SUCCESS, result);
    }
}

void CommandBuffer::Reset(VkCommandBufferResetFlags flags) { ASSERT_EQ(VK_SUCCESS, vk::ResetCommandBuffer(handle(), flags)); }

VkCommandBufferAllocateInfo CommandBuffer::CreateInfo(VkCommandPool const &pool) {
    VkCommandBufferAllocateInfo info = vku::InitStructHelper();
    info.commandPool = pool;
    info.commandBufferCount = 1;
    return info;
}

void CommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents) {
    vk::CmdBeginRenderPass(handle(), &info, contents);
}

void CommandBuffer::BeginRenderPass(VkRenderPass rp, VkFramebuffer fb, uint32_t render_area_width, uint32_t render_area_height,
                                    uint32_t clear_count, VkClearValue *clear_values) {
    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = rp;
    render_pass_begin_info.framebuffer = fb;
    render_pass_begin_info.renderArea.extent.width = render_area_width;
    render_pass_begin_info.renderArea.extent.height = render_area_height;
    render_pass_begin_info.clearValueCount = clear_count;
    render_pass_begin_info.pClearValues = clear_values;
    vk::CmdBeginRenderPass(handle(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::NextSubpass(VkSubpassContents contents) { vk::CmdNextSubpass(handle(), contents); }

void CommandBuffer::EndRenderPass() { vk::CmdEndRenderPass(handle()); }

void CommandBuffer::BeginRendering(const VkRenderingInfo &renderingInfo) {
    if (vk::CmdBeginRenderingKHR) {
        vk::CmdBeginRenderingKHR(handle(), &renderingInfo);
    } else {
        vk::CmdBeginRendering(handle(), &renderingInfo);
    }
}

void CommandBuffer::BeginRenderingColor(const VkImageView imageView, VkRect2D render_area) {
    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = imageView;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo renderingInfo = vku::InitStructHelper();
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &color_attachment;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = render_area;

    BeginRendering(renderingInfo);
}

void CommandBuffer::EndRendering() {
    if (vk::CmdEndRenderingKHR) {
        vk::CmdEndRenderingKHR(handle());
    } else {
        vk::CmdEndRendering(handle());
    }
}

void CommandBuffer::BindVertFragShader(const vkt::Shader &vert_shader, const vkt::Shader &frag_shader) {
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT};
    const VkShaderEXT shaders[] = {vert_shader.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, frag_shader.handle()};
    vk::CmdBindShadersEXT(handle(), 5, stages, shaders);
}

void CommandBuffer::BindCompShader(const vkt::Shader &comp_shader) {
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    const VkShaderEXT shaders[] = {comp_shader.handle()};
    vk::CmdBindShadersEXT(handle(), 1, stages, shaders);
}

void CommandBuffer::BeginVideoCoding(const VkVideoBeginCodingInfoKHR &beginInfo) {
    vk::CmdBeginVideoCodingKHR(handle(), &beginInfo);
}

void CommandBuffer::ControlVideoCoding(const VkVideoCodingControlInfoKHR &controlInfo) {
    vk::CmdControlVideoCodingKHR(handle(), &controlInfo);
}

void CommandBuffer::DecodeVideo(const VkVideoDecodeInfoKHR &decodeInfo) { vk::CmdDecodeVideoKHR(handle(), &decodeInfo); }

void CommandBuffer::EncodeVideo(const VkVideoEncodeInfoKHR &encodeInfo) { vk::CmdEncodeVideoKHR(handle(), &encodeInfo); }

void CommandBuffer::EndVideoCoding(const VkVideoEndCodingInfoKHR &endInfo) { vk::CmdEndVideoCodingKHR(handle(), &endInfo); }

void CommandBuffer::Copy(const Buffer &src, const Buffer &dst) {
    assert(src.CreateInfo().size == dst.CreateInfo().size);
    const VkBufferCopy region = {0, 0, src.CreateInfo().size};
    vk::CmdCopyBuffer(handle(), src.handle(), dst.handle(), 1, &region);
}

void CommandBuffer::ExecuteCommands(const CommandBuffer &secondary) { vk::CmdExecuteCommands(handle(), 1, &secondary.handle()); }

void RenderPass::init(const Device &dev, const VkRenderPassCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateRenderPass, dev, &info);
}

void RenderPass::init(const Device &dev, const VkRenderPassCreateInfo2 &info) {
    if (vk::CreateRenderPass2KHR) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateRenderPass2KHR, dev, &info);
    } else {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateRenderPass2, dev, &info);
    }
}

NON_DISPATCHABLE_HANDLE_DTOR(RenderPass, vk::DestroyRenderPass)

void Framebuffer::init(const Device &dev, const VkFramebufferCreateInfo &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateFramebuffer, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(Framebuffer, vk::DestroyFramebuffer)

void SamplerYcbcrConversion::init(const Device &dev, const VkSamplerYcbcrConversionCreateInfo &info) {
    if (vk::CreateSamplerYcbcrConversionKHR) {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSamplerYcbcrConversionKHR, dev, &info);
    } else {
        NON_DISPATCHABLE_HANDLE_INIT(vk::CreateSamplerYcbcrConversion, dev, &info);
    }
}

VkSamplerYcbcrConversionInfo SamplerYcbcrConversion::ConversionInfo() {
    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = handle();
    return ycbcr_info;
}

// static
VkSamplerYcbcrConversionCreateInfo SamplerYcbcrConversion::DefaultConversionInfo(VkFormat format) {
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
    ycbcr_create_info.format = format;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;
    return ycbcr_create_info;
}

void SamplerYcbcrConversion::destroy() noexcept {
    if (!initialized()) {
        return;
    }
    if (vk::DestroySamplerYcbcrConversionKHR) {
        vk::DestroySamplerYcbcrConversionKHR(device(), handle(), nullptr);
    } else {
        vk::DestroySamplerYcbcrConversion(device(), handle(), nullptr);
    }
    handle_ = VK_NULL_HANDLE;
    internal::NonDispHandle<decltype(handle_)>::destroy();
}

SamplerYcbcrConversion::~SamplerYcbcrConversion() noexcept { destroy(); }

NON_DISPATCHABLE_HANDLE_DTOR(Swapchain, vk::DestroySwapchainKHR)

void Swapchain::Init(const Device &dev, const VkSwapchainCreateInfoKHR &info) {
    assert(!initialized());
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    auto result = vk::CreateSwapchainKHR(dev.handle(), &info, nullptr, &handle);
    // NOTE: Swapchain creation has more error codes comparing to other objects that use NON_DISPATCHABLE_HANDLE_INIT macro
    ASSERT_TRUE((result == VK_SUCCESS) || (result == VK_ERROR_VALIDATION_FAILED_EXT) || (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) ||
                (result == VK_ERROR_OUT_OF_HOST_MEMORY) || (result == VK_ERROR_SURFACE_LOST_KHR) ||
                (result == VK_ERROR_DEVICE_LOST) || (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) ||
                (result == VK_ERROR_INITIALIZATION_FAILED));
    if (result == VK_SUCCESS) {
        NonDispHandle::init(dev.handle(), handle);
    }
}

uint32_t Swapchain::GetImageCount() const {
    uint32_t image_count = 0;
    vk::GetSwapchainImagesKHR(device(), handle(), &image_count, nullptr);
    return image_count;
}

std::vector<VkImage> Swapchain::GetImages() const {
    uint32_t image_count = GetImageCount();
    std::vector<VkImage> images(image_count);
    vk::GetSwapchainImagesKHR(device(), handle(), &image_count, images.data());
    return images;
}

uint32_t Swapchain::AcquireNextImage(const Semaphore &image_acquired, uint64_t timeout, VkResult *result) {
    uint32_t image_index = 0;
    VkResult acquire_result = vk::AcquireNextImageKHR(device(), handle(), timeout, image_acquired, VK_NULL_HANDLE, &image_index);
    if (result) {
        *result = acquire_result;
    }
    return image_index;
}

uint32_t Swapchain::AcquireNextImage(const Fence &image_acquired, uint64_t timeout, VkResult *result) {
    uint32_t image_index = 0;
    VkResult acquire_result = vk::AcquireNextImageKHR(device(), handle(), timeout, VK_NULL_HANDLE, image_acquired, &image_index);
    if (result) {
        *result = acquire_result;
    }
    return image_index;
}

NON_DISPATCHABLE_HANDLE_DTOR(IndirectCommandsLayout, vk::DestroyIndirectCommandsLayoutEXT)
void IndirectCommandsLayout::Init(const Device &dev, const VkIndirectCommandsLayoutCreateInfoEXT &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateIndirectCommandsLayoutEXT, dev, &info);
}

NON_DISPATCHABLE_HANDLE_DTOR(IndirectExecutionSet, vk::DestroyIndirectExecutionSetEXT)
void IndirectExecutionSet::Init(const Device &dev, const VkIndirectExecutionSetCreateInfoEXT &info) {
    NON_DISPATCHABLE_HANDLE_INIT(vk::CreateIndirectExecutionSetEXT, dev, &info);
}

IndirectExecutionSet::IndirectExecutionSet(const Device &dev, VkPipeline init_pipeline, uint32_t max_pipelines) {
    VkIndirectExecutionSetPipelineInfoEXT exe_set_pipeline_info = vku::InitStructHelper();
    exe_set_pipeline_info.initialPipeline = init_pipeline;
    exe_set_pipeline_info.maxPipelineCount = max_pipelines;

    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_PIPELINES_EXT;
    exe_set_ci.info.pPipelineInfo = &exe_set_pipeline_info;
    Init(dev, exe_set_ci);
}
IndirectExecutionSet::IndirectExecutionSet(const Device &dev, const VkIndirectExecutionSetShaderInfoEXT &shader_info) {
    VkIndirectExecutionSetCreateInfoEXT exe_set_ci = vku::InitStructHelper();
    exe_set_ci.type = VK_INDIRECT_EXECUTION_SET_INFO_TYPE_SHADER_OBJECTS_EXT;
    exe_set_ci.info.pShaderInfo = &shader_info;
    Init(dev, exe_set_ci);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkResult Surface::Init(VkInstance instance, const VkWin32SurfaceCreateInfoKHR &info) {
    VkResult result = vk::CreateWin32SurfaceKHR(instance, &info, nullptr, &handle_);
    instance_ = instance;
    return result;
}
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
VkResult Surface::Init(VkInstance instance, const VkMetalSurfaceCreateInfoEXT &info) {
    VkResult result = vk::CreateMetalSurfaceEXT(instance, &info, nullptr, &handle_);
    instance_ = instance;
    return result;
}
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VkResult Surface::Init(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR &info) {
    VkResult result = vk::CreateAndroidSurfaceKHR(instance, &info, nullptr, &handle_);
    instance_ = instance;
    return result;
}
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
VkResult Surface::Init(VkInstance instance, const VkXlibSurfaceCreateInfoKHR &info) {
    VkResult result = vk::CreateXlibSurfaceKHR(instance, &info, nullptr, &handle_);
    instance_ = instance;
    return result;
}
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
VkResult Surface::Init(VkInstance instance, const VkXcbSurfaceCreateInfoKHR &info) {
    VkResult result = vk::CreateXcbSurfaceKHR(instance, &info, nullptr, &handle_);
    instance_ = instance;
    return result;
}
#endif
}  // namespace vkt
