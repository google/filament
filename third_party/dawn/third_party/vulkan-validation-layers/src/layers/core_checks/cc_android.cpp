/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "utils/vk_layer_utils.h"
#include <vulkan/vk_enum_string_helper.h>
#include "core_validation.h"
#include "state_tracker/image_state.h"
#include "state_tracker/sampler_state.h"
#include "generated/dispatch_functions.h"
#include "error_message/error_strings.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
// Android-specific validation that uses types defined only on Android and only for NDK versions
// that support the VK_ANDROID_external_memory_android_hardware_buffer extension.

// Based on vkspec.html#memory-external-android-hardware-buffer-formats
// The AHARDWAREBUFFER_FORMAT_* are an enum in the NDK headers, but get passed in to Vulkan
// as uint32_t. Casting the enums here avoids scattering casts around in the code.
static VkFormat GetAhbToVkFormat(uint32_t ahb_format) {
    switch (ahb_format) {
        case (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
            return VK_FORMAT_R8G8B8_UNORM;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_D24_UNORM:
            return VK_FORMAT_X8_D24_UNORM_PACK32;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_D32_FLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case (uint32_t)AHARDWAREBUFFER_FORMAT_S8_UINT:
            return VK_FORMAT_S8_UINT;
        default:
            break;
    }
    return VK_FORMAT_UNDEFINED;
}

// Only designed to print a single usage
static inline const char *string_AHardwareBufferGpuUsage(uint64_t usage) {
    if (usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
        return "AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE";
    } else if (usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
        return "AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER";
    } else if (usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) {
        return "AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER";
    } else if (usage & AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP) {
        return "AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP";
    } else if (usage & AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE) {
        return "AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE";
    } else {
        return "Unknown AHARDWAREBUFFER_USAGE_GPU";
    }
}

//
// AHB-extension new APIs
//
bool CoreChecks::PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer *buffer,
                                                                          VkAndroidHardwareBufferPropertiesANDROID *pProperties,
                                                                          const ErrorObject &error_obj) const {
    bool skip = false;
    //  buffer must be a valid Android hardware buffer object with at least one of the AHARDWAREBUFFER_USAGE_GPU_* usage flags.
    AHardwareBuffer_Desc ahb_desc;
    AHardwareBuffer_describe(buffer, &ahb_desc);
    const uint32_t required_flags = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                                    AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
                                    AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    if (0 == (ahb_desc.usage & required_flags)) {
        skip |= LogError(
            "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884", device, error_obj.location.dot(Field::buffer),
            "AHardwareBuffer_Desc.usage (0x%" PRIx64 ") does not have any AHARDWAREBUFFER_USAGE_GPU_* flags set. (AHB = %p).",
            ahb_desc.usage, buffer);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetMemoryAndroidHardwareBufferANDROID(VkDevice device,
                                                                      const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo,
                                                                      struct AHardwareBuffer **pBuffer,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    auto mem_info = Get<vvl::DeviceMemory>(pInfo->memory);
    ASSERT_AND_RETURN_SKIP(mem_info);

    // VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID must have been included in
    // VkExportMemoryAllocateInfo::handleTypes when memory was created.
    if (!mem_info->IsExport() ||
        (0 == (mem_info->export_handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID))) {
        skip |= LogError("VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882", device,
                         error_obj.location.dot(Field::pInfo).dot(Field::memory),
                         "(%s) was not allocated for export, or the "
                         "export handleTypes (0x%" PRIx32
                         ") did not contain VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.",
                         FormatHandle(pInfo->memory).c_str(), mem_info->export_handle_types);
    }

    // If the pNext chain of the VkMemoryAllocateInfo used to allocate memory included a VkMemoryDedicatedAllocateInfo
    // with non-NULL image member, then that image must already be bound to memory.
    const VkImage dedicated_image = mem_info->GetDedicatedImage();
    if (dedicated_image != VK_NULL_HANDLE) {
        auto image_state = Get<vvl::Image>(dedicated_image);
        if (!image_state || (0 == (image_state->CountDeviceMemory(mem_info->VkHandle())))) {
            const LogObjectList objlist(device, pInfo->memory, dedicated_image);
            skip |= LogError("VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883", objlist,
                             error_obj.location.dot(Field::pInfo).dot(Field::memory),
                             "(%s) was allocated using a dedicated "
                             "%s, but that image is not bound to the VkDeviceMemory object.",
                             FormatHandle(pInfo->memory).c_str(), FormatHandle(dedicated_image).c_str());
        }
    }

    return skip;
}

//
// AHB-specific validation within non-AHB APIs
//
bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo &allocate_info, const Location &allocate_info_loc) const {
    bool skip = false;
    auto import_ahb_info = vku::FindStructInPNextChain<VkImportAndroidHardwareBufferInfoANDROID>(allocate_info.pNext);
    auto exp_mem_alloc_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(allocate_info.pNext);
    auto mem_ded_alloc_info = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(allocate_info.pNext);

    if ((import_ahb_info) && (NULL != import_ahb_info->buffer)) {
        const Location ahb_loc = allocate_info_loc.dot(Struct::VkImportAndroidHardwareBufferInfoANDROID, Field::buffer);

        // This is an import with handleType of VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID
        AHardwareBuffer_Desc ahb_desc = {};
        AHardwareBuffer_describe(import_ahb_info->buffer, &ahb_desc);

        //  Validate AHardwareBuffer_Desc::usage is a valid usage for imported AHB
        //
        //  BLOB & GPU_DATA_BUFFER combo specifically allowed
        if ((AHARDWAREBUFFER_FORMAT_BLOB != ahb_desc.format) || (0 == (ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
            // Otherwise, must be a combination from the AHardwareBuffer Format and Usage Equivalence tables
            // Usage must have at least one bit from the table. It may have additional bits not in the table
            uint64_t ahb_equiv_usage_bits = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER |
                                            AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE |
                                            AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
            if (0 == (ahb_desc.usage & ahb_equiv_usage_bits)) {
                skip |= LogError("VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881", device, ahb_loc,
                                 "AHardwareBuffer_Desc's usage (0x%" PRIx64 ") is not compatible with Vulkan. (AHB = %p).",
                                 ahb_desc.usage, import_ahb_info->buffer);
            }
        }

        // Collect external buffer info
        VkPhysicalDeviceExternalBufferInfo pdebi = vku::InitStructHelper();
        pdebi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
        if (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE & ahb_desc.usage) {
            pdebi.usage |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        }
        if (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER & ahb_desc.usage) {
            pdebi.usage |= (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }
        VkExternalBufferProperties ext_buf_props = vku::InitStructHelper();
        DispatchGetPhysicalDeviceExternalBufferPropertiesHelper(api_version, physical_device, &pdebi, &ext_buf_props);

        //  If buffer is not NULL, Android hardware buffers must be supported for import, as reported by
        //  VkExternalImageFormatProperties or VkExternalBufferProperties.
        if (0 == (ext_buf_props.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
            // Collect external format info
            VkPhysicalDeviceExternalImageFormatInfo pdeifi = vku::InitStructHelper();
            pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
            VkPhysicalDeviceImageFormatInfo2 pdifi2 = vku::InitStructHelper(&pdeifi);
            pdifi2.format = GetAhbToVkFormat(ahb_desc.format);
            pdifi2.type = VK_IMAGE_TYPE_2D;           // Seems likely
            pdifi2.tiling = VK_IMAGE_TILING_OPTIMAL;  // Ditto
            if (AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE & ahb_desc.usage) {
                pdifi2.usage |= (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
            }
            if (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER & ahb_desc.usage) {
                pdifi2.usage |= (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
            }
            if (AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP & ahb_desc.usage) {
                pdifi2.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            }
            if (AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT & ahb_desc.usage) {
                pdifi2.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
            }

            VkExternalImageFormatProperties ext_img_fmt_props = vku::InitStructHelper();
            VkImageFormatProperties2 ifp2 = vku::InitStructHelper(&ext_img_fmt_props);

            VkResult fmt_lookup_result =
                DispatchGetPhysicalDeviceImageFormatProperties2Helper(api_version, physical_device, &pdifi2, &ifp2);

            if ((VK_SUCCESS != fmt_lookup_result) || (0 == (ext_img_fmt_props.externalMemoryProperties.externalMemoryFeatures &
                                                            VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT))) {
                skip |= LogError(
                    "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01880", device, allocate_info_loc,
                    "Neither the VkExternalImageFormatProperties nor the VkExternalBufferProperties "
                    "structs for the AHardwareBuffer include the VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT flag. (AHB = %p).",
                    import_ahb_info->buffer);
            }
        }

        // Retrieve buffer and format properties of the provided AHardwareBuffer
        VkAndroidHardwareBufferPropertiesANDROID ahb_props = vku::InitStructHelper();
        DispatchGetAndroidHardwareBufferPropertiesANDROID(device, import_ahb_info->buffer, &ahb_props);

        // allocationSize must be the size returned by vkGetAndroidHardwareBufferPropertiesANDROID for the Android hardware buffer
        if (allocate_info.allocationSize != ahb_props.allocationSize) {
            skip |=
                LogError("VUID-VkMemoryAllocateInfo-allocationSize-02383", device, allocate_info_loc.dot(Field::allocationSize),
                         "(%" PRIu64 ") does not match the %s AHardwareBuffer's allocationSize (%" PRIu64 "). (AHB = %p).",
                         allocate_info.allocationSize, ahb_loc.Fields().c_str(), ahb_props.allocationSize, import_ahb_info->buffer);
        }

        // memoryTypeIndex must be one of those returned by vkGetAndroidHardwareBufferPropertiesANDROID for the AHardwareBuffer
        // Note: memoryTypeIndex is an index, memoryTypeBits is a bitmask
        uint32_t mem_type_bitmask = 1 << allocate_info.memoryTypeIndex;
        if (0 == (mem_type_bitmask & ahb_props.memoryTypeBits)) {
            skip |= LogError(
                "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385", device, allocate_info_loc.dot(Field::memoryTypeIndex),
                "(%" PRIu32
                ") does not correspond to a bit set in %s "
                "AHardwareBuffer's memoryTypeBits bitmask (0x%" PRIx32 "). (AHB = %p).",
                allocate_info.memoryTypeIndex, ahb_loc.Fields().c_str(), ahb_props.memoryTypeBits, import_ahb_info->buffer);
        }

        // Checks for allocations without a dedicated allocation requirement
        if ((nullptr == mem_ded_alloc_info) || (VK_NULL_HANDLE == mem_ded_alloc_info->image)) {
            // the Android hardware buffer must have a format of AHARDWAREBUFFER_FORMAT_BLOB and a usage that includes
            // AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER
            if ((uint64_t)AHARDWAREBUFFER_FORMAT_BLOB != ahb_desc.format) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02384", device, ahb_loc,
                                 "AHardwareBuffer_Desc's format (%u) is not AHARDWAREBUFFER_FORMAT_BLOB. (AHB = %p).",
                                 ahb_desc.format, import_ahb_info->buffer);
            } else if ((ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) == 0) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02384", device, ahb_loc,
                                 "AHardwareBuffer's usage (0x%" PRIx64
                                 ") does not include AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER. (AHB = %p).",
                                 ahb_desc.usage, import_ahb_info->buffer);
            }
        } else {  // Checks specific to import with a dedicated allocation requirement

            // Dedicated allocation have limit usage flags supported
            if (0 == (ahb_desc.usage & (AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                                        AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER))) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02386", device, ahb_loc,
                                 "AHardwareBuffer's usage is 0x%" PRIx64 ". (AHB = %p).", ahb_desc.usage, import_ahb_info->buffer);
            }

            auto image_state = Get<vvl::Image>(mem_ded_alloc_info->image);
            ASSERT_AND_RETURN_SKIP(image_state);
            const auto *ici = &image_state->create_info;
            const Location &dedicated_image_loc = allocate_info_loc.dot(Struct::VkMemoryDedicatedAllocateInfo, Field::image);

            //  the format of image must be VK_FORMAT_UNDEFINED or the format returned by
            //  vkGetAndroidHardwareBufferPropertiesANDROID
            if (VK_FORMAT_UNDEFINED != ici->format) {
                // Mali drivers will not return a valid VkAndroidHardwareBufferPropertiesANDROID::allocationSize if the
                // FormatPropertiesANDROID is passed in as well so need to query again for the format
                VkAndroidHardwareBufferFormatPropertiesANDROID ahb_format_props = vku::InitStructHelper();
                VkAndroidHardwareBufferPropertiesANDROID dummy_ahb_props = vku::InitStructHelper(&ahb_format_props);
                DispatchGetAndroidHardwareBufferPropertiesANDROID(device, import_ahb_info->buffer, &dummy_ahb_props);
                if (ici->format != ahb_format_props.format) {
                    skip |= LogError(
                        "VUID-VkMemoryAllocateInfo-pNext-02387", mem_ded_alloc_info->image, dedicated_image_loc,
                        "was created with format (%s) which does not match the %s AHardwareBuffer's format (%s). (AHB = %p).",
                        string_VkFormat(ici->format), ahb_loc.Fields().c_str(), string_VkFormat(ahb_format_props.format),
                        import_ahb_info->buffer);
                }
            }

            // The width, height, and array layer dimensions of image and the Android hardwarebuffer must be identical
            if ((ici->extent.width != ahb_desc.width) || (ici->extent.height != ahb_desc.height) ||
                (ici->arrayLayers != ahb_desc.layers)) {
                skip |=
                    LogError("VUID-VkMemoryAllocateInfo-pNext-02388", mem_ded_alloc_info->image, dedicated_image_loc,
                             "was created with width (%" PRId32 "),  height (%" PRId32 "), and arrayLayers (%" PRId32
                             ") which not match those of the %s AHardwareBuffer (%" PRId32 " %" PRId32 " %" PRId32 "). (AHB = %p).",
                             ici->extent.width, ici->extent.height, ici->arrayLayers, ahb_loc.Fields().c_str(), ahb_desc.width,
                             ahb_desc.height, ahb_desc.layers, import_ahb_info->buffer);
            }

            if ((ahb_desc.usage & AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE) != 0) {
                if ((ici->mipLevels != 1) && (ici->mipLevels != FullMipChainLevels(ici->extent))) {
                    skip |= LogError(
                        "VUID-VkMemoryAllocateInfo-pNext-02389", mem_ded_alloc_info->image, ahb_loc,
                        "AHardwareBuffer_Desc's usage includes AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE but %s mipLevels (%" PRIu32
                        ") is neither 1 nor full mip "
                        "chain levels (%" PRIu32 "). (AHB = %p).",
                        dedicated_image_loc.Fields().c_str(), ici->mipLevels, FullMipChainLevels(ici->extent),
                        import_ahb_info->buffer);
                }
            } else {
                if (ici->mipLevels != 1) {
                    skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02586", mem_ded_alloc_info->image, ahb_loc,
                                     "AHardwareBuffer_Desc's  usage is 0x%" PRIx64 " but %s mipLevels is %" PRIu32 ". (AHB = %p).",
                                     ahb_desc.usage, dedicated_image_loc.Fields().c_str(), ici->mipLevels, import_ahb_info->buffer);
                }
            }

            // First check if any invalid Vulkan usages, then make sure for each used, the matching AHB usage is also included
            const VkImageUsageFlags valid_vk_usages = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
            if (ici->usage & ~(valid_vk_usages)) {
                skip |=
                    LogError("VUID-VkMemoryAllocateInfo-pNext-02390", mem_ded_alloc_info->image, dedicated_image_loc,
                             "was created with %s which are not listed in the AHardwareBuffer Usage Equivalence table. (AHB = %p).",
                             string_VkImageUsageFlags(ici->usage & ~(valid_vk_usages)).c_str(), import_ahb_info->buffer);
            }

            // Based on vkspec.html#memory-external-android-hardware-buffer-usage
            static std::unordered_map<VkImageUsageFlags, uint64_t> ahb_usage_map_v2a = {
                {VK_IMAGE_USAGE_SAMPLED_BIT, (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE},
                {VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, (uint64_t)AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE},
                {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, (uint64_t)AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER},
                {VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, (uint64_t)AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER},
                {VK_IMAGE_USAGE_STORAGE_BIT, (uint64_t)AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER},
            };

            for (const auto &[vk_usage, ahb_usage] : ahb_usage_map_v2a) {
                if (ici->usage & vk_usage) {
                    if (0 == (ahb_usage & ahb_desc.usage)) {
                        skip |= LogError(
                            "VUID-VkMemoryAllocateInfo-pNext-02390", mem_ded_alloc_info->image, dedicated_image_loc,
                            "was created with %s, but the AHB equivalent (%s) is not in AHardwareBuffer_Desc.usage (0x%" PRIx64
                            "). (AHB = %p).",
                            string_VkImageUsageFlags(vk_usage).c_str(), string_AHardwareBufferGpuUsage(ahb_usage), ahb_desc.usage,
                            import_ahb_info->buffer);
                    }
                }
            }
        }
    } else {  // Not an import
              // auto exp_mem_alloc_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(allocate_info.pNext);
              // auto mem_ded_alloc_info = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(allocate_info.pNext);

        if (exp_mem_alloc_info &&
            (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID & exp_mem_alloc_info->handleTypes)) {
            if (mem_ded_alloc_info) {
                if (mem_ded_alloc_info->image != VK_NULL_HANDLE && allocate_info.allocationSize != 0) {
                    skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-01874", mem_ded_alloc_info->image,
                                     allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                     "is %s but allocationSize is %" PRIu64 ".", FormatHandle(mem_ded_alloc_info->image).c_str(),
                                     allocate_info.allocationSize);
                }
                if (mem_ded_alloc_info->buffer != VK_NULL_HANDLE && allocate_info.allocationSize == 0) {
                    skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-07901", mem_ded_alloc_info->buffer,
                                     allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                     "is %s but allocationSize is 0.", FormatHandle(mem_ded_alloc_info->buffer).c_str());
                }
            } else if (0 == allocate_info.allocationSize) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-07900", device, allocate_info_loc.dot(Field::pNext),
                                 "chain does not contain an instance of VkMemoryDedicatedAllocateInfo, but allocationSize is 0.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const Location &loc) const {
    bool skip = false;
    if (auto image_state = Get<vvl::Image>(image)) {
        if (image_state->IsExternalBuffer() && (0 == image_state->GetBoundMemoryStates().size())) {
            const char *vuid = loc.function == Func::vkGetImageMemoryRequirements
                                   ? "VUID-vkGetImageMemoryRequirements-image-04004"
                                   : "VUID-VkImageMemoryRequirementsInfo2-image-01897";
            skip |= LogError(vuid, image, loc,
                             "was created with a VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID handleType, "
                             "which has not yet been "
                             "bound to memory, so image memory requirements can't yet be queried.");
        }
    }
    return skip;
}

bool core::Instance::ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(
    VkPhysicalDevice physical_device, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    const VkImageFormatProperties2 *pImageFormatProperties, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *ahb_usage = vku::FindStructInPNextChain<VkAndroidHardwareBufferUsageANDROID>(pImageFormatProperties->pNext);
    if (ahb_usage) {
        const auto *pdeifi = vku::FindStructInPNextChain<VkPhysicalDeviceExternalImageFormatInfo>(pImageFormatInfo->pNext);
        if ((!pdeifi) || (VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID != pdeifi->handleType)) {
            skip |= LogError("VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868", physical_device,
                             error_obj.location.dot(Field::pImageFormatProperties),
                             "includes a chained "
                             "VkAndroidHardwareBufferUsageANDROID struct, but pImageFormatInfo does not include a chained "
                             "VkPhysicalDeviceExternalImageFormatInfo struct with handleType "
                             "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateBufferImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                     VkBuffer buffer, const Location &loc) const {
    bool skip = false;
    if ((handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) == 0) {
        const char *vuid = loc.function == Func::vkBindBufferMemory ? "VUID-vkBindBufferMemory-memory-02986"
                                                                    : "VUID-VkBindBufferMemoryInfo-memory-02986";
        const LogObjectList objlist(buffer, memory);
        skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                         "(%s) was created with an AHB import operation which is not set "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID in the VkBuffer (%s) "
                         "VkExternalMemoryBufferCreateInfo::handleTypes (%s)",
                         FormatHandle(memory).c_str(), FormatHandle(buffer).c_str(),
                         string_VkExternalMemoryHandleTypeFlags(handle_types).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateImageImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                    VkImage image, const Location &loc) const {
    bool skip = false;
    if ((handle_types & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) == 0) {
        const char *vuid = loc.function == Func::vkBindImageMemory ? "VUID-vkBindImageMemory-memory-02990"
                                                                   : "VUID-VkBindImageMemoryInfo-memory-02990";
        const LogObjectList objlist(image, memory);
        skip |= LogError(vuid, objlist, loc.dot(Field::memory),
                         "(%s) was created with an AHB import operation which is not set "
                         "VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID in the VkImage (%s) "
                         "VkExternalMemoryImageCreateInfo::handleTypes (%s)",
                         FormatHandle(memory).c_str(), FormatHandle(image).c_str(),
                         string_VkExternalMemoryHandleTypeFlags(handle_types).c_str());
    }
    return skip;
}

// Validate creating an image with an external format
bool CoreChecks::ValidateCreateImageANDROID(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;

    const VkExternalFormatANDROID *ext_fmt_android = vku::FindStructInPNextChain<VkExternalFormatANDROID>(create_info.pNext);
    if (ext_fmt_android && (0 != ext_fmt_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED != create_info.format) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-01974", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") is non-zero, format is %s.", ext_fmt_android->externalFormat,
                             string_VkFormat(create_info.format));
        }

        if (0 != (VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT & create_info.flags)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02396", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") is non-zero, but flags is %s.", ext_fmt_android->externalFormat,
                             string_VkImageCreateFlags(create_info.flags).c_str());
        }

        if (0 != (~(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) &
                  create_info.usage)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02397", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") is non-zero, but usage is %s.", ext_fmt_android->externalFormat,
                             string_VkImageUsageFlags(create_info.usage).c_str());
        } else if (!enabled_features.externalFormatResolve &&
                   ((VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) & create_info.usage)) {
            skip |= LogError(
                "VUID-VkImageCreateInfo-pNext-09457", device,
                create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                "(%" PRIu64
                ") is non-zero, but usage is %s (without externalFormatResolve, only VK_IMAGE_USAGE_SAMPLED_BIT is allowed).",
                ext_fmt_android->externalFormat, string_VkImageUsageFlags(create_info.usage).c_str());
        }

        if (VK_IMAGE_TILING_OPTIMAL != create_info.tiling) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02398", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") is non-zero, but layout is %s.", ext_fmt_android->externalFormat,
                             string_VkImageTiling(create_info.tiling));
        }
        if (ahb_ext_formats_map.find(ext_fmt_android->externalFormat) == ahb_ext_formats_map.end()) {
            skip |= LogError("VUID-VkExternalFormatANDROID-externalFormat-01894", device,
                             create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                             "(%" PRIu64 ") has not been previously retrieved by vkGetAndroidHardwareBufferPropertiesANDROID().",
                             ext_fmt_android->externalFormat);
        }
    }

    if ((nullptr == ext_fmt_android) || (0 == ext_fmt_android->externalFormat)) {
        if (VK_FORMAT_UNDEFINED == create_info.format) {
            if (ext_fmt_android) {
                skip |= LogError("VUID-VkImageCreateInfo-pNext-01975", device, create_info_loc.dot(Field::format),
                                 "is VK_FORMAT_UNDEFINED, but the chained VkExternalFormatANDROID has an externalFormat of 0.");
            } else {
                skip |= LogError("VUID-VkImageCreateInfo-pNext-01975", device, create_info_loc.dot(Field::format),
                                 "is VK_FORMAT_UNDEFINED, but does not have a chained VkExternalFormatANDROID struct.");
            }
        }
    }

    const VkExternalMemoryImageCreateInfo *emici = vku::FindStructInPNextChain<VkExternalMemoryImageCreateInfo>(create_info.pNext);
    if (emici && (emici->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        if (create_info.imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02393", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfo, Field::handleTypes),
                             "includes VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, but imageType is %s.",
                             string_VkImageType(create_info.imageType));
        }

        if ((create_info.mipLevels != 1) && (create_info.mipLevels != FullMipChainLevels(create_info.extent))) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-02394", device,
                             create_info_loc.pNext(Struct::VkExternalMemoryImageCreateInfo, Field::handleTypes),
                             "includes VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, "
                             "but mipLevels is %" PRIu32 " (full chain mipLevels are %" PRIu32 ").",
                             create_info.mipLevels, FullMipChainLevels(create_info.extent));
        }
    }

    return skip;
}

