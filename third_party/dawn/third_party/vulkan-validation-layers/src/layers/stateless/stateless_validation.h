/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_struct_helper.hpp>
#include "generated/error_location_helper.h"
#include "sync/sync_utils.h"
#include "utils/vk_layer_utils.h"
#include "chassis/validation_object.h"
#include "generated/device_features.h"

enum FlagType { kRequiredFlags, kOptionalFlags, kRequiredSingleBit, kOptionalSingleBit };

namespace stateless {
// Per-call information needed to check enums, flags and pNext contents
class Context {
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    const Logger &log;
    const ErrorObject &error_obj;
    const DeviceExtensions &extensions;
    // Used to control the VK_KHR_maintenance5 behavior of VkPhysicalDevice calls
    // where unknown enum values are allowed.
    bool ignore_unknown_enums;

    Context(const Logger &log_, const ErrorObject &err_, const DeviceExtensions &exts_, bool ignore_unknown_enums_ = false)
        : log(log_), error_obj(err_), extensions(exts_), ignore_unknown_enums(ignore_unknown_enums_) {}

    bool ValidateNotZero(bool is_zero, const char *vuid, const Location &loc) const;

    bool ValidateRequiredPointer(const Location &loc, const void *value, const char *vuid) const;

    bool ValidateAllocationCallbacks(const VkAllocationCallbacks &callback, const Location &loc) const;

    template <typename T1, typename T2>
    bool ValidateArray(const Location &count_loc, const Location &array_loc, T1 count, const T2 *array, bool count_required,
                       bool array_required, const char *count_required_vuid, const char *array_required_vuid) const {
        bool skip = false;

        // Count parameters not tagged as optional cannot be 0
        if (count_required && (count == 0)) {
            skip |= log.LogError(count_required_vuid, error_obj.handle, count_loc, "must be greater than 0.");
        }

        // Array parameters not tagged as optional cannot be NULL, unless the count is 0
        if (array_required && (count != 0) && (*array == nullptr)) {
            skip |= log.LogError(array_required_vuid, error_obj.handle, array_loc, "is NULL.");
        }

        return skip;
    }

    template <typename T1, typename T2>
    bool ValidatePointerArray(const Location &count_loc, const Location &array_loc, const T1 *count, const T2 *array,
                              bool count_ptr_required, bool count_value_required, bool array_required,
                              const char *count_ptr_required_vuid, const char *count_required_vuid,
                              const char *array_required_vuid) const {
        bool skip = false;

        if (count == nullptr) {
            if (count_ptr_required) {
                skip |= log.LogError(count_ptr_required_vuid, error_obj.handle, count_loc, "is NULL.");
            }
        } else {
            skip |= ValidateArray(count_loc, array_loc, *array ? (*count) : 0, &array, count_value_required, array_required,
                                  count_required_vuid, array_required_vuid);
        }

        return skip;
    }

    template <typename T>
    bool ValidateStructType(const Location &loc, const T *value, VkStructureType sType, bool required, const char *struct_vuid,
                            const char *stype_vuid) const {
        bool skip = false;

        if (value == nullptr) {
            if (required) {
                skip |= log.LogError(struct_vuid, error_obj.handle, loc, "is NULL.");
            }
        } else if (value->sType != sType) {
            skip |= log.LogError(stype_vuid, error_obj.handle, loc.dot(Field::sType), "must be %s.", string_VkStructureType(sType));
        }

        return skip;
    }

