//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_render_utils.h:
//    Defines the class interface for RenderUtils, which contains many utility functions and shaders
//    for converting, blitting, copying as well as generating data, and many more.
//

#ifndef LIBANGLE_RENDERER_METAL_MTL_RENDER_UTILS_H_
#define LIBANGLE_RENDERER_METAL_MTL_RENDER_UTILS_H_

#import <Metal/Metal.h>
#include <unordered_map>

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/metal/RenderTargetMtl.h"
#include "libANGLE/renderer/metal/mtl_command_buffer.h"
#include "libANGLE/renderer/metal/mtl_context_device.h"
#include "libANGLE/renderer/metal/mtl_state_cache.h"
#include "libANGLE/renderer/metal/shaders/constants.h"

namespace rx
{

class BufferMtl;
class ContextMtl;
class DisplayMtl;
class VisibilityBufferOffsetsMtl;

namespace mtl
{

struct ClearRectParams
{
    ClearRectParams() { clearWriteMaskArray.fill(MTLColorWriteMaskAll); }

    Optional<ClearColorValue> clearColor;
    Optional<float> clearDepth;
    Optional<uint32_t> clearStencil;

    WriteMaskArray clearWriteMaskArray;

    const mtl::Format *colorFormat = nullptr;
    gl::Extents dstTextureSize;

    // Only clear enabled buffers
    gl::DrawBufferMask enabledBuffers;
    gl::Rectangle clearArea;

    bool flipY = false;
};

struct NormalizedCoords
{
    NormalizedCoords();
    NormalizedCoords(float x, float y, float width, float height, const gl::Rectangle &rect);
    NormalizedCoords(const gl::Rectangle &rect, const gl::Extents &extents);
    float v[4];
};

struct BlitParams
{
    gl::Extents dstTextureSize;
    gl::Rectangle dstRect;
    gl::Rectangle dstScissorRect;
    // Destination texture needs to have viewport Y flipped?
    // The difference between this param and unpackFlipY is that unpackFlipY is from
    // glCopyImageCHROMIUM()/glBlitFramebuffer(), and dstFlipY controls whether the final viewport
    // needs to be flipped when drawing to destination texture. It is possible to combine the two
    // flags before passing to RenderUtils. However, to avoid duplicated works, just pass the two
    // flags to RenderUtils, they will be combined internally by RenderUtils logic.
    bool dstFlipY = false;
    bool dstFlipX = false;

    TextureRef src;
    MipmapNativeLevel srcLevel = kZeroNativeMipLevel;
    uint32_t srcLayer          = 0;

    // Source rectangle:
    // NOTE: if srcYFlipped=true, this rectangle will be converted internally to flipped rect before
    // blitting.
    NormalizedCoords srcNormalizedCoords;

    bool srcYFlipped = false;  // source texture has data flipped in Y direction
    bool unpackFlipX = false;  // flip texture data copying process in X direction
    bool unpackFlipY = false;  // flip texture data copying process in Y direction
};

struct ColorBlitParams : public BlitParams
{
    ColorBlitParams() {}

    gl::DrawBufferMask enabledBuffers;
    GLenum filter               = GL_NEAREST;
    bool unpackPremultiplyAlpha = false;
    bool unpackUnmultiplyAlpha  = false;
    bool transformLinearToSrgb  = false;
    bool dstLuminance           = false;
};

struct DepthStencilBlitParams : public BlitParams
{
    TextureRef srcStencil;
};

// Stencil blit via an intermediate buffer. NOTE: source depth texture parameter is ignored.
// See DepthStencilBlitUtils::blitStencilViaCopyBuffer()
struct StencilBlitViaBufferParams : public DepthStencilBlitParams
{
    StencilBlitViaBufferParams();
    StencilBlitViaBufferParams(const DepthStencilBlitParams &src);

