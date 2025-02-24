/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "chassis/validation_object.h"
#include "utils/hash_vk_types.h"
#include "state_tracker/video_session_state.h"
#include "chassis/dispatch_object.h"
#include "generated/device_features.h"
#include "generated/dispatch_functions.h"
#include "error_message/logging.h"
#include "containers/custom_containers.h"
#include "utils/android_ndk_types.h"
#include "containers/range_vector.h"
#include <vulkan/utility/vk_struct_helper.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace vvl {
struct AllocateDescriptorSetsData;
class Fence;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class DescriptorUpdateTemplate;
class Queue;
class Semaphore;
class Buffer;
class BufferView;
class Sampler;
class SamplerYcbcrConversion;
class Framebuffer;
class RenderPass;
class PipelineCache;
class Surface;
class PhysicalDevice;
class DisplayMode;
class Event;
class PipelineLayout;
class Image;
class ImageView;
class Swapchain;
struct SwapchainImage;
class CommandPool;
class CommandBuffer;
class Pipeline;
class DeviceMemory;
class AccelerationStructureNV;
class AccelerationStructureKHR;
class IndirectExecutionSet;
class IndirectCommandsLayout;
class QueryPool;
struct DedicatedBinding;
struct ShaderModule;
struct ShaderObject;
}  // namespace vvl

namespace chassis {
struct CreateShaderModule;
}  // namespace chassis

namespace spirv {
struct StatelessData;
}  // namespace spirv

#define VALSTATETRACK_MAP_AND_TRAITS(handle_type, state_type, map_member)               \
    vvl::concurrent_unordered_map<handle_type, std::shared_ptr<state_type>> map_member; \
    template <typename Dummy>                                                           \
    struct MapTraits<state_type, Dummy> {                                               \
        static constexpr bool kInstanceScope = false;                                   \
        using MapType = decltype(map_member);                                           \
        static MapType vvl::Device::*Map() { return &vvl::Device::map_member; }         \
    };

#define VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(handle_type, state_type, map_member) \
    vvl::concurrent_unordered_map<handle_type, std::shared_ptr<state_type>> map_member;  \
    template <typename Dummy>                                                            \
    struct MapTraits<state_type, Dummy> {                                                \
        static constexpr bool kInstanceScope = false;                                    \
        using MapType = decltype(map_member);                                            \
        static MapType vvl::Instance::*Map() { return &vvl::Instance::map_member; }      \
    };

namespace state_object {
// Traits for State function resolution.  Specializations defined in the macros below.
template <typename StateType>
struct Traits {};

// Helper object to make the macros simpler
// HandleType_ is a vulkan handle type
// StateType_ is the type of the corresponding state object, which may be a derived type
// BaseType_ is the type of object stored in the state tracker, there
//            *must* be a corresponding map using this type
template <typename HandleType_, typename StateType_, typename BaseType_ = StateType_>
struct TraitsBase {
    using StateType = StateType_;
    using BaseType = BaseType_;
    using HandleType = HandleType_;
    using SharedType = std::shared_ptr<StateType>;
    using ConstSharedType = std::shared_ptr<const StateType>;
    using ReadLockedType = LockedSharedPtr<const StateType, ReadLockGuard>;
    using WriteLockedType = LockedSharedPtr<StateType, WriteLockGuard>;
};
}  // namespace state_object

#define VALSTATETRACK_STATE_OBJECT(handle_type, state_type)                    \
    namespace state_object {                                                   \
    template <>                                                                \
    struct Traits<state_type> : public TraitsBase<handle_type, state_type> {}; \
    }

#define VALSTATETRACK_DERIVED_STATE_OBJECT(handle_type, state_type, base_type)            \
    namespace state_object {                                                              \
    template <>                                                                           \
    struct Traits<state_type> : public TraitsBase<handle_type, state_type, base_type> {}; \
    }

VALSTATETRACK_STATE_OBJECT(VkQueue, vvl::Queue)
VALSTATETRACK_STATE_OBJECT(VkAccelerationStructureNV, vvl::AccelerationStructureNV)
VALSTATETRACK_STATE_OBJECT(VkRenderPass, vvl::RenderPass)
VALSTATETRACK_STATE_OBJECT(VkDescriptorSetLayout, vvl::DescriptorSetLayout)
VALSTATETRACK_STATE_OBJECT(VkSampler, vvl::Sampler)
VALSTATETRACK_STATE_OBJECT(VkImageView, vvl::ImageView)
VALSTATETRACK_STATE_OBJECT(VkImage, vvl::Image)
VALSTATETRACK_STATE_OBJECT(VkBufferView, vvl::BufferView)
VALSTATETRACK_STATE_OBJECT(VkBuffer, vvl::Buffer)
VALSTATETRACK_STATE_OBJECT(VkPipelineCache, vvl::PipelineCache)
VALSTATETRACK_STATE_OBJECT(VkPipeline, vvl::Pipeline)
VALSTATETRACK_STATE_OBJECT(VkShaderEXT, vvl::ShaderObject)
VALSTATETRACK_STATE_OBJECT(VkDeviceMemory, vvl::DeviceMemory)
VALSTATETRACK_STATE_OBJECT(VkFramebuffer, vvl::Framebuffer)
VALSTATETRACK_STATE_OBJECT(VkShaderModule, vvl::ShaderModule)
VALSTATETRACK_STATE_OBJECT(VkDescriptorUpdateTemplate, vvl::DescriptorUpdateTemplate)
VALSTATETRACK_STATE_OBJECT(VkSwapchainKHR, vvl::Swapchain)
VALSTATETRACK_STATE_OBJECT(VkDescriptorPool, vvl::DescriptorPool)
VALSTATETRACK_STATE_OBJECT(VkDescriptorSet, vvl::DescriptorSet)
VALSTATETRACK_STATE_OBJECT(VkCommandBuffer, vvl::CommandBuffer)
VALSTATETRACK_STATE_OBJECT(VkCommandPool, vvl::CommandPool)
VALSTATETRACK_STATE_OBJECT(VkPipelineLayout, vvl::PipelineLayout)
VALSTATETRACK_STATE_OBJECT(VkFence, vvl::Fence)
VALSTATETRACK_STATE_OBJECT(VkQueryPool, vvl::QueryPool)
VALSTATETRACK_STATE_OBJECT(VkSemaphore, vvl::Semaphore)
VALSTATETRACK_STATE_OBJECT(VkEvent, vvl::Event)
VALSTATETRACK_STATE_OBJECT(VkSamplerYcbcrConversion, vvl::SamplerYcbcrConversion)
VALSTATETRACK_STATE_OBJECT(VkVideoSessionKHR, vvl::VideoSession)
VALSTATETRACK_STATE_OBJECT(VkVideoSessionParametersKHR, vvl::VideoSessionParameters)
VALSTATETRACK_STATE_OBJECT(VkAccelerationStructureKHR, vvl::AccelerationStructureKHR)
VALSTATETRACK_STATE_OBJECT(VkSurfaceKHR, vvl::Surface)
VALSTATETRACK_STATE_OBJECT(VkDisplayModeKHR, vvl::DisplayMode)
VALSTATETRACK_STATE_OBJECT(VkPhysicalDevice, vvl::PhysicalDevice)
VALSTATETRACK_STATE_OBJECT(VkIndirectExecutionSetEXT, vvl::IndirectExecutionSet)
VALSTATETRACK_STATE_OBJECT(VkIndirectCommandsLayoutEXT, vvl::IndirectCommandsLayout)

namespace vvl {
class Instance : public vvl::base::Instance {
    using Func = vvl::Func;
    using BaseClass = vvl::base::Instance;

  public:
    Instance(vvl::dispatch::Instance* dispatch, LayerObjectTypeId type) : BaseClass(dispatch, type) {}

