//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkLinux.cpp:
//    Implements the class methods for DisplayVkLinux.
//

#include "libANGLE/renderer/vulkan/linux/DisplayVkLinux.h"

#include "common/linux/dma_buf_utils.h"
#include "libANGLE/renderer/vulkan/linux/DeviceVkLinux.h"
#include "libANGLE/renderer/vulkan/linux/DmaBufImageSiblingVkLinux.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

DisplayVkLinux::DisplayVkLinux(const egl::DisplayState &state)
    : DisplayVk(state), mDrmFormatsInitialized(false)
{}

DeviceImpl *DisplayVkLinux::createDevice()
{
    return new DeviceVkLinux(this);
}

ExternalImageSiblingImpl *DisplayVkLinux::createExternalImageSibling(
    const gl::Context *context,
    EGLenum target,
    EGLClientBuffer buffer,
    const egl::AttributeMap &attribs)
{
    switch (target)
    {
        case EGL_LINUX_DMA_BUF_EXT:
            ASSERT(context == nullptr);
            ASSERT(buffer == nullptr);
            return new DmaBufImageSiblingVkLinux(attribs);

        default:
            return DisplayVk::createExternalImageSibling(context, target, buffer, attribs);
    }
}

// Returns the list of DRM modifiers that a VkFormat supports
std::vector<VkDrmFormatModifierPropertiesEXT> DisplayVkLinux::GetDrmModifiers(
    const DisplayVk *displayVk,
    VkFormat vkFormat)
{
    vk::Renderer *renderer = displayVk->getRenderer();

    // Query list of drm format modifiers compatible with VkFormat.
    VkDrmFormatModifierPropertiesListEXT formatModifierPropertiesList = {};
    formatModifierPropertiesList.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
    formatModifierPropertiesList.drmFormatModifierCount = 0;

    VkFormatProperties2 formatProperties = {};
    formatProperties.sType               = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    formatProperties.pNext               = &formatModifierPropertiesList;

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(), vkFormat,
                                         &formatProperties);

    std::vector<VkDrmFormatModifierPropertiesEXT> formatModifierProperties(
        formatModifierPropertiesList.drmFormatModifierCount);
    formatModifierPropertiesList.pDrmFormatModifierProperties = formatModifierProperties.data();

    vkGetPhysicalDeviceFormatProperties2(renderer->getPhysicalDevice(), vkFormat,
                                         &formatProperties);

    return formatModifierProperties;
}

// Returns true if that VkFormat has at least on format modifier in its properties
bool DisplayVkLinux::SupportsDrmModifiers(VkPhysicalDevice device, VkFormat vkFormat)
{
    // Query list of drm format modifiers compatible with VkFormat.
    VkDrmFormatModifierPropertiesListEXT formatModifierPropertiesList = {};
    formatModifierPropertiesList.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
    formatModifierPropertiesList.drmFormatModifierCount = 0;

    VkFormatProperties2 formatProperties = {};
    formatProperties.sType               = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    formatProperties.pNext               = &formatModifierPropertiesList;

    vkGetPhysicalDeviceFormatProperties2(device, vkFormat, &formatProperties);

    // If there is at least one DRM format modifier, it is supported
    return formatModifierPropertiesList.drmFormatModifierCount > 0;
}

// Returns a list of VkFormats supporting at least one DRM format modifier
std::vector<VkFormat> DisplayVkLinux::GetVkFormatsWithDrmModifiers(const vk::Renderer *renderer)
{
    std::vector<VkFormat> vkFormats;
    for (size_t formatIndex = 1; formatIndex < angle::kNumANGLEFormats; ++formatIndex)
    {
        const vk::Format &format = renderer->getFormat(angle::FormatID(formatIndex));
        VkFormat vkFormat =
            format.getActualImageVkFormat(renderer, rx::vk::ImageAccess::Renderable);

        if (vkFormat != VK_FORMAT_UNDEFINED &&
            SupportsDrmModifiers(renderer->getPhysicalDevice(), vkFormat))
        {
            vkFormats.push_back(vkFormat);
        }
    }

    return vkFormats;
}

