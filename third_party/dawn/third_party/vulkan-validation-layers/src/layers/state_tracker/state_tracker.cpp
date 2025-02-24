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

#include <algorithm>

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_struct_helper.hpp>

#include "containers/custom_containers.h"
#include "utils/vk_layer_utils.h"

#include "state_tracker/state_tracker.h"
#include "sync/sync_utils.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/device_state.h"
#include "state_tracker/queue_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/ray_tracing_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/device_generated_commands_state.h"
#include "chassis/chassis_modification_state.h"
#include "spirv-tools/optimizer.hpp"

#include "chassis/chassis.h"

namespace vvl {
Device::~Device() { DestroyObjectMaps(); }

// NOTE:  Beware the lifespan of the rp_begin when holding  the return.  If the rp_begin isn't a "safe" copy, "IMAGELESS"
//        attachments won't persist past the API entry point exit.
static std::pair<uint32_t, const VkImageView *> GetFramebufferAttachments(const VkRenderPassBeginInfo &rp_begin,
                                                                          const Framebuffer &fb_state) {
    const VkImageView *attachments = fb_state.create_info.pAttachments;
    uint32_t count = fb_state.create_info.attachmentCount;
    if (fb_state.create_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
        const auto *framebuffer_attachments = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(rp_begin.pNext);
        if (framebuffer_attachments) {
            attachments = framebuffer_attachments->pAttachments;
            count = framebuffer_attachments->attachmentCount;
        }
    }
    return std::make_pair(count, attachments);
}

template <typename ImageViewPointer, typename Get>
std::vector<ImageViewPointer> GetAttachmentViewsImpl(const VkRenderPassBeginInfo &rp_begin, const Framebuffer &fb_state,
                                                     const Get &get_fn) {
    std::vector<ImageViewPointer> views;

    const auto count_attachment = GetFramebufferAttachments(rp_begin, fb_state);
    const auto attachment_count = count_attachment.first;
    const auto *attachments = count_attachment.second;
    views.resize(attachment_count, nullptr);
    for (uint32_t i = 0; i < attachment_count; i++) {
        if (attachments[i] != VK_NULL_HANDLE) {
            views[i] = get_fn(attachments[i]);
        }
    }
    return views;
}

std::vector<std::shared_ptr<const ImageView>> Device::GetAttachmentViews(const VkRenderPassBeginInfo &rp_begin,
                                                                         const Framebuffer &fb_state) const {
    auto get_fn = [this](VkImageView handle) { return this->Get<ImageView>(handle); };
    return GetAttachmentViewsImpl<std::shared_ptr<const ImageView>>(rp_begin, fb_state, get_fn);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Android-specific validation that uses types defined only with VK_USE_PLATFORM_ANDROID_KHR
// This could also move into a seperate core_validation_android.cpp file... ?

VkFormatFeatureFlags2KHR Device::GetExternalFormatFeaturesANDROID(const void *pNext) const {
    VkFormatFeatureFlags2KHR format_features = 0;
    const uint64_t external_format = GetExternalFormat(pNext);
    if ((0 != external_format)) {
        // VUID 01894 will catch if not found in map
        auto it = ahb_ext_formats_map.find(external_format);
        if (it != ahb_ext_formats_map.end()) {
            format_features = it->second;
        }
    }
    return format_features;
}

void Device::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer *buffer,
                                                                     VkAndroidHardwareBufferPropertiesANDROID *pProperties,
                                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    uint64_t external_format = 0;
    auto ahb_format_props2 = vku::FindStructInPNextChain<VkAndroidHardwareBufferFormatProperties2ANDROID>(pProperties->pNext);
    if (ahb_format_props2) {
        external_format = ahb_format_props2->externalFormat;
        ahb_ext_formats_map.insert(external_format, ahb_format_props2->formatFeatures);
    } else {
        auto ahb_format_props = vku::FindStructInPNextChain<VkAndroidHardwareBufferFormatPropertiesANDROID>(pProperties->pNext);
        if (ahb_format_props) {
            external_format = ahb_format_props->externalFormat;
            ahb_ext_formats_map.insert(external_format, static_cast<VkFormatFeatureFlags2KHR>(ahb_format_props->formatFeatures));
        }
    }

    // For external format resolve, we need to also track externalFormat with its color attachment property
    if (enabled_features.externalFormatResolve) {
        auto ahb_format_resolve_props =
            vku::FindStructInPNextChain<VkAndroidHardwareBufferFormatResolvePropertiesANDROID>(pProperties->pNext);
        if (ahb_format_resolve_props && external_format != 0) {
            // easy case, caller provided both structs for us
            ahb_ext_resolve_formats_map.insert(external_format, ahb_format_resolve_props->colorAttachmentFormat);
        } else {
            // If caller didn't provide both struct, re-call for them
            VkAndroidHardwareBufferFormatResolvePropertiesANDROID new_ahb_format_resolve_props = vku::InitStructHelper();
            VkAndroidHardwareBufferFormatPropertiesANDROID new_ahb_format_props =
                vku::InitStructHelper(&new_ahb_format_resolve_props);
            VkAndroidHardwareBufferPropertiesANDROID new_ahb_props = vku::InitStructHelper(&new_ahb_format_props);
            DispatchGetAndroidHardwareBufferPropertiesANDROID(device, buffer, &new_ahb_props);
            ahb_ext_resolve_formats_map.insert(new_ahb_format_props.externalFormat,
                                               new_ahb_format_resolve_props.colorAttachmentFormat);
        }
    }
}

#else

VkFormatFeatureFlags2KHR Device::GetExternalFormatFeaturesANDROID(const void *pNext) const {
    (void)pNext;
    return 0;
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR

VkFormatFeatureFlags2 Instance::GetImageFormatFeatures(VkPhysicalDevice physical_device, bool has_format_feature2,
                                                       bool has_drm_modifiers, VkDevice device, VkImage image, VkFormat format,
                                                       VkImageTiling tiling) {
    VkFormatFeatureFlags2 format_features = 0;

    // Add feature support according to Image Format Features (vkspec.html#resources-image-format-features)
    // if format is AHB external format then the features are already set
    if (has_format_feature2) {
        VkDrmFormatModifierPropertiesList2EXT fmt_drm_props = vku::InitStructHelper();
        auto fmt_props_3 = vku::InitStruct<VkFormatProperties3KHR>(has_drm_modifiers ? &fmt_drm_props : nullptr);
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);

        DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);

        fmt_props_3.linearTilingFeatures |= fmt_props_2.formatProperties.linearTilingFeatures;
        fmt_props_3.optimalTilingFeatures |= fmt_props_2.formatProperties.optimalTilingFeatures;
        fmt_props_3.bufferFeatures |= fmt_props_2.formatProperties.bufferFeatures;

        if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            VkImageDrmFormatModifierPropertiesEXT drm_format_props = vku::InitStructHelper();

            // Find the image modifier
            DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, &drm_format_props);

            std::vector<VkDrmFormatModifierProperties2EXT> drm_mod_props;
            drm_mod_props.resize(fmt_drm_props.drmFormatModifierCount);
            fmt_drm_props.pDrmFormatModifierProperties = &drm_mod_props[0];

            // Second query to have all the modifiers filled
            DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);

            // Look for the image modifier in the list
            for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
                if (fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifier == drm_format_props.drmFormatModifier) {
                    format_features = fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                    break;
                }
            }
        } else {
            format_features =
                (tiling == VK_IMAGE_TILING_LINEAR) ? fmt_props_3.linearTilingFeatures : fmt_props_3.optimalTilingFeatures;
        }
    } else if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = vku::InitStructHelper();
        DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, &drm_format_properties);

        VkFormatProperties2 format_properties_2 = vku::InitStructHelper();
        VkDrmFormatModifierPropertiesListEXT drm_properties_list = vku::InitStructHelper();
        format_properties_2.pNext = (void *)&drm_properties_list;
        DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &format_properties_2);
        std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
        drm_properties.resize(drm_properties_list.drmFormatModifierCount);
        drm_properties_list.pDrmFormatModifierProperties = &drm_properties[0];
        DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &format_properties_2);

        for (uint32_t i = 0; i < drm_properties_list.drmFormatModifierCount; i++) {
            if (drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifier == drm_format_properties.drmFormatModifier) {
                format_features = drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                break;
            }
        }
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
        format_features =
            (tiling == VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures : format_properties.optimalTilingFeatures;
    }
    return format_features;
}

std::shared_ptr<Image> Device::CreateImageState(VkImage handle, const VkImageCreateInfo *create_info,
                                                VkFormatFeatureFlags2 features) {
    return std::make_shared<Image>(*this, handle, create_info, features);
}

std::shared_ptr<Image> Device::CreateImageState(VkImage handle, const VkImageCreateInfo *create_info, VkSwapchainKHR swapchain,
                                                uint32_t swapchain_index, VkFormatFeatureFlags2 features) {
    return std::make_shared<Image>(*this, handle, create_info, swapchain, swapchain_index, features);
}

void Device::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    VkFormatFeatureFlags2KHR format_features = 0;
    if (IsExtEnabled(extensions.vk_android_external_memory_android_hardware_buffer)) {
        format_features = GetExternalFormatFeaturesANDROID(pCreateInfo->pNext);
    }
    if (format_features == 0) {
        format_features = instance_state->GetImageFormatFeatures(physical_device, has_format_feature2,
                                                                 IsExtEnabled(extensions.vk_ext_image_drm_format_modifier), device,
                                                                 *pImage, pCreateInfo->format, pCreateInfo->tiling);
    }
    Add(CreateImageState(*pImage, pCreateInfo, format_features));
}

void Device::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator,
                                       const RecordObject &record_obj) {
    Destroy<Image>(image);
}

void Device::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                             const VkClearColorValue *pColor, uint32_t rangeCount,
                                             const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (cb_state) {
        cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(image));
    }
}

void Device::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                    const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (cb_state) {
        cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(image));
    }
}

void Device::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkImageCopy *pRegions, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(srcImage), Get<Image>(dstImage));
}

void Device::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                           const RecordObject &record_obj) {
    PreCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
}

void Device::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                        const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(pCopyImageInfo->srcImage),
                                Get<Image>(pCopyImageInfo->dstImage));
}

void Device::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                          VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                          const VkImageResolve *pRegions, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(srcImage), Get<Image>(dstImage));
}

void Device::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                              const RecordObject &record_obj) {
    PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
}

void Device::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                           const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(pResolveImageInfo->srcImage),
                                Get<Image>(pResolveImageInfo->dstImage));
}

void Device::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const VkImageBlit *pRegions, VkFilter filter, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(srcImage), Get<Image>(dstImage));
}

void Device::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                           const RecordObject &record_obj) {
    PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void Device::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                        const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(pBlitImageInfo->srcImage),
                                Get<Image>(pBlitImageInfo->dstImage));
}

struct BufferAddressInfillUpdateOps {
    using Map = typename Device::BufferAddressRangeMap;
    using Iterator = typename Map::iterator;
    using Value = typename Map::value_type;
    using Mapped = typename Map::mapped_type;
    using Range = typename Map::key_type;
    void infill(Map &map, const Iterator &pos, const Range &infill_range) const {
        map.insert(pos, Value(infill_range, insert_value));
    }
    void update(const Iterator &pos) const {
        auto &current_buffer_list = pos->second;
        assert(!current_buffer_list.empty());
        const auto buffer_found_it = std::find(current_buffer_list.begin(), current_buffer_list.end(), insert_value[0]);
        if (buffer_found_it == current_buffer_list.end()) {
            if (current_buffer_list.capacity() <= (current_buffer_list.size() + 1)) {
                current_buffer_list.reserve(current_buffer_list.capacity() * 2);
            }
            current_buffer_list.emplace_back(insert_value[0]);
        }
    }
    const Mapped &insert_value;
};

std::shared_ptr<Buffer> Device::CreateBufferState(VkBuffer handle, const VkBufferCreateInfo *create_info) {
    return std::make_shared<Buffer>(*this, handle, create_info);
}

void Device::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                        const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    std::shared_ptr<Buffer> buffer_state = CreateBufferState(*pBuffer, pCreateInfo);

    const auto *opaque_capture_address = vku::FindStructInPNextChain<VkBufferOpaqueCaptureAddressCreateInfo>(pCreateInfo->pNext);
    if (opaque_capture_address && (opaque_capture_address->opaqueCaptureAddress != 0)) {
        WriteLockGuard guard(buffer_address_lock_);
        // address is used for GPU-AV and ray tracing buffer validation
        buffer_state->deviceAddress = opaque_capture_address->opaqueCaptureAddress;
        const auto address_range = buffer_state->DeviceAddressRange();

        BufferAddressInfillUpdateOps ops{{buffer_state.get()}};
        sparse_container::infill_update_range(buffer_address_map_, address_range, ops);
    }

    const VkBufferUsageFlags descriptor_buffer_usages =
        VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    if ((buffer_state->usage & descriptor_buffer_usages) != 0) {
        descriptorBufferAddressSpaceSize += pCreateInfo->size;

        if ((buffer_state->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) != 0) {
            resourceDescriptorBufferAddressSpaceSize += pCreateInfo->size;
        }

        if ((buffer_state->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) != 0) {
            samplerDescriptorBufferAddressSpaceSize += pCreateInfo->size;
        }
    }
    Add(std::move(buffer_state));
}

std::shared_ptr<BufferView> Device::CreateBufferViewState(const std::shared_ptr<Buffer> &buffer, VkBufferView handle,
                                                          const VkBufferViewCreateInfo *create_info,
                                                          VkFormatFeatureFlags2KHR format_features) {
    return std::make_shared<BufferView>(buffer, handle, create_info, format_features);
}

void Device::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkBufferView *pView,
                                            const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    auto buffer_state = Get<Buffer>(pCreateInfo->buffer);

    VkFormatFeatureFlags2KHR buffer_features;
    if (has_format_feature2) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
        DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, pCreateInfo->format, &fmt_props_2);
        buffer_features = fmt_props_3.bufferFeatures | fmt_props_2.formatProperties.bufferFeatures;
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, pCreateInfo->format, &format_properties);
        buffer_features = format_properties.bufferFeatures;
    }

    Add(CreateBufferViewState(buffer_state, *pView, pCreateInfo, buffer_features));
}

std::shared_ptr<ImageView> Device::CreateImageViewState(const std::shared_ptr<Image> &image_state, VkImageView handle,
                                                        const VkImageViewCreateInfo *create_info,
                                                        VkFormatFeatureFlags2KHR format_features,
                                                        const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageView>(image_state, handle, create_info, format_features, cubic_props);
}

void Device::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                           const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    auto image_state = Get<Image>(pCreateInfo->image);
    ASSERT_AND_RETURN(image_state);

    VkFormatFeatureFlags2KHR format_features = 0;
    if (image_state->HasAHBFormat() == true) {
        // The ImageView uses same Image's format feature since they share same AHB
        format_features = image_state->format_features;
    } else {
        format_features = instance_state->GetImageFormatFeatures(
            physical_device, has_format_feature2, IsExtEnabled(extensions.vk_ext_image_drm_format_modifier), device,
            image_state->VkHandle(), pCreateInfo->format, image_state->create_info.tiling);
    }

    // filter_cubic_props is used in CmdDraw validation. But it takes a lot of performance if it does in CmdDraw.
    VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props = vku::InitStructHelper();
    if (IsExtEnabled(extensions.vk_ext_filter_cubic)) {
        VkPhysicalDeviceImageViewImageFormatInfoEXT imageview_format_info = vku::InitStructHelper();
        imageview_format_info.imageViewType = pCreateInfo->viewType;
        VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper(&imageview_format_info);
        image_format_info.type = image_state->create_info.imageType;
        image_format_info.format = image_state->create_info.format;
        image_format_info.tiling = image_state->create_info.tiling;
        auto usage_create_info = vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(pCreateInfo->pNext);
        image_format_info.usage = usage_create_info ? usage_create_info->usage : image_state->create_info.usage;
        image_format_info.flags = image_state->create_info.flags;

        VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&filter_cubic_props);

        DispatchGetPhysicalDeviceImageFormatProperties2Helper(api_version, physical_device, &image_format_info,
                                                              &image_format_properties);
    }

    Add(CreateImageViewState(image_state, *pView, pCreateInfo, format_features, filter_cubic_props));
}

void Device::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                        const VkBufferCopy *pRegions, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Buffer>(srcBuffer), Get<Buffer>(dstBuffer));
}

void Device::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                            const RecordObject &record_obj) {
    PreCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj);
}

void Device::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                         const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Buffer>(pCopyBufferInfo->srcBuffer),
                                Get<Buffer>(pCopyBufferInfo->dstBuffer));
}

void Device::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator,
                                           const RecordObject &record_obj) {
    Destroy<ImageView>(imageView);
}

void Device::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator,
                                        const RecordObject &record_obj) {
    if (auto buffer_state = Get<Buffer>(buffer)) {
        WriteLockGuard guard(buffer_address_lock_);

        const VkBufferUsageFlags descriptor_buffer_usages =
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        if ((buffer_state->usage & descriptor_buffer_usages) != 0) {
            descriptorBufferAddressSpaceSize -= buffer_state->create_info.size;

            if (buffer_state->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) {
                resourceDescriptorBufferAddressSpaceSize -= buffer_state->create_info.size;
            }

            if (buffer_state->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) {
                samplerDescriptorBufferAddressSpaceSize -= buffer_state->create_info.size;
            }
        }

        if (buffer_state->deviceAddress != 0) {
            const auto address_range = buffer_state->DeviceAddressRange();

            buffer_address_map_.erase_range_or_touch(address_range, [buffer_state_raw = buffer_state.get()](auto &buffers) {
                assert(!buffers.empty());
                const auto buffer_found_it = std::find(buffers.begin(), buffers.end(), buffer_state_raw);
                assert(buffer_found_it != buffers.end());

                // If buffer list only has one element, remove range map entry.
                // Else, remove target buffer from buffer list.
                if (buffer_found_it != buffers.end()) {
                    if (buffers.size() == 1) {
                        return true;
                    } else {
                        assert(!buffers.empty());
                        const size_t i = std::distance(buffers.begin(), buffer_found_it);
                        std::swap(buffers[i], buffers[buffers.size() - 1]);
                        buffers.resize(buffers.size() - 1);
                        return false;
                    }
                }

                return false;
            });
        }
    }
    Destroy<Buffer>(buffer);
}

void Device::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator,
                                            const RecordObject &record_obj) {
    Destroy<BufferView>(bufferView);
}

void Device::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                        VkDeviceSize size, uint32_t data, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Buffer>(dstBuffer));
}

void Device::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                               const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(srcImage), Get<Buffer>(dstBuffer));
}

void Device::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                   const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                   const RecordObject &record_obj) {
    PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void Device::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Image>(pCopyImageToBufferInfo->srcImage),
                                Get<Buffer>(pCopyImageToBufferInfo->dstBuffer));
}

void Device::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                               const VkBufferImageCopy *pRegions, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Buffer>(srcBuffer), Get<Image>(dstImage));
}

void Device::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                   const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                   const RecordObject &record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void Device::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Buffer>(pCopyBufferToImageInfo->srcBuffer),
                                Get<Image>(pCopyBufferToImageInfo->dstImage));
}

// Gets union of all features defined by Potential Format Features
// except, does not handle the external format case for AHB as that only can be used for sampled images
VkFormatFeatureFlags2KHR Device::GetPotentialFormatFeatures(VkFormat format) const {
    VkFormatFeatureFlags2KHR format_features = 0;

    if (format != VK_FORMAT_UNDEFINED) {
        if (has_format_feature2) {
            VkDrmFormatModifierPropertiesList2EXT fmt_drm_props = vku::InitStructHelper();
            auto fmt_props_3 = vku::InitStruct<VkFormatProperties3KHR>(
                IsExtEnabled(extensions.vk_ext_image_drm_format_modifier) ? &fmt_drm_props : nullptr);
            VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);

            DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);

            format_features |= fmt_props_2.formatProperties.linearTilingFeatures;
            format_features |= fmt_props_2.formatProperties.optimalTilingFeatures;

            format_features |= fmt_props_3.linearTilingFeatures;
            format_features |= fmt_props_3.optimalTilingFeatures;

            if (IsExtEnabled(extensions.vk_ext_image_drm_format_modifier)) {
                std::vector<VkDrmFormatModifierProperties2EXT> drm_properties;
                drm_properties.resize(fmt_drm_props.drmFormatModifierCount);
                fmt_drm_props.pDrmFormatModifierProperties = drm_properties.data();
                DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);

                for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
                    format_features |= fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                }
            }
        } else {
            VkFormatProperties format_properties;
            DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
            format_features |= format_properties.linearTilingFeatures;
            format_features |= format_properties.optimalTilingFeatures;

            if (IsExtEnabled(extensions.vk_ext_image_drm_format_modifier)) {
                VkDrmFormatModifierPropertiesListEXT fmt_drm_props = vku::InitStructHelper();
                VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_drm_props);

                DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);

                std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
                drm_properties.resize(fmt_drm_props.drmFormatModifierCount);
                fmt_drm_props.pDrmFormatModifierProperties = drm_properties.data();
                DispatchGetPhysicalDeviceFormatProperties2Helper(api_version, physical_device, format, &fmt_props_2);

                for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
                    format_features |= fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                }
            }
        }
    }

    return format_features;
}

