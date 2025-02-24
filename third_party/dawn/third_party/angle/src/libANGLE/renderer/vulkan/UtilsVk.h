//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UtilsVk.h:
//    Defines the UtilsVk class, a helper for various internal draw/dispatch utilities such as
//    buffer clear and copy, image clear and copy, texture mip map generation, etc.
//
//    - Convert index buffer:
//      * Used by VertexArrayVk::convertIndexBufferGPU() to convert a ubyte element array to ushort
//    - Convert vertex buffer:
//      * Used by VertexArrayVk::convertVertexBufferGPU() to convert vertex attributes from
//        unsupported formats to their fallbacks.
//    - Image clear: Used by FramebufferVk::clearWithDraw().
//    - Image copy: Used by TextureVk::copySubImageImplWithDraw().
//    - Image copy bits: Used by ImageHelper::CopyImageSubData() to perform bitwise copies between
//      RGB formats where at least one of src and dst use RGBA as fallback.
//    - Color blit/resolve: Used by FramebufferVk::blit() to implement blit or multisample resolve
//      on color images.
//    - Depth/Stencil blit/resolve: Used by FramebufferVk::blit() to implement blit or multisample
//      resolve on depth/stencil images.
//    - Generate mipmap: Used by TextureVk::generateMipmapsWithCompute().
//    - Overlay Draw: Used by OverlayVk to draw a UI for debugging.
//    - Mipmap generation: Used by TextureVk to generate mipmaps more efficiently in compute.
//

#ifndef LIBANGLE_RENDERER_VULKAN_UTILSVK_H_
#define LIBANGLE_RENDERER_VULKAN_UTILSVK_H_

#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_internal_shaders_autogen.h"

namespace rx
{
class UtilsVk : angle::NonCopyable
{
  public:
    UtilsVk();
    ~UtilsVk();

    void destroy(ContextVk *contextVk);

    struct ConvertIndexParameters
    {
        uint32_t srcOffset = 0;
        uint32_t dstOffset = 0;
        uint32_t maxIndex  = 0;
    };

    struct ConvertIndexIndirectParameters
    {
        uint32_t srcIndirectBufOffset = 0;
        uint32_t srcIndexBufOffset    = 0;
        uint32_t dstIndexBufOffset    = 0;
        uint32_t maxIndex             = 0;
        uint32_t dstIndirectBufOffset = 0;
    };

    struct ConvertLineLoopIndexIndirectParameters
    {
        uint32_t indirectBufferOffset    = 0;
        uint32_t dstIndirectBufferOffset = 0;
        uint32_t srcIndexBufferOffset    = 0;
        uint32_t dstIndexBufferOffset    = 0;
        uint32_t indicesBitsWidth        = 0;
    };

    struct ConvertLineLoopArrayIndirectParameters
    {
        uint32_t indirectBufferOffset    = 0;
        uint32_t dstIndirectBufferOffset = 0;
        uint32_t dstIndexBufferOffset    = 0;
    };

    struct OffsetAndVertexCount
    {
        uint32_t srcOffset;
        uint32_t dstOffset;
        uint32_t vertexCount;
    };
    using OffsetAndVertexCounts = std::vector<OffsetAndVertexCount>;

    struct ConvertVertexParameters
    {
        size_t vertexCount;
        const angle::Format *srcFormat;
        const angle::Format *dstFormat;
        size_t srcStride;
        size_t srcOffset;
        size_t dstOffset;
    };

    struct ClearFramebufferParameters
    {
        // Satisfy chromium-style with a constructor that does what = {} was already doing in a
        // safer way.
        ClearFramebufferParameters();

        gl::Rectangle clearArea;

        bool clearColor;
        bool clearDepth;
        bool clearStencil;

        uint8_t stencilMask;
        VkColorComponentFlags colorMaskFlags;
        uint32_t colorAttachmentIndexGL;
        const angle::Format *colorFormat;

        VkClearColorValue colorClearValue;
        VkClearDepthStencilValue depthStencilClearValue;
    };

