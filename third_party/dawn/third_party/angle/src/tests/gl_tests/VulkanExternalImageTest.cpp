//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VulkanExternalImageTest.cpp : Tests of images allocated externally using Vulkan.

#include "test_utils/ANGLETest.h"

#include "common/debug.h"
#include "test_utils/VulkanHelper.h"
#include "test_utils/gl_raii.h"

namespace angle
{

namespace
{

constexpr int kInvalidFd = -1;

constexpr VkImageUsageFlags kDefaultImageUsageFlags =
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
    VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
constexpr VkImageCreateFlags kDefaultImageCreateFlags = 0;

constexpr VkImageUsageFlags kNoStorageImageUsageFlags =
    kDefaultImageUsageFlags & ~VK_IMAGE_USAGE_STORAGE_BIT;
constexpr VkImageCreateFlags kMutableImageCreateFlags =
    kDefaultImageCreateFlags | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

// List of VkFormat/internalformat combinations Chrome uses.
// This is compiled from the maps in
// components/viz/common/resources/resource_format_utils.cc.
const struct ImageFormatPair
{
    VkFormat vkFormat;
    GLenum internalFormat;
    const char *requiredExtension;
} kChromeFormats[] = {
    {VK_FORMAT_R8G8B8A8_UNORM, GL_RGBA8_OES},                    // RGBA_8888
    {VK_FORMAT_B8G8R8A8_UNORM, GL_BGRA8_EXT},                    // BGRA_8888
    {VK_FORMAT_R4G4B4A4_UNORM_PACK16, GL_RGBA4},                 // RGBA_4444
    {VK_FORMAT_R16G16B16A16_SFLOAT, GL_RGBA16F_EXT},             // RGBA_F16
    {VK_FORMAT_R8_UNORM, GL_R8_EXT},                             // RED_8
    {VK_FORMAT_R5G6B5_UNORM_PACK16, GL_RGB565},                  // RGB_565
    {VK_FORMAT_R16_UNORM, GL_R16_EXT, "GL_EXT_texture_norm16"},  // R16_EXT
    {VK_FORMAT_A2B10G10R10_UNORM_PACK32, GL_RGB10_A2_EXT},       // RGBA_1010102
    {VK_FORMAT_R8_UNORM, GL_ALPHA8_EXT},                         // ALPHA_8
    {VK_FORMAT_R8_UNORM, GL_LUMINANCE8_EXT},                     // LUMINANCE_8
    {VK_FORMAT_R8G8_UNORM, GL_RG8_EXT},                          // RG_88
    {VK_FORMAT_R8G8B8A8_UNORM, GL_RGB8_OES},                     // RGBX_8888
};

struct OpaqueFdTraits
{
    using Handle = int;
    static Handle InvalidHandle() { return kInvalidFd; }

    static const char *MemoryObjectExtension() { return "GL_EXT_memory_object_fd"; }
    static const char *SemaphoreExtension() { return "GL_EXT_semaphore_fd"; }

    static bool CanCreateSemaphore(const VulkanHelper &helper)
    {
        return helper.canCreateSemaphoreOpaqueFd();
    }

    static VkResult CreateSemaphore(VulkanHelper *helper, VkSemaphore *semaphore)
    {
        return helper->createSemaphoreOpaqueFd(semaphore);
    }

    static VkResult ExportSemaphore(VulkanHelper *helper, VkSemaphore semaphore, Handle *handle)
    {
        return helper->exportSemaphoreOpaqueFd(semaphore, handle);
    }

    static void ImportSemaphore(GLuint semaphore, Handle handle)
    {
        glImportSemaphoreFdEXT(semaphore, GL_HANDLE_TYPE_OPAQUE_FD_EXT, handle);
    }

    static bool CanCreateImage(const VulkanHelper &helper,
                               VkFormat format,
                               VkImageType type,
                               VkImageTiling tiling,
                               VkImageCreateFlags createFlags,
                               VkImageUsageFlags usageFlags)
    {
        return helper.canCreateImageOpaqueFd(format, type, tiling, createFlags, usageFlags);
    }

    static VkResult CreateImage2D(VulkanHelper *helper,
                                  VkFormat format,
                                  VkImageCreateFlags createFlags,
                                  VkImageUsageFlags usageFlags,
                                  const void *imageCreateInfoPNext,
                                  VkExtent3D extent,
                                  VkImage *imageOut,
                                  VkDeviceMemory *deviceMemoryOut,
                                  VkDeviceSize *deviceMemorySizeOut)
    {
        return helper->createImage2DOpaqueFd(format, createFlags, usageFlags, imageCreateInfoPNext,
                                             extent, imageOut, deviceMemoryOut,
                                             deviceMemorySizeOut);
    }

    static VkResult ExportMemory(VulkanHelper *helper, VkDeviceMemory deviceMemory, Handle *handle)
    {
        return helper->exportMemoryOpaqueFd(deviceMemory, handle);
    }

    static void ImportMemory(GLuint memoryObject, GLuint64 size, Handle handle)
    {
        glImportMemoryFdEXT(memoryObject, size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, handle);
    }
};

struct FuchsiaTraits
{
    using Handle = zx_handle_t;

    static Handle InvalidHandle() { return ZX_HANDLE_INVALID; }

    static const char *MemoryObjectExtension() { return "GL_ANGLE_memory_object_fuchsia"; }
    static const char *SemaphoreExtension() { return "GL_ANGLE_semaphore_fuchsia"; }

    static bool CanCreateSemaphore(const VulkanHelper &helper)
    {
        return helper.canCreateSemaphoreZirconEvent();
    }

    static VkResult CreateSemaphore(VulkanHelper *helper, VkSemaphore *semaphore)
    {
        return helper->createSemaphoreZirconEvent(semaphore);
    }

    static VkResult ExportSemaphore(VulkanHelper *helper, VkSemaphore semaphore, Handle *handle)
    {
        return helper->exportSemaphoreZirconEvent(semaphore, handle);
    }

    static void ImportSemaphore(GLuint semaphore, Handle handle)
    {
        glImportSemaphoreZirconHandleANGLE(semaphore, GL_HANDLE_TYPE_ZIRCON_EVENT_ANGLE, handle);
    }

    static bool CanCreateImage(const VulkanHelper &helper,
                               VkFormat format,
                               VkImageType type,
                               VkImageTiling tiling,
                               VkImageCreateFlags createFlags,
                               VkImageUsageFlags usageFlags)
    {
        return helper.canCreateImageZirconVmo(format, type, tiling, createFlags, usageFlags);
    }

    static VkResult CreateImage2D(VulkanHelper *helper,
                                  VkFormat format,
                                  VkImageCreateFlags createFlags,
                                  VkImageUsageFlags usageFlags,
                                  const void *imageCreateInfoPNext,
                                  VkExtent3D extent,
                                  VkImage *imageOut,
                                  VkDeviceMemory *deviceMemoryOut,
                                  VkDeviceSize *deviceMemorySizeOut)
    {
        return helper->createImage2DZirconVmo(format, createFlags, usageFlags, imageCreateInfoPNext,
                                              extent, imageOut, deviceMemoryOut,
                                              deviceMemorySizeOut);
    }