void Instance::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    // The current object represents the VkInstance, look up / create the object for the device.
    dispatch::Device *device_object = dispatch::GetData(*pDevice);
    auto *validation_data = device_object->GetValidationObject(this->container_type);
    auto *device_state = static_cast<Device *>(validation_data);

    // finish setup in the object representing the device
    device_state->PostCreateDevice(pCreateInfo, record_obj.location);

    if (IsExtEnabled(extensions.vk_nv_cooperative_matrix)) {
        uint32_t num_cooperative_matrix_properties_nv = 0;
        DispatchGetPhysicalDeviceCooperativeMatrixPropertiesNV(gpu, &num_cooperative_matrix_properties_nv, NULL);
        device_state->cooperative_matrix_properties_nv.resize(num_cooperative_matrix_properties_nv,
                                                              vku::InitStruct<VkCooperativeMatrixPropertiesNV>());

        DispatchGetPhysicalDeviceCooperativeMatrixPropertiesNV(gpu, &num_cooperative_matrix_properties_nv,
                                                               device_state->cooperative_matrix_properties_nv.data());
    }

    if (IsExtEnabled(extensions.vk_khr_cooperative_matrix)) {
        uint32_t num_cooperative_matrix_properties_khr = 0;
        DispatchGetPhysicalDeviceCooperativeMatrixPropertiesKHR(gpu, &num_cooperative_matrix_properties_khr, NULL);
        device_state->cooperative_matrix_properties_khr.resize(num_cooperative_matrix_properties_khr,
                                                               vku::InitStruct<VkCooperativeMatrixPropertiesKHR>());

        DispatchGetPhysicalDeviceCooperativeMatrixPropertiesKHR(gpu, &num_cooperative_matrix_properties_khr,
                                                                device_state->cooperative_matrix_properties_khr.data());
    }

    if (IsExtEnabled(extensions.vk_nv_cooperative_matrix2)) {
        uint32_t num_cooperative_matrix_flexible_dimensions_properties = 0;
        DispatchGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
            gpu, &num_cooperative_matrix_flexible_dimensions_properties, NULL);
        device_state->cooperative_matrix_flexible_dimensions_properties.resize(
            num_cooperative_matrix_flexible_dimensions_properties,
            vku::InitStruct<VkCooperativeMatrixFlexibleDimensionsPropertiesNV>());

        DispatchGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(
            gpu, &num_cooperative_matrix_flexible_dimensions_properties,
            device_state->cooperative_matrix_flexible_dimensions_properties.data());
    }

    if (IsExtEnabled(extensions.vk_nv_cooperative_vector)) {
        uint32_t num_cooperative_vector_properties_nv = 0;
        DispatchGetPhysicalDeviceCooperativeVectorPropertiesNV(gpu, &num_cooperative_vector_properties_nv, NULL);
        device_state->cooperative_vector_properties_nv.resize(num_cooperative_vector_properties_nv,
                                                              vku::InitStruct<VkCooperativeVectorPropertiesNV>());

        DispatchGetPhysicalDeviceCooperativeVectorPropertiesNV(gpu, &num_cooperative_vector_properties_nv,
                                                               device_state->cooperative_vector_properties_nv.data());
    }
#if defined(VVL_TRACY_GPU)
    std::vector<VkTimeDomainKHR> time_domains;
    uint32_t time_domain_count = 0;
    VkResult result = DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu, &time_domain_count, nullptr);
    assert(result == VK_SUCCESS);
    time_domains.resize(time_domain_count);
    result = DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(gpu, &time_domain_count, time_domains.data());
    assert(result == VK_SUCCESS);

    bool found_tracy_required_time_domain = false;
    for (VkTimeDomainEXT time_domain : time_domains) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        if (time_domain == VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT) {
            found_tracy_required_time_domain = true;
            break;
        }
#else
        if (time_domain == VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT) {
            found_tracy_required_time_domain = true;
            break;
        }
#endif
    }
    (void)found_tracy_required_time_domain;
    assert(found_tracy_required_time_domain);

#endif
}

std::shared_ptr<Queue> Device::CreateQueue(VkQueue handle, uint32_t family_index, uint32_t queue_index,
                                           VkDeviceQueueCreateFlags flags, const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::make_shared<Queue>(*this, handle, family_index, queue_index, flags, queueFamilyProperties);
}

void Device::PostCreateDevice(const VkDeviceCreateInfo *pCreateInfo, const Location &loc) {
    const auto *device_group_ci = vku::FindStructInPNextChain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext);
    if (device_group_ci) {
        physical_device_count = device_group_ci->physicalDeviceCount;
        if (physical_device_count == 0) {
            physical_device_count =
                1;  // see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html
        }
        device_group_create_info = *device_group_ci;
    } else {
        device_group_create_info = vku::InitStructHelper();
        device_group_create_info.physicalDeviceCount = 1;  // see previous VkDeviceGroupDeviceCreateInfo link
        device_group_create_info.pPhysicalDevices = &physical_device;
        physical_device_count = 1;
    }

    {
        uint32_t n_props = 0;
        std::vector<VkExtensionProperties> props;
        DispatchEnumerateDeviceExtensionProperties(physical_device, NULL, &n_props, NULL);
        props.resize(n_props);
        DispatchEnumerateDeviceExtensionProperties(physical_device, NULL, &n_props, props.data());

        unordered_set<Extension> phys_dev_extensions;
        for (const auto &ext_prop : props) {
            phys_dev_extensions.insert(GetExtension(ext_prop.extensionName));
        }

        // Even if VK_KHR_format_feature_flags2 is available, we need to have
        // a path to grab that information from the physical device. This
        // requires to have VK_KHR_get_physical_device_properties2 enabled or
        // Vulkan 1.1 (which made this core).
        has_format_feature2 =
            (api_version >= VK_API_VERSION_1_1 || IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) &&
            phys_dev_extensions.find(Extension::_VK_KHR_format_feature_flags2) != phys_dev_extensions.end();

        // feature is required if 1.3 or extension is supported
        has_robust_image_access =
            (api_version >= VK_API_VERSION_1_3 || IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) &&
            phys_dev_extensions.find(Extension::_VK_EXT_image_robustness) != phys_dev_extensions.end();

        if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2) &&
            phys_dev_extensions.find(Extension::_VK_EXT_robustness2) != phys_dev_extensions.end()) {
            VkPhysicalDeviceRobustness2FeaturesEXT robustness_2_features = vku::InitStructHelper();
            VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper(&robustness_2_features);
            DispatchGetPhysicalDeviceFeatures2Helper(api_version, physical_device, &features2);
            has_robust_image_access2 = robustness_2_features.robustImageAccess2;
            has_robust_buffer_access2 = robustness_2_features.robustBufferAccess2;
        } else {
            has_robust_image_access2 = false;
            has_robust_buffer_access2 = false;
        }
    }

    // Store queue family data
    if (pCreateInfo->pQueueCreateInfos != nullptr) {
        uint32_t num_queue_families = 0;
        DispatchGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties_list(num_queue_families);
        DispatchGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families,
                                                                       queue_family_properties_list.data());

        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            const VkDeviceQueueCreateInfo &queue_create_info = pCreateInfo->pQueueCreateInfos[i];
            queue_family_index_set.insert(queue_create_info.queueFamilyIndex);
            device_queue_info_list.emplace_back(
                DeviceQueueInfo{i, queue_create_info.queueFamilyIndex, queue_create_info.flags, queue_create_info.queueCount});
        }
        for (const auto &queue_info : device_queue_info_list) {
            for (uint32_t i = 0; i < queue_info.queue_count; i++) {
                VkQueue queue = VK_NULL_HANDLE;
                // vkGetDeviceQueue2() was added in vulkan 1.1, and there was never a KHR version of it.
                if (api_version >= VK_API_VERSION_1_1 && queue_info.flags != 0) {
                    VkDeviceQueueInfo2 get_info = vku::InitStructHelper();
                    get_info.flags = queue_info.flags;
                    get_info.queueFamilyIndex = queue_info.queue_family_index;
                    get_info.queueIndex = i;
                    DispatchGetDeviceQueue2(device, &get_info, &queue);
                } else {
                    DispatchGetDeviceQueue(device, queue_info.queue_family_index, i, &queue);
                }
                assert(queue != VK_NULL_HANDLE);
                Add(CreateQueue(queue, queue_info.queue_family_index, i, queue_info.flags,
                                queue_family_properties_list[queue_info.queue_family_index]));
            }
        }
    }

    // Query queue family extension properties
    if (IsExtEnabled(extensions.vk_khr_get_physical_device_properties2)) {
        uint32_t queue_family_count = (uint32_t)physical_device_state->queue_family_properties.size();
        auto &ext_props = queue_family_ext_props;
        ext_props.resize(queue_family_count);

        std::vector<VkQueueFamilyProperties2> props(queue_family_count, vku::InitStruct<VkQueueFamilyProperties2>());

        if (extensions.vk_khr_video_queue) {
            for (uint32_t i = 0; i < queue_family_count; ++i) {
                ext_props[i].query_result_status_props = vku::InitStructHelper();
                ext_props[i].video_props = vku::InitStructHelper(&ext_props[i].query_result_status_props);
                props[i].pNext = &ext_props[i].video_props;
            }
        }

        DispatchGetPhysicalDeviceQueueFamilyProperties2Helper(api_version, physical_device, &queue_family_count, props.data());
    }

    if (IsExtEnabled(extensions.vk_khr_performance_query)) {
        uint32_t queue_family_count = (uint32_t)physical_device_state->queue_family_properties.size();
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            uint32_t counterCount;
            DispatchEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physical_device, i, &counterCount, nullptr,
                                                                                  nullptr);

            std::unique_ptr<QueueFamilyPerfCounters> queue_family_counters(new QueueFamilyPerfCounters());
            queue_family_counters->counters.resize(counterCount);

            DispatchEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physical_device, i, &counterCount,
                                                                                  queue_family_counters->counters.data(), nullptr);

            physical_device_state->perf_counters[i] = std::move(queue_family_counters);
        }
    }

    // internal pipeline cache control
    const auto *cache_control = vku::FindStructInPNextChain<VkDevicePipelineBinaryInternalCacheControlKHR>(pCreateInfo->pNext);
    disable_internal_pipeline_cache = cache_control && cache_control->disableInternalCache;
}

void Device::DestroyObjectMaps() {
    command_pool_map_.clear();
    assert(command_buffer_map_.empty());
    pipeline_map_.clear();
    shader_object_map_.clear();
    render_pass_map_.clear();

    // This will also delete all sets in the pool & remove them from setMap
    descriptor_pool_map_.clear();
    // All sets should be removed
    assert(descriptor_set_map_.empty());
    desc_template_map_.clear();
    descriptor_set_layout_map_.clear();
    // Because swapchains are associated with Surfaces, which are at instance level,
    // they need to be explicitly destroyed here to avoid continued references to
    // the device we're destroying.
    for (auto &entry : swapchain_map_.snapshot()) {
        entry.second->Destroy();
    }
    swapchain_map_.clear();
    image_view_map_.clear();
    image_map_.clear();
    buffer_view_map_.clear();
    buffer_map_.clear();
    // Queues persist until device is destroyed
    for (auto &entry : queue_map_.snapshot()) {
        entry.second->Destroy();
    }
    queue_map_.clear();
}

void Device::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    if (!device) return;

    DestroyObjectMaps();
}

static void UpdateCmdBufLabelStack(const CommandBuffer &cb_state, Queue &queue_state) {
    if (queue_state.found_unbalanced_cmdbuf_label) return;
    for (const auto &command : cb_state.GetLabelCommands()) {
        if (command.begin) {
            queue_state.cmdbuf_label_stack.push_back(command.label_name);
        } else {
            if (queue_state.cmdbuf_label_stack.empty()) {
                queue_state.found_unbalanced_cmdbuf_label = true;
                return;
            }
            queue_state.last_closed_cmdbuf_label = queue_state.cmdbuf_label_stack.back();
            queue_state.cmdbuf_label_stack.pop_back();
        }
    }
}

void Device::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                      const RecordObject &record_obj) {
    auto queue_state = Get<Queue>(queue);

    std::vector<QueueSubmission> submissions;
    submissions.reserve(submitCount);
    if (submitCount == 0) {
        QueueSubmission submission(record_obj.location);
        submission.AddFence(Get<Fence>(fence));
        submissions.emplace_back(std::move(submission));
    }
    // Now process each individual submit
    for (uint32_t submit_i = 0; submit_i < submitCount; submit_i++) {
        Location submit_loc = record_obj.location.dot(Struct::VkSubmitInfo, Field::pSubmits, submit_i);
        QueueSubmission submission(submit_loc);
        const VkSubmitInfo *submit = &pSubmits[submit_i];
        auto *timeline_info = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(submit->pNext);
        for (uint32_t i = 0; i < submit->waitSemaphoreCount; ++i) {
            auto wait_semaphore = Get<Semaphore>(submit->pWaitSemaphores[i]);
            uint64_t value{0};
            if (wait_semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE && timeline_info && timeline_info->pWaitSemaphoreValues &&
                i < timeline_info->waitSemaphoreValueCount) {
                value = timeline_info->pWaitSemaphoreValues[i];
            }
            submission.AddWaitSemaphore(std::move(wait_semaphore), value);
        }
        for (uint32_t i = 0; i < submit->signalSemaphoreCount; ++i) {
            auto signal_semaphore = Get<Semaphore>(submit->pSignalSemaphores[i]);
            uint64_t value{0};
            if (signal_semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE && timeline_info && timeline_info->pSignalSemaphoreValues &&
                i < timeline_info->signalSemaphoreValueCount) {
                value = timeline_info->pSignalSemaphoreValues[i];
            }
            submission.AddSignalSemaphore(std::move(signal_semaphore), value);
        }

        const auto perf_submit = vku::FindStructInPNextChain<VkPerformanceQuerySubmitInfoKHR>(submit->pNext);
        submission.perf_submit_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        for (const VkCommandBuffer &cb : make_span(submit->pCommandBuffers, submit->commandBufferCount)) {
            if (auto cb_state = GetWrite<CommandBuffer>(cb)) {
                submission.AddCommandBuffer(cb_state, queue_state->cmdbuf_label_stack);
                UpdateCmdBufLabelStack(*cb_state, *queue_state);
            }
        }
        if (submit_i == (submitCount - 1) && fence != VK_NULL_HANDLE) {
            submission.AddFence(Get<Fence>(fence));
        }
        submissions.emplace_back(std::move(submission));
    }

    PreSubmitResult result = queue_state->PreSubmit(std::move(submissions));
    if (result.has_external_fence) {
        queue_state->NotifyAndWait(record_obj.location, result.submission_with_external_fence_seq);
    }
}

void Device::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                       const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    auto queue_state = Get<Queue>(queue);
    queue_state->PostSubmit();
}

void Device::PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                          const RecordObject &record_obj) {
    PreCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void Device::PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                       const RecordObject &record_obj) {
    auto queue_state = Get<Queue>(queue);
    std::vector<QueueSubmission> submissions;
    submissions.reserve(submitCount);
    if (submitCount == 0) {
        QueueSubmission submission(record_obj.location);
        submission.AddFence(Get<Fence>(fence));
        submissions.emplace_back(std::move(submission));
    }

    for (uint32_t submit_i = 0; submit_i < submitCount; submit_i++) {
        Location submit_loc = record_obj.location.dot(Struct::VkSubmitInfo2, Field::pSubmits, submit_i);
        QueueSubmission submission(submit_loc);
        const VkSubmitInfo2KHR &submit = pSubmits[submit_i];
        for (const VkSemaphoreSubmitInfo &wait_sem_info : make_span(submit.pWaitSemaphoreInfos, submit.waitSemaphoreInfoCount)) {
            auto wait_semaphore = Get<Semaphore>(wait_sem_info.semaphore);
            ASSERT_AND_CONTINUE(wait_semaphore);
            const uint64_t value = (wait_semaphore->type == VK_SEMAPHORE_TYPE_BINARY) ? 0 : wait_sem_info.value;
            submission.AddWaitSemaphore(std::move(wait_semaphore), value);
        }
        for (const VkSemaphoreSubmitInfo &sig_sem_info : make_span(submit.pSignalSemaphoreInfos, submit.signalSemaphoreInfoCount)) {
            submission.AddSignalSemaphore(Get<Semaphore>(sig_sem_info.semaphore), sig_sem_info.value);
        }
        const auto perf_submit = vku::FindStructInPNextChain<VkPerformanceQuerySubmitInfoKHR>(submit.pNext);
        submission.perf_submit_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        for (const VkCommandBufferSubmitInfo &cb_info : make_span(submit.pCommandBufferInfos, submit.commandBufferInfoCount)) {
            if (auto cb_state = GetWrite<CommandBuffer>(cb_info.commandBuffer)) {
                submission.AddCommandBuffer(cb_state, queue_state->cmdbuf_label_stack);
                UpdateCmdBufLabelStack(*cb_state, *queue_state);
            }
        }
        if (submit_i == (submitCount - 1)) {
            submission.AddFence(Get<Fence>(fence));
        }
        submissions.emplace_back(std::move(submission));
    }
    PreSubmitResult result = queue_state->PreSubmit(std::move(submissions));
    if (result.has_external_fence) {
        queue_state->NotifyAndWait(record_obj.location, result.submission_with_external_fence_seq);
    }
}

void Device::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                           const RecordObject &record_obj) {
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
}

void Device::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                        const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    auto queue_state = Get<Queue>(queue);
    queue_state->PostSubmit();
}

void Device::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) {
        return;
    }
    const auto &memory_type = phys_dev_mem_props.memoryTypes[pAllocateInfo->memoryTypeIndex];
    const auto &memory_heap = phys_dev_mem_props.memoryHeaps[memory_type.heapIndex];
    auto fake_address = fake_memory.Alloc(pAllocateInfo->allocationSize);

    std::optional<DedicatedBinding> dedicated_binding;
    if (const auto dedicated = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext)) {
        if (dedicated->buffer) {
            auto buffer_state = Get<Buffer>(dedicated->buffer);
            ASSERT_AND_RETURN(buffer_state);

            dedicated_binding.emplace(dedicated->buffer, buffer_state->create_info);
        } else if (dedicated->image) {
            auto image_state = Get<Image>(dedicated->image);
            ASSERT_AND_RETURN(image_state);

            dedicated_binding.emplace(dedicated->image, image_state->create_info);
        }
    }
    if (const auto import_memory_fd_info = vku::FindStructInPNextChain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext)) {
        // Successful import operation transfers POSIX handle ownership to the driver.
        // Stop tracking handle at this point. It can not be used for import operations anymore.
        // The map's erase is a no-op for externally created handles that are not tracked here.
        // NOTE: In contrast, the successful import does not transfer ownership of a Win32 handle.
        WriteLockGuard guard(fd_handle_map_lock_);
        fd_handle_map_.erase(import_memory_fd_info->fd);
    }
    Add(CreateDeviceMemoryState(*pMemory, pAllocateInfo, fake_address, memory_type, memory_heap, std::move(dedicated_binding),
                                physical_device_count));
    return;
}

void Device::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator,
                                     const RecordObject &record_obj) {
    if (auto mem_info = Get<DeviceMemory>(mem)) {
        fake_memory.Free(mem_info->fake_base_address);
    }
    {
        WriteLockGuard guard(fd_handle_map_lock_);
        for (auto it = fd_handle_map_.begin(); it != fd_handle_map_.end();) {
            if (it->second.device_memory == mem) {
                it = fd_handle_map_.erase(it);
                break;
            } else {
                ++it;
            }
        }
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {
        WriteLockGuard guard(win32_handle_map_lock_);
        for (auto it = win32_handle_map_.begin(); it != win32_handle_map_.end();) {
            if (it->second.device_memory == mem) {
                it = win32_handle_map_.erase(it);
                break;
            } else {
                ++it;
            }
        }
    }
#endif
    Destroy<DeviceMemory>(mem);
}

void Device::PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence,
                                          const RecordObject &record_obj) {
    auto queue_state = Get<Queue>(queue);

    std::vector<QueueSubmission> submissions;
    submissions.reserve(bindInfoCount);
    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; ++bind_idx) {
        const VkBindSparseInfo &bind_info = pBindInfo[bind_idx];
        // Track objects tied to memory
        for (uint32_t j = 0; j < bind_info.bufferBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pBufferBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pBufferBinds[j].pBinds[k];
                auto memory_state = Get<DeviceMemory>(sparse_binding.memory);
                if (auto buffer_state = Get<Buffer>(bind_info.pBufferBinds[j].buffer)) {
                    buffer_state->BindMemory(buffer_state.get(), memory_state, sparse_binding.memoryOffset,
                                             sparse_binding.resourceOffset, sparse_binding.size);
                }
            }
        }
        for (uint32_t j = 0; j < bind_info.imageOpaqueBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pImageOpaqueBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pImageOpaqueBinds[j].pBinds[k];
                auto memory_state = Get<DeviceMemory>(sparse_binding.memory);
                if (auto image_state = Get<Image>(bind_info.pImageOpaqueBinds[j].image)) {
                    // An Android special image cannot get VkSubresourceLayout until the image binds a memory.
                    // See: VUID-vkGetImageSubresourceLayout-image-09432
                    if (!image_state->fragment_encoder) {
                        image_state->fragment_encoder =
                            std::make_unique<const subresource_adapter::ImageRangeEncoder>(*image_state);
                    }
                    image_state->BindMemory(image_state.get(), memory_state, sparse_binding.memoryOffset,
                                            sparse_binding.resourceOffset, sparse_binding.size);
                }
            }
        }
        for (uint32_t j = 0; j < bind_info.imageBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pImageBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pImageBinds[j].pBinds[k];
                // TODO: This size is broken for non-opaque bindings, need to update to comprehend full sparse binding data
                VkDeviceSize size = sparse_binding.extent.depth * sparse_binding.extent.height * sparse_binding.extent.width * 4;
                VkDeviceSize offset = sparse_binding.offset.z * sparse_binding.offset.y * sparse_binding.offset.x * 4;
                auto memory_state = Get<DeviceMemory>(sparse_binding.memory);
                if (auto image_state = Get<Image>(bind_info.pImageBinds[j].image)) {
                    // An Android special image cannot get VkSubresourceLayout until the image binds a memory.
                    // See: VUID-vkGetImageSubresourceLayout-image-09432
                    if (!image_state->fragment_encoder) {
                        image_state->fragment_encoder =
                            std::make_unique<const subresource_adapter::ImageRangeEncoder>(*image_state);
                    }
                    image_state->BindMemory(image_state.get(), memory_state, sparse_binding.memoryOffset, offset, size);
                }
            }
        }
        auto* timeline_info = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(bind_info.pNext);
        Location submit_loc = record_obj.location.dot(Struct::VkBindSparseInfo, Field::pBindInfo, bind_idx);
        QueueSubmission submission(submit_loc);
        for (uint32_t i = 0; i < bind_info.waitSemaphoreCount; ++i) {
            auto wait_semaphore = Get<Semaphore>(bind_info.pWaitSemaphores[i]);
            uint64_t value{0};
            if (wait_semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE && timeline_info && timeline_info->pWaitSemaphoreValues &&
                i < timeline_info->waitSemaphoreValueCount) {
                value = timeline_info->pWaitSemaphoreValues[i];
            }
            submission.AddWaitSemaphore(std::move(wait_semaphore), value);
        }
        for (uint32_t i = 0; i < bind_info.signalSemaphoreCount; ++i) {
            auto signal_semaphore = Get<Semaphore>(bind_info.pSignalSemaphores[i]);
            uint64_t value{0};
            if (signal_semaphore->type == VK_SEMAPHORE_TYPE_TIMELINE && timeline_info && timeline_info->pSignalSemaphoreValues &&
                i < timeline_info->signalSemaphoreValueCount) {
                value = timeline_info->pSignalSemaphoreValues[i];
            }
            submission.AddSignalSemaphore(std::move(signal_semaphore), value);
        }
        if (bind_idx == (bindInfoCount - 1)) {
            submission.AddFence(Get<Fence>(fence));
        }
        submissions.emplace_back(std::move(submission));
    }

    PreSubmitResult result = queue_state->PreSubmit(std::move(submissions));
    if (result.has_external_fence) {
        queue_state->NotifyAndWait(record_obj.location, result.submission_with_external_fence_seq);
    }
}