    virtual std::shared_ptr<vvl::PhysicalDevice> CreatePhysicalDeviceState(VkPhysicalDevice handle);
    void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkInstance* pInstance, const RecordObject& record_obj) override;
    void RecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCounters(VkPhysicalDevice physicalDevice,
                                                                          uint32_t queueFamilyIndex, uint32_t* pCounterCount,
                                                                          VkPerformanceCounterKHR* pCounters);
    void RecordGetPhysicalDeviceDisplayPlanePropertiesState(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                            void* pProperties);
    void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                  VkDisplayPlanePropertiesKHR* pProperties,
                                                                  const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                                   VkDisplayPlaneProperties2KHR* pProperties,
                                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                              VkQueueFamilyProperties* pQueueFamilyProperties,
                                                              const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                  uint32_t* pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                  const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               VkSurfaceCapabilitiesKHR* pSurfaceCapabilities,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                                VkSurfaceCapabilities2KHR* pSurfaceCapabilities,
                                                                const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                VkSurfaceCapabilities2EXT* pSurfaceCapabilities,
                                                                const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                          uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                           const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo,
                                                           uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats,
                                                           const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                               uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                          VkSurfaceKHR surface, VkBool32* pSupported,
                                                          const RecordObject& record_obj) override;

    void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, const RecordObject& record_obj,
                                   vku::safe_VkDeviceCreateInfo* modified_create_info) override;
    void PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                    const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                    const RecordObject& record_obj) override;

    void PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                            const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                            VkDisplayModeKHR* pMode, const RecordObject& record_obj) override;

    VkFormatFeatureFlags2KHR GetImageFormatFeatures(VkPhysicalDevice physical_device, bool has_format_feature2,
                                                    bool has_drm_modifiers, VkDevice device, VkImage image, VkFormat format,
                                                    VkImageTiling tiling);
    void RecordVulkanSurface(VkSurfaceKHR* pSurface);
    void PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                    const RecordObject& record_obj) override;
    void PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_FUCHSIA
    void PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance, const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                     const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_IOS_MVK
    void PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
    void PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_MACOS_MVK
#ifdef VK_USE_PLATFORM_METAL_EXT
    void PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                             const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    void PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                               const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    void PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                           const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    void PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                            const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    void PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                              const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    void PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface,
                                                const RecordObject& record_obj) override;

    template <typename State, typename HandleType = typename state_object::Traits<State>::HandleType>
    void Add(std::shared_ptr<State>&& state_object) {
        auto& map = GetStateMap<State>();
        auto handle = state_object->Handle().template Cast<HandleType>();
        state_object->SetId(object_id_++);
        // Finish setting up the object node tree, which cannot be done from the state object contructors
        // due to use of shared_from_this()
        state_object->LinkChildNodes();
        map.insert_or_assign(handle, std::move(state_object));
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    void Destroy(typename Traits::HandleType handle) {
        auto& map = GetStateMap<State>();
        auto iter = map.pop(handle);
        if (iter != map.end()) {
            iter->second->Destroy();
        }
    }

    template <typename State>
    size_t Count() const {
        return GetStateMap<State>().size();
    }

    template <typename State, typename Fn>
    void ForEachShared(Fn&& fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            fn(entry.second);
        }
    }

    template <typename State>
    void ForEach(std::function<void(const State& s)> fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            fn(*entry.second);
        }
    }

    template <typename State>
    bool AnyOf(std::function<bool(const State& s)> fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            if (fn(*entry.second)) {
                return true;
            }
        }
        return false;
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    typename Traits::SharedType Get(typename Traits::HandleType handle) {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        // NOTE: vvl::concurrent_unordered_map::find() makes a copy of the value, so it is safe to move out.
        // But this will break everything, when switching to a different map type.
        return std::static_pointer_cast<State>(std::move(found_it->second));
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    typename Traits::ConstSharedType Get(typename Traits::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return std::static_pointer_cast<State>(std::move(found_it->second));
    }

    // GetRead() and GetWrite() return an already locked state object. Currently this is only supported by
    // vvl::CommandBuffer, because it has public ReadLock() and WriteLock() methods.
    // NOTE: Calling base class hook methods with a vvl::CommandBuffer lock held will lead to deadlock. Instead,
    // call the base class hook method before getting/locking the command buffer state for processing in the
    // derived class method.
    template <typename State, typename Traits = typename state_object::Traits<State>,
              typename ReadLockedType = typename Traits::ReadLockedType>
    ReadLockedType GetRead(typename Traits::HandleType handle) const {
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->ReadLock();
            return ReadLockedType(std::move(ptr), std::move(guard));
        } else {
            return ReadLockedType();
        }
    }

    template <typename State, typename Traits = state_object::Traits<State>,
              typename WriteLockedType = typename Traits::WriteLockedType>
    WriteLockedType GetWrite(typename Traits::HandleType handle) {
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->WriteLock();
            return WriteLockedType(std::move(ptr), std::move(guard));
        } else {
            return WriteLockedType();
        }
    }

    // When needing to share ownership, control over constness of access with another object (i.e. adding references while
    // not modifying the contents of the state object)
    template <typename State, typename Traits = state_object::Traits<State>>
    typename Traits::SharedType GetConstCastShared(typename Traits::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return found_it->second;
    }

#ifdef VK_USE_PLATFORM_METAL_EXT
    std::vector<VkExportMetalObjectTypeFlagBitsEXT> export_metal_flags;
#endif  // VK_USE_PLATFORM_METAL_EXT
  private:
    // NOTE: The Dummy argument allows for *partial* specialization at class scope, as full specialization at class scope
    //       isn't supported until C++17.  Since the Dummy has a default all instantiations of the template can ignore it, but all
    //       specializations of the template must list it (and not give it a default).
    // These must be declared at the same access level as the map declarations (below).
    template <typename State, typename Dummy = int>
    struct MapTraits {};

    template <typename State, typename BaseType = typename state_object::Traits<State>::BaseType,
              typename MapTraits = MapTraits<BaseType>>
    typename MapTraits::MapType& GetStateMap() {
        auto map_member = MapTraits::Map();
        return this->*map_member;
    }
    template <typename State, typename BaseType = typename state_object::Traits<State>::BaseType,
              typename MapTraits = MapTraits<BaseType>>
    const typename MapTraits::MapType& GetStateMap() const {
        auto map_member = MapTraits::Map();
        return this->*map_member;
    }

    std::atomic<uint32_t> object_id_{1};  // 0 is an invalid id

    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkSurfaceKHR, vvl::Surface, surface_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkDisplayModeKHR, vvl::DisplayMode, display_mode_map_)
    VALSTATETRACK_MAP_AND_TRAITS_INSTANCE_SCOPE(VkPhysicalDevice, vvl::PhysicalDevice, physical_device_map_)
};

class Device : public vvl::base::Device {
    using Func = vvl::Func;
    using BaseClass = vvl::base::Device;

  private:
    // NOTE: The Dummy argument allows for *partial* specialization at class scope, as full specialization at class scope
    //       isn't supported until C++17.  Since the Dummy has a default all instantiations of the template can ignore it, but all
    //       specializations of the template must list it (and not give it a default).
    // These must be declared at the same access level as the map declarations (below).
    template <typename State, typename Dummy = int>
    struct MapTraits {};

    template <typename State, typename BaseType = typename state_object::Traits<State>::BaseType,
              typename MapTraits = MapTraits<BaseType>>
    typename MapTraits::MapType& GetStateMap() {
        auto map_member = MapTraits::Map();
        return this->*map_member;
    }
    template <typename State, typename BaseType = typename state_object::Traits<State>::BaseType,
              typename MapTraits = MapTraits<BaseType>>
    const typename MapTraits::MapType& GetStateMap() const {
        auto map_member = MapTraits::Map();
        return this->*map_member;
    }

    // Helper to clean up the state object maps in the correct order
    void DestroyObjectMaps();

  public:
    Device(vvl::dispatch::Device* dev, Instance* instance, LayerObjectTypeId type)
        : BaseClass(dev, instance, type), instance_state(instance) {
        physical_device_state = instance_state->Get<vvl::PhysicalDevice>(physical_device).get();
    }
    ~Device();

