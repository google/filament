/*
 * Copyright (c) 2015-2016, 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2016, 2020-2025 Valve Corporation
 * Copyright (c) 2015-2016, 2020-2025 LunarG, Inc.
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

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <vector>
#include <optional>

#include "containers/custom_containers.h"
#include "generated/vk_function_pointers.h"
#include "utils/cast_utils.h"
#include "test_common.h"

namespace vkt {

template <class Dst, class Src>
std::vector<Dst> MakeVkHandles(const std::vector<Src> &v) {
    std::vector<Dst> handles;
    handles.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(handles), [](const Src &o) { return o.handle(); });
    return handles;
}

template <class Dst, class Src>
std::vector<Dst> MakeVkHandles(const std::vector<Src *> &v) {
    std::vector<Dst> handles;
    handles.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(handles),
                   [](const Src *o) { return (o) ? o->handle() : VK_NULL_HANDLE; });
    return handles;
}

template <class Dst, class Src>
std::vector<Dst> MakeVkHandles(const vvl::span<Src *> &v) {
    std::vector<Dst> handles;
    handles.reserve(v.size());
    std::transform(v.begin(), v.end(), std::back_inserter(handles), [](const Src *o) { return o->handle(); });
    return handles;
}

class PhysicalDevice;
class Device;
class Queue;
class DeviceMemory;
class Fence;
class Semaphore;
class Event;
class QueryPool;
class Buffer;
class BufferView;
class Image;
class ImageView;
class DepthStencilView;
class PipelineCache;
class Pipeline;
class PipelineDelta;
class Sampler;
class DescriptorSetLayout;
class PipelineLayout;
class DescriptorSetPool;
class DescriptorSet;
class CommandBuffer;
class CommandPool;
class Swapchain;
class IndirectCommandsLayout;
class IndirectExecutionSet;

std::vector<VkLayerProperties> GetGlobalLayers();
std::vector<VkExtensionProperties> GetGlobalExtensions();
std::vector<VkExtensionProperties> GetGlobalExtensions(const char *pLayerName);

namespace internal {

template <typename T>
class Handle {
  public:
    const T &handle() const noexcept { return handle_; }
    T &handle() noexcept { return handle_; }
    bool initialized() const noexcept { return (handle_ != T{}); }

    operator T() const noexcept { return handle(); }

    void SetName(VkDevice device, VkObjectType object_type, const char *name) {
        VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
        name_info.objectType = object_type;
        name_info.objectHandle = CastToUint64(handle_);
        name_info.pObjectName = name;
        vk::SetDebugUtilsObjectNameEXT(device, &name_info);
    }

  protected:
    typedef T handle_type;

    explicit Handle() noexcept : handle_{} {}
    explicit Handle(T handle) noexcept : handle_(handle) {}

    // handles are non-copyable
    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;

    // handles can be moved out
    Handle(Handle &&src) noexcept : handle_{src.handle_} { src.handle_ = {}; }
    Handle &operator=(Handle &&src) noexcept {
        handle_ = src.handle_;
        src.handle_ = {};
        return *this;
    }

    void init(T handle) noexcept {
        assert(!initialized());
        handle_ = handle;
    }

  protected:
    T handle_;
};

template <typename T>
class NonDispHandle : public Handle<T> {
  protected:
    explicit NonDispHandle() noexcept : Handle<T>(), dev_handle_(VK_NULL_HANDLE) {}
    explicit NonDispHandle(VkDevice dev, T handle) noexcept : Handle<T>(handle), dev_handle_(dev) {}

    NonDispHandle(NonDispHandle &&src) noexcept : Handle<T>(std::move(src)) {
        dev_handle_ = src.dev_handle_;
        src.dev_handle_ = VK_NULL_HANDLE;
    }
    NonDispHandle &operator=(NonDispHandle &&src) noexcept {
        Handle<T>::operator=(std::move(src));
        dev_handle_ = src.dev_handle_;
        src.dev_handle_ = VK_NULL_HANDLE;
        return *this;
    }

    const VkDevice &device() const noexcept { return dev_handle_; }

    void init(VkDevice dev, T handle) noexcept {
        assert(!Handle<T>::initialized() && dev_handle_ == VK_NULL_HANDLE);
        Handle<T>::init(handle);
        dev_handle_ = dev;
    }

    void SetDevice(VkDevice device) { dev_handle_ = device; }

    void destroy() noexcept { dev_handle_ = VK_NULL_HANDLE; }

  public:
    void SetName(VkObjectType object_type, const char *name) { Handle<T>::SetName(dev_handle_, object_type, name); }

  private:
    VkDevice dev_handle_;
};

}  // namespace internal
   //
class Instance {
  public:
    Instance(const VkInstanceCreateInfo &info) { Init(info); }
    void Init(const VkInstanceCreateInfo &info);

    ~Instance() noexcept { Destroy(); }
    void Destroy() noexcept;

    VkInstance Handle() const { return handle_; }

    Instance(Instance &&src) noexcept : handle_{src.handle_} { src.handle_ = {}; }
    Instance &operator=(Instance &&src) noexcept {
        handle_ = src.handle_;
        src.handle_ = {};
        return *this;
    }

  private:
    VkInstance handle_{};
};

class PhysicalDevice : public internal::Handle<VkPhysicalDevice> {
  public:
    explicit PhysicalDevice(VkPhysicalDevice phy)
        : Handle(phy),
          properties_(Properties()),
          limits_(properties_.limits),
          memory_properties_(MemoryProperties()),
          queue_properties_(QueueProperties()) {}

    void SetName(VkDevice device, const char *name) {
        Handle<VkPhysicalDevice>::SetName(device, VK_OBJECT_TYPE_PHYSICAL_DEVICE, name);
    }
    VkPhysicalDeviceFeatures Features() const;

    bool SetMemoryType(const uint32_t type_bits, VkMemoryAllocateInfo *info, const VkMemoryPropertyFlags properties,
                       const VkMemoryPropertyFlags forbid = 0) const;

    // vkEnumerateDeviceExtensionProperties()
    std::vector<VkExtensionProperties> Extensions(const char *pLayerName = nullptr) const;

    // vkEnumerateLayers()
    std::vector<VkLayerProperties> Layers() const;

    const VkPhysicalDeviceProperties properties_;
    const VkPhysicalDeviceLimits limits_;
    const VkPhysicalDeviceMemoryProperties memory_properties_;
    const std::vector<VkQueueFamilyProperties> queue_properties_;

  private:
    VkPhysicalDeviceProperties Properties() const;
    std::vector<VkQueueFamilyProperties> QueueProperties() const;
    VkPhysicalDeviceMemoryProperties MemoryProperties() const;
};

class QueueCreateInfoArray {
  private:
    std::vector<VkDeviceQueueCreateInfo> queue_info_;
    std::vector<std::vector<float>> queue_priorities_;

  public:
    QueueCreateInfoArray(const std::vector<VkQueueFamilyProperties> &queue_props, bool all_queue_count = false);
    size_t Size() const { return queue_info_.size(); }
    const VkDeviceQueueCreateInfo *Data() const { return queue_info_.data(); }
};

class Device : public internal::Handle<VkDevice> {
  public:
    explicit Device(VkPhysicalDevice phy) : physical_device_(phy) { init(); }
    explicit Device(VkPhysicalDevice phy, const VkDeviceCreateInfo &info) : physical_device_(phy) { init(info); }
    explicit Device(VkPhysicalDevice phy, std::vector<const char *> &extension_names, VkPhysicalDeviceFeatures *features = nullptr,
                    void *create_device_pnext = nullptr, bool all_queue_count = false)
        : physical_device_(phy) {
        init(extension_names, features, create_device_pnext, all_queue_count);
    }

    ~Device() noexcept;
    void destroy() noexcept;

    // vkCreateDevice()
    void init(const VkDeviceCreateInfo &info);
    void init(std::vector<const char *> &extensions, VkPhysicalDeviceFeatures *features = nullptr,
              void *create_device_pnext = nullptr, bool all_queue_count = false);  // all queues, all extensions, etc
    void init() {
        std::vector<const char *> extensions;
        init(extensions);
    };

    void SetName(const char *name) { Handle<VkDevice>::SetName(handle_, VK_OBJECT_TYPE_DEVICE, name); }
    const PhysicalDevice &Physical() const { return physical_device_; }

    std::vector<const char *> GetEnabledExtensions() { return enabled_extensions_; }
    bool IsEnabledExtension(const char *extension) const;

    const std::vector<Queue *> &QueuesWithGraphicsCapability() const { return queues_[GRAPHICS]; }
    const std::vector<Queue *> &QueuesWithComputeCapability() const { return queues_[COMPUTE]; }
    const std::vector<Queue *> &QueuesWithTransferCapability() const { return queues_[TRANSFER]; }
    const std::vector<Queue *> &QueuesWithSparseCapability() const { return queues_[SPARSE]; }

    using QueueFamilyQueues = std::vector<std::unique_ptr<Queue>>;
    const QueueFamilyQueues &QueuesFromFamily(uint32_t queue_family) const;

    // Queue family that has "with" capabilities and optionally without "without" capabilities.
    std::optional<uint32_t> QueueFamily(VkQueueFlags with, VkQueueFlags without = 0) const;

    // Queue family that does not have "without" capabilities
    std::optional<uint32_t> QueueFamilyWithoutCapabilities(VkQueueFlags without) const;
    Queue *QueueWithoutCapabilities(VkQueueFlags without) const;

    // Dedicated compute queue family: has compute but no graphics
    std::optional<uint32_t> ComputeOnlyQueueFamily() const;
    Queue *ComputeOnlyQueue() const;

    // Dedicated transfer queue family: has tranfer but no graphics/compute
    std::optional<uint32_t> TransferOnlyQueueFamily() const;
    Queue *TransferOnlyQueue() const;

    // Compute or transfer
    std::optional<uint32_t> NonGraphicsQueueFamily() const;
    Queue *NonGraphicsQueue() const;

    uint32_t graphics_queue_node_index_ = vvl::kU32Max;

    const PhysicalDevice physical_device_;

    struct Format {
        VkFormat format;
        VkImageTiling tiling;
        VkFlags features;
    };

    VkFormatFeatureFlags2 FormatFeaturesLinear(VkFormat format) const;
    VkFormatFeatureFlags2 FormatFeaturesOptimal(VkFormat format) const;
    VkFormatFeatureFlags2 FormatFeaturesBuffer(VkFormat format) const;

    // vkDeviceWaitIdle()
    void Wait() const;

    // vkWaitForFences()
    VkResult Wait(const std::vector<const Fence *> &fences, bool wait_all, uint64_t timeout);
    VkResult Wait(const Fence &fence) { return Wait(std::vector<const Fence *>(1, &fence), true, (uint64_t)-1); }

    static VkWriteDescriptorSet WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                   VkDescriptorType type, uint32_t count, const VkDescriptorImageInfo *image_info);
    static VkWriteDescriptorSet WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                   VkDescriptorType type, uint32_t count,
                                                   const VkDescriptorBufferInfo *buffer_info);
    static VkWriteDescriptorSet WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                   VkDescriptorType type, uint32_t count, const VkBufferView *buffer_views);
    static VkWriteDescriptorSet WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                   VkDescriptorType type, const std::vector<VkDescriptorImageInfo> &image_info);
    static VkWriteDescriptorSet WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                   VkDescriptorType type, const std::vector<VkDescriptorBufferInfo> &buffer_info);
    static VkWriteDescriptorSet WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                   VkDescriptorType type, const std::vector<VkBufferView> &buffer_views);

  private:
    enum QueueCapabilityIndex {
        GRAPHICS = 0,
        COMPUTE = 1,
        TRANSFER = 2,
        SPARSE = 3,
        VIDEO_DECODE = 4,
        VIDEO_ENCODE = 5,
        QUEUE_CAPABILITY_COUNT = 6,
    };

    void InitQueues(const VkDeviceCreateInfo &info);

    std::vector<const char *> enabled_extensions_;

    std::vector<QueueFamilyQueues> queue_families_;
    std::vector<Queue *> queues_[QUEUE_CAPABILITY_COUNT];
};

class DeviceMemory : public internal::NonDispHandle<VkDeviceMemory> {
  public:
    DeviceMemory() = default;
    DeviceMemory(const Device &dev, const VkMemoryAllocateInfo &info) { init(dev, info); }
    ~DeviceMemory() noexcept;
    void destroy() noexcept;
    DeviceMemory &operator=(DeviceMemory &&) = default;

    // vkAllocateMemory()
    // Fails the test when allocation is unsuccessful
    void init(const Device &dev, const VkMemoryAllocateInfo &info);
    // Does not fail the test when allocation is unsuccessful and instead returns error code
    VkResult TryInit(const Device &dev, const VkMemoryAllocateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkDeviceMemory>::SetName(VK_OBJECT_TYPE_DEVICE_MEMORY, name); }

    // vkMapMemory()
    const void *Map(VkFlags flags) const;
    void *Map(VkFlags flags);
    const void *Map() const { return Map(0); }
    void *Map() { return Map(0); }

    // vkUnmapMemory()
    void Unmap() const;

    static VkMemoryAllocateInfo GetResourceAllocInfo(const Device &dev, const VkMemoryRequirements &reqs,
                                                     VkMemoryPropertyFlags mem_props, void *alloc_info_pnext = nullptr);

  private:
    VkMemoryAllocateInfo memory_allocate_info_{};
};

class Fence : public internal::NonDispHandle<VkFence> {
  public:
    Fence() = default;
    Fence(const Device &dev, const VkFenceCreateFlags flags = 0) { Init(dev, flags); }
    Fence(const Device &dev, const VkFenceCreateInfo &info) { Init(dev, info); }
    Fence(Fence &&rhs) noexcept : NonDispHandle(std::move(rhs)) {}
    Fence &operator=(Fence &&) noexcept;
    ~Fence() noexcept;
    void destroy() noexcept;

    // vkCreateFence()
    void Init(const Device &dev, const VkFenceCreateInfo &info);
    void Init(const Device &dev, const VkFenceCreateFlags flags = 0);
    void SetName(const char *name) { NonDispHandle<VkFence>::SetName(VK_OBJECT_TYPE_FENCE, name); }

    // vkGetFenceStatus()
    VkResult GetStatus() const { return vk::GetFenceStatus(device(), handle()); }
    VkResult Wait(uint64_t timeout) const;

    VkResult Reset() const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult ExportHandle(HANDLE &win32_handle, VkExternalFenceHandleTypeFlagBits handle_type);
    VkResult ImportHandle(HANDLE win32_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags = 0);
#endif
    VkResult ExportHandle(int &fd_handle, VkExternalFenceHandleTypeFlagBits handle_type);
    VkResult ImportHandle(int fd_handle, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags = 0);
};

inline const Fence no_fence;

class Semaphore : public internal::NonDispHandle<VkSemaphore> {
  public:
    Semaphore() = default;
    Semaphore(const Device &dev, VkSemaphoreType type = VK_SEMAPHORE_TYPE_BINARY, uint64_t initial_value = 0);
    Semaphore(const Device &dev, const VkSemaphoreCreateInfo &info) { init(dev, info); }
    Semaphore(Semaphore &&rhs) noexcept : NonDispHandle(std::move(rhs)) {}
    Semaphore &operator=(Semaphore &&) noexcept;
    ~Semaphore() noexcept;
    void destroy() noexcept;

    // vkCreateSemaphore()
    void init(const Device &dev, const VkSemaphoreCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkSemaphore>::SetName(VK_OBJECT_TYPE_SEMAPHORE, name); }

    VkResult Wait(uint64_t value, uint64_t timeout);
    VkResult WaitKHR(uint64_t value, uint64_t timeout);
    VkResult Signal(uint64_t value);
    VkResult SignalKHR(uint64_t value);
    uint64_t GetCounterValue(bool use_khr = false) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkResult ExportHandle(HANDLE &win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type);
    VkResult ImportHandle(HANDLE win32_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags = 0);
#endif
    VkResult ExportHandle(int &fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type);
    VkResult ImportHandle(int fd_handle, VkExternalSemaphoreHandleTypeFlagBits handle_type, VkSemaphoreImportFlags flags = 0);
};

inline const Semaphore no_semaphore;  // equivalent to vkt::Semaphore{}

class Event : public internal::NonDispHandle<VkEvent> {
  public:
    Event() = default;
    Event(const Device &dev) { init(dev, CreateInfo(0)); }
    Event(const Device &dev, const VkEventCreateInfo &info) { init(dev, info); }
    ~Event() noexcept;
    void destroy() noexcept;

    // vkCreateEvent()
    void init(const Device &dev, const VkEventCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkEvent>::SetName(VK_OBJECT_TYPE_EVENT, name); }

    // vkGetEventStatus()
    // vkSetEvent()
    // vkResetEvent()
    VkResult GetStatus() const { return vk::GetEventStatus(device(), handle()); }
    void Set();
    void CmdSet(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask);
    void CmdReset(const CommandBuffer &cmd, VkPipelineStageFlags stage_mask);
    void CmdWait(const CommandBuffer &cmd, VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                 const std::vector<VkMemoryBarrier> &memory_barriers, const std::vector<VkBufferMemoryBarrier> &buffer_barriers,
                 const std::vector<VkImageMemoryBarrier> &image_barriers);
    void Reset();

    static VkEventCreateInfo CreateInfo(VkFlags flags);
};

struct Wait {
    const Semaphore &semaphore;
    VkPipelineStageFlags2 stage_mask;

    explicit Wait(const Semaphore &semaphore, VkPipelineStageFlags2 stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)
        : semaphore(semaphore), stage_mask(stage_mask) {}
};

struct Signal {
    const Semaphore &semaphore;
    VkPipelineStageFlags2 stage_mask;

    explicit Signal(const Semaphore &semaphore, VkPipelineStageFlags2 stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)
        : semaphore(semaphore), stage_mask(stage_mask) {}
};

struct TimelineWait {
    const Semaphore &semaphore;
    uint64_t value;
    VkPipelineStageFlags2 stage_mask;

    explicit TimelineWait(const Semaphore &semaphore, uint64_t value,
                          VkPipelineStageFlags2 stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)
        : semaphore(semaphore), value(value), stage_mask(stage_mask) {}
};

struct TimelineSignal {
    const Semaphore &semaphore;
    uint64_t value;
    VkPipelineStageFlags2 stage_mask;

    explicit TimelineSignal(const Semaphore &semaphore, uint64_t value,
                            VkPipelineStageFlags2 stage_mask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT)
        : semaphore(semaphore), value(value), stage_mask(stage_mask) {}
};

class Queue : public internal::Handle<VkQueue> {
  public:
    explicit Queue(VkQueue queue, uint32_t index) : Handle(queue), family_index(index) {}
    void SetName(const Device &device, const char *name) { Handle<VkQueue>::SetName(device.handle(), VK_OBJECT_TYPE_QUEUE, name); }

    // vkQueueSubmit()
    VkResult Submit(const CommandBuffer &cmd, const Fence &fence = no_fence);
    VkResult Submit(const vvl::span<CommandBuffer *> &cmds, const Fence &fence = no_fence);

    VkResult Submit(const CommandBuffer &cmd, const Wait &wait, const Fence &fence = no_fence);
    VkResult Submit(const CommandBuffer &cmd, const Signal &signal, const Fence &fence = no_fence);
    VkResult Submit(const CommandBuffer &cmd, const Wait &wait, const Signal &signal, const Fence &fence = no_fence);

    VkResult Submit(const CommandBuffer &cmd, const TimelineWait &wait, const Fence &fence = no_fence);
    VkResult Submit(const CommandBuffer &cmd, const TimelineSignal &signal, const Fence &fence = no_fence);
    VkResult Submit(const CommandBuffer &cmd, const TimelineWait &wait, const TimelineSignal &signal,
                    const Fence &fence = no_fence);

    // vkQueueSubmit2()
    VkResult Submit2(const CommandBuffer &cmd, const Fence &fence = no_fence, bool use_khr = false);
    VkResult Submit2(const vvl::span<const CommandBuffer> &cmds, const Fence &fence = no_fence, bool use_khr = false);

    VkResult Submit2(const CommandBuffer &cmd, const Wait &wait, const Fence &fence = no_fence, bool use_khr = false);
    VkResult Submit2(const CommandBuffer &cmd, const Signal &signal, const Fence &fence = no_fence, bool use_khr = false);
    VkResult Submit2(const CommandBuffer &cmd, const Wait &wait, const Signal &signal, const Fence &fence = no_fence,
                     bool use_khr = false);

    VkResult Submit2(const CommandBuffer &cmd, const TimelineWait &wait, const Fence &fence = no_fence, bool use_khr = false);
    VkResult Submit2(const CommandBuffer &cmd, const TimelineSignal &signal, const Fence &fence = no_fence, bool use_khr = false);
    VkResult Submit2(const CommandBuffer &cmd, const TimelineWait &wait, const TimelineSignal &signal,
                     const Fence &fence = no_fence, bool use_khr = false);

    VkResult Submit2(const CommandBuffer &cmd, const Wait &wait, const TimelineSignal &signal, const Fence &fence = no_fence);
    VkResult Submit2(const CommandBuffer &cmd, const TimelineWait &wait, const Signal &signal, const Fence &fence = no_fence);

    // vkQueuePresentKHR()
    VkResult Present(const Swapchain &swapchain, uint32_t image_index, const Semaphore &wait_semaphore,
                     void *present_info_pnext = nullptr);

    // vkQueueWaitIdle()
    VkResult Wait();

    const uint32_t family_index;
};

class QueryPool : public internal::NonDispHandle<VkQueryPool> {
  public:
    QueryPool() = default;
    QueryPool(const Device &dev, const VkQueryPoolCreateInfo &info) { init(dev, info); }
    QueryPool(const Device &dev, VkQueryType query_type, uint32_t query_count) {
        VkQueryPoolCreateInfo info = CreateInfo(query_type, query_count);
        init(dev, info);
    }
    ~QueryPool() noexcept;
    void destroy() noexcept;

    // vkCreateQueryPool()
    void init(const Device &dev, const VkQueryPoolCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkQueryPool>::SetName(VK_OBJECT_TYPE_QUERY_POOL, name); }

    // vkGetQueryPoolResults()
    VkResult Results(uint32_t first, uint32_t count, size_t size, void *data, size_t stride);

    static VkQueryPoolCreateInfo CreateInfo(VkQueryType type, uint32_t slot_count);
};

struct NoMemT {};
static constexpr NoMemT no_mem{};
struct DeviceAddressT {};
static constexpr DeviceAddressT device_address{};
struct SetLayoutT {};
static constexpr SetLayoutT set_layout{};

class Buffer : public internal::NonDispHandle<VkBuffer> {
  public:
    explicit Buffer() : NonDispHandle(), create_info_(vku::InitStruct<decltype(create_info_)>()) {}
    explicit Buffer(const Device &dev, const VkBufferCreateInfo &info, VkMemoryPropertyFlags mem_props = 0,
                    void *alloc_info_pnext = nullptr) {
        init(dev, info, mem_props, alloc_info_pnext);
    }
    explicit Buffer(const Device &dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_props = 0,
                    void *alloc_info_pnext = nullptr) {
        init(dev, size, usage, mem_props, alloc_info_pnext);
    }
    explicit Buffer(const Device &dev, const VkBufferCreateInfo &info, NoMemT) { InitNoMemory(dev, info); }

    // Various spots need a host visible buffer they can call GetBufferDeviceAddress on
    explicit Buffer(const Device &dev, VkDeviceSize size, VkBufferUsageFlags usage, DeviceAddressT) {
        usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;  // always add
        VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
        allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        init(dev, CreateInfo(size, usage), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             &allocate_flag_info);
    }

    explicit Buffer(const Device &dev, VkBufferUsageFlags usage, const void *data, size_t data_size,
                    const vvl::span<uint32_t> &queue_families = {}) {
        InitHostVisibleWithData(dev, usage, data, data_size, queue_families);
    }

    Buffer(Buffer &&rhs) noexcept;
    Buffer &operator=(Buffer &&rhs) noexcept;
    ~Buffer() noexcept;
    void destroy() noexcept;

    // vkCreateBuffer()
    void init(const Device &dev, const VkBufferCreateInfo &info, VkMemoryPropertyFlags mem_props = 0,
              void *alloc_info_pnext = nullptr);
    void init(const Device &dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_props = 0,
              void *alloc_info_pnext = nullptr, const vvl::span<uint32_t> &queue_families = {}) {
        init(dev, CreateInfo(size, usage, queue_families), mem_props, alloc_info_pnext);
    }
    void InitHostVisibleWithData(const Device &dev, VkBufferUsageFlags usage, const void *data, size_t data_size,
                                 const vvl::span<uint32_t> &queue_families = {});
    void InitNoMemory(const Device &dev, const VkBufferCreateInfo &info);

    void SetName(const char *name) { NonDispHandle<VkBuffer>::SetName(VK_OBJECT_TYPE_BUFFER, name); }

    // get the internal memory
    const DeviceMemory &Memory() const { return internal_mem_; }
    DeviceMemory &Memory() { return internal_mem_; }

    // vkGetObjectMemoryRequirements()
    VkMemoryRequirements MemoryRequirements() const;

    // Allocate and bind memory
    // The assumption that this object was created in no_mem configuration
    void AllocateAndBindMemory(const Device &dev, VkMemoryPropertyFlags mem_props = 0, void *alloc_info_pnext = nullptr);

    // Bind to existing memory object
    void BindMemory(const DeviceMemory &mem, VkDeviceSize mem_offset);

    const VkBufferCreateInfo &CreateInfo() const { return create_info_; }
    static VkBufferCreateInfo CreateInfo(VkDeviceSize size, VkFlags usage, const vvl::span<uint32_t> &queue_families = {},
                                         void *create_info_pnext = nullptr);

    VkBufferMemoryBarrier BufferMemoryBarrier(VkFlags output_mask, VkFlags input_mask, VkDeviceSize offset,
                                              VkDeviceSize size) const {
        VkBufferMemoryBarrier barrier = vku::InitStructHelper();
        barrier.buffer = handle();
        barrier.srcAccessMask = output_mask;
        barrier.dstAccessMask = input_mask;
        barrier.offset = offset;
        barrier.size = size;
        if (create_info_.sharingMode == VK_SHARING_MODE_CONCURRENT) {
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }
        return barrier;
    }

    VkBufferMemoryBarrier2 BufferMemoryBarrier(VkPipelineStageFlags2KHR src_stage, VkPipelineStageFlags2KHR dst_stage,
                                               VkAccessFlags2KHR src_access, VkAccessFlags2KHR dst_access, VkDeviceSize offset,
                                               VkDeviceSize size) const {
        VkBufferMemoryBarrier2 barrier = vku::InitStructHelper();
        barrier.buffer = handle();
        barrier.srcStageMask = src_stage;
        barrier.dstStageMask = dst_stage;
        barrier.srcAccessMask = src_access;
        barrier.dstAccessMask = dst_access;
        barrier.offset = offset;
        barrier.size = size;
        if (create_info_.sharingMode == VK_SHARING_MODE_CONCURRENT) {
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }
        return barrier;
    }

    [[nodiscard]] VkDeviceAddress Address() const;

  private:
    VkBufferCreateInfo create_info_;
    DeviceMemory internal_mem_;
};

class BufferView : public internal::NonDispHandle<VkBufferView> {
  public:
    BufferView() = default;
    BufferView(const Device &dev, const VkBufferViewCreateInfo &info) { init(dev, info); }
    ~BufferView() noexcept;
    void destroy() noexcept;

    // vkCreateBufferView()
    void init(const Device &dev, const VkBufferViewCreateInfo &info);
    static VkBufferViewCreateInfo CreateInfo(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0,
                                             VkDeviceSize range = VK_WHOLE_SIZE);
    void SetName(const char *name) { NonDispHandle<VkBufferView>::SetName(VK_OBJECT_TYPE_BUFFER_VIEW, name); }
};

inline VkBufferViewCreateInfo BufferView::CreateInfo(VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range) {
    VkBufferViewCreateInfo info = vku::InitStructHelper();
    info.flags = VkFlags(0);
    info.buffer = buffer;
    info.format = format;
    info.offset = offset;
    info.range = range;
    return info;
}

class Image : public internal::NonDispHandle<VkImage> {
  public:
    explicit Image() : NonDispHandle() {}
    explicit Image(const Device &dev, const VkImageCreateInfo &info);
    explicit Image(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props,
                   void *alloc_info_pnext = nullptr);
    explicit Image(const Device &dev, uint32_t const width, uint32_t const height, uint32_t const mip_levels, VkFormat const format,
                   VkFlags const usage);

    explicit Image(const Device &dev, const VkImageCreateInfo &info, NoMemT);
    explicit Image(const Device &dev, const VkImageCreateInfo &info, SetLayoutT);
    Image(Image &&rhs) noexcept;
    Image &operator=(Image &&rhs) noexcept;

    ~Image() noexcept;
    void destroy() noexcept;

    void init(const Device &dev, const VkImageCreateInfo &info, VkMemoryPropertyFlags mem_props, void *alloc_info_pnext = nullptr);
    void Init(const Device &dev, uint32_t const width, uint32_t const height, uint32_t const mip_levels, VkFormat const format,
              VkFlags const usage);
    void InitNoMemory(const Device &dev, const VkImageCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkImage>::SetName(VK_OBJECT_TYPE_IMAGE, name); }

    static VkImageCreateInfo ImageCreateInfo2D(uint32_t const width, uint32_t const height, uint32_t const mip_levels,
                                               uint32_t const layers, VkFormat const format, VkFlags const usage,
                                               VkImageTiling const requested_tiling = VK_IMAGE_TILING_OPTIMAL,
                                               const vvl::span<uint32_t> &queue_families = {});

    static bool IsCompatible(const Device &dev, VkImageUsageFlags usages, VkFormatFeatureFlags2 features);

    // get the internal memory
    const DeviceMemory &Memory() const { return internal_mem_; }
    DeviceMemory &Memory() { return internal_mem_; }

    // vkGetObjectMemoryRequirements()
    VkMemoryRequirements MemoryRequirements() const;

    // Allocate and bind memory
    // The assumption that this object was created in no_mem configuration
    void AllocateAndBindMemory(const Device &dev, VkMemoryPropertyFlags mem_props = 0, void *alloc_info_pnext = nullptr);

    // Bind to existing memory object
    void BindMemory(const DeviceMemory &mem, VkDeviceSize mem_offset);

    uint32_t Width() const { return create_info_.extent.width; }
    uint32_t Height() const { return create_info_.extent.height; }
    VkFormat Format() const { return create_info_.format; }
    VkImageUsageFlags Usage() const { return create_info_.usage; }

    VkImageMemoryBarrier ImageMemoryBarrier(VkFlags output_mask, VkFlags input_mask, VkImageLayout old_layout,
                                            VkImageLayout new_layout, const VkImageSubresourceRange &range) const {
        VkImageMemoryBarrier barrier = vku::InitStructHelper();
        barrier.srcAccessMask = output_mask;
        barrier.dstAccessMask = input_mask;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.image = handle();
        barrier.subresourceRange = range;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return barrier;
    }

    VkImageMemoryBarrier2 ImageMemoryBarrier(VkPipelineStageFlags2KHR src_stage, VkPipelineStageFlags2KHR dst_stage,
                                             VkAccessFlags2KHR src_access, VkAccessFlags2KHR dst_access, VkImageLayout old_layout,
                                             VkImageLayout new_layout, const VkImageSubresourceRange &range) const {
        VkImageMemoryBarrier2 barrier = vku::InitStructHelper();
        barrier.srcStageMask = src_stage;
        barrier.dstStageMask = dst_stage;
        barrier.srcAccessMask = src_access;
        barrier.dstAccessMask = dst_access;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.image = handle();
        barrier.subresourceRange = range;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return barrier;
    }

    void ImageMemoryBarrier(CommandBuffer &cmd, VkImageAspectFlags aspect, VkFlags output_mask, VkFlags input_mask,
                            VkImageLayout image_layout, VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    static VkImageCreateInfo CreateInfo();

    static VkImageSubresource Subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer);
    static VkImageSubresource Subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer);
    static VkImageSubresourceLayers Subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer,
                                                uint32_t array_size);
    static VkImageSubresourceLayers Subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer,
                                                uint32_t array_size);

    VkImageSubresourceRange SubresourceRange(VkImageAspectFlags aspect) const { return SubresourceRange(create_info_, aspect); }
    static VkImageSubresourceRange SubresourceRange(VkImageAspectFlags aspect_mask, uint32_t base_mip_level, uint32_t mip_levels,
                                                    uint32_t base_array_layer, uint32_t num_layers);
    static VkImageSubresourceRange SubresourceRange(const VkImageCreateInfo &info, VkImageAspectFlags aspect_mask);
    static VkImageSubresourceRange SubresourceRange(const VkImageSubresource &subres);

    static VkImageAspectFlags AspectMask(VkFormat format);

    void Layout(VkImageLayout const layout) { image_layout_ = layout; }
    VkImageLayout Layout() const { return image_layout_; }

    void SetLayout(CommandBuffer &cmd_buf, VkImageAspectFlags aspect, VkImageLayout image_layout);
    void SetLayout(VkImageAspectFlags aspect, VkImageLayout image_layout);
    void SetLayout(VkImageLayout image_layout) { SetLayout(AspectMask(Format()), image_layout); };

    VkImageViewCreateInfo BasicViewCreatInfo(VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) const;
    ImageView CreateView(VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, void *pNext = nullptr) const;
    ImageView CreateView(VkImageViewType type, uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
                         uint32_t baseArrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS,
                         VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT) const;

  private:
    // We need this to do ImageView and SetLayout actions
    const Device *device_ = nullptr;

    VkImageCreateInfo create_info_;

    DeviceMemory internal_mem_;
    VkImageLayout image_layout_ = VK_IMAGE_LAYOUT_GENERAL;
};

class ImageView : public internal::NonDispHandle<VkImageView> {
  public:
    explicit ImageView() = default;
    explicit ImageView(const Device &dev, const VkImageViewCreateInfo &info) { init(dev, info); }
    ImageView(ImageView &&rhs) noexcept : NonDispHandle(std::move(rhs)) {}
    ImageView &operator=(ImageView &&src) noexcept {
        this->~ImageView();
        this->NonDispHandle::operator=(std::move(src));
        return *this;
    }
    ~ImageView() noexcept;
    void destroy() noexcept;

    // vkCreateImageView()
    void init(const Device &dev, const VkImageViewCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkImageView>::SetName(VK_OBJECT_TYPE_IMAGE_VIEW, name); }
};

class AccelerationStructureNV : public internal::NonDispHandle<VkAccelerationStructureNV> {
  public:
    explicit AccelerationStructureNV(const Device &dev, const VkAccelerationStructureCreateInfoNV &info, bool init_memory = true) {
        init(dev, info, init_memory);
    }
    ~AccelerationStructureNV() noexcept;
    void destroy() noexcept;

    // vkCreateAccelerationStructureNV
    void init(const Device &dev, const VkAccelerationStructureCreateInfoNV &info, bool init_memory = true);
    void SetName(const char *name) {
        NonDispHandle<VkAccelerationStructureNV>::SetName(VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV, name);
    }
    // vkGetAccelerationStructureMemoryRequirementsNV()
    VkMemoryRequirements2 MemoryRequirements() const;
    VkMemoryRequirements2 BuildScratchMemoryRequirements() const;

    uint64_t OpaqueHandle() const { return opaque_handle_; }

    const VkAccelerationStructureInfoNV &Info() const { return info_; }

    [[nodiscard]] Buffer CreateScratchBuffer(const Device &device, VkBufferCreateInfo *pCreateInfo = nullptr,
                                             bool buffer_device_address = false) const;

  private:
    VkAccelerationStructureInfoNV info_;
    DeviceMemory memory_;
    uint64_t opaque_handle_;
};

class ShaderModule : public internal::NonDispHandle<VkShaderModule> {
  public:
    ShaderModule() = default;
    ShaderModule(const Device &dev, const VkShaderModuleCreateInfo &info) { init(dev, info); }
    ~ShaderModule() noexcept;
    void destroy() noexcept;

    // vkCreateShaderModule()
    void init(const Device &dev, const VkShaderModuleCreateInfo &info);
    VkResult InitTry(const Device &dev, const VkShaderModuleCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkShaderModule>::SetName(VK_OBJECT_TYPE_SHADER_MODULE, name); }

    static VkShaderModuleCreateInfo CreateInfo(size_t code_size, const uint32_t *code, VkFlags flags);
};

class Shader : public internal::NonDispHandle<VkShaderEXT> {
  public:
    Shader() = default;
    Shader(const Device &dev, const VkShaderCreateInfoEXT &info) { init(dev, info); }
    Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint32_t> &spv,
           const VkDescriptorSetLayout *descriptorSetLayout = nullptr, const VkPushConstantRange *pushConstRange = nullptr);
    Shader(const Device &dev, const VkShaderStageFlagBits stage, const std::vector<uint8_t> &binary,
           const VkDescriptorSetLayout *descriptorSetLayout = nullptr, const VkPushConstantRange *pushConstRange = nullptr);
    ~Shader() noexcept;
    void destroy() noexcept;

    // vkCreateShaderModule()
    void init(const Device &dev, const VkShaderCreateInfoEXT &info);
    VkResult InitTry(const Device &dev, const VkShaderCreateInfoEXT &info);
    void SetName(const char *name) { NonDispHandle<VkShaderEXT>::SetName(VK_OBJECT_TYPE_SHADER_EXT, name); }
};

class PipelineCache : public internal::NonDispHandle<VkPipelineCache> {
  public:
    PipelineCache() = default;
    PipelineCache(const Device &dev, const VkPipelineCacheCreateInfo &info) { init(dev, info); }
    ~PipelineCache() noexcept;
    void destroy() noexcept;

    void init(const Device &dev, const VkPipelineCacheCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkPipelineCache>::SetName(VK_OBJECT_TYPE_PIPELINE_CACHE, name); }
};

class Pipeline : public internal::NonDispHandle<VkPipeline> {
  public:
    Pipeline() = default;
    Pipeline(const Device &dev, const VkGraphicsPipelineCreateInfo &info) { init(dev, info); }
    Pipeline(const Device &dev, const VkGraphicsPipelineCreateInfo &info, const VkPipeline basePipeline) {
        init(dev, info, basePipeline);
    }
    Pipeline(const Device &dev, const VkComputePipelineCreateInfo &info) { init(dev, info); }
    Pipeline(const Device &dev, const VkRayTracingPipelineCreateInfoKHR &info) { init(dev, info); }
    ~Pipeline() noexcept;
    void destroy() noexcept;

    // vkCreateGraphicsPipeline()
    void init(const Device &dev, const VkGraphicsPipelineCreateInfo &info);
    // vkCreateGraphicsPipelineDerivative()
    void init(const Device &dev, const VkGraphicsPipelineCreateInfo &info, const VkPipeline basePipeline);
    // vkCreateComputePipeline()
    void init(const Device &dev, const VkComputePipelineCreateInfo &info);
    // vkCreateRayTracingPipelinesKHR
    void init(const Device &dev, const VkRayTracingPipelineCreateInfoKHR &info);
    // vkCreateRayTracingPipelinesKHR with deferredOperation
    void InitDeferred(const Device &dev, const VkRayTracingPipelineCreateInfoKHR &info, VkDeferredOperationKHR deferred_op);
    // vkLoadPipeline()
    void init(const Device &dev, size_t size, const void *data);
    // vkLoadPipelineDerivative()
    void init(const Device &dev, size_t size, const void *data, VkPipeline basePipeline);

    // vkCreateGraphicsPipeline with error return
    VkResult InitTry(const Device &dev, const VkGraphicsPipelineCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkPipeline>::SetName(VK_OBJECT_TYPE_PIPELINE, name); }
};

class PipelineLayout : public internal::NonDispHandle<VkPipelineLayout> {
  public:
    PipelineLayout() noexcept : NonDispHandle() {}
    PipelineLayout(const Device &dev, VkPipelineLayoutCreateInfo &info, const std::vector<const DescriptorSetLayout *> &layouts) {
        init(dev, info, layouts);
    }
    PipelineLayout(const Device &dev, VkPipelineLayoutCreateInfo &info) { init(dev, info); }
    PipelineLayout(const Device &dev, const std::vector<const DescriptorSetLayout *> &layouts = {},
                   const std::vector<VkPushConstantRange> &push_constant_ranges = {},
                   VkPipelineLayoutCreateFlags flags = static_cast<VkPipelineLayoutCreateFlags>(0)) {
        VkPipelineLayoutCreateInfo info = vku::InitStructHelper();
        info.flags = flags;
        info.pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size());
        info.pPushConstantRanges = push_constant_ranges.data();

        init(dev, info, layouts);
    }
    ~PipelineLayout() noexcept;
    void destroy() noexcept;

    // Move constructor for Visual Studio 2013
    PipelineLayout(PipelineLayout &&src) noexcept : NonDispHandle(std::move(src)){};

    PipelineLayout &operator=(PipelineLayout &&src) noexcept {
        this->~PipelineLayout();
        this->NonDispHandle::operator=(std::move(src));
        return *this;
    };

    // vCreatePipelineLayout()
    void init(const Device &dev, VkPipelineLayoutCreateInfo &info, const std::vector<const DescriptorSetLayout *> &layouts);
    void init(const Device &dev, VkPipelineLayoutCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkPipelineLayout>::SetName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, name); }
};

class Sampler : public internal::NonDispHandle<VkSampler> {
  public:
    Sampler() = default;
    Sampler(const Device &dev, const VkSamplerCreateInfo &info) { init(dev, info); }
    ~Sampler() noexcept;
    void destroy() noexcept;

    // vkCreateSampler()
    void init(const Device &dev, const VkSamplerCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkSampler>::SetName(VK_OBJECT_TYPE_SAMPLER, name); }
};

class DescriptorSetLayout : public internal::NonDispHandle<VkDescriptorSetLayout> {
  public:
    DescriptorSetLayout() noexcept : NonDispHandle(){};
    DescriptorSetLayout(const Device &dev, const VkDescriptorSetLayoutCreateInfo &info) { init(dev, info); }
    DescriptorSetLayout(const Device &dev, const std::vector<VkDescriptorSetLayoutBinding> &descriptor_set_bindings = {},
                        VkDescriptorSetLayoutCreateFlags flags = 0, void *pNext = nullptr) {
        VkDescriptorSetLayoutCreateInfo info = vku::InitStructHelper(pNext);
        info.flags = flags;
        info.bindingCount = static_cast<uint32_t>(descriptor_set_bindings.size());
        info.pBindings = descriptor_set_bindings.data();
        init(dev, info);
    }
    DescriptorSetLayout(const Device &dev, const VkDescriptorSetLayoutBinding &descriptor_set_binding,
                        VkDescriptorSetLayoutCreateFlags flags = 0, void *pNext = nullptr) {
        VkDescriptorSetLayoutCreateInfo info = vku::InitStructHelper(pNext);
        info.flags = flags;
        info.bindingCount = 1;
        info.pBindings = &descriptor_set_binding;
        init(dev, info);
    }

    ~DescriptorSetLayout() noexcept;
    void destroy() noexcept;

    // Move constructor for Visual Studio 2013
    DescriptorSetLayout(DescriptorSetLayout &&src) noexcept : NonDispHandle(std::move(src)){};

    DescriptorSetLayout &operator=(DescriptorSetLayout &&src) noexcept {
        this->~DescriptorSetLayout();
        this->NonDispHandle::operator=(std::move(src));
        return *this;
    }

    // vkCreateDescriptorSetLayout()
    void init(const Device &dev, const VkDescriptorSetLayoutCreateInfo &info);
};

class DescriptorPool : public internal::NonDispHandle<VkDescriptorPool> {
  public:
    DescriptorPool() = default;
    DescriptorPool(const Device &dev, const VkDescriptorPoolCreateInfo &info) { init(dev, info); }
    ~DescriptorPool() noexcept;
    void destroy() noexcept;

    // vkCreateDescriptorPool()
    void init(const Device &dev, const VkDescriptorPoolCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkDescriptorPool>::SetName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, name); }

    // vkResetDescriptorPool()
    void Reset();

    // vkFreeDescriptorSet()
    bool GetDynamicUsage() { return dynamic_usage_; }

    // vkAllocateDescriptorSets()
    std::vector<DescriptorSet *> AllocateSets(const Device &dev, const std::vector<const DescriptorSetLayout *> &layouts);
    std::vector<DescriptorSet *> AllocateSets(const Device &dev, const DescriptorSetLayout &layout, uint32_t count);
    DescriptorSet *AllocateSets(const Device &dev, const DescriptorSetLayout &layout);

    template <typename PoolSizes>
    static VkDescriptorPoolCreateInfo CreateInfo(VkDescriptorPoolCreateFlags flags, uint32_t max_sets, const PoolSizes &pool_sizes);

  private:
    // Track whether this pool's usage is VK_DESCRIPTOR_POOL_USAGE_DYNAMIC
    bool dynamic_usage_;
};

template <typename PoolSizes>
inline VkDescriptorPoolCreateInfo DescriptorPool::CreateInfo(VkDescriptorPoolCreateFlags flags, uint32_t max_sets,
                                                             const PoolSizes &pool_sizes) {
    VkDescriptorPoolCreateInfo info = vku::InitStructHelper();
    info.flags = flags;
    info.maxSets = max_sets;
    info.poolSizeCount = pool_sizes.size();
    info.pPoolSizes = (info.poolSizeCount) ? pool_sizes.data() : nullptr;
    return info;
}

class DescriptorSet : public internal::NonDispHandle<VkDescriptorSet> {
  public:
    ~DescriptorSet() noexcept;
    void destroy() noexcept;

    explicit DescriptorSet() : NonDispHandle() {}
    explicit DescriptorSet(const Device &dev, DescriptorPool *pool, VkDescriptorSet set) : NonDispHandle(dev.handle(), set) {
        containing_pool_ = pool;
    }
    void SetName(const char *name) { NonDispHandle<VkDescriptorSet>::SetName(VK_OBJECT_TYPE_DESCRIPTOR_SET, name); }

  private:
    DescriptorPool *containing_pool_;
};

class DescriptorUpdateTemplate : public internal::NonDispHandle<VkDescriptorUpdateTemplate> {
  public:
    ~DescriptorUpdateTemplate() noexcept;
    void destroy() noexcept;

    explicit DescriptorUpdateTemplate() : NonDispHandle() {}
    explicit DescriptorUpdateTemplate(const Device &dev, const VkDescriptorUpdateTemplateCreateInfo &info) { Init(dev, info); }
    void Init(const Device &dev, const VkDescriptorUpdateTemplateCreateInfo &info);
    void SetName(const char *name) {
        NonDispHandle<VkDescriptorUpdateTemplate>::SetName(VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE, name);
    }
};

class CommandPool : public internal::NonDispHandle<VkCommandPool> {
  public:
    ~CommandPool() noexcept;
    void destroy() noexcept;

    explicit CommandPool() : NonDispHandle() {}
    explicit CommandPool(const Device &dev, const VkCommandPoolCreateInfo &info) { Init(dev, info); }
    explicit CommandPool(const Device &dev, uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
    void Init(const Device &dev, const VkCommandPoolCreateInfo &info);
    void Init(const Device &dev, uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
    void SetName(const char *name) { NonDispHandle<VkCommandPool>::SetName(VK_OBJECT_TYPE_COMMAND_POOL, name); }
};

class CommandBuffer : public internal::Handle<VkCommandBuffer> {
  public:
    ~CommandBuffer() noexcept;
    void destroy() noexcept;

    explicit CommandBuffer() : Handle() {}
    explicit CommandBuffer(const Device &dev, const VkCommandBufferAllocateInfo &info) { init(dev, info); }
    explicit CommandBuffer(const Device &dev, const CommandPool &pool,
                           VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        Init(dev, pool, level);
    }
    CommandBuffer(CommandBuffer &&rhs) noexcept : Handle(std::move(rhs)) {
        dev_handle_ = rhs.dev_handle_;
        rhs.dev_handle_ = VK_NULL_HANDLE;
        cmd_pool_ = rhs.cmd_pool_;
        rhs.cmd_pool_ = VK_NULL_HANDLE;
    }

    // vkAllocateCommandBuffers()
    void init(const Device &dev, const VkCommandBufferAllocateInfo &info);
    void Init(const Device &dev, const CommandPool &pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void SetName(const Device &device, const char *name) {
        Handle<VkCommandBuffer>::SetName(device.handle(), VK_OBJECT_TYPE_COMMAND_BUFFER, name);
    }

    // vkBeginCommandBuffer()
    void Begin(const VkCommandBufferBeginInfo *info);
    void Begin(VkCommandBufferUsageFlags flags);
    void Begin() { Begin(0u); }

    // vkEndCommandBuffer()
    // vkResetCommandBuffer()
    void End();
    void Reset(VkCommandBufferResetFlags flags);
    void Reset() { Reset(VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); }

    static VkCommandBufferAllocateInfo CreateInfo(VkCommandPool const &pool);

    void BeginRenderPass(const VkRenderPassBeginInfo &info, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void BeginRenderPass(VkRenderPass rp, VkFramebuffer fb, uint32_t render_area_width = 1, uint32_t render_area_height = 1,
                         uint32_t clear_count = 0, VkClearValue *clear_values = nullptr);
    void NextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void EndRenderPass();
    void BeginRendering(const VkRenderingInfo &renderingInfo);
    void BeginRenderingColor(const VkImageView imageView, VkRect2D render_area);
    void EndRendering();

    void BindVertFragShader(const vkt::Shader &vert_shader, const vkt::Shader &frag_shader);
    void BindCompShader(const vkt::Shader &comp_shader);

    void BeginVideoCoding(const VkVideoBeginCodingInfoKHR &beginInfo);
    void ControlVideoCoding(const VkVideoCodingControlInfoKHR &controlInfo);
    void DecodeVideo(const VkVideoDecodeInfoKHR &decodeInfo);
    void EncodeVideo(const VkVideoEncodeInfoKHR &encodeInfo);
    void EndVideoCoding(const VkVideoEndCodingInfoKHR &endInfo);

    void SetEvent(Event &event, VkPipelineStageFlags stageMask) { event.CmdSet(*this, stageMask); }
    void ResetEvent(Event &event, VkPipelineStageFlags stageMask) { event.CmdReset(*this, stageMask); }
    void WaitEvents(uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                    VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
        vk::CmdWaitEvents(handle(), eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                          bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }

    void Copy(const Buffer &src, const Buffer &dst);
    void ExecuteCommands(const CommandBuffer &secondary);

  private:
    VkDevice dev_handle_;
    VkCommandPool cmd_pool_;
};

// shortcut to vkt::CommandBuffer{}. Useful in submit helpers: queue->submit(vkt::no_cmd, ...)
inline const CommandBuffer no_cmd;

class RenderPass : public internal::NonDispHandle<VkRenderPass> {
  public:
    RenderPass() = default;
    // vkCreateRenderPass()
    RenderPass(const Device &dev, const VkRenderPassCreateInfo &info) { init(dev, info); }
    // vkCreateRenderPass2()
    RenderPass(const Device &dev, const VkRenderPassCreateInfo2 &info) { init(dev, info); }
    ~RenderPass() noexcept;
    void destroy() noexcept;

    // vkCreateRenderPass()
    void init(const Device &dev, const VkRenderPassCreateInfo &info);
    // vkCreateRenderPass2()
    void init(const Device &dev, const VkRenderPassCreateInfo2 &info);
    void SetName(const char *name) { NonDispHandle<VkRenderPass>::SetName(VK_OBJECT_TYPE_RENDER_PASS, name); }
};

class Framebuffer : public internal::NonDispHandle<VkFramebuffer> {
  public:
    Framebuffer() = default;
    Framebuffer(const Device &dev, const VkFramebufferCreateInfo &info) { init(dev, info); }
    // The most common case, anything outside of this should create there own VkFramebufferCreateInfo
    Framebuffer(const Device &dev, VkRenderPass rp, uint32_t attchment_count, const VkImageView *attchments, uint32_t width = 32,
                uint32_t height = 32) {
        VkFramebufferCreateInfo info = vku::InitStructHelper();
        info.renderPass = rp;
        info.attachmentCount = attchment_count;
        info.pAttachments = attchments;
        info.width = width;
        info.height = height;
        info.layers = 1;
        init(dev, info);
    }
    ~Framebuffer() noexcept;
    void destroy() noexcept;

    // vkCreateFramebuffer()
    void init(const Device &dev, const VkFramebufferCreateInfo &info);
    void SetName(const char *name) { NonDispHandle<VkFramebuffer>::SetName(VK_OBJECT_TYPE_FRAMEBUFFER, name); }
};

class SamplerYcbcrConversion : public internal::NonDispHandle<VkSamplerYcbcrConversion> {
  public:
    SamplerYcbcrConversion() = default;
    SamplerYcbcrConversion(const Device &dev, VkFormat format) { init(dev, DefaultConversionInfo(format)); }
    SamplerYcbcrConversion(const Device &dev, const VkSamplerYcbcrConversionCreateInfo &info) { init(dev, info); }
    ~SamplerYcbcrConversion() noexcept;
    void destroy() noexcept;

    void init(const Device &dev, const VkSamplerYcbcrConversionCreateInfo &info);
    void SetName(const char *name) {
        NonDispHandle<VkSamplerYcbcrConversion>::SetName(VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION, name);
    }
    VkSamplerYcbcrConversionInfo ConversionInfo();

    static VkSamplerYcbcrConversionCreateInfo DefaultConversionInfo(VkFormat format);
};

inline VkBufferCreateInfo Buffer::CreateInfo(VkDeviceSize size, VkFlags usage, const vvl::span<uint32_t> &queue_families,
                                             void *create_info_pnext) {
    VkBufferCreateInfo info = vku::InitStructHelper(create_info_pnext);
    info.size = size;
    info.usage = usage;

    if (queue_families.size() > 1) {
        info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = static_cast<uint32_t>(queue_families.size());
        info.pQueueFamilyIndices = queue_families.data();
    }

    return info;
}

inline VkEventCreateInfo Event::CreateInfo(VkFlags flags) {
    VkEventCreateInfo info = vku::InitStructHelper();
    info.flags = flags;
    return info;
}

inline VkQueryPoolCreateInfo QueryPool::CreateInfo(VkQueryType type, uint32_t slot_count) {
    VkQueryPoolCreateInfo info = vku::InitStructHelper();
    info.queryType = type;
    info.queryCount = slot_count;
    return info;
}

inline VkImageCreateInfo Image::CreateInfo() {
    VkImageCreateInfo info = vku::InitStructHelper();
    info.extent.width = 1;
    info.extent.height = 1;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    return info;
}

inline VkImageSubresource Image::Subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer) {
    VkImageSubresource subres = {};
    if (aspect == 0) {
        assert(false && "Invalid VkImageAspectFlags");
    }
    subres.aspectMask = aspect;
    subres.mipLevel = mip_level;
    subres.arrayLayer = array_layer;
    return subres;
}

inline VkImageSubresource Image::Subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer) {
    return Subresource(range.aspectMask, range.baseMipLevel + mip_level, range.baseArrayLayer + array_layer);
}

inline VkImageSubresourceLayers Image::Subresource(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer,
                                                   uint32_t array_size) {
    VkImageSubresourceLayers subres = {};
    switch (aspect) {
        case VK_IMAGE_ASPECT_COLOR_BIT:
        case VK_IMAGE_ASPECT_DEPTH_BIT:
        case VK_IMAGE_ASPECT_STENCIL_BIT:
        case VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT:
            /* valid */
            break;
        default:
            assert(false && "Invalid VkImageAspectFlags");
    }
    subres.aspectMask = aspect;
    subres.mipLevel = mip_level;
    subres.baseArrayLayer = array_layer;
    subres.layerCount = array_size;
    return subres;
}