void Device::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence,
                                           const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    auto queue_state = Get<Queue>(queue);
    queue_state->PostSubmit();
}

void Device::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<Semaphore>(*this, *pSemaphore, pCreateInfo));
}

void Device::RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                        VkSemaphoreImportFlags flags) {
    auto semaphore_state = Get<Semaphore>(semaphore);
    if (semaphore_state) {
        semaphore_state->Import(handle_type, flags);
    }
}

void Device::PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                          const RecordObject &record_obj) {
    auto semaphore_state = Get<Semaphore>(pSignalInfo->semaphore);
    if (semaphore_state) {
        auto value = pSignalInfo->value;  // const workaround
        semaphore_state->EnqueueSignal(SubmissionReference{}, value);
    }
}

void Device::PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                             const RecordObject &record_obj) {
    PreCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void Device::RecordMappedMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void **ppData) {
    if (auto mem_info = Get<DeviceMemory>(mem)) {
        mem_info->mapped_range.offset = offset;
        mem_info->mapped_range.size = size;
        mem_info->p_driver_data = *ppData;
    }
}

void Device::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                         uint64_t timeout, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    // When we know that all fences are complete we can clean/remove their CBs
    if ((VK_TRUE == waitAll) || (1 == fenceCount)) {
        for (uint32_t i = 0; i < fenceCount; i++) {
            if (auto fence_state = Get<Fence>(pFences[i])) {
                fence_state->NotifyAndWait(record_obj.location.dot(Field::pFences, i));
            }
        }
    }
    // NOTE : Alternate case not handled here is when some fences have completed. In
    //  this case for app to guarantee which fences completed it will have to call
    //  vkGetFenceStatus() at which point we'll clean/remove their CBs if complete.
}

void Device::PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                         const RecordObject &record_obj) {
    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
        if (auto semaphore_state = Get<Semaphore>(pWaitInfo->pSemaphores[i])) {
            auto value = pWaitInfo->pValues[i];  // const workaround
            semaphore_state->EnqueueWait(SubmissionReference{}, value);
        }
    }
}

void Device::PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                            const RecordObject &record_obj) {
    PreCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void Device::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    // Same logic as vkWaitForFences(). If some semaphores are not signaled, we will get their status when
    // the application calls vkGetSemaphoreCounterValue() on each of them.
    if ((pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT) == 0 || pWaitInfo->semaphoreCount == 1) {
        const Location wait_info_loc = record_obj.location.dot(Field::pWaitInfo);
        for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
            if (auto semaphore_state = Get<Semaphore>(pWaitInfo->pSemaphores[i])) {
                Location wait_value_loc = wait_info_loc.dot(Field::pValues, i);
                semaphore_state->RetireWait(nullptr, pWaitInfo->pValues[i], wait_value_loc);
            }
        }
    }
}

void Device::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                             const RecordObject &record_obj) {
    PostCallRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void Device::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                    const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (auto semaphore_state = Get<Semaphore>(semaphore)) {
        semaphore_state->RetireWait(nullptr, *pValue, record_obj.location);
    }
}

void Device::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                       const RecordObject &record_obj) {
    PostCallRecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void Device::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (auto fence_state = Get<Fence>(fence)) {
        fence_state->NotifyAndWait(record_obj.location);
    }
}

void Device::RecordGetDeviceQueueState(uint32_t queue_family_index, uint32_t queue_index, VkDeviceQueueCreateFlags flags,
                                       VkQueue queue) {
    if (Get<Queue>(queue) == nullptr) {
        uint32_t num_queue_families = 0;
        DispatchGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties_list(num_queue_families);
        DispatchGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families,
                                                                       queue_family_properties_list.data());

        Add(CreateQueue(queue, queue_family_index, queue_index, flags, queue_family_properties_list[queue_family_index]));
    }
}

void Device::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue,
                                          const RecordObject &record_obj) {
    RecordGetDeviceQueueState(queueFamilyIndex, queueIndex, {}, *pQueue);
}

void Device::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue,
                                           const RecordObject &record_obj) {
    RecordGetDeviceQueueState(pQueueInfo->queueFamilyIndex, pQueueInfo->queueIndex, pQueueInfo->flags, *pQueue);
}

void Device::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) {
    if (auto queue_state = Get<Queue>(queue)) {
        queue_state->NotifyAndWait(record_obj.location);
    }
}

void Device::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) {
    // Sort the queues by id to notify in deterministic order (queue creation order).
    // This is not needed for correctness, but gives deterministic behavior to certain
    // types of bugs in the queue thread.
    std::vector<std::shared_ptr<Queue>> queues;
    queues.reserve(queue_map_.size());
    for (const auto &entry : queue_map_.snapshot()) {
        queues.push_back(entry.second);
    }
    std::sort(queues.begin(), queues.end(), [](const auto &q1, const auto &q2) { return q1->GetId() < q2->GetId(); });

    // Notify all queues before waiting.
    // NotifyAndWait is not safe here. It deadlocks when a wait depends on the not yet issued notify.
    for (auto &queue : queues) {
        queue->Notify();
    }
    // All possible forward progress is initiated. Now it's safe to wait.
    for (auto &queue : queues) {
        queue->Wait(record_obj.location);
    }
}

void Device::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator,
                                       const RecordObject &record_obj) {
    Destroy<Fence>(fence);
}

void Device::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator,
                                           const RecordObject &record_obj) {
    Destroy<Semaphore>(semaphore);
}

void Device::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator,
                                       const RecordObject &record_obj) {
    Destroy<Event>(event);
}

void Device::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator,
                                           const RecordObject &record_obj) {
    Destroy<QueryPool>(queryPool);
}

void Device::UpdateBindBufferMemoryState(const VkBindBufferMemoryInfo &bind_info) {
    auto buffer_state = Get<Buffer>(bind_info.buffer);
    if (!buffer_state) return;

    // Track objects tied to memory
    if (auto memory_state = Get<DeviceMemory>(bind_info.memory)) {
        buffer_state->BindMemory(buffer_state.get(), memory_state, bind_info.memoryOffset, 0u, buffer_state->requirements.size);
    }
}

void Device::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    VkBindBufferMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.buffer = buffer;
    bind_info.memory = memory;
    bind_info.memoryOffset = memoryOffset;
    UpdateBindBufferMemoryState(bind_info);
}

void Device::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos,
                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) {
        // if bindInfoCount is 1, we know for sure if that single buffer was bound or not
        if (bindInfoCount > 1) {
            for (uint32_t i = 0; i < bindInfoCount; i++) {
                // If user passed in VkBindMemoryStatus, we can update which buffers are valid or not
                if (auto *bind_memory_status = vku::FindStructInPNextChain<VkBindMemoryStatus>(pBindInfos[i].pNext)) {
                    if (bind_memory_status->pResult && *bind_memory_status->pResult == VK_SUCCESS) {
                        UpdateBindBufferMemoryState(pBindInfos[i]);
                    }
                } else if (auto buffer_state = Get<Buffer>(pBindInfos[i].buffer)) {
                    buffer_state->indeterminate_state = true;
                }
            }
        }
    } else {
        for (uint32_t i = 0; i < bindInfoCount; i++) {
            UpdateBindBufferMemoryState(pBindInfos[i]);
        }
    }
}

void Device::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos,
                                                const RecordObject &record_obj) {
    PostCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void Device::RecordGetImageMemoryRequirementsState(VkImage image, const VkImageMemoryRequirementsInfo2 *pInfo) {
    const VkImagePlaneMemoryRequirementsInfo *plane_info =
        (pInfo == nullptr) ? nullptr : vku::FindStructInPNextChain<VkImagePlaneMemoryRequirementsInfo>(pInfo->pNext);
    if (auto image_state = Get<Image>(image)) {
        if (plane_info != nullptr) {
            // Multi-plane image
            if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_0_BIT) {
                image_state->memory_requirements_checked[0] = true;
            } else if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_1_BIT) {
                image_state->memory_requirements_checked[1] = true;
            } else if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_2_BIT) {
                image_state->memory_requirements_checked[2] = true;
            }
        } else if (!image_state->disjoint) {
            // Single Plane image
            image_state->memory_requirements_checked[0] = true;
        }
    }
}

void Device::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements,
                                                      const RecordObject &record_obj) {
    RecordGetImageMemoryRequirementsState(image, nullptr);
}

void Device::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                       VkMemoryRequirements2 *pMemoryRequirements, const RecordObject &record_obj) {
    RecordGetImageMemoryRequirementsState(pInfo->image, pInfo);
}

void Device::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                          VkMemoryRequirements2 *pMemoryRequirements,
                                                          const RecordObject &record_obj) {
    RecordGetImageMemoryRequirementsState(pInfo->image, pInfo);
}

void Device::PostCallRecordGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
                                                            VkSparseImageMemoryRequirements *pSparseMemoryRequirements,
                                                            const RecordObject &record_obj) {
    if (auto image_state = Get<Image>(image)) {
        image_state->get_sparse_reqs_called = true;
    }
}

void Device::PostCallRecordGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
                                                             uint32_t *pSparseMemoryRequirementCount,
                                                             VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements,
                                                             const RecordObject &record_obj) {
    if (auto image_state = Get<Image>(pInfo->image)) {
        image_state->get_sparse_reqs_called = true;
    }
}

void Device::PostCallRecordGetImageSparseMemoryRequirements2KHR(VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo,
                                                                uint32_t *pSparseMemoryRequirementCount,
                                                                VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements,
                                                                const RecordObject &record_obj) {
    PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements,
                                                    record_obj);
}

void Device::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks *pAllocator,
                                              const RecordObject &record_obj) {
    Destroy<ShaderModule>(shaderModule);
}

void Device::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader, const VkAllocationCallbacks *pAllocator,
                                           const RecordObject &record_obj) {
    Destroy<ShaderObject>(shader);
}

void Device::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator,
                                          const RecordObject &record_obj) {
    Destroy<Pipeline>(pipeline);
}

void Device::PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                             const VkShaderStageFlagBits *pStages, const VkShaderEXT *pShaders,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < stageCount; ++i) {
        ShaderObject *shader_object_state = nullptr;
        if (pShaders && pShaders[i] != VK_NULL_HANDLE) {
            shader_object_state = Get<ShaderObject>(pShaders[i]).get();
        }
        cb_state->BindShader(pStages[i], shader_object_state);
    }
}

void Device::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<PipelineLayout>(pipelineLayout);
}

void Device::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator,
                                         const RecordObject &record_obj) {
    if (!sampler) return;
    // Any bound cmd buffers are now invalid
    if (auto sampler_state = Get<Sampler>(sampler)) {
        if (sampler_state->create_info.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
            sampler_state->create_info.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
            custom_border_color_sampler_count--;
        }
    }
    Destroy<Sampler>(sampler);
}

void Device::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                     const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<DescriptorSetLayout>(descriptorSetLayout);
}

void Device::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<DescriptorPool>(descriptorPool);
}

void Device::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                             const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    if (auto pool = Get<CommandPool>(commandPool)) {
        pool->Free(commandBufferCount, pCommandBuffers);
    }
}

std::shared_ptr<CommandPool> Device::CreateCommandPoolState(VkCommandPool handle, const VkCommandPoolCreateInfo *create_info) {
    auto queue_flags = physical_device_state->queue_family_properties[create_info->queueFamilyIndex].queueFlags;
    return std::make_shared<CommandPool>(*this, handle, create_info, queue_flags);
}

void Device::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreateCommandPoolState(*pCommandPool, pCreateInfo));
}

void Device::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                           const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    uint32_t index_count = 0;
    uint32_t perf_queue_family_index = 0;
    uint32_t n_perf_pass = 0;
    bool has_cb = false, has_rb = false;
    if (pCreateInfo->queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        const auto *perf = vku::FindStructInPNextChain<VkQueryPoolPerformanceCreateInfoKHR>(pCreateInfo->pNext);
        perf_queue_family_index = perf->queueFamilyIndex;
        index_count = perf->counterIndexCount;

        const QueueFamilyPerfCounters &counters = *physical_device_state->perf_counters[perf_queue_family_index];
        for (uint32_t i = 0; i < perf->counterIndexCount; i++) {
            const auto &counter = counters.counters[perf->pCounterIndices[i]];
            switch (counter.scope) {
                case VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR:
                    has_cb = true;
                    break;
                case VK_PERFORMANCE_COUNTER_SCOPE_RENDER_PASS_KHR:
                    has_rb = true;
                    break;
                default:
                    break;
            }
        }

        DispatchGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physical_device_state->VkHandle(), perf, &n_perf_pass);
    }

    VkVideoEncodeFeedbackFlagsKHR video_encode_feedback_flags = 0;
    if (pCreateInfo->queryType == VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR) {
        const auto *feedback_info = vku::FindStructInPNextChain<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(pCreateInfo->pNext);
        if (feedback_info) {
            video_encode_feedback_flags = feedback_info->encodeFeedbackFlags;
        }
    }

    Add(std::make_shared<QueryPool>(
        *pQueryPool, pCreateInfo, index_count, perf_queue_family_index, n_perf_pass, has_cb, has_rb,
        video_profile_cache_.Get(physical_device, vku::FindStructInPNextChain<VkVideoProfileInfoKHR>(pCreateInfo->pNext)),
        video_encode_feedback_flags));
}

void Device::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator,
                                             const RecordObject &record_obj) {
    Destroy<CommandPool>(commandPool);
}

void Device::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    // Reset all of the CBs allocated from this pool
    if (auto pool = Get<CommandPool>(commandPool)) {
        pool->Reset(record_obj.location);
    }
}

void Device::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                       const RecordObject &record_obj) {
    for (uint32_t i = 0; i < fenceCount; ++i) {
        if (auto fence_state = Get<Fence>(pFences[i])) {
            fence_state->Reset();
        }
    }
}

void Device::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks *pAllocator,
                                             const RecordObject &record_obj) {
    Destroy<Framebuffer>(framebuffer);
}

void Device::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator,
                                            const RecordObject &record_obj) {
    Destroy<RenderPass>(renderPass);
}

void Device::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<Fence>(*this, *pFence, pCreateInfo));
}

std::shared_ptr<PipelineCache> Device::CreatePipelineCacheState(VkPipelineCache handle,
                                                                const VkPipelineCacheCreateInfo *create_info) const {
    return std::make_shared<PipelineCache>(handle, create_info);
}

void Device::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache,
                                               const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreatePipelineCacheState(*pPipelineCache, pCreateInfo));
}

void Device::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                               const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<PipelineCache>(pipelineCache);
}

std::shared_ptr<Pipeline> Device::CreateGraphicsPipelineState(
    const VkGraphicsPipelineCreateInfo *create_info, std::shared_ptr<const PipelineCache> pipeline_cache,
    std::shared_ptr<const RenderPass> &&render_pass, std::shared_ptr<const PipelineLayout> &&layout,
    spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]) const {
    return std::make_shared<Pipeline>(*this, create_info, std::move(pipeline_cache), std::move(render_pass), std::move(layout),
                                      stateless_data);
}

bool Device::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                    const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                    chassis::CreateGraphicsPipelines &chassis_state) const {
    bool skip = false;
    // Set up the state that CoreChecks, gpu_validation and later StateTracker Record will use.
    pipeline_states.reserve(count);
    auto pipeline_cache = Get<PipelineCache>(pipelineCache);
    for (uint32_t i = 0; i < count; i++) {
        const auto &create_info = pCreateInfos[i];
        auto layout_state = Get<PipelineLayout>(create_info.layout);
        std::shared_ptr<const RenderPass> render_pass;

        if (pCreateInfos[i].renderPass != VK_NULL_HANDLE) {
            render_pass = Get<RenderPass>(create_info.renderPass);
        } else if (enabled_features.dynamicRendering) {
            auto dynamic_rendering = vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(create_info.pNext);
            const bool rasterization_enabled = Pipeline::EnablesRasterizationStates(*this, create_info);
            const bool has_fragment_output_state =
                Pipeline::ContainsSubState(this, create_info, VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT);
            render_pass = std::make_shared<RenderPass>(dynamic_rendering, rasterization_enabled && has_fragment_output_state);
        } else {
            const bool is_graphics_lib = GetGraphicsLibType(create_info) != static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
            const bool has_link_info = vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext) != nullptr;
            if (!is_graphics_lib && !has_link_info) {
                skip = true;
            }
        }

        pipeline_states.push_back(CreateGraphicsPipelineState(&create_info, pipeline_cache, std::move(render_pass),
                                                              std::move(layout_state), chassis_state.stateless_data));
    }
    return skip;
}

void Device::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                   chassis::CreateGraphicsPipelines &chassis_state) {
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            pipeline_states[i]->SetHandle(pPipelines[i]);
            Add(std::move(pipeline_states[i]));
        }
    }
    pipeline_states.clear();
}

std::shared_ptr<Pipeline> Device::CreateComputePipelineState(const VkComputePipelineCreateInfo *create_info,
                                                             std::shared_ptr<const PipelineCache> pipeline_cache,
                                                             std::shared_ptr<const PipelineLayout> &&layout,
                                                             spirv::StatelessData *stateless_data) const {
    return std::make_shared<Pipeline>(*this, create_info, std::move(pipeline_cache), std::move(layout), stateless_data);
}

bool Device::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                   const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                   chassis::CreateComputePipelines &chassis_state) const {
    pipeline_states.reserve(count);
    auto pipeline_cache = Get<PipelineCache>(pipelineCache);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        pipeline_states.push_back(CreateComputePipelineState(
            &pCreateInfos[i], pipeline_cache, Get<PipelineLayout>(pCreateInfos[i].layout), &chassis_state.stateless_data));
    }
    return false;
}

void Device::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                  const VkComputePipelineCreateInfo *pCreateInfos,
                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                  const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                  chassis::CreateComputePipelines &chassis_state) {
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            pipeline_states[i]->SetHandle(pPipelines[i]);
            Add(std::move(pipeline_states[i]));
        }
    }
    pipeline_states.clear();
}

// TODO - Add tests and pass down StatelessData
std::shared_ptr<Pipeline> Device::CreateRayTracingPipelineState(const VkRayTracingPipelineCreateInfoNV *create_info,
                                                                std::shared_ptr<const PipelineCache> pipeline_cache,
                                                                std::shared_ptr<const PipelineLayout> &&layout,
                                                                spirv::StatelessData *stateless_data) const {
    return std::make_shared<Pipeline>(*this, create_info, std::move(pipeline_cache), std::move(layout), stateless_data);
}

bool Device::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                        chassis::CreateRayTracingPipelinesNV &chassis_state) const {
    pipeline_states.reserve(count);
    auto pipeline_cache = Get<PipelineCache>(pipelineCache);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        pipeline_states.push_back(
            CreateRayTracingPipelineState(&pCreateInfos[i], pipeline_cache, Get<PipelineLayout>(pCreateInfos[i].layout), nullptr));
    }
    return false;
}

void Device::PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                       chassis::CreateRayTracingPipelinesNV &chassis_state) {
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            pipeline_states[i]->SetHandle(pPipelines[i]);
            Add(std::move(pipeline_states[i]));
        }
    }
    pipeline_states.clear();
}

// TODO - Add tests and pass down StatelessData
std::shared_ptr<Pipeline> Device::CreateRayTracingPipelineState(const VkRayTracingPipelineCreateInfoKHR *create_info,
                                                                std::shared_ptr<const PipelineCache> pipeline_cache,
                                                                std::shared_ptr<const PipelineLayout> &&layout,
                                                                spirv::StatelessData *stateless_data) const {
    return std::make_shared<Pipeline>(*this, create_info, std::move(pipeline_cache), std::move(layout), stateless_data);
}

bool Device::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                         VkPipelineCache pipelineCache, uint32_t count,
                                                         const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                         const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                         const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                         chassis::CreateRayTracingPipelinesKHR &chassis_state) const {
    pipeline_states.reserve(count);
    auto pipeline_cache = Get<PipelineCache>(pipelineCache);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        pipeline_states.push_back(
            CreateRayTracingPipelineState(&pCreateInfos[i], pipeline_cache, Get<PipelineLayout>(pCreateInfos[i].layout), nullptr));
    }
    return false;
}

void Device::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        const RecordObject &record_obj, PipelineStates &pipeline_states,
                                                        std::shared_ptr<chassis::CreateRayTracingPipelinesKHR> chassis_state) {
    const bool is_operation_deferred = (deferredOperation != VK_NULL_HANDLE && record_obj.result == VK_OPERATION_DEFERRED_KHR);
    // This API may create pipelines regardless of the return value

    if (!is_operation_deferred) {
        for (uint32_t i = 0; i < count; i++) {
            if (pPipelines[i] != VK_NULL_HANDLE) {
                pipeline_states[i]->SetHandle(pPipelines[i]);
                Add(std::move(pipeline_states[i]));
            }
        }
    } else {
        // Deferred creation: pipelines will be considered created once the defferedOperation object
        // signals it, via usage of vkDeferredOperationJoinKHR and then vkGetDeferredOperationResultKHR
        // Hence pipeline state tracking needs to be deferred to the corresponding call to
        // vkGetDeferredOperationResultKHR => Store the deferred logic to do that in
        // `deferred_operation_post_check`.

        if (dispatch_device_->wrap_handles) {
            deferredOperation = dispatch_device_->Unwrap(deferredOperation);
        }
        std::vector<std::function<void(const std::vector<VkPipeline> &)>> cleanup_fn;
        auto find_res = dispatch_device_->deferred_operation_post_check.pop(deferredOperation);
        if (find_res->first) {
            cleanup_fn = std::move(find_res->second);
        }
        // Mutable lambda because we want to move the shared pointer contained in the copied vector
        cleanup_fn.emplace_back([this, chassis_state, pipeline_states](const std::vector<VkPipeline> &pipelines) mutable {
            // Just need to capture chassis state to maintain pipeline creations parameters alive, see
            // https://vkdoc.net/chapters/deferred-host-operations#deferred-host-operations-requesting
            (void)chassis_state;
            for (size_t i = 0; i < pipeline_states.size(); ++i) {
                pipeline_states[i]->SetHandle(pipelines[i]);
                this->Add(std::move(pipeline_states[i]));
            }
        });
        dispatch_device_->deferred_operation_post_check.insert(deferredOperation, cleanup_fn);
    }
}