    template <typename T>
    bool ValidateStructTypeArray(const Location &count_loc, const Location &array_loc, uint32_t count, const T *array,
                                 VkStructureType sType, bool count_required, bool array_required, const char *stype_vuid,
                                 const char *param_vuid, const char *count_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |=
                ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i].sType != sType) {
                    skip |= log.LogError(stype_vuid, error_obj.handle, array_loc.dot(i).dot(Field::sType), "must be %s",
                                         string_VkStructureType(sType));
                }
            }
        }

        return skip;
    }

    template <typename T>
    bool ValidateStructPointerTypeArray(const Location &count_loc, const Location &array_loc, uint32_t count, const T *array,
                                        VkStructureType sType, bool count_required, bool array_required, const char *stype_vuid,
                                        const char *param_vuid, const char *count_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |=
                ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid, param_vuid);
        } else {
            // Verify that all structs in the array have the correct type
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i]->sType != sType) {
                    skip |= log.LogError(stype_vuid, error_obj.handle, array_loc.dot(i).dot(Field::sType), "must be %s",
                                         string_VkStructureType(sType));
                }
            }
        }

        return skip;
    }

    template <typename T>
    bool ValidateStructTypeArray(const Location &count_loc, const Location &array_loc, uint32_t *count, const T *array,
                                 VkStructureType sType, bool count_ptr_required, bool count_value_required, bool array_required,
                                 const char *stype_vuid, const char *param_vuid, const char *count_ptr_required_vuid,
                                 const char *count_required_vuid) const {
        bool skip = false;

        if (count == nullptr) {
            if (count_ptr_required) {
                skip |= log.LogError(count_ptr_required_vuid, error_obj.handle, count_loc, "is NULL.");
            }
        } else {
            skip |=
                ValidateStructTypeArray(count_loc, array_loc, (*count), array, sType, count_value_required && (array != nullptr),
                                        array_required, stype_vuid, param_vuid, count_required_vuid);
        }

        return skip;
    }

    template <typename T>
    bool ValidateRequiredHandle(const Location &loc, T value) const {
        bool skip = false;
        if (value == VK_NULL_HANDLE) {
            skip |= log.LogError("UNASSIGNED-GeneralParameterError-RequiredHandle", error_obj.handle, loc, "is VK_NULL_HANDLE.");
        }
        return skip;
    }

    template <typename T>
    bool ValidateHandleArray(const Location &count_loc, const Location &array_loc, uint32_t count, const T *array,
                             bool count_required, bool array_required, const char *count_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid,
                                  kVUIDUndefined);
        } else {
            // Verify that no handles in the array are VK_NULL_HANDLE
            for (uint32_t i = 0; i < count; ++i) {
                if (array[i] == VK_NULL_HANDLE) {
                    skip |= log.LogError("UNASSIGNED-GeneralParameterError-RequiredHandleArray", error_obj.handle, array_loc.dot(i),
                                         "is VK_NULL_HANDLE.");
                }
            }
        }

        return skip;
    }

    bool ValidateString(const Location &loc, const char *vuid, const char *validate_string) const;

    bool ValidateStringArray(const Location &count_loc, const Location &array_loc, uint32_t count, const char *const *array,
                             bool count_required, bool array_required, const char *count_required_vuid,
                             const char *array_required_vuid) const;

    bool ValidatePnextFeatureStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                            bool is_const_param = true) const;
    bool ValidatePnextStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                     bool is_const_param = true) const;

    bool ValidateStructPnext(const Location &loc, const void *next, size_t allowed_type_count, const VkStructureType *allowed_types,
                             uint32_t header_version, const char *pnext_vuid, const char *stype_vuid,
                             const bool is_const_param = true) const;

    bool ValidateBool32(const Location &loc, VkBool32 value) const;

    bool ValidateBool32Array(const Location &count_loc, const Location &array_loc, uint32_t count, const VkBool32 *array,
                             bool count_required, bool array_required, const char *count_required_vuid,
                             const char *array_required_vuid) const;

    template <typename T>
    bool ValidateRangedEnum(const Location &loc, vvl::Enum name, T value, const char *vuid) const {
        bool skip = false;
        if (ignore_unknown_enums) {
            return skip;
        }
        ValidValue result = IsValidEnumValue(value);

        if (result == ValidValue::NotFound) {
            skip |= log.LogError(vuid, error_obj.handle, loc,
                                 "(%" PRIu32
                                 ") does not fall within the begin..end range of the %s enumeration tokens and is "
                                 "not an extension added token.",
                                 value, String(name));
        } else if (result == ValidValue::NoExtension) {
            // If called from an instance function, there is no device to base extension support off of
            auto extensions = GetEnumExtensions(value);
            skip |= log.LogError(vuid, error_obj.handle, loc, "(%s) requires the extensions %s.", DescribeEnum(value),
                                 String(extensions).c_str());
        }

        return skip;
    }

    template <typename T>
    bool ValidateRangedEnumArray(const Location &count_loc, const Location &array_loc, vvl::Enum name, uint32_t count,
                                 const T *array, bool count_required, bool array_required, const char *count_required_vuid,
                                 const char *array_required_vuid) const {
        bool skip = false;

        if ((array == nullptr) || (count == 0)) {
            skip |= ValidateArray(count_loc, array_loc, count, &array, count_required, array_required, count_required_vuid,
                                  array_required_vuid);
        } else {
            for (uint32_t i = 0; i < count; ++i) {
                ValidValue result = IsValidEnumValue(array[i]);
                if (result == ValidValue::NotFound) {
                    skip |= log.LogError(array_required_vuid, error_obj.handle, array_loc.dot(i),
                                         "(%" PRIu32
                                         ") does not fall within the begin..end range of the %s enumeration tokens and is "
                                         "not an extension added token.",
                                         array[i], String(name));
                } else if (result == ValidValue::NoExtension) {
                    // If called from an instance function, there is no device to base extension support off of
                    auto extensions = GetEnumExtensions(array[i]);
                    skip |= log.LogError(array_required_vuid, error_obj.handle, array_loc.dot(i),
                                         "(%s) requires the extensions %s.", DescribeEnum(array[i]), String(extensions).c_str());
                }
            }
        }

        return skip;
    }

    bool ValidateReservedFlags(const Location &loc, VkFlags value, const char *vuid) const;

    // helper to implement validation of both 32 bit and 64 bit flags.
    template <typename FlagTypedef>
    bool ValidateFlagsImplementation(const Location &loc, vvl::FlagBitmask flag_bitmask, FlagTypedef all_flags, FlagTypedef value,
                                     const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlags(const Location &loc, vvl::FlagBitmask flag_bitmask, VkFlags all_flags, VkFlags value,
                       const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlags(const Location &loc, vvl::FlagBitmask flag_bitmask, VkFlags64 all_flags, VkFlags64 value,
                       const FlagType flag_type, const char *vuid, const char *flags_zero_vuid = nullptr) const;

    bool ValidateFlagsArray(const Location &count_loc, const Location &array_loc, vvl::FlagBitmask flag_bitmask, VkFlags all_flags,
                            uint32_t count, const VkFlags *array, bool count_required, const char *count_required_vuid,
                            const char *array_required_vuid) const;

    template <typename T>
    ValidValue IsValidEnumValue(T value) const;
    template <typename T>
    vvl::Extensions GetEnumExtensions(T value) const;
    template <typename T>
    const char *DescribeEnum(T value) const;

    bool IsDuplicatePnext(VkStructureType input_value) const;

    // VkFlags values don't have a way overload, so need to use vvl::FlagBitmask
    vvl::Extensions IsValidFlagValue(vvl::FlagBitmask flag_bitmask, VkFlags value) const;
    vvl::Extensions IsValidFlag64Value(vvl::FlagBitmask flag_bitmask, VkFlags64 value) const;
    std::string DescribeFlagBitmaskValue(vvl::FlagBitmask flag_bitmask, VkFlags value) const;
    std::string DescribeFlagBitmaskValue64(vvl::FlagBitmask flag_bitmask, VkFlags64 value) const;
};

class Instance : public vvl::base::Instance {
    using BaseClass = vvl::base::Instance;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    vvl::unordered_map<VkPhysicalDevice, VkPhysicalDeviceProperties *> physical_device_properties_map;
    vvl::unordered_map<VkPhysicalDevice, DeviceExtensions> physical_device_extensions{};
    // We have a copy of this in Stateless and vvl::Instance, could move the base::Instance, but we don't have a way to

    Instance(vvl::dispatch::Instance *dispatch) : BaseClass(dispatch, LayerObjectTypeParameterValidation) {}

    bool OutputExtensionError(const Location &loc, const vvl::Extensions &exentsions) const;

    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                       const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                       VkImageFormatProperties2 *pImageFormatProperties,
                                                                       const Context &context) const;
    bool manual_PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                      VkImageType type, VkImageTiling tiling,
                                                                      VkImageUsageFlags usage, VkImageCreateFlags flags,
                                                                      VkImageFormatProperties *pImageFormatProperties,
                                                                      const Context &context) const;

    bool CheckPromotedApiAgainstVulkanVersion(VkInstance instance, const Location &loc, const uint32_t promoted_version) const;
    bool CheckPromotedApiAgainstVulkanVersion(VkPhysicalDevice pdev, const Location &loc, const uint32_t promoted_version) const;

    bool SupportedByPdev(const VkPhysicalDevice physical_device, vvl::Extension extension, bool skip_gpdp2 = false) const;

    bool ValidatePnextFeatureStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE, bool is_const_param = true) const;
    bool ValidatePnextStructContents(const Location &loc, const VkBaseOutStructure *header, const char *pnext_vuid,
                                     VkPhysicalDevice physicalDevice = VK_NULL_HANDLE, bool is_const_param = true) const;

    bool ValidateStructPnext(const Location &loc, const void *next, size_t allowed_type_count, const VkStructureType *allowed_types,
                             uint32_t header_version, const char *pnext_vuid, const char *stype_vuid,
                             VkPhysicalDevice physicalDevice = VK_NULL_HANDLE, const bool is_const_param = true) const;

    template <typename ExtensionState>
    bool ValidateExtensionReqs(const ExtensionState &extensions, const char *vuid, const char *extension_type,
                               vvl::Extension extension, const Location &extension_loc) const;

    void PostCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                    const RecordObject &record_obj) override;

    bool PreCallValidateCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                       VkInstance *pInstance, const ErrorObject &error_obj) const override;

    void CommonPostCallRecordEnumeratePhysicalDevice(const VkPhysicalDevice *phys_devices, const int count);
    void PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                VkPhysicalDevice *pPhysicalDevices, const RecordObject &record_obj) override;

    void PostCallRecordEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t *pPhysicalDeviceGroupCount,
                                                     VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
                                                     const RecordObject &record_obj) override;
    void PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator,
                                      const RecordObject &record_obj) override;

    bool manual_PreCallValidateGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice,
                                                            VkPhysicalDeviceProperties2 *pProperties, const Context &context) const;
    bool manual_PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                            const Context &context) const;

    bool manual_PreCallValidateGetPhysicalDeviceExternalBufferProperties(
        VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
        VkExternalBufferProperties *pExternalBufferProperties, const Context &context) const;

    bool manual_PreCallValidateCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                    const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode,
                                                    const Context &context) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                     const Context &context) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    bool manual_PreCallValidateCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                       const Context &context) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
    bool manual_PreCallValidateCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                   const Context &context) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool manual_PreCallValidateCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                    const Context &context) const;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    bool manual_PreCallValidateCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                       const Context &context) const;

