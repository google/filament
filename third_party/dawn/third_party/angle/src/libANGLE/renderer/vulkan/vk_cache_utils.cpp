//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_cache_utils.cpp:
//    Contains the classes for the Pipeline State Object cache as well as the RenderPass cache.
//    Also contains the structures for the packed descriptions for the RenderPass and Pipeline.
//

#include "libANGLE/renderer/vulkan/vk_cache_utils.h"

#include "common/aligned_memory.h"
#include "common/system_utils.h"
#include "libANGLE/BlobCache.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/VertexArrayVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include <type_traits>

namespace rx
{
#if defined(ANGLE_DUMP_PIPELINE_CACHE_GRAPH)
constexpr bool kDumpPipelineCacheGraph = true;
#else
constexpr bool kDumpPipelineCacheGraph = false;
#endif  // ANGLE_DUMP_PIPELINE_CACHE_GRAPH

template <typename T>
bool AllCacheEntriesHaveUniqueReference(const T &payload)
{
    bool unique = true;
    for (auto &item : payload)
    {
        if (!item.second.unique())
        {
            unique = false;
        }
    }
    return unique;
}

namespace vk
{
namespace
{
static_assert(static_cast<uint32_t>(RenderPassLoadOp::Load) == VK_ATTACHMENT_LOAD_OP_LOAD,
              "ConvertRenderPassLoadOpToVkLoadOp must be updated");
static_assert(static_cast<uint32_t>(RenderPassLoadOp::Clear) == VK_ATTACHMENT_LOAD_OP_CLEAR,
              "ConvertRenderPassLoadOpToVkLoadOp must be updated");
static_assert(static_cast<uint32_t>(RenderPassLoadOp::DontCare) == VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              "ConvertRenderPassLoadOpToVkLoadOp must be updated");
static_assert(static_cast<uint32_t>(RenderPassLoadOp::None) == 3,
              "ConvertRenderPassLoadOpToVkLoadOp must be updated");

static_assert(static_cast<uint32_t>(RenderPassStoreOp::Store) == VK_ATTACHMENT_STORE_OP_STORE,
              "ConvertRenderPassStoreOpToVkStoreOp must be updated");
static_assert(static_cast<uint32_t>(RenderPassStoreOp::DontCare) ==
                  VK_ATTACHMENT_STORE_OP_DONT_CARE,
              "ConvertRenderPassStoreOpToVkStoreOp must be updated");
static_assert(static_cast<uint32_t>(RenderPassStoreOp::None) == 2,
              "ConvertRenderPassStoreOpToVkStoreOp must be updated");

constexpr uint16_t kMinSampleShadingScale = angle::BitMask<uint16_t>(8);

VkAttachmentLoadOp ConvertRenderPassLoadOpToVkLoadOp(RenderPassLoadOp loadOp)
{
    return loadOp == RenderPassLoadOp::None ? VK_ATTACHMENT_LOAD_OP_NONE_EXT
                                            : static_cast<VkAttachmentLoadOp>(loadOp);
}
VkAttachmentStoreOp ConvertRenderPassStoreOpToVkStoreOp(RenderPassStoreOp storeOp)
{
    return storeOp == RenderPassStoreOp::None ? VK_ATTACHMENT_STORE_OP_NONE_EXT
                                              : static_cast<VkAttachmentStoreOp>(storeOp);
}

constexpr size_t TransitionBits(size_t size)
{
    return size / kGraphicsPipelineDirtyBitBytes;
}

constexpr size_t kPipelineShadersDescOffset = 0;
constexpr size_t kPipelineShadersDescSize =
    kGraphicsPipelineShadersStateSize + kGraphicsPipelineSharedNonVertexInputStateSize;

constexpr size_t kPipelineFragmentOutputDescOffset = kGraphicsPipelineShadersStateSize;
constexpr size_t kPipelineFragmentOutputDescSize =
    kGraphicsPipelineSharedNonVertexInputStateSize + kGraphicsPipelineFragmentOutputStateSize;

constexpr size_t kPipelineVertexInputDescOffset =
    kGraphicsPipelineShadersStateSize + kPipelineFragmentOutputDescSize;
constexpr size_t kPipelineVertexInputDescSize = kGraphicsPipelineVertexInputStateSize;

static_assert(kPipelineShadersDescOffset % kGraphicsPipelineDirtyBitBytes == 0);
static_assert(kPipelineShadersDescSize % kGraphicsPipelineDirtyBitBytes == 0);

static_assert(kPipelineFragmentOutputDescOffset % kGraphicsPipelineDirtyBitBytes == 0);
static_assert(kPipelineFragmentOutputDescSize % kGraphicsPipelineDirtyBitBytes == 0);

static_assert(kPipelineVertexInputDescOffset % kGraphicsPipelineDirtyBitBytes == 0);
static_assert(kPipelineVertexInputDescSize % kGraphicsPipelineDirtyBitBytes == 0);

constexpr GraphicsPipelineTransitionBits kPipelineShadersTransitionBitsMask =
    GraphicsPipelineTransitionBits::Mask(TransitionBits(kPipelineShadersDescSize) +
                                         TransitionBits(kPipelineShadersDescOffset)) &
    ~GraphicsPipelineTransitionBits::Mask(TransitionBits(kPipelineShadersDescOffset));

constexpr GraphicsPipelineTransitionBits kPipelineFragmentOutputTransitionBitsMask =
    GraphicsPipelineTransitionBits::Mask(TransitionBits(kPipelineFragmentOutputDescSize) +
                                         TransitionBits(kPipelineFragmentOutputDescOffset)) &
    ~GraphicsPipelineTransitionBits::Mask(TransitionBits(kPipelineFragmentOutputDescOffset));

constexpr GraphicsPipelineTransitionBits kPipelineVertexInputTransitionBitsMask =
    GraphicsPipelineTransitionBits::Mask(TransitionBits(kPipelineVertexInputDescSize) +
                                         TransitionBits(kPipelineVertexInputDescOffset)) &
    ~GraphicsPipelineTransitionBits::Mask(TransitionBits(kPipelineVertexInputDescOffset));

bool GraphicsPipelineHasVertexInput(GraphicsPipelineSubset subset)
{
    return subset == GraphicsPipelineSubset::Complete ||
           subset == GraphicsPipelineSubset::VertexInput;
}

bool GraphicsPipelineHasShaders(GraphicsPipelineSubset subset)
{
    return subset == GraphicsPipelineSubset::Complete || subset == GraphicsPipelineSubset::Shaders;
}

bool GraphicsPipelineHasShadersOrFragmentOutput(GraphicsPipelineSubset subset)
{
    return subset != GraphicsPipelineSubset::VertexInput;
}

bool GraphicsPipelineHasFragmentOutput(GraphicsPipelineSubset subset)
{
    return subset == GraphicsPipelineSubset::Complete ||
           subset == GraphicsPipelineSubset::FragmentOutput;
}

uint8_t PackGLBlendOp(gl::BlendEquationType blendOp)
{
    switch (blendOp)
    {
        case gl::BlendEquationType::Add:
            return static_cast<uint8_t>(VK_BLEND_OP_ADD);
        case gl::BlendEquationType::Subtract:
            return static_cast<uint8_t>(VK_BLEND_OP_SUBTRACT);
        case gl::BlendEquationType::ReverseSubtract:
            return static_cast<uint8_t>(VK_BLEND_OP_REVERSE_SUBTRACT);
        case gl::BlendEquationType::Min:
            return static_cast<uint8_t>(VK_BLEND_OP_MIN);
        case gl::BlendEquationType::Max:
            return static_cast<uint8_t>(VK_BLEND_OP_MAX);
        case gl::BlendEquationType::Multiply:
            return static_cast<uint8_t>(VK_BLEND_OP_MULTIPLY_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Screen:
            return static_cast<uint8_t>(VK_BLEND_OP_SCREEN_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Overlay:
            return static_cast<uint8_t>(VK_BLEND_OP_OVERLAY_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Darken:
            return static_cast<uint8_t>(VK_BLEND_OP_DARKEN_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Lighten:
            return static_cast<uint8_t>(VK_BLEND_OP_LIGHTEN_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Colordodge:
            return static_cast<uint8_t>(VK_BLEND_OP_COLORDODGE_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Colorburn:
            return static_cast<uint8_t>(VK_BLEND_OP_COLORBURN_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Hardlight:
            return static_cast<uint8_t>(VK_BLEND_OP_HARDLIGHT_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Softlight:
            return static_cast<uint8_t>(VK_BLEND_OP_SOFTLIGHT_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Difference:
            return static_cast<uint8_t>(VK_BLEND_OP_DIFFERENCE_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::Exclusion:
            return static_cast<uint8_t>(VK_BLEND_OP_EXCLUSION_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::HslHue:
            return static_cast<uint8_t>(VK_BLEND_OP_HSL_HUE_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::HslSaturation:
            return static_cast<uint8_t>(VK_BLEND_OP_HSL_SATURATION_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::HslColor:
            return static_cast<uint8_t>(VK_BLEND_OP_HSL_COLOR_EXT - VK_BLEND_OP_ZERO_EXT);
        case gl::BlendEquationType::HslLuminosity:
            return static_cast<uint8_t>(VK_BLEND_OP_HSL_LUMINOSITY_EXT - VK_BLEND_OP_ZERO_EXT);
        default:
            UNREACHABLE();
            return 0;
    }
}

VkBlendOp UnpackBlendOp(uint8_t packedBlendOp)
{
    if (packedBlendOp <= VK_BLEND_OP_MAX)
    {
        return static_cast<VkBlendOp>(packedBlendOp);
    }
    return static_cast<VkBlendOp>(packedBlendOp + VK_BLEND_OP_ZERO_EXT);
}

uint8_t PackGLBlendFactor(gl::BlendFactorType blendFactor)
{
    switch (blendFactor)
    {
        case gl::BlendFactorType::Zero:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ZERO);
        case gl::BlendFactorType::One:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE);
        case gl::BlendFactorType::SrcColor:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_SRC_COLOR);
        case gl::BlendFactorType::DstColor:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_DST_COLOR);
        case gl::BlendFactorType::OneMinusSrcColor:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR);
        case gl::BlendFactorType::SrcAlpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_SRC_ALPHA);
        case gl::BlendFactorType::OneMinusSrcAlpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
        case gl::BlendFactorType::DstAlpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_DST_ALPHA);
        case gl::BlendFactorType::OneMinusDstAlpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA);
        case gl::BlendFactorType::OneMinusDstColor:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR);
        case gl::BlendFactorType::SrcAlphaSaturate:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_SRC_ALPHA_SATURATE);
        case gl::BlendFactorType::ConstantColor:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_CONSTANT_COLOR);
        case gl::BlendFactorType::ConstantAlpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_CONSTANT_ALPHA);
        case gl::BlendFactorType::OneMinusConstantColor:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR);
        case gl::BlendFactorType::OneMinusConstantAlpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA);
        case gl::BlendFactorType::Src1Color:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_SRC1_COLOR);
        case gl::BlendFactorType::Src1Alpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_SRC1_ALPHA);
        case gl::BlendFactorType::OneMinusSrc1Color:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR);
        case gl::BlendFactorType::OneMinusSrc1Alpha:
            return static_cast<uint8_t>(VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
        default:
            UNREACHABLE();
            return 0;
    }
}

// A struct that contains render pass information derived from RenderPassDesc.  It contains dynamic
// rendering structs that could be directly used when creating pipelines or starting a render pass.
// When using render pass objects, the contents are converted to RenderPassInfo.
struct DynamicRenderingInfo : angle::NonCopyable
{
    VkRenderingInfo renderingInfo;
    // Storage for VkRenderingInfo
    gl::DrawBuffersArray<VkRenderingAttachmentInfo> colorAttachmentInfo;
    VkRenderingAttachmentInfo depthAttachmentInfo;
    VkRenderingAttachmentInfo stencilAttachmentInfo;

    // Attachment formats for VkPipelineRenderingCreateInfo and
    // VkCommandBufferInheritanceRenderingInfo.
    gl::DrawBuffersArray<VkFormat> colorAttachmentFormats;
    VkFormat depthAttachmentFormat;
    VkFormat stencilAttachmentFormat;

    // Attachment and input location mapping for VkRenderingAttachmentLocationInfoKHR and
    // VkRenderingInputAttachmentIndexInfoKHR respectively.
    gl::DrawBuffersArray<uint32_t> colorAttachmentLocations;

    // Support for VK_EXT_multisampled_render_to_single_sampled
    VkMultisampledRenderToSingleSampledInfoEXT msrtss;

    // Support for VK_KHR_fragment_shading_rate
    VkRenderingFragmentShadingRateAttachmentInfoKHR fragmentShadingRateInfo;

#if defined(ANGLE_PLATFORM_ANDROID)
    // For VK_ANDROID_external_format_resolve
    VkExternalFormatANDROID externalFormat;
#endif
};

void UnpackAttachmentInfo(VkImageLayout layout,
                          RenderPassLoadOp loadOp,
                          RenderPassStoreOp storeOp,
                          VkImageLayout resolveLayout,
                          VkResolveModeFlagBits resolveMode,
                          VkRenderingAttachmentInfo *infoOut)
{
    *infoOut                    = {};
    infoOut->sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    infoOut->imageLayout        = layout;
    infoOut->resolveImageLayout = resolveLayout;
    infoOut->resolveMode        = resolveMode;
    infoOut->loadOp             = ConvertRenderPassLoadOpToVkLoadOp(loadOp);
    infoOut->storeOp            = ConvertRenderPassStoreOpToVkStoreOp(storeOp);
    // The image views and clear values are specified when starting the render pass itself.
}

enum class DynamicRenderingInfoSubset
{
    // The complete info as needed by vkCmdBeginRendering.
    Full,
    // Only the subset that is needed for pipeline creation / inheritance info.  Equivalent to a
    // compatible render pass.  Note that parts of renderingInfo may still be filled, such as
    // viewMask etc.
    Pipeline,
};

void DeriveRenderingInfo(Renderer *renderer,
                         const RenderPassDesc &desc,
                         DynamicRenderingInfoSubset subset,
                         const gl::Rectangle &renderArea,
                         VkSubpassContents subpassContents,
                         const FramebufferAttachmentsVector<VkImageView> &attachmentViews,
                         const vk::AttachmentOpsArray &ops,
                         const PackedClearValuesArray &clearValues,
                         uint32_t layerCount,
                         DynamicRenderingInfo *infoOut)
{
    ASSERT(renderer->getFeatures().preferDynamicRendering.enabled);
    // MSRTT cannot be emulated over dynamic rendering.
    ASSERT(!renderer->getFeatures().enableMultisampledRenderToTexture.enabled ||
           renderer->getFeatures().supportsMultisampledRenderToSingleSampled.enabled);

#if defined(ANGLE_ENABLE_ASSERTS)
    // Try to catch errors if the entire struct is not filled but uninitialized data is
    // retrieved later.
    memset(infoOut, 0xab, sizeof(*infoOut));
#endif

    const bool hasDitheringThroughExtension = desc.isLegacyDitherEnabled();
    ASSERT(!hasDitheringThroughExtension ||
           renderer->getFeatures().supportsLegacyDithering.enabled);

    // renderArea and layerCount are determined when beginning the render pass
    infoOut->renderingInfo       = {};
    infoOut->renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    infoOut->renderingInfo.flags =
        hasDitheringThroughExtension ? VK_RENDERING_ENABLE_LEGACY_DITHERING_BIT_EXT : 0;
    if (subpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS)
    {
        infoOut->renderingInfo.flags |= VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    }

    infoOut->renderingInfo.viewMask =
        desc.viewCount() > 0 ? angle::BitMask<uint32_t>(desc.viewCount()) : 0;

    // Pack color attachments
    vk::PackedAttachmentIndex attachmentCount(0);
    for (uint32_t colorIndexGL = 0; colorIndexGL < desc.colorAttachmentRange(); ++colorIndexGL)
    {
        // For dynamic rendering, the mapping of attachments to shader output locations is given in
        // VkRenderingAttachmentLocationInfoKHR.
        //
        // For render pass objects, the mapping is specified by
        // VkSubpassDescription2::pColorAttachments.

        if (!desc.isColorAttachmentEnabled(colorIndexGL))
        {
            ASSERT(!desc.hasColorResolveAttachment(colorIndexGL));
            continue;
        }

        infoOut->colorAttachmentLocations[attachmentCount.get()] = colorIndexGL;

        angle::FormatID attachmentFormatID = desc[colorIndexGL];
        ASSERT(attachmentFormatID != angle::FormatID::NONE);
        VkFormat attachmentFormat = GetVkFormatFromFormatID(renderer, attachmentFormatID);

        const bool isYUVExternalFormat = vk::IsYUVExternalFormat(attachmentFormatID);
#if defined(ANGLE_PLATFORM_ANDROID)
        // if yuv, we're going to chain this on to VkCommandBufferInheritanceRenderingInfo and
        // VkPipelineRenderingCreateInfo (or some VkAttachmentDescription2 if falling back to render
        // pass objects).
        if (isYUVExternalFormat)
        {
            infoOut->externalFormat = {VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID, nullptr, 0};

            const vk::ExternalYuvFormatInfo &externalFormatInfo =
                renderer->getExternalFormatTable()->getExternalFormatInfo(attachmentFormatID);
            infoOut->externalFormat.externalFormat = externalFormatInfo.externalFormat;
            attachmentFormat                       = externalFormatInfo.colorAttachmentFormat;
        }
#endif

        ASSERT(attachmentFormat != VK_FORMAT_UNDEFINED);
        infoOut->colorAttachmentFormats[attachmentCount.get()] = attachmentFormat;

        if (subset == DynamicRenderingInfoSubset::Full)
        {
            const VkImageLayout layout = vk::ConvertImageLayoutToVkImageLayout(
                renderer, static_cast<vk::ImageLayout>(ops[attachmentCount].initialLayout));
            const VkImageLayout resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            const VkResolveModeFlagBits resolveMode =
                isYUVExternalFormat ? VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID
                : desc.hasColorResolveAttachment(colorIndexGL) ? VK_RESOLVE_MODE_AVERAGE_BIT
                                                               : VK_RESOLVE_MODE_NONE;
            const RenderPassLoadOp loadOp =
                static_cast<RenderPassLoadOp>(ops[attachmentCount].loadOp);
            const RenderPassStoreOp storeOp =
                static_cast<RenderPassStoreOp>(ops[attachmentCount].storeOp);

            UnpackAttachmentInfo(layout, loadOp, storeOp, resolveImageLayout, resolveMode,
                                 &infoOut->colorAttachmentInfo[attachmentCount.get()]);

            // See description of RenderPassFramebuffer::mImageViews regarding the layout of the
            // attachmentViews.  In short, the draw attachments are packed, while the resolve
            // attachments are not.
            infoOut->colorAttachmentInfo[attachmentCount.get()].imageView =
                attachmentViews[attachmentCount.get()];
            if (resolveMode != VK_RESOLVE_MODE_NONE)
            {
                infoOut->colorAttachmentInfo[attachmentCount.get()].resolveImageView =
                    attachmentViews[RenderPassFramebuffer::kColorResolveAttachmentBegin +
                                    colorIndexGL];
            }
            infoOut->colorAttachmentInfo[attachmentCount.get()].clearValue =
                clearValues[attachmentCount];
        }

        ++attachmentCount;
    }

    infoOut->renderingInfo.colorAttachmentCount = attachmentCount.get();
    infoOut->renderingInfo.pColorAttachments    = infoOut->colorAttachmentInfo.data();

    infoOut->depthAttachmentFormat   = VK_FORMAT_UNDEFINED;
    infoOut->stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    // Depth/stencil attachment, if any
    if (desc.hasDepthStencilAttachment())
    {
        const uint32_t depthStencilIndexGL =
            static_cast<uint32_t>(desc.depthStencilAttachmentIndex());

        const angle::FormatID attachmentFormatID = desc[depthStencilIndexGL];
        ASSERT(attachmentFormatID != angle::FormatID::NONE);
        const angle::Format &angleFormat = angle::Format::Get(attachmentFormatID);
        const VkFormat attachmentFormat  = GetVkFormatFromFormatID(renderer, attachmentFormatID);

        infoOut->depthAttachmentFormat =
            angleFormat.depthBits == 0 ? VK_FORMAT_UNDEFINED : attachmentFormat;
        infoOut->stencilAttachmentFormat =
            angleFormat.stencilBits == 0 ? VK_FORMAT_UNDEFINED : attachmentFormat;

        if (subset == DynamicRenderingInfoSubset::Full)
        {
            const bool resolveDepth =
                angleFormat.depthBits != 0 && desc.hasDepthResolveAttachment();
            const bool resolveStencil =
                angleFormat.stencilBits != 0 && desc.hasStencilResolveAttachment();

            const VkImageLayout layout = ConvertImageLayoutToVkImageLayout(
                renderer, static_cast<vk::ImageLayout>(ops[attachmentCount].initialLayout));
            const VkImageLayout resolveImageLayout =
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            const VkResolveModeFlagBits depthResolveMode =
                resolveDepth ? VK_RESOLVE_MODE_SAMPLE_ZERO_BIT : VK_RESOLVE_MODE_NONE;
            const VkResolveModeFlagBits stencilResolveMode =
                resolveStencil ? VK_RESOLVE_MODE_SAMPLE_ZERO_BIT : VK_RESOLVE_MODE_NONE;
            const RenderPassLoadOp loadOp =
                static_cast<RenderPassLoadOp>(ops[attachmentCount].loadOp);
            const RenderPassStoreOp storeOp =
                static_cast<RenderPassStoreOp>(ops[attachmentCount].storeOp);
            const RenderPassLoadOp stencilLoadOp =
                static_cast<RenderPassLoadOp>(ops[attachmentCount].stencilLoadOp);
            const RenderPassStoreOp stencilStoreOp =
                static_cast<RenderPassStoreOp>(ops[attachmentCount].stencilStoreOp);

            UnpackAttachmentInfo(layout, loadOp, storeOp, resolveImageLayout, depthResolveMode,
                                 &infoOut->depthAttachmentInfo);
            UnpackAttachmentInfo(layout, stencilLoadOp, stencilStoreOp, resolveImageLayout,
                                 stencilResolveMode, &infoOut->stencilAttachmentInfo);

            infoOut->depthAttachmentInfo.imageView = attachmentViews[attachmentCount.get()];
            if (resolveDepth)
            {
                infoOut->depthAttachmentInfo.resolveImageView =
                    attachmentViews[RenderPassFramebuffer::kDepthStencilResolveAttachment];
            }
            infoOut->depthAttachmentInfo.clearValue = clearValues[attachmentCount];

            infoOut->stencilAttachmentInfo.imageView = attachmentViews[attachmentCount.get()];
            if (resolveStencil)
            {
                infoOut->stencilAttachmentInfo.resolveImageView =
                    attachmentViews[RenderPassFramebuffer::kDepthStencilResolveAttachment];
            }
            infoOut->stencilAttachmentInfo.clearValue = clearValues[attachmentCount];

            infoOut->renderingInfo.pDepthAttachment =
                angleFormat.depthBits == 0 ? nullptr : &infoOut->depthAttachmentInfo;
            infoOut->renderingInfo.pStencilAttachment =
                angleFormat.stencilBits == 0 ? nullptr : &infoOut->stencilAttachmentInfo;
        }

        ++attachmentCount;
    }

    if (subset == DynamicRenderingInfoSubset::Full)
    {
        infoOut->renderingInfo.renderArea.offset.x      = static_cast<uint32_t>(renderArea.x);
        infoOut->renderingInfo.renderArea.offset.y      = static_cast<uint32_t>(renderArea.y);
        infoOut->renderingInfo.renderArea.extent.width  = static_cast<uint32_t>(renderArea.width);
        infoOut->renderingInfo.renderArea.extent.height = static_cast<uint32_t>(renderArea.height);
        infoOut->renderingInfo.layerCount               = layerCount;

        if (desc.isRenderToTexture())
        {
            ASSERT(renderer->getFeatures().supportsMultisampledRenderToSingleSampled.enabled);

            infoOut->msrtss = {};
            infoOut->msrtss.sType =
                VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT;
            infoOut->msrtss.multisampledRenderToSingleSampledEnable = true;
            infoOut->msrtss.rasterizationSamples                    = gl_vk::GetSamples(
                desc.samples(), renderer->getFeatures().limitSampleCountTo2.enabled);
            AddToPNextChain(&infoOut->renderingInfo, &infoOut->msrtss);
        }

        // Fragment shading rate attachment, if any
        if (desc.hasFragmentShadingAttachment())
        {
            infoOut->fragmentShadingRateInfo = {};
            infoOut->fragmentShadingRateInfo.sType =
                VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
            infoOut->fragmentShadingRateInfo.imageView = attachmentViews[attachmentCount.get()];
            infoOut->fragmentShadingRateInfo.imageLayout =
                VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
            infoOut->fragmentShadingRateInfo.shadingRateAttachmentTexelSize =
                renderer->getMaxFragmentShadingRateAttachmentTexelSize();

            AddToPNextChain(&infoOut->renderingInfo, &infoOut->fragmentShadingRateInfo);
        }
    }
}

void AttachPipelineRenderingInfo(ErrorContext *context,
                                 const RenderPassDesc &desc,
                                 const DynamicRenderingInfo &renderingInfo,
                                 GraphicsPipelineSubset subset,
                                 VkPipelineRenderingCreateInfoKHR *pipelineRenderingInfoOut,
                                 VkRenderingAttachmentLocationInfoKHR *attachmentLocationsOut,
                                 VkRenderingInputAttachmentIndexInfoKHR *inputLocationsOut,
                                 VkPipelineCreateFlags2CreateInfoKHR *createFlags2,
                                 VkGraphicsPipelineCreateInfo *createInfoOut)
{
    *pipelineRenderingInfoOut          = {};
    pipelineRenderingInfoOut->sType    = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingInfoOut->viewMask = renderingInfo.renderingInfo.viewMask;
    // Note: the formats only affect the fragment output subset.
    if (GraphicsPipelineHasFragmentOutput(subset))
    {
        pipelineRenderingInfoOut->colorAttachmentCount =
            renderingInfo.renderingInfo.colorAttachmentCount;
        pipelineRenderingInfoOut->pColorAttachmentFormats =
            renderingInfo.colorAttachmentFormats.data();
        pipelineRenderingInfoOut->depthAttachmentFormat   = renderingInfo.depthAttachmentFormat;
        pipelineRenderingInfoOut->stencilAttachmentFormat = renderingInfo.stencilAttachmentFormat;
    }
    AddToPNextChain(createInfoOut, pipelineRenderingInfoOut);

    *attachmentLocationsOut       = {};
    attachmentLocationsOut->sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR;
    attachmentLocationsOut->colorAttachmentCount = renderingInfo.renderingInfo.colorAttachmentCount;
    attachmentLocationsOut->pColorAttachmentLocations =
        renderingInfo.colorAttachmentLocations.data();
    AddToPNextChain(createInfoOut, attachmentLocationsOut);

    // Note: VkRenderingInputAttachmentIndexInfoKHR only affects the fragment stage subset.
    if (desc.hasColorFramebufferFetch() && GraphicsPipelineHasShaders(subset))
    {
        *inputLocationsOut       = {};
        inputLocationsOut->sType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR;
        if (desc.hasColorFramebufferFetch())
        {
            inputLocationsOut->colorAttachmentCount =
                renderingInfo.renderingInfo.colorAttachmentCount;
            inputLocationsOut->pColorAttachmentInputIndices =
                renderingInfo.colorAttachmentLocations.data();
        }
        // Note: for depth/stencil, there is no need to explicitly set |pDepthInputAttachmentIndex|,
        // |pStencilInputAttachmentIndex|.  When NULL, they automatically map to input attachments
        // without a |InputAttachmentIndex| decoration, which is exactly how ANGLE produces its
        // SPIR-V.

        AddToPNextChain(createInfoOut, inputLocationsOut);
    }

    if (desc.hasFragmentShadingAttachment())
    {
        createInfoOut->flags |=
            VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }

    if (desc.isLegacyDitherEnabled())
    {
        ASSERT(context->getFeatures().supportsMaintenance5.enabled);
        ASSERT(context->getFeatures().supportsLegacyDithering.enabled);

        *createFlags2       = {};
        createFlags2->sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR;
        createFlags2->flags = createInfoOut->flags;
        createFlags2->flags |= VK_PIPELINE_CREATE_2_ENABLE_LEGACY_DITHERING_BIT_EXT;
        createInfoOut->flags = 0;

        AddToPNextChain(createInfoOut, createFlags2);
    }
}

void UnpackAttachmentDesc(Renderer *renderer,
                          VkAttachmentDescription2 *desc,
                          angle::FormatID formatID,
                          uint8_t samples,
                          const PackedAttachmentOpsDesc &ops)
{
    *desc         = {};
    desc->sType   = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    desc->format  = GetVkFormatFromFormatID(renderer, formatID);
    desc->samples = gl_vk::GetSamples(samples, renderer->getFeatures().limitSampleCountTo2.enabled);
    desc->loadOp  = ConvertRenderPassLoadOpToVkLoadOp(static_cast<RenderPassLoadOp>(ops.loadOp));
    desc->storeOp =
        ConvertRenderPassStoreOpToVkStoreOp(static_cast<RenderPassStoreOp>(ops.storeOp));
    desc->stencilLoadOp =
        ConvertRenderPassLoadOpToVkLoadOp(static_cast<RenderPassLoadOp>(ops.stencilLoadOp));
    desc->stencilStoreOp =
        ConvertRenderPassStoreOpToVkStoreOp(static_cast<RenderPassStoreOp>(ops.stencilStoreOp));
    desc->initialLayout =
        ConvertImageLayoutToVkImageLayout(renderer, static_cast<ImageLayout>(ops.initialLayout));
    desc->finalLayout =
        ConvertImageLayoutToVkImageLayout(renderer, static_cast<ImageLayout>(ops.finalLayout));
}

struct AttachmentInfo
{
    bool usedAsInputAttachment;
    bool isInvalidated;
    // If only one aspect of a depth/stencil image is resolved, the following is used to retain the
    // other aspect.
    bool isUnused;
};

void UnpackColorResolveAttachmentDesc(Renderer *renderer,
                                      VkAttachmentDescription2 *desc,
                                      angle::FormatID formatID,
                                      const AttachmentInfo &info,
                                      ImageLayout finalLayout)
{
    *desc        = {};
    desc->sType  = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    desc->format = GetVkFormatFromFormatID(renderer, formatID);

    // This function is for color resolve attachments.
    const angle::Format &angleFormat = angle::Format::Get(formatID);
    ASSERT(angleFormat.depthBits == 0 && angleFormat.stencilBits == 0);

    // Resolve attachments always have a sample count of 1.
    //
    // If the corresponding color attachment needs to take its initial value from the resolve
    // attachment (i.e. needs to be unresolved), loadOp needs to be set to LOAD, otherwise it should
    // be DONT_CARE as it gets overwritten during resolve.
    //
    // storeOp should be STORE.  If the attachment is invalidated, it is set to DONT_CARE.
    desc->samples = VK_SAMPLE_COUNT_1_BIT;
    desc->loadOp =
        info.usedAsInputAttachment ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc->storeOp =
        info.isInvalidated ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
    desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc->initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    desc->finalLayout    = ConvertImageLayoutToVkImageLayout(renderer, finalLayout);
}

void UnpackDepthStencilResolveAttachmentDesc(vk::ErrorContext *context,
                                             VkAttachmentDescription2 *desc,
                                             angle::FormatID formatID,
                                             const AttachmentInfo &depthInfo,
                                             const AttachmentInfo &stencilInfo)
{
    vk::Renderer *renderer = context->getRenderer();
    *desc                  = {};
    desc->sType            = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    desc->format           = GetVkFormatFromFormatID(renderer, formatID);

    // This function is for depth/stencil resolve attachment.
    const angle::Format &angleFormat = angle::Format::Get(formatID);
    ASSERT(angleFormat.depthBits != 0 || angleFormat.stencilBits != 0);

    // Missing aspects are folded in isInvalidate parameters, so no need to double check.
    ASSERT(angleFormat.depthBits > 0 || depthInfo.isInvalidated);
    ASSERT(angleFormat.stencilBits > 0 || stencilInfo.isInvalidated);

    const bool supportsLoadStoreOpNone =
        context->getFeatures().supportsRenderPassLoadStoreOpNone.enabled;
    const bool supportsStoreOpNone =
        supportsLoadStoreOpNone || context->getFeatures().supportsRenderPassStoreOpNone.enabled;

    const VkAttachmentLoadOp preserveLoadOp =
        supportsLoadStoreOpNone ? VK_ATTACHMENT_LOAD_OP_NONE_EXT : VK_ATTACHMENT_LOAD_OP_LOAD;
    const VkAttachmentStoreOp preserveStoreOp =
        supportsStoreOpNone ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE;

    // Similarly to color resolve attachments, sample count is 1, loadOp is LOAD or DONT_CARE based
    // on whether unresolve is required, and storeOp is STORE or DONT_CARE based on whether the
    // attachment is invalidated or the aspect exists.
    desc->samples = VK_SAMPLE_COUNT_1_BIT;
    if (depthInfo.isUnused)
    {
        desc->loadOp  = preserveLoadOp;
        desc->storeOp = preserveStoreOp;
    }
    else
    {
        desc->loadOp  = depthInfo.usedAsInputAttachment ? VK_ATTACHMENT_LOAD_OP_LOAD
                                                        : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc->storeOp = depthInfo.isInvalidated ? VK_ATTACHMENT_STORE_OP_DONT_CARE
                                                : VK_ATTACHMENT_STORE_OP_STORE;
    }
    if (stencilInfo.isUnused)
    {
        desc->stencilLoadOp  = preserveLoadOp;
        desc->stencilStoreOp = preserveStoreOp;
    }
    else
    {
        desc->stencilLoadOp  = stencilInfo.usedAsInputAttachment ? VK_ATTACHMENT_LOAD_OP_LOAD
                                                                 : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc->stencilStoreOp = stencilInfo.isInvalidated ? VK_ATTACHMENT_STORE_OP_DONT_CARE
                                                         : VK_ATTACHMENT_STORE_OP_STORE;
    }
    desc->initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    desc->finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void UnpackFragmentShadingRateAttachmentDesc(VkAttachmentDescription2 *desc)
{
    *desc                = {};
    desc->sType          = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
    desc->flags          = 0;
    desc->format         = VK_FORMAT_R8_UINT;
    desc->samples        = VK_SAMPLE_COUNT_1_BIT;
    desc->loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
    desc->storeOp        = VK_ATTACHMENT_STORE_OP_NONE;
    desc->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc->initialLayout  = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
    desc->finalLayout    = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
}

void UnpackStencilState(const PackedStencilOpState &packedState,
                        VkStencilOpState *stateOut,
                        bool writeMaskWorkaround)
{
    // Any non-zero value works for the purposes of the useNonZeroStencilWriteMaskStaticState driver
    // bug workaround.
    constexpr uint32_t kNonZeroWriteMaskForWorkaround = 1;

    stateOut->failOp      = static_cast<VkStencilOp>(packedState.fail);
    stateOut->passOp      = static_cast<VkStencilOp>(packedState.pass);
    stateOut->depthFailOp = static_cast<VkStencilOp>(packedState.depthFail);
    stateOut->compareOp   = static_cast<VkCompareOp>(packedState.compare);
    stateOut->compareMask = 0;
    stateOut->writeMask   = writeMaskWorkaround ? kNonZeroWriteMaskForWorkaround : 0;
    stateOut->reference   = 0;
}

void UnpackBlendAttachmentState(const PackedColorBlendAttachmentState &packedState,
                                VkPipelineColorBlendAttachmentState *stateOut)
{
    stateOut->srcColorBlendFactor = static_cast<VkBlendFactor>(packedState.srcColorBlendFactor);
    stateOut->dstColorBlendFactor = static_cast<VkBlendFactor>(packedState.dstColorBlendFactor);
    stateOut->colorBlendOp        = UnpackBlendOp(packedState.colorBlendOp);
    stateOut->srcAlphaBlendFactor = static_cast<VkBlendFactor>(packedState.srcAlphaBlendFactor);
    stateOut->dstAlphaBlendFactor = static_cast<VkBlendFactor>(packedState.dstAlphaBlendFactor);
    stateOut->alphaBlendOp        = UnpackBlendOp(packedState.alphaBlendOp);
}

void SetPipelineShaderStageInfo(const VkStructureType type,
                                const VkShaderStageFlagBits stage,
                                const VkShaderModule module,
                                const VkSpecializationInfo &specializationInfo,
                                VkPipelineShaderStageCreateInfo *shaderStage)
{
    shaderStage->sType               = type;
    shaderStage->flags               = 0;
    shaderStage->stage               = stage;
    shaderStage->module              = module;
    shaderStage->pName               = "main";
    shaderStage->pSpecializationInfo = &specializationInfo;
}

// Defines a subpass that uses the resolve attachments as input attachments to initialize color and
// depth/stencil attachments that need to be "unresolved" at the start of the render pass.  The
// subpass will only contain the attachments that need to be unresolved to simplify the shader that
// performs the operations.
void InitializeUnresolveSubpass(
    const RenderPassDesc &desc,
    const gl::DrawBuffersVector<VkAttachmentReference2> &drawSubpassColorAttachmentRefs,
    const gl::DrawBuffersVector<VkAttachmentReference2> &drawSubpassResolveAttachmentRefs,
    const VkAttachmentReference2 &depthStencilAttachmentRef,
    const VkAttachmentReference2 &depthStencilResolveAttachmentRef,
    gl::DrawBuffersVector<VkAttachmentReference2> *unresolveColorAttachmentRefs,
    VkAttachmentReference2 *unresolveDepthStencilAttachmentRef,
    FramebufferAttachmentsVector<VkAttachmentReference2> *unresolveInputAttachmentRefs,
    FramebufferAttachmentsVector<uint32_t> *unresolvePreserveAttachmentRefs,
    VkSubpassDescription2 *subpassDesc)
{
    // Assume the GL Framebuffer has the following attachments enabled:
    //
    //     GL Color 0
    //     GL Color 3
    //     GL Color 4
    //     GL Color 6
    //     GL Color 7
    //     GL Depth/Stencil
    //
    // Additionally, assume Color 0, 4 and 6 are multisampled-render-to-texture (or for any other
    // reason) have corresponding resolve attachments.  Furthermore, say Color 4 and 6 require an
    // initial unresolve operation.
    //
    // In the above example, the render pass is created with the following attachments:
    //
    //     RP Attachment[0] <- corresponding to GL Color 0
    //     RP Attachment[1] <- corresponding to GL Color 3
    //     RP Attachment[2] <- corresponding to GL Color 4
    //     RP Attachment[3] <- corresponding to GL Color 6
    //     RP Attachment[4] <- corresponding to GL Color 7
    //     RP Attachment[5] <- corresponding to GL Depth/Stencil
    //     RP Attachment[6] <- corresponding to resolve attachment of GL Color 0
    //     RP Attachment[7] <- corresponding to resolve attachment of GL Color 4
    //     RP Attachment[8] <- corresponding to resolve attachment of GL Color 6
    //
    // If the depth/stencil attachment is to be resolved, the following attachment would also be
    // present:
    //
    //     RP Attachment[9] <- corresponding to resolve attachment of GL Depth/Stencil
    //
    // The subpass that takes the application draw calls has the following attachments, creating the
    // mapping from the Vulkan attachment indices (i.e. RP attachment indices) to GL indices as
    // indicated by the GL shaders:
    //
    //     Subpass[1] Color[0] -> RP Attachment[0]
    //     Subpass[1] Color[1] -> VK_ATTACHMENT_UNUSED
    //     Subpass[1] Color[2] -> VK_ATTACHMENT_UNUSED
    //     Subpass[1] Color[3] -> RP Attachment[1]
    //     Subpass[1] Color[4] -> RP Attachment[2]
    //     Subpass[1] Color[5] -> VK_ATTACHMENT_UNUSED
    //     Subpass[1] Color[6] -> RP Attachment[3]
    //     Subpass[1] Color[7] -> RP Attachment[4]
    //     Subpass[1] Depth/Stencil -> RP Attachment[5]
    //     Subpass[1] Resolve[0] -> RP Attachment[6]
    //     Subpass[1] Resolve[1] -> VK_ATTACHMENT_UNUSED
    //     Subpass[1] Resolve[2] -> VK_ATTACHMENT_UNUSED
    //     Subpass[1] Resolve[3] -> VK_ATTACHMENT_UNUSED
    //     Subpass[1] Resolve[4] -> RP Attachment[7]
    //     Subpass[1] Resolve[5] -> VK_ATTACHMENT_UNUSED
    //     Subpass[1] Resolve[6] -> RP Attachment[8]
    //     Subpass[1] Resolve[7] -> VK_ATTACHMENT_UNUSED
    //
    // With depth/stencil resolve attachment:
    //
    //     Subpass[1] Depth/Stencil Resolve -> RP Attachment[9]
    //
    // The initial subpass that's created here is (remember that in the above example Color 4 and 6
    // need to be unresolved):
    //
    //     Subpass[0] Input[0] -> RP Attachment[7] = Subpass[1] Resolve[4]
    //     Subpass[0] Input[1] -> RP Attachment[8] = Subpass[1] Resolve[6]
    //     Subpass[0] Color[0] -> RP Attachment[2] = Subpass[1] Color[4]
    //     Subpass[0] Color[1] -> RP Attachment[3] = Subpass[1] Color[6]
    //
    // The trick here therefore is to use the color attachment refs already created for the
    // application draw subpass indexed with colorIndexGL.
    //
    // If depth/stencil needs to be unresolved (note that as input attachment, it's inserted before
    // the color attachments.  See UtilsVk::unresolve()):
    //
    //     Subpass[0] Input[0]      -> RP Attachment[9] = Subpass[1] Depth/Stencil Resolve
    //     Subpass[0] Depth/Stencil -> RP Attachment[5] = Subpass[1] Depth/Stencil
    //
    // As an additional note, the attachments that are not used in the unresolve subpass must be
    // preserved.  That is color attachments and the depth/stencil attachment if any.  Resolve
    // attachments are rewritten by the next subpass, so they don't need to be preserved.  Note that
    // there's no need to preserve attachments whose loadOp is DONT_CARE.  For simplicity, we
    // preserve those as well.  The driver would ideally avoid preserving attachments with
    // loadOp=DONT_CARE.
    //
    // With the above example:
    //
    //     Subpass[0] Preserve[0] -> RP Attachment[0] = Subpass[1] Color[0]
    //     Subpass[0] Preserve[1] -> RP Attachment[1] = Subpass[1] Color[3]
    //     Subpass[0] Preserve[2] -> RP Attachment[4] = Subpass[1] Color[7]
    //
    // If depth/stencil is not unresolved:
    //
    //     Subpass[0] Preserve[3] -> RP Attachment[5] = Subpass[1] Depth/Stencil
    //
    // Again, the color attachment refs already created for the application draw subpass can be used
    // indexed with colorIndexGL.
    if (desc.hasDepthStencilUnresolveAttachment())
    {
        ASSERT(desc.hasDepthStencilAttachment());
        ASSERT(desc.hasDepthStencilResolveAttachment());

        *unresolveDepthStencilAttachmentRef = depthStencilAttachmentRef;

        VkAttachmentReference2 unresolveDepthStencilInputAttachmentRef = {};
        unresolveDepthStencilInputAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        unresolveDepthStencilInputAttachmentRef.attachment =
            depthStencilResolveAttachmentRef.attachment;
        unresolveDepthStencilInputAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        unresolveDepthStencilInputAttachmentRef.aspectMask = 0;
        if (desc.hasDepthUnresolveAttachment())
        {
            unresolveDepthStencilInputAttachmentRef.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (desc.hasStencilUnresolveAttachment())
        {
            unresolveDepthStencilInputAttachmentRef.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        unresolveInputAttachmentRefs->push_back(unresolveDepthStencilInputAttachmentRef);
    }
    else if (desc.hasDepthStencilAttachment())
    {
        // Preserve the depth/stencil attachment if not unresolved.  Again, there's no need to
        // preserve this attachment if loadOp=DONT_CARE, but we do for simplicity.
        unresolvePreserveAttachmentRefs->push_back(depthStencilAttachmentRef.attachment);
    }

    for (uint32_t colorIndexGL = 0; colorIndexGL < desc.colorAttachmentRange(); ++colorIndexGL)
    {
        if (!desc.hasColorUnresolveAttachment(colorIndexGL))
        {
            if (desc.isColorAttachmentEnabled(colorIndexGL))
            {
                unresolvePreserveAttachmentRefs->push_back(
                    drawSubpassColorAttachmentRefs[colorIndexGL].attachment);
            }
            continue;
        }
        ASSERT(desc.isColorAttachmentEnabled(colorIndexGL));
        ASSERT(desc.hasColorResolveAttachment(colorIndexGL));
        ASSERT(drawSubpassColorAttachmentRefs[colorIndexGL].attachment != VK_ATTACHMENT_UNUSED);
        ASSERT(drawSubpassResolveAttachmentRefs[colorIndexGL].attachment != VK_ATTACHMENT_UNUSED);

        unresolveColorAttachmentRefs->push_back(drawSubpassColorAttachmentRefs[colorIndexGL]);
        unresolveInputAttachmentRefs->push_back(drawSubpassResolveAttachmentRefs[colorIndexGL]);

        // Note the input attachment layout should be shader read-only.  The subpass dependency
        // will take care of transitioning the layout of the resolve attachment to color attachment
        // automatically.
        unresolveInputAttachmentRefs->back().layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    ASSERT(!unresolveColorAttachmentRefs->empty() ||
           unresolveDepthStencilAttachmentRef->attachment != VK_ATTACHMENT_UNUSED);
    ASSERT(unresolveColorAttachmentRefs->size() +
               (desc.hasDepthStencilUnresolveAttachment() ? 1 : 0) ==
           unresolveInputAttachmentRefs->size());

    subpassDesc->sType                = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
    subpassDesc->pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc->inputAttachmentCount = static_cast<uint32_t>(unresolveInputAttachmentRefs->size());
    subpassDesc->pInputAttachments    = unresolveInputAttachmentRefs->data();
    subpassDesc->colorAttachmentCount = static_cast<uint32_t>(unresolveColorAttachmentRefs->size());
    subpassDesc->pColorAttachments    = unresolveColorAttachmentRefs->data();
    subpassDesc->pDepthStencilAttachment = unresolveDepthStencilAttachmentRef;
    subpassDesc->preserveAttachmentCount =
        static_cast<uint32_t>(unresolvePreserveAttachmentRefs->size());
    subpassDesc->pPreserveAttachments = unresolvePreserveAttachmentRefs->data();
}

// There is normally one subpass, and occasionally another for the unresolve operation.
constexpr size_t kSubpassFastVectorSize = 2;
template <typename T>
using SubpassVector = angle::FastVector<T, kSubpassFastVectorSize>;

void InitializeUnresolveSubpassDependencies(const SubpassVector<VkSubpassDescription2> &subpassDesc,
                                            bool unresolveColor,
                                            bool unresolveDepthStencil,
                                            std::vector<VkSubpassDependency2> *subpassDependencies)
{
    ASSERT(subpassDesc.size() >= 2);
    ASSERT(unresolveColor || unresolveDepthStencil);

    // The unresolve subpass is the first subpass.  The application draw subpass is the next one.
    constexpr uint32_t kUnresolveSubpassIndex = 0;
    constexpr uint32_t kDrawSubpassIndex      = 1;

    // A subpass dependency is needed between the unresolve and draw subpasses.  There are two
    // hazards here:
    //
    // - Subpass 0 writes to color/depth/stencil attachments, subpass 1 writes to the same
    //   attachments.  This is a WaW hazard (color/depth/stencil write -> color/depth/stencil write)
    //   similar to when two subsequent render passes write to the same images.
    // - Subpass 0 reads from resolve attachments, subpass 1 writes to the same resolve attachments.
    //   This is a WaR hazard (fragment shader read -> color write) which only requires an execution
    //   barrier.
    //
    // Note: the DEPENDENCY_BY_REGION flag is necessary to create a "framebuffer-local" dependency,
    // as opposed to "framebuffer-global".  The latter is effectively a render pass break.  The
    // former creates a dependency per framebuffer region.  If dependency scopes correspond to
    // attachments with:
    //
    // - Same sample count: dependency is at sample granularity
    // - Different sample count: dependency is at pixel granularity
    //
    // The latter is clarified by the spec as such:
    //
    // > Practically, the pixel vs sample granularity dependency means that if an input attachment
    // > has a different number of samples than the pipeline's rasterizationSamples, then a fragment
    // > can access any sample in the input attachment's pixel even if it only uses
    // > framebuffer-local dependencies.
    //
    // The dependency for the first hazard above (attachment write -> attachment write) is on
    // same-sample attachments, so it will not allow the use of input attachments as required by the
    // unresolve subpass.  As a result, even though the second hazard seems to be subsumed by the
    // first (its src stage is earlier and its dst stage is the same), a separate dependency is
    // created for it just to obtain a pixel granularity dependency.
    //
    // Note: depth/stencil resolve is considered to be done in the color write stage:
    //
    // > Moving to the next subpass automatically performs any multisample resolve operations in the
    // > subpass being ended. End-of-subpass multisample resolves are treated as color attachment
    // > writes for the purposes of synchronization. This applies to resolve operations for both
    // > color and depth/stencil attachments. That is, they are considered to execute in the
    // > VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT pipeline stage and their writes are
    // > synchronized with VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT.

    subpassDependencies->push_back({});
    VkSubpassDependency2 *dependency = &subpassDependencies->back();

    constexpr VkPipelineStageFlags kColorWriteStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    constexpr VkPipelineStageFlags kColorReadWriteStage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    constexpr VkAccessFlags kColorWriteFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    constexpr VkAccessFlags kColorReadWriteFlags =
        kColorWriteFlags | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    constexpr VkPipelineStageFlags kDepthStencilWriteStage =
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    constexpr VkPipelineStageFlags kDepthStencilReadWriteStage =
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    constexpr VkAccessFlags kDepthStencilWriteFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    constexpr VkAccessFlags kDepthStencilReadWriteFlags =
        kDepthStencilWriteFlags | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    VkPipelineStageFlags attachmentWriteStages     = 0;
    VkPipelineStageFlags attachmentReadWriteStages = 0;
    VkAccessFlags attachmentWriteFlags             = 0;
    VkAccessFlags attachmentReadWriteFlags         = 0;

    if (unresolveColor)
    {
        attachmentWriteStages |= kColorWriteStage;
        attachmentReadWriteStages |= kColorReadWriteStage;
        attachmentWriteFlags |= kColorWriteFlags;
        attachmentReadWriteFlags |= kColorReadWriteFlags;
    }

    if (unresolveDepthStencil)
    {
        attachmentWriteStages |= kDepthStencilWriteStage;
        attachmentReadWriteStages |= kDepthStencilReadWriteStage;
        attachmentWriteFlags |= kDepthStencilWriteFlags;
        attachmentReadWriteFlags |= kDepthStencilReadWriteFlags;
    }

    dependency->sType           = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
    dependency->srcSubpass      = kUnresolveSubpassIndex;
    dependency->dstSubpass      = kDrawSubpassIndex;
    dependency->srcStageMask    = attachmentWriteStages;
    dependency->dstStageMask    = attachmentReadWriteStages;
    dependency->srcAccessMask   = attachmentWriteFlags;
    dependency->dstAccessMask   = attachmentReadWriteFlags;
    dependency->dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpassDependencies->push_back({});
    dependency = &subpassDependencies->back();

    // Note again that depth/stencil resolve is considered to be done in the color output stage.
    dependency->sType           = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
    dependency->srcSubpass      = kUnresolveSubpassIndex;
    dependency->dstSubpass      = kDrawSubpassIndex;
    dependency->srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency->dstStageMask    = kColorWriteStage;
    dependency->srcAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    dependency->dstAccessMask   = kColorWriteFlags;
    dependency->dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
}

// glFramebufferFetchBarrierEXT and glBlendBarrierKHR require a pipeline barrier to be inserted in
// the render pass.  This requires a subpass self-dependency.
//
// For framebuffer fetch:
//
//     srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
//     dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
//     srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
//     dstAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT
//
// For advanced blend:
//
//     srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
//     dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
//     srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
//     dstAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT
//
// Subpass dependencies cannot be added after the fact at the end of the render pass due to render
// pass compatibility rules.  ANGLE specifies a subpass self-dependency with the above stage/access
// masks in preparation of potential framebuffer fetch and advanced blend barriers.  This is known
// not to add any overhead on any hardware we have been able to gather information from.
void InitializeDefaultSubpassSelfDependencies(
    ErrorContext *context,
    const RenderPassDesc &desc,
    uint32_t subpassIndex,
    std::vector<VkSubpassDependency2> *subpassDependencies)
{
    Renderer *renderer = context->getRenderer();
    const bool hasRasterizationOrderAttachmentAccess =
        renderer->getFeatures().supportsRasterizationOrderAttachmentAccess.enabled;
    const bool hasBlendOperationAdvanced =
        renderer->getFeatures().supportsBlendOperationAdvanced.enabled;
    const bool hasCoherentBlendOperationAdvanced =
        renderer->getFeatures().supportsBlendOperationAdvancedCoherent.enabled;

    if (hasRasterizationOrderAttachmentAccess &&
        (!hasBlendOperationAdvanced || hasCoherentBlendOperationAdvanced))
    {
        // No need to specify a subpass dependency if VK_EXT_rasterization_order_attachment_access
        // is enabled, as that extension makes this subpass dependency implicit.
        return;
    }

    subpassDependencies->push_back({});
    VkSubpassDependency2 *dependency = &subpassDependencies->back();

    dependency->sType         = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
    dependency->srcSubpass    = subpassIndex;
    dependency->dstSubpass    = subpassIndex;
    dependency->srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency->dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency->srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency->dstAccessMask = 0;
    if (!hasRasterizationOrderAttachmentAccess)
    {
        dependency->dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency->dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    }
    if (hasBlendOperationAdvanced && !hasCoherentBlendOperationAdvanced)
    {
        dependency->dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
    }
    dependency->dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    if (desc.viewCount() > 0)
    {
        dependency->dependencyFlags |= VK_DEPENDENCY_VIEW_LOCAL_BIT;
    }
}

void InitializeMSRTSS(ErrorContext *context,
                      uint8_t renderToTextureSamples,
                      VkSubpassDescription2 *subpass,
                      VkSubpassDescriptionDepthStencilResolve *msrtssResolve,
                      VkMultisampledRenderToSingleSampledInfoEXT *msrtss)
{
    Renderer *renderer = context->getRenderer();

    ASSERT(renderer->getFeatures().supportsMultisampledRenderToSingleSampled.enabled);

    *msrtssResolve                    = {};
    msrtssResolve->sType              = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
    msrtssResolve->depthResolveMode   = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    msrtssResolve->stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;

    *msrtss       = {};
    msrtss->sType = VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT;
    msrtss->pNext = msrtssResolve;
    msrtss->multisampledRenderToSingleSampledEnable = true;
    msrtss->rasterizationSamples                    = gl_vk::GetSamples(
        renderToTextureSamples, context->getFeatures().limitSampleCountTo2.enabled);

    // msrtss->pNext is not null so can't use AddToPNextChain
    AppendToPNextChain(subpass, msrtss);
}

void SetRenderPassViewMask(ErrorContext *context,
                           const uint32_t *viewMask,
                           VkRenderPassCreateInfo2 *createInfo,
                           SubpassVector<VkSubpassDescription2> *subpassDesc)
{
    for (VkSubpassDescription2 &subpass : *subpassDesc)
    {
        subpass.viewMask = *viewMask;
    }

    // For VR, the views are correlated, so this would be an optimization.  However, an
    // application can also use multiview for example to render to all 6 faces of a cubemap, in
    // which case the views are actually not so correlated.  In the absence of any hints from
    // the application, we have to decide on one or the other.  Since VR is more expensive, the
    // views are marked as correlated to optimize that use case.
    createInfo->correlatedViewMaskCount = 1;
    createInfo->pCorrelatedViewMasks    = viewMask;
}

void ToAttachmentDesciption1(const VkAttachmentDescription2 &desc,
                             VkAttachmentDescription *desc1Out)
{
    ASSERT(desc.pNext == nullptr);

    *desc1Out                = {};
    desc1Out->flags          = desc.flags;
    desc1Out->format         = desc.format;
    desc1Out->samples        = desc.samples;
    desc1Out->loadOp         = desc.loadOp;
    desc1Out->storeOp        = desc.storeOp;
    desc1Out->stencilLoadOp  = desc.stencilLoadOp;
    desc1Out->stencilStoreOp = desc.stencilStoreOp;
    desc1Out->initialLayout  = desc.initialLayout;
    desc1Out->finalLayout    = desc.finalLayout;
}

void ToAttachmentReference1(const VkAttachmentReference2 &ref, VkAttachmentReference *ref1Out)
{
    ASSERT(ref.pNext == nullptr);

    *ref1Out            = {};
    ref1Out->attachment = ref.attachment;
    ref1Out->layout     = ref.layout;
}

void ToSubpassDescription1(const VkSubpassDescription2 &desc,
                           const FramebufferAttachmentsVector<VkAttachmentReference> &inputRefs,
                           const gl::DrawBuffersVector<VkAttachmentReference> &colorRefs,
                           const gl::DrawBuffersVector<VkAttachmentReference> &resolveRefs,
                           const VkAttachmentReference &depthStencilRef,
                           VkSubpassDescription *desc1Out)
{
    ASSERT(desc.pNext == nullptr);

    *desc1Out                         = {};
    desc1Out->flags                   = desc.flags;
    desc1Out->pipelineBindPoint       = desc.pipelineBindPoint;
    desc1Out->inputAttachmentCount    = static_cast<uint32_t>(inputRefs.size());
    desc1Out->pInputAttachments       = !inputRefs.empty() ? inputRefs.data() : nullptr;
    desc1Out->colorAttachmentCount    = static_cast<uint32_t>(colorRefs.size());
    desc1Out->pColorAttachments       = !colorRefs.empty() ? colorRefs.data() : nullptr;
    desc1Out->pResolveAttachments     = !resolveRefs.empty() ? resolveRefs.data() : nullptr;
    desc1Out->pDepthStencilAttachment = desc.pDepthStencilAttachment ? &depthStencilRef : nullptr;
    desc1Out->preserveAttachmentCount = desc.preserveAttachmentCount;
    desc1Out->pPreserveAttachments    = desc.pPreserveAttachments;
}

void ToSubpassDependency1(const VkSubpassDependency2 &dep, VkSubpassDependency *dep1Out)
{
    ASSERT(dep.pNext == nullptr);

    *dep1Out                 = {};
    dep1Out->srcSubpass      = dep.srcSubpass;
    dep1Out->dstSubpass      = dep.dstSubpass;
    dep1Out->srcStageMask    = dep.srcStageMask;
    dep1Out->dstStageMask    = dep.dstStageMask;
    dep1Out->srcAccessMask   = dep.srcAccessMask;
    dep1Out->dstAccessMask   = dep.dstAccessMask;
    dep1Out->dependencyFlags = dep.dependencyFlags;
}

void ToRenderPassMultiviewCreateInfo(const VkRenderPassCreateInfo2 &createInfo,
                                     VkRenderPassCreateInfo *createInfo1,
                                     SubpassVector<uint32_t> *viewMasks,
                                     VkRenderPassMultiviewCreateInfo *multiviewInfo)
{
    ASSERT(createInfo.correlatedViewMaskCount == 1);
    const uint32_t viewMask = createInfo.pCorrelatedViewMasks[0];

    viewMasks->resize(createInfo.subpassCount, viewMask);

    multiviewInfo->sType                = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
    multiviewInfo->subpassCount         = createInfo.subpassCount;
    multiviewInfo->pViewMasks           = viewMasks->data();
    multiviewInfo->correlationMaskCount = createInfo.correlatedViewMaskCount;
    multiviewInfo->pCorrelationMasks    = createInfo.pCorrelatedViewMasks;

    AddToPNextChain(createInfo1, multiviewInfo);
}

angle::Result CreateRenderPass1(ErrorContext *context,
                                const VkRenderPassCreateInfo2 &createInfo,
                                uint8_t viewCount,
                                RenderPass *renderPass)
{
    // Convert the attachments to VkAttachmentDescription.
    FramebufferAttachmentArray<VkAttachmentDescription> attachmentDescs;
    for (uint32_t index = 0; index < createInfo.attachmentCount; ++index)
    {
        ToAttachmentDesciption1(createInfo.pAttachments[index], &attachmentDescs[index]);
    }

    // Convert subpass attachments to VkAttachmentReference and the subpass description to
    // VkSubpassDescription.
    SubpassVector<FramebufferAttachmentsVector<VkAttachmentReference>> subpassInputAttachmentRefs(
        createInfo.subpassCount);
    SubpassVector<gl::DrawBuffersVector<VkAttachmentReference>> subpassColorAttachmentRefs(
        createInfo.subpassCount);
    SubpassVector<gl::DrawBuffersVector<VkAttachmentReference>> subpassResolveAttachmentRefs(
        createInfo.subpassCount);
    SubpassVector<VkAttachmentReference> subpassDepthStencilAttachmentRefs(createInfo.subpassCount);
    SubpassVector<VkSubpassDescription> subpassDescriptions(createInfo.subpassCount);
    for (uint32_t subpass = 0; subpass < createInfo.subpassCount; ++subpass)
    {
        const VkSubpassDescription2 &desc = createInfo.pSubpasses[subpass];
        FramebufferAttachmentsVector<VkAttachmentReference> &inputRefs =
            subpassInputAttachmentRefs[subpass];
        gl::DrawBuffersVector<VkAttachmentReference> &colorRefs =
            subpassColorAttachmentRefs[subpass];
        gl::DrawBuffersVector<VkAttachmentReference> &resolveRefs =
            subpassResolveAttachmentRefs[subpass];
        VkAttachmentReference &depthStencilRef = subpassDepthStencilAttachmentRefs[subpass];

        inputRefs.resize(desc.inputAttachmentCount);
        colorRefs.resize(desc.colorAttachmentCount);

        for (uint32_t index = 0; index < desc.inputAttachmentCount; ++index)
        {
            ToAttachmentReference1(desc.pInputAttachments[index], &inputRefs[index]);
        }

        for (uint32_t index = 0; index < desc.colorAttachmentCount; ++index)
        {
            ToAttachmentReference1(desc.pColorAttachments[index], &colorRefs[index]);
        }
        if (desc.pResolveAttachments)
        {
            resolveRefs.resize(desc.colorAttachmentCount);
            for (uint32_t index = 0; index < desc.colorAttachmentCount; ++index)
            {
                ToAttachmentReference1(desc.pResolveAttachments[index], &resolveRefs[index]);
            }
        }
        if (desc.pDepthStencilAttachment)
        {
            ToAttachmentReference1(*desc.pDepthStencilAttachment, &depthStencilRef);
        }

        // Convert subpass itself.
        ToSubpassDescription1(desc, inputRefs, colorRefs, resolveRefs, depthStencilRef,
                              &subpassDescriptions[subpass]);
    }

    // Convert subpass dependencies to VkSubpassDependency.
    std::vector<VkSubpassDependency> subpassDependencies(createInfo.dependencyCount);
    for (uint32_t index = 0; index < createInfo.dependencyCount; ++index)
    {
        ToSubpassDependency1(createInfo.pDependencies[index], &subpassDependencies[index]);
    }

    // Convert CreateInfo itself
    VkRenderPassCreateInfo createInfo1 = {};
    createInfo1.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo1.flags                  = createInfo.flags;
    createInfo1.attachmentCount        = createInfo.attachmentCount;
    createInfo1.pAttachments           = attachmentDescs.data();
    createInfo1.subpassCount           = createInfo.subpassCount;
    createInfo1.pSubpasses             = subpassDescriptions.data();
    createInfo1.dependencyCount        = static_cast<uint32_t>(subpassDependencies.size());
    createInfo1.pDependencies = !subpassDependencies.empty() ? subpassDependencies.data() : nullptr;

    SubpassVector<uint32_t> viewMasks;
    VkRenderPassMultiviewCreateInfo multiviewInfo = {};
    if (viewCount > 0)
    {
        ToRenderPassMultiviewCreateInfo(createInfo, &createInfo1, &viewMasks, &multiviewInfo);
    }

    // Initialize the render pass.
    ANGLE_VK_TRY(context, renderPass->init(context->getDevice(), createInfo1));

    return angle::Result::Continue;
}

void UpdateRenderPassColorPerfCounters(const VkRenderPassCreateInfo2 &createInfo,
                                       FramebufferAttachmentMask depthStencilAttachmentIndices,
                                       RenderPassPerfCounters *countersOut)
{
    for (uint32_t index = 0; index < createInfo.attachmentCount; index++)
    {
        if (depthStencilAttachmentIndices.test(index))
        {
            continue;
        }

        VkAttachmentLoadOp loadOp   = createInfo.pAttachments[index].loadOp;
        VkAttachmentStoreOp storeOp = createInfo.pAttachments[index].storeOp;
        countersOut->colorLoadOpClears += loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
        countersOut->colorLoadOpLoads += loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
        countersOut->colorLoadOpNones += loadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT ? 1 : 0;
        countersOut->colorStoreOpStores += storeOp == VK_ATTACHMENT_STORE_OP_STORE ? 1 : 0;
        countersOut->colorStoreOpNones += storeOp == VK_ATTACHMENT_STORE_OP_NONE_EXT ? 1 : 0;
    }
}

void UpdateSubpassColorPerfCounters(const VkRenderPassCreateInfo2 &createInfo,
                                    const VkSubpassDescription2 &subpass,
                                    RenderPassPerfCounters *countersOut)
{
    // Color resolve counters.
    if (subpass.pResolveAttachments == nullptr)
    {
        return;
    }

    for (uint32_t colorSubpassIndex = 0; colorSubpassIndex < subpass.colorAttachmentCount;
         ++colorSubpassIndex)
    {
        uint32_t resolveRenderPassIndex = subpass.pResolveAttachments[colorSubpassIndex].attachment;

        if (resolveRenderPassIndex == VK_ATTACHMENT_UNUSED)
        {
            continue;
        }

        ++countersOut->colorAttachmentResolves;
    }
}

void UpdateRenderPassDepthStencilPerfCounters(const VkRenderPassCreateInfo2 &createInfo,
                                              size_t renderPassIndex,
                                              RenderPassPerfCounters *countersOut)
{
    ASSERT(renderPassIndex != VK_ATTACHMENT_UNUSED);

    // Depth/stencil ops counters.
    const VkAttachmentDescription2 &ds = createInfo.pAttachments[renderPassIndex];

    countersOut->depthLoadOpClears += ds.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
    countersOut->depthLoadOpLoads += ds.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
    countersOut->depthLoadOpNones += ds.loadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT ? 1 : 0;
    countersOut->depthStoreOpStores += ds.storeOp == VK_ATTACHMENT_STORE_OP_STORE ? 1 : 0;
    countersOut->depthStoreOpNones += ds.storeOp == VK_ATTACHMENT_STORE_OP_NONE_EXT ? 1 : 0;

    countersOut->stencilLoadOpClears += ds.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
    countersOut->stencilLoadOpLoads += ds.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
    countersOut->stencilLoadOpNones += ds.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT ? 1 : 0;
    countersOut->stencilStoreOpStores += ds.stencilStoreOp == VK_ATTACHMENT_STORE_OP_STORE ? 1 : 0;
    countersOut->stencilStoreOpNones +=
        ds.stencilStoreOp == VK_ATTACHMENT_STORE_OP_NONE_EXT ? 1 : 0;

    // Depth/stencil read-only mode.
    countersOut->readOnlyDepthStencil +=
        ds.finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ? 1 : 0;
}

void UpdateRenderPassDepthStencilResolvePerfCounters(
    const VkRenderPassCreateInfo2 &createInfo,
    const VkSubpassDescriptionDepthStencilResolve &depthStencilResolve,
    RenderPassPerfCounters *countersOut)
{
    if (depthStencilResolve.pDepthStencilResolveAttachment == nullptr)
    {
        return;
    }

    uint32_t resolveRenderPassIndex =
        depthStencilResolve.pDepthStencilResolveAttachment->attachment;

    if (resolveRenderPassIndex == VK_ATTACHMENT_UNUSED)
    {
        return;
    }

    const VkAttachmentDescription2 &dsResolve = createInfo.pAttachments[resolveRenderPassIndex];

    // Resolve depth/stencil ops counters.
    countersOut->depthLoadOpClears += dsResolve.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
    countersOut->depthLoadOpLoads += dsResolve.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
    countersOut->depthStoreOpStores +=
        dsResolve.storeOp == static_cast<uint16_t>(RenderPassStoreOp::Store) ? 1 : 0;

    countersOut->stencilLoadOpClears +=
        dsResolve.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
    countersOut->stencilLoadOpLoads +=
        dsResolve.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
    countersOut->stencilStoreOpStores +=
        dsResolve.stencilStoreOp == static_cast<uint16_t>(RenderPassStoreOp::Store) ? 1 : 0;

    // Depth/stencil resolve counters.
    countersOut->depthAttachmentResolves +=
        depthStencilResolve.depthResolveMode != VK_RESOLVE_MODE_NONE ? 1 : 0;
    countersOut->stencilAttachmentResolves +=
        depthStencilResolve.stencilResolveMode != VK_RESOLVE_MODE_NONE ? 1 : 0;
}

void UpdateRenderPassPerfCounters(
    const RenderPassDesc &desc,
    const VkRenderPassCreateInfo2 &createInfo,
    const VkSubpassDescriptionDepthStencilResolve &depthStencilResolve,
    RenderPassPerfCounters *countersOut)
{
    // Accumulate depth/stencil attachment indices in all subpasses to avoid double-counting
    // counters.
    FramebufferAttachmentMask depthStencilAttachmentIndices;

    for (uint32_t subpassIndex = 0; subpassIndex < createInfo.subpassCount; ++subpassIndex)
    {
        const VkSubpassDescription2 &subpass = createInfo.pSubpasses[subpassIndex];

        // Color counters.
        // NOTE: For simplicity, this will accumulate counts for all subpasses in the renderpass.
        UpdateSubpassColorPerfCounters(createInfo, subpass, countersOut);

        // Record index of depth/stencil attachment.
        if (subpass.pDepthStencilAttachment != nullptr)
        {
            uint32_t attachmentRenderPassIndex = subpass.pDepthStencilAttachment->attachment;
            if (attachmentRenderPassIndex != VK_ATTACHMENT_UNUSED)
            {
                depthStencilAttachmentIndices.set(attachmentRenderPassIndex);
            }
        }
    }

    UpdateRenderPassColorPerfCounters(createInfo, depthStencilAttachmentIndices, countersOut);

    // Depth/stencil counters.  Currently, both subpasses use the same depth/stencil attachment (if
    // any).
    ASSERT(depthStencilAttachmentIndices.count() <= 1);
    for (size_t attachmentRenderPassIndex : depthStencilAttachmentIndices)
    {
        UpdateRenderPassDepthStencilPerfCounters(createInfo, attachmentRenderPassIndex,
                                                 countersOut);
    }

    UpdateRenderPassDepthStencilResolvePerfCounters(createInfo, depthStencilResolve, countersOut);

    // Determine unresolve counters from the render pass desc, to avoid making guesses from subpass
    // count etc.
    countersOut->colorAttachmentUnresolves += desc.getColorUnresolveAttachmentMask().count();
    countersOut->depthAttachmentUnresolves += desc.hasDepthUnresolveAttachment() ? 1 : 0;
    countersOut->stencilAttachmentUnresolves += desc.hasStencilUnresolveAttachment() ? 1 : 0;
}

void GetRenderPassAndUpdateCounters(ContextVk *contextVk,
                                    bool updatePerfCounters,
                                    RenderPassHelper *renderPassHelper,
                                    const RenderPass **renderPassOut)
{
    *renderPassOut = &renderPassHelper->getRenderPass();
    if (updatePerfCounters)
    {
        angle::VulkanPerfCounters &counters      = contextVk->getPerfCounters();
        const RenderPassPerfCounters &rpCounters = renderPassHelper->getPerfCounters();

        counters.colorLoadOpClears += rpCounters.colorLoadOpClears;
        counters.colorLoadOpLoads += rpCounters.colorLoadOpLoads;
        counters.colorLoadOpNones += rpCounters.colorLoadOpNones;
        counters.colorStoreOpStores += rpCounters.colorStoreOpStores;
        counters.colorStoreOpNones += rpCounters.colorStoreOpNones;
        counters.depthLoadOpClears += rpCounters.depthLoadOpClears;
        counters.depthLoadOpLoads += rpCounters.depthLoadOpLoads;
        counters.depthLoadOpNones += rpCounters.depthLoadOpNones;
        counters.depthStoreOpStores += rpCounters.depthStoreOpStores;
        counters.depthStoreOpNones += rpCounters.depthStoreOpNones;
        counters.stencilLoadOpClears += rpCounters.stencilLoadOpClears;
        counters.stencilLoadOpLoads += rpCounters.stencilLoadOpLoads;
        counters.stencilLoadOpNones += rpCounters.stencilLoadOpNones;
        counters.stencilStoreOpStores += rpCounters.stencilStoreOpStores;
        counters.stencilStoreOpNones += rpCounters.stencilStoreOpNones;
        counters.colorAttachmentUnresolves += rpCounters.colorAttachmentUnresolves;
        counters.colorAttachmentResolves += rpCounters.colorAttachmentResolves;
        counters.depthAttachmentUnresolves += rpCounters.depthAttachmentUnresolves;
        counters.depthAttachmentResolves += rpCounters.depthAttachmentResolves;
        counters.stencilAttachmentUnresolves += rpCounters.stencilAttachmentUnresolves;
        counters.stencilAttachmentResolves += rpCounters.stencilAttachmentResolves;
        counters.readOnlyDepthStencilRenderPasses += rpCounters.readOnlyDepthStencil;
    }
}

void InitializeSpecializationInfo(
    const SpecializationConstants &specConsts,
    SpecializationConstantMap<VkSpecializationMapEntry> *specializationEntriesOut,
    VkSpecializationInfo *specializationInfoOut)
{
    // Collect specialization constants.
    for (const sh::vk::SpecializationConstantId id :
         angle::AllEnums<sh::vk::SpecializationConstantId>())
    {
        (*specializationEntriesOut)[id].constantID = static_cast<uint32_t>(id);
        switch (id)
        {
            case sh::vk::SpecializationConstantId::SurfaceRotation:
                (*specializationEntriesOut)[id].offset =
                    offsetof(SpecializationConstants, surfaceRotation);
                (*specializationEntriesOut)[id].size = sizeof(specConsts.surfaceRotation);
                break;
            case sh::vk::SpecializationConstantId::Dither:
                (*specializationEntriesOut)[id].offset =
                    offsetof(vk::SpecializationConstants, dither);
                (*specializationEntriesOut)[id].size = sizeof(specConsts.dither);
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    specializationInfoOut->mapEntryCount = static_cast<uint32_t>(specializationEntriesOut->size());
    specializationInfoOut->pMapEntries   = specializationEntriesOut->data();
    specializationInfoOut->dataSize      = sizeof(specConsts);
    specializationInfoOut->pData         = &specConsts;
}

// Utility for setting a value on a packed 4-bit integer array.
template <typename SrcT>
void Int4Array_Set(uint8_t *arrayBytes, uint32_t arrayIndex, SrcT value)
{
    uint32_t byteIndex = arrayIndex >> 1;
    ASSERT(value < 16);

    if ((arrayIndex & 1) == 0)
    {
        arrayBytes[byteIndex] &= 0xF0;
        arrayBytes[byteIndex] |= static_cast<uint8_t>(value);
    }
    else
    {
        arrayBytes[byteIndex] &= 0x0F;
        arrayBytes[byteIndex] |= static_cast<uint8_t>(value) << 4;
    }
}

// Utility for getting a value from a packed 4-bit integer array.
template <typename DestT>
DestT Int4Array_Get(const uint8_t *arrayBytes, uint32_t arrayIndex)
{
    uint32_t byteIndex = arrayIndex >> 1;

    if ((arrayIndex & 1) == 0)
    {
        return static_cast<DestT>(arrayBytes[byteIndex] & 0xF);
    }
    else
    {
        return static_cast<DestT>(arrayBytes[byteIndex] >> 4);
    }
}

// When converting a byte number to a transition bit index we can shift instead of divide.
constexpr size_t kTransitionByteShift = Log2(kGraphicsPipelineDirtyBitBytes);

// When converting a number of bits offset to a transition bit index we can also shift.
constexpr size_t kBitsPerByte        = 8;
constexpr size_t kTransitionBitShift = kTransitionByteShift + Log2(kBitsPerByte);

// Helper macro to map from a PipelineDesc struct and field to a dirty bit index.
// Uses the 'offsetof' macro to compute the offset 'Member' within the PipelineDesc.
// We can optimize the dirty bit setting by computing the shifted dirty bit at compile time instead
// of calling "set".
#define ANGLE_GET_TRANSITION_BIT(Member) \
    (offsetof(GraphicsPipelineDesc, Member) >> kTransitionByteShift)

// Indexed dirty bits cannot be entirely computed at compile time since the index is passed to
// the update function.
#define ANGLE_GET_INDEXED_TRANSITION_BIT(Member, Index, BitWidth) \
    (((BitWidth * Index) >> kTransitionBitShift) + ANGLE_GET_TRANSITION_BIT(Member))

constexpr char kDescriptorTypeNameMap[][30] = {"sampler",
                                               "combined image sampler",
                                               "sampled image",
                                               "storage image",
                                               "uniform texel buffer",
                                               "storage texel buffer",
                                               "uniform buffer",
                                               "storage buffer",
                                               "uniform buffer dynamic",
                                               "storage buffer dynamic",
                                               "input attachment"};

// Helpers for creating a readable dump of the graphics pipeline graph.  Each program generates a
// group of nodes.  The group's description is the common state among all nodes.  Each node contains
// the diff with the shared state.  Arrows between nodes indicate the GraphicsPipelineTransitionBits
// that have caused the transition.  State that is 0 is not output for brevity.
enum class PipelineState
{
    VertexAttribFormat,
    VertexAttribDivisor             = VertexAttribFormat + gl::MAX_VERTEX_ATTRIBS,
    VertexAttribOffset              = VertexAttribDivisor + gl::MAX_VERTEX_ATTRIBS,
    VertexAttribStride              = VertexAttribOffset + gl::MAX_VERTEX_ATTRIBS,
    VertexAttribCompressed          = VertexAttribStride + gl::MAX_VERTEX_ATTRIBS,
    VertexAttribShaderComponentType = VertexAttribCompressed + gl::MAX_VERTEX_ATTRIBS,
    RenderPassSamples               = VertexAttribShaderComponentType + gl::MAX_VERTEX_ATTRIBS,
    RenderPassColorAttachmentRange,
    RenderPassViewCount,
    RenderPassSrgbWriteControl,
    RenderPassHasColorFramebufferFetch,
    RenderPassHasDepthStencilFramebufferFetch,
    RenderPassIsRenderToTexture,
    RenderPassResolveDepth,
    RenderPassResolveStencil,
    RenderPassUnresolveDepth,
    RenderPassUnresolveStencil,
    RenderPassColorResolveMask,
    RenderPassColorUnresolveMask,
    RenderPassColorFormat,
    RenderPassDepthStencilFormat = RenderPassColorFormat + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    Subpass,
    Topology,
    PatchVertices,
    PrimitiveRestartEnable,
    PolygonMode,
    CullMode,
    FrontFace,
    SurfaceRotation,
    ViewportNegativeOneToOne,
    SampleShadingEnable,
    RasterizationSamples,
    MinSampleShading,
    SampleMask,
    AlphaToCoverageEnable,
    AlphaToOneEnable,
    LogicOpEnable,
    LogicOp,
    RasterizerDiscardEnable,
    ColorWriteMask,
    BlendEnableMask = ColorWriteMask + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    MissingOutputsMask,
    SrcColorBlendFactor,
    DstColorBlendFactor   = SrcColorBlendFactor + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    ColorBlendOp          = DstColorBlendFactor + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    SrcAlphaBlendFactor   = ColorBlendOp + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    DstAlphaBlendFactor   = SrcAlphaBlendFactor + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    AlphaBlendOp          = DstAlphaBlendFactor + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    EmulatedDitherControl = AlphaBlendOp + gl::IMPLEMENTATION_MAX_DRAW_BUFFERS,
    DepthClampEnable,
    DepthBoundsTest,
    DepthCompareOp,
    DepthTest,
    DepthWrite,
    StencilTest,
    DepthBiasEnable,
    StencilOpFailFront,
    StencilOpPassFront,
    StencilOpDepthFailFront,
    StencilCompareFront,
    StencilOpFailBack,
    StencilOpPassBack,
    StencilOpDepthFailBack,
    StencilCompareBack,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

using UnpackedPipelineState = angle::PackedEnumMap<PipelineState, uint32_t>;
using PipelineStateBitSet   = angle::BitSetArray<angle::EnumSize<PipelineState>()>;

[[maybe_unused]] void UnpackPipelineState(const GraphicsPipelineDesc &state,
                                          GraphicsPipelineSubset subset,
                                          UnpackedPipelineState *valuesOut)
{
    const bool hasVertexInput             = GraphicsPipelineHasVertexInput(subset);
    const bool hasShaders                 = GraphicsPipelineHasShaders(subset);
    const bool hasShadersOrFragmentOutput = GraphicsPipelineHasShadersOrFragmentOutput(subset);
    const bool hasFragmentOutput          = GraphicsPipelineHasFragmentOutput(subset);

    valuesOut->fill(0);

    if (hasVertexInput)
    {
        const PipelineVertexInputState &vertexInputState = state.getVertexInputStateForLog();

        const PackedVertexInputAttributes &vertex = vertexInputState.vertex;
        uint32_t *vaFormats    = &(*valuesOut)[PipelineState::VertexAttribFormat];
        uint32_t *vaDivisors   = &(*valuesOut)[PipelineState::VertexAttribDivisor];
        uint32_t *vaOffsets    = &(*valuesOut)[PipelineState::VertexAttribOffset];
        uint32_t *vaStrides    = &(*valuesOut)[PipelineState::VertexAttribStride];
        uint32_t *vaCompressed = &(*valuesOut)[PipelineState::VertexAttribCompressed];
        uint32_t *vaShaderComponentType =
            &(*valuesOut)[PipelineState::VertexAttribShaderComponentType];
        for (uint32_t attribIndex = 0; attribIndex < gl::MAX_VERTEX_ATTRIBS; ++attribIndex)
        {
            vaFormats[attribIndex]    = vertex.attribs[attribIndex].format;
            vaDivisors[attribIndex]   = vertex.attribs[attribIndex].divisor;
            vaOffsets[attribIndex]    = vertex.attribs[attribIndex].offset;
            vaStrides[attribIndex]    = vertex.strides[attribIndex];
            vaCompressed[attribIndex] = vertex.attribs[attribIndex].compressed;

            gl::ComponentType componentType = gl::GetComponentTypeMask(
                gl::ComponentTypeMask(vertex.shaderAttribComponentType), attribIndex);
            vaShaderComponentType[attribIndex] = componentType == gl::ComponentType::InvalidEnum
                                                     ? 0
                                                     : static_cast<uint32_t>(componentType);
        }

        const PackedInputAssemblyState &inputAssembly = vertexInputState.inputAssembly;
        (*valuesOut)[PipelineState::Topology]         = inputAssembly.bits.topology;
        (*valuesOut)[PipelineState::PrimitiveRestartEnable] =
            inputAssembly.bits.primitiveRestartEnable;
    }

    if (hasShaders)
    {
        const PipelineShadersState &shadersState = state.getShadersStateForLog();

        const PackedPreRasterizationAndFragmentStates &shaders = shadersState.shaders;
        (*valuesOut)[PipelineState::ViewportNegativeOneToOne] =
            shaders.bits.viewportNegativeOneToOne;
        (*valuesOut)[PipelineState::DepthClampEnable]        = shaders.bits.depthClampEnable;
        (*valuesOut)[PipelineState::PolygonMode]             = shaders.bits.polygonMode;
        (*valuesOut)[PipelineState::CullMode]                = shaders.bits.cullMode;
        (*valuesOut)[PipelineState::FrontFace]               = shaders.bits.frontFace;
        (*valuesOut)[PipelineState::RasterizerDiscardEnable] = shaders.bits.rasterizerDiscardEnable;
        (*valuesOut)[PipelineState::DepthBiasEnable]         = shaders.bits.depthBiasEnable;
        (*valuesOut)[PipelineState::PatchVertices]           = shaders.bits.patchVertices;
        (*valuesOut)[PipelineState::DepthBoundsTest]         = shaders.bits.depthBoundsTest;
        (*valuesOut)[PipelineState::DepthTest]               = shaders.bits.depthTest;
        (*valuesOut)[PipelineState::DepthWrite]              = shaders.bits.depthWrite;
        (*valuesOut)[PipelineState::StencilTest]             = shaders.bits.stencilTest;
        (*valuesOut)[PipelineState::DepthCompareOp]          = shaders.bits.depthCompareOp;
        (*valuesOut)[PipelineState::SurfaceRotation]         = shaders.bits.surfaceRotation;
        (*valuesOut)[PipelineState::EmulatedDitherControl]   = shaders.emulatedDitherControl;
        (*valuesOut)[PipelineState::StencilOpFailFront]      = shaders.front.fail;
        (*valuesOut)[PipelineState::StencilOpPassFront]      = shaders.front.pass;
        (*valuesOut)[PipelineState::StencilOpDepthFailFront] = shaders.front.depthFail;
        (*valuesOut)[PipelineState::StencilCompareFront]     = shaders.front.compare;
        (*valuesOut)[PipelineState::StencilOpFailBack]       = shaders.back.fail;
        (*valuesOut)[PipelineState::StencilOpPassBack]       = shaders.back.pass;
        (*valuesOut)[PipelineState::StencilOpDepthFailBack]  = shaders.back.depthFail;
        (*valuesOut)[PipelineState::StencilCompareBack]      = shaders.back.compare;
    }

    if (hasShadersOrFragmentOutput)
    {
        const PipelineSharedNonVertexInputState &sharedNonVertexInputState =
            state.getSharedNonVertexInputStateForLog();

        const PackedMultisampleAndSubpassState &multisample = sharedNonVertexInputState.multisample;
        (*valuesOut)[PipelineState::SampleMask]             = multisample.bits.sampleMask;
        (*valuesOut)[PipelineState::RasterizationSamples] =
            multisample.bits.rasterizationSamplesMinusOne + 1;
        (*valuesOut)[PipelineState::SampleShadingEnable]   = multisample.bits.sampleShadingEnable;
        (*valuesOut)[PipelineState::AlphaToCoverageEnable] = multisample.bits.alphaToCoverageEnable;
        (*valuesOut)[PipelineState::AlphaToOneEnable]      = multisample.bits.alphaToOneEnable;
        (*valuesOut)[PipelineState::Subpass]               = multisample.bits.subpass;
        (*valuesOut)[PipelineState::MinSampleShading]      = multisample.bits.minSampleShading;

        const RenderPassDesc renderPass                = sharedNonVertexInputState.renderPass;
        (*valuesOut)[PipelineState::RenderPassSamples] = renderPass.samples();
        (*valuesOut)[PipelineState::RenderPassColorAttachmentRange] =
            static_cast<uint32_t>(renderPass.colorAttachmentRange());
        (*valuesOut)[PipelineState::RenderPassViewCount] = renderPass.viewCount();
        (*valuesOut)[PipelineState::RenderPassSrgbWriteControl] =
            static_cast<uint32_t>(renderPass.getSRGBWriteControlMode());
        (*valuesOut)[PipelineState::RenderPassHasColorFramebufferFetch] =
            renderPass.hasColorFramebufferFetch();
        (*valuesOut)[PipelineState::RenderPassHasDepthStencilFramebufferFetch] =
            renderPass.hasDepthStencilFramebufferFetch();
        (*valuesOut)[PipelineState::RenderPassIsRenderToTexture] = renderPass.isRenderToTexture();
        (*valuesOut)[PipelineState::RenderPassResolveDepth] =
            renderPass.hasDepthResolveAttachment();
        (*valuesOut)[PipelineState::RenderPassResolveStencil] =
            renderPass.hasStencilResolveAttachment();
        (*valuesOut)[PipelineState::RenderPassUnresolveDepth] =
            renderPass.hasDepthUnresolveAttachment();
        (*valuesOut)[PipelineState::RenderPassUnresolveStencil] =
            renderPass.hasStencilUnresolveAttachment();
        (*valuesOut)[PipelineState::RenderPassColorResolveMask] =
            renderPass.getColorResolveAttachmentMask().bits();
        (*valuesOut)[PipelineState::RenderPassColorUnresolveMask] =
            renderPass.getColorUnresolveAttachmentMask().bits();

        uint32_t *colorFormats = &(*valuesOut)[PipelineState::RenderPassColorFormat];
        for (uint32_t colorIndex = 0; colorIndex < renderPass.colorAttachmentRange(); ++colorIndex)
        {
            colorFormats[colorIndex] = static_cast<uint32_t>(renderPass[colorIndex]);
        }
        (*valuesOut)[PipelineState::RenderPassDepthStencilFormat] =
            static_cast<uint32_t>(renderPass[renderPass.depthStencilAttachmentIndex()]);
    }

    if (hasFragmentOutput)
    {
        const PipelineFragmentOutputState &fragmentOutputState =
            state.getFragmentOutputStateForLog();

        const PackedColorBlendState &blend = fragmentOutputState.blend;
        uint32_t *colorWriteMasks          = &(*valuesOut)[PipelineState::ColorWriteMask];
        uint32_t *srcColorBlendFactors     = &(*valuesOut)[PipelineState::SrcColorBlendFactor];
        uint32_t *dstColorBlendFactors     = &(*valuesOut)[PipelineState::DstColorBlendFactor];
        uint32_t *colorBlendOps            = &(*valuesOut)[PipelineState::ColorBlendOp];
        uint32_t *srcAlphaBlendFactors     = &(*valuesOut)[PipelineState::SrcAlphaBlendFactor];
        uint32_t *dstAlphaBlendFactors     = &(*valuesOut)[PipelineState::DstAlphaBlendFactor];
        uint32_t *alphaBlendOps            = &(*valuesOut)[PipelineState::AlphaBlendOp];
        for (uint32_t colorIndex = 0; colorIndex < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
             ++colorIndex)
        {
            colorWriteMasks[colorIndex] =
                Int4Array_Get<VkColorComponentFlags>(blend.colorWriteMaskBits, colorIndex);

            srcColorBlendFactors[colorIndex] = blend.attachments[colorIndex].srcColorBlendFactor;
            dstColorBlendFactors[colorIndex] = blend.attachments[colorIndex].dstColorBlendFactor;
            colorBlendOps[colorIndex]        = blend.attachments[colorIndex].colorBlendOp;
            srcAlphaBlendFactors[colorIndex] = blend.attachments[colorIndex].srcAlphaBlendFactor;
            dstAlphaBlendFactors[colorIndex] = blend.attachments[colorIndex].dstAlphaBlendFactor;
            alphaBlendOps[colorIndex]        = blend.attachments[colorIndex].alphaBlendOp;
        }

        const PackedBlendMaskAndLogicOpState &blendMaskAndLogic =
            fragmentOutputState.blendMaskAndLogic;
        (*valuesOut)[PipelineState::BlendEnableMask]    = blendMaskAndLogic.bits.blendEnableMask;
        (*valuesOut)[PipelineState::LogicOpEnable]      = blendMaskAndLogic.bits.logicOpEnable;
        (*valuesOut)[PipelineState::LogicOp]            = blendMaskAndLogic.bits.logicOp;
        (*valuesOut)[PipelineState::MissingOutputsMask] = blendMaskAndLogic.bits.missingOutputsMask;
    }
}

[[maybe_unused]] PipelineStateBitSet GetCommonPipelineState(
    const std::vector<UnpackedPipelineState> &pipelines)
{
    PipelineStateBitSet commonState;
    commonState.set();

    ASSERT(!pipelines.empty());
    const UnpackedPipelineState &firstPipeline = pipelines[0];

    for (const UnpackedPipelineState &pipeline : pipelines)
    {
        for (size_t stateIndex = 0; stateIndex < firstPipeline.size(); ++stateIndex)
        {
            if (pipeline.data()[stateIndex] != firstPipeline.data()[stateIndex])
            {
                commonState.reset(stateIndex);
            }
        }
    }

    return commonState;
}

bool IsPipelineState(size_t stateIndex, PipelineState pipelineState, size_t range)
{
    size_t pipelineStateAsInt = static_cast<size_t>(pipelineState);

    return stateIndex >= pipelineStateAsInt && stateIndex < pipelineStateAsInt + range;
}

size_t GetPipelineStateSubIndex(size_t stateIndex, PipelineState pipelineState)
{
    return stateIndex - static_cast<size_t>(pipelineState);
}

PipelineState GetPipelineState(size_t stateIndex, bool *isRangedOut, size_t *subIndexOut)
{
    constexpr PipelineState kRangedStates[] = {
        PipelineState::VertexAttribFormat,     PipelineState::VertexAttribDivisor,
        PipelineState::VertexAttribOffset,     PipelineState::VertexAttribStride,
        PipelineState::VertexAttribCompressed, PipelineState::VertexAttribShaderComponentType,
        PipelineState::RenderPassColorFormat,  PipelineState::ColorWriteMask,
        PipelineState::SrcColorBlendFactor,    PipelineState::DstColorBlendFactor,
        PipelineState::ColorBlendOp,           PipelineState::SrcAlphaBlendFactor,
        PipelineState::DstAlphaBlendFactor,    PipelineState::AlphaBlendOp,
    };

    for (PipelineState ps : kRangedStates)
    {
        size_t range;
        switch (ps)
        {
            case PipelineState::VertexAttribFormat:
            case PipelineState::VertexAttribDivisor:
            case PipelineState::VertexAttribOffset:
            case PipelineState::VertexAttribStride:
            case PipelineState::VertexAttribCompressed:
            case PipelineState::VertexAttribShaderComponentType:
                range = gl::MAX_VERTEX_ATTRIBS;
                break;
            default:
                range = gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
                break;
        }

        if (IsPipelineState(stateIndex, ps, range))
        {
            *subIndexOut = GetPipelineStateSubIndex(stateIndex, ps);
            *isRangedOut = true;
            return ps;
        }
    }

    *isRangedOut = false;
    return static_cast<PipelineState>(stateIndex);
}

[[maybe_unused]] void OutputPipelineState(std::ostream &out, size_t stateIndex, uint32_t state)
{
    size_t subIndex             = 0;
    bool isRanged               = false;
    PipelineState pipelineState = GetPipelineState(stateIndex, &isRanged, &subIndex);

    constexpr angle::PackedEnumMap<PipelineState, const char *> kStateNames = {{
        {PipelineState::VertexAttribFormat, "va_format"},
        {PipelineState::VertexAttribDivisor, "va_divisor"},
        {PipelineState::VertexAttribOffset, "va_offset"},
        {PipelineState::VertexAttribStride, "va_stride"},
        {PipelineState::VertexAttribCompressed, "va_compressed"},
        {PipelineState::VertexAttribShaderComponentType, "va_shader_component_type"},
        {PipelineState::RenderPassSamples, "rp_samples"},
        {PipelineState::RenderPassColorAttachmentRange, "rp_color_range"},
        {PipelineState::RenderPassViewCount, "rp_views"},
        {PipelineState::RenderPassSrgbWriteControl, "rp_srgb"},
        {PipelineState::RenderPassHasColorFramebufferFetch, "rp_has_color_framebuffer_fetch"},
        {PipelineState::RenderPassHasDepthStencilFramebufferFetch,
         "rp_has_depth_stencil_framebuffer_fetch"},
        {PipelineState::RenderPassIsRenderToTexture, "rp_is_msrtt"},
        {PipelineState::RenderPassResolveDepth, "rp_resolve_depth"},
        {PipelineState::RenderPassResolveStencil, "rp_resolve_stencil"},
        {PipelineState::RenderPassUnresolveDepth, "rp_unresolve_depth"},
        {PipelineState::RenderPassUnresolveStencil, "rp_unresolve_stencil"},
        {PipelineState::RenderPassColorResolveMask, "rp_resolve_color"},
        {PipelineState::RenderPassColorUnresolveMask, "rp_unresolve_color"},
        {PipelineState::RenderPassColorFormat, "rp_color"},
        {PipelineState::RenderPassDepthStencilFormat, "rp_depth_stencil"},
        {PipelineState::Subpass, "subpass"},
        {PipelineState::Topology, "topology"},
        {PipelineState::PatchVertices, "patch_vertices"},
        {PipelineState::PrimitiveRestartEnable, "primitive_restart"},
        {PipelineState::PolygonMode, "polygon_mode"},
        {PipelineState::CullMode, "cull_mode"},
        {PipelineState::FrontFace, "front_face"},
        {PipelineState::SurfaceRotation, "rotated_surface"},
        {PipelineState::ViewportNegativeOneToOne, "viewport_depth_[-1,1]"},
        {PipelineState::SampleShadingEnable, "sample_shading"},
        {PipelineState::RasterizationSamples, "rasterization_samples"},
        {PipelineState::MinSampleShading, "min_sample_shading"},
        {PipelineState::SampleMask, "sample_mask"},
        {PipelineState::AlphaToCoverageEnable, "alpha_to_coverage"},
        {PipelineState::AlphaToOneEnable, "alpha_to_one"},
        {PipelineState::LogicOpEnable, "logic_op_enable"},
        {PipelineState::LogicOp, "logic_op"},
        {PipelineState::RasterizerDiscardEnable, "rasterization_discard"},
        {PipelineState::ColorWriteMask, "color_write"},
        {PipelineState::BlendEnableMask, "blend_mask"},
        {PipelineState::MissingOutputsMask, "missing_outputs_mask"},
        {PipelineState::SrcColorBlendFactor, "src_color_blend"},
        {PipelineState::DstColorBlendFactor, "dst_color_blend"},
        {PipelineState::ColorBlendOp, "color_blend"},
        {PipelineState::SrcAlphaBlendFactor, "src_alpha_blend"},
        {PipelineState::DstAlphaBlendFactor, "dst_alpha_blend"},
        {PipelineState::AlphaBlendOp, "alpha_blend"},
        {PipelineState::EmulatedDitherControl, "dither"},
        {PipelineState::DepthClampEnable, "depth_clamp"},
        {PipelineState::DepthBoundsTest, "depth_bounds_test"},
        {PipelineState::DepthCompareOp, "depth_compare"},
        {PipelineState::DepthTest, "depth_test"},
        {PipelineState::DepthWrite, "depth_write"},
        {PipelineState::StencilTest, "stencil_test"},
        {PipelineState::DepthBiasEnable, "depth_bias"},
        {PipelineState::StencilOpFailFront, "stencil_front_fail"},
        {PipelineState::StencilOpPassFront, "stencil_front_pass"},
        {PipelineState::StencilOpDepthFailFront, "stencil_front_depth_fail"},
        {PipelineState::StencilCompareFront, "stencil_front_compare"},
        {PipelineState::StencilOpFailBack, "stencil_back_fail"},
        {PipelineState::StencilOpPassBack, "stencil_back_pass"},
        {PipelineState::StencilOpDepthFailBack, "stencil_back_depth_fail"},
        {PipelineState::StencilCompareBack, "stencil_back_compare"},
    }};

    out << kStateNames[pipelineState];
    if (isRanged)
    {
        out << "_" << subIndex;
    }

    switch (pipelineState)
    {
        // Given that state == 0 produces no output, binary state doesn't require anything but
        // its name specified, as it being enabled would be implied.
        case PipelineState::VertexAttribCompressed:
        case PipelineState::RenderPassSrgbWriteControl:
        case PipelineState::RenderPassHasColorFramebufferFetch:
        case PipelineState::RenderPassHasDepthStencilFramebufferFetch:
        case PipelineState::RenderPassIsRenderToTexture:
        case PipelineState::RenderPassResolveDepth:
        case PipelineState::RenderPassResolveStencil:
        case PipelineState::RenderPassUnresolveDepth:
        case PipelineState::RenderPassUnresolveStencil:
        case PipelineState::PrimitiveRestartEnable:
        case PipelineState::SurfaceRotation:
        case PipelineState::ViewportNegativeOneToOne:
        case PipelineState::SampleShadingEnable:
        case PipelineState::AlphaToCoverageEnable:
        case PipelineState::AlphaToOneEnable:
        case PipelineState::LogicOpEnable:
        case PipelineState::RasterizerDiscardEnable:
        case PipelineState::DepthClampEnable:
        case PipelineState::DepthBoundsTest:
        case PipelineState::DepthTest:
        case PipelineState::DepthWrite:
        case PipelineState::StencilTest:
        case PipelineState::DepthBiasEnable:
            break;

        // Special formatting for some state
        case PipelineState::VertexAttribShaderComponentType:
            out << "=";
            switch (state)
            {
                case 0:
                    static_assert(static_cast<uint32_t>(gl::ComponentType::Float) == 0);
                    out << "float";
                    break;
                case 1:
                    static_assert(static_cast<uint32_t>(gl::ComponentType::Int) == 1);
                    out << "int";
                    break;
                case 2:
                    static_assert(static_cast<uint32_t>(gl::ComponentType::UnsignedInt) == 2);
                    out << "uint";
                    break;
                case 3:
                    static_assert(static_cast<uint32_t>(gl::ComponentType::NoType) == 3);
                    out << "none";
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case PipelineState::Topology:
            out << "=";
            switch (state)
            {
                case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
                    out << "points";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
                    out << "lines";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
                    out << "line_strip";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
                    out << "tris";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
                    out << "tri_strip";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
                    out << "tri_fan";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
                    out << "lines_with_adj";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
                    out << "line_strip_with_adj";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
                    out << "tris_with_adj";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
                    out << "tri_strip_with_adj";
                    break;
                case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
                    out << "patches";
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case PipelineState::PolygonMode:
            out << "=";
            switch (state)
            {
                case VK_POLYGON_MODE_FILL:
                    out << "fill";
                    break;
                case VK_POLYGON_MODE_LINE:
                    out << "line";
                    break;
                case VK_POLYGON_MODE_POINT:
                    out << "point";
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case PipelineState::CullMode:
            out << "=";
            if ((state & VK_CULL_MODE_FRONT_BIT) != 0)
            {
                out << "front";
            }
            if (state == VK_CULL_MODE_FRONT_AND_BACK)
            {
                out << "+";
            }
            if ((state & VK_CULL_MODE_BACK_BIT) != 0)
            {
                out << "back";
            }
            break;
        case PipelineState::FrontFace:
            out << "=" << (state == VK_FRONT_FACE_COUNTER_CLOCKWISE ? "ccw" : "cw");
            break;
        case PipelineState::MinSampleShading:
            out << "=" << (static_cast<float>(state) / kMinSampleShadingScale);
            break;
        case PipelineState::SrcColorBlendFactor:
        case PipelineState::DstColorBlendFactor:
        case PipelineState::SrcAlphaBlendFactor:
        case PipelineState::DstAlphaBlendFactor:
            out << "=";
            switch (state)
            {
                case VK_BLEND_FACTOR_ZERO:
                    out << "0";
                    break;
                case VK_BLEND_FACTOR_ONE:
                    out << "1";
                    break;
                case VK_BLEND_FACTOR_SRC_COLOR:
                    out << "sc";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
                    out << "1-sc";
                    break;
                case VK_BLEND_FACTOR_DST_COLOR:
                    out << "dc";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
                    out << "1-dc";
                    break;
                case VK_BLEND_FACTOR_SRC_ALPHA:
                    out << "sa";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
                    out << "1-sa";
                    break;
                case VK_BLEND_FACTOR_DST_ALPHA:
                    out << "da";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
                    out << "1-da";
                    break;
                case VK_BLEND_FACTOR_CONSTANT_COLOR:
                    out << "const_color";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
                    out << "1-const_color";
                    break;
                case VK_BLEND_FACTOR_CONSTANT_ALPHA:
                    out << "const_alpha";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
                    out << "1-const_alpha";
                    break;
                case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
                    out << "sat(sa)";
                    break;
                case VK_BLEND_FACTOR_SRC1_COLOR:
                    out << "sc1";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR:
                    out << "1-sc1";
                    break;
                case VK_BLEND_FACTOR_SRC1_ALPHA:
                    out << "sa1";
                    break;
                case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA:
                    out << "1-sa1";
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case PipelineState::ColorBlendOp:
        case PipelineState::AlphaBlendOp:
            out << "=";
            switch (UnpackBlendOp(static_cast<uint8_t>(state)))
            {
                case VK_BLEND_OP_ADD:
                    out << "add";
                    break;
                case VK_BLEND_OP_SUBTRACT:
                    out << "sub";
                    break;
                case VK_BLEND_OP_REVERSE_SUBTRACT:
                    out << "reverse_sub";
                    break;
                case VK_BLEND_OP_MIN:
                    out << "min";
                    break;
                case VK_BLEND_OP_MAX:
                    out << "max";
                    break;
                case VK_BLEND_OP_MULTIPLY_EXT:
                    out << "multiply";
                    break;
                case VK_BLEND_OP_SCREEN_EXT:
                    out << "screen";
                    break;
                case VK_BLEND_OP_OVERLAY_EXT:
                    out << "overlay";
                    break;
                case VK_BLEND_OP_DARKEN_EXT:
                    out << "darken";
                    break;
                case VK_BLEND_OP_LIGHTEN_EXT:
                    out << "lighten";
                    break;
                case VK_BLEND_OP_COLORDODGE_EXT:
                    out << "dodge";
                    break;
                case VK_BLEND_OP_COLORBURN_EXT:
                    out << "burn";
                    break;
                case VK_BLEND_OP_HARDLIGHT_EXT:
                    out << "hardlight";
                    break;
                case VK_BLEND_OP_SOFTLIGHT_EXT:
                    out << "softlight";
                    break;
                case VK_BLEND_OP_DIFFERENCE_EXT:
                    out << "difference";
                    break;
                case VK_BLEND_OP_EXCLUSION_EXT:
                    out << "exclusion";
                    break;
                case VK_BLEND_OP_HSL_HUE_EXT:
                    out << "hsl_hue";
                    break;
                case VK_BLEND_OP_HSL_SATURATION_EXT:
                    out << "hsl_sat";
                    break;
                case VK_BLEND_OP_HSL_COLOR_EXT:
                    out << "hsl_color";
                    break;
                case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
                    out << "hsl_lum";
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case PipelineState::DepthCompareOp:
        case PipelineState::StencilCompareFront:
        case PipelineState::StencilCompareBack:
            out << "=";
            switch (state)
            {
                case VK_COMPARE_OP_NEVER:
                    out << "never";
                    break;
                case VK_COMPARE_OP_LESS:
                    out << "'<'";
                    break;
                case VK_COMPARE_OP_EQUAL:
                    out << "'='";
                    break;
                case VK_COMPARE_OP_LESS_OR_EQUAL:
                    out << "'<='";
                    break;
                case VK_COMPARE_OP_GREATER:
                    out << "'>'";
                    break;
                case VK_COMPARE_OP_NOT_EQUAL:
                    out << "'!='";
                    break;
                case VK_COMPARE_OP_GREATER_OR_EQUAL:
                    out << "'>='";
                    break;
                case VK_COMPARE_OP_ALWAYS:
                    out << "always";
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case PipelineState::StencilOpFailFront:
        case PipelineState::StencilOpPassFront:
        case PipelineState::StencilOpDepthFailFront:
        case PipelineState::StencilOpFailBack:
        case PipelineState::StencilOpPassBack:
        case PipelineState::StencilOpDepthFailBack:
            out << "=";
            switch (state)
            {
                case VK_STENCIL_OP_KEEP:
                    out << "keep";
                    break;
                case VK_STENCIL_OP_ZERO:
                    out << "0";
                    break;
                case VK_STENCIL_OP_REPLACE:
                    out << "replace";
                    break;
                case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
                    out << "clamp++";
                    break;
                case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
                    out << "clamp--";
                    break;
                case VK_STENCIL_OP_INVERT:
                    out << "'~'";
                    break;
                case VK_STENCIL_OP_INCREMENT_AND_WRAP:
                    out << "wrap++";
                    break;
                case VK_STENCIL_OP_DECREMENT_AND_WRAP:
                    out << "wrap--";
                    break;
                default:
                    UNREACHABLE();
            }
            break;

        // Some state output the value as hex because they are bitmasks
        case PipelineState::RenderPassColorResolveMask:
        case PipelineState::RenderPassColorUnresolveMask:
        case PipelineState::SampleMask:
        case PipelineState::ColorWriteMask:
        case PipelineState::BlendEnableMask:
        case PipelineState::MissingOutputsMask:
        case PipelineState::EmulatedDitherControl:
            out << "=0x" << std::hex << state << std::dec;
            break;

        // The rest will simply output the state value
        default:
            out << "=" << state;
            break;
    }

    out << "\\n";
}

[[maybe_unused]] void OutputAllPipelineState(ErrorContext *context,
                                             std::ostream &out,
                                             const UnpackedPipelineState &pipeline,
                                             GraphicsPipelineSubset subset,
                                             const PipelineStateBitSet &include,
                                             bool isCommonState)
{
    // Default non-existing state to 0, so they are automatically not output as
    // UnpackedPipelineState also sets them to 0.
    const bool hasVertexInput             = GraphicsPipelineHasVertexInput(subset);
    const bool hasShaders                 = GraphicsPipelineHasShaders(subset);
    const bool hasShadersOrFragmentOutput = GraphicsPipelineHasShadersOrFragmentOutput(subset);
    const bool hasFragmentOutput          = GraphicsPipelineHasFragmentOutput(subset);

    const angle::PackedEnumMap<PipelineState, uint32_t> kDefaultState = {{
        // Vertex input state
        {PipelineState::VertexAttribFormat,
         hasVertexInput
             ? static_cast<uint32_t>(GetCurrentValueFormatID(gl::VertexAttribType::Float))
             : 0},
        {PipelineState::VertexAttribDivisor, 0},
        {PipelineState::VertexAttribOffset, 0},
        {PipelineState::VertexAttribStride, 0},
        {PipelineState::VertexAttribCompressed, 0},
        {PipelineState::VertexAttribShaderComponentType, 0},
        {PipelineState::Topology, hasVertexInput ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : 0},
        {PipelineState::PrimitiveRestartEnable, 0},

        // Shaders state
        {PipelineState::ViewportNegativeOneToOne,
         hasShaders && context->getFeatures().supportsDepthClipControl.enabled},
        {PipelineState::DepthClampEnable, 0},
        {PipelineState::PolygonMode, hasShaders ? VK_POLYGON_MODE_FILL : 0},
        {PipelineState::CullMode, hasShaders ? VK_CULL_MODE_NONE : 0},
        {PipelineState::FrontFace, hasShaders ? VK_FRONT_FACE_COUNTER_CLOCKWISE : 0},
        {PipelineState::RasterizerDiscardEnable, 0},
        {PipelineState::DepthBiasEnable, 0},
        {PipelineState::PatchVertices, hasShaders ? 3 : 0},
        {PipelineState::DepthBoundsTest, 0},
        {PipelineState::DepthTest, 0},
        {PipelineState::DepthWrite, 0},
        {PipelineState::StencilTest, 0},
        {PipelineState::DepthCompareOp, hasShaders ? VK_COMPARE_OP_LESS : 0},
        {PipelineState::SurfaceRotation, 0},
        {PipelineState::EmulatedDitherControl, 0},
        {PipelineState::StencilOpFailFront, hasShaders ? VK_STENCIL_OP_KEEP : 0},
        {PipelineState::StencilOpPassFront, hasShaders ? VK_STENCIL_OP_KEEP : 0},
        {PipelineState::StencilOpDepthFailFront, hasShaders ? VK_STENCIL_OP_KEEP : 0},
        {PipelineState::StencilCompareFront, hasShaders ? VK_COMPARE_OP_ALWAYS : 0},
        {PipelineState::StencilOpFailBack, hasShaders ? VK_STENCIL_OP_KEEP : 0},
        {PipelineState::StencilOpPassBack, hasShaders ? VK_STENCIL_OP_KEEP : 0},
        {PipelineState::StencilOpDepthFailBack, hasShaders ? VK_STENCIL_OP_KEEP : 0},
        {PipelineState::StencilCompareBack, hasShaders ? VK_COMPARE_OP_ALWAYS : 0},

        // Shared shaders and fragment output state
        {PipelineState::SampleMask,
         hasShadersOrFragmentOutput ? std::numeric_limits<uint16_t>::max() : 0},
        {PipelineState::RasterizationSamples, hasShadersOrFragmentOutput ? 1 : 0},
        {PipelineState::SampleShadingEnable, 0},
        {PipelineState::MinSampleShading, hasShadersOrFragmentOutput ? kMinSampleShadingScale : 0},
        {PipelineState::AlphaToCoverageEnable, 0},
        {PipelineState::AlphaToOneEnable, 0},
        {PipelineState::RenderPassSamples, hasShadersOrFragmentOutput ? 1 : 0},
        {PipelineState::RenderPassColorAttachmentRange, 0},
        {PipelineState::RenderPassViewCount, 0},
        {PipelineState::RenderPassSrgbWriteControl, 0},
        {PipelineState::RenderPassHasColorFramebufferFetch, 0},
        {PipelineState::RenderPassHasDepthStencilFramebufferFetch, 0},
        {PipelineState::RenderPassIsRenderToTexture, 0},
        {PipelineState::RenderPassResolveDepth, 0},
        {PipelineState::RenderPassResolveStencil, 0},
        {PipelineState::RenderPassUnresolveDepth, 0},
        {PipelineState::RenderPassUnresolveStencil, 0},
        {PipelineState::RenderPassColorResolveMask, 0},
        {PipelineState::RenderPassColorUnresolveMask, 0},
        {PipelineState::RenderPassColorFormat, 0},
        {PipelineState::RenderPassDepthStencilFormat, 0},
        {PipelineState::Subpass, 0},

        // Fragment output state
        {PipelineState::ColorWriteMask, 0},
        {PipelineState::SrcColorBlendFactor, hasFragmentOutput ? VK_BLEND_FACTOR_ONE : 0},
        {PipelineState::DstColorBlendFactor, hasFragmentOutput ? VK_BLEND_FACTOR_ZERO : 0},
        {PipelineState::ColorBlendOp, hasFragmentOutput ? VK_BLEND_OP_ADD : 0},
        {PipelineState::SrcAlphaBlendFactor, hasFragmentOutput ? VK_BLEND_FACTOR_ONE : 0},
        {PipelineState::DstAlphaBlendFactor, hasFragmentOutput ? VK_BLEND_FACTOR_ZERO : 0},
        {PipelineState::AlphaBlendOp, hasFragmentOutput ? VK_BLEND_OP_ADD : 0},
        {PipelineState::BlendEnableMask, 0},
        {PipelineState::LogicOpEnable, 0},
        {PipelineState::LogicOp, hasFragmentOutput ? VK_LOGIC_OP_COPY : 0},
        {PipelineState::MissingOutputsMask, 0},
    }};

    bool anyStateOutput = false;
    for (size_t stateIndex : include)
    {
        size_t subIndex             = 0;
        bool isRanged               = false;
        PipelineState pipelineState = GetPipelineState(stateIndex, &isRanged, &subIndex);

        const uint32_t state = pipeline.data()[stateIndex];
        if (state != kDefaultState[pipelineState])
        {
            OutputPipelineState(out, stateIndex, state);
            anyStateOutput = true;
        }
    }

    if (!isCommonState)
    {
        out << "(" << (anyStateOutput ? "+" : "") << "common state)\\n";
    }
}

template <typename Hash>
void DumpPipelineCacheGraph(
    ErrorContext *context,
    const std::unordered_map<GraphicsPipelineDesc,
                             PipelineHelper,
                             Hash,
                             typename GraphicsPipelineCacheTypeHelper<Hash>::KeyEqual> &cache)
{
    constexpr GraphicsPipelineSubset kSubset = GraphicsPipelineCacheTypeHelper<Hash>::kSubset;

    std::ostream &out = context->getRenderer()->getPipelineCacheGraphStream();

    static std::atomic<uint32_t> sCacheSerial(0);
    angle::HashMap<GraphicsPipelineDesc, uint32_t, Hash,
                   typename GraphicsPipelineCacheTypeHelper<Hash>::KeyEqual>
        descToId;

    uint32_t cacheSerial = sCacheSerial.fetch_add(1);
    uint32_t descId      = 0;

    // Unpack pipeline states
    std::vector<UnpackedPipelineState> pipelines(cache.size());
    for (const auto &descAndPipeline : cache)
    {
        UnpackPipelineState(descAndPipeline.first, kSubset, &pipelines[descId++]);
    }

    // Extract common state between all pipelines.
    PipelineStateBitSet commonState = GetCommonPipelineState(pipelines);
    PipelineStateBitSet nodeState   = ~commonState;

    const char *subsetDescription = "";
    const char *subsetTag         = "";
    switch (kSubset)
    {
        case GraphicsPipelineSubset::VertexInput:
            subsetDescription = "(vertex input)\\n";
            subsetTag         = "VI_";
            break;
        case GraphicsPipelineSubset::Shaders:
            subsetDescription = "(shaders)\\n";
            subsetTag         = "S_";
            break;
        case GraphicsPipelineSubset::FragmentOutput:
            subsetDescription = "(fragment output)\\n";
            subsetTag         = "FO_";
            break;
        default:
            break;
    }

    out << " subgraph cluster_" << subsetTag << cacheSerial << "{\n";
    out << "  label=\"Program " << cacheSerial << "\\n"
        << subsetDescription << "\\nCommon state:\\n";
    OutputAllPipelineState(context, out, pipelines[0], kSubset, commonState, true);
    out << "\";\n";

    descId = 0;
    for (const auto &descAndPipeline : cache)
    {
        const GraphicsPipelineDesc &desc = descAndPipeline.first;

        const char *style        = "";
        const char *feedbackDesc = "";
        switch (descAndPipeline.second.getCacheLookUpFeedback())
        {
            case CacheLookUpFeedback::Hit:
                // Default is green already
                break;
            case CacheLookUpFeedback::Miss:
                style = "[color=red]";
                break;
            case CacheLookUpFeedback::LinkedDrawHit:
                // Default is green already
                style        = "[style=dotted]";
                feedbackDesc = "(linked)\\n";
                break;
            case CacheLookUpFeedback::LinkedDrawMiss:
                style        = "[style=dotted,color=red]";
                feedbackDesc = "(linked)\\n";
                break;
            case CacheLookUpFeedback::WarmUpHit:
                // Default is green already
                style        = "[style=dashed]";
                feedbackDesc = "(warm up)\\n";
                break;
            case CacheLookUpFeedback::WarmUpMiss:
                style        = "[style=dashed,color=red]";
                feedbackDesc = "(warm up)\\n";
                break;
            case CacheLookUpFeedback::UtilsHit:
                style        = "[color=yellow]";
                feedbackDesc = "(utils)\\n";
                break;
            case CacheLookUpFeedback::UtilsMiss:
                style        = "[color=purple]";
                feedbackDesc = "(utils)\\n";
                break;
            default:
                // No feedback available
                break;
        }

        out << "  p" << subsetTag << cacheSerial << "_" << descId << "[label=\"Pipeline " << descId
            << "\\n"
            << feedbackDesc << "\\n";
        OutputAllPipelineState(context, out, pipelines[descId], kSubset, nodeState, false);
        out << "\"]" << style << ";\n";

        descToId[desc] = descId++;
    }
    for (const auto &descAndPipeline : cache)
    {
        const GraphicsPipelineDesc &desc     = descAndPipeline.first;
        const PipelineHelper &pipelineHelper = descAndPipeline.second;
        const std::vector<GraphicsPipelineTransition> &transitions =
            pipelineHelper.getTransitions();

        for (const GraphicsPipelineTransition &transition : transitions)
        {
#if defined(ANGLE_IS_64_BIT_CPU)
            const uint64_t transitionBits = transition.bits.bits();
#else
            const uint64_t transitionBits =
                static_cast<uint64_t>(transition.bits.bits(1)) << 32 | transition.bits.bits(0);
#endif
            out << "  p" << subsetTag << cacheSerial << "_" << descToId[desc] << " -> p"
                << subsetTag << cacheSerial << "_" << descToId[*transition.desc] << " [label=\"'0x"
                << std::hex << transitionBits << std::dec << "'\"];\n";
        }
    }
    out << " }\n";
}

// Used by SharedCacheKeyManager
void MakeInvalidCachedObject(SharedFramebufferCacheKey *cacheKeyOut)
{
    *cacheKeyOut = SharedFramebufferCacheKey::MakeShared(VK_NULL_HANDLE);
    // So that it will mark as invalid.
    (*cacheKeyOut)->destroy(VK_NULL_HANDLE);
}

void MakeInvalidCachedObject(SharedDescriptorSetCacheKey *cacheKeyOut)
{
    *cacheKeyOut = SharedDescriptorSetCacheKey::MakeShared(VK_NULL_HANDLE);
}

angle::Result InitializePipelineFromLibraries(ErrorContext *context,
                                              PipelineCacheAccess *pipelineCache,
                                              const vk::PipelineLayout &pipelineLayout,
                                              const vk::PipelineHelper &vertexInputPipeline,
                                              const vk::PipelineHelper &shadersPipeline,
                                              const vk::PipelineHelper &fragmentOutputPipeline,
                                              const vk::GraphicsPipelineDesc &desc,
                                              Pipeline *pipelineOut,
                                              CacheLookUpFeedback *feedbackOut)
{
    // Nothing in the create info, everything comes from the libraries.
    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.layout                       = pipelineLayout.getHandle();

    if (context->getFeatures().preferDynamicRendering.enabled && desc.getRenderPassFoveation())
    {
        createInfo.flags |= VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    }

    const std::array<VkPipeline, 3> pipelines = {
        vertexInputPipeline.getPipeline().getHandle(),
        shadersPipeline.getPipeline().getHandle(),
        fragmentOutputPipeline.getPipeline().getHandle(),
    };

    // Specify the three subsets as input libraries.
    VkPipelineLibraryCreateInfoKHR libraryInfo = {};
    libraryInfo.sType                          = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
    libraryInfo.libraryCount                   = 3;
    libraryInfo.pLibraries                     = pipelines.data();

    AddToPNextChain(&createInfo, &libraryInfo);

    // If supported, get feedback.
    VkPipelineCreationFeedback feedback               = {};
    VkPipelineCreationFeedbackCreateInfo feedbackInfo = {};
    feedbackInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO;

    const bool supportsFeedback = context->getFeatures().supportsPipelineCreationFeedback.enabled;
    if (supportsFeedback)
    {
        feedbackInfo.pPipelineCreationFeedback = &feedback;
        AddToPNextChain(&createInfo, &feedbackInfo);
    }

    // Create the pipeline
    ANGLE_VK_TRY(context, pipelineCache->createGraphicsPipeline(context, createInfo, pipelineOut));

    if (supportsFeedback)
    {
        const bool cacheHit =
            (feedback.flags & VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT) !=
            0;

        *feedbackOut = cacheHit ? CacheLookUpFeedback::Hit : CacheLookUpFeedback::Miss;
        ApplyPipelineCreationFeedback(context, feedback);
    }

    return angle::Result::Continue;
}

bool ShouldDumpPipelineCacheGraph(ErrorContext *context)
{
    return kDumpPipelineCacheGraph && context->getRenderer()->isPipelineCacheGraphDumpEnabled();
}
}  // anonymous namespace

FramebufferFetchMode GetProgramFramebufferFetchMode(const gl::ProgramExecutable *executable)
{
    if (executable == nullptr)
    {
        return FramebufferFetchMode::None;
    }

    const bool hasColorFramebufferFetch = executable->usesColorFramebufferFetch();
    const bool hasDepthStencilFramebufferFetch =
        executable->usesDepthFramebufferFetch() || executable->usesStencilFramebufferFetch();

    if (hasDepthStencilFramebufferFetch)
    {
        return hasColorFramebufferFetch ? FramebufferFetchMode::ColorAndDepthStencil
                                        : FramebufferFetchMode::DepthStencil;
    }
    else
    {
        return hasColorFramebufferFetch ? FramebufferFetchMode::Color : FramebufferFetchMode::None;
    }
}

GraphicsPipelineTransitionBits GetGraphicsPipelineTransitionBitsMask(GraphicsPipelineSubset subset)
{
    switch (subset)
    {
        case GraphicsPipelineSubset::VertexInput:
            return kPipelineVertexInputTransitionBitsMask;
        case GraphicsPipelineSubset::Shaders:
            return kPipelineShadersTransitionBitsMask;
        case GraphicsPipelineSubset::FragmentOutput:
            return kPipelineFragmentOutputTransitionBitsMask;
        default:
            break;
    }

    ASSERT(subset == GraphicsPipelineSubset::Complete);

    GraphicsPipelineTransitionBits allBits;
    allBits.set();

    return allBits;
}

// RenderPassDesc implementation.
RenderPassDesc::RenderPassDesc()
{
    memset(this, 0, sizeof(RenderPassDesc));
}

RenderPassDesc::~RenderPassDesc() = default;

RenderPassDesc::RenderPassDesc(const RenderPassDesc &other)
{
    memcpy(this, &other, sizeof(RenderPassDesc));
}

void RenderPassDesc::packColorAttachment(size_t colorIndexGL, angle::FormatID formatID)
{
    ASSERT(colorIndexGL < mAttachmentFormats.size());
    static_assert(angle::kNumANGLEFormats < std::numeric_limits<uint8_t>::max(),
                  "Too many ANGLE formats to fit in uint8_t");
    // Force the user to pack the depth/stencil attachment last.
    ASSERT(!hasDepthStencilAttachment());
    // This function should only be called for enabled GL color attachments.
    ASSERT(formatID != angle::FormatID::NONE);

    uint8_t &packedFormat = mAttachmentFormats[colorIndexGL];
    SetBitField(packedFormat, formatID);

    // Set color attachment range such that it covers the range from index 0 through last active
    // index.  This is the reasons why we need depth/stencil to be packed last.
    SetBitField(mColorAttachmentRange, std::max<size_t>(mColorAttachmentRange, colorIndexGL + 1));
}

void RenderPassDesc::packColorAttachmentGap(size_t colorIndexGL)
{
    ASSERT(colorIndexGL < mAttachmentFormats.size());
    static_assert(angle::kNumANGLEFormats < std::numeric_limits<uint8_t>::max(),
                  "Too many ANGLE formats to fit in uint8_t");
    // Force the user to pack the depth/stencil attachment last.
    ASSERT(!hasDepthStencilAttachment());

    // Use NONE as a flag for gaps in GL color attachments.
    uint8_t &packedFormat = mAttachmentFormats[colorIndexGL];
    SetBitField(packedFormat, angle::FormatID::NONE);
}

void RenderPassDesc::packDepthStencilAttachment(angle::FormatID formatID)
{
    ASSERT(!hasDepthStencilAttachment());

    size_t index = depthStencilAttachmentIndex();
    ASSERT(index < mAttachmentFormats.size());

    uint8_t &packedFormat = mAttachmentFormats[index];
    SetBitField(packedFormat, formatID);
}

void RenderPassDesc::packColorResolveAttachment(size_t colorIndexGL)
{
    ASSERT(isColorAttachmentEnabled(colorIndexGL));
    ASSERT(!mColorResolveAttachmentMask.test(colorIndexGL));
    ASSERT(mSamples > 1);
    mColorResolveAttachmentMask.set(colorIndexGL);
}

void RenderPassDesc::packYUVResolveAttachment(size_t colorIndexGL)
{
    ASSERT(isColorAttachmentEnabled(colorIndexGL));
    ASSERT(!mColorResolveAttachmentMask.test(colorIndexGL));
    mColorResolveAttachmentMask.set(colorIndexGL);
    SetBitField(mIsYUVResolve, 1);
}

void RenderPassDesc::removeColorResolveAttachment(size_t colorIndexGL)
{
    ASSERT(mColorResolveAttachmentMask.test(colorIndexGL));
    mColorResolveAttachmentMask.reset(colorIndexGL);
}

void RenderPassDesc::packColorUnresolveAttachment(size_t colorIndexGL)
{
    mColorUnresolveAttachmentMask.set(colorIndexGL);
}

void RenderPassDesc::removeColorUnresolveAttachment(size_t colorIndexGL)
{
    mColorUnresolveAttachmentMask.reset(colorIndexGL);
}

void RenderPassDesc::packDepthResolveAttachment()
{
    ASSERT(hasDepthStencilAttachment());
    ASSERT(!hasDepthResolveAttachment());

    mResolveDepth = true;
}

void RenderPassDesc::packStencilResolveAttachment()
{
    ASSERT(hasDepthStencilAttachment());
    ASSERT(!hasStencilResolveAttachment());

    mResolveStencil = true;
}

void RenderPassDesc::packDepthUnresolveAttachment()
{
    ASSERT(hasDepthStencilAttachment());

    mUnresolveDepth = true;
}

void RenderPassDesc::packStencilUnresolveAttachment()
{
    ASSERT(hasDepthStencilAttachment());

    mUnresolveStencil = true;
}

void RenderPassDesc::removeDepthStencilUnresolveAttachment()
{
    mUnresolveDepth   = false;
    mUnresolveStencil = false;
}

PackedAttachmentIndex RenderPassDesc::getPackedColorAttachmentIndex(size_t colorIndexGL)
{
    ASSERT(colorIndexGL < colorAttachmentRange());
    ASSERT(isColorAttachmentEnabled(colorIndexGL));

    vk::PackedAttachmentIndex colorIndexVk(0);
    for (uint32_t index = 0; index < colorIndexGL; ++index)
    {
        if (isColorAttachmentEnabled(index))
        {
            ++colorIndexVk;
        }
    }

    return colorIndexVk;
}

RenderPassDesc &RenderPassDesc::operator=(const RenderPassDesc &other)
{
    memcpy(this, &other, sizeof(RenderPassDesc));
    return *this;
}

void RenderPassDesc::setWriteControlMode(gl::SrgbWriteControlMode mode)
{
    SetBitField(mSrgbWriteControl, mode);
}

size_t RenderPassDesc::hash() const
{
    return angle::ComputeGenericHash(*this);
}

bool RenderPassDesc::isColorAttachmentEnabled(size_t colorIndexGL) const
{
    angle::FormatID formatID = operator[](colorIndexGL);
    return formatID != angle::FormatID::NONE;
}

bool RenderPassDesc::hasDepthStencilAttachment() const
{
    angle::FormatID formatID = operator[](depthStencilAttachmentIndex());
    return formatID != angle::FormatID::NONE;
}

size_t RenderPassDesc::clearableAttachmentCount() const
{
    size_t colorAttachmentCount = 0;
    for (size_t i = 0; i < mColorAttachmentRange; ++i)
    {
        colorAttachmentCount += isColorAttachmentEnabled(i);
    }

    // Note that there are no gaps in depth/stencil attachments.  In fact there is a maximum of 1 of
    // it + 1 for its resolve attachment.
    size_t depthStencilCount        = hasDepthStencilAttachment() ? 1 : 0;
    size_t depthStencilResolveCount = hasDepthStencilResolveAttachment() ? 1 : 0;
    return colorAttachmentCount + mColorResolveAttachmentMask.count() + depthStencilCount +
           depthStencilResolveCount;
}

size_t RenderPassDesc::attachmentCount() const
{
    return clearableAttachmentCount() + (hasFragmentShadingAttachment() ? 1 : 0);
}

void RenderPassDesc::setLegacyDither(bool enabled)
{
    SetBitField(mLegacyDitherEnabled, enabled ? 1 : 0);
}

void RenderPassDesc::beginRenderPass(
    ErrorContext *context,
    PrimaryCommandBuffer *primary,
    const RenderPass &renderPass,
    VkFramebuffer framebuffer,
    const gl::Rectangle &renderArea,
    VkSubpassContents subpassContents,
    PackedClearValuesArray &clearValues,
    const VkRenderPassAttachmentBeginInfo *attachmentBeginInfo) const
{
    VkRenderPassBeginInfo beginInfo    = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext                    = attachmentBeginInfo;
    beginInfo.renderPass               = renderPass.getHandle();
    beginInfo.framebuffer              = framebuffer;
    beginInfo.renderArea.offset.x      = static_cast<uint32_t>(renderArea.x);
    beginInfo.renderArea.offset.y      = static_cast<uint32_t>(renderArea.y);
    beginInfo.renderArea.extent.width  = static_cast<uint32_t>(renderArea.width);
    beginInfo.renderArea.extent.height = static_cast<uint32_t>(renderArea.height);
    beginInfo.clearValueCount          = static_cast<uint32_t>(clearableAttachmentCount());
    beginInfo.pClearValues             = clearValues.data();

    primary->beginRenderPass(beginInfo, subpassContents);
}

void RenderPassDesc::beginRendering(
    ErrorContext *context,
    PrimaryCommandBuffer *primary,
    const gl::Rectangle &renderArea,
    VkSubpassContents subpassContents,
    const FramebufferAttachmentsVector<VkImageView> &attachmentViews,
    const AttachmentOpsArray &ops,
    PackedClearValuesArray &clearValues,
    uint32_t layerCount) const
{
    DynamicRenderingInfo info;
    DeriveRenderingInfo(context->getRenderer(), *this, DynamicRenderingInfoSubset::Full, renderArea,
                        subpassContents, attachmentViews, ops, clearValues, layerCount, &info);

    primary->beginRendering(info.renderingInfo);

    VkRenderingAttachmentLocationInfoKHR attachmentLocations = {};
    attachmentLocations.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_LOCATION_INFO_KHR;
    attachmentLocations.colorAttachmentCount      = info.renderingInfo.colorAttachmentCount;
    attachmentLocations.pColorAttachmentLocations = info.colorAttachmentLocations.data();

    primary->setRenderingAttachmentLocations(&attachmentLocations);

    if (hasColorFramebufferFetch())
    {
        VkRenderingInputAttachmentIndexInfoKHR inputLocations = {};
        inputLocations.sType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR;
        inputLocations.colorAttachmentCount         = info.renderingInfo.colorAttachmentCount;
        inputLocations.pColorAttachmentInputIndices = info.colorAttachmentLocations.data();

        primary->setRenderingInputAttachmentIndicates(&inputLocations);
    }
}

void RenderPassDesc::populateRenderingInheritanceInfo(
    Renderer *renderer,
    VkCommandBufferInheritanceRenderingInfo *infoOut,
    gl::DrawBuffersArray<VkFormat> *colorFormatStorageOut) const
{
    DynamicRenderingInfo renderingInfo;
    DeriveRenderingInfo(renderer, *this, DynamicRenderingInfoSubset::Pipeline, {},
                        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, {}, {}, {}, 0,
                        &renderingInfo);
    *colorFormatStorageOut = renderingInfo.colorAttachmentFormats;

    *infoOut       = {};
    infoOut->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
    infoOut->flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    if (isLegacyDitherEnabled())
    {
        ASSERT(renderer->getFeatures().supportsLegacyDithering.enabled);
        infoOut->flags |= VK_RENDERING_ENABLE_LEGACY_DITHERING_BIT_EXT;
    }
    infoOut->viewMask                = renderingInfo.renderingInfo.viewMask;
    infoOut->colorAttachmentCount    = renderingInfo.renderingInfo.colorAttachmentCount;
    infoOut->pColorAttachmentFormats = colorFormatStorageOut->data();
    infoOut->depthAttachmentFormat   = renderingInfo.depthAttachmentFormat;
    infoOut->stencilAttachmentFormat = renderingInfo.stencilAttachmentFormat;
    infoOut->rasterizationSamples =
        gl_vk::GetSamples(samples(), renderer->getFeatures().limitSampleCountTo2.enabled);
}

void RenderPassDesc::updatePerfCounters(
    ErrorContext *context,
    const FramebufferAttachmentsVector<VkImageView> &attachmentViews,
    const AttachmentOpsArray &ops,
    angle::VulkanPerfCounters *countersOut)
{
    DynamicRenderingInfo info;
    DeriveRenderingInfo(context->getRenderer(), *this, DynamicRenderingInfoSubset::Full, {},
                        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, attachmentViews, ops, {}, 0,
                        &info);

    // Note: resolve attachments don't have ops with dynamic rendering and they are implicit.
    // Counter-tests should take the |preferDynamicRendering| flag into account.  For color, it's
    // trivial to assume DONT_CARE/STORE, but it gets more complicated with depth/stencil when only
    // one aspect is resolved.

    for (uint32_t index = 0; index < info.renderingInfo.colorAttachmentCount; ++index)
    {
        const VkRenderingAttachmentInfo &colorInfo = info.renderingInfo.pColorAttachments[index];
        ASSERT(colorInfo.imageView != VK_NULL_HANDLE);

        countersOut->colorLoadOpClears += colorInfo.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
        countersOut->colorLoadOpLoads += colorInfo.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
        countersOut->colorLoadOpNones += colorInfo.loadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT ? 1 : 0;
        countersOut->colorStoreOpStores +=
            colorInfo.storeOp == VK_ATTACHMENT_STORE_OP_STORE ? 1 : 0;
        countersOut->colorStoreOpNones +=
            colorInfo.storeOp == VK_ATTACHMENT_STORE_OP_NONE_EXT ? 1 : 0;

        if (colorInfo.resolveMode != VK_RESOLVE_MODE_NONE)
        {
            countersOut->colorStoreOpStores += 1;
            countersOut->colorAttachmentResolves += 1;
        }
    }

    if (info.renderingInfo.pDepthAttachment != nullptr)
    {
        ASSERT(info.renderingInfo.pDepthAttachment->imageView != VK_NULL_HANDLE);

        const VkRenderingAttachmentInfo &depthInfo = *info.renderingInfo.pDepthAttachment;

        countersOut->depthLoadOpClears += depthInfo.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
        countersOut->depthLoadOpLoads += depthInfo.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
        countersOut->depthLoadOpNones += depthInfo.loadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT ? 1 : 0;
        countersOut->depthStoreOpStores +=
            depthInfo.storeOp == VK_ATTACHMENT_STORE_OP_STORE ? 1 : 0;
        countersOut->depthStoreOpNones +=
            depthInfo.storeOp == VK_ATTACHMENT_STORE_OP_NONE_EXT ? 1 : 0;

        if (depthInfo.resolveMode != VK_RESOLVE_MODE_NONE)
        {
            countersOut->depthStoreOpStores += 1;
            countersOut->depthAttachmentResolves += 1;
        }
    }

    if (info.renderingInfo.pStencilAttachment != nullptr)
    {
        ASSERT(info.renderingInfo.pStencilAttachment->imageView != VK_NULL_HANDLE);

        const VkRenderingAttachmentInfo &stencilInfo = *info.renderingInfo.pStencilAttachment;

        countersOut->stencilLoadOpClears +=
            stencilInfo.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR ? 1 : 0;
        countersOut->stencilLoadOpLoads += stencilInfo.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD ? 1 : 0;
        countersOut->stencilLoadOpNones +=
            stencilInfo.loadOp == VK_ATTACHMENT_LOAD_OP_NONE_EXT ? 1 : 0;
        countersOut->stencilStoreOpStores +=
            stencilInfo.storeOp == VK_ATTACHMENT_STORE_OP_STORE ? 1 : 0;
        countersOut->stencilStoreOpNones +=
            stencilInfo.storeOp == VK_ATTACHMENT_STORE_OP_NONE_EXT ? 1 : 0;

        if (stencilInfo.resolveMode != VK_RESOLVE_MODE_NONE)
        {
            countersOut->stencilStoreOpStores += 1;
            countersOut->stencilAttachmentResolves += 1;
        }
    }

    if (info.renderingInfo.pDepthAttachment != nullptr ||
        info.renderingInfo.pStencilAttachment != nullptr)
    {
        ASSERT(info.renderingInfo.pDepthAttachment == nullptr ||
               info.renderingInfo.pStencilAttachment == nullptr ||
               info.renderingInfo.pDepthAttachment->imageLayout ==
                   info.renderingInfo.pStencilAttachment->imageLayout);

        const VkImageLayout layout = info.renderingInfo.pDepthAttachment != nullptr
                                         ? info.renderingInfo.pDepthAttachment->imageLayout
                                         : info.renderingInfo.pStencilAttachment->imageLayout;

        countersOut->readOnlyDepthStencilRenderPasses +=
            layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ? 1 : 0;
    }
}

bool operator==(const RenderPassDesc &lhs, const RenderPassDesc &rhs)
{
    return memcmp(&lhs, &rhs, sizeof(RenderPassDesc)) == 0;
}

// Compute Pipeline Description implementation.
// Use aligned allocation and free so we can use the alignas keyword.
void *ComputePipelineDesc::operator new(std::size_t size)
{
    return angle::AlignedAlloc(size, 32);
}

void ComputePipelineDesc::operator delete(void *ptr)
{
    return angle::AlignedFree(ptr);
}

ComputePipelineDesc::ComputePipelineDesc()
{
    ASSERT(mPipelineOptions.permutationIndex == 0);
    ASSERT(std::all_of(mPadding, mPadding + sizeof(mPadding), [](char c) { return c == 0; }));
}

ComputePipelineDesc::ComputePipelineDesc(const ComputePipelineDesc &other)
    : mConstantIds{other.getConstantIds()},
      mConstants{other.getConstants()},
      mPipelineOptions{other.getPipelineOptions()}
{}

ComputePipelineDesc &ComputePipelineDesc::operator=(const ComputePipelineDesc &other)
{
    mPipelineOptions = other.getPipelineOptions();
    mConstantIds     = other.getConstantIds();
    mConstants       = other.getConstants();
    return *this;
}

ComputePipelineDesc::ComputePipelineDesc(VkSpecializationInfo *specializationInfo,
                                         ComputePipelineOptions pipelineOptions)
    : mConstantIds{}, mConstants{}, mPipelineOptions{pipelineOptions}
{
    if (specializationInfo != nullptr && specializationInfo->pMapEntries &&
        specializationInfo->mapEntryCount != 0)
    {
        const VkSpecializationMapEntry *mapEntries = specializationInfo->pMapEntries;
        mConstantIds.resize(specializationInfo->mapEntryCount);
        for (size_t mapEntryCount = 0; mapEntryCount < mConstantIds.size(); mapEntryCount++)
            mConstantIds[mapEntryCount] = mapEntries[mapEntryCount].constantID;
    }
    if (specializationInfo != nullptr && specializationInfo->pData &&
        specializationInfo->dataSize != 0)
    {
        const uint32_t *constDataEntries = (const uint32_t *)specializationInfo->pData;
        mConstants.resize(specializationInfo->dataSize / sizeof(uint32_t));
        for (size_t constantEntryCount = 0; constantEntryCount < mConstants.size();
             constantEntryCount++)
            mConstants[constantEntryCount] = constDataEntries[constantEntryCount];
    }
}

size_t ComputePipelineDesc::hash() const
{
    // Union is static-asserted, just another sanity check here
    ASSERT(sizeof(ComputePipelineOptions) == 1);

    size_t paddedPipelineOptions = mPipelineOptions.permutationIndex;
    size_t pipelineOptionsHash =
        angle::ComputeGenericHash(&paddedPipelineOptions, sizeof(paddedPipelineOptions));

    size_t specializationConstantIDsHash = 0;
    if (!mConstantIds.empty())
    {
        specializationConstantIDsHash =
            angle::ComputeGenericHash(mConstantIds.data(), mConstantIds.size() * sizeof(uint32_t));
    }

    size_t specializationConstantsHash = 0;
    if (!mConstants.empty())
    {
        specializationConstantsHash =
            angle::ComputeGenericHash(mConstants.data(), mConstants.size() * sizeof(uint32_t));
    }

    return pipelineOptionsHash ^ specializationConstantIDsHash ^ specializationConstantsHash;
}

bool ComputePipelineDesc::keyEqual(const ComputePipelineDesc &other) const
{
    return mPipelineOptions.permutationIndex == other.getPipelineOptions().permutationIndex &&
           mConstantIds == other.getConstantIds() && mConstants == other.getConstants();
}

// GraphicsPipelineDesc implementation.
// Use aligned allocation and free so we can use the alignas keyword.
void *GraphicsPipelineDesc::operator new(std::size_t size)
{
    return angle::AlignedAlloc(size, 32);
}

void GraphicsPipelineDesc::operator delete(void *ptr)
{
    return angle::AlignedFree(ptr);
}

GraphicsPipelineDesc::GraphicsPipelineDesc()
{
    memset(this, 0, sizeof(GraphicsPipelineDesc));
}

GraphicsPipelineDesc::~GraphicsPipelineDesc() = default;

GraphicsPipelineDesc::GraphicsPipelineDesc(const GraphicsPipelineDesc &other)
{
    *this = other;
}

GraphicsPipelineDesc &GraphicsPipelineDesc::operator=(const GraphicsPipelineDesc &other)
{
    memcpy(this, &other, sizeof(*this));
    return *this;
}

const void *GraphicsPipelineDesc::getPipelineSubsetMemory(GraphicsPipelineSubset subset,
                                                          size_t *sizeOut) const
{
    // GraphicsPipelineDesc must be laid out such that the three subsets are contiguous.  The layout
    // is:
    //
    //     Shaders State                 \
    //                                    )--> Pre-rasterization + fragment subset
    //     Shared Non-Vertex-Input State /  \
    //                                       )--> fragment output subset
    //     Fragment Output State            /
    //
    //     Vertex Input State            ----> Vertex input subset
    static_assert(offsetof(GraphicsPipelineDesc, mShaders) == kPipelineShadersDescOffset);
    static_assert(offsetof(GraphicsPipelineDesc, mSharedNonVertexInput) ==
                  kPipelineShadersDescOffset + kGraphicsPipelineShadersStateSize);
    static_assert(offsetof(GraphicsPipelineDesc, mSharedNonVertexInput) ==
                  kPipelineFragmentOutputDescOffset);
    static_assert(offsetof(GraphicsPipelineDesc, mFragmentOutput) ==
                  kPipelineFragmentOutputDescOffset +
                      kGraphicsPipelineSharedNonVertexInputStateSize);
    static_assert(offsetof(GraphicsPipelineDesc, mVertexInput) == kPipelineVertexInputDescOffset);

    // Exclude the full vertex or only vertex strides from the hash. It's conveniently placed last,
    // so it would be easy to exclude it from hash.
    static_assert(offsetof(GraphicsPipelineDesc, mVertexInput.vertex.strides) +
                      sizeof(PackedVertexInputAttributes::strides) ==
                  sizeof(GraphicsPipelineDesc));
    static_assert(offsetof(GraphicsPipelineDesc, mVertexInput.vertex) +
                      sizeof(PackedVertexInputAttributes) ==
                  sizeof(GraphicsPipelineDesc));

    size_t vertexInputReduceSize = 0;
    if (mVertexInput.inputAssembly.bits.useVertexInputBindingStrideDynamicState)
    {
        vertexInputReduceSize = sizeof(PackedVertexInputAttributes::strides);
    }
    else if (mVertexInput.inputAssembly.bits.useVertexInputDynamicState)
    {
        vertexInputReduceSize = sizeof(PackedVertexInputAttributes);
    }

    switch (subset)
    {
        case GraphicsPipelineSubset::VertexInput:
            *sizeOut = kPipelineVertexInputDescSize - vertexInputReduceSize;
            return &mVertexInput;

        case GraphicsPipelineSubset::Shaders:
            *sizeOut = kPipelineShadersDescSize;
            return &mShaders;

        case GraphicsPipelineSubset::FragmentOutput:
            *sizeOut = kPipelineFragmentOutputDescSize;
            return &mSharedNonVertexInput;

        case GraphicsPipelineSubset::Complete:
        default:
            *sizeOut = sizeof(*this) - vertexInputReduceSize;
            return this;
    }
}

size_t GraphicsPipelineDesc::hash(GraphicsPipelineSubset subset) const
{
    size_t keySize  = 0;
    const void *key = getPipelineSubsetMemory(subset, &keySize);

    return angle::ComputeGenericHash(key, keySize);
}

bool GraphicsPipelineDesc::keyEqual(const GraphicsPipelineDesc &other,
                                    GraphicsPipelineSubset subset) const
{
    size_t keySize  = 0;
    const void *key = getPipelineSubsetMemory(subset, &keySize);

    size_t otherKeySize  = 0;
    const void *otherKey = other.getPipelineSubsetMemory(subset, &otherKeySize);

    // Compare the relevant part of the desc memory.  Note that due to workarounds (e.g.
    // useVertexInputBindingStrideDynamicState), |this| or |other| may produce different key sizes.
    // In that case, comparing the minimum of the two is sufficient; if the workarounds are
    // different, the comparison would fail anyway.
    return memcmp(key, otherKey, std::min(keySize, otherKeySize)) == 0;
}

// Initialize PSO states, it is consistent with initial value of gl::State.
//
// Some states affect the pipeline, but they are not derived from the GL state, but rather the
// properties of the Vulkan device or the context itself; such as whether a workaround is in
// effect, or the context is robust.  For VK_EXT_graphics_pipeline_library, such state that affects
// multiple subsets of the pipeline is duplicated in each subset (for example, there are two
// copies of isRobustContext, one for vertex input and one for shader stages).
void GraphicsPipelineDesc::initDefaults(const ErrorContext *context,
                                        GraphicsPipelineSubset subset,
                                        PipelineRobustness pipelineRobustness,
                                        PipelineProtectedAccess pipelineProtectedAccess)
{
    if (GraphicsPipelineHasVertexInput(subset))
    {
        // Set all vertex input attributes to default, the default format is Float
        angle::FormatID defaultFormat = GetCurrentValueFormatID(gl::VertexAttribType::Float);
        for (PackedAttribDesc &packedAttrib : mVertexInput.vertex.attribs)
        {
            SetBitField(packedAttrib.divisor, 0);
            SetBitField(packedAttrib.format, defaultFormat);
            SetBitField(packedAttrib.compressed, 0);
            SetBitField(packedAttrib.offset, 0);
        }
        mVertexInput.vertex.shaderAttribComponentType = 0;
        memset(mVertexInput.vertex.strides, 0, sizeof(mVertexInput.vertex.strides));

        SetBitField(mVertexInput.inputAssembly.bits.topology, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        mVertexInput.inputAssembly.bits.primitiveRestartEnable = 0;
        mVertexInput.inputAssembly.bits.useVertexInputBindingStrideDynamicState =
            context->getFeatures().useVertexInputBindingStrideDynamicState.enabled;
        mVertexInput.inputAssembly.bits.useVertexInputDynamicState =
            context->getFeatures().supportsVertexInputDynamicState.enabled;
        mVertexInput.inputAssembly.bits.padding = 0;
    }

    if (GraphicsPipelineHasShaders(subset))
    {
        mShaders.shaders.bits.viewportNegativeOneToOne =
            context->getFeatures().supportsDepthClipControl.enabled;
        mShaders.shaders.bits.depthClampEnable = 0;
        SetBitField(mShaders.shaders.bits.polygonMode, VK_POLYGON_MODE_FILL);
        SetBitField(mShaders.shaders.bits.cullMode, VK_CULL_MODE_NONE);
        SetBitField(mShaders.shaders.bits.frontFace, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        mShaders.shaders.bits.rasterizerDiscardEnable = 0;
        mShaders.shaders.bits.depthBiasEnable         = 0;
        SetBitField(mShaders.shaders.bits.patchVertices, 3);
        mShaders.shaders.bits.depthBoundsTest                   = 0;
        mShaders.shaders.bits.depthTest                         = 0;
        mShaders.shaders.bits.depthWrite                        = 0;
        mShaders.shaders.bits.stencilTest                       = 0;
        mShaders.shaders.bits.nonZeroStencilWriteMaskWorkaround = 0;
        SetBitField(mShaders.shaders.bits.depthCompareOp, VK_COMPARE_OP_LESS);
        mShaders.shaders.bits.surfaceRotation  = 0;
        mShaders.shaders.emulatedDitherControl = 0;
        mShaders.shaders.padding               = 0;
        SetBitField(mShaders.shaders.front.fail, VK_STENCIL_OP_KEEP);
        SetBitField(mShaders.shaders.front.pass, VK_STENCIL_OP_KEEP);
        SetBitField(mShaders.shaders.front.depthFail, VK_STENCIL_OP_KEEP);
        SetBitField(mShaders.shaders.front.compare, VK_COMPARE_OP_ALWAYS);
        SetBitField(mShaders.shaders.back.fail, VK_STENCIL_OP_KEEP);
        SetBitField(mShaders.shaders.back.pass, VK_STENCIL_OP_KEEP);
        SetBitField(mShaders.shaders.back.depthFail, VK_STENCIL_OP_KEEP);
        SetBitField(mShaders.shaders.back.compare, VK_COMPARE_OP_ALWAYS);
    }

    if (GraphicsPipelineHasShadersOrFragmentOutput(subset))
    {
        mSharedNonVertexInput.multisample.bits.sampleMask = std::numeric_limits<uint16_t>::max();
        mSharedNonVertexInput.multisample.bits.rasterizationSamplesMinusOne = 0;
        mSharedNonVertexInput.multisample.bits.sampleShadingEnable          = 0;
        mSharedNonVertexInput.multisample.bits.alphaToCoverageEnable        = 0;
        mSharedNonVertexInput.multisample.bits.alphaToOneEnable             = 0;
        mSharedNonVertexInput.multisample.bits.subpass                      = 0;
        mSharedNonVertexInput.multisample.bits.minSampleShading = kMinSampleShadingScale;
    }

    if (GraphicsPipelineHasFragmentOutput(subset))
    {
        constexpr VkFlags kAllColorBits = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);

        for (uint32_t colorIndexGL = 0; colorIndexGL < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
             ++colorIndexGL)
        {
            Int4Array_Set(mFragmentOutput.blend.colorWriteMaskBits, colorIndexGL, kAllColorBits);
        }

        PackedColorBlendAttachmentState blendAttachmentState;
        SetBitField(blendAttachmentState.srcColorBlendFactor, VK_BLEND_FACTOR_ONE);
        SetBitField(blendAttachmentState.dstColorBlendFactor, VK_BLEND_FACTOR_ZERO);
        SetBitField(blendAttachmentState.colorBlendOp, VK_BLEND_OP_ADD);
        SetBitField(blendAttachmentState.srcAlphaBlendFactor, VK_BLEND_FACTOR_ONE);
        SetBitField(blendAttachmentState.dstAlphaBlendFactor, VK_BLEND_FACTOR_ZERO);
        SetBitField(blendAttachmentState.alphaBlendOp, VK_BLEND_OP_ADD);

        std::fill(&mFragmentOutput.blend.attachments[0],
                  &mFragmentOutput.blend.attachments[gl::IMPLEMENTATION_MAX_DRAW_BUFFERS],
                  blendAttachmentState);

        mFragmentOutput.blendMaskAndLogic.bits.blendEnableMask = 0;
        mFragmentOutput.blendMaskAndLogic.bits.logicOpEnable   = 0;
        SetBitField(mFragmentOutput.blendMaskAndLogic.bits.logicOp, VK_LOGIC_OP_COPY);
        mFragmentOutput.blendMaskAndLogic.bits.padding = 0;
    }

    // Context robustness affects vertex input and shader stages.
    mVertexInput.inputAssembly.bits.isRobustContext = mShaders.shaders.bits.isRobustContext =
        pipelineRobustness == PipelineRobustness::Robust;

    // Context protected-ness affects all subsets.
    mVertexInput.inputAssembly.bits.isProtectedContext = mShaders.shaders.bits.isProtectedContext =
        mFragmentOutput.blendMaskAndLogic.bits.isProtectedContext =
            pipelineProtectedAccess == PipelineProtectedAccess::Protected;
}

VkResult GraphicsPipelineDesc::initializePipeline(ErrorContext *context,
                                                  PipelineCacheAccess *pipelineCache,
                                                  GraphicsPipelineSubset subset,
                                                  const RenderPass &compatibleRenderPass,
                                                  const PipelineLayout &pipelineLayout,
                                                  const ShaderModuleMap &shaders,
                                                  const SpecializationConstants &specConsts,
                                                  Pipeline *pipelineOut,
                                                  CacheLookUpFeedback *feedbackOut) const
{
    GraphicsPipelineVertexInputVulkanStructs vertexInputState;
    GraphicsPipelineShadersVulkanStructs shadersState;
    GraphicsPipelineSharedNonVertexInputVulkanStructs sharedNonVertexInputState;
    GraphicsPipelineFragmentOutputVulkanStructs fragmentOutputState;
    GraphicsPipelineDynamicStateList dynamicStateList;

    // With dynamic rendering, there are no render pass objects.
    ASSERT(!compatibleRenderPass.valid() || !context->getFeatures().preferDynamicRendering.enabled);

    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.flags                        = 0;
    createInfo.renderPass                   = compatibleRenderPass.getHandle();
    createInfo.subpass                      = mSharedNonVertexInput.multisample.bits.subpass;

    const bool hasVertexInput             = GraphicsPipelineHasVertexInput(subset);
    const bool hasShaders                 = GraphicsPipelineHasShaders(subset);
    const bool hasShadersOrFragmentOutput = GraphicsPipelineHasShadersOrFragmentOutput(subset);
    const bool hasFragmentOutput          = GraphicsPipelineHasFragmentOutput(subset);

    if (hasVertexInput)
    {
        initializePipelineVertexInputState(context, &vertexInputState, &dynamicStateList);

        createInfo.pVertexInputState   = &vertexInputState.vertexInputState;
        createInfo.pInputAssemblyState = &vertexInputState.inputAssemblyState;
    }

    if (hasShaders)
    {
        initializePipelineShadersState(context, shaders, specConsts, &shadersState,
                                       &dynamicStateList);

        createInfo.stageCount          = static_cast<uint32_t>(shadersState.shaderStages.size());
        createInfo.pStages             = shadersState.shaderStages.data();
        createInfo.pTessellationState  = &shadersState.tessellationState;
        createInfo.pViewportState      = &shadersState.viewportState;
        createInfo.pRasterizationState = &shadersState.rasterState;
        createInfo.pDepthStencilState  = &shadersState.depthStencilState;
        createInfo.layout              = pipelineLayout.getHandle();
    }

    if (hasShadersOrFragmentOutput)
    {
        initializePipelineSharedNonVertexInputState(context, &sharedNonVertexInputState,
                                                    &dynamicStateList);

        createInfo.pMultisampleState = &sharedNonVertexInputState.multisampleState;
    }

    if (hasFragmentOutput)
    {
        initializePipelineFragmentOutputState(context, &fragmentOutputState, &dynamicStateList);

        createInfo.pColorBlendState = &fragmentOutputState.blendState;
    }

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateList.size());
    dynamicState.pDynamicStates    = dynamicStateList.data();
    createInfo.pDynamicState       = dynamicStateList.empty() ? nullptr : &dynamicState;

    // If not a complete pipeline, specify which subset is being created
    VkGraphicsPipelineLibraryCreateInfoEXT libraryInfo = {};
    libraryInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT;

    if (subset != GraphicsPipelineSubset::Complete)
    {
        switch (subset)
        {
            case GraphicsPipelineSubset::VertexInput:
                libraryInfo.flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT;
                break;
            case GraphicsPipelineSubset::Shaders:
                libraryInfo.flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                    VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
                break;
            case GraphicsPipelineSubset::FragmentOutput:
                libraryInfo.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
                break;
            default:
                UNREACHABLE();
                break;
        }

        createInfo.flags |= VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;

        AddToPNextChain(&createInfo, &libraryInfo);
    }

    VkPipelineRobustnessCreateInfoEXT robustness = {};
    robustness.sType = VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT;

    // Enable robustness on the pipeline if needed.  Note that the global robustBufferAccess feature
    // must be disabled by default.
    if ((hasVertexInput && mVertexInput.inputAssembly.bits.isRobustContext) ||
        (hasShaders && mShaders.shaders.bits.isRobustContext))
    {
        ASSERT(context->getFeatures().supportsPipelineRobustness.enabled);

        robustness.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
        robustness.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
        robustness.vertexInputs   = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
        robustness.images         = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT;

        AddToPNextChain(&createInfo, &robustness);
    }

    if ((hasVertexInput && mVertexInput.inputAssembly.bits.isProtectedContext) ||
        (hasShaders && mShaders.shaders.bits.isProtectedContext) ||
        (hasFragmentOutput && mFragmentOutput.blendMaskAndLogic.bits.isProtectedContext))
    {
        ASSERT(context->getFeatures().supportsPipelineProtectedAccess.enabled);
        createInfo.flags |= VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
    }
    else if (context->getFeatures().supportsPipelineProtectedAccess.enabled)
    {
        createInfo.flags |= VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;
    }

    VkPipelineCreationFeedback feedback = {};
    gl::ShaderMap<VkPipelineCreationFeedback> perStageFeedback;

    VkPipelineCreationFeedbackCreateInfo feedbackInfo = {};
    feedbackInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO;

    const bool supportsFeedback = context->getFeatures().supportsPipelineCreationFeedback.enabled;
    if (supportsFeedback)
    {
        feedbackInfo.pPipelineCreationFeedback = &feedback;
        // Provide some storage for per-stage data, even though it's not used.  This first works
        // around a VVL bug that doesn't allow `pipelineStageCreationFeedbackCount=0` despite the
        // spec (See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4161).  Even
        // with fixed VVL, several drivers crash when this storage is missing too.
        feedbackInfo.pipelineStageCreationFeedbackCount = createInfo.stageCount;
        feedbackInfo.pPipelineStageCreationFeedbacks    = perStageFeedback.data();

        AddToPNextChain(&createInfo, &feedbackInfo);
    }

    // Attach dynamic rendering info if needed.  This is done last because the flags may need to
    // be transfered to |VkPipelineCreateFlags2CreateInfoKHR|, so it must be done after every
    // other flag is set.
    DynamicRenderingInfo renderingInfo;
    VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo;
    VkRenderingAttachmentLocationInfoKHR attachmentLocations;
    VkRenderingInputAttachmentIndexInfoKHR inputLocations;
    VkPipelineCreateFlags2CreateInfoKHR createFlags2;
    if (hasShadersOrFragmentOutput && context->getFeatures().preferDynamicRendering.enabled)
    {
        DeriveRenderingInfo(context->getRenderer(), getRenderPassDesc(),
                            DynamicRenderingInfoSubset::Pipeline, {}, VK_SUBPASS_CONTENTS_INLINE,
                            {}, {}, {}, 0, &renderingInfo);
        AttachPipelineRenderingInfo(context, getRenderPassDesc(), renderingInfo, subset,
                                    &pipelineRenderingInfo, &attachmentLocations, &inputLocations,
                                    &createFlags2, &createInfo);
    }

    VkResult result = pipelineCache->createGraphicsPipeline(context, createInfo, pipelineOut);

    if (supportsFeedback)
    {
        const bool cacheHit =
            (feedback.flags & VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT) !=
            0;

        *feedbackOut = cacheHit ? CacheLookUpFeedback::Hit : CacheLookUpFeedback::Miss;
        ApplyPipelineCreationFeedback(context, feedback);
    }

    return result;
}

angle::FormatID patchVertexAttribComponentType(angle::FormatID format,
                                               gl::ComponentType vsInputType)
{
    const gl::VertexFormat &vertexFormat = gl::GetVertexFormatFromID(format);
    // For normalized format, keep the same ?
    EGLBoolean normalized = vertexFormat.normalized;
    if (normalized)
    {
        return format;
    }
    gl::VertexAttribType attribType = gl::FromGLenum<gl::VertexAttribType>(vertexFormat.type);
    if (vsInputType != gl::ComponentType::Float)
    {
        ASSERT(vsInputType == gl::ComponentType::Int ||
               vsInputType == gl::ComponentType::UnsignedInt);
        switch (attribType)
        {
            case gl::VertexAttribType::Float:
            case gl::VertexAttribType::Fixed:
            case gl::VertexAttribType::UnsignedInt:
            case gl::VertexAttribType::Int:
                attribType = vsInputType == gl::ComponentType::Int
                                 ? gl::VertexAttribType::Int
                                 : gl::VertexAttribType::UnsignedInt;
                break;
            case gl::VertexAttribType::HalfFloat:
            case gl::VertexAttribType::HalfFloatOES:
            case gl::VertexAttribType::Short:
            case gl::VertexAttribType::UnsignedShort:
                attribType = vsInputType == gl::ComponentType::Int
                                 ? gl::VertexAttribType::Short
                                 : gl::VertexAttribType::UnsignedShort;
                break;
            case gl::VertexAttribType::Byte:
            case gl::VertexAttribType::UnsignedByte:
                attribType = vsInputType == gl::ComponentType::Int
                                 ? gl::VertexAttribType::Byte
                                 : gl::VertexAttribType::UnsignedByte;
                break;
            case gl::VertexAttribType::UnsignedInt2101010:
            case gl::VertexAttribType::Int2101010:
                attribType = vsInputType == gl::ComponentType::Int
                                 ? gl::VertexAttribType::Int2101010
                                 : gl::VertexAttribType::UnsignedInt2101010;
                break;
            case gl::VertexAttribType::UnsignedInt1010102:
            case gl::VertexAttribType::Int1010102:
                attribType = vsInputType == gl::ComponentType::Int
                                 ? gl::VertexAttribType::Int1010102
                                 : gl::VertexAttribType::UnsignedInt1010102;
                break;
            default:
                ASSERT(0);
                break;
        }
    }
    return gl::GetVertexFormatID(attribType, vertexFormat.normalized, vertexFormat.components,
                                 !vertexFormat.pureInteger);
}

VkFormat GraphicsPipelineDesc::getPipelineVertexInputStateFormat(
    ErrorContext *context,
    angle::FormatID formatID,
    bool compressed,
    const gl::ComponentType programAttribType,
    uint32_t attribIndex)
{
    vk::Renderer *renderer = context->getRenderer();
    // Get the corresponding VkFormat for the attrib's format.
    const Format &format                = renderer->getFormat(formatID);
    const angle::Format &intendedFormat = format.getIntendedFormat();
    VkFormat vkFormat                   = format.getActualBufferVkFormat(renderer, compressed);

    const gl::ComponentType attribType = GetVertexAttributeComponentType(
        intendedFormat.isPureInt(), intendedFormat.vertexAttribType);

    if (attribType != programAttribType)
    {
        VkFormat origVkFormat = vkFormat;
        if (attribType == gl::ComponentType::Float || programAttribType == gl::ComponentType::Float)
        {
            angle::FormatID patchFormatID =
                patchVertexAttribComponentType(formatID, programAttribType);
            vkFormat =
                renderer->getFormat(patchFormatID).getActualBufferVkFormat(renderer, compressed);
        }
        else
        {
            // When converting from an unsigned to a signed format or vice versa, attempt to
            // match the bit width.
            angle::FormatID convertedFormatID = gl::ConvertFormatSignedness(intendedFormat);
            const Format &convertedFormat     = renderer->getFormat(convertedFormatID);
            ASSERT(intendedFormat.channelCount == convertedFormat.getIntendedFormat().channelCount);
            ASSERT(intendedFormat.redBits == convertedFormat.getIntendedFormat().redBits);
            ASSERT(intendedFormat.greenBits == convertedFormat.getIntendedFormat().greenBits);
            ASSERT(intendedFormat.blueBits == convertedFormat.getIntendedFormat().blueBits);
            ASSERT(intendedFormat.alphaBits == convertedFormat.getIntendedFormat().alphaBits);

            vkFormat = convertedFormat.getActualBufferVkFormat(renderer, compressed);
        }
        const Format &origFormat  = renderer->getFormat(GetFormatIDFromVkFormat(origVkFormat));
        const Format &patchFormat = renderer->getFormat(GetFormatIDFromVkFormat(vkFormat));
        ASSERT(origFormat.getIntendedFormat().pixelBytes ==
               patchFormat.getIntendedFormat().pixelBytes);
        ASSERT(renderer->getNativeExtensions().relaxedVertexAttributeTypeANGLE);
    }

    return vkFormat;
}

void GraphicsPipelineDesc::initializePipelineVertexInputState(
    ErrorContext *context,
    GraphicsPipelineVertexInputVulkanStructs *stateOut,
    GraphicsPipelineDynamicStateList *dynamicStateListOut) const
{
    // TODO(jmadill): Possibly use different path for ES 3.1 split bindings/attribs.
    uint32_t vertexAttribCount = 0;

    stateOut->divisorState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
    stateOut->divisorState.pVertexBindingDivisors = stateOut->divisorDesc.data();
    for (size_t attribIndexSizeT :
         gl::AttributesMask(mVertexInput.inputAssembly.bits.programActiveAttributeLocations))
    {
        const uint32_t attribIndex = static_cast<uint32_t>(attribIndexSizeT);

        VkVertexInputBindingDescription &bindingDesc  = stateOut->bindingDescs[vertexAttribCount];
        VkVertexInputAttributeDescription &attribDesc = stateOut->attributeDescs[vertexAttribCount];
        const PackedAttribDesc &packedAttrib          = mVertexInput.vertex.attribs[attribIndex];

        bindingDesc.binding = attribIndex;
        bindingDesc.stride  = static_cast<uint32_t>(mVertexInput.vertex.strides[attribIndex]);
        if (packedAttrib.divisor != 0)
        {
            bindingDesc.inputRate = static_cast<VkVertexInputRate>(VK_VERTEX_INPUT_RATE_INSTANCE);
            stateOut->divisorDesc[stateOut->divisorState.vertexBindingDivisorCount].binding =
                bindingDesc.binding;
            stateOut->divisorDesc[stateOut->divisorState.vertexBindingDivisorCount].divisor =
                packedAttrib.divisor;
            ++stateOut->divisorState.vertexBindingDivisorCount;
        }
        else
        {
            bindingDesc.inputRate = static_cast<VkVertexInputRate>(VK_VERTEX_INPUT_RATE_VERTEX);
        }

        // If using dynamic state for stride, the value for stride is unconditionally 0 here.
        // |ContextVk::handleDirtyGraphicsVertexBuffers| implements the same fix when setting stride
        // dynamically.
        ASSERT(!context->getFeatures().useVertexInputBindingStrideDynamicState.enabled ||
               bindingDesc.stride == 0);

        // Get the corresponding VkFormat for the attrib's format.
        angle::FormatID formatID = static_cast<angle::FormatID>(packedAttrib.format);
        const gl::ComponentType programAttribType = gl::GetComponentTypeMask(
            gl::ComponentTypeMask(mVertexInput.vertex.shaderAttribComponentType), attribIndex);

        attribDesc.binding = attribIndex;
        attribDesc.format  = getPipelineVertexInputStateFormat(
            context, formatID, packedAttrib.compressed, programAttribType, attribIndex);
        attribDesc.location = static_cast<uint32_t>(attribIndex);
        attribDesc.offset   = packedAttrib.offset;

        vertexAttribCount++;
    }

    // The binding descriptions are filled in at draw time.
    stateOut->vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    stateOut->vertexInputState.flags = 0;
    stateOut->vertexInputState.vertexBindingDescriptionCount   = vertexAttribCount;
    stateOut->vertexInputState.pVertexBindingDescriptions      = stateOut->bindingDescs.data();
    stateOut->vertexInputState.vertexAttributeDescriptionCount = vertexAttribCount;
    stateOut->vertexInputState.pVertexAttributeDescriptions    = stateOut->attributeDescs.data();
    if (stateOut->divisorState.vertexBindingDivisorCount)
    {
        stateOut->vertexInputState.pNext = &stateOut->divisorState;
    }

    const PackedInputAssemblyState &inputAssembly = mVertexInput.inputAssembly;

    // Primitive topology is filled in at draw time.
    stateOut->inputAssemblyState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    stateOut->inputAssemblyState.flags = 0;
    stateOut->inputAssemblyState.topology =
        static_cast<VkPrimitiveTopology>(inputAssembly.bits.topology);
    stateOut->inputAssemblyState.primitiveRestartEnable =
        static_cast<VkBool32>(inputAssembly.bits.primitiveRestartEnable);

    // Dynamic state
    if (context->getFeatures().useVertexInputBindingStrideDynamicState.enabled &&
        vertexAttribCount > 0)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    }
    if (context->getFeatures().usePrimitiveRestartEnableDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
    }
    if (context->getFeatures().supportsVertexInputDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    }
}

void GraphicsPipelineDesc::initializePipelineShadersState(
    ErrorContext *context,
    const ShaderModuleMap &shaders,
    const SpecializationConstants &specConsts,
    GraphicsPipelineShadersVulkanStructs *stateOut,
    GraphicsPipelineDynamicStateList *dynamicStateListOut) const
{
    InitializeSpecializationInfo(specConsts, &stateOut->specializationEntries,
                                 &stateOut->specializationInfo);

    // Vertex shader is always expected to be present.
    const ShaderModule &vertexModule = *shaders[gl::ShaderType::Vertex];
    ASSERT(vertexModule.valid());
    VkPipelineShaderStageCreateInfo vertexStage = {};
    SetPipelineShaderStageInfo(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                               VK_SHADER_STAGE_VERTEX_BIT, vertexModule.getHandle(),
                               stateOut->specializationInfo, &vertexStage);
    stateOut->shaderStages.push_back(vertexStage);

    const ShaderModulePtr &tessControlPointer = shaders[gl::ShaderType::TessControl];
    if (tessControlPointer)
    {
        const ShaderModule &tessControlModule            = *tessControlPointer;
        VkPipelineShaderStageCreateInfo tessControlStage = {};
        SetPipelineShaderStageInfo(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                   tessControlModule.getHandle(), stateOut->specializationInfo,
                                   &tessControlStage);
        stateOut->shaderStages.push_back(tessControlStage);
    }

    const ShaderModulePtr &tessEvaluationPointer = shaders[gl::ShaderType::TessEvaluation];
    if (tessEvaluationPointer)
    {
        const ShaderModule &tessEvaluationModule            = *tessEvaluationPointer;
        VkPipelineShaderStageCreateInfo tessEvaluationStage = {};
        SetPipelineShaderStageInfo(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                   tessEvaluationModule.getHandle(), stateOut->specializationInfo,
                                   &tessEvaluationStage);
        stateOut->shaderStages.push_back(tessEvaluationStage);
    }

    const ShaderModulePtr &geometryPointer = shaders[gl::ShaderType::Geometry];
    if (geometryPointer)
    {
        const ShaderModule &geometryModule            = *geometryPointer;
        VkPipelineShaderStageCreateInfo geometryStage = {};
        SetPipelineShaderStageInfo(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                   VK_SHADER_STAGE_GEOMETRY_BIT, geometryModule.getHandle(),
                                   stateOut->specializationInfo, &geometryStage);
        stateOut->shaderStages.push_back(geometryStage);
    }

    // Fragment shader is optional.
    const ShaderModulePtr &fragmentPointer = shaders[gl::ShaderType::Fragment];
    if (fragmentPointer && !mShaders.shaders.bits.rasterizerDiscardEnable)
    {
        const ShaderModule &fragmentModule            = *fragmentPointer;
        VkPipelineShaderStageCreateInfo fragmentStage = {};
        SetPipelineShaderStageInfo(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, fragmentModule.getHandle(),
                                   stateOut->specializationInfo, &fragmentStage);
        stateOut->shaderStages.push_back(fragmentStage);
    }

    // Set initial viewport and scissor state.
    stateOut->viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    stateOut->viewportState.flags         = 0;
    stateOut->viewportState.viewportCount = 1;
    stateOut->viewportState.pViewports    = nullptr;
    stateOut->viewportState.scissorCount  = 1;
    stateOut->viewportState.pScissors     = nullptr;

    if (context->getFeatures().supportsDepthClipControl.enabled)
    {
        stateOut->depthClipControl.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT;
        stateOut->depthClipControl.negativeOneToOne =
            static_cast<VkBool32>(mShaders.shaders.bits.viewportNegativeOneToOne);

        stateOut->viewportState.pNext = &stateOut->depthClipControl;
    }

    // Rasterizer state.
    stateOut->rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    stateOut->rasterState.flags = 0;
    stateOut->rasterState.depthClampEnable =
        static_cast<VkBool32>(mShaders.shaders.bits.depthClampEnable);
    stateOut->rasterState.rasterizerDiscardEnable =
        static_cast<VkBool32>(mShaders.shaders.bits.rasterizerDiscardEnable);
    stateOut->rasterState.polygonMode =
        static_cast<VkPolygonMode>(mShaders.shaders.bits.polygonMode);
    stateOut->rasterState.cullMode  = static_cast<VkCullModeFlags>(mShaders.shaders.bits.cullMode);
    stateOut->rasterState.frontFace = static_cast<VkFrontFace>(mShaders.shaders.bits.frontFace);
    stateOut->rasterState.depthBiasEnable =
        static_cast<VkBool32>(mShaders.shaders.bits.depthBiasEnable);
    stateOut->rasterState.lineWidth = 0;
    const void **pNextPtr           = &stateOut->rasterState.pNext;

    const PackedMultisampleAndSubpassState &multisample = mSharedNonVertexInput.multisample;

    stateOut->rasterLineState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;
    // Enable Bresenham line rasterization if available and the following conditions are met:
    // 1.) not multisampling
    // 2.) VUID-VkGraphicsPipelineCreateInfo-lineRasterizationMode-02766:
    // The Vulkan spec states: If the lineRasterizationMode member of a
    // VkPipelineRasterizationLineStateCreateInfoEXT structure included in the pNext chain of
    // pRasterizationState is VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT or
    // VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT and if rasterization is enabled, then the
    // alphaToCoverageEnable, alphaToOneEnable, and sampleShadingEnable members of pMultisampleState
    // must all be VK_FALSE.
    if (multisample.bits.rasterizationSamplesMinusOne == 0 &&
        !mShaders.shaders.bits.rasterizerDiscardEnable && !multisample.bits.alphaToCoverageEnable &&
        !multisample.bits.alphaToOneEnable && !multisample.bits.sampleShadingEnable &&
        context->getFeatures().bresenhamLineRasterization.enabled)
    {
        stateOut->rasterLineState.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
        *pNextPtr                                       = &stateOut->rasterLineState;
        pNextPtr                                        = &stateOut->rasterLineState.pNext;
    }

    // Always set provoking vertex mode to last if available.
    if (context->getFeatures().provokingVertex.enabled)
    {
        stateOut->provokingVertexState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT;
        stateOut->provokingVertexState.provokingVertexMode =
            VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT;
        *pNextPtr = &stateOut->provokingVertexState;
        pNextPtr  = &stateOut->provokingVertexState.pNext;
    }

    if (context->getFeatures().supportsGeometryStreamsCapability.enabled)
    {
        stateOut->rasterStreamState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT;
        stateOut->rasterStreamState.rasterizationStream = 0;
        *pNextPtr                                       = &stateOut->rasterStreamState;
        pNextPtr                                        = &stateOut->rasterStreamState.pNext;
    }

    // Depth/stencil state.
    stateOut->depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stateOut->depthStencilState.flags = 0;
    stateOut->depthStencilState.depthTestEnable =
        static_cast<VkBool32>(mShaders.shaders.bits.depthTest);
    stateOut->depthStencilState.depthWriteEnable =
        static_cast<VkBool32>(mShaders.shaders.bits.depthWrite);
    stateOut->depthStencilState.depthCompareOp =
        static_cast<VkCompareOp>(mShaders.shaders.bits.depthCompareOp);
    stateOut->depthStencilState.depthBoundsTestEnable =
        static_cast<VkBool32>(mShaders.shaders.bits.depthBoundsTest);
    stateOut->depthStencilState.stencilTestEnable =
        static_cast<VkBool32>(mShaders.shaders.bits.stencilTest);
    UnpackStencilState(mShaders.shaders.front, &stateOut->depthStencilState.front,
                       mShaders.shaders.bits.nonZeroStencilWriteMaskWorkaround);
    UnpackStencilState(mShaders.shaders.back, &stateOut->depthStencilState.back,
                       mShaders.shaders.bits.nonZeroStencilWriteMaskWorkaround);
    stateOut->depthStencilState.minDepthBounds = 0;
    stateOut->depthStencilState.maxDepthBounds = 0;

    if (getRenderPassDepthStencilFramebufferFetchMode())
    {
        stateOut->depthStencilState.flags |=
            VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
            VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT;
    }

    // tessellation State
    if (tessControlPointer && tessEvaluationPointer)
    {
        stateOut->domainOriginState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO;
        stateOut->domainOriginState.pNext        = NULL;
        stateOut->domainOriginState.domainOrigin = VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT;

        stateOut->tessellationState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        stateOut->tessellationState.flags = 0;
        stateOut->tessellationState.pNext = &stateOut->domainOriginState;
        stateOut->tessellationState.patchControlPoints =
            static_cast<uint32_t>(mShaders.shaders.bits.patchVertices);
    }

    // Dynamic state
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_SCISSOR);
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
    if (context->getFeatures().useCullModeDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_CULL_MODE_EXT);
    }
    if (context->getFeatures().useFrontFaceDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_FRONT_FACE_EXT);
    }
    if (context->getFeatures().useDepthTestEnableDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    }
    if (context->getFeatures().useDepthWriteEnableDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    }
    if (context->getFeatures().useDepthCompareOpDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
    }
    if (context->getFeatures().useStencilTestEnableDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
    }
    if (context->getFeatures().useStencilOpDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_STENCIL_OP);
    }
    if (context->getFeatures().useRasterizerDiscardEnableDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
    }
    if (context->getFeatures().useDepthBiasEnableDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    }
    if (context->getFeatures().supportsFragmentShadingRate.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
    }
}

void GraphicsPipelineDesc::initializePipelineSharedNonVertexInputState(
    ErrorContext *context,
    GraphicsPipelineSharedNonVertexInputVulkanStructs *stateOut,
    GraphicsPipelineDynamicStateList *dynamicStateListOut) const
{
    const PackedMultisampleAndSubpassState &multisample = mSharedNonVertexInput.multisample;

    stateOut->sampleMask = multisample.bits.sampleMask;

    // Multisample state.
    stateOut->multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    stateOut->multisampleState.flags = 0;
    stateOut->multisampleState.rasterizationSamples =
        gl_vk::GetSamples(multisample.bits.rasterizationSamplesMinusOne + 1,
                          context->getFeatures().limitSampleCountTo2.enabled);
    stateOut->multisampleState.sampleShadingEnable =
        static_cast<VkBool32>(multisample.bits.sampleShadingEnable);
    stateOut->multisampleState.minSampleShading =
        static_cast<float>(multisample.bits.minSampleShading) / kMinSampleShadingScale;
    stateOut->multisampleState.pSampleMask = &stateOut->sampleMask;
    stateOut->multisampleState.alphaToCoverageEnable =
        static_cast<VkBool32>(multisample.bits.alphaToCoverageEnable);
    stateOut->multisampleState.alphaToOneEnable =
        static_cast<VkBool32>(multisample.bits.alphaToOneEnable);
}

void GraphicsPipelineDesc::initializePipelineFragmentOutputState(
    ErrorContext *context,
    GraphicsPipelineFragmentOutputVulkanStructs *stateOut,
    GraphicsPipelineDynamicStateList *dynamicStateListOut) const
{
    stateOut->blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    stateOut->blendState.flags = 0;
    stateOut->blendState.logicOpEnable =
        static_cast<VkBool32>(mFragmentOutput.blendMaskAndLogic.bits.logicOpEnable);
    stateOut->blendState.logicOp =
        static_cast<VkLogicOp>(mFragmentOutput.blendMaskAndLogic.bits.logicOp);
    stateOut->blendState.attachmentCount =
        static_cast<uint32_t>(mSharedNonVertexInput.renderPass.colorAttachmentRange());
    stateOut->blendState.pAttachments = stateOut->blendAttachmentState.data();

    // If this graphics pipeline is for the unresolve operation, correct the color attachment count
    // for that subpass.
    if ((mSharedNonVertexInput.renderPass.getColorUnresolveAttachmentMask().any() ||
         mSharedNonVertexInput.renderPass.hasDepthStencilUnresolveAttachment()) &&
        mSharedNonVertexInput.multisample.bits.subpass == 0)
    {
        stateOut->blendState.attachmentCount = static_cast<uint32_t>(
            mSharedNonVertexInput.renderPass.getColorUnresolveAttachmentMask().count());
    }

    // Specify rasterization order for color when available and there is framebuffer fetch.  This
    // allows implementation of coherent framebuffer fetch / advanced blend.
    if (context->getFeatures().supportsRasterizationOrderAttachmentAccess.enabled &&
        getRenderPassColorFramebufferFetchMode())
    {
        stateOut->blendState.flags |=
            VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT;
    }

    const gl::DrawBufferMask blendEnableMask(
        mFragmentOutput.blendMaskAndLogic.bits.blendEnableMask);

    // Zero-init all states.
    stateOut->blendAttachmentState = {};

    const PackedColorBlendState &colorBlend = mFragmentOutput.blend;

    // With render pass objects, the blend state is indexed by the subpass-mapped locations.
    // With dynamic rendering, it is indexed by the actual attachment index.
    uint32_t colorAttachmentIndex = 0;
    for (uint32_t colorIndexGL = 0; colorIndexGL < stateOut->blendState.attachmentCount;
         ++colorIndexGL)
    {
        if (context->getFeatures().preferDynamicRendering.enabled &&
            !mSharedNonVertexInput.renderPass.isColorAttachmentEnabled(colorIndexGL))
        {
            continue;
        }

        VkPipelineColorBlendAttachmentState &state =
            stateOut->blendAttachmentState[colorAttachmentIndex++];

        if (blendEnableMask[colorIndexGL])
        {
            // To avoid triggering valid usage error, blending must be disabled for formats that do
            // not have VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT feature bit set.
            // From OpenGL ES clients, this means disabling blending for integer formats.
            if (!angle::Format::Get(mSharedNonVertexInput.renderPass[colorIndexGL]).isInt())
            {
                ASSERT(!context->getRenderer()
                            ->getFormat(mSharedNonVertexInput.renderPass[colorIndexGL])
                            .getActualRenderableImageFormat()
                            .isInt());

                // The blend fixed-function is enabled with normal blend as well as advanced blend
                // when the Vulkan extension is present.  When emulating advanced blend in the
                // shader, the blend fixed-function must be disabled.
                const PackedColorBlendAttachmentState &packedBlendState =
                    colorBlend.attachments[colorIndexGL];
                if (packedBlendState.colorBlendOp <= static_cast<uint8_t>(VK_BLEND_OP_MAX) ||
                    context->getFeatures().supportsBlendOperationAdvanced.enabled)
                {
                    state.blendEnable = VK_TRUE;
                    UnpackBlendAttachmentState(packedBlendState, &state);
                }
            }
        }

        ASSERT(context->getRenderer()->getNativeExtensions().robustFragmentShaderOutputANGLE);
        if ((mFragmentOutput.blendMaskAndLogic.bits.missingOutputsMask >> colorIndexGL & 1) != 0)
        {
            state.colorWriteMask = 0;
        }
        else
        {
            state.colorWriteMask =
                Int4Array_Get<VkColorComponentFlags>(colorBlend.colorWriteMaskBits, colorIndexGL);
        }
    }

    if (context->getFeatures().preferDynamicRendering.enabled)
    {
        stateOut->blendState.attachmentCount = colorAttachmentIndex;
    }

    // Dynamic state
    dynamicStateListOut->push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
    if (context->getFeatures().supportsLogicOpDynamicState.enabled)
    {
        dynamicStateListOut->push_back(VK_DYNAMIC_STATE_LOGIC_OP_EXT);
    }
}

void GraphicsPipelineDesc::updateVertexInput(ContextVk *contextVk,
                                             GraphicsPipelineTransitionBits *transition,
                                             uint32_t attribIndex,
                                             GLuint stride,
                                             GLuint divisor,
                                             angle::FormatID format,
                                             bool compressed,
                                             GLuint relativeOffset)
{
    PackedAttribDesc &packedAttrib = mVertexInput.vertex.attribs[attribIndex];

    SetBitField(packedAttrib.divisor, divisor);

    if (format == angle::FormatID::NONE)
    {
        UNIMPLEMENTED();
    }

    SetBitField(packedAttrib.format, format);
    SetBitField(packedAttrib.compressed, compressed);
    SetBitField(packedAttrib.offset, relativeOffset);

    constexpr size_t kAttribBits = kPackedAttribDescSize * kBitsPerByte;
    const size_t kBit =
        ANGLE_GET_INDEXED_TRANSITION_BIT(mVertexInput.vertex.attribs, attribIndex, kAttribBits);

    // Each attribute is 4 bytes, so only one transition bit needs to be set.
    static_assert(kPackedAttribDescSize == kGraphicsPipelineDirtyBitBytes,
                  "Adjust transition bits");
    transition->set(kBit);

    if (!contextVk->getFeatures().useVertexInputBindingStrideDynamicState.enabled)
    {
        SetBitField(mVertexInput.vertex.strides[attribIndex], stride);
        transition->set(ANGLE_GET_INDEXED_TRANSITION_BIT(
            mVertexInput.vertex.strides, attribIndex,
            sizeof(mVertexInput.vertex.strides[0]) * kBitsPerByte));
    }
}

void GraphicsPipelineDesc::setVertexShaderComponentTypes(gl::AttributesMask activeAttribLocations,
                                                         gl::ComponentTypeMask componentTypeMask)
{
    SetBitField(mVertexInput.inputAssembly.bits.programActiveAttributeLocations,
                activeAttribLocations.bits());

    const gl::ComponentTypeMask activeComponentTypeMask =
        componentTypeMask & gl::GetActiveComponentTypeMask(activeAttribLocations);

    SetBitField(mVertexInput.vertex.shaderAttribComponentType, activeComponentTypeMask.bits());
}

void GraphicsPipelineDesc::updateVertexShaderComponentTypes(
    GraphicsPipelineTransitionBits *transition,
    gl::AttributesMask activeAttribLocations,
    gl::ComponentTypeMask componentTypeMask)
{
    if (mVertexInput.inputAssembly.bits.programActiveAttributeLocations !=
        activeAttribLocations.bits())
    {
        SetBitField(mVertexInput.inputAssembly.bits.programActiveAttributeLocations,
                    activeAttribLocations.bits());
        transition->set(ANGLE_GET_TRANSITION_BIT(mVertexInput.inputAssembly.bits));
    }

    const gl::ComponentTypeMask activeComponentTypeMask =
        componentTypeMask & gl::GetActiveComponentTypeMask(activeAttribLocations);

    if (mVertexInput.vertex.shaderAttribComponentType != activeComponentTypeMask.bits())
    {
        SetBitField(mVertexInput.vertex.shaderAttribComponentType, activeComponentTypeMask.bits());
        transition->set(ANGLE_GET_TRANSITION_BIT(mVertexInput.vertex.shaderAttribComponentType));
    }
}

void GraphicsPipelineDesc::setTopology(gl::PrimitiveMode drawMode)
{
    VkPrimitiveTopology vkTopology = gl_vk::GetPrimitiveTopology(drawMode);
    SetBitField(mVertexInput.inputAssembly.bits.topology, vkTopology);
}

void GraphicsPipelineDesc::updateTopology(GraphicsPipelineTransitionBits *transition,
                                          gl::PrimitiveMode drawMode)
{
    setTopology(drawMode);
    transition->set(ANGLE_GET_TRANSITION_BIT(mVertexInput.inputAssembly.bits));
}

void GraphicsPipelineDesc::updateDepthClipControl(GraphicsPipelineTransitionBits *transition,
                                                  bool negativeOneToOne)
{
    SetBitField(mShaders.shaders.bits.viewportNegativeOneToOne, negativeOneToOne);
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updatePrimitiveRestartEnabled(GraphicsPipelineTransitionBits *transition,
                                                         bool primitiveRestartEnabled)
{
    mVertexInput.inputAssembly.bits.primitiveRestartEnable =
        static_cast<uint16_t>(primitiveRestartEnabled);
    transition->set(ANGLE_GET_TRANSITION_BIT(mVertexInput.inputAssembly.bits));
}

void GraphicsPipelineDesc::updatePolygonMode(GraphicsPipelineTransitionBits *transition,
                                             gl::PolygonMode polygonMode)
{
    mShaders.shaders.bits.polygonMode = gl_vk::GetPolygonMode(polygonMode);
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateCullMode(GraphicsPipelineTransitionBits *transition,
                                          const gl::RasterizerState &rasterState)
{
    SetBitField(mShaders.shaders.bits.cullMode, gl_vk::GetCullMode(rasterState));
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateFrontFace(GraphicsPipelineTransitionBits *transition,
                                           const gl::RasterizerState &rasterState,
                                           bool invertFrontFace)
{
    SetBitField(mShaders.shaders.bits.frontFace,
                gl_vk::GetFrontFace(rasterState.frontFace, invertFrontFace));
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateRasterizerDiscardEnabled(
    GraphicsPipelineTransitionBits *transition,
    bool rasterizerDiscardEnabled)
{
    mShaders.shaders.bits.rasterizerDiscardEnable = static_cast<uint32_t>(rasterizerDiscardEnabled);
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

uint32_t GraphicsPipelineDesc::getRasterizationSamples() const
{
    return mSharedNonVertexInput.multisample.bits.rasterizationSamplesMinusOne + 1;
}

void GraphicsPipelineDesc::setRasterizationSamples(uint32_t rasterizationSamples)
{
    ASSERT(rasterizationSamples > 0);
    mSharedNonVertexInput.multisample.bits.rasterizationSamplesMinusOne = rasterizationSamples - 1;
}

void GraphicsPipelineDesc::updateRasterizationSamples(GraphicsPipelineTransitionBits *transition,
                                                      uint32_t rasterizationSamples)
{
    setRasterizationSamples(rasterizationSamples);
    transition->set(ANGLE_GET_TRANSITION_BIT(mSharedNonVertexInput.multisample.bits));
}

void GraphicsPipelineDesc::updateAlphaToCoverageEnable(GraphicsPipelineTransitionBits *transition,
                                                       bool enable)
{
    mSharedNonVertexInput.multisample.bits.alphaToCoverageEnable = enable;
    transition->set(ANGLE_GET_TRANSITION_BIT(mSharedNonVertexInput.multisample.bits));
}

void GraphicsPipelineDesc::updateAlphaToOneEnable(GraphicsPipelineTransitionBits *transition,
                                                  bool enable)
{
    mSharedNonVertexInput.multisample.bits.alphaToOneEnable = enable;
    transition->set(ANGLE_GET_TRANSITION_BIT(mSharedNonVertexInput.multisample.bits));
}

void GraphicsPipelineDesc::updateSampleMask(GraphicsPipelineTransitionBits *transition,
                                            uint32_t maskNumber,
                                            uint32_t mask)
{
    ASSERT(maskNumber == 0);
    SetBitField(mSharedNonVertexInput.multisample.bits.sampleMask, mask);

    transition->set(ANGLE_GET_TRANSITION_BIT(mSharedNonVertexInput.multisample.bits));
}

void GraphicsPipelineDesc::updateSampleShading(GraphicsPipelineTransitionBits *transition,
                                               bool enable,
                                               float value)
{
    mSharedNonVertexInput.multisample.bits.sampleShadingEnable = enable;
    if (enable)
    {
        SetBitField(mSharedNonVertexInput.multisample.bits.minSampleShading,
                    static_cast<uint16_t>(value * kMinSampleShadingScale));
    }
    else
    {
        mSharedNonVertexInput.multisample.bits.minSampleShading = kMinSampleShadingScale;
    }

    transition->set(ANGLE_GET_TRANSITION_BIT(mSharedNonVertexInput.multisample.bits));
}

void GraphicsPipelineDesc::setSingleBlend(uint32_t colorIndexGL,
                                          bool enabled,
                                          VkBlendOp op,
                                          VkBlendFactor srcFactor,
                                          VkBlendFactor dstFactor)
{
    mFragmentOutput.blendMaskAndLogic.bits.blendEnableMask |=
        static_cast<uint8_t>(1 << colorIndexGL);

    PackedColorBlendAttachmentState &blendAttachmentState =
        mFragmentOutput.blend.attachments[colorIndexGL];

    SetBitField(blendAttachmentState.colorBlendOp, op);
    SetBitField(blendAttachmentState.alphaBlendOp, op);

    SetBitField(blendAttachmentState.srcColorBlendFactor, srcFactor);
    SetBitField(blendAttachmentState.dstColorBlendFactor, dstFactor);
    SetBitField(blendAttachmentState.srcAlphaBlendFactor, VK_BLEND_FACTOR_ZERO);
    SetBitField(blendAttachmentState.dstAlphaBlendFactor, VK_BLEND_FACTOR_ONE);
}

void GraphicsPipelineDesc::updateBlendEnabled(GraphicsPipelineTransitionBits *transition,
                                              gl::DrawBufferMask blendEnabledMask)
{
    SetBitField(mFragmentOutput.blendMaskAndLogic.bits.blendEnableMask, blendEnabledMask.bits());
    transition->set(ANGLE_GET_TRANSITION_BIT(mFragmentOutput.blendMaskAndLogic.bits));
}

void GraphicsPipelineDesc::updateBlendEquations(GraphicsPipelineTransitionBits *transition,
                                                const gl::BlendStateExt &blendStateExt,
                                                gl::DrawBufferMask attachmentMask)
{
    constexpr size_t kSizeBits = sizeof(PackedColorBlendAttachmentState) * 8;

    for (size_t attachmentIndex : attachmentMask)
    {
        PackedColorBlendAttachmentState &blendAttachmentState =
            mFragmentOutput.blend.attachments[attachmentIndex];
        blendAttachmentState.colorBlendOp =
            PackGLBlendOp(blendStateExt.getEquationColorIndexed(attachmentIndex));
        blendAttachmentState.alphaBlendOp =
            PackGLBlendOp(blendStateExt.getEquationAlphaIndexed(attachmentIndex));
        transition->set(ANGLE_GET_INDEXED_TRANSITION_BIT(mFragmentOutput.blend.attachments,
                                                         attachmentIndex, kSizeBits));
    }
}

void GraphicsPipelineDesc::updateBlendFuncs(GraphicsPipelineTransitionBits *transition,
                                            const gl::BlendStateExt &blendStateExt,
                                            gl::DrawBufferMask attachmentMask)
{
    constexpr size_t kSizeBits = sizeof(PackedColorBlendAttachmentState) * 8;
    for (size_t attachmentIndex : attachmentMask)
    {
        PackedColorBlendAttachmentState &blendAttachmentState =
            mFragmentOutput.blend.attachments[attachmentIndex];
        blendAttachmentState.srcColorBlendFactor =
            PackGLBlendFactor(blendStateExt.getSrcColorIndexed(attachmentIndex));
        blendAttachmentState.dstColorBlendFactor =
            PackGLBlendFactor(blendStateExt.getDstColorIndexed(attachmentIndex));
        blendAttachmentState.srcAlphaBlendFactor =
            PackGLBlendFactor(blendStateExt.getSrcAlphaIndexed(attachmentIndex));
        blendAttachmentState.dstAlphaBlendFactor =
            PackGLBlendFactor(blendStateExt.getDstAlphaIndexed(attachmentIndex));
        transition->set(ANGLE_GET_INDEXED_TRANSITION_BIT(mFragmentOutput.blend.attachments,
                                                         attachmentIndex, kSizeBits));
    }
}

void GraphicsPipelineDesc::resetBlendFuncsAndEquations(GraphicsPipelineTransitionBits *transition,
                                                       const gl::BlendStateExt &blendStateExt,
                                                       gl::DrawBufferMask previousAttachmentsMask,
                                                       gl::DrawBufferMask newAttachmentsMask)
{
    // A framebuffer with attachments in P was bound, and now one with attachments in N is bound.
    // We need to clear blend funcs and equations for attachments in P that are not in N.  That is
    // attachments in P&~N.
    const gl::DrawBufferMask attachmentsToClear = previousAttachmentsMask & ~newAttachmentsMask;
    // We also need to restore blend funcs and equations for attachments in N that are not in P.
    const gl::DrawBufferMask attachmentsToAdd = newAttachmentsMask & ~previousAttachmentsMask;
    constexpr size_t kSizeBits                = sizeof(PackedColorBlendAttachmentState) * 8;

    for (size_t attachmentIndex : attachmentsToClear)
    {
        PackedColorBlendAttachmentState &blendAttachmentState =
            mFragmentOutput.blend.attachments[attachmentIndex];

        blendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
        blendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;
        blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

        transition->set(ANGLE_GET_INDEXED_TRANSITION_BIT(mFragmentOutput.blend.attachments,
                                                         attachmentIndex, kSizeBits));
    }

    if (attachmentsToAdd.any())
    {
        updateBlendFuncs(transition, blendStateExt, attachmentsToAdd);
        updateBlendEquations(transition, blendStateExt, attachmentsToAdd);
    }
}

void GraphicsPipelineDesc::setColorWriteMasks(gl::BlendStateExt::ColorMaskStorage::Type colorMasks,
                                              const gl::DrawBufferMask &alphaMask,
                                              const gl::DrawBufferMask &enabledDrawBuffers)
{
    for (uint32_t colorIndexGL = 0; colorIndexGL < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
         colorIndexGL++)
    {
        uint8_t colorMask =
            gl::BlendStateExt::ColorMaskStorage::GetValueIndexed(colorIndexGL, colorMasks);

        uint8_t mask = 0;
        if (enabledDrawBuffers.test(colorIndexGL))
        {
            mask = alphaMask[colorIndexGL] ? (colorMask & ~VK_COLOR_COMPONENT_A_BIT) : colorMask;
        }
        Int4Array_Set(mFragmentOutput.blend.colorWriteMaskBits, colorIndexGL, mask);
    }
}

void GraphicsPipelineDesc::setSingleColorWriteMask(uint32_t colorIndexGL,
                                                   VkColorComponentFlags colorComponentFlags)
{
    uint8_t colorMask = static_cast<uint8_t>(colorComponentFlags);
    Int4Array_Set(mFragmentOutput.blend.colorWriteMaskBits, colorIndexGL, colorMask);
}

void GraphicsPipelineDesc::updateColorWriteMasks(
    GraphicsPipelineTransitionBits *transition,
    gl::BlendStateExt::ColorMaskStorage::Type colorMasks,
    const gl::DrawBufferMask &alphaMask,
    const gl::DrawBufferMask &enabledDrawBuffers)
{
    setColorWriteMasks(colorMasks, alphaMask, enabledDrawBuffers);

    for (size_t colorIndexGL = 0; colorIndexGL < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
         colorIndexGL++)
    {
        transition->set(ANGLE_GET_INDEXED_TRANSITION_BIT(mFragmentOutput.blend.colorWriteMaskBits,
                                                         colorIndexGL, 4));
    }
}

void GraphicsPipelineDesc::updateMissingOutputsMask(GraphicsPipelineTransitionBits *transition,
                                                    gl::DrawBufferMask missingOutputsMask)
{
    if (mFragmentOutput.blendMaskAndLogic.bits.missingOutputsMask != missingOutputsMask.bits())
    {
        SetBitField(mFragmentOutput.blendMaskAndLogic.bits.missingOutputsMask,
                    missingOutputsMask.bits());
        transition->set(ANGLE_GET_TRANSITION_BIT(mFragmentOutput.blendMaskAndLogic.bits));
    }
}

void GraphicsPipelineDesc::updateLogicOpEnabled(GraphicsPipelineTransitionBits *transition,
                                                bool enable)
{
    mFragmentOutput.blendMaskAndLogic.bits.logicOpEnable = enable;
    transition->set(ANGLE_GET_TRANSITION_BIT(mFragmentOutput.blendMaskAndLogic.bits));
}

void GraphicsPipelineDesc::updateLogicOp(GraphicsPipelineTransitionBits *transition,
                                         VkLogicOp logicOp)
{
    SetBitField(mFragmentOutput.blendMaskAndLogic.bits.logicOp, logicOp);
    transition->set(ANGLE_GET_TRANSITION_BIT(mFragmentOutput.blendMaskAndLogic.bits));
}

void GraphicsPipelineDesc::setDepthTestEnabled(bool enabled)
{
    mShaders.shaders.bits.depthTest = enabled;
}

void GraphicsPipelineDesc::setDepthWriteEnabled(bool enabled)
{
    mShaders.shaders.bits.depthWrite = enabled;
}

void GraphicsPipelineDesc::setDepthFunc(VkCompareOp op)
{
    SetBitField(mShaders.shaders.bits.depthCompareOp, op);
}

void GraphicsPipelineDesc::setDepthClampEnabled(bool enabled)
{
    mShaders.shaders.bits.depthClampEnable = enabled;
}

void GraphicsPipelineDesc::setStencilTestEnabled(bool enabled)
{
    mShaders.shaders.bits.stencilTest = enabled;
}

void GraphicsPipelineDesc::setStencilFrontFuncs(VkCompareOp compareOp)
{
    SetBitField(mShaders.shaders.front.compare, compareOp);
}

void GraphicsPipelineDesc::setStencilBackFuncs(VkCompareOp compareOp)
{
    SetBitField(mShaders.shaders.back.compare, compareOp);
}

void GraphicsPipelineDesc::setStencilFrontOps(VkStencilOp failOp,
                                              VkStencilOp passOp,
                                              VkStencilOp depthFailOp)
{
    SetBitField(mShaders.shaders.front.fail, failOp);
    SetBitField(mShaders.shaders.front.pass, passOp);
    SetBitField(mShaders.shaders.front.depthFail, depthFailOp);
}

void GraphicsPipelineDesc::setStencilBackOps(VkStencilOp failOp,
                                             VkStencilOp passOp,
                                             VkStencilOp depthFailOp)
{
    SetBitField(mShaders.shaders.back.fail, failOp);
    SetBitField(mShaders.shaders.back.pass, passOp);
    SetBitField(mShaders.shaders.back.depthFail, depthFailOp);
}

void GraphicsPipelineDesc::updateDepthTestEnabled(GraphicsPipelineTransitionBits *transition,
                                                  const gl::DepthStencilState &depthStencilState,
                                                  const gl::Framebuffer *drawFramebuffer)
{
    // Only enable the depth test if the draw framebuffer has a depth buffer.  It's possible that
    // we're emulating a stencil-only buffer with a depth-stencil buffer
    setDepthTestEnabled(depthStencilState.depthTest && drawFramebuffer->hasDepth());
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateDepthFunc(GraphicsPipelineTransitionBits *transition,
                                           const gl::DepthStencilState &depthStencilState)
{
    setDepthFunc(gl_vk::GetCompareOp(depthStencilState.depthFunc));
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateDepthClampEnabled(GraphicsPipelineTransitionBits *transition,
                                                   bool enabled)
{
    setDepthClampEnabled(enabled);
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateSurfaceRotation(GraphicsPipelineTransitionBits *transition,
                                                 bool isRotatedAspectRatio)
{
    SetBitField(mShaders.shaders.bits.surfaceRotation, isRotatedAspectRatio);
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateDepthWriteEnabled(GraphicsPipelineTransitionBits *transition,
                                                   const gl::DepthStencilState &depthStencilState,
                                                   const gl::Framebuffer *drawFramebuffer)
{
    // Don't write to depth buffers that should not exist
    const bool depthWriteEnabled =
        drawFramebuffer->hasDepth() && depthStencilState.depthTest && depthStencilState.depthMask;
    if (static_cast<bool>(mShaders.shaders.bits.depthWrite) != depthWriteEnabled)
    {
        setDepthWriteEnabled(depthWriteEnabled);
        transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
    }
}

void GraphicsPipelineDesc::updateStencilTestEnabled(GraphicsPipelineTransitionBits *transition,
                                                    const gl::DepthStencilState &depthStencilState,
                                                    const gl::Framebuffer *drawFramebuffer)
{
    // Only enable the stencil test if the draw framebuffer has a stencil buffer.  It's possible
    // that we're emulating a depth-only buffer with a depth-stencil buffer
    setStencilTestEnabled(depthStencilState.stencilTest && drawFramebuffer->hasStencil());
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateStencilFrontFuncs(GraphicsPipelineTransitionBits *transition,
                                                   const gl::DepthStencilState &depthStencilState)
{
    setStencilFrontFuncs(gl_vk::GetCompareOp(depthStencilState.stencilFunc));
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.front));
}

void GraphicsPipelineDesc::updateStencilBackFuncs(GraphicsPipelineTransitionBits *transition,
                                                  const gl::DepthStencilState &depthStencilState)
{
    setStencilBackFuncs(gl_vk::GetCompareOp(depthStencilState.stencilBackFunc));
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.back));
}

void GraphicsPipelineDesc::updateStencilFrontOps(GraphicsPipelineTransitionBits *transition,
                                                 const gl::DepthStencilState &depthStencilState)
{
    setStencilFrontOps(gl_vk::GetStencilOp(depthStencilState.stencilFail),
                       gl_vk::GetStencilOp(depthStencilState.stencilPassDepthPass),
                       gl_vk::GetStencilOp(depthStencilState.stencilPassDepthFail));
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.front));
}

void GraphicsPipelineDesc::updateStencilBackOps(GraphicsPipelineTransitionBits *transition,
                                                const gl::DepthStencilState &depthStencilState)
{
    setStencilBackOps(gl_vk::GetStencilOp(depthStencilState.stencilBackFail),
                      gl_vk::GetStencilOp(depthStencilState.stencilBackPassDepthPass),
                      gl_vk::GetStencilOp(depthStencilState.stencilBackPassDepthFail));
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.back));
}

void GraphicsPipelineDesc::updatePolygonOffsetEnabled(GraphicsPipelineTransitionBits *transition,
                                                      bool enabled)
{
    mShaders.shaders.bits.depthBiasEnable = enabled;
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::setRenderPassDesc(const RenderPassDesc &renderPassDesc)
{
    mSharedNonVertexInput.renderPass = renderPassDesc;
}

void GraphicsPipelineDesc::updateSubpass(GraphicsPipelineTransitionBits *transition,
                                         uint32_t subpass)
{
    if (mSharedNonVertexInput.multisample.bits.subpass != subpass)
    {
        SetBitField(mSharedNonVertexInput.multisample.bits.subpass, subpass);
        transition->set(ANGLE_GET_TRANSITION_BIT(mSharedNonVertexInput.multisample.bits));
    }
}

void GraphicsPipelineDesc::updatePatchVertices(GraphicsPipelineTransitionBits *transition,
                                               GLuint value)
{
    SetBitField(mShaders.shaders.bits.patchVertices, value);
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::resetSubpass(GraphicsPipelineTransitionBits *transition)
{
    updateSubpass(transition, 0);
}

void GraphicsPipelineDesc::nextSubpass(GraphicsPipelineTransitionBits *transition)
{
    updateSubpass(transition, mSharedNonVertexInput.multisample.bits.subpass + 1);
}

void GraphicsPipelineDesc::setSubpass(uint32_t subpass)
{
    SetBitField(mSharedNonVertexInput.multisample.bits.subpass, subpass);
}

uint32_t GraphicsPipelineDesc::getSubpass() const
{
    return mSharedNonVertexInput.multisample.bits.subpass;
}

void GraphicsPipelineDesc::updateEmulatedDitherControl(GraphicsPipelineTransitionBits *transition,
                                                       uint16_t value)
{
    // Make sure we don't waste time resetting this to zero in the common no-dither case.
    ASSERT(value != 0 || mShaders.shaders.emulatedDitherControl != 0);

    mShaders.shaders.emulatedDitherControl = value;
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.emulatedDitherControl));
}

void GraphicsPipelineDesc::updateNonZeroStencilWriteMaskWorkaround(
    GraphicsPipelineTransitionBits *transition,
    bool enabled)
{
    mShaders.shaders.bits.nonZeroStencilWriteMaskWorkaround = enabled;
    transition->set(ANGLE_GET_TRANSITION_BIT(mShaders.shaders.bits));
}

void GraphicsPipelineDesc::updateRenderPassDesc(GraphicsPipelineTransitionBits *transition,
                                                const angle::FeaturesVk &features,
                                                const RenderPassDesc &renderPassDesc,
                                                FramebufferFetchMode framebufferFetchMode)
{
    setRenderPassDesc(renderPassDesc);

    // Framebuffer fetch mode is an inherent property of the executable.  With dynamic rendering, it
    // is not tracked in the framebuffer's render pass desc (the source of |renderPassDesc|), and is
    // overriden at this point.
    if (features.preferDynamicRendering.enabled)
    {
        setRenderPassFramebufferFetchMode(framebufferFetchMode);
    }

    // The RenderPass is a special case where it spans multiple bits but has no member.
    constexpr size_t kFirstBit =
        offsetof(GraphicsPipelineDesc, mSharedNonVertexInput.renderPass) >> kTransitionByteShift;
    constexpr size_t kBitCount = kRenderPassDescSize >> kTransitionByteShift;
    for (size_t bit = 0; bit < kBitCount; ++bit)
    {
        transition->set(kFirstBit + bit);
    }
}

void GraphicsPipelineDesc::setRenderPassSampleCount(GLint samples)
{
    mSharedNonVertexInput.renderPass.setSamples(samples);
}

void GraphicsPipelineDesc::setRenderPassFramebufferFetchMode(
    FramebufferFetchMode framebufferFetchMode)
{
    mSharedNonVertexInput.renderPass.setFramebufferFetchMode(framebufferFetchMode);
}

void GraphicsPipelineDesc::setRenderPassColorAttachmentFormat(size_t colorIndexGL,
                                                              angle::FormatID formatID)
{
    mSharedNonVertexInput.renderPass.packColorAttachment(colorIndexGL, formatID);
}

void GraphicsPipelineDesc::setRenderPassFoveation(bool isFoveated)
{
    mSharedNonVertexInput.renderPass.setFragmentShadingAttachment(isFoveated);
}

// AttachmentOpsArray implementation.
AttachmentOpsArray::AttachmentOpsArray()
{
    memset(&mOps, 0, sizeof(PackedAttachmentOpsDesc) * mOps.size());
}

AttachmentOpsArray::~AttachmentOpsArray() = default;

AttachmentOpsArray::AttachmentOpsArray(const AttachmentOpsArray &other)
{
    memcpy(&mOps, &other.mOps, sizeof(PackedAttachmentOpsDesc) * mOps.size());
}

AttachmentOpsArray &AttachmentOpsArray::operator=(const AttachmentOpsArray &other)
{
    memcpy(&mOps, &other.mOps, sizeof(PackedAttachmentOpsDesc) * mOps.size());
    return *this;
}

void AttachmentOpsArray::initWithLoadStore(PackedAttachmentIndex index,
                                           ImageLayout initialLayout,
                                           ImageLayout finalLayout)
{
    setLayouts(index, initialLayout, finalLayout);
    setOps(index, RenderPassLoadOp::Load, RenderPassStoreOp::Store);
    setStencilOps(index, RenderPassLoadOp::Load, RenderPassStoreOp::Store);
}

void AttachmentOpsArray::setLayouts(PackedAttachmentIndex index,
                                    ImageLayout initialLayout,
                                    ImageLayout finalLayout)
{
    PackedAttachmentOpsDesc &ops = mOps[index.get()];
    SetBitField(ops.initialLayout, initialLayout);
    SetBitField(ops.finalLayout, finalLayout);
    SetBitField(ops.finalResolveLayout, finalLayout);
}

void AttachmentOpsArray::setOps(PackedAttachmentIndex index,
                                RenderPassLoadOp loadOp,
                                RenderPassStoreOp storeOp)
{
    PackedAttachmentOpsDesc &ops = mOps[index.get()];
    SetBitField(ops.loadOp, loadOp);
    SetBitField(ops.storeOp, storeOp);
    ops.isInvalidated = false;
}

void AttachmentOpsArray::setStencilOps(PackedAttachmentIndex index,
                                       RenderPassLoadOp loadOp,
                                       RenderPassStoreOp storeOp)
{
    PackedAttachmentOpsDesc &ops = mOps[index.get()];
    SetBitField(ops.stencilLoadOp, loadOp);
    SetBitField(ops.stencilStoreOp, storeOp);
    ops.isStencilInvalidated = false;
}

void AttachmentOpsArray::setClearOp(PackedAttachmentIndex index)
{
    PackedAttachmentOpsDesc &ops = mOps[index.get()];
    SetBitField(ops.loadOp, RenderPassLoadOp::Clear);
}

void AttachmentOpsArray::setClearStencilOp(PackedAttachmentIndex index)
{
    PackedAttachmentOpsDesc &ops = mOps[index.get()];
    SetBitField(ops.stencilLoadOp, RenderPassLoadOp::Clear);
}

size_t AttachmentOpsArray::hash() const
{
    return angle::ComputeGenericHash(mOps);
}

bool operator==(const AttachmentOpsArray &lhs, const AttachmentOpsArray &rhs)
{
    return memcmp(&lhs, &rhs, sizeof(AttachmentOpsArray)) == 0;
}

// DescriptorSetLayoutDesc implementation.
DescriptorSetLayoutDesc::DescriptorSetLayoutDesc()
    : mImmutableSamplers{}, mDescriptorSetLayoutBindings{}
{}

DescriptorSetLayoutDesc::~DescriptorSetLayoutDesc() = default;

DescriptorSetLayoutDesc::DescriptorSetLayoutDesc(const DescriptorSetLayoutDesc &other) = default;

DescriptorSetLayoutDesc &DescriptorSetLayoutDesc::operator=(const DescriptorSetLayoutDesc &other) =
    default;

size_t DescriptorSetLayoutDesc::hash() const
{
    size_t validDescriptorSetLayoutBindingsCount = mDescriptorSetLayoutBindings.size();
    size_t validImmutableSamplersCount           = mImmutableSamplers.size();

    ASSERT(validDescriptorSetLayoutBindingsCount != 0 || validImmutableSamplersCount == 0);

    size_t genericHash = 0;
    if (validDescriptorSetLayoutBindingsCount > 0)
    {
        genericHash = angle::ComputeGenericHash(
            mDescriptorSetLayoutBindings.data(),
            validDescriptorSetLayoutBindingsCount * sizeof(PackedDescriptorSetBinding));
    }

    if (validImmutableSamplersCount > 0)
    {
        genericHash ^= angle::ComputeGenericHash(mImmutableSamplers.data(),
                                                 validImmutableSamplersCount * sizeof(VkSampler));
    }

    return genericHash;
}

bool DescriptorSetLayoutDesc::operator==(const DescriptorSetLayoutDesc &other) const
{
    return mDescriptorSetLayoutBindings == other.mDescriptorSetLayoutBindings &&
           mImmutableSamplers == other.mImmutableSamplers;
}

void DescriptorSetLayoutDesc::addBinding(uint32_t bindingIndex,
                                         VkDescriptorType descriptorType,
                                         uint32_t count,
                                         VkShaderStageFlags stages,
                                         const Sampler *immutableSampler)
{
    ASSERT(static_cast<uint8_t>(descriptorType) != PackedDescriptorSetBinding::kInvalidType);
    ASSERT(static_cast<size_t>(descriptorType) < std::numeric_limits<uint8_t>::max());
    ASSERT(count < std::numeric_limits<uint16_t>::max());
    ASSERT(bindingIndex < std::numeric_limits<uint16_t>::max());

    if (bindingIndex >= mDescriptorSetLayoutBindings.size())
    {
        PackedDescriptorSetBinding invalid = {};
        invalid.type                       = PackedDescriptorSetBinding::kInvalidType;
        mDescriptorSetLayoutBindings.resize(bindingIndex + 1, invalid);
    }

    PackedDescriptorSetBinding &packedBinding = mDescriptorSetLayoutBindings[bindingIndex];
    ASSERT(packedBinding.type == PackedDescriptorSetBinding::kInvalidType);
    SetBitField(packedBinding.type, descriptorType);
    SetBitField(packedBinding.count, count);
    SetBitField(packedBinding.stages, stages);
    SetBitField(packedBinding.hasImmutableSampler, 0);

    if (immutableSampler)
    {
        if (bindingIndex >= mImmutableSamplers.size())
        {
            mImmutableSamplers.resize(bindingIndex + 1);
        }

        ASSERT(count == 1);
        SetBitField(packedBinding.hasImmutableSampler, 1);
        mImmutableSamplers[bindingIndex] = immutableSampler->getHandle();
    }
}

void DescriptorSetLayoutDesc::unpackBindings(DescriptorSetLayoutBindingVector *bindings) const
{
    // Unpack all valid descriptor set layout bindings
    for (size_t bindingIndex = 0; bindingIndex < mDescriptorSetLayoutBindings.size();
         ++bindingIndex)
    {
        const PackedDescriptorSetBinding &packedBinding =
            mDescriptorSetLayoutBindings[bindingIndex];
        if (packedBinding.type == PackedDescriptorSetBinding::kInvalidType)
        {
            continue;
        }

        ASSERT(packedBinding.count != 0);

        VkDescriptorSetLayoutBinding binding = {};
        binding.binding                      = static_cast<uint32_t>(bindingIndex);
        binding.descriptorCount              = packedBinding.count;
        binding.descriptorType               = static_cast<VkDescriptorType>(packedBinding.type);
        binding.stageFlags = static_cast<VkShaderStageFlags>(packedBinding.stages);

        if (packedBinding.hasImmutableSampler)
        {
            ASSERT(packedBinding.count == 1);
            binding.pImmutableSamplers = &mImmutableSamplers[bindingIndex];
        }

        bindings->push_back(binding);
    }
}

// PipelineLayoutDesc implementation.
PipelineLayoutDesc::PipelineLayoutDesc()
    : mDescriptorSetLayouts{}, mPushConstantRange{}, mPadding(0)
{}

PipelineLayoutDesc::~PipelineLayoutDesc() = default;

PipelineLayoutDesc::PipelineLayoutDesc(const PipelineLayoutDesc &other) = default;

PipelineLayoutDesc &PipelineLayoutDesc::operator=(const PipelineLayoutDesc &rhs)
{
    mDescriptorSetLayouts = rhs.mDescriptorSetLayouts;
    mPushConstantRange    = rhs.mPushConstantRange;
    return *this;
}

size_t PipelineLayoutDesc::hash() const
{
    size_t genericHash = angle::ComputeGenericHash(mPushConstantRange);
    for (const DescriptorSetLayoutDesc &descriptorSetLayoutDesc : mDescriptorSetLayouts)
    {
        genericHash ^= descriptorSetLayoutDesc.hash();
    }
    return genericHash;
}

bool PipelineLayoutDesc::operator==(const PipelineLayoutDesc &other) const
{
    return mPushConstantRange == other.mPushConstantRange &&
           mDescriptorSetLayouts == other.mDescriptorSetLayouts;
}

void PipelineLayoutDesc::updateDescriptorSetLayout(DescriptorSetIndex setIndex,
                                                   const DescriptorSetLayoutDesc &desc)
{
    mDescriptorSetLayouts[setIndex] = desc;
}

void PipelineLayoutDesc::updatePushConstantRange(VkShaderStageFlags stageMask,
                                                 uint32_t offset,
                                                 uint32_t size)
{
    SetBitField(mPushConstantRange.offset, offset);
    SetBitField(mPushConstantRange.size, size);
    SetBitField(mPushConstantRange.stageMask, stageMask);
}

// CreateMonolithicPipelineTask implementation.
CreateMonolithicPipelineTask::CreateMonolithicPipelineTask(
    Renderer *renderer,
    const PipelineCacheAccess &pipelineCache,
    const PipelineLayout &pipelineLayout,
    const ShaderModuleMap &shaders,
    const SpecializationConstants &specConsts,
    const GraphicsPipelineDesc &desc)
    : ErrorContext(renderer),
      mPipelineCache(pipelineCache),
      mCompatibleRenderPass(nullptr),
      mPipelineLayout(pipelineLayout),
      mShaders(shaders),
      mSpecConsts(specConsts),
      mDesc(desc),
      mResult(VK_NOT_READY),
      mFeedback(CacheLookUpFeedback::None)
{}

void CreateMonolithicPipelineTask::setCompatibleRenderPass(const RenderPass *compatibleRenderPass)
{
    mCompatibleRenderPass = compatibleRenderPass;
}

void CreateMonolithicPipelineTask::operator()()
{
    const RenderPass unusedRenderPass;
    const RenderPass *compatibleRenderPass =
        mCompatibleRenderPass ? mCompatibleRenderPass : &unusedRenderPass;

    ANGLE_TRACE_EVENT0("gpu.angle", "CreateMonolithicPipelineTask");
    mResult = mDesc.initializePipeline(this, &mPipelineCache, vk::GraphicsPipelineSubset::Complete,
                                       *compatibleRenderPass, mPipelineLayout, mShaders,
                                       mSpecConsts, &mPipeline, &mFeedback);

    if (mRenderer->getFeatures().slowDownMonolithicPipelineCreationForTesting.enabled)
    {
        constexpr double kSlowdownTime = 0.05;

        double startTime = angle::GetCurrentSystemTime();
        while (angle::GetCurrentSystemTime() - startTime < kSlowdownTime)
        {
            // Busy waiting
        }
    }
}

void CreateMonolithicPipelineTask::handleError(VkResult result,
                                               const char *file,
                                               const char *function,
                                               unsigned int line)
{
    UNREACHABLE();
}

// WaitableMonolithicPipelineCreationTask implementation
WaitableMonolithicPipelineCreationTask::~WaitableMonolithicPipelineCreationTask()
{
    ASSERT(!mWaitableEvent);
    ASSERT(!mTask);
}

// PipelineHelper implementation.
PipelineHelper::PipelineHelper() = default;

PipelineHelper::~PipelineHelper() = default;

void PipelineHelper::destroy(VkDevice device)
{
    mPipeline.destroy(device);
    mLinkedPipelineToRelease.destroy(device);

    // If there is a pending task, wait for it before destruction.
    if (mMonolithicPipelineCreationTask.isValid())
    {
        if (mMonolithicPipelineCreationTask.isPosted())
        {
            mMonolithicPipelineCreationTask.wait();
            mMonolithicPipelineCreationTask.getTask()->getPipeline().destroy(device);
        }
        mMonolithicPipelineCreationTask.reset();
    }

    reset();
}

void PipelineHelper::release(ErrorContext *context)
{
    Renderer *renderer = context->getRenderer();

    renderer->collectGarbage(mUse, &mPipeline);
    renderer->collectGarbage(mUse, &mLinkedPipelineToRelease);

    // If there is a pending task, wait for it before release.
    if (mMonolithicPipelineCreationTask.isValid())
    {
        if (mMonolithicPipelineCreationTask.isPosted())
        {
            mMonolithicPipelineCreationTask.wait();
            renderer->collectGarbage(mUse,
                                     &mMonolithicPipelineCreationTask.getTask()->getPipeline());
        }
        mMonolithicPipelineCreationTask.reset();
    }

    reset();
}

void PipelineHelper::reset()
{
    mCacheLookUpFeedback           = CacheLookUpFeedback::None;
    mMonolithicCacheLookUpFeedback = CacheLookUpFeedback::None;

    mLinkedShaders = nullptr;
}

angle::Result PipelineHelper::getPreferredPipeline(ContextVk *contextVk,
                                                   const Pipeline **pipelineOut)
{
    if (mMonolithicPipelineCreationTask.isValid())
    {
        // If there is a monolithic task pending, attempt to post it if not already.  Once the task
        // is done, retrieve the results and replace the pipeline.
        if (!mMonolithicPipelineCreationTask.isPosted())
        {
            ANGLE_TRY(contextVk->getShareGroup()->scheduleMonolithicPipelineCreationTask(
                contextVk, &mMonolithicPipelineCreationTask));
        }
        else if (mMonolithicPipelineCreationTask.isReady())
        {
            CreateMonolithicPipelineTask *task = &*mMonolithicPipelineCreationTask.getTask();
            ANGLE_VK_TRY(contextVk, task->getResult());

            mMonolithicCacheLookUpFeedback = task->getFeedback();

            // The pipeline will not be used anymore.  Every context that has used this pipeline has
            // already updated the serial.
            mLinkedPipelineToRelease = std::move(mPipeline);

            // Replace it with the monolithic one.
            mPipeline = std::move(task->getPipeline());

            mLinkedShaders = nullptr;

            mMonolithicPipelineCreationTask.reset();

            ++contextVk->getPerfCounters().monolithicPipelineCreation;
        }
    }

    *pipelineOut = &mPipeline;

    return angle::Result::Continue;
}

void PipelineHelper::addTransition(GraphicsPipelineTransitionBits bits,
                                   const GraphicsPipelineDesc *desc,
                                   PipelineHelper *pipeline)
{
    mTransitions.emplace_back(bits, desc, pipeline);
}

void PipelineHelper::setLinkedLibraryReferences(vk::PipelineHelper *shadersPipeline)
{
    mLinkedShaders = shadersPipeline;
}

void PipelineHelper::retainInRenderPass(RenderPassCommandBufferHelper *renderPassCommands)
{
    renderPassCommands->retainResource(this);

    // Keep references to the linked libraries alive.  Note that currently only need to do this for
    // the shaders library, as the vertex and fragment libraries live in the context until
    // destruction.
    if (mLinkedShaders != nullptr)
    {
        renderPassCommands->retainResource(mLinkedShaders);
    }
}

// FramebufferHelper implementation.
FramebufferHelper::FramebufferHelper() = default;

FramebufferHelper::~FramebufferHelper() = default;

FramebufferHelper::FramebufferHelper(FramebufferHelper &&other) : Resource(std::move(other))
{
    mFramebuffer = std::move(other.mFramebuffer);
}

FramebufferHelper &FramebufferHelper::operator=(FramebufferHelper &&other)
{
    Resource::operator=(std::move(other));
    std::swap(mFramebuffer, other.mFramebuffer);
    return *this;
}

angle::Result FramebufferHelper::init(ErrorContext *context,
                                      const VkFramebufferCreateInfo &createInfo)
{
    ANGLE_VK_TRY(context, mFramebuffer.init(context->getDevice(), createInfo));
    return angle::Result::Continue;
}

void FramebufferHelper::destroy(Renderer *renderer)
{
    mFramebuffer.destroy(renderer->getDevice());
}

void FramebufferHelper::release(ContextVk *contextVk)
{
    contextVk->addGarbage(&mFramebuffer);
}

// DescriptorSetDesc implementation.
size_t DescriptorSetDesc::hash() const
{
    if (mDescriptorInfos.empty())
    {
        return 0;
    }

    return angle::ComputeGenericHash(mDescriptorInfos.data(),
                                     sizeof(mDescriptorInfos[0]) * mDescriptorInfos.size());
}

// FramebufferDesc implementation.

FramebufferDesc::FramebufferDesc()
{
    reset();
}

FramebufferDesc::~FramebufferDesc()                                       = default;
FramebufferDesc::FramebufferDesc(const FramebufferDesc &other)            = default;
FramebufferDesc &FramebufferDesc::operator=(const FramebufferDesc &other) = default;

void FramebufferDesc::update(uint32_t index, ImageOrBufferViewSubresourceSerial serial)
{
    static_assert(kMaxFramebufferAttachments + 1 < std::numeric_limits<uint8_t>::max(),
                  "mMaxIndex size is too small");
    ASSERT(index < kMaxFramebufferAttachments);
    mSerials[index] = serial;
    if (serial.viewSerial.valid())
    {
        SetBitField(mMaxIndex, std::max(mMaxIndex, static_cast<uint16_t>(index + 1)));
    }
}

void FramebufferDesc::updateColor(uint32_t index, ImageOrBufferViewSubresourceSerial serial)
{
    update(kFramebufferDescColorIndexOffset + index, serial);
}

void FramebufferDesc::updateColorResolve(uint32_t index, ImageOrBufferViewSubresourceSerial serial)
{
    update(kFramebufferDescColorResolveIndexOffset + index, serial);
}

void FramebufferDesc::updateUnresolveMask(FramebufferNonResolveAttachmentMask unresolveMask)
{
    SetBitField(mUnresolveAttachmentMask, unresolveMask.bits());
}

void FramebufferDesc::updateDepthStencil(ImageOrBufferViewSubresourceSerial serial)
{
    update(kFramebufferDescDepthStencilIndex, serial);
}

void FramebufferDesc::updateDepthStencilResolve(ImageOrBufferViewSubresourceSerial serial)
{
    update(kFramebufferDescDepthStencilResolveIndexOffset, serial);
}

void FramebufferDesc::updateFragmentShadingRate(ImageOrBufferViewSubresourceSerial serial)
{
    update(kFramebufferDescFragmentShadingRateAttachmentIndexOffset, serial);
}

bool FramebufferDesc::hasFragmentShadingRateAttachment() const
{
    return mSerials[kFramebufferDescFragmentShadingRateAttachmentIndexOffset].viewSerial.valid();
}

size_t FramebufferDesc::hash() const
{
    return angle::ComputeGenericHash(&mSerials, sizeof(mSerials[0]) * mMaxIndex) ^
           mHasColorFramebufferFetch << 26 ^ mIsRenderToTexture << 25 ^ mLayerCount << 16 ^
           mUnresolveAttachmentMask;
}

void FramebufferDesc::reset()
{
    mMaxIndex                 = 0;
    mHasColorFramebufferFetch = false;
    mLayerCount               = 0;
    mSrgbWriteControlMode     = 0;
    mUnresolveAttachmentMask  = 0;
    mIsRenderToTexture        = 0;
    // An empty FramebufferDesc is still a valid desc. It becomes invalid when it is explicitly
    // destroyed or released by SharedFramebufferCacheKey.
    mIsValid = 1;
    memset(&mSerials, 0, sizeof(mSerials));
}

bool FramebufferDesc::operator==(const FramebufferDesc &other) const
{
    if (mMaxIndex != other.mMaxIndex || mLayerCount != other.mLayerCount ||
        mUnresolveAttachmentMask != other.mUnresolveAttachmentMask ||
        mHasColorFramebufferFetch != other.mHasColorFramebufferFetch ||
        mSrgbWriteControlMode != other.mSrgbWriteControlMode ||
        mIsRenderToTexture != other.mIsRenderToTexture)
    {
        return false;
    }

    size_t validRegionSize = sizeof(mSerials[0]) * mMaxIndex;
    return memcmp(&mSerials, &other.mSerials, validRegionSize) == 0;
}

uint32_t FramebufferDesc::attachmentCount() const
{
    uint32_t count = 0;
    for (const ImageOrBufferViewSubresourceSerial &serial : mSerials)
    {
        if (serial.viewSerial.valid())
        {
            count++;
        }
    }
    return count;
}

FramebufferNonResolveAttachmentMask FramebufferDesc::getUnresolveAttachmentMask() const
{
    return FramebufferNonResolveAttachmentMask(mUnresolveAttachmentMask);
}

void FramebufferDesc::updateLayerCount(uint32_t layerCount)
{
    SetBitField(mLayerCount, layerCount);
}

void FramebufferDesc::setColorFramebufferFetchMode(bool hasColorFramebufferFetch)
{
    SetBitField(mHasColorFramebufferFetch, hasColorFramebufferFetch);
}

void FramebufferDesc::updateRenderToTexture(bool isRenderToTexture)
{
    SetBitField(mIsRenderToTexture, isRenderToTexture);
}

void FramebufferDesc::destroyCachedObject(Renderer *renderer)
{
    ASSERT(valid());
    // Framebuffer cache are implemented in a way that each cache entry tracks GPU progress and we
    // always guarantee cache entries are released before calling destroy.
    SetBitField(mIsValid, 0);
}

void FramebufferDesc::releaseCachedObject(ContextVk *contextVk)
{
    ASSERT(valid());
    contextVk->getShareGroup()->getFramebufferCache().erase(contextVk, *this);
    SetBitField(mIsValid, 0);
}

// YcbcrConversionDesc implementation
YcbcrConversionDesc::YcbcrConversionDesc()
{
    reset();
}

YcbcrConversionDesc::~YcbcrConversionDesc() = default;

YcbcrConversionDesc::YcbcrConversionDesc(const YcbcrConversionDesc &other) = default;

YcbcrConversionDesc &YcbcrConversionDesc::operator=(const YcbcrConversionDesc &rhs) = default;

size_t YcbcrConversionDesc::hash() const
{
    return angle::ComputeGenericHash(*this);
}

bool YcbcrConversionDesc::operator==(const YcbcrConversionDesc &other) const
{
    return memcmp(this, &other, sizeof(YcbcrConversionDesc)) == 0;
}

void YcbcrConversionDesc::reset()
{
    mExternalOrVkFormat    = 0;
    mIsExternalFormat      = 0;
    mConversionModel       = 0;
    mColorRange            = 0;
    mXChromaOffset         = 0;
    mYChromaOffset         = 0;
    mChromaFilter          = 0;
    mRSwizzle              = 0;
    mGSwizzle              = 0;
    mBSwizzle              = 0;
    mASwizzle              = 0;
    mLinearFilterSupported = 0;
    mPadding               = 0;
    mReserved              = 0;
}

void YcbcrConversionDesc::update(Renderer *renderer,
                                 uint64_t externalFormat,
                                 VkSamplerYcbcrModelConversion conversionModel,
                                 VkSamplerYcbcrRange colorRange,
                                 VkChromaLocation xChromaOffset,
                                 VkChromaLocation yChromaOffset,
                                 VkFilter chromaFilter,
                                 VkComponentMapping components,
                                 angle::FormatID intendedFormatID,
                                 YcbcrLinearFilterSupport linearFilterSupported)
{
    const vk::Format &vkFormat = renderer->getFormat(intendedFormatID);
    ASSERT(externalFormat != 0 || vkFormat.getIntendedFormat().isYUV);

    SetBitField(mIsExternalFormat, (externalFormat) ? 1 : 0);
    SetBitField(mLinearFilterSupported,
                linearFilterSupported == YcbcrLinearFilterSupport::Supported);
    mExternalOrVkFormat =
        (externalFormat) ? externalFormat
                         : vkFormat.getActualImageVkFormat(renderer, vk::ImageAccess::SampleOnly);

    updateChromaFilter(renderer, chromaFilter);

    SetBitField(mConversionModel, conversionModel);
    SetBitField(mColorRange, colorRange);
    SetBitField(mXChromaOffset, xChromaOffset);
    SetBitField(mYChromaOffset, yChromaOffset);
    SetBitField(mRSwizzle, components.r);
    SetBitField(mGSwizzle, components.g);
    SetBitField(mBSwizzle, components.b);
    SetBitField(mASwizzle, components.a);
}

bool YcbcrConversionDesc::updateChromaFilter(Renderer *renderer, VkFilter filter)
{
    // The app has requested a specific min/mag filter, reconcile that with the filter
    // requested by preferLinearFilterForYUV feature.
    //
    // preferLinearFilterForYUV enforces linear filter while forceNearestFiltering and
    // forceNearestMipFiltering enforces nearest filter, enabling one precludes the other.
    ASSERT(!renderer->getFeatures().preferLinearFilterForYUV.enabled ||
           (!renderer->getFeatures().forceNearestFiltering.enabled &&
            !renderer->getFeatures().forceNearestMipFiltering.enabled));

    VkFilter preferredChromaFilter = renderer->getPreferredFilterForYUV(filter);
    ASSERT(preferredChromaFilter == VK_FILTER_LINEAR || preferredChromaFilter == VK_FILTER_NEAREST);

    if (preferredChromaFilter == VK_FILTER_LINEAR && !mLinearFilterSupported)
    {
        // Vulkan implementations may not support linear filtering in all cases. If not supported,
        // use nearest filtering instead.
        preferredChromaFilter = VK_FILTER_NEAREST;
    }

    if (getChromaFilter() != preferredChromaFilter)
    {
        SetBitField(mChromaFilter, preferredChromaFilter);
        return true;
    }
    return false;
}

void YcbcrConversionDesc::updateConversionModel(VkSamplerYcbcrModelConversion conversionModel)
{
    SetBitField(mConversionModel, conversionModel);
}

angle::Result YcbcrConversionDesc::init(ErrorContext *context,
                                        SamplerYcbcrConversion *conversionOut) const
{
    // Create the VkSamplerYcbcrConversion
    VkSamplerYcbcrConversionCreateInfo samplerYcbcrConversionInfo = {};
    samplerYcbcrConversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    samplerYcbcrConversionInfo.format =
        getExternalFormat() == 0 ? static_cast<VkFormat>(mExternalOrVkFormat) : VK_FORMAT_UNDEFINED;
    samplerYcbcrConversionInfo.xChromaOffset = static_cast<VkChromaLocation>(mXChromaOffset);
    samplerYcbcrConversionInfo.yChromaOffset = static_cast<VkChromaLocation>(mYChromaOffset);
    samplerYcbcrConversionInfo.ycbcrModel =
        static_cast<VkSamplerYcbcrModelConversion>(mConversionModel);
    samplerYcbcrConversionInfo.ycbcrRange   = static_cast<VkSamplerYcbcrRange>(mColorRange);
    samplerYcbcrConversionInfo.chromaFilter = static_cast<VkFilter>(mChromaFilter);
    samplerYcbcrConversionInfo.components   = {
        static_cast<VkComponentSwizzle>(mRSwizzle), static_cast<VkComponentSwizzle>(mGSwizzle),
        static_cast<VkComponentSwizzle>(mBSwizzle), static_cast<VkComponentSwizzle>(mASwizzle)};

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    VkExternalFormatANDROID externalFormat = {};
    if (getExternalFormat() != 0)
    {
        externalFormat.sType             = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.externalFormat    = mExternalOrVkFormat;
        samplerYcbcrConversionInfo.pNext = &externalFormat;
    }
#else
    // We do not support external format for any platform other than Android.
    ASSERT(mIsExternalFormat == 0);
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    ANGLE_VK_TRY(context, conversionOut->init(context->getDevice(), samplerYcbcrConversionInfo));
    return angle::Result::Continue;
}

// SamplerDesc implementation.
SamplerDesc::SamplerDesc()
{
    reset();
}

SamplerDesc::~SamplerDesc() = default;

SamplerDesc::SamplerDesc(const SamplerDesc &other) = default;

SamplerDesc &SamplerDesc::operator=(const SamplerDesc &rhs) = default;

SamplerDesc::SamplerDesc(ErrorContext *context,
                         const gl::SamplerState &samplerState,
                         bool stencilMode,
                         const YcbcrConversionDesc *ycbcrConversionDesc,
                         angle::FormatID intendedFormatID)
{
    update(context->getRenderer(), samplerState, stencilMode, ycbcrConversionDesc,
           intendedFormatID);
}

void SamplerDesc::reset()
{
    mMipLodBias    = 0.0f;
    mMaxAnisotropy = 0.0f;
    mMinLod        = 0.0f;
    mMaxLod        = 0.0f;
    mYcbcrConversionDesc.reset();
    mMagFilter         = 0;
    mMinFilter         = 0;
    mMipmapMode        = 0;
    mAddressModeU      = 0;
    mAddressModeV      = 0;
    mAddressModeW      = 0;
    mCompareEnabled    = 0;
    mCompareOp         = 0;
    mPadding           = 0;
    mBorderColorType   = 0;
    mBorderColor.red   = 0.0f;
    mBorderColor.green = 0.0f;
    mBorderColor.blue  = 0.0f;
    mBorderColor.alpha = 0.0f;
    mReserved          = 0;
}

void SamplerDesc::update(Renderer *renderer,
                         const gl::SamplerState &samplerState,
                         bool stencilMode,
                         const YcbcrConversionDesc *ycbcrConversionDesc,
                         angle::FormatID intendedFormatID)
{
    const angle::FeaturesVk &featuresVk = renderer->getFeatures();
    mMipLodBias                         = 0.0f;
    if (featuresVk.forceTextureLodOffset1.enabled)
    {
        mMipLodBias = 1.0f;
    }
    else if (featuresVk.forceTextureLodOffset2.enabled)
    {
        mMipLodBias = 2.0f;
    }
    else if (featuresVk.forceTextureLodOffset3.enabled)
    {
        mMipLodBias = 3.0f;
    }
    else if (featuresVk.forceTextureLodOffset4.enabled)
    {
        mMipLodBias = 4.0f;
    }

    mMaxAnisotropy = samplerState.getMaxAnisotropy();
    mMinLod        = samplerState.getMinLod();
    mMaxLod        = samplerState.getMaxLod();

    GLenum minFilter = samplerState.getMinFilter();
    GLenum magFilter = samplerState.getMagFilter();
    if (ycbcrConversionDesc && ycbcrConversionDesc->valid())
    {
        // Update the SamplerYcbcrConversionCache key
        mYcbcrConversionDesc = *ycbcrConversionDesc;

        // Reconcile chroma filter and min/mag filters.
        //
        // VUID-VkSamplerCreateInfo-minFilter-01645
        // If sampler YCBCR conversion is enabled and the potential format features of the
        // sampler YCBCR conversion do not support
        // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,
        // minFilter and magFilter must be equal to the sampler YCBCR conversions chromaFilter.
        //
        // For simplicity assume external formats do not support that feature bit.
        ASSERT((mYcbcrConversionDesc.getExternalFormat() != 0) ||
               (angle::Format::Get(intendedFormatID).isYUV));
        const bool filtersMustMatch =
            (mYcbcrConversionDesc.getExternalFormat() != 0) ||
            !renderer->hasImageFormatFeatureBits(
                intendedFormatID,
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT);
        if (filtersMustMatch)
        {
            GLenum glFilter = (mYcbcrConversionDesc.getChromaFilter() == VK_FILTER_LINEAR)
                                  ? GL_LINEAR
                                  : GL_NEAREST;
            minFilter       = glFilter;
            magFilter       = glFilter;
        }
    }

    bool compareEnable    = samplerState.getCompareMode() == GL_COMPARE_REF_TO_TEXTURE;
    VkCompareOp compareOp = gl_vk::GetCompareOp(samplerState.getCompareFunc());
    // When sampling from stencil, deqp tests expect texture compare to have no effect
    // dEQP - GLES31.functional.stencil_texturing.misc.compare_mode_effect
    // states: NOTE: Texture compare mode has no effect when reading stencil values.
    if (stencilMode)
    {
        compareEnable = VK_FALSE;
        compareOp     = VK_COMPARE_OP_ALWAYS;
    }

    if (featuresVk.forceNearestFiltering.enabled)
    {
        magFilter = gl::ConvertToNearestFilterMode(magFilter);
        minFilter = gl::ConvertToNearestFilterMode(minFilter);
    }
    if (featuresVk.forceNearestMipFiltering.enabled)
    {
        minFilter = gl::ConvertToNearestMipFilterMode(minFilter);
    }

    SetBitField(mMagFilter, gl_vk::GetFilter(magFilter));
    SetBitField(mMinFilter, gl_vk::GetFilter(minFilter));
    SetBitField(mMipmapMode, gl_vk::GetSamplerMipmapMode(samplerState.getMinFilter()));
    SetBitField(mAddressModeU, gl_vk::GetSamplerAddressMode(samplerState.getWrapS()));
    SetBitField(mAddressModeV, gl_vk::GetSamplerAddressMode(samplerState.getWrapT()));
    SetBitField(mAddressModeW, gl_vk::GetSamplerAddressMode(samplerState.getWrapR()));
    SetBitField(mCompareEnabled, compareEnable);
    SetBitField(mCompareOp, compareOp);

    if (!gl::IsMipmapFiltered(minFilter))
    {
        // Per the Vulkan spec, GL_NEAREST and GL_LINEAR do not map directly to Vulkan, so
        // they must be emulated (See "Mapping of OpenGL to Vulkan filter modes")
        SetBitField(mMipmapMode, VK_SAMPLER_MIPMAP_MODE_NEAREST);
        mMinLod = 0.0f;
        mMaxLod = 0.25f;
    }

    mPadding = 0;

    mBorderColorType =
        (samplerState.getBorderColor().type == angle::ColorGeneric::Type::Float) ? 0 : 1;

    // Adjust border color according to intended format
    const vk::Format &vkFormat = renderer->getFormat(intendedFormatID);
    gl::ColorGeneric adjustedBorderColor =
        AdjustBorderColor(samplerState.getBorderColor(), vkFormat.getIntendedFormat(), stencilMode);
    mBorderColor = adjustedBorderColor.colorF;

    mReserved = 0;
}

angle::Result SamplerDesc::init(ContextVk *contextVk, Sampler *sampler) const
{
    const gl::Extensions &extensions = contextVk->getExtensions();

    bool anisotropyEnable = extensions.textureFilterAnisotropicEXT && mMaxAnisotropy > 1.0f;

    VkSamplerCreateInfo createInfo     = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.flags                   = 0;
    createInfo.magFilter               = static_cast<VkFilter>(mMagFilter);
    createInfo.minFilter               = static_cast<VkFilter>(mMinFilter);
    createInfo.mipmapMode              = static_cast<VkSamplerMipmapMode>(mMipmapMode);
    createInfo.addressModeU            = static_cast<VkSamplerAddressMode>(mAddressModeU);
    createInfo.addressModeV            = static_cast<VkSamplerAddressMode>(mAddressModeV);
    createInfo.addressModeW            = static_cast<VkSamplerAddressMode>(mAddressModeW);
    createInfo.mipLodBias              = mMipLodBias;
    createInfo.anisotropyEnable        = anisotropyEnable;
    createInfo.maxAnisotropy           = mMaxAnisotropy;
    createInfo.compareEnable           = mCompareEnabled ? VK_TRUE : VK_FALSE;
    createInfo.compareOp               = static_cast<VkCompareOp>(mCompareOp);
    createInfo.minLod                  = mMinLod;
    createInfo.maxLod                  = mMaxLod;
    createInfo.borderColor             = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VkSamplerYcbcrConversionInfo samplerYcbcrConversionInfo = {};
    if (mYcbcrConversionDesc.valid())
    {
        ASSERT((contextVk->getFeatures().supportsYUVSamplerConversion.enabled));
        samplerYcbcrConversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
        samplerYcbcrConversionInfo.pNext = nullptr;
        ANGLE_TRY(contextVk->getRenderer()->getYuvConversionCache().getSamplerYcbcrConversion(
            contextVk, mYcbcrConversionDesc, &samplerYcbcrConversionInfo.conversion));
        AddToPNextChain(&createInfo, &samplerYcbcrConversionInfo);

        // Vulkan spec requires these settings:
        createInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.anisotropyEnable        = VK_FALSE;
        createInfo.unnormalizedCoordinates = VK_FALSE;
    }

    VkSamplerCustomBorderColorCreateInfoEXT customBorderColorInfo = {};
    if (createInfo.addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
        createInfo.addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
        createInfo.addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
    {
        ASSERT((contextVk->getFeatures().supportsCustomBorderColor.enabled));
        customBorderColorInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;

        customBorderColorInfo.customBorderColor.float32[0] = mBorderColor.red;
        customBorderColorInfo.customBorderColor.float32[1] = mBorderColor.green;
        customBorderColorInfo.customBorderColor.float32[2] = mBorderColor.blue;
        customBorderColorInfo.customBorderColor.float32[3] = mBorderColor.alpha;

        if (mBorderColorType == static_cast<uint32_t>(angle::ColorGeneric::Type::Float))
        {
            createInfo.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
        }
        else
        {
            createInfo.borderColor = VK_BORDER_COLOR_INT_CUSTOM_EXT;
        }

        vk::AddToPNextChain(&createInfo, &customBorderColorInfo);
    }
    ANGLE_VK_TRY(contextVk, sampler->init(contextVk->getDevice(), createInfo));

    return angle::Result::Continue;
}

size_t SamplerDesc::hash() const
{
    return angle::ComputeGenericHash(*this);
}

bool SamplerDesc::operator==(const SamplerDesc &other) const
{
    return memcmp(this, &other, sizeof(SamplerDesc)) == 0;
}

SamplerHelper::SamplerHelper(SamplerHelper &&samplerHelper)
{
    *this = std::move(samplerHelper);
}

SamplerHelper &SamplerHelper::operator=(SamplerHelper &&rhs)
{
    std::swap(mSampler, rhs.mSampler);
    std::swap(mSamplerSerial, rhs.mSamplerSerial);
    return *this;
}

angle::Result SamplerHelper::init(ErrorContext *context, const VkSamplerCreateInfo &createInfo)
{
    mSamplerSerial = context->getRenderer()->getResourceSerialFactory().generateSamplerSerial();
    ANGLE_VK_TRY(context, mSampler.init(context->getDevice(), createInfo));
    return angle::Result::Continue;
}
angle::Result SamplerHelper::init(ContextVk *contextVk, const SamplerDesc &desc)
{
    mSamplerSerial = contextVk->getRenderer()->getResourceSerialFactory().generateSamplerSerial();
    return desc.init(contextVk, &mSampler);
}

// RenderPassHelper implementation.
RenderPassHelper::RenderPassHelper() : mPerfCounters{} {}

RenderPassHelper::~RenderPassHelper() = default;

RenderPassHelper::RenderPassHelper(RenderPassHelper &&other)
{
    *this = std::move(other);
}

RenderPassHelper &RenderPassHelper::operator=(RenderPassHelper &&other)
{
    mRenderPass   = std::move(other.mRenderPass);
    mPerfCounters = std::move(other.mPerfCounters);
    return *this;
}

void RenderPassHelper::destroy(VkDevice device)
{
    mRenderPass.destroy(device);
}

void RenderPassHelper::release(ContextVk *contextVk)
{
    contextVk->addGarbage(&mRenderPass);
}

const RenderPass &RenderPassHelper::getRenderPass() const
{
    return mRenderPass;
}

RenderPass &RenderPassHelper::getRenderPass()
{
    return mRenderPass;
}

const RenderPassPerfCounters &RenderPassHelper::getPerfCounters() const
{
    return mPerfCounters;
}

RenderPassPerfCounters &RenderPassHelper::getPerfCounters()
{
    return mPerfCounters;
}

// WriteDescriptorDescs implementation.
void WriteDescriptorDescs::updateWriteDesc(uint32_t bindingIndex,
                                           VkDescriptorType descriptorType,
                                           uint32_t descriptorCount)
{
    if (hasWriteDescAtIndex(bindingIndex))
    {
        uint32_t infoIndex          = mDescs[bindingIndex].descriptorInfoIndex;
        uint32_t oldDescriptorCount = mDescs[bindingIndex].descriptorCount;
        if (descriptorCount != oldDescriptorCount)
        {
            ASSERT(infoIndex + oldDescriptorCount == mCurrentInfoIndex);
            ASSERT(descriptorCount > oldDescriptorCount);
            uint32_t additionalDescriptors = descriptorCount - oldDescriptorCount;
            incrementDescriptorCount(bindingIndex, additionalDescriptors);
            mCurrentInfoIndex += additionalDescriptors;
        }
    }
    else
    {
        WriteDescriptorDesc &writeDesc = mDescs[bindingIndex];
        SetBitField(writeDesc.binding, bindingIndex);
        SetBitField(writeDesc.descriptorCount, descriptorCount);
        SetBitField(writeDesc.descriptorType, descriptorType);
        SetBitField(writeDesc.descriptorInfoIndex, mCurrentInfoIndex);
        mCurrentInfoIndex += descriptorCount;
        ASSERT(writeDesc.descriptorCount > 0);
    }
}

void WriteDescriptorDescs::updateShaderBuffers(
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const std::vector<gl::InterfaceBlock> &blocks,
    VkDescriptorType descriptorType)
{
    // Initialize the descriptor writes in a first pass. This ensures we can pack the structures
    // corresponding to array elements tightly.
    for (uint32_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex)
    {
        const gl::InterfaceBlock &block = blocks[blockIndex];

        if (block.activeShaders().none())
        {
            continue;
        }

        const gl::ShaderType firstShaderType = block.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            variableInfoMap.getVariableById(firstShaderType, block.getId(firstShaderType));

        if (block.pod.isArray && block.pod.arrayElement > 0)
        {
            incrementDescriptorCount(info.binding, 1);
            mCurrentInfoIndex++;
        }
        else
        {
            updateWriteDesc(info.binding, descriptorType, 1);
        }
    }
}

void WriteDescriptorDescs::updateAtomicCounters(
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers)
{
    if (atomicCounterBuffers.empty())
    {
        return;
    }

    static_assert(!IsDynamicDescriptor(kStorageBufferDescriptorType),
                  "This method needs an update to handle dynamic descriptors");

    uint32_t binding = variableInfoMap.getAtomicCounterBufferBinding(
        atomicCounterBuffers[0].getFirstActiveShaderType(), 0);

    updateWriteDesc(binding, kStorageBufferDescriptorType,
                    gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
}

void WriteDescriptorDescs::updateImages(const gl::ProgramExecutable &executable,
                                        const ShaderInterfaceVariableInfoMap &variableInfoMap)
{
    const std::vector<gl::ImageBinding> &imageBindings = executable.getImageBindings();
    const std::vector<gl::LinkedUniform> &uniforms     = executable.getUniforms();

    if (imageBindings.empty())
    {
        return;
    }

    for (uint32_t imageIndex = 0; imageIndex < imageBindings.size(); ++imageIndex)
    {
        const gl::ImageBinding &imageBinding = imageBindings[imageIndex];
        uint32_t uniformIndex                = executable.getUniformIndexFromImageIndex(imageIndex);
        const gl::LinkedUniform &imageUniform = uniforms[uniformIndex];

        if (imageUniform.activeShaders().none())
        {
            continue;
        }

        const gl::ShaderType firstShaderType = imageUniform.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            variableInfoMap.getVariableById(firstShaderType, imageUniform.getId(firstShaderType));

        uint32_t arraySize       = static_cast<uint32_t>(imageBinding.boundImageUnits.size());
        uint32_t descriptorCount = arraySize * imageUniform.getOuterArraySizeProduct();
        VkDescriptorType descriptorType = (imageBinding.textureType == gl::TextureType::Buffer)
                                              ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                                              : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        updateWriteDesc(info.binding, descriptorType, descriptorCount);
    }
}

void WriteDescriptorDescs::updateInputAttachments(
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    FramebufferVk *framebufferVk)
{
    if (framebufferVk->getDepthStencilRenderTarget() != nullptr)
    {
        if (executable.usesDepthFramebufferFetch())
        {
            const uint32_t depthBinding =
                variableInfoMap
                    .getVariableById(gl::ShaderType::Fragment,
                                     sh::vk::spirv::kIdDepthInputAttachment)
                    .binding;
            updateWriteDesc(depthBinding, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1);
        }

        if (executable.usesStencilFramebufferFetch())
        {
            const uint32_t stencilBinding =
                variableInfoMap
                    .getVariableById(gl::ShaderType::Fragment,
                                     sh::vk::spirv::kIdStencilInputAttachment)
                    .binding;
            updateWriteDesc(stencilBinding, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1);
        }
    }

    if (!executable.usesColorFramebufferFetch())
    {
        return;
    }

    const uint32_t firstColorInputAttachment =
        static_cast<uint32_t>(executable.getFragmentInoutIndices().first());

    const ShaderInterfaceVariableInfo &baseColorInfo = variableInfoMap.getVariableById(
        gl::ShaderType::Fragment, sh::vk::spirv::kIdInputAttachment0 + firstColorInputAttachment);

    const uint32_t baseColorBinding = baseColorInfo.binding - firstColorInputAttachment;

    for (size_t colorIndex : framebufferVk->getState().getColorAttachmentsMask())
    {
        uint32_t binding = baseColorBinding + static_cast<uint32_t>(colorIndex);
        updateWriteDesc(binding, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1);
    }
}

void WriteDescriptorDescs::updateExecutableActiveTextures(
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::ProgramExecutable &executable)
{
    const std::vector<gl::SamplerBinding> &samplerBindings = executable.getSamplerBindings();
    const std::vector<gl::LinkedUniform> &uniforms         = executable.getUniforms();

    for (uint32_t samplerIndex = 0; samplerIndex < samplerBindings.size(); ++samplerIndex)
    {
        const gl::SamplerBinding &samplerBinding = samplerBindings[samplerIndex];
        uint32_t uniformIndex = executable.getUniformIndexFromSamplerIndex(samplerIndex);
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];

        if (samplerUniform.activeShaders().none())
        {
            continue;
        }

        const gl::ShaderType firstShaderType = samplerUniform.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            variableInfoMap.getVariableById(firstShaderType, samplerUniform.getId(firstShaderType));

        uint32_t arraySize              = static_cast<uint32_t>(samplerBinding.textureUnitsCount);
        uint32_t descriptorCount        = arraySize * samplerUniform.getOuterArraySizeProduct();
        VkDescriptorType descriptorType = (samplerBinding.textureType == gl::TextureType::Buffer)
                                              ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
                                              : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        updateWriteDesc(info.binding, descriptorType, descriptorCount);
    }
}

void WriteDescriptorDescs::updateDefaultUniform(
    gl::ShaderBitSet shaderTypes,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::ProgramExecutable &executable)
{
    for (const gl::ShaderType shaderType : shaderTypes)
    {
        uint32_t binding = variableInfoMap.getDefaultUniformBinding(shaderType);
        updateWriteDesc(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);
    }
}

void WriteDescriptorDescs::updateTransformFeedbackWrite(
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::ProgramExecutable &executable)
{
    uint32_t xfbBufferCount = static_cast<uint32_t>(executable.getTransformFeedbackBufferCount());
    updateWriteDesc(variableInfoMap.getEmulatedXfbBufferBinding(0),
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, xfbBufferCount);
}

void WriteDescriptorDescs::updateDynamicDescriptorsCount()
{
    mDynamicDescriptorSetCount = 0;
    for (uint32_t index = 0; index < mDescs.size(); ++index)
    {
        const WriteDescriptorDesc &writeDesc = mDescs[index];
        if (IsDynamicDescriptor(static_cast<VkDescriptorType>(writeDesc.descriptorType)))
        {
            mDynamicDescriptorSetCount += writeDesc.descriptorCount;
        }
    }
}

std::ostream &operator<<(std::ostream &os, const WriteDescriptorDescs &desc)
{
    os << " WriteDescriptorDescs[" << desc.size() << "]:";
    for (uint32_t index = 0; index < desc.size(); ++index)
    {
        const WriteDescriptorDesc &writeDesc = desc[index];
        os << static_cast<int>(writeDesc.binding) << ": "
           << static_cast<int>(writeDesc.descriptorCount) << ": "
           << kDescriptorTypeNameMap[writeDesc.descriptorType] << ": "
           << writeDesc.descriptorInfoIndex;
    }
    return os;
}

// DescriptorSetDesc implementation.
void DescriptorSetDesc::updateDescriptorSet(Renderer *renderer,
                                            const WriteDescriptorDescs &writeDescriptorDescs,
                                            UpdateDescriptorSetsBuilder *updateBuilder,
                                            const DescriptorDescHandles *handles,
                                            VkDescriptorSet descriptorSet) const
{
    for (uint32_t writeIndex = 0; writeIndex < writeDescriptorDescs.size(); ++writeIndex)
    {
        const WriteDescriptorDesc &writeDesc = writeDescriptorDescs[writeIndex];

        if (writeDesc.descriptorCount == 0)
        {
            continue;
        }

        VkWriteDescriptorSet &writeSet = updateBuilder->allocWriteDescriptorSet();

        writeSet.descriptorCount  = writeDesc.descriptorCount;
        writeSet.descriptorType   = static_cast<VkDescriptorType>(writeDesc.descriptorType);
        writeSet.dstArrayElement  = 0;
        writeSet.dstBinding       = writeIndex;
        writeSet.dstSet           = descriptorSet;
        writeSet.pBufferInfo      = nullptr;
        writeSet.pImageInfo       = nullptr;
        writeSet.pNext            = nullptr;
        writeSet.pTexelBufferView = nullptr;
        writeSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

        uint32_t infoDescIndex = writeDesc.descriptorInfoIndex;

        switch (writeSet.descriptorType)
        {
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            {
                ASSERT(writeDesc.descriptorCount == 1);
                VkBufferView &bufferView  = updateBuilder->allocBufferView();
                bufferView                = handles[infoDescIndex].bufferView;
                writeSet.pTexelBufferView = &bufferView;
                break;
            }
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            {
                VkDescriptorBufferInfo *writeBuffers =
                    updateBuilder->allocDescriptorBufferInfos(writeSet.descriptorCount);
                for (uint32_t arrayElement = 0; arrayElement < writeSet.descriptorCount;
                     ++arrayElement)
                {
                    const DescriptorInfoDesc &infoDesc =
                        mDescriptorInfos[infoDescIndex + arrayElement];
                    VkDescriptorBufferInfo &bufferInfo = writeBuffers[arrayElement];
                    bufferInfo.buffer = handles[infoDescIndex + arrayElement].buffer;
                    bufferInfo.offset = infoDesc.imageViewSerialOrOffset;
                    bufferInfo.range  = infoDesc.imageLayoutOrRange;
                }
                writeSet.pBufferInfo = writeBuffers;
                break;
            }
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            {
                VkDescriptorImageInfo *writeImages =
                    updateBuilder->allocDescriptorImageInfos(writeSet.descriptorCount);
                for (uint32_t arrayElement = 0; arrayElement < writeSet.descriptorCount;
                     ++arrayElement)
                {
                    const DescriptorInfoDesc &infoDesc =
                        mDescriptorInfos[infoDescIndex + arrayElement];
                    VkDescriptorImageInfo &imageInfo = writeImages[arrayElement];

                    ImageLayout imageLayout = static_cast<ImageLayout>(infoDesc.imageLayoutOrRange);

                    imageInfo.imageLayout =
                        ConvertImageLayoutToVkImageLayout(renderer, imageLayout);
                    imageInfo.imageView = handles[infoDescIndex + arrayElement].imageView;
                    imageInfo.sampler   = handles[infoDescIndex + arrayElement].sampler;
                }
                writeSet.pImageInfo = writeImages;
                break;
            }

            default:
                UNREACHABLE();
                break;
        }
    }
}

std::ostream &operator<<(std::ostream &os, const DescriptorSetDesc &desc)
{
    os << " desc[" << desc.size() << "]:";
    for (uint32_t index = 0; index < desc.size(); ++index)
    {
        const DescriptorInfoDesc &infoDesc = desc.getInfoDesc(index);
        os << "{" << infoDesc.samplerOrBufferSerial << ", " << infoDesc.imageViewSerialOrOffset
           << ", " << infoDesc.imageLayoutOrRange << ", " << infoDesc.imageSubresourceRange << "}";
    }
    return os;
}

// DescriptorSetDescAndPool implementation
void DescriptorSetDescAndPool::destroyCachedObject(Renderer *renderer)
{
    ASSERT(valid());
    mPool->destroyCachedDescriptorSet(renderer, mDesc);
    mPool = nullptr;
}

void DescriptorSetDescAndPool::releaseCachedObject(Renderer *renderer)
{
    ASSERT(valid());
    mPool->releaseCachedDescriptorSet(renderer, mDesc);
    mPool = nullptr;
}

// DescriptorSetDescBuilder implementation.
DescriptorSetDescBuilder::DescriptorSetDescBuilder() = default;
DescriptorSetDescBuilder::DescriptorSetDescBuilder(size_t descriptorCount)
{
    resize(descriptorCount);
}

DescriptorSetDescBuilder::~DescriptorSetDescBuilder() {}

DescriptorSetDescBuilder::DescriptorSetDescBuilder(const DescriptorSetDescBuilder &other)
    : mDesc(other.mDesc), mHandles(other.mHandles), mDynamicOffsets(other.mDynamicOffsets)
{}

DescriptorSetDescBuilder &DescriptorSetDescBuilder::operator=(const DescriptorSetDescBuilder &other)
{
    mDesc           = other.mDesc;
    mHandles        = other.mHandles;
    mDynamicOffsets = other.mDynamicOffsets;
    return *this;
}

void DescriptorSetDescBuilder::updateUniformBuffer(uint32_t bindingIndex,
                                                   const WriteDescriptorDescs &writeDescriptorDescs,
                                                   const BufferHelper &bufferHelper,
                                                   VkDeviceSize bufferRange)
{
    uint32_t infoIndex           = writeDescriptorDescs[bindingIndex].descriptorInfoIndex;
    DescriptorInfoDesc &infoDesc = mDesc.getInfoDesc(infoIndex);

    infoDesc.samplerOrBufferSerial   = bufferHelper.getBlockSerial().getValue();
    infoDesc.imageViewSerialOrOffset = 0;
    SetBitField(infoDesc.imageLayoutOrRange, bufferRange);
    infoDesc.imageSubresourceRange = 0;

    mHandles[infoIndex].buffer = bufferHelper.getBuffer().getHandle();
}

void DescriptorSetDescBuilder::updateTransformFeedbackBuffer(
    const Context *context,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const WriteDescriptorDescs &writeDescriptorDescs,
    uint32_t xfbBufferIndex,
    const BufferHelper &bufferHelper,
    VkDeviceSize bufferOffset,
    VkDeviceSize bufferRange)
{
    const uint32_t baseBinding = variableInfoMap.getEmulatedXfbBufferBinding(0);

    Renderer *renderer = context->getRenderer();
    VkDeviceSize offsetAlignment =
        renderer->getPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
    // Set the offset as close as possible to the requested offset while remaining aligned.
    VkDeviceSize alignedOffset = (bufferOffset / offsetAlignment) * offsetAlignment;
    VkDeviceSize adjustedRange = bufferRange + (bufferOffset - alignedOffset);

    uint32_t infoIndex = writeDescriptorDescs[baseBinding].descriptorInfoIndex + xfbBufferIndex;
    DescriptorInfoDesc &infoDesc   = mDesc.getInfoDesc(infoIndex);
    infoDesc.samplerOrBufferSerial = bufferHelper.getBufferSerial().getValue();
    SetBitField(infoDesc.imageViewSerialOrOffset, alignedOffset);
    SetBitField(infoDesc.imageLayoutOrRange, adjustedRange);
    infoDesc.imageSubresourceRange = 0;

    mHandles[infoIndex].buffer = bufferHelper.getBuffer().getHandle();
}

void DescriptorSetDescBuilder::updateUniformsAndXfb(
    Context *context,
    const gl::ProgramExecutable &executable,
    const WriteDescriptorDescs &writeDescriptorDescs,
    const BufferHelper *currentUniformBuffer,
    const BufferHelper &emptyBuffer,
    bool activeUnpaused,
    TransformFeedbackVk *transformFeedbackVk)
{
    const ProgramExecutableVk *executableVk = vk::GetImpl(&executable);
    gl::ShaderBitSet linkedStages           = executable.getLinkedShaderStages();

    const ShaderInterfaceVariableInfoMap &variableInfoMap = executableVk->getVariableInfoMap();

    for (const gl::ShaderType shaderType : linkedStages)
    {
        uint32_t binding         = variableInfoMap.getDefaultUniformBinding(shaderType);
        VkDeviceSize bufferRange = executableVk->getDefaultUniformAlignedSize(context, shaderType);
        if (bufferRange == 0)
        {
            updateUniformBuffer(binding, writeDescriptorDescs, emptyBuffer, emptyBuffer.getSize());
        }
        else
        {
            ASSERT(currentUniformBuffer);
            updateUniformBuffer(binding, writeDescriptorDescs, *currentUniformBuffer, bufferRange);
        }

        if (transformFeedbackVk && shaderType == gl::ShaderType::Vertex &&
            context->getFeatures().emulateTransformFeedback.enabled)
        {
            transformFeedbackVk->updateTransformFeedbackDescriptorDesc(
                context, executable, variableInfoMap, writeDescriptorDescs, emptyBuffer,
                activeUnpaused, this);
        }
    }
}

void DescriptorSetDescBuilder::updatePreCacheActiveTextures(
    Context *context,
    const gl::ProgramExecutable &executable,
    const gl::ActiveTextureArray<TextureVk *> &textures,
    const gl::SamplerBindingVector &samplers)
{
    const std::vector<gl::SamplerBinding> &samplerBindings = executable.getSamplerBindings();
    const gl::ActiveTextureMask &activeTextures            = executable.getActiveSamplersMask();
    const ProgramExecutableVk *executableVk                = vk::GetImpl(&executable);

    resize(executableVk->getTextureWriteDescriptorDescs().getTotalDescriptorCount());
    const WriteDescriptorDescs &writeDescriptorDescs =
        executableVk->getTextureWriteDescriptorDescs();

    const ShaderInterfaceVariableInfoMap &variableInfoMap = executableVk->getVariableInfoMap();
    const std::vector<gl::LinkedUniform> &uniforms        = executable.getUniforms();

    for (uint32_t samplerIndex = 0; samplerIndex < samplerBindings.size(); ++samplerIndex)
    {
        const gl::SamplerBinding &samplerBinding = samplerBindings[samplerIndex];
        uint16_t arraySize                       = samplerBinding.textureUnitsCount;
        bool isSamplerExternalY2Y = samplerBinding.samplerType == GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT;

        uint32_t uniformIndex = executable.getUniformIndexFromSamplerIndex(samplerIndex);
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];

        if (samplerUniform.activeShaders().none())
        {
            continue;
        }

        const gl::ShaderType firstShaderType = samplerUniform.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            variableInfoMap.getVariableById(firstShaderType, samplerUniform.getId(firstShaderType));

        for (uint16_t arrayElement = 0; arrayElement < arraySize; ++arrayElement)
        {
            GLuint textureUnit = samplerBinding.getTextureUnit(
                executable.getSamplerBoundTextureUnits(), arrayElement);
            if (!activeTextures.test(textureUnit))
                continue;
            TextureVk *textureVk = textures[textureUnit];

            uint32_t infoIndex = writeDescriptorDescs[info.binding].descriptorInfoIndex +
                                 arrayElement + samplerUniform.getOuterArrayOffset();
            DescriptorInfoDesc &infoDesc = mDesc.getInfoDesc(infoIndex);

            if (textureVk->getState().getType() == gl::TextureType::Buffer)
            {
                ImageOrBufferViewSubresourceSerial imageViewSerial =
                    textureVk->getBufferViewSerial();
                infoDesc.imageViewSerialOrOffset = imageViewSerial.viewSerial.getValue();
                infoDesc.imageLayoutOrRange      = 0;
                infoDesc.samplerOrBufferSerial   = 0;
                infoDesc.imageSubresourceRange   = 0;
            }
            else
            {
                gl::Sampler *sampler       = samplers[textureUnit].get();
                const SamplerVk *samplerVk = sampler ? vk::GetImpl(sampler) : nullptr;

                const SamplerHelper &samplerHelper =
                    samplerVk ? samplerVk->getSampler()
                              : textureVk->getSampler(isSamplerExternalY2Y);
                const gl::SamplerState &samplerState =
                    sampler ? sampler->getSamplerState() : textureVk->getState().getSamplerState();

                ImageOrBufferViewSubresourceSerial imageViewSerial =
                    textureVk->getImageViewSubresourceSerial(
                        samplerState, samplerUniform.isTexelFetchStaticUse());

                ImageLayout imageLayout = textureVk->getImage().getCurrentImageLayout();
                SetBitField(infoDesc.imageLayoutOrRange, imageLayout);
                infoDesc.imageViewSerialOrOffset = imageViewSerial.viewSerial.getValue();
                infoDesc.samplerOrBufferSerial   = samplerHelper.getSamplerSerial().getValue();
                memcpy(&infoDesc.imageSubresourceRange, &imageViewSerial.subresource,
                       sizeof(uint32_t));
            }
        }
    }
}

void DescriptorSetDescBuilder::setEmptyBuffer(uint32_t infoDescIndex,
                                              VkDescriptorType descriptorType,
                                              const BufferHelper &emptyBuffer)
{
    DescriptorInfoDesc &emptyDesc = mDesc.getInfoDesc(infoDescIndex);
    SetBitField(emptyDesc.imageLayoutOrRange, emptyBuffer.getSize());
    emptyDesc.imageViewSerialOrOffset = 0;
    emptyDesc.samplerOrBufferSerial   = emptyBuffer.getBlockSerial().getValue();

    mHandles[infoDescIndex].buffer = emptyBuffer.getBuffer().getHandle();

    if (IsDynamicDescriptor(descriptorType))
    {
        mDynamicOffsets[infoDescIndex] = 0;
    }
}

template <typename CommandBufferT>
void DescriptorSetDescBuilder::updateOneShaderBuffer(
    Context *context,
    CommandBufferT *commandBufferHelper,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const gl::InterfaceBlock &block,
    uint32_t bufferIndex,
    VkDescriptorType descriptorType,
    VkDeviceSize maxBoundBufferRange,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs,
    const GLbitfield memoryBarrierBits)
{
    if (block.activeShaders().none())
    {
        return;
    }

    const gl::ShaderType firstShaderType = block.getFirstActiveShaderType();
    const ShaderInterfaceVariableInfo &info =
        variableInfoMap.getVariableById(firstShaderType, block.getId(firstShaderType));

    uint32_t binding       = info.binding;
    uint32_t arrayElement  = block.pod.isArray ? block.pod.arrayElement : 0;
    uint32_t infoDescIndex = writeDescriptorDescs[binding].descriptorInfoIndex + arrayElement;

    const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = buffers[bufferIndex];
    if (bufferBinding.get() == nullptr)
    {
        setEmptyBuffer(infoDescIndex, descriptorType, emptyBuffer);
        return;
    }

    // Limit bound buffer size to maximum resource binding size.
    GLsizeiptr boundBufferSize = gl::GetBoundBufferAvailableSize(bufferBinding);
    VkDeviceSize size          = std::min<VkDeviceSize>(boundBufferSize, maxBoundBufferRange);

    // Make sure there's no possible under/overflow with binding size.
    static_assert(sizeof(VkDeviceSize) >= sizeof(bufferBinding.getSize()),
                  "VkDeviceSize too small");
    ASSERT(bufferBinding.getSize() >= 0);

    BufferVk *bufferVk         = vk::GetImpl(bufferBinding.get());
    BufferHelper &bufferHelper = bufferVk->getBuffer();

    const bool isUniformBuffer = descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                                 descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    if (isUniformBuffer)
    {
        commandBufferHelper->bufferRead(context, VK_ACCESS_UNIFORM_READ_BIT, block.activeShaders(),
                                        &bufferHelper);
    }
    else
    {
        ASSERT(descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
               descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
        if (block.pod.isReadOnly)
        {
            // Avoid unnecessary barriers for readonly SSBOs by making sure the buffers are
            // marked read-only.  This also helps BufferVk make better decisions during
            // buffer data uploads and copies by knowing that the buffers are not actually
            // being written to.
            commandBufferHelper->bufferRead(context, VK_ACCESS_SHADER_READ_BIT,
                                            block.activeShaders(), &bufferHelper);
        }
        else if ((bufferHelper.getCurrentWriteAccess() & VK_ACCESS_SHADER_WRITE_BIT) != 0 &&
                 (memoryBarrierBits & kBufferMemoryBarrierBits) == 0)
        {
            // Buffer is already in shader write access, and this is not from memoryBarrier call,
            // then skip the WAW barrier since GL spec says driver is not required to insert barrier
            // here. We still need to maintain object life time tracking here.
            // Based on discussion here https://gitlab.khronos.org/opengl/API/-/issues/144, the
            // above check of VK_ACCESS_SHADER_WRITE_BIT bit can be removed and instead rely on app
            // issue glMemoryBarrier. But almost all usage I am seeing does not issue
            // glMemoryBarrier before SSBO write. They only issue glMemoryBarrier after the SSBO
            // write. This is to ensure we do not break the existing usage even if we think they are
            // out of spec.
            commandBufferHelper->retainResourceForWrite(&bufferHelper);
        }
        else
        {
            // We set the SHADER_READ_BIT to be conservative.
            VkAccessFlags accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            commandBufferHelper->bufferWrite(context, accessFlags, block.activeShaders(),
                                             &bufferHelper);
        }
    }

    VkDeviceSize offset = bufferBinding.getOffset() + bufferHelper.getOffset();

    DescriptorInfoDesc &infoDesc   = mDesc.getInfoDesc(infoDescIndex);
    infoDesc.samplerOrBufferSerial = bufferHelper.getBlockSerial().getValue();
    if (IsDynamicDescriptor(descriptorType))
    {
        SetBitField(mDynamicOffsets[infoDescIndex], offset);
        infoDesc.imageViewSerialOrOffset = 0;
    }
    else
    {
        SetBitField(infoDesc.imageViewSerialOrOffset, offset);
    }
    SetBitField(infoDesc.imageLayoutOrRange, size);
    infoDesc.imageSubresourceRange = 0;

    mHandles[infoDescIndex].buffer = bufferHelper.getBuffer().getHandle();
}

template <typename CommandBufferT>
void DescriptorSetDescBuilder::updateShaderBuffers(
    Context *context,
    CommandBufferT *commandBufferHelper,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const std::vector<gl::InterfaceBlock> &blocks,
    VkDescriptorType descriptorType,
    VkDeviceSize maxBoundBufferRange,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs,
    const GLbitfield memoryBarrierBits)
{
    const bool isUniformBuffer = descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                                 descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    // Now that we have the proper array elements counts, initialize the info structures.
    for (uint32_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex)
    {
        const GLuint binding = isUniformBuffer
                                   ? executable.getUniformBlockBinding(blockIndex)
                                   : executable.getShaderStorageBlockBinding(blockIndex);
        updateOneShaderBuffer(context, commandBufferHelper, variableInfoMap, buffers,
                              blocks[blockIndex], binding, descriptorType, maxBoundBufferRange,
                              emptyBuffer, writeDescriptorDescs, memoryBarrierBits);
    }
}

template <typename CommandBufferT>
void DescriptorSetDescBuilder::updateAtomicCounters(
    Context *context,
    CommandBufferT *commandBufferHelper,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers,
    const VkDeviceSize requiredOffsetAlignment,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs)
{
    ASSERT(!atomicCounterBuffers.empty());
    static_assert(!IsDynamicDescriptor(kStorageBufferDescriptorType),
                  "This method needs an update to handle dynamic descriptors");

    if (atomicCounterBuffers.empty())
    {
        return;
    }

    uint32_t binding = variableInfoMap.getAtomicCounterBufferBinding(
        atomicCounterBuffers[0].getFirstActiveShaderType(), 0);
    uint32_t baseInfoIndex = writeDescriptorDescs[binding].descriptorInfoIndex;

    // Bind the empty buffer to every array slot that's unused.
    for (uint32_t arrayElement = 0;
         arrayElement < gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS; ++arrayElement)
    {
        uint32_t infoIndex = baseInfoIndex + arrayElement;
        setEmptyBuffer(infoIndex, kStorageBufferDescriptorType, emptyBuffer);
    }

    for (uint32_t bufferIndex = 0; bufferIndex < atomicCounterBuffers.size(); ++bufferIndex)
    {
        const gl::AtomicCounterBuffer &atomicCounterBuffer = atomicCounterBuffers[bufferIndex];
        const GLuint arrayElement = executable.getAtomicCounterBufferBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = buffers[arrayElement];

        uint32_t infoIndex = baseInfoIndex + arrayElement;

        if (bufferBinding.get() == nullptr)
        {
            setEmptyBuffer(infoIndex, kStorageBufferDescriptorType, emptyBuffer);
            continue;
        }

        BufferVk *bufferVk             = vk::GetImpl(bufferBinding.get());
        vk::BufferHelper &bufferHelper = bufferVk->getBuffer();

        VkAccessFlags accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        commandBufferHelper->bufferWrite(context, accessFlags, atomicCounterBuffer.activeShaders(),
                                         &bufferHelper);

        VkDeviceSize offset = bufferBinding.getOffset() + bufferHelper.getOffset();

        VkDeviceSize alignedOffset = (offset / requiredOffsetAlignment) * requiredOffsetAlignment;
        VkDeviceSize offsetDiff    = offset - alignedOffset;

        offset = alignedOffset;

        VkDeviceSize range = gl::GetBoundBufferAvailableSize(bufferBinding) + offsetDiff;

        DescriptorInfoDesc &infoDesc = mDesc.getInfoDesc(infoIndex);
        SetBitField(infoDesc.imageLayoutOrRange, range);
        SetBitField(infoDesc.imageViewSerialOrOffset, offset);
        infoDesc.samplerOrBufferSerial = bufferHelper.getBlockSerial().getValue();
        infoDesc.imageSubresourceRange = 0;

        mHandles[infoIndex].buffer = bufferHelper.getBuffer().getHandle();
    }
}

// Explicit instantiation
template void DescriptorSetDescBuilder::updateOneShaderBuffer<vk::RenderPassCommandBufferHelper>(
    Context *context,
    RenderPassCommandBufferHelper *commandBufferHelper,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const gl::InterfaceBlock &block,
    uint32_t bufferIndex,
    VkDescriptorType descriptorType,
    VkDeviceSize maxBoundBufferRange,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs,
    const GLbitfield memoryBarrierBits);

template void DescriptorSetDescBuilder::updateOneShaderBuffer<OutsideRenderPassCommandBufferHelper>(
    Context *context,
    OutsideRenderPassCommandBufferHelper *commandBufferHelper,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const gl::InterfaceBlock &block,
    uint32_t bufferIndex,
    VkDescriptorType descriptorType,
    VkDeviceSize maxBoundBufferRange,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs,
    const GLbitfield memoryBarrierBits);

template void DescriptorSetDescBuilder::updateShaderBuffers<OutsideRenderPassCommandBufferHelper>(
    Context *context,
    OutsideRenderPassCommandBufferHelper *commandBufferHelper,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const std::vector<gl::InterfaceBlock> &blocks,
    VkDescriptorType descriptorType,
    VkDeviceSize maxBoundBufferRange,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs,
    const GLbitfield memoryBarrierBits);

template void DescriptorSetDescBuilder::updateShaderBuffers<RenderPassCommandBufferHelper>(
    Context *context,
    RenderPassCommandBufferHelper *commandBufferHelper,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const std::vector<gl::InterfaceBlock> &blocks,
    VkDescriptorType descriptorType,
    VkDeviceSize maxBoundBufferRange,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs,
    const GLbitfield memoryBarrierBits);

template void DescriptorSetDescBuilder::updateAtomicCounters<OutsideRenderPassCommandBufferHelper>(
    Context *context,
    OutsideRenderPassCommandBufferHelper *commandBufferHelper,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers,
    const VkDeviceSize requiredOffsetAlignment,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs);

template void DescriptorSetDescBuilder::updateAtomicCounters<RenderPassCommandBufferHelper>(
    Context *context,
    RenderPassCommandBufferHelper *commandBufferHelper,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::BufferVector &buffers,
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers,
    const VkDeviceSize requiredOffsetAlignment,
    const BufferHelper &emptyBuffer,
    const WriteDescriptorDescs &writeDescriptorDescs);

angle::Result DescriptorSetDescBuilder::updateImages(
    Context *context,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    const gl::ActiveTextureArray<TextureVk *> &activeImages,
    const std::vector<gl::ImageUnit> &imageUnits,
    const WriteDescriptorDescs &writeDescriptorDescs)
{
    Renderer *renderer                                 = context->getRenderer();
    const std::vector<gl::ImageBinding> &imageBindings = executable.getImageBindings();
    const std::vector<gl::LinkedUniform> &uniforms     = executable.getUniforms();

    if (imageBindings.empty())
    {
        return angle::Result::Continue;
    }

    for (uint32_t imageIndex = 0; imageIndex < imageBindings.size(); ++imageIndex)
    {
        const gl::ImageBinding &imageBinding = imageBindings[imageIndex];
        uint32_t uniformIndex                = executable.getUniformIndexFromImageIndex(imageIndex);
        const gl::LinkedUniform &imageUniform = uniforms[uniformIndex];

        if (imageUniform.activeShaders().none())
        {
            continue;
        }

        const gl::ShaderType firstShaderType = imageUniform.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            variableInfoMap.getVariableById(firstShaderType, imageUniform.getId(firstShaderType));

        uint32_t arraySize = static_cast<uint32_t>(imageBinding.boundImageUnits.size());

        // Texture buffers use buffer views, so they are especially handled.
        if (imageBinding.textureType == gl::TextureType::Buffer)
        {
            // Handle format reinterpretation by looking for a view with the format specified in
            // the shader (if any, instead of the format specified to glTexBuffer).
            const vk::Format *format = nullptr;
            if (imageUniform.getImageUnitFormat() != GL_NONE)
            {
                format = &renderer->getFormat(imageUniform.getImageUnitFormat());
            }

            for (uint32_t arrayElement = 0; arrayElement < arraySize; ++arrayElement)
            {
                GLuint imageUnit     = imageBinding.boundImageUnits[arrayElement];
                TextureVk *textureVk = activeImages[imageUnit];

                uint32_t infoIndex = writeDescriptorDescs[info.binding].descriptorInfoIndex +
                                     arrayElement + imageUniform.getOuterArrayOffset();

                const vk::BufferView *view = nullptr;
                ANGLE_TRY(textureVk->getBufferView(context, format, nullptr, true, &view));

                DescriptorInfoDesc &infoDesc = mDesc.getInfoDesc(infoIndex);
                infoDesc.imageViewSerialOrOffset =
                    textureVk->getBufferViewSerial().viewSerial.getValue();
                infoDesc.imageLayoutOrRange    = 0;
                infoDesc.imageSubresourceRange = 0;
                infoDesc.samplerOrBufferSerial = 0;

                mHandles[infoIndex].bufferView = view->getHandle();
            }
        }
        else
        {
            for (uint32_t arrayElement = 0; arrayElement < arraySize; ++arrayElement)
            {
                GLuint imageUnit             = imageBinding.boundImageUnits[arrayElement];
                const gl::ImageUnit &binding = imageUnits[imageUnit];
                TextureVk *textureVk         = activeImages[imageUnit];

                vk::ImageHelper *image         = &textureVk->getImage();
                const vk::ImageView *imageView = nullptr;

                vk::ImageOrBufferViewSubresourceSerial serial =
                    textureVk->getStorageImageViewSerial(binding);

                ANGLE_TRY(textureVk->getStorageImageView(context, binding, &imageView));

                uint32_t infoIndex = writeDescriptorDescs[info.binding].descriptorInfoIndex +
                                     arrayElement + imageUniform.getOuterArrayOffset();

                // Note: binding.access is unused because it is implied by the shader.

                DescriptorInfoDesc &infoDesc = mDesc.getInfoDesc(infoIndex);
                SetBitField(infoDesc.imageLayoutOrRange, image->getCurrentImageLayout());
                memcpy(&infoDesc.imageSubresourceRange, &serial.subresource, sizeof(uint32_t));
                infoDesc.imageViewSerialOrOffset = serial.viewSerial.getValue();
                infoDesc.samplerOrBufferSerial   = 0;

                mHandles[infoIndex].imageView = imageView->getHandle();
            }
        }
    }

    return angle::Result::Continue;
}

angle::Result DescriptorSetDescBuilder::updateInputAttachments(
    vk::Context *context,
    const gl::ProgramExecutable &executable,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    FramebufferVk *framebufferVk,
    const WriteDescriptorDescs &writeDescriptorDescs)
{
    // Note: Depth/stencil input attachments are only supported in ANGLE when using
    // VK_KHR_dynamic_rendering_local_read, so the layout is chosen to be the one specifically made
    // for that extension.
    if (executable.usesDepthFramebufferFetch() || executable.usesStencilFramebufferFetch())
    {
        RenderTargetVk *renderTargetVk = framebufferVk->getDepthStencilRenderTarget();
        ASSERT(context->getFeatures().preferDynamicRendering.enabled);

        if (renderTargetVk != nullptr)
        {
            const ImageOrBufferViewSubresourceSerial serial =
                renderTargetVk->getDrawSubresourceSerial();
            const VkImageAspectFlags aspects =
                renderTargetVk->getImageForRenderPass().getAspectFlags();

            if (executable.usesDepthFramebufferFetch() &&
                (aspects & VK_IMAGE_ASPECT_DEPTH_BIT) != 0)
            {
                const vk::ImageView *imageView = nullptr;
                ANGLE_TRY(renderTargetVk->getDepthOrStencilImageView(
                    context, VK_IMAGE_ASPECT_DEPTH_BIT, &imageView));

                const uint32_t depthBinding =
                    variableInfoMap
                        .getVariableById(gl::ShaderType::Fragment,
                                         sh::vk::spirv::kIdDepthInputAttachment)
                        .binding;
                updateInputAttachment(context, depthBinding, ImageLayout::DepthStencilWriteAndInput,
                                      imageView, serial, writeDescriptorDescs);
            }

            if (executable.usesStencilFramebufferFetch() &&
                (aspects & VK_IMAGE_ASPECT_STENCIL_BIT) != 0)
            {
                const vk::ImageView *imageView = nullptr;
                ANGLE_TRY(renderTargetVk->getDepthOrStencilImageView(
                    context, VK_IMAGE_ASPECT_STENCIL_BIT, &imageView));

                const uint32_t stencilBinding =
                    variableInfoMap
                        .getVariableById(gl::ShaderType::Fragment,
                                         sh::vk::spirv::kIdStencilInputAttachment)
                        .binding;
                updateInputAttachment(context, stencilBinding,
                                      ImageLayout::DepthStencilWriteAndInput, imageView, serial,
                                      writeDescriptorDescs);
            }
        }
    }

    if (!executable.usesColorFramebufferFetch())
    {
        return angle::Result::Continue;
    }

    const uint32_t firstColorInputAttachment =
        static_cast<uint32_t>(executable.getFragmentInoutIndices().first());

    const ShaderInterfaceVariableInfo &baseColorInfo = variableInfoMap.getVariableById(
        gl::ShaderType::Fragment, sh::vk::spirv::kIdInputAttachment0 + firstColorInputAttachment);

    const uint32_t baseColorBinding = baseColorInfo.binding - firstColorInputAttachment;

    for (size_t colorIndex : framebufferVk->getState().getColorAttachmentsMask())
    {
        uint32_t binding               = baseColorBinding + static_cast<uint32_t>(colorIndex);
        RenderTargetVk *renderTargetVk = framebufferVk->getColorDrawRenderTarget(colorIndex);

        const vk::ImageView *imageView = nullptr;
        ANGLE_TRY(renderTargetVk->getImageView(context, &imageView));
        const ImageOrBufferViewSubresourceSerial serial =
            renderTargetVk->getDrawSubresourceSerial();

        // We just need any layout that represents GENERAL for render pass objects.  With dynamic
        // rendering, there's a specific layout.
        updateInputAttachment(context, binding,
                              context->getFeatures().preferDynamicRendering.enabled
                                  ? ImageLayout::ColorWriteAndInput
                                  : ImageLayout::FragmentShaderWrite,
                              imageView, serial, writeDescriptorDescs);
    }

    return angle::Result::Continue;
}

void DescriptorSetDescBuilder::updateInputAttachment(
    Context *context,
    uint32_t binding,
    ImageLayout layout,
    const vk::ImageView *imageView,
    ImageOrBufferViewSubresourceSerial serial,
    const WriteDescriptorDescs &writeDescriptorDescs)
{
    uint32_t infoIndex = writeDescriptorDescs[binding].descriptorInfoIndex;

    DescriptorInfoDesc &infoDesc = mDesc.getInfoDesc(infoIndex);

    // The serial is not totally precise.
    SetBitField(infoDesc.imageLayoutOrRange, layout);
    infoDesc.imageViewSerialOrOffset = serial.viewSerial.getValue();
    memcpy(&infoDesc.imageSubresourceRange, &serial.subresource, sizeof(uint32_t));
    infoDesc.samplerOrBufferSerial = 0;

    mHandles[infoIndex].imageView = imageView->getHandle();
}

void DescriptorSetDescBuilder::updateDescriptorSet(Renderer *renderer,
                                                   const WriteDescriptorDescs &writeDescriptorDescs,
                                                   UpdateDescriptorSetsBuilder *updateBuilder,
                                                   VkDescriptorSet descriptorSet) const
{
    mDesc.updateDescriptorSet(renderer, writeDescriptorDescs, updateBuilder, mHandles.data(),
                              descriptorSet);
}

// SharedCacheKeyManager implementation.
template <class SharedCacheKeyT>
size_t SharedCacheKeyManager<SharedCacheKeyT>::updateEmptySlotBits()
{
    ASSERT(mSharedCacheKeys.size() == mEmptySlotBits.size() * kSlotBitCount);
    size_t emptySlot = kInvalidSlot;
    for (size_t slot = 0; slot < mSharedCacheKeys.size(); ++slot)
    {
        SharedCacheKeyT &sharedCacheKey = mSharedCacheKeys[slot];
        if (!sharedCacheKey->valid())
        {
            mEmptySlotBits[slot / kSlotBitCount].set(slot % kSlotBitCount);
            emptySlot = slot;
        }
    }
    return emptySlot;
}

template <class SharedCacheKeyT>
void SharedCacheKeyManager<SharedCacheKeyT>::addKey(const SharedCacheKeyT &key)
{
    // Search for available slots and use that if any
    size_t slot = 0;
    for (SlotBitMask &emptyBits : mEmptySlotBits)
    {
        if (emptyBits.any())
        {
            slot += emptyBits.first();
            SharedCacheKeyT &sharedCacheKey = mSharedCacheKeys[slot];
            ASSERT(!sharedCacheKey->valid());
            sharedCacheKey = key;
            emptyBits.reset(slot % kSlotBitCount);
            return;
        }
        slot += kSlotBitCount;
    }

    // Some cached entries may have been released. Try to update and use any available slot if any.
    slot = updateEmptySlotBits();
    if (slot != kInvalidSlot)
    {
        SharedCacheKeyT &sharedCacheKey = mSharedCacheKeys[slot];
        ASSERT(!sharedCacheKey->valid());
        sharedCacheKey         = key;
        SlotBitMask &emptyBits = mEmptySlotBits[slot / kSlotBitCount];
        emptyBits.reset(slot % kSlotBitCount);
        return;
    }

    // No slot available, expand mSharedCacheKeys
    ASSERT(mSharedCacheKeys.size() == mEmptySlotBits.size() * kSlotBitCount);
    if (!mEmptySlotBits.empty())
    {
        // On first insertion, let std::vector allocate a single entry for minimal memory overhead,
        // since this is the most common usage case. If that exceeds, reserve a larger chunk to
        // avoid storage reallocation for efficiency (enough storage enough for 512 cache entries).
        mEmptySlotBits.reserve(8);
    }
    mEmptySlotBits.emplace_back(0xFFFFFFFE);
    mSharedCacheKeys.emplace_back(key);
    while (mSharedCacheKeys.size() < mEmptySlotBits.size() * kSlotBitCount)
    {
        mSharedCacheKeys.emplace_back();
        SharedCacheKeyT &sharedCacheKey = mSharedCacheKeys.back();
        // Insert an empty cache key so that sharedCacheKey will not be null.
        MakeInvalidCachedObject(&sharedCacheKey);
        ASSERT(!sharedCacheKey->valid());
    }
}

template <class SharedCacheKeyT>
void SharedCacheKeyManager<SharedCacheKeyT>::releaseKeys(ContextVk *contextVk)
{
    ASSERT(mSharedCacheKeys.size() == mEmptySlotBits.size() * kSlotBitCount);
    for (SharedCacheKeyT &sharedCacheKey : mSharedCacheKeys)
    {
        if (sharedCacheKey->valid())
        {
            // Immediate destroy the cached object and the key itself when first releaseRef call is
            // made
            sharedCacheKey->releaseCachedObject(contextVk);
        }
    }
    mSharedCacheKeys.clear();
    mEmptySlotBits.clear();
}

template <class SharedCacheKeyT>
void SharedCacheKeyManager<SharedCacheKeyT>::releaseKeys(Renderer *renderer)
{
    ASSERT(mSharedCacheKeys.size() == mEmptySlotBits.size() * kSlotBitCount);
    for (SharedCacheKeyT &sharedCacheKey : mSharedCacheKeys)
    {
        if (sharedCacheKey->valid())
        {
            // Immediate destroy the cached object and the key itself when first releaseKeys call is
            // made
            sharedCacheKey->releaseCachedObject(renderer);
        }
    }
    mSharedCacheKeys.clear();
    mEmptySlotBits.clear();
}

template <class SharedCacheKeyT>
void SharedCacheKeyManager<SharedCacheKeyT>::destroyKeys(Renderer *renderer)
{
    ASSERT(mSharedCacheKeys.size() == mEmptySlotBits.size() * kSlotBitCount);
    for (SharedCacheKeyT &sharedCacheKey : mSharedCacheKeys)
    {
        // destroy the cache key
        if (sharedCacheKey->valid())
        {
            // Immediate destroy the cached object and the key
            sharedCacheKey->destroyCachedObject(renderer);
        }
    }
    mSharedCacheKeys.clear();
    mEmptySlotBits.clear();
}

template <class SharedCacheKeyT>
void SharedCacheKeyManager<SharedCacheKeyT>::clear()
{
    // Caller must have already freed all caches
    assertAllEntriesDestroyed();
    mSharedCacheKeys.clear();
    mEmptySlotBits.clear();
}

template <class SharedCacheKeyT>
bool SharedCacheKeyManager<SharedCacheKeyT>::containsKey(const SharedCacheKeyT &key) const
{
    for (const SharedCacheKeyT &sharedCacheKey : mSharedCacheKeys)
    {
        if (*key == *sharedCacheKey)
        {
            return true;
        }
    }
    return false;
}

template <class SharedCacheKeyT>
void SharedCacheKeyManager<SharedCacheKeyT>::assertAllEntriesDestroyed()
{
    // Caller must have already freed all caches
    for (SharedCacheKeyT &sharedCacheKey : mSharedCacheKeys)
    {
        ASSERT(!sharedCacheKey->valid());
    }
}

// Explict instantiate for FramebufferCacheManager
template class SharedCacheKeyManager<SharedFramebufferCacheKey>;
// Explict instantiate for DescriptorSetCacheManager
template class SharedCacheKeyManager<SharedDescriptorSetCacheKey>;

// PipelineCacheAccess implementation.
std::unique_lock<angle::SimpleMutex> PipelineCacheAccess::getLock()
{
    if (mMutex == nullptr)
    {
        return std::unique_lock<angle::SimpleMutex>();
    }

    return std::unique_lock<angle::SimpleMutex>(*mMutex);
}

VkResult PipelineCacheAccess::createGraphicsPipeline(vk::ErrorContext *context,
                                                     const VkGraphicsPipelineCreateInfo &createInfo,
                                                     vk::Pipeline *pipelineOut)
{
    std::unique_lock<angle::SimpleMutex> lock = getLock();

    return pipelineOut->initGraphics(context->getDevice(), createInfo, *mPipelineCache);
}

VkResult PipelineCacheAccess::createComputePipeline(vk::ErrorContext *context,
                                                    const VkComputePipelineCreateInfo &createInfo,
                                                    vk::Pipeline *pipelineOut)
{
    std::unique_lock<angle::SimpleMutex> lock = getLock();

    return pipelineOut->initCompute(context->getDevice(), createInfo, *mPipelineCache);
}

VkResult PipelineCacheAccess::getCacheData(vk::ErrorContext *context,
                                           size_t *cacheSize,
                                           void *cacheData)
{
    std::unique_lock<angle::SimpleMutex> lock = getLock();
    return mPipelineCache->getCacheData(context->getDevice(), cacheSize, cacheData);
}

void PipelineCacheAccess::merge(Renderer *renderer, const vk::PipelineCache &pipelineCache)
{
    ASSERT(isThreadSafe());

    std::unique_lock<angle::SimpleMutex> lock = getLock();

    mPipelineCache->merge(renderer->getDevice(), 1, pipelineCache.ptr());
}
}  // namespace vk

// UpdateDescriptorSetsBuilder implementation.
UpdateDescriptorSetsBuilder::UpdateDescriptorSetsBuilder()
{
    // Reserve reasonable amount of spaces so that for majority of apps we don't need to grow at all
    constexpr size_t kDescriptorBufferInfosInitialSize = 8;
    constexpr size_t kDescriptorImageInfosInitialSize  = 4;
    constexpr size_t kDescriptorWriteInfosInitialSize =
        kDescriptorBufferInfosInitialSize + kDescriptorImageInfosInitialSize;
    constexpr size_t kDescriptorBufferViewsInitialSize = 0;

    mDescriptorBufferInfos.reserve(kDescriptorBufferInfosInitialSize);
    mDescriptorImageInfos.reserve(kDescriptorImageInfosInitialSize);
    mWriteDescriptorSets.reserve(kDescriptorWriteInfosInitialSize);
    mBufferViews.reserve(kDescriptorBufferViewsInitialSize);
}

UpdateDescriptorSetsBuilder::~UpdateDescriptorSetsBuilder() = default;

template <typename T, const T *VkWriteDescriptorSet::*pInfo>
void UpdateDescriptorSetsBuilder::growDescriptorCapacity(std::vector<T> *descriptorVector,
                                                         size_t newSize)
{
    const T *const oldInfoStart = descriptorVector->empty() ? nullptr : &(*descriptorVector)[0];
    size_t newCapacity          = std::max(descriptorVector->capacity() << 1, newSize);
    descriptorVector->reserve(newCapacity);

    if (oldInfoStart)
    {
        // patch mWriteInfo with new BufferInfo/ImageInfo pointers
        for (VkWriteDescriptorSet &set : mWriteDescriptorSets)
        {
            if (set.*pInfo)
            {
                size_t index = set.*pInfo - oldInfoStart;
                set.*pInfo   = &(*descriptorVector)[index];
            }
        }
    }
}

template <typename T, const T *VkWriteDescriptorSet::*pInfo>
T *UpdateDescriptorSetsBuilder::allocDescriptorInfos(std::vector<T> *descriptorVector, size_t count)
{
    size_t oldSize = descriptorVector->size();
    size_t newSize = oldSize + count;
    if (newSize > descriptorVector->capacity())
    {
        // If we have reached capacity, grow the storage and patch the descriptor set with new
        // buffer info pointer
        growDescriptorCapacity<T, pInfo>(descriptorVector, newSize);
    }
    descriptorVector->resize(newSize);
    return &(*descriptorVector)[oldSize];
}

VkDescriptorBufferInfo *UpdateDescriptorSetsBuilder::allocDescriptorBufferInfos(size_t count)
{
    return allocDescriptorInfos<VkDescriptorBufferInfo, &VkWriteDescriptorSet::pBufferInfo>(
        &mDescriptorBufferInfos, count);
}

VkDescriptorImageInfo *UpdateDescriptorSetsBuilder::allocDescriptorImageInfos(size_t count)
{
    return allocDescriptorInfos<VkDescriptorImageInfo, &VkWriteDescriptorSet::pImageInfo>(
        &mDescriptorImageInfos, count);
}

VkWriteDescriptorSet *UpdateDescriptorSetsBuilder::allocWriteDescriptorSets(size_t count)
{
    size_t oldSize = mWriteDescriptorSets.size();
    size_t newSize = oldSize + count;
    mWriteDescriptorSets.resize(newSize);
    return &mWriteDescriptorSets[oldSize];
}

VkBufferView *UpdateDescriptorSetsBuilder::allocBufferViews(size_t count)
{
    return allocDescriptorInfos<VkBufferView, &VkWriteDescriptorSet::pTexelBufferView>(
        &mBufferViews, count);
}

uint32_t UpdateDescriptorSetsBuilder::flushDescriptorSetUpdates(VkDevice device)
{
    if (mWriteDescriptorSets.empty())
    {
        ASSERT(mDescriptorBufferInfos.empty());
        ASSERT(mDescriptorImageInfos.empty());
        return 0;
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(mWriteDescriptorSets.size()),
                           mWriteDescriptorSets.data(), 0, nullptr);

    uint32_t retVal = static_cast<uint32_t>(mWriteDescriptorSets.size());

    mWriteDescriptorSets.clear();
    mDescriptorBufferInfos.clear();
    mDescriptorImageInfos.clear();
    mBufferViews.clear();

    return retVal;
}

// FramebufferCache implementation.
void FramebufferCache::destroy(vk::Renderer *renderer)
{
    renderer->accumulateCacheStats(VulkanCacheType::Framebuffer, mCacheStats);
    for (auto &entry : mPayload)
    {
        vk::FramebufferHelper &tmpFB = entry.second;
        tmpFB.destroy(renderer);
    }
    mPayload.clear();
}

bool FramebufferCache::get(ContextVk *contextVk,
                           const vk::FramebufferDesc &desc,
                           vk::Framebuffer &framebuffer)
{
    ASSERT(!contextVk->getFeatures().supportsImagelessFramebuffer.enabled);

    auto iter = mPayload.find(desc);
    if (iter != mPayload.end())
    {
        framebuffer.setHandle(iter->second.getFramebuffer().getHandle());
        mCacheStats.hit();
        return true;
    }

    mCacheStats.miss();
    return false;
}

void FramebufferCache::insert(ContextVk *contextVk,
                              const vk::FramebufferDesc &desc,
                              vk::FramebufferHelper &&framebufferHelper)
{
    ASSERT(!contextVk->getFeatures().supportsImagelessFramebuffer.enabled);

    mPayload.emplace(desc, std::move(framebufferHelper));
}

void FramebufferCache::erase(ContextVk *contextVk, const vk::FramebufferDesc &desc)
{
    ASSERT(!contextVk->getFeatures().supportsImagelessFramebuffer.enabled);

    auto iter = mPayload.find(desc);
    if (iter != mPayload.end())
    {
        vk::FramebufferHelper &tmpFB = iter->second;
        tmpFB.release(contextVk);
        mPayload.erase(desc);
    }
}

// RenderPassCache implementation.
RenderPassCache::RenderPassCache() = default;

RenderPassCache::~RenderPassCache()
{
    ASSERT(mPayload.empty());
}

void RenderPassCache::destroy(ContextVk *contextVk)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    renderer->accumulateCacheStats(VulkanCacheType::CompatibleRenderPass,
                                   mCompatibleRenderPassCacheStats);
    renderer->accumulateCacheStats(VulkanCacheType::RenderPassWithOps,
                                   mRenderPassWithOpsCacheStats);

    VkDevice device = renderer->getDevice();

    // Make sure there are no jobs referencing the render pass cache.
    contextVk->getShareGroup()->waitForCurrentMonolithicPipelineCreationTask();

    for (auto &outerIt : mPayload)
    {
        for (auto &innerIt : outerIt.second)
        {
            innerIt.second.destroy(device);
        }
    }
    mPayload.clear();
}

void RenderPassCache::clear(ContextVk *contextVk)
{
    // Make sure there are no jobs referencing the render pass cache.
    contextVk->getShareGroup()->waitForCurrentMonolithicPipelineCreationTask();

    for (auto &outerIt : mPayload)
    {
        for (auto &innerIt : outerIt.second)
        {
            innerIt.second.release(contextVk);
        }
    }
    mPayload.clear();
}

// static
void RenderPassCache::InitializeOpsForCompatibleRenderPass(const vk::RenderPassDesc &desc,
                                                           vk::AttachmentOpsArray *opsOut)
{
    // This API is only used by getCompatibleRenderPass() to create a compatible render pass.  The
    // following does not participate in render pass compatibility, so could take any value:
    //
    // - Load and store ops
    // - Attachment layouts
    // - Existance of resolve attachment (if single subpass)
    //
    // The values chosen here are arbitrary.

    vk::PackedAttachmentIndex colorIndexVk(0);
    for (uint32_t colorIndexGL = 0; colorIndexGL < desc.colorAttachmentRange(); ++colorIndexGL)
    {
        if (!desc.isColorAttachmentEnabled(colorIndexGL))
        {
            continue;
        }

        const vk::ImageLayout imageLayout = vk::ImageLayout::ColorWrite;
        opsOut->initWithLoadStore(colorIndexVk, imageLayout, imageLayout);
        ++colorIndexVk;
    }

    if (desc.hasDepthStencilAttachment())
    {
        const vk::ImageLayout imageLayout = vk::ImageLayout::DepthWriteStencilWrite;
        opsOut->initWithLoadStore(colorIndexVk, imageLayout, imageLayout);
    }
}

angle::Result RenderPassCache::addCompatibleRenderPass(ContextVk *contextVk,
                                                       const vk::RenderPassDesc &desc,
                                                       const vk::RenderPass **renderPassOut)
{
    vk::AttachmentOpsArray ops;
    InitializeOpsForCompatibleRenderPass(desc, &ops);

    return getRenderPassWithOpsImpl(contextVk, desc, ops, false, renderPassOut);
}

angle::Result RenderPassCache::getRenderPassWithOps(ContextVk *contextVk,
                                                    const vk::RenderPassDesc &desc,
                                                    const vk::AttachmentOpsArray &attachmentOps,
                                                    const vk::RenderPass **renderPassOut)
{
    return getRenderPassWithOpsImpl(contextVk, desc, attachmentOps, true, renderPassOut);
}

angle::Result RenderPassCache::getRenderPassWithOpsImpl(ContextVk *contextVk,
                                                        const vk::RenderPassDesc &desc,
                                                        const vk::AttachmentOpsArray &attachmentOps,
                                                        bool updatePerfCounters,
                                                        const vk::RenderPass **renderPassOut)
{
    ASSERT(!contextVk->getFeatures().preferDynamicRendering.enabled);

    auto outerIt = mPayload.find(desc);
    if (outerIt != mPayload.end())
    {
        InnerCache &innerCache = outerIt->second;

        auto innerIt = innerCache.find(attachmentOps);
        if (innerIt != innerCache.end())
        {
            // TODO(jmadill): Could possibly use an MRU cache here.
            vk::GetRenderPassAndUpdateCounters(contextVk, updatePerfCounters, &innerIt->second,
                                               renderPassOut);
            mRenderPassWithOpsCacheStats.hit();
            return angle::Result::Continue;
        }
    }
    else
    {
        auto emplaceResult = mPayload.emplace(desc, InnerCache());
        outerIt            = emplaceResult.first;
    }

    mRenderPassWithOpsCacheStats.missAndIncrementSize();
    vk::RenderPassHelper newRenderPass;
    ANGLE_TRY(MakeRenderPass(contextVk, desc, attachmentOps, &newRenderPass.getRenderPass(),
                             &newRenderPass.getPerfCounters()));

    InnerCache &innerCache = outerIt->second;
    auto insertPos         = innerCache.emplace(attachmentOps, std::move(newRenderPass));
    vk::GetRenderPassAndUpdateCounters(contextVk, updatePerfCounters, &insertPos.first->second,
                                       renderPassOut);

    // TODO(jmadill): Trim cache, and pre-populate with the most common RPs on startup.
    return angle::Result::Continue;
}

// static
angle::Result RenderPassCache::MakeRenderPass(vk::ErrorContext *context,
                                              const vk::RenderPassDesc &desc,
                                              const vk::AttachmentOpsArray &ops,
                                              vk::RenderPass *renderPass,
                                              vk::RenderPassPerfCounters *renderPassCounters)
{
    ASSERT(!context->getFeatures().preferDynamicRendering.enabled);

    vk::Renderer *renderer                             = context->getRenderer();
    constexpr VkAttachmentReference2 kUnusedAttachment = {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                                                          nullptr, VK_ATTACHMENT_UNUSED,
                                                          VK_IMAGE_LAYOUT_UNDEFINED, 0};

    ASSERT(!desc.hasDepthStencilFramebufferFetch());
    const bool needInputAttachments = desc.hasColorFramebufferFetch();
    const bool isRenderToTextureThroughExtension =
        desc.isRenderToTexture() &&
        renderer->getFeatures().supportsMultisampledRenderToSingleSampled.enabled;
    const bool isRenderToTextureThroughEmulation =
        desc.isRenderToTexture() && !isRenderToTextureThroughExtension;

    const uint8_t descSamples            = desc.samples();
    const uint8_t attachmentSamples      = isRenderToTextureThroughExtension ? 1 : descSamples;
    const uint8_t renderToTextureSamples = isRenderToTextureThroughExtension ? descSamples : 1;

    // Unpack the packed and split representation into the format required by Vulkan.
    gl::DrawBuffersVector<VkAttachmentReference2> colorAttachmentRefs;
    gl::DrawBuffersVector<VkAttachmentReference2> colorResolveAttachmentRefs;
    VkAttachmentReference2 depthStencilAttachmentRef        = kUnusedAttachment;
    VkAttachmentReference2 depthStencilResolveAttachmentRef = kUnusedAttachment;
    VkAttachmentReference2 fragmentShadingRateAttachmentRef = kUnusedAttachment;

    // The list of attachments includes all non-resolve and resolve attachments.
    vk::FramebufferAttachmentArray<VkAttachmentDescription2> attachmentDescs;

    // Track invalidated attachments so their resolve attachments can be invalidated as well.
    // Resolve attachments can be removed in that case if the render pass has only one subpass
    // (which is the case if there are no unresolve attachments).
    gl::DrawBufferMask isMSRTTEmulationColorInvalidated;
    bool isMSRTTEmulationDepthInvalidated   = false;
    bool isMSRTTEmulationStencilInvalidated = false;
    const bool hasUnresolveAttachments =
        desc.getColorUnresolveAttachmentMask().any() || desc.hasDepthStencilUnresolveAttachment();
    const bool canRemoveResolveAttachments =
        isRenderToTextureThroughEmulation && !hasUnresolveAttachments;

#if defined(ANGLE_PLATFORM_ANDROID)
    // if yuv, we're going to chain this on to some VkAttachmentDescription2
    VkExternalFormatANDROID externalFormat = {VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID, nullptr,
                                              0};
#endif

    gl::DrawBuffersArray<vk::ImageLayout> colorResolveImageLayout = {};

    // Pack color attachments
    vk::PackedAttachmentIndex attachmentCount(0);
    for (uint32_t colorIndexGL = 0; colorIndexGL < desc.colorAttachmentRange(); ++colorIndexGL)
    {
        // Vulkan says:
        //
        // > Each element of the pColorAttachments array corresponds to an output location in the
        // > shader, i.e. if the shader declares an output variable decorated with a Location value
        // > of X, then it uses the attachment provided in pColorAttachments[X].
        //
        // This means that colorAttachmentRefs is indexed by colorIndexGL.  Where the color
        // attachment is disabled, a reference with VK_ATTACHMENT_UNUSED is given.

        if (!desc.isColorAttachmentEnabled(colorIndexGL))
        {
            colorAttachmentRefs.push_back(kUnusedAttachment);
            continue;
        }

        angle::FormatID attachmentFormatID = desc[colorIndexGL];
        ASSERT(attachmentFormatID != angle::FormatID::NONE);

        bool isYUVExternalFormat = vk::IsYUVExternalFormat(attachmentFormatID);
        if (isYUVExternalFormat && renderer->nullColorAttachmentWithExternalFormatResolve())
        {
            colorAttachmentRefs.push_back(kUnusedAttachment);
            // temporary workaround for ARM driver assertion. Will remove once driver fix lands
            colorAttachmentRefs.back().layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRefs.back().aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            continue;
        }

        VkAttachmentReference2 colorRef = {};
        colorRef.sType                  = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        colorRef.attachment             = attachmentCount.get();
        colorRef.layout =
            needInputAttachments
                ? VK_IMAGE_LAYOUT_GENERAL
                : vk::ConvertImageLayoutToVkImageLayout(
                      renderer, static_cast<vk::ImageLayout>(ops[attachmentCount].initialLayout));
        colorRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentRefs.push_back(colorRef);

        vk::UnpackAttachmentDesc(renderer, &attachmentDescs[attachmentCount.get()],
                                 attachmentFormatID, attachmentSamples, ops[attachmentCount]);
        colorResolveImageLayout[colorIndexGL] =
            static_cast<vk::ImageLayout>(ops[attachmentCount].finalResolveLayout);

        if (isYUVExternalFormat)
        {
            const vk::ExternalYuvFormatInfo &externalFormatInfo =
                renderer->getExternalFormatTable()->getExternalFormatInfo(attachmentFormatID);
            attachmentDescs[attachmentCount.get()].format =
                externalFormatInfo.colorAttachmentFormat;
        }
        else
        {
            attachmentDescs[attachmentCount.get()].format =
                vk::GetVkFormatFromFormatID(renderer, attachmentFormatID);
        }
        ASSERT(attachmentDescs[attachmentCount.get()].format != VK_FORMAT_UNDEFINED);

        // When multisampled-render-to-texture is used, invalidating an attachment invalidates both
        // the multisampled and the resolve attachments.  Otherwise, the resolve attachment is
        // independent of the multisampled attachment, and is never invalidated.
        // This is also the case for external format resolve
        if (isRenderToTextureThroughEmulation)
        {
            isMSRTTEmulationColorInvalidated.set(colorIndexGL, ops[attachmentCount].isInvalidated);
        }

        ++attachmentCount;
    }

    // Pack depth/stencil attachment, if any
    if (desc.hasDepthStencilAttachment())
    {
        uint32_t depthStencilIndexGL = static_cast<uint32_t>(desc.depthStencilAttachmentIndex());

        angle::FormatID attachmentFormatID = desc[depthStencilIndexGL];
        ASSERT(attachmentFormatID != angle::FormatID::NONE);

        depthStencilAttachmentRef.sType      = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        depthStencilAttachmentRef.attachment = attachmentCount.get();
        depthStencilAttachmentRef.layout     = ConvertImageLayoutToVkImageLayout(
            renderer, static_cast<vk::ImageLayout>(ops[attachmentCount].initialLayout));
        depthStencilAttachmentRef.aspectMask =
            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

        vk::UnpackAttachmentDesc(renderer, &attachmentDescs[attachmentCount.get()],
                                 attachmentFormatID, attachmentSamples, ops[attachmentCount]);

        if (isRenderToTextureThroughEmulation)
        {
            isMSRTTEmulationDepthInvalidated   = ops[attachmentCount].isInvalidated;
            isMSRTTEmulationStencilInvalidated = ops[attachmentCount].isStencilInvalidated;
        }

        ++attachmentCount;
    }

    // Pack fragment shading rate attachment, if any
    if (desc.hasFragmentShadingAttachment())
    {
        vk::UnpackFragmentShadingRateAttachmentDesc(&attachmentDescs[attachmentCount.get()]);

        fragmentShadingRateAttachmentRef.sType      = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        fragmentShadingRateAttachmentRef.attachment = attachmentCount.get();
        fragmentShadingRateAttachmentRef.layout =
            VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;

        ++attachmentCount;
    }

    // Pack color resolve attachments
    const uint32_t nonResolveAttachmentCount = attachmentCount.get();
    for (uint32_t colorIndexGL = 0; colorIndexGL < desc.colorAttachmentRange(); ++colorIndexGL)
    {
        if (!desc.hasColorResolveAttachment(colorIndexGL))
        {
            colorResolveAttachmentRefs.push_back(kUnusedAttachment);
            continue;
        }

        ASSERT(desc.isColorAttachmentEnabled(colorIndexGL));

        angle::FormatID attachmentFormatID = desc[colorIndexGL];
        bool isYUVExternalFormat           = vk::IsYUVExternalFormat(attachmentFormatID);

        VkAttachmentReference2 colorRef = {};
        colorRef.sType                  = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        colorRef.attachment             = attachmentCount.get();
        colorRef.layout                 = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorRef.aspectMask             = VK_IMAGE_ASPECT_COLOR_BIT;

        // If color attachment is invalidated, try to remove its resolve attachment altogether.
        if (canRemoveResolveAttachments && isMSRTTEmulationColorInvalidated.test(colorIndexGL))
        {
            colorResolveAttachmentRefs.push_back(kUnusedAttachment);
        }
        else
        {
            colorResolveAttachmentRefs.push_back(colorRef);
        }

        const bool isInvalidated = isMSRTTEmulationColorInvalidated.test(colorIndexGL);

        if (isYUVExternalFormat && renderer->nullColorAttachmentWithExternalFormatResolve())
        {
            vk::UnpackAttachmentDesc(renderer, &attachmentDescs[attachmentCount.get()],
                                     attachmentFormatID, attachmentSamples, ops[attachmentCount]);
        }
        else
        {
            vk::UnpackColorResolveAttachmentDesc(
                renderer, &attachmentDescs[attachmentCount.get()], attachmentFormatID,
                {desc.hasColorUnresolveAttachment(colorIndexGL), isInvalidated, false},
                colorResolveImageLayout[colorIndexGL]);
        }

#if defined(ANGLE_PLATFORM_ANDROID)
        // For rendering to YUV, chain on the external format info to the resolve attachment
        if (isYUVExternalFormat)
        {
            const vk::ExternalYuvFormatInfo &externalFormatInfo =
                renderer->getExternalFormatTable()->getExternalFormatInfo(attachmentFormatID);
            externalFormat.externalFormat        = externalFormatInfo.externalFormat;
            VkAttachmentDescription2 &attachment = attachmentDescs[attachmentCount.get()];
            attachment.pNext                     = &externalFormat;
            ASSERT(attachment.format == VK_FORMAT_UNDEFINED);
        }
#endif

        ++attachmentCount;
    }

    // Pack depth/stencil resolve attachment, if any
    if (desc.hasDepthStencilResolveAttachment())
    {
        ASSERT(desc.hasDepthStencilAttachment());

        uint32_t depthStencilIndexGL = static_cast<uint32_t>(desc.depthStencilAttachmentIndex());

        angle::FormatID attachmentFormatID = desc[depthStencilIndexGL];
        const angle::Format &angleFormat   = angle::Format::Get(attachmentFormatID);

        bool isDepthUnused   = false;
        bool isStencilUnused = false;

        // Treat missing aspect as invalidated for the purpose of the resolve attachment.
        if (angleFormat.depthBits == 0)
        {
            isMSRTTEmulationDepthInvalidated = true;
        }
        else if (!desc.hasDepthResolveAttachment())
        {
            isDepthUnused = true;
        }
        if (angleFormat.stencilBits == 0)
        {
            isMSRTTEmulationStencilInvalidated = true;
        }
        else if (!desc.hasStencilResolveAttachment())
        {
            isStencilUnused = true;
        }

        depthStencilResolveAttachmentRef.attachment = attachmentCount.get();
        depthStencilResolveAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilResolveAttachmentRef.aspectMask = 0;

        if (!isMSRTTEmulationDepthInvalidated && !isDepthUnused)
        {
            depthStencilResolveAttachmentRef.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (!isMSRTTEmulationStencilInvalidated && !isStencilUnused)
        {
            depthStencilResolveAttachmentRef.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        vk::UnpackDepthStencilResolveAttachmentDesc(
            context, &attachmentDescs[attachmentCount.get()], attachmentFormatID,
            {desc.hasDepthUnresolveAttachment(), isMSRTTEmulationDepthInvalidated, isDepthUnused},
            {desc.hasStencilUnresolveAttachment(), isMSRTTEmulationStencilInvalidated,
             isStencilUnused});

        ++attachmentCount;
    }

    vk::SubpassVector<VkSubpassDescription2> subpassDesc;

    // If any attachment needs to be unresolved, create an initial subpass for this purpose.  Note
    // that the following arrays are used in initializing a VkSubpassDescription2 in subpassDesc,
    // which is in turn used in VkRenderPassCreateInfo below.  That is why they are declared in the
    // same scope.
    gl::DrawBuffersVector<VkAttachmentReference2> unresolveColorAttachmentRefs;
    VkAttachmentReference2 unresolveDepthStencilAttachmentRef = kUnusedAttachment;
    vk::FramebufferAttachmentsVector<VkAttachmentReference2> unresolveInputAttachmentRefs;
    vk::FramebufferAttachmentsVector<uint32_t> unresolvePreserveAttachmentRefs;
    if (hasUnresolveAttachments)
    {
        subpassDesc.push_back({});
        vk::InitializeUnresolveSubpass(
            desc, colorAttachmentRefs, colorResolveAttachmentRefs, depthStencilAttachmentRef,
            depthStencilResolveAttachmentRef, &unresolveColorAttachmentRefs,
            &unresolveDepthStencilAttachmentRef, &unresolveInputAttachmentRefs,
            &unresolvePreserveAttachmentRefs, &subpassDesc.back());
    }

    subpassDesc.push_back({});
    VkSubpassDescription2 *applicationSubpass = &subpassDesc.back();

    applicationSubpass->sType             = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
    applicationSubpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    applicationSubpass->inputAttachmentCount =
        needInputAttachments ? static_cast<uint32_t>(colorAttachmentRefs.size()) : 0;
    applicationSubpass->pInputAttachments =
        needInputAttachments ? colorAttachmentRefs.data() : nullptr;
    applicationSubpass->colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    applicationSubpass->pColorAttachments    = colorAttachmentRefs.data();
    applicationSubpass->pResolveAttachments  = attachmentCount.get() > nonResolveAttachmentCount
                                                   ? colorResolveAttachmentRefs.data()
                                                   : nullptr;
    applicationSubpass->pDepthStencilAttachment =
        (depthStencilAttachmentRef.attachment != VK_ATTACHMENT_UNUSED ? &depthStencilAttachmentRef
                                                                      : nullptr);

    // Specify rasterization order for color on the subpass when available and
    // there is framebuffer fetch.  This is required when the corresponding
    // flag is set on the pipeline.
    if (renderer->getFeatures().supportsRasterizationOrderAttachmentAccess.enabled &&
        desc.hasColorFramebufferFetch())
    {
        for (VkSubpassDescription2 &subpass : subpassDesc)
        {
            subpass.flags |=
                VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_EXT;
        }
    }

    if (desc.isLegacyDitherEnabled())
    {
        ASSERT(renderer->getFeatures().supportsLegacyDithering.enabled);
        subpassDesc.back().flags |= VK_SUBPASS_DESCRIPTION_ENABLE_LEGACY_DITHERING_BIT_EXT;
    }

    // If depth/stencil is to be resolved, add a VkSubpassDescriptionDepthStencilResolve to the
    // pNext chain of the subpass description.
    VkSubpassDescriptionDepthStencilResolve depthStencilResolve = {};
    VkSubpassDescriptionDepthStencilResolve msrtssResolve       = {};
    VkMultisampledRenderToSingleSampledInfoEXT msrtss           = {};
    if (desc.hasDepthStencilResolveAttachment())
    {
        ASSERT(!isRenderToTextureThroughExtension);

        uint32_t depthStencilIndexGL = static_cast<uint32_t>(desc.depthStencilAttachmentIndex());
        const angle::Format &angleFormat = angle::Format::Get(desc[depthStencilIndexGL]);

        depthStencilResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;

        if (!renderer->getFeatures().supportsDepthStencilIndependentResolveNone.enabled)
        {
            // Assert that depth/stencil is not separately resolved without this feature
            ASSERT(desc.hasDepthResolveAttachment() || angleFormat.depthBits == 0);
            ASSERT(desc.hasStencilResolveAttachment() || angleFormat.stencilBits == 0);

            depthStencilResolve.depthResolveMode   = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            depthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
        }
        else
        {
            depthStencilResolve.depthResolveMode =
                desc.hasDepthResolveAttachment() && !isMSRTTEmulationDepthInvalidated
                    ? VK_RESOLVE_MODE_SAMPLE_ZERO_BIT
                    : VK_RESOLVE_MODE_NONE;
            depthStencilResolve.stencilResolveMode =
                desc.hasStencilResolveAttachment() && !isMSRTTEmulationStencilInvalidated
                    ? VK_RESOLVE_MODE_SAMPLE_ZERO_BIT
                    : VK_RESOLVE_MODE_NONE;
        }

        // If depth/stencil attachment is invalidated or is otherwise not really resolved, don't set
        // it as the resolve attachment in the first place.
        const bool isResolvingDepth = !isMSRTTEmulationDepthInvalidated &&
                                      angleFormat.depthBits > 0 &&
                                      depthStencilResolve.depthResolveMode != VK_RESOLVE_MODE_NONE;
        const bool isResolvingStencil =
            !isMSRTTEmulationStencilInvalidated && angleFormat.stencilBits > 0 &&
            depthStencilResolve.stencilResolveMode != VK_RESOLVE_MODE_NONE;

        if (isResolvingDepth || isResolvingStencil)
        {
            depthStencilResolve.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentRef;
            vk::AddToPNextChain(&subpassDesc.back(), &depthStencilResolve);
        }
    }
    else if (isRenderToTextureThroughExtension)
    {
        ASSERT(subpassDesc.size() == 1);
        vk::InitializeMSRTSS(context, renderToTextureSamples, &subpassDesc.back(), &msrtssResolve,
                             &msrtss);
    }

    VkFragmentShadingRateAttachmentInfoKHR fragmentShadingRateAttachmentInfo = {};
    if (desc.hasFragmentShadingAttachment())
    {
        fragmentShadingRateAttachmentInfo.sType =
            VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
        fragmentShadingRateAttachmentInfo.pFragmentShadingRateAttachment =
            &fragmentShadingRateAttachmentRef;
        fragmentShadingRateAttachmentInfo.shadingRateAttachmentTexelSize =
            renderer->getMaxFragmentShadingRateAttachmentTexelSize();

        vk::AddToPNextChain(&subpassDesc.back(), &fragmentShadingRateAttachmentInfo);
    }

    std::vector<VkSubpassDependency2> subpassDependencies;
    if (hasUnresolveAttachments)
    {
        vk::InitializeUnresolveSubpassDependencies(
            subpassDesc, desc.getColorUnresolveAttachmentMask().any(),
            desc.hasDepthStencilUnresolveAttachment(), &subpassDependencies);
    }

    const uint32_t drawSubpassIndex = static_cast<uint32_t>(subpassDesc.size()) - 1;
    vk::InitializeDefaultSubpassSelfDependencies(context, desc, drawSubpassIndex,
                                                 &subpassDependencies);

    VkRenderPassCreateInfo2 createInfo = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
    createInfo.attachmentCount         = attachmentCount.get();
    createInfo.pAttachments            = attachmentDescs.data();
    createInfo.subpassCount            = static_cast<uint32_t>(subpassDesc.size());
    createInfo.pSubpasses              = subpassDesc.data();

    if (!subpassDependencies.empty())
    {
        createInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        createInfo.pDependencies   = subpassDependencies.data();
    }

    const uint32_t viewMask = angle::BitMask<uint32_t>(desc.viewCount());
    if (desc.viewCount() > 0)
    {
        vk::SetRenderPassViewMask(context, &viewMask, &createInfo, &subpassDesc);
    }

    // If VK_KHR_create_renderpass2 is not supported, we must use core Vulkan 1.0.  This is
    // increasingly uncommon.  Note that extensions that require chaining information to subpasses
    // are automatically not used when this extension is not available.
    if (!renderer->getFeatures().supportsRenderpass2.enabled)
    {
        ANGLE_TRY(vk::CreateRenderPass1(context, createInfo, desc.viewCount(), renderPass));
    }
    else
    {
        ANGLE_VK_TRY(context, renderPass->init2(context->getDevice(), createInfo));
    }

    if (renderPassCounters != nullptr)
    {
        // Calculate perf counters associated with this render pass, such as load/store ops,
        // unresolve and resolve operations etc.  This information is taken out of the render pass
        // create info.  Depth/stencil resolve attachment uses RenderPass2 structures, so it's
        // passed in separately.
        vk::UpdateRenderPassPerfCounters(desc, createInfo, depthStencilResolve, renderPassCounters);
    }

    return angle::Result::Continue;
}

// ComputePipelineCache implementation
void ComputePipelineCache::destroy(vk::ErrorContext *context)
{
    VkDevice device = context->getDevice();

    for (auto &item : mPayload)
    {
        vk::PipelineHelper &pipeline = item.second;
        ASSERT(context->getRenderer()->hasResourceUseFinished(pipeline.getResourceUse()));
        pipeline.destroy(device);
    }

    mPayload.clear();
}

void ComputePipelineCache::release(vk::ErrorContext *context)
{
    for (auto &item : mPayload)
    {
        vk::PipelineHelper &pipeline = item.second;
        pipeline.release(context);
    }

    mPayload.clear();
}

angle::Result ComputePipelineCache::getOrCreatePipeline(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::PipelineLayout &pipelineLayout,
    rx::vk::ComputePipelineOptions &pipelineOptions,
    PipelineSource source,
    vk::PipelineHelper **pipelineOut,
    const char *shaderName,
    VkSpecializationInfo *specializationInfo,
    const vk::ShaderModuleMap &shaderModuleMap)
{
    vk::ComputePipelineDesc desc(specializationInfo, pipelineOptions);

    auto iter = mPayload.find(desc);
    if (iter != mPayload.end())
    {
        mCacheStats.hit();
        *pipelineOut = &iter->second;
        return angle::Result::Continue;
    }
    return createPipeline(context, pipelineCache, pipelineLayout, pipelineOptions, source,
                          shaderName, *shaderModuleMap[gl::ShaderType::Compute].get(),
                          specializationInfo, desc, pipelineOut);
}

angle::Result ComputePipelineCache::createPipeline(vk::ErrorContext *context,
                                                   vk::PipelineCacheAccess *pipelineCache,
                                                   const vk::PipelineLayout &pipelineLayout,
                                                   vk::ComputePipelineOptions &pipelineOptions,
                                                   PipelineSource source,
                                                   const char *shaderName,
                                                   const vk::ShaderModule &shaderModule,
                                                   VkSpecializationInfo *specializationInfo,
                                                   const vk::ComputePipelineDesc &desc,
                                                   vk::PipelineHelper **pipelineOut)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    VkComputePipelineCreateInfo createInfo      = {};

    shaderStage.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.flags               = 0;
    shaderStage.stage               = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStage.module              = shaderModule.getHandle();
    shaderStage.pName               = shaderName ? shaderName : "main";
    shaderStage.pSpecializationInfo = specializationInfo;

    createInfo.sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.flags              = 0;
    createInfo.stage              = shaderStage;
    createInfo.layout             = pipelineLayout.getHandle();
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex  = 0;

    VkPipelineRobustnessCreateInfoEXT robustness = {};
    robustness.sType = VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT;

    // Enable robustness on the pipeline if needed.  Note that the global robustBufferAccess feature
    // must be disabled by default.
    if (pipelineOptions.robustness != 0)
    {
        ASSERT(context->getFeatures().supportsPipelineRobustness.enabled);

        robustness.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
        robustness.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
        robustness.vertexInputs   = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_DEVICE_DEFAULT_EXT;
        robustness.images         = VK_PIPELINE_ROBUSTNESS_IMAGE_BEHAVIOR_DEVICE_DEFAULT_EXT;

        vk::AddToPNextChain(&createInfo, &robustness);
    }

    // Restrict pipeline to protected or unprotected command buffers if possible.
    if (pipelineOptions.protectedAccess != 0)
    {
        ASSERT(context->getFeatures().supportsPipelineProtectedAccess.enabled);
        createInfo.flags |= VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
    }
    else if (context->getFeatures().supportsPipelineProtectedAccess.enabled)
    {
        createInfo.flags |= VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT;
    }

    VkPipelineCreationFeedback feedback               = {};
    VkPipelineCreationFeedback perStageFeedback       = {};
    VkPipelineCreationFeedbackCreateInfo feedbackInfo = {};
    feedbackInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO;
    feedbackInfo.pPipelineCreationFeedback = &feedback;
    // Note: see comment in GraphicsPipelineDesc::initializePipeline about why per-stage feedback is
    // specified even though unused.
    feedbackInfo.pipelineStageCreationFeedbackCount = 1;
    feedbackInfo.pPipelineStageCreationFeedbacks    = &perStageFeedback;

    const bool supportsFeedback =
        context->getRenderer()->getFeatures().supportsPipelineCreationFeedback.enabled;
    if (supportsFeedback)
    {
        vk::AddToPNextChain(&createInfo, &feedbackInfo);
    }

    vk::Pipeline pipeline;
    ANGLE_VK_TRY(context, pipelineCache->createComputePipeline(context, createInfo, &pipeline));

    vk::CacheLookUpFeedback lookUpFeedback = vk::CacheLookUpFeedback::None;

    if (supportsFeedback)
    {
        const bool cacheHit =
            (feedback.flags & VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT) !=
            0;

        lookUpFeedback = cacheHit ? vk::CacheLookUpFeedback::Hit : vk::CacheLookUpFeedback::Miss;
        ApplyPipelineCreationFeedback(context, feedback);
    }
    vk::PipelineHelper computePipeline = vk::PipelineHelper();
    computePipeline.setComputePipeline(std::move(pipeline), lookUpFeedback);

    mCacheStats.missAndIncrementSize();
    mPayload[desc] = std::move(computePipeline);
    *pipelineOut   = &mPayload[desc];

    return angle::Result::Continue;
}

// GraphicsPipelineCache implementation.
template <typename Hash>
void GraphicsPipelineCache<Hash>::destroy(vk::ErrorContext *context)
{
    if (vk::ShouldDumpPipelineCacheGraph(context) && !mPayload.empty())
    {
        vk::DumpPipelineCacheGraph<Hash>(context, mPayload);
    }

    accumulateCacheStats(context->getRenderer());

    VkDevice device = context->getDevice();

    for (auto &item : mPayload)
    {
        vk::PipelineHelper &pipeline = item.second;
        pipeline.destroy(device);
    }

    mPayload.clear();
}

template <typename Hash>
void GraphicsPipelineCache<Hash>::release(vk::ErrorContext *context)
{
    if (vk::ShouldDumpPipelineCacheGraph(context) && !mPayload.empty())
    {
        vk::DumpPipelineCacheGraph<Hash>(context, mPayload);
    }

    for (auto &item : mPayload)
    {
        vk::PipelineHelper &pipeline = item.second;
        pipeline.release(context);
    }

    mPayload.clear();
}

template <typename Hash>
angle::Result GraphicsPipelineCache<Hash>::createPipeline(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::RenderPass &compatibleRenderPass,
    const vk::PipelineLayout &pipelineLayout,
    const vk::ShaderModuleMap &shaders,
    const vk::SpecializationConstants &specConsts,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    vk::Pipeline newPipeline;
    vk::CacheLookUpFeedback feedback = vk::CacheLookUpFeedback::None;

    // This "if" is left here for the benefit of VulkanPipelineCachePerfTest.
    if (context != nullptr)
    {
        constexpr vk::GraphicsPipelineSubset kSubset =
            GraphicsPipelineCacheTypeHelper<Hash>::kSubset;

        ANGLE_VK_TRY(context, desc.initializePipeline(context, pipelineCache, kSubset,
                                                      compatibleRenderPass, pipelineLayout, shaders,
                                                      specConsts, &newPipeline, &feedback));
    }

    if (source == PipelineSource::WarmUp)
    {
        // Warm up task will pass in the placeholder PipelineHelper created in
        // ProgramExecutableVk::getPipelineCacheWarmUpTasks. Update that with the newly created
        // pipeline.
        **pipelineOut =
            vk::PipelineHelper(std::move(newPipeline), vk::CacheLookUpFeedback::WarmUpMiss);
    }
    else
    {
        addToCache(source, desc, std::move(newPipeline), feedback, descPtrOut, pipelineOut);
    }
    return angle::Result::Continue;
}

template <typename Hash>
angle::Result GraphicsPipelineCache<Hash>::linkLibraries(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::GraphicsPipelineDesc &desc,
    const vk::PipelineLayout &pipelineLayout,
    vk::PipelineHelper *vertexInputPipeline,
    vk::PipelineHelper *shadersPipeline,
    vk::PipelineHelper *fragmentOutputPipeline,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    vk::Pipeline newPipeline;
    vk::CacheLookUpFeedback feedback = vk::CacheLookUpFeedback::None;

    ANGLE_TRY(vk::InitializePipelineFromLibraries(
        context, pipelineCache, pipelineLayout, *vertexInputPipeline, *shadersPipeline,
        *fragmentOutputPipeline, desc, &newPipeline, &feedback));

    addToCache(PipelineSource::DrawLinked, desc, std::move(newPipeline), feedback, descPtrOut,
               pipelineOut);
    (*pipelineOut)->setLinkedLibraryReferences(shadersPipeline);

    return angle::Result::Continue;
}

template <typename Hash>
void GraphicsPipelineCache<Hash>::addToCache(PipelineSource source,
                                             const vk::GraphicsPipelineDesc &desc,
                                             vk::Pipeline &&pipeline,
                                             vk::CacheLookUpFeedback feedback,
                                             const vk::GraphicsPipelineDesc **descPtrOut,
                                             vk::PipelineHelper **pipelineOut)
{
    ASSERT(mPayload.find(desc) == mPayload.end());

    mCacheStats.missAndIncrementSize();

    switch (source)
    {
        case PipelineSource::WarmUp:
            feedback = feedback == vk::CacheLookUpFeedback::Hit
                           ? vk::CacheLookUpFeedback::WarmUpHit
                           : vk::CacheLookUpFeedback::WarmUpMiss;
            break;
        case PipelineSource::DrawLinked:
            feedback = feedback == vk::CacheLookUpFeedback::Hit
                           ? vk::CacheLookUpFeedback::LinkedDrawHit
                           : vk::CacheLookUpFeedback::LinkedDrawMiss;
            break;
        case PipelineSource::Utils:
            feedback = feedback == vk::CacheLookUpFeedback::Hit
                           ? vk::CacheLookUpFeedback::UtilsHit
                           : vk::CacheLookUpFeedback::UtilsMiss;
            break;
        default:
            break;
    }

    auto insertedItem = mPayload.emplace(std::piecewise_construct, std::forward_as_tuple(desc),
                                         std::forward_as_tuple(std::move(pipeline), feedback));
    *descPtrOut       = &insertedItem.first->first;
    *pipelineOut      = &insertedItem.first->second;
}

template <typename Hash>
void GraphicsPipelineCache<Hash>::populate(const vk::GraphicsPipelineDesc &desc,
                                           vk::Pipeline &&pipeline,
                                           vk::PipelineHelper **pipelineHelperOut)
{
    auto item = mPayload.find(desc);
    if (item != mPayload.end())
    {
        return;
    }

    // This function is used by -
    // 1. WarmUp tasks to insert placeholder pipelines
    // 2. VulkanPipelineCachePerfTest
    auto insertedItem =
        mPayload.emplace(std::piecewise_construct, std::forward_as_tuple(desc),
                         std::forward_as_tuple(std::move(pipeline), vk::CacheLookUpFeedback::None));

    if (pipelineHelperOut)
    {
        ASSERT(insertedItem.second);
        *pipelineHelperOut = &insertedItem.first->second;
    }
}

// Instantiate the pipeline cache functions
template void GraphicsPipelineCache<GraphicsPipelineDescCompleteHash>::destroy(
    vk::ErrorContext *context);
template void GraphicsPipelineCache<GraphicsPipelineDescCompleteHash>::release(
    vk::ErrorContext *context);
template angle::Result GraphicsPipelineCache<GraphicsPipelineDescCompleteHash>::createPipeline(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::RenderPass &compatibleRenderPass,
    const vk::PipelineLayout &pipelineLayout,
    const vk::ShaderModuleMap &shaders,
    const vk::SpecializationConstants &specConsts,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut);
template angle::Result GraphicsPipelineCache<GraphicsPipelineDescCompleteHash>::linkLibraries(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::GraphicsPipelineDesc &desc,
    const vk::PipelineLayout &pipelineLayout,
    vk::PipelineHelper *vertexInputPipeline,
    vk::PipelineHelper *shadersPipeline,
    vk::PipelineHelper *fragmentOutputPipeline,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut);
template void GraphicsPipelineCache<GraphicsPipelineDescCompleteHash>::populate(
    const vk::GraphicsPipelineDesc &desc,
    vk::Pipeline &&pipeline,
    vk::PipelineHelper **pipelineHelperOut);

template void GraphicsPipelineCache<GraphicsPipelineDescVertexInputHash>::destroy(
    vk::ErrorContext *context);
template void GraphicsPipelineCache<GraphicsPipelineDescVertexInputHash>::release(
    vk::ErrorContext *context);
template angle::Result GraphicsPipelineCache<GraphicsPipelineDescVertexInputHash>::createPipeline(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::RenderPass &compatibleRenderPass,
    const vk::PipelineLayout &pipelineLayout,
    const vk::ShaderModuleMap &shaders,
    const vk::SpecializationConstants &specConsts,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut);
template void GraphicsPipelineCache<GraphicsPipelineDescVertexInputHash>::populate(
    const vk::GraphicsPipelineDesc &desc,
    vk::Pipeline &&pipeline,
    vk::PipelineHelper **pipelineHelperOut);

template void GraphicsPipelineCache<GraphicsPipelineDescShadersHash>::destroy(
    vk::ErrorContext *context);
template void GraphicsPipelineCache<GraphicsPipelineDescShadersHash>::release(
    vk::ErrorContext *context);
template angle::Result GraphicsPipelineCache<GraphicsPipelineDescShadersHash>::createPipeline(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::RenderPass &compatibleRenderPass,
    const vk::PipelineLayout &pipelineLayout,
    const vk::ShaderModuleMap &shaders,
    const vk::SpecializationConstants &specConsts,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut);
template void GraphicsPipelineCache<GraphicsPipelineDescShadersHash>::populate(
    const vk::GraphicsPipelineDesc &desc,
    vk::Pipeline &&pipeline,
    vk::PipelineHelper **pipelineHelperOut);

template void GraphicsPipelineCache<GraphicsPipelineDescFragmentOutputHash>::destroy(
    vk::ErrorContext *context);
template void GraphicsPipelineCache<GraphicsPipelineDescFragmentOutputHash>::release(
    vk::ErrorContext *context);
template angle::Result
GraphicsPipelineCache<GraphicsPipelineDescFragmentOutputHash>::createPipeline(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::RenderPass &compatibleRenderPass,
    const vk::PipelineLayout &pipelineLayout,
    const vk::ShaderModuleMap &shaders,
    const vk::SpecializationConstants &specConsts,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut);
template void GraphicsPipelineCache<GraphicsPipelineDescFragmentOutputHash>::populate(
    const vk::GraphicsPipelineDesc &desc,
    vk::Pipeline &&pipeline,
    vk::PipelineHelper **pipelineHelperOut);

// DescriptorSetLayoutCache implementation.
DescriptorSetLayoutCache::DescriptorSetLayoutCache() = default;

DescriptorSetLayoutCache::~DescriptorSetLayoutCache()
{
    ASSERT(mPayload.empty());
}

void DescriptorSetLayoutCache::destroy(vk::Renderer *renderer)
{
    renderer->accumulateCacheStats(VulkanCacheType::DescriptorSetLayout, mCacheStats);
    ASSERT(AllCacheEntriesHaveUniqueReference(mPayload));
    mPayload.clear();
}

angle::Result DescriptorSetLayoutCache::getDescriptorSetLayout(
    vk::ErrorContext *context,
    const vk::DescriptorSetLayoutDesc &desc,
    vk::DescriptorSetLayoutPtr *descriptorSetLayoutOut)
{
    // Note: this function may be called without holding the share group lock.
    std::unique_lock<angle::SimpleMutex> lock(mMutex);

    auto iter = mPayload.find(desc);
    if (iter != mPayload.end())
    {
        *descriptorSetLayoutOut = iter->second;
        mCacheStats.hit();
        return angle::Result::Continue;
    }

    // If DescriptorSetLayoutDesc is empty, reuse placeholder descriptor set layout handle
    if (desc.empty())
    {
        *descriptorSetLayoutOut = context->getRenderer()->getEmptyDescriptorLayout();
        return angle::Result::Continue;
    }

    mCacheStats.missAndIncrementSize();
    // We must unpack the descriptor set layout description.
    vk::DescriptorSetLayoutBindingVector bindingVector;
    desc.unpackBindings(&bindingVector);

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.flags        = 0;
    createInfo.bindingCount = static_cast<uint32_t>(bindingVector.size());
    createInfo.pBindings    = bindingVector.data();

    vk::DescriptorSetLayoutPtr newLayout =
        vk::DescriptorSetLayoutPtr::MakeShared(context->getDevice());
    ANGLE_VK_TRY(context, newLayout->init(context->getDevice(), createInfo));

    *descriptorSetLayoutOut = newLayout;
    mPayload.emplace(desc, std::move(newLayout));

    return angle::Result::Continue;
}

// PipelineLayoutCache implementation.
PipelineLayoutCache::PipelineLayoutCache() = default;

PipelineLayoutCache::~PipelineLayoutCache()
{
    ASSERT(mPayload.empty());
}

void PipelineLayoutCache::destroy(vk::Renderer *renderer)
{
    accumulateCacheStats(renderer);
    ASSERT(AllCacheEntriesHaveUniqueReference(mPayload));
    mPayload.clear();
}

angle::Result PipelineLayoutCache::getPipelineLayout(
    vk::ErrorContext *context,
    const vk::PipelineLayoutDesc &desc,
    const vk::DescriptorSetLayoutPointerArray &descriptorSetLayouts,
    vk::PipelineLayoutPtr *pipelineLayoutOut)
{
    // Note: this function may be called without holding the share group lock.
    std::unique_lock<angle::SimpleMutex> lock(mMutex);

    auto iter = mPayload.find(desc);
    if (iter != mPayload.end())
    {
        *pipelineLayoutOut = iter->second;
        mCacheStats.hit();
        return angle::Result::Continue;
    }

    mCacheStats.missAndIncrementSize();
    // Note this does not handle gaps in descriptor set layouts gracefully.
    angle::FixedVector<VkDescriptorSetLayout, vk::kMaxDescriptorSetLayouts> setLayoutHandles;
    for (const vk::DescriptorSetLayoutPtr &layoutPtr : descriptorSetLayouts)
    {
        if (layoutPtr)
        {
            VkDescriptorSetLayout setLayout = layoutPtr->getHandle();
            ASSERT(setLayout != VK_NULL_HANDLE);
            setLayoutHandles.push_back(setLayout);
        }
    }

    const vk::PackedPushConstantRange &descPushConstantRange = desc.getPushConstantRange();
    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = descPushConstantRange.stageMask;
    pushConstantRange.offset     = descPushConstantRange.offset;
    pushConstantRange.size       = descPushConstantRange.size;

    // No pipeline layout found. We must create a new one.
    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.flags                      = 0;
    createInfo.setLayoutCount             = static_cast<uint32_t>(setLayoutHandles.size());
    createInfo.pSetLayouts                = setLayoutHandles.data();
    if (pushConstantRange.size > 0)
    {
        createInfo.pushConstantRangeCount = 1;
        createInfo.pPushConstantRanges    = &pushConstantRange;
    }

    vk::PipelineLayoutPtr newLayout = vk::PipelineLayoutPtr::MakeShared(context->getDevice());
    ANGLE_VK_TRY(context, newLayout->init(context->getDevice(), createInfo));

    *pipelineLayoutOut = newLayout;
    mPayload.emplace(desc, std::move(newLayout));

    return angle::Result::Continue;
}

// YuvConversionCache implementation
SamplerYcbcrConversionCache::SamplerYcbcrConversionCache() = default;

SamplerYcbcrConversionCache::~SamplerYcbcrConversionCache()
{
    ASSERT(mExternalFormatPayload.empty() && mVkFormatPayload.empty());
}

void SamplerYcbcrConversionCache::destroy(vk::Renderer *renderer)
{
    renderer->accumulateCacheStats(VulkanCacheType::SamplerYcbcrConversion, mCacheStats);

    VkDevice device = renderer->getDevice();

    uint32_t count = static_cast<uint32_t>(mExternalFormatPayload.size());
    for (auto &iter : mExternalFormatPayload)
    {
        vk::SamplerYcbcrConversion &samplerYcbcrConversion = iter.second;
        samplerYcbcrConversion.destroy(device);
    }
    renderer->onDeallocateHandle(vk::HandleType::SamplerYcbcrConversion, count);

    count = static_cast<uint32_t>(mExternalFormatPayload.size());
    for (auto &iter : mVkFormatPayload)
    {
        vk::SamplerYcbcrConversion &samplerYcbcrConversion = iter.second;
        samplerYcbcrConversion.destroy(device);
    }
    renderer->onDeallocateHandle(vk::HandleType::SamplerYcbcrConversion, count);

    mExternalFormatPayload.clear();
    mVkFormatPayload.clear();
}

angle::Result SamplerYcbcrConversionCache::getSamplerYcbcrConversion(
    vk::ErrorContext *context,
    const vk::YcbcrConversionDesc &ycbcrConversionDesc,
    VkSamplerYcbcrConversion *vkSamplerYcbcrConversionOut)
{
    ASSERT(ycbcrConversionDesc.valid());
    ASSERT(vkSamplerYcbcrConversionOut);

    SamplerYcbcrConversionMap &payload =
        (ycbcrConversionDesc.getExternalFormat() != 0) ? mExternalFormatPayload : mVkFormatPayload;
    const auto iter = payload.find(ycbcrConversionDesc);
    if (iter != payload.end())
    {
        vk::SamplerYcbcrConversion &samplerYcbcrConversion = iter->second;
        mCacheStats.hit();
        *vkSamplerYcbcrConversionOut = samplerYcbcrConversion.getHandle();
        return angle::Result::Continue;
    }

    mCacheStats.missAndIncrementSize();

    // Create the VkSamplerYcbcrConversion
    vk::SamplerYcbcrConversion wrappedSamplerYcbcrConversion;
    ANGLE_TRY(ycbcrConversionDesc.init(context, &wrappedSamplerYcbcrConversion));

    auto insertedItem = payload.emplace(
        ycbcrConversionDesc, vk::SamplerYcbcrConversion(std::move(wrappedSamplerYcbcrConversion)));
    vk::SamplerYcbcrConversion &insertedSamplerYcbcrConversion = insertedItem.first->second;
    *vkSamplerYcbcrConversionOut = insertedSamplerYcbcrConversion.getHandle();

    context->getRenderer()->onAllocateHandle(vk::HandleType::SamplerYcbcrConversion);

    return angle::Result::Continue;
}

// SamplerCache implementation.
SamplerCache::SamplerCache() = default;

SamplerCache::~SamplerCache()
{
    ASSERT(mPayload.empty());
}

void SamplerCache::destroy(vk::Renderer *renderer)
{
    renderer->accumulateCacheStats(VulkanCacheType::Sampler, mCacheStats);

    uint32_t count = static_cast<uint32_t>(mPayload.size());
    ASSERT(AllCacheEntriesHaveUniqueReference(mPayload));
    mPayload.clear();
    renderer->onDeallocateHandle(vk::HandleType::Sampler, count);
}

angle::Result SamplerCache::getSampler(ContextVk *contextVk,
                                       const vk::SamplerDesc &desc,
                                       vk::SharedSamplerPtr *samplerOut)
{
    auto iter = mPayload.find(desc);
    if (iter != mPayload.end())
    {
        *samplerOut = iter->second;
        mCacheStats.hit();
        return angle::Result::Continue;
    }

    mCacheStats.missAndIncrementSize();
    vk::SharedSamplerPtr newSampler = vk::SharedSamplerPtr::MakeShared(contextVk->getDevice());
    ANGLE_TRY(newSampler->init(contextVk, desc));

    (*samplerOut) = newSampler;
    mPayload.emplace(desc, std::move(newSampler));

    contextVk->getRenderer()->onAllocateHandle(vk::HandleType::Sampler);

    return angle::Result::Continue;
}
}  // namespace rx