    TextureRef dstStencil;
    MipmapNativeLevel dstStencilLevel = kZeroNativeMipLevel;
    uint32_t dstStencilLayer          = 0;
    bool dstPackedDepthStencilFormat  = false;
};

struct TriFanOrLineLoopFromArrayParams
{
    uint32_t firstVertex;
    uint32_t vertexCount;
    BufferRef dstBuffer;
    // Must be multiples of kIndexBufferOffsetAlignment
    uint32_t dstOffset;
};

struct IndexConversionParams
{

    gl::DrawElementsType srcType;
    uint32_t indexCount;
    const BufferRef &srcBuffer;
    uint32_t srcOffset;
    const BufferRef &dstBuffer;
    // Must be multiples of kIndexBufferOffsetAlignment
    uint32_t dstOffset;
    bool primitiveRestartEnabled = false;
};

struct IndexGenerationParams
{
    gl::DrawElementsType srcType;
    GLsizei indexCount;
    const void *indices;
    BufferRef dstBuffer;
    uint32_t dstOffset;
    bool primitiveRestartEnabled = false;
};

struct CopyPixelsCommonParams
{
    BufferRef buffer;
    uint32_t bufferStartOffset = 0;
    uint32_t bufferRowPitch    = 0;

    TextureRef texture;
};

struct CopyPixelsFromBufferParams : CopyPixelsCommonParams
{
    uint32_t bufferDepthPitch = 0;

    // z offset is:
    //  - slice index if texture is array.
    //  - depth if texture is 3d.
    gl::Box textureArea;
};

struct CopyPixelsToBufferParams : CopyPixelsCommonParams
{
    gl::Rectangle textureArea;
    MipmapNativeLevel textureLevel = kZeroNativeMipLevel;
    uint32_t textureSliceOrDeph    = 0;
    bool reverseTextureRowOrder;
};

struct VertexFormatConvertParams
{
    BufferRef srcBuffer;
    uint32_t srcBufferStartOffset = 0;
    uint32_t srcStride            = 0;
    uint32_t srcDefaultAlphaData  = 0;  // casted as uint

    BufferRef dstBuffer;
    uint32_t dstBufferStartOffset = 0;
    uint32_t dstStride            = 0;
    uint32_t dstComponents        = 0;

    uint32_t vertexCount = 0;
};

struct BlockLinearizationParams
{
    BufferRef srcBuffer;
    BufferRef dstBuffer;
    uint32_t srcBufferOffset;
    uint32_t blocksWide;
    uint32_t blocksHigh;
};

struct DepthSaturationParams
{
    BufferRef srcBuffer;
    BufferRef dstBuffer;
    uint32_t srcBufferOffset;
    uint32_t dstWidth;
    uint32_t dstHeight;
    uint32_t srcPitch;
};

// Utils class for clear & blitting
class ClearUtils final : angle::NonCopyable
{
  public:
    ClearUtils() = delete;
    ClearUtils(const std::string &fragmentShaderName);

    // Clear current framebuffer
    angle::Result clearWithDraw(const gl::Context *context,
                                RenderCommandEncoder *cmdEncoder,
                                const ClearRectParams &params);

  private:
    angle::Result ensureShadersInitialized(ContextMtl *ctx, uint32_t numColorAttachments);

    angle::Result setupClearWithDraw(const gl::Context *context,
                                     RenderCommandEncoder *cmdEncoder,
                                     const ClearRectParams &params);
    id<MTLDepthStencilState> getClearDepthStencilState(const gl::Context *context,
                                                       const ClearRectParams &params);
    angle::Result getClearRenderPipelineState(
        const gl::Context *context,
        RenderCommandEncoder *cmdEncoder,
        const ClearRectParams &params,
        AutoObjCPtr<id<MTLRenderPipelineState>> *outPipelineState);

    const std::string mFragmentShaderName;

    AutoObjCPtr<id<MTLFunction>> mVertexShader;
    std::array<AutoObjCPtr<id<MTLFunction>>, kMaxRenderTargets + 1> mFragmentShaders;
};

class ColorBlitUtils final : angle::NonCopyable
{
  public:
    ColorBlitUtils() = delete;
    ColorBlitUtils(const std::string &fragmentShaderName);