inline VkImageSubresourceLayers Image::Subresource(const VkImageSubresourceRange &range, uint32_t mip_level, uint32_t array_layer,
                                                   uint32_t array_size) {
    return Subresource(range.aspectMask, range.baseMipLevel + mip_level, range.baseArrayLayer + array_layer, array_size);
}

inline VkImageSubresourceRange Image::SubresourceRange(VkImageAspectFlags aspect_mask, uint32_t base_mip_level, uint32_t mip_levels,
                                                       uint32_t base_array_layer, uint32_t num_layers) {
    VkImageSubresourceRange range = {};
    if (aspect_mask == 0) {
        assert(false && "Invalid VkImageAspectFlags");
    }
    range.aspectMask = aspect_mask;
    range.baseMipLevel = base_mip_level;
    range.levelCount = mip_levels;
    range.baseArrayLayer = base_array_layer;
    range.layerCount = num_layers;
    return range;
}

inline VkImageSubresourceRange Image::SubresourceRange(const VkImageCreateInfo &info, VkImageAspectFlags aspect_mask) {
    return SubresourceRange(aspect_mask, 0, info.mipLevels, 0, info.arrayLayers);
}

inline VkImageSubresourceRange Image::SubresourceRange(const VkImageSubresource &subres) {
    return SubresourceRange(subres.aspectMask, subres.mipLevel, 1, subres.arrayLayer, 1);
}

