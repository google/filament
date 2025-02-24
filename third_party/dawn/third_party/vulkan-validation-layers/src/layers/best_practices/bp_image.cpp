/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/bp_state.h"
#include "state_tracker/queue_state.h"

bool BestPractices::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImage* pImage,
                                               const ErrorObject& error_obj) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        skip |= LogWarning("BestPractices-vkCreateImage-sharing-mode-exclusive", device,
                           error_obj.location.dot(Field::pCreateInfo).dot(Field::sharingMode),
                           "is VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                           "(queueFamilyIndexCount of %" PRIu32 ").",
                           pCreateInfo->queueFamilyIndexCount);
    }

    if ((pCreateInfo->flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT) && !(pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
        skip |= LogWarning("BestPractices-vkCreateImage-CreateFlags", device,
                           error_obj.location.dot(Field::pCreateInfo).dot(Field::flags),
                           "has VK_IMAGE_CREATE_EXTENDED_USAGE_BIT set, but not "
                           "VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, therefore image views created from this image will have to use the "
                           "same format and VK_IMAGE_CREATE_EXTENDED_USAGE_BIT will not have any effect.");
    }

    if (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) {
        if (pCreateInfo->samples > VK_SAMPLE_COUNT_1_BIT && !(pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
            skip |= LogPerformanceWarning(
                "BestPractices-vkCreateImage-non-transient-ms-image", device, error_obj.location,
                "%s %s Trying to create a multisampled image, but pCreateInfo->usage did not have "
                "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. Multisampled images may be resolved on-chip, "
                "and do not need to be backed by physical storage. "
                "TRANSIENT_ATTACHMENT allows tiled GPUs to not back the multisampled image with physical memory.",
                VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG));
        }
    }

    if (VendorCheckEnabled(kBPVendorArm) && pCreateInfo->samples > kMaxEfficientSamplesArm) {
        skip |= LogPerformanceWarning(
            "BestPractices-Arm-vkCreateImage-too-large-sample-count", device, error_obj.location,
            "%s Trying to create an image with %u samples. "
            "The hardware revision may not have full throughput for framebuffers with more than %u samples.",
            VendorSpecificTag(kBPVendorArm), static_cast<uint32_t>(pCreateInfo->samples), kMaxEfficientSamplesArm);
    }

    if (VendorCheckEnabled(kBPVendorIMG) && pCreateInfo->samples > kMaxEfficientSamplesImg) {
        skip |= LogPerformanceWarning(
            "BestPractices-IMG-vkCreateImage-too-large-sample-count", device, error_obj.location,
            "%s Trying to create an image with %u samples. "
            "The device may not have full support for true multisampling for images with more than %u samples. "
            "XT devices support up to 8 samples, XE up to 4 samples.",
            VendorSpecificTag(kBPVendorIMG), static_cast<uint32_t>(pCreateInfo->samples), kMaxEfficientSamplesImg);
    }

    if (VendorCheckEnabled(kBPVendorIMG) && (pCreateInfo->format == VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG ||
                                             pCreateInfo->format == VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)) {
        skip |= LogPerformanceWarning("BestPractices-IMG-Texture-Format-PVRTC-Outdated", device, error_obj.location,
                                      "%s Trying to create an image with a PVRTC format. Both PVRTC1 and PVRTC2 "
                                      "are slower than standard image formats on PowerVR GPUs, prefer ETC, BC, ASTC, etc.",
                                      VendorSpecificTag(kBPVendorIMG));
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        if ((pCreateInfo->usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT)) {
            skip |= LogPerformanceWarning("BestPractices-AMD-vkImage-AvoidConcurrentRenderTargets", device, error_obj.location,
                                          "%s Trying to create an image as a render target with VK_SHARING_MODE_CONCURRENT. "
                                          "Using a SHARING_MODE_CONCURRENT "
                                          "is not recommended with color and depth targets",
                                          VendorSpecificTag(kBPVendorAMD));
        }

        if ((pCreateInfo->usage &
             (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)) {
            skip |=
                LogPerformanceWarning("BestPractices-AMD-vkImage-DontUseMutableRenderTargets", device, error_obj.location,
                                      "%s Trying to create an image as a render target with VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT. "
                                      "Using a MUTABLE_FORMAT is not recommended with color, depth, and storage targets",
                                      VendorSpecificTag(kBPVendorAMD));
        }

        if ((pCreateInfo->usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) &&
            (pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT)) {
            skip |=
                LogPerformanceWarning("BestPractices-AMD-vkImage-DontUseStorageRenderTargets", device, error_obj.location,
                                      "%s Trying to create an image as a render target with VK_IMAGE_USAGE_STORAGE_BIT. Using a "
                                      "VK_IMAGE_USAGE_STORAGE_BIT is not recommended with color and depth targets",
                                      VendorSpecificTag(kBPVendorAMD));
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
            skip |= LogPerformanceWarning("BestPractices-NVIDIA-CreateImage-TilingLinear", device, error_obj.location,
                                          "%s Trying to create an image with tiling VK_IMAGE_TILING_LINEAR. "
                                          "Use VK_IMAGE_TILING_OPTIMAL instead.",
                                          VendorSpecificTag(kBPVendorNVIDIA));
        }

        if (pCreateInfo->format == VK_FORMAT_D32_SFLOAT || pCreateInfo->format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
            skip |=
                LogPerformanceWarning("BestPractices-NVIDIA-CreateImage-Depth32Format", device, error_obj.location,
                                      "%s Trying to create an image with a 32-bit depth format. Use VK_FORMAT_D24_UNORM_S8_UINT or "
                                      "VK_FORMAT_D16_UNORM instead, unless the extra precision is needed.",
                                      VendorSpecificTag(kBPVendorNVIDIA));
        }
    }

    return skip;
}

void BestPractices::QueueValidateImageView(QueueCallbacks& funcs, Func command, vvl::ImageView* view,
                                           IMAGE_SUBRESOURCE_USAGE_BP usage) {
    if (view) {
        auto image_state = std::static_pointer_cast<bp_state::Image>(view->image_state);
        QueueValidateImage(funcs, command, image_state, usage, view->normalized_subresource_range);
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, Func command, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceRange& subresource_range) {
    // If we're viewing a 3D slice, ignore base array layer.
    // The entire 3D subresource is accessed as one atomic unit.
    const uint32_t base_array_layer = state->create_info.imageType == VK_IMAGE_TYPE_3D ? 0 : subresource_range.baseArrayLayer;

    const uint32_t max_layers = state->create_info.arrayLayers - base_array_layer;
    const uint32_t array_layers = std::min(subresource_range.layerCount, max_layers);
    const uint32_t max_levels = state->create_info.mipLevels - subresource_range.baseMipLevel;
    const uint32_t mip_levels = std::min(state->create_info.mipLevels, max_levels);

    for (uint32_t layer = 0; layer < array_layers; layer++) {
        for (uint32_t level = 0; level < mip_levels; level++) {
            QueueValidateImage(funcs, command, state, usage, layer + base_array_layer, level + subresource_range.baseMipLevel);
        }
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, Func command, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, const VkImageSubresourceLayers& subresource_layers) {
    const uint32_t max_layers = state->create_info.arrayLayers - subresource_layers.baseArrayLayer;
    const uint32_t array_layers = std::min(subresource_layers.layerCount, max_layers);

    for (uint32_t layer = 0; layer < array_layers; layer++) {
        QueueValidateImage(funcs, command, state, usage, layer + subresource_layers.baseArrayLayer, subresource_layers.mipLevel);
    }
}

void BestPractices::QueueValidateImage(QueueCallbacks& funcs, Func command, std::shared_ptr<bp_state::Image>& state,
                                       IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level) {
    funcs.emplace_back([this, command, state, usage, array_layer, mip_level](const vvl::Device& vst, const vvl::Queue& qs,
                                                                             const vvl::CommandBuffer& cbs) -> bool {
        ValidateImageInQueue(qs, cbs, command, *state, usage, array_layer, mip_level);
        return false;
    });
}

void BestPractices::ValidateImageInQueueArmImg(Func command, const bp_state::Image& image, IMAGE_SUBRESOURCE_USAGE_BP last_usage,
                                               IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level) {
    // Swapchain images are implicitly read so clear after store is expected.
    const Location loc(command);
    if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED && last_usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_STORED &&
        !image.IsSwapchainImage()) {
        LogPerformanceWarning(
            "BestPractices-RenderPass-redundant-store", device, loc,
            "%s %s Subresource (arrayLayer: %u, mipLevel: %u) of image was cleared as part of LOAD_OP_CLEAR, but last time "
            "image was used, it was written to with STORE_OP_STORE. "
            "Storing to the image is probably redundant in this case, and wastes bandwidth on tile-based "
            "architectures.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level);
    } else if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_CLEARED && last_usage == IMAGE_SUBRESOURCE_USAGE_BP::CLEARED) {
        LogPerformanceWarning(
            "BestPractices-RenderPass-redundant-clear", device, loc,
            "%s %s Subresource (arrayLayer: %u, mipLevel: %u) of image was cleared as part of LOAD_OP_CLEAR, but last time "
            "image was used, it was written to with vkCmdClear*Image(). "
            "Clearing the image with vkCmdClear*Image() is probably redundant in this case, and wastes bandwidth on "
            "tile-based architectures.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level);
    } else if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE &&
               (last_usage == IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE || last_usage == IMAGE_SUBRESOURCE_USAGE_BP::CLEARED ||
                last_usage == IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE || last_usage == IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE)) {
        const char* last_cmd = nullptr;
        const char* vuid = nullptr;
        const char* suggestion = nullptr;

        switch (last_usage) {
            case IMAGE_SUBRESOURCE_USAGE_BP::BLIT_WRITE:
                vuid = "BestPractices-RenderPass-blitimage-loadopload";
                last_cmd = "vkCmdBlitImage";
                suggestion =
                    "The blit is probably redundant in this case, and wastes bandwidth on tile-based architectures. "
                    "Rather than blitting, just render the source image in a fragment shader in this render pass, "
                    "which avoids the memory roundtrip.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::CLEARED:
                vuid = "BestPractices-RenderPass-inefficient-clear";
                last_cmd = "vkCmdClear*Image";
                suggestion =
                    "Clearing the image with vkCmdClear*Image() is probably redundant in this case, and wastes bandwidth on "
                    "tile-based architectures. "
                    "Use LOAD_OP_CLEAR instead to clear the image for free.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::COPY_WRITE:
                vuid = "BestPractices-RenderPass-copyimage-loadopload";
                last_cmd = "vkCmdCopy*Image";
                suggestion =
                    "The copy is probably redundant in this case, and wastes bandwidth on tile-based architectures. "
                    "Rather than copying, just render the source image in a fragment shader in this render pass, "
                    "which avoids the memory roundtrip.";
                break;
            case IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_WRITE:
                vuid = "BestPractices-RenderPass-resolveimage-loadopload";
                last_cmd = "vkCmdResolveImage";
                suggestion =
                    "The resolve is probably redundant in this case, and wastes a lot of bandwidth on tile-based architectures. "
                    "Rather than resolving, and then loading, try to keep rendering in the same render pass, "
                    "which avoids the memory roundtrip.";
                break;
            default:
                break;
        }

        LogPerformanceWarning(
            vuid, device, loc,
            "%s %s Subresource (arrayLayer: %u, mipLevel: %u) of image was loaded to tile as part of LOAD_OP_LOAD, but last "
            "time image was used, it was written to with %s. %s",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorIMG), array_layer, mip_level, last_cmd, suggestion);
    }
}

void BestPractices::ValidateImageInQueue(const vvl::Queue& qs, const vvl::CommandBuffer& cbs, Func command, bp_state::Image& state,
                                         IMAGE_SUBRESOURCE_USAGE_BP usage, uint32_t array_layer, uint32_t mip_level) {
    auto queue_family = qs.queue_family_index;
    auto last_usage = state.UpdateUsage(array_layer, mip_level, usage, queue_family);

    // Concurrent sharing usage of image with exclusive sharing mode
    if (state.create_info.sharingMode == VK_SHARING_MODE_EXCLUSIVE && last_usage.queue_family_index != queue_family) {
        // if UNDEFINED then first use/acquisition of subresource
        if (last_usage.type != IMAGE_SUBRESOURCE_USAGE_BP::UNDEFINED) {
            // If usage might read from the subresource, as contents are undefined
            // so write only is fine
            if (usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE || usage == IMAGE_SUBRESOURCE_USAGE_BP::BLIT_READ ||
                usage == IMAGE_SUBRESOURCE_USAGE_BP::COPY_READ || usage == IMAGE_SUBRESOURCE_USAGE_BP::DESCRIPTOR_ACCESS ||
                usage == IMAGE_SUBRESOURCE_USAGE_BP::RESOLVE_READ) {
                Location loc(command);
                LogWarning(
                    "BestPractices-ConcurrentUsageOfExclusiveImage", state.Handle(), loc,
                    "Subresource (arrayLayer: %" PRIu32 ", mipLevel: %" PRIu32 ") of image is used on queue family index %" PRIu32
                    " after being used on "
                    "queue family index %" PRIu32
                    ", "
                    "but has VK_SHARING_MODE_EXCLUSIVE, and has not been acquired and released with a ownership transfer operation",
                    array_layer, mip_level, queue_family, last_usage.queue_family_index);
            }
        }
    }

    // When image was discarded with StoreOpDontCare but is now being read with LoadOpLoad
    if (last_usage.type == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_DISCARDED &&
        usage == IMAGE_SUBRESOURCE_USAGE_BP::RENDER_PASS_READ_TO_TILE) {
        Location loc(command);
        LogWarning("BestPractices-StoreOpDontCareThenLoadOpLoad", device, loc,
                   "Trying to load an attachment with LOAD_OP_LOAD that was previously stored with STORE_OP_DONT_CARE. This may "
                   "result in undefined behaviour.");
    }

    if (VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorIMG)) {
        ValidateImageInQueueArmImg(command, state, last_usage.type, usage, array_layer, mip_level);
    }
}

std::shared_ptr<vvl::Image> BestPractices::CreateImageState(VkImage handle, const VkImageCreateInfo* create_info,
                                                            VkFormatFeatureFlags2 features) {
    return std::make_shared<bp_state::Image>(*this, handle, create_info, features);
}

std::shared_ptr<vvl::Image> BestPractices::CreateImageState(VkImage handle, const VkImageCreateInfo* create_info,
                                                            VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                            VkFormatFeatureFlags2 features) {
    return std::make_shared<bp_state::Image>(*this, handle, create_info, swapchain, swapchain_index, features);
}
