//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VulkanImageTest.cpp : Tests of EGL_ANGLE_vulkan_image & GL_ANGLE_vulkan_image extensions.

#include "test_utils/ANGLETest.h"

#include "common/debug.h"
#include "test_utils/VulkanHelper.h"
#include "test_utils/angle_test_instantiate.h"
#include "test_utils/gl_raii.h"

namespace angle
{

constexpr GLuint kWidth  = 64u;
constexpr GLuint kHeight = 64u;
constexpr GLuint kWhite  = 0xffffffff;
constexpr GLuint kRed    = 0xff0000ff;

class VulkanImageTest : public ANGLETest<>
{
  protected:
    VulkanImageTest() { setRobustResourceInit(true); }
};

class VulkanMemoryTest : public ANGLETest<>
{
  protected:
    VulkanMemoryTest() { setRobustResourceInit(true); }

    bool compatibleMemorySizesForDeviceOOMTest(VkPhysicalDevice physicalDevice,
                                               VkDeviceSize *totalDeviceMemorySizeOut);

    angle::VulkanPerfCounters getPerfCounters()
    {
        if (mIndexMap.empty())
        {
            mIndexMap = BuildCounterNameToIndexMap();
        }

        return GetPerfCounters(mIndexMap);
    }

    CounterNameToIndexMap mIndexMap;
};

bool VulkanMemoryTest::compatibleMemorySizesForDeviceOOMTest(VkPhysicalDevice physicalDevice,
                                                             VkDeviceSize *totalDeviceMemorySizeOut)
{
    // Acquire the sizes and memory property flags for all available memory types. There should be
    // at least one memory heap without the device local bit (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).
    // Otherwise, the test should be skipped.
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    *totalDeviceMemorySizeOut                 = 0;
    uint32_t heapsWithoutLocalDeviceMemoryBit = 0;
    for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
    {
        if ((memoryProperties.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0)
        {
            heapsWithoutLocalDeviceMemoryBit++;
        }
        else
        {
            *totalDeviceMemorySizeOut += memoryProperties.memoryHeaps[i].size;
        }
    }

    bool isCompatible = heapsWithoutLocalDeviceMemoryBit != 0 && *totalDeviceMemorySizeOut != 0;
    return isCompatible;
}

// Check extensions with Vukan backend.
TEST_P(VulkanImageTest, HasVulkanImageExtensions)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    EXPECT_TRUE(IsEGLClientExtensionEnabled("EGL_EXT_device_query"));
    EXPECT_TRUE(IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));
    EXPECT_TRUE(IsGLExtensionEnabled("GL_ANGLE_vulkan_image"));

    EGLAttrib result = 0;
    EXPECT_EGL_TRUE(eglQueryDisplayAttribEXT(display, EGL_DEVICE_EXT, &result));

    EGLDeviceEXT device = reinterpret_cast<EGLDeviceEXT>(result);
    EXPECT_NE(EGL_NO_DEVICE_EXT, device);
    EXPECT_TRUE(IsEGLDeviceExtensionEnabled(device, "EGL_ANGLE_device_vulkan"));
}

