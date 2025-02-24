/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <assert.h>

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "generated/pnext_chain_extraction.h"
#include "core_validation.h"
#include "cc_buffer_address.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/ray_tracing_state.h"

// For given mem object, verify that it is not null or UNBOUND, if it is, report error. Return skip value.
bool CoreChecks::VerifyBoundMemoryIsValid(const vvl::DeviceMemory *memory_state, const LogObjectList &objlist,
                                          const VulkanTypedHandle &typed_handle, const Location &loc, const char *vuid) const {
    bool skip = false;
    if (!memory_state) {
        const char *type_name = string_VulkanObjectType(typed_handle.type);
        skip |=
            LogError(vuid, objlist, loc, "(%s) is used with no memory bound. Memory should be bound by calling vkBind%sMemory().",
                     FormatHandle(typed_handle).c_str(), type_name + 2);
    } else if (memory_state->Destroyed()) {
        skip |= LogError(vuid, objlist, loc,
                         "(%s) is used, but bound memory was freed. Memory must not be freed prior to this operation.",
                         FormatHandle(typed_handle).c_str());
    }
    return skip;
}

bool CoreChecks::VerifyBoundMemoryIsDeviceVisible(const vvl::DeviceMemory *memory_state, const LogObjectList &objlist,
                                                  const VulkanTypedHandle &typed_handle, const Location &loc,
                                                  const char *vuid) const {
    bool result = false;
    if (memory_state) {
        if ((phys_dev_mem_props.memoryTypes[memory_state->allocate_info.memoryTypeIndex].propertyFlags &
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0) {
            result |= LogError(vuid, objlist, loc, "(%s) used with memory that is not device visible.",
                               FormatHandle(typed_handle).c_str());
        }
    }
    return result;
}

// Check to see if memory was ever bound to this image
bool CoreChecks::ValidateMemoryIsBoundToImage(const LogObjectList &objlist, const vvl::Image &image_state, const Location &loc,
                                              const char *vuid) const {
    bool result = false;
    if (image_state.create_from_swapchain != VK_NULL_HANDLE) {
        if (!image_state.bind_swapchain) {
            result |= LogError(
                vuid, objlist, loc,
                "(%s) is created by %s, and the image should be bound by calling vkBindImageMemory2(), and the pNext chain "
                "includes VkBindImageMemorySwapchainInfoKHR.",
                FormatHandle(image_state).c_str(), FormatHandle(image_state.create_from_swapchain).c_str());
        } else if (image_state.create_from_swapchain != image_state.bind_swapchain->VkHandle()) {
            result |=
                LogError(vuid, objlist, loc,
                         "(%s) is created by %s, but the image is bound by %s. The image should be created and bound by the same "
                         "swapchain",
                         FormatHandle(image_state).c_str(), FormatHandle(image_state.create_from_swapchain).c_str(),
                         FormatHandle(image_state.bind_swapchain->Handle()).c_str());
        }
    } else if (image_state.IsExternalBuffer()) {
        // TODO look into how to properly check for a valid bound memory for an external AHB
    } else if (!image_state.sparse) {
        // No need to optimize this since the size will only be 3 at most
        const auto &memory_states = image_state.GetBoundMemoryStates();
        if (memory_states.empty()) {
            result |=
                LogError(vuid, objlist, loc, "%s used with no memory bound. Memory should be bound by calling vkBindImageMemory().",
                         FormatHandle(image_state).c_str());
        } else {
            for (const auto &state : memory_states) {
                result |= VerifyBoundMemoryIsValid(state.get(), objlist, image_state.Handle(), loc, vuid);
            }
        }
    }
    return result;
}

bool CoreChecks::ValidateAccelStructsMemoryDoNotOverlap(const Location &function_loc, LogObjectList objlist,
                                                        const vvl::AccelerationStructureKHR &accel_struct_a, const Location &loc_a,
                                                        const vvl::AccelerationStructureKHR &accel_struct_b, const Location &loc_b,
                                                        const char *vuid) const {
    bool skip = false;

    const vvl::Buffer &buffer_a = *accel_struct_a.buffer_state;
    const vvl::Buffer &buffer_b = *accel_struct_b.buffer_state;

    const sparse_container::range<VkDeviceSize> range_a(accel_struct_a.create_info.offset, accel_struct_a.create_info.size);
    const sparse_container::range<VkDeviceSize> range_b(accel_struct_b.create_info.offset, accel_struct_b.create_info.size);

    if (const auto [memory, overlap_range] = buffer_a.GetResourceMemoryOverlap(range_a, &buffer_b, range_b);
        memory != VK_NULL_HANDLE) {
        objlist.add(accel_struct_a.Handle(), buffer_a.Handle(), accel_struct_b.Handle(), buffer_b.Handle());

        skip |= LogError(vuid, objlist, function_loc,
                         "memory backing buffer (%s) used as storage for %s (%s) overlaps memory backing buffer (%s) used as "
                         "storage for %s (%s). Overlapped memory is (%s) on range %s.",
                         FormatHandle(buffer_a).c_str(), loc_a.Fields().c_str(), FormatHandle(accel_struct_a.Handle()).c_str(),
                         FormatHandle(buffer_b).c_str(), loc_b.Fields().c_str(), FormatHandle(accel_struct_b.Handle()).c_str(),
                         FormatHandle(memory).c_str(), string_range_hex(overlap_range).c_str());
    }

    return skip;
}

static bool ValidateBufferAndAccelStructsMemoryDoNotOverlap(const CoreChecks &validator, const vvl::Buffer &buffer_a,
                                                            sparse_container::range<VkDeviceSize> range_a,
                                                            const vvl::AccelerationStructureKHR &accel_struct_b,
                                                            const Location &loc_b, std::string *err_msg) {
    const vvl::Buffer &buffer_b = *accel_struct_b.buffer_state;
    const sparse_container::range<VkDeviceSize> range_b(accel_struct_b.create_info.offset, accel_struct_b.create_info.size);

    if (const auto [memory, overlap_range] = buffer_a.GetResourceMemoryOverlap(range_a, &buffer_b, range_b);
        memory != VK_NULL_HANDLE) {
        if (err_msg) {
            std::stringstream err_msg_strm;
            err_msg_strm << "memory backing buffer (" << validator.FormatHandle(buffer_a) << ") overlaps memory backing buffer ("
                         << validator.FormatHandle(buffer_b) << ") used as storage for " << loc_b.Fields() << " ("
                         << validator.FormatHandle(accel_struct_b.Handle()) << "). Overlapped memory is ("
                         << validator.FormatHandle(memory) << ") on range " << string_range_hex(overlap_range);
            *err_msg = err_msg_strm.str();
        }
        return false;
    }

    return true;
}

bool CoreChecks::ValidateScratchMemoryNoOverlap(const Location &function_loc, LogObjectList objlist,
                                                const vvl::span<vvl::Buffer *const> &scratches_a, VkDeviceAddress scratch_a_address,
                                                VkDeviceSize scratch_a_size, const Location &loc_scratches_a,
                                                const vvl::AccelerationStructureKHR *src_accel_struct,
                                                const Location &loc_src_accel_struct,
                                                const vvl::AccelerationStructureKHR &dst_accel_struct,
                                                const Location &loc_dst_accel_struct,
                                                const vvl::span<vvl::Buffer *const> &scratches_b, VkDeviceAddress scratch_b_address,
                                                VkDeviceSize scratch_b_size, const Location *loc_scratches_b) const {
    bool skip = false;

    const char *scratch_dst_accel_struct_overlap = "VUID-vkCmdBuildAccelerationStructuresKHR-dstAccelerationStructure-03703";
    const char *scratch_src_accel_struct_overlap = "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03705";
    const char *scratch_scratch_overlap = "VUID-vkCmdBuildAccelerationStructuresKHR-scratchData-03704";
    const std::string scratch_b_address_str = [&]() {
        std::stringstream ss;
        ss << std::hex << "0x" << scratch_b_address;
        return ss.str();
    }();

    BufferAddressValidation<3> buffer_address_validator = {{{
        {scratch_dst_accel_struct_overlap,
         [this, scratch_a_address, scratch_a_size, &dst_accel_struct, &loc_dst_accel_struct](vvl::Buffer *const scratch_a,
                                                                                             std::string *out_error_msg) {
             const VkDeviceSize scratch_a_offset = scratch_a_address - scratch_a->deviceAddress;
             const sparse_container::range<VkDeviceSize> scratch_a_range(scratch_a_offset, scratch_a_offset + scratch_a_size);

             const bool no_overlap_found = ValidateBufferAndAccelStructsMemoryDoNotOverlap(
                 *this, *scratch_a, scratch_a_range, dst_accel_struct, loc_dst_accel_struct, out_error_msg);

             return no_overlap_found;
         },
         [scratch_a_size, &loc_dst_accel_struct]() {
             return std::string("The following scratch buffers associated to this device address (assumed scratch byte size: ") +
                    std::to_string(scratch_a_size) + ") overlap with memory backing " + loc_dst_accel_struct.Fields();
         }},

    }}};

    if (src_accel_struct) {
        buffer_address_validator.AddVuidValidation(
            {scratch_src_accel_struct_overlap,
             [this, scratch_a_address, scratch_a_size, src_accel_struct, &loc_src_accel_struct](vvl::Buffer *const scratch_a,
                                                                                                std::string *out_error_msg) {
                 const VkDeviceSize scratch_a_offset = scratch_a_address - scratch_a->deviceAddress;
                 const sparse_container::range<VkDeviceSize> scratch_a_range(scratch_a_offset, scratch_a_offset + scratch_a_size);

                 const bool no_overlap_found = ValidateBufferAndAccelStructsMemoryDoNotOverlap(
                     *this, *scratch_a, scratch_a_range, *src_accel_struct, loc_src_accel_struct, out_error_msg);

                 return no_overlap_found;
             },
             [scratch_a_size, &loc_src_accel_struct]() {
                 return std::string(
                            "The following scratch buffers associated to this device address (assumed scratch byte size: ") +
                        std::to_string(scratch_a_size) + ") overlap with memory backing " + loc_src_accel_struct.Fields();
             }});
    }

    if (!scratches_b.empty()) {
        assert(loc_scratches_b);
        buffer_address_validator.AddVuidValidation(
            {scratch_scratch_overlap,
             [this, scratch_a_address, scratch_a_size, scratches_b, scratch_b_address, scratch_b_size](vvl::Buffer *const scratch_a,
                                                                                                       std::string *out_error_msg) {
                 const VkDeviceSize scratch_a_offset = scratch_a_address - scratch_a->deviceAddress;
                 const sparse_container::range<VkDeviceSize> scratch_a_range(scratch_a_offset, scratch_a_offset + scratch_a_size);

                 for (auto scratch_b : scratches_b) {
                     const VkDeviceSize scratch_b_offset = scratch_b_address - scratch_b->deviceAddress;
                     const sparse_container::range<VkDeviceSize> scratch_b_range(scratch_b_offset,
                                                                                 scratch_b_offset + scratch_b_size);

                     if (auto [mem, mem_range] = scratch_b->GetResourceMemoryOverlap(scratch_b_range, scratch_a, scratch_a_range);
                         mem != VK_NULL_HANDLE) {
                         if (out_error_msg) {
                             *out_error_msg +=
                                 "Memory (" + FormatHandle(mem) + ") overlap on memory range " + string_range_hex(mem_range);
                         }
                         return false;
                     }
                 }
                 return true;
             },
             [loc_scratches_b, scratch_a_size, scratch_b_size, &scratch_b_address_str]() {
                 return std::string("The following scratch buffers associated to this device address (assumed scratch byte size: " +
                                    std::to_string(scratch_a_size) + ") overlap with at least one scratch buffer associated to ") +
                        loc_scratches_b->Fields() + " (" + scratch_b_address_str +
                        ") (assumed scratch byte size: " + std::to_string(scratch_b_size) + "):";
             }});
    }

    skip |= buffer_address_validator.LogErrorsIfNoValidBuffer(*this, scratches_a, loc_scratches_a, objlist, scratch_a_address);

    return skip;
}

// Check to see if host-visible memory was bound to this buffer
bool CoreChecks::ValidateAccelStructBufferMemoryIsHostVisible(const vvl::AccelerationStructureKHR &accel_struct,
                                                              const Location &buffer_loc, const char *vuid) const {
    bool result = false;
    result |= ValidateMemoryIsBoundToBuffer(device, *accel_struct.buffer_state, buffer_loc, vuid);
    if (!result) {
        if (const auto memory_state = accel_struct.buffer_state->MemoryState()) {
            if ((phys_dev_mem_props.memoryTypes[memory_state->allocate_info.memoryTypeIndex].propertyFlags &
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
                const LogObjectList objlist(accel_struct.Handle(), accel_struct.buffer_state->Handle(), memory_state->Handle());
                result |=
                    LogError(vuid, objlist, buffer_loc, "has been created with a buffer whose bound memory is not host visible.");
            }
        }
    }
    return result;
}

bool CoreChecks::ValidateAccelStructBufferMemoryIsNotMultiInstance(const vvl::AccelerationStructureKHR &accel_struct,
                                                                   const Location &accel_struct_loc, const char *vuid) const {
    bool skip = false;
    if (const vvl::DeviceMemory *memory_state = accel_struct.buffer_state->MemoryState()) {
        if (memory_state->multi_instance) {
            const LogObjectList objlist(accel_struct.Handle(), accel_struct.buffer_state->Handle(), memory_state->Handle());
            skip |= LogError(vuid, objlist, accel_struct_loc,
                             "has been created with a buffer bound to memory (%s) that was allocated with multiple instances.",
                             FormatHandle(memory_state->Handle()).c_str());
        }
    }
    return skip;
}

// Valid usage checks for a call to SetMemBinding().
// For NULL mem case, output warning
// Make sure given object is in global object map
//  IF a previous binding existed, output validation error
//  Otherwise, add reference from objectInfo to memoryInfo
//  Add reference off of objInfo
bool CoreChecks::ValidateSetMemBinding(const vvl::DeviceMemory &memory_state, const vvl::Bindable &mem_binding,
                                       const Location &loc) const {
    bool skip = false;

    const bool bind_2 = (loc.function != Func::vkBindBufferMemory) && (loc.function != Func::vkBindImageMemory);
    auto typed_handle = mem_binding.Handle();
    const bool is_buffer = typed_handle.type == kVulkanObjectTypeBuffer;

    if (mem_binding.sparse) {
        const char *vuid = kVUIDUndefined;
        const char *handle_type = is_buffer ? "BUFFER" : "IMAGE";
        if (is_buffer) {
            vuid = bind_2 ? "VUID-VkBindBufferMemoryInfo-buffer-01030" : "VUID-vkBindBufferMemory-buffer-01030";
        } else {
            vuid = bind_2 ? "VUID-VkBindImageMemoryInfo-image-01045" : "VUID-vkBindImageMemory-image-01045";
        }

        const LogObjectList objlist(memory_state.Handle(), typed_handle);
        skip |= LogError(vuid, objlist, loc,
                         "attempting to bind %s to %s which was created with sparse memory flags "
                         "(VK_%s_CREATE_SPARSE_*_BIT).",
                         FormatHandle(memory_state.Handle()).c_str(), FormatHandle(typed_handle).c_str(), handle_type);
    }

    const auto *prev_binding = mem_binding.MemoryState();
    if (prev_binding || mem_binding.indeterminate_state) {
        const char *vuid = kVUIDUndefined;
        if (is_buffer) {
            vuid = bind_2 ? "VUID-VkBindBufferMemoryInfo-buffer-07459" : "VUID-vkBindBufferMemory-buffer-07459";
        } else {
            vuid = bind_2 ? "VUID-VkBindImageMemoryInfo-image-07460" : "VUID-vkBindImageMemory-image-07460";
        }

        if (mem_binding.indeterminate_state) {
            Func bind_call = is_buffer ? Func::vkBindBufferMemory2 : Func::vkBindImageMemory2;
            const char *handle_type = is_buffer ? "buffer" : "image";
            const LogObjectList objlist(memory_state.Handle(), typed_handle);
            skip |= LogError(
                vuid, objlist, loc,
                "attempting to bind %s to %s which is in an indeterminate (possibly bound) state. A previous call to %s failed and "
                "we have to assume the %s was bound (but best advise is to handle the case and recreate the %s).",
                FormatHandle(memory_state.Handle()).c_str(), FormatHandle(typed_handle).c_str(), String(bind_call), handle_type,
                handle_type);
        } else {
            const LogObjectList objlist(memory_state.Handle(), typed_handle, prev_binding->Handle());
            skip |= LogError(vuid, objlist, loc, "attempting to bind %s to %s which has already been bound to %s.",
                             FormatHandle(memory_state.Handle()).c_str(), FormatHandle(typed_handle).c_str(),
                             FormatHandle(prev_binding->Handle()).c_str());
        }
    }
    return skip;
}

bool CoreChecks::IgnoreAllocationSize(const VkMemoryAllocateInfo &allocate_info) const {
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const VkExternalMemoryHandleTypeFlags ignored_allocation = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT |
                                                               VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT |
                                                               VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
    const auto import_memory_win32 = vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoKHR>(allocate_info.pNext);
    if (import_memory_win32 && (import_memory_win32->handleType & ignored_allocation) != 0) {
        return true;
    }
#elif VK_USE_PLATFORM_METAL_EXT
    const VkExternalMemoryHandleTypeFlags ignored_allocation = VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLBUFFER_BIT_EXT |
                                                               VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLTEXTURE_BIT_EXT |
                                                               VK_EXTERNAL_MEMORY_HANDLE_TYPE_MTLHEAP_BIT_EXT;
    const auto import_memory_metal = vku::FindStructInPNextChain<VkImportMemoryMetalHandleInfoEXT>(allocate_info.pNext);
    if (import_memory_metal && (import_memory_metal->handleType & ignored_allocation) != 0) {
        return true;
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    // Handles 01874 cases
    const auto export_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(allocate_info.pNext);
    if (export_info && (export_info->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        const auto dedicated_info = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(allocate_info.pNext);
        if (dedicated_info && dedicated_info->image) {
            return true;
        }
    }
    return false;
}

bool CoreChecks::HasExternalMemoryImportSupport(const vvl::Buffer &buffer, VkExternalMemoryHandleTypeFlagBits handle_type) const {
    VkPhysicalDeviceExternalBufferInfo info = vku::InitStructHelper();
    info.flags = buffer.create_info.flags;
    // TODO - Add VkBufferUsageFlags2CreateInfo support
    info.usage = buffer.create_info.usage;
    info.handleType = handle_type;
    VkExternalBufferProperties properties = vku::InitStructHelper();
    DispatchGetPhysicalDeviceExternalBufferPropertiesHelper(api_version, physical_device, &info, &properties);
    return (properties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
}

bool CoreChecks::HasExternalMemoryImportSupport(const vvl::Image &image, VkExternalMemoryHandleTypeFlagBits handle_type) const {
    VkPhysicalDeviceExternalImageFormatInfo external_info = vku::InitStructHelper();
    external_info.handleType = handle_type;
    VkPhysicalDeviceImageFormatInfo2 info = vku::InitStructHelper(&external_info);
    info.format = image.create_info.format;
    info.type = image.create_info.imageType;
    info.tiling = image.create_info.tiling;
    info.usage = image.create_info.usage;
    info.flags = image.create_info.flags;

    VkExternalImageFormatProperties external_properties = vku::InitStructHelper();
    VkImageFormatProperties2 properties = vku::InitStructHelper(&external_properties);
    if (image.create_info.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        // Can't get into function with using external memory extensions which require GPDP2
        if (DispatchGetPhysicalDeviceImageFormatProperties2Helper(api_version, physical_device, &info, &properties) != VK_SUCCESS) {
            return false;
        }
    } else {
        VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
        drm_format_modifier.sharingMode = image.create_info.sharingMode;
        drm_format_modifier.queueFamilyIndexCount = image.create_info.queueFamilyIndexCount;
        drm_format_modifier.pQueueFamilyIndices = image.create_info.pQueueFamilyIndices;
        vvl::PnextChainScopedAdd scoped_add_drm_fmt_mod(&info, &drm_format_modifier);

        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = vku::InitStructHelper();
        if (DispatchGetImageDrmFormatModifierPropertiesEXT(device, image.VkHandle(), &drm_format_properties) != VK_SUCCESS) {
            return false;
        }
        drm_format_modifier.drmFormatModifier = drm_format_properties.drmFormatModifier;
        if (DispatchGetPhysicalDeviceImageFormatProperties2Helper(api_version, physical_device, &info, &properties) != VK_SUCCESS) {
            return false;
        }
    }
    return (external_properties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
}

bool CoreChecks::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    if (Count<vvl::DeviceMemory>() >= phys_dev_props.limits.maxMemoryAllocationCount) {
        skip |=
            LogError("VUID-vkAllocateMemory-maxMemoryAllocationCount-04101", device, error_obj.location,
                     "The number of currently valid memory objects (%zu) is not less than maxMemoryAllocationCount (%" PRIu32 ").",
                     Count<vvl::DeviceMemory>(), phys_dev_props.limits.maxMemoryAllocationCount);
    }

    const Location allocate_info_loc = error_obj.location.dot(Field::pAllocateInfo);
    if (IsExtEnabled(extensions.vk_khr_maintenance3) &&
        pAllocateInfo->allocationSize > phys_dev_props_core11.maxMemoryAllocationSize) {
        // Discussed in https://gitlab.khronos.org/vulkan/vulkan/-/issues/4119 and finalized in
        // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7073 This is a limit the Working Group feels should be alerted
        // to the user as if it was a VU. While some drivers should report VK_ERROR_OUT_OF_DEVICE_MEMORY, each platform has small
        // quirks to it and there is no way for us to test the drivers will return the correct VkError here.
        LogError("UNASSIGNED-vkAllocateMemory-maxMemoryAllocationSize", device, allocate_info_loc.dot(Field::allocationSize),
                 "(%" PRIu64 ") is larger than maxMemoryAllocationSize (%" PRIu64
                 "). While this might work locally on your machine, there are many external factors each platform has that is used "
                 "to determine this limit. You should receive VK_ERROR_OUT_OF_DEVICE_MEMORY from this call, but even if you do "
                 "not, it is highly advised from all hardware vendors to not ignore this limit.",
                 pAllocateInfo->allocationSize, phys_dev_props_core11.maxMemoryAllocationSize);
    }

    if (IsExtEnabled(extensions.vk_android_external_memory_android_hardware_buffer)) {
        skip |= ValidateAllocateMemoryANDROID(*pAllocateInfo, allocate_info_loc);
    } else {
        if (!IgnoreAllocationSize(*pAllocateInfo) && 0 == pAllocateInfo->allocationSize) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-allocationSize-07899", device, allocate_info_loc.dot(Field::allocationSize),
                             "is 0.");
        }
    }

    auto chained_flags_struct = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
    if (chained_flags_struct && chained_flags_struct->flags == VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT) {
        const LogObjectList objlist(device);
        skip |=
            ValidateDeviceMaskToPhysicalDeviceCount(chained_flags_struct->deviceMask, objlist,
                                                    allocate_info_loc.pNext(Struct::VkMemoryAllocateFlagsInfo, Field::deviceMask),
                                                    "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00675");
        skip |= ValidateDeviceMaskToZero(chained_flags_struct->deviceMask, objlist,
                                         allocate_info_loc.pNext(Struct::VkMemoryAllocateFlagsInfo, Field::deviceMask),
                                         "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00676");
    }

    if (pAllocateInfo->memoryTypeIndex >= phys_dev_mem_props.memoryTypeCount) {
        skip |= LogError("VUID-vkAllocateMemory-pAllocateInfo-01714", device, allocate_info_loc.dot(Field::memoryTypeIndex),
                         "%" PRIu32 " is not a valid index. Device only advertises %" PRIu32 " memory types.",
                         pAllocateInfo->memoryTypeIndex, phys_dev_mem_props.memoryTypeCount);
    } else {
        const VkMemoryType memory_type = phys_dev_mem_props.memoryTypes[pAllocateInfo->memoryTypeIndex];
        if (!IgnoreAllocationSize(*pAllocateInfo) &&
            pAllocateInfo->allocationSize > phys_dev_mem_props.memoryHeaps[memory_type.heapIndex].size) {
            skip |= LogError("VUID-vkAllocateMemory-pAllocateInfo-01713", device, allocate_info_loc.dot(Field::allocationSize),
                             "is %" PRIu64 " bytes from heap %" PRIu32
                             ","
                             "but size of that heap is only %" PRIu64 " bytes.",
                             pAllocateInfo->allocationSize, memory_type.heapIndex,
                             phys_dev_mem_props.memoryHeaps[memory_type.heapIndex].size);
        }

        if (!enabled_features.deviceCoherentMemory &&
            ((memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) != 0)) {
            skip |= LogError(
                "VUID-vkAllocateMemory-deviceCoherentMemory-02790", device, allocate_info_loc.dot(Field::memoryTypeIndex),
                "%" PRIu32
                " includes the VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD memory property, but the deviceCoherentMemory feature "
                "is not enabled.",
                pAllocateInfo->memoryTypeIndex);
        }

        if ((enabled_features.protectedMemory == VK_FALSE) &&
            ((memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0)) {
            skip |=
                LogError("VUID-VkMemoryAllocateInfo-memoryTypeIndex-01872", device, allocate_info_loc.dot(Field::memoryTypeIndex),
                         "%" PRIu32
                         " includes the VK_MEMORY_PROPERTY_PROTECTED_BIT memory property, but the protectedMemory feature "
                         "is not enabled.",
                         pAllocateInfo->memoryTypeIndex);
        }
    }

    bool imported_ahb_buffer = false;
    bool imported_qnx_buffer = false;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    //  "memory is not an imported Android Hardware Buffer" refers to VkImportAndroidHardwareBufferInfoANDROID with a non-NULL
    //  buffer value. Memory imported has another VUID to check size and allocationSize match up
    if (auto imported_ahb_info = vku::FindStructInPNextChain<VkImportAndroidHardwareBufferInfoANDROID>(pAllocateInfo->pNext);
        imported_ahb_info != nullptr) {
        imported_ahb_buffer = imported_ahb_info->buffer != nullptr;
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    //  "memory is not an imported QNX Screen Buffer" refers to VkImportScreenBufferInfoQNX with a non-NULL
    //  buffer value. Memory imported has another VUID to check size and allocationSize match up
    if (auto imported_buffer_info = vku::FindStructInPNextChain<VkImportScreenBufferInfoQNX>(pAllocateInfo->pNext);
        imported_buffer_info != nullptr) {
        imported_qnx_buffer = imported_buffer_info->buffer != nullptr;
    }
#endif  // VK_USE_PLATFORM_SCREEN_QNX

    VkBuffer dedicated_buffer = VK_NULL_HANDLE;
    VkImage dedicated_image = VK_NULL_HANDLE;
    auto dedicated_allocate_info = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext);
    if (dedicated_allocate_info) {
        dedicated_buffer = dedicated_allocate_info->buffer;
        dedicated_image = dedicated_allocate_info->image;
        if ((dedicated_buffer != VK_NULL_HANDLE) && (dedicated_image != VK_NULL_HANDLE)) {
            skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-01432", device, allocate_info_loc,
                             "pNext<VkMemoryDedicatedAllocateInfo> buffer (%s) or image (%s) has to be VK_NULL_HANDLE.",
                             FormatHandle(dedicated_buffer).c_str(), FormatHandle(dedicated_image).c_str());
        } else if (dedicated_image != VK_NULL_HANDLE) {
            // Dedicated VkImage
            const LogObjectList objlist(device, dedicated_image);
            const Location image_loc = allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image);
            auto image_state = Get<vvl::Image>(dedicated_image);
            ASSERT_AND_RETURN_SKIP(image_state);
            if (image_state->disjoint == true) {
                skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-01797", objlist, image_loc,
                                 "(%s) was created with VK_IMAGE_CREATE_DISJOINT_BIT.", FormatHandle(dedicated_image).c_str());
            } else {
                if (!IgnoreAllocationSize(*pAllocateInfo) && (pAllocateInfo->allocationSize != image_state->requirements[0].size) &&
                    !imported_ahb_buffer && !imported_qnx_buffer) {
                    skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-02964", objlist,
                                     allocate_info_loc.dot(Field::allocationSize),
                                     "(%" PRIu64 ") needs to be equal to %s (%s) VkMemoryRequirements::size (%" PRIu64 ").",
                                     pAllocateInfo->allocationSize, image_loc.Fields().c_str(),
                                     FormatHandle(dedicated_image).c_str(), image_state->requirements[0].size);
                }
                if ((image_state->create_info.flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0) {
                    skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-01434", objlist, image_loc,
                                     "(%s): was created with VK_IMAGE_CREATE_SPARSE_BINDING_BIT.",
                                     FormatHandle(dedicated_image).c_str());
                }
            }
        } else if (dedicated_buffer != VK_NULL_HANDLE) {
            // Dedicated VkBuffer
            const LogObjectList objlist(device, dedicated_buffer);
            const Location buffer_loc = allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer);
            if (auto buffer_state = Get<vvl::Buffer>(dedicated_buffer)) {
                if (!IgnoreAllocationSize(*pAllocateInfo) && (pAllocateInfo->allocationSize != buffer_state->requirements.size) &&
                    !imported_ahb_buffer && !imported_qnx_buffer) {
                    skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-buffer-02965", objlist,
                                     allocate_info_loc.dot(Field::allocationSize),
                                     "(%" PRIu64 ") needs to be equal to %s (%s) VkMemoryRequirements::size (%" PRIu64 ").",
                                     pAllocateInfo->allocationSize, buffer_loc.Fields().c_str(),
                                     FormatHandle(dedicated_buffer).c_str(), buffer_state->requirements.size);
                }
                if ((buffer_state->create_info.flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != 0) {
                    skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01436", objlist, buffer_loc,
                                     "(%s) was created with VK_BUFFER_CREATE_SPARSE_BINDING_BIT.",
                                     FormatHandle(dedicated_buffer).c_str());
                }
            }
        }
    }

    const auto import_memory_fd_info = vku::FindStructInPNextChain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext);
    const bool imported_opaque_fd =
        import_memory_fd_info && import_memory_fd_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    if (imported_opaque_fd) {
        const Location import_loc = allocate_info_loc.pNext(Struct::VkImportMemoryFdInfoKHR, Field::fd);
        if (const auto payload_info = GetOpaqueInfoFromFdHandle(import_memory_fd_info->fd)) {
            if (pAllocateInfo->allocationSize != payload_info->allocation_size) {
                skip |=
                    LogError("VUID-VkMemoryAllocateInfo-allocationSize-01742", device, allocate_info_loc.dot(Field::allocationSize),
                             "allocationSize (%" PRIu64 ") does not match %s (%d) allocationSize (%" PRIu64 ").",
                             pAllocateInfo->allocationSize, import_loc.Fields().c_str(), import_memory_fd_info->fd,
                             payload_info->allocation_size);
            }
            if (pAllocateInfo->memoryTypeIndex != payload_info->memory_type_index) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-allocationSize-01742", device,
                                 allocate_info_loc.dot(Field::memoryTypeIndex),
                                 "memoryTypeIndex (%" PRIu32 ") does not match %s (%d) memoryTypeIndex (%" PRIu32 ").",
                                 pAllocateInfo->memoryTypeIndex, import_loc.Fields().c_str(), import_memory_fd_info->fd,
                                 payload_info->memory_type_index);
            }
            if (dedicated_image != VK_NULL_HANDLE) {
                if (payload_info->dedicated_image == VK_NULL_HANDLE) {
                    skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-01878", dedicated_image,
                                     allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                     "is %s but %s (%d) was not created with a dedicated image.",
                                     FormatHandle(dedicated_image).c_str(), import_loc.Fields().c_str(), import_memory_fd_info->fd);

                } else {
                    auto dedicated_image_state = Get<vvl::Image>(dedicated_image);
                    auto payload_image_state = Get<vvl::Image>(payload_info->dedicated_image);
                    if (!dedicated_image_state || !payload_image_state ||
                        !dedicated_image_state->CompareCreateInfo(*payload_image_state)) {
                        // TODO - Print out info about image creation info
                        const LogObjectList objlist(payload_info->dedicated_image, dedicated_image);
                        skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-01878", objlist,
                                         allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                         "is %s but %s (%d) was created with a dedicated image %s.",
                                         FormatHandle(dedicated_image).c_str(), import_loc.Fields().c_str(),
                                         import_memory_fd_info->fd, FormatHandle(payload_info->dedicated_image).c_str());
                    }
                }
            }
            if (dedicated_buffer != VK_NULL_HANDLE) {
                if (payload_info->dedicated_buffer == VK_NULL_HANDLE) {
                    skip |=
                        LogError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01879", dedicated_buffer,
                                 allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                 "is %s but %s (%d) was not created with a dedicated buffer.",
                                 FormatHandle(dedicated_buffer).c_str(), import_loc.Fields().c_str(), import_memory_fd_info->fd);

                } else {
                    auto dedicated_buffer_state = Get<vvl::Buffer>(dedicated_buffer);
                    auto payload_buffer_state = Get<vvl::Buffer>(payload_info->dedicated_buffer);
                    if (!dedicated_buffer_state || !payload_buffer_state ||
                        !dedicated_buffer_state->CompareCreateInfo(*payload_buffer_state)) {
                        // TODO - Print out info about buffer creation info
                        const LogObjectList objlist(payload_info->dedicated_buffer, dedicated_buffer);
                        skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01879", objlist,
                                         allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                         "is %s but %s (%d) was created with a dedicated buffer %s.",
                                         FormatHandle(dedicated_buffer).c_str(), import_loc.Fields().c_str(),
                                         import_memory_fd_info->fd, FormatHandle(payload_info->dedicated_buffer).c_str());
                    }
                }
            }
        }

        // There is no reasonable way to query all variations of Image/Buffer creation to see what is supported, but if the import
        // has dedicated Image/Buffer, we can at least validate that it has import support
        // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3667
        if (dedicated_image != VK_NULL_HANDLE) {
            auto dedicated_image_state = Get<vvl::Image>(dedicated_image);
            if (dedicated_image_state &&
                !HasExternalMemoryImportSupport(*dedicated_image_state, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)) {
                skip |= LogError("VUID-VkImportMemoryFdInfoKHR-handleType-00667", dedicated_image,
                                 allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                 "is %s but vkGetPhysicalDeviceImageFormatProperties2 shows no support for "
                                 "VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT.",
                                 FormatHandle(dedicated_image).c_str());
            }
        }
        if (dedicated_buffer != VK_NULL_HANDLE) {
            auto dedicated_buffer_state = Get<vvl::Buffer>(dedicated_buffer);
            if (dedicated_buffer_state &&
                !HasExternalMemoryImportSupport(*dedicated_buffer_state, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)) {
                skip |= LogError("VUID-VkImportMemoryFdInfoKHR-handleType-00667", dedicated_buffer,
                                 allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                 "is %s but vkGetPhysicalDeviceExternalBufferProperties shows no support for "
                                 "VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT.",
                                 FormatHandle(dedicated_buffer).c_str());
            }
        }
    }