    static VkResult ExportMemory(VulkanHelper *helper, VkDeviceMemory deviceMemory, Handle *handle)
    {
        return helper->exportMemoryZirconVmo(deviceMemory, handle);
    }

    static void ImportMemory(GLuint memoryObject, GLuint64 size, Handle handle)
    {
        glImportMemoryZirconHandleANGLE(memoryObject, size, GL_HANDLE_TYPE_ZIRCON_VMO_ANGLE,
                                        handle);
    }
};

VkImageLayout GetPostReleaseVulkanLayout(GLenum glLayout)
{
    switch (glLayout)
    {
        case GL_NONE:
        case GL_LAYOUT_GENERAL_EXT:
        default:
            return VK_IMAGE_LAYOUT_GENERAL;
        case GL_LAYOUT_COLOR_ATTACHMENT_EXT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case GL_LAYOUT_DEPTH_STENCIL_ATTACHMENT_EXT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case GL_LAYOUT_DEPTH_STENCIL_READ_ONLY_EXT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case GL_LAYOUT_SHADER_READ_ONLY_EXT:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case GL_LAYOUT_TRANSFER_SRC_EXT:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case GL_LAYOUT_TRANSFER_DST_EXT:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case GL_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_EXT:
            return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR;
        case GL_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_EXT:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR;
    }
}

void AdjustCreateFlags(bool useMemoryObjectFlags, VkImageCreateFlags *createFlags)
{
    // If the GL_ANGLE_memory_object_flags extension is not supported, GL assumes that the mutable
    // create flag is specified.
    if (!useMemoryObjectFlags)
    {
        *createFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    // The spec is not clear about the other create flags.
}

constexpr uint32_t kWidth  = 64;
constexpr uint32_t kHeight = 64;

}  // namespace

class VulkanExternalImageTest : public ANGLETest<>
{
  protected:
    VulkanExternalImageTest()
    {
        setWindowWidth(1);
        setWindowHeight(1);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    template <typename Traits>
    void runShouldDrawTest(bool isSwiftshader, bool enableDebugLayers);

    template <typename Traits>
    void runWaitSemaphoresRetainsContentTest(bool isSwiftshader, bool enableDebugLayers);
};

class VulkanExternalImageTestES31 : public VulkanExternalImageTest
{};

template <typename Traits>
void RunShouldImportMemoryTest(VkImageCreateFlags createFlags,
                               VkImageUsageFlags usageFlags,
                               bool isSwiftshader,
                               bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateImage(helper, format, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, createFlags, usageFlags));

    VkImage image                 = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
    VkDeviceSize deviceMemorySize = 0;

    VkExtent3D extent = {1, 1, 1};
    VkResult result   = Traits::CreateImage2D(&helper, format, createFlags, usageFlags, nullptr,
                                              extent, &image, &deviceMemory, &deviceMemorySize);
    EXPECT_EQ(result, VK_SUCCESS);

    typename Traits::Handle memoryHandle = Traits::InvalidHandle();
    result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(memoryHandle, Traits::InvalidHandle());

    {
        GLMemoryObject memoryObject;
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

        // Test that after calling glImportMemoryFdEXT, the parameters of the memory object cannot
        // be changed
        dedicatedMemory = GL_FALSE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }

    EXPECT_GL_NO_ERROR();

    vkDestroyImage(helper.getDevice(), image, nullptr);
    vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
}

// glImportMemoryFdEXT must be able to import a valid opaque fd.
TEST_P(VulkanExternalImageTest, ShouldImportMemoryOpaqueFd)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    RunShouldImportMemoryTest<OpaqueFdTraits>(kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                                              isSwiftshader(), enableDebugLayers());
}

// glImportMemoryZirconHandleANGLE must be able to import a valid vmo.
TEST_P(VulkanExternalImageTest, ShouldImportMemoryZirconVmo)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    RunShouldImportMemoryTest<FuchsiaTraits>(kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                                             isSwiftshader(), enableDebugLayers());
}

template <typename Traits>
void RunShouldImportSemaphoreTest(bool isSwiftshader, bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::SemaphoreExtension()));

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    ANGLE_SKIP_TEST_IF(!Traits::CanCreateSemaphore(helper));

    VkSemaphore vkSemaphore = VK_NULL_HANDLE;
    VkResult result         = helper.createSemaphoreOpaqueFd(&vkSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);

    typename Traits::Handle semaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkSemaphore, &semaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(semaphoreHandle, Traits::InvalidHandle());

    {
        GLSemaphore glSemaphore;
        Traits::ImportSemaphore(glSemaphore, semaphoreHandle);
    }

    EXPECT_GL_NO_ERROR();

    vkDestroySemaphore(helper.getDevice(), vkSemaphore, nullptr);
}

// glImportSemaphoreFdEXT must be able to import a valid opaque fd.
TEST_P(VulkanExternalImageTest, ShouldImportSemaphoreOpaqueFd)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));
    RunShouldImportSemaphoreTest<OpaqueFdTraits>(isSwiftshader(), enableDebugLayers());
}

// glImportSemaphoreZirconHandleANGLE must be able to import a valid handle.
TEST_P(VulkanExternalImageTest, ShouldImportSemaphoreZirconEvent)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    RunShouldImportSemaphoreTest<FuchsiaTraits>(isSwiftshader(), enableDebugLayers());
}

template <typename Traits>
void RunShouldClearTest(bool useMemoryObjectFlags,
                        VkImageCreateFlags createFlags,
                        VkImageUsageFlags usageFlags,
                        bool isSwiftshader,
                        bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    AdjustCreateFlags(useMemoryObjectFlags, &createFlags);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateImage(helper, format, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, createFlags, usageFlags));

    VkImage image                 = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
    VkDeviceSize deviceMemorySize = 0;

    VkExtent3D extent = {1, 1, 1};
    VkResult result   = Traits::CreateImage2D(&helper, format, createFlags, usageFlags, nullptr,
                                              extent, &image, &deviceMemory, &deviceMemorySize);
    EXPECT_EQ(result, VK_SUCCESS);

    typename Traits::Handle memoryHandle = Traits::InvalidHandle();
    result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(memoryHandle, Traits::InvalidHandle());

    {
        GLMemoryObject memoryObject;
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        if (useMemoryObjectFlags)
        {
            glTexStorageMemFlags2DANGLE(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, memoryObject, 0,
                                        createFlags, usageFlags, nullptr);
        }
        else
        {
            glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, memoryObject, 0);
        }

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT);

        EXPECT_PIXEL_NEAR(0, 0, 128, 128, 128, 128, 1.0);
    }

    EXPECT_GL_NO_ERROR();

    vkDestroyImage(helper.getDevice(), image, nullptr);
    vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
}

