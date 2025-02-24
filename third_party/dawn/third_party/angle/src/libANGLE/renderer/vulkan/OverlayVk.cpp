//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OverlayVk.cpp:
//    Implements the OverlayVk class.
//

#include "libANGLE/renderer/vulkan/OverlayVk.h"

#include "common/system_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Overlay_font_autogen.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

#include <numeric>

namespace rx
{
OverlayVk::OverlayVk(const gl::OverlayState &state) : OverlayImpl(state) {}
OverlayVk::~OverlayVk() = default;

void OverlayVk::onDestroy(const gl::Context *context)
{
    vk::Renderer *renderer = vk::GetImpl(context)->getRenderer();
    VkDevice device        = renderer->getDevice();

    mFontImage.destroy(renderer);
    mFontImageView.destroy(device);
}

angle::Result OverlayVk::createFont(ContextVk *contextVk)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    // Create a buffer to stage font data upload.
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size               = gl::overlay::kFontTotalDataSize;
    bufferCreateInfo.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    vk::RendererScoped<vk::BufferHelper> fontDataBuffer(renderer);

    ANGLE_TRY(fontDataBuffer.get().init(contextVk, bufferCreateInfo,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    uint8_t *mappedFontData;
    ANGLE_TRY(fontDataBuffer.get().map(contextVk, &mappedFontData));

    const uint8_t *fontData = mState.getFontData();
    memcpy(mappedFontData, fontData, gl::overlay::kFontTotalDataSize * sizeof(*fontData));

    ANGLE_TRY(fontDataBuffer.get().flush(renderer, 0, fontDataBuffer.get().getSize()));
    fontDataBuffer.get().unmap(renderer);

    // Don't use robust resource init for overlay widgets.
    constexpr bool kNoRobustInit = false;

    // Create the font image.
    ANGLE_TRY(mFontImage.init(
        contextVk, gl::TextureType::_2D,
        VkExtent3D{gl::overlay::kFontGlyphWidth, gl::overlay::kFontGlyphHeight, 1},
        renderer->getFormat(angle::FormatID::R8_UNORM), 1,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, gl::LevelIndex(0),
        gl::overlay::kFontMipCount, gl::overlay::kFontCharacters, kNoRobustInit, false));

    ANGLE_TRY(contextVk->initImageAllocation(&mFontImage, false, renderer->getMemoryProperties(),
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                             vk::MemoryAllocationType::FontImage));

    ANGLE_TRY(mFontImage.initLayerImageView(
        contextVk, gl::TextureType::_2DArray, VK_IMAGE_ASPECT_COLOR_BIT, gl::SwizzleState(),
        &mFontImageView, vk::LevelIndex(0), gl::overlay::kFontMipCount, 0,
        mFontImage.getLayerCount()));

    // Copy font data from staging buffer.
    vk::CommandBufferAccess access;
    access.onBufferTransferRead(&fontDataBuffer.get());
    access.onImageTransferWrite(gl::LevelIndex(0), gl::overlay::kFontMipCount, 0,
                                gl::overlay::kFontCharacters, VK_IMAGE_ASPECT_COLOR_BIT,
                                &mFontImage);
    vk::OutsideRenderPassCommandBuffer *fontDataUpload;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &fontDataUpload));

    VkBufferImageCopy copy           = {};
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.layerCount = gl::overlay::kFontCharacters;
    copy.imageExtent.depth           = 1;

    for (uint32_t mip = 0; mip < gl::overlay::kFontMipCount; ++mip)
    {
        copy.bufferOffset              = gl::overlay::kFontMipDataOffset[mip];
        copy.bufferRowLength           = gl::overlay::kFontGlyphWidth >> mip;
        copy.bufferImageHeight         = gl::overlay::kFontGlyphHeight >> mip;
        copy.imageSubresource.mipLevel = mip;
        copy.imageExtent.width         = gl::overlay::kFontGlyphWidth >> mip;
        copy.imageExtent.height        = gl::overlay::kFontGlyphHeight >> mip;

        fontDataUpload->copyBufferToImage(fontDataBuffer.get().getBuffer().getHandle(),
                                          mFontImage.getImage(),
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    }

    return angle::Result::Continue;
}

angle::Result OverlayVk::onPresent(ContextVk *contextVk,
                                   vk::ImageHelper *imageToPresent,
                                   const vk::ImageView *imageToPresentView,
                                   bool is90DegreeRotation)
{
    if (mState.getEnabledWidgetCount() == 0)
    {
        return angle::Result::Continue;
    }

    // Lazily initialize the font on first use
    if (!mFontImage.valid())
    {
        ANGLE_TRY(createFont(contextVk));
    }

    vk::Renderer *renderer = contextVk->getRenderer();

    vk::RendererScoped<vk::BufferHelper> textDataBuffer(renderer);
    vk::RendererScoped<vk::BufferHelper> graphDataBuffer(renderer);

    VkBufferCreateInfo textBufferCreateInfo = {};
    textBufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    textBufferCreateInfo.size               = mState.getTextWidgetsBufferSize();
    textBufferCreateInfo.usage              = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    textBufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

    VkBufferCreateInfo graphBufferCreateInfo = textBufferCreateInfo;
    graphBufferCreateInfo.size               = mState.getGraphWidgetsBufferSize();

    ANGLE_TRY(textDataBuffer.get().init(contextVk, textBufferCreateInfo,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    ANGLE_TRY(graphDataBuffer.get().init(contextVk, graphBufferCreateInfo,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

    uint8_t *textData;
    uint8_t *graphData;
    ANGLE_TRY(textDataBuffer.get().map(contextVk, &textData));
    ANGLE_TRY(graphDataBuffer.get().map(contextVk, &graphData));

    uint32_t textWidgetCount  = 0;
    uint32_t graphWidgetCount = 0;

    gl::Extents presentImageExtents(imageToPresent->getExtents().width,
                                    imageToPresent->getExtents().height, 1);
    mState.fillWidgetData(presentImageExtents, textData, graphData, &textWidgetCount,
                          &graphWidgetCount);

    ANGLE_TRY(textDataBuffer.get().flush(renderer, 0, textDataBuffer.get().getSize()));
    ANGLE_TRY(graphDataBuffer.get().flush(renderer, 0, graphDataBuffer.get().getSize()));
    textDataBuffer.get().unmap(renderer);
    graphDataBuffer.get().unmap(renderer);

    UtilsVk::OverlayDrawParameters params;
    params.textWidgetCount  = textWidgetCount;
    params.graphWidgetCount = graphWidgetCount;
    params.rotateXY         = is90DegreeRotation;

    return contextVk->getUtils().drawOverlay(contextVk, &textDataBuffer.get(),
                                             &graphDataBuffer.get(), &mFontImage, &mFontImageView,
                                             imageToPresent, imageToPresentView, params);
}

}  // namespace rx