    struct ClearTextureParameters
    {
        VkImageAspectFlags aspectFlags;
        vk::LevelIndex level;
        uint32_t layer;
        gl::Box clearArea;
        VkClearValue clearValue;
    };

    struct BlitResolveParameters
    {
        // |srcOffset| and |dstIndexBufferOffset| define the original blit/resolve offsets, possibly
        // flipped.
        int srcOffset[2];
        int dstOffset[2];
        // Amount to add to x and y axis for certain rotations
        int rotatedOffsetFactor[2];
        // |stretch| is SourceDimension / DestDimension used to transfer dst coordinates to source.
        float stretch[2];
        // |srcExtents| is used to normalize source coordinates for sampling.
        int srcExtents[2];
        // |blitArea| is the area in destination where blit happens.  It's expected that scissor
        // and source clipping effects have already been applied to it.
        gl::Rectangle blitArea;
        int srcLayer;
        // Whether linear or point sampling should be used.
        bool linear;
        bool flipX;
        bool flipY;
        SurfaceRotation rotation;
    };

    struct ClearImageParameters
    {
        gl::Rectangle clearArea;

        vk::LevelIndex dstMip;
        int dstLayer;

        VkColorComponentFlags colorMaskFlags;
        VkClearColorValue colorClearValue;
    };

    struct CopyImageParameters
    {
        int srcOffset[2];
        int srcExtents[2];
        int dstOffset[2];
        int srcMip;
        int srcLayer;
        int srcSampleCount;
        int srcHeight;
        gl::LevelIndex dstMip;
        int dstLayer;
        bool srcPremultiplyAlpha;
        bool srcUnmultiplyAlpha;
        bool srcFlipY;
        bool dstFlipY;
        SurfaceRotation srcRotation;
        GLenum srcColorEncoding;
        GLenum dstColorEncoding;
    };

    struct CopyImageBitsParameters
    {
        int srcOffset[3];
        gl::LevelIndex srcLevel;
        int dstOffset[3];
        gl::LevelIndex dstLevel;
        uint32_t copyExtents[3];
    };

    struct CopyImageToBufferParameters
    {
        int srcOffset[2];
        vk::LevelIndex srcMip;
        int srcLayer;
        uint32_t size[2];
        ptrdiff_t outputOffset;
        uint32_t outputPitch;
        bool reverseRowOrder;
        const angle::Format *outputFormat;
    };

    struct OverlayDrawParameters
    {
        uint32_t textWidgetCount;
        uint32_t graphWidgetCount;
        bool rotateXY;
    };

    struct GenerateMipmapParameters
    {
        uint32_t srcLevel;
        uint32_t dstLevelCount;
    };

    struct UnresolveParameters
    {
        gl::DrawBufferMask unresolveColorMask;
        bool unresolveDepth;
        bool unresolveStencil;
    };

    struct GenerateFragmentShadingRateParameters
    {
        uint32_t textureWidth;
        uint32_t textureHeight;
        uint32_t attachmentWidth;
        uint32_t attachmentHeight;
        uint32_t attachmentBlockWidth;
        uint32_t attachmentBlockHeight;
        uint32_t numFocalPoints;
        gl::FocalPoint focalPoints[gl::IMPLEMENTATION_MAX_FOCAL_POINTS];
    };

    // Based on the maximum number of levels in GenerateMipmap.comp.
    static constexpr uint32_t kGenerateMipmapMaxLevels = 6;
    static uint32_t GetGenerateMipmapMaxLevels(ContextVk *contextVk);

    angle::Result convertIndexBuffer(ContextVk *contextVk,
                                     vk::BufferHelper *dst,
                                     vk::BufferHelper *src,
                                     const ConvertIndexParameters &params);
    angle::Result convertIndexIndirectBuffer(ContextVk *contextVk,
                                             vk::BufferHelper *srcIndirectBuf,
                                             vk::BufferHelper *srcIndexBuf,
                                             vk::BufferHelper *dstIndirectBuf,
                                             vk::BufferHelper *dstIndexBuf,
                                             const ConvertIndexIndirectParameters &params);