inline VkShaderModuleCreateInfo ShaderModule::CreateInfo(size_t code_size, const uint32_t *code, VkFlags flags) {
    VkShaderModuleCreateInfo info = vku::InitStructHelper();
    info.codeSize = code_size;
    info.pCode = code;
    info.flags = flags;
    return info;
}

inline VkWriteDescriptorSet Device::WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                       VkDescriptorType type, uint32_t count,
                                                       const VkDescriptorImageInfo *image_info) {
    VkWriteDescriptorSet write = vku::InitStructHelper();
    write.dstSet = set.handle();
    write.dstBinding = binding;
    write.dstArrayElement = array_element;
    write.descriptorCount = count;
    write.descriptorType = type;
    write.pImageInfo = image_info;
    return write;
}

inline VkWriteDescriptorSet Device::WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                       VkDescriptorType type, uint32_t count,
                                                       const VkDescriptorBufferInfo *buffer_info) {
    VkWriteDescriptorSet write = vku::InitStructHelper();
    write.dstSet = set.handle();
    write.dstBinding = binding;
    write.dstArrayElement = array_element;
    write.descriptorCount = count;
    write.descriptorType = type;
    write.pBufferInfo = buffer_info;
    return write;
}

inline VkWriteDescriptorSet Device::WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                       VkDescriptorType type, uint32_t count, const VkBufferView *buffer_views) {
    VkWriteDescriptorSet write = vku::InitStructHelper();
    write.dstSet = set.handle();
    write.dstBinding = binding;
    write.dstArrayElement = array_element;
    write.descriptorCount = count;
    write.descriptorType = type;
    write.pTexelBufferView = buffer_views;
    return write;
}