// Returns a list of supported DRM formats
std::vector<EGLint> DisplayVkLinux::GetDrmFormats(const vk::Renderer *renderer)
{
    std::unordered_set<EGLint> drmFormatsSet;
    for (VkFormat vkFormat : GetVkFormatsWithDrmModifiers(renderer))
    {
        std::vector<EGLint> drmFormats = angle::VkFormatToDrmFourCCFormat(vkFormat);
        for (EGLint drmFormat : drmFormats)
        {
            drmFormatsSet.insert(drmFormat);
        }
    }

    std::vector<EGLint> drmFormats;
    std::copy(std::begin(drmFormatsSet), std::end(drmFormatsSet), std::back_inserter(drmFormats));

    return drmFormats;
}

bool DisplayVkLinux::supportsDmaBufFormat(EGLint format) const
{
    return std::find(std::begin(mDrmFormats), std::end(mDrmFormats), format) !=
           std::end(mDrmFormats);
}

egl::Error DisplayVkLinux::queryDmaBufFormats(EGLint maxFormats,
                                              EGLint *formats,
                                              EGLint *numFormats)
{
    if (!mDrmFormatsInitialized)
    {
        mDrmFormats            = GetDrmFormats(getRenderer());
        mDrmFormatsInitialized = true;
    }

    EGLint formatsSize = static_cast<EGLint>(mDrmFormats.size());
    *numFormats        = formatsSize;
    if (maxFormats > 0)
    {
        // Do not copy data beyond the limits of the vector
        maxFormats = std::min(maxFormats, formatsSize);
        std::memcpy(formats, mDrmFormats.data(), maxFormats * sizeof(EGLint));
    }

    return egl::NoError();
}

// Queries DRM format modifiers associated to `drmFormat`.
// When `maxModifiers` is zero, it will only return the number of modifiers associated to
// `drmFormat` using the out parameter `numModifiers`. When `maxModifiers` is greater than zero, it
// will put that number of DRM format modifiers into the out parameter `modifiers`.
egl::Error DisplayVkLinux::queryDmaBufModifiers(EGLint drmFormat,
                                                EGLint maxModifiers,
                                                EGLuint64KHR *modifiers,
                                                EGLBoolean *externalOnly,
                                                EGLint *numModifiers)
{
    // A DRM format may correspond to multiple Vulkan formats
    std::vector<VkFormat> vkFormats = angle::DrmFourCCFormatToVkFormats(drmFormat);

    std::vector<EGLuint64KHR> drmModifiers;
    // Collect DRM format modifiers common to all those Vulkan formats
    for (size_t i = 0; i < vkFormats.size(); ++i)
    {
        VkFormat vkFmt = vkFormats[i];

        std::vector<VkDrmFormatModifierPropertiesEXT> vkDrmMods = GetDrmModifiers(this, vkFmt);

        std::vector<EGLuint64KHR> drmMods(vkDrmMods.size());
        std::transform(std::begin(vkDrmMods), std::end(vkDrmMods), std::begin(drmMods),
                       [](VkDrmFormatModifierPropertiesEXT props) {
                           return static_cast<EGLuint64KHR>(props.drmFormatModifier);
                       });
        std::sort(std::begin(drmMods), std::end(drmMods));

        if (i == 0)
        {
            // Just take the modifiers for the first format
            drmModifiers = drmMods;
        }
        else
        {
            // Intersect the modifiers of all the other associated Vulkan formats
            std::vector<EGLuint64KHR> prevMods = drmModifiers;
            drmModifiers.clear();
            std::set_intersection(std::begin(drmMods), std::end(drmMods), std::begin(prevMods),
                                  std::end(prevMods), std::back_inserter(drmModifiers));
        }
    }

    EGLint drmModifiersSize = static_cast<EGLint>(drmModifiers.size());

    *numModifiers = drmModifiersSize;
    if (maxModifiers > 0)
    {
        maxModifiers = std::min(maxModifiers, drmModifiersSize);
        std::memcpy(modifiers, drmModifiers.data(), maxModifiers * sizeof(drmModifiers[0]));
    }

    return egl::NoError();
}
}  // namespace rx