std::shared_ptr<Sampler> Device::CreateSamplerState(VkSampler handle, const VkSamplerCreateInfo *create_info) {
    return std::make_shared<Sampler>(handle, create_info);
}

void Device::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                         const RecordObject &record_obj) {
    Add(CreateSamplerState(*pSampler, pCreateInfo));
    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        custom_border_color_sampler_count++;
    }
}

void Device::PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout,
                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<DescriptorSetLayout>(pCreateInfo, *pSetLayout));
}

void Device::PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                         VkDeviceSize *pLayoutSizeInBytes, const RecordObject &record_obj) {
    if (auto descriptor_set_layout = Get<DescriptorSetLayout>(layout)) {
        descriptor_set_layout->SetLayoutSizeInBytes(pLayoutSizeInBytes);
    }
}

void Device::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout,
                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<PipelineLayout>(*this, *pPipelineLayout, pCreateInfo));
}

std::shared_ptr<DescriptorPool> Device::CreateDescriptorPoolState(VkDescriptorPool handle,
                                                                  const VkDescriptorPoolCreateInfo *create_info) {
    return std::make_shared<DescriptorPool>(*this, handle, create_info);
}

std::shared_ptr<DescriptorSet> Device::CreateDescriptorSet(VkDescriptorSet handle, DescriptorPool *pool,
                                                           const std::shared_ptr<DescriptorSetLayout const> &layout,
                                                           uint32_t variable_count) {
    return std::make_shared<DescriptorSet>(handle, pool, layout, variable_count, this);
}

void Device::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool,
                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreateDescriptorPoolState(*pDescriptorPool, pCreateInfo));
}

void Device::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags,
                                               const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (auto ds_pool_state = Get<DescriptorPool>(descriptorPool)) {
        ds_pool_state->Reset();
    }
}

bool Device::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                   VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj,
                                                   AllocateDescriptorSetsData &ads_state) const {
    // Always update common data
    UpdateAllocateDescriptorSetsData(pAllocateInfo, ads_state);

    return false;
}

// Allocation state was good and call down chain was made so update state based on allocating descriptor sets
void Device::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                  VkDescriptorSet *pDescriptorSets, const RecordObject &record_obj,
                                                  AllocateDescriptorSetsData &ads_state) {
    if (VK_SUCCESS != record_obj.result) return;
    // All the updates are contained in a single vvl function
    if (auto ds_pool_state = Get<DescriptorPool>(pAllocateInfo->descriptorPool)) {
        ds_pool_state->Allocate(pAllocateInfo, pDescriptorSets, ads_state);
    }
}

void Device::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                             const VkDescriptorSet *pDescriptorSets, const RecordObject &record_obj) {
    if (auto ds_pool_state = Get<DescriptorPool>(descriptorPool)) {
        ds_pool_state->Free(count, pDescriptorSets);
    }
}

void Device::PerformUpdateDescriptorSets(uint32_t write_count, const VkWriteDescriptorSet *p_wds, uint32_t copy_count,
                                         const VkCopyDescriptorSet *p_cds) {
    // Write updates first
    uint32_t i = 0;
    for (i = 0; i < write_count; ++i) {
        auto dest_set = p_wds[i].dstSet;
        if (auto set_node = Get<DescriptorSet>(dest_set)) {
            set_node->PerformWriteUpdate(p_wds[i]);
        }
    }
    // Now copy updates
    for (i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = Get<DescriptorSet>(src_set);
        auto dst_node = Get<DescriptorSet>(dst_set);
        if (src_node && dst_node) {
            dst_node->PerformCopyUpdate(p_cds[i], *src_node);
        }
    }
}

void Device::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                               const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                               const VkCopyDescriptorSet *pDescriptorCopies, const RecordObject &record_obj) {
    PerformUpdateDescriptorSets(descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

void Device::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                  VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (auto pool = Get<CommandPool>(pAllocateInfo->commandPool)) {
        pool->Allocate(pAllocateInfo, pCommandBuffers);
    }
}

void Device::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->Begin(pBeginInfo);
}

void Device::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->End(record_obj.result);
}

void Device::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                              const RecordObject &record_obj) {
    if (VK_SUCCESS == record_obj.result) {
        auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
        if (cb_state) {
            cb_state->Reset(record_obj.location);
        }
    }
}

// Validation cache:
// CV is the bottommost implementor of this extension. Don't pass calls down.

void Device::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline,
                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);

    auto pipe_state = Get<Pipeline>(pipeline);
    ASSERT_AND_RETURN(pipe_state);
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == pipelineBindPoint) {
        const auto *viewport_state = pipe_state->ViewportState();

        cb_state->dynamic_state_status.pipeline.reset();

        // Make a copy and then xor the new change
        // This gives us which state has been invalidated, allows us to save time for most cases where nothing changes
        CBDynamicFlags invalidated_state = cb_state->dynamic_state_status.cb;

        // Spec: "[dynamic state] made invalid by another pipeline bind with that state specified as static"
        // So unset the bitmask for the command buffer lifetime tracking
        cb_state->dynamic_state_status.cb &= pipe_state->dynamic_state;

        invalidated_state ^= cb_state->dynamic_state_status.cb;
        if (invalidated_state.any()) {
            // Reset dynamic state values
            cb_state->dynamic_state_value.reset(invalidated_state);

            for (int index = 1; index < CB_DYNAMIC_STATE_STATUS_NUM; ++index) {
                CBDynamicState status = static_cast<CBDynamicState>(index);
                if (invalidated_state[status]) {
                    cb_state->invalidated_state_pipe[index] = pipeline;
                }
            }
        }

        if (!pipe_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT) &&
            !pipe_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE) && pipe_state->vertex_input_state) {
            for (const auto &[binding_index, binding_state] : pipe_state->vertex_input_state->bindings) {
                cb_state->current_vertex_buffer_binding_info[binding_index].stride = binding_state.desc.stride;
            }
        }

        // Used to calculate CommandBuffer::usedViewportScissorCount upon draw command with this graphics pipeline.
        // If rasterization disabled (no viewport/scissors used), or the actual number of viewports/scissors is dynamic (unknown at
        // this time), then these are set to 0 to disable this checking.
        const auto has_dynamic_viewport_count = pipe_state->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        const auto has_dynamic_scissor_count = pipe_state->IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        cb_state->pipelineStaticViewportCount = (has_dynamic_viewport_count || !viewport_state) ? 0 : viewport_state->viewportCount;
        cb_state->pipelineStaticScissorCount = (has_dynamic_scissor_count || !viewport_state) ? 0 : viewport_state->scissorCount;

        // Trash dynamic viewport/scissor state if pipeline defines static state and enabled rasterization.
        // akeley98 NOTE: There's a bit of an ambiguity in the spec, whether binding such a pipeline overwrites
        // the entire viewport (scissor) array, or only the subsection defined by the viewport (scissor) count.
        // I am taking the latter interpretation based on the implementation details of NVIDIA's Vulkan driver.
        if (!has_dynamic_viewport_count) {
            cb_state->trashedViewportCount = true;
            if (viewport_state && (!pipe_state->IsDynamic(CB_DYNAMIC_STATE_VIEWPORT))) {
                cb_state->trashedViewportMask |= (1u << viewport_state->viewportCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
        if (!has_dynamic_scissor_count) {
            cb_state->trashedScissorCount = true;
            if (viewport_state && (!pipe_state->IsDynamic(CB_DYNAMIC_STATE_SCISSOR))) {
                cb_state->trashedScissorMask |= (1u << viewport_state->scissorCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
    } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        cb_state->dynamic_state_status.rtx_stack_size_pipeline = false;
        if (!pipe_state->IsDynamic(CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR)) {
            cb_state->dynamic_state_status.rtx_stack_size_cb = false;  // invalidated
        }
    }

    cb_state->BindPipeline(ConvertToLvlBindPoint(pipelineBindPoint), pipe_state.get());
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(pipe_state);
    }
}

void Device::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                           VkPipeline pipeline, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto pipe_state = Get<Pipeline>(pipeline);
    ASSERT_AND_RETURN(pipe_state);

    if (enabled_features.variableMultisampleRate == VK_FALSE) {
        if (const auto *multisample_state = pipe_state->MultisampleState(); multisample_state) {
            if (const auto &render_pass = cb_state->active_render_pass) {
                const uint32_t subpass = cb_state->GetActiveSubpass();
                // if render pass uses no attachment, all bound pipelines in the same subpass must have the same
                // pMultisampleState->rasterizationSamples. To check that, record pMultisampleState->rasterizationSamples of the
                // first bound pipeline.
                if (render_pass->UsesNoAttachment(subpass)) {
                    if (std::optional<VkSampleCountFlagBits> subpass_rasterization_samples =
                            cb_state->GetActiveSubpassRasterizationSampleCount();
                        !subpass_rasterization_samples) {
                        cb_state->SetActiveSubpassRasterizationSampleCount(multisample_state->rasterizationSamples);
                    }
                }
            }
        }
    }

    cb_state->dirtyStaticState = false;
}

void Device::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                          const VkViewport *pViewports, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT);
    uint32_t bits = ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->viewportMask |= bits;
    cb_state->trashedViewportMask &= ~bits;
    if (cb_state->dynamic_state_value.viewports.size() < firstViewport + viewportCount) {
        cb_state->dynamic_state_value.viewports.resize(firstViewport + viewportCount);
    }
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamic_state_value.viewports[firstViewport + i] = pViewports[i];
    }
}

void Device::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                    uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors,
                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV);
    // TODO: We don't have VUIDs for validating that all exclusive scissors have been set.
    // cb_state->exclusiveScissorMask |= ((1u << exclusiveScissorCount) - 1u) << firstExclusiveScissor;

    cb_state->dynamic_state_value.exclusive_scissor_first = firstExclusiveScissor;
    cb_state->dynamic_state_value.exclusive_scissor_count = exclusiveScissorCount;
    cb_state->dynamic_state_value.exclusive_scissors.resize(firstExclusiveScissor + exclusiveScissorCount);
    for (size_t i = 0; i < exclusiveScissorCount; ++i) {
        cb_state->dynamic_state_value.exclusive_scissors[firstExclusiveScissor + i] = pExclusiveScissors[i];
    }
}

void Device::PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                          uint32_t exclusiveScissorCount, const VkBool32 *pExclusiveScissorEnables,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV);

    cb_state->dynamic_state_value.exclusive_scissor_enable_first = firstExclusiveScissor;
    cb_state->dynamic_state_value.exclusive_scissor_enable_count = exclusiveScissorCount;
    cb_state->dynamic_state_value.exclusive_scissor_enables.resize(firstExclusiveScissor + exclusiveScissorCount);
    for (size_t i = 0; i < exclusiveScissorCount; ++i) {
        cb_state->dynamic_state_value.exclusive_scissor_enables[firstExclusiveScissor + i] = pExclusiveScissorEnables[i];
    }
}

void Device::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout,
                                                    const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);

    if (imageView != VK_NULL_HANDLE) {
        auto view_state = Get<ImageView>(imageView);
        cb_state->AddChild(view_state);
    }
}

void Device::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                              uint32_t viewportCount,
                                                              const VkShadingRatePaletteNV *pShadingRatePalettes,
                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV);
    // TODO: We don't have VUIDs for validating that all shading rate palettes have been set.
    // cb_state->shadingRatePaletteMask |= ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->dynamic_state_value.shading_rate_palette_count = viewportCount;
}

std::shared_ptr<AccelerationStructureNV> Device::CreateAccelerationStructureState(
    VkAccelerationStructureNV handle, const VkAccelerationStructureCreateInfoNV *create_info) {
    return std::make_shared<AccelerationStructureNV>(device, handle, create_info);
}

void Device::PostCallRecordCreateAccelerationStructureNV(VkDevice device, const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkAccelerationStructureNV *pAccelerationStructure,
                                                         const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreateAccelerationStructureState(*pAccelerationStructure, pCreateInfo));
}

std::shared_ptr<AccelerationStructureKHR> Device::CreateAccelerationStructureState(
    VkAccelerationStructureKHR handle, const VkAccelerationStructureCreateInfoKHR *create_info,
    std::shared_ptr<Buffer> &&buf_state) {
    return std::make_shared<AccelerationStructureKHR>(handle, create_info, std::move(buf_state));
}

void Device::PostCallRecordCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkAccelerationStructureKHR *pAccelerationStructure,
                                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto buffer_state = Get<Buffer>(pCreateInfo->buffer);
    Add(CreateAccelerationStructureState(*pAccelerationStructure, pCreateInfo, std::move(buffer_state)));
}

void Device::PostCallRecordBuildAccelerationStructuresKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                          uint32_t infoCount,
                                                          const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                          const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos,
                                                          const RecordObject &record_obj) {
    for (uint32_t i = 0; i < infoCount; ++i) {
        if (auto dst_as_state = Get<AccelerationStructureKHR>(pInfos[i].dstAccelerationStructure)) {
            dst_as_state->Build(&pInfos[i], true, *ppBuildRangeInfos);
        }
    }
}

// helper method for device side acceleration structure builds
void Device::RecordDeviceAccelerationStructureBuildInfo(CommandBuffer &cb_state,
                                                        const VkAccelerationStructureBuildGeometryInfoKHR &info) {
    auto dst_as_state = Get<AccelerationStructureKHR>(info.dstAccelerationStructure);
    if (dst_as_state) {
        dst_as_state->Build(&info, false, nullptr);
    }
    if (disabled[command_buffer_state]) {
        return;
    }
    if (dst_as_state) {
        cb_state.AddChild(dst_as_state);
    }
    auto src_as_state = Get<AccelerationStructureKHR>(info.srcAccelerationStructure);
    if (src_as_state) {
        cb_state.AddChild(src_as_state);
    }

    // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
    // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a command
    // buffer
}

void Device::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);

    cb_state->RecordCmd(record_obj.location.function);
    for (const auto [i, info] : enumerate(pInfos, infoCount)) {
        RecordDeviceAccelerationStructureBuildInfo(*cb_state, info);
        if (auto dst_as_state = Get<AccelerationStructureKHR>(info.dstAccelerationStructure)) {
            dst_as_state->UpdateBuildRangeInfos(ppBuildRangeInfos[i], info.geometryCount);
        }
    }
    cb_state->has_build_as_cmd = true;
}

void Device::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                                                     const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
                                                                     const VkDeviceAddress *pIndirectDeviceAddresses,
                                                                     const uint32_t *pIndirectStrides,
                                                                     const uint32_t *const *ppMaxPrimitiveCounts,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);

    cb_state->RecordCmd(record_obj.location.function);
    for (uint32_t i = 0; i < infoCount; i++) {
        RecordDeviceAccelerationStructureBuildInfo(*cb_state, pInfos[i]);

        // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
        // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a command
        // buffer
    }
    cb_state->has_build_as_cmd = true;
}

void Device::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2 *pMemoryRequirements,
    const RecordObject &record_obj) {
    if (auto as_state = Get<AccelerationStructureNV>(pInfo->accelerationStructure)) {
        if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV) {
            as_state->memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV) {
            as_state->build_scratch_memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV) {
            as_state->update_scratch_memory_requirements_checked = true;
        }
    }
}

void Device::PostCallRecordBindAccelerationStructureMemoryNV(VkDevice device, uint32_t bindInfoCount,
                                                             const VkBindAccelerationStructureMemoryInfoNV *pBindInfos,
                                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];

        if (auto as_state = Get<AccelerationStructureNV>(info.accelerationStructure)) {
            // Track objects tied to memory
            if (auto memory_state = Get<DeviceMemory>(info.memory)) {
                as_state->BindMemory(as_state.get(), memory_state, info.memoryOffset, 0u, as_state->memory_requirements.size);
            }

            // GPU validation of top level acceleration structure building needs acceleration structure handles.
            // XXX TODO: Query device address for KHR extension
            if (enabled[gpu_validation]) {
                DispatchGetAccelerationStructureHandleNV(device, info.accelerationStructure, 8, &as_state->opaque_handle);
            }
        }
    }
}

void Device::PostCallRecordCmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                           const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData,
                                                           VkDeviceSize instanceOffset, VkBool32 update,
                                                           VkAccelerationStructureNV dst, VkAccelerationStructureNV src,
                                                           VkBuffer scratch, VkDeviceSize scratchOffset,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (!cb_state) {
        return;
    }
    cb_state->RecordCmd(record_obj.location.function);

    auto dst_as_state = Get<AccelerationStructureNV>(dst);
    if (dst_as_state) {
        dst_as_state->Build(pInfo);
        if (!disabled[command_buffer_state]) {
            cb_state->AddChild(dst_as_state);
        }
    }
    if (!disabled[command_buffer_state]) {
        if (auto src_as_state = Get<AccelerationStructureNV>(src)) {
            cb_state->AddChild(src_as_state);
        }
        if (auto instance_buffer = Get<Buffer>(instanceData)) {
            cb_state->AddChild(instance_buffer);
        }
        if (auto scratch_buffer = Get<Buffer>(scratch)) {
            cb_state->AddChild(scratch_buffer);
        }

        for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
            const auto &geom = pInfo->pGeometries[i];

            if (auto vertex_buffer = Get<Buffer>(geom.geometry.triangles.vertexData)) {
                cb_state->AddChild(vertex_buffer);
            }
            if (auto index_buffer = Get<Buffer>(geom.geometry.triangles.indexData)) {
                cb_state->AddChild(index_buffer);
            }
            if (auto transform_buffer = Get<Buffer>(geom.geometry.triangles.transformData)) {
                cb_state->AddChild(transform_buffer);
            }
            if (auto aabb_buffer = Get<Buffer>(geom.geometry.aabbs.aabbData)) {
                cb_state->AddChild(aabb_buffer);
            }
        }
    }
    cb_state->has_build_as_cmd = true;
}

void Device::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst,
                                                          VkAccelerationStructureNV src, VkCopyAccelerationStructureModeNV mode,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (cb_state) {
        auto src_as_state = Get<AccelerationStructureNV>(src);
        auto dst_as_state = Get<AccelerationStructureNV>(dst);
        if (dst_as_state && src_as_state) {
            if (!disabled[command_buffer_state]) {
                cb_state->RecordTransferCmd(record_obj.location.function, src_as_state, dst_as_state);
            }
            dst_as_state->built = true;
            dst_as_state->build_info = src_as_state->build_info;
        }
    }
}

void Device::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<AccelerationStructureKHR>(accelerationStructure);
}

void Device::PreCallRecordDestroyAccelerationStructureNV(VkDevice device, VkAccelerationStructureNV accelerationStructure,
                                                         const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<AccelerationStructureNV>(accelerationStructure);
}

void Device::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                    const VkViewportWScalingNV *pViewportWScalings,
                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
    cb_state->dynamic_state_value.viewport_w_scaling_first = firstViewport;
    cb_state->dynamic_state_value.viewport_w_scaling_count = viewportCount;
    cb_state->dynamic_state_value.viewport_w_scalings.resize(viewportCount);
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamic_state_value.viewport_w_scalings[i] = pViewportWScalings[i];
    }
}

void Device::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_WIDTH);
}

void Device::PostCallRecordCmdSetLineStipple(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_STIPPLE);
}

void Device::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern, const RecordObject &record_obj) {
    PostCallRecordCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, record_obj);
}

void Device::PostCallRecordCmdSetLineStippleKHR(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                uint16_t lineStipplePattern, const RecordObject &record_obj) {
    PostCallRecordCmdSetLineStipple(commandBuffer, lineStippleFactor, lineStipplePattern, record_obj);
}

void Device::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                           float depthBiasSlopeFactor, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_BIAS);
}

void Device::PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer, const VkDepthBiasInfoEXT *pDepthBiasInfo,
                                               const RecordObject &record_obj) {
    PostCallRecordCmdSetDepthBias(commandBuffer, pDepthBiasInfo->depthBiasConstantFactor, pDepthBiasInfo->depthBiasClamp,
                                  pDepthBiasInfo->depthBiasSlopeFactor, record_obj);
}

void Device::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                         const VkRect2D *pScissors, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SCISSOR);
    uint32_t bits = ((1u << scissorCount) - 1u) << firstScissor;
    cb_state->scissorMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
}

void Device::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_BLEND_CONSTANTS);
}

void Device::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_BOUNDS);
}

void Device::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                    uint32_t compareMask, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
}

void Device::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask,
                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    if (faceMask == VK_STENCIL_FACE_FRONT_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.write_mask_front = writeMask;
    }
    if (faceMask == VK_STENCIL_FACE_BACK_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.write_mask_back = writeMask;
    }
}

void Device::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference,
                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_REFERENCE);
}

// Update the bound state for the bind point, including the effects of incompatible pipeline layouts
void Device::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                const uint32_t *pDynamicOffsets, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto pipeline_layout = Get<PipelineLayout>(layout);
    if (!cb_state || !pipeline_layout) {
        return;
    }
    cb_state->RecordCmd(record_obj.location.function);

    std::shared_ptr<DescriptorSet> no_push_desc;

    cb_state->UpdateLastBoundDescriptorSets(pipelineBindPoint, *pipeline_layout, record_obj.location.function, firstSet, setCount,
                                            pDescriptorSets, no_push_desc, dynamicOffsetCount, pDynamicOffsets);
}