TEST_P(VulkanImageTest, DeviceVulkan)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    EGLAttrib result = 0;
    EXPECT_EGL_TRUE(eglQueryDisplayAttribEXT(display, EGL_DEVICE_EXT, &result));

    EGLDeviceEXT device = reinterpret_cast<EGLDeviceEXT>(result);
    EXPECT_NE(EGL_NO_DEVICE_EXT, device);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_INSTANCE_ANGLE, &result));
    VkInstance instance = reinterpret_cast<VkInstance>(result);
    EXPECT_NE(instance, static_cast<VkInstance>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_PHYSICAL_DEVICE_ANGLE, &result));
    VkPhysicalDevice physical_device = reinterpret_cast<VkPhysicalDevice>(result);
    EXPECT_NE(physical_device, static_cast<VkPhysicalDevice>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_ANGLE, &result));
    VkDevice vk_device = reinterpret_cast<VkDevice>(result);
    EXPECT_NE(vk_device, static_cast<VkDevice>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_ANGLE, &result));
    VkQueue queue = reinterpret_cast<VkQueue>(result);
    EXPECT_NE(queue, static_cast<VkQueue>(VK_NULL_HANDLE));

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_QUEUE_FAMILIY_INDEX_ANGLE, &result));

    {
        EXPECT_EGL_TRUE(
            eglQueryDeviceAttribEXT(device, EGL_VULKAN_DEVICE_EXTENSIONS_ANGLE, &result));
        const char *const *extensions = reinterpret_cast<const char *const *>(result);
        EXPECT_NE(extensions, nullptr);
        int extension_count = 0;
        while (extensions[extension_count])
        {
            extension_count++;
        }
        EXPECT_NE(extension_count, 0);
    }

    {
        EXPECT_EGL_TRUE(
            eglQueryDeviceAttribEXT(device, EGL_VULKAN_INSTANCE_EXTENSIONS_ANGLE, &result));
        const char *const *extensions = reinterpret_cast<const char *const *>(result);
        EXPECT_NE(extensions, nullptr);
        int extension_count = 0;
        while (extensions[extension_count])
        {
            extension_count++;
        }
        EXPECT_NE(extension_count, 0);
    }

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_FEATURES_ANGLE, &result));
    const VkPhysicalDeviceFeatures2 *features =
        reinterpret_cast<const VkPhysicalDeviceFeatures2 *>(result);
    EXPECT_NE(features, nullptr);
    EXPECT_EQ(features->sType, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);

    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(device, EGL_VULKAN_GET_INSTANCE_PROC_ADDR, &result));
    PFN_vkGetInstanceProcAddr get_instance_proc_addr =
        reinterpret_cast<PFN_vkGetInstanceProcAddr>(result);
    EXPECT_NE(get_instance_proc_addr, nullptr);
}

TEST_P(VulkanImageTest, ExportVKImage)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    EXPECT_GL_NO_ERROR();

    EGLContext context   = window->getContext();
    EGLImageKHR eglImage = eglCreateImageKHR(
        display, context, EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), nullptr);
    EXPECT_NE(eglImage, EGL_NO_IMAGE_KHR);

    VkImage vkImage        = VK_NULL_HANDLE;
    VkImageCreateInfo info = {};
    EXPECT_EGL_TRUE(eglExportVkImageANGLE(display, eglImage, &vkImage, &info));
    EXPECT_NE(vkImage, static_cast<VkImage>(VK_NULL_HANDLE));
    EXPECT_EQ(info.sType, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    EXPECT_EQ(info.pNext, nullptr);
    EXPECT_EQ(info.imageType, VK_IMAGE_TYPE_2D);
    EXPECT_EQ(info.format, VK_FORMAT_R8G8B8A8_UNORM);
    EXPECT_EQ(info.extent.width, kWidth);
    EXPECT_EQ(info.extent.height, kHeight);
    EXPECT_EQ(info.extent.depth, 1u);
    EXPECT_EQ(info.queueFamilyIndexCount, 0u);
    EXPECT_EQ(info.pQueueFamilyIndices, nullptr);
    EXPECT_EQ(info.initialLayout, VK_IMAGE_LAYOUT_UNDEFINED);

    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, eglImage));
}

// Check pixels after glTexImage2D
TEST_P(VulkanImageTest, PixelTestTexImage2D)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));

    VulkanHelper helper;
    helper.initializeFromANGLE();

    constexpr GLuint kColor = 0xafbfcfdf;

    GLTexture texture;

    {
        glBindTexture(GL_TEXTURE_2D, texture);
        std::vector<GLuint> pixels(kWidth * kHeight, kColor);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    EGLContext context   = window->getContext();
    EGLImageKHR eglImage = eglCreateImageKHR(
        display, context, EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), nullptr);
    EXPECT_NE(eglImage, EGL_NO_IMAGE_KHR);

    VkImage vkImage        = VK_NULL_HANDLE;
    VkImageCreateInfo info = {};
    EXPECT_EGL_TRUE(eglExportVkImageANGLE(display, eglImage, &vkImage, &info));
    EXPECT_NE(vkImage, static_cast<VkImage>(VK_NULL_HANDLE));

    GLuint textures[1] = {texture};
    GLenum layouts[1]  = {GL_NONE};
    glReleaseTexturesANGLE(1, textures, layouts);
    EXPECT_EQ(layouts[0], static_cast<GLenum>(GL_LAYOUT_TRANSFER_DST_EXT));

    {
        std::vector<GLuint> pixels(kWidth * kHeight);
        helper.readPixels(vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, info.format, {},
                          info.extent, pixels.data(), pixels.size() * sizeof(GLuint));
        EXPECT_EQ(pixels, std::vector<GLuint>(kWidth * kHeight, kColor));
    }

    layouts[0] = GL_LAYOUT_TRANSFER_SRC_EXT;
    glAcquireTexturesANGLE(1, textures, layouts);

    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, eglImage));
}

