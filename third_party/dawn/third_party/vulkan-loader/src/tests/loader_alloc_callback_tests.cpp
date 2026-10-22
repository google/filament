/*
 * Copyright (c) 2021-2023 The Khronos Group Inc.
 * Copyright (c) 2021-2023 Valve Corporation
 * Copyright (c) 2021-2023 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "manifest_builders.h"
#include "test_environment.h"

#include <mutex>

struct MemoryTrackerSettings {
    bool should_fail_on_allocation = false;
    size_t fail_after_allocations = 0;  // fail after this number of allocations in total
    bool should_fail_after_set_number_of_calls = false;
    size_t fail_after_calls = 0;  // fail after this number of calls to alloc or realloc
};

class MemoryTracker {
    std::mutex main_mutex;
    MemoryTrackerSettings settings{};
    VkAllocationCallbacks callbacks{};
    // Implementation internals
    struct AllocationDetails {
        std::unique_ptr<char[]> allocation;
        size_t requested_size_bytes;
        size_t actual_size_bytes;
        size_t alignment;
        VkSystemAllocationScope alloc_scope;
    };
    const static size_t UNKNOWN_ALLOCATION = std::numeric_limits<size_t>::max();
    size_t allocation_count = 0;
    size_t call_count = 0;
    bool fail_next_growing_reallocation = false;
    bool fail_growing_reallocation_after_skips = false;
    size_t growing_reallocations_to_skip = 0;
    bool poison_growing_reallocations = false;
    std::unordered_map<void*, AllocationDetails> allocations;

    void* allocate(size_t size, size_t alignment, VkSystemAllocationScope alloc_scope) {
        if ((settings.should_fail_on_allocation && allocation_count == settings.fail_after_allocations) ||
            (settings.should_fail_after_set_number_of_calls && call_count == settings.fail_after_calls)) {
            return nullptr;
        }
        call_count++;
        allocation_count++;
        AllocationDetails detail{nullptr, size, size + (alignment - 1), alignment, alloc_scope};
        detail.allocation = std::unique_ptr<char[]>(new char[detail.actual_size_bytes]);
        if (!detail.allocation) {
            abort();
        };
        uint64_t addr = (uint64_t)detail.allocation.get();
        addr += (alignment - 1);
        addr &= ~(alignment - 1);
        void* aligned_alloc = (void*)addr;
        allocations.insert(std::make_pair(aligned_alloc, std::move(detail)));
        return aligned_alloc;
    }
    void* reallocate(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope alloc_scope) {
        if (pOriginal == nullptr) {
            return allocate(size, alignment, alloc_scope);
        }
        auto elem = allocations.find(pOriginal);
        if (elem == allocations.end()) return nullptr;
        size_t original_size = elem->second.requested_size_bytes;

        // We only care about the case where realloc is used to increase the size
        if (size >= original_size && fail_next_growing_reallocation) {
            fail_next_growing_reallocation = false;
            return nullptr;
        }
        if (size >= original_size && fail_growing_reallocation_after_skips) {
            if (growing_reallocations_to_skip == 0) {
                fail_growing_reallocation_after_skips = false;
                return nullptr;
            }
            growing_reallocations_to_skip--;
        }
        if (size >= original_size && settings.should_fail_after_set_number_of_calls && call_count == settings.fail_after_calls)
            return nullptr;
        call_count++;
        if (size == 0) {
            allocations.erase(elem);
            allocation_count--;
            return nullptr;
        } else if (size < original_size) {
            return pOriginal;
        } else {
            void* new_alloc = allocate(size, alignment, alloc_scope);
            if (new_alloc == nullptr) return nullptr;
            allocation_count--;  // allocate() increments this, we we don't want that
            call_count--;        // allocate() also increments this, we don't want that
            memcpy(new_alloc, pOriginal, original_size);
            if (poison_growing_reallocations && size > original_size) {
                // The loader does not zero the region an app pfnReallocation grows, so fill it with a non-zero
                // pattern to make any read of an unwritten grown slot deterministic.
                memset(static_cast<char*>(new_alloc) + original_size, 0xff, size - original_size);
            }
            allocations.erase(elem);
            return new_alloc;
        }
    }
    void free(void* pMemory) {
        if (pMemory == nullptr) return;
        auto elem = allocations.find(pMemory);
        if (elem == allocations.end()) {
            assert(false && "Should never be freeing memory that wasn't allocated by the MemoryTracker!");
            return;
        }
        allocations.erase(elem);
        assert(allocation_count != 0 && "Cant free when there are no valid allocations");
        allocation_count--;
    }

    // Implementation of public functions
    void* impl_allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope) noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        void* addr = allocate(size, alignment, allocationScope);
        return addr;
    }
    void* impl_reallocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        void* addr = reallocate(pOriginal, size, alignment, allocationScope);
        return addr;
    }
    void impl_free(void* pMemory) noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        free(pMemory);
    }
    void impl_internal_allocation_notification([[maybe_unused]] size_t size,
                                               [[maybe_unused]] VkInternalAllocationType allocationType,
                                               [[maybe_unused]] VkSystemAllocationScope allocationScope) noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        // TODO?
    }
    void impl_internal_free([[maybe_unused]] size_t size, [[maybe_unused]] VkInternalAllocationType allocationType,
                            [[maybe_unused]] VkSystemAllocationScope allocationScope) noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        // TODO?
    }

   public:
    MemoryTracker(MemoryTrackerSettings settings) noexcept : settings(settings) {
        allocations.reserve(3000);

        callbacks.pUserData = this;
        callbacks.pfnAllocation = public_allocation;
        callbacks.pfnReallocation = public_reallocation;
        callbacks.pfnFree = public_free;
        callbacks.pfnInternalAllocation = public_internal_allocation_notification;
        callbacks.pfnInternalFree = public_internal_free;
    }
    MemoryTracker() noexcept : MemoryTracker(MemoryTrackerSettings{}) {}

    VkAllocationCallbacks* get() noexcept { return &callbacks; }

    bool empty() noexcept { return allocation_count == 0; }

    // Arm a single failure for the next reallocation that grows an existing block. Used to force the used-object
    // list resize inside loader_get_next_available_entry to fail without disturbing prior allocations.
    void arm_next_growing_reallocation_failure() noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        fail_next_growing_reallocation = true;
    }

    // Arm a single failure for a later growing reallocation, allowing skip_count earlier growing reallocations
    // to succeed first. Lets a test target a resize that is not the first one in a single loader call, eg the
    // per-ICD debug list resize that runs after the used-object status list resize.
    void arm_growing_reallocation_failure_after(size_t skip_count) noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        fail_growing_reallocation_after_skips = true;
        growing_reallocations_to_skip = skip_count;
    }

    // Fill the newly grown region of every growing reallocation with a non-zero pattern, matching what an app
    // pfnReallocation that does not zero grown memory would leave behind.
    void poison_grown_reallocations() noexcept {
        std::lock_guard<std::mutex> lg(main_mutex);
        poison_growing_reallocations = true;
    }

    // Static callbacks
    static VKAPI_ATTR void* VKAPI_CALL public_allocation(void* pUserData, size_t size, size_t alignment,
                                                         VkSystemAllocationScope allocationScope) noexcept {
        return reinterpret_cast<MemoryTracker*>(pUserData)->impl_allocation(size, alignment, allocationScope);
    }
    static VKAPI_ATTR void* VKAPI_CALL public_reallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment,
                                                           VkSystemAllocationScope allocationScope) noexcept {
        return reinterpret_cast<MemoryTracker*>(pUserData)->impl_reallocation(pOriginal, size, alignment, allocationScope);
    }
    static VKAPI_ATTR void VKAPI_CALL public_free(void* pUserData, void* pMemory) noexcept {
        reinterpret_cast<MemoryTracker*>(pUserData)->impl_free(pMemory);
    }
    static VKAPI_ATTR void VKAPI_CALL public_internal_allocation_notification(void* pUserData, size_t size,
                                                                              VkInternalAllocationType allocationType,
                                                                              VkSystemAllocationScope allocationScope) noexcept {
        reinterpret_cast<MemoryTracker*>(pUserData)->impl_internal_allocation_notification(size, allocationType, allocationScope);
    }
    static VKAPI_ATTR void VKAPI_CALL public_internal_free(void* pUserData, size_t size, VkInternalAllocationType allocationType,
                                                           VkSystemAllocationScope allocationScope) noexcept {
        reinterpret_cast<MemoryTracker*>(pUserData)->impl_internal_free(size, allocationType, allocationScope);
    }
};

// Test making sure the allocation functions are called to allocate and cleanup everything during
// a CreateInstance/DestroyInstance call pair.
TEST(Allocation, Instance) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    MemoryTracker tracker;
    {
        InstWrapper inst{env.vulkan_functions, tracker.get()};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
    }
    ASSERT_TRUE(tracker.empty());
}

// Test making sure the allocation functions are called to allocate and cleanup everything during
// a CreateInstance/DestroyInstance call pair with a call to GetInstanceProcAddr.
TEST(Allocation, GetInstanceProcAddr) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    MemoryTracker tracker;
    {
        InstWrapper inst{env.vulkan_functions, tracker.get()};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

        auto* pfnCreateDevice = inst->vkGetInstanceProcAddr(inst, "vkCreateDevice");
        auto* pfnDestroyDevice = inst->vkGetInstanceProcAddr(inst, "vkDestroyDevice");
        ASSERT_TRUE(pfnCreateDevice != nullptr && pfnDestroyDevice != nullptr);
    }
    ASSERT_TRUE(tracker.empty());
}

// Test making sure the allocation functions are called to allocate and cleanup everything during
// a vkEnumeratePhysicalDevices call pair.
TEST(Allocation, EnumeratePhysicalDevices) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2).add_physical_device("physical_device_0");

    MemoryTracker tracker;
    {
        InstWrapper inst{env.vulkan_functions, tracker.get()};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
        uint32_t physical_count = 1;
        uint32_t returned_physical_count = 0;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        ASSERT_EQ(physical_count, returned_physical_count);

        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, &physical_device));
        ASSERT_EQ(physical_count, returned_physical_count);
    }
    ASSERT_TRUE(tracker.empty());
}

// Test making sure the allocation functions are called to allocate and cleanup everything from
// vkCreateInstance, to vkCreateDevicce, and then through their destructors.  With special
// allocators used on both the instance and device.
TEST(Allocation, InstanceAndDevice) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2)
        .add_physical_device(
            PhysicalDevice{"physical_device_0"}.add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false}));

    MemoryTracker tracker;
    {
        InstWrapper inst{env.vulkan_functions, tracker.get()};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

        uint32_t physical_count = 1;
        uint32_t returned_physical_count = 0;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        ASSERT_EQ(physical_count, returned_physical_count);

        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, &physical_device));
        ASSERT_EQ(physical_count, returned_physical_count);

        uint32_t family_count = 1;
        uint32_t returned_family_count = 0;
        env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &returned_family_count, nullptr);
        ASSERT_EQ(returned_family_count, family_count);

        VkQueueFamilyProperties family;
        env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &returned_family_count, &family);
        ASSERT_EQ(returned_family_count, family_count);
        ASSERT_EQ(family.queueFlags, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT));
        ASSERT_EQ(family.queueCount, family_count);
        ASSERT_EQ(family.timestampValidBits, 0U);

        DeviceCreateInfo dev_create_info;
        dev_create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));

        VkDevice device;
        ASSERT_EQ(inst->vkCreateDevice(physical_device, dev_create_info.get(), tracker.get(), &device), VK_SUCCESS);

        VkQueue queue;
        inst->vkGetDeviceQueue(device, 0, 0, &queue);

        inst->vkDestroyDevice(device, tracker.get());
    }
    ASSERT_TRUE(tracker.empty());
}
// Test making sure the allocation functions are called to allocate and cleanup everything from
// vkCreateInstance, to vkCreateDevicce, and then through their destructors.  With special
// allocators used on only the instance and not the device.
TEST(Allocation, InstanceButNotDevice) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2)
        .add_physical_device(
            PhysicalDevice{"physical_device_0"}.add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false}));

    MemoryTracker tracker;
    {
        InstWrapper inst{env.vulkan_functions, tracker.get()};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

        uint32_t physical_count = 1;
        uint32_t returned_physical_count = 0;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        ASSERT_EQ(physical_count, returned_physical_count);

        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, &physical_device));
        ASSERT_EQ(physical_count, returned_physical_count);

        uint32_t family_count = 1;
        uint32_t returned_family_count = 0;
        env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &returned_family_count, nullptr);
        ASSERT_EQ(returned_family_count, family_count);

        VkQueueFamilyProperties family;
        env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &returned_family_count, &family);
        ASSERT_EQ(returned_family_count, family_count);
        ASSERT_EQ(family.queueFlags, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT));
        ASSERT_EQ(family.queueCount, family_count);
        ASSERT_EQ(family.timestampValidBits, 0U);

        DeviceCreateInfo dev_create_info;
        dev_create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));

        VkDevice device;
        ASSERT_EQ(inst->vkCreateDevice(physical_device, dev_create_info.get(), nullptr, &device), VK_SUCCESS);
        VkQueue queue;
        inst->vkGetDeviceQueue(device, 0, 0, &queue);

        inst->vkDestroyDevice(device, nullptr);
    }
    ASSERT_TRUE(tracker.empty());
}

// Test making sure the allocation functions are called to allocate and cleanup everything from
// vkCreateInstance, to vkCreateDevicce, and then through their destructors.  With special
// allocators used on only the device and not the instance.
TEST(Allocation, DeviceButNotInstance) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2)
        .add_physical_device(
            PhysicalDevice{"physical_device_0"}.add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false}));

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    MemoryTracker tracker;
    {
        InstWrapper inst{env.vulkan_functions};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

        uint32_t physical_count = 1;
        uint32_t returned_physical_count = 0;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
        ASSERT_EQ(physical_count, returned_physical_count);

        VkPhysicalDevice physical_device;
        ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, &physical_device));
        ASSERT_EQ(physical_count, returned_physical_count);

        uint32_t family_count = 1;
        uint32_t returned_family_count = 0;
        env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &returned_family_count, nullptr);
        ASSERT_EQ(returned_family_count, family_count);

        VkQueueFamilyProperties family;
        env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &returned_family_count, &family);
        ASSERT_EQ(returned_family_count, family_count);
        ASSERT_EQ(family.queueFlags, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT));
        ASSERT_EQ(family.queueCount, family_count);
        ASSERT_EQ(family.timestampValidBits, 0U);

        DeviceCreateInfo dev_create_info;
        dev_create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));

        VkDevice device;
        ASSERT_EQ(inst->vkCreateDevice(physical_device, dev_create_info.get(), tracker.get(), &device), VK_SUCCESS);

        VkQueue queue;
        inst->vkGetDeviceQueue(device, 0, 0, &queue);

        inst->vkDestroyDevice(device, tracker.get());
    }
    ASSERT_TRUE(tracker.empty());
}

// Test failure during vkCreateInstance to make sure we don't leak memory if
// one of the out-of-memory conditions trigger.
TEST(Allocation, CreateInstanceIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_SUCCESS) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        }
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

// Test failure during vkCreateInstance to make sure we don't leak memory if
// one of the out-of-memory conditions trigger and there are invalid jsons in the same folder
TEST(Allocation, CreateInstanceIntentionalAllocFailInvalidManifests) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    std::vector<std::string> invalid_jsons;
    invalid_jsons.push_back(",");
    invalid_jsons.push_back("{},[]");
    invalid_jsons.push_back("{ \"foo\":\"bar\", }");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": [], },");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": [{},] },");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": {\"fee\"} },");
    invalid_jsons.push_back("{\"\":\"bar\", \"baz\": {}");
    invalid_jsons.push_back("{\"foo\":\"bar\", \"baz\": {\"fee\":1234, true, \"ab\":\"bc\"} },");

    for (size_t i = 0; i < invalid_jsons.size(); i++) {
        auto file_name = std::string("invalid_implicit_layer_") + std::to_string(i) + ".json";
        std::filesystem::path new_path =
            env.get_folder(ManifestLocation::implicit_layer).write_manifest(file_name, invalid_jsons[i]);
#if defined(WIN32)
        env.platform_shim->add_manifest_to_registry(ManifestCategory::implicit_layer, new_path);
#endif
    }

    const char* layer_name = "VkLayerImplicit0";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_SUCCESS) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        }
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

// Test failure during vkCreateInstance & surface creation to make sure we don't leak memory if
// one of the out-of-memory conditions trigger.
TEST(Allocation, CreateSurfaceIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2).setup_WSI();

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        inst_create_info.setup_WSI();
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            ASSERT_TRUE(tracker.empty());
            fail_index++;
            continue;
        }

        VkSurfaceKHR surface{};
        result = create_surface(&env.vulkan_functions, instance, surface);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            fail_index++;
            continue;
        }
        env.vulkan_functions.vkDestroySurfaceKHR(instance, surface, tracker.get());

        env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL noop_debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                                VkDebugUtilsMessageTypeFlagsEXT,
                                                                const VkDebugUtilsMessengerCallbackDataEXT*, void*) {
    return VK_FALSE;
}

// When a debug messenger create can't reserve a slot (the used-object status list is full and its resize is
// refused), the error rollback used to be gated on the pNextIndex allocation instead of on a slot actually
// being reserved. With next_index left at 0 the rollback destroyed the ICD handle of the messenger already
// living in slot 0 - a driver-side double free once the application later destroyed that live messenger.
TEST(Allocation, DebugUtilsMessengerReservationFailKeepsLiveMessenger) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2)
        .set_min_icd_interface_version(5)
        .add_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
        .add_physical_device({});

    MemoryTracker tracker{};

    VkInstance instance = VK_NULL_HANDLE;
    InstanceCreateInfo inst_create_info{};
    inst_create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance));

    PFN_vkCreateDebugUtilsMessengerEXT create_messenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        env.vulkan_functions.vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_messenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        env.vulkan_functions.vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    ASSERT_NE(create_messenger, nullptr);
    ASSERT_NE(destroy_messenger, nullptr);

    VkDebugUtilsMessengerCreateInfoEXT messenger_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    messenger_info.pfnUserCallback = noop_debug_utils_callback;

    // The used-object status list starts with room for 32 entries, so filling it makes the next reservation grow it.
    std::vector<VkDebugUtilsMessengerEXT> messengers;
    for (uint32_t i = 0; i < 32; i++) {
        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, create_messenger(instance, &messenger_info, nullptr, &messenger));
        messengers.push_back(messenger);
    }

    auto& icd_details = env.get_test_icd(0).created_instance_details.begin()->second;
    ASSERT_EQ(icd_details.messenger_handles.size(), 32u);

    // Make the used-object list resize fail so the reservation fails after pNextIndex is already allocated.
    tracker.arm_next_growing_reallocation_failure();
    VkDebugUtilsMessengerEXT failed_messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_ERROR_OUT_OF_HOST_MEMORY, create_messenger(instance, &messenger_info, nullptr, &failed_messenger));

    // No slot was reserved, so the still-live messenger occupying slot 0 must remain untouched in the ICD.
    EXPECT_EQ(icd_details.messenger_handles.size(), 32u);

    for (auto messenger : messengers) {
        destroy_messenger(instance, messenger, nullptr);
    }
    EXPECT_EQ(icd_details.messenger_handles.size(), 0u);
    env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
    ASSERT_TRUE(tracker.empty());
}

// Each ICD keeps its own debug_utils_messenger_list, resized one ICD at a time inside the create loop. If a
// create fails after the used-object slot was reserved but before an ICD's list has been grown to cover the
// reserved index (eg that ICD's resize is refused), the error rollback indexed list[next_index] without
// checking that ICD's list capacity, reading past the end of the smaller allocation. Force the per-ICD resize
// to fail on the entry that first needs the list grown and confirm the rollback stays in bounds.
TEST(Allocation, DebugUtilsMessengerCreateRollbackStaysInBounds) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2)
        .set_min_icd_interface_version(5)
        .add_instance_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
        .add_physical_device({});

    MemoryTracker tracker{};

    VkInstance instance = VK_NULL_HANDLE;
    InstanceCreateInfo inst_create_info{};
    inst_create_info.add_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance));

    PFN_vkCreateDebugUtilsMessengerEXT create_messenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        env.vulkan_functions.vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_messenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        env.vulkan_functions.vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    ASSERT_NE(create_messenger, nullptr);
    ASSERT_NE(destroy_messenger, nullptr);

    VkDebugUtilsMessengerCreateInfoEXT messenger_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    messenger_info.pfnUserCallback = noop_debug_utils_callback;

    // Both the used-object status list and the per-ICD list start with room for 32 entries, so the 33rd create
    // is the first that grows them.
    std::vector<VkDebugUtilsMessengerEXT> messengers;
    for (uint32_t i = 0; i < 32; i++) {
        VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, create_messenger(instance, &messenger_info, nullptr, &messenger));
        messengers.push_back(messenger);
    }

    auto& icd_details = env.get_test_icd(0).created_instance_details.begin()->second;
    ASSERT_EQ(icd_details.messenger_handles.size(), 32u);

    // Let the used-object list resize succeed (so the slot is reserved) but fail the following per-ICD list
    // resize, leaving that ICD's list too small for the reserved index when the rollback runs.
    tracker.arm_growing_reallocation_failure_after(1);
    VkDebugUtilsMessengerEXT failed_messenger = VK_NULL_HANDLE;
    ASSERT_EQ(VK_ERROR_OUT_OF_HOST_MEMORY, create_messenger(instance, &messenger_info, nullptr, &failed_messenger));

    // The 32 live messengers are untouched by the rollback and still destroy cleanly.
    EXPECT_EQ(icd_details.messenger_handles.size(), 32u);
    for (auto messenger : messengers) {
        destroy_messenger(instance, messenger, nullptr);
    }
    EXPECT_EQ(icd_details.messenger_handles.size(), 0u);
    env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
    ASSERT_TRUE(tracker.empty());
}

// Test failure during vkCreateInstance to make sure we don't leak memory if
// one of the out-of-memory conditions trigger.
TEST(Allocation, CreateInstanceIntentionalAllocFailWithSettingsFilePresent) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("auto")
                .set_path(env.get_shimmed_layer_manifest_path(0)))));

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_SUCCESS) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        }
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

// An active layer configuration in the settings file routes layer selection through
// enable_correct_layers_from_settings, which reads VK_INSTANCE_LAYERS itself. On Windows loader_getenv
// allocates through the application's allocator, so that read has to be released like the ones in
// loader_enable_instance_layers and loader_validate_instance_extensions.
TEST(Allocation, SettingsLayerConfigurationWithInstanceLayersEnvVar) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    const char* layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        {}, ManifestLayer{}.add_layer(
                ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)));

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0)))));

    EnvVarWrapper instance_layers_env_var{"VK_INSTANCE_LAYERS", layer_name};

    MemoryTracker tracker{};
    {
        InstWrapper inst{env.vulkan_functions, tracker.get()};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
    }
    ASSERT_TRUE(tracker.empty());
}

// Test failure during vkCreateInstance & surface creation to make sure we don't leak memory if
// one of the out-of-memory conditions trigger.
TEST(Allocation, CreateSurfaceIntentionalAllocFailWithSettingsFilePresent) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2).setup_WSI();

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);
    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("auto")
                .set_path(env.get_shimmed_layer_manifest_path(0)))));

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        inst_create_info.setup_WSI();
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            ASSERT_TRUE(tracker.empty());
            fail_index++;
            continue;
        }

        VkSurfaceKHR surface{};
        result = create_surface(&env.vulkan_functions, instance, surface);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            fail_index++;
            continue;
        }
        env.vulkan_functions.vkDestroySurfaceKHR(instance, surface, tracker.get());

        env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

// Test failure during vkCreateInstance to make sure we don't leak memory if
// one of the out-of-memory conditions trigger.
TEST(Allocation, DriverEnvVarIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2, ManifestOptions{}.set_discovery_type(ManifestDiscoveryType::env_var));

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    env.env_var_vk_icd_filenames.add_to_list("totally_made_up/path_to_fake/jason_file.json");
    env.env_var_vk_icd_filenames.add_to_list("another\\bonkers\\file_path.json");
    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_SUCCESS) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        }
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

// Test failure during vkCreateDevice to make sure we don't leak memory if
// one of the out-of-memory conditions trigger.
// Use 2 physical devices so that anything which copies a list of devices item by item
// may fail.
TEST(Allocation, CreateDeviceIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2)
        .add_physical_device(
            PhysicalDevice{"physical_device_0"}.add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false}))
        .add_physical_device(
            PhysicalDevice{"physical_device_1"}.add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false}));

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    InstWrapper inst{env.vulkan_functions};
    ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());

    uint32_t physical_count = 2;
    uint32_t returned_physical_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, nullptr));
    ASSERT_EQ(physical_count, returned_physical_count);

    VkPhysicalDevice physical_devices[2];
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst.inst, &returned_physical_count, physical_devices));
    ASSERT_EQ(physical_count, returned_physical_count);

    uint32_t family_count = 1;
    uint32_t returned_family_count = 0;
    env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0], &returned_family_count, nullptr);
    ASSERT_EQ(returned_family_count, family_count);

    VkQueueFamilyProperties family;
    env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0], &returned_family_count, &family);
    ASSERT_EQ(returned_family_count, family_count);
    ASSERT_EQ(family.queueFlags, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT));
    ASSERT_EQ(family.queueCount, family_count);
    ASSERT_EQ(family.timestampValidBits, 0U);

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
        MemoryTracker tracker({false, 0, true, fail_index});

        DeviceCreateInfo dev_create_info;
        dev_create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));

        VkDevice device;
        result = inst->vkCreateDevice(physical_devices[0], dev_create_info.get(), tracker.get(), &device);
        if (result == VK_SUCCESS || fail_index > 10000) {
            VkQueue queue;
            inst->vkGetDeviceQueue(device, 0, 0, &queue);

            inst->vkDestroyDevice(device, tracker.get());
            break;
        }
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

// Test failure during vkCreateInstance and vkCreateDevice to make sure we don't
// leak memory if one of the out-of-memory conditions trigger.
// Includes drivers with several instance extensions, drivers that will fail to load, directly loaded drivers
TEST(Allocation, CreateInstanceDeviceIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    uint32_t num_physical_devices = 4;
    uint32_t num_implicit_layers = 3;
    for (uint32_t i = 0; i < num_physical_devices; i++) {
        env.add_icd(TEST_ICD_PATH_VERSION_2, {},
                    ManifestICD{}.set_is_portability_driver(false).set_library_arch(sizeof(void*) == 8 ? "64" : "32"))
            .set_icd_api_version(VK_API_VERSION_1_1)
            .add_instance_extension("VK_KHR_get_physical_device_properties2")
            .add_and_get_physical_device("physical_device_0")
            .add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false})
            .add_extensions({"VK_EXT_one", "VK_EXT_two", "VK_EXT_three", "VK_EXT_four", "VK_EXT_five"});
    }

    env.add_icd(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE, ManifestOptions{}.set_is_fake(true));

    env.add_icd(TEST_ICD_PATH_VERSION_7, ManifestOptions{}.set_discovery_type(ManifestDiscoveryType::none));

    VkDirectDriverLoadingInfoLUNARG ddl_info{};
    ddl_info.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG;
    ddl_info.pfnGetInstanceProcAddr = env.icds.back().library.get_symbol("vk_icdGetInstanceProcAddr");

    VkDirectDriverLoadingListLUNARG ddl_list{};
    ddl_list.sType = VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG;
    ddl_list.mode = VK_DIRECT_DRIVER_LOADING_MODE_INCLUSIVE_LUNARG;
    ddl_list.driverCount = 1;
    ddl_list.pDrivers = &ddl_info;

    const char* layer_name = "VK_LAYER_ImplicitAllocFail";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);
    for (uint32_t i = 1; i < num_implicit_layers + 1; i++) {
        env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                 .set_name("VK_LAYER_Implicit1" + std::to_string(i))
                                                                 .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                 .set_disable_environment("DISABLE_ENV")));
    }
    // Throw in a complex json file to flex the json allocation routines
    env.write_file_from_source(COMPLEX_JSON_FILE, ManifestCategory::explicit_layer, ManifestLocation::explicit_layer,
                               "VK_LAYER_complex_file.json");

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker{{false, 0, true, fail_index}};
        fail_index++;  // applies to the next loop

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        inst_create_info.add_extension(VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME);
        inst_create_info.instance_info.pNext = reinterpret_cast<const void*>(&ddl_list);
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            ASSERT_TRUE(tracker.empty());
            continue;
        }
        ASSERT_EQ(result, VK_SUCCESS);

        uint32_t returned_physical_count = 0;
        result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, nullptr);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            continue;
        }
        ASSERT_EQ(result, VK_SUCCESS);
        ASSERT_EQ(num_physical_devices, returned_physical_count);

        std::vector<VkPhysicalDevice> physical_devices;
        physical_devices.resize(returned_physical_count);
        result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, physical_devices.data());
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            continue;
        }
        ASSERT_EQ(result, VK_SUCCESS);
        ASSERT_EQ(num_physical_devices, returned_physical_count);
        for (uint32_t i = 0; i < returned_physical_count; i++) {
            uint32_t family_count = 1;
            uint32_t returned_family_count = 0;
            env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices.at(i), &returned_family_count, nullptr);
            ASSERT_EQ(returned_family_count, family_count);

            VkQueueFamilyProperties family;
            env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices.at(i), &returned_family_count, &family);
            ASSERT_EQ(returned_family_count, family_count);
            ASSERT_EQ(family.queueFlags, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT));
            ASSERT_EQ(family.queueCount, family_count);
            ASSERT_EQ(family.timestampValidBits, 0U);

            DeviceCreateInfo dev_create_info;
            dev_create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));

            VkDevice device;
            result = env.vulkan_functions.vkCreateDevice(physical_devices.at(i), dev_create_info.get(), tracker.get(), &device);
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                break;
            }
            ASSERT_EQ(result, VK_SUCCESS);

            VkQueue queue;
            env.vulkan_functions.vkGetDeviceQueue(device, 0, 0, &queue);

            env.vulkan_functions.vkDestroyDevice(device, tracker.get());
        }
        env.vulkan_functions.vkDestroyInstance(instance, tracker.get());

        ASSERT_TRUE(tracker.empty());
    }
}

// Test failure during vkCreateInstance when a driver of the wrong architecture is present
// to make sure the loader uses the valid ICD and doesn't report incompatible driver just because
// an incompatible driver exists
TEST(TryLoadWrongBinaries, CreateInstanceIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2);
    env.add_icd(CURRENT_PLATFORM_DUMMY_BINARY_WRONG_TYPE, ManifestOptions{}.set_is_fake(true));

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_SUCCESS) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        }
        ASSERT_NE(result, VK_ERROR_INCOMPATIBLE_DRIVER);
        ASSERT_TRUE(tracker.empty());
        fail_index++;
    }
}

// Test failure during vkCreateInstance and vkCreateDevice to make sure we don't
// leak memory if one of the out-of-memory conditions trigger.
TEST(Allocation, EnumeratePhysicalDevicesIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    size_t fail_index = 0;
    bool reached_the_end = false;
    uint32_t starting_physical_dev_count = 3;
    while (!reached_the_end && fail_index <= 10000) {
        fail_index++;  // applies to the next loop
        uint32_t physical_dev_count = starting_physical_dev_count;
        VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
        auto& driver = env.reset_icd();

        for (uint32_t i = 0; i < physical_dev_count; i++) {
            driver.add_and_get_physical_device(std::string("physical_device_") + std::to_string(i))
                .add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false});
        }
        MemoryTracker tracker{{false, 0, true, fail_index}};
        InstanceCreateInfo inst_create_info;
        VkInstance instance;
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            ASSERT_TRUE(tracker.empty());
            continue;
        }

        uint32_t returned_physical_count = 0;
        result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, nullptr);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            continue;
        }
        ASSERT_EQ(physical_dev_count, returned_physical_count);

        for (uint32_t i = 0; i < 2; i++) {
            driver.add_and_get_physical_device(std::string("physical_device_") + std::to_string(physical_dev_count))
                .add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false});
            physical_dev_count += 1;
        }

        std::vector<VkPhysicalDevice> physical_devices{physical_dev_count, VK_NULL_HANDLE};
        result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, physical_devices.data());
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            continue;
        }
        if (result == VK_INCOMPLETE) {
            result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, nullptr);
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
                ASSERT_TRUE(tracker.empty());
                continue;
            }
            physical_devices.resize(returned_physical_count);
            result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, physical_devices.data());
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
                ASSERT_TRUE(tracker.empty());
                continue;
            }
        }
        ASSERT_EQ(physical_dev_count, returned_physical_count);

        std::array<VkDevice, 5> devices;
        for (uint32_t i = 0; i < returned_physical_count; i++) {
            uint32_t family_count = 1;
            uint32_t returned_family_count = 0;
            env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &returned_family_count, nullptr);
            ASSERT_EQ(returned_family_count, family_count);

            VkQueueFamilyProperties family;
            env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &returned_family_count, &family);
            ASSERT_EQ(returned_family_count, family_count);
            ASSERT_EQ(family.queueFlags, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT));
            ASSERT_EQ(family.queueCount, family_count);
            ASSERT_EQ(family.timestampValidBits, 0U);

            DeviceCreateInfo dev_create_info;
            dev_create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));

            result = env.vulkan_functions.vkCreateDevice(physical_devices[i], dev_create_info.get(), tracker.get(), &devices[i]);

            VkQueue queue;
            if (result == VK_SUCCESS) {
                env.vulkan_functions.vkGetDeviceQueue(devices[i], 0, 0, &queue);
            }
        }
        for (uint32_t i = 0; i < returned_physical_count; i++) {
            if (result == VK_SUCCESS) {
                env.vulkan_functions.vkDestroyDevice(devices[i], tracker.get());
            }
        }

        env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
        ASSERT_TRUE(tracker.empty());
        reached_the_end = true;
    }
}

// A layer device_extension entry with no "entrypoints" leaves loader_dev_ext_props::entrypoints unwritten.
// The first 32 list slots come from a zero-filled allocation, but the 33rd lands in the region grown by the app
// pfnReallocation, which the loader does not zero. Freeing the discovered layer then walks that indeterminate
// loader_string_list. Poison the grown region so the read is deterministic.
TEST(Allocation, LayerDeviceExtensionsWithoutEntrypoints) {
    FrameworkEnvironment env{};
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    std::vector<ManifestLayer::LayerDescription::Extension> layer_exts;
    for (uint32_t i = 0; i < 40; i++) {
        layer_exts.emplace_back(std::string("VK_TEST_dev_ext_") + std::to_string(i), i + 1);
    }
    env.add_explicit_layer(ManifestOptions{}.set_json_name("test_dev_ext_layer.json"),
                           ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_test_dev_ext")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .add_device_extensions(layer_exts)));

    MemoryTracker tracker;
    tracker.poison_grown_reallocations();
    {
        InstWrapper inst{env.vulkan_functions, tracker.get()};
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
    }
    ASSERT_TRUE(tracker.empty());
}
#if defined(WIN32)
// Test failure during vkCreateInstance and vkCreateDevice to make sure we don't
// leak memory if one of the out-of-memory conditions trigger.
TEST(Allocation, CreateInstanceDeviceWithDXGIDriverIntentionalAllocFail) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("error,warn")};
    env.add_icd(TEST_ICD_PATH_VERSION_6, ManifestOptions{}.set_discovery_type(ManifestDiscoveryType::null_dir));
    env.add_icd(TEST_ICD_PATH_VERSION_2);

    for (uint32_t i = 0; i < 2; i++) {
        auto& driver = env.get_test_icd(i);
        driver.add_and_get_physical_device(std::string("physical_device_") + std::to_string(i))
            .add_queue_family_properties({{VK_QUEUE_GRAPHICS_BIT, 1, 0, {1, 1, 1}}, false});
    }

    const char* layer_name = "VK_LAYER_implicit";
    env.add_implicit_layer({}, ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                             .set_name(layer_name)
                                                             .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                             .set_disable_environment("DISABLE_ENV")));
    env.get_test_layer().set_do_spurious_allocations_in_create_instance(true).set_do_spurious_allocations_in_create_device(true);

    auto& known_driver = known_driver_list.at(2);  // which drive this test pretends to be
    DXGI_ADAPTER_DESC1 desc1{};
    desc1.VendorId = known_driver.vendor_id;
    desc1.AdapterLuid = _LUID{10, 1000};
    env.platform_shim->add_dxgi_adapter(GpuType::discrete, desc1);
    env.get_test_icd(0).set_adapterLUID(desc1.AdapterLuid);

    env.platform_shim->add_d3dkmt_adapter(D3DKMT_Adapter{0, _LUID{10, 1000}}.add_driver_manifest_path(env.get_icd_manifest_path()));

    size_t fail_index = 0;
    VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
    while (result == VK_ERROR_OUT_OF_HOST_MEMORY && fail_index <= 10000) {
        MemoryTracker tracker({false, 0, true, fail_index});
        fail_index++;  // applies to the next loop

        VkInstance instance;
        InstanceCreateInfo inst_create_info{};
        result = env.vulkan_functions.vkCreateInstance(inst_create_info.get(), tracker.get(), &instance);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            ASSERT_TRUE(tracker.empty());
            continue;
        }

        uint32_t physical_count = 2;
        uint32_t returned_physical_count = 0;
        result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, nullptr);
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_INCOMPLETE) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            continue;
        }
        ASSERT_EQ(physical_count, returned_physical_count);

        std::array<VkPhysicalDevice, 2> physical_devices;
        result = env.vulkan_functions.vkEnumeratePhysicalDevices(instance, &returned_physical_count, physical_devices.data());
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_INCOMPLETE) {
            env.vulkan_functions.vkDestroyInstance(instance, tracker.get());
            ASSERT_TRUE(tracker.empty());
            continue;
        }
        ASSERT_EQ(physical_count, returned_physical_count);

        std::array<VkDevice, 2> devices;
        for (uint32_t i = 0; i < returned_physical_count; i++) {
            uint32_t family_count = 1;
            uint32_t returned_family_count = 0;
            env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &returned_family_count, nullptr);
            ASSERT_EQ(returned_family_count, family_count);

            VkQueueFamilyProperties family;
            env.vulkan_functions.vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &returned_family_count, &family);
            ASSERT_EQ(returned_family_count, family_count);
            ASSERT_EQ(family.queueFlags, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT));
            ASSERT_EQ(family.queueCount, family_count);
            ASSERT_EQ(family.timestampValidBits, 0U);

            DeviceCreateInfo dev_create_info;
            dev_create_info.add_device_queue(DeviceQueueCreateInfo{}.add_priority(0.0f));

            result = env.vulkan_functions.vkCreateDevice(physical_devices[i], dev_create_info.get(), tracker.get(), &devices[i]);
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                devices[i] = VK_NULL_HANDLE;
            } else {
                VkQueue queue;
                env.vulkan_functions.vkGetDeviceQueue(devices[i], 0, 0, &queue);
            }
        }
        for (uint32_t i = 0; i < returned_physical_count; i++) {
            if (devices[i] != VK_NULL_HANDLE) {
                env.vulkan_functions.vkDestroyDevice(devices[i], tracker.get());
            }
        }
        env.vulkan_functions.vkDestroyInstance(instance, tracker.get());

        ASSERT_TRUE(tracker.empty());
    }
}
#endif