inline VkWriteDescriptorSet Device::WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                       VkDescriptorType type,
                                                       const std::vector<VkDescriptorImageInfo> &image_info) {
    return WriteDescriptorSet(set, binding, array_element, type, static_cast<uint32_t>(image_info.size()), &image_info[0]);
}

inline VkWriteDescriptorSet Device::WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                       VkDescriptorType type,
                                                       const std::vector<VkDescriptorBufferInfo> &buffer_info) {
    return WriteDescriptorSet(set, binding, array_element, type, static_cast<uint32_t>(buffer_info.size()), &buffer_info[0]);
}

inline VkWriteDescriptorSet Device::WriteDescriptorSet(const DescriptorSet &set, uint32_t binding, uint32_t array_element,
                                                       VkDescriptorType type, const std::vector<VkBufferView> &buffer_views) {
    return WriteDescriptorSet(set, binding, array_element, type, static_cast<uint32_t>(buffer_views.size()), &buffer_views[0]);
}

class Swapchain : public internal::NonDispHandle<VkSwapchainKHR> {
  public:
    explicit Swapchain() = default;
    explicit Swapchain(const Device &dev, const VkSwapchainCreateInfoKHR &info) { Init(dev, info); }
    Swapchain(Swapchain &&rhs) noexcept : NonDispHandle(std::move(rhs)) {}
    Swapchain &operator=(Swapchain &&) = default;
    ~Swapchain() noexcept;
    void destroy() noexcept;