// Test creating and clearing a simple RGBA8 texture in an opaque fd.
TEST_P(VulkanExternalImageTest, ShouldClearOpaqueFdRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    // http://anglebug.com/42263236
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL() && (IsPixel2() || IsPixel2XL()));
    RunShouldClearTest<OpaqueFdTraits>(false, kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                                       isSwiftshader(), enableDebugLayers());
}

// Test creating and clearing a simple RGBA8 texture in an opaque fd, using
// GL_ANGLE_memory_object_flags.
TEST_P(VulkanExternalImageTest, ShouldClearOpaqueWithFlagsFdRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearTest<OpaqueFdTraits>(true, kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                                       isSwiftshader(), enableDebugLayers());
}

// Test creating and clearing a simple RGBA8 texture without STORAGE usage in an opaque fd.
TEST_P(VulkanExternalImageTest, ShouldClearNoStorageUsageOpaqueFdRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearTest<OpaqueFdTraits>(true, kDefaultImageCreateFlags, kNoStorageImageUsageFlags,
                                       isSwiftshader(), enableDebugLayers());
}

// Test creating and clearing a simple RGBA8 texture without STORAGE usage but with MUTABLE in an
// opaque fd.
TEST_P(VulkanExternalImageTest, ShouldClearMutableNoStorageUsageOpaqueFdRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearTest<OpaqueFdTraits>(true, kMutableImageCreateFlags, kNoStorageImageUsageFlags,
                                       isSwiftshader(), enableDebugLayers());
}

// Test creating and clearing a simple RGBA8 texture in a zircon vmo.
TEST_P(VulkanExternalImageTest, ShouldClearZirconVmoRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    RunShouldClearTest<FuchsiaTraits>(false, kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                                      isSwiftshader(), enableDebugLayers());
}

// Test creating and clearing a simple RGBA8 texture in a zircon vmo, using
// GL_ANGLE_memory_object_flags.
TEST_P(VulkanExternalImageTest, ShouldClearZirconWithFlagsVmoRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearTest<FuchsiaTraits>(true, kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                                      isSwiftshader(), enableDebugLayers());
}

// Test creating and clearing a simple RGBA8 texture without STORAGE usage in a zircon vmo.
TEST_P(VulkanExternalImageTest, ShouldClearNoStorageUsageZirconVmoRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearTest<FuchsiaTraits>(true, kDefaultImageCreateFlags, kNoStorageImageUsageFlags,
                                      isSwiftshader(), enableDebugLayers());
}

// Test creating and clearing a simple RGBA8 texture without STORAGE usage but with MUTABLE in a
// zircon vmo.
TEST_P(VulkanExternalImageTest, ShouldClearMutableNoStorageUsageZirconVmoRGBA8)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearTest<FuchsiaTraits>(true, kMutableImageCreateFlags, kNoStorageImageUsageFlags,
                                      isSwiftshader(), enableDebugLayers());
}

template <typename Traits>
void RunTextureFormatCompatChromiumTest(bool useMemoryObjectFlags,
                                        VkImageCreateFlags createFlags,
                                        VkImageUsageFlags usageFlags,
                                        bool isSwiftshader,
                                        bool enableDebugLayers,
                                        bool isES3)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));

    AdjustCreateFlags(useMemoryObjectFlags, &createFlags);

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);
    for (const ImageFormatPair &format : kChromeFormats)
    {
        // https://crbug.com/angleproject/5046
        if ((format.vkFormat == VK_FORMAT_R4G4B4A4_UNORM_PACK16) && IsIntel())
        {
            continue;
        }

        if (!Traits::CanCreateImage(helper, format.vkFormat, VK_IMAGE_TYPE_2D,
                                    VK_IMAGE_TILING_OPTIMAL, createFlags, usageFlags))
        {
            continue;
        }

        if (format.requiredExtension && !IsGLExtensionEnabled(format.requiredExtension))
        {
            continue;
        }

        if (format.internalFormat == GL_RGB10_A2_EXT && !isES3 &&
            !IsGLExtensionEnabled("GL_EXT_texture_type_2_10_10_10_REV"))
        {
            continue;
        }

        VkImage image                 = VK_NULL_HANDLE;
        VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
        VkDeviceSize deviceMemorySize = 0;

        VkExtent3D extent = {113, 211, 1};
        VkResult result =
            Traits::CreateImage2D(&helper, format.vkFormat, createFlags, usageFlags, nullptr,
                                  extent, &image, &deviceMemory, &deviceMemorySize);
        EXPECT_EQ(result, VK_SUCCESS);

        typename Traits::Handle memoryHandle = Traits::InvalidHandle();
        result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
        EXPECT_EQ(result, VK_SUCCESS);
        EXPECT_NE(memoryHandle, Traits::InvalidHandle());

        {
            GLMemoryObject memoryObject;
            GLint dedicatedMemory = GL_TRUE;
            glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                         &dedicatedMemory);
            Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            if (useMemoryObjectFlags)
            {
                glTexStorageMemFlags2DANGLE(GL_TEXTURE_2D, 1, format.internalFormat, extent.width,
                                            extent.height, memoryObject, 0, createFlags, usageFlags,
                                            nullptr);
            }
            else
            {
                glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, format.internalFormat, extent.width,
                                     extent.height, memoryObject, 0);
            }
        }

        EXPECT_GL_NO_ERROR();

        vkDestroyImage(helper.getDevice(), image, nullptr);
        vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
    }
}

// Test all format combinations used by Chrome import successfully (opaque fd).
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumFd)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    RunTextureFormatCompatChromiumTest<OpaqueFdTraits>(
        false, kDefaultImageCreateFlags, kDefaultImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

// Test all format combinations used by Chrome import successfully (opaque fd), using
// GL_ANGLE_memory_object_flags.
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumWithFlagsFd)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunTextureFormatCompatChromiumTest<OpaqueFdTraits>(
        true, kDefaultImageCreateFlags, kDefaultImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

// Test all format combinations used by Chrome import successfully (opaque fd), without STORAGE
// usage.
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumNoStorageFd)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunTextureFormatCompatChromiumTest<OpaqueFdTraits>(
        true, kDefaultImageCreateFlags, kNoStorageImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

// Test all format combinations used by Chrome import successfully (opaque fd), without STORAGE
// usage but with MUTABLE.
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumMutableNoStorageFd)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));

    // http://anglebug.com/42264218
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsVulkan());

    RunTextureFormatCompatChromiumTest<OpaqueFdTraits>(
        true, kMutableImageCreateFlags, kNoStorageImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

// Test all format combinations used by Chrome import successfully (fuchsia).
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumZirconVmo)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    RunTextureFormatCompatChromiumTest<FuchsiaTraits>(
        false, kDefaultImageCreateFlags, kDefaultImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

// Test all format combinations used by Chrome import successfully (fuchsia), using
// GL_ANGLE_memory_object_flags.
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumWithFlagsZirconVmo)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunTextureFormatCompatChromiumTest<FuchsiaTraits>(
        true, kDefaultImageCreateFlags, kDefaultImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

// Test all format combinations used by Chrome import successfully (fuchsia), without STORAGE usage.
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumNoStorageZirconVmo)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunTextureFormatCompatChromiumTest<FuchsiaTraits>(
        true, kDefaultImageCreateFlags, kNoStorageImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

// Test all format combinations used by Chrome import successfully (fuchsia), without STORAGE usage
// but with MUTABLE.
TEST_P(VulkanExternalImageTest, TextureFormatCompatChromiumMutableNoStorageZirconVmo)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunTextureFormatCompatChromiumTest<FuchsiaTraits>(
        true, kMutableImageCreateFlags, kNoStorageImageUsageFlags, isSwiftshader(),
        enableDebugLayers(), getClientMajorVersion() >= 3);
}

