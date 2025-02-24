/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "stateless/stateless_validation.h"

namespace stateless {

bool Device::manual_PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                     const Context &context) const {
    // Validation for pAttachments which is excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        skip |= context.ValidateArray(create_info_loc.dot(Field::attachmentCount), error_obj.location.dot(Field::pAttachments),
                                      pCreateInfo->attachmentCount, &pCreateInfo->pAttachments, false, true, kVUIDUndefined,
                                      "VUID-VkFramebufferCreateInfo-flags-02778");
        // VUID-VkFramebufferCreateInfo-flags-02778 above is already checking if pAttachments is NULL
        if (pCreateInfo->pAttachments) {
            for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
                if (pCreateInfo->pAttachments[i] == VK_NULL_HANDLE) {
                    skip |=
                        LogError("VUID-VkFramebufferCreateInfo-flags-02778", device, error_obj.location.dot(Field::pAttachments, i),
                                 "is VK_NULL_HANDLE, but must be a valid VkImageView handle");
                }
            }
        }
    } else {
        if (!enabled_features.imagelessFramebuffer) {
            skip |= LogError("VUID-VkFramebufferCreateInfo-flags-03189", device, create_info_loc.dot(Field::flags),
                             "includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, but the imagelessFramebuffer feature is not enabled.");
        }

        const auto *framebuffer_attachments_create_info =
            vku::FindStructInPNextChain<VkFramebufferAttachmentsCreateInfo>(pCreateInfo->pNext);
        if (framebuffer_attachments_create_info == nullptr) {
            skip |= LogError("VUID-VkFramebufferCreateInfo-flags-03190", device, create_info_loc.dot(Field::flags),
                             "includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, but no instance of VkFramebufferAttachmentsCreateInfo "
                             "is present in the pNext chain.");
        } else {
            if (framebuffer_attachments_create_info->attachmentImageInfoCount != 0 &&
                framebuffer_attachments_create_info->attachmentImageInfoCount != pCreateInfo->attachmentCount) {
                skip |= LogError("VUID-VkFramebufferCreateInfo-flags-03191", device,
                                 create_info_loc.pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::attachmentImageInfoCount),
                                 "is %" PRIu32 " which is not equal to pCreateInfo->attachmentCount (%" PRIu32 ").",
                                 framebuffer_attachments_create_info->attachmentImageInfoCount, pCreateInfo->attachmentCount);
            }
        }
    }

    // Verify FB dimensions are greater than zero
    if (pCreateInfo->width == 0) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-width-00885", device, create_info_loc.dot(Field::width), "is zero.");
    }
    if (pCreateInfo->height == 0) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-height-00887", device, create_info_loc.dot(Field::height), "is zero.");
    }
    if (pCreateInfo->layers == 0) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-layers-00889", device, create_info_loc.dot(Field::layers), "is zero.");
    }

    if (pCreateInfo->width > device_limits.maxFramebufferWidth) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-width-00886", device, create_info_loc.dot(Field::width),
                         "(%" PRIu32 ") is greater than maxFramebufferWidth (%" PRIu32 ").", pCreateInfo->width,
                         device_limits.maxFramebufferWidth);
    }
    if (pCreateInfo->height > device_limits.maxFramebufferHeight) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-height-00888", device, create_info_loc.dot(Field::height),
                         "(%" PRIu32 ") is greater than maxFramebufferHeight (%" PRIu32 ").", pCreateInfo->height,
                         device_limits.maxFramebufferHeight);
    }
    if (pCreateInfo->layers > device_limits.maxFramebufferLayers) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-layers-00890", device, create_info_loc.dot(Field::layers),
                         "(%" PRIu32 ") is greater than maxFramebufferLayers (%" PRIu32 ").", pCreateInfo->layers,
                         device_limits.maxFramebufferLayers);
    }

    return skip;
}
}  // namespace stateless