    void Init(const Device &dev, const VkSwapchainCreateInfoKHR &info);
    void SetName(const char *name) { NonDispHandle<VkSwapchainKHR>::SetName(VK_OBJECT_TYPE_SWAPCHAIN_KHR, name); }

    uint32_t GetImageCount() const;
    std::vector<VkImage> GetImages() const;

    uint32_t AcquireNextImage(const Semaphore &image_acquired, uint64_t timeout, VkResult *result = nullptr);
    uint32_t AcquireNextImage(const Fence &image_acquired, uint64_t timeout, VkResult *result = nullptr);
};

class IndirectCommandsLayout : public internal::NonDispHandle<VkIndirectCommandsLayoutEXT> {
  public:
    ~IndirectCommandsLayout() noexcept;
    void destroy() noexcept;

    explicit IndirectCommandsLayout() : NonDispHandle() {}
    explicit IndirectCommandsLayout(const Device &dev, const VkIndirectCommandsLayoutCreateInfoEXT &info) { Init(dev, info); }
    void Init(const Device &dev, const VkIndirectCommandsLayoutCreateInfoEXT &info);
};

class IndirectExecutionSet : public internal::NonDispHandle<VkIndirectExecutionSetEXT> {
  public:
    ~IndirectExecutionSet() noexcept;
    void destroy() noexcept;