    // Blit texture data to current framebuffer
    angle::Result blitColorWithDraw(const gl::Context *context,
                                    RenderCommandEncoder *cmdEncoder,
                                    const ColorBlitParams &params);

  private:
    struct ShaderKey
    {
        uint32_t numColorAttachments = 0;
        int sourceTextureType        = 0;
        bool unmultiplyAlpha         = false;
        bool premultiplyAlpha        = false;
        bool transformLinearToSrgb   = false;
        bool operator==(const ShaderKey &other) const
        {
            return numColorAttachments == other.numColorAttachments &&
                   unmultiplyAlpha == other.unmultiplyAlpha &&
                   premultiplyAlpha == other.premultiplyAlpha &&
                   transformLinearToSrgb == other.transformLinearToSrgb &&
                   sourceTextureType == other.sourceTextureType;
        }
        struct Hash
        {
            size_t operator()(const ShaderKey &k) const noexcept
            {
                return angle::HashMultiple(k.numColorAttachments, k.unmultiplyAlpha,
                                           k.premultiplyAlpha, k.sourceTextureType);
            }
        };
    };
    angle::Result ensureShadersInitialized(ContextMtl *ctx,
                                           const ShaderKey &key,
                                           AutoObjCPtr<id<MTLFunction>> *fragmentShaderOut);

    angle::Result setupColorBlitWithDraw(const gl::Context *context,
                                         RenderCommandEncoder *cmdEncoder,
                                         const ColorBlitParams &params);

    angle::Result getColorBlitRenderPipelineState(
        const gl::Context *context,
        RenderCommandEncoder *cmdEncoder,
        const ColorBlitParams &params,
        AutoObjCPtr<id<MTLRenderPipelineState>> *outPipelineState);

    const std::string mFragmentShaderName;

    AutoObjCPtr<id<MTLFunction>> mVertexShader;

    // Blit fragment shaders.
    std::unordered_map<ShaderKey, AutoObjCPtr<id<MTLFunction>>, ShaderKey::Hash>
        mBlitFragmentShaders;
};

class DepthStencilBlitUtils final : angle::NonCopyable
{
  public:
    angle::Result blitDepthStencilWithDraw(const gl::Context *context,
                                           RenderCommandEncoder *cmdEncoder,
                                           const DepthStencilBlitParams &params);

    // Blit stencil data using intermediate buffer. This function is used on devices with no
    // support for direct stencil write in shader. Thus an intermediate buffer storing copied
    // stencil data is needed.
    // NOTE: this function shares the params struct with depth & stencil blit, but depth texture
    // parameter is not used. This function will break existing render pass.
    angle::Result blitStencilViaCopyBuffer(const gl::Context *context,
                                           const StencilBlitViaBufferParams &params);

  private:
    angle::Result ensureShadersInitialized(ContextMtl *ctx,
                                           int sourceDepthTextureType,
                                           int sourceStencilTextureType,
                                           AutoObjCPtr<id<MTLFunction>> *fragmentShaderOut);

    angle::Result setupDepthStencilBlitWithDraw(const gl::Context *context,
                                                RenderCommandEncoder *cmdEncoder,
                                                const DepthStencilBlitParams &params);

    angle::Result getDepthStencilBlitRenderPipelineState(
        const gl::Context *context,
        RenderCommandEncoder *cmdEncoder,
        const DepthStencilBlitParams &params,
        AutoObjCPtr<id<MTLRenderPipelineState>> *outRenderPipelineState);

    angle::Result getStencilToBufferComputePipelineState(
        ContextMtl *ctx,
        const StencilBlitViaBufferParams &params,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipelineState);

    AutoObjCPtr<id<MTLFunction>> mVertexShader;

    std::array<AutoObjCPtr<id<MTLFunction>>, mtl_shader::kTextureTypeCount>
        mDepthBlitFragmentShaders;
    std::array<AutoObjCPtr<id<MTLFunction>>, mtl_shader::kTextureTypeCount>
        mStencilBlitFragmentShaders;
    std::array<std::array<AutoObjCPtr<id<MTLFunction>>, mtl_shader::kTextureTypeCount>,
               mtl_shader::kTextureTypeCount>
        mDepthStencilBlitFragmentShaders;