    angle::Result convertLineLoopIndexIndirectBuffer(
        ContextVk *contextVk,
        vk::BufferHelper *srcIndirectBuffer,
        vk::BufferHelper *srcIndexBuffer,
        vk::BufferHelper *dstIndirectBuffer,
        vk::BufferHelper *dstIndexBuffer,
        const ConvertLineLoopIndexIndirectParameters &params);

    angle::Result convertLineLoopArrayIndirectBuffer(
        ContextVk *contextVk,
        vk::BufferHelper *srcIndirectBuffer,
        vk::BufferHelper *dstIndirectBuffer,
        vk::BufferHelper *dstIndexBuffer,
        const ConvertLineLoopArrayIndirectParameters &params);

    angle::Result convertVertexBuffer(ContextVk *contextVk,
                                      vk::BufferHelper *dst,
                                      vk::BufferHelper *src,
                                      const ConvertVertexParameters &params,
                                      const OffsetAndVertexCounts &additionalOffsetVertexCounts);

    // EXT_clear_texture
    angle::Result clearTexture(ContextVk *contextVk,
                               vk::ImageHelper *dst,
                               ClearTextureParameters &params);

    angle::Result clearFramebuffer(ContextVk *contextVk,
                                   FramebufferVk *framebuffer,
                                   const ClearFramebufferParameters &params);

    // Resolve images if multisampled.  Blit otherwise.
    angle::Result colorBlitResolve(ContextVk *contextVk,
                                   FramebufferVk *framebuffer,
                                   vk::ImageHelper *src,
                                   const vk::ImageView *srcView,
                                   const BlitResolveParameters &params);
    angle::Result depthStencilBlitResolve(ContextVk *contextVk,
                                          FramebufferVk *framebuffer,
                                          vk::ImageHelper *src,
                                          const vk::ImageView *srcDepthView,
                                          const vk::ImageView *srcStencilView,
                                          const BlitResolveParameters &params);
    angle::Result stencilBlitResolveNoShaderExport(ContextVk *contextVk,
                                                   FramebufferVk *framebuffer,
                                                   vk::ImageHelper *src,
                                                   const vk::ImageView *srcStencilView,
                                                   const BlitResolveParameters &params);

    angle::Result clearImage(ContextVk *contextVk,
                             vk::ImageHelper *dst,
                             const ClearImageParameters &params);

    angle::Result copyImage(ContextVk *contextVk,
                            vk::ImageHelper *dst,
                            const vk::ImageView *dstView,
                            vk::ImageHelper *src,
                            const vk::ImageView *srcView,
                            const CopyImageParameters &params);

    angle::Result copyImageBits(ContextVk *contextVk,
                                vk::ImageHelper *dst,
                                vk::ImageHelper *src,
                                const CopyImageBitsParameters &params);

    angle::Result copyImageToBuffer(ContextVk *contextVk,
                                    vk::BufferHelper *dst,
                                    vk::ImageHelper *src,
                                    const CopyImageToBufferParameters &params);

    angle::Result copyRgbToRgba(ContextVk *contextVk,
                                const angle::Format &srcFormat,
                                vk::BufferHelper *srcBuffer,
                                uint32_t srcOffset,
                                uint32_t pixelCount,
                                vk::BufferHelper *dstBuffer);

    angle::Result transCodeEtcToBc(ContextVk *contextVk,
                                   vk::BufferHelper *srcBuffer,
                                   vk::ImageHelper *dstImage,
                                   const VkBufferImageCopy *copyRegion);

    using GenerateMipmapDestLevelViews =
        std::array<const vk::ImageView *, kGenerateMipmapMaxLevels>;
    angle::Result generateMipmap(ContextVk *contextVk,
                                 vk::ImageHelper *src,
                                 const vk::ImageView *srcLevelZeroView,
                                 vk::ImageHelper *dst,
                                 const GenerateMipmapDestLevelViews &dstLevelViews,
                                 const vk::Sampler &sampler,
                                 const GenerateMipmapParameters &params);
    angle::Result generateMipmapWithDraw(ContextVk *contextVk,
                                         vk::ImageHelper *image,
                                         const angle::FormatID actualFormatID,
                                         const bool isMipmapFiltered);

