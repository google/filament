/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
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

#include <atomic>
#include <chrono>
#include <shared_mutex>
#include <string>
#include <thread>
#include "chassis/validation_object.h"
#include "utils/vk_layer_utils.h"

namespace threadsafety {

VK_DEFINE_NON_DISPATCHABLE_HANDLE(DISTINCT_NONDISPATCHABLE_PHONY_HANDLE)
// The following line must match the vulkan_core.h condition guarding VK_DEFINE_NON_DISPATCHABLE_HANDLE
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || \
    defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
// If pointers are 64-bit, then there can be separate counters for each
// NONDISPATCHABLE_HANDLE type.  Otherwise they are all typedef uint64_t.
#define DISTINCT_NONDISPATCHABLE_HANDLES
// Make sure we catch any disagreement between us and the vulkan definition
static_assert(std::is_pointer<DISTINCT_NONDISPATCHABLE_PHONY_HANDLE>::value,
              "Mismatched non-dispatchable handle handle, expected pointer type.");
#else
// Make sure we catch any disagreement between us and the vulkan definition
static_assert(std::is_same<uint64_t, DISTINCT_NONDISPATCHABLE_PHONY_HANDLE>::value,
              "Mismatched non-dispatchable handle handle, expected uint64_t.");
#endif

// Modern CPUs have 64 or 128-byte cache line sizes (Apple M1 has 128-byte cache line size).
// Use alignment of 64 bytes (instead of 128) to prioritize using less memory and decrease
// cache pressure.
inline constexpr size_t kObjectUserDataAlignment = 64;
static_assert(vku::concurrent::get_hardware_destructive_interference_size() % kObjectUserDataAlignment ==
              0);  // sanity check on the build machine

class alignas(kObjectUserDataAlignment) ObjectUseData {
  public:
    class WriteReadCount {
      public:
        explicit WriteReadCount(int64_t v) : count(v) {}

        int32_t GetReadCount() const { return static_cast<int32_t>(count & 0xFFFFFFFF); }
        int32_t GetWriteCount() const { return static_cast<int32_t>(count >> 32); }

      private:
        int64_t count{};
    };

    WriteReadCount AddWriter() {
        int64_t prev = writer_reader_count.fetch_add(1ULL << 32);
        return WriteReadCount(prev);
    }
    WriteReadCount AddReader() {
        int64_t prev = writer_reader_count.fetch_add(1ULL);
        return WriteReadCount(prev);
    }
    WriteReadCount RemoveWriter() {
        int64_t prev = writer_reader_count.fetch_add(-(1LL << 32));
        assert(prev > 0);
        return WriteReadCount(prev);
    }
    WriteReadCount RemoveReader() {
        int64_t prev = writer_reader_count.fetch_add(-1LL);
        assert(prev > 0);
        return WriteReadCount(prev);
    }
    WriteReadCount GetCount() { return WriteReadCount(writer_reader_count); }

    void WaitForObjectIdle(bool is_writer) {
        // Wait for thread-safe access to object instead of skipping call.
        while (GetCount().GetReadCount() > (int)(!is_writer) || GetCount().GetWriteCount() > (int)is_writer) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

    std::atomic<std::thread::id> thread{};

  private:
    // Need to update write and read counts atomically. Writer in high 32 bits, reader in low 32 bits.
    std::atomic<int64_t> writer_reader_count{};
};

template <typename T>
class Counter {
  public:
    VulkanObjectType object_type{};
    Logger *logger{};

    vvl::concurrent_unordered_map<T, std::shared_ptr<ObjectUseData>, 6> object_table;

    void Init(VulkanObjectType type, Logger *val_obj) {
        object_type = type;
        logger = val_obj;
    }

    void CreateObject(T object) { object_table.insert(object, std::make_shared<ObjectUseData>()); }

    void DestroyObject(T object) {
        if (object) {
            object_table.erase(object);
        }
    }

    std::shared_ptr<ObjectUseData> FindObject(T object, const Location& loc) {
        assert(object_table.contains(object));
        auto iter = object_table.find(object);
        if (iter != object_table.end()) {
            return iter->second;
        } else {
            logger->LogError("UNASSIGNED-Threading-Info", object, loc,
                             "Couldn't find %s Object 0x%" PRIxLEAST64
                             ". This should not happen and may indicate a bug in the application.",
                             string_VulkanObjectType(object_type), (uint64_t)(object));
            return nullptr;
        }
    }

    void StartWrite(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        auto use_data = FindObject(object, loc);
        if (!use_data) {
            return;
        }

        const std::thread::id tid = std::this_thread::get_id();
        const ObjectUseData::WriteReadCount prev_count = use_data->AddWriter();
        const bool prev_read = prev_count.GetReadCount() != 0;
        const bool prev_write = prev_count.GetWriteCount() != 0;

        if (!prev_read && !prev_write) {
            // There is no current use of the object. Record writer thread.
            use_data->thread = tid;
        } else if (!prev_read) {
            assert(prev_write);
            // There are no other readers but there is another writer. Two writers just collided.
            if (use_data->thread != tid) {
                HandleErrorOnWrite(use_data, object, loc);
            } else {
                // This is either safe multiple use in one call, or recursive use.
                // There is no way to make recursion safe. Just forge ahead.
            }
        } else {
            assert(prev_read);
            // There are other readers. This writer collided with them.
            if (use_data->thread != tid) {
                HandleErrorOnWrite(use_data, object, loc);
            } else {
                // This is either safe multiple use in one call, or recursive use.
                // There is no way to make recursion safe. Just forge ahead.
            }
        }
    }

