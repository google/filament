//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanFormatTablesTest:
//   Tests to validate our Vulkan support tables match hardware support.
//

#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace
{

class VulkanFormatTablesTest : public ANGLETest<>
{};

struct ParametersToTest
{
    VkImageType imageType;
    VkImageCreateFlags createFlags;
};

// This test enumerates all GL formats - for each, it queries the Vulkan support for
// using it as a texture, filterable, and a render target. It checks this against our
// speed-optimized baked tables, and validates they would give the same result.
TEST_P(VulkanFormatTablesTest, TestFormatSupport)
{
    ASSERT_TRUE(IsVulkan());

    // Hack the angle!
    egl::Display *display   = static_cast<egl::Display *>(getEGLWindow()->getDisplay());
    gl::ContextID contextID = {
        static_cast<GLuint>(reinterpret_cast<uintptr_t>(getEGLWindow()->getContext()))};
    gl::Context *context       = display->getContext(contextID);
    auto *contextVk            = rx::GetImplAs<rx::ContextVk>(context);
    rx::vk::Renderer *renderer = contextVk->getRenderer();

    // We need to test normal 2D images as well as Cube images.
    const std::vector<ParametersToTest> parametersToTest = {
        {VK_IMAGE_TYPE_2D, 0}, {VK_IMAGE_TYPE_2D, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT}};

    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const rx::vk::Format &vkFormat = renderer->getFormat(internalFormat);

        // Similar loop as when we build caps in vk_caps_utils.cpp, but query using
        // vkGetPhysicalDeviceImageFormatProperties instead of vkGetPhysicalDeviceFormatProperties
        // and verify we have all the same caps.
        if (!vkFormat.valid())
        {
            // TODO(jmadill): Every angle format should be mapped to a vkFormat.
            // This hasn't been defined in our vk_format_map.json yet so the caps won't be filled.
            continue;
        }

        const gl::TextureCaps &textureCaps = renderer->getNativeTextureCaps().get(internalFormat);

        for (const ParametersToTest params : parametersToTest)
        {
            VkFormat actualImageVkFormat = rx::vk::GetVkFormatFromFormatID(
                renderer, vkFormat.getActualImageFormatID(rx::vk::ImageAccess::SampleOnly));

            // Now let's verify that against vulkan.
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(renderer->getPhysicalDevice(), actualImageVkFormat,
                                                &formatProperties);

            VkImageFormatProperties imageProperties;

            // isTexturable?
            bool isTexturable =
                vkGetPhysicalDeviceImageFormatProperties(
                    renderer->getPhysicalDevice(), actualImageVkFormat, params.imageType,
                    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, params.createFlags,
                    &imageProperties) == VK_SUCCESS;
            EXPECT_EQ(isTexturable, textureCaps.texturable) << actualImageVkFormat;

            // TODO(jmadill): Support ES3 textures.

            // isFilterable?
            bool isFilterable = (formatProperties.optimalTilingFeatures &
                                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) ==
                                VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
            EXPECT_EQ(isFilterable, textureCaps.filterable) << actualImageVkFormat;

            // isRenderable?
            VkFormat actualRenderableImageVkFormat = rx::vk::GetVkFormatFromFormatID(
                renderer, vkFormat.getActualRenderableImageFormatID());
            const bool isRenderableColor =
                (vkGetPhysicalDeviceImageFormatProperties(
                    renderer->getPhysicalDevice(), actualRenderableImageVkFormat, params.imageType,
                    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    params.createFlags, &imageProperties)) == VK_SUCCESS;
            const bool isRenderableDepthStencil =
                (vkGetPhysicalDeviceImageFormatProperties(
                    renderer->getPhysicalDevice(), actualRenderableImageVkFormat, params.imageType,
                    VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    params.createFlags, &imageProperties)) == VK_SUCCESS;

            bool isRenderable = isRenderableColor || isRenderableDepthStencil;
            EXPECT_EQ(isRenderable, textureCaps.textureAttachment) << actualImageVkFormat;
            EXPECT_EQ(isRenderable, textureCaps.renderbuffer) << actualImageVkFormat;
        }
    }
}

ANGLE_INSTANTIATE_TEST(VulkanFormatTablesTest, ES2_VULKAN());

}  // anonymous namespace