    template <typename State, typename HandleType = typename state_object::Traits<State>::HandleType>
    void Add(std::shared_ptr<State>&& state_object) {
        auto& map = GetStateMap<State>();
        auto handle = state_object->Handle().template Cast<HandleType>();
        state_object->SetId(object_id_++);
        // Finish setting up the object node tree, which cannot be done from the state object contructors
        // due to use of shared_from_this()
        state_object->LinkChildNodes();
        map.insert_or_assign(handle, std::move(state_object));
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    void Destroy(typename Traits::HandleType handle) {
        auto& map = GetStateMap<State>();
        auto iter = map.pop(handle);
        if (iter != map.end()) {
            iter->second->Destroy();
        }
    }

    template <typename State>
    size_t Count() const {
        return GetStateMap<State>().size();
    }

    template <typename State, typename Fn>
    void ForEachShared(Fn&& fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            fn(entry.second);
        }
    }

    template <typename State>
    void ForEach(std::function<void(const State& s)> fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            fn(*entry.second);
        }
    }

    template <typename State>
    bool AnyOf(std::function<bool(const State& s)> fn) const {
        const auto& map = GetStateMap<State>();
        for (const auto& entry : map.snapshot()) {
            if (fn(*entry.second)) {
                return true;
            }
        }
        return false;
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    typename Traits::SharedType Get(typename Traits::HandleType handle) {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        // NOTE: vvl::concurrent_unordered_map::find() makes a copy of the value, so it is safe to move out.
        // But this will break everything, when switching to a different map type.
        return std::static_pointer_cast<State>(std::move(found_it->second));
    }

    template <typename State, typename Traits = typename state_object::Traits<State>>
    typename Traits::ConstSharedType Get(typename Traits::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return std::static_pointer_cast<State>(std::move(found_it->second));
    }

    // GetRead() and GetWrite() return an already locked state object. Currently this is only supported by
    // vvl::CommandBuffer, because it has public ReadLock() and WriteLock() methods.
    // NOTE: Calling base class hook methods with a vvl::CommandBuffer lock held will lead to deadlock. Instead,
    // call the base class hook method before getting/locking the command buffer state for processing in the
    // derived class method.
    template <typename State, typename Traits = typename state_object::Traits<State>,
              typename ReadLockedType = typename Traits::ReadLockedType>
    ReadLockedType GetRead(typename Traits::HandleType handle) const {
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->ReadLock();
            return ReadLockedType(std::move(ptr), std::move(guard));
        } else {
            return ReadLockedType();
        }
    }

    template <typename State, typename Traits = state_object::Traits<State>,
              typename WriteLockedType = typename Traits::WriteLockedType>
    WriteLockedType GetWrite(typename Traits::HandleType handle) {
        auto ptr = Get<State>(handle);
        if (ptr) {
            auto guard = ptr->WriteLock();
            return WriteLockedType(std::move(ptr), std::move(guard));
        } else {
            return WriteLockedType();
        }
    }

    // When needing to share ownership, control over constness of access with another object (i.e. adding references while
    // not modifying the contents of the state tracker)
    template <typename State, typename Traits = state_object::Traits<State>>
    typename Traits::SharedType GetConstCastShared(typename Traits::HandleType handle) const {
        const auto& map = GetStateMap<State>();
        const auto found_it = map.find(handle);
        if (found_it == map.end()) {
            return nullptr;
        }
        return found_it->second;
    }

    // From the spec:
    // If multiple VkBuffer objects are bound to overlapping ranges of VkDeviceMemory, implementations may return
    // address ranges which overlap. In this case, it is ambiguous which VkBuffer is associated with any given
    // device address. For purposes of valid usage, if multiple VkBuffer objects can be attributed to
    // a device address, a VkBuffer is selected such that valid usage passes, if it exists.
    // Regarding using raw pointers instead of shared: The reason is performance, because arrays of vvl::Buffer* are used, it is
    // more efficient to store them using raw pointers. It is safe to do so (at time of writing) because those raw pointers come
    // from shared ones created when the buffer is first recorded, and they are removed from buffer_address_map_ at BufferDestroy
    // time
    vvl::span<vvl::Buffer*> GetBuffersByAddress(VkDeviceAddress address) {
        ReadLockGuard guard(buffer_address_lock_);
        auto found_it = buffer_address_map_.find(address);
        if (found_it == buffer_address_map_.end()) {
            return vvl::make_span<vvl::Buffer*>(nullptr, static_cast<size_t>(0));
        }
        return found_it->second;
    }

    vvl::span<vvl::Buffer* const> GetBuffersByAddress(VkDeviceAddress address) const {
        ReadLockGuard guard(buffer_address_lock_);
        auto found_it = buffer_address_map_.find(address);
        if (found_it == buffer_address_map_.end()) {
            return vvl::make_span<vvl::Buffer* const>(nullptr, static_cast<size_t>(0));
        }
        return found_it->second;
    }

    // Return a count pair, {written addresses count, total address ranges count}
    using BufferAddressRange = sparse_container::range<VkDeviceAddress>;
    [[nodiscard]] std::pair<size_t, size_t> GetBufferAddressRanges(BufferAddressRange* ranges, size_t ranges_size) const {
        ReadLockGuard guard(buffer_address_lock_);

        size_t written_count = 0;
        for (const auto& [address_range, buffers] : buffer_address_map_) {
            if (written_count == ranges_size) {
                break;
            }
            ranges[written_count++] = address_range;
        }
        return {written_count, buffer_address_map_.size()};
    }

    VkDeviceSize AllocFakeMemory(VkDeviceSize size) { return fake_memory.Alloc(size); }
    void FreeFakeMemory(VkDeviceSize address) { fake_memory.Free(address); }

    void PostCallRecordGetAccelerationStructureMemoryRequirementsNV(VkDevice device,
                                                                    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
                                                                    VkMemoryRequirements2* pMemoryRequirements,
                                                                    const RecordObject& record_obj) override;

    void PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                            uint32_t* pMemoryRequirementsCount,
                                                            VkVideoSessionMemoryRequirementsKHR* pMemoryRequirements,
                                                            const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::Queue> CreateQueue(VkQueue handle, uint32_t queue_family_index, uint32_t queue_index,
                                                    VkDeviceQueueCreateFlags flags,
                                                    const VkQueueFamilyProperties& queueFamilyProperties);

    void PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue,
                                      const RecordObject& record_obj) override;
    void PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue,
                                       const RecordObject& record_obj) override;
    void PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd,
                                     const RecordObject& record_obj) override;
    void PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                              HANDLE* pHandle, const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                   VkMemoryRequirements2* pMemoryRequirements,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo,
                                                      VkMemoryRequirements2* pMemoryRequirements,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount,
                                                        VkSparseImageMemoryRequirements* pSparseMemoryRequirements,
                                                        const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                         uint32_t* pSparseMemoryRequirementCount,
                                                         VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo,
                                                            uint32_t* pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements2* pSparseMemoryRequirements,
                                                            const RecordObject& record_obj) override;
    void PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd,
                                         const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                                  HANDLE* pHandle, const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo,
                                               HANDLE* pHandle, const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd,
                                      const RecordObject& record_obj) override;
    void PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo,
                                        const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                 const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo,
                                                 const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo,
                                            const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                     const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo,
                                                     const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_WIN32_KHR
    void PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                         const RecordObject& record_obj) override;
    void PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo,
                                      const RecordObject& record_obj) override;

    // Create/Destroy/Bind
    void PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                         const VkBindAccelerationStructureMemoryInfoNV* pBindInfos,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                 uint32_t bindSessionMemoryInfoCount,
                                                 const VkBindVideoSessionMemoryInfoKHR* pBindSessionMemoryInfos,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                        const RecordObject& record_obj) override;
    void PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                         const RecordObject& record_obj) override;
    void PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos,
                                            const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                       const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                        const RecordObject& record_obj) override;
    void PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos,
                                           const RecordObject& record_obj) override;

    virtual void PostCreateDevice(const VkDeviceCreateInfo* pCreateInfo, const Location& loc);

    void PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::AccelerationStructureNV> CreateAccelerationStructureState(
        VkAccelerationStructureNV handle, const VkAccelerationStructureCreateInfoNV* create_info);
    void PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkAccelerationStructureNV* pAccelerationStructure,
                                                     const RecordObject& record_obj) override;
    void PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::AccelerationStructureKHR> CreateAccelerationStructureState(
        VkAccelerationStructureKHR handle, const VkAccelerationStructureCreateInfoKHR* create_info,
        std::shared_ptr<vvl::Buffer>&& buf_state);
    void PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkAccelerationStructureKHR* pAccelerationStructure,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                      const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                      const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                      const RecordObject& record_obj) override;
    void RecordDeviceAccelerationStructureBuildInfo(vvl::CommandBuffer& cb_state,
                                                    const VkAccelerationStructureBuildGeometryInfoKHR& info);
    void PostCallRecordCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                         const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                         const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos,
                                                         const RecordObject& record_obj) override;

    void PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                 const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                                                 const VkDeviceAddress* pIndirectDeviceAddresses,
                                                                 const uint32_t* pIndirectStrides,
                                                                 const uint32_t* const* ppMaxPrimitiveCounts,
                                                                 const RecordObject& record_obj) override;
    void PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::Buffer> CreateBufferState(VkBuffer handle, const VkBufferCreateInfo* create_info);
    void PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                    VkBuffer* pBuffer, const RecordObject& record_obj) override;
    void PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator,
                                    const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::BufferView> CreateBufferViewState(const std::shared_ptr<vvl::Buffer>& buffer, VkBufferView handle,
                                                                   const VkBufferViewCreateInfo* create_info,
                                                                   VkFormatFeatureFlags2KHR format_features);
    void PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkBufferView* pView,
                                        const RecordObject& record_obj) override;
    void PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) override;
    virtual std::shared_ptr<vvl::CommandPool> CreateCommandPoolState(VkCommandPool handle,
                                                                     const VkCommandPoolCreateInfo* create_info);
    void PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool,
                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkEvent* pEvent, const RecordObject& record_obj) override;
    void PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::DescriptorPool> CreateDescriptorPoolState(VkDescriptorPool handle,
                                                                           const VkDescriptorPoolCreateInfo* create_info);
    virtual std::shared_ptr<vvl::DescriptorSet> CreateDescriptorSet(VkDescriptorSet handle, vvl::DescriptorPool* pool,
                                                                    const std::shared_ptr<vvl::DescriptorSetLayout const>& layout,
                                                                    uint32_t variable_count);

    void PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool,
                                            const RecordObject& record_obj) override;
    void PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                            const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;
    void PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                          const RecordObject& record_obj) override;
    void PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                        const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::Pipeline> CreateComputePipelineState(const VkComputePipelineCreateInfo* create_info,
                                                                      std::shared_ptr<const vvl::PipelineCache> pipeline_cache,
                                                                      std::shared_ptr<const vvl::PipelineLayout>&& layout,
                                                                      spirv::StatelessData* stateless_data) const;
    bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                               chassis::CreateComputePipelines& chassis_state) const override;
    void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                              const VkComputePipelineCreateInfo* pCreateInfos,
                                              const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                              const RecordObject& record_obj, PipelineStates& pipeline_states,
                                              chassis::CreateComputePipelines& chassis_state) override;
    void PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                           const RecordObject& record_obj) override;
    bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                               VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj,
                                               vvl::AllocateDescriptorSetsData& ads_state_data) const override;
    void PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                 const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;
    void PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate,
                                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      const RecordObject& record_obj) override;
    void PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkFence* pFence, const RecordObject& record_obj) override;
    void PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject& record_obj) override;
    void PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences,
                                   const RecordObject& record_obj) override;
    void PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator,
                                         const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::PipelineCache> CreatePipelineCacheState(VkPipelineCache handle,
                                                                         const VkPipelineCacheCreateInfo* create_info) const;
    void PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo,
                                           const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache,
                                           const RecordObject& record_obj) override;
    void PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator,
                                           const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::Pipeline> CreateGraphicsPipelineState(
        const VkGraphicsPipelineCreateInfo* create_info, std::shared_ptr<const vvl::PipelineCache> pipeline_cache,
        std::shared_ptr<const vvl::RenderPass>&& render_pass, std::shared_ptr<const vvl::PipelineLayout>&& layout,
        spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]) const;

    bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                chassis::CreateGraphicsPipelines& chassis_state) const override;
    void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                               const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                               const RecordObject& record_obj, PipelineStates& pipeline_states,
                                               chassis::CreateGraphicsPipelines& chassis_state) override;

    virtual std::shared_ptr<vvl::Image> CreateImageState(VkImage handle, const VkImageCreateInfo* create_info,
                                                         VkFormatFeatureFlags2 features);
    virtual std::shared_ptr<vvl::Image> CreateImageState(VkImage handle, const VkImageCreateInfo* create_info,
                                                         VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                         VkFormatFeatureFlags2 features);
    void PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                   VkImage* pImage, const RecordObject& record_obj) override;
    void PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator,
                                   const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::ImageView> CreateImageViewState(const std::shared_ptr<vvl::Image>& image_state, VkImageView handle,
                                                                 const VkImageViewCreateInfo* create_info,
                                                                 VkFormatFeatureFlags2 format_features,
                                                                 const VkFilterCubicImageViewImageFormatPropertiesEXT& cubic_props);
    void PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkImageView* pView,
                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) override;

    void PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                        const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                        const RecordObject& record_obj, chassis::ShaderObject& chassis_state) override;
    void PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                         const VkShaderEXT* pShaders, const RecordObject& record_obj) override;
    void PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                            const RecordObject& record_obj) override;
    void PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                            const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;
    void PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool,
                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) override;
    void PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                         const RecordObject& record_obj) override;
    void PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                      const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::Pipeline> CreateRayTracingPipelineState(const VkRayTracingPipelineCreateInfoNV* create_info,
                                                                         std::shared_ptr<const vvl::PipelineCache> pipeline_cache,
                                                                         std::shared_ptr<const vvl::PipelineLayout>&& layout,
                                                                         spirv::StatelessData* stateless_data) const;
    bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                    chassis::CreateRayTracingPipelinesNV& chassis_state) const override;
    void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                   const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                   chassis::CreateRayTracingPipelinesNV& chassis_state) override;
    virtual std::shared_ptr<vvl::Pipeline> CreateRayTracingPipelineState(const VkRayTracingPipelineCreateInfoKHR* create_info,
                                                                         std::shared_ptr<const vvl::PipelineCache> pipeline_cache,
                                                                         std::shared_ptr<const vvl::PipelineLayout>&& layout,
                                                                         spirv::StatelessData* stateless_data) const;
    bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                     VkPipelineCache pipelineCache, uint32_t count,
                                                     const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                     const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                     const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                     chassis::CreateRayTracingPipelinesKHR& chassis_state) const override;
    void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                    const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                    std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) override;
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                        const RecordObject& record_obj) override;
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                            const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const RecordObject& record_obj) override;
    void PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator,
                                        const RecordObject& record_obj) override;
    void PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession,
                                             const RecordObject& record_obj) override;
    void PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                             const VkAllocationCallbacks* pAllocator, const RecordObject& record_obj) override;
    void PostCallRecordCreateVideoSessionParametersKHR(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkVideoSessionParametersKHR* pVideoSessionParameters,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                       const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo,
                                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::Sampler> CreateSamplerState(VkSampler handle, const VkSamplerCreateInfo* create_info);
    void PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                                     const VkAllocationCallbacks* pAllocator, VkSampler* pSampler,
                                     const RecordObject& record_obj) override;
    void PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator,
                                     const RecordObject& record_obj) override;
    void PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkSamplerYcbcrConversion* pYcbcrConversion,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkSamplerYcbcrConversion* pYcbcrConversion,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        const RecordObject& record_obj) override;
    void PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                                       const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore,
                                       const RecordObject& record_obj) override;
    void PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator,
                                       const RecordObject& record_obj) override;

    void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                         const RecordObject& record_obj, chassis::CreateShaderModule& chassis_state) override;
    void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos,
                                       const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders,
                                       const RecordObject& record_obj, chassis::ShaderObject& chassis_state) override;

    void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule,
                                          const RecordObject& record_obj, chassis::CreateShaderModule& chassis_state) override;
    void PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) override;
    void PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                 const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                 const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain,
                                          const RecordObject& record_obj) override;
    void PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator,
                                          const RecordObject& record_obj) override;
    // CommandBuffer/Queue Control
    void PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject& record_obj) override;
    void PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                      const RecordObject& record_obj) override;
    void PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence,
                                       const RecordObject& record_obj) override;
    void PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo,
                                       const RecordObject& record_obj) override;
    void PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT* pReleaseInfo,
                                                 const RecordObject& record_obj) override;
    void PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                  const RecordObject& record_obj) override;
    void PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                   const RecordObject& record_obj) override;
    void PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject& record_obj) override;
    void PreCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject& record_obj) override;
    void PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll,
                                     uint64_t timeout, const RecordObject& record_obj) override;
    void PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                     const RecordObject& record_obj) override;
    void PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                        const RecordObject& record_obj) override;
    void PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                      const RecordObject& record_obj) override;
    void PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout,
                                         const RecordObject& record_obj) override;
    void PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                const RecordObject& record_obj) override;
    void PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t* pValue,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR* pInfo,
                                               const RecordObject& record_obj) override;
    void PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::CommandBuffer> CreateCmdBufferState(VkCommandBuffer handle,
                                                                     const VkCommandBufferAllocateInfo* allocate_info,
                                                                     const vvl::CommandPool* pool);
    // Allocate/Free
    void PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                              VkCommandBuffer* pCommandBuffer, const RecordObject& record_obj) override;
    void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                              VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj,
                                              vvl::AllocateDescriptorSetsData& ads_state) override;
    void PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                      const RecordObject& record_obj) override;
    void PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                         const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) override;
    void PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                         const VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj) override;
    void PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks* pAllocator,
                                 const RecordObject& record_obj) override;

    void PerformUpdateDescriptorSets(uint32_t, const VkWriteDescriptorSet*, uint32_t, const VkCopyDescriptorSet*);

    void PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount,
                                           const VkCopyDescriptorSet* pDescriptorCopies, const RecordObject& record_obj) override;
    void PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                      VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                      const RecordObject& record_obj) override;
    void PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData,
                                                         const RecordObject& record_obj) override;

    virtual std::shared_ptr<vvl::DeviceMemory> CreateDeviceMemoryState(
        VkDeviceMemory handle, const VkMemoryAllocateInfo* allocate_infos, uint64_t fake_address, const VkMemoryType& memory_type,
        const VkMemoryHeap& memory_heap, std::optional<vvl::DedicatedBinding>&& dedicated_binding, uint32_t physical_device_count);

    // Memory mapping
    void PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                 void** ppData, const RecordObject& record_obj) override;
    void PostCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo* pMemoryMapInfo, void** ppData,
                                  const RecordObject& record_obj) override;
    void PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR* pMemoryMapInfo, void** ppData,
                                     const RecordObject& record_obj) override;
    void PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem, const RecordObject& record_obj) override;
    void PreCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo* pMemoryUnmapInfo,
                                   const RecordObject& record_obj) override;
    void PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR* pMemoryUnmapInfo,
                                      const RecordObject& record_obj) override;

    // Recorded Commands
    void PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags,
                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                                               VkQueryControlFlags flags, uint32_t index, const RecordObject& record_obj) override;
    void PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                         VkSubpassContents contents, const RecordObject& record_obj) override;
    void PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR* pRenderingInfo,
                                           const RecordObject& record_obj) override;
    void PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo* pRenderingInfo,
                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                          const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;
    void PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                             const VkSubpassBeginInfo* pSubpassBeginInfo, const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                    uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                    const VkDeviceSize* pCounterBufferOffsets,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                  uint32_t counterBufferCount, const VkBuffer* pCounterBuffers,
                                                  const VkDeviceSize* pCounterBufferOffsets,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                       const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorSets2(VkCommandBuffer commandBuffer, const VkBindDescriptorSetsInfo* pBindDescriptorSetsInfo,
                                             const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                const VkBindDescriptorSetsInfoKHR* pBindDescriptorSetsInfo,
                                                const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                            VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                            const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
                                            const uint32_t* pDynamicOffsets, const RecordObject& record_obj) override;
    void PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                          VkIndexType indexType, const RecordObject& record_obj) override;
    void PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size,
                                             VkIndexType indexType, const RecordObject& record_obj) override;
    void PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                       const RecordObject& record_obj) override;
    void PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                                const RecordObject& record_obj) override;
    void PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                           const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
                                           const RecordObject& record_obj) override;
    void PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter,
                                   const RecordObject& record_obj) override;
    void PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR* pBlitImageInfo,
                                       const RecordObject& record_obj) override;
    void PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo,
                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo,
                                                       VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update,
                                                       VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                       VkBuffer scratch, VkDeviceSize scratchOffset,
                                                       const RecordObject& record_obj) override;
    void PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                         const VkClearColorValue* pColor, uint32_t rangeCount,
                                         const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) override;
    void PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount,
                                                const VkImageSubresourceRange* pRanges, const RecordObject& record_obj) override;
    void PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                const VkVideoCodingControlInfoKHR* pCodingControlInfo,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                      VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode,
                                                      const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                    const VkBufferCopy* pRegions, const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR* pCopyBufferInfo,
                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo,
                                     const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                           VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyBufferToImageInfo2KHR* pCopyBufferToImageInfo,
                                               const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo,
                                            const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                   VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions,
                                   const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR* pCopyImageInfo,
                                       const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo,
                                    const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                           VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions,
                                           const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                               const VkCopyImageToBufferInfo2KHR* pCopyImageToBufferInfo,
                                               const RecordObject& record_obj) override;
    void PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                               uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                                               VkQueryResultFlags flags, const RecordObject& record_obj) override;
    void PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pDecodeInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z,
                                          uint32_t x, uint32_t y, uint32_t z, const RecordObject& record_obj) override;
    void PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z, uint32_t x,
                                       uint32_t y, uint32_t z, const RecordObject& record_obj) override;
    void PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                               uint32_t firstInstance, const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
                                       uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                              const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount,
                                              uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                              uint32_t stride, const RecordObject& record_obj) override;
    void PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                       uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                     uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                              uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                           uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                       uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t drawCount, uint32_t stride, const RecordObject& record_obj) override;
    void PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                          uint32_t groupCountZ, const RecordObject& record_obj) override;
    void PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                      VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                      VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                      VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                      VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                      VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                      uint32_t width, uint32_t height, uint32_t depth, const RecordObject& record_obj) override;
    void PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                       const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                       const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width,
                                       uint32_t height, uint32_t depth, const RecordObject& record_obj) override;
    void PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                                               VkDeviceAddress indirectDeviceAddress, const RecordObject& record_obj) override;
    void PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                      const VkGeneratedCommandsInfoEXT* pGeneratedCommandsInfo,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index,
                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo,
                                            const RecordObject& record_obj) override;
    void PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                         const VkCommandBuffer* pCommandBuffers, const RecordObject& record_obj) override;
    void PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
                                    uint32_t data, const RecordObject& record_obj) override;
    void PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                          const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    void PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                       const VkSubpassEndInfo* pSubpassEndInfo, const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                           const VkWriteDescriptorSet* pDescriptorWrites, const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                              VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                              const VkWriteDescriptorSet* pDescriptorWrites,
                                              const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSet2(VkCommandBuffer commandBuffer, const VkPushDescriptorSetInfo* pPushDescriptorSetInfo,
                                            const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                               const VkPushDescriptorSetInfoKHR* pPushDescriptorSetInfo,
                                               const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetWithTemplate2(
        VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR* pPushDescriptorSetWithTemplateInfo,
        const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetWithTemplate2KHR(
        VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR* pPushDescriptorSetWithTemplateInfo,
        const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                       VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout,
                                                       uint32_t set, const void* pData, const RecordObject& record_obj) override;
    void PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          VkPipelineLayout layout, uint32_t set, const void* pData,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                        uint32_t offset, uint32_t size, const void* pValues,
                                        const RecordObject& record_obj) override;
    void PostCallRecordCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo* pPushConstantsInfo,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfoKHR* pPushConstantsInfo,
                                            const RecordObject& record_obj) override;
    void PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                         uint32_t queryCount, const RecordObject& record_obj) override;
    void PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const VkImageResolve* pRegions, const RecordObject& record_obj) override;
    void PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR* pResolveImageInfo,
                                          const RecordObject& record_obj) override;
    void PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                       float depthBiasSlopeFactor, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT* pDepthBiasInfo,
                                           const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                         const RecordObject& record_obj) override;
    void PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                      uint32_t exclusiveScissorCount, const VkBool32* pExclusiveScissorEnables,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                     const VkRect2D* pScissors, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                      const VkViewport* pViewports, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                          uint32_t viewportCount,
                                                          const VkShadingRatePaletteNV* pShadingRatePalettes,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                       VkDeviceSize dataSize, const void* pData, const RecordObject& record_obj) override;
    void PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                    VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                    uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                         VkQueryPool queryPool, uint32_t slot, const RecordObject& record_obj) override;
    void PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                   uint32_t accelerationStructureCount,
                                                                   const VkAccelerationStructureKHR* pAccelerationStructures,
                                                                   VkQueryType queryType, VkQueryPool queryPool,
                                                                   uint32_t firstQuery, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                const VkViewportWScalingNV* pViewportWScalings,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                            const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
                                            uint32_t vertexAttributeDescriptionCount,
                                            const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                 const VkBool32* pColorWriteEnables, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                             const RecordObject& record_obj) override;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    void PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                         const RecordObject& record_obj) override;