#endif  // VK_USE_PLATFORM_ANDROID_KHR

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                  uint32_t *pSurfaceFormatCount,
                                                                  VkSurfaceFormatKHR *pSurfaceFormats,
                                                                  const Context &context) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                       uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes,
                                                                       const Context &context) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        VkSurfaceCapabilities2KHR *pSurfaceCapabilities,
                                                                        const Context &context) const;

    bool manual_PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                   const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                   uint32_t *pSurfaceFormatCount,
                                                                   VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                   const Context &context) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                        uint32_t *pPresentModeCount,
                                                                        VkPresentModeKHR *pPresentModes,
                                                                        const Context &context) const;
#endif  // VK_USE_PLATFORM_WIN32_KHR

#include "generated/stateless_instance_methods.h"
};

class Device : public vvl::base::Device {
    using BaseClass = vvl::base::Device;
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    Device(vvl::dispatch::Device *dev, Instance *instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeParameterValidation), instance(instance_vo) {}
    ~Device() {}

    Instance *instance;
    VkPhysicalDeviceLimits device_limits = {};

    // This was a special case where it was decided to use the extension version for validation
    // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5671
    uint32_t discard_rectangles_extension_version = 0;
    uint32_t scissor_exclusive_extension_version = 0;

    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    struct SubpassesUsageStates {
        vvl::unordered_set<uint32_t> subpasses_using_color_attachment;
        vvl::unordered_set<uint32_t> subpasses_using_depthstencil_attachment;
    };