template <typename Traits>
void RunShouldClearWithSemaphoresTest(bool useMemoryObjectFlags,
                                      VkImageCreateFlags createFlags,
                                      VkImageUsageFlags usageFlags,
                                      bool isSwiftshader,
                                      bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));
    ASSERT(EnsureGLExtensionEnabled(Traits::SemaphoreExtension()));

    AdjustCreateFlags(useMemoryObjectFlags, &createFlags);

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateImage(helper, format, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, createFlags, usageFlags));
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateSemaphore(helper));

    VkSemaphore vkAcquireSemaphore = VK_NULL_HANDLE;
    VkResult result                = Traits::CreateSemaphore(&helper, &vkAcquireSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkAcquireSemaphore != VK_NULL_HANDLE);

    VkSemaphore vkReleaseSemaphore = VK_NULL_HANDLE;
    result                         = Traits::CreateSemaphore(&helper, &vkReleaseSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkReleaseSemaphore != VK_NULL_HANDLE);

    typename Traits::Handle acquireSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkAcquireSemaphore, &acquireSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(acquireSemaphoreHandle, Traits::InvalidHandle());

    typename Traits::Handle releaseSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkReleaseSemaphore, &releaseSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(releaseSemaphoreHandle, Traits::InvalidHandle());

    VkImage image                 = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
    VkDeviceSize deviceMemorySize = 0;

    VkExtent3D extent = {1, 1, 1};
    result = Traits::CreateImage2D(&helper, format, createFlags, usageFlags, nullptr, extent,
                                   &image, &deviceMemory, &deviceMemorySize);
    EXPECT_EQ(result, VK_SUCCESS);

    typename Traits::Handle memoryHandle = Traits::InvalidHandle();
    result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(memoryHandle, Traits::InvalidHandle());

    {
        GLMemoryObject memoryObject;
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        if (useMemoryObjectFlags)
        {
            glTexStorageMemFlags2DANGLE(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, memoryObject, 0,
                                        createFlags, usageFlags, nullptr);
        }
        else
        {
            glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, memoryObject, 0);
        }

        GLSemaphore glAcquireSemaphore;
        Traits::ImportSemaphore(glAcquireSemaphore, acquireSemaphoreHandle);

        helper.releaseImageAndSignalSemaphore(image, VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_IMAGE_LAYOUT_GENERAL, vkAcquireSemaphore);

        const GLuint barrierTextures[] = {
            texture,
        };
        constexpr uint32_t textureBarriersCount = std::extent<decltype(barrierTextures)>();
        const GLenum textureSrcLayouts[]        = {
            GL_LAYOUT_GENERAL_EXT,
        };
        constexpr uint32_t textureSrcLayoutsCount = std::extent<decltype(textureSrcLayouts)>();
        static_assert(textureBarriersCount == textureSrcLayoutsCount,
                      "barrierTextures and textureSrcLayouts must be the same length");
        glWaitSemaphoreEXT(glAcquireSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                           textureSrcLayouts);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT);

        GLSemaphore glReleaseSemaphore;
        Traits::ImportSemaphore(glReleaseSemaphore, releaseSemaphoreHandle);

        const GLenum textureDstLayouts[] = {
            GL_LAYOUT_TRANSFER_SRC_EXT,
        };
        constexpr uint32_t textureDstLayoutsCount = std::extent<decltype(textureSrcLayouts)>();
        static_assert(textureBarriersCount == textureDstLayoutsCount,
                      "barrierTextures and textureDstLayouts must be the same length");
        glSignalSemaphoreEXT(glReleaseSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                             textureDstLayouts);

        helper.waitSemaphoreAndAcquireImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            vkReleaseSemaphore);
        uint8_t pixels[4];
        VkOffset3D offset = {};
        helper.readPixels(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, format, offset, extent,
                          pixels, sizeof(pixels));

        EXPECT_NEAR(0x80, pixels[0], 1);
        EXPECT_NEAR(0x80, pixels[1], 1);
        EXPECT_NEAR(0x80, pixels[2], 1);
        EXPECT_NEAR(0x80, pixels[3], 1);
    }

    EXPECT_GL_NO_ERROR();

    vkDeviceWaitIdle(helper.getDevice());
    vkDestroyImage(helper.getDevice(), image, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkAcquireSemaphore, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkReleaseSemaphore, nullptr);
    vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
}

// Test creating and clearing RGBA8 texture in opaque fd with acquire/release.
TEST_P(VulkanExternalImageTest, ShouldClearOpaqueFdWithSemaphores)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    // http://anglebug.com/42263923
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    RunShouldClearWithSemaphoresTest<OpaqueFdTraits>(false, kDefaultImageCreateFlags,
                                                     kDefaultImageUsageFlags, isSwiftshader(),
                                                     enableDebugLayers());
}

// Test creating and clearing RGBA8 texture in opaque fd with acquire/release, using
// GL_ANGLE_memory_object_flags.
TEST_P(VulkanExternalImageTest, ShouldClearOpaqueFdWithSemaphoresWithFlags)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));

    RunShouldClearWithSemaphoresTest<OpaqueFdTraits>(true, kDefaultImageCreateFlags,
                                                     kDefaultImageUsageFlags, isSwiftshader(),
                                                     enableDebugLayers());
}

// Test creating and clearing RGBA8 texture without STORAGE usage in opaque fd with acquire/release.
TEST_P(VulkanExternalImageTest, ShouldClearOpaqueFdWithSemaphoresNoStorage)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));

    RunShouldClearWithSemaphoresTest<OpaqueFdTraits>(true, kDefaultImageCreateFlags,
                                                     kNoStorageImageUsageFlags, isSwiftshader(),
                                                     enableDebugLayers());
}

// Test creating and clearing RGBA8 texture without STORAGE usage but with MUTABLE in opaque fd with
// acquire/release.
TEST_P(VulkanExternalImageTest, ShouldClearOpaqueFdWithSemaphoresMutableNoStorage)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));

    RunShouldClearWithSemaphoresTest<OpaqueFdTraits>(true, kMutableImageCreateFlags,
                                                     kNoStorageImageUsageFlags, isSwiftshader(),
                                                     enableDebugLayers());
}