// Check pixels after glClear
TEST_P(VulkanImageTest, PixelTestClear)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));

    VulkanHelper helper;
    helper.initializeFromANGLE();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    EGLContext context   = window->getContext();
    EGLImageKHR eglImage = eglCreateImageKHR(
        display, context, EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), nullptr);
    EXPECT_NE(eglImage, EGL_NO_IMAGE_KHR);

    VkImage vkImage        = VK_NULL_HANDLE;
    VkImageCreateInfo info = {};
    EXPECT_EGL_TRUE(eglExportVkImageANGLE(display, eglImage, &vkImage, &info));
    EXPECT_NE(vkImage, static_cast<VkImage>(VK_NULL_HANDLE));

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, kWidth, kHeight);
    // clear framebuffer with white color.
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint textures[1] = {texture};
    GLenum layouts[1]  = {GL_NONE};
    glReleaseTexturesANGLE(1, textures, layouts);
    EXPECT_EQ(layouts[0], static_cast<GLenum>(GL_LAYOUT_TRANSFER_DST_EXT));

    std::vector<GLuint> pixels(kWidth * kHeight);
    helper.readPixels(vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, info.format, {}, info.extent,
                      pixels.data(), pixels.size() * sizeof(GLuint));
    EXPECT_EQ(pixels, std::vector<GLuint>(kWidth * kHeight, kWhite));

    layouts[0] = GL_LAYOUT_TRANSFER_SRC_EXT;
    glAcquireTexturesANGLE(1, textures, layouts);

    // clear framebuffer with red color.
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glReleaseTexturesANGLE(1, textures, layouts);
    EXPECT_EQ(layouts[0], static_cast<GLenum>(GL_LAYOUT_TRANSFER_DST_EXT));

    helper.readPixels(vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, info.format, {}, info.extent,
                      pixels.data(), pixels.size() * sizeof(GLuint));
    EXPECT_EQ(pixels, std::vector<GLuint>(kWidth * kHeight, kRed));

    layouts[0] = GL_LAYOUT_TRANSFER_SRC_EXT;
    glAcquireTexturesANGLE(1, textures, layouts);

    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, eglImage));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Check pixels after GL draw.
TEST_P(VulkanImageTest, PixelTestDrawQuad)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));

    VulkanHelper helper;
    helper.initializeFromANGLE();

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    EGLContext context   = window->getContext();
    EGLImageKHR eglImage = eglCreateImageKHR(
        display, context, EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), nullptr);
    EXPECT_NE(eglImage, EGL_NO_IMAGE_KHR);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, kWidth, kHeight);
    // clear framebuffer with black color.
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw red quad
    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.5f);

    GLuint textures[1] = {texture};
    GLenum layouts[1]  = {GL_NONE};
    glReleaseTexturesANGLE(1, textures, layouts);
    EXPECT_EQ(layouts[0], static_cast<GLenum>(GL_LAYOUT_COLOR_ATTACHMENT_EXT));

    VkImage vkImage        = VK_NULL_HANDLE;
    VkImageCreateInfo info = {};
    EXPECT_EGL_TRUE(eglExportVkImageANGLE(display, eglImage, &vkImage, &info));
    EXPECT_NE(vkImage, static_cast<VkImage>(VK_NULL_HANDLE));

    std::vector<GLuint> pixels(kWidth * kHeight);
    helper.readPixels(vkImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, info.format, {},
                      info.extent, pixels.data(), pixels.size() * sizeof(GLuint));
    EXPECT_EQ(pixels, std::vector<GLuint>(kWidth * kHeight, kRed));

    layouts[0] = GL_LAYOUT_TRANSFER_SRC_EXT;
    glAcquireTexturesANGLE(1, textures, layouts);

    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, eglImage));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Test importing VkImage with eglCreateImageKHR