#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (const auto import_memory_win32_info = vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext)) {
        if (const auto payload_info = GetOpaqueInfoFromWin32Handle(import_memory_win32_info->handle)) {
            if (IsValueIn(import_memory_win32_info->handleType,
                          {VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT})) {
                const Location import_loc = allocate_info_loc.pNext(Struct::VkImportMemoryWin32HandleInfoKHR, Field::handle);
                static_assert(sizeof(HANDLE) == sizeof(uintptr_t));  // to use PRIxPTR for HANDLE formatting
                if (pAllocateInfo->allocationSize != payload_info->allocation_size) {
                    skip |= LogError(
                        "VUID-VkMemoryAllocateInfo-allocationSize-01743", device, allocate_info_loc.dot(Field::allocationSize),
                        "allocationSize (%" PRIu64 ") does not match %s (0x%" PRIxPTR ") of type %s allocationSize (%" PRIu64 ").",
                        pAllocateInfo->allocationSize, import_loc.Fields().c_str(),
                        reinterpret_cast<std::uintptr_t>(import_memory_win32_info->handle),
                        string_VkExternalMemoryHandleTypeFlagBits(import_memory_win32_info->handleType),
                        payload_info->allocation_size);
                }
                if (pAllocateInfo->memoryTypeIndex != payload_info->memory_type_index) {
                    skip |= LogError("VUID-VkMemoryAllocateInfo-allocationSize-01743", device,
                                     allocate_info_loc.dot(Field::memoryTypeIndex),
                                     "memoryTypeIndex (%" PRIu32 ") does not match %s (0x%" PRIxPTR
                                     ") of type %s memoryTypeIndex (%" PRIu32 ").",
                                     pAllocateInfo->memoryTypeIndex, import_loc.Fields().c_str(),
                                     reinterpret_cast<std::uintptr_t>(import_memory_win32_info->handle),
                                     string_VkExternalMemoryHandleTypeFlagBits(import_memory_win32_info->handleType),
                                     payload_info->memory_type_index);
                }
            }
            const Location import_loc = allocate_info_loc.pNext(Struct::VkImportMemoryWin32HandleInfoKHR, Field::handle);
            if (dedicated_image != VK_NULL_HANDLE) {
                if (IsValueIn(
                        import_memory_win32_info->handleType,
                        {VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT,
                         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT,
                         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT})) {
                    if (payload_info->dedicated_image == VK_NULL_HANDLE) {
                        skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-01876", dedicated_image,
                                         allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                         "is %s but %s (0x%" PRIxPTR ") was not created with a dedicated image.",
                                         FormatHandle(dedicated_image).c_str(), import_loc.Fields().c_str(),
                                         reinterpret_cast<std::uintptr_t>(import_memory_win32_info->handle));
                    } else {
                        auto dedicated_image_state = Get<vvl::Image>(dedicated_image);
                        auto payload_image_state = Get<vvl::Image>(payload_info->dedicated_image);
                        if (!dedicated_image_state || !payload_image_state ||
                            !dedicated_image_state->CompareCreateInfo(*payload_image_state)) {
                            // TODO - Print out info about image creation info
                            const LogObjectList objlist(payload_info->dedicated_image, dedicated_image);
                            skip |= LogError("VUID-VkMemoryDedicatedAllocateInfo-image-01876", objlist,
                                             allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                             "is %s but %s (0x%" PRIxPTR ") was created with a dedicated image %s.",
                                             FormatHandle(dedicated_image).c_str(), import_loc.Fields().c_str(),
                                             reinterpret_cast<std::uintptr_t>(import_memory_win32_info->handle),
                                             FormatHandle(payload_info->dedicated_image).c_str());
                        }
                    }
                }
            }
            if (dedicated_buffer != VK_NULL_HANDLE) {
                if (IsValueIn(
                        import_memory_win32_info->handleType,
                        {VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT,
                         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT,
                         VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT})) {
                    if (payload_info->dedicated_buffer == VK_NULL_HANDLE) {
                        skip |=
                            LogError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01877", dedicated_buffer,
                                     allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                     "is %s but %s (0x%" PRIxPTR ") was not created with a dedicated buffer with handleType %s.",
                                     FormatHandle(dedicated_buffer).c_str(), import_loc.Fields().c_str(),
                                     reinterpret_cast<std::uintptr_t>(import_memory_win32_info->handle),
                                     string_VkExternalMemoryHandleTypeFlagBits(import_memory_win32_info->handleType));

                    } else {
                        auto dedicated_buffer_state = Get<vvl::Buffer>(dedicated_buffer);
                        auto payload_buffer_state = Get<vvl::Buffer>(payload_info->dedicated_buffer);
                        if (!dedicated_buffer_state || !payload_buffer_state ||
                            !dedicated_buffer_state->CompareCreateInfo(*payload_buffer_state)) {
                            // TODO - Print out info about buffer creation info
                            const LogObjectList objlist(payload_info->dedicated_buffer, dedicated_buffer);
                            skip |=
                                LogError("VUID-VkMemoryDedicatedAllocateInfo-buffer-01877", objlist,
                                         allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                         "is %s but %s (0x%" PRIxPTR ") was created with a dedicated buffer %s with handleType %s.",
                                         FormatHandle(dedicated_buffer).c_str(), import_loc.Fields().c_str(),
                                         reinterpret_cast<std::uintptr_t>(import_memory_win32_info->handle),
                                         FormatHandle(payload_info->dedicated_buffer).c_str(),
                                         string_VkExternalMemoryHandleTypeFlagBits(import_memory_win32_info->handleType));
                        }
                    }
                }
            }
        } else {  // Handle created by non-Vulkan API (as opposite to using vkGetMemoryWin32HandleKHR)
            constexpr VkExternalMemoryHandleTypeFlags nt_handles_outside_vulkan_api =
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT |
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
            constexpr VkExternalMemoryHandleTypeFlags global_share_handles_outside_vulkan_api =
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;

            if ((import_memory_win32_info->handleType &
                 (nt_handles_outside_vulkan_api | global_share_handles_outside_vulkan_api)) != 0) {
                VkMemoryWin32HandlePropertiesKHR handle_properties = vku::InitStructHelper();
                DispatchGetMemoryWin32HandlePropertiesKHR(device, import_memory_win32_info->handleType,
                                                          import_memory_win32_info->handle, &handle_properties);
                if (((1 << pAllocateInfo->memoryTypeIndex) & handle_properties.memoryTypeBits) == 0) {
                    skip |= LogError(
                        "VUID-VkMemoryAllocateInfo-memoryTypeIndex-00645", device, allocate_info_loc.dot(Field::memoryTypeIndex),
                        "is %" PRIu32 " but VkMemoryWin32HandlePropertiesKHR::memoryTypeBits is 0x%" PRIx32 " with handleType %s.",
                        pAllocateInfo->memoryTypeIndex, handle_properties.memoryTypeBits,
                        string_VkExternalMemoryHandleTypeFlagBits(import_memory_win32_info->handleType));
                }
            }
        }
    }
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
    if (IsExtEnabled(extensions.vk_ext_external_memory_metal)) {
        skip |= ValidateAllocateMemoryMetal(*pAllocateInfo, dedicated_allocate_info, allocate_info_loc);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT

    return skip;
}