// Test creating and clearing RGBA8 texture in zircon vmo with acquire/release.
TEST_P(VulkanExternalImageTest, ShouldClearZirconVmoWithSemaphores)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    RunShouldClearWithSemaphoresTest<FuchsiaTraits>(false, kDefaultImageCreateFlags,
                                                    kDefaultImageUsageFlags, isSwiftshader(),
                                                    enableDebugLayers());
}

// Test creating and clearing RGBA8 texture in zircon vmo with acquire/release, using
// GL_ANGLE_memory_object_flags.
TEST_P(VulkanExternalImageTest, ShouldClearZirconVmoWithSemaphoresWithFlags)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearWithSemaphoresTest<FuchsiaTraits>(true, kDefaultImageCreateFlags,
                                                    kDefaultImageUsageFlags, isSwiftshader(),
                                                    enableDebugLayers());
}

// Test creating and clearing RGBA8 texture without STORAGE usage in zircon vmo with
// acquire/release.
TEST_P(VulkanExternalImageTest, ShouldClearZirconVmoWithSemaphoresNoStorage)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearWithSemaphoresTest<FuchsiaTraits>(true, kDefaultImageCreateFlags,
                                                    kNoStorageImageUsageFlags, isSwiftshader(),
                                                    enableDebugLayers());
}

// Test creating and clearing RGBA8 texture without STORAGE usage but with MUTABLE in zircon vmo
// with acquire/release.
TEST_P(VulkanExternalImageTest, ShouldClearZirconVmoWithSemaphoresMutableNoStorage)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));
    RunShouldClearWithSemaphoresTest<FuchsiaTraits>(true, kMutableImageCreateFlags,
                                                    kNoStorageImageUsageFlags, isSwiftshader(),
                                                    enableDebugLayers());
}

template <typename Traits>
void VulkanExternalImageTest::runShouldDrawTest(bool isSwiftshader, bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));
    ASSERT(EnsureGLExtensionEnabled(Traits::SemaphoreExtension()));

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateImage(helper, format, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, kDefaultImageCreateFlags,
                                               kDefaultImageUsageFlags));
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateSemaphore(helper));

    VkSemaphore vkAcquireSemaphore = VK_NULL_HANDLE;
    VkResult result                = Traits::CreateSemaphore(&helper, &vkAcquireSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkAcquireSemaphore != VK_NULL_HANDLE);

    VkSemaphore vkReleaseSemaphore = VK_NULL_HANDLE;
    result                         = Traits::CreateSemaphore(&helper, &vkReleaseSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkReleaseSemaphore != VK_NULL_HANDLE);

    typename Traits::Handle acquireSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkAcquireSemaphore, &acquireSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(acquireSemaphoreHandle, Traits::InvalidHandle());

    typename Traits::Handle releaseSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkReleaseSemaphore, &releaseSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(releaseSemaphoreHandle, Traits::InvalidHandle());

    VkImage image                 = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
    VkDeviceSize deviceMemorySize = 0;

    VkExtent3D extent = {1, 1, 1};
    result =
        Traits::CreateImage2D(&helper, format, kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                              nullptr, extent, &image, &deviceMemory, &deviceMemorySize);
    EXPECT_EQ(result, VK_SUCCESS);

    typename Traits::Handle memoryHandle = Traits::InvalidHandle();
    result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(memoryHandle, Traits::InvalidHandle());

    {
        GLMemoryObject memoryObject;
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, memoryObject, 0);

        GLSemaphore glAcquireSemaphore;
        Traits::ImportSemaphore(glAcquireSemaphore, acquireSemaphoreHandle);

        // Transfer ownership to GL.
        helper.releaseImageAndSignalSemaphore(image, VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_IMAGE_LAYOUT_GENERAL, vkAcquireSemaphore);

        const GLuint barrierTextures[] = {
            texture,
        };
        constexpr uint32_t textureBarriersCount = std::extent<decltype(barrierTextures)>();
        const GLenum textureSrcLayouts[]        = {
            GL_LAYOUT_GENERAL_EXT,
        };
        constexpr uint32_t textureSrcLayoutsCount = std::extent<decltype(textureSrcLayouts)>();
        static_assert(textureBarriersCount == textureSrcLayoutsCount,
                      "barrierTextures and textureSrcLayouts must be the same length");
        glWaitSemaphoreEXT(glAcquireSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                           textureSrcLayouts);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Make the texture red.
        ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
        EXPECT_GL_NO_ERROR();

        // Transfer ownership back to test.
        GLSemaphore glReleaseSemaphore;
        Traits::ImportSemaphore(glReleaseSemaphore, releaseSemaphoreHandle);

        const GLenum textureDstLayouts[] = {
            GL_LAYOUT_TRANSFER_SRC_EXT,
        };
        constexpr uint32_t textureDstLayoutsCount = std::extent<decltype(textureSrcLayouts)>();
        static_assert(textureBarriersCount == textureDstLayoutsCount,
                      "barrierTextures and textureDstLayouts must be the same length");
        glSignalSemaphoreEXT(glReleaseSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                             textureDstLayouts);

        helper.waitSemaphoreAndAcquireImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            vkReleaseSemaphore);

        uint8_t pixels[4];
        VkOffset3D offset = {};
        helper.readPixels(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, format, offset, extent,
                          pixels, sizeof(pixels));

        EXPECT_EQ(0xFF, pixels[0]);
        EXPECT_EQ(0x00, pixels[1]);
        EXPECT_EQ(0x00, pixels[2]);
        EXPECT_EQ(0xFF, pixels[3]);
    }

    EXPECT_GL_NO_ERROR();

    vkDeviceWaitIdle(helper.getDevice());
    vkDestroyImage(helper.getDevice(), image, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkAcquireSemaphore, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkReleaseSemaphore, nullptr);
    vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
}

// Test drawing to RGBA8 texture in opaque fd with acquire/release.
TEST_P(VulkanExternalImageTest, ShouldDrawOpaqueFdWithSemaphores)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    // http://anglebug.com/42263923
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    runShouldDrawTest<OpaqueFdTraits>(isSwiftshader(), enableDebugLayers());
}

// Test drawing to RGBA8 texture in zircon vmo with acquire/release multiple times.
TEST_P(VulkanExternalImageTest, ShouldDrawZirconVmoWithSemaphores)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    runShouldDrawTest<FuchsiaTraits>(isSwiftshader(), enableDebugLayers());
}