TEST_P(VulkanImageTest, ClientBuffer)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));

    VulkanHelper helper;
    helper.initializeFromANGLE();

    constexpr VkImageUsageFlags kDefaultImageUsageFlags =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    VkImage vkImage                   = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory     = VK_NULL_HANDLE;
    VkDeviceSize deviceSize           = 0u;
    VkImageCreateInfo imageCreateInfo = {};

    VkResult result = VK_SUCCESS;
    result          = helper.createImage2D(VK_FORMAT_R8G8B8A8_UNORM, 0, kDefaultImageUsageFlags,
                                           {kWidth, kHeight, 1}, &vkImage, &vkDeviceMemory, &deviceSize,
                                           &imageCreateInfo);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_EQ(imageCreateInfo.sType, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);

    uint64_t info    = reinterpret_cast<uint64_t>(&imageCreateInfo);
    EGLint attribs[] = {
        EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE,
        static_cast<EGLint>((info >> 32) & 0xffffffff),
        EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE,
        static_cast<EGLint>(info & 0xffffffff),
        EGL_NONE,
    };
    EGLImageKHR eglImage = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_VULKAN_IMAGE_ANGLE,
                                             reinterpret_cast<EGLClientBuffer>(&vkImage), attribs);
    EXPECT_NE(eglImage, EGL_NO_IMAGE_KHR);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);

    GLuint textures[1] = {texture};
    GLenum layouts[1]  = {GL_NONE};
    glAcquireTexturesANGLE(1, textures, layouts);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, kWidth, kHeight);
    // clear framebuffer with white color.
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    textures[0] = texture;
    layouts[0]  = GL_NONE;
    glReleaseTexturesANGLE(1, textures, layouts);
    EXPECT_EQ(layouts[0], static_cast<GLenum>(GL_LAYOUT_TRANSFER_DST_EXT));

    std::vector<GLuint> pixels(kWidth * kHeight);
    helper.readPixels(vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageCreateInfo.format, {},
                      imageCreateInfo.extent, pixels.data(), pixels.size() * sizeof(GLuint));
    EXPECT_EQ(pixels, std::vector<GLuint>(kWidth * kHeight, kWhite));

    layouts[0] = GL_LAYOUT_TRANSFER_SRC_EXT;
    glAcquireTexturesANGLE(1, textures, layouts);

    // clear framebuffer with red color.
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glReleaseTexturesANGLE(1, textures, layouts);
    EXPECT_EQ(layouts[0], static_cast<GLenum>(GL_LAYOUT_TRANSFER_DST_EXT));

    helper.readPixels(vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageCreateInfo.format, {},
                      imageCreateInfo.extent, pixels.data(), pixels.size() * sizeof(GLuint));
    EXPECT_EQ(pixels, std::vector<GLuint>(kWidth * kHeight, kRed));

    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    framebuffer.reset();
    texture.reset();

    glFinish();

    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, eglImage));
    vkDestroyImage(helper.getDevice(), vkImage, nullptr);
    vkFreeMemory(helper.getDevice(), vkDeviceMemory, nullptr);
}