    // Though this validation object is predominantly statless, the Framebuffer checks are greatly simplified by creating and
    // updating a map of the renderpass usage states, and these accesses need thread protection. Use a mutex separate from the
    // parent object's to maintain that functionality.
    mutable std::mutex renderpass_map_mutex;
    vvl::unordered_map<VkRenderPass, SubpassesUsageStates> renderpasses_states;

    bool OutputExtensionError(const Location &loc, const vvl::Extensions &exentsions) const;

    bool ValidateSubpassGraphicsFlags(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo, uint32_t subpass,
                                      VkPipelineStageFlags2 stages, const char *vuid, const Location &loc) const;

    bool ValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                  const ErrorObject &error_obj) const;

    void RecordRenderPass(VkRenderPass renderPass, const VkRenderPassCreateInfo2 *pCreateInfo);

    // Pre/PostCallRecord declarations
    void PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                        const RecordObject &record_obj) override;
    void PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                            const RecordObject &record_obj) override;
    void PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator,
                                         const RecordObject &record_obj) override;

    bool ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV &order, const Location &order_loc) const;

    bool ValidateGeometryTrianglesNV(const VkGeometryTrianglesNV &triangles, VkAccelerationStructureNV object_handle,
                                     const Location &loc) const;
    bool ValidateGeometryAABBNV(const VkGeometryAABBNV &geometry, VkAccelerationStructureNV object_handle,
                                const Location &loc) const;
    bool ValidateGeometryNV(const VkGeometryNV &geometry, VkAccelerationStructureNV object_handle, const Location &loc) const;
    bool ValidateAccelerationStructureInfoNV(const Context &context, const VkAccelerationStructureInfoNV &info,
                                             VkAccelerationStructureNV object_handle, const Location &loc) const;
    bool ValidateSwapchainCreateInfoMaintenance1(const VkSwapchainCreateInfoKHR &create_info, const Location &loc) const;
    bool ValidateSwapchainCreateInfo(const Context &context, const VkSwapchainCreateInfoKHR &create_info, const Location &loc) const;

    bool manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                               const Context &context) const;

    bool manual_PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                            const Context &context) const;

    bool manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImage *pImage, const Context &context) const;
    bool ValidateCreateImageSparse(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageFragmentShadingRate(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageCornerSampled(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageStencilUsage(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageCompressionControl(const Context &context, const VkImageCreateInfo &create_info,
                                               const Location &create_info_loc) const;
    bool ValidateCreateImageSwapchain(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageFormatList(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageMetalObject(const VkImageCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateCreateImageDrmFormatModifiers(const VkImageCreateInfo &create_info, const Location &create_info_loc,
                                               std::vector<uint64_t> &image_create_drm_format_modifiers) const;

    bool manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                               const Context &context) const;

    bool manual_PreCallValidateGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo,
                                                               VkSubresourceLayout2 *pLayout, const Context &context) const;

    bool ValidateViewport(const VkViewport &viewport, VkCommandBuffer object, const Location &loc) const;

    bool manual_PreCallValidateCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                  const Context &context) const;
    bool manual_PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                    const Context &context) const;
    bool ValidatePushConstantRange(uint32_t push_constant_range_count, const VkPushConstantRange *push_constant_ranges,
                                   const Location &loc) const;

    bool ValidatePipelineShaderStageCreateInfoCommon(const Context &context, const VkPipelineShaderStageCreateInfo &create_info,
                                                     const Location &loc) const;
    bool ValidatePipelineBinaryInfo(const void *next, VkPipelineCreateFlags flags, VkPipelineCache pipelineCache,
                                    const Location &loc) const;
    bool ValidatePipelineRenderingCreateInfo(const Context &context, const VkPipelineRenderingCreateInfo &rendering_struct,
                                             const Location &loc) const;
    bool ValidateCreateGraphicsPipelinesFlags(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool manual_PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                       const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       const Context &context) const;
    bool ValidateCreateComputePipelinesFlags(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool manual_PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                      const Context &context) const;

    bool ValidateSamplerFilterMinMax(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateSamplerCustomBoarderColor(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateSamplerSubsampled(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool ValidateSamplerImageProcessingQCOM(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const;
    bool manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                             const Context &context) const;
    bool ValidateMutableDescriptorTypeCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                 const VkMutableDescriptorTypeCreateInfoEXT &mutable_create_info,
                                                 const Location &loc) const;
    bool ValidateDescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                               const Location &create_info_loc) const;
    bool manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout,
                                                         const Context &context) const;
    bool manual_PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                             VkDescriptorSetLayoutSupport *pSupport, const Context &context) const;

    bool manual_PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                               const Context &context) const;

    bool manual_PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkEvent *pEvent, const Context &context) const;

    bool manual_PreCallValidateCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkBufferView *pBufferView,
                                                const Context &context) const;

#ifdef VK_USE_PLATFORM_METAL_EXT
    bool ExportMetalObjectsPNextUtil(VkExportMetalObjectTypeFlagBitsEXT bit, const char *vuid, const Location &loc,
                                     const char *sType, const void *pNext) const;