template <typename Traits>
void VulkanExternalImageTest::runWaitSemaphoresRetainsContentTest(bool isSwiftshader,
                                                                  bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));
    ASSERT(EnsureGLExtensionEnabled(Traits::SemaphoreExtension()));

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateImage(helper, format, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, kDefaultImageCreateFlags,
                                               kDefaultImageUsageFlags));
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateSemaphore(helper));

    VkSemaphore vkAcquireSemaphore = VK_NULL_HANDLE;
    VkResult result                = Traits::CreateSemaphore(&helper, &vkAcquireSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkAcquireSemaphore != VK_NULL_HANDLE);

    VkSemaphore vkReleaseSemaphore = VK_NULL_HANDLE;
    result                         = Traits::CreateSemaphore(&helper, &vkReleaseSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkReleaseSemaphore != VK_NULL_HANDLE);

    typename Traits::Handle acquireSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkAcquireSemaphore, &acquireSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(acquireSemaphoreHandle, Traits::InvalidHandle());

    typename Traits::Handle releaseSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkReleaseSemaphore, &releaseSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(releaseSemaphoreHandle, Traits::InvalidHandle());

    VkImage image                 = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
    VkDeviceSize deviceMemorySize = 0;

    VkExtent3D extent = {kWidth, kHeight, 1};
    result =
        Traits::CreateImage2D(&helper, format, kDefaultImageCreateFlags, kDefaultImageUsageFlags,
                              nullptr, extent, &image, &deviceMemory, &deviceMemorySize);
    EXPECT_EQ(result, VK_SUCCESS);

    typename Traits::Handle memoryHandle = Traits::InvalidHandle();
    result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(memoryHandle, Traits::InvalidHandle());

    {
        GLMemoryObject memoryObject;
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight, memoryObject, 0);

        GLSemaphore glAcquireSemaphore;
        Traits::ImportSemaphore(glAcquireSemaphore, acquireSemaphoreHandle);

        // Transfer ownership to GL.
        helper.releaseImageAndSignalSemaphore(image, VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_IMAGE_LAYOUT_GENERAL, vkAcquireSemaphore);

        const GLuint barrierTextures[] = {
            texture,
        };
        constexpr uint32_t textureBarriersCount = std::extent<decltype(barrierTextures)>();
        const GLenum textureSrcLayouts[]        = {
            GL_LAYOUT_GENERAL_EXT,
        };
        constexpr uint32_t textureSrcLayoutsCount = std::extent<decltype(textureSrcLayouts)>();
        static_assert(textureBarriersCount == textureSrcLayoutsCount,
                      "barrierTextures and textureSrcLayouts must be the same length");
        glWaitSemaphoreEXT(glAcquireSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                           textureSrcLayouts);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Make the texture red.
        ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        glViewport(0, 0, kWidth, kHeight);
        drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
        EXPECT_GL_NO_ERROR();

        // Transfer ownership back to test.
        GLSemaphore glReleaseSemaphore;
        Traits::ImportSemaphore(glReleaseSemaphore, releaseSemaphoreHandle);

        const GLenum textureDstLayouts[] = {
            GL_LAYOUT_TRANSFER_SRC_EXT,
        };
        constexpr uint32_t textureDstLayoutsCount = std::extent<decltype(textureSrcLayouts)>();
        static_assert(textureBarriersCount == textureDstLayoutsCount,
                      "barrierTextures and textureDstLayouts must be the same length");
        glSignalSemaphoreEXT(glReleaseSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                             textureDstLayouts);

        helper.waitSemaphoreAndAcquireImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            vkReleaseSemaphore);

        // Transfer ownership to GL again, and make sure the contents are preserved.
        helper.releaseImageAndSignalSemaphore(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                              VK_IMAGE_LAYOUT_GENERAL, vkAcquireSemaphore);
        glWaitSemaphoreEXT(glAcquireSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                           textureSrcLayouts);

        // Blend green
        ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.0f);

        // Transfer ownership back to test
        glSignalSemaphoreEXT(glReleaseSemaphore, 0, nullptr, textureBarriersCount, barrierTextures,
                             textureDstLayouts);
        helper.waitSemaphoreAndAcquireImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            vkReleaseSemaphore);

        uint8_t pixels[4 * kWidth * kHeight];
        VkOffset3D offset = {};
        helper.readPixels(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, format, offset, extent,
                          pixels, sizeof(pixels));

        EXPECT_EQ(0xFF, pixels[0]);
        EXPECT_EQ(0xFF, pixels[1]);
        EXPECT_EQ(0x00, pixels[2]);
        EXPECT_EQ(0xFF, pixels[3]);

        EXPECT_EQ(0xFF, pixels[4]);
        EXPECT_EQ(0xFF, pixels[5]);
        EXPECT_EQ(0x00, pixels[6]);
        EXPECT_EQ(0xFF, pixels[7]);
    }

    EXPECT_GL_NO_ERROR();

    vkDeviceWaitIdle(helper.getDevice());
    vkDestroyImage(helper.getDevice(), image, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkAcquireSemaphore, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkReleaseSemaphore, nullptr);
    vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
}

// Test drawing to RGBA8 texture in opaque fd with acquire/release multiple times.
TEST_P(VulkanExternalImageTest, WaitSemaphoresRetainsContentOpaqueFd)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    // http://anglebug.com/42263923
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    runWaitSemaphoresRetainsContentTest<OpaqueFdTraits>(isSwiftshader(), enableDebugLayers());
}

// Test drawing to RGBA8 texture in zircon vmo with acquire/release multiple times.
TEST_P(VulkanExternalImageTest, WaitSemaphoresRetainsContentZirconVmo)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    runWaitSemaphoresRetainsContentTest<FuchsiaTraits>(isSwiftshader(), enableDebugLayers());
}

// Support for Zircon handle types is mandatory on Fuchsia.
TEST_P(VulkanExternalImageTest, ShouldSupportExternalHandlesFuchsia)
{
    ANGLE_SKIP_TEST_IF(!IsFuchsia());
    EXPECT_TRUE(EnsureGLExtensionEnabled("GL_ANGLE_memory_object_fuchsia"));
    EXPECT_TRUE(EnsureGLExtensionEnabled("GL_ANGLE_semaphore_fuchsia"));
    VulkanHelper helper;
    helper.initialize(isSwiftshader(), enableDebugLayers());
    EXPECT_TRUE(helper.canCreateSemaphoreZirconEvent());
    EXPECT_TRUE(helper.canCreateImageZirconVmo(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, kDefaultImageCreateFlags,
                                               kDefaultImageUsageFlags));
}