// Test importing VkImage with eglCreateImageKHR and drawing to make sure no errors occur in setting
// up the framebuffer, including an imageless framebuffer.
TEST_P(VulkanImageTest, ClientBufferWithDraw)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay display = window->getDisplay();

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_vulkan_image"));

    VulkanHelper helper;
    helper.initializeFromANGLE();

    constexpr VkImageUsageFlags kDefaultImageUsageFlags =
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    VkImage vkImage                   = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory     = VK_NULL_HANDLE;
    VkDeviceSize deviceSize           = 0u;
    VkImageCreateInfo imageCreateInfo = {};

    VkResult result = VK_SUCCESS;
    result          = helper.createImage2D(VK_FORMAT_R8G8B8A8_UNORM, 0, kDefaultImageUsageFlags,
                                           {kWidth, kHeight, 1}, &vkImage, &vkDeviceMemory, &deviceSize,
                                           &imageCreateInfo);
    EXPECT_EQ(result, VK_SUCCESS);
    EXPECT_EQ(imageCreateInfo.sType, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);

    uint64_t info    = reinterpret_cast<uint64_t>(&imageCreateInfo);
    EGLint attribs[] = {
        EGL_VULKAN_IMAGE_CREATE_INFO_HI_ANGLE,
        static_cast<EGLint>((info >> 32) & 0xffffffff),
        EGL_VULKAN_IMAGE_CREATE_INFO_LO_ANGLE,
        static_cast<EGLint>(info & 0xffffffff),
        EGL_NONE,
    };
    EGLImageKHR eglImage = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_VULKAN_IMAGE_ANGLE,
                                             reinterpret_cast<EGLClientBuffer>(&vkImage), attribs);
    EXPECT_NE(eglImage, EGL_NO_IMAGE_KHR);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);

    GLuint textures[1] = {texture};
    GLenum layouts[1]  = {GL_NONE};
    glAcquireTexturesANGLE(1, textures, layouts);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    framebuffer.reset();
    texture.reset();

    glFinish();

    EXPECT_EGL_TRUE(eglDestroyImageKHR(display, eglImage));
    vkDestroyImage(helper.getDevice(), vkImage, nullptr);
    vkFreeMemory(helper.getDevice(), vkDeviceMemory, nullptr);
}

// Test that when VMA image suballocation is used, image memory can be allocated from the system in
// case the device memory runs out.
TEST_P(VulkanMemoryTest, AllocateVMAImageWhenDeviceOOM)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::UseVmaForImageSuballocation));

    GLPerfMonitor monitor;
    glBeginPerfMonitorAMD(monitor);

    VulkanHelper helper;
    helper.initializeFromANGLE();
    uint64_t expectedAllocationFallbacks =
        getPerfCounters().deviceMemoryImageAllocationFallbacks + 1;
    uint64_t expectedAllocationFallbacksAfterLastTexture =
        getPerfCounters().deviceMemoryImageAllocationFallbacks + 2;

    VkDeviceSize totalDeviceLocalMemoryHeapSize = 0;
    ANGLE_SKIP_TEST_IF(!compatibleMemorySizesForDeviceOOMTest(helper.getPhysicalDevice(),
                                                              &totalDeviceLocalMemoryHeapSize));

    // Device memory is the first choice for image memory allocation. However, in case it runs out,
    // memory should be allocated from the system if available. Therefore, we want to make sure that
    // we can still allocate image memory even if the device memory is full.
    constexpr VkDeviceSize kTextureWidth  = 2048;
    constexpr VkDeviceSize kTextureHeight = 2048;
    constexpr VkDeviceSize kTextureSize   = kTextureWidth * kTextureHeight * 4;
    VkDeviceSize textureCount             = (totalDeviceLocalMemoryHeapSize / kTextureSize) + 1;

    std::vector<GLTexture> textures;
    textures.resize(textureCount);
    for (uint32_t i = 0; i < textureCount; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureWidth, kTextureHeight);
        glDrawArrays(GL_POINTS, 0, 1);
        EXPECT_GL_NO_ERROR();

        // This process only needs to continue until the allocation is no longer on the device.
        if (getPerfCounters().deviceMemoryImageAllocationFallbacks >= expectedAllocationFallbacks)
        {
            break;
        }
    }
    EXPECT_EQ(getPerfCounters().deviceMemoryImageAllocationFallbacks, expectedAllocationFallbacks);

    // Verify that the texture allocated on the system memory can attach to a framebuffer correctly.
    GLTexture texture;
    std::vector<GLColor> textureColor(kTextureWidth * kTextureHeight, GLColor::magenta);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureWidth, kTextureHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureWidth, kTextureHeight, GL_RGBA,
                    GL_UNSIGNED_BYTE, textureColor.data());
    EXPECT_EQ(getPerfCounters().deviceMemoryImageAllocationFallbacks,
              expectedAllocationFallbacksAfterLastTexture);

    glEndPerfMonitorAMD(monitor);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::magenta);
}