void Device::PreCallRecordCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                 const VkBindDescriptorSetsInfo *pBindDescriptorSetsInfo,
                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto pipeline_layout = Get<PipelineLayout>(pBindDescriptorSetsInfo->layout);
    ASSERT_AND_RETURN(cb_state && pipeline_layout);

    cb_state->RecordCmd(record_obj.location.function);

    std::shared_ptr<DescriptorSet> no_push_desc;

    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        cb_state->UpdateLastBoundDescriptorSets(
            VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_layout, record_obj.location.function, pBindDescriptorSetsInfo->firstSet,
            pBindDescriptorSetsInfo->descriptorSetCount, pBindDescriptorSetsInfo->pDescriptorSets, no_push_desc,
            pBindDescriptorSetsInfo->dynamicOffsetCount, pBindDescriptorSetsInfo->pDynamicOffsets);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        cb_state->UpdateLastBoundDescriptorSets(
            VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline_layout, record_obj.location.function, pBindDescriptorSetsInfo->firstSet,
            pBindDescriptorSetsInfo->descriptorSetCount, pBindDescriptorSetsInfo->pDescriptorSets, no_push_desc,
            pBindDescriptorSetsInfo->dynamicOffsetCount, pBindDescriptorSetsInfo->pDynamicOffsets);
    }
    if (IsStageInPipelineBindPoint(pBindDescriptorSetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        cb_state->UpdateLastBoundDescriptorSets(
            VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline_layout, record_obj.location.function,
            pBindDescriptorSetsInfo->firstSet, pBindDescriptorSetsInfo->descriptorSetCount,
            pBindDescriptorSetsInfo->pDescriptorSets, no_push_desc, pBindDescriptorSetsInfo->dynamicOffsetCount,
            pBindDescriptorSetsInfo->pDynamicOffsets);
    }
}

void Device::PreCallRecordCmdBindDescriptorSets2KHR(VkCommandBuffer commandBuffer,
                                                    const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
                                                    const RecordObject &record_obj) {
    PreCallRecordCmdBindDescriptorSets2(commandBuffer, pBindDescriptorSetsInfo, record_obj);
}

void Device::PreCallRecordCmdPushDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                               VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                               const VkWriteDescriptorSet *pDescriptorWrites, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto pipeline_layout = Get<PipelineLayout>(layout);
    ASSERT_AND_RETURN(pipeline_layout);
    cb_state->PushDescriptorSetState(pipelineBindPoint, *pipeline_layout, record_obj.location.function, set, descriptorWriteCount,
                                     pDescriptorWrites);
}

void Device::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                  const VkWriteDescriptorSet *pDescriptorWrites, const RecordObject &record_obj) {
    PreCallRecordCmdPushDescriptorSet(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites,
                                      record_obj);
}

void Device::PreCallRecordCmdPushDescriptorSet2(VkCommandBuffer commandBuffer,
                                                const VkPushDescriptorSetInfo *pPushDescriptorSetInfo,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto pipeline_layout = Get<PipelineLayout>(pPushDescriptorSetInfo->layout);
    ASSERT_AND_RETURN(pipeline_layout);
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        cb_state->PushDescriptorSetState(VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_layout, record_obj.location.function,
                                         pPushDescriptorSetInfo->set, pPushDescriptorSetInfo->descriptorWriteCount,
                                         pPushDescriptorSetInfo->pDescriptorWrites);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        cb_state->PushDescriptorSetState(VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline_layout, record_obj.location.function,
                                         pPushDescriptorSetInfo->set, pPushDescriptorSetInfo->descriptorWriteCount,
                                         pPushDescriptorSetInfo->pDescriptorWrites);
    }
    if (IsStageInPipelineBindPoint(pPushDescriptorSetInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        cb_state->PushDescriptorSetState(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline_layout, record_obj.location.function,
                                         pPushDescriptorSetInfo->set, pPushDescriptorSetInfo->descriptorWriteCount,
                                         pPushDescriptorSetInfo->pDescriptorWrites);
    }
}

void Device::PreCallRecordCmdPushDescriptorSet2KHR(VkCommandBuffer commandBuffer,
                                                   const VkPushDescriptorSetInfoKHR *pPushDescriptorSetInfo,
                                                   const RecordObject &record_obj) {
    PreCallRecordCmdPushDescriptorSet2(commandBuffer, pPushDescriptorSetInfo, record_obj);
}

void Device::PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                      const VkDescriptorBufferBindingInfoEXT *pBindingInfos,
                                                      const RecordObject &record_obj) {
    auto cb_state = Get<CommandBuffer>(commandBuffer);

    cb_state->descriptor_buffer_binding_info.resize(bufferCount);

    std::copy(pBindingInfos, pBindingInfos + bufferCount, cb_state->descriptor_buffer_binding_info.data());
}

void Device::PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                           VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount,
                                                           const uint32_t *pBufferIndices, const VkDeviceSize *pOffsets,
                                                           const RecordObject &record_obj) {
    auto cb_state = Get<CommandBuffer>(commandBuffer);
    auto pipeline_layout = Get<PipelineLayout>(layout);
    ASSERT_AND_RETURN(pipeline_layout);

    cb_state->UpdateLastBoundDescriptorBuffers(pipelineBindPoint, *pipeline_layout, firstSet, setCount, pBufferIndices, pOffsets);
}

void Device::PreCallRecordCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo,
    const RecordObject &record_obj) {
    auto cb_state = Get<CommandBuffer>(commandBuffer);
    auto pipeline_layout = Get<PipelineLayout>(pSetDescriptorBufferOffsetsInfo->layout);
    ASSERT_AND_RETURN(pipeline_layout);

    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        cb_state->UpdateLastBoundDescriptorBuffers(
            VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_layout, pSetDescriptorBufferOffsetsInfo->firstSet,
            pSetDescriptorBufferOffsetsInfo->setCount, pSetDescriptorBufferOffsetsInfo->pBufferIndices,
            pSetDescriptorBufferOffsetsInfo->pOffsets);
    }
    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        cb_state->UpdateLastBoundDescriptorBuffers(
            VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline_layout, pSetDescriptorBufferOffsetsInfo->firstSet,
            pSetDescriptorBufferOffsetsInfo->setCount, pSetDescriptorBufferOffsetsInfo->pBufferIndices,
            pSetDescriptorBufferOffsetsInfo->pOffsets);
    }
    if (IsStageInPipelineBindPoint(pSetDescriptorBufferOffsetsInfo->stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        cb_state->UpdateLastBoundDescriptorBuffers(
            VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline_layout, pSetDescriptorBufferOffsetsInfo->firstSet,
            pSetDescriptorBufferOffsetsInfo->setCount, pSetDescriptorBufferOffsetsInfo->pBufferIndices,
            pSetDescriptorBufferOffsetsInfo->pOffsets);
    }
}

void Device::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
                                            uint32_t offset, uint32_t size, const void *pValues, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);

    cb_state->RecordCmd(record_obj.location.function);
    auto layout_state = Get<PipelineLayout>(layout);
    cb_state->ResetPushConstantRangesLayoutIfIncompatible(*layout_state);

    if (IsStageInPipelineBindPoint(stageFlags, VK_PIPELINE_BIND_POINT_GRAPHICS)) {
        cb_state->push_constant_latest_used_layout[BindPoint_Graphics] = layout;
    } else if (IsStageInPipelineBindPoint(stageFlags, VK_PIPELINE_BIND_POINT_COMPUTE)) {
        cb_state->push_constant_latest_used_layout[BindPoint_Compute] = layout;
    } else if (IsStageInPipelineBindPoint(stageFlags, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)) {
        cb_state->push_constant_latest_used_layout[BindPoint_Ray_Tracing] = layout;
    } else {
        // Need to handle new binding point
        assert(false);
    }

    CommandBuffer::PushConstantData push_constant_data;
    push_constant_data.layout = layout;
    push_constant_data.stage_flags = stageFlags;
    push_constant_data.offset = offset;
    push_constant_data.values.resize(size);
    auto byte_values = static_cast<const std::byte *>(pValues);
    std::copy(byte_values, byte_values + size, push_constant_data.values.data());
    // Always add submitted push constant values, even if the same data is already stored.
    // Storing duplicated data, or data submitted by one vkCmdPushConstants call
    // and overridden by a subsequent one is not a problem.
    // push_constant_data_chunks is intended to be parsed from 0 to N,
    // thus going through the history in order, so even though it is
    // possibly suboptimal push constant data is correct.
    cb_state->push_constant_data_chunks.emplace_back(push_constant_data);
}

void Device::PostCallRecordCmdPushConstants2(VkCommandBuffer commandBuffer, const VkPushConstantsInfo *pPushConstantsInfo,
                                             const RecordObject &record_obj) {
    PostCallRecordCmdPushConstants(commandBuffer, pPushConstantsInfo->layout, pPushConstantsInfo->stageFlags,
                                   pPushConstantsInfo->offset, pPushConstantsInfo->size, pPushConstantsInfo->pValues, record_obj);
}

void Device::PostCallRecordCmdPushConstants2KHR(VkCommandBuffer commandBuffer, const VkPushConstantsInfoKHR *pPushConstantsInfo,
                                                const RecordObject &record_obj) {
    PostCallRecordCmdPushConstants2(commandBuffer, pPushConstantsInfo, record_obj);
}

void Device::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                             VkIndexType indexType, const RecordObject &record_obj) {
    if (buffer == VK_NULL_HANDLE) {
        return;  // allowed in maintenance6
    }
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    auto buffer_state = Get<Buffer>(buffer);
    // Being able to set the size was added in VK_KHR_maintenance5 via vkCmdBindIndexBuffer2KHR
    // Using this function is the same as passing in VK_WHOLE_SIZE
    VkDeviceSize buffer_size = Buffer::GetRegionSize(buffer_state, offset, VK_WHOLE_SIZE);
    cb_state->index_buffer_binding = IndexBufferBinding(buffer, buffer_size, offset, indexType);

    // Add binding for this index buffer to this commandbuffer
    if (!disabled[command_buffer_state] && buffer) {
        cb_state->AddChild(buffer_state);
    }
}

void Device::PreCallRecordCmdBindIndexBuffer2(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                              VkDeviceSize size, VkIndexType indexType, const RecordObject &record_obj) {
    if (buffer == VK_NULL_HANDLE) {
        return;  // allowed in maintenance6
    }
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    auto buffer_state = Get<Buffer>(buffer);
    VkDeviceSize buffer_size = Buffer::GetRegionSize(buffer_state, offset, size);
    cb_state->index_buffer_binding = IndexBufferBinding(buffer, buffer_size, offset, indexType);

    // Add binding for this index buffer to this commandbuffer
    if (!disabled[command_buffer_state] && buffer) {
        cb_state->AddChild(buffer_state);
    }
}

void Device::PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkDeviceSize size, VkIndexType indexType, const RecordObject &record_obj) {
    PreCallRecordCmdBindIndexBuffer2(commandBuffer, buffer, offset, size, indexType, record_obj);
}

void Device::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                               const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                               const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = Get<Buffer>(pBuffers[i]);
        // the stride is set from the pipeline or dynamic state
        vvl::VertexBufferBinding &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info[i + firstBinding];
        vertex_buffer_binding.bound = true;
        vertex_buffer_binding.buffer = pBuffers[i];
        vertex_buffer_binding.offset = pOffsets[i];
        vertex_buffer_binding.effective_size = Buffer::GetRegionSize(buffer_state, vertex_buffer_binding.offset, VK_WHOLE_SIZE);

        // Add binding for this vertex buffer to this commandbuffer
        if (pBuffers[i] && !disabled[command_buffer_state]) {
            cb_state->AddChild(buffer_state);
        }
    }
}

void Device::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                           VkDeviceSize dataSize, const void *pData, const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<Buffer>(dstBuffer));
}

void Device::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                      const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordSetEvent(record_obj.location.function, event, stageMask);
}

void Device::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfoKHR *pDependencyInfo,
                                          const RecordObject &record_obj) {
    PreCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
}

void Device::PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto stage_masks = sync_utils::GetGlobalStageMasks(*pDependencyInfo);

    cb_state->RecordSetEvent(record_obj.location.function, event, stage_masks.src);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void Device::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordResetEvent(record_obj.location.function, event, stageMask);
}

void Device::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2KHR stageMask,
                                            const RecordObject &record_obj) {
    PreCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
}

void Device::PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordResetEvent(record_obj.location.function, event, stageMask);
}

void Device::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                        VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                        uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                        uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                        uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordWaitEvents(record_obj.location.function, eventCount, pEvents, sourceStageMask);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void Device::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                            const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
}

void Device::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                         const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);
        cb_state->RecordWaitEvents(record_obj.location.function, 1, &pEvents[i], stage_masks.src);
        cb_state->RecordBarriers(dep_info);
    }
}

void Device::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void Device::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                 const RecordObject &record_obj) {
    PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo, record_obj);
}

void Device::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void Device::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                         VkQueryControlFlags flags, const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    uint32_t num_queries = 1;
    uint32_t subpass = 0;
    const bool inside_render_pass = cb_state->active_render_pass != nullptr;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (inside_render_pass) {
        subpass = cb_state->GetActiveSubpass();
        uint32_t bits = cb_state->active_render_pass->GetViewMaskBits(subpass);
        num_queries = std::max(num_queries, bits);
    }
    for (uint32_t i = 0; i < num_queries; ++i) {
        cb_state->RecordCmd(record_obj.location.function);
        if (!disabled[query_validation]) {
            QueryObject query_obj = {queryPool, slot, flags};
            query_obj.inside_render_pass = inside_render_pass;
            query_obj.subpass = subpass;
            cb_state->BeginQuery(query_obj);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QueryPool>(queryPool);
            cb_state->AddChild(pool_state);
        }
    }
}

void Device::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                       const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    uint32_t num_queries = 1;
    uint32_t subpass = 0;
    const bool inside_render_pass = cb_state->active_render_pass != nullptr;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (inside_render_pass) {
        subpass = cb_state->GetActiveSubpass();
        uint32_t bits = cb_state->active_render_pass->GetViewMaskBits(subpass);
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        cb_state->RecordCmd(record_obj.location.function);
        if (!disabled[query_validation]) {
            QueryObject query_obj = {queryPool, slot + i};
            query_obj.inside_render_pass = inside_render_pass;
            query_obj.subpass = subpass;
            cb_state->EndQuery(query_obj);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QueryPool>(queryPool);
            cb_state->AddChild(pool_state);
        }
    }
}

void Device::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                             uint32_t queryCount, const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->ResetQueryPool(queryPool, firstQuery, queryCount);

    if (!disabled[command_buffer_state]) {
        auto pool_state = Get<QueryPool>(queryPool);
        cb_state->AddChild(pool_state);
    }
}

void Device::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                   uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize stride, VkQueryResultFlags flags, const RecordObject &record_obj) {
    if (disabled[query_validation] || disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    auto dst_buff_state = Get<Buffer>(dstBuffer);
    cb_state->AddChild(dst_buff_state);
    auto pool_state = Get<QueryPool>(queryPool);
    cb_state->AddChild(pool_state);
}

void Device::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                             VkQueryPool queryPool, uint32_t slot, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordWriteTimestamp(record_obj.location.function, pipelineStage, queryPool, slot);
}

void Device::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                 VkQueryPool queryPool, uint32_t slot, const RecordObject &record_obj) {
    PostCallRecordCmdWriteTimestamp2(commandBuffer, pipelineStage, queryPool, slot, record_obj);
}

void Device::PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                              VkQueryPool queryPool, uint32_t slot, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordWriteTimestamp(record_obj.location.function, pipelineStage, queryPool, slot);
}

void Device::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer,
                                                                       uint32_t accelerationStructureCount,
                                                                       const VkAccelerationStructureKHR *pAccelerationStructures,
                                                                       VkQueryType queryType, VkQueryPool queryPool,
                                                                       uint32_t firstQuery, const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        auto pool_state = Get<QueryPool>(queryPool);
        cb_state->AddChild(pool_state);
    }
    cb_state->EndQueries(queryPool, firstQuery, accelerationStructureCount);
}

void Device::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkVideoSessionKHR *pVideoSession,
                                                 const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    auto profile_desc = video_profile_cache_.Get(physical_device, pCreateInfo->pVideoProfile);
    Add(std::make_shared<VideoSession>(*this, *pVideoSession, pCreateInfo, std::move(profile_desc)));
}

void Device::PostCallRecordGetVideoSessionMemoryRequirementsKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                uint32_t *pMemoryRequirementsCount,
                                                                VkVideoSessionMemoryRequirementsKHR *pMemoryRequirements,
                                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    auto vs_state = Get<VideoSession>(videoSession);
    ASSERT_AND_RETURN(vs_state);

    if (pMemoryRequirements != nullptr) {
        if (*pMemoryRequirementsCount > vs_state->memory_bindings_queried) {
            vs_state->memory_bindings_queried = *pMemoryRequirementsCount;
        }
    } else {
        vs_state->memory_binding_count_queried = true;
    }
}

void Device::PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                     uint32_t bindSessionMemoryInfoCount,
                                                     const VkBindVideoSessionMemoryInfoKHR *pBindSessionMemoryInfos,
                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    auto vs_state = Get<VideoSession>(videoSession);
    ASSERT_AND_RETURN(vs_state);

    for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
        vs_state->BindMemoryBindingIndex(pBindSessionMemoryInfos[i].memoryBindIndex);
    }
}

void Device::PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                 const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<VideoSession>(videoSession);
}

void Device::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                           const VkVideoSessionParametersCreateInfoKHR *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator,
                                                           VkVideoSessionParametersKHR *pVideoSessionParameters,
                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    Add(std::make_shared<VideoSessionParameters>(*pVideoSessionParameters, pCreateInfo,
                                                 Get<VideoSession>(pCreateInfo->videoSession),
                                                 Get<VideoSessionParameters>(pCreateInfo->videoSessionParametersTemplate)));
}

void Device::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                           const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo,
                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    Get<VideoSessionParameters>(videoSessionParameters)->Update(pUpdateInfo);
}

void Device::PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                           const VkAllocationCallbacks *pAllocator,
                                                           const RecordObject &record_obj) {
    Destroy<VideoSessionParameters>(videoSessionParameters);
}

void Device::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    std::vector<std::shared_ptr<ImageView>> views;
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        views.resize(pCreateInfo->attachmentCount);

        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            views[i] = Get<ImageView>(pCreateInfo->pAttachments[i]);
        }
    }

    Add(std::make_shared<Framebuffer>(*pFramebuffer, pCreateInfo, Get<RenderPass>(pCreateInfo->renderPass), std::move(views)));
}

void Device::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<RenderPass>(*pRenderPass, pCreateInfo));
}

void Device::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                const RecordObject &record_obj) {
    PostCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, record_obj);
}

void Device::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    Add(std::make_shared<RenderPass>(*pRenderPass, pCreateInfo));
}

void Device::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                             VkSubpassContents contents, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->BeginRenderPass(record_obj.location.function, pRenderPassBegin, contents);
}

void Device::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                 const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) {
    PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
}

void Device::PostCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo,
                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->BeginVideoCoding(pBeginInfo);
}

void Device::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                        uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                        const VkDeviceSize *pCounterBufferOffsets, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->transform_feedback_active = true;
}

void Device::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                      uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                      const VkDeviceSize *pCounterBufferOffsets, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->transform_feedback_active = false;
}

void Device::PostCallRecordCmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                           const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->conditional_rendering_active = true;
    cb_state->conditional_rendering_inside_render_pass = cb_state->active_render_pass != nullptr;
    cb_state->conditional_rendering_subpass = cb_state->GetActiveSubpass();
}

void Device::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->conditional_rendering_active = false;
    cb_state->conditional_rendering_inside_render_pass = false;
    cb_state->conditional_rendering_subpass = 0;
}

void Device::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                               const RecordObject &record_obj) {
    PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void Device::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                            const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->BeginRendering(record_obj.location.function, pRenderingInfo);
}

void Device::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    PreCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void Device::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->EndRendering(record_obj.location.function);
}

void Device::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                              const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->BeginRenderPass(record_obj.location.function, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void Device::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->NextSubpass(record_obj.location.function, contents);
}

void Device::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                              const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
}

void Device::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->NextSubpass(record_obj.location.function, pSubpassBeginInfo->contents);
}

void Device::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->EndRenderPass(record_obj.location.function);
}

void Device::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                const RecordObject &record_obj) {
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void Device::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->EndRenderPass(record_obj.location.function);
}

void Device::PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR *pEndCodingInfo,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->EndVideoCoding(pEndCodingInfo);
}

void Device::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                             const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->ExecuteCommands({pCommandBuffers, commandBuffersCount});
}

void Device::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkFlags flags,
                                     void **ppData, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordMappedMemory(mem, offset, size, ppData);
}

void Device::PostCallRecordMapMemory2(VkDevice device, const VkMemoryMapInfo *pMemoryMapInfo, void **ppData,
                                      const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordMappedMemory(pMemoryMapInfo->memory, pMemoryMapInfo->offset, pMemoryMapInfo->size, ppData);
}

void Device::PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR *pMemoryMapInfo, void **ppData,
                                         const RecordObject &record_obj) {
    PostCallRecordMapMemory2(device, pMemoryMapInfo, ppData, record_obj);
}

void Device::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem, const RecordObject &record_obj) {
    if (auto mem_info = Get<DeviceMemory>(mem)) {
        mem_info->mapped_range = MemRange();
        mem_info->p_driver_data = nullptr;
    }
}

void Device::PreCallRecordUnmapMemory2(VkDevice device, const VkMemoryUnmapInfo *pMemoryUnmapInfo, const RecordObject &record_obj) {
    if (auto mem_info = Get<DeviceMemory>(pMemoryUnmapInfo->memory)) {
        mem_info->mapped_range = MemRange();
        mem_info->p_driver_data = nullptr;
    }
}

void Device::PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR *pMemoryUnmapInfo,
                                          const RecordObject &record_obj) {
    PreCallRecordUnmapMemory2(device, pMemoryUnmapInfo, record_obj);
}