bool CoreChecks::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator,
                                           const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto mem_info = Get<vvl::DeviceMemory>(memory)) {
        skip |= ValidateObjectNotInUse(mem_info.get(), error_obj.location, "VUID-vkFreeMemory-memory-00677");
    }
    return skip;
}

bool CoreChecks::ValidateInsertMemoryRange(const VulkanTypedHandle &typed_handle, const vvl::DeviceMemory &mem_info,
                                           VkDeviceSize memoryOffset, const Location &loc) const {
    bool skip = false;

    if (!IgnoreAllocationSize(mem_info.allocate_info) && memoryOffset >= mem_info.allocate_info.allocationSize) {
        const bool bind_2 = (loc.function != Func::vkBindBufferMemory) && (loc.function != Func::vkBindImageMemory);
        const char *vuid = nullptr;
        if (typed_handle.type == kVulkanObjectTypeBuffer) {
            vuid = bind_2 ? "VUID-VkBindBufferMemoryInfo-memoryOffset-01031" : "VUID-vkBindBufferMemory-memoryOffset-01031";
        } else if (typed_handle.type == kVulkanObjectTypeImage) {
            vuid = bind_2 ? "VUID-VkBindImageMemoryInfo-memoryOffset-01046" : "VUID-vkBindImageMemory-memoryOffset-01046";
        } else if (typed_handle.type == kVulkanObjectTypeAccelerationStructureNV) {
            vuid = "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03621";
        } else {
            assert(false);  // Unsupported object type
        }

        LogObjectList objlist(mem_info.Handle(), typed_handle);
        skip |= LogError(vuid, objlist, loc,
                         "attempting to bind %s to %s, memoryOffset (%" PRIu64
                         ") must be less than the memory allocation size (%" PRIu64 ").",
                         FormatHandle(mem_info.Handle()).c_str(), FormatHandle(typed_handle).c_str(), memoryOffset,
                         mem_info.allocate_info.allocationSize);
    }

    return skip;
}

bool CoreChecks::ValidateInsertImageMemoryRange(VkImage image, const vvl::DeviceMemory &mem_info, VkDeviceSize mem_offset,
                                                const Location &loc) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(image, kVulkanObjectTypeImage), mem_info, mem_offset, loc);
}

bool CoreChecks::ValidateInsertBufferMemoryRange(VkBuffer buffer, const vvl::DeviceMemory &mem_info, VkDeviceSize mem_offset,
                                                 const Location &loc) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(buffer, kVulkanObjectTypeBuffer), mem_info, mem_offset, loc);
}

bool CoreChecks::ValidateMemoryTypes(const vvl::DeviceMemory &mem_info, const uint32_t memory_type_bits,
                                     const Location &resource_loc, const char *vuid) const {
    bool skip = false;
    if (((1 << mem_info.allocate_info.memoryTypeIndex) & memory_type_bits) == 0) {
        skip |= LogError(vuid, mem_info.Handle(), resource_loc,
                         "require memoryTypeBits (0x%x) but %s was allocated with memoryTypeIndex (%" PRIu32 ").", memory_type_bits,
                         FormatHandle(mem_info.Handle()).c_str(), mem_info.allocate_info.memoryTypeIndex);
    }
    return skip;
}