#endif
    void PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                const VkDepthClampRangeEXT* pDepthClampRange,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                           const VkSampleMask* pSampleMask, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                                 const VkBool32* pColorBlendEnables, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorBlendEquationEXT* pColorBlendEquations,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment, uint32_t attachmentCount,
                                               const VkColorComponentFlags* pColorWriteMasks,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                              VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                 float extraPrimitiveOverestimationSize,
                                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorBlendAdvancedEXT* pColorBlendAdvanced,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                      VkLineRasterizationModeEXT lineRasterizationMode,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewportSwizzleNV* pViewportSwizzles,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                      VkCoverageModulationModeNV coverageModulationMode,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageModulationTableEnable,
                                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                       const float* pCoverageModulationTable,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                VkBool32 representativeFragmentTestEnable,
                                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer, VkCoverageReductionModeNV coverageReductionMode,
                                                     const RecordObject& record_obj) override;
    void PreCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                     const RecordObject& record_obj) override;
    void PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo,
                                      const RecordObject& record_obj) override;

    VkFormatFeatureFlags2KHR GetExternalFormatFeaturesANDROID(const void* pNext) const;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    void PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer,
                                                                 VkAndroidHardwareBufferPropertiesANDROID* pProperties,
                                                                 const RecordObject& record_obj) override;
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    // WSI
    void PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                           VkFence fence, uint32_t* pImageIndex, const RecordObject& record_obj) override;
    void PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex,
                                            const RecordObject& record_obj) override;
    // State Utilty functions
    std::vector<std::shared_ptr<const vvl::ImageView>> GetAttachmentViews(const VkRenderPassBeginInfo& rp_begin,
                                                                          const vvl::Framebuffer& fb_state) const;

    VkFormatFeatureFlags2KHR GetPotentialFormatFeatures(VkFormat format) const;
    void PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet,
                                                    const vvl::DescriptorUpdateTemplate* template_state, const void* pData);
    void RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                     VkFence fence, uint32_t* pImageIndex, vvl::Func command);
    virtual std::shared_ptr<vvl::Swapchain> CreateSwapchainState(const VkSwapchainCreateInfoKHR* create_info,
                                                                 VkSwapchainKHR handle);
    void RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain,
                                    std::shared_ptr<vvl::Surface>&& surface_state, vvl::Swapchain* old_swapchain_state);
    void RecordGetDeviceQueueState(uint32_t queue_family_index, uint32_t queue_index, VkDeviceQueueCreateFlags flags,
                                   VkQueue queue);
    void RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type, const Location& loc);
    void RecordGetImageMemoryRequirementsState(VkImage image, const VkImageMemoryRequirementsInfo2* pInfo);
    void RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                    VkSemaphoreImportFlags flags);
    void RecordGetExternalSemaphoreState(vvl::Semaphore& semaphore_state, VkExternalSemaphoreHandleTypeFlagBits handle_type);
    void RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags);
    void RecordMappedMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void** ppData);
    void UpdateBindBufferMemoryState(const VkBindBufferMemoryInfo& bind_info);
    void UpdateBindImageMemoryState(const VkBindImageMemoryInfo& bind_info);
    void UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo*, vvl::AllocateDescriptorSetsData&) const;

    void PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                    const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                       const VkCopyAccelerationStructureInfoKHR* pInfo,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo,
                                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                         const RecordObject& record_obj) override;
    void PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                  const VkViewport* pViewports, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                const VkDeviceSize* pStrides, const RecordObject& record_obj) override;
    void PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                             const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                             const VkDeviceSize* pStrides, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                              const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                            const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                  const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                               const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                          VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                       VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                 uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer, VkDiscardRectangleModeEXT discardRectangleMode,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                 uint32_t customSampleOrderCount,
                                                 const VkCoarseSampleOrderCustomNV* pCustomSampleOrders,
                                                 const RecordObject& record_obj) override;

    void PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                   const RecordObject& record_obj) override;
    void PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                        const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                const RecordObject& record_obj) override;
    void PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                       const RecordObject& record_obj) override;
    void PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize,
                                                    const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                    const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                          const VkRenderingAttachmentLocationInfo* pLocationInfo,
                                                          const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                             const VkRenderingAttachmentLocationInfoKHR* pLocationInfo,
                                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer,
                                                             const VkRenderingInputAttachmentIndexInfo* pLocationInfo,
                                                             const RecordObject& record_obj) override;
    void PostCallRecordCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer commandBuffer,
                                                                const VkRenderingInputAttachmentIndexInfoKHR* pLocationInfo,
                                                                const RecordObject& record_obj) override;

    void PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                            const RecordObject& record_obj) override;

    void PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                          uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                          uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                          uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers,
                                          const RecordObject& record_obj) override;

    void PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR* pDependencyInfo,
                                             const RecordObject& record_obj) override;
    void PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo* pDependencyInfo,
                                          const RecordObject& record_obj) override;

    void PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR* pDependencyInfo,
                                      const RecordObject& record_obj) override;
    void PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo* pDependencyInfo,
                                   const RecordObject& record_obj) override;
    void PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                     const RecordObject& record_obj) override;
    void PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                        const VkDependencyInfoKHR* pDependencyInfos, const RecordObject& record_obj) override;
    void PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                     const VkDependencyInfo* pDependencyInfos, const RecordObject& record_obj) override;
    void PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR stage, VkQueryPool queryPool,
                                             uint32_t query, const RecordObject& record_obj) override;
    void PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                                          uint32_t query, const RecordObject& record_obj) override;
    void PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                      const RecordObject& record_obj) override;
    void PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                   const RecordObject& record_obj) override;
    void PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits, VkFence fence,
                                       const RecordObject& record_obj) override;
    void PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                    const RecordObject& record_obj) override;

    void PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                     VkDeviceSize* pLayoutSizeInBytes, const RecordObject& record_obj) override;
    void PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                       VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                       const uint32_t* pBufferIndices, const VkDeviceSize* pOffsets,
                                                       const RecordObject& record_obj) override;
    void PreCallRecordCmdSetDescriptorBufferOffsets2EXT(VkCommandBuffer commandBuffer,
                                                        const VkSetDescriptorBufferOffsetsInfoEXT* pSetDescriptorBufferOffsetsInfo,
                                                        const RecordObject& record_obj) override;
    void PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                  const VkDescriptorBufferBindingInfoEXT* pBindingInfos,
                                                  const RecordObject& record_obj) override;

    void PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                              const RecordObject& record_obj) override;
    void PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 const RecordObject& record_obj) override;
    void PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo* pInfo,
                                                 const RecordObject& record_obj) override;

    void PostCallRecordGetShaderModuleIdentifierEXT(VkDevice device, VkShaderModule shaderModule,
                                                    VkShaderModuleIdentifierEXT* pIdentifier,
                                                    const RecordObject& record_obj) override;
    void PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                              VkShaderModuleIdentifierEXT* pIdentifier,
                                                              const RecordObject& record_obj) override;
    void PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount, const VkShaderStageFlagBits* pStages,
                                        const VkShaderEXT* pShaders, const RecordObject& record_obj) override;

    void PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                          uint32_t bindingCount, const VkBuffer* pBuffers,
                                                          const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes,
                                                          const RecordObject& record_obj) override;

    void PostCallRecordCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkIndirectExecutionSetEXT* pIndirectExecutionSet,
                                                     const RecordObject& record_obj) override;
    void PostCallRecordDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                      const VkAllocationCallbacks* pAllocator,
                                                      const RecordObject& record_obj) override;
    void PostCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device, const VkIndirectCommandsLayoutCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkIndirectCommandsLayoutEXT* pIndirectCommandsLayout,
                                                       const RecordObject& record_ob) override;
    void PostCallRecordDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                        const VkAllocationCallbacks* pAllocator,
                                                        const RecordObject& record_obj) override;

    inline std::shared_ptr<vvl::ShaderModule> GetShaderModuleStateFromIdentifier(const VkShaderModuleIdentifierEXT& ident) {
        ReadLockGuard guard(shader_identifier_map_lock_);
        if (const auto itr = shader_identifier_map_.find(ident); itr != shader_identifier_map_.cend()) {
            return itr->second;
        }
        return {};
    }

    inline std::shared_ptr<vvl::ShaderModule> GetShaderModuleStateFromIdentifier(
        const VkPipelineShaderStageModuleIdentifierCreateInfoEXT& shader_stage_id) const {
        if (shader_stage_id.pIdentifier) {
            VkShaderModuleIdentifierEXT shader_id = vku::InitStructHelper();
            shader_id.identifierSize = shader_stage_id.identifierSize;
            const uint32_t copy_size = std::min(VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT, shader_stage_id.identifierSize);
            std::copy(shader_stage_id.pIdentifier, shader_stage_id.pIdentifier + copy_size, shader_id.identifier);
            ReadLockGuard guard(shader_identifier_map_lock_);
            if (const auto itr = shader_identifier_map_.find(shader_id); itr != shader_identifier_map_.cend()) {
                return itr->second;
            }
        }
        return {};
    }

    // the VK_EXTERNAL_*_HANDLE_TYPE_OPAQUE_* handles are designed to created/exported in Vulkan, that means we can track the
    // values and compare when re-importing later. While FD and Win32 have differnt handles to access the struct, the information
    // needed is non-platform specific Vulkan values.
    //
    // This also works on the assumption that an FD/Win32 Handle is never allowed to be 2 different primatives (fence, semaphore,
    // memory) at the same time.
    struct ExternalOpaqueInfo {
        // External Memory
        VkDeviceSize allocation_size;
        uint32_t memory_type_index;
        VkBuffer dedicated_buffer;
        VkImage dedicated_image;
        VkDeviceMemory device_memory;

        // External Semaphore
        VkSemaphoreCreateFlags semaphore_flags = 0;
        VkSemaphoreType semaphore_type = VK_SEMAPHORE_TYPE_BINARY;
    };

    inline std::optional<ExternalOpaqueInfo> GetOpaqueInfoFromFdHandle(int fd) const {
        ReadLockGuard guard(fd_handle_map_lock_);
        if (const auto itr = fd_handle_map_.find(fd); itr != fd_handle_map_.cend()) {
            return itr->second;
        }
        return {};
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR
    inline std::optional<ExternalOpaqueInfo> GetOpaqueInfoFromWin32Handle(HANDLE handle) const {
        ReadLockGuard guard(win32_handle_map_lock_);
        if (const auto itr = win32_handle_map_.find(handle); itr != win32_handle_map_.cend()) {
            return itr->second;
        }
        return {};
    }
#endif

    virtual bool ValidateProtectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state,
                                        const Location& image_loc, const char* vuid, const char* more_message = "") const {
        return false;
    }
    virtual bool ValidateUnprotectedImage(const vvl::CommandBuffer& cb_state, const vvl::Image& image_state,
                                          const Location& image_loc, const char* vuid, const char* more_message = "") const {
        return false;
    }
    virtual bool ValidateProtectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state,
                                         const Location& buffer_loc, const char* vuid, const char* more_message = "") const {
        return false;
    }
    virtual bool ValidateUnprotectedBuffer(const vvl::CommandBuffer& cb_state, const vvl::Buffer& buffer_state,
                                           const Location& buffer_loc, const char* vuid, const char* more_message = "") const {
        return false;
    }
    virtual bool VerifyImageLayout(const vvl::CommandBuffer& cb_state, const vvl::ImageView& image_view_state,
                                   VkImageLayout explicit_layout, const Location& image_loc, const char* mismatch_layout_vuid,
                                   bool* error) const {
        return false;
    }

    // Link to the device's physical-device data
    vvl::PhysicalDevice* physical_device_state;

    // Link for derived device objects back to their parent instance object
    vvl::Instance* instance_state;

    VkDeviceGroupDeviceCreateInfo device_group_create_info = {};
    uint32_t physical_device_count;
    uint32_t custom_border_color_sampler_count = 0;
    bool disable_internal_pipeline_cache;

    // Some extensions/features changes the behavior of the app/layers/spec if present.
    // So it needs its own special boolean unlike the enabled_fatures.
    bool has_format_feature2;  // VK_KHR_format_feature_flags2
    // VK_EXT_pipeline_robustness was designed to be a subset of robustness extensions
    // Enabling the other robustness features can reduce performance on GPU, so just the
    // support is needed to check
    bool has_robust_image_access;  // VK_EXT_image_robustness
    // Validation requires special handling for VkPhysicalDeviceRobustness2FeaturesEXT, because for some cases robustness features
    // // need to only be supported, not enabled
    bool has_robust_image_access2;   // VK_EXT_robustness2
    bool has_robust_buffer_access2;  // VK_EXT_robustness2

    std::vector<VkCooperativeMatrixPropertiesNV> cooperative_matrix_properties_nv;
    std::vector<VkCooperativeMatrixPropertiesKHR> cooperative_matrix_properties_khr;
    std::vector<VkCooperativeMatrixFlexibleDimensionsPropertiesNV> cooperative_matrix_flexible_dimensions_properties;

    std::vector<VkCooperativeVectorPropertiesNV> cooperative_vector_properties_nv;

    // Features and properties that depend on platforms being defined
    // They will be false if platform is not defined
    bool android_external_format_resolve_null_color_attachment_prop = false;  // VK_ANDROID_external_format_resolve

    // Queue family extension properties -- storing queue family properties gathered from
    // VkQueueFamilyProperties2::pNext chain
    struct QueueFamilyExtensionProperties {
        VkQueueFamilyVideoPropertiesKHR video_props;
        VkQueueFamilyQueryResultStatusPropertiesKHR query_result_status_props;
    };
    std::vector<QueueFamilyExtensionProperties> queue_family_ext_props;

    bool performance_lock_acquired = false;
    uint32_t buffer_device_address_ranges_version = 0;

    mutable vvl::VideoProfileDesc::Cache video_profile_cache_;

    using BufferAddressMapStore = small_vector<vvl::Buffer*, 1, size_t>;
    using BufferAddressRangeMap = sparse_container::range_map<VkDeviceAddress, BufferAddressMapStore>;

  protected:
    // tracks which queue family index were used when creating the device for quick lookup
    vvl::unordered_set<uint32_t> queue_family_index_set;
    // The queue count can different for the same queueFamilyIndex if the create flag are different
    struct DeviceQueueInfo {
        uint32_t index;  // from VkDeviceCreateInfo
        uint32_t queue_family_index;
        VkDeviceQueueCreateFlags flags;
        uint32_t queue_count;
    };
    std::vector<DeviceQueueInfo> device_queue_info_list;
    // If vkGetBufferDeviceAddress is called, keep track of buffer <-> address mapping.
    BufferAddressRangeMap buffer_address_map_;
    mutable std::shared_mutex buffer_address_lock_;

    // < external format, features >
    vvl::concurrent_unordered_map<uint64_t, VkFormatFeatureFlags2KHR> ahb_ext_formats_map;
    // < external format, colorAttachmentFormat > (VK_ANDROID_external_format_resolve)
    vvl::concurrent_unordered_map<uint64_t, VkFormat> ahb_ext_resolve_formats_map;

    std::atomic<VkDeviceSize> descriptorBufferAddressSpaceSize = {0u};
    std::atomic<VkDeviceSize> resourceDescriptorBufferAddressSpaceSize = {0u};
    std::atomic<VkDeviceSize> samplerDescriptorBufferAddressSpaceSize = {0u};

    // Keep track of identifier -> state
    vvl::unordered_map<VkShaderModuleIdentifierEXT, std::shared_ptr<vvl::ShaderModule>> shader_identifier_map_;
    mutable std::shared_mutex shader_identifier_map_lock_;

    // If vkGetMemoryFdKHR is called, keep track of fd handle -> allocation info
    vvl::unordered_map<int, ExternalOpaqueInfo> fd_handle_map_;
    mutable std::shared_mutex fd_handle_map_lock_;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    // If vkGetMemoryWin32HandleKHR is called, keep track of HANDLE -> allocation info
    vvl::unordered_map<HANDLE, ExternalOpaqueInfo> win32_handle_map_;
    mutable std::shared_mutex win32_handle_map_lock_;