    angle::Result unresolve(ContextVk *contextVk,
                            const FramebufferVk *framebuffer,
                            const UnresolveParameters &params);

    // Overlay utilities.
    angle::Result drawOverlay(ContextVk *contextVk,
                              vk::BufferHelper *textWidgetsBuffer,
                              vk::BufferHelper *graphWidgetsBuffer,
                              vk::ImageHelper *font,
                              const vk::ImageView *fontView,
                              vk::ImageHelper *dst,
                              const vk::ImageView *dstView,
                              const OverlayDrawParameters &params);

    // Fragment shading rate utility
    angle::Result generateFragmentShadingRate(
        ContextVk *contextVk,
        vk::ImageHelper *shadingRateAttachmentImageHelper,
        vk::ImageViewHelper *shadingRateAttachmentImageViewHelper,
        const GenerateFragmentShadingRateParameters &shadingRateParameters);

  private:
    ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

    struct ConvertIndexShaderParams
    {
        uint32_t srcOffset     = 0;
        uint32_t dstOffsetDiv4 = 0;
        uint32_t maxIndex      = 0;
        uint32_t _padding      = 0;
    };

    struct ConvertIndexIndirectShaderParams
    {
        uint32_t srcIndirectOffsetDiv4 = 0;
        uint32_t srcOffset             = 0;
        uint32_t dstOffsetDiv4         = 0;
        uint32_t maxIndex              = 0;
        uint32_t dstIndirectOffsetDiv4 = 0;
    };

    struct ConvertIndexIndirectLineLoopShaderParams
    {
        uint32_t cmdOffsetDiv4    = 0;
        uint32_t dstCmdOffsetDiv4 = 0;
        uint32_t srcOffset        = 0;
        uint32_t dstOffsetDiv4    = 0;
        uint32_t isRestartEnabled = 0;
    };

    struct ConvertIndirectLineLoopShaderParams
    {
        uint32_t cmdOffsetDiv4    = 0;
        uint32_t dstCmdOffsetDiv4 = 0;
        uint32_t dstOffsetDiv4    = 0;
    };

    struct ConvertVertexShaderParams
    {
        ConvertVertexShaderParams();

        // Structure matching PushConstants in ConvertVertex.comp
        uint32_t outputCount      = 0;
        uint32_t componentCount   = 0;
        uint32_t srcOffset        = 0;
        uint32_t dstOffset        = 0;
        uint32_t Ns               = 0;
        uint32_t Bs               = 0;
        uint32_t Ss               = 0;
        uint32_t Es               = 0;
        uint32_t Nd               = 0;
        uint32_t Bd               = 0;
        uint32_t Sd               = 0;
        uint32_t Ed               = 0;
        uint32_t srcEmulatedAlpha = 0;
        uint32_t isSrcHDR         = 0;
        uint32_t isSrcA2BGR10     = 0;
        uint32_t _padding         = 0;
    };

    struct ImageClearShaderParams
    {
        // Structure matching PushConstants in ImageClear.frag
        VkClearColorValue clearValue = {};
        float clearDepth             = 0.0f;
    };

    struct ImageCopyShaderParams
    {
        ImageCopyShaderParams();

        // Structure matching PushConstants in ImageCopy.frag
        int32_t srcOffset[2]            = {};
        int32_t dstOffset[2]            = {};
        int32_t srcMip                  = 0;
        int32_t srcLayer                = 0;
        int32_t srcSampleCount          = 0;
        uint32_t flipX                  = 0;
        uint32_t flipY                  = 0;
        uint32_t premultiplyAlpha       = 0;
        uint32_t unmultiplyAlpha        = 0;
        uint32_t dstHasLuminance        = 0;
        uint32_t dstIsAlpha             = 0;
        uint32_t srcIsSRGB              = 0;
        uint32_t dstIsSRGB              = 0;
        uint32_t dstDefaultChannelsMask = 0;
        uint32_t rotateXY               = 0;
    };