    std::array<AutoObjCPtr<id<MTLFunction>>, mtl_shader::kTextureTypeCount>
        mStencilBlitToBufferComputeShaders;

    // Intermediate buffer for storing copied stencil data. Used when device doesn't support
    // writing stencil in shader.
    BufferRef mStencilCopyBuffer;
};

// util class for generating index buffer
class IndexGeneratorUtils final : angle::NonCopyable
{
  public:
    angle::Result convertIndexBufferGPU(ContextMtl *contextMtl,
                                        const IndexConversionParams &params);
    angle::Result generateTriFanBufferFromArrays(ContextMtl *contextMtl,
                                                 const TriFanOrLineLoopFromArrayParams &params);
    // Generate triangle fan index buffer for glDrawElements().
    angle::Result generateTriFanBufferFromElementsArray(ContextMtl *contextMtl,
                                                        const IndexGenerationParams &params,
                                                        uint32_t *indicesGenerated);

    angle::Result generateLineLoopBufferFromArrays(ContextMtl *contextMtl,
                                                   const TriFanOrLineLoopFromArrayParams &params);
    angle::Result generateLineLoopLastSegment(ContextMtl *contextMtl,
                                              uint32_t firstVertex,
                                              uint32_t lastVertex,
                                              const BufferRef &dstBuffer,
                                              uint32_t dstOffset);
    // Generate line loop index buffer for glDrawElements().
    // Destination buffer must have at least 2x the number of original indices if primitive restart
    // is enabled.
    angle::Result generateLineLoopBufferFromElementsArray(ContextMtl *contextMtl,
                                                          const IndexGenerationParams &params,
                                                          uint32_t *indicesGenerated);
    // Generate line loop's last segment index buffer for glDrawElements().
    // NOTE: this function assumes primitive restart is not enabled.
    angle::Result generateLineLoopLastSegmentFromElementsArray(ContextMtl *contextMtl,
                                                               const IndexGenerationParams &params);

    angle::Result generatePrimitiveRestartPointsBuffer(ContextMtl *contextMtl,
                                                       const IndexGenerationParams &params,
                                                       size_t *indicesGenerated);

    angle::Result generatePrimitiveRestartLinesBuffer(ContextMtl *contextMtl,
                                                      const IndexGenerationParams &params,
                                                      size_t *indicesGenerated);

    angle::Result generatePrimitiveRestartTrianglesBuffer(ContextMtl *contextMtl,
                                                          const IndexGenerationParams &params,
                                                          size_t *indicesGenerated);

  private:
    // Index generator compute shaders:
    //  - First dimension: index type.
    //  - second dimension: source buffer's offset is aligned or not.
    using IndexConversionShaderArray = std::array<std::array<AutoObjCPtr<id<MTLFunction>>, 2>,
                                                  angle::EnumSize<gl::DrawElementsType>()>;