void Device::UpdateBindImageMemoryState(const VkBindImageMemoryInfo &bind_info) {
    auto image_state = Get<Image>(bind_info.image);
    if (!image_state) return;

    // An Android sepcial image cannot get VkSubresourceLayout until the image binds a memory.
    // See: VUID-vkGetImageSubresourceLayout-image-09432
    image_state->fragment_encoder =
        std::unique_ptr<const subresource_adapter::ImageRangeEncoder>(new subresource_adapter::ImageRangeEncoder(*image_state));
    const auto swapchain_info = vku::FindStructInPNextChain<VkBindImageMemorySwapchainInfoKHR>(bind_info.pNext);
    if (swapchain_info) {
        if (auto swapchain = Get<Swapchain>(swapchain_info->swapchain)) {
            // All images bound to this swapchain and index are aliases
            image_state->SetSwapchain(swapchain, swapchain_info->imageIndex);
        }
    } else {
        // Track bound memory range information
        if (auto mem_info = Get<DeviceMemory>(bind_info.memory)) {
            VkDeviceSize plane_index = 0u;
            if (image_state->disjoint && image_state->IsExternalBuffer() == false) {
                auto plane_info = vku::FindStructInPNextChain<VkBindImagePlaneMemoryInfo>(bind_info.pNext);
                plane_index = vkuGetPlaneIndex(plane_info->planeAspect);
            }
            image_state->BindMemory(
                image_state.get(), mem_info, bind_info.memoryOffset, plane_index,
                image_state->requirements[static_cast<decltype(image_state->requirements)::size_type>(plane_index)].size);
        }
    }
}

void Device::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset,
                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    VkBindImageMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.image = image;
    bind_info.memory = memory;
    bind_info.memoryOffset = memoryOffset;
    UpdateBindImageMemoryState(bind_info);
}

void Device::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) {
        // if bindInfoCount is 1, we know for sure if that single image was bound or not
        if (bindInfoCount > 1) {
            for (uint32_t i = 0; i < bindInfoCount; i++) {
                // If user passed in VkBindMemoryStatus, we can update which images are valid or not
                if (auto *bind_memory_status = vku::FindStructInPNextChain<VkBindMemoryStatus>(pBindInfos[i].pNext)) {
                    if (bind_memory_status->pResult && *bind_memory_status->pResult == VK_SUCCESS) {
                        UpdateBindImageMemoryState(pBindInfos[i]);
                    }
                } else if (auto image_state = Get<Image>(pBindInfos[i].image)) {
                    image_state->indeterminate_state = true;
                }
            }
        }
    } else {
        for (uint32_t i = 0; i < bindInfoCount; i++) {
            UpdateBindImageMemoryState(pBindInfos[i]);
        }
    }
}

void Device::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                               const RecordObject &record_obj) {
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
}

void Device::PreCallRecordSetEvent(VkDevice device, VkEvent event, const RecordObject &record_obj) {
    if (auto event_state = Get<Event>(event)) {
        event_state->signaled = true;
        event_state->signal_src_stage_mask = VK_PIPELINE_STAGE_HOST_BIT;
        event_state->signaling_queue = VK_NULL_HANDLE;
    }
}

void Device::PostCallRecordImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportSemaphoreState(pImportSemaphoreFdInfo->semaphore, pImportSemaphoreFdInfo->handleType,
                               pImportSemaphoreFdInfo->flags);
}

void Device::RecordGetExternalSemaphoreState(Semaphore &semaphore_state, VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    semaphore_state.Export(handle_type);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PostCallRecordImportSemaphoreWin32HandleKHR(VkDevice device,
                                                         const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo,
                                                         const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportSemaphoreState(pImportSemaphoreWin32HandleInfo->semaphore, pImportSemaphoreWin32HandleInfo->handleType,
                               pImportSemaphoreWin32HandleInfo->flags);
}

void Device::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                      HANDLE *pHandle, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (auto semaphore_state = Get<Semaphore>(pGetWin32HandleInfo->semaphore)) {
        RecordGetExternalSemaphoreState(*semaphore_state, pGetWin32HandleInfo->handleType);
    }
}

void Device::PostCallRecordImportFenceWin32HandleKHR(VkDevice device,
                                                     const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo,
                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportFenceState(pImportFenceWin32HandleInfo->fence, pImportFenceWin32HandleInfo->handleType,
                           pImportFenceWin32HandleInfo->flags);
}

void Device::PostCallRecordGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                  HANDLE *pHandle, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordGetExternalFenceState(pGetWin32HandleInfo->fence, pGetWin32HandleInfo->handleType, record_obj.location);
}
#endif

void Device::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (auto semaphore_state = Get<Semaphore>(pGetFdInfo->semaphore)) {
        // Record before locking with the WriteLockGuard
        RecordGetExternalSemaphoreState(*semaphore_state, pGetFdInfo->handleType);

        ExternalOpaqueInfo external_info = {};
        external_info.semaphore_flags = semaphore_state->flags;
        external_info.semaphore_type = semaphore_state->type;

        WriteLockGuard guard(fd_handle_map_lock_);
        fd_handle_map_.insert_or_assign(*pFd, external_info);
    }
}

void Device::RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type, VkFenceImportFlags flags) {
    if (auto fence_node = Get<Fence>(fence)) {
        fence_node->Import(handle_type, flags);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                   HANDLE *pHandle, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (const auto memory_state = Get<DeviceMemory>(pGetWin32HandleInfo->memory)) {
        // For validation purposes we need to keep allocation size and memory type index.
        // There is no need to keep pNext chain.
        ExternalOpaqueInfo external_info = {};
        external_info.allocation_size = memory_state->allocate_info.allocationSize;
        external_info.memory_type_index = memory_state->allocate_info.memoryTypeIndex;
        external_info.dedicated_buffer = memory_state->GetDedicatedBuffer();
        external_info.dedicated_image = memory_state->GetDedicatedImage();
        external_info.device_memory = pGetWin32HandleInfo->memory;

        WriteLockGuard guard(win32_handle_map_lock_);
        // `insert_or_assign` ensures that information is updated when the system decides to re-use
        // closed handle value for a new handle. The validation layer does not track handle close operation
        // which is performed by 'CloseHandle' system call.
        win32_handle_map_.insert_or_assign(*pHandle, external_info);
    }
}
#endif

void Device::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (const auto memory_state = Get<DeviceMemory>(pGetFdInfo->memory)) {
        // For validation purposes we need to keep allocation size and memory type index.
        // There is no need to keep pNext chain.
        ExternalOpaqueInfo external_info = {};
        external_info.allocation_size = memory_state->allocate_info.allocationSize;
        external_info.memory_type_index = memory_state->allocate_info.memoryTypeIndex;
        external_info.dedicated_buffer = memory_state->GetDedicatedBuffer();
        external_info.dedicated_image = memory_state->GetDedicatedImage();
        external_info.device_memory = memory_state->VkHandle();

        WriteLockGuard guard(fd_handle_map_lock_);
        // `insert_or_assign` ensures that information is updated when the system decides to re-use
        // closed handle value for a new handle. The fd handle created inside Vulkan _can_ be closed
        // using the 'close' system call, which is not tracked by the validation layer.
        fd_handle_map_.insert_or_assign(*pFd, external_info);
    }
}

void Device::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportFenceState(pImportFenceFdInfo->fence, pImportFenceFdInfo->handleType, pImportFenceFdInfo->flags);
}

void Device::RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type, const Location &loc) {
    if (auto fence_state = Get<Fence>(fence)) {
        // We no longer can track inflight fence after the export - perform early retire.
        fence_state->NotifyAndWait(loc);
        fence_state->Export(handle_type);
    }
}

void Device::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                         const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordGetExternalFenceState(pGetFdInfo->fence, pGetFdInfo->handleType, record_obj.location);
}

void Device::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                       const VkAllocationCallbacks *pAllocator, VkEvent *pEvent, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<Event>(*pEvent, pCreateInfo));
}

void Device::RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR *pCreateInfo, VkSwapchainKHR *pSwapchain,
                                        std::shared_ptr<Surface> &&surface_state, Swapchain *old_swapchain_state) {
    if (VK_SUCCESS == result) {
        if (surface_state->swapchain) {
            surface_state->RemoveParent(surface_state->swapchain);
        }
        auto swapchain = CreateSwapchainState(pCreateInfo, *pSwapchain);
        surface_state->AddParent(swapchain.get());
        surface_state->swapchain = swapchain.get();
        swapchain->surface = std::move(surface_state);
        auto swapchain_present_modes_ci = vku::FindStructInPNextChain<VkSwapchainPresentModesCreateInfoEXT>(pCreateInfo->pNext);
        if (swapchain_present_modes_ci) {
            const uint32_t present_mode_count = swapchain_present_modes_ci->presentModeCount;
            swapchain->present_modes.reserve(present_mode_count);
            std::copy(swapchain_present_modes_ci->pPresentModes, swapchain_present_modes_ci->pPresentModes + present_mode_count,
                      std::back_inserter(swapchain->present_modes));
        }

        // Initialize swapchain image state
        {
            uint32_t swapchain_image_count = 0;
            DispatchGetSwapchainImagesKHR(device, *pSwapchain, &swapchain_image_count, nullptr);
            std::vector<VkImage> swapchain_images(swapchain_image_count);
            DispatchGetSwapchainImagesKHR(device, *pSwapchain, &swapchain_image_count, swapchain_images.data());
            swapchain->images.resize(swapchain_image_count);
            const auto &image_ci = swapchain->image_create_info;
            for (uint32_t i = 0; i < swapchain_image_count; ++i) {
                auto format_features = instance_state->GetImageFormatFeatures(
                    physical_device, has_format_feature2, IsExtEnabled(extensions.vk_ext_image_drm_format_modifier), device,
                    swapchain_images[i], image_ci.format, image_ci.tiling);
                auto image_state = CreateImageState(swapchain_images[i], image_ci.ptr(), swapchain->VkHandle(), i, format_features);
                image_state->SetSwapchain(swapchain, i);
                image_state->SetInitialLayoutMap();
                swapchain->images[i].image_state = image_state.get();
                Add(std::move(image_state));
            }
        }
        Add(std::move(swapchain));
    } else {
        surface_state->swapchain = nullptr;
    }
    // Spec requires that even if CreateSwapchainKHR fails, oldSwapchain is retired
    // Retired swapchains remain associated with the surface until they are destroyed.
    if (old_swapchain_state) {
        old_swapchain_state->retired = true;
    }
    return;
}

void Device::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                              const RecordObject &record_obj) {
    auto surface_state = instance_state->Get<Surface>(pCreateInfo->surface);
    auto old_swapchain_state = Get<Swapchain>(pCreateInfo->oldSwapchain);
    RecordCreateSwapchainState(record_obj.result, pCreateInfo, pSwapchain, std::move(surface_state), old_swapchain_state.get());
}

void Device::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator,
                                              const RecordObject &record_obj) {
    Destroy<Swapchain>(swapchain);
}

void Instance::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                  const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode,
                                                  const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (!pMode) return;
    Add(std::make_shared<DisplayMode>(*pMode, physicalDevice));
}

void Device::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, const RecordObject &record_obj) {
    // spec: If vkQueuePresentKHR fails to enqueue the corresponding set of queue operations, it may return
    // VK_ERROR_OUT_OF_HOST_MEMORY or VK_ERROR_OUT_OF_DEVICE_MEMORY. If it does, the implementation must ensure that the state and
    // contents of any resources or synchronization primitives referenced is unaffected by the call or its failure.
    //
    // If vkQueuePresentKHR fails in such a way that the implementation is unable to make that guarantee, the implementation must
    // return VK_ERROR_DEVICE_LOST.
    //
    // However, if the presentation request is rejected by the presentation engine with an error VK_ERROR_OUT_OF_DATE_KHR,
    // VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, or VK_ERROR_SURFACE_LOST_KHR, the set of queue operations are still considered
    // to be enqueued and thus any semaphore wait operation specified in VkPresentInfoKHR will execute when the corresponding queue
    // operation is complete.
    //
    // NOTE: This is the only queue submit-like call that has its state updated in PostCallRecord(). In part that is because of
    // these non-fatal error cases. Also we need a place to handle the swapchain image bookkeeping, which really should be happening
    // once all the wait semaphores have completed. Since most of the PostCall queue submit race conditions are related to timeline
    // semaphores, and acquire sempaphores are always binary, this seems ok-ish.
    if (record_obj.result == VK_ERROR_OUT_OF_HOST_MEMORY || record_obj.result == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
        record_obj.result == VK_ERROR_DEVICE_LOST) {
        return;
    }

    const Location present_loc = record_obj.location.dot(Field::pPresentInfo);
    const auto *present_fence_info = vku::FindStructInPNextChain<VkSwapchainPresentFenceInfoEXT>(pPresentInfo->pNext);

    std::vector<QueueSubmission> present_submissions;  // TODO: use small_vector. Update interfaces to use span
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        present_submissions.emplace_back(present_loc.dot(Field::pSwapchains, i));
        if (present_fence_info) {
            present_submissions.back().AddFence(Get<Fence>(present_fence_info->pFences[i]));
        }
    }

    AcquireFenceSync acquire_fence_sync;
    for (uint32_t i = 0; i < pPresentInfo->waitSemaphoreCount; ++i) {
        if (auto semaphore_state = Get<Semaphore>(pPresentInfo->pWaitSemaphores[i])) {
            if (auto submission_ref = semaphore_state->GetPendingBinarySignalSubmission()) {
                acquire_fence_sync.submission_refs.emplace_back(submission_ref.value());
            }
            // Register present wait semaphores only in the first present batch.
            // NOTE: when presenting images from multiple swapchains, if some swapchains use
            // present fences, waiting on any present fence will retire all previous present batches.
            // As a result, the present wait semaphores from the first batch will always be retired.
            if (!present_submissions.empty()) {
                present_submissions[0].AddWaitSemaphore(std::move(semaphore_state), 0);
            }
        }
    }

    const auto *present_id_info = vku::FindStructInPNextChain<VkPresentIdKHR>(pPresentInfo->pNext);
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        // For multi-swapchain present pResults are always available (chassis adds pResults if necessary)
        assert(pPresentInfo->swapchainCount < 2 || pPresentInfo->pResults);
        auto local_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : record_obj.result;
        if (local_result != VK_SUCCESS && local_result != VK_SUBOPTIMAL_KHR) continue;  // this present didn't actually happen.
        // Mark the image as having been released to the WSI
        if (auto swapchain_data = Get<Swapchain>(pPresentInfo->pSwapchains[i])) {
            uint64_t present_id = (present_id_info && i < present_id_info->swapchainCount) ? present_id_info->pPresentIds[i] : 0;
            swapchain_data->PresentImage(pPresentInfo->pImageIndices[i], present_id, acquire_fence_sync);
        }
    }

    auto queue_state = Get<Queue>(queue);
    PreSubmitResult result = queue_state->PreSubmit(std::move(present_submissions));

    if (result.has_external_fence) {
        queue_state->NotifyAndWait(record_obj.location, result.submission_with_external_fence_seq);
    }
}

void Device::PostCallRecordReleaseSwapchainImagesEXT(VkDevice device, const VkReleaseSwapchainImagesInfoEXT *pReleaseInfo,
                                                     const RecordObject &record_obj) {
    if (auto swapchain_data = Get<Swapchain>(pReleaseInfo->swapchain)) {
        for (uint32_t i = 0; i < pReleaseInfo->imageIndexCount; ++i) {
            swapchain_data->ReleaseImage(pReleaseInfo->pImageIndices[i]);
        }
    }
}

void Device::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                     const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                     const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchains,
                                                     const RecordObject &record_obj) {
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = instance_state->Get<Surface>(pCreateInfos[i].surface);
            auto old_swapchain_state = Get<Swapchain>(pCreateInfos[i].oldSwapchain);
            RecordCreateSwapchainState(record_obj.result, &pCreateInfos[i], &pSwapchains[i], std::move(surface_state),
                                       old_swapchain_state.get());
        }
    }
}

void Device::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                         VkFence fence, uint32_t *pImageIndex, Func command) {
    auto fence_state = Get<Fence>(fence);
    if (fence_state) {
        // Treat as inflight since it is valid to wait on this fence, even in cases where it is technically a temporary
        // import
        fence_state->EnqueueSignal(nullptr, 0);
    }

    auto semaphore_state = Get<Semaphore>(semaphore);
    if (semaphore_state) {
        // Treat as signaled since it is valid to wait on this semaphore, even in cases where it is technically a
        // temporary import
        semaphore_state->EnqueueAcquire(command);
    }

    // Mark the image as acquired.
    auto swapchain_data = Get<Swapchain>(swapchain);
    if (swapchain_data) {
        swapchain_data->AcquireImage(*pImageIndex, semaphore_state, fence_state);
    }
}

void Device::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                               VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj.location.function);
}

void Device::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                uint32_t *pImageIndex, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex, record_obj.location.function);
}

std::shared_ptr<PhysicalDevice> Instance::CreatePhysicalDeviceState(VkPhysicalDevice handle) {
    return std::make_shared<PhysicalDevice>(handle);
}

void Instance::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                            VkInstance *pInstance, const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }

    uint32_t count = 0;
    // this can fail if the allocator fails
    VkResult result = DispatchEnumeratePhysicalDevices(*pInstance, &count, nullptr);
    if (result != VK_SUCCESS) {
        return;
    }
    std::vector<VkPhysicalDevice> physdev_handles(count);
    result = DispatchEnumeratePhysicalDevices(*pInstance, &count, physdev_handles.data());
    if (result != VK_SUCCESS) {
        return;
    }

    for (auto physdev : physdev_handles) {
        Add(CreatePhysicalDeviceState(physdev));
    }

#ifdef VK_USE_PLATFORM_METAL_EXT
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
    while (export_metal_object_info) {
        export_metal_flags.push_back(export_metal_object_info->exportObjectType);
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
}

void Instance::PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                         const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, const RecordObject &record_obj,
                                         vku::safe_VkDeviceCreateInfo *modified_create_info) {
#if defined(VVL_TRACY_GPU)
    auto ext_already_enabled = [](const vku::safe_VkDeviceCreateInfo *dci, const char *ext_name) {
        bool ext_enabled = false;
        for (auto ext : make_span(dci->ppEnabledExtensionNames, dci->enabledExtensionCount)) {
            if (strcmp(ext, ext_name) == 0) {
                ext_enabled = true;
                break;
            }
        }
        return ext_enabled;
    };

    auto enable_ext = [](vku::safe_VkDeviceCreateInfo *dci, const char *ext_name) {
        const char **tmp_ppEnabledExtensionNames = new const char *[dci->enabledExtensionCount + 1];
        for (uint32_t i = 0; i < dci->enabledExtensionCount; ++i) {
            tmp_ppEnabledExtensionNames[i] = vku::SafeStringCopy(dci->ppEnabledExtensionNames[i]);
        }
        tmp_ppEnabledExtensionNames[dci->enabledExtensionCount] = vku::SafeStringCopy(ext_name);
        dci->ppEnabledExtensionNames = tmp_ppEnabledExtensionNames;
        ++dci->enabledExtensionCount;
    };

    if (!ext_already_enabled(modified_create_info, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        enable_ext(modified_create_info, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    }
    auto host_query_reset_feature = const_cast<VkPhysicalDeviceHostQueryResetFeatures *>(
        vku::FindStructInPNextChain<VkPhysicalDeviceHostQueryResetFeatures>(pCreateInfo->pNext));
    if (host_query_reset_feature) {
        host_query_reset_feature->hostQueryReset = VK_TRUE;
    } else {
        VkPhysicalDeviceHostQueryResetFeatures new_host_query_reset_feature = vku::InitStructHelper();
        new_host_query_reset_feature.hostQueryReset = VK_TRUE;
        vku::AddToPnext(*modified_create_info, new_host_query_reset_feature);
    }

    if (!ext_already_enabled(modified_create_info, VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME)) {
        enable_ext(modified_create_info, VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
    }
#endif
}

// Common function to update state for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
static void StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice *pd_state, uint32_t count) {
    pd_state->queue_family_known_count = std::max(pd_state->queue_family_known_count, count);
}

void Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                    uint32_t *pQueueFamilyPropertyCount,
                                                                    VkQueueFamilyProperties *pQueueFamilyProperties,
                                                                    const RecordObject &record_obj) {
    auto pd_state = Get<PhysicalDevice>(physicalDevice);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state.get(), *pQueueFamilyPropertyCount);
}

void Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                     uint32_t *pQueueFamilyPropertyCount,
                                                                     VkQueueFamilyProperties2 *pQueueFamilyProperties,
                                                                     const RecordObject &record_obj) {
    auto pd_state = Get<PhysicalDevice>(physicalDevice);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state.get(), *pQueueFamilyPropertyCount);
}

void Instance::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                        uint32_t *pQueueFamilyPropertyCount,
                                                                        VkQueueFamilyProperties2 *pQueueFamilyProperties,
                                                                        const RecordObject &record_obj) {
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties,
                                                          record_obj);
}

void Instance::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator,
                                              const RecordObject &record_obj) {
    Destroy<Surface>(surface);
}

void Instance::RecordVulkanSurface(VkSurfaceKHR *pSurface) { Add(std::make_shared<Surface>(*pSurface)); }