    struct CopyImageToBufferShaderParams
    {
        // Structure matching PushConstants in CopyImageToBuffer.comp
        int32_t srcOffset[2]     = {};
        int32_t srcDepth         = 0;
        uint32_t reverseRowOrder = 0;
        uint32_t size[2]         = {};
        uint32_t outputOffset    = 0;
        uint32_t outputPitch     = 0;
        uint32_t isDstSnorm      = 0;
    };

    union BlitResolveOffset
    {
        int32_t resolve[2];
        float blit[2];
    };

    struct BlitResolveShaderParams
    {
        // Structure matching PushConstants in BlitResolve.frag
        BlitResolveOffset offset = {};
        float stretch[2]         = {};
        float invSrcExtent[2]    = {};
        int32_t srcLayer         = 0;
        int32_t samples          = 0;
        float invSamples         = 0;
        uint32_t outputMask      = 0;
        uint32_t flipX           = 0;
        uint32_t flipY           = 0;
        uint32_t rotateXY        = 0;
    };

    struct BlitResolveStencilNoExportShaderParams
    {
        // Structure matching PushConstants in BlitResolveStencilNoExport.comp
        BlitResolveOffset offset = {};
        float stretch[2]         = {};
        float invSrcExtent[2]    = {};
        int32_t srcLayer         = 0;
        int32_t srcWidth         = 0;
        int32_t blitArea[4]      = {};
        int32_t dstPitch         = 0;
        uint32_t flipX           = 0;
        uint32_t flipY           = 0;
        uint32_t rotateXY        = 0;
    };

    struct ExportStencilShaderParams
    {
        uint32_t bit = 0;
    };

    struct OverlayDrawShaderParams
    {
        // Structure matching PushConstants in OverlayDraw.vert and OverlayDraw.frag
        uint32_t viewportSize[2] = {};
        uint32_t isText          = 0;
        uint32_t rotateXY        = 0;
    };

    struct GenerateMipmapShaderParams
    {
        // Structure matching PushConstants in GenerateMipmap.comp
        float invSrcExtent[2] = {};
        uint32_t levelCount   = 0;
    };

    struct EtcToBcShaderParams
    {
        uint32_t offsetX;
        uint32_t offsetY;
        int32_t texelOffset;
        uint32_t width;
        uint32_t height;
        uint32_t alphaBits;
        uint32_t isSigned;
        uint32_t isEacRg;
    };

    ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

    // Functions implemented by the class:
    enum class Function
    {
        // Functions implemented in graphics
        ImageClear,
        ImageCopy,
        BlitResolve,
        Blit3DSrc,
        ExportStencil,
        OverlayDraw,
        // Note: unresolve is special as it has a different layout per attachment count.  Depth and
        // stencil each require a binding, so are counted separately.
        Unresolve1Attachment,
        Unresolve2Attachments,
        Unresolve3Attachments,
        Unresolve4Attachments,
        Unresolve5Attachments,
        Unresolve6Attachments,
        Unresolve7Attachments,
        Unresolve8Attachments,
        Unresolve9Attachments,
        Unresolve10Attachments,

        // Functions implemented in compute
        ComputeStartIndex,  // Special value to separate draw and dispatch functions.
        ConvertIndexBuffer = ComputeStartIndex,
        ConvertVertexBuffer,
        ClearTexture,
        BlitResolveStencilNoExport,
        ConvertIndexIndirectBuffer,
        ConvertIndexIndirectLineLoopBuffer,
        ConvertIndirectLineLoopBuffer,
        GenerateMipmap,
        TransCodeEtcToBc,
        CopyImageToBuffer,
        GenerateFragmentShadingRate,

        InvalidEnum,
        EnumCount = InvalidEnum,
    };

    struct GraphicsShaderProgramAndPipelines
    {
        vk::ShaderProgramHelper program;
        CompleteGraphicsPipelineCache pipelines;
    };
    struct ComputeShaderProgramAndPipelines
    {
        vk::ShaderProgramHelper program;
        ComputePipelineCache pipelines;
    };