// Test that when VMA image suballocation is used, it is possible to free space for a new image on
// the device by freeing garbage memory from a 2D texture array.
TEST_P(VulkanMemoryTest, AllocateVMAImageAfterFreeing2DArrayGarbageWhenDeviceOOM)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::UseVmaForImageSuballocation));

    GLPerfMonitor monitor;
    glBeginPerfMonitorAMD(monitor);

    VulkanHelper helper;
    helper.initializeFromANGLE();
    uint64_t expectedAllocationFallbacks =
        getPerfCounters().deviceMemoryImageAllocationFallbacks + 1;

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(helper.getPhysicalDevice(), &memoryProperties);

    VkDeviceSize totalDeviceLocalMemoryHeapSize = 0;
    ANGLE_SKIP_TEST_IF(!compatibleMemorySizesForDeviceOOMTest(helper.getPhysicalDevice(),
                                                              &totalDeviceLocalMemoryHeapSize));

    // Use a 2D texture array to allocate some of the available device memory and draw with it.
    GLuint texture2DArray;
    constexpr VkDeviceSize kTextureWidth  = 512;
    constexpr VkDeviceSize kTextureHeight = 512;
    VkDeviceSize texture2DArrayLayerCount = 10;
    glGenTextures(1, &texture2DArray);

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture2DArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, kTextureWidth, kTextureHeight,
                 static_cast<GLsizei>(texture2DArrayLayerCount), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    for (size_t i = 0; i < texture2DArrayLayerCount; i++)
    {
        std::vector<GLColor> textureColor(kTextureWidth * kTextureHeight, GLColor::green);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<GLint>(i), kTextureWidth,
                        kTextureHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE, textureColor.data());
    }

    ANGLE_GL_PROGRAM(drawTex2DArray, essl1_shaders::vs::Texture2DArray(),
                     essl1_shaders::fs::Texture2DArray());
    drawQuad(drawTex2DArray, essl1_shaders::PositionAttrib(), 0.5f);

    // Fill up the device memory until we start allocating on the system memory.
    // Device memory is the first choice for image memory allocation. However, in case it runs out,
    // memory should be allocated from the system if available.
    std::vector<GLTexture> textures2D;
    constexpr VkDeviceSize kTextureSize = kTextureWidth * kTextureHeight * 4;
    VkDeviceSize texture2DCount         = (totalDeviceLocalMemoryHeapSize / kTextureSize) + 1;
    textures2D.resize(texture2DCount);

    for (uint32_t i = 0; i < texture2DCount; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures2D[i]);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureWidth, kTextureHeight);
        EXPECT_GL_NO_ERROR();

        // This process only needs to continue until the allocation is no longer on the device.
        if (getPerfCounters().deviceMemoryImageAllocationFallbacks >= expectedAllocationFallbacks)
        {
            break;
        }
    }
    EXPECT_EQ(getPerfCounters().deviceMemoryImageAllocationFallbacks, expectedAllocationFallbacks);

    // Wait until GPU finishes execution.
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_NO_ERROR();

    // Delete the 2D array texture. This frees the memory due to context flushing from the memory
    // allocation fallbacks.
    glDeleteTextures(1, &texture2DArray);

    // The next texture should be allocated on the device, which will only be possible after freeing
    // the garbage.
    GLTexture lastTexture;
    std::vector<GLColor> lastTextureColor(kTextureWidth * kTextureHeight, GLColor::blue);
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureWidth, kTextureHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureWidth, kTextureHeight, GL_RGBA,
                    GL_UNSIGNED_BYTE, lastTextureColor.data());
    EXPECT_EQ(getPerfCounters().deviceMemoryImageAllocationFallbacks, expectedAllocationFallbacks);

    glEndPerfMonitorAMD(monitor);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lastTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::blue);
}