    void FinishWrite(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        auto use_data = FindObject(object, loc);
        if (!use_data) {
            return;
        }
        use_data->RemoveWriter();
    }

    void StartRead(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        auto use_data = FindObject(object, loc);
        if (!use_data) {
            return;
        }

        const std::thread::id tid = std::this_thread::get_id();
        const ObjectUseData::WriteReadCount prev_count = use_data->AddReader();
        const bool prev_read = prev_count.GetReadCount() != 0;
        const bool prev_write = prev_count.GetWriteCount() != 0;

        if (!prev_read && !prev_write) {
            // There is no current use of the object. Record reader thread.
            use_data->thread = tid;
        } else if (prev_write && use_data->thread != tid) {
            HandleErrorOnRead(use_data, object, loc);
        } else {
            // There are other readers of the object.
        }
    }

    void FinishRead(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        auto use_data = FindObject(object, loc);
        if (!use_data) {
            return;
        }
        use_data->RemoveReader();
    }

  private:
    std::string GetErrorMessage(std::thread::id tid, std::thread::id other_tid) const {
        std::stringstream err_str;
        err_str << "THREADING ERROR : object of type " << string_VulkanObjectType(object_type)
                << " is simultaneously used in current thread " << tid << " and thread " << other_tid;
        return err_str.str();
    }

    void HandleErrorOnWrite(const std::shared_ptr<ObjectUseData> &use_data, T object, const Location& loc) {
        const std::thread::id tid = std::this_thread::get_id();
        const std::string error_message = GetErrorMessage(tid, use_data->thread.load(std::memory_order_relaxed));
        const bool skip = logger->LogError("UNASSIGNED-Threading-MultipleThreads-Write", object, loc, "%s", error_message.c_str());
        if (skip) {
            // Wait for thread-safe access to object instead of skipping call.
            use_data->WaitForObjectIdle(true);
            // There is now no current use of the object. Record writer thread.
            use_data->thread = tid;
        } else {
            // There is now no current use of the object. Record writer thread.
            use_data->thread = tid;
        }
    }

    void HandleErrorOnRead(const std::shared_ptr<ObjectUseData> &use_data, T object, const Location& loc) {
        const std::thread::id tid = std::this_thread::get_id();
        // There is a writer of the object.
        const auto error_message = GetErrorMessage(tid, use_data->thread.load(std::memory_order_relaxed));
        const bool skip = logger->LogError("UNASSIGNED-Threading-MultipleThreads-Read", object, loc, "%s", error_message.c_str());
        if (skip) {
            // Wait for thread-safe access to object instead of skipping call.
            use_data->WaitForObjectIdle(false);
            use_data->thread = tid;
        }
    }
};

#define WRAPPER(type)                                                                               \
    void StartWriteObject(type object, const Location &loc) { c_##type.StartWrite(object, loc); }   \
    void FinishWriteObject(type object, const Location &loc) { c_##type.FinishWrite(object, loc); } \
    void StartReadObject(type object, const Location &loc) { c_##type.StartRead(object, loc); }     \
    void FinishReadObject(type object, const Location &loc) { c_##type.FinishRead(object, loc); }   \
    void CreateObject(type object) { c_##type.CreateObject(object); }                               \
    void DestroyObject(type object) { c_##type.DestroyObject(object); }

class Instance : public vvl::base::Instance {
    using BaseClass = vvl::base::Instance;

  public:
    std::shared_mutex thread_safety_lock;

    Instance(vvl::dispatch::Instance *dispatch) : BaseClass(dispatch, LayerObjectTypeThreading) { InitCounters(); }

    void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                  VkDisplayPlanePropertiesKHR *pProperties,
                                                                  const RecordObject &record_obj) override;

    void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                   VkDisplayPlaneProperties2KHR *pProperties,
                                                                   const RecordObject &record_obj) override;

    void PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                             VkDisplayPropertiesKHR *pProperties,
                                                             const RecordObject &record_obj) override;

    void PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                              VkDisplayProperties2KHR *pProperties,
                                                              const RecordObject &record_obj) override;

    void PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                      const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                      VkDisplayPlaneCapabilities2KHR *pCapabilities,
                                                      const RecordObject &record_obj) override;

    void PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                       const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                       VkDisplayPlaneCapabilities2KHR *pCapabilities,
                                                       const RecordObject &record_obj) override;

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

    void PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display *dpy, RROutput rrOutput,
                                                VkDisplayKHR *pDisplay, const RecordObject &record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    void PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR *display,
                                        const RecordObject &record_obj) override;
#include "generated/thread_safety_instance_defs.h"
};

#define WRAPPER_PARENT_INSTANCE(type)                                                                                           \
    void StartWriteObjectParentInstance(type object, const Location &loc) { parent_instance->StartWriteObject(object, loc); }   \
    void FinishWriteObjectParentInstance(type object, const Location &loc) { parent_instance->FinishWriteObject(object, loc); } \
    void StartReadObjectParentInstance(type object, const Location &loc) { parent_instance->StartReadObject(object, loc); }     \
    void FinishReadObjectParentInstance(type object, const Location &loc) { parent_instance->FinishReadObject(object, loc); }

class Device : public vvl::base::Device {
    using BaseClass = vvl::base::Device;