#endif  // VK_USE_PLATFORM_METAL_EXT

    bool ValidateWriteDescriptorSet(const Context &context, const Location &loc, const uint32_t descriptorWriteCount,
                                    const VkWriteDescriptorSet *pDescriptorWrites) const;
    bool manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                    const VkCopyDescriptorSet *pDescriptorCopies, const Context &context) const;

    bool manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets, const Context &context) const;

    bool manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                const Context &context) const;

    bool manual_PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                 const Context &context) const;

    bool manual_PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                  const VkCommandBuffer *pCommandBuffers, const Context &context) const;

    bool manual_PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                  const Context &context) const;

    bool manual_PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                              const VkViewport *pViewports, const Context &context) const;

    bool manual_PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                             const VkRect2D *pScissors, const Context &context) const;
    bool manual_PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const Context &context) const;

    bool manual_PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                   const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                   const VkClearRect *pRects, const Context &context) const;

    bool manual_PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferCopy *pRegions, const Context &context) const;

    bool manual_PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                              const Context &context) const;

    bool manual_PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize dataSize, const void *pData, const Context &context) const;

    bool manual_PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                             VkDeviceSize size, uint32_t data, const Context &context) const;

    bool manual_PreCallValidateCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                           const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                           const Context &context) const;

    bool manual_PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                  const Context &context) const;
    bool manual_PreCallValidateReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT *pReleaseInfo,
                                                         const Context &context) const;
    bool manual_PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                         const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains,
                                                         const Context &context) const;
    bool manual_PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, const Context &context) const;
    bool manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool,
                                                    const Context &context) const;
    bool manual_PreCallValidateCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                    VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                    const VkWriteDescriptorSet *pDescriptorWrites, const Context &context) const;
    bool manual_PreCallValidateCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                     const VkPushDescriptorSetInfo *pPushDescriptorSetInfo,
                                                     const Context &context) const;
    bool manual_PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                        uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors,
                                                        const Context &context) const;
    bool manual_PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                  uint32_t viewportCount,
                                                                  const VkShadingRatePaletteNV *pShadingRatePalettes,
                                                                  const Context &context) const;

    bool manual_PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                         uint32_t customSampleOrderCount,
                                                         const VkCoarseSampleOrderCustomNV *pCustomSampleOrders,
                                                         const Context &context) const;

    bool manual_PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                              const Context &context) const;

    bool manual_PreCallValidateCreateAccelerationStructureNV(VkDevice device,
                                                             const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkAccelerationStructureNV *pAccelerationStructure,
                                                             const Context &context) const;
    bool manual_PreCallValidateCreateAccelerationStructureKHR(VkDevice device,
                                                              const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkAccelerationStructureKHR *pAccelerationStructure,
                                                              const Context &context) const;
    bool manual_PreCallValidateDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               const Context &context) const;
    bool manual_PreCallValidateCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                               const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                               VkDeviceSize instanceOffset, VkBool32 update,
                                                               VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                               VkBuffer scratch, VkDeviceSize scratchOffset,
                                                               const Context &context) const;
    bool manual_PreCallValidateGetAccelerationStructureHandleNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                                size_t dataSize, void *pData, const Context &context) const;

    bool manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer,
                                                                          uint32_t accelerationStructureCount,
                                                                          const VkAccelerationStructureNV *pAccelerationStructures,
                                                                          VkQueryType queryType, VkQueryPool queryPool,
                                                                          uint32_t firstQuery, const Context &context) const;
    bool ValidateCreateRayTracingPipelinesFlagsNV(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool ValidateCreateRayTracingPipelinesFlagsKHR(const VkPipelineCreateFlags2KHR flags, const Location &flags_loc) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                           const Context &context) const;
    bool manual_PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                            const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            const Context &context) const;
    bool manual_PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount, const VkViewportWScalingNV *pViewportWScalings,
                                                        const Context &context) const;
    bool manual_PreCallValidateCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                        const VkDepthClampRangeEXT *pDepthClampRange, const Context &context) const;

    bool manual_PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                const VkShaderCreateInfoEXT *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                                                VkShaderEXT *pShaders, const Context &context) const;
    bool manual_PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData,
                                                      const Context &context) const;

    bool manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                 const Context &context) const;

    bool manual_PreCallValidateCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                 uint16_t lineStipplePattern, const Context &context) const;

    bool ValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType,
                                    const Location &loc) const;
    bool manual_PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkIndexType indexType, const Context &context) const;
    bool manual_PreCallValidateCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkDeviceSize size, VkIndexType indexType, const Context &context) const;
    bool manual_PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                    const Context &context) const;

    bool manual_PreCallValidateSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo,
                                                          const Context &context) const;

    bool manual_PreCallValidateSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo,
                                                         const Context &context) const;

    bool manual_PreCallValidateAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                   VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                   const Context &context) const;

    bool manual_PreCallValidateAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                    uint32_t *pImageIndex, const Context &context) const;

    bool manual_PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                  uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                  const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                  const Context &context) const;

    bool manual_PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                            uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                            const VkDeviceSize *pCounterBufferOffsets,
                                                            const Context &context) const;

    bool manual_PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                          uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                          const VkDeviceSize *pCounterBufferOffsets, const Context &context) const;

    bool manual_PreCallValidateCmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount,
                                                           uint32_t firstInstance, VkBuffer counterBuffer,
                                                           VkDeviceSize counterBufferOffset, uint32_t counterOffset,
                                                           uint32_t vertexStride, const Context &context) const;

    bool manual_PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkSamplerYcbcrConversion *pYcbcrConversion,
                                                            const Context &context) const;

    bool manual_PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize,
                                                void *pDescriptor, const Context &context) const;
    bool manual_PreCallValidateCmdSetDescriptorBufferOffsets2EXT(
        VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo,
        const Context &context) const;
    bool manual_PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
        VkCommandBuffer commandBuffer,
        const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo,
        const Context &context) const;
    bool manual_PreCallValidateCmdPushDescriptorSetWithTemplate2(
        VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo,
        const Context &context) const;
    bool manual_PreCallValidateCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                      const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
                                                      const Context &context) const;
    bool manual_PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                              const Context &context) const;
    bool manual_PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                        VkMemoryFdPropertiesKHR *pMemoryFdProperties, const Context &context) const;
    bool ValidateExternalSemaphoreHandleType(VkSemaphore semaphore, const char *vuid, const Location &handle_type_loc,
                                             VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                             VkExternalSemaphoreHandleTypeFlags allowed_types) const;
    bool ValidateExternalFenceHandleType(VkFence fence, const char *vuid, const Location &handle_type_loc,
                                         VkExternalFenceHandleTypeFlagBits handle_type,
                                         VkExternalFenceHandleTypeFlags allowed_types) const;
    bool manual_PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                    const Context &context) const;
    bool manual_PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                 const Context &context) const;

    bool manual_PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                const Context &context) const;
    bool manual_PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                             const Context &context) const;

    bool manual_PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 const void *pHostPointer,
                                                                 VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties,
                                                                 const Context &context) const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool manual_PreCallValidateGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                       HANDLE *pHandle, const Context &context) const;
    bool manual_PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                 HANDLE handle,
                                                                 VkMemoryWin32HandlePropertiesKHR *pMemoryWin32HandleProperties,
                                                                 const Context &context) const;
    bool manual_PreCallValidateImportSemaphoreWin32HandleKHR(
        VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo, const Context &context) const;
    bool manual_PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device,
                                                          const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                          HANDLE *pHandle, const Context &context) const;

    bool manual_PreCallValidateImportFenceWin32HandleKHR(VkDevice device,
                                                         const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo,
                                                         const Context &context) const;
    bool manual_PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                      HANDLE *pHandle, const Context &context) const;

    bool PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(VkDevice device, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                              VkDeviceGroupPresentModeFlagsKHR *pModes,
                                                              const ErrorObject &error_obj) const override;