// Validate creating an image view with an AHB format
bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    auto image_state = Get<vvl::Image>(create_info.image);
    ASSERT_AND_RETURN_SKIP(image_state);

    if (image_state->HasAHBFormat()) {
        if (VK_FORMAT_UNDEFINED != create_info.format) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02399", create_info.image, create_info_loc.dot(Field::format),
                             "is %s (not VK_FORMAT_UNDEFINED) but the VkImageViewCreateInfo struct has a chained "
                             "VkExternalFormatANDROID struct.",
                             string_VkFormat(create_info.format));
        }

        // Chain must include a compatible ycbcr conversion
        bool conv_found = false;
        uint64_t external_format = 0;
        const VkSamplerYcbcrConversionInfo *ycbcr_conv_info =
            vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(create_info.pNext);
        if (ycbcr_conv_info != nullptr) {
            if (auto ycbcr_state = Get<vvl::SamplerYcbcrConversion>(ycbcr_conv_info->conversion)) {
                conv_found = true;
                external_format = ycbcr_state->external_format;
            }
        }
        if (!conv_found) {
            const LogObjectList objlist(create_info.image);
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02400", objlist,
                             create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                             "is not valid (or forgot to add VkSamplerYcbcrConversionInfo).");
        } else if ((external_format != image_state->ahb_format)) {
            const LogObjectList objlist(create_info.image, ycbcr_conv_info->conversion);
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02400", objlist,
                             create_info_loc.pNext(Struct::VkSamplerYcbcrConversionInfo, Field::conversion),
                             "(%s) was created with externalFormat (%" PRIu64
                             ") which is different then image chain VkExternalFormatANDROID::externalFormat (%" PRIu64 ").",
                             FormatHandle(ycbcr_conv_info->conversion).c_str(), external_format, image_state->ahb_format);
        }

        // Errors in create_info swizzles
        if (IsIdentitySwizzle(create_info.components) == false) {
            skip |= LogError("VUID-VkImageViewCreateInfo-image-02401", create_info.image, create_info_loc.dot(Field::image),
                             "was chained with a VkExternalFormatANDROID struct, but "
                             "includes one or more non-identity component swizzles\n%s.",
                             string_VkComponentMapping(create_info.components).c_str());
        }
    }

    return skip;
}

#else  // !defined(VK_USE_PLATFORM_ANDROID_KHR)

bool CoreChecks::ValidateAllocateMemoryANDROID(const VkMemoryAllocateInfo &allocate_info, const Location &allocate_info_loc) const {
    return false;
}

bool core::Instance::ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(
    VkPhysicalDevice physical_device, const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
    const VkImageFormatProperties2 *pImageFormatProperties, const ErrorObject &error_obj) const {
    return false;
}

bool CoreChecks::ValidateGetImageMemoryRequirementsANDROID(const VkImage image, const Location &loc) const { return false; }

bool CoreChecks::ValidateBufferImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                     VkBuffer buffer, const Location &loc) const {
    return false;
}

bool CoreChecks::ValidateImageImportedHandleANDROID(VkExternalMemoryHandleTypeFlags handle_types, VkDeviceMemory memory,
                                                    VkImage image, const Location &loc) const {
    return false;
}

bool CoreChecks::ValidateCreateImageANDROID(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    return false;
}

bool CoreChecks::ValidateCreateImageViewANDROID(const VkImageViewCreateInfo &create_info, const Location &create_info_loc) const {
    return false;
}

#endif