    explicit IndirectExecutionSet() : NonDispHandle() {}
    explicit IndirectExecutionSet(const Device &dev, const VkIndirectExecutionSetCreateInfoEXT &info) { Init(dev, info); }
    explicit IndirectExecutionSet(const Device &dev, VkPipeline init_pipeline, uint32_t max_pipelines);
    explicit IndirectExecutionSet(const Device &dev, const VkIndirectExecutionSetShaderInfoEXT &shader_info);
    void Init(const Device &dev, const VkIndirectExecutionSetCreateInfoEXT &info);
};

class Surface {
  public:
    Surface() : instance_(VK_NULL_HANDLE), handle_(VK_NULL_HANDLE) {}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkResult Init(VkInstance, const VkWin32SurfaceCreateInfoKHR &);
#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    VkResult Init(VkInstance, const VkMetalSurfaceCreateInfoEXT &);
#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkResult Init(VkInstance, const VkAndroidSurfaceCreateInfoKHR &);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    VkResult Init(VkInstance, const VkXlibSurfaceCreateInfoKHR &);
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    VkResult Init(VkInstance, const VkXcbSurfaceCreateInfoKHR &);
#endif

    ~Surface() noexcept { Destroy(); }
    void Destroy() noexcept {
        if (handle_ != VK_NULL_HANDLE) {
            vk::DestroySurfaceKHR(instance_, handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
    VkSurfaceKHR Handle() const { return handle_; }

    Surface(Surface &&src) noexcept : instance_{src.instance_}, handle_{src.handle_} {
        src.instance_ = {};
        src.handle_ = {};
    }
    Surface &operator=(Surface &&src) noexcept {
        instance_ = src.instance_;
        src.instance_ = {};
        handle_ = src.handle_;
        src.handle_ = {};
        return *this;
    }

    // This is ONLY for tests that need a way test destroying an instance and leak the Surface object (and calling
    // vkDestroySurfaceKHR will be invalid)
    void DestroyExplicitly() {
        handle_ = VK_NULL_HANDLE;
        instance_ = VK_NULL_HANDLE;
    }

  private:
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR handle_ = VK_NULL_HANDLE;
};
}  // namespace vkt
