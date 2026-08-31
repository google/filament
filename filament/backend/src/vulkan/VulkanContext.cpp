/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "VulkanContext.h"

#include "VulkanCommands.h"
#include "VulkanConstants.h"
#include "VulkanHandles.h"
#include "VulkanMemory.h"
#include "VulkanTexture.h"

#include <backend/PixelBufferDescriptor.h>

#include <utils/Panic.h>

#include <algorithm> // for std::max
#include <cstring>

using namespace bluevk;

namespace {

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT* cbdata,
        void* pUserData) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        FVK_LOGE << "VULKAN ERROR: (" << cbdata->pMessageIdName << ") " << cbdata->pMessage;
    } else {
        // TODO: emit best practices warnings about aggressive pipeline barriers.
        if (strstr(cbdata->pMessage, "ALL_GRAPHICS_BIT") ||
                strstr(cbdata->pMessage, "ALL_COMMANDS_BIT")) {
            return VK_FALSE;
        }
        FVK_LOGW << "VULKAN WARNING: (" << cbdata->pMessageIdName << ") " << cbdata->pMessage;
    }
    FVK_LOGE << "";
    return VK_FALSE;
}
#endif

} // end anonymous namespace

namespace filament::backend {

void VulkanContext::DebugUtils::init(VkInstance instance, VkDevice device, bool enabled) {
    if (!enabled) {
        return;
    }
    mInstance = instance;
    mDevice = device;

#if FVK_ENABLED(FVK_DEBUG_VALIDATION)
    VkDebugUtilsMessengerCreateInfoEXT const createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = debugUtilsCallback,
    };
    VkResult result =
            vkCreateDebugUtilsMessengerEXT(instance, &createInfo, VKALLOC, &mDebugMessenger);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "Unable to create Vulkan debug messenger. error=" << static_cast<int32_t>(result);
#endif
}

void VulkanContext::DebugUtils::terminate() {
    if (mDebugMessenger != VK_NULL_HANDLE) {
        vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, VKALLOC);
        mDebugMessenger = VK_NULL_HANDLE;
    }
    mInstance = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

void VulkanContext::DebugUtils::setName(VkObjectType type, uint64_t handle,
        char const* name) const {
    if (mDevice == VK_NULL_HANDLE) {
        return;
    }
    VkDebugUtilsObjectNameInfoEXT const info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name,
    };
    vkSetDebugUtilsObjectNameEXT(mDevice, &info);
}

VkImage VulkanAttachment::getImage() const {
    return texture ? texture->getVkImage() : VK_NULL_HANDLE;
}

VkFormat VulkanAttachment::getFormat() const {
    return texture ? texture->getVkFormat() : VK_FORMAT_UNDEFINED;
}

VulkanLayout VulkanAttachment::getLayout() const {
    return texture ? texture->getLayout(layer, level) : VulkanLayout::UNDEFINED;
}

VkExtent2D VulkanAttachment::getExtent2D() const {
    assert_invariant(texture);
    return { std::max(1u, texture->width >> level), std::max(1u, texture->height >> level) };
}

VkImageView VulkanAttachment::getImageView() {
    assert_invariant(texture);
    VkImageSubresourceRange range = getSubresourceRange();
    if (range.layerCount > 1) {
        return texture->getAttachmentView(range, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
    }
    return texture->getAttachmentView(range, VK_IMAGE_VIEW_TYPE_2D);
}

bool VulkanAttachment::isDepth() const {
    return texture->getImageAspect() & VK_IMAGE_ASPECT_DEPTH_BIT;
}

VkImageSubresourceRange VulkanAttachment::getSubresourceRange() const {
    assert_invariant(texture);
    return {
        .aspectMask = texture->getImageAspect(),
        .baseMipLevel = uint32_t(level),
        .levelCount = 1,
        .baseArrayLayer = uint32_t(layer),
        .layerCount = layerCount,
    };
}

} // namespace filament::backend