bool CoreChecks::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset, const void *pNext,
                                          const Location &loc) const {
    bool skip = false;

    // Validate device group information
    if (const auto *bind_buffer_memory_device_group_info = vku::FindStructInPNextChain<VkBindBufferMemoryDeviceGroupInfo>(pNext)) {
        if (bind_buffer_memory_device_group_info->deviceIndexCount != 0 &&
            bind_buffer_memory_device_group_info->deviceIndexCount != device_group_create_info.physicalDeviceCount &&
            device_group_create_info.physicalDeviceCount > 0) {
            const LogObjectList objlist(buffer, memory);
            skip |= LogError("VUID-VkBindBufferMemoryDeviceGroupInfo-deviceIndexCount-01606", objlist,
                             loc.pNext(Struct::VkBindBufferMemoryDeviceGroupInfo, Field::deviceIndexCount),
                             "(%" PRIu32 ") is not the same as the number of physical devices in the logical device (%" PRIu32 ").",
                             bind_buffer_memory_device_group_info->deviceIndexCount, device_group_create_info.physicalDeviceCount);
        } else {
            for (uint32_t i = 0; i < bind_buffer_memory_device_group_info->deviceIndexCount; ++i) {
                if (bind_buffer_memory_device_group_info->pDeviceIndices[i] >= device_group_create_info.physicalDeviceCount) {
                    const LogObjectList objlist(buffer, memory);
                    skip |= LogError(
                        "VUID-VkBindBufferMemoryDeviceGroupInfo-pDeviceIndices-01607", objlist,
                        loc.pNext(Struct::VkBindBufferMemoryDeviceGroupInfo, Field::pDeviceIndices, i),
                        "(%" PRIu32 ") larger then the number of physical devices in the logical device (%" PRIu32 ").",
                        bind_buffer_memory_device_group_info->pDeviceIndices[i], device_group_create_info.physicalDeviceCount);
                }
            }
        }
    }

    auto buffer_state = Get<vvl::Buffer>(buffer);
    ASSERT_AND_RETURN_SKIP(buffer_state);

    const bool bind_buffer_mem_2 = loc.function != Func::vkBindBufferMemory;

    // Validate memory requirements alignment
    if (SafeModulo(memoryOffset, buffer_state->requirements.alignment) != 0) {
        const char *vuid =
            bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memoryOffset-01036" : "VUID-vkBindBufferMemory-memoryOffset-01036";
        const LogObjectList objlist(buffer, memory);
        skip |= LogError(vuid, objlist, loc.dot(Field::memoryOffset),
                         "is %" PRIu64 " but must be an integer multiple of the VkMemoryRequirements::alignment value %" PRIu64
                         ", returned from a call to vkGetBufferMemoryRequirements with buffer.",
                         memoryOffset, buffer_state->requirements.alignment);
    }

    if (auto mem_info = Get<vvl::DeviceMemory>(memory)) {
        // Track objects tied to memory
        skip |= ValidateSetMemBinding(*mem_info, *buffer_state, loc);

        // Validate VkExportMemoryAllocateInfo's VUs that can't be checked during vkAllocateMemory
        // because they require buffer information.
        if (mem_info->IsExport()) {
            VkPhysicalDeviceExternalBufferInfo external_info = vku::InitStructHelper();
            external_info.flags = buffer_state->create_info.flags;
            // TODO: for now, there is no VkBufferUsageFlags2KHR flag that exceeds 32-bit but should be revisited later
            external_info.usage = VkBufferUsageFlags(buffer_state->usage);
            VkExternalBufferProperties external_properties = vku::InitStructHelper();
            bool export_supported = true;

            auto validate_export_handle_types = [&](VkExternalMemoryHandleTypeFlagBits flag) {
                external_info.handleType = flag;
                DispatchGetPhysicalDeviceExternalBufferPropertiesHelper(api_version, physical_device, &external_info,
                                                                        &external_properties);
                const auto external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
                if ((external_features & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) == 0) {
                    export_supported = false;
                    const LogObjectList objlist(buffer, memory);
                    skip |= LogError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656", objlist, loc,
                                     "The VkDeviceMemory (%s) has VkExportMemoryAllocateInfo::handleTypes with the %s flag "
                                     "set, which does not support VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT with the buffer "
                                     "create flags (%s) and usage flags (%s).",
                                     FormatHandle(memory).c_str(), string_VkExternalMemoryHandleTypeFlagBits(flag),
                                     string_VkBufferCreateFlags(external_info.flags).c_str(),
                                     string_VkBufferUsageFlags(external_info.usage).c_str());
                }
                if ((external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0) {
                    if (!mem_info->IsDedicatedBuffer()) {
                        const LogObjectList objlist(buffer, memory);
                        skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-00639", objlist, loc.dot(Field::memory),
                                         "(%s) has VkExportMemoryAllocateInfo::handleTypes with the %s flag "
                                         "set, which requires dedicated allocation for the buffer created with flags (%s) and "
                                         "usage flags (%s), but the memory is allocated without dedicated allocation support.",
                                         FormatHandle(memory).c_str(), string_VkExternalMemoryHandleTypeFlagBits(flag),
                                         string_VkBufferCreateFlags(external_info.flags).c_str(),
                                         string_VkBufferUsageFlags(external_info.usage).c_str());
                    }
                }
            };
            IterateFlags<VkExternalMemoryHandleTypeFlagBits>(mem_info->export_handle_types, validate_export_handle_types);

            // The types of external memory handles must be compatible
            const auto compatible_types = external_properties.externalMemoryProperties.compatibleHandleTypes;
            if (export_supported && (mem_info->export_handle_types & compatible_types) != mem_info->export_handle_types) {
                const LogObjectList objlist(buffer, memory);
                skip |= LogError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656", objlist, loc.dot(Field::memory),
                                 "(%s) has VkExportMemoryAllocateInfo::handleTypes (%s) that are not "
                                 "reported as compatible by vkGetPhysicalDeviceExternalBufferProperties with the buffer create "
                                 "flags (%s) and usage flags (%s).",
                                 FormatHandle(memory).c_str(),
                                 string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_types).c_str(),
                                 string_VkBufferCreateFlags(external_info.flags).c_str(),
                                 string_VkBufferUsageFlags(external_info.usage).c_str());
            }
        }

        // Validate bound memory range information
        skip |= ValidateInsertBufferMemoryRange(buffer, *mem_info, memoryOffset, loc);

        const char *mem_type_vuid =
            bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memory-01035" : "VUID-vkBindBufferMemory-memory-01035";
        skip |= ValidateMemoryTypes(*mem_info, buffer_state->requirements.memoryTypeBits, loc.dot(Field::buffer), mem_type_vuid);

        // Validate memory requirements size
        if (buffer_state->requirements.size > (mem_info->allocate_info.allocationSize - memoryOffset)) {
            const char *vuid = bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-size-01037" : "VUID-vkBindBufferMemory-size-01037";
            const LogObjectList objlist(buffer, memory);
            skip |= LogError(vuid, objlist, loc,
                             "allocationSize (%" PRIu64 ") minus memoryOffset (%" PRIu64 ") is %" PRIu64
                             " but must be at least as large as VkMemoryRequirements::size value %" PRIu64
                             ", returned from a call to vkGetBufferMemoryRequirements with buffer.",
                             mem_info->allocate_info.allocationSize, memoryOffset,
                             mem_info->allocate_info.allocationSize - memoryOffset, buffer_state->requirements.size);
        }

        // Validate dedicated allocation
        const VkBuffer dedicated_buffer = mem_info->GetDedicatedBuffer();
        if (dedicated_buffer != VK_NULL_HANDLE && ((dedicated_buffer != buffer) || (memoryOffset != 0))) {
            const char *vuid =
                bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memory-01508" : "VUID-vkBindBufferMemory-memory-01508";
            const LogObjectList objlist(buffer, memory, dedicated_buffer);
            skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                             "(%s) is dedicated allocation, but VkMemoryDedicatedAllocateInfo::buffer %s must be equal "
                             "to %s and memoryOffset %" PRIu64 " must be zero.",
                             FormatHandle(memory).c_str(), FormatHandle(dedicated_buffer).c_str(), FormatHandle(buffer).c_str(),
                             memoryOffset);
        } else if (IsExtEnabled(extensions.vk_khr_dedicated_allocation)) {
            VkBufferMemoryRequirementsInfo2 buffer_memory_requirements_info_2 = vku::InitStructHelper();
            buffer_memory_requirements_info_2.buffer = buffer;
            VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();
            VkMemoryRequirements2 memory_requirements = vku::InitStructHelper(&memory_dedicated_requirements);
            DispatchGetBufferMemoryRequirements2Helper(api_version, device, &buffer_memory_requirements_info_2, &memory_requirements);

            if (memory_dedicated_requirements.requiresDedicatedAllocation) {
                const char *vuid =
                    bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-buffer-01444" : "VUID-vkBindBufferMemory-buffer-01444";
                if (dedicated_buffer == VK_NULL_HANDLE) {
                    const LogObjectList objlist(buffer, memory);
                    skip |= LogError(
                        vuid, objlist, loc.dot(Field::memory),
                        "was created without a VkMemoryDedicatedAllocateInfo in the pNext chain, but the buffer, if queried with "
                        "vkGetBufferMemoryRequirements2() reports requiresDedicatedAllocation is VK_TRUE.");
                } else if (dedicated_buffer != buffer) {
                    const LogObjectList objlist(buffer, memory, dedicated_buffer);
                    skip |=
                        LogError(vuid, objlist, loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::pNext).dot(Field::buffer),
                                 "is %s, but VkBindBufferMemoryInfo::buffer is %s.", FormatHandle(dedicated_buffer).c_str(),
                                 FormatHandle(buffer).c_str());
                }
            }
        }

        auto chained_flags_struct = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(mem_info->allocate_info.pNext);
        if (enabled_features.bufferDeviceAddress && (buffer_state->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) &&
            (!chained_flags_struct || !(chained_flags_struct->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT))) {
            const LogObjectList objlist(buffer, memory);
            const char *vuid = bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-bufferDeviceAddress-03339"
                                                 : "VUID-vkBindBufferMemory-bufferDeviceAddress-03339";
            skip |= LogError(vuid, objlist, loc.dot(Field::buffer),
                             "was created with the VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT bit set, "
                             "memory must have been allocated with the VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT bit set.");
        }
        const VkMemoryAllocateFlags memory_allocate_flags = chained_flags_struct ? chained_flags_struct->flags : 0;
        if (buffer_state->create_info.flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) {
            if (!(memory_allocate_flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)) {
                const char *vuid = bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-descriptorBufferCaptureReplay-08112"
                                                     : "VUID-vkBindBufferMemory-descriptorBufferCaptureReplay-08112";
                const LogObjectList objlist(buffer, memory);
                skip |= LogError(vuid, objlist, loc.dot(Field::buffer),
                                 "was created with the VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT bit set,"
                                 "but the bound memory was allocated with %s and needs VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT.",
                                 string_VkMemoryAllocateFlags(memory_allocate_flags).c_str());
            }

            if (!(memory_allocate_flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                const char *vuid =
                    bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-buffer-09201" : "VUID-vkBindBufferMemory-buffer-09201";
                const LogObjectList objlist(buffer, memory);
                skip |= LogError(vuid, objlist, loc.dot(Field::buffer),
                                 "was created with the VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT bit set,"
                                 "but the bound memory was allocated with %s and needs "
                                 "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                                 string_VkMemoryAllocateFlags(memory_allocate_flags).c_str());
            }

            if (enabled_features.descriptorBufferCaptureReplay) {
                if (!(memory_allocate_flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
                    const char *vuid = bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-bufferDeviceAddressCaptureReplay-09200"
                                                         : "VUID-vkBindBufferMemory-bufferDeviceAddressCaptureReplay-09200";
                    const LogObjectList objlist(buffer, memory);
                    skip |= LogError(vuid, objlist, loc.dot(Field::buffer),
                                     "was created with the VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT bit set,"
                                     "but the bound memory was allocated with %s and needs "
                                     "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                                     string_VkMemoryAllocateFlags(memory_allocate_flags).c_str());
                }
            }
        }

        // Validate export memory handles. Check if the memory meets the buffer's external memory requirements
        if (mem_info->IsExport() && (mem_info->export_handle_types & buffer_state->external_memory_handle_types) == 0) {
            const char *vuid =
                bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memory-02726" : "VUID-vkBindBufferMemory-memory-02726";
            const LogObjectList objlist(buffer, memory);
            skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                             "(%s) has an external handleType of %s which does not include at least one "
                             "handle from VkBuffer (%s) handleType %s.",
                             FormatHandle(memory).c_str(),
                             string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_types).c_str(),
                             FormatHandle(buffer).c_str(),
                             string_VkExternalMemoryHandleTypeFlags(buffer_state->external_memory_handle_types).c_str());
        }

        // Validate import memory handles
        if (mem_info->IsImportAHB()) {
            skip |= ValidateBufferImportedHandleANDROID(buffer_state->external_memory_handle_types, memory, buffer, loc);
        } else if (mem_info->IsImport()) {
            if ((mem_info->import_handle_type.value() & buffer_state->external_memory_handle_types) == 0) {
                const char *vuid =
                    bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memory-02985" : "VUID-vkBindBufferMemory-memory-02985";
                const LogObjectList objlist(buffer, memory);
                skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                                 "(%s) was created with an import operation with handleType of %s which "
                                 "is not set in the VkBuffer (%s) VkExternalMemoryBufferCreateInfo::handleType (%s)",
                                 FormatHandle(memory).c_str(),
                                 string_VkExternalMemoryHandleTypeFlagBits(mem_info->import_handle_type.value()),
                                 FormatHandle(buffer).c_str(),
                                 string_VkExternalMemoryHandleTypeFlags(buffer_state->external_memory_handle_types).c_str());
            }
            // Check if buffer can be bound to memory imported from specific handle type
            if (!HasExternalMemoryImportSupport(*buffer_state, mem_info->import_handle_type.value())) {
                const LogObjectList objlist(buffer, memory);
                skip |= LogError(
                    "VUID-VkImportMemoryWin32HandleInfoKHR-handleType-00658", objlist, loc.dot(Field::memory),
                    "(%s) was imported from handleType %s but VkExternalBufferProperties does not report it as importable.",
                    FormatHandle(memory).c_str(), string_VkExternalMemoryHandleTypeFlagBits(mem_info->import_handle_type.value()));
            }
        }

        // Validate mix of protected buffer and memory
        if ((buffer_state->unprotected == false) && (mem_info->unprotected == true)) {
            const char *vuid = bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-None-01898" : "VUID-vkBindBufferMemory-None-01898";
            const LogObjectList objlist(buffer, memory);
            skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                             "(%s) was not created with protected memory but the VkBuffer (%s) was set "
                             "to use protected memory.",
                             FormatHandle(memory).c_str(), FormatHandle(buffer).c_str());
        } else if ((buffer_state->unprotected == true) && (mem_info->unprotected == false)) {
            const char *vuid = bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-None-01899" : "VUID-vkBindBufferMemory-None-01899";
            const LogObjectList objlist(buffer, memory);
            skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                             "(%s) was created with protected memory but the VkBuffer (%s) was not set "
                             "to use protected memory.",
                             FormatHandle(memory).c_str(), FormatHandle(buffer).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                 const ErrorObject &error_obj) const {
    return ValidateBindBufferMemory(buffer, memory, memoryOffset, nullptr, error_obj.location);
}