#endif

  private:
    VALSTATETRACK_MAP_AND_TRAITS(VkQueue, vvl::Queue, queue_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkAccelerationStructureNV, vvl::AccelerationStructureNV, acceleration_structure_nv_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkRenderPass, vvl::RenderPass, render_pass_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorSetLayout, vvl::DescriptorSetLayout, descriptor_set_layout_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSampler, vvl::Sampler, sampler_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkImageView, vvl::ImageView, image_view_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkImage, vvl::Image, image_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkBufferView, vvl::BufferView, buffer_view_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkBuffer, vvl::Buffer, buffer_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipelineCache, vvl::PipelineCache, pipeline_cache_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipeline, vvl::Pipeline, pipeline_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkShaderEXT, vvl::ShaderObject, shader_object_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDeviceMemory, vvl::DeviceMemory, mem_obj_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkFramebuffer, vvl::Framebuffer, frame_buffer_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkShaderModule, vvl::ShaderModule, shader_module_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorUpdateTemplate, vvl::DescriptorUpdateTemplate, desc_template_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSwapchainKHR, vvl::Swapchain, swapchain_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorPool, vvl::DescriptorPool, descriptor_pool_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkDescriptorSet, vvl::DescriptorSet, descriptor_set_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkCommandBuffer, vvl::CommandBuffer, command_buffer_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkCommandPool, vvl::CommandPool, command_pool_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkPipelineLayout, vvl::PipelineLayout, pipeline_layout_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkFence, vvl::Fence, fence_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkQueryPool, vvl::QueryPool, query_pool_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSemaphore, vvl::Semaphore, semaphore_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkEvent, vvl::Event, event_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkSamplerYcbcrConversion, vvl::SamplerYcbcrConversion, sampler_ycbcr_conversion_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkVideoSessionKHR, vvl::VideoSession, video_session_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkVideoSessionParametersKHR, vvl::VideoSessionParameters, video_session_parameters_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkAccelerationStructureKHR, vvl::AccelerationStructureKHR, acceleration_structure_khr_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkIndirectExecutionSetEXT, vvl::IndirectExecutionSet, indirect_execution_set_ext_map_)
    VALSTATETRACK_MAP_AND_TRAITS(VkIndirectCommandsLayoutEXT, vvl::IndirectCommandsLayout, indirect_commands_layout_ext_map_)

    std::atomic<uint32_t> object_id_{1};  // 0 is an invalid id

    // Simple base address allocator allow allow VkDeviceMemory allocations to appear to exist in a common address space.
    // At 256GB allocated/sec  ( > 8GB at 30Hz), will overflow in just over 2 years
    class FakeAllocator {
      public:
        void Free(VkDeviceSize fake_address){};  // Define the interface just in case we ever need to be cleverer.
        VkDeviceSize Alloc(VkDeviceSize size) {
            const auto alloc = free_.fetch_add(size);
            assert(std::numeric_limits<VkDeviceSize>::max() - size >= alloc);  //  776.722963 days later...
            return alloc;
        }

      private:
        std::atomic<VkDeviceSize> free_{1U << 20};  // start at 1mb to leave room for a NULL address
    };
    FakeAllocator fake_memory;
};

