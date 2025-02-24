// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryObjectVk.cpp: Defines the class interface for MemoryObjectVk, implementing
// MemoryObjectImpl.

#include "libANGLE/renderer/vulkan/MemoryObjectVk.h"

#include "common/debug.h"
#include "common/vulkan/vk_headers.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "vulkan/vulkan_fuchsia_ext.h"

#if !defined(ANGLE_PLATFORM_WINDOWS)
#    include <unistd.h>
#else
#    include <io.h>
#endif

#if defined(ANGLE_PLATFORM_FUCHSIA)
#    include <zircon/status.h>
#    include <zircon/syscalls.h>
#endif

namespace rx
{

namespace
{

#if defined(ANGLE_PLATFORM_WINDOWS)
int close(int fd)
{
    return _close(fd);
}
#endif

void CloseZirconVmo(zx_handle_t handle)
{
#if defined(ANGLE_PLATFORM_FUCHSIA)
    zx_handle_close(handle);
#else
    UNREACHABLE();
#endif
}

angle::Result DuplicateZirconVmo(ContextVk *contextVk, zx_handle_t handle, zx_handle_t *duplicate)
{
#if defined(ANGLE_PLATFORM_FUCHSIA)
    zx_status_t status = zx_handle_duplicate(handle, ZX_RIGHT_SAME_RIGHTS, duplicate);
    ANGLE_VK_CHECK(contextVk, status == ZX_OK, VK_ERROR_INVALID_EXTERNAL_HANDLE);
    return angle::Result::Continue;
#else
    UNREACHABLE();
    return angle::Result::Stop;
#endif
}

VkExternalMemoryHandleTypeFlagBits ToVulkanHandleType(gl::HandleType handleType)
{
    switch (handleType)
    {
        case gl::HandleType::OpaqueFd:
            return VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
        case gl::HandleType::ZirconVmo:
            return VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA;
        default:
            // Not a memory handle type.
            UNREACHABLE();
            return VK_EXTERNAL_MEMORY_HANDLE_TYPE_FLAG_BITS_MAX_ENUM;
    }
}

}  // namespace

MemoryObjectVk::MemoryObjectVk() {}

MemoryObjectVk::~MemoryObjectVk() = default;

void MemoryObjectVk::onDestroy(const gl::Context *context)
{
    if (mFd != kInvalidFd)
    {
        close(mFd);
        mFd = kInvalidFd;
    }

    if (mZirconHandle != ZX_HANDLE_INVALID)
    {
        CloseZirconVmo(mZirconHandle);
        mZirconHandle = ZX_HANDLE_INVALID;
    }
}

angle::Result MemoryObjectVk::setDedicatedMemory(const gl::Context *context, bool dedicatedMemory)
{
    mDedicatedMemory = dedicatedMemory;
    return angle::Result::Continue;
}

angle::Result MemoryObjectVk::setProtectedMemory(const gl::Context *context, bool protectedMemory)
{
    mProtectedMemory = protectedMemory;
    return angle::Result::Continue;
}

angle::Result MemoryObjectVk::importFd(gl::Context *context,
                                       GLuint64 size,
                                       gl::HandleType handleType,
                                       GLint fd)
{
    ContextVk *contextVk = vk::GetImpl(context);

    switch (handleType)
    {
        case gl::HandleType::OpaqueFd:
            return importOpaqueFd(contextVk, size, fd);

        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }
}

angle::Result MemoryObjectVk::importZirconHandle(gl::Context *context,
                                                 GLuint64 size,
                                                 gl::HandleType handleType,
                                                 GLuint handle)
{
    ContextVk *contextVk = vk::GetImpl(context);

    switch (handleType)
    {
        case gl::HandleType::ZirconVmo:
            return importZirconVmo(contextVk, size, handle);

        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }
}

angle::Result MemoryObjectVk::importOpaqueFd(ContextVk *contextVk, GLuint64 size, GLint fd)
{
    ASSERT(mHandleType == gl::HandleType::InvalidEnum);
    ASSERT(mFd == kInvalidFd);
    ASSERT(fd != kInvalidFd);
    mHandleType = gl::HandleType::OpaqueFd;
    mFd         = fd;
    mSize       = size;
    return angle::Result::Continue;
}

angle::Result MemoryObjectVk::importZirconVmo(ContextVk *contextVk, GLuint64 size, GLuint handle)
{
    ASSERT(mHandleType == gl::HandleType::InvalidEnum);
    ASSERT(mZirconHandle == ZX_HANDLE_INVALID);
    ASSERT(handle != ZX_HANDLE_INVALID);
    mHandleType   = gl::HandleType::ZirconVmo;
    mZirconHandle = handle;
    mSize         = size;
    return angle::Result::Continue;
}

angle::Result MemoryObjectVk::createImage(ContextVk *contextVk,
                                          gl::TextureType type,
                                          size_t levels,
                                          GLenum internalFormat,
                                          const gl::Extents &size,
                                          GLuint64 offset,
                                          vk::ImageHelper *image,
                                          GLbitfield createFlags,
                                          GLbitfield usageFlags,
                                          const void *imageCreateInfoPNext)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    const vk::Format &vkFormat     = renderer->getFormat(internalFormat);
    angle::FormatID actualFormatID = vkFormat.getActualRenderableImageFormatID();