  public:
    std::shared_mutex thread_safety_lock;

    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    vvl::concurrent_unordered_map<VkCommandBuffer, VkCommandPool, 6> command_pool_map;
    vvl::unordered_map<VkCommandPool, vvl::unordered_set<VkCommandBuffer>> pool_command_buffers_map;
    vvl::unordered_map<VkDevice, vvl::unordered_set<VkQueue>> device_queues_map;

    // Track per-descriptorsetlayout and per-descriptorset whether read_only is used.
    // This is used to (sloppily) implement the relaxed externsync rules for read_only
    // descriptors. We model updates of read_only descriptors as if they were reads
    // rather than writes, because they only conflict with the set being freed or reset.
    //
    // We don't track the read_only state per-binding for a couple reasons:
    // (1) We only have one counter per object, and if we treated non-UAB as writes
    //     and UAB as reads then they'd appear to conflict with each other.
    // (2) Avoid additional tracking of descriptor binding state in the descriptor set
    //     layout, and tracking of which bindings are accessed by a VkDescriptorUpdateTemplate.
    // Descriptor sets using VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT can also
    // be used simultaneously in multiple threads
    vvl::concurrent_unordered_map<VkDescriptorSetLayout, bool, 4> dsl_read_only_map;
    vvl::concurrent_unordered_map<VkDescriptorSet, bool, 6> ds_read_only_map;
    bool DsReadOnly(VkDescriptorSet) const;
    // Map of wrapped swapchain handles to arrays of wrapped swapchain image IDs
    // Each swapchain has an immutable list of wrapped swapchain image IDs -- always return these IDs if they exist
    vvl::unordered_map<VkSwapchainKHR, std::vector<VkImage>> swapchain_wrapped_image_handle_map;
    // Map of wrapped descriptor pools to set of wrapped descriptor sets allocated from each pool
    vvl::unordered_map<VkDescriptorPool, vvl::unordered_set<VkDescriptorSet>> pool_descriptor_sets_map;

    // Special entry to allow tracking of command pool Reset and Destroy
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES
    Counter<VkCommandPool> c_VkCommandPoolContents;
#else   // DISTINCT_NONDISPATCHABLE_HANDLES
    Counter<uint64_t> c_VkCommandPoolContents;
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES

    Instance *parent_instance;

    Device(vvl::dispatch::Device *dev, Instance *instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeThreading), parent_instance(instance_vo) {
        c_VkCommandPoolContents.Init(kVulkanObjectTypeCommandPool, this);
        InitCounters();
    }

    void CreateObject(VkCommandBuffer object) { c_VkCommandBuffer.CreateObject(object); }
    void DestroyObject(VkCommandBuffer object) { c_VkCommandBuffer.DestroyObject(object); }

    // VkCommandBuffer needs check for implicit use of command pool
    void StartWriteObject(VkCommandBuffer object, const Location& loc, bool lockPool = true) {
        if (lockPool) {
            auto iter = command_pool_map.find(object);
            if (iter != command_pool_map.end()) {
                VkCommandPool pool = iter->second;
                StartWriteObject(pool, loc);
            }
        }
        c_VkCommandBuffer.StartWrite(object, loc);
    }
    void FinishWriteObject(VkCommandBuffer object, const Location& loc, bool lockPool = true) {
        c_VkCommandBuffer.FinishWrite(object, loc);
        if (lockPool) {
            auto iter = command_pool_map.find(object);
            if (iter != command_pool_map.end()) {
                VkCommandPool pool = iter->second;
                FinishWriteObject(pool, loc);
            }
        }
    }
    void StartReadObject(VkCommandBuffer object, const Location& loc) {
        auto iter = command_pool_map.find(object);
        if (iter != command_pool_map.end()) {
            VkCommandPool pool = iter->second;
            // We set up a read guard against the "Contents" counter to catch conflict vs. vkResetCommandPool and
            // vkDestroyCommandPool while *not* establishing a read guard against the command pool counter itself to avoid false
            // positive for non-externally sync'd command buffers
            c_VkCommandPoolContents.StartRead(pool, loc);
        }
        c_VkCommandBuffer.StartRead(object, loc);
    }
    void FinishReadObject(VkCommandBuffer object, const Location& loc) {
        c_VkCommandBuffer.FinishRead(object, loc);
        auto iter = command_pool_map.find(object);
        if (iter != command_pool_map.end()) {
            VkCommandPool pool = iter->second;
            c_VkCommandPoolContents.FinishRead(pool, loc);
        }
    }

#include "generated/thread_safety_device_defs.h"
};
}  // namespace threadsafety