    // Common functions that create the pipeline for the specified function, binds it and prepares
    // the draw/dispatch call.
    angle::Result setupComputeProgram(
        ContextVk *contextVk,
        Function function,
        const vk::ShaderModulePtr &csShader,
        ComputeShaderProgramAndPipelines *programAndPipelines,
        const VkDescriptorSet descriptorSet,
        const void *pushConstants,
        size_t pushConstantsSize,
        vk::OutsideRenderPassCommandBufferHelper *commandBufferHelper);
    angle::Result setupGraphicsProgramWithLayout(
        ContextVk *contextVk,
        const vk::PipelineLayout &pipelineLayout,
        const vk::ShaderModulePtr &vsShader,
        const vk::ShaderModulePtr &fsShader,
        GraphicsShaderProgramAndPipelines *programAndPipelines,
        const vk::GraphicsPipelineDesc *pipelineDesc,
        const VkDescriptorSet descriptorSet,
        const void *pushConstants,
        size_t pushConstantsSize,
        vk::RenderPassCommandBuffer *commandBuffer);
    angle::Result setupGraphicsProgram(ContextVk *contextVk,
                                       Function function,
                                       const vk::ShaderModulePtr &vsShader,
                                       const vk::ShaderModulePtr &fsShader,
                                       GraphicsShaderProgramAndPipelines *programAndPipelines,
                                       const vk::GraphicsPipelineDesc *pipelineDesc,
                                       const VkDescriptorSet descriptorSet,
                                       const void *pushConstants,
                                       size_t pushConstantsSize,
                                       vk::RenderPassCommandBuffer *commandBuffer);

    // Initializes descriptor set layout, pipeline layout and descriptor pool corresponding to given
    // function, if not already initialized.  Uses setSizes to create the layout.  For example, if
    // this array has two entries {STORAGE_TEXEL_BUFFER, 1} and {UNIFORM_TEXEL_BUFFER, 3}, then the
    // created set layout would be binding 0 for storage texel buffer and bindings 1 through 3 for
    // uniform texel buffer.  All resources are put in set 0.
    angle::Result ensureResourcesInitialized(ContextVk *contextVk,
                                             Function function,
                                             VkDescriptorPoolSize *setSizes,
                                             size_t setSizesCount,
                                             size_t pushConstantsSize);

    // Initializers corresponding to functions, calling into ensureResourcesInitialized with the
    // appropriate parameters.
    angle::Result ensureConvertIndexResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureConvertIndexIndirectResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureConvertIndexIndirectLineLoopResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureConvertIndirectLineLoopResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureConvertVertexResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureImageClearResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureImageCopyResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureCopyImageToBufferResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureBlitResolveResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureBlitResolveStencilNoExportResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureExportStencilResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureOverlayDrawResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureGenerateMipmapResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureTransCodeEtcToBcResourcesInitialized(ContextVk *contextVk);
    angle::Result ensureUnresolveResourcesInitialized(ContextVk *contextVk,
                                                      Function function,
                                                      uint32_t attachmentIndex);

    angle::Result ensureImageCopyResourcesInitializedWithSampler(
        ContextVk *contextVk,
        const vk::SamplerDesc &samplerDesc);

    angle::Result ensureSamplersInitialized(ContextVk *context);

    angle::Result ensureGenerateFragmentShadingRateResourcesInitialized(ContextVk *contextVk);

    angle::Result startRenderPass(ContextVk *contextVk,
                                  vk::ImageHelper *image,
                                  const vk::ImageView *imageView,
                                  const vk::RenderPassDesc &renderPassDesc,
                                  const gl::Rectangle &renderArea,
                                  const VkImageAspectFlags aspectFlags,
                                  const VkClearValue *clearValue,
                                  vk::RenderPassSource renderPassSource,
                                  vk::RenderPassCommandBuffer **commandBufferOut);