    angle::Result getIndexConversionPipeline(
        ContextMtl *contextMtl,
        gl::DrawElementsType srcType,
        uint32_t srcOffset,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    // Get compute pipeline to generate tri fan/line loop index for glDrawElements().
    angle::Result getIndicesFromElemArrayGeneratorPipeline(
        ContextMtl *contextMtl,
        gl::DrawElementsType srcType,
        uint32_t srcOffset,
        NSString *shaderName,
        IndexConversionShaderArray *pipelineCacheArray,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    // Defer loading of compute pipeline to generate tri fan index for glDrawArrays().
    angle::Result getTriFanFromArrayGeneratorPipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    // Defer loading of compute pipeline to generate line loop index for glDrawArrays().
    angle::Result getLineLoopFromArrayGeneratorPipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);

    angle::Result generateTriFanBufferFromElementsArrayGPU(
        ContextMtl *contextMtl,
        gl::DrawElementsType srcType,
        uint32_t indexCount,
        const BufferRef &srcBuffer,
        uint32_t srcOffset,
        const BufferRef &dstBuffer,
        // Must be multiples of kIndexBufferOffsetAlignment
        uint32_t dstOffset);
    angle::Result generateTriFanBufferFromElementsArrayCPU(ContextMtl *contextMtl,
                                                           const IndexGenerationParams &params,
                                                           uint32_t *indicesGenerated);

    angle::Result generateLineLoopBufferFromElementsArrayGPU(
        ContextMtl *contextMtl,
        gl::DrawElementsType srcType,
        uint32_t indexCount,
        const BufferRef &srcBuffer,
        uint32_t srcOffset,
        const BufferRef &dstBuffer,
        // Must be multiples of kIndexBufferOffsetAlignment
        uint32_t dstOffset);
    angle::Result generateLineLoopBufferFromElementsArrayCPU(ContextMtl *contextMtl,
                                                             const IndexGenerationParams &params,
                                                             uint32_t *indicesGenerated);
    angle::Result generateLineLoopLastSegmentFromElementsArrayCPU(
        ContextMtl *contextMtl,
        const IndexGenerationParams &params);

    angle::Result generatePrimitiveRestartBuffer(ContextMtl *contextMtl,
                                                 unsigned numVerticesPerPrimitive,
                                                 const IndexGenerationParams &params,
                                                 size_t *indicesGenerated);

    IndexConversionShaderArray mIndexConversionShaders;

    IndexConversionShaderArray mTriFanFromElemArrayGeneratorShaders;
    AutoObjCPtr<id<MTLFunction>> mTriFanFromArraysGeneratorShader;

    IndexConversionShaderArray mLineLoopFromElemArrayGeneratorShaders;
    AutoObjCPtr<id<MTLFunction>> mLineLoopFromArraysGeneratorShader;
};

// Util class for handling visibility query result
class VisibilityResultUtils final : angle::NonCopyable
{
  public:
    angle::Result combineVisibilityResult(
        ContextMtl *contextMtl,
        bool keepOldValue,
        const VisibilityBufferOffsetsMtl &renderPassResultBufOffsets,
        const BufferRef &renderPassResultBuf,
        const BufferRef &finalResultBuf);

  private:
    angle::Result getVisibilityResultCombinePipeline(
        ContextMtl *contextMtl,
        bool keepOldValue,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    // Visibility combination compute shaders:
    // - 0: This compute shader only combines the new values and discard old value.
    // - 1: This compute shader keep the old value and combines with new values.
    std::array<AutoObjCPtr<id<MTLFunction>>, 2> mVisibilityResultCombineComputeShaders;
};

// Util class for handling mipmap generation
class MipmapUtils final : angle::NonCopyable
{
  public:
    // Compute based mipmap generation.
    angle::Result generateMipmapCS(ContextMtl *contextMtl,
                                   const TextureRef &srcTexture,
                                   bool sRGBMipmap,
                                   NativeTexLevelArray *mipmapOutputViews);

  private:
    angle::Result get3DMipGeneratorPipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    angle::Result get2DMipGeneratorPipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    angle::Result get2DArrayMipGeneratorPipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    angle::Result getCubeMipGeneratorPipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);

    // Mipmaps generating compute pipeline:
    AutoObjCPtr<id<MTLFunction>> m3DMipGeneratorShader;
    AutoObjCPtr<id<MTLFunction>> m2DMipGeneratorShader;
    AutoObjCPtr<id<MTLFunction>> m2DArrayMipGeneratorShader;
    AutoObjCPtr<id<MTLFunction>> mCubeMipGeneratorShader;
};

// Util class for handling pixels copy between buffers and textures
class CopyPixelsUtils final : angle::NonCopyable
{
  public:
    CopyPixelsUtils() = default;
    CopyPixelsUtils(const std::string &readShaderName, const std::string &writeShaderName);