void Instance::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void Instance::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_FUCHSIA
void Instance::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                           const VkImagePipeSurfaceCreateInfoFUCHSIA *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_IOS_MVK
void Instance::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                 const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
void Instance::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                   const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#ifdef VK_USE_PLATFORM_METAL_EXT
void Instance::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                   const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void Instance::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Instance::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                   const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
void Instance::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                 const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
void Instance::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                  const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void Instance::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                    const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void Instance::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance, const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                      const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
                                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto surface_state = Get<Surface>(surface);
    ASSERT_AND_RETURN(surface_state);
    surface_state->UpdateCapabilitiesCache(physicalDevice, *pSurfaceCapabilities);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                                      const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                      VkSurfaceCapabilities2KHR *pSurfaceCapabilities,
                                                                      const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    if (pSurfaceInfo->surface) {
        auto surface_state = Get<Surface>(pSurfaceInfo->surface);
        ASSERT_AND_RETURN(surface_state);
        if (!pSurfaceInfo->pNext) {
            surface_state->UpdateCapabilitiesCache(physicalDevice, pSurfaceCapabilities->surfaceCapabilities);
        } else if (IsExtEnabled(extensions.vk_ext_surface_maintenance1)) {
            const auto *surface_present_mode = vku::FindStructInPNextChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext);
            if (surface_present_mode) {
                // The surface caps caching should take into account pSurfaceInfo->pNext chain structure,
                // because each pNext element can affect query result. Here we support caching for a common
                // case when pNext chain is a single VkSurfacePresentModeEXT structure.
                const bool single_pnext_element = (pSurfaceInfo->pNext == surface_present_mode) && !surface_present_mode->pNext;
                if (single_pnext_element) {
                    surface_state->UpdateCapabilitiesCache(physicalDevice, *pSurfaceCapabilities,
                                                           surface_present_mode->presentMode);
                }
            }
        }
    } else if (IsExtEnabled(extensions.vk_google_surfaceless_query) &&
               vku::FindStructInPNextChain<VkSurfaceProtectedCapabilitiesKHR>(pSurfaceCapabilities->pNext)) {
        auto pd_state = Get<PhysicalDevice>(physicalDevice);
        ASSERT_AND_RETURN(pd_state);
        pd_state->surfaceless_query_state.capabilities = vku::safe_VkSurfaceCapabilities2KHR(pSurfaceCapabilities);
    }
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      VkSurfaceCapabilities2EXT *pSurfaceCapabilities,
                                                                      const RecordObject &record_obj) {
    const VkSurfaceCapabilitiesKHR caps{
        pSurfaceCapabilities->minImageCount,           pSurfaceCapabilities->maxImageCount,
        pSurfaceCapabilities->currentExtent,           pSurfaceCapabilities->minImageExtent,
        pSurfaceCapabilities->maxImageExtent,          pSurfaceCapabilities->maxImageArrayLayers,
        pSurfaceCapabilities->supportedTransforms,     pSurfaceCapabilities->currentTransform,
        pSurfaceCapabilities->supportedCompositeAlpha, pSurfaceCapabilities->supportedUsageFlags,
    };
    auto surface_state = Get<Surface>(surface);
    ASSERT_AND_RETURN(surface_state);
    surface_state->UpdateCapabilitiesCache(physicalDevice, caps);
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
                                                                VkSurfaceKHR surface, VkBool32 *pSupported,
                                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto surface_state = Get<Surface>(surface);
    ASSERT_AND_RETURN(surface_state);
    surface_state->SetQueueSupport(physicalDevice, queueFamilyIndex, (*pSupported == VK_TRUE));
}

void Instance::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                     uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes,
                                                                     const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;

    if (pPresentModes) {
        if (surface) {
            auto surface_state = Get<Surface>(surface);
            ASSERT_AND_RETURN(surface_state);
            surface_state->SetPresentModes(physicalDevice, span<const VkPresentModeKHR>(pPresentModes, *pPresentModeCount));
        } else if (IsExtEnabled(extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PhysicalDevice>(physicalDevice);
            ASSERT_AND_RETURN(pd_state);
            pd_state->surfaceless_query_state.present_modes =
                std::vector<VkPresentModeKHR>(pPresentModes, pPresentModes + *pPresentModeCount);
        }
    }
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats,
                                                                const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;

    if (pSurfaceFormats) {
        std::vector<vku::safe_VkSurfaceFormat2KHR> formats2(*pSurfaceFormatCount);
        for (uint32_t surface_format_index = 0; surface_format_index < *pSurfaceFormatCount; surface_format_index++) {
            formats2[surface_format_index].surfaceFormat = pSurfaceFormats[surface_format_index];
        }
        if (surface) {
            auto surface_state = Get<Surface>(surface);
            ASSERT_AND_RETURN(surface_state);
            surface_state->SetFormats(physicalDevice, std::move(formats2));
        } else if (IsExtEnabled(extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PhysicalDevice>(physicalDevice);
            ASSERT_AND_RETURN(pd_state);
            pd_state->surfaceless_query_state.formats = std::move(formats2);
        }
    }
}

void Instance::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                 const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                 uint32_t *pSurfaceFormatCount,
                                                                 VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                 const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;

    if (pSurfaceFormats) {
        if (pSurfaceInfo->surface) {
            auto surface_state = Get<Surface>(pSurfaceInfo->surface);
            ASSERT_AND_RETURN(surface_state);
            std::vector<vku::safe_VkSurfaceFormat2KHR> formats2(*pSurfaceFormatCount);
            for (uint32_t surface_format_index = 0; surface_format_index < *pSurfaceFormatCount; surface_format_index++) {
                formats2[surface_format_index].initialize(&pSurfaceFormats[surface_format_index]);
            }
            surface_state->SetFormats(physicalDevice, std::move(formats2));
        } else if (IsExtEnabled(extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PhysicalDevice>(physicalDevice);
            ASSERT_AND_RETURN(pd_state);
            pd_state->surfaceless_query_state.formats.clear();
            pd_state->surfaceless_query_state.formats.reserve(*pSurfaceFormatCount);
            for (uint32_t surface_format_index = 0; surface_format_index < *pSurfaceFormatCount; ++surface_format_index) {
                pd_state->surfaceless_query_state.formats.emplace_back(&pSurfaceFormats[surface_format_index]);
            }
        }
    }
}

void Device::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    debug_report->BeginCmdDebugUtilsLabel(commandBuffer, pLabelInfo);
}

void Device::PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo,
                                                      const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->BeginLabel((pLabelInfo && pLabelInfo->pLabelName) ? pLabelInfo->pLabelName : "");
}

void Device::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    cb_state->EndLabel();
    debug_report->EndCmdDebugUtilsLabel(commandBuffer);
}

void Device::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo,
                                                      const RecordObject &record_obj) {
    debug_report->InsertCmdDebugUtilsLabel(commandBuffer, pLabelInfo);

    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    // Squirrel away an easily accessible copy.
    cb_state->debug_label = LoggingLabel(pLabelInfo);
}

void Device::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR *pInfo,
                                                   const RecordObject &record_obj) {
    if (record_obj.result == VK_SUCCESS) performance_lock_acquired = true;
}

void Device::PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject &record_obj) {
    performance_lock_acquired = false;
    for (auto &cmd_buffer : command_buffer_map_.snapshot()) {
        cmd_buffer.second->performance_lock_released = true;
    }
}

void Device::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<DescriptorUpdateTemplate>(descriptorUpdateTemplate);
}

void Device::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             const RecordObject &record_obj) {
    PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator, record_obj);
}

void Device::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator,
                                                          VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<DescriptorUpdateTemplate>(*pDescriptorUpdateTemplate, pCreateInfo));
}

void Device::PostCallRecordCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                             const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                             const RecordObject &record_obj) {
    PostCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate, record_obj);
}

void Device::PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData,
                                                          const RecordObject &record_obj) {
    if (auto const template_state = Get<DescriptorUpdateTemplate>(descriptorUpdateTemplate)) {
        // TODO: Record template push descriptor updates
        if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
            PerformUpdateDescriptorSetsWithTemplateKHR(descriptorSet, template_state.get(), pData);
        }
    }
}

void Device::PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                             VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void *pData,
                                                             const RecordObject &record_obj) {
    PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData, record_obj);
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplate(VkCommandBuffer commandBuffer,
                                                           VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                           VkPipelineLayout layout, uint32_t set, const void *pData,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto template_state = Get<DescriptorUpdateTemplate>(descriptorUpdateTemplate);
    auto pipeline_layout = Get<PipelineLayout>(layout);
    if (!cb_state || !template_state || !pipeline_layout) {
        return;
    }

    cb_state->RecordCmd(record_obj.location.function);
    auto dsl = pipeline_layout->set_layouts[set];
    const auto &template_ci = template_state->create_info;
    // Decode the template into a set of write updates
    DecodedTemplateUpdate decoded_template(*this, VK_NULL_HANDLE, template_state.get(), pData, dsl->VkHandle());
    cb_state->PushDescriptorSetState(template_ci.pipelineBindPoint, *pipeline_layout, record_obj.location.function, set,
                                     static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                     decoded_template.desc_writes.data());
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                              VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                              VkPipelineLayout layout, uint32_t set, const void *pData,
                                                              const RecordObject &record_obj) {
    PreCallRecordCmdPushDescriptorSetWithTemplate(commandBuffer, descriptorUpdateTemplate, layout, set, pData, record_obj);
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo,
    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto template_state = Get<DescriptorUpdateTemplate>(pPushDescriptorSetWithTemplateInfo->descriptorUpdateTemplate);
    auto pipeline_layout = Get<PipelineLayout>(pPushDescriptorSetWithTemplateInfo->layout);
    if (!cb_state || !template_state || !pipeline_layout) {
        return;
    }

    cb_state->RecordCmd(record_obj.location.function);
    auto dsl = pipeline_layout->set_layouts[pPushDescriptorSetWithTemplateInfo->set];
    const auto &template_ci = template_state->create_info;
    // Decode the template into a set of write updates
    DecodedTemplateUpdate decoded_template(*this, VK_NULL_HANDLE, template_state.get(), pPushDescriptorSetWithTemplateInfo->pData,
                                           dsl->VkHandle());
    cb_state->PushDescriptorSetState(
        template_ci.pipelineBindPoint, *pipeline_layout, record_obj.location.function, pPushDescriptorSetWithTemplateInfo->set,
        static_cast<uint32_t>(decoded_template.desc_writes.size()), decoded_template.desc_writes.data());
}

void Device::PreCallRecordCmdPushDescriptorSetWithTemplate2KHR(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfoKHR *pPushDescriptorSetWithTemplateInfo,
    const RecordObject &record_obj) {
    PreCallRecordCmdPushDescriptorSetWithTemplate2(commandBuffer, pPushDescriptorSetWithTemplateInfo, record_obj);
}

void Instance::RecordGetPhysicalDeviceDisplayPlanePropertiesState(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                  void *pProperties) {
    auto pd_state = Get<PhysicalDevice>(physicalDevice);
    if (*pPropertyCount) {
        pd_state->display_plane_property_count = *pPropertyCount;
    }
    if (*pPropertyCount || pProperties) {
        pd_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called = true;
    }
}

void Instance::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                        VkDisplayPlanePropertiesKHR *pProperties,
                                                                        const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void Instance::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                         VkDisplayPlaneProperties2KHR *pProperties,
                                                                         const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void Device::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                   VkQueryControlFlags flags, uint32_t index, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    uint32_t num_queries = 1;
    uint32_t subpass = 0;
    const bool inside_render_pass = cb_state->active_render_pass != nullptr;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (inside_render_pass) {
        subpass = cb_state->GetActiveSubpass();
        uint32_t bits = cb_state->active_render_pass->GetViewMaskBits(subpass);
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        cb_state->RecordCmd(record_obj.location.function);
        if (!disabled[query_validation]) {
            QueryObject query_obj = {queryPool, slot, flags, 0, true, index + i};
            query_obj.inside_render_pass = inside_render_pass;
            query_obj.subpass = subpass;
            cb_state->BeginQuery(query_obj);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QueryPool>(queryPool);
            cb_state->AddChild(pool_state);
        }
    }
}

void Device::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                 uint32_t index, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    uint32_t num_queries = 1;
    uint32_t subpass = 0;
    const bool inside_render_pass = cb_state->active_render_pass != nullptr;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (inside_render_pass) {
        subpass = cb_state->GetActiveSubpass();
        uint32_t bits = cb_state->active_render_pass->GetViewMaskBits(subpass);
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        cb_state->RecordCmd(record_obj.location.function);
        if (!disabled[query_validation]) {
            QueryObject query_obj = {queryPool, slot, 0, 0, true, index + i};
            query_obj.inside_render_pass = inside_render_pass;
            query_obj.subpass = subpass;
            cb_state->EndQuery(query_obj);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QueryPool>(queryPool);
            cb_state->AddChild(pool_state);
        }
    }
}

void Device::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator,
                                                        VkSamplerYcbcrConversion *pYcbcrConversion,
                                                        const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    VkFormatFeatureFlags2KHR format_features = 0;

    if (pCreateInfo->format != VK_FORMAT_UNDEFINED) {
        format_features = GetPotentialFormatFeatures(pCreateInfo->format);
    } else if (IsExtEnabled(extensions.vk_android_external_memory_android_hardware_buffer)) {
        // If format is VK_FORMAT_UNDEFINED, format_features will be set by external AHB features
        format_features = GetExternalFormatFeaturesANDROID(pCreateInfo->pNext);
    }

    Add(std::make_shared<SamplerYcbcrConversion>(*pYcbcrConversion, pCreateInfo, format_features));
}

void Device::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator,
                                                           VkSamplerYcbcrConversion *pYcbcrConversion,
                                                           const RecordObject &record_obj) {
    PostCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, record_obj);
}

void Device::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                         const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<SamplerYcbcrConversion>(ycbcrConversion);
}

void Device::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            const RecordObject &record_obj) {
    PostCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator, record_obj);
}

void Device::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                             const RecordObject &record_obj) {
    PostCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount, record_obj);
}

void Device::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                          const RecordObject &record_obj) {
    // Do nothing if the feature is not enabled.
    if (!enabled_features.hostQueryReset) return;

    // Do nothing if the query pool has been destroyed.
    auto query_pool_state = Get<QueryPool>(queryPool);
    ASSERT_AND_RETURN(query_pool_state);

    // Reset the state of existing entries.
    const uint32_t max_query_count = std::min(queryCount, query_pool_state->create_info.queryCount - firstQuery);
    for (uint32_t i = 0; i < max_query_count; ++i) {
        auto query_index = firstQuery + i;
        query_pool_state->SetQueryState(query_index, 0, QUERYSTATE_RESET);
        if (query_pool_state->create_info.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
            for (uint32_t pass_index = 0; pass_index < query_pool_state->n_performance_passes; pass_index++) {
                query_pool_state->SetQueryState(query_index, pass_index, QUERYSTATE_RESET);
            }
        }
    }
}

void Device::PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet,
                                                        const DescriptorUpdateTemplate *template_state, const void *pData) {
    // Translate the templated update into a normal update for validation...
    DecodedTemplateUpdate decoded_update(*this, descriptorSet, template_state, pData);
    PerformUpdateDescriptorSets(static_cast<uint32_t>(decoded_update.desc_writes.size()), decoded_update.desc_writes.data(), 0,
                                NULL);
}

// Update the common AllocateDescriptorSetsData
void Device::UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo *allocate_info,
                                              AllocateDescriptorSetsData &ds_data) const {
    const auto *count_allocate_info =
        vku::FindStructInPNextChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(allocate_info->pNext);
    for (uint32_t i = 0; i < allocate_info->descriptorSetCount; i++) {
        if (auto layout = Get<DescriptorSetLayout>(allocate_info->pSetLayouts[i])) {
            ds_data.layout_nodes[i] = layout;
            // Count total descriptors required per type
            for (uint32_t j = 0; j < layout->GetBindingCount(); ++j) {
                const auto &binding_layout = layout->GetDescriptorSetLayoutBindingPtrFromIndex(j);
                uint32_t type_index = static_cast<uint32_t>(binding_layout->descriptorType);
                uint32_t descriptor_count = binding_layout->descriptorCount;
                if (count_allocate_info && i < count_allocate_info->descriptorSetCount) {
                    descriptor_count = count_allocate_info->pDescriptorCounts[i];
                }
                ds_data.required_descriptors_by_type[type_index] += descriptor_count;
            }
        }
        // Any unknown layouts will be flagged as errors during ValidateAllocateDescriptorSets() call
    }
}

void Device::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT *pVertexInfo,
                                           uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                  const VkMultiDrawIndexedInfoEXT *pIndexInfo, uint32_t instanceCount,
                                                  uint32_t firstInstance, uint32_t stride, const int32_t *pVertexOffset,
                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count,
                                           uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto buffer_state = Get<Buffer>(buffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void Device::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    auto buffer_state = Get<Buffer>(buffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void Device::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDispatchCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDispatchCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<Buffer>(buffer);
        cb_state->AddChild(buffer_state);
    }
}

void Device::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t base_x, uint32_t base_y, uint32_t base_z,
                                              uint32_t x, uint32_t y, uint32_t z, const RecordObject &record_obj) {
    PostCallRecordCmdDispatchBase(commandBuffer, x, y, z, base_x, base_y, base_z, record_obj);
}

void Device::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                                           uint32_t, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDispatchCmd(record_obj.location.function);
}

void Device::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                  VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                  uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
}

void Device::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<Buffer>(buffer);
        auto count_buffer_state = Get<Buffer>(countBuffer);
        cb_state->AddChild(buffer_state);
        cb_state->AddChild(count_buffer_state);
    }
}

void Device::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
}

void Device::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<Buffer>(buffer);
        auto count_buffer_state = Get<Buffer>(countBuffer);
        cb_state->AddChild(buffer_state);
        cb_state->AddChild(count_buffer_state);
    }
}

void Device::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    auto buffer_state = Get<Buffer>(buffer);
    if (!disabled[command_buffer_state] && buffer_state) {
        cb_state->AddChild(buffer_state);
    }
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                          uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        if (auto buffer_state = Get<Buffer>(buffer)) {
            cb_state->AddChild(buffer_state);
        }
        if (auto count_buffer_state = Get<Buffer>(countBuffer)) {
            cb_state->AddChild(count_buffer_state);
        }
    }
}

void Device::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                              uint32_t groupCountZ, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    auto buffer_state = Get<Buffer>(buffer);
    if (!disabled[command_buffer_state] && buffer_state) {
        cb_state->AddChild(buffer_state);
    }
}

void Device::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        if (auto buffer_state = Get<Buffer>(buffer)) {
            cb_state->AddChild(buffer_state);
        }
        if (auto count_buffer_state = Get<Buffer>(countBuffer)) {
            cb_state->AddChild(count_buffer_state);
        }
    }
}

void Device::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                          VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                          VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                          VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                          VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                          VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                          uint32_t width, uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                           const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                           const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                           uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                   const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                   const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                   VkDeviceAddress indirectDeviceAddress, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress,
                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void Device::PostCallRecordCmdExecuteGeneratedCommandsEXT(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed,
                                                          const VkGeneratedCommandsInfoEXT *pGeneratedCommandsInfo,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    const VkPipelineBindPoint bind_point = ConvertToPipelineBindPoint(pGeneratedCommandsInfo->shaderStages);
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        cb_state->UpdateDrawCmd(record_obj.location.function);
    } else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) {
        cb_state->UpdateDispatchCmd(record_obj.location.function);
    } else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
        cb_state->UpdateTraceRayCmd(record_obj.location.function);
    }
}

void Device::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                             const RecordObject &record_obj, chassis::CreateShaderModule &chassis_state) {
    if (pCreateInfo->codeSize == 0 || !pCreateInfo->pCode) {
        return;
    }

    chassis_state.module_state =
        std::make_shared<spirv::Module>(pCreateInfo->codeSize, pCreateInfo->pCode, &chassis_state.stateless_data);
    if (chassis_state.module_state && chassis_state.stateless_data.has_group_decoration) {
        spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(extensions.vk_khr_spirv_1_4));
        spvtools::Optimizer optimizer(spirv_environment);
        optimizer.RegisterPass(spvtools::CreateFlattenDecorationPass());
        std::vector<uint32_t> optimized_binary;
        // Run optimizer to flatten decorations only, set skip_validation so as to not re-run validator
        auto result = optimizer.Run(chassis_state.module_state->words_.data(), chassis_state.module_state->words_.size(),
                                    &optimized_binary, spvtools::ValidatorOptions(), true);

        if (result) {
            // Easier to just re-create the ShaderModule as StaticData uses itself when building itself up
            // It is really rare this will get here as Group Decorations have been deprecated and before this was added no one ever
            // raised an issue for a bug that would crash the layers that was around for many releases
            chassis_state.module_state = std::make_shared<spirv::Module>(optimized_binary.size() * sizeof(uint32_t),
                                                                         optimized_binary.data(), &chassis_state.stateless_data);
        }
    }
}

void Device::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                           const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                           const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (pCreateInfos[i].codeSize == 0 || !pCreateInfos[i].pCode) {
            continue;
        }
        // don't need to worry about GroupDecoration with VK_EXT_shader_object
        if (pCreateInfos[i].codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            chassis_state.module_states[i] = std::make_shared<spirv::Module>(
                pCreateInfos[i].codeSize, static_cast<const uint32_t *>(pCreateInfos[i].pCode), &chassis_state.stateless_data[i]);
        }
    }
}

void Device::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                              const RecordObject &record_obj, chassis::CreateShaderModule &chassis_state) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<ShaderModule>(*pShaderModule, chassis_state.module_state));
}

void Device::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT *pCreateInfos,
                                            const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                            const RecordObject &record_obj, chassis::ShaderObject &chassis_state) {
    if (VK_SUCCESS != record_obj.result) return;
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (pShaders[i] != VK_NULL_HANDLE) {
            Add(std::make_shared<ShaderObject>(*this, pCreateInfos[i], pShaders[i], chassis_state.module_states[i], createInfoCount,
                                               pShaders));
        }
    }
}

void Device::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                        const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                        const RecordObject &record_obj) {
    auto src_as_state = Get<AccelerationStructureKHR>(pInfo->src);
    auto dst_as_state = Get<AccelerationStructureKHR>(pInfo->dst);
    if (dst_as_state && src_as_state) {
        dst_as_state->is_built = true;

        dst_as_state->build_info_khr = src_as_state->build_info_khr;
    }
}

void Device::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    cb_state->RecordCmd(record_obj.location.function);
    auto src_as_state = Get<AccelerationStructureKHR>(pInfo->src);
    auto dst_as_state = Get<AccelerationStructureKHR>(pInfo->dst);
    if (dst_as_state && src_as_state) {
        dst_as_state->is_built = true;
        dst_as_state->build_info_khr = src_as_state->build_info_khr;
        if (!disabled[command_buffer_state]) {
            cb_state->AddChild(dst_as_state);
            cb_state->AddChild(src_as_state);
        }
    }
}

void Device::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo,
                                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    cb_state->RecordCmd(record_obj.location.function);
    auto src_as_state = Get<AccelerationStructureKHR>(pInfo->src);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(src_as_state);
    }
    // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
    // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a command
    // buffer
}

void Device::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo,
                                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    ASSERT_AND_RETURN(cb_state);
    cb_state->RecordCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        auto dst_as_state = Get<AccelerationStructureKHR>(pInfo->dst);
        ASSERT_AND_RETURN(dst_as_state);
        cb_state->AddChild(dst_as_state);
        dst_as_state->is_built = true;

        // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
        // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a
        // command buffer
    }
}

void Device::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                             const RecordObject &record_obj) {
    PostCallRecordCmdSetCullMode(commandBuffer, cullMode, record_obj);
}