// Test that when VMA image suballocation is used, it is possible to free space for a new image on
// the device by freeing finished garbage memory from a 2D texture.
TEST_P(VulkanMemoryTest, AllocateVMAImageAfterFreeingFinished2DGarbageWhenDeviceOOM)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::UseVmaForImageSuballocation));

    GLPerfMonitor monitor;
    glBeginPerfMonitorAMD(monitor);

    VulkanHelper helper;
    helper.initializeFromANGLE();
    uint64_t expectedAllocationFallbacks =
        getPerfCounters().deviceMemoryImageAllocationFallbacks + 1;

    VkDeviceSize totalDeviceLocalMemoryHeapSize = 0;
    ANGLE_SKIP_TEST_IF(!compatibleMemorySizesForDeviceOOMTest(helper.getPhysicalDevice(),
                                                              &totalDeviceLocalMemoryHeapSize));

    // Use a large 2D texture to allocate some of the available device memory and draw with it.
    GLuint largeTexture;
    constexpr VkDeviceSize kLargeTextureWidth  = 2048;
    constexpr VkDeviceSize kLargeTextureHeight = 2048;
    std::vector<GLColor> firstTextureColor(kLargeTextureWidth * kLargeTextureHeight,
                                           GLColor::green);
    glGenTextures(1, &largeTexture);
    glBindTexture(GL_TEXTURE_2D, largeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kLargeTextureWidth, kLargeTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kLargeTextureWidth, kLargeTextureHeight, GL_RGBA,
                    GL_UNSIGNED_BYTE, firstTextureColor.data());

    ANGLE_GL_PROGRAM(drawTex2D, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(drawTex2D, essl1_shaders::PositionAttrib(), 0.5f);

    // Fill up the device memory until we start allocating on the system memory.
    // Device memory is the first choice for image memory allocation. However, in case it runs out,
    // memory should be allocated from the system if available.
    std::vector<GLTexture> textures2D;
    constexpr VkDeviceSize kTextureWidth  = 512;
    constexpr VkDeviceSize kTextureHeight = 512;
    constexpr VkDeviceSize kTextureSize   = kTextureWidth * kTextureHeight * 4;
    VkDeviceSize texture2DCount           = (totalDeviceLocalMemoryHeapSize / kTextureSize) + 1;
    textures2D.resize(texture2DCount);

    for (uint32_t i = 0; i < texture2DCount; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures2D[i]);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureWidth, kTextureHeight);
        EXPECT_GL_NO_ERROR();

        // This process only needs to continue until the allocation is no longer on the device.
        if (getPerfCounters().deviceMemoryImageAllocationFallbacks >= expectedAllocationFallbacks)
        {
            break;
        }
    }
    EXPECT_EQ(getPerfCounters().deviceMemoryImageAllocationFallbacks, expectedAllocationFallbacks);

    // Wait until GPU finishes execution.
    GLsync syncOne = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glWaitSync(syncOne, 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_NO_ERROR();

    // Delete the large 2D texture. It should free the memory due to context flushing performed
    // during memory allocation fallbacks. Then we allocate and draw with this texture again.
    glDeleteTextures(1, &largeTexture);

    glBindTexture(GL_TEXTURE_2D, largeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kLargeTextureWidth, kLargeTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kLargeTextureWidth, kLargeTextureHeight, GL_RGBA,
                    GL_UNSIGNED_BYTE, firstTextureColor.data());

    drawQuad(drawTex2D, essl1_shaders::PositionAttrib(), 0.5f);

    // Wait until GPU finishes execution one more time.
    GLsync syncTwo = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glWaitSync(syncTwo, 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_NO_ERROR();

    // Delete the large 2D texture. Even though it is marked as deallocated, the device memory is
    // not freed from the garbage yet.
    glDeleteTextures(1, &largeTexture);

    // The next texture should be allocated on the device, which will only be possible after freeing
    // the garbage from the finished commands. There should be no context flushing.
    uint64_t expectedSubmitCalls = getPerfCounters().commandQueueSubmitCallsTotal;
    GLTexture lastTexture;
    std::vector<GLColor> textureColor(kLargeTextureWidth * kLargeTextureWidth, GLColor::red);
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kLargeTextureWidth, kLargeTextureWidth);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kLargeTextureWidth, kLargeTextureWidth, GL_RGBA,
                    GL_UNSIGNED_BYTE, textureColor.data());
    EXPECT_EQ(getPerfCounters().deviceMemoryImageAllocationFallbacks, expectedAllocationFallbacks);
    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedSubmitCalls);

    glEndPerfMonitorAMD(monitor);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lastTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);
}