    angle::Result unpackPixelsFromBufferToTexture(ContextMtl *contextMtl,
                                                  const angle::Format &srcAngleFormat,
                                                  const CopyPixelsFromBufferParams &params);
    angle::Result packPixelsFromTextureToBuffer(ContextMtl *contextMtl,
                                                const angle::Format &dstAngleFormat,
                                                const CopyPixelsToBufferParams &params);

  private:
    angle::Result getPixelsCopyPipeline(
        ContextMtl *contextMtl,
        const angle::Format &angleFormat,
        const TextureRef &texture,
        bool bufferWrite,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);
    // Copy pixels between buffer and texture compute pipelines:
    // - First dimension: pixel format.
    // - Second dimension: texture type * (buffer read/write flag)
    using PixelsCopyComputeShaderArray =
        std::array<std::array<AutoObjCPtr<id<MTLFunction>>, mtl_shader::kTextureTypeCount * 2>,
                   angle::kNumANGLEFormats>;
    PixelsCopyComputeShaderArray mPixelsCopyComputeShaders;

    const std::string mReadShaderName;
    const std::string mWriteShaderName;
};

// Util class for handling vertex format conversion on GPU
class VertexFormatConversionUtils final : angle::NonCopyable
{
  public:
    // Convert vertex format to float. Compute shader version.
    angle::Result convertVertexFormatToFloatCS(ContextMtl *contextMtl,
                                               const angle::Format &srcAngleFormat,
                                               const VertexFormatConvertParams &params);
    // Convert vertex format to float. Vertex shader version. This version should be used if
    // a render pass is active and we don't want to break it. Explicit memory barrier must be
    // supported.
    angle::Result convertVertexFormatToFloatVS(const gl::Context *context,
                                               RenderCommandEncoder *renderEncoder,
                                               const angle::Format &srcAngleFormat,
                                               const VertexFormatConvertParams &params);
    // Expand number of components per vertex's attribute (or just simply copy components between
    // buffers with different stride and offset)
    angle::Result expandVertexFormatComponentsCS(ContextMtl *contextMtl,
                                                 const angle::Format &srcAngleFormat,
                                                 const VertexFormatConvertParams &params);
    angle::Result expandVertexFormatComponentsVS(const gl::Context *context,
                                                 RenderCommandEncoder *renderEncoder,
                                                 const angle::Format &srcAngleFormat,
                                                 const VertexFormatConvertParams &params);

  private:
    angle::Result getComponentsExpandComputePipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outPipelineState);
    angle::Result getComponentsExpandRenderPipeline(
        ContextMtl *contextMtl,
        RenderCommandEncoder *renderEncoder,
        AutoObjCPtr<id<MTLRenderPipelineState>> *outPipelineState);

    angle::Result getFloatConverstionComputePipeline(
        ContextMtl *contextMtl,
        const angle::Format &srcAngleFormat,
        AutoObjCPtr<id<MTLComputePipelineState>> *outPipelineState);

    angle::Result getFloatConverstionRenderPipeline(
        ContextMtl *contextMtl,
        RenderCommandEncoder *renderEncoder,
        const angle::Format &srcAngleFormat,
        AutoObjCPtr<id<MTLRenderPipelineState>> *outPipelineState);

    template <typename EncoderType, typename PipelineType>
    angle::Result setupCommonConvertVertexFormatToFloat(ContextMtl *contextMtl,
                                                        EncoderType cmdEncoder,
                                                        const PipelineType &pipeline,
                                                        const angle::Format &srcAngleFormat,
                                                        const VertexFormatConvertParams &params);
    template <typename EncoderType, typename PipelineType>
    angle::Result setupCommonExpandVertexFormatComponents(ContextMtl *contextMtl,
                                                          EncoderType cmdEncoder,
                                                          const PipelineType &pipeline,
                                                          const angle::Format &srcAngleFormat,
                                                          const VertexFormatConvertParams &params);

    using ConvertToFloatComputeShaderArray =
        std::array<AutoObjCPtr<id<MTLFunction>>, angle::kNumANGLEFormats>;
    using ConvertToFloatVertexShaderArray =
        std::array<AutoObjCPtr<id<MTLFunction>>, angle::kNumANGLEFormats>;