template <typename Traits>
void RunPreInitializedOnGLImportTest(bool useMemoryObjectFlags,
                                     VkImageTiling tiling,
                                     bool isSwiftshader,
                                     bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));
    ASSERT(EnsureGLExtensionEnabled(Traits::SemaphoreExtension()));

    VkImageCreateFlags createFlags = kDefaultImageCreateFlags;
    VkImageUsageFlags usageFlags   = kDefaultImageUsageFlags;

    AdjustCreateFlags(useMemoryObjectFlags, &createFlags);

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    ANGLE_SKIP_TEST_IF(
        !Traits::CanCreateImage(helper, format, VK_IMAGE_TYPE_2D, tiling, createFlags, usageFlags));
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateSemaphore(helper));

    VkSemaphore vkAcquireSemaphore = VK_NULL_HANDLE;
    VkResult result                = Traits::CreateSemaphore(&helper, &vkAcquireSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkAcquireSemaphore != VK_NULL_HANDLE);

    VkSemaphore vkReleaseSemaphore = VK_NULL_HANDLE;
    result                         = Traits::CreateSemaphore(&helper, &vkReleaseSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkReleaseSemaphore != VK_NULL_HANDLE);

    typename Traits::Handle acquireSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkAcquireSemaphore, &acquireSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(acquireSemaphoreHandle, Traits::InvalidHandle());

    typename Traits::Handle releaseSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkReleaseSemaphore, &releaseSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(releaseSemaphoreHandle, Traits::InvalidHandle());

    VkImage image                 = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
    VkDeviceSize deviceMemorySize = 0;

    VkExtent3D extent = {kWidth, kHeight, 1};
    result = Traits::CreateImage2D(&helper, format, createFlags, usageFlags, nullptr, extent,
                                   &image, &deviceMemory, &deviceMemorySize);
    EXPECT_EQ(result, VK_SUCCESS);

    // Initialize a pixel in the image
    constexpr uint32_t kPixel = 0x12345678;
    helper.writePixels(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_FORMAT_R8G8B8A8_UNORM, {0, 0, 0},
                       {1, 1, 1}, static_cast<const void *>(&kPixel), sizeof(kPixel));

    typename Traits::Handle memoryHandle = Traits::InvalidHandle();
    result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(memoryHandle, Traits::InvalidHandle());

    {
        GLMemoryObject memoryObject;
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (tiling == VK_IMAGE_TILING_LINEAR)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_TILING_EXT, GL_LINEAR_TILING_EXT);
        }
        if (useMemoryObjectFlags)
        {
            glTexStorageMemFlags2DANGLE(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight, memoryObject,
                                        0, createFlags, usageFlags, nullptr);
        }
        else
        {
            glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight, memoryObject, 0);
        }

        GLSemaphore glAcquireSemaphore;
        Traits::ImportSemaphore(glAcquireSemaphore, acquireSemaphoreHandle);

        // Note: writePixels leaves the image in TRANSFER_DST_OPTIMAL layout.
        helper.releaseImageAndSignalSemaphore(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              vkAcquireSemaphore);

        const GLuint barrierTexture   = texture;
        const GLenum textureSrcLayout = GL_LAYOUT_COLOR_ATTACHMENT_EXT;
        glWaitSemaphoreEXT(glAcquireSemaphore, 0, nullptr, 1, &barrierTexture, &textureSrcLayout);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        // Readback the initialized pixel, ensure it contains the value written to it.
        uint32_t pixel = 0u;
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
        EXPECT_EQ(pixel, kPixel);
        EXPECT_GL_NO_ERROR();

        GLSemaphore glReleaseSemaphore;
        Traits::ImportSemaphore(glReleaseSemaphore, releaseSemaphoreHandle);

        const GLenum textureDstLayout = GL_LAYOUT_TRANSFER_SRC_EXT;
        glSignalSemaphoreEXT(glReleaseSemaphore, 0, nullptr, 1, &barrierTexture, &textureDstLayout);

        helper.waitSemaphoreAndAcquireImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            vkReleaseSemaphore);
    }

    EXPECT_GL_NO_ERROR();

    vkDeviceWaitIdle(helper.getDevice());
    vkDestroyImage(helper.getDevice(), image, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkAcquireSemaphore, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkReleaseSemaphore, nullptr);
    vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
}

// Test that texture storage created from VkImage memory can be considered pre-initialized in GL.
TEST_P(VulkanExternalImageTest, PreInitializedOnGLImport)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    RunPreInitializedOnGLImportTest<OpaqueFdTraits>(false, VK_IMAGE_TILING_OPTIMAL, isSwiftshader(),
                                                    enableDebugLayers());
}

// Test that texture storage created from VkImage memory can be considered pre-initialized in GL.
// Uses linear tiling.
TEST_P(VulkanExternalImageTest, PreInitializedOnGLImportLinear)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    RunPreInitializedOnGLImportTest<OpaqueFdTraits>(false, VK_IMAGE_TILING_LINEAR, isSwiftshader(),
                                                    enableDebugLayers());
}

// Test that texture storage created from VkImage memory can be considered pre-initialized in GL,
// using GL_ANGLE_memory_object_flags.
TEST_P(VulkanExternalImageTest, PreInitializedOnGLImportWithFlags)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));

    RunPreInitializedOnGLImportTest<OpaqueFdTraits>(true, VK_IMAGE_TILING_OPTIMAL, isSwiftshader(),
                                                    enableDebugLayers());
}

// Test that texture storage created from VkImage memory can be considered pre-initialized in GL,
// using GL_ANGLE_memory_object_flags.  Uses linear tiling.
TEST_P(VulkanExternalImageTest, PreInitializedOnGLImportLinearWithFlags)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_memory_object_flags"));

    RunPreInitializedOnGLImportTest<OpaqueFdTraits>(true, VK_IMAGE_TILING_LINEAR, isSwiftshader(),
                                                    enableDebugLayers());
}

