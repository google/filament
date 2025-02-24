//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VkImageImageSiblingVk.cpp: Implements VkImageImageSiblingVk.

#include "libANGLE/renderer/vulkan/VkImageImageSiblingVk.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

VkImageImageSiblingVk::VkImageImageSiblingVk(EGLClientBuffer buffer,
                                             const egl::AttributeMap &attribs)
{
    mVkImage.setHandle(*reinterpret_cast<VkImage *>(buffer));

    ASSERT(attribs.contains(EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE));
    ASSERT(attribs.contains(EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE));
    uint64_t hi = static_cast<uint64_t>(attribs.get(EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE));
    uint64_t lo = static_cast<uint64_t>(attribs.get(EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE));
    const VkImageCreateInfo *info =
        reinterpret_cast<const VkImageCreateInfo *>((hi << 32) | (lo & 0xffffffff));
    ASSERT(info->sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    mVkImageInfo = *info;
    // TODO(penghuang): support extensions.
    mVkImageInfo.pNext = nullptr;
    mInternalFormat = static_cast<GLenum>(attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE, GL_NONE));
}

VkImageImageSiblingVk::~VkImageImageSiblingVk() = default;

egl::Error VkImageImageSiblingVk::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    return angle::ToEGL(initImpl(displayVk), EGL_BAD_PARAMETER);
}

angle::Result VkImageImageSiblingVk::initImpl(DisplayVk *displayVk)
{
    vk::Renderer *renderer = displayVk->getRenderer();

    const angle::FormatID formatID = vk::GetFormatIDFromVkFormat(mVkImageInfo.format);
    ANGLE_VK_CHECK(displayVk, formatID != angle::FormatID::NONE, VK_ERROR_FORMAT_NOT_SUPPORTED);

    const vk::Format &vkFormat = renderer->getFormat(formatID);
    const vk::ImageAccess imageAccess =
        isRenderable(nullptr) ? vk::ImageAccess::Renderable : vk::ImageAccess::SampleOnly;
    const angle::FormatID actualImageFormatID = vkFormat.getActualImageFormatID(imageAccess);
    const angle::Format &format               = angle::Format::Get(actualImageFormatID);

    angle::FormatID intendedFormatID;
    if (mInternalFormat != GL_NONE)
    {
        // If EGL_TEXTURE_INTERNAL_FORMAT_ANGLE is provided for eglCreateImageKHR(),
        // the provided format will be used for mFormat and intendedFormat.
        GLenum type      = gl::GetSizedInternalFormatInfo(format.glInternalFormat).type;
        mFormat          = gl::Format(mInternalFormat, type);
        intendedFormatID = angle::Format::InternalFormatToID(mFormat.info->sizedInternalFormat);
    }
    else
    {
        intendedFormatID = vkFormat.getIntendedFormatID();
        mFormat          = gl::Format(format.glInternalFormat);
    }

    // Create the image
    constexpr bool kIsRobustInitEnabled = false;
    mImage                              = new vk::ImageHelper();
    mImage->init2DWeakReference(displayVk, mVkImage.release(), getSize(), false, intendedFormatID,
                                actualImageFormatID, mVkImageInfo.flags, mVkImageInfo.usage, 1,
                                kIsRobustInitEnabled);

    return angle::Result::Continue;
}

void VkImageImageSiblingVk::onDestroy(const egl::Display *display)
{
    ASSERT(mImage == nullptr);
}

gl::Format VkImageImageSiblingVk::getFormat() const
{
    return mFormat;
}

bool VkImageImageSiblingVk::isRenderable(const gl::Context *context) const
{
    return mVkImageInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}

bool VkImageImageSiblingVk::isTexturable(const gl::Context *context) const
{
    return mVkImageInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT;
}

bool VkImageImageSiblingVk::isYUV() const
{
    return false;
}

bool VkImageImageSiblingVk::hasProtectedContent() const
{
    return false;
}

gl::Extents VkImageImageSiblingVk::getSize() const
{
    return gl::Extents(mVkImageInfo.extent.width, mVkImageInfo.extent.height,
                       mVkImageInfo.extent.depth);
}

size_t VkImageImageSiblingVk::getSamples() const
{
    return 0;
}

// ExternalImageSiblingVk interface
vk::ImageHelper *VkImageImageSiblingVk::getImage() const
{
    return mImage;
}

void VkImageImageSiblingVk::release(vk::Renderer *renderer)
{
    if (mImage != nullptr)
    {
        // TODO: Handle the case where the EGLImage is used in two contexts not in the same share
        // group.  https://issuetracker.google.com/169868803
        mImage->resetImageWeakReference();
        mImage->destroy(renderer);
        SafeDelete(mImage);
    }
}

}  // namespace rx