    ConvertToFloatComputeShaderArray mConvertToFloatCompPipelineCaches;
    ConvertToFloatVertexShaderArray mConvertToFloatVertexShaders;

    AutoObjCPtr<id<MTLFunction>> mComponentsExpandComputeShader;
    AutoObjCPtr<id<MTLFunction>> mComponentsExpandVertexShader;
};

// Util class for linearizing PVRTC1 data for buffer to texture uploads
class BlockLinearizationUtils final : angle::NonCopyable
{
  public:
    angle::Result linearizeBlocks(ContextMtl *contextMtl, const BlockLinearizationParams &params);

  private:
    angle::Result getBlockLinearizationComputePipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);

    AutoObjCPtr<id<MTLFunction>> mLinearizeBlocksComputeShader;
};

// Util class for saturating floating-pont depth data for texture uploads
class DepthSaturationUtils final : angle::NonCopyable
{
  public:
    angle::Result saturateDepth(ContextMtl *contextMtl, const DepthSaturationParams &params);

  private:
    angle::Result getDepthSaturationComputePipeline(
        ContextMtl *contextMtl,
        AutoObjCPtr<id<MTLComputePipelineState>> *outComputePipeline);

    AutoObjCPtr<id<MTLFunction>> mSaturateDepthComputeShader;
};

// RenderUtils: container class of various util classes above
class RenderUtils : angle::NonCopyable
{
  public:
    RenderUtils();

    // Clear current framebuffer
    angle::Result clearWithDraw(const gl::Context *context,
                                RenderCommandEncoder *cmdEncoder,
                                const ClearRectParams &params);
    // Blit texture data to current framebuffer
    angle::Result blitColorWithDraw(const gl::Context *context,
                                    RenderCommandEncoder *cmdEncoder,
                                    const angle::Format &srcAngleFormat,
                                    const ColorBlitParams &params);
    // Same as above but blit the whole texture to the whole of current framebuffer.
    // This function assumes the framebuffer and the source texture have same size.
    angle::Result blitColorWithDraw(const gl::Context *context,
                                    RenderCommandEncoder *cmdEncoder,
                                    const angle::Format &srcAngleFormat,
                                    const TextureRef &srcTexture);
    angle::Result copyTextureWithDraw(const gl::Context *context,
                                      RenderCommandEncoder *cmdEncoder,
                                      const angle::Format &srcAngleFormat,
                                      const angle::Format &dstAngleFormat,
                                      const ColorBlitParams &params);

    angle::Result blitDepthStencilWithDraw(const gl::Context *context,
                                           RenderCommandEncoder *cmdEncoder,
                                           const DepthStencilBlitParams &params);
    // See DepthStencilBlitUtils::blitStencilViaCopyBuffer()
    angle::Result blitStencilViaCopyBuffer(const gl::Context *context,
                                           const StencilBlitViaBufferParams &params);

    // See IndexGeneratorUtils
    angle::Result convertIndexBufferGPU(ContextMtl *contextMtl,
                                        const IndexConversionParams &params);
    angle::Result generateTriFanBufferFromArrays(ContextMtl *contextMtl,
                                                 const TriFanOrLineLoopFromArrayParams &params);
    angle::Result generateTriFanBufferFromElementsArray(ContextMtl *contextMtl,
                                                        const IndexGenerationParams &params,
                                                        uint32_t *indicesGenerated);

    angle::Result generateLineLoopBufferFromArrays(ContextMtl *contextMtl,
                                                   const TriFanOrLineLoopFromArrayParams &params);
    angle::Result generateLineLoopLastSegment(ContextMtl *contextMtl,
                                              uint32_t firstVertex,
                                              uint32_t lastVertex,
                                              const BufferRef &dstBuffer,
                                              uint32_t dstOffset);
    angle::Result generateLineLoopBufferFromElementsArray(ContextMtl *contextMtl,
                                                          const IndexGenerationParams &params,
                                                          uint32_t *indicesGenerated);
    angle::Result generateLineLoopLastSegmentFromElementsArray(ContextMtl *contextMtl,
                                                               const IndexGenerationParams &params);