bool CoreChecks::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const Location loc = error_obj.location.dot(Field::pBindInfos, i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset,
                                         pBindInfos[i].pNext, loc);
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo *pBindInfos, const ErrorObject &error_obj) const {
    return PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos, error_obj);
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                           VkMemoryRequirements *pMemoryRequirements,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const Location image_loc = error_obj.location.dot(Field::image);
    skip |= ValidateGetImageMemoryRequirementsANDROID(image, image_loc);

    if (auto image_state = Get<vvl::Image>(image)) {
        // Checks for no disjoint bit
        if (image_state->disjoint == true) {
            skip |= LogError("VUID-vkGetImageMemoryRequirements-image-01588", image, image_loc,
                             "(%s) must not have been created with the VK_IMAGE_CREATE_DISJOINT_BIT "
                             "(need to use vkGetImageMemoryRequirements2).",
                             FormatHandle(image).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                            VkMemoryRequirements2 *pMemoryRequirements,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const Location info_loc = error_obj.location.dot(Field::pInfo);
    const Location image_loc = info_loc.dot(Field::image);
    skip |= ValidateGetImageMemoryRequirementsANDROID(pInfo->image, image_loc);

    auto image_state = Get<vvl::Image>(pInfo->image);
    ASSERT_AND_RETURN_SKIP(image_state);
    const VkFormat image_format = image_state->create_info.format;
    const VkImageTiling image_tiling = image_state->create_info.tiling;
    const auto *image_plane_info = vku::FindStructInPNextChain<VkImagePlaneMemoryRequirementsInfo>(pInfo->pNext);
    if (!image_plane_info && image_state->disjoint) {
        if (vkuFormatIsMultiplane(image_format)) {
            skip |= LogError("VUID-VkImageMemoryRequirementsInfo2-image-01589", pInfo->image, image_loc,
                             "(%s) was created with a multi-planar format (%s) and "
                             "VK_IMAGE_CREATE_DISJOINT_BIT, but the current pNext doesn't include a "
                             "VkImagePlaneMemoryRequirementsInfo struct",
                             FormatHandle(pInfo->image).c_str(), string_VkFormat(image_format));
        }
        if (image_state->create_info.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            skip |= LogError("VUID-VkImageMemoryRequirementsInfo2-image-02279", pInfo->image, image_loc,
                             "(%s) was created with VK_IMAGE_CREATE_DISJOINT_BIT and has tiling of "
                             "VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, "
                             "but the current pNext does not include a VkImagePlaneMemoryRequirementsInfo struct",
                             FormatHandle(pInfo->image).c_str());
        }
    } else if (image_plane_info) {
        if ((image_state->disjoint == false)) {
            skip |= LogError("VUID-VkImageMemoryRequirementsInfo2-image-01590", pInfo->image, image_loc,
                             "(%s) was not created with VK_IMAGE_CREATE_DISJOINT_BIT,"
                             "but the current pNext includes a VkImagePlaneMemoryRequirementsInfo struct",
                             FormatHandle(pInfo->image).c_str());
        }

        if ((vkuFormatIsMultiplane(image_format) == false) && (image_tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)) {
            skip |= LogError("VUID-VkImageMemoryRequirementsInfo2-image-02280", pInfo->image, image_loc,
                             "(%s) is a single-plane format (%s) and does not have tiling of "
                             "VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT,"
                             "but the current pNext includes a VkImagePlaneMemoryRequirementsInfo struct",
                             FormatHandle(pInfo->image).c_str(), string_VkFormat(image_format));
        }

        const VkImageAspectFlags aspect = image_plane_info->planeAspect;
        if ((image_tiling == VK_IMAGE_TILING_LINEAR) || (image_tiling == VK_IMAGE_TILING_OPTIMAL)) {
            // Make sure planeAspect is only a single, valid plane
            if (vkuFormatIsMultiplane(image_format) && !IsOnlyOneValidPlaneAspect(image_format, aspect)) {
                skip |=
                    LogError("VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281", pInfo->image,
                             info_loc.pNext(Struct::VkImagePlaneMemoryRequirementsInfo, Field::planeAspect),
                             "%s but is invalid for %s.", string_VkImageAspectFlags(aspect).c_str(), string_VkFormat(image_format));
            }
        } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            // TODO - Need to also check if lower then drmFormatModifierPlaneCount
            if (GetBitSetCount(aspect) > 1 ||
                !IsValueIn(VkImageAspectFlagBits(aspect),
                           {VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
                            VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT})) {
                skip |=
                    LogError("VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02282", pInfo->image,
                             info_loc.pNext(Struct::VkImagePlaneMemoryRequirementsInfo, Field::planeAspect),
                             "%s but is invalid for %s.", string_VkImageAspectFlags(aspect).c_str(), string_VkFormat(image_format));
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                               VkMemoryRequirements2 *pMemoryRequirements,
                                                               const ErrorObject &error_obj) const {
    return PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements, error_obj);
}

bool CoreChecks::ValidateMapMemory(const vvl::DeviceMemory &mem_info, VkDeviceSize offset, VkDeviceSize size,
                                   const Location &offset_loc, const Location &size_loc) const {
    bool skip = false;
    const bool map2 = offset_loc.function != Func::vkMapMemory;
    const Location loc(offset_loc.function);
    const VkDeviceMemory memory = mem_info.VkHandle();

    const uint32_t memoryTypeIndex = mem_info.allocate_info.memoryTypeIndex;
    const VkMemoryPropertyFlags propertyFlags = phys_dev_mem_props.memoryTypes[memoryTypeIndex].propertyFlags;
    if ((propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
        skip |= LogError(map2 ? "VUID-VkMemoryMapInfo-memory-07962" : "VUID-vkMapMemory-memory-00682", memory, loc,
                         "Mapping memory without VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set. "
                         "Memory has type %" PRIu32 " which has properties %s.",
                         memoryTypeIndex, string_VkMemoryPropertyFlags(propertyFlags).c_str());
    }

    if (mem_info.multi_instance) {
        skip |= LogError(map2 ? "VUID-VkMemoryMapInfo-memory-07963" : "VUID-vkMapMemory-memory-00683", memory, loc,
                         "Memory allocated with multiple instances.");
    }

    if (size == 0) {
        skip |= LogError(map2 ? "VUID-VkMemoryMapInfo-size-07960" : "VUID-vkMapMemory-size-00680", memory, size_loc, "is zero.");
    }

    // It is an application error to call VkMapMemory on an object that is already mapped
    if (mem_info.mapped_range.size != 0) {
        skip |= LogError(map2 ? "VUID-VkMemoryMapInfo-memory-07958" : "VUID-vkMapMemory-memory-00678", memory, loc,
                         "memory has already be mapped.");
    }

    // Validate offset is not over allocation size
    const VkDeviceSize allocationSize = mem_info.allocate_info.allocationSize;
    if (offset >= allocationSize) {
        skip |= LogError(map2 ? "VUID-VkMemoryMapInfo-offset-07959" : "VUID-vkMapMemory-offset-00679", memory, offset_loc,
                         "0x%" PRIx64 " is larger than the total array size 0x%" PRIx64, offset, allocationSize);
    }
    // Validate that offset + size is within object's allocationSize
    if (size != VK_WHOLE_SIZE) {
        if ((offset + size) > allocationSize) {
            skip |=
                LogError(map2 ? "VUID-VkMemoryMapInfo-size-07961" : "VUID-vkMapMemory-size-00681", memory, offset_loc,
                         "0x%" PRIx64 " plus size 0x%" PRIx64 " (total 0x%" PRIx64 ") oversteps total array size 0x%" PRIx64 ".",
                         offset, size, size + offset, allocationSize);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
                                          VkFlags flags, void **ppData, const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto mem_info = Get<vvl::DeviceMemory>(memory)) {
        skip |= ValidateMapMemory(*mem_info.get(), offset, size, error_obj.location.dot(Field::offset),
                                  error_obj.location.dot(Field::size));

        if (flags & VK_MEMORY_MAP_PLACED_BIT_EXT) {
            skip |= LogError("VUID-vkMapMemory-flags-09568", memory, error_obj.location.dot(Field::flags),
                             "VK_MEMORY_MAP_PLACED_BIT_EXT is not allowed in vkMapMemory()");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateMapMemory2(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData,
                                           const ErrorObject &error_obj) const {
    bool skip = false;
    auto mem_info = Get<vvl::DeviceMemory>(pMemoryMapInfo->memory);
    ASSERT_AND_RETURN_SKIP(mem_info);

    const Location info_loc = error_obj.location.dot(Field::pMemoryMapInfo);
    skip |= ValidateMapMemory(*mem_info.get(), pMemoryMapInfo->offset, pMemoryMapInfo->size, info_loc.dot(Field::offset),
                              info_loc.dot(Field::size));

    if (pMemoryMapInfo->flags & VK_MEMORY_MAP_PLACED_BIT_EXT) {
        if (!enabled_features.memoryMapPlaced) {
            skip |= LogError("VUID-VkMemoryMapInfo-flags-09569", pMemoryMapInfo->memory, error_obj.location,
                             "(%s) has VK_MEMORY_MAP_PLACED_BIT_EXT set but memoryMapPlaced is not enabled",
                             string_VkMemoryMapFlags(pMemoryMapInfo->flags).c_str());
        }

        if (enabled_features.memoryMapRangePlaced) {
            if (pMemoryMapInfo->offset % phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment != 0) {
                skip |= LogError("VUID-VkMemoryMapInfo-flags-09573", pMemoryMapInfo->memory, info_loc.dot(Field::offset),
                                 "(0x%" PRIx64
                                 ") is not an integer multiple of "
                                 "minPlacedMemoryMapAlignment (0x%" PRIx64 ") while VK_MEMORY_MAP_PLACED_BIT_EXT is set",
                                 pMemoryMapInfo->offset, phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment);
            }
        } else {
            if (pMemoryMapInfo->offset != 0) {
                skip |= LogError("VUID-VkMemoryMapInfo-flags-09571", pMemoryMapInfo->memory, info_loc.dot(Field::offset),
                                 "(0x%" PRIx64
                                 ") is not zero while VK_MEMORY_MAP_PLACED_BIT_EXT is set and "
                                 "memoryMapRangePlaced is not enabled.",
                                 pMemoryMapInfo->offset);
            }

            if (pMemoryMapInfo->size != VK_WHOLE_SIZE && pMemoryMapInfo->size != mem_info->allocate_info.allocationSize) {
                skip |= LogError("VUID-VkMemoryMapInfo-flags-09572", pMemoryMapInfo->memory, info_loc.dot(Field::size),
                                 "(0x%" PRIx64 ") is not VK_WHOLE_SIZE or the size of memory (%" PRIu64
                                 ") while VK_MEMORY_MAP_PLACED_BIT_EXT is set and memoryMapRangePlaced is not enabled.",
                                 pMemoryMapInfo->size, mem_info->allocate_info.allocationSize);
            }
        }

        if (pMemoryMapInfo->size == VK_WHOLE_SIZE) {
            if (mem_info->allocate_info.allocationSize % phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment !=
                0) {
                skip |= LogError("VUID-VkMemoryMapInfo-flags-09651", pMemoryMapInfo->memory, info_loc.dot(Field::size),
                                 "is VK_WHOLE_SIZE but the size of the memory (0x%" PRIx64
                                 ") is not an integer multiple of minPlacedMemoryMapAlignment (0x%" PRIx64 ")",
                                 mem_info->allocate_info.allocationSize,
                                 phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment);
            }
        } else {
            if (pMemoryMapInfo->size % phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment != 0) {
                skip |= LogError("VUID-VkMemoryMapInfo-flags-09574", pMemoryMapInfo->memory, info_loc.dot(Field::size),
                                 "(0x%" PRIx64
                                 ") is not VK_WHOLE_SIZE and is not an integer multiple of "
                                 "minPlacedMemoryMapAlignment (0x%" PRIx64 ") while VK_MEMORY_MAP_PLACED_BIT_EXT is set",
                                 pMemoryMapInfo->size, phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment);
            }
        }

        if (mem_info->IsImport() &&
            (mem_info->import_handle_type.value() == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT ||
             mem_info->import_handle_type.value() == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT)) {
            skip |= LogError("VUID-VkMemoryMapInfo-flags-09575", pMemoryMapInfo->memory, info_loc.dot(Field::flags),
                             "(%s) has VK_MEMORY_MAP_PLACED_BIT_EXT set but memory was imported with a handle type of %s",
                             string_VkMemoryMapFlags(pMemoryMapInfo->flags).c_str(),
                             string_VkExternalMemoryHandleTypeFlagBits(mem_info->import_handle_type.value()));
        }

        const auto placed_info = vku::FindStructInPNextChain<VkMemoryMapPlacedInfoEXT>(pMemoryMapInfo->pNext);
        const auto addr_loc = info_loc.pNext(Struct::VkMemoryMapPlacedInfoEXT, Field::pPlacedAddress);
        if (placed_info == NULL) {
            skip |= LogError("VUID-VkMemoryMapInfo-flags-09570", pMemoryMapInfo->memory, info_loc.dot(Field::pNext),
                             "does not contain VkMemoryMapPlacedInfoEXT, but VK_MEMORY_MAP_PLACED_BIT_EXT was set in flags (%s)",
                             string_VkMemoryMapFlags(pMemoryMapInfo->flags).c_str());
        } else if (placed_info->pPlacedAddress == NULL) {
            skip |= LogError("VUID-VkMemoryMapInfo-flags-09570", pMemoryMapInfo->memory, addr_loc,
                             "is NULL, but VK_MEMORY_MAP_PLACED_BIT_EXT was set in flags (%s)",
                             string_VkMemoryMapFlags(pMemoryMapInfo->flags).c_str());
        } else if (reinterpret_cast<std::uintptr_t>(placed_info->pPlacedAddress) %
                       phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment !=
                   0) {
            skip |= LogError("VUID-VkMemoryMapPlacedInfoEXT-pPlacedAddress-09577", pMemoryMapInfo->memory, addr_loc,
                             "(%p) is not an integer multiple of "
                             "minPlacedMemoryMapAlignment (0x%" PRIx64 ")",
                             placed_info->pPlacedAddress, phys_dev_ext_props.map_memory_placed_props.minPlacedMemoryMapAlignment);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR *pMemoryMapInfo, void **ppData,
                                              const ErrorObject &error_obj) const {
    return PreCallValidateMapMemory2(device, pMemoryMapInfo, ppData, error_obj);
}

bool CoreChecks::PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory memory, const ErrorObject &error_obj) const {
    bool skip = false;
    auto mem_info = Get<vvl::DeviceMemory>(memory);
    ASSERT_AND_RETURN_SKIP(mem_info);
    if (!mem_info->mapped_range.size) {
        skip |= LogError("VUID-vkUnmapMemory-memory-00689", memory, error_obj.location,
                         "Unmapping Memory without memory being mapped.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo,
                                             const ErrorObject &error_obj) const {
    bool skip = false;
    auto mem_info = Get<vvl::DeviceMemory>(pMemoryUnmapInfo->memory);
    ASSERT_AND_RETURN_SKIP(mem_info);
    if (!mem_info->mapped_range.size) {
        const Location info_loc = error_obj.location.dot(Field::pMemoryUnmapInfo);
        skip |= LogError("VUID-VkMemoryUnmapInfo-memory-07964", pMemoryUnmapInfo->memory, error_obj.location,
                         "Unmapping Memory without memory being mapped.");

        if (pMemoryUnmapInfo->flags & VK_MEMORY_UNMAP_RESERVE_BIT_EXT) {
            if (!enabled_features.memoryUnmapReserve) {
                skip |= LogError("VUID-VkMemoryUnmapInfo-flags-09579", pMemoryUnmapInfo->memory, info_loc.dot(Field::flags),
                                 "VK_MEMORY_MAP_PLACED_BIT_EXT is set but memoryUnmapReserve is not enabled");
            }

            if (mem_info->IsImport() &&
                (mem_info->import_handle_type.value() == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT ||
                 mem_info->import_handle_type.value() == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT)) {
                skip |= LogError("VUID-VkMemoryUnmapInfo-flags-09580", pMemoryUnmapInfo->memory, info_loc.dot(Field::flags),
                                 "VK_MEMORY_MAP_PLACED_BIT_EXT is set but memory was imported with a handle type of %s",
                                 string_VkExternalMemoryHandleTypeFlagBits(mem_info->import_handle_type.value()));
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR *pMemoryUnmapInfo,
                                                const ErrorObject &error_obj) const {
    return PreCallValidateUnmapMemory2(device, pMemoryUnmapInfo, error_obj);
}

bool CoreChecks::ValidateMemoryIsMapped(uint32_t mem_range_count, const VkMappedMemoryRange *mem_ranges,
                                        const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < mem_range_count; ++i) {
        const Location memory_range_loc = error_obj.location.dot(Field::pMemoryRanges, i);
        auto mem_info = Get<vvl::DeviceMemory>(mem_ranges[i].memory);
        ASSERT_AND_CONTINUE(mem_info);
        // Makes sure the memory is already mapped
        if (mem_info->mapped_range.size == 0) {
            skip |= LogError("VUID-VkMappedMemoryRange-memory-00684", mem_ranges[i].memory, memory_range_loc,
                             "Attempting to use memory (%s) that is not currently host mapped.",
                             FormatHandle(mem_ranges[i].memory).c_str());
        }

        if (mem_ranges[i].size == VK_WHOLE_SIZE) {
            if (mem_info->mapped_range.offset > mem_ranges[i].offset) {
                skip |= LogError("VUID-VkMappedMemoryRange-size-00686", mem_ranges[i].memory, memory_range_loc.dot(Field::offset),
                                 "(%" PRIu64 ") is less than the mapped memory offset (%" PRIu64 ") (and size is VK_WHOLE_SIZE).",
                                 mem_ranges[i].offset, mem_info->mapped_range.offset);
            }
        } else {
            if (mem_info->mapped_range.offset > mem_ranges[i].offset) {
                skip |=
                    LogError("VUID-VkMappedMemoryRange-size-00685", mem_ranges[i].memory, memory_range_loc.dot(Field::offset),
                             "(%" PRIu64 ") is less than the mapped memory offset (%" PRIu64 ") (and size is not VK_WHOLE_SIZE).",
                             mem_ranges[i].offset, mem_info->mapped_range.offset);
            }
            const uint64_t data_end = (mem_info->mapped_range.size == VK_WHOLE_SIZE)
                                          ? mem_info->allocate_info.allocationSize
                                          : (mem_info->mapped_range.offset + mem_info->mapped_range.size);
            if ((data_end < (mem_ranges[i].offset + mem_ranges[i].size))) {
                skip |= LogError("VUID-VkMappedMemoryRange-size-00685", mem_ranges[i].memory, memory_range_loc,
                                 "size (%" PRIu64 ") + offset (%" PRIu64
                                 ") "
                                 "exceed the Memory Object's upper-bound (%" PRIu64 ").",
                                 mem_ranges[i].size, mem_ranges[i].offset, data_end);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateMappedMemoryRangeDeviceLimits(uint32_t mem_range_count, const VkMappedMemoryRange *mem_ranges,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    for (uint32_t i = 0; i < mem_range_count; ++i) {
        const Location memory_range_loc = error_obj.location.dot(Field::pMemoryRanges, i);
        const uint64_t atom_size = phys_dev_props.limits.nonCoherentAtomSize;
        const VkDeviceSize offset = mem_ranges[i].offset;
        const VkDeviceSize size = mem_ranges[i].size;

        if (SafeModulo(offset, atom_size) != 0) {
            skip |= LogError("VUID-VkMappedMemoryRange-offset-00687", mem_ranges->memory, memory_range_loc.dot(Field::offset),
                             "(%" PRIu64 ") is not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (%" PRIu64 ").",
                             offset, atom_size);
        }
        auto mem_info = Get<vvl::DeviceMemory>(mem_ranges[i].memory);
        ASSERT_AND_CONTINUE(mem_info);

        const auto allocation_size = mem_info->allocate_info.allocationSize;
        if (size == VK_WHOLE_SIZE) {
            const auto mapping_offset = mem_info->mapped_range.offset;
            const auto mapping_size = mem_info->mapped_range.size;
            const auto mapping_end = ((mapping_size == VK_WHOLE_SIZE) ? allocation_size : mapping_offset + mapping_size);
            if (SafeModulo(mapping_end, atom_size) != 0 && mapping_end != allocation_size) {
                skip |= LogError("VUID-VkMappedMemoryRange-size-01389", mem_ranges->memory, memory_range_loc.dot(Field::size),
                                 "is VK_WHOLE_SIZE and the mapping end (%" PRIu64 " = %" PRIu64 " + %" PRIu64
                                 ") not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (%" PRIu64
                                 ") and not equal to the end of the memory object (%" PRIu64 ").",
                                 mapping_end, mapping_offset, mapping_size, atom_size, allocation_size);
            }
        } else {
            const auto range_end = size + offset;
            if (range_end != allocation_size && SafeModulo(size, atom_size) != 0) {
                skip |= LogError("VUID-VkMappedMemoryRange-size-01390", mem_ranges->memory, memory_range_loc.dot(Field::size),
                                 "(%" PRIu64 ") is not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (%" PRIu64
                                 ") and offset + size (%" PRIu64 " + %" PRIu64 " = %" PRIu64
                                 ") not equal to the memory size (%" PRIu64 ").",
                                 size, atom_size, offset, size, range_end, allocation_size);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                        const VkMappedMemoryRange *pMemoryRanges,
                                                        const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateMappedMemoryRangeDeviceLimits(memoryRangeCount, pMemoryRanges, error_obj);
    skip |= ValidateMemoryIsMapped(memoryRangeCount, pMemoryRanges, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
                                                             const VkMappedMemoryRange *pMemoryRanges,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    skip |= ValidateMappedMemoryRangeDeviceLimits(memoryRangeCount, pMemoryRanges, error_obj);
    skip |= ValidateMemoryIsMapped(memoryRangeCount, pMemoryRanges, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize *pCommittedMem,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto mem_info = Get<vvl::DeviceMemory>(memory)) {
        if ((phys_dev_mem_props.memoryTypes[mem_info->allocate_info.memoryTypeIndex].propertyFlags &
             VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == 0) {
            skip |= LogError("VUID-vkGetDeviceMemoryCommitment-memory-00690", memory, error_obj.location,
                             "Querying commitment for memory without "
                             "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT set: %s.",
                             FormatHandle(memory).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateBindImageMemory(uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                         const ErrorObject &error_obj) const {
    bool skip = false;
    const bool bind_image_mem_2 = error_obj.location.function != Func::vkBindImageMemory;

    // Track all image sub resources if they are bound for bind_image_mem_2
    // uint32_t[3] is which index in pBindInfos for max 3 planes
    // Non disjoint images act as a single plane
    vvl::unordered_map<VkImage, std::array<uint32_t, 3>> resources_bound;
    bool is_drm = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const Location loc = bind_image_mem_2 ? error_obj.location.dot(Field::pBindInfos, i) : error_obj.location.function;
        const VkBindImageMemoryInfo &bind_info = pBindInfos[i];
        if (auto image_state = Get<vvl::Image>(bind_info.image)) {
            auto mem_info = Get<vvl::DeviceMemory>(bind_info.memory);
            if (mem_info) {
                // Track objects tied to memory
                skip |= ValidateSetMemBinding(*mem_info, *image_state, loc);
            }

            const auto plane_info = vku::FindStructInPNextChain<VkBindImagePlaneMemoryInfo>(bind_info.pNext);

            if (image_state->disjoint && plane_info == nullptr) {
                const LogObjectList objlist(bind_info.image, bind_info.memory);
                skip |= LogError("VUID-VkBindImageMemoryInfo-image-07736", objlist, loc.dot(Field::image),
                                 "is disjoint, add a VkBindImagePlaneMemoryInfo structure to the pNext chain of "
                                 "VkBindImageMemoryInfo in order to bind planes of a disjoint image.");
            }

            // Currently disjoint planes only work with non-DRM
            if (plane_info && IsValueIn(plane_info->planeAspect,
                                        {VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
                                         VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT})) {
                is_drm = true;
            }

            // Need extra check for disjoint flag incase called without bindImage2 and don't want false positive errors
            // no 'else' case as if that happens another VUID is already being triggered for it being invalid
            if ((plane_info == nullptr) && (image_state->disjoint == false)) {
                // Check non-disjoint images VkMemoryRequirements

                // All validation using the image_state->requirements for external AHB is check in android only section
                if (image_state->IsExternalBuffer() == false) {
                    const VkMemoryRequirements &mem_req = image_state->requirements[0];

                    // Validate memory requirements alignment
                    if (SafeModulo(bind_info.memoryOffset, mem_req.alignment) != 0) {
                        const char *vuid = bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-pNext-01616"
                                                            : "VUID-vkBindImageMemory-memoryOffset-01048";
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError(vuid, objlist, loc.dot(Field::memoryOffset),
                                         "is %" PRIu64
                                         " but must be an integer multiple of the VkMemoryRequirements::alignment value %" PRIu64
                                         ", returned from a call to vkGetImageMemoryRequirements with image.",
                                         bind_info.memoryOffset, mem_req.alignment);
                    }

                    if (mem_info) {
                        const VkMemoryAllocateInfo &allocate_info = mem_info->allocate_info;
                        // Validate memory requirements size
                        if (!IgnoreAllocationSize(allocate_info) &&
                            mem_req.size > allocate_info.allocationSize - bind_info.memoryOffset) {
                            const char *vuid =
                                bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-pNext-01617" : "VUID-vkBindImageMemory-size-01049";
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError(vuid, objlist, loc,
                                             "allocationSize (%" PRIu64 ") minus memoryOffset (%" PRIu64 ") is %" PRIu64
                                             " but must be at least as large as VkMemoryRequirements::size value %" PRIu64
                                             ", returned from a call to vkGetImageMemoryRequirements with image.",
                                             allocate_info.allocationSize, bind_info.memoryOffset,
                                             allocate_info.allocationSize - bind_info.memoryOffset, mem_req.size);
                        }

                        // Validate memory type used
                        {
                            const char *vuid =
                                bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-pNext-01615" : "VUID-vkBindImageMemory-memory-01047";
                            skip |= ValidateMemoryTypes(*mem_info, mem_req.memoryTypeBits, loc.dot(Field::image), vuid);
                        }
                    }
                }

                if (bind_image_mem_2 == true) {
                    // since its a non-disjoint image, finding VkImage in map is a duplicate
                    auto it = resources_bound.find(image_state->VkHandle());
                    if (it == resources_bound.end()) {
                        std::array<uint32_t, 3> bound_index = {i, vvl::kU32Max, vvl::kU32Max};
                        resources_bound.emplace(image_state->VkHandle(), bound_index);
                    } else {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError("VUID-vkBindImageMemory2-pBindInfos-04006", objlist, loc.dot(Field::image),
                                         "is non-disjoint and is being bound twice at pBindInfos[%d]", it->second[0]);
                    }
                }
            } else if ((plane_info != nullptr) && (image_state->disjoint == true) && !is_drm) {
                // Check disjoint images VkMemoryRequirements for given plane
                int plane = 0;

                // All validation using the image_state->plane*_requirements for external AHB is check in android only section
                if (image_state->IsExternalBuffer() == false) {
                    const VkImageAspectFlagBits aspect = plane_info->planeAspect;
                    plane = vkuGetPlaneIndex(aspect);
                    const VkMemoryRequirements &disjoint_mem_req = image_state->requirements[plane];

                    // Validate memory requirements alignment
                    if (SafeModulo(bind_info.memoryOffset, disjoint_mem_req.alignment) != 0) {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError(
                            "VUID-VkBindImageMemoryInfo-pNext-01620", objlist, loc.dot(Field::memoryOffset),
                            "is %" PRIu64 " but must be an integer multiple of the VkMemoryRequirements::alignment value %" PRIu64
                            ", returned from a call to vkGetImageMemoryRequirements2 with disjoint image for aspect plane %s.",
                            bind_info.memoryOffset, disjoint_mem_req.alignment, string_VkImageAspectFlagBits(aspect));
                    }

                    if (mem_info) {
                        const VkMemoryAllocateInfo &allocate_info = mem_info->allocate_info;

                        // Validate memory requirements size
                        if (disjoint_mem_req.size > allocate_info.allocationSize - bind_info.memoryOffset) {
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError(
                                "VUID-VkBindImageMemoryInfo-pNext-01621", objlist, loc,
                                "allocationSize (%" PRIu64 ") minus memoryOffset (%" PRIu64 ") is %" PRIu64
                                " but must be at least as large as VkMemoryRequirements::size value %" PRIu64
                                ", returned from a call to vkGetImageMemoryRequirements with disjoint image for aspect plane %s.",
                                allocate_info.allocationSize, bind_info.memoryOffset,
                                allocate_info.allocationSize - bind_info.memoryOffset, disjoint_mem_req.size,
                                string_VkImageAspectFlagBits(aspect));
                        }

                        // Validate memory type used
                        {
                            skip |= ValidateMemoryTypes(*mem_info, disjoint_mem_req.memoryTypeBits, loc.dot(Field::image),
                                                        "VUID-VkBindImageMemoryInfo-pNext-01619");
                        }
                    }
                }

                auto it = resources_bound.find(image_state->VkHandle());
                if (it == resources_bound.end()) {
                    std::array<uint32_t, 3> bound_index = {vvl::kU32Max, vvl::kU32Max, vvl::kU32Max};
                    bound_index[plane] = i;
                    resources_bound.emplace(image_state->VkHandle(), bound_index);
                } else {
                    if (it->second[plane] == vvl::kU32Max) {
                        it->second[plane] = i;
                    } else {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError("VUID-vkBindImageMemory2-pBindInfos-04006", objlist, loc.dot(Field::image),
                                         "is a disjoint image for plane %d but is being bound twice at "
                                         "pBindInfos[%d]",
                                         plane, it->second[plane]);
                    }
                }
            }

            if (mem_info) {
                // Validate bound memory range information
                // if memory is exported to an AHB then the mem_info->allocationSize must be zero and this check is not needed
                if ((mem_info->IsExport() == false) ||
                    ((mem_info->export_handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) == 0)) {
                    skip |= ValidateInsertImageMemoryRange(bind_info.image, *mem_info, bind_info.memoryOffset, loc);
                }

                // Validate dedicated allocation
                const VkImage dedicated_image = mem_info->GetDedicatedImage();
                if (dedicated_image != VK_NULL_HANDLE) {
                    if (enabled_features.dedicatedAllocationImageAliasing) {
                        auto current_image_state = Get<vvl::Image>(bind_info.image);
                        if ((bind_info.memoryOffset != 0) || !current_image_state ||
                            !current_image_state->IsCreateInfoDedicatedAllocationImageAliasingCompatible(
                                mem_info->dedicated->create_info.image)) {
                            const char *vuid = bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-memory-02629"
                                                                : "VUID-vkBindImageMemory-memory-02629";
                            const LogObjectList objlist(bind_info.image, bind_info.memory, dedicated_image);
                            skip |= LogError(
                                vuid, objlist, loc.dot(Field::memory),
                                "(%s) is a dedicated memory allocation, but VkMemoryDedicatedAllocateInfo:: %s must compatible "
                                "with %s and memoryOffset %" PRIu64 " must be zero.",
                                FormatHandle(bind_info.memory).c_str(), FormatHandle(dedicated_image).c_str(),
                                FormatHandle(bind_info.image).c_str(), bind_info.memoryOffset);
                        }
                    } else {
                        if ((bind_info.memoryOffset != 0) || (dedicated_image != bind_info.image)) {
                            const char *vuid = bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-memory-02628"
                                                                : "VUID-vkBindImageMemory-memory-02628";
                            const LogObjectList objlist(bind_info.image, bind_info.memory, dedicated_image);
                            skip |= LogError(
                                vuid, objlist, loc.dot(Field::memory),
                                "(%s) is a dedicated memory allocation, but VkMemoryDedicatedAllocateInfo:: %s must be equal "
                                "to %s and memoryOffset %" PRIu64 " must be zero.",
                                FormatHandle(bind_info.memory).c_str(), FormatHandle(dedicated_image).c_str(),
                                FormatHandle(bind_info.image).c_str(), bind_info.memoryOffset);
                        }
                    }
                } else if (IsExtEnabled(extensions.vk_khr_dedicated_allocation)) {
                    VkImageMemoryRequirementsInfo2 image_memory_requirements_info_2 = vku::InitStructHelper();
                    image_memory_requirements_info_2.image = bind_info.image;
                    VkMemoryDedicatedRequirements memory_dedicated_requirements = vku::InitStructHelper();
                    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper(&memory_dedicated_requirements);
                    DispatchGetImageMemoryRequirements2Helper(api_version, device, &image_memory_requirements_info_2,
                                                              &memory_requirements);

                    if (memory_dedicated_requirements.requiresDedicatedAllocation) {
                        const char *vuid =
                            bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-image-01445" : "VUID-vkBindImageMemory-image-01445";
                        if (dedicated_image == VK_NULL_HANDLE) {
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                                             "was created without a VkMemoryDedicatedAllocateInfo in the pNext chain, but "
                                             "vkGetImageMemoryRequirements2() reports "
                                             "VkImageMemoryRequirementsInfo2::requiresDedicatedAllocation = VK_TRUE.");
                        } else if (dedicated_image != bind_info.image) {
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError(vuid, objlist,
                                             loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::pNext).dot(Field::image),
                                             "is %s, but VkBindImageMemoryInfo::image is %s.",
                                             FormatHandle(dedicated_image).c_str(), FormatHandle(bind_info.image).c_str());
                        }
                    }
                }

                auto chained_flags_struct = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(mem_info->allocate_info.pNext);
                const VkMemoryAllocateFlags memory_allocate_flags = chained_flags_struct ? chained_flags_struct->flags : 0;
                if ((image_state->create_info.flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
                    !(memory_allocate_flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)) {
                    const char *vuid = bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-descriptorBufferCaptureReplay-08113"
                                                        : "VUID-vkBindImageMemory-descriptorBufferCaptureReplay-08113";
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(vuid, objlist, loc.dot(Field::image),
                                     "was created with the VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT bit set,"
                                     "but the bound memory was allocated with %s and needs "
                                     "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT.",
                                     string_VkMemoryAllocateFlags(memory_allocate_flags).c_str());
                }
                if ((image_state->create_info.flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
                    !(memory_allocate_flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)) {
                    const char *vuid =
                        bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-image-09202" : "VUID-vkBindImageMemory-image-09202";
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(vuid, objlist, loc.dot(Field::image),
                                     "was created with the VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT bit set,"
                                     "but the bound memory was allocated with %s and needs "
                                     "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                                     string_VkMemoryAllocateFlags(memory_allocate_flags).c_str());
                }

                // Validate export memory handles
                if (mem_info->IsExport()) {
                    VkPhysicalDeviceImageDrmFormatModifierInfoEXT drm_format_modifier = vku::InitStructHelper();
                    drm_format_modifier.sharingMode = image_state->create_info.sharingMode;
                    drm_format_modifier.queueFamilyIndexCount = image_state->create_info.queueFamilyIndexCount;
                    drm_format_modifier.pQueueFamilyIndices = image_state->create_info.pQueueFamilyIndices;
                    VkPhysicalDeviceExternalImageFormatInfo external_info = vku::InitStructHelper();
                    VkPhysicalDeviceImageFormatInfo2 image_info = vku::InitStructHelper(&external_info);
                    image_info.format = image_state->create_info.format;
                    image_info.type = image_state->create_info.imageType;
                    image_info.tiling = image_state->create_info.tiling;
                    image_info.usage = image_state->create_info.usage;
                    image_info.flags = image_state->create_info.flags;
                    VkExternalImageFormatProperties external_properties = vku::InitStructHelper();
                    VkImageFormatProperties2 image_properties = vku::InitStructHelper(&external_properties);
                    bool export_supported = true;

                    auto validate_export_handle_types = [&](VkExternalMemoryHandleTypeFlagBits flag) {
                        external_info.handleType = flag;
                        external_info.pNext = NULL;
                        if (image_state->create_info.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                            VkImageDrmFormatModifierPropertiesEXT drm_modifier_properties = vku::InitStructHelper();
                            auto result =
                                DispatchGetImageDrmFormatModifierPropertiesEXT(device, bind_info.image, &drm_modifier_properties);
                            if (result == VK_SUCCESS) {
                                external_info.pNext = &drm_format_modifier;
                                drm_format_modifier.drmFormatModifier = drm_modifier_properties.drmFormatModifier;
                            }
                        }
                        auto result = DispatchGetPhysicalDeviceImageFormatProperties2Helper(api_version, physical_device,
                                                                                            &image_info, &image_properties);
                        if (result != VK_SUCCESS) {
                            export_supported = false;
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError(
                                "VUID-VkExportMemoryAllocateInfo-handleTypes-00656", objlist, loc,
                                "The handle type (%s) specified by the memory's VkExportMemoryAllocateInfo, and format (%s), "
                                "type (%s), tiling (%s), usage (%s), flags (%s) specified by the image's VkImageCreateInfo is not "
                                "supported combination of parameters. vkGetPhysicalDeviceImageFormatProperties2 returned back %s.",
                                string_VkExternalMemoryHandleTypeFlagBits(flag), string_VkFormat(image_info.format),
                                string_VkImageType(image_info.type), string_VkImageTiling(image_info.tiling),
                                string_VkImageUsageFlags(image_info.usage).c_str(),
                                string_VkImageCreateFlags(image_info.flags).c_str(), string_VkResult(result));
                            return;  // this exits lambda, not parent function
                        }
                        const auto external_features = external_properties.externalMemoryProperties.externalMemoryFeatures;
                        if ((external_features & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) == 0) {
                            export_supported = false;
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656", objlist, loc.dot(Field::memory),
                                             "(%s) has VkExportMemoryAllocateInfo::handleTypes with the %s "
                                             "flag set, which does not support VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT with the "
                                             "image create format (%s), type (%s), tiling (%s), usage (%s), flags (%s).",
                                             FormatHandle(bind_info.memory).c_str(),
                                             string_VkExternalMemoryHandleTypeFlagBits(flag), string_VkFormat(image_info.format),
                                             string_VkImageType(image_info.type), string_VkImageTiling(image_info.tiling),
                                             string_VkImageUsageFlags(image_info.usage).c_str(),
                                             string_VkImageCreateFlags(image_info.flags).c_str());
                        }
                        if ((external_features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0) {
                            if (!mem_info->IsDedicatedImage()) {
                                const LogObjectList objlist(bind_info.image, bind_info.memory);
                                skip |= LogError(
                                    "VUID-VkMemoryAllocateInfo-pNext-00639", objlist, loc.dot(Field::memory),
                                    "(%s) has VkExportMemoryAllocateInfo::handleTypes with the %s "
                                    "flag set, which requires dedicated allocation for the image created with format "
                                    "(%s), type (%s), tiling (%s), usage (%s), flags (%s), but the memory is allocated "
                                    "without dedicated allocation support.",
                                    FormatHandle(bind_info.memory).c_str(), string_VkExternalMemoryHandleTypeFlagBits(flag),
                                    string_VkFormat(image_info.format), string_VkImageType(image_info.type),
                                    string_VkImageTiling(image_info.tiling), string_VkImageUsageFlags(image_info.usage).c_str(),
                                    string_VkImageCreateFlags(image_info.flags).c_str());
                            }
                        }
                    };
                    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(mem_info->export_handle_types, validate_export_handle_types);

                    // The types of external memory handles must be compatible
                    const auto compatible_types = external_properties.externalMemoryProperties.compatibleHandleTypes;
                    if (export_supported && (mem_info->export_handle_types & compatible_types) != mem_info->export_handle_types) {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |=
                            LogError("VUID-VkExportMemoryAllocateInfo-handleTypes-00656", objlist, loc.dot(Field::memory),
                                     "(%s) has VkExportMemoryAllocateInfo::handleTypes (%s) that are not "
                                     "reported as compatible by vkGetPhysicalDeviceImageFormatProperties2 with the image create "
                                     "format (%s), type (%s), tiling (%s), usage (%s), flags (%s).",
                                     FormatHandle(bind_info.memory).c_str(),
                                     string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_types).c_str(),
                                     string_VkFormat(image_info.format), string_VkImageType(image_info.type),
                                     string_VkImageTiling(image_info.tiling), string_VkImageUsageFlags(image_info.usage).c_str(),
                                     string_VkImageCreateFlags(image_info.flags).c_str());
                    }

                    // Check if the memory meets the image's external memory requirements
                    if ((mem_info->export_handle_types & image_state->external_memory_handle_types) == 0) {
                        const char *vuid =
                            bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-memory-02728" : "VUID-vkBindImageMemory-memory-02728";
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                                         "(%s) has an external handleType of %s which does not include at least "
                                         "one handle from VkImage (%s) handleType %s.",
                                         FormatHandle(bind_info.memory).c_str(),
                                         string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_types).c_str(),
                                         FormatHandle(bind_info.image).c_str(),
                                         string_VkExternalMemoryHandleTypeFlags(image_state->external_memory_handle_types).c_str());
                    }
                }

                // Validate import memory handles
                if (mem_info->IsImportAHB() == true) {
                    skip |= ValidateImageImportedHandleANDROID(image_state->external_memory_handle_types, bind_info.memory,
                                                               bind_info.image, loc);
                } else if (mem_info->IsImport() == true) {
                    if ((mem_info->import_handle_type.value() & image_state->external_memory_handle_types) == 0) {
                        const char *vuid =
                            bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-memory-02989" : "VUID-vkBindImageMemory-memory-02989";
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                                         "(%s) was created with an import operation with handleType of %s "
                                         "which is not set in the VkImage (%s) VkExternalMemoryImageCreateInfo::handleType (%s)",
                                         FormatHandle(bind_info.memory).c_str(),
                                         string_VkExternalMemoryHandleTypeFlagBits(mem_info->import_handle_type.value()),
                                         FormatHandle(bind_info.image).c_str(),
                                         string_VkExternalMemoryHandleTypeFlags(image_state->external_memory_handle_types).c_str());
                    }
                    // Check if image can be bound to memory imported from specific handle type
                    if (!HasExternalMemoryImportSupport(*image_state, mem_info->import_handle_type.value())) {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError("VUID-VkImportMemoryWin32HandleInfoKHR-handleType-00658", objlist, loc.dot(Field::memory),
                                         "(%s) was imported from handleType %s but VkExternalImageFormatProperties does not report "
                                         "it as importable.",
                                         FormatHandle(bind_info.memory).c_str(),
                                         string_VkExternalMemoryHandleTypeFlagBits(mem_info->import_handle_type.value()));
                    }
                }

                // Validate mix of protected buffer and memory
                if ((image_state->unprotected == false) && (mem_info->unprotected == true)) {
                    const char *vuid =
                        bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-None-01901" : "VUID-vkBindImageMemory-None-01901";
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                                     "(%s) was not created with protected memory but the VkImage (%s) was "
                                     "set to use protected memory.",
                                     FormatHandle(bind_info.memory).c_str(), FormatHandle(bind_info.image).c_str());
                } else if ((image_state->unprotected == true) && (mem_info->unprotected == false)) {
                    const char *vuid =
                        bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-None-01902" : "VUID-vkBindImageMemory-None-01902";
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                                     "(%s) was created with protected memory but the VkImage (%s) was not "
                                     "set to use protected memory.",
                                     FormatHandle(bind_info.memory).c_str(), FormatHandle(bind_info.image).c_str());
                }
            }

            const auto swapchain_info = vku::FindStructInPNextChain<VkBindImageMemorySwapchainInfoKHR>(bind_info.pNext);
            if (swapchain_info) {
                if (bind_info.memory != VK_NULL_HANDLE) {
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError("VUID-VkBindImageMemoryInfo-pNext-01631", objlist, loc.dot(Field::memory),
                                     "(%s) is not VK_NULL_HANDLE.", FormatHandle(bind_info.memory).c_str());
                }
                if (image_state->create_from_swapchain != swapchain_info->swapchain) {
                    const LogObjectList objlist(bind_info.image, image_state->create_from_swapchain, swapchain_info->swapchain);
                    // VU being worked on https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/5078
                    skip |= LogError(
                        "UNASSIGNED-CoreValidation-BindImageMemory-Swapchain", objlist, loc.dot(Field::image),
                        "(%s) is created by %s, but the image is bound by %s. The image should be created and bound by the same "
                        "swapchain",
                        FormatHandle(bind_info.image).c_str(), FormatHandle(image_state->create_from_swapchain).c_str(),
                        FormatHandle(swapchain_info->swapchain).c_str());
                }
                if (auto swapchain_state = Get<vvl::Swapchain>(swapchain_info->swapchain)) {
                    if (swapchain_state->images.size() <= swapchain_info->imageIndex) {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError("VUID-VkBindImageMemorySwapchainInfoKHR-imageIndex-01644", objlist,
                                         loc.pNext(Struct::VkBindImageMemorySwapchainInfoKHR, Field::swapchain),
                                         "imageIndex (%" PRIu32 ") is out of bounds of %s images (size: %zu)",
                                         swapchain_info->imageIndex, FormatHandle(swapchain_info->swapchain).c_str(),
                                         swapchain_state->images.size());
                    }
                    if (IsExtEnabled(extensions.vk_ext_swapchain_maintenance1) &&
                        (swapchain_state->create_info.flags & VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT)) {
                        if (swapchain_state->images[swapchain_info->imageIndex].acquired == false) {
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError("VUID-VkBindImageMemorySwapchainInfoKHR-swapchain-07756", objlist,
                                             loc.pNext(Struct::VkBindImageMemorySwapchainInfoKHR, Field::swapchain),
                                             "was created with VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT but "
                                             "imageIndex (%" PRIu32 ") has not been acquired",
                                             swapchain_info->imageIndex);
                        }
                    }
                }
            } else {
                if (image_state->create_from_swapchain) {
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError("VUID-VkBindImageMemoryInfo-image-01630", objlist, loc,
                                     "pNext doesn't include VkBindImageMemorySwapchainInfoKHR.");
                }
                if (!mem_info) {
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError("VUID-VkBindImageMemoryInfo-pNext-01632", objlist, loc.dot(Field::memory), "(%s) is invalid.",
                                     FormatHandle(bind_info.memory).c_str());
                }
            }

            const auto bind_image_memory_device_group_info = vku::FindStructInPNextChain<VkBindImageMemoryDeviceGroupInfo>(bind_info.pNext);
            if (bind_image_memory_device_group_info && bind_image_memory_device_group_info->splitInstanceBindRegionCount != 0) {
                if (!(image_state->create_info.flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT)) {
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError("VUID-VkBindImageMemoryInfo-pNext-01627", objlist,
                                     loc.pNext(Struct::VkBindImageMemoryDeviceGroupInfo, Field::splitInstanceBindRegionCount),
                                     "(%" PRId32
                                     ") is not 0 and %s is not created with "
                                     "VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT.",
                                     bind_image_memory_device_group_info->splitInstanceBindRegionCount,
                                     FormatHandle(bind_info.image).c_str());
                }
                uint32_t phy_dev_square = 1;
                if (device_group_create_info.physicalDeviceCount > 0) {
                    phy_dev_square = device_group_create_info.physicalDeviceCount * device_group_create_info.physicalDeviceCount;
                }
                if (bind_image_memory_device_group_info->splitInstanceBindRegionCount != phy_dev_square) {
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(
                        "VUID-VkBindImageMemoryDeviceGroupInfo-splitInstanceBindRegionCount-01636", objlist,
                        loc.pNext(Struct::VkBindImageMemoryDeviceGroupInfo, Field::splitInstanceBindRegionCount),
                        "(%" PRId32
                        ") is not 0 and different from the number of physical devices in the logical device squared (%" PRIu32 ").",
                        bind_image_memory_device_group_info->splitInstanceBindRegionCount, phy_dev_square);
                }
            }

            if (plane_info) {
                // Checks for disjoint bit in image
                if (image_state->disjoint == false) {
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(
                        "VUID-VkBindImageMemoryInfo-pNext-01618", objlist, loc.dot(Field::image),
                        "(%s) is not created with VK_IMAGE_CREATE_DISJOINT_BIT, but pNext contains VkBindImagePlaneMemoryInfo.",
                        FormatHandle(bind_info.image).c_str());
                }

                // Make sure planeAspect is only a single, valid plane
                const VkFormat image_format = image_state->create_info.format;
                const VkImageAspectFlags aspect = plane_info->planeAspect;
                const VkImageTiling image_tiling = image_state->create_info.tiling;

                if ((image_tiling == VK_IMAGE_TILING_LINEAR) || (image_tiling == VK_IMAGE_TILING_OPTIMAL)) {
                    if (vkuFormatIsMultiplane(image_format) && !IsOnlyOneValidPlaneAspect(image_format, aspect)) {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError("VUID-VkBindImagePlaneMemoryInfo-planeAspect-02283", objlist,
                                         loc.pNext(Struct::VkBindImagePlaneMemoryInfo, Field::planeAspect),
                                         "is %s but is invalid for %s.", string_VkImageAspectFlags(aspect).c_str(),
                                         string_VkFormat(image_format));
                    }
                } else if (image_tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
                    // TODO - Need to also check if lower then drmFormatModifierPlaneCount
                    if (GetBitSetCount(aspect) > 1 ||
                        !IsValueIn(VkImageAspectFlagBits(aspect),
                                   {VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
                                    VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT, VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT})) {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError("VUID-VkBindImagePlaneMemoryInfo-planeAspect-02284", objlist,
                                         loc.pNext(Struct::VkBindImagePlaneMemoryInfo, Field::planeAspect),
                                         "is %s but is invalid for %s.", string_VkImageAspectFlags(aspect).c_str(),
                                         string_VkFormat(image_format));
                    }
                }
            }
        }

        const auto bind_image_memory_device_group = vku::FindStructInPNextChain<VkBindImageMemoryDeviceGroupInfo>(bind_info.pNext);
        if (bind_image_memory_device_group) {
            if (bind_image_memory_device_group->deviceIndexCount > 0 &&
                bind_image_memory_device_group->splitInstanceBindRegionCount > 0) {
                const LogObjectList objlist(bind_info.image, bind_info.memory);
                skip |= LogError("VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01633", objlist, loc,
                                 "VkBindImageMemoryDeviceGroupInfo has both deviceIndexCount (%" PRIu32
                                 ") and splitInstanceBindRegionCount (%" PRIu32 ") greater than 0.",
                                 bind_image_memory_device_group->deviceIndexCount,
                                 bind_image_memory_device_group->splitInstanceBindRegionCount);
            }
            if (bind_image_memory_device_group->deviceIndexCount != 0 &&
                bind_image_memory_device_group->deviceIndexCount != device_group_create_info.physicalDeviceCount &&
                device_group_create_info.physicalDeviceCount > 0) {
                const LogObjectList objlist(bind_info.image, bind_info.memory);
                skip |= LogError("VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01634", objlist,
                                 loc.pNext(Struct::VkBindImageMemoryDeviceGroupInfo, Field::deviceIndexCount),
                                 "is %" PRIu32 ", but the number of physical devices in the logical device is %" PRIu32 ".",
                                 bind_image_memory_device_group->deviceIndexCount, device_group_create_info.physicalDeviceCount);
            }
        }
    }

    // Check to make sure all disjoint planes were bound
    for (auto &resource : resources_bound) {
        auto image_state = Get<vvl::Image>(resource.first);
        if (image_state && image_state->disjoint == true && !is_drm) {
            uint32_t total_planes = vkuFormatPlaneCount(image_state->create_info.format);
            for (uint32_t i = 0; i < total_planes; i++) {
                if (resource.second[i] == vvl::kU32Max) {
                    skip |= LogError("VUID-vkBindImageMemory2-pBindInfos-02858", resource.first, error_obj.location,
                                     "Plane %u of the disjoint image was not bound. All %d planes need to bound individually "
                                     "in separate pBindInfos in a single call.",
                                     i, total_planes);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                                const ErrorObject &error_obj) const {
    bool skip = false;
    if (auto image_state = Get<vvl::Image>(image)) {
        // Checks for no disjoint bit
        if (image_state->disjoint == true) {
            const LogObjectList objlist(image, memory);
            skip |= LogError("VUID-vkBindImageMemory-image-01608", objlist, error_obj.location.dot(Field::image),
                             "was created with the VK_IMAGE_CREATE_DISJOINT_BIT (need to use vkBindImageMemory2).");
        }
    }

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.image = image;
    bind_info.memory = memory;
    bind_info.memoryOffset = memoryOffset;
    skip |= ValidateBindImageMemory(1, &bind_info, error_obj);
    return skip;
}

void CoreChecks::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                               const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    BaseClass::PostCallRecordBindImageMemory(device, image, memory, memoryOffset, record_obj);
    if (auto image_state = Get<vvl::Image>(image)) {
        image_state->SetInitialLayoutMap();
    }
}

bool CoreChecks::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                 const ErrorObject &error_obj) const {
    return ValidateBindImageMemory(bindInfoCount, pBindInfos, error_obj);
}

void CoreChecks::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                const RecordObject &record_obj) {
    // Don't check |record_obj.result| as some binds might still be valid
    BaseClass::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        if (auto image_state = Get<vvl::Image>(pBindInfos[i].image)) {
            // Need to protect if some VkBindMemoryStatus are not VK_SUCCESS
            if (!image_state->HasBeenBound()) continue;

            image_state->SetInitialLayoutMap();
        }
    }
}

bool CoreChecks::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo *pBindInfos, const ErrorObject &error_obj) const {
    return PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos, error_obj);
}

void CoreChecks::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                   const RecordObject &record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

bool CoreChecks::ValidateBufferSparseMemoryBindAlignments(const VkSparseMemoryBind &bind, const vvl::Buffer &buffer,
                                                          const Location &bind_loc, const Location &buffer_bind_info_loc) const {
    bool skip = false;

    if (SafeModulo(bind.resourceOffset, buffer.requirements.alignment) != 0) {
        const LogObjectList objlist(bind.memory, buffer.Handle());
        skip |=
            LogError("VUID-VkSparseMemoryBind-resourceOffset-09491", objlist, buffer_bind_info_loc.dot(Field::buffer),
                     "(%s) is being bound, but %s.resourceOffset (%" PRIu64
                     ") is not a multiple of required memory alignment (%" PRIu64 ").",
                     FormatHandle(buffer).c_str(), bind_loc.Fields().c_str(), bind.resourceOffset, buffer.requirements.alignment);
    }

    if (SafeModulo(bind.memoryOffset, buffer.requirements.alignment) != 0) {
        const LogObjectList objlist(bind.memory, buffer.Handle());
        skip |= LogError("VUID-VkSparseMemoryBind-resourceOffset-09491", objlist, buffer_bind_info_loc.dot(Field::buffer),
                         "(%s) is being bound, but %s.memoryOffset (%" PRIu64
                         ") is not a multiple of required memory alignment (%" PRIu64 ").",
                         FormatHandle(buffer).c_str(), bind_loc.Fields().c_str(), bind.memoryOffset, buffer.requirements.alignment);
    }

    if (SafeModulo(bind.size, buffer.requirements.alignment) != 0) {
        const LogObjectList objlist(bind.memory, buffer.Handle());
        skip |=
            LogError("VUID-VkSparseMemoryBind-resourceOffset-09491", objlist, buffer_bind_info_loc.dot(Field::buffer),
                     "(%s) is being bound, but %s.size (%" PRIu64 ") is not a multiple of required memory alignment (%" PRIu64 ").",
                     FormatHandle(buffer).c_str(), bind_loc.Fields().c_str(), bind.size, buffer.requirements.alignment);
    }

    return skip;
}

bool CoreChecks::ValidateImageSparseMemoryBindAlignments(const VkSparseMemoryBind &bind, const vvl::Image &image,
                                                         const Location &bind_loc, const Location &image_bind_info_loc) const {
    bool skip = false;

    if (SafeModulo(bind.resourceOffset, image.requirements[0].alignment) != 0) {
        const LogObjectList objlist(bind.memory, image.Handle());
        skip |=
            LogError("VUID-VkSparseMemoryBind-resourceOffset-09492", objlist, image_bind_info_loc.dot(Field::image),
                     "(%s) is being bound, but %s.resourceOffset (%" PRIu64
                     ") is not a multiple of required memory alignment (%" PRIu64 ").",
                     FormatHandle(image).c_str(), bind_loc.Fields().c_str(), bind.resourceOffset, image.requirements[0].alignment);
    }

    if (SafeModulo(bind.memoryOffset, image.requirements[0].alignment) != 0) {
        const LogObjectList objlist(bind.memory, image.Handle());
        skip |= LogError(
            "VUID-VkSparseMemoryBind-resourceOffset-09492", objlist, image_bind_info_loc.dot(Field::image),
            "(%s) is being bound, but %s.memoryOffset (%" PRIu64 ") is not a multiple of required memory alignment (%" PRIu64 ").",
            FormatHandle(image).c_str(), bind_loc.Fields().c_str(), bind.memoryOffset, image.requirements[0].alignment);
    }

    return skip;
}

bool CoreChecks::ValidateSparseMemoryBind(const VkSparseMemoryBind &bind, const VkMemoryRequirements &requirements,
                                          VkDeviceSize resource_size, VkExternalMemoryHandleTypeFlags external_handle_types,
                                          const VulkanTypedHandle &resource_handle, const Location &loc) const {
    bool skip = false;
    if (auto memory_state = Get<vvl::DeviceMemory>(bind.memory)) {
        if (!((uint32_t(1) << memory_state->allocate_info.memoryTypeIndex) & requirements.memoryTypeBits)) {
            const LogObjectList objlist(bind.memory, resource_handle);
            skip |= LogError("VUID-VkSparseMemoryBind-memory-01096", objlist, loc.dot(Field::memory),
                             "has a type index (%" PRIu32 ") that is not among the allowed types mask (0x%" PRIX32
                             ") for this resource.",
                             memory_state->allocate_info.memoryTypeIndex, requirements.memoryTypeBits);
        }

        if (SafeModulo(bind.memoryOffset, requirements.alignment) != 0) {
            const LogObjectList objlist(bind.memory, resource_handle);
            skip |= LogError("VUID-VkSparseMemoryBind-memory-01096", objlist, loc.dot(Field::memoryOffset),
                             "(%" PRIu64 ") is not a multiple of required memory alignment (%" PRIu64 ")", bind.memoryOffset,
                             requirements.alignment);
        }

        if (phys_dev_mem_props.memoryTypes[memory_state->allocate_info.memoryTypeIndex].propertyFlags &
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            const LogObjectList objlist(bind.memory, resource_handle);
            skip |= LogError("VUID-VkSparseMemoryBind-memory-01097", objlist, loc.dot(Field::memory),
                             "type has VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT bit set.");
        }

        if (bind.memoryOffset >= memory_state->allocate_info.allocationSize) {
            const LogObjectList objlist(bind.memory, resource_handle);
            skip |= LogError("VUID-VkSparseMemoryBind-memoryOffset-01101", objlist, loc.dot(Field::memoryOffset),
                             "(%" PRIu64 ") must be less than the size of memory (%" PRIu64 ")", bind.memoryOffset,
                             memory_state->allocate_info.allocationSize);
        }

        if ((memory_state->allocate_info.allocationSize - bind.memoryOffset) < bind.size) {
            const LogObjectList objlist(bind.memory, resource_handle);
            skip |= LogError("VUID-VkSparseMemoryBind-size-01102", objlist, loc.dot(Field::size),
                             "(%" PRIu64 ") must be less than or equal to the size of memory (%" PRIu64
                             ") minus memoryOffset (%" PRIu64 ").",
                             bind.size, memory_state->allocate_info.allocationSize, bind.memoryOffset);
        }

        if (memory_state->IsExport()) {
            if (!(memory_state->export_handle_types & external_handle_types)) {
                const LogObjectList objlist(bind.memory, resource_handle);
                skip |= LogError("VUID-VkSparseMemoryBind-memory-02730", objlist,
                                 loc.dot(Field::memory).pNext(Struct::VkExportMemoryAllocateInfo).dot(Field::handleTypes),
                                 "is %s, but the external handle types specified in resource are %s.",
                                 string_VkExternalMemoryHandleTypeFlags(memory_state->export_handle_types).c_str(),
                                 string_VkExternalMemoryHandleTypeFlags(external_handle_types).c_str());
            }
        }

        if (memory_state->IsImport()) {
            if (!(*memory_state->import_handle_type & external_handle_types)) {
                const LogObjectList objlist(bind.memory, resource_handle);
                skip |= LogError("VUID-VkSparseMemoryBind-memory-02731", objlist, loc.dot(Field::memory),
                                 "was created with memory import operation, with handle type %s, but the external handle types "
                                 "specified in resource are %s.",
                                 string_VkExternalMemoryHandleTypeFlagBits(*memory_state->import_handle_type),
                                 string_VkExternalMemoryHandleTypeFlags(external_handle_types).c_str());
            }
        }
    }

    if (bind.size <= 0) {
        const LogObjectList objlist(bind.memory, resource_handle);
        skip |= LogError("VUID-VkSparseMemoryBind-size-01098", objlist, loc.dot(Field::size),
                         "(%" PRIu64 ") must be greater than 0.", bind.size);
    }

    if (resource_size <= bind.resourceOffset) {
        const LogObjectList objlist(bind.memory, resource_handle);
        skip |=
            LogError("VUID-VkSparseMemoryBind-resourceOffset-01099", objlist, loc.dot(Field::resourceOffset),
                     "(%" PRIu64 ") must be less than the size of the resource (%" PRIu64 ").", bind.resourceOffset, resource_size);
    }

    if ((resource_size - bind.resourceOffset) < bind.size) {
        const LogObjectList objlist(bind.memory, resource_handle);
        skip |= LogError("VUID-VkSparseMemoryBind-size-01100", objlist, loc.dot(Field::size),
                         "(%" PRIu64 ") must be less than or equal to the size of the resource (%" PRIu64
                         ") minus resourceOffset (%" PRIu64 ").",
                         bind.size, resource_size, bind.resourceOffset);
    }

    return skip;
}

bool CoreChecks::ValidateImageSubresourceSparseImageMemoryBind(vvl::Image const &image_state, VkImageSubresource const &subresource,
                                                               const Location &bind_loc, const Location &subresource_loc) const {
    bool skip = false;
    skip |= ValidateImageAspectMask(image_state.VkHandle(), image_state.create_info.format, subresource.aspectMask,
                                    image_state.disjoint, bind_loc, "VUID-VkSparseImageMemoryBindInfo-subresource-01106");

    if (subresource.mipLevel >= image_state.create_info.mipLevels) {
        skip |=
            LogError("VUID-VkSparseImageMemoryBindInfo-subresource-01722", image_state.Handle(),
                     subresource_loc.dot(Field::mipLevel), "(%" PRIu32 ") is not less than mipLevels (%" PRIu32 ") of %s.image.",
                     subresource.mipLevel, image_state.create_info.mipLevels, bind_loc.Fields().c_str());
    }

    if (subresource.arrayLayer >= image_state.create_info.arrayLayers) {
        skip |= LogError("VUID-VkSparseImageMemoryBindInfo-subresource-01723", image_state.Handle(),
                         subresource_loc.dot(Field::arrayLayer),
                         "(%" PRIu32 ") is not less than arrayLayers (%" PRIu32 ") of %s.image.", subresource.arrayLayer,
                         image_state.create_info.arrayLayers, bind_loc.Fields().c_str());
    }

    return skip;
}

// This will only be called after we are sure the image was created with VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT
bool CoreChecks::ValidateSparseImageMemoryBind(vvl::Image const *image_state, VkSparseImageMemoryBind const &bind,
                                               const Location &bind_loc, const Location &memory_loc) const {
    bool skip = false;

    if (auto const memory_state = Get<vvl::DeviceMemory>(bind.memory)) {
        // TODO: The closest one should be VUID-VkSparseImageMemoryBind-memory-01105 instead of the mentioned
        // one. We also need to check memory_bind.memory
        if (bind.memoryOffset >= memory_state->allocate_info.allocationSize) {
            skip |= LogError("VUID-VkSparseMemoryBind-memoryOffset-01101", bind.memory, bind_loc.dot(Field::memoryOffset),
                             "(%" PRIu64 ") is not less than the size (%" PRIu64 ") of memory.", bind.memoryOffset,
                             memory_state->allocate_info.allocationSize);
        }

        // TODO: We cannot validate the requirement size since there is no way
        // to calculate the size of an optimal tiled arbitrary image region (as of now).
        const VkMemoryRequirements &requirement = image_state->requirements[0];

        if (SafeModulo(bind.memoryOffset, requirement.alignment) != 0) {
            skip |= LogError("VUID-VkSparseImageMemoryBind-memory-01105", bind.memory, memory_loc.dot(Field::memoryOffset),
                             "(%" PRIu64 ") is not a multiple of the required alignment (%" PRIu64 ").", bind.memoryOffset,
                             requirement.alignment);
        }

        skip |= ValidateMemoryTypes(*memory_state.get(), requirement.memoryTypeBits, memory_loc.dot(Field::memory),
                                    "VUID-VkSparseImageMemoryBind-memory-01105");

        if (memory_state->IsExport()) {
            if (!(memory_state->export_handle_types & image_state->external_memory_handle_types)) {
                const LogObjectList objlist(bind.memory, image_state->Handle());
                skip |= LogError("VUID-VkSparseImageMemoryBind-memory-02732", objlist,
                                 memory_loc.dot(Field::memory).pNext(Struct::VkExportMemoryAllocateInfo).dot(Field::handleTypes),
                                 "is %s, but the external handle types specified in resource are %s.",
                                 string_VkExternalMemoryHandleTypeFlags(memory_state->export_handle_types).c_str(),
                                 string_VkExternalMemoryHandleTypeFlags(image_state->external_memory_handle_types).c_str());
            }
        }

        if (memory_state->IsImport()) {
            if (!(*memory_state->import_handle_type & image_state->external_memory_handle_types)) {
                const LogObjectList objlist(bind.memory, image_state->Handle());
                skip |= LogError("VUID-VkSparseImageMemoryBind-memory-02733", objlist, memory_loc.dot(Field::memory),
                                 "was created with memory import operation, with handle type %s, but the external handle types "
                                 "specified in resource are %s.",
                                 string_VkExternalMemoryHandleTypeFlagBits(*memory_state->import_handle_type),
                                 string_VkExternalMemoryHandleTypeFlags(image_state->external_memory_handle_types).c_str());
            }
        }
    }

    if (!image_state) {
        return skip;
    }

    skip |=
        ValidateImageSubresourceSparseImageMemoryBind(*image_state, bind.subresource, bind_loc, memory_loc.dot(Field::subresource));

    for (auto const &requirements : image_state->sparse_requirements) {
        VkExtent3D const &granularity = requirements.formatProperties.imageGranularity;
        if (SafeModulo(bind.offset.x, granularity.width) != 0) {
            skip |= LogError("VUID-VkSparseImageMemoryBind-offset-01107", image_state->Handle(),
                             bind_loc.dot(Field::offset).dot(Field::x),
                             "(%" PRId32
                             ") must be a multiple of the sparse image block width "
                             "(VkSparseImageFormatProperties::imageGranularity.width (%" PRIu32 ")) of the image.",
                             bind.offset.x, granularity.width);
        }

        if (SafeModulo(bind.offset.y, granularity.height) != 0) {
            skip |= LogError("VUID-VkSparseImageMemoryBind-offset-01109", image_state->Handle(),
                             bind_loc.dot(Field::offset).dot(Field::y),
                             "(%" PRId32
                             ") must be a multiple of the sparse image block height "
                             "(VkSparseImageFormatProperties::imageGranularity.height (%" PRIu32 ")) of the image.",
                             bind.offset.y, granularity.height);
        }

        if (SafeModulo(bind.offset.z, granularity.depth) != 0) {
            skip |= LogError("VUID-VkSparseImageMemoryBind-offset-01111", image_state->Handle(),
                             bind_loc.dot(Field::offset).dot(Field::z),
                             "(%" PRId32
                             ") must be a multiple of the sparse image block depth "
                             "(VkSparseImageFormatProperties::imageGranularity.depth (%" PRIu32 ")) of the image.",
                             bind.offset.z, granularity.depth);
        }

        VkExtent3D const subresource_extent = image_state->GetEffectiveSubresourceExtent(bind.subresource);
        if ((SafeModulo(bind.extent.width, granularity.width) != 0) &&
            ((bind.extent.width + bind.offset.x) != subresource_extent.width)) {
            skip |= LogError("VUID-VkSparseImageMemoryBind-extent-01108", image_state->Handle(),
                             bind_loc.dot(Field::extent).dot(Field::width),
                             "(%" PRIu32
                             ") must either be a multiple of the sparse image block width "
                             "(VkSparseImageFormatProperties::imageGranularity.width (%" PRIu32
                             ")) of the image, or else (extent.width + offset.x) (%" PRIu32
                             ") must equal the width of the image subresource (%" PRIu32 ").",
                             bind.extent.width, granularity.width, bind.extent.width + bind.offset.x, subresource_extent.width);
        }

        if ((SafeModulo(bind.extent.height, granularity.height) != 0) &&
            ((bind.extent.height + bind.offset.y) != subresource_extent.height)) {
            skip |= LogError("VUID-VkSparseImageMemoryBind-extent-01110", image_state->Handle(),
                             bind_loc.dot(Field::extent).dot(Field::height),
                             "(%" PRIu32
                             ") must either be a multiple of the sparse image block height "
                             "(VkSparseImageFormatProperties::imageGranularity.height (%" PRIu32
                             ")) of the image, or else (extent.height + offset.y) (%" PRIu32
                             ") must equal the height of the image subresource (%" PRIu32 ").",
                             bind.extent.height, granularity.height, bind.extent.height + bind.offset.y, subresource_extent.height);
        }

        if ((SafeModulo(bind.extent.depth, granularity.depth) != 0) &&
            ((bind.extent.depth + bind.offset.z) != subresource_extent.depth)) {
            skip |= LogError("VUID-VkSparseImageMemoryBind-extent-01112", image_state->Handle(),
                             bind_loc.dot(Field::extent).dot(Field::depth),
                             "(%" PRIu32
                             ") must either be a multiple of the sparse image block depth "
                             "(VkSparseImageFormatProperties::imageGranularity.depth (%" PRIu32
                             ")) of the image, or else (extent.depth + offset.z) (%" PRIu32
                             ") must equal the depth of the image subresource (%" PRIu32 ").",
                             bind.extent.depth, granularity.depth, bind.extent.depth + bind.offset.z, subresource_extent.depth);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    if (!enabled_features.bufferDeviceAddress && !enabled_features.bufferDeviceAddressEXT) {
        skip |= LogError("VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324", pInfo->buffer, error_obj.location,
                         "The bufferDeviceAddress feature must be enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice &&
        !enabled_features.bufferDeviceAddressMultiDeviceEXT) {
        skip |= LogError("VUID-vkGetBufferDeviceAddress-device-03325", pInfo->buffer, error_obj.location,
                         "If device was created with multiple physical devices, then the "
                         "bufferDeviceAddressMultiDevice feature must be enabled.");
    }

    if (auto buffer_state = Get<vvl::Buffer>(pInfo->buffer)) {
        const Location info_loc = error_obj.location.dot(Field::pInfo);
        if (!(buffer_state->create_info.flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
            skip |= ValidateMemoryIsBoundToBuffer(device, *buffer_state, info_loc.dot(Field::buffer),
                                                  "VUID-VkBufferDeviceAddressInfo-buffer-02600");
        }

        skip |= ValidateBufferUsageFlags(LogObjectList(device), *buffer_state, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, true,
                                         "VUID-VkBufferDeviceAddressInfo-buffer-02601", info_loc.dot(Field::buffer));
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateGetBufferDeviceAddress(device, pInfo, error_obj);
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateGetBufferDeviceAddress(device, pInfo, error_obj);
}

bool CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                              const ErrorObject &error_obj) const {
    bool skip = false;
    const LogObjectList objlist(device, pInfo->buffer);

    if (!enabled_features.bufferDeviceAddress) {
        skip |= LogError("VUID-vkGetBufferOpaqueCaptureAddress-None-03326", objlist, error_obj.location,
                         "The bufferDeviceAddress feature must be enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice) {
        skip |= LogError("VUID-vkGetBufferOpaqueCaptureAddress-device-03327", objlist, error_obj.location,
                         "If device was created with multiple physical devices, then the "
                         "bufferDeviceAddressMultiDevice feature must be enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                                 const ErrorObject &error_obj) const {
    return PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo, error_obj);
}

bool CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                    const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo,
                                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const LogObjectList objlst(device, pInfo->memory);

    if (!enabled_features.bufferDeviceAddress) {
        skip |= LogError("VUID-vkGetDeviceMemoryOpaqueCaptureAddress-None-03334", objlst, error_obj.location,
                         "The bufferDeviceAddress feature was not enabled.");
    }

    if (physical_device_count > 1 && !enabled_features.bufferDeviceAddressMultiDevice) {
        skip |= LogError("VUID-vkGetDeviceMemoryOpaqueCaptureAddress-device-03335", objlst, error_obj.location,
                         "If device was created with multiple physical devices, then the "
                         "bufferDeviceAddressMultiDevice feature was not enabled.");
    }

    if (auto mem_info = Get<vvl::DeviceMemory>(pInfo->memory)) {
        auto chained_flags_struct = vku::FindStructInPNextChain<VkMemoryAllocateFlagsInfo>(mem_info->allocate_info.pNext);
        if (!chained_flags_struct || !(chained_flags_struct->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)) {
            skip |= LogError("VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336", objlst, error_obj.location,
                             "memory must have been allocated with VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                       const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo,
                                                                       const ErrorObject &error_obj) const {
    return PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo, error_obj);
}

bool CoreChecks::ValidateMemoryIsBoundToBuffer(LogObjectList objlist, const vvl::Buffer &buffer_state, const Location &buffer_loc,
                                               const char *vuid) const {
    bool skip = false;
    if (!buffer_state.sparse) {
        objlist.add(buffer_state.Handle());
        skip |= VerifyBoundMemoryIsValid(buffer_state.MemoryState(), objlist, buffer_state.Handle(), buffer_loc, vuid);
    }
    return skip;
}