template <typename Traits>
void RunUninitializedOnGLImportTest(bool useMemoryObjectFlags,
                                    std::function<GLenum(GLuint)> useTexture,
                                    const uint32_t *expectInVulkan,
                                    bool isSwiftshader,
                                    bool enableDebugLayers)
{
    ASSERT(EnsureGLExtensionEnabled(Traits::MemoryObjectExtension()));
    ASSERT(EnsureGLExtensionEnabled(Traits::SemaphoreExtension()));

    VkImageCreateFlags createFlags = kDefaultImageCreateFlags;
    VkImageUsageFlags usageFlags   = kDefaultImageUsageFlags;

    AdjustCreateFlags(useMemoryObjectFlags, &createFlags);

    VulkanHelper helper;
    helper.initialize(isSwiftshader, enableDebugLayers);

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateImage(helper, format, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, createFlags, usageFlags));
    ANGLE_SKIP_TEST_IF(!Traits::CanCreateSemaphore(helper));

    VkSemaphore vkAcquireSemaphore = VK_NULL_HANDLE;
    VkResult result                = Traits::CreateSemaphore(&helper, &vkAcquireSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkAcquireSemaphore != VK_NULL_HANDLE);

    VkSemaphore vkReleaseSemaphore = VK_NULL_HANDLE;
    result                         = Traits::CreateSemaphore(&helper, &vkReleaseSemaphore);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_TRUE(vkReleaseSemaphore != VK_NULL_HANDLE);

    typename Traits::Handle acquireSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkAcquireSemaphore, &acquireSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(acquireSemaphoreHandle, Traits::InvalidHandle());

    typename Traits::Handle releaseSemaphoreHandle = Traits::InvalidHandle();
    result = Traits::ExportSemaphore(&helper, vkReleaseSemaphore, &releaseSemaphoreHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(releaseSemaphoreHandle, Traits::InvalidHandle());

    VkImage image                 = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory   = VK_NULL_HANDLE;
    VkDeviceSize deviceMemorySize = 0;

    VkExtent3D extent = {kWidth, kHeight, 1};
    result = Traits::CreateImage2D(&helper, format, createFlags, usageFlags, nullptr, extent,
                                   &image, &deviceMemory, &deviceMemorySize);
    EXPECT_EQ(result, VK_SUCCESS);

    typename Traits::Handle memoryHandle = Traits::InvalidHandle();
    result = Traits::ExportMemory(&helper, deviceMemory, &memoryHandle);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_NE(memoryHandle, Traits::InvalidHandle());

    {
        GLMemoryObject memoryObject;
        GLint dedicatedMemory = GL_TRUE;
        glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT,
                                     &dedicatedMemory);
        Traits::ImportMemory(memoryObject, deviceMemorySize, memoryHandle);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (useMemoryObjectFlags)
        {
            glTexStorageMemFlags2DANGLE(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight, memoryObject,
                                        0, createFlags, usageFlags, nullptr);
        }
        else
        {
            glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight, memoryObject, 0);
        }

        GLSemaphore glAcquireSemaphore;
        Traits::ImportSemaphore(glAcquireSemaphore, acquireSemaphoreHandle);

        // Submit the semaphore without touching the image
        helper.signalSemaphore(vkAcquireSemaphore);

        const GLuint barrierTexture   = texture;
        const GLenum textureSrcLayout = GL_NONE;
        glWaitSemaphoreEXT(glAcquireSemaphore, 0, nullptr, 1, &barrierTexture, &textureSrcLayout);

        GLSemaphore glReleaseSemaphore;
        Traits::ImportSemaphore(glReleaseSemaphore, releaseSemaphoreHandle);

        const GLenum textureDstLayout = useTexture(texture);
        glSignalSemaphoreEXT(glReleaseSemaphore, 0, nullptr, 1, &barrierTexture, &textureDstLayout);

        const VkImageLayout imageLayout = GetPostReleaseVulkanLayout(textureDstLayout);
        helper.waitSemaphoreAndAcquireImage(
            image, imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkReleaseSemaphore);
    }

    EXPECT_GL_NO_ERROR();

    // Verify the contents of the image from the Vulkan side too if needed
    if (expectInVulkan != nullptr)
    {
        uint8_t pixels[4];
        const VkOffset3D offset     = {};
        const VkExtent3D readExtent = {1, 1, 1};
        helper.readPixels(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_FORMAT_R8G8B8A8_UNORM,
                          offset, readExtent, pixels, sizeof(pixels));

        const uint32_t pixel = pixels[0] | pixels[1] << 8 | pixels[2] << 16 | pixels[3] << 24;

        EXPECT_EQ(pixel, *expectInVulkan);
    }

    vkDeviceWaitIdle(helper.getDevice());
    vkDestroyImage(helper.getDevice(), image, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkAcquireSemaphore, nullptr);
    vkDestroySemaphore(helper.getDevice(), vkReleaseSemaphore, nullptr);
    vkFreeMemory(helper.getDevice(), deviceMemory, nullptr);
}

// Test that texture storage created from VkImage memory can be imported as uninitialized in GL.
TEST_P(VulkanExternalImageTest, UninitializedOnGLImport)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    constexpr uint32_t kExpect = 0xFF0000FF;

    auto render = [kExpect](GLuint texture) {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        uint32_t pixel = 0u;
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
        EXPECT_GL_NO_ERROR();

        EXPECT_EQ(pixel, kExpect);
        EXPECT_GL_NO_ERROR();

        return GL_LAYOUT_TRANSFER_SRC_EXT;
    };

    RunUninitializedOnGLImportTest<OpaqueFdTraits>(false, render, &kExpect, isSwiftshader(),
                                                   enableDebugLayers());
}

// Test that texture storage created from VkImage memory can be imported as uninitialized in GL and
// released without being touched.
TEST_P(VulkanExternalImageTest, UninitializedOnGLImportAndExport)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    auto doNothing = [](GLuint) { return GL_NONE; };

    RunUninitializedOnGLImportTest<OpaqueFdTraits>(false, doNothing, nullptr, isSwiftshader(),
                                                   enableDebugLayers());
}

// Test that texture storage created from VkImage memory can be imported as uninitialized in GL and
// then used as the target of a copy.
TEST_P(VulkanExternalImageTest, UninitializedOnGLImportAndCopy)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    constexpr uint32_t kExpect = 0xFF00FF00;

    auto copy = [kExpect](GLuint texture) {
        std::vector<GLColor> initData(kWidth * kHeight, GLColor::green);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                        initData.data());

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        uint32_t pixel = 0u;
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
        EXPECT_GL_NO_ERROR();

        EXPECT_EQ(pixel, kExpect);
        EXPECT_GL_NO_ERROR();

        return GL_LAYOUT_TRANSFER_SRC_EXT;
    };

    RunUninitializedOnGLImportTest<OpaqueFdTraits>(false, copy, &kExpect, isSwiftshader(),
                                                   enableDebugLayers());
}

// Test that texture storage created from VkImage memory can be imported as uninitialized in GL and
// then used as sampler.  Because the image is initialized, sampled results would be garbage, so
// this test is primarily ensuring no validation errors are generated.
TEST_P(VulkanExternalImageTest, UninitializedOnGLImportAndSample)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    auto sample = [this](GLuint texture) {
        GLProgram program;
        program.makeRaster(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.0f);
        EXPECT_GL_NO_ERROR();

        return GL_LAYOUT_SHADER_READ_ONLY_EXT;
    };

    RunUninitializedOnGLImportTest<OpaqueFdTraits>(false, sample, nullptr, isSwiftshader(),
                                                   enableDebugLayers());
}

// Test that texture storage created from VkImage memory can be imported as uninitialized in GL and
// then used as storage image.
TEST_P(VulkanExternalImageTestES31, UninitializedOnGLImportAndStorageWrite)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_memory_object_fd"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_semaphore_fd"));

    constexpr uint32_t kExpect = 0xFF00FFFF;

    auto storageWrite = [kExpect](GLuint texture) {
        constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=8, local_size_y=8) in;
layout(rgba8) uniform highp writeonly image2D img;
void main()
{
    imageStore(img, ivec2(gl_GlobalInvocationID.xy), vec4(1, 1, 0, 1));
})";

        GLProgram program;
        program.makeCompute(kCS);
        glUseProgram(program);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glDispatchCompute(kWidth / 8, kHeight / 8, 1);
        EXPECT_GL_NO_ERROR();

        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        uint32_t pixel = 0u;
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
        EXPECT_GL_NO_ERROR();

        EXPECT_EQ(pixel, kExpect);
        EXPECT_GL_NO_ERROR();

        return GL_LAYOUT_TRANSFER_SRC_EXT;
    };

    RunUninitializedOnGLImportTest<OpaqueFdTraits>(false, storageWrite, &kExpect, isSwiftshader(),
                                                   enableDebugLayers());
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(VulkanExternalImageTest);
ANGLE_INSTANTIATE_TEST_ES31(VulkanExternalImageTestES31);
}  // namespace angle