    // Set up descriptor set and call dispatch.
    angle::Result convertVertexBufferImpl(
        ContextVk *contextVk,
        vk::BufferHelper *dst,
        vk::BufferHelper *src,
        uint32_t flags,
        vk::OutsideRenderPassCommandBufferHelper *commandBufferHelper,
        const ConvertVertexShaderParams &shaderParams,
        const OffsetAndVertexCounts &additionalOffsetVertexCounts);

    // Blits or resolves either color or depth/stencil, based on which view is given.
    angle::Result blitResolveImpl(ContextVk *contextVk,
                                  FramebufferVk *framebuffer,
                                  vk::ImageHelper *src,
                                  const vk::ImageView *srcColorView,
                                  const vk::ImageView *srcDepthView,
                                  const vk::ImageView *srcStencilView,
                                  const BlitResolveParameters &params);

    // Allocates a single descriptor set.
    angle::Result allocateDescriptorSetWithLayout(
        ContextVk *contextVk,
        vk::CommandBufferHelperCommon *commandBufferHelper,
        vk::DynamicDescriptorPool &descriptorPool,
        const vk::DescriptorSetLayout &descriptorSetLayout,
        VkDescriptorSet *descriptorSetOut);

    angle::Result allocateDescriptorSet(ContextVk *contextVk,
                                        vk::CommandBufferHelperCommon *commandBufferHelper,
                                        Function function,
                                        VkDescriptorSet *descriptorSetOut);

    angle::Result allocateDescriptorSetForImageCopyWithSampler(
        ContextVk *contextVk,
        vk::CommandBufferHelperCommon *commandBufferHelper,
        const vk::SamplerDesc &samplerDesc,
        VkDescriptorSet *descriptorSetOut);

    angle::PackedEnumMap<Function, vk::DescriptorSetLayoutPointerArray> mDescriptorSetLayouts;
    angle::PackedEnumMap<Function, vk::PipelineLayoutPtr> mPipelineLayouts;
    angle::PackedEnumMap<Function, vk::DynamicDescriptorPool> mDescriptorPools;

    std::unordered_map<vk::SamplerDesc, vk::DescriptorSetLayoutPointerArray>
        mImageCopyWithSamplerDescriptorSetLayouts;
    std::unordered_map<vk::SamplerDesc, vk::PipelineLayoutPtr> mImageCopyWithSamplerPipelineLayouts;
    std::unordered_map<vk::SamplerDesc, vk::DynamicDescriptorPool>
        mImageCopyWithSamplerDescriptorPools;

    ComputeShaderProgramAndPipelines
        mConvertIndex[vk::InternalShader::ConvertIndex_comp::kArrayLen];
    ComputeShaderProgramAndPipelines mConvertIndexIndirectLineLoop
        [vk::InternalShader::ConvertIndexIndirectLineLoop_comp::kArrayLen];
    ComputeShaderProgramAndPipelines
        mConvertIndirectLineLoop[vk::InternalShader::ConvertIndirectLineLoop_comp::kArrayLen];
    ComputeShaderProgramAndPipelines
        mConvertVertex[vk::InternalShader::ConvertVertex_comp::kArrayLen];
    GraphicsShaderProgramAndPipelines mImageClearVSOnly;
    GraphicsShaderProgramAndPipelines mImageClear[vk::InternalShader::ImageClear_frag::kArrayLen];
    GraphicsShaderProgramAndPipelines mImageCopy[vk::InternalShader::ImageCopy_frag::kArrayLen];
    GraphicsShaderProgramAndPipelines mImageCopyFloat;
    std::unordered_map<vk::SamplerDesc, GraphicsShaderProgramAndPipelines> mImageCopyWithSampler;
    ComputeShaderProgramAndPipelines
        mCopyImageToBuffer[vk::InternalShader::CopyImageToBuffer_comp::kArrayLen];
    GraphicsShaderProgramAndPipelines mBlitResolve[vk::InternalShader::BlitResolve_frag::kArrayLen];
    GraphicsShaderProgramAndPipelines mBlit3DSrc[vk::InternalShader::Blit3DSrc_frag::kArrayLen];
    ComputeShaderProgramAndPipelines
        mBlitResolveStencilNoExport[vk::InternalShader::BlitResolveStencilNoExport_comp::kArrayLen];
    GraphicsShaderProgramAndPipelines mExportStencil;
    GraphicsShaderProgramAndPipelines mOverlayDraw;
    ComputeShaderProgramAndPipelines
        mGenerateMipmap[vk::InternalShader::GenerateMipmap_comp::kArrayLen];
    ComputeShaderProgramAndPipelines mEtcToBc[vk::InternalShader::EtcToBc_comp::kArrayLen];