// Test that when VMA image suballocation is used, it is possible to free space for a new buffer on
// the device by freeing garbage memory from a 2D texture.
TEST_P(VulkanMemoryTest, AllocateBufferAfterFreeing2DGarbageWhenDeviceOOM)
{
    ANGLE_SKIP_TEST_IF(!getEGLWindow()->isFeatureEnabled(Feature::UseVmaForImageSuballocation));

    GLPerfMonitor monitor;
    glBeginPerfMonitorAMD(monitor);

    VulkanHelper helper;
    helper.initializeFromANGLE();
    uint64_t expectedAllocationFallbacks =
        getPerfCounters().deviceMemoryImageAllocationFallbacks + 1;

    VkDeviceSize totalDeviceLocalMemoryHeapSize = 0;
    ANGLE_SKIP_TEST_IF(!compatibleMemorySizesForDeviceOOMTest(helper.getPhysicalDevice(),
                                                              &totalDeviceLocalMemoryHeapSize));

    // Use a large 2D texture to allocate some of the available device memory and draw with it.
    GLuint firstTexture;
    constexpr VkDeviceSize kFirstTextureWidth  = 2048;
    constexpr VkDeviceSize kFirstTextureHeight = 2048;
    glGenTextures(1, &firstTexture);

    glBindTexture(GL_TEXTURE_2D, firstTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kFirstTextureWidth, kFirstTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    {
        std::vector<GLColor> firstTextureColor(kFirstTextureWidth * kFirstTextureHeight,
                                               GLColor::green);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kFirstTextureWidth, kFirstTextureHeight, GL_RGBA,
                        GL_UNSIGNED_BYTE, firstTextureColor.data());
    }

    ANGLE_GL_PROGRAM(drawTex2D, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(drawTex2D, essl1_shaders::PositionAttrib(), 0.5f);

    // Fill up the device memory until we start allocating on the system memory.
    // Device memory is the first choice for image memory allocation. However, in case it runs out,
    // memory should be allocated from the system if available.
    std::vector<GLTexture> textures2D;
    constexpr VkDeviceSize kTextureWidth  = 512;
    constexpr VkDeviceSize kTextureHeight = 512;
    constexpr VkDeviceSize kTextureSize   = kTextureWidth * kTextureHeight * 4;
    VkDeviceSize texture2DCount           = (totalDeviceLocalMemoryHeapSize / kTextureSize) + 1;
    textures2D.resize(texture2DCount);

    for (uint32_t i = 0; i < texture2DCount; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures2D[i]);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kTextureWidth, kTextureHeight);
        EXPECT_GL_NO_ERROR();

        // This process only needs to continue until the allocation is no longer on the device.
        if (getPerfCounters().deviceMemoryImageAllocationFallbacks >= expectedAllocationFallbacks)
        {
            break;
        }
    }
    EXPECT_EQ(getPerfCounters().deviceMemoryImageAllocationFallbacks, expectedAllocationFallbacks);

    glEndPerfMonitorAMD(monitor);

    // Wait until GPU finishes execution.
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_NO_ERROR();

    // Delete the 2D array texture. This frees the memory due to context flushing from the memory
    // allocation fallbacks.
    glDeleteTextures(1, &firstTexture);

    // The buffer should be allocated on the device, which will only be possible after freeing the
    // garbage.
    GLBuffer lastBuffer;
    constexpr VkDeviceSize kBufferSize = kTextureWidth * kTextureHeight * 4;
    std::vector<uint8_t> bufferData(kBufferSize, 255);
    glBindBuffer(GL_ARRAY_BUFFER, lastBuffer);
    glBufferData(GL_ARRAY_BUFFER, kBufferSize, bufferData.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(VulkanImageTest);
ANGLE_INSTANTIATE_TEST_ES3(VulkanMemoryTest);

}  // namespace angle