    void combineVisibilityResult(ContextMtl *contextMtl,
                                 bool keepOldValue,
                                 const VisibilityBufferOffsetsMtl &renderPassResultBufOffsets,
                                 const BufferRef &renderPassResultBuf,
                                 const BufferRef &finalResultBuf);

    // Compute based mipmap generation. Only possible for 3D texture for now.
    angle::Result generateMipmapCS(ContextMtl *contextMtl,
                                   const TextureRef &srcTexture,
                                   bool sRGBMipmap,
                                   NativeTexLevelArray *mipmapOutputViews);

    angle::Result unpackPixelsFromBufferToTexture(ContextMtl *contextMtl,
                                                  const angle::Format &srcAngleFormat,
                                                  const CopyPixelsFromBufferParams &params);
    angle::Result packPixelsFromTextureToBuffer(ContextMtl *contextMtl,
                                                const angle::Format &dstAngleFormat,
                                                const CopyPixelsToBufferParams &params);

    // See VertexFormatConversionUtils::convertVertexFormatToFloatCS()
    angle::Result convertVertexFormatToFloatCS(ContextMtl *contextMtl,
                                               const angle::Format &srcAngleFormat,
                                               const VertexFormatConvertParams &params);
    // See VertexFormatConversionUtils::convertVertexFormatToFloatVS()
    angle::Result convertVertexFormatToFloatVS(const gl::Context *context,
                                               RenderCommandEncoder *renderEncoder,
                                               const angle::Format &srcAngleFormat,
                                               const VertexFormatConvertParams &params);
    // See VertexFormatConversionUtils::expandVertexFormatComponentsCS()
    angle::Result expandVertexFormatComponentsCS(ContextMtl *contextMtl,
                                                 const angle::Format &srcAngleFormat,
                                                 const VertexFormatConvertParams &params);
    // See VertexFormatConversionUtils::expandVertexFormatComponentsVS()
    angle::Result expandVertexFormatComponentsVS(const gl::Context *context,
                                                 RenderCommandEncoder *renderEncoder,
                                                 const angle::Format &srcAngleFormat,
                                                 const VertexFormatConvertParams &params);

    angle::Result generatePrimitiveRestartPointsBuffer(ContextMtl *contextMtl,
                                                       const IndexGenerationParams &params,
                                                       size_t *indicesGenerated);
    angle::Result generatePrimitiveRestartLinesBuffer(ContextMtl *contextMtl,
                                                      const IndexGenerationParams &params,
                                                      size_t *indicesGenerated);
    angle::Result generatePrimitiveRestartTrianglesBuffer(ContextMtl *contextMtl,
                                                          const IndexGenerationParams &params,
                                                          size_t *indicesGenerated);

    // See BlockLinearizationUtils::linearizeBlocks()
    angle::Result linearizeBlocks(ContextMtl *contextMtl, const BlockLinearizationParams &params);

    // See DepthSaturationUtils::saturateDepth()
    angle::Result saturateDepth(ContextMtl *contextMtl, const DepthSaturationParams &params);

  private:
    std::array<ClearUtils, angle::EnumSize<PixelType>()> mClearUtils;

    std::array<ColorBlitUtils, angle::EnumSize<PixelType>()> mColorBlitUtils;
    ColorBlitUtils mCopyTextureFloatToUIntUtils;

    DepthStencilBlitUtils mDepthStencilBlitUtils;
    IndexGeneratorUtils mIndexUtils;
    VisibilityResultUtils mVisibilityResultUtils;
    MipmapUtils mMipmapUtils;
    std::array<CopyPixelsUtils, angle::EnumSize<PixelType>()> mCopyPixelsUtils;
    VertexFormatConversionUtils mVertexFormatUtils;
    BlockLinearizationUtils mBlockLinearizationUtils;
    DepthSaturationUtils mDepthSaturationUtils;
};

}  // namespace mtl
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_MTL_RENDER_UTILS_H_ */