void Device::PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_CULL_MODE);
    cb_state->dynamic_state_value.cull_mode = cullMode;
}

void Device::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                              const RecordObject &record_obj) {
    PostCallRecordCmdSetFrontFace(commandBuffer, frontFace, record_obj);
}

void Device::PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_FRONT_FACE);
}

void Device::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                      const RecordObject &record_obj) {
    PostCallRecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, record_obj);
}

void Device::PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    cb_state->dynamic_state_value.primitive_topology = primitiveTopology;
}

void Device::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                      const VkViewport *pViewports, const RecordObject &record_obj) {
    PostCallRecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, record_obj);
}

void Device::PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                   const VkViewport *pViewports, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    uint32_t bits = (1u << viewportCount) - 1u;
    cb_state->viewportWithCountMask |= bits;
    cb_state->trashedViewportMask &= ~bits;
    cb_state->dynamic_state_value.viewport_count = viewportCount;
    cb_state->trashedViewportCount = false;

    cb_state->dynamic_state_value.viewports.resize(viewportCount);
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamic_state_value.viewports[i] = pViewports[i];
    }
}

void Device::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                     const VkRect2D *pScissors, const RecordObject &record_obj) {
    PostCallRecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, record_obj);
}

void Device::PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D *pScissors,
                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    uint32_t bits = (1u << scissorCount) - 1u;
    cb_state->scissorWithCountMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
    cb_state->dynamic_state_value.scissor_count = scissorCount;
    cb_state->trashedScissorCount = false;
}

void Device::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                    const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                    const VkDeviceSize *pSizes, const VkDeviceSize *pStrides,
                                                    const RecordObject &record_obj) {
    PostCallRecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                        record_obj);
}

void Device::PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                 const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                 const VkDeviceSize *pStrides, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (pStrides) {
        cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = Get<vvl::Buffer>(pBuffers[i]);
        vvl::VertexBufferBinding &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info[i + firstBinding];
        vertex_buffer_binding.bound = true;
        vertex_buffer_binding.buffer = pBuffers[i];
        vertex_buffer_binding.offset = pOffsets[i];
        vertex_buffer_binding.effective_size = pSizes ? pSizes[i] : VK_WHOLE_SIZE;
        if (vertex_buffer_binding.effective_size == VK_WHOLE_SIZE) {
            vertex_buffer_binding.effective_size = Buffer::GetRegionSize(buffer_state, pOffsets[i], VK_WHOLE_SIZE);
        }

        if (pStrides) {
            vertex_buffer_binding.stride = pStrides[i];
        }

        // Add binding for this vertex buffer to this commandbuffer
        if (!disabled[command_buffer_state] && pBuffers[i]) {
            cb_state->AddChild(buffer_state);
        }
    }
}

void Device::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                    const RecordObject &record_obj) {
    PostCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    cb_state->dynamic_state_value.depth_test_enable = depthTestEnable;
}

void Device::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                     const RecordObject &record_obj) {
    PostCallRecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    cb_state->dynamic_state_value.depth_write_enable = depthWriteEnable;
}

void Device::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                   const RecordObject &record_obj) {
    PostCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj);
}

void Device::PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_COMPARE_OP);
}

void Device::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                          const RecordObject &record_obj) {
    PostCallRecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
    cb_state->dynamic_state_value.depth_bounds_test_enable = depthBoundsTestEnable;
}

void Device::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                      const RecordObject &record_obj) {
    PostCallRecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, record_obj);
}

void Device::PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
    cb_state->dynamic_state_value.stencil_test_enable = stencilTestEnable;
}

void Device::PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                              VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                              const RecordObject &record_obj) {
    PostCallRecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, record_obj);
}

void Device::PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                           VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_OP);
    if (faceMask == VK_STENCIL_FACE_FRONT_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.fail_op_front = failOp;
        cb_state->dynamic_state_value.pass_op_front = passOp;
        cb_state->dynamic_state_value.depth_fail_op_front = depthFailOp;
    }
    if (faceMask == VK_STENCIL_FACE_BACK_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.fail_op_back = failOp;
        cb_state->dynamic_state_value.pass_op_back = passOp;
        cb_state->dynamic_state_value.depth_fail_op_back = depthFailOp;
    }
}

void Device::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                     uint32_t discardRectangleCount, const VkRect2D *pDiscardRectangles,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT);
    for (uint32_t i = 0; i < discardRectangleCount; i++) {
        cb_state->dynamic_state_value.discard_rectangles.set(firstDiscardRectangle + i);
    }
}

void Device::PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 discardRectangleEnable,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT);
    cb_state->dynamic_state_value.discard_rectangle_enable = discardRectangleEnable;
}

void Device::PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                         VkDiscardRectangleModeEXT discardRectangleMode,
                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT);
}

void Device::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                    const VkSampleLocationsInfoEXT *pSampleLocationsInfo,
                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
    cb_state->dynamic_state_value.sample_locations_info = *pSampleLocationsInfo;
}

void Device::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                     uint32_t customSampleOrderCount,
                                                     const VkCoarseSampleOrderCustomNV *pCustomSampleOrders,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV);
}

void Device::PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
}

void Device::PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LOGIC_OP_EXT);
}

void Device::PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                            const RecordObject &record_obj) {
    PostCallRecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, record_obj);
}

void Device::PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
    cb_state->dynamic_state_value.rasterizer_discard_enable = (rasterizerDiscardEnable == VK_TRUE);
}

void Device::PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                    const RecordObject &record_obj) {
    PostCallRecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, record_obj);
}

void Device::PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    cb_state->dynamic_state_value.depth_bias_enable = depthBiasEnable;
}

void Device::PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                           const RecordObject &record_obj) {
    PostCallRecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, record_obj);
}

void Device::PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    cb_state->dynamic_state_value.primitive_restart_enable = primitiveRestartEnable;
}

void Device::PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D *pFragmentSize,
                                                        const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
    cb_state->dynamic_state_value.fragment_size = *pFragmentSize;
}

void Device::PostCallRecordCmdSetRenderingAttachmentLocations(VkCommandBuffer commandBuffer,
                                                              const VkRenderingAttachmentLocationInfo *pLocationInfo,
                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->rendering_attachments.set_color_locations = true;
    cb_state->rendering_attachments.color_locations.resize(pLocationInfo->colorAttachmentCount);
    for (size_t i = 0; i < pLocationInfo->colorAttachmentCount; ++i) {
        cb_state->rendering_attachments.color_locations[i] = pLocationInfo->pColorAttachmentLocations[i];
    }
}

void Device::PostCallRecordCmdSetRenderingAttachmentLocationsKHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderingAttachmentLocationInfoKHR *pLocationInfo,
                                                                 const RecordObject &record_obj) {
    PostCallRecordCmdSetRenderingAttachmentLocations(commandBuffer, pLocationInfo, record_obj);
}

void Device::PostCallRecordCmdSetRenderingInputAttachmentIndices(VkCommandBuffer commandBuffer,
                                                                 const VkRenderingInputAttachmentIndexInfo *pLocationInfo,
                                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);

    cb_state->rendering_attachments.set_color_indexes = true;
    cb_state->rendering_attachments.color_indexes.resize(pLocationInfo->colorAttachmentCount);
    for (size_t i = 0; i < pLocationInfo->colorAttachmentCount; ++i) {
        cb_state->rendering_attachments.color_indexes[i] = pLocationInfo->pColorAttachmentInputIndices[i];
    }
    cb_state->rendering_attachments.depth_index = pLocationInfo->pDepthInputAttachmentIndex;
    cb_state->rendering_attachments.stencil_index = pLocationInfo->pStencilInputAttachmentIndex;
}

void Device::PostCallRecordCmdSetRenderingInputAttachmentIndicesKHR(VkCommandBuffer commandBuffer,
                                                                    const VkRenderingInputAttachmentIndexInfoKHR *pLocationInfo,
                                                                    const RecordObject &record_obj) {
    PostCallRecordCmdSetRenderingInputAttachmentIndices(commandBuffer, pLocationInfo, record_obj);
}

void Device::PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize,
                                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    // CB_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR);
    cb_state->dynamic_state_status.rtx_stack_size_cb = true;
    cb_state->dynamic_state_status.rtx_stack_size_pipeline = true;
}

void Device::PostCallRecordCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
                                                const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
                                                uint32_t vertexAttributeDescriptionCount,
                                                const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VERTEX_INPUT_EXT);

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (pipeline_state && pipeline_state->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)) {
        cb_state->RecordDynamicState(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    }
    auto &vertex_bindings = cb_state->dynamic_state_value.vertex_bindings;

    // When using Dynamic state, anything not set is invalid, so need to reset map
    // "The vertex attribute description for any location not specified in the pVertexAttributeDescriptions array becomes undefined"
    vertex_bindings.clear();

    for (const auto [i, description] : vvl::enumerate(pVertexBindingDescriptions, vertexBindingDescriptionCount)) {
        vertex_bindings.insert_or_assign(description.binding, VertexBindingState(i, &description));

        cb_state->current_vertex_buffer_binding_info[description.binding].stride = description.stride;
    }

    for (const auto [i, description] : vvl::enumerate(pVertexAttributeDescriptions, vertexAttributeDescriptionCount)) {
        if (auto *binding_state = vvl::Find(vertex_bindings, description.binding)) {
            binding_state->locations.insert_or_assign(description.location, VertexAttrState(i, &description));
        }
    }
}

void Device::PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkBool32 *pColorWriteEnables, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT);
    cb_state->dynamic_state_value.color_write_enable_attachment_count = attachmentCount;
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        if (pColorWriteEnables[i]) {
            cb_state->dynamic_state_value.color_write_enabled.set(i);
        } else {
            cb_state->dynamic_state_value.color_write_enabled.reset(i);
        }
    }
}

void Device::PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer, VkImageAspectFlags aspectMask,
                                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT);
    cb_state->dynamic_state_value.attachment_feedback_loop_enable = aspectMask;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void Device::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                             const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    auto swapchain_state = Get<Swapchain>(swapchain);
    ASSERT_AND_RETURN(swapchain_state);
    swapchain_state->exclusive_full_screen_access = true;
}

void Device::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                             const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    auto swapchain_state = Get<Swapchain>(swapchain);
    ASSERT_AND_RETURN(swapchain_state);
    swapchain_state->exclusive_full_screen_access = false;
}
#endif

void Device::PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer, VkTessellationDomainOrigin domainOrigin,
                                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT);
}

void Device::PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT);
    cb_state->dynamic_state_value.depth_clamp_enable = depthClampEnable;
}

void Device::PostCallRecordCmdSetDepthClampRangeEXT(VkCommandBuffer commandBuffer, VkDepthClampModeEXT depthClampMode,
                                                    const VkDepthClampRangeEXT *pDepthClampRange, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_CLAMP_RANGE_EXT);
}

void Device::PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_POLYGON_MODE_EXT);
    cb_state->dynamic_state_value.polygon_mode = polygonMode;
}

void Device::PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits rasterizationSamples,
                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    cb_state->dynamic_state_value.rasterization_samples = rasterizationSamples;
}

void Device::PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                               const VkSampleMask *pSampleMask, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SAMPLE_MASK_EXT);
    cb_state->dynamic_state_value.samples_mask_samples = samples;
}

void Device::PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToCoverageEnable,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    cb_state->dynamic_state_value.alpha_to_coverage_enable = alphaToCoverageEnable;
}

void Device::PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
}

void Device::PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    cb_state->dynamic_state_value.logic_op_enable = logicOpEnable;
}

void Device::PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                     uint32_t attachmentCount, const VkBool32 *pColorBlendEnables,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_enable_attachments.set(firstAttachment + i);
        if (pColorBlendEnables[i]) {
            cb_state->dynamic_state_value.color_blend_enabled.set(firstAttachment + i);
        } else {
            cb_state->dynamic_state_value.color_blend_enabled.reset(firstAttachment + i);
        }
    }
}

void Device::PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount,
                                                       const VkColorBlendEquationEXT *pColorBlendEquations,
                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    if (cb_state->dynamic_state_value.color_blend_equations.size() < firstAttachment + attachmentCount) {
        cb_state->dynamic_state_value.color_blend_equations.resize(firstAttachment + attachmentCount);
    }
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_equation_attachments.set(firstAttachment + i);
        cb_state->dynamic_state_value.color_blend_equations[firstAttachment + i] = pColorBlendEquations[i];
    }
}

void Device::PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                   uint32_t attachmentCount, const VkColorComponentFlags *pColorWriteMasks,
                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    if (cb_state->dynamic_state_value.color_write_masks.size() < firstAttachment + attachmentCount) {
        cb_state->dynamic_state_value.color_write_masks.resize(firstAttachment + attachmentCount);
    }
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_write_mask_attachments.set(firstAttachment + i);
        cb_state->dynamic_state_value.color_write_masks[i] = pColorWriteMasks[i];
    }
}

void Device::PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT);
    cb_state->dynamic_state_value.rasterization_stream = rasterizationStream;
}

void Device::PostCallRecordCmdSetConservativeRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                  VkConservativeRasterizationModeEXT conservativeRasterizationMode,
                                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT);
    cb_state->dynamic_state_value.conservative_rasterization_mode = conservativeRasterizationMode;
}

void Device::PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                     float extraPrimitiveOverestimationSize,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT);
}

void Device::PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT);
}

void Device::PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer, VkBool32 sampleLocationsEnable,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT);
    cb_state->dynamic_state_value.sample_locations_enable = sampleLocationsEnable;
}

void Device::PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                       uint32_t attachmentCount, const VkColorBlendAdvancedEXT *pColorBlendAdvanced,
                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_advanced_attachments.set(firstAttachment + i);
    }
}

void Device::PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer, VkProvokingVertexModeEXT provokingVertexMode,
                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT);
}

void Device::PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                          VkLineRasterizationModeEXT lineRasterizationMode,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
    cb_state->dynamic_state_value.line_rasterization_mode = lineRasterizationMode;
}

void Device::PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                      const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
    cb_state->dynamic_state_value.stippled_line_enable = stippledLineEnable;
}

void Device::PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne,
                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT);
}

void Device::PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer, VkBool32 viewportWScalingEnable,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV);
    cb_state->dynamic_state_value.viewport_w_scaling_enable = viewportWScalingEnable;
}

void Device::PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                                   const VkViewportSwizzleNV *pViewportSwizzles, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV);
    cb_state->dynamic_state_value.viewport_swizzle_count = viewportCount;
}

void Device::PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable,
                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV);
    cb_state->dynamic_state_value.coverage_to_color_enable = coverageToColorEnable;
}

void Device::PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer, uint32_t coverageToColorLocation,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV);
    cb_state->dynamic_state_value.coverage_to_color_location = coverageToColorLocation;
}

void Device::PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                          VkCoverageModulationModeNV coverageModulationMode,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV);
    cb_state->dynamic_state_value.coverage_modulation_mode = coverageModulationMode;
}

void Device::PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                 VkBool32 coverageModulationTableEnable,
                                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV);
    cb_state->dynamic_state_value.coverage_modulation_table_enable = coverageModulationTableEnable;
}

void Device::PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer, uint32_t coverageModulationTableCount,
                                                           const float *pCoverageModulationTable, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV);
}

void Device::PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer, VkBool32 shadingRateImageEnable,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV);
    cb_state->dynamic_state_value.shading_rate_image_enable = shadingRateImageEnable;
}

void Device::PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                    VkBool32 representativeFragmentTestEnable,
                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV);
}

void Device::PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                         VkCoverageReductionModeNV coverageReductionMode,
                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV);
}

void Device::PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                    const VkVideoCodingControlInfoKHR *pCodingControlInfo,
                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->ControlVideoCoding(pCodingControlInfo);
}

void Device::PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->DecodeVideo(pDecodeInfo);
}

void Device::PostCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->EncodeVideo(pEncodeInfo);
}

void Device::PostCallRecordGetShaderModuleIdentifierEXT(VkDevice, const VkShaderModule shaderModule,
                                                        VkShaderModuleIdentifierEXT *pIdentifier, const RecordObject &record_obj) {
    if (const auto shader_state = Get<ShaderModule>(shaderModule); shader_state) {
        WriteLockGuard guard(shader_identifier_map_lock_);
        shader_identifier_map_.emplace(*pIdentifier, std::move(shader_state));
    }
}

void Device::PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice, const VkShaderModuleCreateInfo *pCreateInfo,
                                                                  VkShaderModuleIdentifierEXT *pIdentifier,
                                                                  const RecordObject &record_obj) {
    WriteLockGuard guard(shader_identifier_map_lock_);
    shader_identifier_map_.emplace(*pIdentifier, std::make_shared<ShaderModule>());
}

void Device::PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                            const VkShaderStageFlagBits *pStages, const VkShaderEXT *pShaders,
                                            const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    if (pStages) {
        for (uint32_t i = 0; i < stageCount; ++i) {
            cb_state->BindPipeline(ConvertToLvlBindPoint(pStages[i]), nullptr);
        }
    }
}

void Device::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                  const RecordObject &record_obj) {
    if (record_obj.device_address == 0) return;
    if (auto buffer_state = Get<Buffer>(pInfo->buffer)) {
        WriteLockGuard guard(buffer_address_lock_);
        // address is used for GPU-AV and ray tracing buffer validation
        buffer_state->deviceAddress = record_obj.device_address;
        const auto address_range = buffer_state->DeviceAddressRange();

        BufferAddressInfillUpdateOps ops{{buffer_state.get()}};
        sparse_container::infill_update_range(buffer_address_map_, address_range, ops);
        buffer_device_address_ranges_version++;
    }
}

void Device::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                     const RecordObject &record_obj) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

void Device::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                     const RecordObject &record_obj) {
    PostCallRecordGetBufferDeviceAddress(device, pInfo, record_obj);
}

std::shared_ptr<Swapchain> Device::CreateSwapchainState(const VkSwapchainCreateInfoKHR *create_info, VkSwapchainKHR handle) {
    return std::make_shared<Swapchain>(*this, create_info, handle);
}

std::shared_ptr<CommandBuffer> Device::CreateCmdBufferState(VkCommandBuffer handle,
                                                            const VkCommandBufferAllocateInfo *allocate_info,
                                                            const CommandPool *pool) {
    return std::make_shared<CommandBuffer>(*this, handle, allocate_info, pool);
}

std::shared_ptr<DeviceMemory> Device::CreateDeviceMemoryState(VkDeviceMemory handle, const VkMemoryAllocateInfo *allocate_info,
                                                              uint64_t fake_address, const VkMemoryType &memory_type,
                                                              const VkMemoryHeap &memory_heap,
                                                              std::optional<DedicatedBinding> &&dedicated_binding,
                                                              uint32_t physical_device_count) {
    return std::make_shared<DeviceMemory>(handle, allocate_info, fake_address, memory_type, memory_heap,
                                          std::move(dedicated_binding), physical_device_count);
}

void Device::PostCallRecordCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                              uint32_t bindingCount, const VkBuffer *pBuffers,
                                                              const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CommandBuffer>(commandBuffer);
    cb_state->transform_feedback_buffers_bound = bindingCount;
}

void Device::PreCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV *pSleepInfo,
                                         const RecordObject &record_obj) {
    if (auto semaphore_state = Get<Semaphore>(pSleepInfo->signalSemaphore)) {
        auto value = pSleepInfo->value;
        semaphore_state->EnqueueSignal(SubmissionReference{}, value);
    }
}

// TODO: PostRecord is not needed. Test this. WaitSemaphores will retire the signal.
// LatencySleepNV does not perform wait but provides information about semaphore to the driver.
void Device::PostCallRecordLatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV *pSleepInfo,
                                          const RecordObject &record_obj) {
    if (auto semaphore_state = Get<Semaphore>(pSleepInfo->signalSemaphore)) {
        semaphore_state->RetireWait(nullptr, pSleepInfo->value, record_obj.location);
    }
}

void Device::PostCallRecordCreateIndirectExecutionSetEXT(VkDevice device, const VkIndirectExecutionSetCreateInfoEXT *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkIndirectExecutionSetEXT *pIndirectExecutionSet,
                                                         const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    std::shared_ptr<IndirectExecutionSet> indirect_execution_state =
        std::make_shared<IndirectExecutionSet>(*this, *pIndirectExecutionSet, pCreateInfo);

    if (indirect_execution_state->is_pipeline && pCreateInfo->info.pPipelineInfo) {
        const VkIndirectExecutionSetPipelineInfoEXT &pipeline_info = *pCreateInfo->info.pPipelineInfo;
        indirect_execution_state->initial_pipeline = Get<Pipeline>(pipeline_info.initialPipeline);
        indirect_execution_state->shader_stage_flags = indirect_execution_state->initial_pipeline->active_shaders;
    } else if (indirect_execution_state->is_shader_objects && pCreateInfo->info.pShaderInfo) {
        const VkIndirectExecutionSetShaderInfoEXT &shader_info = *pCreateInfo->info.pShaderInfo;
        for (uint32_t i = 0; i < shader_info.shaderCount; i++) {
            const VkShaderEXT shader_handle = shader_info.pInitialShaders[i];
            const auto shader_object = Get<ShaderObject>(shader_handle);
            ASSERT_AND_CONTINUE(shader_object);
            indirect_execution_state->shader_stage_flags |= shader_object->create_info.stage;
            if (shader_object->create_info.stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
                indirect_execution_state->initial_fragment_shader_object = shader_object;
            }
        }
    }

    Add(std::move(indirect_execution_state));
}

void Device::PostCallRecordDestroyIndirectExecutionSetEXT(VkDevice device, VkIndirectExecutionSetEXT indirectExecutionSet,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    Destroy<IndirectExecutionSet>(indirectExecutionSet);
}

void Device::PostCallRecordCreateIndirectCommandsLayoutEXT(VkDevice device,
                                                           const VkIndirectCommandsLayoutCreateInfoEXT *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator,
                                                           VkIndirectCommandsLayoutEXT *pIndirectCommandsLayout,
                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<IndirectCommandsLayout>(*this, *pIndirectCommandsLayout, pCreateInfo));
}

void Device::PostCallRecordDestroyIndirectCommandsLayoutEXT(VkDevice device, VkIndirectCommandsLayoutEXT indirectCommandsLayout,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            const RecordObject &record_obj) {
    Destroy<IndirectCommandsLayout>(indirectCommandsLayout);
}
}  // namespace vvl