// Get buffer size from VkBufferImageCopy / VkBufferImageCopy2KHR structure, for a given format
template <typename RegionType>
static inline VkDeviceSize GetBufferSizeFromCopyImage(const RegionType& region, VkFormat image_format,
                                                      uint32_t image_layout_count) {
    VkDeviceSize buffer_size = 0;
    VkExtent3D copy_extent = region.imageExtent;
    VkDeviceSize buffer_width = (0 == region.bufferRowLength ? copy_extent.width : region.bufferRowLength);
    VkDeviceSize buffer_height = (0 == region.bufferImageHeight ? copy_extent.height : region.bufferImageHeight);
    uint32_t layer_count = region.imageSubresource.layerCount != VK_REMAINING_ARRAY_LAYERS
                               ? region.imageSubresource.layerCount
                               : image_layout_count - region.imageSubresource.baseArrayLayer;
    // VUID-VkImageCreateInfo-imageType-00961 prevents having both depth and layerCount ever both be greater than 1 together. Take
    // max to logic simple. This is the number of 'slices' to copy.
    const uint32_t z_copies = std::max(copy_extent.depth, layer_count);

    // Invalid if copy size is 0 and other validation checks will catch it. Returns zero as the caller should have fallback already
    // to ignore.
    if (copy_extent.width == 0 || copy_extent.height == 0 || copy_extent.depth == 0 || z_copies == 0) {
        return 0;
    }

    VkDeviceSize unit_size = 0;
    if (region.imageSubresource.aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT)) {
        // Spec in VkBufferImageCopy section list special cases for each format
        if (region.imageSubresource.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
            unit_size = 1;
        } else {
            // VK_IMAGE_ASPECT_DEPTH_BIT
            switch (image_format) {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D16_UNORM_S8_UINT:
                    unit_size = 2;
                    break;
                case VK_FORMAT_D32_SFLOAT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                // packed with the D24 value in the LSBs of the word, and undefined values in the eight MSBs
                case VK_FORMAT_X8_D24_UNORM_PACK32:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                    unit_size = 4;
                    break;
                default:
                    // Any misuse of formats vs aspect mask should be caught before here
                    return 0;
            }
        }
    } else {
        // size (bytes) of texel or block
        unit_size =
            vkuFormatElementSizeWithAspect(image_format, static_cast<VkImageAspectFlagBits>(region.imageSubresource.aspectMask));
    }

    if (vkuFormatIsBlockedImage(image_format)) {
        // Switch to texel block units, rounding up for any partially-used blocks
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(image_format);
        buffer_width = (buffer_width + block_extent.width - 1) / block_extent.width;
        buffer_height = (buffer_height + block_extent.height - 1) / block_extent.height;

        copy_extent.width = (copy_extent.width + block_extent.width - 1) / block_extent.width;
        copy_extent.height = (copy_extent.height + block_extent.height - 1) / block_extent.height;
        copy_extent.depth = (copy_extent.depth + block_extent.depth - 1) / block_extent.depth;
    }

    // Calculate buffer offset of final copied byte, + 1.
    buffer_size = (z_copies - 1) * buffer_height * buffer_width;                   // offset to slice
    buffer_size += ((copy_extent.height - 1) * buffer_width) + copy_extent.width;  // add row,col
    buffer_size *= unit_size;                                                      // convert to bytes
    return buffer_size;
}
}  // namespace vvl