    // Unresolve shaders are special as they are generated on the fly due to the large number of
    // combinations.
    std::unordered_map<uint32_t, vk::ShaderModulePtr> mUnresolveFragShaders;
    std::unordered_map<uint32_t, GraphicsShaderProgramAndPipelines> mUnresolve;

    ComputeShaderProgramAndPipelines mGenerateFragmentShadingRateAttachment;

    vk::Sampler mPointSampler;
    vk::Sampler mLinearSampler;
};

// This class' responsibility is to create index buffers needed to support line loops in Vulkan.
// In the setup phase of drawing, the createIndexBuffer method should be called with the
// current draw call parameters. If an element array buffer is bound for an indexed draw, use
// createIndexBufferFromElementArrayBuffer.
//
// If the user wants to draw a loop between [v1, v2, v3], we will create an indexed buffer with
// these indexes: [0, 1, 2, 3, 0] to emulate the loop.
class LineLoopHelper final : angle::NonCopyable
{
  public:
    LineLoopHelper(vk::Renderer *renderer);
    ~LineLoopHelper();

    angle::Result getIndexBufferForDrawArrays(ContextVk *contextVk,
                                              uint32_t clampedVertexCount,
                                              GLint firstVertex,
                                              vk::BufferHelper **bufferOut);

    angle::Result getIndexBufferForElementArrayBuffer(ContextVk *contextVk,
                                                      BufferVk *elementArrayBufferVk,
                                                      gl::DrawElementsType glIndexType,
                                                      int indexCount,
                                                      intptr_t elementArrayOffset,
                                                      vk::BufferHelper **bufferOut,
                                                      uint32_t *indexCountOut);

    angle::Result streamIndices(ContextVk *contextVk,
                                gl::DrawElementsType glIndexType,
                                GLsizei indexCount,
                                const uint8_t *srcPtr,
                                vk::BufferHelper **bufferOut,
                                uint32_t *indexCountOut);

    angle::Result streamIndicesIndirect(ContextVk *contextVk,
                                        gl::DrawElementsType glIndexType,
                                        vk::BufferHelper *indexBuffer,
                                        vk::BufferHelper *indirectBuffer,
                                        VkDeviceSize indirectBufferOffset,
                                        vk::BufferHelper **indexBufferOut,
                                        vk::BufferHelper **indirectBufferOut);

    angle::Result streamArrayIndirect(ContextVk *contextVk,
                                      size_t vertexCount,
                                      vk::BufferHelper *arrayIndirectBuffer,
                                      VkDeviceSize arrayIndirectBufferOffset,
                                      vk::BufferHelper **indexBufferOut,
                                      vk::BufferHelper **indexIndirectBufferOut);

    void release(ContextVk *contextVk);
    void destroy(vk::Renderer *renderer);

    vk::BufferHelper *getCurrentIndexBuffer() { return mDynamicIndexBuffer.getBuffer(); }

    static void Draw(uint32_t count,
                     uint32_t baseVertex,
                     vk::RenderPassCommandBuffer *commandBuffer)
    {
        // Our first index is always 0 because that's how we set it up in createIndexBuffer*.
        commandBuffer->drawIndexedBaseVertex(count, baseVertex);
    }

  private:
    ConversionBuffer mDynamicIndexBuffer;
    ConversionBuffer mDynamicIndirectBuffer;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_UTILSVK_H_