    VkExternalMemoryImageCreateInfo externalMemoryImageCreateInfo = {};
    externalMemoryImageCreateInfo.sType       = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageCreateInfo.pNext       = imageCreateInfoPNext;
    externalMemoryImageCreateInfo.handleTypes = ToVulkanHandleType(mHandleType);

    VkExtent3D vkExtents;
    uint32_t layerCount;
    gl_vk::GetExtentsAndLayerCount(type, size, &vkExtents, &layerCount);

    // ANGLE_external_objects_flags allows create flags to be specified by the application instead
    // of getting defaulted to zero.  Note that the GL enum values constituting the bits of
    // |createFlags| are identical to their corresponding Vulkan value.  There are no additional
    // structs allowed to be chained to VkImageCreateInfo other than
    // VkExternalMemoryImageCreateInfo.
    bool hasProtectedContent = mProtectedMemory;
    ANGLE_TRY(image->initExternal(
        contextVk, type, vkExtents, vkFormat.getIntendedFormatID(), actualFormatID, 1, usageFlags,
        createFlags, vk::ImageLayout::ExternalPreInitialized, &externalMemoryImageCreateInfo,
        gl::LevelIndex(0), static_cast<uint32_t>(levels), layerCount,
        contextVk->isRobustResourceInitEnabled(), hasProtectedContent, vk::YcbcrConversionDesc{},
        nullptr));

    VkMemoryRequirements externalMemoryRequirements;
    image->getImage().getMemoryRequirements(renderer->getDevice(), &externalMemoryRequirements);

    const void *importMemoryInfo                              = nullptr;
    VkMemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo = {};
    if (mDedicatedMemory)
    {
        memoryDedicatedAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
        memoryDedicatedAllocateInfo.image = image->getImage().getHandle();
        importMemoryInfo                  = &memoryDedicatedAllocateInfo;
    }

    VkImportMemoryFdInfoKHR importMemoryFdInfo                         = {};
    VkImportMemoryZirconHandleInfoFUCHSIA importMemoryZirconHandleInfo = {};
    switch (mHandleType)
    {
        case gl::HandleType::OpaqueFd:
            ASSERT(mFd != kInvalidFd);
            importMemoryFdInfo.sType      = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
            importMemoryFdInfo.pNext      = importMemoryInfo;
            importMemoryFdInfo.handleType = ToVulkanHandleType(mHandleType);
            importMemoryFdInfo.fd         = dup(mFd);
            importMemoryInfo              = &importMemoryFdInfo;
            break;
        case gl::HandleType::ZirconVmo:
            ASSERT(mZirconHandle != ZX_HANDLE_INVALID);
            importMemoryZirconHandleInfo.sType =
                VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA;
            importMemoryZirconHandleInfo.pNext      = importMemoryInfo;
            importMemoryZirconHandleInfo.handleType = ToVulkanHandleType(mHandleType);
            ANGLE_TRY(
                DuplicateZirconVmo(contextVk, mZirconHandle, &importMemoryZirconHandleInfo.handle));
            importMemoryInfo = &importMemoryZirconHandleInfo;
            break;
        default:
            UNREACHABLE();
    }

    // TODO(jmadill, spang): Memory sub-allocation. http://anglebug.com/40096464
    ASSERT(offset == 0);
    ASSERT(externalMemoryRequirements.size == mSize);

    VkMemoryPropertyFlags flags = hasProtectedContent ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0;
    ANGLE_TRY(image->initExternalMemory(contextVk, renderer->getMemoryProperties(),
                                        externalMemoryRequirements, 1, &importMemoryInfo,
                                        contextVk->getDeviceQueueIndex(), flags));

    return angle::Result::Continue;
}

}  // namespace rx