#endif

    bool manual_PreCallValidateCopyAccelerationStructureToMemoryKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                    const Context &context) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                       const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                       const Context &context) const;

    bool manual_PreCallValidateCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                            const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                            const Context &context) const;

    bool manual_PreCallValidateCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                               const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                               const Context &context) const;
    bool ValidateCopyAccelerationStructureInfoKHR(const VkCopyAccelerationStructureInfoKHR &as_info,
                                                  const VulkanTypedHandle &handle, const Location &info_loc) const;
    bool ValidateCopyMemoryToAccelerationStructureInfoKHR(const VkCopyMemoryToAccelerationStructureInfoKHR &as_info,
                                                          const VulkanTypedHandle &handle, const Location &loc) const;

    bool manual_PreCallValidateCopyMemoryToAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                    const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                    const Context &context) const;
    bool manual_PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                       const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                       const Context &context) const;

    bool manual_PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(
        VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
        const VkAccelerationStructureKHR *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool,
        uint32_t firstQuery, const Context &context) const;
    bool manual_PreCallValidateWriteAccelerationStructuresPropertiesKHR(VkDevice device, uint32_t accelerationStructureCount,
                                                                        const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                        VkQueryType queryType, size_t dataSize, void *pData,
                                                                        size_t stride, const Context &context) const;
    bool manual_PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline,
                                                                               uint32_t firstGroup, uint32_t groupCount,
                                                                               size_t dataSize, void *pData,
                                                                               const Context &context) const;

    bool manual_PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                         const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                         const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                         const uint32_t *pIndirectStrides,
                                                                         const uint32_t *const *ppMaxPrimitiveCounts,
                                                                         const Context &context) const;

    bool manual_PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(
        VkDevice device, const VkAccelerationStructureVersionInfoKHR *pVersionInfo,
        VkAccelerationStructureCompatibilityKHR *pCompatibility, const Context &context) const;
    bool manual_PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                       const VkViewport *pViewports, const Context &context) const;

    bool manual_PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                      const VkRect2D *pScissors, const Context &context) const;
    bool manual_PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                     const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                     const Context &context) const;

    [[nodiscard]] bool ValidateTotalPrimitivesCount(uint64_t total_triangles_count, uint64_t total_aabbs_count,
                                                    const VulkanTypedHandle &handle, const Location &loc) const;
    [[nodiscard]] bool ValidateAccelerationStructureBuildGeometryInfoKHR(const Context &context,
                                                                         const VkAccelerationStructureBuildGeometryInfoKHR &info,
                                                                         const VulkanTypedHandle &handle,
                                                                         const Location &info_loc) const;
    bool manual_PreCallValidateCmdBuildAccelerationStructuresKHR(
        VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const Context &context) const;

    bool manual_PreCallValidateBuildAccelerationStructuresKHR(
        VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
        const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const Context &context) const;

    bool manual_PreCallValidateGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo,
                                                                     const uint32_t *pMaxPrimitiveCounts,
                                                                     VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo,
                                                                     const Context &context) const;

    bool ValidateTraceRaysRaygenShaderBindingTable(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR &raygen_shader_binding_table,
                                                   const Location &table_loc) const;
    bool ValidateTraceRaysMissShaderBindingTable(VkCommandBuffer commandBuffer,
                                                 const VkStridedDeviceAddressRegionKHR &miss_shader_binding_table,
                                                 const Location &table_loc) const;
    bool ValidateTraceRaysHitShaderBindingTable(VkCommandBuffer commandBuffer,
                                                const VkStridedDeviceAddressRegionKHR &hit_shader_binding_table,
                                                const Location &table_loc) const;
    bool ValidateTraceRaysCallableShaderBindingTable(VkCommandBuffer commandBuffer,
                                                     const VkStridedDeviceAddressRegionKHR &callable_shader_binding_table,
                                                     const Location &table_loc) const;

    bool manual_PreCallValidateCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                               uint32_t height, uint32_t depth, const Context &context) const;

    bool manual_PreCallValidateCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                       const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                       const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                       VkDeviceAddress indirectDeviceAddress, const Context &context) const;
    bool manual_PreCallValidateCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                        const Context &context) const;

    bool manual_PreCallValidateCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
                                                    uint32_t vertexAttributeDescriptionCount,
                                                    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions,
                                                    const Context &context) const;

    bool ValidateCmdPushConstants(VkCommandBuffer commandBuffer, uint32_t offset, uint32_t size, const Location &loc) const;
    bool manual_PreCallValidateCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo,
                                                 const Context &context) const;
    bool manual_PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void *pValues,
                                                const Context &context) const;

    bool manual_PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache,
                                                   const Context &context) const;
    bool manual_PreCallValidateMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
                                                   const VkPipelineCache *pSrcCaches, const Context &context) const;
    bool manual_PreCallValidateGetPipelinePropertiesEXT(VkDevice device, const VkPipelineInfoEXT *pPipelineInfo,
                                                        VkBaseOutStructure *pPipelineProperties, const Context &context) const;

    bool ValidateDepthClampRange(const VkDepthClampRangeEXT &depth_clamp_range, const Location &loc) const;
    bool manual_PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                  const VkClearColorValue *pColor, uint32_t rangeCount,
                                                  const VkImageSubresourceRange *pRanges, const Context &context) const;

    bool ValidateRenderPassStripeBeginInfo(VkCommandBuffer commandBuffer, const void *pNext, const VkRect2D render_area,
                                           const Location &loc) const;
    bool ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *const rp_begin,
                                    const ErrorObject &error_obj) const;
    bool manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                  VkSubpassContents, const Context &context) const;
    bool manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                   const VkSubpassBeginInfo *, const Context &context) const;
    bool manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                 const Context &context) const;

    bool ValidateBeginRenderingColorAttachment(VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
                                               const Location &rendering_info_loc) const;
    bool ValidateBeginRenderingDepthAttachment(VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
                                               const Location &rendering_info_loc) const;
    bool ValidateBeginRenderingStencilAttachment(VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
                                                 const Location &rendering_info_loc) const;
    bool ValidateBeginRenderingFragmentShadingRateAttachment(
        VkCommandBuffer commandBuffer, const VkRenderingInfo &rendering_info,
        const VkRenderingFragmentShadingRateAttachmentInfoKHR &rendering_fsr_attachment_info,
        const Location &rendering_info_loc) const;

    bool manual_PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                         uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles,
                                                         const Context &context) const;
    bool manual_PreCallValidateGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                   size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags,
                                                   const Context &context) const;
    bool manual_PreCallValidateCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                               const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
                                                               const Context &context) const;

    bool ValidateDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements &memory_requirements,
                                               const Location &loc) const;

    bool manual_PreCallValidateCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                               const Context &context) const;
    bool manual_PreCallValidateCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                             VkDiscardRectangleModeEXT discardRectangleMode,
                                                             const Context &context) const;
    bool manual_PreCallValidateCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                              uint32_t exclusiveScissorCount,
                                                              const VkBool32 *pExclusiveScissorEnables,
                                                              const Context &context) const;

    bool manual_PreCallValidateSetDeviceMemoryPriorityEXT(VkDevice device, VkDeviceMemory memory, float priority,
                                                          const Context &context) const;

    bool manual_PreCallValidateGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                VkMemoryRequirements2 *pMemoryRequirements,
                                                                const Context &context) const;
    bool manual_PreCallValidateGetDeviceImageSparseMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo,
                                                                      uint32_t *pSparseMemoryRequirementCount,
                                                                      VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements,
                                                                      const Context &context) const;
    bool manual_PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                               VkFence fence, const Context &context) const;

    bool ValidateIndirectExecutionSetPipelineInfo(const Context &context,
                                                  const VkIndirectExecutionSetPipelineInfoEXT &pipeline_info,
                                                  const Location &pipeline_info_loc) const;
    bool ValidateIndirectExecutionSetShaderInfo(const Context &context, const VkIndirectExecutionSetShaderInfoEXT &shader_info,
                                                const Location &shader_info_loc) const;
    bool manual_PreCallValidateCreateIndirectExecutionSetEXT(VkDevice device,
                                                             const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkIndirectExecutionSetEXT *pIndirectExecutionSet,
                                                             const Context &context) const;
    bool ValidateIndirectCommandsPushConstantToken(const Context &context,
                                                   const VkIndirectCommandsPushConstantTokenEXT &push_constant_token,
                                                   VkIndirectCommandsTokenTypeEXT token_type,
                                                   const Location &push_constant_token_loc) const;
    bool ValidateIndirectCommandsIndexBufferToken(const Context &context,
                                                  const VkIndirectCommandsIndexBufferTokenEXT &index_buffer_token,
                                                  const Location &index_buffer_token_loc) const;
    bool ValidateIndirectCommandsExecutionSetToken(const Context &context,
                                                   const VkIndirectCommandsExecutionSetTokenEXT &exe_set_token,
                                                   const Location &exe_set_token_loc) const;
    bool ValidateIndirectCommandsLayoutToken(const Context &context, const VkIndirectCommandsLayoutTokenEXT &token,
                                             const Location &token_loc) const;
    bool ValidateIndirectCommandsLayoutStage(const Context &context, const VkIndirectCommandsLayoutTokenEXT &token,
                                             const Location &token_loc, VkShaderStageFlags shader_stages, bool has_stage_graphics,
                                             bool has_stage_compute, bool has_stage_ray_tracing, bool has_stage_mesh) const;
    bool manual_PreCallValidateCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                               const VkIndirectCommandsLayoutCreateInfoEXT *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkIndirectCommandsLayoutEXT *pIndirectCommandsLayout,
                                                               const Context &context) const;
    bool ValidateGeneratedCommandsInfo(VkCommandBuffer command_buffer, const VkGeneratedCommandsInfoEXT &generated_commands_info,
                                       const Location &info_loc) const;

    bool manual_PreCallValidateCmdPreprocessGeneratedCommandsEXT(VkCommandBuffer commandBuffer,
                                                                 const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo,
                                                                 VkCommandBuffer stateCommandBuffer, const Context &context) const;

    bool manual_PreCallValidateCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                              const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo,
                                                              const Context &context) const;

    bool manual_PreCallValidateCreateMicromapEXT(VkDevice device, const VkMicromapCreateInfoEXT *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkMicromapEXT *pMicromap,
                                                 const Context &context) const;

    bool manual_PreCallValidateDestroyMicromapEXT(VkDevice device, VkMicromapEXT micromap, const VkAllocationCallbacks *pAllocator,
                                                  const Context &context) const;

    bool manual_PreCallValidateCmdBuildMicromapsEXT(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                    const VkMicromapBuildInfoEXT *pInfos, const Context &context) const;

    bool manual_PreCallValidateBuildMicromapsEXT(VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
                                                 const VkMicromapBuildInfoEXT *pInfos, const Context &context) const;

    bool manual_PreCallValidateCopyMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                               const VkCopyMicromapInfoEXT *pInfo, const Context &context) const;

    bool manual_PreCallValidateCopyMicromapToMemoryEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyMicromapToMemoryInfoEXT *pInfo, const Context &context) const;

    bool manual_PreCallValidateCopyMemoryToMicromapEXT(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                       const VkCopyMemoryToMicromapInfoEXT *pInfo, const Context &context) const;

    bool manual_PreCallValidateWriteMicromapsPropertiesEXT(VkDevice device, uint32_t micromapCount, const VkMicromapEXT *pMicromaps,
                                                           VkQueryType queryType, size_t dataSize, void *pData, size_t stride,
                                                           const Context &context) const;

    bool manual_PreCallValidateCmdCopyMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapInfoEXT *pInfo,
                                                  const Context &context) const;

    bool manual_PreCallValidateCmdCopyMicromapToMemoryEXT(VkCommandBuffer commandBuffer, const VkCopyMicromapToMemoryInfoEXT *pInfo,
                                                          const Context &context) const;

    bool manual_PreCallValidateCmdCopyMemoryToMicromapEXT(VkCommandBuffer commandBuffer, const VkCopyMemoryToMicromapInfoEXT *pInfo,
                                                          const Context &context) const;

    bool manual_PreCallValidateCmdWriteMicromapsPropertiesEXT(VkCommandBuffer commandBuffer, uint32_t micromapCount,
                                                              const VkMicromapEXT *pMicromaps, VkQueryType queryType,
                                                              VkQueryPool queryPool, uint32_t firstQuery,
                                                              const Context &context) const;

    bool manual_PreCallValidateGetDeviceMicromapCompatibilityEXT(VkDevice device, const VkMicromapVersionInfoEXT *pVersionInfo,
                                                                 VkAccelerationStructureCompatibilityKHR *pCompatibility,
                                                                 const Context &context) const;

    bool manual_PreCallValidateGetMicromapBuildSizesEXT(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                                        const VkMicromapBuildInfoEXT *pBuildInfo,
                                                        VkMicromapBuildSizesInfoEXT *pSizeInfo, const Context &context) const;

    bool ValidateAllocateMemoryExternal(VkDevice device, const VkMemoryAllocateInfo &allocate_info, VkMemoryAllocateFlags flags,
                                        const Location &allocate_info_loc) const;

    bool ValidateVkConvertCooperativeVectorMatrixInfoNV(const LogObjectList &objlist,
                                                        const VkConvertCooperativeVectorMatrixInfoNV &info,
                                                        const Location &info_loc) const;

    bool manual_PreCallValidateConvertCooperativeVectorMatrixNV(VkDevice device,
                                                                const VkConvertCooperativeVectorMatrixInfoNV *pInfo,
                                                                const Context &context) const;

    bool manual_PreCallValidateCmdConvertCooperativeVectorMatrixNV(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                   const VkConvertCooperativeVectorMatrixInfoNV *pInfos,
                                                                   const Context &context) const;

#include "generated/stateless_device_methods.h"
};

}  // namespace stateless
