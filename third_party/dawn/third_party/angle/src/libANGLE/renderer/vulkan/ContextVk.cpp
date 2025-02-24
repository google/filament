//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextVk.cpp:
//    Implements the class methods for ContextVk.
//

#include "libANGLE/renderer/vulkan/ContextVk.h"

#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/system_utils.h"
#include "common/utilities.h"
#include "image_util/loadimage.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Program.h"
#include "libANGLE/Semaphore.h"
#include "libANGLE/ShareGroup.h"
#include "libANGLE/Surface.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/CompilerVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FenceNVVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/MemoryObjectVk.h"
#include "libANGLE/renderer/vulkan/OverlayVk.h"
#include "libANGLE/renderer/vulkan/ProgramPipelineVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/QueryVk.h"
#include "libANGLE/renderer/vulkan/RenderbufferVk.h"
#include "libANGLE/renderer/vulkan/SamplerVk.h"
#include "libANGLE/renderer/vulkan/SemaphoreVk.h"
#include "libANGLE/renderer/vulkan/ShaderVk.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"
#include "libANGLE/renderer/vulkan/SyncVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"
#include "libANGLE/renderer/vulkan/VertexArrayVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace rx
{
namespace
{
// If the total size of copyBufferToImage commands in the outside command buffer reaches the
// threshold below, the latter is flushed.
static constexpr VkDeviceSize kMaxBufferToImageCopySize = 64 * 1024 * 1024;
// The number of queueSerials we will reserve for outsideRenderPassCommands when we generate one for
// RenderPassCommands.
static constexpr size_t kMaxReservedOutsideRenderPassQueueSerials = 15;

// Dumping the command stream is disabled by default.
static constexpr bool kEnableCommandStreamDiagnostics = false;

// All glMemoryBarrier bits that related to texture usage
static constexpr GLbitfield kWriteAfterAccessImageMemoryBarriers =
    GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
static constexpr GLbitfield kWriteAfterAccessMemoryBarriers =
    kWriteAfterAccessImageMemoryBarriers | GL_SHADER_STORAGE_BARRIER_BIT;

// For shader uniforms such as gl_DepthRange and the viewport size.
struct GraphicsDriverUniforms
{
    // Contain packed 8-bit values for atomic counter buffer offsets.  These offsets are within
    // Vulkan's minStorageBufferOffsetAlignment limit and are used to support unaligned offsets
    // allowed in GL.
    std::array<uint32_t, 2> acbBufferOffsets;

    // .x is near, .y is far
    std::array<float, 2> depthRange;

    // Used to flip gl_FragCoord.  Packed uvec2
    uint32_t renderArea;

    // Packed vec4 of snorm8
    uint32_t flipXY;

    // Only the lower 16 bits used
    uint32_t dither;

    // Various bits of state:
    // - Surface rotation
    // - Advanced blend equation
    // - Sample count
    // - Enabled clip planes
    // - Depth transformation
    uint32_t misc;
};
static_assert(sizeof(GraphicsDriverUniforms) % (sizeof(uint32_t) * 4) == 0,
              "GraphicsDriverUniforms should be 16bytes aligned");

// Only used when transform feedback is emulated.
struct GraphicsDriverUniformsExtended
{
    GraphicsDriverUniforms common;

    // Only used with transform feedback emulation
    std::array<int32_t, 4> xfbBufferOffsets;
    int32_t xfbVerticesPerInstance;

    int32_t padding[3];
};
static_assert(sizeof(GraphicsDriverUniformsExtended) % (sizeof(uint32_t) * 4) == 0,
              "GraphicsDriverUniformsExtended should be 16bytes aligned");
// Driver uniforms are updated using push constants and Vulkan spec guarantees universal support for
// 128 bytes worth of push constants. For maximum compatibility ensure
// GraphicsDriverUniformsExtended size is within that limit.
static_assert(sizeof(GraphicsDriverUniformsExtended) <= 128,
              "Only 128 bytes are guranteed for push constants");

struct ComputeDriverUniforms
{
    // Atomic counter buffer offsets with the same layout as in GraphicsDriverUniforms.
    std::array<uint32_t, 4> acbBufferOffsets;
};

uint32_t MakeFlipUniform(bool flipX, bool flipY, bool invertViewport)
{
    // Create snorm values of either -1 or 1, based on whether flipping is enabled or not
    // respectively.
    constexpr uint8_t kSnormOne      = 0x7F;
    constexpr uint8_t kSnormMinusOne = 0x81;

    // .xy are flips for the fragment stage.
    uint32_t x = flipX ? kSnormMinusOne : kSnormOne;
    uint32_t y = flipY ? kSnormMinusOne : kSnormOne;

    // .zw are flips for the vertex stage.
    uint32_t z = x;
    uint32_t w = flipY != invertViewport ? kSnormMinusOne : kSnormOne;

    return x | y << 8 | z << 16 | w << 24;
}

GLenum DefaultGLErrorCode(VkResult result)
{
    switch (result)
    {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        case VK_ERROR_TOO_MANY_OBJECTS:
            return GL_OUT_OF_MEMORY;
        case VK_ERROR_DEVICE_LOST:
            return GL_CONTEXT_LOST;
        default:
            return GL_INVALID_OPERATION;
    }
}

constexpr gl::ShaderMap<vk::ImageLayout> kShaderReadOnlyImageLayouts = {
    {gl::ShaderType::Vertex, vk::ImageLayout::VertexShaderReadOnly},
    {gl::ShaderType::TessControl, vk::ImageLayout::PreFragmentShadersReadOnly},
    {gl::ShaderType::TessEvaluation, vk::ImageLayout::PreFragmentShadersReadOnly},
    {gl::ShaderType::Geometry, vk::ImageLayout::PreFragmentShadersReadOnly},
    {gl::ShaderType::Fragment, vk::ImageLayout::FragmentShaderReadOnly},
    {gl::ShaderType::Compute, vk::ImageLayout::ComputeShaderReadOnly}};

constexpr gl::ShaderMap<vk::ImageLayout> kShaderWriteImageLayouts = {
    {gl::ShaderType::Vertex, vk::ImageLayout::VertexShaderWrite},
    {gl::ShaderType::TessControl, vk::ImageLayout::PreFragmentShadersWrite},
    {gl::ShaderType::TessEvaluation, vk::ImageLayout::PreFragmentShadersWrite},
    {gl::ShaderType::Geometry, vk::ImageLayout::PreFragmentShadersWrite},
    {gl::ShaderType::Fragment, vk::ImageLayout::FragmentShaderWrite},
    {gl::ShaderType::Compute, vk::ImageLayout::ComputeShaderWrite}};

constexpr VkBufferUsageFlags kVertexBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
constexpr size_t kDynamicVertexDataSize         = 16 * 1024;

bool CanMultiDrawIndirectUseCmd(ContextVk *contextVk,
                                VertexArrayVk *vertexArray,
                                gl::PrimitiveMode mode,
                                GLsizei drawcount,
                                GLsizei stride)
{
    // Use the generic implementation if multiDrawIndirect is disabled, if line loop is being used
    // for multiDraw, if drawcount is greater than maxDrawIndirectCount, or if there are streaming
    // vertex attributes.
    ASSERT(drawcount > 1);
    const bool supportsMultiDrawIndirect =
        contextVk->getFeatures().supportsMultiDrawIndirect.enabled;
    const bool isMultiDrawLineLoop = (mode == gl::PrimitiveMode::LineLoop);
    const bool isDrawCountBeyondLimit =
        (static_cast<uint32_t>(drawcount) >
         contextVk->getRenderer()->getPhysicalDeviceProperties().limits.maxDrawIndirectCount);
    const bool isMultiDrawWithStreamingAttribs = vertexArray->getStreamingVertexAttribsMask().any();

    const bool canMultiDrawIndirectUseCmd = supportsMultiDrawIndirect && !isMultiDrawLineLoop &&
                                            !isDrawCountBeyondLimit &&
                                            !isMultiDrawWithStreamingAttribs;
    return canMultiDrawIndirectUseCmd;
}

uint32_t GetCoverageSampleCount(const gl::State &glState, GLint samples)
{
    ASSERT(glState.isSampleCoverageEnabled());

    // Get a fraction of the samples based on the coverage parameters.
    // There are multiple ways to obtain an integer value from a float -
    //     truncation, ceil and round
    //
    // round() provides a more even distribution of values but doesn't seem to play well
    // with all vendors (AMD). A way to work around this is to increase the comparison threshold
    // of deqp tests. Though this takes care of deqp tests other apps would still have issues.
    //
    // Truncation provides an uneven distribution near the edges of the interval but seems to
    // play well with all vendors.
    //
    // We are going with truncation for expediency.
    return static_cast<uint32_t>(glState.getSampleCoverageValue() * samples);
}

void ApplySampleCoverage(const gl::State &glState, uint32_t coverageSampleCount, uint32_t *maskOut)
{
    ASSERT(glState.isSampleCoverageEnabled());

    uint32_t coverageMask = angle::BitMask<uint32_t>(coverageSampleCount);

    if (glState.getSampleCoverageInvert())
    {
        coverageMask = ~coverageMask;
    }

    *maskOut &= coverageMask;
}

SurfaceRotation DetermineSurfaceRotation(const gl::Framebuffer *framebuffer,
                                         const WindowSurfaceVk *windowSurface)
{
    if (windowSurface && framebuffer->isDefault())
    {
        switch (windowSurface->getPreTransform())
        {
            case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:
                // Do not rotate gl_Position (surface matches the device's orientation):
                return SurfaceRotation::Identity;
            case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
                // Rotate gl_Position 90 degrees:
                return SurfaceRotation::Rotated90Degrees;
            case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
                // Rotate gl_Position 180 degrees:
                return SurfaceRotation::Rotated180Degrees;
            case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
                // Rotate gl_Position 270 degrees:
                return SurfaceRotation::Rotated270Degrees;
            default:
                UNREACHABLE();
                return SurfaceRotation::Identity;
        }
    }
    else
    {
        // Do not rotate gl_Position (offscreen framebuffer):
        return SurfaceRotation::Identity;
    }
}

// Should not generate a copy with modern C++.
EventName GetTraceEventName(const char *title, uint64_t counter)
{
    EventName buf;
    snprintf(buf.data(), kMaxGpuEventNameLen - 1, "%s %llu", title,
             static_cast<unsigned long long>(counter));
    return buf;
}

vk::ResourceAccess GetColorAccess(const gl::State &state,
                                  const gl::FramebufferState &framebufferState,
                                  const gl::DrawBufferMask &emulatedAlphaMask,
                                  const gl::ProgramExecutable *executable,
                                  size_t colorIndexGL)
{
    // No access if draw buffer is disabled altogether
    // Without framebuffer fetch:
    //   No access if color output is masked, or rasterizer discard is enabled
    // With framebuffer fetch:
    //   Read access if color output is masked, or rasterizer discard is enabled

    if (!framebufferState.getEnabledDrawBuffers().test(colorIndexGL))
    {
        return vk::ResourceAccess::Unused;
    }

    const gl::BlendStateExt &blendStateExt = state.getBlendStateExt();
    uint8_t colorMask                      = gl::BlendStateExt::ColorMaskStorage::GetValueIndexed(
        colorIndexGL, blendStateExt.getColorMaskBits());
    if (emulatedAlphaMask[colorIndexGL])
    {
        colorMask &= ~VK_COLOR_COMPONENT_A_BIT;
    }
    const bool isOutputMasked = colorMask == 0 || state.isRasterizerDiscardEnabled();

    if (isOutputMasked)
    {
        const bool hasFramebufferFetch =
            executable ? executable->usesColorFramebufferFetch() : false;
        return hasFramebufferFetch ? vk::ResourceAccess::ReadOnly : vk::ResourceAccess::Unused;
    }

    return vk::ResourceAccess::ReadWrite;
}

vk::ResourceAccess GetDepthAccess(const gl::DepthStencilState &dsState,
                                  const gl::ProgramExecutable *executable,
                                  UpdateDepthFeedbackLoopReason reason)
{
    // Skip if depth/stencil not actually accessed.
    if (reason == UpdateDepthFeedbackLoopReason::None)
    {
        return vk::ResourceAccess::Unused;
    }

    // Note that clear commands don't respect depth test enable, only the mask
    // Note Other state can be stated here too in the future, such as rasterizer discard.
    if (!dsState.depthTest && reason != UpdateDepthFeedbackLoopReason::Clear)
    {
        return vk::ResourceAccess::Unused;
    }

    if (dsState.isDepthMaskedOut())
    {
        const bool hasFramebufferFetch =
            executable ? executable->usesDepthFramebufferFetch() : false;

        // If depthFunc is GL_ALWAYS or GL_NEVER, we do not need to load depth value.
        return (dsState.depthFunc == GL_ALWAYS || dsState.depthFunc == GL_NEVER) &&
                       !hasFramebufferFetch
                   ? vk::ResourceAccess::Unused
                   : vk::ResourceAccess::ReadOnly;
    }

    return vk::ResourceAccess::ReadWrite;
}

vk::ResourceAccess GetStencilAccess(const gl::DepthStencilState &dsState,
                                    GLuint framebufferStencilSize,
                                    const gl::ProgramExecutable *executable,
                                    UpdateDepthFeedbackLoopReason reason)
{
    // Skip if depth/stencil not actually accessed.
    if (reason == UpdateDepthFeedbackLoopReason::None)
    {
        return vk::ResourceAccess::Unused;
    }

    // Note that clear commands don't respect stencil test enable, only the mask
    // Note Other state can be stated here too in the future, such as rasterizer discard.
    if (!dsState.stencilTest && reason != UpdateDepthFeedbackLoopReason::Clear)
    {
        return vk::ResourceAccess::Unused;
    }

    const bool hasFramebufferFetch = executable ? executable->usesStencilFramebufferFetch() : false;

    return dsState.isStencilNoOp(framebufferStencilSize) &&
                   dsState.isStencilBackNoOp(framebufferStencilSize) && !hasFramebufferFetch
               ? vk::ResourceAccess::ReadOnly
               : vk::ResourceAccess::ReadWrite;
}

egl::ContextPriority GetContextPriority(const gl::State &state)
{
    return egl::FromEGLenum<egl::ContextPriority>(state.getContextPriority());
}

bool IsStencilSamplerBinding(const gl::ProgramExecutable &executable, size_t textureUnit)
{
    const gl::SamplerFormat format = executable.getSamplerFormatForTextureUnitIndex(textureUnit);
    const bool isStencilTexture    = format == gl::SamplerFormat::Unsigned;
    return isStencilTexture;
}

vk::ImageLayout GetDepthStencilAttachmentImageReadLayout(const vk::ImageHelper &image,
                                                         gl::ShaderType firstShader)
{
    const bool isDepthTexture =
        image.hasRenderPassUsageFlag(vk::RenderPassUsage::DepthTextureSampler);
    const bool isStencilTexture =
        image.hasRenderPassUsageFlag(vk::RenderPassUsage::StencilTextureSampler);

    const bool isDepthReadOnlyAttachment =
        image.hasRenderPassUsageFlag(vk::RenderPassUsage::DepthReadOnlyAttachment);
    const bool isStencilReadOnlyAttachment =
        image.hasRenderPassUsageFlag(vk::RenderPassUsage::StencilReadOnlyAttachment);

    const bool isFS = firstShader == gl::ShaderType::Fragment;

    // Only called when at least one aspect of the image is bound as texture
    ASSERT(isDepthTexture || isStencilTexture);

    // Check for feedback loop; this is when depth or stencil is both bound as a texture and is used
    // in a non-read-only way as attachment.
    if ((isDepthTexture && !isDepthReadOnlyAttachment) ||
        (isStencilTexture && !isStencilReadOnlyAttachment))
    {
        return isFS ? vk::ImageLayout::DepthStencilFragmentShaderFeedback
                    : vk::ImageLayout::DepthStencilAllShadersFeedback;
    }

    if (isDepthReadOnlyAttachment)
    {
        if (isStencilReadOnlyAttachment)
        {
            // Depth read + stencil read
            return isFS ? vk::ImageLayout::DepthReadStencilReadFragmentShaderRead
                        : vk::ImageLayout::DepthReadStencilReadAllShadersRead;
        }
        else
        {
            // Depth read + stencil write
            return isFS ? vk::ImageLayout::DepthReadStencilWriteFragmentShaderDepthRead
                        : vk::ImageLayout::DepthReadStencilWriteAllShadersDepthRead;
        }
    }
    else
    {
        if (isStencilReadOnlyAttachment)
        {
            // Depth write + stencil read
            return isFS ? vk::ImageLayout::DepthWriteStencilReadFragmentShaderStencilRead
                        : vk::ImageLayout::DepthWriteStencilReadAllShadersStencilRead;
        }
        else
        {
            // Depth write + stencil write: This is definitely a feedback loop and is handled above.
            UNREACHABLE();
            return vk::ImageLayout::DepthStencilAllShadersFeedback;
        }
    }
}

vk::ImageLayout GetImageReadLayout(TextureVk *textureVk,
                                   const gl::ProgramExecutable &executable,
                                   size_t textureUnit,
                                   PipelineType pipelineType)
{
    vk::ImageHelper &image = textureVk->getImage();

    // If this texture has been bound as image and the current executable program accesses images,
    // we consider this image's layout as writeable.
    if (textureVk->hasBeenBoundAsImage() && executable.hasImages())
    {
        return pipelineType == PipelineType::Compute ? vk::ImageLayout::ComputeShaderWrite
                                                     : vk::ImageLayout::AllGraphicsShadersWrite;
    }

    gl::ShaderBitSet remainingShaderBits =
        executable.getSamplerShaderBitsForTextureUnitIndex(textureUnit);
    ASSERT(remainingShaderBits.any());
    gl::ShaderType firstShader = remainingShaderBits.first();
    gl::ShaderType lastShader  = remainingShaderBits.last();
    remainingShaderBits.reset(firstShader);
    remainingShaderBits.reset(lastShader);

    const bool isFragmentShaderOnly = firstShader == gl::ShaderType::Fragment;
    if (isFragmentShaderOnly)
    {
        ASSERT(remainingShaderBits.none() && lastShader == firstShader);
    }

    if (image.hasRenderPassUsageFlag(vk::RenderPassUsage::RenderTargetAttachment))
    {
        // Right now we set the *TextureSampler flag only when RenderTargetAttachment is set since
        // we do not track all textures in the render pass.

        if (image.isDepthOrStencil())
        {
            if (IsStencilSamplerBinding(executable, textureUnit))
            {
                image.setRenderPassUsageFlag(vk::RenderPassUsage::StencilTextureSampler);
            }
            else
            {
                image.setRenderPassUsageFlag(vk::RenderPassUsage::DepthTextureSampler);
            }

            return GetDepthStencilAttachmentImageReadLayout(image, firstShader);
        }

        image.setRenderPassUsageFlag(vk::RenderPassUsage::ColorTextureSampler);

        return isFragmentShaderOnly ? vk::ImageLayout::ColorWriteFragmentShaderFeedback
                                    : vk::ImageLayout::ColorWriteAllShadersFeedback;
    }

    if (image.isDepthOrStencil())
    {
        // We always use a depth-stencil read-only layout for any depth Textures to simplify
        // our implementation's handling of depth-stencil read-only mode. We don't have to
        // split a RenderPass to transition a depth texture from shader-read to read-only.
        // This improves performance in Manhattan. Future optimizations are likely possible
        // here including using specialized barriers without breaking the RenderPass.
        return isFragmentShaderOnly ? vk::ImageLayout::DepthReadStencilReadFragmentShaderRead
                                    : vk::ImageLayout::DepthReadStencilReadAllShadersRead;
    }

    // We barrier against either:
    // - Vertex only
    // - Fragment only
    // - Pre-fragment only (vertex, geometry and tessellation together)
    if (remainingShaderBits.any() || firstShader != lastShader)
    {
        return lastShader == gl::ShaderType::Fragment ? vk::ImageLayout::AllGraphicsShadersReadOnly
                                                      : vk::ImageLayout::PreFragmentShadersReadOnly;
    }

    return kShaderReadOnlyImageLayouts[firstShader];
}

vk::ImageLayout GetImageWriteLayoutAndSubresource(const gl::ImageUnit &imageUnit,
                                                  vk::ImageHelper &image,
                                                  gl::ShaderBitSet shaderStages,
                                                  gl::LevelIndex *levelOut,
                                                  uint32_t *layerStartOut,
                                                  uint32_t *layerCountOut)
{
    *levelOut = gl::LevelIndex(static_cast<uint32_t>(imageUnit.level));

    *layerStartOut = 0;
    *layerCountOut = image.getLayerCount();
    if (imageUnit.layered)
    {
        *layerStartOut = imageUnit.layered;
        *layerCountOut = 1;
    }

    gl::ShaderType firstShader = shaderStages.first();
    gl::ShaderType lastShader  = shaderStages.last();
    shaderStages.reset(firstShader);
    shaderStages.reset(lastShader);
    // We barrier against either:
    // - Vertex only
    // - Fragment only
    // - Pre-fragment only (vertex, geometry and tessellation together)
    if (shaderStages.any() || firstShader != lastShader)
    {
        return lastShader == gl::ShaderType::Fragment ? vk::ImageLayout::AllGraphicsShadersWrite
                                                      : vk::ImageLayout::PreFragmentShadersWrite;
    }

    return kShaderWriteImageLayouts[firstShader];
}

template <typename CommandBufferT>
void OnTextureBufferRead(vk::Context *context,
                         vk::BufferHelper *buffer,
                         gl::ShaderBitSet stages,
                         CommandBufferT *commandBufferHelper)
{
    ASSERT(stages.any());

    // TODO: accept multiple stages in bufferRead.  http://anglebug.com/42262235
    for (gl::ShaderType stage : stages)
    {
        // Note: if another range of the same buffer is simultaneously used for storage,
        // such as for transform feedback output, or SSBO, unnecessary barriers can be
        // generated.
        commandBufferHelper->bufferRead(context, VK_ACCESS_SHADER_READ_BIT,
                                        vk::GetPipelineStage(stage), buffer);
    }
}

template <typename CommandBufferT>
void OnImageBufferWrite(vk::Context *context,
                        BufferVk *bufferVk,
                        gl::ShaderBitSet stages,
                        CommandBufferT *commandBufferHelper)
{
    vk::BufferHelper &buffer = bufferVk->getBuffer();
    VkAccessFlags accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    commandBufferHelper->bufferWrite(context, accessFlags, stages, &buffer);
}

constexpr angle::PackedEnumMap<RenderPassClosureReason, const char *> kRenderPassClosureReason = {{
    {RenderPassClosureReason::AlreadySpecifiedElsewhere, nullptr},
    {RenderPassClosureReason::ContextDestruction, "Render pass closed due to context destruction"},
    {RenderPassClosureReason::ContextChange, "Render pass closed due to context change"},
    {RenderPassClosureReason::GLFlush, "Render pass closed due to glFlush()"},
    {RenderPassClosureReason::GLFinish, "Render pass closed due to glFinish()"},
    {RenderPassClosureReason::EGLSwapBuffers, "Render pass closed due to eglSwapBuffers()"},
    {RenderPassClosureReason::EGLWaitClient, "Render pass closed due to eglWaitClient()"},
    {RenderPassClosureReason::SurfaceUnMakeCurrent,
     "Render pass closed due to onSurfaceUnMakeCurrent()"},
    {RenderPassClosureReason::FramebufferBindingChange,
     "Render pass closed due to framebuffer binding change"},
    {RenderPassClosureReason::FramebufferChange, "Render pass closed due to framebuffer change"},
    {RenderPassClosureReason::NewRenderPass,
     "Render pass closed due to starting a new render pass"},
    {RenderPassClosureReason::BufferUseThenXfbWrite,
     "Render pass closed due to buffer use as transform feedback output after prior use in render "
     "pass"},
    {RenderPassClosureReason::XfbWriteThenVertexIndexBuffer,
     "Render pass closed due to transform feedback buffer use as vertex/index input"},
    {RenderPassClosureReason::XfbWriteThenIndirectDrawBuffer,
     "Render pass closed due to indirect draw buffer previously used as transform feedback output "
     "in render pass"},
    {RenderPassClosureReason::XfbResumeAfterDrawBasedClear,
     "Render pass closed due to transform feedback resume after clear through draw"},
    {RenderPassClosureReason::DepthStencilUseInFeedbackLoop,
     "Render pass closed due to depth/stencil attachment use under feedback loop"},
    {RenderPassClosureReason::DepthStencilWriteAfterFeedbackLoop,
     "Render pass closed due to depth/stencil attachment write after feedback loop"},
    {RenderPassClosureReason::PipelineBindWhileXfbActive,
     "Render pass closed due to graphics pipeline change while transform feedback is active"},
    {RenderPassClosureReason::BufferWriteThenMap,
     "Render pass closed due to mapping buffer being written to by said render pass"},
    {RenderPassClosureReason::BufferWriteThenOutOfRPRead,
     "Render pass closed due to non-render-pass read of buffer that was written to in render pass"},
    {RenderPassClosureReason::BufferUseThenOutOfRPWrite,
     "Render pass closed due to non-render-pass write of buffer that was used in render pass"},
    {RenderPassClosureReason::ImageUseThenOutOfRPRead,
     "Render pass closed due to non-render-pass read of image that was used in render pass"},
    {RenderPassClosureReason::ImageUseThenOutOfRPWrite,
     "Render pass closed due to non-render-pass write of image that was used in render pass"},
    {RenderPassClosureReason::XfbWriteThenComputeRead,
     "Render pass closed due to compute read of buffer previously used as transform feedback "
     "output in render pass"},
    {RenderPassClosureReason::XfbWriteThenIndirectDispatchBuffer,
     "Render pass closed due to indirect dispatch buffer previously used as transform feedback "
     "output in render pass"},
    {RenderPassClosureReason::ImageAttachmentThenComputeRead,
     "Render pass closed due to compute read of image previously used as framebuffer attachment in "
     "render pass"},
    {RenderPassClosureReason::GetQueryResult, "Render pass closed due to getting query result"},
    {RenderPassClosureReason::BeginNonRenderPassQuery,
     "Render pass closed due to non-render-pass query begin"},
    {RenderPassClosureReason::EndNonRenderPassQuery,
     "Render pass closed due to non-render-pass query end"},
    {RenderPassClosureReason::TimestampQuery, "Render pass closed due to timestamp query"},
    {RenderPassClosureReason::EndRenderPassQuery,
     "Render pass closed due to switch from query enabled draw to query disabled draw"},
    {RenderPassClosureReason::GLReadPixels, "Render pass closed due to glReadPixels()"},
    {RenderPassClosureReason::BufferUseThenReleaseToExternal,
     "Render pass closed due to buffer (used by render pass) release to external"},
    {RenderPassClosureReason::ImageUseThenReleaseToExternal,
     "Render pass closed due to image (used by render pass) release to external"},
    {RenderPassClosureReason::BufferInUseWhenSynchronizedMap,
     "Render pass closed due to mapping buffer in use by GPU without GL_MAP_UNSYNCHRONIZED_BIT"},
    {RenderPassClosureReason::GLMemoryBarrierThenStorageResource,
     "Render pass closed due to glMemoryBarrier before storage output in render pass"},
    {RenderPassClosureReason::StorageResourceUseThenGLMemoryBarrier,
     "Render pass closed due to glMemoryBarrier after storage output in render pass"},
    {RenderPassClosureReason::ExternalSemaphoreSignal,
     "Render pass closed due to external semaphore signal"},
    {RenderPassClosureReason::SyncObjectInit, "Render pass closed due to sync object insertion"},
    {RenderPassClosureReason::SyncObjectWithFdInit,
     "Render pass closed due to sync object with fd insertion"},
    {RenderPassClosureReason::SyncObjectClientWait,
     "Render pass closed due to sync object client wait"},
    {RenderPassClosureReason::SyncObjectServerWait,
     "Render pass closed due to sync object server wait"},
    {RenderPassClosureReason::SyncObjectGetStatus,
     "Render pass closed due to sync object get status"},
    {RenderPassClosureReason::XfbPause, "Render pass closed due to transform feedback pause"},
    {RenderPassClosureReason::FramebufferFetchEmulation,
     "Render pass closed due to framebuffer fetch emulation"},
    {RenderPassClosureReason::ColorBufferWithEmulatedAlphaInvalidate,
     "Render pass closed due to color attachment with emulated alpha channel being invalidated"},
    {RenderPassClosureReason::GenerateMipmapOnCPU,
     "Render pass closed due to fallback to CPU when generating mipmaps"},
    {RenderPassClosureReason::CopyTextureOnCPU,
     "Render pass closed due to fallback to CPU when copying texture"},
    {RenderPassClosureReason::TextureReformatToRenderable,
     "Render pass closed due to reformatting texture to a renderable fallback"},
    {RenderPassClosureReason::DeviceLocalBufferMap,
     "Render pass closed due to mapping device local buffer"},
    {RenderPassClosureReason::PrepareForBlit, "Render pass closed prior to draw-based blit"},
    {RenderPassClosureReason::PrepareForImageCopy,
     "Render pass closed prior to draw-based image copy"},
    {RenderPassClosureReason::TemporaryForImageClear,
     "Temporary render pass used for image clear closed"},
    {RenderPassClosureReason::TemporaryForImageCopy,
     "Temporary render pass used for image copy closed"},
    {RenderPassClosureReason::TemporaryForOverlayDraw,
     "Temporary render pass used for overlay draw closed"},
}};

VkDependencyFlags GetLocalDependencyFlags(ContextVk *contextVk)
{
    VkDependencyFlags dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    if (contextVk->getCurrentViewCount() > 0)
    {
        dependencyFlags |= VK_DEPENDENCY_VIEW_LOCAL_BIT;
    }
    return dependencyFlags;
}

bool BlendModeSupportsDither(const ContextVk *contextVk, size_t colorIndex)
{
    const gl::State &state = contextVk->getState();

    // Specific combinations of color blend modes are known to work with our dithering emulation.
    // Note we specifically don't check alpha blend, as dither isn't applied to alpha.
    // See http://b/232574868 for more discussion and reasoning.
    gl::BlendFactorType srcBlendFactor = state.getBlendStateExt().getSrcColorIndexed(colorIndex);
    gl::BlendFactorType dstBlendFactor = state.getBlendStateExt().getDstColorIndexed(colorIndex);

    const bool ditheringCompatibleBlendFactors =
        (srcBlendFactor == gl::BlendFactorType::SrcAlpha &&
         dstBlendFactor == gl::BlendFactorType::OneMinusSrcAlpha);

    const bool allowAdditionalBlendFactors =
        contextVk->getFeatures().enableAdditionalBlendFactorsForDithering.enabled &&
        (srcBlendFactor == gl::BlendFactorType::One &&
         dstBlendFactor == gl::BlendFactorType::OneMinusSrcAlpha);

    return ditheringCompatibleBlendFactors || allowAdditionalBlendFactors;
}

bool ShouldUseGraphicsDriverUniformsExtended(const vk::ErrorContext *context)
{
    return context->getFeatures().emulateTransformFeedback.enabled;
}

bool IsAnySamplesQuery(gl::QueryType type)
{
    return type == gl::QueryType::AnySamples || type == gl::QueryType::AnySamplesConservative;
}

enum class GraphicsPipelineSubsetRenderPass
{
    Unused,
    Required,
};

template <typename Cache>
angle::Result CreateGraphicsPipelineSubset(ContextVk *contextVk,
                                           const vk::GraphicsPipelineDesc &desc,
                                           vk::GraphicsPipelineTransitionBits transition,
                                           GraphicsPipelineSubsetRenderPass renderPass,
                                           Cache *cache,
                                           vk::PipelineCacheAccess *pipelineCache,
                                           vk::PipelineHelper **pipelineOut)
{
    const vk::PipelineLayout unusedPipelineLayout;
    const vk::ShaderModuleMap unusedShaders;
    const vk::SpecializationConstants unusedSpecConsts = {};

    if (*pipelineOut != nullptr && !transition.any())
    {
        return angle::Result::Continue;
    }

    if (*pipelineOut != nullptr)
    {
        ASSERT((*pipelineOut)->valid());
        if ((*pipelineOut)->findTransition(transition, desc, pipelineOut))
        {
            return angle::Result::Continue;
        }
    }

    vk::PipelineHelper *oldPipeline = *pipelineOut;

    const vk::GraphicsPipelineDesc *descPtr = nullptr;
    if (!cache->getPipeline(desc, &descPtr, pipelineOut))
    {
        const vk::RenderPass unusedRenderPass;
        const vk::RenderPass *compatibleRenderPass = &unusedRenderPass;
        if (renderPass == GraphicsPipelineSubsetRenderPass::Required)
        {
            // Pull in a compatible RenderPass if used by this subset.
            ANGLE_TRY(contextVk->getCompatibleRenderPass(desc.getRenderPassDesc(),
                                                         &compatibleRenderPass));
        }

        ANGLE_TRY(cache->createPipeline(contextVk, pipelineCache, *compatibleRenderPass,
                                        unusedPipelineLayout, unusedShaders, unusedSpecConsts,
                                        PipelineSource::Draw, desc, &descPtr, pipelineOut));
    }

    if (oldPipeline)
    {
        oldPipeline->addTransition(transition, descPtr, *pipelineOut);
    }

    return angle::Result::Continue;
}

bool QueueSerialsHaveDifferentIndexOrSmaller(const QueueSerial &queueSerial1,
                                             const QueueSerial &queueSerial2)
{
    return queueSerial1.getIndex() != queueSerial2.getIndex() || queueSerial1 < queueSerial2;
}

void UpdateImagesWithSharedCacheKey(const gl::ActiveTextureArray<TextureVk *> &activeImages,
                                    const std::vector<gl::ImageBinding> &imageBindings,
                                    const vk::SharedDescriptorSetCacheKey &sharedCacheKey)
{
    for (const gl::ImageBinding &imageBinding : imageBindings)
    {
        uint32_t arraySize = static_cast<uint32_t>(imageBinding.boundImageUnits.size());
        for (uint32_t arrayElement = 0; arrayElement < arraySize; ++arrayElement)
        {
            GLuint imageUnit = imageBinding.boundImageUnits[arrayElement];
            // For simplicity, we do not check if uniform is active or duplicate. The worst case is
            // we unnecessarily delete the cache entry when image bound to inactive uniform is
            // destroyed.
            activeImages[imageUnit]->onNewDescriptorSet(sharedCacheKey);
        }
    }
}

void UpdateBufferWithSharedCacheKey(const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding,
                                    VkDescriptorType descriptorType,
                                    const vk::SharedDescriptorSetCacheKey &sharedCacheKey)
{
    if (bufferBinding.get() != nullptr)
    {
        // For simplicity, we do not check if uniform is active or duplicate. The worst case is
        // we unnecessarily delete the cache entry when buffer bound to inactive uniform is
        // destroyed.
        BufferVk *bufferVk             = vk::GetImpl(bufferBinding.get());
        vk::BufferHelper &bufferHelper = bufferVk->getBuffer();
        if (vk::IsDynamicDescriptor(descriptorType))
        {
            bufferHelper.getBufferBlock()->onNewDescriptorSet(sharedCacheKey);
        }
        else
        {
            bufferHelper.onNewDescriptorSet(sharedCacheKey);
        }
    }
}

void GenerateTextureUnitSamplerIndexMap(
    const std::vector<GLuint> &samplerBoundTextureUnits,
    std::unordered_map<size_t, uint32_t> *textureUnitSamplerIndexMapOut)
{
    // Create a map of textureUnit <-> samplerIndex
    for (size_t samplerIndex = 0; samplerIndex < samplerBoundTextureUnits.size(); samplerIndex++)
    {
        textureUnitSamplerIndexMapOut->insert(
            {samplerBoundTextureUnits[samplerIndex], static_cast<uint32_t>(samplerIndex)});
    }
}
}  // anonymous namespace

void ContextVk::flushDescriptorSetUpdates()
{
    mPerfCounters.writeDescriptorSets +=
        mShareGroupVk->getUpdateDescriptorSetsBuilder()->flushDescriptorSetUpdates(getDevice());
}

ANGLE_INLINE void ContextVk::onRenderPassFinished(RenderPassClosureReason reason)
{
    if (mRenderPassCommandBuffer != nullptr)
    {
        pauseRenderPassQueriesIfActive();

        // If reason is specified, add it to the command buffer right before ending the render pass,
        // so it will show up in GPU debuggers.
        const char *reasonText = kRenderPassClosureReason[reason];
        if (reasonText)
        {
            insertEventMarkerImpl(GL_DEBUG_SOURCE_API, reasonText);
        }

        mRenderPassCommandBuffer = nullptr;

        // Restart at subpass 0.
        mGraphicsPipelineDesc->resetSubpass(&mGraphicsPipelineTransition);
    }

    mGraphicsDirtyBits.set(DIRTY_BIT_RENDER_PASS);
}

// ContextVk implementation.
ContextVk::ContextVk(const gl::State &state, gl::ErrorSet *errorSet, vk::Renderer *renderer)
    : ContextImpl(state, errorSet),
      vk::Context(renderer),
      mGraphicsDirtyBitHandlers{},
      mComputeDirtyBitHandlers{},
      mRenderPassCommandBuffer(nullptr),
      mCurrentGraphicsPipeline(nullptr),
      mCurrentGraphicsPipelineShaders(nullptr),
      mCurrentGraphicsPipelineVertexInput(nullptr),
      mCurrentGraphicsPipelineFragmentOutput(nullptr),
      mCurrentComputePipeline(nullptr),
      mCurrentDrawMode(gl::PrimitiveMode::InvalidEnum),
      mCurrentWindowSurface(nullptr),
      mCurrentRotationDrawFramebuffer(SurfaceRotation::Identity),
      mCurrentRotationReadFramebuffer(SurfaceRotation::Identity),
      mActiveRenderPassQueries{},
      mLastIndexBufferOffset(nullptr),
      mCurrentIndexBuffer(nullptr),
      mCurrentIndexBufferOffset(0),
      mCurrentDrawElementsType(gl::DrawElementsType::InvalidEnum),
      mXfbBaseVertex(0),
      mXfbVertexCountPerInstance(0),
      mClearColorValue{},
      mClearDepthStencilValue{},
      mClearColorMasks(0),
      mDeferredMemoryBarriers(0),
      mFlipYForCurrentSurface(false),
      mFlipViewportForDrawFramebuffer(false),
      mFlipViewportForReadFramebuffer(false),
      mIsAnyHostVisibleBufferWritten(false),
      mCurrentQueueSerialIndex(kInvalidQueueSerialIndex),
      mOutsideRenderPassCommands(nullptr),
      mRenderPassCommands(nullptr),
      mQueryEventType(GraphicsEventCmdBuf::NotInQueryCmd),
      mGpuEventsEnabled(false),
      mPrimaryBufferEventCounter(0),
      mHasDeferredFlush(false),
      mHasAnyCommandsPendingSubmission(false),
      mIsInColorFramebufferFetchMode(false),
      mAllowRenderPassToReactivate(true),
      mTotalBufferToImageCopySize(0),
      mEstimatedPendingImageGarbageSize(0),
      mHasWaitSemaphoresPendingSubmission(false),
      mGpuClockSync{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()},
      mGpuEventTimestampOrigin(0),
      mInitialContextPriority(renderer->getDriverPriority(GetContextPriority(state))),
      mContextPriority(mInitialContextPriority),
      mProtectionType(vk::ConvertProtectionBoolToType(state.hasProtectedContent())),
      mShareGroupVk(vk::GetImpl(state.getShareGroup()))
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::ContextVk");
    memset(&mClearColorValue, 0, sizeof(mClearColorValue));
    memset(&mClearDepthStencilValue, 0, sizeof(mClearDepthStencilValue));
    memset(&mViewport, 0, sizeof(mViewport));
    memset(&mScissor, 0, sizeof(mScissor));

    // Ensure viewport is within Vulkan requirements
    vk::ClampViewport(&mViewport);

    mNonIndexedDirtyBitsMask.set();
    mNonIndexedDirtyBitsMask.reset(DIRTY_BIT_INDEX_BUFFER);

    mIndexedDirtyBitsMask.set();

    // Once a command buffer is ended, all bindings (through |vkCmdBind*| calls) are lost per Vulkan
    // spec.  Once a new command buffer is allocated, we must make sure every previously bound
    // resource is bound again.
    //
    // Note that currently these dirty bits are set every time a new render pass command buffer is
    // begun.  However, using ANGLE's SecondaryCommandBuffer, the Vulkan command buffer (which is
    // the primary command buffer) is not ended, so technically we don't need to rebind these.
    mNewGraphicsCommandBufferDirtyBits = DirtyBits{
        DIRTY_BIT_RENDER_PASS,      DIRTY_BIT_COLOR_ACCESS,     DIRTY_BIT_DEPTH_STENCIL_ACCESS,
        DIRTY_BIT_PIPELINE_BINDING, DIRTY_BIT_TEXTURES,         DIRTY_BIT_VERTEX_BUFFERS,
        DIRTY_BIT_INDEX_BUFFER,     DIRTY_BIT_SHADER_RESOURCES, DIRTY_BIT_DESCRIPTOR_SETS,
        DIRTY_BIT_DRIVER_UNIFORMS,
    };
    if (getFeatures().supportsTransformFeedbackExtension.enabled ||
        getFeatures().emulateTransformFeedback.enabled)
    {
        mNewGraphicsCommandBufferDirtyBits.set(DIRTY_BIT_TRANSFORM_FEEDBACK_BUFFERS);
    }

    mNewComputeCommandBufferDirtyBits =
        DirtyBits{DIRTY_BIT_PIPELINE_BINDING, DIRTY_BIT_TEXTURES, DIRTY_BIT_SHADER_RESOURCES,
                  DIRTY_BIT_DESCRIPTOR_SETS, DIRTY_BIT_DRIVER_UNIFORMS};

    mDynamicStateDirtyBits = DirtyBits{
        DIRTY_BIT_DYNAMIC_VIEWPORT,           DIRTY_BIT_DYNAMIC_SCISSOR,
        DIRTY_BIT_DYNAMIC_LINE_WIDTH,         DIRTY_BIT_DYNAMIC_DEPTH_BIAS,
        DIRTY_BIT_DYNAMIC_BLEND_CONSTANTS,    DIRTY_BIT_DYNAMIC_STENCIL_COMPARE_MASK,
        DIRTY_BIT_DYNAMIC_STENCIL_WRITE_MASK, DIRTY_BIT_DYNAMIC_STENCIL_REFERENCE,
    };
    if (mRenderer->getFeatures().useVertexInputBindingStrideDynamicState.enabled ||
        getFeatures().supportsVertexInputDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_VERTEX_BUFFERS);
    }
    if (mRenderer->getFeatures().useCullModeDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_CULL_MODE);
    }
    if (mRenderer->getFeatures().useFrontFaceDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_FRONT_FACE);
    }
    if (mRenderer->getFeatures().useDepthTestEnableDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_TEST_ENABLE);
    }
    if (mRenderer->getFeatures().useDepthWriteEnableDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_WRITE_ENABLE);
    }
    if (mRenderer->getFeatures().useDepthCompareOpDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_COMPARE_OP);
    }
    if (mRenderer->getFeatures().useStencilTestEnableDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_TEST_ENABLE);
    }
    if (mRenderer->getFeatures().useStencilOpDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_OP);
    }
    if (mRenderer->getFeatures().usePrimitiveRestartEnableDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_PRIMITIVE_RESTART_ENABLE);
    }
    if (mRenderer->getFeatures().useRasterizerDiscardEnableDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_RASTERIZER_DISCARD_ENABLE);
    }
    if (mRenderer->getFeatures().useDepthBiasEnableDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_BIAS_ENABLE);
    }
    if (mRenderer->getFeatures().supportsLogicOpDynamicState.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_LOGIC_OP);
    }
    if (getFeatures().supportsFragmentShadingRate.enabled)
    {
        mDynamicStateDirtyBits.set(DIRTY_BIT_DYNAMIC_FRAGMENT_SHADING_RATE);
    }

    mNewGraphicsCommandBufferDirtyBits |= mDynamicStateDirtyBits;

    mGraphicsDirtyBitHandlers[DIRTY_BIT_MEMORY_BARRIER] =
        &ContextVk::handleDirtyGraphicsMemoryBarrier;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DEFAULT_ATTRIBS] =
        &ContextVk::handleDirtyGraphicsDefaultAttribs;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_PIPELINE_DESC] =
        &ContextVk::handleDirtyGraphicsPipelineDesc;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_READ_ONLY_DEPTH_FEEDBACK_LOOP_MODE] =
        &ContextVk::handleDirtyGraphicsReadOnlyDepthFeedbackLoopMode;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_ANY_SAMPLE_PASSED_QUERY_END] =
        &ContextVk::handleDirtyAnySamplePassedQueryEnd;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_RENDER_PASS]  = &ContextVk::handleDirtyGraphicsRenderPass;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_EVENT_LOG]    = &ContextVk::handleDirtyGraphicsEventLog;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_COLOR_ACCESS] = &ContextVk::handleDirtyGraphicsColorAccess;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DEPTH_STENCIL_ACCESS] =
        &ContextVk::handleDirtyGraphicsDepthStencilAccess;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_PIPELINE_BINDING] =
        &ContextVk::handleDirtyGraphicsPipelineBinding;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_TEXTURES] = &ContextVk::handleDirtyGraphicsTextures;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_VERTEX_BUFFERS] =
        &ContextVk::handleDirtyGraphicsVertexBuffers;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_INDEX_BUFFER] = &ContextVk::handleDirtyGraphicsIndexBuffer;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_UNIFORMS]     = &ContextVk::handleDirtyGraphicsUniforms;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DRIVER_UNIFORMS] =
        &ContextVk::handleDirtyGraphicsDriverUniforms;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_SHADER_RESOURCES] =
        &ContextVk::handleDirtyGraphicsShaderResources;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_UNIFORM_BUFFERS] =
        &ContextVk::handleDirtyGraphicsUniformBuffers;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_FRAMEBUFFER_FETCH_BARRIER] =
        &ContextVk::handleDirtyGraphicsFramebufferFetchBarrier;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_BLEND_BARRIER] =
        &ContextVk::handleDirtyGraphicsBlendBarrier;
    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        mGraphicsDirtyBitHandlers[DIRTY_BIT_TRANSFORM_FEEDBACK_BUFFERS] =
            &ContextVk::handleDirtyGraphicsTransformFeedbackBuffersExtension;
        mGraphicsDirtyBitHandlers[DIRTY_BIT_TRANSFORM_FEEDBACK_RESUME] =
            &ContextVk::handleDirtyGraphicsTransformFeedbackResume;
    }
    else if (getFeatures().emulateTransformFeedback.enabled)
    {
        mGraphicsDirtyBitHandlers[DIRTY_BIT_TRANSFORM_FEEDBACK_BUFFERS] =
            &ContextVk::handleDirtyGraphicsTransformFeedbackBuffersEmulation;
    }

    mGraphicsDirtyBitHandlers[DIRTY_BIT_DESCRIPTOR_SETS] =
        &ContextVk::handleDirtyGraphicsDescriptorSets;

    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_VIEWPORT] =
        &ContextVk::handleDirtyGraphicsDynamicViewport;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_SCISSOR] =
        &ContextVk::handleDirtyGraphicsDynamicScissor;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_LINE_WIDTH] =
        &ContextVk::handleDirtyGraphicsDynamicLineWidth;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_DEPTH_BIAS] =
        &ContextVk::handleDirtyGraphicsDynamicDepthBias;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_BLEND_CONSTANTS] =
        &ContextVk::handleDirtyGraphicsDynamicBlendConstants;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_STENCIL_COMPARE_MASK] =
        &ContextVk::handleDirtyGraphicsDynamicStencilCompareMask;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_STENCIL_WRITE_MASK] =
        &ContextVk::handleDirtyGraphicsDynamicStencilWriteMask;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_STENCIL_REFERENCE] =
        &ContextVk::handleDirtyGraphicsDynamicStencilReference;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_CULL_MODE] =
        &ContextVk::handleDirtyGraphicsDynamicCullMode;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_FRONT_FACE] =
        &ContextVk::handleDirtyGraphicsDynamicFrontFace;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_DEPTH_TEST_ENABLE] =
        &ContextVk::handleDirtyGraphicsDynamicDepthTestEnable;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_DEPTH_WRITE_ENABLE] =
        &ContextVk::handleDirtyGraphicsDynamicDepthWriteEnable;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_DEPTH_COMPARE_OP] =
        &ContextVk::handleDirtyGraphicsDynamicDepthCompareOp;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_STENCIL_TEST_ENABLE] =
        &ContextVk::handleDirtyGraphicsDynamicStencilTestEnable;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_STENCIL_OP] =
        &ContextVk::handleDirtyGraphicsDynamicStencilOp;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_RASTERIZER_DISCARD_ENABLE] =
        &ContextVk::handleDirtyGraphicsDynamicRasterizerDiscardEnable;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_DEPTH_BIAS_ENABLE] =
        &ContextVk::handleDirtyGraphicsDynamicDepthBiasEnable;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_LOGIC_OP] =
        &ContextVk::handleDirtyGraphicsDynamicLogicOp;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_PRIMITIVE_RESTART_ENABLE] =
        &ContextVk::handleDirtyGraphicsDynamicPrimitiveRestartEnable;
    mGraphicsDirtyBitHandlers[DIRTY_BIT_DYNAMIC_FRAGMENT_SHADING_RATE] =
        &ContextVk::handleDirtyGraphicsDynamicFragmentShadingRate;

    mComputeDirtyBitHandlers[DIRTY_BIT_MEMORY_BARRIER] =
        &ContextVk::handleDirtyComputeMemoryBarrier;
    mComputeDirtyBitHandlers[DIRTY_BIT_EVENT_LOG]     = &ContextVk::handleDirtyComputeEventLog;
    mComputeDirtyBitHandlers[DIRTY_BIT_PIPELINE_DESC] = &ContextVk::handleDirtyComputePipelineDesc;
    mComputeDirtyBitHandlers[DIRTY_BIT_PIPELINE_BINDING] =
        &ContextVk::handleDirtyComputePipelineBinding;
    mComputeDirtyBitHandlers[DIRTY_BIT_TEXTURES] = &ContextVk::handleDirtyComputeTextures;
    mComputeDirtyBitHandlers[DIRTY_BIT_UNIFORMS] = &ContextVk::handleDirtyComputeUniforms;
    mComputeDirtyBitHandlers[DIRTY_BIT_DRIVER_UNIFORMS] =
        &ContextVk::handleDirtyComputeDriverUniforms;
    mComputeDirtyBitHandlers[DIRTY_BIT_SHADER_RESOURCES] =
        &ContextVk::handleDirtyComputeShaderResources;
    mComputeDirtyBitHandlers[DIRTY_BIT_UNIFORM_BUFFERS] =
        &ContextVk::handleDirtyComputeUniformBuffers;
    mComputeDirtyBitHandlers[DIRTY_BIT_DESCRIPTOR_SETS] =
        &ContextVk::handleDirtyComputeDescriptorSets;

    mGraphicsDirtyBits = mNewGraphicsCommandBufferDirtyBits;
    mComputeDirtyBits  = mNewComputeCommandBufferDirtyBits;

    // If coherent framebuffer fetch is emulated, a barrier is implicitly issued between draw calls
    // that use framebuffer fetch.  As such, the corresponding dirty bit shouldn't be cleared until
    // a program without framebuffer fetch is used.
    if (mRenderer->isCoherentColorFramebufferFetchEmulated())
    {
        mPersistentGraphicsDirtyBits.set(DIRTY_BIT_FRAMEBUFFER_FETCH_BARRIER);
    }

    FillWithNullptr(&mActiveImages);

    // The following dirty bits don't affect the program pipeline:
    //
    // - READ_FRAMEBUFFER_BINDING only affects operations that read from said framebuffer,
    // - CLEAR_* only affect following clear calls,
    // - PACK/UNPACK_STATE only affect texture data upload/download,
    // - *_BINDING only affect descriptor sets.
    //
    // Additionally, state that is set dynamically doesn't invalidate the program pipeline.
    //
    mPipelineDirtyBitsMask.set();
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_CLEAR_COLOR);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_CLEAR_DEPTH);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_CLEAR_STENCIL);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_UNPACK_STATE);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_UNPACK_BUFFER_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_PACK_STATE);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_PACK_BUFFER_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_RENDERBUFFER_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_DRAW_INDIRECT_BUFFER_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_DISPATCH_INDIRECT_BUFFER_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_SAMPLER_BINDINGS);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_TEXTURE_BINDINGS);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_IMAGE_BINDINGS);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_TRANSFORM_FEEDBACK_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_UNIFORM_BUFFER_BINDINGS);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING);

    // Dynamic state in core Vulkan 1.0:
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_VIEWPORT);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_SCISSOR_TEST_ENABLED);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_SCISSOR);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_LINE_WIDTH);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_POLYGON_OFFSET);
    mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_BLEND_COLOR);
    if (!getFeatures().useNonZeroStencilWriteMaskStaticState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_STENCIL_WRITEMASK_BACK);
    }

    // Dynamic state in VK_EXT_extended_dynamic_state:
    if (mRenderer->getFeatures().useCullModeDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_CULL_FACE_ENABLED);
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_CULL_FACE);
    }
    if (mRenderer->getFeatures().useFrontFaceDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_FRONT_FACE);
    }
    if (mRenderer->getFeatures().useDepthTestEnableDynamicState.enabled)
    {
        // Depth test affects depth write state too in GraphicsPipelineDesc, so the pipeline needs
        // to stay dirty if depth test changes while depth write state is static.
        if (mRenderer->getFeatures().useDepthWriteEnableDynamicState.enabled)
        {
            mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_DEPTH_TEST_ENABLED);
        }
    }
    if (mRenderer->getFeatures().useDepthWriteEnableDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_DEPTH_MASK);
    }
    if (mRenderer->getFeatures().useDepthCompareOpDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_DEPTH_FUNC);
    }
    if (mRenderer->getFeatures().useStencilTestEnableDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_STENCIL_TEST_ENABLED);
    }
    if (mRenderer->getFeatures().useStencilOpDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_STENCIL_FUNCS_FRONT);
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_STENCIL_FUNCS_BACK);
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_STENCIL_OPS_FRONT);
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_STENCIL_OPS_BACK);
    }
    // Dynamic state in VK_EXT_extended_dynamic_state2:
    if (mRenderer->getFeatures().usePrimitiveRestartEnableDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED);
    }
    if (mRenderer->getFeatures().useRasterizerDiscardEnableDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED);
    }
    if (mRenderer->getFeatures().useDepthBiasEnableDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED);
    }
    if (getFeatures().supportsVertexInputDynamicState.enabled)
    {
        mPipelineDirtyBitsMask.reset(gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING);
    }

    // Stash the mRefCountedEventRecycler in vk::ErrorContext for ImageHelper to conveniently access
    mShareGroupRefCountedEventsGarbageRecycler =
        mShareGroupVk->getRefCountedEventsGarbageRecycler();

    mDeviceQueueIndex = renderer->getDeviceQueueIndex(mContextPriority);

    angle::PerfMonitorCounterGroup vulkanGroup;
    vulkanGroup.name = "vulkan";

#define ANGLE_ADD_PERF_MONITOR_COUNTER_GROUP(COUNTER) \
    {                                                 \
        angle::PerfMonitorCounter counter;            \
        counter.name  = #COUNTER;                     \
        counter.value = 0;                            \
        vulkanGroup.counters.push_back(counter);      \
    }

    ANGLE_VK_PERF_COUNTERS_X(ANGLE_ADD_PERF_MONITOR_COUNTER_GROUP)

#undef ANGLE_ADD_PERF_MONITOR_COUNTER_GROUP

    mPerfMonitorCounters.push_back(vulkanGroup);
}

ContextVk::~ContextVk() {}

void ContextVk::onDestroy(const gl::Context *context)
{
    // If there is a context lost, destroy all the command buffers and resources regardless of
    // whether they finished execution on GPU.
    if (mRenderer->isDeviceLost())
    {
        mRenderer->handleDeviceLost();
    }

    // This will not destroy any resources. It will release them to be collected after finish.
    mIncompleteTextures.onDestroy(context);

    // Flush and complete current outstanding work before destruction.
    (void)finishImpl(RenderPassClosureReason::ContextDestruction);

    // The finish call could also generate device loss.
    if (mRenderer->isDeviceLost())
    {
        mRenderer->handleDeviceLost();
    }

    // Everything must be finished
    ASSERT(mRenderer->hasResourceUseFinished(mSubmittedResourceUse));

    VkDevice device = getDevice();

    mShareGroupVk->cleanupRefCountedEventGarbage();

    mDefaultUniformStorage.release(this);
    mEmptyBuffer.release(this);

    for (vk::DynamicBuffer &defaultBuffer : mStreamedVertexBuffers)
    {
        defaultBuffer.destroy(mRenderer);
    }

    for (vk::DynamicQueryPool &queryPool : mQueryPools)
    {
        queryPool.destroy(device);
    }

    // Recycle current command buffers.

    // Release functions are only used for Vulkan secondary command buffers.
    mOutsideRenderPassCommands->releaseCommandPool();
    mRenderPassCommands->releaseCommandPool();

    // Detach functions are only used for ring buffer allocators.
    mOutsideRenderPassCommands->detachAllocator();
    mRenderPassCommands->detachAllocator();

    mRenderer->recycleOutsideRenderPassCommandBufferHelper(&mOutsideRenderPassCommands);
    mRenderer->recycleRenderPassCommandBufferHelper(&mRenderPassCommands);

    mInterfacePipelinesCache.destroy(device);

    mUtils.destroy(this);

    mRenderPassCache.destroy(this);
    mShaderLibrary.destroy(device);
    mGpuEventQueryPool.destroy(device);

    // Must release all Vulkan secondary command buffers before destroying the pools.
    if ((!vk::OutsideRenderPassCommandBuffer::ExecutesInline() ||
         !vk::RenderPassCommandBuffer::ExecutesInline()) &&
        mRenderer->isAsyncCommandBufferResetAndGarbageCleanupEnabled())
    {
        // This will also reset Primary command buffers which is REQUIRED on some buggy Vulkan
        // implementations.
        (void)mRenderer->releaseFinishedCommands(this);
    }

    mCommandPools.outsideRenderPassPool.destroy(device);
    mCommandPools.renderPassPool.destroy(device);

    ASSERT(mCurrentGarbage.empty());

    if (mCurrentQueueSerialIndex != kInvalidQueueSerialIndex)
    {
        releaseQueueSerialIndex();
    }

    mImageLoadContext = {};
}

VertexArrayVk *ContextVk::getVertexArray() const
{
    return vk::GetImpl(mState.getVertexArray());
}

FramebufferVk *ContextVk::getDrawFramebuffer() const
{
    return vk::GetImpl(mState.getDrawFramebuffer());
}

angle::Result ContextVk::getIncompleteTexture(const gl::Context *context,
                                              gl::TextureType type,
                                              gl::SamplerFormat format,
                                              gl::Texture **textureOut)
{
    return mIncompleteTextures.getIncompleteTexture(context, type, format, this, textureOut);
}

angle::Result ContextVk::initialize(const angle::ImageLoadContext &imageLoadContext)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::initialize");

    mImageLoadContext = imageLoadContext;

    ANGLE_TRY(mShareGroupVk->unifyContextsPriority(this));

    ANGLE_TRY(mQueryPools[gl::QueryType::AnySamples].init(this, VK_QUERY_TYPE_OCCLUSION,
                                                          vk::kDefaultOcclusionQueryPoolSize));
    ANGLE_TRY(mQueryPools[gl::QueryType::AnySamplesConservative].init(
        this, VK_QUERY_TYPE_OCCLUSION, vk::kDefaultOcclusionQueryPoolSize));

    // Only initialize the timestamp query pools if the extension is available.
    if (mRenderer->getQueueFamilyProperties().timestampValidBits > 0)
    {
        ANGLE_TRY(mQueryPools[gl::QueryType::Timestamp].init(this, VK_QUERY_TYPE_TIMESTAMP,
                                                             vk::kDefaultTimestampQueryPoolSize));
        ANGLE_TRY(mQueryPools[gl::QueryType::TimeElapsed].init(this, VK_QUERY_TYPE_TIMESTAMP,
                                                               vk::kDefaultTimestampQueryPoolSize));
    }

    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        ANGLE_TRY(mQueryPools[gl::QueryType::TransformFeedbackPrimitivesWritten].init(
            this, VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT,
            vk::kDefaultTransformFeedbackQueryPoolSize));
    }

    // If VK_EXT_primitives_generated_query is supported, use that to implement the OpenGL query.
    // Otherwise, the primitives generated query is provided through the Vulkan pipeline statistics
    // query if supported.
    if (getFeatures().supportsPrimitivesGeneratedQuery.enabled)
    {
        ANGLE_TRY(mQueryPools[gl::QueryType::PrimitivesGenerated].init(
            this, VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT,
            vk::kDefaultPrimitivesGeneratedQueryPoolSize));
    }
    else if (getFeatures().supportsPipelineStatisticsQuery.enabled)
    {
        ANGLE_TRY(mQueryPools[gl::QueryType::PrimitivesGenerated].init(
            this, VK_QUERY_TYPE_PIPELINE_STATISTICS, vk::kDefaultPrimitivesGeneratedQueryPoolSize));
    }

    // Init GLES to Vulkan index type map.
    initIndexTypeMap();

    mGraphicsPipelineDesc.reset(new vk::GraphicsPipelineDesc());
    mGraphicsPipelineDesc->initDefaults(this, vk::GraphicsPipelineSubset::Complete,
                                        pipelineRobustness(), pipelineProtectedAccess());

    // Initialize current value/default attribute buffers.
    for (vk::DynamicBuffer &buffer : mStreamedVertexBuffers)
    {
        buffer.init(mRenderer, kVertexBufferUsage, vk::kVertexBufferAlignment,
                    kDynamicVertexDataSize, true);
    }

#if ANGLE_ENABLE_VULKAN_GPU_TRACE_EVENTS
    angle::PlatformMethods *platform = ANGLEPlatformCurrent();
    ASSERT(platform);

    // GPU tracing workaround for anglebug.com/42261625.  The renderer should not emit gpu events
    // during platform discovery.
    const unsigned char *gpuEventsEnabled =
        platform->getTraceCategoryEnabledFlag(platform, "gpu.angle.gpu");
    mGpuEventsEnabled = gpuEventsEnabled && *gpuEventsEnabled;
#endif

    // Assign initial command buffers from queue
    ANGLE_TRY(vk::OutsideRenderPassCommandBuffer::InitializeCommandPool(
        this, &mCommandPools.outsideRenderPassPool, mRenderer->getQueueFamilyIndex(),
        getProtectionType()));
    ANGLE_TRY(vk::RenderPassCommandBuffer::InitializeCommandPool(
        this, &mCommandPools.renderPassPool, mRenderer->getQueueFamilyIndex(),
        getProtectionType()));
    ANGLE_TRY(mRenderer->getOutsideRenderPassCommandBufferHelper(
        this, &mCommandPools.outsideRenderPassPool, &mOutsideRenderPassCommandsAllocator,
        &mOutsideRenderPassCommands));
    ANGLE_TRY(mRenderer->getRenderPassCommandBufferHelper(
        this, &mCommandPools.renderPassPool, &mRenderPassCommandsAllocator, &mRenderPassCommands));

    // Allocate queueSerial index and generate queue serial for commands.
    ANGLE_TRY(allocateQueueSerialIndex());

    // Initialize serials to be valid but appear submitted and finished.
    mLastFlushedQueueSerial   = QueueSerial(mCurrentQueueSerialIndex, Serial());
    mLastSubmittedQueueSerial = mLastFlushedQueueSerial;

    if (mGpuEventsEnabled)
    {
        // GPU events should only be available if timestamp queries are available.
        ASSERT(mRenderer->getQueueFamilyProperties().timestampValidBits > 0);
        // Calculate the difference between CPU and GPU clocks for GPU event reporting.
        ANGLE_TRY(mGpuEventQueryPool.init(this, VK_QUERY_TYPE_TIMESTAMP,
                                          vk::kDefaultTimestampQueryPoolSize));
        ANGLE_TRY(synchronizeCpuGpuTime());

        EventName eventName = GetTraceEventName("Primary", mPrimaryBufferEventCounter);
        ANGLE_TRY(traceGpuEvent(&mOutsideRenderPassCommands->getCommandBuffer(),
                                TRACE_EVENT_PHASE_BEGIN, eventName));
    }

    size_t minAlignment = static_cast<size_t>(
        mRenderer->getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment);
    mDefaultUniformStorage.init(mRenderer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, minAlignment,
                                mRenderer->getDefaultUniformBufferSize(), true);

    // Initialize an "empty" buffer for use with default uniform blocks where there are no uniforms,
    // or atomic counter buffer array indices that are unused.
    constexpr VkBufferUsageFlags kEmptyBufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkBufferCreateInfo emptyBufferInfo          = {};
    emptyBufferInfo.sType                       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    emptyBufferInfo.flags                       = 0;
    emptyBufferInfo.size                        = 16;
    emptyBufferInfo.usage                       = kEmptyBufferUsage;
    emptyBufferInfo.sharingMode                 = VK_SHARING_MODE_EXCLUSIVE;
    emptyBufferInfo.queueFamilyIndexCount       = 0;
    emptyBufferInfo.pQueueFamilyIndices         = nullptr;
    constexpr VkMemoryPropertyFlags kMemoryType = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    ANGLE_TRY(mEmptyBuffer.init(this, emptyBufferInfo, kMemoryType));

    // If the share group has one context and is about to add the second one, the first context's
    // mutable textures should be flushed.
    if (isEligibleForMutableTextureFlush())
    {
        ASSERT(mShareGroupVk->getContexts().size() == 1);
        for (auto context : mShareGroupVk->getContexts())
        {
            ANGLE_TRY(vk::GetImpl(context.second)->flushOutsideRenderPassCommands());
        }
    }

    return angle::Result::Continue;
}

bool ContextVk::isSingleBufferedWindowCurrent() const
{
    return (mCurrentWindowSurface != nullptr && mCurrentWindowSurface->isSharedPresentMode());
}

bool ContextVk::hasSomethingToFlush() const
{
    // Don't skip flushes for single-buffered windows with staged updates. It is expected that a
    // flush call on a single-buffered window ensures any pending updates reach the screen.
    const bool isSingleBufferedWindowWithStagedUpdates =
        isSingleBufferedWindowCurrent() && mCurrentWindowSurface->hasStagedUpdates();

    return (mHasAnyCommandsPendingSubmission || hasActiveRenderPass() ||
            !mOutsideRenderPassCommands->empty() || isSingleBufferedWindowWithStagedUpdates);
}

angle::Result ContextVk::flushImpl(const gl::Context *context)
{
    // Skip if there's nothing to flush.
    if (!hasSomethingToFlush())
    {
        return angle::Result::Continue;
    }

    // Don't defer flushes when performing front buffer rendering. This can happen when -
    // 1. we have a single-buffered window, in this mode the application is not required to
    //    call eglSwapBuffers(), and glFlush() is expected to ensure that work is submitted.
    // 2. the framebuffer attachment has FRONT_BUFFER usage. Attachments being rendered to with such
    //    usage flags are expected to behave similar to a single-buffered window
    FramebufferVk *drawFramebufferVk = getDrawFramebuffer();
    ASSERT(drawFramebufferVk == vk::GetImpl(mState.getDrawFramebuffer()));
    const bool isSingleBufferedWindow = isSingleBufferedWindowCurrent();
    const bool frontBufferRenderingEnabled =
        isSingleBufferedWindow || drawFramebufferVk->hasFrontBufferUsage();

    if (hasActiveRenderPass() && !frontBufferRenderingEnabled)
    {
        mHasDeferredFlush = true;
        return angle::Result::Continue;
    }

    if (isSingleBufferedWindow &&
        mRenderer->getFeatures().swapbuffersOnFlushOrFinishWithSingleBuffer.enabled)
    {
        return mCurrentWindowSurface->onSharedPresentContextFlush(context);
    }

    return flushAndSubmitCommands(nullptr, nullptr, RenderPassClosureReason::GLFlush);
}

angle::Result ContextVk::flush(const gl::Context *context)
{
    ANGLE_TRY(flushImpl(context));

    if (!mCurrentWindowSurface || isSingleBufferedWindowCurrent())
    {
        ANGLE_TRY(onFramebufferBoundary(context));
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::finish(const gl::Context *context)
{
    const bool singleBufferedFlush = isSingleBufferedWindowCurrent() && hasSomethingToFlush();

    if (mRenderer->getFeatures().swapbuffersOnFlushOrFinishWithSingleBuffer.enabled &&
        singleBufferedFlush)
    {
        ANGLE_TRY(mCurrentWindowSurface->onSharedPresentContextFlush(context));
        // While call above performs implicit flush, don't skip |finishImpl| below, since we still
        // need to wait for submitted commands.
    }

    ANGLE_TRY(finishImpl(RenderPassClosureReason::GLFinish));

    syncObjectPerfCounters(mRenderer->getCommandQueuePerfCounters());

    if (!mCurrentWindowSurface || singleBufferedFlush)
    {
        ANGLE_TRY(onFramebufferBoundary(context));
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::onFramebufferBoundary(const gl::Context *contextGL)
{
    mShareGroupVk->onFramebufferBoundary();
    return mRenderer->syncPipelineCacheVk(this, mRenderer->getGlobalOps(), contextGL);
}

angle::Result ContextVk::setupDraw(const gl::Context *context,
                                   gl::PrimitiveMode mode,
                                   GLint firstVertexOrInvalid,
                                   GLsizei vertexOrIndexCount,
                                   GLsizei instanceCount,
                                   gl::DrawElementsType indexTypeOrInvalid,
                                   const void *indices,
                                   DirtyBits dirtyBitMask)
{
    // Set any dirty bits that depend on draw call parameters or other objects.
    if (mode != mCurrentDrawMode)
    {
        invalidateCurrentGraphicsPipeline();
        mCurrentDrawMode = mode;
        mGraphicsPipelineDesc->updateTopology(&mGraphicsPipelineTransition, mCurrentDrawMode);
    }

    // Must be called before the command buffer is started. Can call finish.
    VertexArrayVk *vertexArrayVk = getVertexArray();
    if (vertexArrayVk->getStreamingVertexAttribsMask().any())
    {
        // All client attribs & any emulated buffered attribs will be updated
        ANGLE_TRY(vertexArrayVk->updateStreamedAttribs(context, firstVertexOrInvalid,
                                                       vertexOrIndexCount, instanceCount,
                                                       indexTypeOrInvalid, indices));

        mGraphicsDirtyBits.set(DIRTY_BIT_VERTEX_BUFFERS);
    }

    ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
    if (executableVk->updateAndCheckDirtyUniforms())
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_UNIFORMS);
    }

    // Update transform feedback offsets on every draw call when emulating transform feedback.  This
    // relies on the fact that no geometry/tessellation, indirect or indexed calls are supported in
    // ES3.1 (and emulation is not done for ES3.2).
    if (getFeatures().emulateTransformFeedback.enabled &&
        mState.isTransformFeedbackActiveUnpaused())
    {
        ASSERT(firstVertexOrInvalid != -1);
        mXfbBaseVertex             = firstVertexOrInvalid;
        mXfbVertexCountPerInstance = vertexOrIndexCount;
        invalidateGraphicsDriverUniforms();
    }

    DirtyBits dirtyBits = mGraphicsDirtyBits & dirtyBitMask;

    if (dirtyBits.any())
    {
        // Flush any relevant dirty bits.
        for (DirtyBits::Iterator dirtyBitIter = dirtyBits.begin(); dirtyBitIter != dirtyBits.end();
             ++dirtyBitIter)
        {
            ASSERT(mGraphicsDirtyBitHandlers[*dirtyBitIter]);
            ANGLE_TRY(
                (this->*mGraphicsDirtyBitHandlers[*dirtyBitIter])(&dirtyBitIter, dirtyBitMask));
        }

        // Reset the processed dirty bits, except for those that are expected to persist between
        // draw calls (such as the framebuffer fetch barrier which needs to be issued again and
        // again).
        mGraphicsDirtyBits &= (~dirtyBitMask | mPersistentGraphicsDirtyBits);
    }

    // Render pass must be always available at this point.
    ASSERT(hasActiveRenderPass());

    ASSERT(mState.getAndResetDirtyUniformBlocks().none());

    return angle::Result::Continue;
}

angle::Result ContextVk::setupIndexedDraw(const gl::Context *context,
                                          gl::PrimitiveMode mode,
                                          GLsizei indexCount,
                                          GLsizei instanceCount,
                                          gl::DrawElementsType indexType,
                                          const void *indices)
{
    ASSERT(mode != gl::PrimitiveMode::LineLoop);

    if (indexType != mCurrentDrawElementsType)
    {
        mCurrentDrawElementsType = indexType;
        ANGLE_TRY(onIndexBufferChange(nullptr));
    }

    VertexArrayVk *vertexArrayVk         = getVertexArray();
    const gl::Buffer *elementArrayBuffer = vertexArrayVk->getState().getElementArrayBuffer();
    if (!elementArrayBuffer)
    {
        BufferBindingDirty bindingDirty;
        ANGLE_TRY(vertexArrayVk->convertIndexBufferCPU(this, indexType, indexCount, indices,
                                                       &bindingDirty));
        mCurrentIndexBufferOffset = 0;

        // We only set dirty bit when the bound buffer actually changed.
        if (bindingDirty == BufferBindingDirty::Yes)
        {
            mGraphicsDirtyBits.set(DIRTY_BIT_INDEX_BUFFER);
        }
    }
    else
    {
        mCurrentIndexBufferOffset = reinterpret_cast<VkDeviceSize>(indices);

        if (indices != mLastIndexBufferOffset)
        {
            mGraphicsDirtyBits.set(DIRTY_BIT_INDEX_BUFFER);
            mLastIndexBufferOffset = indices;
        }

        // When you draw with LineLoop mode or GL_UNSIGNED_BYTE type, we may allocate its own
        // element buffer and modify mCurrentElementArrayBuffer. When we switch out of that draw
        // mode, we must reset mCurrentElementArrayBuffer back to the vertexArray's element buffer.
        // Since in either case we set DIRTY_BIT_INDEX_BUFFER dirty bit, we use this bit to re-sync
        // mCurrentElementArrayBuffer.
        if (mGraphicsDirtyBits[DIRTY_BIT_INDEX_BUFFER])
        {
            vertexArrayVk->updateCurrentElementArrayBuffer();
        }

        if (shouldConvertUint8VkIndexType(indexType) && mGraphicsDirtyBits[DIRTY_BIT_INDEX_BUFFER])
        {
            ANGLE_VK_PERF_WARNING(this, GL_DEBUG_SEVERITY_LOW,
                                  "Potential inefficiency emulating uint8 vertex attributes due to "
                                  "lack of hardware support");

            BufferVk *bufferVk             = vk::GetImpl(elementArrayBuffer);
            vk::BufferHelper &bufferHelper = bufferVk->getBuffer();

            if (bufferHelper.isHostVisible() &&
                mRenderer->hasResourceUseFinished(bufferHelper.getResourceUse()))
            {
                uint8_t *src = nullptr;
                ANGLE_TRY(
                    bufferVk->mapImpl(this, GL_MAP_READ_BIT, reinterpret_cast<void **>(&src)));
                // Note: bufferOffset is not added here because mapImpl already adds it.
                src += reinterpret_cast<uintptr_t>(indices);
                const size_t byteCount = static_cast<size_t>(elementArrayBuffer->getSize()) -
                                         reinterpret_cast<uintptr_t>(indices);
                BufferBindingDirty bindingDirty;
                ANGLE_TRY(vertexArrayVk->convertIndexBufferCPU(this, indexType, byteCount, src,
                                                               &bindingDirty));
                ANGLE_TRY(bufferVk->unmapImpl(this));
            }
            else
            {
                ANGLE_TRY(vertexArrayVk->convertIndexBufferGPU(this, bufferVk, indices));
            }

            mCurrentIndexBufferOffset = 0;
        }
    }

    mCurrentIndexBuffer = vertexArrayVk->getCurrentElementArrayBuffer();
    return setupDraw(context, mode, 0, indexCount, instanceCount, indexType, indices,
                     mIndexedDirtyBitsMask);
}

angle::Result ContextVk::setupIndirectDraw(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           DirtyBits dirtyBitMask,
                                           vk::BufferHelper *indirectBuffer)
{
    GLint firstVertex     = -1;
    GLsizei vertexCount   = 0;
    GLsizei instanceCount = 1;

    // Break the render pass if the indirect buffer was previously used as the output from transform
    // feedback.
    if (mCurrentTransformFeedbackQueueSerial.valid() &&
        indirectBuffer->writtenByCommandBuffer(mCurrentTransformFeedbackQueueSerial))
    {
        ANGLE_TRY(
            flushCommandsAndEndRenderPass(RenderPassClosureReason::XfbWriteThenIndirectDrawBuffer));
    }

    ANGLE_TRY(setupDraw(context, mode, firstVertex, vertexCount, instanceCount,
                        gl::DrawElementsType::InvalidEnum, nullptr, dirtyBitMask));

    // Process indirect buffer after render pass has started.
    mRenderPassCommands->bufferRead(this, VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                    vk::PipelineStage::DrawIndirect, indirectBuffer);

    return angle::Result::Continue;
}

angle::Result ContextVk::setupIndexedIndirectDraw(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  gl::DrawElementsType indexType,
                                                  vk::BufferHelper *indirectBuffer)
{
    ASSERT(mode != gl::PrimitiveMode::LineLoop);

    VertexArrayVk *vertexArrayVk = getVertexArray();
    mCurrentIndexBuffer          = vertexArrayVk->getCurrentElementArrayBuffer();
    if (indexType != mCurrentDrawElementsType)
    {
        mCurrentDrawElementsType = indexType;
        ANGLE_TRY(onIndexBufferChange(nullptr));
    }

    return setupIndirectDraw(context, mode, mIndexedDirtyBitsMask, indirectBuffer);
}

angle::Result ContextVk::setupLineLoopIndexedIndirectDraw(const gl::Context *context,
                                                          gl::PrimitiveMode mode,
                                                          gl::DrawElementsType indexType,
                                                          vk::BufferHelper *srcIndexBuffer,
                                                          vk::BufferHelper *srcIndirectBuffer,
                                                          VkDeviceSize indirectBufferOffset,
                                                          vk::BufferHelper **indirectBufferOut)
{
    ASSERT(mode == gl::PrimitiveMode::LineLoop);

    vk::BufferHelper *dstIndexBuffer    = nullptr;
    vk::BufferHelper *dstIndirectBuffer = nullptr;

    VertexArrayVk *vertexArrayVk = getVertexArray();
    ANGLE_TRY(vertexArrayVk->handleLineLoopIndexIndirect(this, indexType, srcIndexBuffer,
                                                         srcIndirectBuffer, indirectBufferOffset,
                                                         &dstIndexBuffer, &dstIndirectBuffer));

    mCurrentIndexBuffer = dstIndexBuffer;
    *indirectBufferOut  = dstIndirectBuffer;

    if (indexType != mCurrentDrawElementsType)
    {
        mCurrentDrawElementsType = indexType;
        ANGLE_TRY(onIndexBufferChange(nullptr));
    }

    return setupIndirectDraw(context, mode, mIndexedDirtyBitsMask, dstIndirectBuffer);
}

angle::Result ContextVk::setupLineLoopIndirectDraw(const gl::Context *context,
                                                   gl::PrimitiveMode mode,
                                                   vk::BufferHelper *indirectBuffer,
                                                   VkDeviceSize indirectBufferOffset,
                                                   vk::BufferHelper **indirectBufferOut)
{
    ASSERT(mode == gl::PrimitiveMode::LineLoop);

    vk::BufferHelper *indexBufferHelperOut    = nullptr;
    vk::BufferHelper *indirectBufferHelperOut = nullptr;

    VertexArrayVk *vertexArrayVk = getVertexArray();
    ANGLE_TRY(vertexArrayVk->handleLineLoopIndirectDraw(context, indirectBuffer,
                                                        indirectBufferOffset, &indexBufferHelperOut,
                                                        &indirectBufferHelperOut));

    *indirectBufferOut = indirectBufferHelperOut;
    mCurrentIndexBuffer = indexBufferHelperOut;

    if (gl::DrawElementsType::UnsignedInt != mCurrentDrawElementsType)
    {
        mCurrentDrawElementsType = gl::DrawElementsType::UnsignedInt;
        ANGLE_TRY(onIndexBufferChange(nullptr));
    }

    return setupIndirectDraw(context, mode, mIndexedDirtyBitsMask, indirectBufferHelperOut);
}

angle::Result ContextVk::setupLineLoopDraw(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           GLint firstVertex,
                                           GLsizei vertexOrIndexCount,
                                           gl::DrawElementsType indexTypeOrInvalid,
                                           const void *indices,
                                           uint32_t *numIndicesOut)
{
    mCurrentIndexBufferOffset    = 0;
    vk::BufferHelper *dstIndexBuffer = mCurrentIndexBuffer;

    VertexArrayVk *vertexArrayVk = getVertexArray();
    ANGLE_TRY(vertexArrayVk->handleLineLoop(this, firstVertex, vertexOrIndexCount,
                                            indexTypeOrInvalid, indices, &dstIndexBuffer,
                                            numIndicesOut));

    mCurrentIndexBuffer = dstIndexBuffer;
    ANGLE_TRY(onIndexBufferChange(nullptr));
    mCurrentDrawElementsType = indexTypeOrInvalid != gl::DrawElementsType::InvalidEnum
                                   ? indexTypeOrInvalid
                                   : gl::DrawElementsType::UnsignedInt;
    return setupDraw(context, mode, firstVertex, vertexOrIndexCount, 1, indexTypeOrInvalid, indices,
                     mIndexedDirtyBitsMask);
}

angle::Result ContextVk::setupDispatch(const gl::Context *context)
{
    // TODO: We don't currently check if this flush is necessary.  It serves to make sure the
    // barriers issued during dirty bit handling aren't reordered too early.
    // http://anglebug.com/382090958
    ANGLE_TRY(flushOutsideRenderPassCommands());

    ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
    if (executableVk->updateAndCheckDirtyUniforms())
    {
        mComputeDirtyBits.set(DIRTY_BIT_UNIFORMS);
    }

    DirtyBits dirtyBits = mComputeDirtyBits;

    // Flush any relevant dirty bits.
    for (DirtyBits::Iterator dirtyBitIter = dirtyBits.begin(); dirtyBitIter != dirtyBits.end();
         ++dirtyBitIter)
    {
        ASSERT(mComputeDirtyBitHandlers[*dirtyBitIter]);
        ANGLE_TRY((this->*mComputeDirtyBitHandlers[*dirtyBitIter])(&dirtyBitIter));
    }

    mComputeDirtyBits.reset();

    ASSERT(mState.getAndResetDirtyUniformBlocks().none());

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsMemoryBarrier(DirtyBits::Iterator *dirtyBitsIterator,
                                                          DirtyBits dirtyBitMask)
{
    return handleDirtyMemoryBarrierImpl(dirtyBitsIterator, dirtyBitMask);
}

angle::Result ContextVk::handleDirtyComputeMemoryBarrier(DirtyBits::Iterator *dirtyBitsIterator)
{
    return handleDirtyMemoryBarrierImpl(nullptr, {});
}

bool ContextVk::renderPassUsesStorageResources() const
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    if (!mRenderPassCommands->started())
    {
        return false;
    }

    // Storage images:
    for (size_t imageUnitIndex : executable->getActiveImagesMask())
    {
        const gl::Texture *texture = mState.getImageUnit(imageUnitIndex).texture.get();
        if (texture == nullptr)
        {
            continue;
        }

        TextureVk *textureVk = vk::GetImpl(texture);

        if (texture->getType() == gl::TextureType::Buffer)
        {
            vk::BufferHelper &buffer = vk::GetImpl(textureVk->getBuffer().get())->getBuffer();
            if (mRenderPassCommands->usesBuffer(buffer))
            {
                return true;
            }
        }
        else
        {
            vk::ImageHelper &image = textureVk->getImage();
            // Images only need to close the render pass if they need a layout transition.  Outside
            // render pass command buffer doesn't need closing as the layout transition barriers are
            // recorded in sequence with the rest of the commands.
            if (mRenderPassCommands->usesImage(image))
            {
                return true;
            }
        }
    }

    // Storage buffers:
    const std::vector<gl::InterfaceBlock> &blocks = executable->getShaderStorageBlocks();
    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
    {
        const uint32_t binding = executable->getShaderStorageBlockBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            mState.getIndexedShaderStorageBuffer(binding);

        if (bufferBinding.get() == nullptr)
        {
            continue;
        }

        vk::BufferHelper &buffer = vk::GetImpl(bufferBinding.get())->getBuffer();
        if (mRenderPassCommands->usesBuffer(buffer))
        {
            return true;
        }
    }

    // Atomic counters:
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers =
        executable->getAtomicCounterBuffers();
    for (uint32_t bufferIndex = 0; bufferIndex < atomicCounterBuffers.size(); ++bufferIndex)
    {
        const uint32_t binding = executable->getAtomicCounterBufferBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            mState.getIndexedAtomicCounterBuffer(binding);

        if (bufferBinding.get() == nullptr)
        {
            continue;
        }

        vk::BufferHelper &buffer = vk::GetImpl(bufferBinding.get())->getBuffer();
        if (mRenderPassCommands->usesBuffer(buffer))
        {
            return true;
        }
    }

    return false;
}

angle::Result ContextVk::handleDirtyMemoryBarrierImpl(DirtyBits::Iterator *dirtyBitsIterator,
                                                      DirtyBits dirtyBitMask)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    const bool hasImages         = executable->hasImages();
    const bool hasStorageBuffers = executable->hasStorageBuffers();
    const bool hasAtomicCounters = executable->hasAtomicCounterBuffers();

    if (!hasImages && !hasStorageBuffers && !hasAtomicCounters)
    {
        return angle::Result::Continue;
    }

    // Break the render pass if necessary.  This is only needed for write-after-read situations, and
    // is done by checking whether current storage buffers and images are used in the render pass.
    if (renderPassUsesStorageResources())
    {
        // Either set later bits (if called during handling of graphics dirty bits), or set the
        // dirty bits directly (if called during handling of compute dirty bits).
        if (dirtyBitsIterator)
        {
            return flushDirtyGraphicsRenderPass(
                dirtyBitsIterator, dirtyBitMask,
                RenderPassClosureReason::GLMemoryBarrierThenStorageResource);
        }
        else
        {
            return flushCommandsAndEndRenderPass(
                RenderPassClosureReason::GLMemoryBarrierThenStorageResource);
        }
    }

    // Flushing outside render pass commands is cheap.  If a memory barrier has been issued in its
    // life time, just flush it instead of wasting time trying to figure out if it's necessary.
    if (mOutsideRenderPassCommands->hasGLMemoryBarrierIssued())
    {
        ANGLE_TRY(flushOutsideRenderPassCommands());
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsEventLog(DirtyBits::Iterator *dirtyBitsIterator,
                                                     DirtyBits dirtyBitMask)
{
    return handleDirtyEventLogImpl(mRenderPassCommandBuffer);
}

angle::Result ContextVk::handleDirtyComputeEventLog(DirtyBits::Iterator *dirtyBitsIterator)
{
    return handleDirtyEventLogImpl(&mOutsideRenderPassCommands->getCommandBuffer());
}

template <typename CommandBufferT>
angle::Result ContextVk::handleDirtyEventLogImpl(CommandBufferT *commandBuffer)
{
    // This method is called when a draw or dispatch command is being processed.  It's purpose is
    // to call the vkCmd*DebugUtilsLabelEXT functions in order to communicate to debuggers
    // (e.g. AGI) the OpenGL ES commands that the application uses.

    // Exit early if no OpenGL ES commands have been logged, or if no command buffer (for a no-op
    // draw), or if calling the vkCmd*DebugUtilsLabelEXT functions is not enabled.
    if (mEventLog.empty() || commandBuffer == nullptr || !mRenderer->angleDebuggerMode())
    {
        return angle::Result::Continue;
    }

    // Insert OpenGL ES commands into debug label.  We create a 3-level cascade here for
    // OpenGL-ES-first debugging in AGI.  Here's the general outline of commands:
    // -glDrawCommand
    // --vkCmdBeginDebugUtilsLabelEXT() #1 for "glDrawCommand"
    // --OpenGL ES Commands
    // ---vkCmdBeginDebugUtilsLabelEXT() #2 for "OpenGL ES Commands"
    // ---Individual OpenGL ES Commands leading up to glDrawCommand
    // ----vkCmdBeginDebugUtilsLabelEXT() #3 for each individual OpenGL ES Command
    // ----vkCmdEndDebugUtilsLabelEXT() #3 for each individual OpenGL ES Command
    // ----...More Individual OGL Commands...
    // ----Final Individual OGL command will be the same glDrawCommand shown in #1 above
    // ---vkCmdEndDebugUtilsLabelEXT() #2 for "OpenGL ES Commands"
    // --VK SetupDraw & Draw-related commands will be embedded here under glDraw #1
    // --vkCmdEndDebugUtilsLabelEXT() #1 is called after each vkDraw* or vkDispatch* call

    // AGI desires no parameters on the top-level of the hierarchy.
    std::string topLevelCommand = mEventLog.back();
    size_t startOfParameters    = topLevelCommand.find("(");
    if (startOfParameters != std::string::npos)
    {
        topLevelCommand = topLevelCommand.substr(0, startOfParameters);
    }
    VkDebugUtilsLabelEXT label = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                                  nullptr,
                                  topLevelCommand.c_str(),
                                  {0.0f, 0.0f, 0.0f, 0.0f}};
    // This is #1 from comment above
    commandBuffer->beginDebugUtilsLabelEXT(label);
    std::string oglCmds = "OpenGL ES Commands";
    label.pLabelName    = oglCmds.c_str();
    // This is #2 from comment above
    commandBuffer->beginDebugUtilsLabelEXT(label);
    for (uint32_t i = 0; i < mEventLog.size(); ++i)
    {
        label.pLabelName = mEventLog[i].c_str();
        // NOTE: We have to use a begin/end pair here because AGI does not promote the
        // pLabelName from an insertDebugUtilsLabelEXT() call to the Commands panel.
        // Internal bug b/169243237 is tracking this and once the insert* call shows the
        // pLabelName similar to begin* call, we can switch these to insert* calls instead.
        // This is #3 from comment above.
        commandBuffer->beginDebugUtilsLabelEXT(label);
        commandBuffer->endDebugUtilsLabelEXT();
    }
    commandBuffer->endDebugUtilsLabelEXT();
    // The final end* call for #1 above is made in the ContextVk::draw* or
    //  ContextVk::dispatch* function calls.

    mEventLog.clear();
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDefaultAttribs(DirtyBits::Iterator *dirtyBitsIterator,
                                                           DirtyBits dirtyBitMask)
{
    ASSERT(mDirtyDefaultAttribsMask.any());

    gl::AttributesMask attribsMask =
        mDirtyDefaultAttribsMask & mState.getProgramExecutable()->getAttributesMask();
    VertexArrayVk *vertexArrayVk = getVertexArray();
    for (size_t attribIndex : attribsMask)
    {
        ANGLE_TRY(vertexArrayVk->updateDefaultAttrib(this, attribIndex));
    }

    mDirtyDefaultAttribsMask.reset();
    return angle::Result::Continue;
}

angle::Result ContextVk::createGraphicsPipeline()
{
    ASSERT(mState.getProgramExecutable() != nullptr);
    ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
    ASSERT(executableVk);

    // Wait for any warm up task if necessary
    executableVk->waitForGraphicsPostLinkTasks(this, *mGraphicsPipelineDesc);

    vk::PipelineCacheAccess pipelineCache;
    ANGLE_TRY(mRenderer->getPipelineCache(this, &pipelineCache));

    vk::PipelineHelper *oldGraphicsPipeline = mCurrentGraphicsPipeline;

    // Attempt to use an existing pipeline.
    const vk::GraphicsPipelineDesc *descPtr = nullptr;
    ANGLE_TRY(executableVk->getGraphicsPipeline(this, vk::GraphicsPipelineSubset::Complete,
                                                *mGraphicsPipelineDesc, &descPtr,
                                                &mCurrentGraphicsPipeline));

    // If no such pipeline exists:
    //
    // - If VK_EXT_graphics_pipeline_library is not supported, create a new monolithic pipeline
    // - If VK_EXT_graphics_pipeline_library is supported:
    //   * Create the Shaders subset of the pipeline through the program executable
    //   * Create the VertexInput and FragmentOutput subsets
    //   * Link them together through the program executable
    if (mCurrentGraphicsPipeline == nullptr)
    {
        // Not found in cache
        ASSERT(descPtr == nullptr);
        if (!getFeatures().supportsGraphicsPipelineLibrary.enabled)
        {
            ANGLE_TRY(executableVk->createGraphicsPipeline(
                this, vk::GraphicsPipelineSubset::Complete, &pipelineCache, PipelineSource::Draw,
                *mGraphicsPipelineDesc, &descPtr, &mCurrentGraphicsPipeline));
        }
        else
        {
            const vk::GraphicsPipelineTransitionBits kShadersTransitionBitsMask =
                vk::GetGraphicsPipelineTransitionBitsMask(vk::GraphicsPipelineSubset::Shaders);
            const vk::GraphicsPipelineTransitionBits kVertexInputTransitionBitsMask =
                vk::GetGraphicsPipelineTransitionBitsMask(vk::GraphicsPipelineSubset::VertexInput);
            const vk::GraphicsPipelineTransitionBits kFragmentOutputTransitionBitsMask =
                vk::GetGraphicsPipelineTransitionBitsMask(
                    vk::GraphicsPipelineSubset::FragmentOutput);

            // Recreate the shaders subset if necessary
            const vk::GraphicsPipelineTransitionBits shadersTransitionBits =
                mGraphicsPipelineLibraryTransition & kShadersTransitionBitsMask;
            if (mCurrentGraphicsPipelineShaders == nullptr || shadersTransitionBits.any())
            {
                bool shouldRecreatePipeline = true;
                if (mCurrentGraphicsPipelineShaders != nullptr)
                {
                    ASSERT(mCurrentGraphicsPipelineShaders->valid());
                    shouldRecreatePipeline = !mCurrentGraphicsPipelineShaders->findTransition(
                        shadersTransitionBits, *mGraphicsPipelineDesc,
                        &mCurrentGraphicsPipelineShaders);
                }

                if (shouldRecreatePipeline)
                {
                    vk::PipelineHelper *oldGraphicsPipelineShaders =
                        mCurrentGraphicsPipelineShaders;

                    const vk::GraphicsPipelineDesc *shadersDescPtr = nullptr;
                    ANGLE_TRY(executableVk->getGraphicsPipeline(
                        this, vk::GraphicsPipelineSubset::Shaders, *mGraphicsPipelineDesc,
                        &shadersDescPtr, &mCurrentGraphicsPipelineShaders));
                    if (shadersDescPtr == nullptr)
                    {
                        ANGLE_TRY(executableVk->createGraphicsPipeline(
                            this, vk::GraphicsPipelineSubset::Shaders, &pipelineCache,
                            PipelineSource::Draw, *mGraphicsPipelineDesc, &shadersDescPtr,
                            &mCurrentGraphicsPipelineShaders));
                    }
                    if (oldGraphicsPipelineShaders)
                    {
                        oldGraphicsPipelineShaders->addTransition(
                            shadersTransitionBits, shadersDescPtr, mCurrentGraphicsPipelineShaders);
                    }
                }
            }

            // If blobs are reused between the pipeline libraries and the monolithic pipelines (so
            // |mergeProgramPipelineCachesToGlobalCache| would be enabled because merging the
            // pipelines would be beneficial), directly use the global cache for the vertex input
            // and fragment output pipelines.  This _may_ cause stalls as the worker thread that
            // creates pipelines is also holding the same lock.
            //
            // On the other hand, if there is not going to be any reuse of blobs, use a private
            // pipeline cache to avoid the aforementioned potential stall.
            vk::PipelineCacheAccess interfacePipelineCacheStorage;
            vk::PipelineCacheAccess *interfacePipelineCache = &pipelineCache;
            if (!getFeatures().mergeProgramPipelineCachesToGlobalCache.enabled)
            {
                ANGLE_TRY(ensureInterfacePipelineCache());
                interfacePipelineCacheStorage.init(&mInterfacePipelinesCache, nullptr);
                interfacePipelineCache = &interfacePipelineCacheStorage;
            }

            // Recreate the vertex input subset if necessary
            ANGLE_TRY(CreateGraphicsPipelineSubset(
                this, *mGraphicsPipelineDesc,
                mGraphicsPipelineLibraryTransition & kVertexInputTransitionBitsMask,
                GraphicsPipelineSubsetRenderPass::Unused,
                mShareGroupVk->getVertexInputGraphicsPipelineCache(), interfacePipelineCache,
                &mCurrentGraphicsPipelineVertexInput));

            // Recreate the fragment output subset if necessary
            ANGLE_TRY(CreateGraphicsPipelineSubset(
                this, *mGraphicsPipelineDesc,
                mGraphicsPipelineLibraryTransition & kFragmentOutputTransitionBitsMask,
                GraphicsPipelineSubsetRenderPass::Required,
                mShareGroupVk->getFragmentOutputGraphicsPipelineCache(), interfacePipelineCache,
                &mCurrentGraphicsPipelineFragmentOutput));

            // Link the three subsets into one pipeline.
            ANGLE_TRY(executableVk->linkGraphicsPipelineLibraries(
                this, &pipelineCache, *mGraphicsPipelineDesc, mCurrentGraphicsPipelineVertexInput,
                mCurrentGraphicsPipelineShaders, mCurrentGraphicsPipelineFragmentOutput, &descPtr,
                &mCurrentGraphicsPipeline));

            // Reset the transition bits for pipeline libraries, they are only made to be up-to-date
            // here.
            mGraphicsPipelineLibraryTransition.reset();
        }
    }

    // Maintain the transition cache
    if (oldGraphicsPipeline)
    {
        oldGraphicsPipeline->addTransition(mGraphicsPipelineTransition, descPtr,
                                           mCurrentGraphicsPipeline);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsPipelineDesc(DirtyBits::Iterator *dirtyBitsIterator,
                                                         DirtyBits dirtyBitMask)
{
    const VkPipeline previousPipeline = mCurrentGraphicsPipeline
                                            ? mCurrentGraphicsPipeline->getPipeline().getHandle()
                                            : VK_NULL_HANDLE;

    // Accumulate transition bits for the sake of pipeline libraries.  If a cache is hit in this
    // path, |mGraphicsPipelineTransition| is reset while the partial pipelines are left stale.  A
    // future partial library recreation would need to know the bits that have changed since.
    mGraphicsPipelineLibraryTransition |= mGraphicsPipelineTransition;

    // Recreate the pipeline if necessary.
    bool shouldRecreatePipeline =
        mCurrentGraphicsPipeline == nullptr || mGraphicsPipelineTransition.any();

    // If one can be found in the transition cache, recover it.
    if (mCurrentGraphicsPipeline != nullptr && mGraphicsPipelineTransition.any())
    {
        ASSERT(mCurrentGraphicsPipeline->valid());
        shouldRecreatePipeline = !mCurrentGraphicsPipeline->findTransition(
            mGraphicsPipelineTransition, *mGraphicsPipelineDesc, &mCurrentGraphicsPipeline);
    }

    // Otherwise either retrieve the pipeline from the cache, or create a new one.
    if (shouldRecreatePipeline)
    {
        ANGLE_TRY(createGraphicsPipeline());
    }

    mGraphicsPipelineTransition.reset();

    // Update the queue serial for the pipeline object.
    ASSERT(mCurrentGraphicsPipeline && mCurrentGraphicsPipeline->valid());

    const VkPipeline newPipeline = mCurrentGraphicsPipeline->getPipeline().getHandle();

    // If there's no change in pipeline, avoid rebinding it later.  If the rebind is due to a new
    // command buffer or UtilsVk, it will happen anyway with DIRTY_BIT_PIPELINE_BINDING.
    if (newPipeline == previousPipeline)
    {
        return angle::Result::Continue;
    }

    // VK_EXT_transform_feedback disallows binding pipelines while transform feedback is active.
    // If a new pipeline needs to be bound, the render pass should necessarily be broken (which
    // implicitly pauses transform feedback), as resuming requires a barrier on the transform
    // feedback counter buffer.
    if (mRenderPassCommands->started())
    {
        mCurrentGraphicsPipeline->retainInRenderPass(mRenderPassCommands);

        if (mRenderPassCommands->isTransformFeedbackActiveUnpaused())
        {
            ANGLE_TRY(
                flushDirtyGraphicsRenderPass(dirtyBitsIterator, dirtyBitMask,
                                             RenderPassClosureReason::PipelineBindWhileXfbActive));

            dirtyBitsIterator->setLaterBit(DIRTY_BIT_TRANSFORM_FEEDBACK_RESUME);
        }
    }

    // The pipeline needs to rebind because it's changed.
    dirtyBitsIterator->setLaterBit(DIRTY_BIT_PIPELINE_BINDING);

    return angle::Result::Continue;
}

angle::Result ContextVk::updateRenderPassDepthFeedbackLoopMode(
    UpdateDepthFeedbackLoopReason depthReason,
    UpdateDepthFeedbackLoopReason stencilReason)
{
    return switchOutReadOnlyDepthStencilMode(nullptr, {}, depthReason, stencilReason);
}

angle::Result ContextVk::switchOutReadOnlyDepthStencilMode(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask,
    UpdateDepthFeedbackLoopReason depthReason,
    UpdateDepthFeedbackLoopReason stencilReason)
{
    FramebufferVk *drawFramebufferVk = getDrawFramebuffer();
    if (!hasActiveRenderPass() || drawFramebufferVk->getDepthStencilRenderTarget() == nullptr)
    {
        return angle::Result::Continue;
    }

    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    const gl::DepthStencilState &dsState = mState.getDepthStencilState();
    vk::ResourceAccess depthAccess          = GetDepthAccess(dsState, executable, depthReason);
    vk::ResourceAccess stencilAccess        = GetStencilAccess(
        dsState, mState.getDrawFramebuffer()->getStencilBitCount(), executable, stencilReason);

    if ((HasResourceWriteAccess(depthAccess) &&
         mDepthStencilAttachmentFlags[vk::RenderPassUsage::DepthReadOnlyAttachment]) ||
        (HasResourceWriteAccess(stencilAccess) &&
         mDepthStencilAttachmentFlags[vk::RenderPassUsage::StencilReadOnlyAttachment]))
    {
        // We should not in the actual feedback mode
        ASSERT((mDepthStencilAttachmentFlags & vk::kDepthStencilFeedbackModeBits).none());

        // If we are switching out of read only mode and we are in feedback loop, we must end
        // render pass here. Otherwise, updating it to writeable layout will produce a writable
        // feedback loop that is illegal in vulkan and will trigger validation errors that depth
        // texture is using the writable layout.
        if (dirtyBitsIterator)
        {
            ANGLE_TRY(flushDirtyGraphicsRenderPass(
                dirtyBitsIterator, dirtyBitMask,
                RenderPassClosureReason::DepthStencilWriteAfterFeedbackLoop));
        }
        else
        {
            ANGLE_TRY(flushCommandsAndEndRenderPass(
                RenderPassClosureReason::DepthStencilWriteAfterFeedbackLoop));
        }
        // Clear read-only depth/stencil feedback mode.
        mDepthStencilAttachmentFlags &= ~vk::kDepthStencilReadOnlyBits;
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsReadOnlyDepthFeedbackLoopMode(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    return switchOutReadOnlyDepthStencilMode(dirtyBitsIterator, dirtyBitMask,
                                             UpdateDepthFeedbackLoopReason::Draw,
                                             UpdateDepthFeedbackLoopReason::Draw);
}

angle::Result ContextVk::handleDirtyAnySamplePassedQueryEnd(DirtyBits::Iterator *dirtyBitsIterator,
                                                            DirtyBits dirtyBitMask)
{
    if (mRenderPassCommands->started())
    {
        // When we switch from query enabled draw to query disabled draw, we do immediate flush to
        // ensure the query result will be ready early so that application thread calling
        // getQueryResult gets unblocked sooner.
        dirtyBitsIterator->setLaterBit(DIRTY_BIT_RENDER_PASS);

        // Don't let next render pass end up reactivate and reuse the current render pass, which
        // defeats the purpose of it.
        mAllowRenderPassToReactivate = false;
        mHasDeferredFlush            = true;
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsRenderPass(DirtyBits::Iterator *dirtyBitsIterator,
                                                       DirtyBits dirtyBitMask)
{
    FramebufferVk *drawFramebufferVk = getDrawFramebuffer();

    gl::Rectangle renderArea = drawFramebufferVk->getRenderArea(this);
    // Check to see if we can reactivate the current renderPass, if all arguments that we use to
    // start the render pass is the same. We don't need to check clear values since mid render pass
    // clear are handled differently.
    bool reactivateStartedRenderPass =
        hasStartedRenderPassWithQueueSerial(drawFramebufferVk->getLastRenderPassQueueSerial()) &&
        mAllowRenderPassToReactivate && renderArea == mRenderPassCommands->getRenderArea();
    if (reactivateStartedRenderPass)
    {
        INFO() << "Reactivate already started render pass on draw.";
        mRenderPassCommandBuffer = &mRenderPassCommands->getCommandBuffer();
        ASSERT(!drawFramebufferVk->hasDeferredClears());
        ASSERT(hasActiveRenderPass());

        vk::RenderPassDesc framebufferRenderPassDesc = drawFramebufferVk->getRenderPassDesc();
        if (getFeatures().preferDynamicRendering.enabled)
        {
            // With dynamic rendering, drawFramebufferVk's render pass desc does not track
            // framebuffer fetch mode.  For the purposes of the following ASSERT, assume they are
            // the same.
            framebufferRenderPassDesc.setFramebufferFetchMode(
                mRenderPassCommands->getRenderPassDesc().framebufferFetchMode());
        }
        ASSERT(framebufferRenderPassDesc == mRenderPassCommands->getRenderPassDesc());

        ANGLE_TRY(resumeRenderPassQueriesIfActive());

        return angle::Result::Continue;
    }

    // If the render pass needs to be recreated, close it using the special mid-dirty-bit-handling
    // function, so later dirty bits can be set.
    if (mRenderPassCommands->started())
    {
        ANGLE_TRY(flushDirtyGraphicsRenderPass(dirtyBitsIterator,
                                               dirtyBitMask & ~DirtyBits{DIRTY_BIT_RENDER_PASS},
                                               RenderPassClosureReason::AlreadySpecifiedElsewhere));
    }

    bool renderPassDescChanged = false;

    ANGLE_TRY(startRenderPass(renderArea, nullptr, &renderPassDescChanged));

    // The render pass desc can change when starting the render pass, for example due to
    // multisampled-render-to-texture needs based on loadOps.  In that case, recreate the graphics
    // pipeline.
    if (renderPassDescChanged)
    {
        ANGLE_TRY(handleDirtyGraphicsPipelineDesc(dirtyBitsIterator, dirtyBitMask));
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsColorAccess(DirtyBits::Iterator *dirtyBitsIterator,
                                                        DirtyBits dirtyBitMask)
{
    FramebufferVk *drawFramebufferVk             = getDrawFramebuffer();
    const gl::FramebufferState &framebufferState = drawFramebufferVk->getState();

    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    // Update color attachment accesses
    vk::PackedAttachmentIndex colorIndexVk(0);
    for (size_t colorIndexGL : framebufferState.getColorAttachmentsMask())
    {
        if (framebufferState.getEnabledDrawBuffers().test(colorIndexGL))
        {
            vk::ResourceAccess colorAccess = GetColorAccess(
                mState, framebufferState, drawFramebufferVk->getEmulatedAlphaAttachmentMask(),
                executable, colorIndexGL);
            mRenderPassCommands->onColorAccess(colorIndexVk, colorAccess);
        }
        ++colorIndexVk;
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDepthStencilAccess(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const FramebufferVk &drawFramebufferVk = *getDrawFramebuffer();
    if (drawFramebufferVk.getDepthStencilRenderTarget() == nullptr)
    {
        return angle::Result::Continue;
    }

    // Update depth/stencil attachment accesses
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    const gl::DepthStencilState &dsState = mState.getDepthStencilState();
    vk::ResourceAccess depthAccess =
        GetDepthAccess(dsState, executable, UpdateDepthFeedbackLoopReason::Draw);
    vk::ResourceAccess stencilAccess =
        GetStencilAccess(dsState, mState.getDrawFramebuffer()->getStencilBitCount(), executable,
                         UpdateDepthFeedbackLoopReason::Draw);
    mRenderPassCommands->onDepthAccess(depthAccess);
    mRenderPassCommands->onStencilAccess(stencilAccess);

    mRenderPassCommands->updateDepthReadOnlyMode(mDepthStencilAttachmentFlags);
    mRenderPassCommands->updateStencilReadOnlyMode(mDepthStencilAttachmentFlags);

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsPipelineBinding(DirtyBits::Iterator *dirtyBitsIterator,
                                                            DirtyBits dirtyBitMask)
{
    ASSERT(mCurrentGraphicsPipeline);

    const vk::Pipeline *pipeline = nullptr;
    ANGLE_TRY(mCurrentGraphicsPipeline->getPreferredPipeline(this, &pipeline));

    mRenderPassCommandBuffer->bindGraphicsPipeline(*pipeline);

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyComputePipelineDesc(DirtyBits::Iterator *dirtyBitsIterator)
{
    if (mCurrentComputePipeline == nullptr)
    {
        vk::PipelineCacheAccess pipelineCache;
        ANGLE_TRY(mRenderer->getPipelineCache(this, &pipelineCache));

        ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
        ASSERT(executableVk);

        executableVk->waitForComputePostLinkTasks(this);
        ANGLE_TRY(executableVk->getOrCreateComputePipeline(
            this, &pipelineCache, PipelineSource::Draw, pipelineRobustness(),
            pipelineProtectedAccess(), &mCurrentComputePipeline));
    }

    ASSERT(mComputeDirtyBits.test(DIRTY_BIT_PIPELINE_BINDING));

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyComputePipelineBinding(DirtyBits::Iterator *dirtyBitsIterator)
{
    ASSERT(mCurrentComputePipeline);

    mOutsideRenderPassCommands->getCommandBuffer().bindComputePipeline(
        mCurrentComputePipeline->getPipeline());
    mOutsideRenderPassCommands->retainResource(mCurrentComputePipeline);

    return angle::Result::Continue;
}

template <typename CommandBufferHelperT>
ANGLE_INLINE angle::Result ContextVk::handleDirtyTexturesImpl(
    CommandBufferHelperT *commandBufferHelper,
    PipelineType pipelineType)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);
    const gl::ActiveTextureMask &activeTextures = executable->getActiveSamplersMask();

    for (size_t textureUnit : activeTextures)
    {
        TextureVk *textureVk = mActiveTextures[textureUnit];

        // If it's a texture buffer, get the attached buffer.
        if (textureVk->getBuffer().get() != nullptr)
        {
            vk::BufferHelper *buffer = textureVk->getPossiblyEmulatedTextureBuffer(this);
            const gl::ShaderBitSet stages =
                executable->getSamplerShaderBitsForTextureUnitIndex(textureUnit);

            OnTextureBufferRead(this, buffer, stages, commandBufferHelper);

            textureVk->retainBufferViews(commandBufferHelper);
            continue;
        }

        // The image should be flushed and ready to use at this point. There may still be
        // lingering staged updates in its staging buffer for unused texture mip levels or
        // layers. Therefore we can't verify it has no staged updates right here.
        vk::ImageHelper &image = textureVk->getImage();

        const vk::ImageLayout imageLayout =
            GetImageReadLayout(textureVk, *executable, textureUnit, pipelineType);

        // Ensure the image is in the desired layout
        commandBufferHelper->imageRead(this, image.getAspectFlags(), imageLayout, &image);
    }

    if (executable->hasTextures())
    {
        ProgramExecutableVk *executableVk = vk::GetImpl(executable);
        ANGLE_TRY(executableVk->updateTexturesDescriptorSet(
            this, getCurrentFrameCount(), mActiveTextures, mState.getSamplers(), pipelineType,
            mShareGroupVk->getUpdateDescriptorSetsBuilder()));
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsTextures(DirtyBits::Iterator *dirtyBitsIterator,
                                                     DirtyBits dirtyBitMask)
{
    return handleDirtyTexturesImpl(mRenderPassCommands, PipelineType::Graphics);
}

angle::Result ContextVk::handleDirtyComputeTextures(DirtyBits::Iterator *dirtyBitsIterator)
{
    return handleDirtyTexturesImpl(mOutsideRenderPassCommands, PipelineType::Compute);
}

angle::Result ContextVk::handleDirtyGraphicsVertexBuffers(DirtyBits::Iterator *dirtyBitsIterator,
                                                          DirtyBits dirtyBitMask)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    VertexArrayVk *vertexArrayVk            = getVertexArray();
    uint32_t maxAttrib = mState.getProgramExecutable()->getMaxActiveAttribLocation();
    const gl::AttribArray<VkBuffer> &bufferHandles = vertexArrayVk->getCurrentArrayBufferHandles();
    const gl::AttribArray<VkDeviceSize> &bufferOffsets =
        vertexArrayVk->getCurrentArrayBufferOffsets();

    if (mRenderer->getFeatures().useVertexInputBindingStrideDynamicState.enabled ||
        getFeatures().supportsVertexInputDynamicState.enabled)
    {
        const gl::AttribArray<GLuint> &bufferStrides =
            vertexArrayVk->getCurrentArrayBufferStrides();
        const gl::AttribArray<angle::FormatID> &bufferFormats =
            vertexArrayVk->getCurrentArrayBufferFormats();
        gl::AttribArray<VkDeviceSize> strides = {};
        const gl::AttribArray<GLuint> &bufferDivisors =
            vertexArrayVk->getCurrentArrayBufferDivisors();
        const gl::AttribArray<GLuint> &bufferRelativeOffsets =
            vertexArrayVk->getCurrentArrayBufferRelativeOffsets();
        const gl::AttributesMask &bufferCompressed =
            vertexArrayVk->getCurrentArrayBufferCompressed();

        gl::AttribVector<VkVertexInputBindingDescription2EXT> bindingDescs;
        gl::AttribVector<VkVertexInputAttributeDescription2EXT> attributeDescs;

        // Set stride to 0 for mismatching formats between the program's declared attribute and that
        // which is specified in glVertexAttribPointer.  See comment in vk_cache_utils.cpp
        // (initializePipeline) for more details.
        const gl::AttributesMask &activeAttribLocations =
            executable->getNonBuiltinAttribLocationsMask();
        const gl::ComponentTypeMask &programAttribsTypeMask = executable->getAttributesTypeMask();

        for (size_t attribIndex : activeAttribLocations)
        {
            const angle::Format &intendedFormat =
                mRenderer->getFormat(bufferFormats[attribIndex]).getIntendedFormat();

            const gl::ComponentType attribType = GetVertexAttributeComponentType(
                intendedFormat.isPureInt(), intendedFormat.vertexAttribType);
            const gl::ComponentType programAttribType =
                gl::GetComponentTypeMask(programAttribsTypeMask, attribIndex);

            const bool mismatchingType =
                attribType != programAttribType && (programAttribType == gl::ComponentType::Float ||
                                                    attribType == gl::ComponentType::Float);
            strides[attribIndex] = mismatchingType ? 0 : bufferStrides[attribIndex];

            if (getFeatures().supportsVertexInputDynamicState.enabled)
            {
                VkVertexInputBindingDescription2EXT bindingDesc  = {};
                VkVertexInputAttributeDescription2EXT attribDesc = {};
                bindingDesc.sType   = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
                bindingDesc.binding = static_cast<uint32_t>(attribIndex);
                bindingDesc.stride  = static_cast<uint32_t>(strides[attribIndex]);
                bindingDesc.divisor =
                    bufferDivisors[attribIndex] > mRenderer->getMaxVertexAttribDivisor()
                        ? 1
                        : bufferDivisors[attribIndex];
                if (bindingDesc.divisor != 0)
                {
                    bindingDesc.inputRate =
                        static_cast<VkVertexInputRate>(VK_VERTEX_INPUT_RATE_INSTANCE);
                }
                else
                {
                    bindingDesc.inputRate =
                        static_cast<VkVertexInputRate>(VK_VERTEX_INPUT_RATE_VERTEX);
                    // Divisor value is ignored by the implementation when using
                    // VK_VERTEX_INPUT_RATE_VERTEX, but it is set to 1 to avoid a validation error
                    // due to a validation layer issue.
                    bindingDesc.divisor = 1;
                }

                attribDesc.sType   = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
                attribDesc.binding = static_cast<uint32_t>(attribIndex);
                attribDesc.format  = vk::GraphicsPipelineDesc::getPipelineVertexInputStateFormat(
                    this, bufferFormats[attribIndex], bufferCompressed[attribIndex],
                    programAttribType, static_cast<uint32_t>(attribIndex));
                attribDesc.location = static_cast<uint32_t>(attribIndex);
                attribDesc.offset   = bufferRelativeOffsets[attribIndex];

                bindingDescs.push_back(bindingDesc);
                attributeDescs.push_back(attribDesc);
            }
        }

        if (getFeatures().supportsVertexInputDynamicState.enabled)
        {
            mRenderPassCommandBuffer->setVertexInput(
                static_cast<uint32_t>(bindingDescs.size()), bindingDescs.data(),
                static_cast<uint32_t>(attributeDescs.size()), attributeDescs.data());
            if (bindingDescs.size() != 0)
            {

                mRenderPassCommandBuffer->bindVertexBuffers(0, maxAttrib, bufferHandles.data(),
                                                            bufferOffsets.data());
            }
        }
        else
        {
            // TODO: Use the sizes parameters here to fix the robustness issue worked around in
            // crbug.com/1310038
            mRenderPassCommandBuffer->bindVertexBuffers2(
                0, maxAttrib, bufferHandles.data(), bufferOffsets.data(), nullptr, strides.data());
        }
    }
    else
    {
        mRenderPassCommandBuffer->bindVertexBuffers(0, maxAttrib, bufferHandles.data(),
                                                    bufferOffsets.data());
    }

    const gl::AttribArray<vk::BufferHelper *> &arrayBufferResources =
        vertexArrayVk->getCurrentArrayBuffers();

    // Mark all active vertex buffers as accessed.
    for (uint32_t attribIndex = 0; attribIndex < maxAttrib; ++attribIndex)
    {
        vk::BufferHelper *arrayBuffer = arrayBufferResources[attribIndex];
        if (arrayBuffer)
        {
            mRenderPassCommands->bufferRead(this, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                                            vk::PipelineStage::VertexInput, arrayBuffer);
        }
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsIndexBuffer(DirtyBits::Iterator *dirtyBitsIterator,
                                                        DirtyBits dirtyBitMask)
{
    vk::BufferHelper *elementArrayBuffer = mCurrentIndexBuffer;
    ASSERT(elementArrayBuffer != nullptr);

    VkDeviceSize bufferOffset;
    const vk::Buffer &buffer = elementArrayBuffer->getBufferForVertexArray(
        this, elementArrayBuffer->getSize(), &bufferOffset);

    mRenderPassCommandBuffer->bindIndexBuffer(buffer, bufferOffset + mCurrentIndexBufferOffset,
                                              getVkIndexType(mCurrentDrawElementsType));

    mRenderPassCommands->bufferRead(this, VK_ACCESS_INDEX_READ_BIT, vk::PipelineStage::VertexInput,
                                    elementArrayBuffer);

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsFramebufferFetchBarrier(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memoryBarrier.dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

    mRenderPassCommandBuffer->pipelineBarrier(
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        GetLocalDependencyFlags(this), 1, &memoryBarrier, 0, nullptr, 0, nullptr);

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsBlendBarrier(DirtyBits::Iterator *dirtyBitsIterator,
                                                         DirtyBits dirtyBitMask)
{
    if (getFeatures().supportsBlendOperationAdvancedCoherent.enabled)
    {
        return angle::Result::Continue;
    }

    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    memoryBarrier.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;

    mRenderPassCommandBuffer->pipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                              GetLocalDependencyFlags(this), 1, &memoryBarrier, 0,
                                              nullptr, 0, nullptr);

    return angle::Result::Continue;
}

template <typename CommandBufferHelperT>
angle::Result ContextVk::handleDirtyShaderResourcesImpl(CommandBufferHelperT *commandBufferHelper,
                                                        PipelineType pipelineType,
                                                        DirtyBits::Iterator *dirtyBitsIterator)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    // DIRTY_BIT_UNIFORM_BUFFERS is set when uniform buffer bindings change.
    // DIRTY_BIT_SHADER_RESOURCES gets set when the program executable has changed. In that case,
    // this function will update entire the shader resource descriptorSet.  This means there is no
    // need to process uniform buffer bindings again.
    dirtyBitsIterator->resetLaterBit(DIRTY_BIT_UNIFORM_BUFFERS);

    // This function processes uniform buffers, so it doesn't matter which are dirty.  The following
    // makes sure the dirty bits are reset.
    mState.getAndResetDirtyUniformBlocks();

    const bool hasImages               = executable->hasImages();
    const bool hasStorageBuffers       = executable->hasStorageBuffers();
    const bool hasAtomicCounterBuffers = executable->hasAtomicCounterBuffers();
    const bool hasUniformBuffers       = executable->hasUniformBuffers();
    const bool hasFramebufferFetch     = executable->usesColorFramebufferFetch() ||
                                     executable->usesDepthFramebufferFetch() ||
                                     executable->usesStencilFramebufferFetch();

    if (!hasUniformBuffers && !hasStorageBuffers && !hasAtomicCounterBuffers && !hasImages &&
        !hasFramebufferFetch)
    {
        return angle::Result::Continue;
    }

    const VkPhysicalDeviceLimits &limits = mRenderer->getPhysicalDeviceProperties().limits;
    ProgramExecutableVk *executableVk    = vk::GetImpl(executable);
    const ShaderInterfaceVariableInfoMap &variableInfoMap = executableVk->getVariableInfoMap();

    mShaderBufferWriteDescriptorDescs = executableVk->getShaderResourceWriteDescriptorDescs();
    // Update writeDescriptorDescs with inputAttachments
    mShaderBufferWriteDescriptorDescs.updateInputAttachments(
        *executable, variableInfoMap, vk::GetImpl(mState.getDrawFramebuffer()));

    mShaderBuffersDescriptorDesc.resize(
        mShaderBufferWriteDescriptorDescs.getTotalDescriptorCount());
    if (hasUniformBuffers)
    {
        mShaderBuffersDescriptorDesc.updateShaderBuffers(
            this, commandBufferHelper, *executable, variableInfoMap,
            mState.getOffsetBindingPointerUniformBuffers(), executable->getUniformBlocks(),
            executableVk->getUniformBufferDescriptorType(), limits.maxUniformBufferRange,
            mEmptyBuffer, mShaderBufferWriteDescriptorDescs, mDeferredMemoryBarriers);
    }
    if (hasStorageBuffers)
    {
        mShaderBuffersDescriptorDesc.updateShaderBuffers(
            this, commandBufferHelper, *executable, variableInfoMap,
            mState.getOffsetBindingPointerShaderStorageBuffers(),
            executable->getShaderStorageBlocks(), executableVk->getStorageBufferDescriptorType(),
            limits.maxStorageBufferRange, mEmptyBuffer, mShaderBufferWriteDescriptorDescs,
            mDeferredMemoryBarriers);
    }
    if (hasAtomicCounterBuffers)
    {
        mShaderBuffersDescriptorDesc.updateAtomicCounters(
            this, commandBufferHelper, *executable, variableInfoMap,
            mState.getOffsetBindingPointerAtomicCounterBuffers(),
            executable->getAtomicCounterBuffers(), limits.minStorageBufferOffsetAlignment,
            mEmptyBuffer, mShaderBufferWriteDescriptorDescs);
    }
    if (hasImages)
    {
        ANGLE_TRY(updateActiveImages(commandBufferHelper));
        ANGLE_TRY(mShaderBuffersDescriptorDesc.updateImages(this, *executable, variableInfoMap,
                                                            mActiveImages, mState.getImageUnits(),
                                                            mShaderBufferWriteDescriptorDescs));
    }
    if (hasFramebufferFetch)
    {
        ANGLE_TRY(mShaderBuffersDescriptorDesc.updateInputAttachments(
            this, *executable, variableInfoMap, vk::GetImpl(mState.getDrawFramebuffer()),
            mShaderBufferWriteDescriptorDescs));
    }

    mDeferredMemoryBarriers = 0;

    vk::SharedDescriptorSetCacheKey newSharedCacheKey;
    ANGLE_TRY(executableVk->updateShaderResourcesDescriptorSet(
        this, getCurrentFrameCount(), mShareGroupVk->getUpdateDescriptorSetsBuilder(),
        mShaderBufferWriteDescriptorDescs, mShaderBuffersDescriptorDesc, &newSharedCacheKey));

    if (newSharedCacheKey)
    {
        // A new cache entry has been created. We record this cache key in the images and buffers so
        // that the descriptorSet cache can be destroyed when buffer/image is destroyed.
        updateShaderResourcesWithSharedCacheKey(newSharedCacheKey);
    }

    // Record usage of storage buffers and images in the command buffer to aid handling of
    // glMemoryBarrier.
    if (hasImages || hasStorageBuffers || hasAtomicCounterBuffers)
    {
        commandBufferHelper->setHasShaderStorageOutput();
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsShaderResources(DirtyBits::Iterator *dirtyBitsIterator,
                                                            DirtyBits dirtyBitMask)
{
    return handleDirtyShaderResourcesImpl(mRenderPassCommands, PipelineType::Graphics,
                                          dirtyBitsIterator);
}

angle::Result ContextVk::handleDirtyComputeShaderResources(DirtyBits::Iterator *dirtyBitsIterator)
{
    return handleDirtyShaderResourcesImpl(mOutsideRenderPassCommands, PipelineType::Compute,
                                          dirtyBitsIterator);
}

template <typename CommandBufferT>
angle::Result ContextVk::handleDirtyUniformBuffersImpl(CommandBufferT *commandBufferHelper)
{
    gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);
    ASSERT(executable->hasUniformBuffers());

    const VkPhysicalDeviceLimits &limits = mRenderer->getPhysicalDeviceProperties().limits;
    ProgramExecutableVk *executableVk    = vk::GetImpl(executable);
    const ShaderInterfaceVariableInfoMap &variableInfoMap = executableVk->getVariableInfoMap();

    gl::ProgramUniformBlockMask dirtyBits = mState.getAndResetDirtyUniformBlocks();
    for (size_t blockIndex : dirtyBits)
    {
        const GLuint binding = executable->getUniformBlockBinding(blockIndex);
        mShaderBuffersDescriptorDesc.updateOneShaderBuffer(
            this, commandBufferHelper, variableInfoMap,
            mState.getOffsetBindingPointerUniformBuffers(),
            executable->getUniformBlocks()[blockIndex], binding,
            executableVk->getUniformBufferDescriptorType(), limits.maxUniformBufferRange,
            mEmptyBuffer, mShaderBufferWriteDescriptorDescs, mDeferredMemoryBarriers);
    }

    vk::SharedDescriptorSetCacheKey newSharedCacheKey;
    ANGLE_TRY(executableVk->updateShaderResourcesDescriptorSet(
        this, getCurrentFrameCount(), mShareGroupVk->getUpdateDescriptorSetsBuilder(),
        mShaderBufferWriteDescriptorDescs, mShaderBuffersDescriptorDesc, &newSharedCacheKey));

    if (newSharedCacheKey)
    {
        // A new cache entry has been created. We record this cache key in the images and
        // buffers so that the descriptorSet cache can be destroyed when buffer/image is
        // destroyed.
        updateShaderResourcesWithSharedCacheKey(newSharedCacheKey);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsUniformBuffers(DirtyBits::Iterator *dirtyBitsIterator,
                                                           DirtyBits dirtyBitMask)
{
    return handleDirtyUniformBuffersImpl(mRenderPassCommands);
}

angle::Result ContextVk::handleDirtyComputeUniformBuffers(DirtyBits::Iterator *dirtyBitsIterator)
{
    return handleDirtyUniformBuffersImpl(mOutsideRenderPassCommands);
}

angle::Result ContextVk::handleDirtyGraphicsTransformFeedbackBuffersEmulation(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    if (!executable->hasTransformFeedbackOutput())
    {
        return angle::Result::Continue;
    }

    TransformFeedbackVk *transformFeedbackVk = vk::GetImpl(mState.getCurrentTransformFeedback());

    if (mState.isTransformFeedbackActiveUnpaused())
    {
        size_t bufferCount = executable->getTransformFeedbackBufferCount();
        const gl::TransformFeedbackBuffersArray<vk::BufferHelper *> &bufferHelpers =
            transformFeedbackVk->getBufferHelpers();

        for (size_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
        {
            vk::BufferHelper *bufferHelper = bufferHelpers[bufferIndex];
            ASSERT(bufferHelper);
            mRenderPassCommands->bufferWrite(this, VK_ACCESS_SHADER_WRITE_BIT,
                                             vk::PipelineStage::VertexShader, bufferHelper);
        }

        mCurrentTransformFeedbackQueueSerial = mRenderPassCommands->getQueueSerial();
    }

    ProgramExecutableVk *executableVk      = vk::GetImpl(executable);
    vk::BufferHelper *currentUniformBuffer = mDefaultUniformStorage.getCurrentBuffer();

    const vk::WriteDescriptorDescs &writeDescriptorDescs =
        executableVk->getDefaultUniformWriteDescriptorDescs(transformFeedbackVk);

    vk::DescriptorSetDescBuilder uniformsAndXfbDesc(writeDescriptorDescs.getTotalDescriptorCount());
    uniformsAndXfbDesc.updateUniformsAndXfb(
        this, *executable, writeDescriptorDescs, currentUniformBuffer, mEmptyBuffer,
        mState.isTransformFeedbackActiveUnpaused(), transformFeedbackVk);

    vk::SharedDescriptorSetCacheKey newSharedCacheKey;
    ANGLE_TRY(executableVk->updateUniformsAndXfbDescriptorSet(
        this, getCurrentFrameCount(), mShareGroupVk->getUpdateDescriptorSetsBuilder(),
        writeDescriptorDescs, currentUniformBuffer, &uniformsAndXfbDesc, &newSharedCacheKey));

    if (newSharedCacheKey)
    {
        if (currentUniformBuffer)
        {
            currentUniformBuffer->getBufferBlock()->onNewDescriptorSet(newSharedCacheKey);
        }
        transformFeedbackVk->onNewDescriptorSet(*executable, newSharedCacheKey);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsTransformFeedbackBuffersExtension(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    if (!executable->hasTransformFeedbackOutput() || !mState.isTransformFeedbackActive())
    {
        return angle::Result::Continue;
    }

    TransformFeedbackVk *transformFeedbackVk = vk::GetImpl(mState.getCurrentTransformFeedback());
    size_t bufferCount                       = executable->getTransformFeedbackBufferCount();

    const gl::TransformFeedbackBuffersArray<vk::BufferHelper *> &buffers =
        transformFeedbackVk->getBufferHelpers();
    gl::TransformFeedbackBuffersArray<vk::BufferHelper> &counterBuffers =
        transformFeedbackVk->getCounterBufferHelpers();

    // Issue necessary barriers for the transform feedback buffers.
    for (size_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
    {
        vk::BufferHelper *bufferHelper = buffers[bufferIndex];
        ASSERT(bufferHelper);
        mRenderPassCommands->bufferWrite(this, VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
                                         vk::PipelineStage::TransformFeedback, bufferHelper);
    }

    // Issue necessary barriers for the transform feedback counter buffer.  Note that the barrier is
    // issued only on the first buffer (which uses a global memory barrier), as all the counter
    // buffers of the transform feedback object are used together.  The rest of the buffers are
    // simply retained so they don't get deleted too early.
    ASSERT(counterBuffers[0].valid());
    mRenderPassCommands->bufferWrite(this,
                                     VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT |
                                         VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
                                     vk::PipelineStage::TransformFeedback, &counterBuffers[0]);
    for (size_t bufferIndex = 1; bufferIndex < bufferCount; ++bufferIndex)
    {
        mRenderPassCommands->retainResourceForWrite(&counterBuffers[bufferIndex]);
    }

    const gl::TransformFeedbackBuffersArray<VkBuffer> &bufferHandles =
        transformFeedbackVk->getBufferHandles();
    const gl::TransformFeedbackBuffersArray<VkDeviceSize> &bufferOffsets =
        transformFeedbackVk->getBufferOffsets();
    const gl::TransformFeedbackBuffersArray<VkDeviceSize> &bufferSizes =
        transformFeedbackVk->getBufferSizes();

    mRenderPassCommandBuffer->bindTransformFeedbackBuffers(
        0, static_cast<uint32_t>(bufferCount), bufferHandles.data(), bufferOffsets.data(),
        bufferSizes.data());

    if (!mState.isTransformFeedbackActiveUnpaused())
    {
        return angle::Result::Continue;
    }

    // We should have same number of counter buffers as xfb buffers have
    const gl::TransformFeedbackBuffersArray<VkBuffer> &counterBufferHandles =
        transformFeedbackVk->getCounterBufferHandles();
    const gl::TransformFeedbackBuffersArray<VkDeviceSize> &counterBufferOffsets =
        transformFeedbackVk->getCounterBufferOffsets();

    bool rebindBuffers = transformFeedbackVk->getAndResetBufferRebindState();

    mRenderPassCommands->beginTransformFeedback(bufferCount, counterBufferHandles.data(),
                                                counterBufferOffsets.data(), rebindBuffers);

    mCurrentTransformFeedbackQueueSerial = mRenderPassCommands->getQueueSerial();

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsTransformFeedbackResume(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    if (mRenderPassCommands->isTransformFeedbackStarted())
    {
        mRenderPassCommands->resumeTransformFeedback();
    }

    ANGLE_TRY(resumeXfbRenderPassQueriesIfActive());

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDescriptorSets(DirtyBits::Iterator *dirtyBitsIterator,
                                                           DirtyBits dirtyBitMask)
{
    return handleDirtyDescriptorSetsImpl(mRenderPassCommands, PipelineType::Graphics);
}

angle::Result ContextVk::handleDirtyGraphicsUniforms(DirtyBits::Iterator *dirtyBitsIterator,
                                                     DirtyBits dirtyBitMask)
{
    return handleDirtyUniformsImpl(dirtyBitsIterator);
}

angle::Result ContextVk::handleDirtyComputeUniforms(DirtyBits::Iterator *dirtyBitsIterator)
{
    return handleDirtyUniformsImpl(dirtyBitsIterator);
}

angle::Result ContextVk::handleDirtyUniformsImpl(DirtyBits::Iterator *dirtyBitsIterator)
{
    dirtyBitsIterator->setLaterBit(DIRTY_BIT_DESCRIPTOR_SETS);

    ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
    TransformFeedbackVk *transformFeedbackVk =
        vk::SafeGetImpl(mState.getCurrentTransformFeedback());
    ANGLE_TRY(executableVk->updateUniforms(
        this, getCurrentFrameCount(), mShareGroupVk->getUpdateDescriptorSetsBuilder(),
        &mEmptyBuffer, &mDefaultUniformStorage, mState.isTransformFeedbackActiveUnpaused(),
        transformFeedbackVk));

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicViewport(DirtyBits::Iterator *dirtyBitsIterator,
                                                            DirtyBits dirtyBitMask)
{
    mRenderPassCommandBuffer->setViewport(0, 1, &mViewport);
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicScissor(DirtyBits::Iterator *dirtyBitsIterator,
                                                           DirtyBits dirtyBitMask)
{
    handleDirtyGraphicsDynamicScissorImpl(mState.isQueryActive(gl::QueryType::PrimitivesGenerated));
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicLineWidth(DirtyBits::Iterator *dirtyBitsIterator,
                                                             DirtyBits dirtyBitMask)
{
    // Clamp line width to min/max allowed values. It's not invalid GL to
    // provide out-of-range line widths, but it _is_ invalid Vulkan.
    const float lineWidth = gl::clamp(mState.getLineWidth(), mState.getCaps().minAliasedLineWidth,
                                      mState.getCaps().maxAliasedLineWidth);
    mRenderPassCommandBuffer->setLineWidth(lineWidth);
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicDepthBias(DirtyBits::Iterator *dirtyBitsIterator,
                                                             DirtyBits dirtyBitMask)
{
    const gl::RasterizerState &rasterState = mState.getRasterizerState();

    float depthBiasConstantFactor = rasterState.polygonOffsetUnits;
    if (getFeatures().doubleDepthBiasConstantFactor.enabled)
    {
        depthBiasConstantFactor *= 2.0f;
    }

    // Note: depth bias clamp is only exposed in EXT_polygon_offset_clamp.
    mRenderPassCommandBuffer->setDepthBias(depthBiasConstantFactor, rasterState.polygonOffsetClamp,
                                           rasterState.polygonOffsetFactor);
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicBlendConstants(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::ColorF &color = mState.getBlendColor();
    mRenderPassCommandBuffer->setBlendConstants(color.data());
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicStencilCompareMask(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::DepthStencilState &depthStencilState = mState.getDepthStencilState();
    mRenderPassCommandBuffer->setStencilCompareMask(depthStencilState.stencilMask,
                                                    depthStencilState.stencilBackMask);
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicStencilWriteMask(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::DepthStencilState &depthStencilState = mState.getDepthStencilState();
    const gl::Framebuffer *drawFramebuffer         = mState.getDrawFramebuffer();
    uint32_t frontWritemask                        = 0;
    uint32_t backWritemask                         = 0;
    // Don't write to stencil buffers that should not exist
    if (drawFramebuffer->hasStencil())
    {
        frontWritemask = depthStencilState.stencilWritemask;
        backWritemask  = depthStencilState.stencilBackWritemask;
    }

    mRenderPassCommandBuffer->setStencilWriteMask(frontWritemask, backWritemask);
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicStencilReference(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    mRenderPassCommandBuffer->setStencilReference(mState.getStencilRef(),
                                                  mState.getStencilBackRef());
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicCullMode(DirtyBits::Iterator *dirtyBitsIterator,
                                                            DirtyBits dirtyBitMask)
{
    const gl::RasterizerState &rasterState = mState.getRasterizerState();
    mRenderPassCommandBuffer->setCullMode(gl_vk::GetCullMode(rasterState));
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicFrontFace(DirtyBits::Iterator *dirtyBitsIterator,
                                                             DirtyBits dirtyBitMask)
{
    const gl::RasterizerState &rasterState = mState.getRasterizerState();
    mRenderPassCommandBuffer->setFrontFace(
        gl_vk::GetFrontFace(rasterState.frontFace, isYFlipEnabledForDrawFBO()));
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicDepthTestEnable(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::DepthStencilState &depthStencilState = mState.getDepthStencilState();
    gl::Framebuffer *drawFramebuffer              = mState.getDrawFramebuffer();

    // Only enable the depth test if the draw framebuffer has a depth buffer.
    mRenderPassCommandBuffer->setDepthTestEnable(depthStencilState.depthTest &&
                                                 drawFramebuffer->hasDepth());
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicDepthWriteEnable(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::DepthStencilState &depthStencilState = mState.getDepthStencilState();
    gl::Framebuffer *drawFramebuffer              = mState.getDrawFramebuffer();

    // Only enable the depth write if the draw framebuffer has a depth buffer.
    const bool depthWriteEnabled =
        drawFramebuffer->hasDepth() && depthStencilState.depthTest && depthStencilState.depthMask;
    mRenderPassCommandBuffer->setDepthWriteEnable(depthWriteEnabled);
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicDepthCompareOp(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::DepthStencilState &depthStencilState = mState.getDepthStencilState();
    mRenderPassCommandBuffer->setDepthCompareOp(gl_vk::GetCompareOp(depthStencilState.depthFunc));
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicStencilTestEnable(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const gl::DepthStencilState &depthStencilState = mState.getDepthStencilState();
    gl::Framebuffer *drawFramebuffer              = mState.getDrawFramebuffer();

    // Only enable the stencil test if the draw framebuffer has a stencil buffer.
    mRenderPassCommandBuffer->setStencilTestEnable(depthStencilState.stencilTest &&
                                                   drawFramebuffer->hasStencil());
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicStencilOp(DirtyBits::Iterator *dirtyBitsIterator,
                                                             DirtyBits dirtyBitMask)
{
    const gl::DepthStencilState &depthStencilState = mState.getDepthStencilState();
    mRenderPassCommandBuffer->setStencilOp(
        VK_STENCIL_FACE_FRONT_BIT, gl_vk::GetStencilOp(depthStencilState.stencilFail),
        gl_vk::GetStencilOp(depthStencilState.stencilPassDepthPass),
        gl_vk::GetStencilOp(depthStencilState.stencilPassDepthFail),
        gl_vk::GetCompareOp(depthStencilState.stencilFunc));
    mRenderPassCommandBuffer->setStencilOp(
        VK_STENCIL_FACE_BACK_BIT, gl_vk::GetStencilOp(depthStencilState.stencilBackFail),
        gl_vk::GetStencilOp(depthStencilState.stencilBackPassDepthPass),
        gl_vk::GetStencilOp(depthStencilState.stencilBackPassDepthFail),
        gl_vk::GetCompareOp(depthStencilState.stencilBackFunc));
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicRasterizerDiscardEnable(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    const bool isEmulatingRasterizerDiscard =
        isEmulatingRasterizerDiscardDuringPrimitivesGeneratedQuery(
            mState.isQueryActive(gl::QueryType::PrimitivesGenerated));
    const bool isRasterizerDiscardEnabled = mState.isRasterizerDiscardEnabled();

    mRenderPassCommandBuffer->setRasterizerDiscardEnable(isRasterizerDiscardEnabled &&
                                                         !isEmulatingRasterizerDiscard);
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicDepthBiasEnable(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    mRenderPassCommandBuffer->setDepthBiasEnable(mState.isPolygonOffsetEnabled());
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicLogicOp(DirtyBits::Iterator *dirtyBitsIterator,
                                                           DirtyBits dirtyBitMask)
{
    mRenderPassCommandBuffer->setLogicOp(gl_vk::GetLogicOp(gl::ToGLenum(mState.getLogicOp())));
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicPrimitiveRestartEnable(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    mRenderPassCommandBuffer->setPrimitiveRestartEnable(mState.isPrimitiveRestartEnabled());
    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyGraphicsDynamicFragmentShadingRate(
    DirtyBits::Iterator *dirtyBitsIterator,
    DirtyBits dirtyBitMask)
{
    FramebufferVk *drawFramebufferVk = vk::GetImpl(mState.getDrawFramebuffer());
    const bool isFoveationEnabled    = drawFramebufferVk->isFoveationEnabled();

    gl::ShadingRate shadingRate =
        isFoveationEnabled ? gl::ShadingRate::_1x1 : getState().getShadingRate();
    if (shadingRate == gl::ShadingRate::Undefined)
    {
        // Shading rate has not been set. Since this is dynamic state, set it to 1x1
        shadingRate = gl::ShadingRate::_1x1;
    }

    const bool shadingRateSupported = mRenderer->isShadingRateSupported(shadingRate);
    VkExtent2D fragmentSize         = {};

    switch (shadingRate)
    {
        case gl::ShadingRate::_1x1:
            ASSERT(shadingRateSupported);
            fragmentSize.width  = 1;
            fragmentSize.height = 1;
            break;
        case gl::ShadingRate::_1x2:
            ASSERT(shadingRateSupported);
            fragmentSize.width  = 1;
            fragmentSize.height = 2;
            break;
        case gl::ShadingRate::_2x1:
            ASSERT(shadingRateSupported);
            fragmentSize.width  = 2;
            fragmentSize.height = 1;
            break;
        case gl::ShadingRate::_2x2:
            ASSERT(shadingRateSupported);
            fragmentSize.width  = 2;
            fragmentSize.height = 2;
            break;
        case gl::ShadingRate::_4x2:
            if (shadingRateSupported)
            {
                fragmentSize.width  = 4;
                fragmentSize.height = 2;
            }
            else
            {
                // Fallback to shading rate that preserves aspect ratio
                fragmentSize.width  = 2;
                fragmentSize.height = 1;
            }
            break;
        case gl::ShadingRate::_4x4:
            if (shadingRateSupported)
            {
                fragmentSize.width  = 4;
                fragmentSize.height = 4;
            }
            else
            {
                // Fallback to shading rate that preserves aspect ratio
                fragmentSize.width  = 2;
                fragmentSize.height = 2;
            }
            break;
        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }

    VkFragmentShadingRateCombinerOpKHR shadingRateCombinerOp[2] = {
        VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
        VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR};

    // If foveated rendering is enabled update combiner op
    if (isFoveationEnabled)
    {
        shadingRateCombinerOp[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR;
    }

    ASSERT(hasActiveRenderPass());
    mRenderPassCommandBuffer->setFragmentShadingRate(&fragmentSize, shadingRateCombinerOp);

    return angle::Result::Continue;
}

void ContextVk::handleDirtyGraphicsDynamicScissorImpl(bool isPrimitivesGeneratedQueryActive)
{
    // If primitives generated query and rasterizer discard are both active, but the Vulkan
    // implementation of the query does not support rasterizer discard, use an empty scissor to
    // emulate it.
    if (isEmulatingRasterizerDiscardDuringPrimitivesGeneratedQuery(
            isPrimitivesGeneratedQueryActive))
    {
        VkRect2D emptyScissor = {};
        mRenderPassCommandBuffer->setScissor(0, 1, &emptyScissor);
    }
    else
    {
        mRenderPassCommandBuffer->setScissor(0, 1, &mScissor);
    }
}

angle::Result ContextVk::handleDirtyComputeDescriptorSets(DirtyBits::Iterator *dirtyBitsIterator)
{
    return handleDirtyDescriptorSetsImpl(mOutsideRenderPassCommands, PipelineType::Compute);
}

template <typename CommandBufferHelperT>
angle::Result ContextVk::handleDirtyDescriptorSetsImpl(CommandBufferHelperT *commandBufferHelper,
                                                       PipelineType pipelineType)
{
    // When using Vulkan secondary command buffers, the descriptor sets need to be updated before
    // they are bound.
    if (!CommandBufferHelperT::ExecutesInline())
    {
        flushDescriptorSetUpdates();
    }

    ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
    return executableVk->bindDescriptorSets(this, getCurrentFrameCount(), commandBufferHelper,
                                            &commandBufferHelper->getCommandBuffer(), pipelineType);
}

void ContextVk::syncObjectPerfCounters(const angle::VulkanPerfCounters &commandQueuePerfCounters)
{
    if (!mState.isPerfMonitorActive())
    {
        return;
    }

    mPerfCounters.descriptorSetCacheTotalSize                = 0;
    mPerfCounters.descriptorSetCacheKeySizeBytes             = 0;
    mPerfCounters.uniformsAndXfbDescriptorSetCacheHits       = 0;
    mPerfCounters.uniformsAndXfbDescriptorSetCacheMisses     = 0;
    mPerfCounters.uniformsAndXfbDescriptorSetCacheTotalSize  = 0;
    mPerfCounters.textureDescriptorSetCacheHits              = 0;
    mPerfCounters.textureDescriptorSetCacheMisses            = 0;
    mPerfCounters.textureDescriptorSetCacheTotalSize         = 0;
    mPerfCounters.shaderResourcesDescriptorSetCacheHits      = 0;
    mPerfCounters.shaderResourcesDescriptorSetCacheMisses    = 0;
    mPerfCounters.shaderResourcesDescriptorSetCacheTotalSize = 0;
    mPerfCounters.dynamicBufferAllocations                   = 0;

    // Share group descriptor set allocations and caching stats.
    memset(mVulkanCacheStats.data(), 0, sizeof(CacheStats) * mVulkanCacheStats.size());
    if (getFeatures().descriptorSetCache.enabled)
    {
        mShareGroupVk->getMetaDescriptorPools()[DescriptorSetIndex::UniformsAndXfb]
            .accumulateDescriptorCacheStats(VulkanCacheType::UniformsAndXfbDescriptors, this);
        mShareGroupVk->getMetaDescriptorPools()[DescriptorSetIndex::Texture]
            .accumulateDescriptorCacheStats(VulkanCacheType::TextureDescriptors, this);
        mShareGroupVk->getMetaDescriptorPools()[DescriptorSetIndex::ShaderResource]
            .accumulateDescriptorCacheStats(VulkanCacheType::ShaderResourcesDescriptors, this);

        const CacheStats &uniCacheStats =
            mVulkanCacheStats[VulkanCacheType::UniformsAndXfbDescriptors];
        mPerfCounters.uniformsAndXfbDescriptorSetCacheHits      = uniCacheStats.getHitCount();
        mPerfCounters.uniformsAndXfbDescriptorSetCacheMisses    = uniCacheStats.getMissCount();
        mPerfCounters.uniformsAndXfbDescriptorSetCacheTotalSize = uniCacheStats.getSize();

        const CacheStats &texCacheStats = mVulkanCacheStats[VulkanCacheType::TextureDescriptors];
        mPerfCounters.textureDescriptorSetCacheHits      = texCacheStats.getHitCount();
        mPerfCounters.textureDescriptorSetCacheMisses    = texCacheStats.getMissCount();
        mPerfCounters.textureDescriptorSetCacheTotalSize = texCacheStats.getSize();

        const CacheStats &resCacheStats =
            mVulkanCacheStats[VulkanCacheType::ShaderResourcesDescriptors];
        mPerfCounters.shaderResourcesDescriptorSetCacheHits      = resCacheStats.getHitCount();
        mPerfCounters.shaderResourcesDescriptorSetCacheMisses    = resCacheStats.getMissCount();
        mPerfCounters.shaderResourcesDescriptorSetCacheTotalSize = resCacheStats.getSize();

        mPerfCounters.descriptorSetCacheTotalSize =
            uniCacheStats.getSize() + texCacheStats.getSize() + resCacheStats.getSize() +
            mVulkanCacheStats[VulkanCacheType::DriverUniformsDescriptors].getSize();

        mPerfCounters.descriptorSetCacheKeySizeBytes = 0;

        for (DescriptorSetIndex descriptorSetIndex : angle::AllEnums<DescriptorSetIndex>())
        {
            vk::MetaDescriptorPool &descriptorPool =
                mShareGroupVk->getMetaDescriptorPools()[descriptorSetIndex];
            mPerfCounters.descriptorSetCacheKeySizeBytes +=
                descriptorPool.getTotalCacheKeySizeBytes();
        }
    }

    // Update perf counters from the renderer as well
    mPerfCounters.commandQueueSubmitCallsTotal =
        commandQueuePerfCounters.commandQueueSubmitCallsTotal;
    mPerfCounters.commandQueueSubmitCallsPerFrame =
        commandQueuePerfCounters.commandQueueSubmitCallsPerFrame;
    mPerfCounters.vkQueueSubmitCallsTotal    = commandQueuePerfCounters.vkQueueSubmitCallsTotal;
    mPerfCounters.vkQueueSubmitCallsPerFrame = commandQueuePerfCounters.vkQueueSubmitCallsPerFrame;
    mPerfCounters.commandQueueWaitSemaphoresTotal =
        commandQueuePerfCounters.commandQueueWaitSemaphoresTotal;

    // Return current drawFramebuffer's cache stats
    mPerfCounters.framebufferCacheSize = mShareGroupVk->getFramebufferCache().getSize();

    mPerfCounters.pendingSubmissionGarbageObjects =
        static_cast<uint64_t>(mRenderer->getPendingSubmissionGarbageSize());
}

void ContextVk::updateOverlayOnPresent()
{
    const gl::OverlayType *overlay = mState.getOverlay();
    ASSERT(overlay->isEnabled());

    angle::VulkanPerfCounters commandQueuePerfCounters = mRenderer->getCommandQueuePerfCounters();
    syncObjectPerfCounters(commandQueuePerfCounters);

    // Update overlay if active.
    {
        gl::RunningGraphWidget *renderPassCount =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanRenderPassCount);
        renderPassCount->add(mRenderPassCommands->getAndResetCounter());
        renderPassCount->next();
    }

    {
        gl::RunningGraphWidget *writeDescriptorSetCount =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanWriteDescriptorSetCount);
        writeDescriptorSetCount->add(mPerfCounters.writeDescriptorSets);
        writeDescriptorSetCount->next();
    }

    {
        gl::RunningGraphWidget *descriptorSetAllocationCount =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanDescriptorSetAllocations);
        descriptorSetAllocationCount->add(mPerfCounters.descriptorSetAllocations);
        descriptorSetAllocationCount->next();
    }

    {
        gl::RunningGraphWidget *shaderResourceHitRate =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanShaderResourceDSHitRate);
        uint64_t numCacheAccesses = mPerfCounters.shaderResourcesDescriptorSetCacheHits +
                                    mPerfCounters.shaderResourcesDescriptorSetCacheMisses;
        if (numCacheAccesses > 0)
        {
            float hitRateFloat =
                static_cast<float>(mPerfCounters.shaderResourcesDescriptorSetCacheHits) /
                static_cast<float>(numCacheAccesses);
            size_t hitRate = static_cast<size_t>(hitRateFloat * 100.0f);
            shaderResourceHitRate->add(hitRate);
            shaderResourceHitRate->next();
        }
    }

    {
        gl::RunningGraphWidget *dynamicBufferAllocations =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanDynamicBufferAllocations);
        dynamicBufferAllocations->next();
    }

    {
        gl::CountWidget *cacheKeySize =
            overlay->getCountWidget(gl::WidgetId::VulkanDescriptorCacheKeySize);
        cacheKeySize->reset();
        cacheKeySize->add(mPerfCounters.descriptorSetCacheKeySizeBytes);
    }

    {
        gl::RunningGraphWidget *dynamicBufferAllocations =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanDynamicBufferAllocations);
        dynamicBufferAllocations->add(mPerfCounters.dynamicBufferAllocations);
    }

    {
        gl::RunningGraphWidget *attemptedSubmissionsWidget =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanAttemptedSubmissions);
        attemptedSubmissionsWidget->add(commandQueuePerfCounters.commandQueueSubmitCallsPerFrame);
        attemptedSubmissionsWidget->next();

        gl::RunningGraphWidget *actualSubmissionsWidget =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanActualSubmissions);
        actualSubmissionsWidget->add(commandQueuePerfCounters.vkQueueSubmitCallsPerFrame);
        actualSubmissionsWidget->next();
    }

    {
        gl::RunningGraphWidget *cacheLookupsWidget =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanPipelineCacheLookups);
        cacheLookupsWidget->add(mPerfCounters.pipelineCreationCacheHits +
                                mPerfCounters.pipelineCreationCacheMisses);
        cacheLookupsWidget->next();

        gl::RunningGraphWidget *cacheMissesWidget =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanPipelineCacheMisses);
        cacheMissesWidget->add(mPerfCounters.pipelineCreationCacheMisses);
        cacheMissesWidget->next();

        overlay->getCountWidget(gl::WidgetId::VulkanTotalPipelineCacheHitTimeMs)
            ->set(mPerfCounters.pipelineCreationTotalCacheHitsDurationNs / 1000'000);
        overlay->getCountWidget(gl::WidgetId::VulkanTotalPipelineCacheMissTimeMs)
            ->set(mPerfCounters.pipelineCreationTotalCacheMissesDurationNs / 1000'000);
    }
}

void ContextVk::addOverlayUsedBuffersCount(vk::CommandBufferHelperCommon *commandBuffer)
{
    const gl::OverlayType *overlay = mState.getOverlay();
    if (!overlay->isEnabled())
    {
        return;
    }

    {
        gl::RunningGraphWidget *textureDescriptorCacheSize =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanTextureDescriptorCacheSize);
        textureDescriptorCacheSize->add(mPerfCounters.textureDescriptorSetCacheTotalSize);
        textureDescriptorCacheSize->next();
    }

    {
        gl::RunningGraphWidget *uniformDescriptorCacheSize =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanUniformDescriptorCacheSize);
        uniformDescriptorCacheSize->add(mPerfCounters.uniformsAndXfbDescriptorSetCacheTotalSize);
        uniformDescriptorCacheSize->next();
    }

    {
        gl::RunningGraphWidget *descriptorCacheSize =
            overlay->getRunningGraphWidget(gl::WidgetId::VulkanDescriptorCacheSize);
        descriptorCacheSize->add(mPerfCounters.descriptorSetCacheTotalSize);
        descriptorCacheSize->next();
    }
}

angle::Result ContextVk::submitCommands(const vk::Semaphore *signalSemaphore,
                                        const vk::SharedExternalFence *externalFence,
                                        Submit submission)
{
    if (kEnableCommandStreamDiagnostics)
    {
        dumpCommandStreamDiagnostics();
    }

    if (!mCurrentGarbage.empty() && submission == Submit::AllCommands)
    {
        // Clean up garbage.
        vk::ResourceUse use(mLastFlushedQueueSerial);
        mRenderer->collectGarbage(use, std::move(mCurrentGarbage));
    }

    ASSERT(mLastFlushedQueueSerial.valid());
    ASSERT(QueueSerialsHaveDifferentIndexOrSmaller(mLastSubmittedQueueSerial,
                                                   mLastFlushedQueueSerial));

    finalizeAllForeignImages();
    ANGLE_TRY(mRenderer->submitCommands(
        this, getProtectionType(), mContextPriority, signalSemaphore, externalFence,
        std::move(mImagesToTransitionToForeign), mLastFlushedQueueSerial));

    mLastSubmittedQueueSerial = mLastFlushedQueueSerial;
    mSubmittedResourceUse.setQueueSerial(mLastSubmittedQueueSerial);

    // Now that we have submitted commands, some of pending garbage may no longer pending
    // and should be moved to garbage list.
    mRenderer->cleanupPendingSubmissionGarbage();
    // In case of big amount of render/submission within one frame, if we accumulate excessive
    // amount of garbage, also trigger the cleanup.
    mShareGroupVk->cleanupExcessiveRefCountedEventGarbage();

    mComputeDirtyBits |= mNewComputeCommandBufferDirtyBits;

    if (mGpuEventsEnabled)
    {
        ANGLE_TRY(checkCompletedGpuEvents());
    }

    mTotalBufferToImageCopySize       = 0;
    mEstimatedPendingImageGarbageSize = 0;

    // If we have destroyed a lot of memory, also prune to ensure memory gets freed as soon as
    // possible. For example we may end here when game launches and uploads a lot of textures before
    // draw the first frame.
    if (mRenderer->getSuballocationDestroyedSize() >= kMaxTotalEmptyBufferBytes)
    {
        mShareGroupVk->pruneDefaultBufferPools();
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::onCopyUpdate(VkDeviceSize size, bool *commandBufferWasFlushedOut)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::onCopyUpdate");
    *commandBufferWasFlushedOut = false;

    mTotalBufferToImageCopySize += size;
    // If the copy size exceeds the specified threshold, submit the outside command buffer.
    if (mTotalBufferToImageCopySize >= kMaxBufferToImageCopySize)
    {
        ANGLE_TRY(flushAndSubmitOutsideRenderPassCommands());
        *commandBufferWasFlushedOut = true;
    }
    return angle::Result::Continue;
}

void ContextVk::addToPendingImageGarbage(vk::ResourceUse use, VkDeviceSize size)
{
    if (!mRenderer->hasResourceUseFinished(use))
    {
        mEstimatedPendingImageGarbageSize += size;
    }
}

bool ContextVk::hasExcessPendingGarbage() const
{
    VkDeviceSize trackedPendingGarbage =
        mRenderer->getPendingSuballocationGarbageSize() + mEstimatedPendingImageGarbageSize;
    return trackedPendingGarbage >= mRenderer->getPendingGarbageSizeLimit();
}

angle::Result ContextVk::synchronizeCpuGpuTime()
{
    ASSERT(mGpuEventsEnabled);

    angle::PlatformMethods *platform = ANGLEPlatformCurrent();
    ASSERT(platform);

    // To synchronize CPU and GPU times, we need to get the CPU timestamp as close as possible
    // to the GPU timestamp.  The process of getting the GPU timestamp is as follows:
    //
    //             CPU                            GPU
    //
    //     Record command buffer
    //     with timestamp query
    //
    //     Submit command buffer
    //
    //     Post-submission work             Begin execution
    //
    //            ????                    Write timestamp Tgpu
    //
    //            ????                       End execution
    //
    //            ????                    Return query results
    //
    //            ????
    //
    //       Get query results
    //
    // The areas of unknown work (????) on the CPU indicate that the CPU may or may not have
    // finished post-submission work while the GPU is executing in parallel. With no further
    // work, querying CPU timestamps before submission and after getting query results give the
    // bounds to Tgpu, which could be quite large.
    //
    // Using VkEvents, the GPU can be made to wait for the CPU and vice versa, in an effort to
    // reduce this range. This function implements the following procedure:
    //
    //             CPU                            GPU
    //
    //     Record command buffer
    //     with timestamp query
    //
    //     Submit command buffer
    //
    //     Post-submission work             Begin execution
    //
    //            ????                    Set Event GPUReady
    //
    //    Wait on Event GPUReady         Wait on Event CPUReady
    //
    //       Get CPU Time Ts             Wait on Event CPUReady
    //
    //      Set Event CPUReady           Wait on Event CPUReady
    //
    //      Get CPU Time Tcpu              Get GPU Time Tgpu
    //
    //    Wait on Event GPUDone            Set Event GPUDone
    //
    //       Get CPU Time Te                 End Execution
    //
    //            Idle                    Return query results
    //
    //      Get query results
    //
    // If Te-Ts > epsilon, a GPU or CPU interruption can be assumed and the operation can be
    // retried.  Once Te-Ts < epsilon, Tcpu can be taken to presumably match Tgpu.  Finding an
    // epsilon that's valid for all devices may be difficult, so the loop can be performed only
    // a limited number of times and the Tcpu,Tgpu pair corresponding to smallest Te-Ts used for
    // calibration.
    //
    // Note: Once VK_EXT_calibrated_timestamps is ubiquitous, this should be redone.

    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::synchronizeCpuGpuTime");

    // Create a query used to receive the GPU timestamp
    vk::QueryHelper timestampQuery;
    ANGLE_TRY(mGpuEventQueryPool.allocateQuery(this, &timestampQuery, 1));

    // Create the three events
    VkEventCreateInfo eventCreateInfo = {};
    eventCreateInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    eventCreateInfo.flags             = 0;

    VkDevice device = getDevice();
    vk::DeviceScoped<vk::Event> cpuReady(device), gpuReady(device), gpuDone(device);
    ANGLE_VK_TRY(this, cpuReady.get().init(device, eventCreateInfo));
    ANGLE_VK_TRY(this, gpuReady.get().init(device, eventCreateInfo));
    ANGLE_VK_TRY(this, gpuDone.get().init(device, eventCreateInfo));

    constexpr uint32_t kRetries = 10;

    // Time suffixes used are S for seconds and Cycles for cycles
    double tightestRangeS = 1e6f;
    double TcpuS          = 0;
    uint64_t TgpuCycles   = 0;
    for (uint32_t i = 0; i < kRetries; ++i)
    {
        // Reset the events
        ANGLE_VK_TRY(this, cpuReady.get().reset(device));
        ANGLE_VK_TRY(this, gpuReady.get().reset(device));
        ANGLE_VK_TRY(this, gpuDone.get().reset(device));

        // Record the command buffer
        vk::ScopedPrimaryCommandBuffer scopedCommandBuffer(device);

        ANGLE_TRY(
            mRenderer->getCommandBufferOneOff(this, getProtectionType(), &scopedCommandBuffer));
        vk::PrimaryCommandBuffer &commandBuffer = scopedCommandBuffer.get();

        commandBuffer.setEvent(gpuReady.get().getHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
        commandBuffer.waitEvents(1, cpuReady.get().ptr(), VK_PIPELINE_STAGE_HOST_BIT,
                                 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, nullptr, 0, nullptr, 0,
                                 nullptr);
        timestampQuery.writeTimestampToPrimary(this, &commandBuffer);

        commandBuffer.setEvent(gpuDone.get().getHandle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

        ANGLE_VK_TRY(this, commandBuffer.end());

        QueueSerial submitSerial;
        // vkEvent's are externally synchronized, therefore need work to be submitted before calling
        // vkGetEventStatus
        ANGLE_TRY(mRenderer->queueSubmitOneOff(this, std::move(scopedCommandBuffer),
                                               getProtectionType(), mContextPriority,
                                               VK_NULL_HANDLE, 0, &submitSerial));

        // Track it with the submitSerial.
        timestampQuery.setQueueSerial(submitSerial);

        // Wait for GPU to be ready.  This is a short busy wait.
        VkResult result = VK_EVENT_RESET;
        do
        {
            result = gpuReady.get().getStatus(device);
            if (result != VK_EVENT_SET && result != VK_EVENT_RESET)
            {
                ANGLE_VK_TRY(this, result);
            }
        } while (result == VK_EVENT_RESET);

        double TsS = platform->monotonicallyIncreasingTime(platform);

        // Tell the GPU to go ahead with the timestamp query.
        ANGLE_VK_TRY(this, cpuReady.get().set(device));
        double cpuTimestampS = platform->monotonicallyIncreasingTime(platform);

        // Wait for GPU to be done.  Another short busy wait.
        do
        {
            result = gpuDone.get().getStatus(device);
            if (result != VK_EVENT_SET && result != VK_EVENT_RESET)
            {
                ANGLE_VK_TRY(this, result);
            }
        } while (result == VK_EVENT_RESET);

        double TeS = platform->monotonicallyIncreasingTime(platform);

        // Get the query results
        ANGLE_TRY(mRenderer->finishQueueSerial(this, submitSerial));

        vk::QueryResult gpuTimestampCycles(1);
        ANGLE_TRY(timestampQuery.getUint64Result(this, &gpuTimestampCycles));

        // Use the first timestamp queried as origin.
        if (mGpuEventTimestampOrigin == 0)
        {
            mGpuEventTimestampOrigin =
                gpuTimestampCycles.getResult(vk::QueryResult::kDefaultResultIndex);
        }

        // Take these CPU and GPU timestamps if there is better confidence.
        double confidenceRangeS = TeS - TsS;
        if (confidenceRangeS < tightestRangeS)
        {
            tightestRangeS = confidenceRangeS;
            TcpuS          = cpuTimestampS;
            TgpuCycles     = gpuTimestampCycles.getResult(vk::QueryResult::kDefaultResultIndex);
        }
    }

    mGpuEventQueryPool.freeQuery(this, &timestampQuery);

    // timestampPeriod gives nanoseconds/cycle.
    double TgpuS =
        (TgpuCycles - mGpuEventTimestampOrigin) *
        static_cast<double>(getRenderer()->getPhysicalDeviceProperties().limits.timestampPeriod) /
        1'000'000'000.0;

    flushGpuEvents(TgpuS, TcpuS);

    mGpuClockSync.gpuTimestampS = TgpuS;
    mGpuClockSync.cpuTimestampS = TcpuS;

    return angle::Result::Continue;
}

angle::Result ContextVk::traceGpuEventImpl(vk::OutsideRenderPassCommandBuffer *commandBuffer,
                                           char phase,
                                           const EventName &name)
{
    ASSERT(mGpuEventsEnabled);

    GpuEventQuery gpuEvent;
    gpuEvent.name  = name;
    gpuEvent.phase = phase;
    ANGLE_TRY(mGpuEventQueryPool.allocateQuery(this, &gpuEvent.queryHelper, 1));

    gpuEvent.queryHelper.writeTimestamp(this, commandBuffer);

    mInFlightGpuEventQueries.push_back(std::move(gpuEvent));
    return angle::Result::Continue;
}

angle::Result ContextVk::checkCompletedGpuEvents()
{
    ASSERT(mGpuEventsEnabled);

    angle::PlatformMethods *platform = ANGLEPlatformCurrent();
    ASSERT(platform);

    int finishedCount = 0;

    for (GpuEventQuery &eventQuery : mInFlightGpuEventQueries)
    {
        ASSERT(mRenderer->hasResourceUseSubmitted(eventQuery.queryHelper.getResourceUse()));
        // Only check the timestamp query if the submission has finished.
        if (!mRenderer->hasResourceUseFinished(eventQuery.queryHelper.getResourceUse()))
        {
            break;
        }

        // See if the results are available.
        vk::QueryResult gpuTimestampCycles(1);
        bool available = false;
        ANGLE_TRY(eventQuery.queryHelper.getUint64ResultNonBlocking(this, &gpuTimestampCycles,
                                                                    &available));
        if (!available)
        {
            break;
        }

        mGpuEventQueryPool.freeQuery(this, &eventQuery.queryHelper);

        GpuEvent gpuEvent;
        gpuEvent.gpuTimestampCycles =
            gpuTimestampCycles.getResult(vk::QueryResult::kDefaultResultIndex);
        gpuEvent.name  = eventQuery.name;
        gpuEvent.phase = eventQuery.phase;

        mGpuEvents.emplace_back(gpuEvent);

        ++finishedCount;
    }

    mInFlightGpuEventQueries.erase(mInFlightGpuEventQueries.begin(),
                                   mInFlightGpuEventQueries.begin() + finishedCount);

    return angle::Result::Continue;
}

void ContextVk::flushGpuEvents(double nextSyncGpuTimestampS, double nextSyncCpuTimestampS)
{
    if (mGpuEvents.empty())
    {
        return;
    }

    angle::PlatformMethods *platform = ANGLEPlatformCurrent();
    ASSERT(platform);

    // Find the slope of the clock drift for adjustment
    double lastGpuSyncTimeS  = mGpuClockSync.gpuTimestampS;
    double lastGpuSyncDiffS  = mGpuClockSync.cpuTimestampS - mGpuClockSync.gpuTimestampS;
    double gpuSyncDriftSlope = 0;

    double nextGpuSyncTimeS = nextSyncGpuTimestampS;
    double nextGpuSyncDiffS = nextSyncCpuTimestampS - nextSyncGpuTimestampS;

    // No gpu trace events should have been generated before the clock sync, so if there is no
    // "previous" clock sync, there should be no gpu events (i.e. the function early-outs
    // above).
    ASSERT(mGpuClockSync.gpuTimestampS != std::numeric_limits<double>::max() &&
           mGpuClockSync.cpuTimestampS != std::numeric_limits<double>::max());

    gpuSyncDriftSlope =
        (nextGpuSyncDiffS - lastGpuSyncDiffS) / (nextGpuSyncTimeS - lastGpuSyncTimeS);

    for (const GpuEvent &gpuEvent : mGpuEvents)
    {
        double gpuTimestampS =
            (gpuEvent.gpuTimestampCycles - mGpuEventTimestampOrigin) *
            static_cast<double>(
                getRenderer()->getPhysicalDeviceProperties().limits.timestampPeriod) *
            1e-9;

        // Account for clock drift.
        gpuTimestampS += lastGpuSyncDiffS + gpuSyncDriftSlope * (gpuTimestampS - lastGpuSyncTimeS);

        // Generate the trace now that the GPU timestamp is available and clock drifts are
        // accounted for.
        static long long eventId = 1;
        static const unsigned char *categoryEnabled =
            TRACE_EVENT_API_GET_CATEGORY_ENABLED(platform, "gpu.angle.gpu");
        platform->addTraceEvent(platform, gpuEvent.phase, categoryEnabled, gpuEvent.name.data(),
                                eventId++, gpuTimestampS, 0, nullptr, nullptr, nullptr,
                                TRACE_EVENT_FLAG_NONE);
    }

    mGpuEvents.clear();
}

void ContextVk::clearAllGarbage()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::clearAllGarbage");

    // The VMA virtual allocator code has assertion to ensure all sub-ranges are freed before
    // virtual block gets freed. We need to ensure all completed garbage objects are actually freed
    // to avoid hitting that assertion.
    mRenderer->cleanupGarbage(nullptr);

    for (vk::GarbageObject &garbage : mCurrentGarbage)
    {
        garbage.destroy(mRenderer);
    }
    mCurrentGarbage.clear();
}

void ContextVk::handleDeviceLost()
{
    vk::SecondaryCommandBufferCollector collector;
    (void)mOutsideRenderPassCommands->reset(this, &collector);
    (void)mRenderPassCommands->reset(this, &collector);
    collector.releaseCommandBuffers();

    mRenderer->notifyDeviceLost();
}

angle::Result ContextVk::drawArrays(const gl::Context *context,
                                    gl::PrimitiveMode mode,
                                    GLint first,
                                    GLsizei count)
{
    uint32_t clampedVertexCount = gl::GetClampedVertexCount<uint32_t>(count);

    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t numIndices;
        ANGLE_TRY(setupLineLoopDraw(context, mode, first, count, gl::DrawElementsType::InvalidEnum,
                                    nullptr, &numIndices));
        LineLoopHelper::Draw(numIndices, 0, mRenderPassCommandBuffer);
    }
    else
    {
        ANGLE_TRY(setupDraw(context, mode, first, count, 1, gl::DrawElementsType::InvalidEnum,
                            nullptr, mNonIndexedDirtyBitsMask));
        mRenderPassCommandBuffer->draw(clampedVertexCount, first);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::drawArraysInstanced(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             GLint first,
                                             GLsizei count,
                                             GLsizei instances)
{
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t clampedVertexCount = gl::GetClampedVertexCount<uint32_t>(count);
        uint32_t numIndices;
        ANGLE_TRY(setupLineLoopDraw(context, mode, first, clampedVertexCount,
                                    gl::DrawElementsType::InvalidEnum, nullptr, &numIndices));
        mRenderPassCommandBuffer->drawIndexedInstanced(numIndices, instances);
        return angle::Result::Continue;
    }

    ANGLE_TRY(setupDraw(context, mode, first, count, instances, gl::DrawElementsType::InvalidEnum,
                        nullptr, mNonIndexedDirtyBitsMask));
    mRenderPassCommandBuffer->drawInstanced(gl::GetClampedVertexCount<uint32_t>(count), instances,
                                            first);
    return angle::Result::Continue;
}

angle::Result ContextVk::drawArraysInstancedBaseInstance(const gl::Context *context,
                                                         gl::PrimitiveMode mode,
                                                         GLint first,
                                                         GLsizei count,
                                                         GLsizei instances,
                                                         GLuint baseInstance)
{
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t clampedVertexCount = gl::GetClampedVertexCount<uint32_t>(count);
        uint32_t numIndices;
        ANGLE_TRY(setupLineLoopDraw(context, mode, first, clampedVertexCount,
                                    gl::DrawElementsType::InvalidEnum, nullptr, &numIndices));
        mRenderPassCommandBuffer->drawIndexedInstancedBaseVertexBaseInstance(numIndices, instances,
                                                                             0, 0, baseInstance);
        return angle::Result::Continue;
    }

    ANGLE_TRY(setupDraw(context, mode, first, count, instances, gl::DrawElementsType::InvalidEnum,
                        nullptr, mNonIndexedDirtyBitsMask));
    mRenderPassCommandBuffer->drawInstancedBaseInstance(gl::GetClampedVertexCount<uint32_t>(count),
                                                        instances, first, baseInstance);
    return angle::Result::Continue;
}

angle::Result ContextVk::drawElements(const gl::Context *context,
                                      gl::PrimitiveMode mode,
                                      GLsizei count,
                                      gl::DrawElementsType type,
                                      const void *indices)
{
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t indexCount;
        ANGLE_TRY(setupLineLoopDraw(context, mode, 0, count, type, indices, &indexCount));
        LineLoopHelper::Draw(indexCount, 0, mRenderPassCommandBuffer);
    }
    else
    {
        ANGLE_TRY(setupIndexedDraw(context, mode, count, 1, type, indices));
        mRenderPassCommandBuffer->drawIndexed(count);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::drawElementsBaseVertex(const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                GLsizei count,
                                                gl::DrawElementsType type,
                                                const void *indices,
                                                GLint baseVertex)
{
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t indexCount;
        ANGLE_TRY(setupLineLoopDraw(context, mode, 0, count, type, indices, &indexCount));
        LineLoopHelper::Draw(indexCount, baseVertex, mRenderPassCommandBuffer);
    }
    else
    {
        ANGLE_TRY(setupIndexedDraw(context, mode, count, 1, type, indices));
        mRenderPassCommandBuffer->drawIndexedBaseVertex(count, baseVertex);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::drawElementsInstanced(const gl::Context *context,
                                               gl::PrimitiveMode mode,
                                               GLsizei count,
                                               gl::DrawElementsType type,
                                               const void *indices,
                                               GLsizei instances)
{
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t indexCount;
        ANGLE_TRY(setupLineLoopDraw(context, mode, 0, count, type, indices, &indexCount));
        count = indexCount;
    }
    else
    {
        ANGLE_TRY(setupIndexedDraw(context, mode, count, instances, type, indices));
    }

    mRenderPassCommandBuffer->drawIndexedInstanced(count, instances);
    return angle::Result::Continue;
}

angle::Result ContextVk::drawElementsInstancedBaseVertex(const gl::Context *context,
                                                         gl::PrimitiveMode mode,
                                                         GLsizei count,
                                                         gl::DrawElementsType type,
                                                         const void *indices,
                                                         GLsizei instances,
                                                         GLint baseVertex)
{
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t indexCount;
        ANGLE_TRY(setupLineLoopDraw(context, mode, 0, count, type, indices, &indexCount));
        count = indexCount;
    }
    else
    {
        ANGLE_TRY(setupIndexedDraw(context, mode, count, instances, type, indices));
    }

    mRenderPassCommandBuffer->drawIndexedInstancedBaseVertex(count, instances, baseVertex);
    return angle::Result::Continue;
}

angle::Result ContextVk::drawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                                     gl::PrimitiveMode mode,
                                                                     GLsizei count,
                                                                     gl::DrawElementsType type,
                                                                     const void *indices,
                                                                     GLsizei instances,
                                                                     GLint baseVertex,
                                                                     GLuint baseInstance)
{
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        uint32_t indexCount;
        ANGLE_TRY(setupLineLoopDraw(context, mode, 0, count, type, indices, &indexCount));
        count = indexCount;
    }
    else
    {
        ANGLE_TRY(setupIndexedDraw(context, mode, count, instances, type, indices));
    }

    mRenderPassCommandBuffer->drawIndexedInstancedBaseVertexBaseInstance(count, instances, 0,
                                                                         baseVertex, baseInstance);
    return angle::Result::Continue;
}

angle::Result ContextVk::drawRangeElements(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           GLuint start,
                                           GLuint end,
                                           GLsizei count,
                                           gl::DrawElementsType type,
                                           const void *indices)
{
    return drawElements(context, mode, count, type, indices);
}

angle::Result ContextVk::drawRangeElementsBaseVertex(const gl::Context *context,
                                                     gl::PrimitiveMode mode,
                                                     GLuint start,
                                                     GLuint end,
                                                     GLsizei count,
                                                     gl::DrawElementsType type,
                                                     const void *indices,
                                                     GLint baseVertex)
{
    return drawElementsBaseVertex(context, mode, count, type, indices, baseVertex);
}

VkDevice ContextVk::getDevice() const
{
    return mRenderer->getDevice();
}

angle::Result ContextVk::drawArraysIndirect(const gl::Context *context,
                                            gl::PrimitiveMode mode,
                                            const void *indirect)
{
    return multiDrawArraysIndirectHelper(context, mode, indirect, 1, 0);
}

angle::Result ContextVk::drawElementsIndirect(const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              gl::DrawElementsType type,
                                              const void *indirect)
{
    return multiDrawElementsIndirectHelper(context, mode, type, indirect, 1, 0);
}

angle::Result ContextVk::multiDrawArrays(const gl::Context *context,
                                         gl::PrimitiveMode mode,
                                         const GLint *firsts,
                                         const GLsizei *counts,
                                         GLsizei drawcount)
{
    return rx::MultiDrawArraysGeneral(this, context, mode, firsts, counts, drawcount);
}

angle::Result ContextVk::multiDrawArraysInstanced(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  const GLint *firsts,
                                                  const GLsizei *counts,
                                                  const GLsizei *instanceCounts,
                                                  GLsizei drawcount)
{
    return rx::MultiDrawArraysInstancedGeneral(this, context, mode, firsts, counts, instanceCounts,
                                               drawcount);
}

angle::Result ContextVk::multiDrawArraysIndirect(const gl::Context *context,
                                                 gl::PrimitiveMode mode,
                                                 const void *indirect,
                                                 GLsizei drawcount,
                                                 GLsizei stride)
{
    return multiDrawArraysIndirectHelper(context, mode, indirect, drawcount, stride);
}

angle::Result ContextVk::multiDrawArraysIndirectHelper(const gl::Context *context,
                                                       gl::PrimitiveMode mode,
                                                       const void *indirect,
                                                       GLsizei drawcount,
                                                       GLsizei stride)
{
    VertexArrayVk *vertexArrayVk = getVertexArray();
    if (drawcount > 1 && !CanMultiDrawIndirectUseCmd(this, vertexArrayVk, mode, drawcount, stride))
    {
        return rx::MultiDrawArraysIndirectGeneral(this, context, mode, indirect, drawcount, stride);
    }

    // Stride must be a multiple of the size of VkDrawIndirectCommand (stride = 0 is invalid when
    // drawcount > 1).
    uint32_t vkStride = (stride == 0 && drawcount > 1) ? sizeof(VkDrawIndirectCommand) : stride;

    gl::Buffer *indirectBuffer            = mState.getTargetBuffer(gl::BufferBinding::DrawIndirect);
    vk::BufferHelper *currentIndirectBuf  = &vk::GetImpl(indirectBuffer)->getBuffer();
    VkDeviceSize currentIndirectBufOffset = reinterpret_cast<VkDeviceSize>(indirect);

    if (vertexArrayVk->getStreamingVertexAttribsMask().any())
    {
        // Handling instanced vertex attributes is not covered for drawcount > 1.
        ASSERT(drawcount <= 1);

        // We have instanced vertex attributes that need to be emulated for Vulkan.
        // invalidate any cache and map the buffer so that we can read the indirect data.
        // Mapping the buffer will cause a flush.
        ANGLE_TRY(currentIndirectBuf->invalidate(mRenderer, 0, sizeof(VkDrawIndirectCommand)));
        uint8_t *buffPtr;
        ANGLE_TRY(currentIndirectBuf->map(this, &buffPtr));
        const VkDrawIndirectCommand *indirectData =
            reinterpret_cast<VkDrawIndirectCommand *>(buffPtr + currentIndirectBufOffset);

        ANGLE_TRY(drawArraysInstanced(context, mode, indirectData->firstVertex,
                                      indirectData->vertexCount, indirectData->instanceCount));

        currentIndirectBuf->unmap(mRenderer);
        return angle::Result::Continue;
    }

    if (mode == gl::PrimitiveMode::LineLoop)
    {
        // Line loop only supports handling at most one indirect parameter.
        ASSERT(drawcount <= 1);

        ASSERT(indirectBuffer);
        vk::BufferHelper *dstIndirectBuf = nullptr;

        ANGLE_TRY(setupLineLoopIndirectDraw(context, mode, currentIndirectBuf,
                                            currentIndirectBufOffset, &dstIndirectBuf));

        mRenderPassCommandBuffer->drawIndexedIndirect(
            dstIndirectBuf->getBuffer(), dstIndirectBuf->getOffset(), drawcount, vkStride);
        return angle::Result::Continue;
    }

    ANGLE_TRY(setupIndirectDraw(context, mode, mNonIndexedDirtyBitsMask, currentIndirectBuf));

    mRenderPassCommandBuffer->drawIndirect(
        currentIndirectBuf->getBuffer(), currentIndirectBuf->getOffset() + currentIndirectBufOffset,
        drawcount, vkStride);

    return angle::Result::Continue;
}

angle::Result ContextVk::multiDrawElements(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           const GLsizei *counts,
                                           gl::DrawElementsType type,
                                           const GLvoid *const *indices,
                                           GLsizei drawcount)
{
    return rx::MultiDrawElementsGeneral(this, context, mode, counts, type, indices, drawcount);
}

angle::Result ContextVk::multiDrawElementsInstanced(const gl::Context *context,
                                                    gl::PrimitiveMode mode,
                                                    const GLsizei *counts,
                                                    gl::DrawElementsType type,
                                                    const GLvoid *const *indices,
                                                    const GLsizei *instanceCounts,
                                                    GLsizei drawcount)
{
    return rx::MultiDrawElementsInstancedGeneral(this, context, mode, counts, type, indices,
                                                 instanceCounts, drawcount);
}

angle::Result ContextVk::multiDrawElementsIndirect(const gl::Context *context,
                                                   gl::PrimitiveMode mode,
                                                   gl::DrawElementsType type,
                                                   const void *indirect,
                                                   GLsizei drawcount,
                                                   GLsizei stride)
{
    return multiDrawElementsIndirectHelper(context, mode, type, indirect, drawcount, stride);
}

angle::Result ContextVk::multiDrawElementsIndirectHelper(const gl::Context *context,
                                                         gl::PrimitiveMode mode,
                                                         gl::DrawElementsType type,
                                                         const void *indirect,
                                                         GLsizei drawcount,
                                                         GLsizei stride)
{
    VertexArrayVk *vertexArrayVk = getVertexArray();
    if (drawcount > 1 && !CanMultiDrawIndirectUseCmd(this, vertexArrayVk, mode, drawcount, stride))
    {
        return rx::MultiDrawElementsIndirectGeneral(this, context, mode, type, indirect, drawcount,
                                                    stride);
    }

    // Stride must be a multiple of the size of VkDrawIndexedIndirectCommand (stride = 0 is invalid
    // when drawcount > 1).
    uint32_t vkStride =
        (stride == 0 && drawcount > 1) ? sizeof(VkDrawIndexedIndirectCommand) : stride;

    gl::Buffer *indirectBuffer = mState.getTargetBuffer(gl::BufferBinding::DrawIndirect);
    ASSERT(indirectBuffer);
    vk::BufferHelper *currentIndirectBuf  = &vk::GetImpl(indirectBuffer)->getBuffer();
    VkDeviceSize currentIndirectBufOffset = reinterpret_cast<VkDeviceSize>(indirect);

    // Reset the index buffer offset
    mGraphicsDirtyBits.set(DIRTY_BIT_INDEX_BUFFER);
    mCurrentIndexBufferOffset = 0;

    if (vertexArrayVk->getStreamingVertexAttribsMask().any())
    {
        // Handling instanced vertex attributes is not covered for drawcount > 1.
        ASSERT(drawcount <= 1);

        // We have instanced vertex attributes that need to be emulated for Vulkan.
        // invalidate any cache and map the buffer so that we can read the indirect data.
        // Mapping the buffer will cause a flush.
        ANGLE_TRY(
            currentIndirectBuf->invalidate(mRenderer, 0, sizeof(VkDrawIndexedIndirectCommand)));
        uint8_t *buffPtr;
        ANGLE_TRY(currentIndirectBuf->map(this, &buffPtr));
        const VkDrawIndexedIndirectCommand *indirectData =
            reinterpret_cast<VkDrawIndexedIndirectCommand *>(buffPtr + currentIndirectBufOffset);

        ANGLE_TRY(drawElementsInstanced(context, mode, indirectData->indexCount, type, nullptr,
                                        indirectData->instanceCount));

        currentIndirectBuf->unmap(mRenderer);
        return angle::Result::Continue;
    }

    if (shouldConvertUint8VkIndexType(type) && mGraphicsDirtyBits[DIRTY_BIT_INDEX_BUFFER])
    {
        ANGLE_VK_PERF_WARNING(
            this, GL_DEBUG_SEVERITY_LOW,
            "Potential inefficiency emulating uint8 vertex attributes due to lack "
            "of hardware support");

        ANGLE_TRY(vertexArrayVk->convertIndexBufferIndirectGPU(
            this, currentIndirectBuf, currentIndirectBufOffset, &currentIndirectBuf));
        currentIndirectBufOffset = 0;
    }

    // If the line-loop handling function modifies the element array buffer in the vertex array,
    // there is a possibility that the modified version is used as a source for the next line-loop
    // draw, which can lead to errors. To avoid this, a local index buffer pointer is used to pass
    // the current index buffer (after translation, in case it is needed) and use the resulting
    // index buffer for draw.
    vk::BufferHelper *currentIndexBuf = vertexArrayVk->getCurrentElementArrayBuffer();
    if (mode == gl::PrimitiveMode::LineLoop)
    {
        // Line loop only supports handling at most one indirect parameter.
        ASSERT(drawcount <= 1);
        ANGLE_TRY(setupLineLoopIndexedIndirectDraw(context, mode, type, currentIndexBuf,
                                                   currentIndirectBuf, currentIndirectBufOffset,
                                                   &currentIndirectBuf));
        currentIndirectBufOffset = 0;
    }
    else
    {
        ANGLE_TRY(setupIndexedIndirectDraw(context, mode, type, currentIndirectBuf));
    }

    mRenderPassCommandBuffer->drawIndexedIndirect(
        currentIndirectBuf->getBuffer(), currentIndirectBuf->getOffset() + currentIndirectBufOffset,
        drawcount, vkStride);

    return angle::Result::Continue;
}

angle::Result ContextVk::multiDrawArraysInstancedBaseInstance(const gl::Context *context,
                                                              gl::PrimitiveMode mode,
                                                              const GLint *firsts,
                                                              const GLsizei *counts,
                                                              const GLsizei *instanceCounts,
                                                              const GLuint *baseInstances,
                                                              GLsizei drawcount)
{
    return rx::MultiDrawArraysInstancedBaseInstanceGeneral(
        this, context, mode, firsts, counts, instanceCounts, baseInstances, drawcount);
}

angle::Result ContextVk::multiDrawElementsInstancedBaseVertexBaseInstance(
    const gl::Context *context,
    gl::PrimitiveMode mode,
    const GLsizei *counts,
    gl::DrawElementsType type,
    const GLvoid *const *indices,
    const GLsizei *instanceCounts,
    const GLint *baseVertices,
    const GLuint *baseInstances,
    GLsizei drawcount)
{
    return rx::MultiDrawElementsInstancedBaseVertexBaseInstanceGeneral(
        this, context, mode, counts, type, indices, instanceCounts, baseVertices, baseInstances,
        drawcount);
}

angle::Result ContextVk::optimizeRenderPassForPresent(vk::ImageViewHelper *colorImageView,
                                                      vk::ImageHelper *colorImage,
                                                      vk::ImageHelper *colorImageMS,
                                                      vk::PresentMode presentMode,
                                                      bool *imageResolved)
{
    // Note: mRenderPassCommandBuffer may be nullptr because the render pass is marked for closure.
    // That doesn't matter and the render pass can continue to be modified.  This function shouldn't
    // rely on mRenderPassCommandBuffer.

    // The caller must have verified this is the right render pass by calling
    // |hasStartedRenderPassWithSwapchainFramebuffer()|.
    ASSERT(mRenderPassCommands->started());

    // EGL1.5 spec: The contents of ancillary buffers are always undefined after calling
    // eglSwapBuffers
    FramebufferVk *drawFramebufferVk         = getDrawFramebuffer();
    RenderTargetVk *depthStencilRenderTarget = drawFramebufferVk->getDepthStencilRenderTarget();
    if (depthStencilRenderTarget != nullptr)
    {
        // Change depth/stencil attachment storeOp to DONT_CARE
        const gl::DepthStencilState &dsState = mState.getDepthStencilState();
        mRenderPassCommands->invalidateRenderPassDepthAttachment(
            dsState, mRenderPassCommands->getRenderArea());
        mRenderPassCommands->invalidateRenderPassStencilAttachment(
            dsState, mState.getDrawFramebuffer()->getStencilBitCount(),
            mRenderPassCommands->getRenderArea());
    }

    // Resolve the multisample image
    vk::RenderPassCommandBufferHelper &commandBufferHelper = getStartedRenderPassCommands();
    gl::Rectangle renderArea                               = commandBufferHelper.getRenderArea();
    const gl::Rectangle fullExtent(0, 0, colorImageMS->getRotatedExtents().width,
                                   colorImageMS->getRotatedExtents().height);
    const bool resolveWithRenderPass = colorImageMS->valid() && renderArea == fullExtent;

    // Handle transition to PRESENT_SRC automatically as part of the render pass.  If the swapchain
    // image is the target of resolve, but that resolve cannot happen with the render pass, do not
    // apply this optimization; the image has to be moved out of PRESENT_SRC to be resolved after
    // this call.
    if (getFeatures().supportsPresentation.enabled &&
        (!colorImageMS->valid() || resolveWithRenderPass))
    {
        ASSERT(colorImage != nullptr);
        mRenderPassCommands->setImageOptimizeForPresent(colorImage);
    }

    if (resolveWithRenderPass)
    {
        // Due to lack of support for GL_MESA_framebuffer_flip_y, it is currently impossible for the
        // application to resolve the default framebuffer into an FBO with a resolve attachment.  If
        // that is ever supported, the path that adds the resolve attachment would invalidate the
        // framebuffer that the render pass holds on to, in which case this function is not called.
        // Either way, there cannot be a resolve attachment here already.
        ASSERT(!mRenderPassCommands->getFramebuffer().hasColorResolveAttachment(0));

        // Add the resolve attachment to the render pass
        const vk::ImageView *resolveImageView = nullptr;
        ANGLE_TRY(colorImageView->getLevelLayerDrawImageView(this, *colorImage, vk::LevelIndex(0),
                                                             0, &resolveImageView));

        mRenderPassCommands->addColorResolveAttachment(0, colorImage, resolveImageView->getHandle(),
                                                       gl::LevelIndex(0), 0, 1, {});
        onImageRenderPassWrite(gl::LevelIndex(0), 0, 1, VK_IMAGE_ASPECT_COLOR_BIT,
                               vk::ImageLayout::ColorWrite, colorImage);

        // Invalidate the surface.  See comment in WindowSurfaceVk::doDeferredAcquireNextImage on
        // why this is not done when in DEMAND_REFRESH mode.
        if (presentMode != vk::PresentMode::SharedDemandRefreshKHR)
        {
            commandBufferHelper.invalidateRenderPassColorAttachment(
                mState, 0, vk::PackedAttachmentIndex(0), fullExtent);
        }

        ANGLE_TRY(
            flushCommandsAndEndRenderPass(RenderPassClosureReason::AlreadySpecifiedElsewhere));

        *imageResolved = true;

        mPerfCounters.swapchainResolveInSubpass++;
    }

    return angle::Result::Continue;
}

gl::GraphicsResetStatus ContextVk::getResetStatus()
{
    if (mRenderer->isDeviceLost())
    {
        // TODO(geofflang): It may be possible to track which context caused the device lost and
        // return either GL_GUILTY_CONTEXT_RESET or GL_INNOCENT_CONTEXT_RESET.
        // http://anglebug.com/42261488
        return gl::GraphicsResetStatus::UnknownContextReset;
    }

    return gl::GraphicsResetStatus::NoError;
}

angle::Result ContextVk::insertEventMarker(GLsizei length, const char *marker)
{
    insertEventMarkerImpl(GL_DEBUG_SOURCE_APPLICATION, marker);
    return angle::Result::Continue;
}

void ContextVk::insertEventMarkerImpl(GLenum source, const char *marker)
{
    if (!isDebugEnabled())
    {
        return;
    }

    VkDebugUtilsLabelEXT label;
    vk::MakeDebugUtilsLabel(source, marker, &label);

    if (hasActiveRenderPass())
    {
        mRenderPassCommandBuffer->insertDebugUtilsLabelEXT(label);
    }
    else
    {
        mOutsideRenderPassCommands->getCommandBuffer().insertDebugUtilsLabelEXT(label);
    }
}

angle::Result ContextVk::pushGroupMarker(GLsizei length, const char *marker)
{
    return pushDebugGroupImpl(GL_DEBUG_SOURCE_APPLICATION, 0, marker);
}

angle::Result ContextVk::popGroupMarker()
{
    return popDebugGroupImpl();
}

angle::Result ContextVk::pushDebugGroup(const gl::Context *context,
                                        GLenum source,
                                        GLuint id,
                                        const std::string &message)
{
    return pushDebugGroupImpl(source, id, message.c_str());
}

angle::Result ContextVk::popDebugGroup(const gl::Context *context)
{
    return popDebugGroupImpl();
}

angle::Result ContextVk::pushDebugGroupImpl(GLenum source, GLuint id, const char *message)
{
    if (!isDebugEnabled())
    {
        return angle::Result::Continue;
    }

    VkDebugUtilsLabelEXT label;
    vk::MakeDebugUtilsLabel(source, message, &label);

    if (hasActiveRenderPass())
    {
        mRenderPassCommandBuffer->beginDebugUtilsLabelEXT(label);
    }
    else
    {
        mOutsideRenderPassCommands->getCommandBuffer().beginDebugUtilsLabelEXT(label);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::popDebugGroupImpl()
{
    if (!isDebugEnabled())
    {
        return angle::Result::Continue;
    }

    if (hasActiveRenderPass())
    {
        mRenderPassCommandBuffer->endDebugUtilsLabelEXT();
    }
    else
    {
        mOutsideRenderPassCommands->getCommandBuffer().endDebugUtilsLabelEXT();
    }

    return angle::Result::Continue;
}

void ContextVk::logEvent(const char *eventString)
{
    if (!mRenderer->angleDebuggerMode())
    {
        return;
    }

    // Save this event (about an OpenGL ES command being called).
    mEventLog.push_back(eventString);

    // Set a dirty bit in order to stay off the "hot path" for when not logging.
    mGraphicsDirtyBits.set(DIRTY_BIT_EVENT_LOG);
    mComputeDirtyBits.set(DIRTY_BIT_EVENT_LOG);
}

void ContextVk::endEventLog(angle::EntryPoint entryPoint, PipelineType pipelineType)
{
    if (!mRenderer->angleDebuggerMode())
    {
        return;
    }

    if (pipelineType == PipelineType::Graphics)
    {
        ASSERT(mRenderPassCommands);
        mRenderPassCommands->getCommandBuffer().endDebugUtilsLabelEXT();
    }
    else
    {
        ASSERT(pipelineType == PipelineType::Compute);
        ASSERT(mOutsideRenderPassCommands);
        mOutsideRenderPassCommands->getCommandBuffer().endDebugUtilsLabelEXT();
    }
}
void ContextVk::endEventLogForClearOrQuery()
{
    if (!mRenderer->angleDebuggerMode())
    {
        return;
    }

    switch (mQueryEventType)
    {
        case GraphicsEventCmdBuf::InOutsideCmdBufQueryCmd:
            ASSERT(mOutsideRenderPassCommands);
            mOutsideRenderPassCommands->getCommandBuffer().endDebugUtilsLabelEXT();
            break;
        case GraphicsEventCmdBuf::InRenderPassCmdBufQueryCmd:
            ASSERT(mRenderPassCommands);
            mRenderPassCommands->getCommandBuffer().endDebugUtilsLabelEXT();
            break;
        case GraphicsEventCmdBuf::NotInQueryCmd:
            // The glClear* or gl*Query* command was noop'd or otherwise ended early.  We could
            // call handleDirtyEventLogImpl() to start the hierarchy, but it isn't clear which (if
            // any) command buffer to use.  We'll just skip processing this command (other than to
            // let it stay queued for the next time handleDirtyEventLogImpl() is called.
            return;
        default:
            UNREACHABLE();
    }

    mQueryEventType = GraphicsEventCmdBuf::NotInQueryCmd;
}

angle::Result ContextVk::handleNoopDrawEvent()
{
    // Even though this draw call is being no-op'd, we still must handle the dirty event log
    return handleDirtyEventLogImpl(mRenderPassCommandBuffer);
}

angle::Result ContextVk::handleGraphicsEventLog(GraphicsEventCmdBuf queryEventType)
{
    ASSERT(mQueryEventType == GraphicsEventCmdBuf::NotInQueryCmd || mEventLog.empty());
    if (!mRenderer->angleDebuggerMode())
    {
        return angle::Result::Continue;
    }

    mQueryEventType = queryEventType;

    switch (mQueryEventType)
    {
        case GraphicsEventCmdBuf::InOutsideCmdBufQueryCmd:
            ASSERT(mOutsideRenderPassCommands);
            return handleDirtyEventLogImpl(&mOutsideRenderPassCommands->getCommandBuffer());
        case GraphicsEventCmdBuf::InRenderPassCmdBufQueryCmd:
            ASSERT(mRenderPassCommands);
            return handleDirtyEventLogImpl(&mRenderPassCommands->getCommandBuffer());
        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }
}

bool ContextVk::isViewportFlipEnabledForDrawFBO() const
{
    return mFlipViewportForDrawFramebuffer && mFlipYForCurrentSurface;
}

bool ContextVk::isViewportFlipEnabledForReadFBO() const
{
    return mFlipViewportForReadFramebuffer;
}

bool ContextVk::isRotatedAspectRatioForDrawFBO() const
{
    return IsRotatedAspectRatio(mCurrentRotationDrawFramebuffer);
}

bool ContextVk::isRotatedAspectRatioForReadFBO() const
{
    return IsRotatedAspectRatio(mCurrentRotationReadFramebuffer);
}

SurfaceRotation ContextVk::getRotationDrawFramebuffer() const
{
    return mCurrentRotationDrawFramebuffer;
}

SurfaceRotation ContextVk::getRotationReadFramebuffer() const
{
    return mCurrentRotationReadFramebuffer;
}

SurfaceRotation ContextVk::getSurfaceRotationImpl(const gl::Framebuffer *framebuffer,
                                                  const egl::Surface *surface)
{
    SurfaceRotation surfaceRotation = SurfaceRotation::Identity;
    if (surface && surface->getType() == EGL_WINDOW_BIT)
    {
        const WindowSurfaceVk *windowSurface = GetImplAs<WindowSurfaceVk>(surface);
        surfaceRotation                      = DetermineSurfaceRotation(framebuffer, windowSurface);
    }
    return surfaceRotation;
}

void ContextVk::updateColorMasks()
{
    const gl::BlendStateExt &blendStateExt = mState.getBlendStateExt();

    mClearColorMasks = blendStateExt.getColorMaskBits();

    FramebufferVk *framebufferVk = vk::GetImpl(mState.getDrawFramebuffer());
    mGraphicsPipelineDesc->updateColorWriteMasks(&mGraphicsPipelineTransition, mClearColorMasks,
                                                 framebufferVk->getEmulatedAlphaAttachmentMask(),
                                                 framebufferVk->getState().getEnabledDrawBuffers());

    // This function may be called outside of ContextVk::syncState, and so invalidates the graphics
    // pipeline.
    invalidateCurrentGraphicsPipeline();

    onColorAccessChange();
}

void ContextVk::updateMissingAttachments()
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    if (executable == nullptr)
    {
        return;
    }

    // Handle missing color outputs
    const gl::DrawBufferMask framebufferMask    = mState.getDrawFramebuffer()->getDrawBufferMask();
    const gl::DrawBufferMask shaderOutMask      = executable->getActiveOutputVariablesMask();
    const gl::DrawBufferMask missingOutputsMask = ~shaderOutMask & framebufferMask;

    mGraphicsPipelineDesc->updateMissingOutputsMask(&mGraphicsPipelineTransition,
                                                    missingOutputsMask);

    // Handle missing depth/stencil attachment input.  If gl_LastFragDepth/StencilARM is used by the
    // shader but there is no depth/stencil attachment, the shader is changed not to read from the
    // input attachment.
    if (executable->usesDepthFramebufferFetch() || executable->usesStencilFramebufferFetch())
    {
        invalidateCurrentGraphicsPipeline();
    }
}

void ContextVk::updateBlendFuncsAndEquations()
{
    const gl::BlendStateExt &blendStateExt = mState.getBlendStateExt();

    FramebufferVk *framebufferVk              = vk::GetImpl(mState.getDrawFramebuffer());
    mCachedDrawFramebufferColorAttachmentMask = framebufferVk->getState().getEnabledDrawBuffers();

    mGraphicsPipelineDesc->updateBlendFuncs(&mGraphicsPipelineTransition, blendStateExt,
                                            mCachedDrawFramebufferColorAttachmentMask);

    mGraphicsPipelineDesc->updateBlendEquations(&mGraphicsPipelineTransition, blendStateExt,
                                                mCachedDrawFramebufferColorAttachmentMask);

    // This function may be called outside of ContextVk::syncState, and so invalidates the graphics
    // pipeline.
    invalidateCurrentGraphicsPipeline();
}

void ContextVk::updateSampleMaskWithRasterizationSamples(const uint32_t rasterizationSamples)
{
    static_assert(sizeof(uint32_t) == sizeof(GLbitfield), "Vulkan assumes 32-bit sample masks");
    ASSERT(mState.getMaxSampleMaskWords() == 1);

    uint32_t mask = std::numeric_limits<uint16_t>::max();

    // The following assumes that supported sample counts for multisampled
    // rendering does not include 1. This is true in the Vulkan backend,
    // where 1x multisampling is disallowed.
    if (rasterizationSamples > 1)
    {
        if (mState.isSampleMaskEnabled())
        {
            mask = mState.getSampleMaskWord(0) & angle::BitMask<uint32_t>(rasterizationSamples);
        }

        // If sample coverage is enabled, emulate it by generating and applying a mask on top of the
        // sample mask.
        if (mState.isSampleCoverageEnabled())
        {
            ApplySampleCoverage(mState, GetCoverageSampleCount(mState, rasterizationSamples),
                                &mask);
        }
    }

    mGraphicsPipelineDesc->updateSampleMask(&mGraphicsPipelineTransition, 0, mask);
}

void ContextVk::updateAlphaToCoverageWithRasterizationSamples(const uint32_t rasterizationSamples)
{
    // The following assumes that supported sample counts for multisampled
    // rendering does not include 1. This is true in the Vulkan backend,
    // where 1x multisampling is disallowed.
    mGraphicsPipelineDesc->updateAlphaToCoverageEnable(
        &mGraphicsPipelineTransition,
        mState.isSampleAlphaToCoverageEnabled() && rasterizationSamples > 1);
}

void ContextVk::updateFrameBufferFetchSamples(const uint32_t prevSamples, const uint32_t curSamples)
{
    const bool isPrevMultisampled = prevSamples > 1;
    const bool isCurMultisampled  = curSamples > 1;
    if (isPrevMultisampled != isCurMultisampled)
    {
        // If we change from single sample to multisample, we need to use the Shader Program with
        // ProgramTransformOptions.multisampleFramebufferFetch == true. Invalidate the graphics
        // pipeline so that we can fetch the shader with the correct permutation option in
        // handleDirtyGraphicsPipelineDesc()
        invalidateCurrentGraphicsPipeline();
    }
}

gl::Rectangle ContextVk::getCorrectedViewport(const gl::Rectangle &viewport) const
{
    const gl::Caps &caps                   = getCaps();
    const VkPhysicalDeviceLimits &limitsVk = mRenderer->getPhysicalDeviceProperties().limits;
    const int viewportBoundsRangeLow       = static_cast<int>(limitsVk.viewportBoundsRange[0]);
    const int viewportBoundsRangeHigh      = static_cast<int>(limitsVk.viewportBoundsRange[1]);

    // Clamp the viewport values to what Vulkan specifies

    // width must be greater than 0.0 and less than or equal to
    // VkPhysicalDeviceLimits::maxViewportDimensions[0]
    int correctedWidth = std::min<int>(viewport.width, caps.maxViewportWidth);
    correctedWidth     = std::max<int>(correctedWidth, 0);
    // height must be greater than 0.0 and less than or equal to
    // VkPhysicalDeviceLimits::maxViewportDimensions[1]
    int correctedHeight = std::min<int>(viewport.height, caps.maxViewportHeight);
    correctedHeight     = std::max<int>(correctedHeight, 0);
    // x and y must each be between viewportBoundsRange[0] and viewportBoundsRange[1], inclusive.
    // Viewport size cannot be 0 so ensure there is always size for a 1x1 viewport
    int correctedX = std::min<int>(viewport.x, viewportBoundsRangeHigh - 1);
    correctedX     = std::max<int>(correctedX, viewportBoundsRangeLow);
    int correctedY = std::min<int>(viewport.y, viewportBoundsRangeHigh - 1);
    correctedY     = std::max<int>(correctedY, viewportBoundsRangeLow);
    // x + width must be less than or equal to viewportBoundsRange[1]
    if ((correctedX + correctedWidth) > viewportBoundsRangeHigh)
    {
        correctedWidth = viewportBoundsRangeHigh - correctedX;
    }
    // y + height must be less than or equal to viewportBoundsRange[1]
    if ((correctedY + correctedHeight) > viewportBoundsRangeHigh)
    {
        correctedHeight = viewportBoundsRangeHigh - correctedY;
    }

    return gl::Rectangle(correctedX, correctedY, correctedWidth, correctedHeight);
}

void ContextVk::updateViewport(FramebufferVk *framebufferVk,
                               const gl::Rectangle &viewport,
                               float nearPlane,
                               float farPlane)
{

    gl::Box fbDimensions        = framebufferVk->getState().getDimensions();
    gl::Rectangle correctedRect = getCorrectedViewport(viewport);
    gl::Rectangle rotatedRect;
    RotateRectangle(getRotationDrawFramebuffer(), false, fbDimensions.width, fbDimensions.height,
                    correctedRect, &rotatedRect);

    const bool invertViewport = isViewportFlipEnabledForDrawFBO();

    gl_vk::GetViewport(
        rotatedRect, nearPlane, farPlane, invertViewport,
        // If clip space origin is upper left, viewport origin's y value will be offset by the
        // height of the viewport when clip space is mapped into screen space.
        mState.getClipOrigin() == gl::ClipOrigin::UpperLeft,
        // If the surface is rotated 90/270 degrees, use the framebuffer's width instead of the
        // height for calculating the final viewport.
        isRotatedAspectRatioForDrawFBO() ? fbDimensions.width : fbDimensions.height, &mViewport);

    // Ensure viewport is within Vulkan requirements
    vk::ClampViewport(&mViewport);

    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_VIEWPORT);
}

void ContextVk::updateFrontFace()
{
    if (mRenderer->getFeatures().useFrontFaceDynamicState.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_FRONT_FACE);
    }
    else
    {
        mGraphicsPipelineDesc->updateFrontFace(
            &mGraphicsPipelineTransition, mState.getRasterizerState(), isYFlipEnabledForDrawFBO());
    }
}

void ContextVk::updateDepthRange(float nearPlane, float farPlane)
{
    // GLES2.0 Section 2.12.1: Each of n and f are clamped to lie within [0, 1], as are all
    // arguments of type clampf.
    ASSERT(nearPlane >= 0.0f && nearPlane <= 1.0f);
    ASSERT(farPlane >= 0.0f && farPlane <= 1.0f);
    mViewport.minDepth = nearPlane;
    mViewport.maxDepth = farPlane;

    invalidateGraphicsDriverUniforms();
    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_VIEWPORT);
}

void ContextVk::updateScissor(const gl::State &glState)
{
    FramebufferVk *framebufferVk = vk::GetImpl(glState.getDrawFramebuffer());
    gl::Rectangle renderArea     = framebufferVk->getNonRotatedCompleteRenderArea();

    // Clip the render area to the viewport.
    gl::Rectangle viewportClippedRenderArea;
    if (!gl::ClipRectangle(renderArea, getCorrectedViewport(glState.getViewport()),
                           &viewportClippedRenderArea))
    {
        viewportClippedRenderArea = gl::Rectangle();
    }

    gl::Rectangle scissoredArea = ClipRectToScissor(getState(), viewportClippedRenderArea, false);
    gl::Rectangle rotatedScissoredArea;
    RotateRectangle(getRotationDrawFramebuffer(), isViewportFlipEnabledForDrawFBO(),
                    renderArea.width, renderArea.height, scissoredArea, &rotatedScissoredArea);
    mScissor = gl_vk::GetRect(rotatedScissoredArea);
    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_SCISSOR);

    // If the scissor has grown beyond the previous scissoredRenderArea, grow the render pass render
    // area.  The only undesirable effect this may have is that if the render area does not cover a
    // previously invalidated area, that invalidate will have to be discarded.
    if (mRenderPassCommandBuffer &&
        !mRenderPassCommands->getRenderArea().encloses(rotatedScissoredArea))
    {
        ASSERT(mRenderPassCommands->started());
        mRenderPassCommands->growRenderArea(this, rotatedScissoredArea);
    }
}

void ContextVk::updateDepthStencil(const gl::State &glState)
{
    updateDepthTestEnabled(glState);
    updateDepthWriteEnabled(glState);
    updateStencilTestEnabled(glState);
    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_WRITE_MASK);
}

void ContextVk::updateDepthTestEnabled(const gl::State &glState)
{
    const gl::DepthStencilState &depthStencilState = glState.getDepthStencilState();
    gl::Framebuffer *drawFramebuffer              = glState.getDrawFramebuffer();

    if (mRenderer->getFeatures().useDepthTestEnableDynamicState.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_TEST_ENABLE);
    }
    else
    {
        mGraphicsPipelineDesc->updateDepthTestEnabled(&mGraphicsPipelineTransition,
                                                      depthStencilState, drawFramebuffer);
    }
}

void ContextVk::updateDepthWriteEnabled(const gl::State &glState)
{
    const gl::DepthStencilState &depthStencilState = glState.getDepthStencilState();
    gl::Framebuffer *drawFramebuffer              = glState.getDrawFramebuffer();

    if (mRenderer->getFeatures().useDepthWriteEnableDynamicState.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_WRITE_ENABLE);
    }
    else
    {
        mGraphicsPipelineDesc->updateDepthWriteEnabled(&mGraphicsPipelineTransition,
                                                       depthStencilState, drawFramebuffer);
    }
}

void ContextVk::updateDepthFunc(const gl::State &glState)
{
    if (mRenderer->getFeatures().useDepthCompareOpDynamicState.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_COMPARE_OP);
    }
    else
    {
        mGraphicsPipelineDesc->updateDepthFunc(&mGraphicsPipelineTransition,
                                               glState.getDepthStencilState());
    }
}

void ContextVk::updateStencilTestEnabled(const gl::State &glState)
{
    const gl::DepthStencilState &depthStencilState = glState.getDepthStencilState();
    gl::Framebuffer *drawFramebuffer              = glState.getDrawFramebuffer();

    if (mRenderer->getFeatures().useStencilTestEnableDynamicState.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_TEST_ENABLE);
    }
    else
    {
        mGraphicsPipelineDesc->updateStencilTestEnabled(&mGraphicsPipelineTransition,
                                                        depthStencilState, drawFramebuffer);
    }
}

// If the target is a single-sampled target, sampleShading should be disabled, to use Bresenham line
// rasterization feature.
void ContextVk::updateSampleShadingWithRasterizationSamples(const uint32_t rasterizationSamples)
{
    bool sampleShadingEnable =
        (rasterizationSamples <= 1 ? false : mState.isSampleShadingEnabled());
    float minSampleShading = mState.getMinSampleShading();

    // If sample shading is not enabled, check if it should be implicitly enabled according to the
    // program.  Normally the driver should do this, but some drivers don't.
    if (rasterizationSamples > 1 && !sampleShadingEnable &&
        getFeatures().explicitlyEnablePerSampleShading.enabled)
    {
        const gl::ProgramExecutable *executable = mState.getProgramExecutable();
        if (executable && executable->enablesPerSampleShading())
        {
            sampleShadingEnable = true;
            minSampleShading    = 1.0;
        }
    }

    mGraphicsPipelineDesc->updateSampleShading(&mGraphicsPipelineTransition, sampleShadingEnable,
                                               minSampleShading);
}

// If the target is switched between a single-sampled and multisample, the dependency related to the
// rasterization sample should be updated.
void ContextVk::updateRasterizationSamples(const uint32_t rasterizationSamples)
{
    uint32_t prevSampleCount = mGraphicsPipelineDesc->getRasterizationSamples();
    updateFrameBufferFetchSamples(prevSampleCount, rasterizationSamples);
    mGraphicsPipelineDesc->updateRasterizationSamples(&mGraphicsPipelineTransition,
                                                      rasterizationSamples);
    updateSampleShadingWithRasterizationSamples(rasterizationSamples);
    updateSampleMaskWithRasterizationSamples(rasterizationSamples);
    updateAlphaToCoverageWithRasterizationSamples(rasterizationSamples);
}

void ContextVk::updateRasterizerDiscardEnabled(bool isPrimitivesGeneratedQueryActive)
{
    // On some devices, when rasterizerDiscardEnable is enabled, the
    // VK_EXT_primitives_generated_query as well as the pipeline statistics query used to emulate it
    // are non-functional.  For VK_EXT_primitives_generated_query there's a feature bit but not for
    // pipeline statistics query.  If the primitives generated query is active (and rasterizer
    // discard is not supported), rasterizerDiscardEnable is set to false and the functionality
    // is otherwise emulated (by using an empty scissor).

    // If the primitives generated query implementation supports rasterizer discard, just set
    // rasterizer discard as requested.  Otherwise disable it.
    const bool isEmulatingRasterizerDiscard =
        isEmulatingRasterizerDiscardDuringPrimitivesGeneratedQuery(
            isPrimitivesGeneratedQueryActive);

    if (mRenderer->getFeatures().useRasterizerDiscardEnableDynamicState.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_RASTERIZER_DISCARD_ENABLE);
    }
    else
    {
        const bool isRasterizerDiscardEnabled = mState.isRasterizerDiscardEnabled();

        mGraphicsPipelineDesc->updateRasterizerDiscardEnabled(
            &mGraphicsPipelineTransition,
            isRasterizerDiscardEnabled && !isEmulatingRasterizerDiscard);

        invalidateCurrentGraphicsPipeline();
    }

    if (isEmulatingRasterizerDiscard)
    {
        // If we are emulating rasterizer discard, update the scissor to use an empty one if
        // rasterizer discard is enabled.
        mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_SCISSOR);
    }
}

void ContextVk::updateAdvancedBlendEquations(const gl::ProgramExecutable *executable)
{
    if (!getFeatures().emulateAdvancedBlendEquations.enabled || executable == nullptr)
    {
        return;
    }

    // If advanced blend equations is emulated and the program uses advanced equations, update the
    // driver uniforms to pass the equation to the shader.
    if (executable->getAdvancedBlendEquations().any())
    {
        invalidateGraphicsDriverUniforms();
    }
}

void ContextVk::updateDither()
{
    if (getFeatures().supportsLegacyDithering.enabled)
    {
        FramebufferVk *framebufferVk = vk::GetImpl(mState.getDrawFramebuffer());
        if (framebufferVk->updateLegacyDither(this))
        {
            // Can't reactivate: same framebuffer but the render pass desc has changed.
            mAllowRenderPassToReactivate = false;

            onRenderPassFinished(RenderPassClosureReason::LegacyDithering);
        }

        // update GraphicsPipelineDesc renderpass legacy dithering bit
        if (isDitherEnabled() != mGraphicsPipelineDesc->isLegacyDitherEnabled())
        {
            const vk::FramebufferFetchMode framebufferFetchMode =
                vk::GetProgramFramebufferFetchMode(mState.getProgramExecutable());
            mGraphicsPipelineDesc->updateRenderPassDesc(&mGraphicsPipelineTransition, getFeatures(),
                                                        framebufferVk->getRenderPassDesc(),
                                                        framebufferFetchMode);
            invalidateCurrentGraphicsPipeline();
        }
    }

    if (!getFeatures().emulateDithering.enabled)
    {
        return;
    }

    FramebufferVk *framebufferVk = vk::GetImpl(mState.getDrawFramebuffer());

    // Dithering in OpenGL is vaguely defined, to the extent that no dithering is also a valid
    // dithering algorithm.  Dithering is enabled by default, but emulating it has a non-negligible
    // cost.  Similarly to some other GLES drivers, ANGLE enables dithering only on low-bit formats
    // where visual banding is particularly common; namely RGBA4444, RGBA5551 and RGB565.
    //
    // Dithering is emulated in the fragment shader and is controlled by a spec constant.  Every 2
    // bits of the spec constant correspond to one attachment, with the value indicating:
    //
    // - 00: No dithering
    // - 01: Dither for RGBA4444
    // - 10: Dither for RGBA5551
    // - 11: Dither for RGB565
    //
    uint16_t ditherControl = 0;
    if (mState.isDitherEnabled())
    {
        const gl::DrawBufferMask attachmentMask =
            framebufferVk->getState().getColorAttachmentsMask();

        for (size_t colorIndex : attachmentMask)
        {
            // As dithering is emulated in the fragment shader itself, there are a number of
            // situations that can lead to incorrect blending.  We only allow blending with specific
            // combinations know to not interfere with dithering.
            if (mState.isBlendEnabledIndexed(static_cast<GLuint>(colorIndex)) &&
                !BlendModeSupportsDither(this, colorIndex))
            {
                continue;
            }

            RenderTargetVk *attachment = framebufferVk->getColorDrawRenderTarget(colorIndex);

            const angle::FormatID format = attachment->getImageActualFormatID();

            uint16_t attachmentDitherControl = sh::vk::kDitherControlNoDither;
            switch (format)
            {
                case angle::FormatID::R4G4B4A4_UNORM:
                case angle::FormatID::B4G4R4A4_UNORM:
                    attachmentDitherControl = sh::vk::kDitherControlDither4444;
                    break;
                case angle::FormatID::R5G5B5A1_UNORM:
                case angle::FormatID::B5G5R5A1_UNORM:
                case angle::FormatID::A1R5G5B5_UNORM:
                    attachmentDitherControl = sh::vk::kDitherControlDither5551;
                    break;
                case angle::FormatID::R5G6B5_UNORM:
                case angle::FormatID::B5G6R5_UNORM:
                    attachmentDitherControl = sh::vk::kDitherControlDither565;
                    break;
                default:
                    break;
            }

            ditherControl |= static_cast<uint16_t>(attachmentDitherControl << 2 * colorIndex);
        }
    }

    if (ditherControl != mGraphicsPipelineDesc->getEmulatedDitherControl())
    {
        mGraphicsPipelineDesc->updateEmulatedDitherControl(&mGraphicsPipelineTransition,
                                                           ditherControl);
        invalidateCurrentGraphicsPipeline();
    }
}

void ContextVk::updateStencilWriteWorkaround()
{
    if (!getFeatures().useNonZeroStencilWriteMaskStaticState.enabled)
    {
        return;
    }

    // On certain drivers, having a stencil write mask of 0 in static state enables optimizations
    // that make the interaction of the stencil write mask dynamic state with discard and alpha to
    // coverage broken.  When the program has discard, or when alpha to coverage is enabled, these
    // optimizations are disabled by specifying a non-zero static state for stencil write mask.
    const bool programHasDiscard        = mState.getProgramExecutable()->hasDiscard();
    const bool isAlphaToCoverageEnabled = mState.isSampleAlphaToCoverageEnabled();

    mGraphicsPipelineDesc->updateNonZeroStencilWriteMaskWorkaround(
        &mGraphicsPipelineTransition, programHasDiscard || isAlphaToCoverageEnabled);
}

angle::Result ContextVk::invalidateProgramExecutableHelper(const gl::Context *context)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();

    if (executable->hasLinkedShaderStage(gl::ShaderType::Compute))
    {
        invalidateCurrentComputePipeline();
    }

    if (executable->hasLinkedShaderStage(gl::ShaderType::Vertex))
    {
        invalidateCurrentGraphicsPipeline();
        // No additional work is needed here. We will update the pipeline desc
        // later.
        invalidateDefaultAttributes(context->getStateCache().getActiveDefaultAttribsMask());
        invalidateVertexAndIndexBuffers();
        // If VK_EXT_vertex_input_dynamic_state is enabled then vkCmdSetVertexInputEXT must be
        // called in the current command buffer prior to the draw command, even if there are no
        // active vertex attributes.
        const bool useVertexBuffer = (executable->getMaxActiveAttribLocation() > 0) ||
                                     getFeatures().supportsVertexInputDynamicState.enabled;
        mNonIndexedDirtyBitsMask.set(DIRTY_BIT_VERTEX_BUFFERS, useVertexBuffer);
        mIndexedDirtyBitsMask.set(DIRTY_BIT_VERTEX_BUFFERS, useVertexBuffer);
        resetCurrentGraphicsPipeline();

        const vk::FramebufferFetchMode framebufferFetchMode =
            vk::GetProgramFramebufferFetchMode(executable);
        const bool hasColorFramebufferFetch =
            framebufferFetchMode != vk::FramebufferFetchMode::None;
        if (getFeatures().preferDynamicRendering.enabled)
        {
            // Update the framebuffer fetch mode on the pipeline desc directly.  This is an inherent
            // property of the executable. Even if the bit is placed in RenderPassDesc because of
            // the non-dynamic-rendering path, updating it without affecting the transition bits is
            // valid because there cannot be a transition link between pipelines of different
            // programs.  This is attested by the fact that |resetCurrentGraphicsPipeline| above
            // sets |mCurrentGraphicsPipeline| to nullptr.
            mGraphicsPipelineDesc->setRenderPassFramebufferFetchMode(framebufferFetchMode);

            if (framebufferFetchMode != vk::FramebufferFetchMode::None)
            {
                onFramebufferFetchUse(framebufferFetchMode);
            }
        }
        else
        {
            ASSERT(!FramebufferFetchModeHasDepthStencil(framebufferFetchMode));
            if (mIsInColorFramebufferFetchMode != hasColorFramebufferFetch)
            {
                ASSERT(getDrawFramebuffer()->getRenderPassDesc().hasColorFramebufferFetch() ==
                       mIsInColorFramebufferFetchMode);

                ANGLE_TRY(switchToColorFramebufferFetchMode(hasColorFramebufferFetch));

                // When framebuffer fetch is enabled, attachments can be read from even if output is
                // masked, so update their access.
                onColorAccessChange();
            }

            // If permanentlySwitchToFramebufferFetchMode is enabled,
            // mIsInColorFramebufferFetchMode will remain true throughout the entire time.
            // If we switch from a program that doesn't use framebuffer fetch and doesn't
            // read/write to the framebuffer color attachment, to a
            // program that uses framebuffer fetch and needs to read from the framebuffer
            // color attachment, we will miss the call
            // onColorAccessChange() above and miss setting the dirty bit
            // DIRTY_BIT_COLOR_ACCESS. This means we will not call
            // handleDirtyGraphicsColorAccess that updates the access value of
            // framebuffer color attachment from unused to readonly. This makes the
            // color attachment to continue using LoadOpNone, and the second program
            // will not be able to read the value in the color attachment.
            if (getFeatures().permanentlySwitchToFramebufferFetchMode.enabled &&
                hasColorFramebufferFetch)
            {
                onColorAccessChange();
            }
        }

        // If framebuffer fetch is exposed but is internally non-coherent, make sure a framebuffer
        // fetch barrier is issued before each draw call as long as a program with framebuffer fetch
        // is used.  If the application would have correctly used non-coherent framebuffer fetch, it
        // would have been optimal _not_ to expose the coherent extension.  However, lots of
        // Android applications expect coherent framebuffer fetch to be available.
        if (mRenderer->isCoherentColorFramebufferFetchEmulated())
        {
            mGraphicsDirtyBits.set(DIRTY_BIT_FRAMEBUFFER_FETCH_BARRIER, hasColorFramebufferFetch);
        }

        updateStencilWriteWorkaround();

        mGraphicsPipelineDesc->updateVertexShaderComponentTypes(
            &mGraphicsPipelineTransition, executable->getNonBuiltinAttribLocationsMask(),
            executable->getAttributesTypeMask());

        updateMissingAttachments();
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::syncState(const gl::Context *context,
                                   const gl::state::DirtyBits dirtyBits,
                                   const gl::state::DirtyBits bitMask,
                                   const gl::state::ExtendedDirtyBits extendedDirtyBits,
                                   const gl::state::ExtendedDirtyBits extendedBitMask,
                                   gl::Command command)
{
    const gl::State &glState                       = context->getState();
    const gl::ProgramExecutable *programExecutable = glState.getProgramExecutable();

    if ((dirtyBits & mPipelineDirtyBitsMask).any() &&
        (programExecutable == nullptr || command != gl::Command::Dispatch))
    {
        invalidateCurrentGraphicsPipeline();
    }

    FramebufferVk *drawFramebufferVk = getDrawFramebuffer();
    VertexArrayVk *vertexArrayVk     = getVertexArray();

    for (auto iter = dirtyBits.begin(), endIter = dirtyBits.end(); iter != endIter; ++iter)
    {
        size_t dirtyBit = *iter;
        switch (dirtyBit)
        {
            case gl::state::DIRTY_BIT_SCISSOR_TEST_ENABLED:
            case gl::state::DIRTY_BIT_SCISSOR:
                updateScissor(glState);
                break;
            case gl::state::DIRTY_BIT_VIEWPORT:
            {
                FramebufferVk *framebufferVk = vk::GetImpl(glState.getDrawFramebuffer());
                updateViewport(framebufferVk, glState.getViewport(), glState.getNearPlane(),
                               glState.getFarPlane());
                // Update the scissor, which will be constrained to the viewport
                updateScissor(glState);
                break;
            }
            case gl::state::DIRTY_BIT_DEPTH_RANGE:
                updateDepthRange(glState.getNearPlane(), glState.getFarPlane());
                break;
            case gl::state::DIRTY_BIT_BLEND_ENABLED:
                mGraphicsPipelineDesc->updateBlendEnabled(
                    &mGraphicsPipelineTransition, glState.getBlendStateExt().getEnabledMask());
                updateDither();
                updateAdvancedBlendEquations(programExecutable);
                break;
            case gl::state::DIRTY_BIT_BLEND_COLOR:
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_BLEND_CONSTANTS);
                break;
            case gl::state::DIRTY_BIT_BLEND_FUNCS:
                mGraphicsPipelineDesc->updateBlendFuncs(
                    &mGraphicsPipelineTransition, glState.getBlendStateExt(),
                    drawFramebufferVk->getState().getColorAttachmentsMask());
                break;
            case gl::state::DIRTY_BIT_BLEND_EQUATIONS:
                mGraphicsPipelineDesc->updateBlendEquations(
                    &mGraphicsPipelineTransition, glState.getBlendStateExt(),
                    drawFramebufferVk->getState().getColorAttachmentsMask());
                updateAdvancedBlendEquations(programExecutable);
                break;
            case gl::state::DIRTY_BIT_COLOR_MASK:
                updateColorMasks();
                break;
            case gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
                updateAlphaToCoverageWithRasterizationSamples(drawFramebufferVk->getSamples());
                updateStencilWriteWorkaround();

                static_assert(gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE >
                                  gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED,
                              "Dirty bit order");
                iter.setLaterBit(gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE);
                break;
            case gl::state::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED:
                updateSampleMaskWithRasterizationSamples(drawFramebufferVk->getSamples());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_COVERAGE:
                updateSampleMaskWithRasterizationSamples(drawFramebufferVk->getSamples());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_MASK_ENABLED:
                updateSampleMaskWithRasterizationSamples(drawFramebufferVk->getSamples());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_MASK:
                updateSampleMaskWithRasterizationSamples(drawFramebufferVk->getSamples());
                break;
            case gl::state::DIRTY_BIT_DEPTH_TEST_ENABLED:
                updateDepthTestEnabled(glState);
                iter.setLaterBit(gl::state::DIRTY_BIT_DEPTH_MASK);
                break;
            case gl::state::DIRTY_BIT_DEPTH_FUNC:
                updateDepthFunc(glState);
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_DEPTH_MASK:
                updateDepthWriteEnabled(glState);
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_STENCIL_TEST_ENABLED:
                updateStencilTestEnabled(glState);
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_FRONT:
                if (mRenderer->getFeatures().useStencilOpDynamicState.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_OP);
                }
                else
                {
                    mGraphicsPipelineDesc->updateStencilFrontFuncs(&mGraphicsPipelineTransition,
                                                                   glState.getDepthStencilState());
                }
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_COMPARE_MASK);
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_REFERENCE);
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_BACK:
                if (mRenderer->getFeatures().useStencilOpDynamicState.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_OP);
                }
                else
                {
                    mGraphicsPipelineDesc->updateStencilBackFuncs(&mGraphicsPipelineTransition,
                                                                  glState.getDepthStencilState());
                }
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_COMPARE_MASK);
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_REFERENCE);
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_STENCIL_OPS_FRONT:
                if (mRenderer->getFeatures().useStencilOpDynamicState.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_OP);
                }
                else
                {
                    mGraphicsPipelineDesc->updateStencilFrontOps(&mGraphicsPipelineTransition,
                                                                 glState.getDepthStencilState());
                }
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_STENCIL_OPS_BACK:
                if (mRenderer->getFeatures().useStencilOpDynamicState.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_OP);
                }
                else
                {
                    mGraphicsPipelineDesc->updateStencilBackOps(&mGraphicsPipelineTransition,
                                                                glState.getDepthStencilState());
                }
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_STENCIL_WRITE_MASK);
                onDepthStencilAccessChange();
                break;
            case gl::state::DIRTY_BIT_CULL_FACE_ENABLED:
            case gl::state::DIRTY_BIT_CULL_FACE:
                if (mRenderer->getFeatures().useCullModeDynamicState.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_CULL_MODE);
                }
                else
                {
                    mGraphicsPipelineDesc->updateCullMode(&mGraphicsPipelineTransition,
                                                          glState.getRasterizerState());
                }
                break;
            case gl::state::DIRTY_BIT_FRONT_FACE:
                updateFrontFace();
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED:
                if (mRenderer->getFeatures().useDepthBiasEnableDynamicState.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_BIAS_ENABLE);
                }
                else
                {
                    mGraphicsPipelineDesc->updatePolygonOffsetEnabled(
                        &mGraphicsPipelineTransition, glState.isPolygonOffsetEnabled());
                }
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET:
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_BIAS);
                break;
            case gl::state::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED:
                updateRasterizerDiscardEnabled(
                    mState.isQueryActive(gl::QueryType::PrimitivesGenerated));
                onColorAccessChange();
                break;
            case gl::state::DIRTY_BIT_LINE_WIDTH:
                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_LINE_WIDTH);
                break;
            case gl::state::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED:
                if (mRenderer->getFeatures().usePrimitiveRestartEnableDynamicState.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_PRIMITIVE_RESTART_ENABLE);
                }
                else
                {
                    mGraphicsPipelineDesc->updatePrimitiveRestartEnabled(
                        &mGraphicsPipelineTransition, glState.isPrimitiveRestartEnabled());
                }
                // Additionally set the index buffer dirty if conversion from uint8 might have been
                // necessary.  Otherwise if primitive restart is enabled and the index buffer is
                // translated to uint16_t with a value of 0xFFFF, it cannot be reused when primitive
                // restart is disabled.
                if (!mRenderer->getFeatures().supportsIndexTypeUint8.enabled)
                {
                    mGraphicsDirtyBits.set(DIRTY_BIT_INDEX_BUFFER);
                }
                break;
            case gl::state::DIRTY_BIT_CLEAR_COLOR:
                mClearColorValue.color.float32[0] = glState.getColorClearValue().red;
                mClearColorValue.color.float32[1] = glState.getColorClearValue().green;
                mClearColorValue.color.float32[2] = glState.getColorClearValue().blue;
                mClearColorValue.color.float32[3] = glState.getColorClearValue().alpha;
                break;
            case gl::state::DIRTY_BIT_CLEAR_DEPTH:
                mClearDepthStencilValue.depthStencil.depth = glState.getDepthClearValue();
                break;
            case gl::state::DIRTY_BIT_CLEAR_STENCIL:
                mClearDepthStencilValue.depthStencil.stencil =
                    static_cast<uint32_t>(glState.getStencilClearValue());
                break;
            case gl::state::DIRTY_BIT_UNPACK_STATE:
                // This is a no-op, it's only important to use the right unpack state when we do
                // setImage or setSubImage in TextureVk, which is plumbed through the frontend
                // call
                break;
            case gl::state::DIRTY_BIT_UNPACK_BUFFER_BINDING:
                break;
            case gl::state::DIRTY_BIT_PACK_STATE:
                // This is a no-op, its only important to use the right pack state when we do
                // call readPixels later on.
                break;
            case gl::state::DIRTY_BIT_PACK_BUFFER_BINDING:
                break;
            case gl::state::DIRTY_BIT_DITHER_ENABLED:
                updateDither();
                break;
            case gl::state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING:
                updateFlipViewportReadFramebuffer(context->getState());
                updateSurfaceRotationReadFramebuffer(glState, context->getCurrentReadSurface());
                break;
            case gl::state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING:
            {
                // FramebufferVk::syncState signals that we should start a new command buffer.
                // But changing the binding can skip FramebufferVk::syncState if the Framebuffer
                // has no dirty bits. Thus we need to explicitly clear the current command
                // buffer to ensure we start a new one. We don't actually close the render pass here
                // as some optimizations in non-draw commands require the render pass to remain
                // open, such as invalidate or blit. Note that we always start a new command buffer
                // because we currently can only support one open RenderPass at a time.
                //
                // The render pass is not closed if binding is changed to the same framebuffer as
                // before.
                if (hasActiveRenderPass() && hasStartedRenderPassWithQueueSerial(
                                                 drawFramebufferVk->getLastRenderPassQueueSerial()))
                {
                    break;
                }

                onRenderPassFinished(RenderPassClosureReason::FramebufferBindingChange);
                // If we are switching from user FBO to system frame buffer, we always submit work
                // first so that these FBO rendering will not have to wait for ANI semaphore (which
                // draw to system frame buffer must wait for).
                if ((getFeatures().preferSubmitAtFBOBoundary.enabled ||
                     mState.getDrawFramebuffer()->isDefault()) &&
                    mRenderPassCommands->started())
                {
                    // This will behave as if user called glFlush, but the actual flush will be
                    // triggered at endRenderPass time.
                    mHasDeferredFlush = true;
                }

                mDepthStencilAttachmentFlags.reset();
                updateFlipViewportDrawFramebuffer(glState);
                updateSurfaceRotationDrawFramebuffer(glState, context->getCurrentDrawSurface());
                updateViewport(drawFramebufferVk, glState.getViewport(), glState.getNearPlane(),
                               glState.getFarPlane());
                updateColorMasks();
                updateMissingAttachments();
                updateRasterizationSamples(drawFramebufferVk->getSamples());
                updateRasterizerDiscardEnabled(
                    mState.isQueryActive(gl::QueryType::PrimitivesGenerated));

                updateFrontFace();
                updateScissor(glState);
                updateDepthStencil(glState);
                updateDither();

                // Clear the blend funcs/equations for color attachment indices that no longer
                // exist.
                gl::DrawBufferMask newColorAttachmentMask =
                    drawFramebufferVk->getState().getColorAttachmentsMask();
                mGraphicsPipelineDesc->resetBlendFuncsAndEquations(
                    &mGraphicsPipelineTransition, glState.getBlendStateExt(),
                    mCachedDrawFramebufferColorAttachmentMask, newColorAttachmentMask);
                mCachedDrawFramebufferColorAttachmentMask = newColorAttachmentMask;

                if (!getFeatures().preferDynamicRendering.enabled)
                {
                    // The framebuffer may not be in sync with usage of framebuffer fetch programs.
                    drawFramebufferVk->switchToColorFramebufferFetchMode(
                        this, mIsInColorFramebufferFetchMode);
                }

                onDrawFramebufferRenderPassDescChange(drawFramebufferVk, nullptr);

                break;
            }
            case gl::state::DIRTY_BIT_RENDERBUFFER_BINDING:
                break;
            case gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING:
            {
                invalidateDefaultAttributes(context->getStateCache().getActiveDefaultAttribsMask());
                ANGLE_TRY(vertexArrayVk->updateActiveAttribInfo(this));
                ANGLE_TRY(onIndexBufferChange(vertexArrayVk->getCurrentElementArrayBuffer()));
                break;
            }
            case gl::state::DIRTY_BIT_DRAW_INDIRECT_BUFFER_BINDING:
                break;
            case gl::state::DIRTY_BIT_DISPATCH_INDIRECT_BUFFER_BINDING:
                break;
            case gl::state::DIRTY_BIT_PROGRAM_BINDING:
                static_assert(
                    gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE > gl::state::DIRTY_BIT_PROGRAM_BINDING,
                    "Dirty bit order");
                iter.setLaterBit(gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE);
                break;
            case gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE:
            {
                ASSERT(programExecutable);
                invalidateCurrentDefaultUniforms();
                updateAdvancedBlendEquations(programExecutable);
                vk::GetImpl(programExecutable)->onProgramBind();
                static_assert(
                    gl::state::DIRTY_BIT_TEXTURE_BINDINGS > gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE,
                    "Dirty bit order");
                iter.setLaterBit(gl::state::DIRTY_BIT_TEXTURE_BINDINGS);
                ANGLE_TRY(invalidateCurrentShaderResources(command));
                invalidateDriverUniforms();
                ANGLE_TRY(invalidateProgramExecutableHelper(context));

                static_assert(
                    gl::state::DIRTY_BIT_SAMPLE_SHADING > gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE,
                    "Dirty bit order");
                if (getFeatures().explicitlyEnablePerSampleShading.enabled)
                {
                    iter.setLaterBit(gl::state::DIRTY_BIT_SAMPLE_SHADING);
                }

                break;
            }
            case gl::state::DIRTY_BIT_SAMPLER_BINDINGS:
            {
                static_assert(
                    gl::state::DIRTY_BIT_TEXTURE_BINDINGS > gl::state::DIRTY_BIT_SAMPLER_BINDINGS,
                    "Dirty bit order");
                iter.setLaterBit(gl::state::DIRTY_BIT_TEXTURE_BINDINGS);
                break;
            }
            case gl::state::DIRTY_BIT_TEXTURE_BINDINGS:
                ANGLE_TRY(invalidateCurrentTextures(context, command));
                break;
            case gl::state::DIRTY_BIT_TRANSFORM_FEEDBACK_BINDING:
                // Nothing to do.
                break;
            case gl::state::DIRTY_BIT_IMAGE_BINDINGS:
                static_assert(gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING >
                                  gl::state::DIRTY_BIT_IMAGE_BINDINGS,
                              "Dirty bit order");
                iter.setLaterBit(gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING);
                break;
            case gl::state::DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING:
                static_assert(gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING >
                                  gl::state::DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING,
                              "Dirty bit order");
                iter.setLaterBit(gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING);
                break;
            case gl::state::DIRTY_BIT_UNIFORM_BUFFER_BINDINGS:
                ANGLE_TRY(invalidateCurrentShaderUniformBuffers(command));
                break;
            case gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING:
                ANGLE_TRY(invalidateCurrentShaderResources(command));
                invalidateDriverUniforms();
                break;
            case gl::state::DIRTY_BIT_MULTISAMPLING:
                // When disabled, this should configure the pipeline to render as if single-sampled,
                // and write the results to all samples of a pixel regardless of coverage. See
                // EXT_multisample_compatibility.  This is not possible in Vulkan without some
                // gymnastics, so continue multisampled rendering anyway.
                // http://anglebug.com/42266123
                //
                // Potentially, the GLES1 renderer can switch rendering between two images and blit
                // from one to the other when the mode changes.  Then this extension wouldn't need
                // to be exposed.
                iter.setLaterBit(gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_ONE);
                break;
            case gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_ONE:
                // This is part of EXT_multisample_compatibility, and requires the alphaToOne Vulkan
                // feature.
                // http://anglebug.com/42266123
                mGraphicsPipelineDesc->updateAlphaToOneEnable(
                    &mGraphicsPipelineTransition,
                    glState.isMultisamplingEnabled() && glState.isSampleAlphaToOneEnabled());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_SHADING:
                updateSampleShadingWithRasterizationSamples(drawFramebufferVk->getSamples());
                break;
            case gl::state::DIRTY_BIT_COVERAGE_MODULATION:
                break;
            case gl::state::DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE:
                break;
            case gl::state::DIRTY_BIT_CURRENT_VALUES:
            {
                invalidateDefaultAttributes(glState.getAndResetDirtyCurrentValues());
                break;
            }
            case gl::state::DIRTY_BIT_PROVOKING_VERTEX:
                break;
            case gl::state::DIRTY_BIT_EXTENDED:
            {
                for (auto extendedIter    = extendedDirtyBits.begin(),
                          extendedEndIter = extendedDirtyBits.end();
                     extendedIter != extendedEndIter; ++extendedIter)
                {
                    const size_t extendedDirtyBit = *extendedIter;
                    switch (extendedDirtyBit)
                    {
                        case gl::state::EXTENDED_DIRTY_BIT_CLIP_CONTROL:
                            updateViewport(vk::GetImpl(glState.getDrawFramebuffer()),
                                           glState.getViewport(), glState.getNearPlane(),
                                           glState.getFarPlane());
                            // Since we are flipping the y coordinate, update front face state
                            updateFrontFace();
                            updateScissor(glState);

                            // If VK_EXT_depth_clip_control is not enabled, there's nothing needed
                            // for depth correction for EXT_clip_control.
                            // glState will be used to toggle control path of depth correction code
                            // in SPIR-V transform options.
                            if (getFeatures().supportsDepthClipControl.enabled)
                            {
                                mGraphicsPipelineDesc->updateDepthClipControl(
                                    &mGraphicsPipelineTransition,
                                    !glState.isClipDepthModeZeroToOne());
                            }
                            else
                            {
                                invalidateGraphicsDriverUniforms();
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_CLIP_DISTANCES:
                            invalidateGraphicsDriverUniforms();
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_DEPTH_CLAMP_ENABLED:
                            // TODO(https://anglebug.com/42266182): Use EDS3
                            mGraphicsPipelineDesc->updateDepthClampEnabled(
                                &mGraphicsPipelineTransition, glState.isDepthClampEnabled());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_MIPMAP_GENERATION_HINT:
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_MODE:
                            // TODO(https://anglebug.com/42266182): Use EDS3
                            mGraphicsPipelineDesc->updatePolygonMode(&mGraphicsPipelineTransition,
                                                                     glState.getPolygonMode());
                            // When polygon mode is changed, depth bias might need to be toggled.
                            static_assert(
                                gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED >
                                    gl::state::EXTENDED_DIRTY_BIT_POLYGON_MODE,
                                "Dirty bit order");
                            extendedIter.setLaterBit(
                                gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED);
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_POINT_ENABLED:
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED:
                            if (mRenderer->getFeatures().useDepthBiasEnableDynamicState.enabled)
                            {
                                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_DEPTH_BIAS_ENABLE);
                            }
                            else
                            {
                                mGraphicsPipelineDesc->updatePolygonOffsetEnabled(
                                    &mGraphicsPipelineTransition, glState.isPolygonOffsetEnabled());
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_SHADER_DERIVATIVE_HINT:
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_LOGIC_OP_ENABLED:
                            mGraphicsPipelineDesc->updateLogicOpEnabled(
                                &mGraphicsPipelineTransition, glState.isLogicOpEnabled());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_LOGIC_OP:
                            if (mRenderer->getFeatures().supportsLogicOpDynamicState.enabled)
                            {
                                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_LOGIC_OP);
                            }
                            else
                            {
                                mGraphicsPipelineDesc->updateLogicOp(
                                    &mGraphicsPipelineTransition,
                                    gl_vk::GetLogicOp(gl::ToGLenum(glState.getLogicOp())));
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_SHADING_RATE:
                            if (getFeatures().supportsFragmentShadingRate.enabled)
                            {
                                mGraphicsDirtyBits.set(DIRTY_BIT_DYNAMIC_FRAGMENT_SHADING_RATE);
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_BLEND_ADVANCED_COHERENT:
                            break;
                        default:
                            UNREACHABLE();
                    }
                }
                break;
            }
            case gl::state::DIRTY_BIT_PATCH_VERTICES:
                mGraphicsPipelineDesc->updatePatchVertices(&mGraphicsPipelineTransition,
                                                           glState.getPatchVertices());
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    return angle::Result::Continue;
}

GLint ContextVk::getGPUDisjoint()
{
    // No extension seems to be available to query this information.
    return 0;
}

GLint64 ContextVk::getTimestamp()
{
    // This function should only be called if timestamp queries are available.
    ASSERT(mRenderer->getQueueFamilyProperties().timestampValidBits > 0);

    uint64_t timestamp = 0;

    (void)getTimestamp(&timestamp);

    return static_cast<GLint64>(timestamp);
}

angle::Result ContextVk::onMakeCurrent(const gl::Context *context)
{
    mRenderer->reloadVolkIfNeeded();

    if (mCurrentQueueSerialIndex == kInvalidQueueSerialIndex)
    {
        ANGLE_TRY(allocateQueueSerialIndex());
    }

    // Flip viewports if the user did not request that the surface is flipped.
    const egl::Surface *drawSurface = context->getCurrentDrawSurface();
    const egl::Surface *readSurface = context->getCurrentReadSurface();
    mFlipYForCurrentSurface =
        drawSurface != nullptr &&
        !IsMaskFlagSet(drawSurface->getOrientation(), EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE);

    if (drawSurface && drawSurface->getType() == EGL_WINDOW_BIT)
    {
        mCurrentWindowSurface = GetImplAs<WindowSurfaceVk>(drawSurface);
    }
    else
    {
        mCurrentWindowSurface = nullptr;
    }

    const gl::State &glState = context->getState();
    updateFlipViewportDrawFramebuffer(glState);
    updateFlipViewportReadFramebuffer(glState);
    updateSurfaceRotationDrawFramebuffer(glState, drawSurface);
    updateSurfaceRotationReadFramebuffer(glState, readSurface);

    invalidateDriverUniforms();

    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    if (executable && executable->hasTransformFeedbackOutput() &&
        mState.isTransformFeedbackActive())
    {
        onTransformFeedbackStateChanged();
        if (getFeatures().supportsTransformFeedbackExtension.enabled)
        {
            mGraphicsDirtyBits.set(DIRTY_BIT_TRANSFORM_FEEDBACK_RESUME);
        }
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::onUnMakeCurrent(const gl::Context *context)
{
    ANGLE_TRY(flushAndSubmitCommands(nullptr, nullptr, RenderPassClosureReason::ContextChange));
    mCurrentWindowSurface = nullptr;

    if (mCurrentQueueSerialIndex != kInvalidQueueSerialIndex)
    {
        releaseQueueSerialIndex();
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::onSurfaceUnMakeCurrent(WindowSurfaceVk *surface)
{
    // It is possible to destroy "WindowSurfaceVk" while not all rendering commands are submitted:
    // 1. Make "WindowSurfaceVk" current.
    // 2. Draw something.
    // 3. Make other Surface current (same Context).
    // 4. (optional) Draw something.
    // 5. Delete "WindowSurfaceVk".
    // 6. UnMake the Context from current.
    // Flush all command to the GPU while still having access to the Context.

    // The above "onUnMakeCurrent()" may have already been called.
    if (mCurrentQueueSerialIndex != kInvalidQueueSerialIndex)
    {
        // May be nullptr if only used as a readSurface.
        ASSERT(mCurrentWindowSurface == surface || mCurrentWindowSurface == nullptr);
        ANGLE_TRY(flushAndSubmitCommands(nullptr, nullptr,
                                         RenderPassClosureReason::SurfaceUnMakeCurrent));
        mCurrentWindowSurface = nullptr;
    }
    ASSERT(mCurrentWindowSurface == nullptr);

    // Everything must be flushed and submitted.
    ASSERT(mOutsideRenderPassCommands->empty());
    ASSERT(!mRenderPassCommands->started());
    ASSERT(mWaitSemaphores.empty());
    ASSERT(!mHasWaitSemaphoresPendingSubmission);
    ASSERT(mLastSubmittedQueueSerial == mLastFlushedQueueSerial);
    return angle::Result::Continue;
}

angle::Result ContextVk::onSurfaceUnMakeCurrent(OffscreenSurfaceVk *surface)
{
    // It is possible to destroy "OffscreenSurfaceVk" while RenderPass is still opened:
    // 1. Make "OffscreenSurfaceVk" current.
    // 2. Draw something with RenderPass.
    // 3. Make other Surface current (same Context)
    // 4. Delete "OffscreenSurfaceVk".
    // 5. UnMake the Context from current.
    // End RenderPass to avoid crash in the "RenderPassCommandBufferHelper::endRenderPass()".
    // Flush commands unconditionally even if surface is not used in the RenderPass to fix possible
    // problems related to other accesses. "flushAndSubmitCommands()" is not required because
    // "OffscreenSurfaceVk" uses GC.

    // The above "onUnMakeCurrent()" may have already been called.
    if (mCurrentQueueSerialIndex != kInvalidQueueSerialIndex)
    {
        ANGLE_TRY(flushCommandsAndEndRenderPass(RenderPassClosureReason::SurfaceUnMakeCurrent));
    }

    // Everything must be flushed but may be pending submission.
    ASSERT(mOutsideRenderPassCommands->empty());
    ASSERT(!mRenderPassCommands->started());
    ASSERT(mWaitSemaphores.empty());
    return angle::Result::Continue;
}

void ContextVk::updateFlipViewportDrawFramebuffer(const gl::State &glState)
{
    // The default framebuffer (originating from the swapchain) is rendered upside-down due to the
    // difference in the coordinate systems of Vulkan and GLES.  Rendering upside-down has the
    // effect that rendering is done the same way as OpenGL.  The KHR_MAINTENANCE_1 extension is
    // subsequently enabled to allow negative viewports.  We inverse rendering to the backbuffer by
    // reversing the height of the viewport and increasing Y by the height.  So if the viewport was
    // (0, 0, width, height), it becomes (0, height, width, -height).  Unfortunately, when we start
    // doing this, we also need to adjust a number of places since the rendering now happens
    // upside-down.  Affected places so far:
    //
    // - readPixels
    // - copyTexImage
    // - framebuffer blit
    // - generating mipmaps
    // - Point sprites tests
    // - texStorage
    gl::Framebuffer *drawFramebuffer = glState.getDrawFramebuffer();
    mFlipViewportForDrawFramebuffer  = drawFramebuffer->isDefault();
}

void ContextVk::updateFlipViewportReadFramebuffer(const gl::State &glState)
{
    gl::Framebuffer *readFramebuffer = glState.getReadFramebuffer();
    mFlipViewportForReadFramebuffer  = readFramebuffer->isDefault();
}

void ContextVk::updateSurfaceRotationDrawFramebuffer(const gl::State &glState,
                                                     const egl::Surface *currentDrawSurface)
{
    const SurfaceRotation rotation =
        getSurfaceRotationImpl(glState.getDrawFramebuffer(), currentDrawSurface);
    mCurrentRotationDrawFramebuffer = rotation;

    if (!getFeatures().preferDriverUniformOverSpecConst.enabled)
    {
        const bool isRotatedAspectRatio = IsRotatedAspectRatio(rotation);
        // Update spec consts
        if (isRotatedAspectRatio != mGraphicsPipelineDesc->getSurfaceRotation())
        {
            // surface rotation are specialization constants, which affects program compilation.
            // When rotation changes, we need to update GraphicsPipelineDesc so that the correct
            // pipeline program object will be retrieved.
            mGraphicsPipelineDesc->updateSurfaceRotation(&mGraphicsPipelineTransition,
                                                         isRotatedAspectRatio);
            invalidateCurrentGraphicsPipeline();
        }
    }
}

void ContextVk::updateSurfaceRotationReadFramebuffer(const gl::State &glState,
                                                     const egl::Surface *currentReadSurface)
{
    mCurrentRotationReadFramebuffer =
        getSurfaceRotationImpl(glState.getReadFramebuffer(), currentReadSurface);
}

gl::Caps ContextVk::getNativeCaps() const
{
    return mRenderer->getNativeCaps();
}

const gl::TextureCapsMap &ContextVk::getNativeTextureCaps() const
{
    return mRenderer->getNativeTextureCaps();
}

const gl::Extensions &ContextVk::getNativeExtensions() const
{
    return mRenderer->getNativeExtensions();
}

const gl::Limitations &ContextVk::getNativeLimitations() const
{
    return mRenderer->getNativeLimitations();
}

const ShPixelLocalStorageOptions &ContextVk::getNativePixelLocalStorageOptions() const
{
    return mRenderer->getNativePixelLocalStorageOptions();
}

CompilerImpl *ContextVk::createCompiler()
{
    return new CompilerVk();
}

ShaderImpl *ContextVk::createShader(const gl::ShaderState &state)
{
    return new ShaderVk(state);
}

ProgramImpl *ContextVk::createProgram(const gl::ProgramState &state)
{
    return new ProgramVk(state);
}

ProgramExecutableImpl *ContextVk::createProgramExecutable(const gl::ProgramExecutable *executable)
{
    return new ProgramExecutableVk(executable);
}

FramebufferImpl *ContextVk::createFramebuffer(const gl::FramebufferState &state)
{
    return new FramebufferVk(mRenderer, state);
}

TextureImpl *ContextVk::createTexture(const gl::TextureState &state)
{
    return new TextureVk(state, mRenderer);
}

RenderbufferImpl *ContextVk::createRenderbuffer(const gl::RenderbufferState &state)
{
    return new RenderbufferVk(state);
}

BufferImpl *ContextVk::createBuffer(const gl::BufferState &state)
{
    return new BufferVk(state);
}

VertexArrayImpl *ContextVk::createVertexArray(const gl::VertexArrayState &state)
{
    return new VertexArrayVk(this, state);
}

QueryImpl *ContextVk::createQuery(gl::QueryType type)
{
    return new QueryVk(type);
}

FenceNVImpl *ContextVk::createFenceNV()
{
    return new FenceNVVk();
}

SyncImpl *ContextVk::createSync()
{
    return new SyncVk();
}

TransformFeedbackImpl *ContextVk::createTransformFeedback(const gl::TransformFeedbackState &state)
{
    return new TransformFeedbackVk(state);
}

SamplerImpl *ContextVk::createSampler(const gl::SamplerState &state)
{
    return new SamplerVk(state);
}

ProgramPipelineImpl *ContextVk::createProgramPipeline(const gl::ProgramPipelineState &state)
{
    return new ProgramPipelineVk(state);
}

MemoryObjectImpl *ContextVk::createMemoryObject()
{
    return new MemoryObjectVk();
}

SemaphoreImpl *ContextVk::createSemaphore()
{
    return new SemaphoreVk();
}

OverlayImpl *ContextVk::createOverlay(const gl::OverlayState &state)
{
    return new OverlayVk(state);
}

void ContextVk::invalidateCurrentDefaultUniforms()
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    if (executable->hasDefaultUniforms())
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DESCRIPTOR_SETS);
        mComputeDirtyBits.set(DIRTY_BIT_DESCRIPTOR_SETS);
    }
}

angle::Result ContextVk::invalidateCurrentTextures(const gl::Context *context, gl::Command command)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    if (executable->hasTextures())
    {
        mGraphicsDirtyBits |= kTexturesAndDescSetDirtyBits;
        mComputeDirtyBits |= kTexturesAndDescSetDirtyBits;

        ANGLE_TRY(updateActiveTextures(context, command));

        if (command == gl::Command::Dispatch)
        {
            ANGLE_TRY(endRenderPassIfComputeAccessAfterGraphicsImageAccess());
        }
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::invalidateCurrentShaderResources(gl::Command command)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    const bool hasImages = executable->hasImages();
    const bool hasStorageBuffers =
        executable->hasStorageBuffers() || executable->hasAtomicCounterBuffers();
    const bool hasUniformBuffers = executable->hasUniformBuffers();

    if (hasUniformBuffers || hasStorageBuffers || hasImages ||
        executable->usesColorFramebufferFetch() || executable->usesDepthFramebufferFetch() ||
        executable->usesStencilFramebufferFetch())
    {
        mGraphicsDirtyBits |= kResourcesAndDescSetDirtyBits;
        mComputeDirtyBits |= kResourcesAndDescSetDirtyBits;
    }

    // Take care of read-after-write hazards that require implicit synchronization.
    if (hasUniformBuffers && command == gl::Command::Dispatch)
    {
        ANGLE_TRY(endRenderPassIfComputeReadAfterTransformFeedbackWrite());
    }

    // Take care of implicit layout transition by compute program access-after-read.
    if (hasImages && command == gl::Command::Dispatch)
    {
        ANGLE_TRY(endRenderPassIfComputeAccessAfterGraphicsImageAccess());
    }

    // If memory barrier has been issued but the command buffers haven't been flushed, make sure
    // they get a chance to do so if necessary on program and storage buffer/image binding change.
    const bool hasGLMemoryBarrierIssuedInCommandBuffers =
        mOutsideRenderPassCommands->hasGLMemoryBarrierIssued() ||
        mRenderPassCommands->hasGLMemoryBarrierIssued();

    if ((hasStorageBuffers || hasImages) && hasGLMemoryBarrierIssuedInCommandBuffers)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_MEMORY_BARRIER);
        mComputeDirtyBits.set(DIRTY_BIT_MEMORY_BARRIER);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::invalidateCurrentShaderUniformBuffers(gl::Command command)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable);

    if (executable->hasUniformBuffers())
    {
        if (executable->hasLinkedShaderStage(gl::ShaderType::Compute))
        {
            mComputeDirtyBits |= kUniformBuffersAndDescSetDirtyBits;
        }
        else
        {
            mGraphicsDirtyBits |= kUniformBuffersAndDescSetDirtyBits;
        }

        if (command == gl::Command::Dispatch)
        {
            // Take care of read-after-write hazards that require implicit synchronization.
            ANGLE_TRY(endRenderPassIfComputeReadAfterTransformFeedbackWrite());
        }
    }
    return angle::Result::Continue;
}

void ContextVk::updateShaderResourcesWithSharedCacheKey(
    const vk::SharedDescriptorSetCacheKey &sharedCacheKey)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ProgramExecutableVk *executableVk       = vk::GetImpl(executable);

    if (executable->hasUniformBuffers())
    {
        const std::vector<gl::InterfaceBlock> &blocks = executable->getUniformBlocks();
        for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
        {
            const GLuint binding = executable->getUniformBlockBinding(bufferIndex);
            UpdateBufferWithSharedCacheKey(mState.getOffsetBindingPointerUniformBuffers()[binding],
                                           executableVk->getUniformBufferDescriptorType(),
                                           sharedCacheKey);
        }
    }
    if (executable->hasStorageBuffers())
    {
        const std::vector<gl::InterfaceBlock> &blocks = executable->getShaderStorageBlocks();
        for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
        {
            const GLuint binding = executable->getShaderStorageBlockBinding(bufferIndex);
            UpdateBufferWithSharedCacheKey(
                mState.getOffsetBindingPointerShaderStorageBuffers()[binding],
                executableVk->getStorageBufferDescriptorType(), sharedCacheKey);
        }
    }
    if (executable->hasAtomicCounterBuffers())
    {
        const std::vector<gl::AtomicCounterBuffer> &blocks = executable->getAtomicCounterBuffers();
        for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
        {
            const GLuint binding = executable->getAtomicCounterBufferBinding(bufferIndex);
            UpdateBufferWithSharedCacheKey(
                mState.getOffsetBindingPointerAtomicCounterBuffers()[binding],
                executableVk->getAtomicCounterBufferDescriptorType(), sharedCacheKey);
        }
    }
    if (executable->hasImages())
    {
        UpdateImagesWithSharedCacheKey(mActiveImages, executable->getImageBindings(),
                                       sharedCacheKey);
    }
}

void ContextVk::invalidateGraphicsDriverUniforms()
{
    mGraphicsDirtyBits.set(DIRTY_BIT_DRIVER_UNIFORMS);
}

void ContextVk::invalidateDriverUniforms()
{
    mGraphicsDirtyBits.set(DIRTY_BIT_DRIVER_UNIFORMS);
    mComputeDirtyBits.set(DIRTY_BIT_DRIVER_UNIFORMS);
}

angle::Result ContextVk::onFramebufferChange(FramebufferVk *framebufferVk, gl::Command command)
{
    // This is called from FramebufferVk::syncState.  Skip these updates if the framebuffer being
    // synced is the read framebuffer (which is not equal the draw framebuffer).
    if (framebufferVk != vk::GetImpl(mState.getDrawFramebuffer()))
    {
        return angle::Result::Continue;
    }

    // Always consider the render pass finished.  FramebufferVk::syncState (caller of this function)
    // normally closes the render pass, except for blit to allow an optimization.  The following
    // code nevertheless must treat the render pass closed.
    onRenderPassFinished(RenderPassClosureReason::FramebufferChange);

    // Ensure that the pipeline description is updated.
    if (mGraphicsPipelineDesc->getRasterizationSamples() !=
        static_cast<uint32_t>(framebufferVk->getSamples()))
    {
        updateRasterizationSamples(framebufferVk->getSamples());
    }

    // Update scissor.
    updateScissor(mState);

    // Update depth and stencil.
    updateDepthStencil(mState);

    // Update dither based on attachment formats.
    updateDither();

    // Attachments might have changed.
    updateMissingAttachments();

    if (mState.getProgramExecutable())
    {
        ANGLE_TRY(invalidateCurrentShaderResources(command));
    }

    onDrawFramebufferRenderPassDescChange(framebufferVk, nullptr);
    return angle::Result::Continue;
}

void ContextVk::onDrawFramebufferRenderPassDescChange(FramebufferVk *framebufferVk,
                                                      bool *renderPassDescChangedOut)
{
    ASSERT(getFeatures().supportsFragmentShadingRate.enabled ||
           !framebufferVk->isFoveationEnabled());

    const vk::FramebufferFetchMode framebufferFetchMode =
        vk::GetProgramFramebufferFetchMode(mState.getProgramExecutable());
    mGraphicsPipelineDesc->updateRenderPassDesc(&mGraphicsPipelineTransition, getFeatures(),
                                                framebufferVk->getRenderPassDesc(),
                                                framebufferFetchMode);

    if (renderPassDescChangedOut)
    {
        // If render pass desc has changed while processing the dirty bits, notify the caller.
        // In most paths, |renderPassDescChangedOut| is nullptr and the pipeline will be
        // invalidated.
        //
        // |renderPassDescChangedOut| only serves |ContextVk::handleDirtyGraphicsRenderPass|, which
        // may need to reprocess the pipeline while processing dirty bits.  At that point, marking
        // the pipeline dirty is ineffective, and the pipeline dirty bit handler is directly called
        // as a result of setting this variable to true.
        *renderPassDescChangedOut = true;
    }
    else
    {
        // Otherwise mark the pipeline as dirty.
        invalidateCurrentGraphicsPipeline();
    }

    // Update render area in the driver uniforms.
    invalidateGraphicsDriverUniforms();
}

void ContextVk::invalidateCurrentTransformFeedbackBuffers()
{
    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_TRANSFORM_FEEDBACK_BUFFERS);
    }
    else if (getFeatures().emulateTransformFeedback.enabled)
    {
        mGraphicsDirtyBits |= kXfbBuffersAndDescSetDirtyBits;
    }
}

void ContextVk::onTransformFeedbackStateChanged()
{
    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_TRANSFORM_FEEDBACK_BUFFERS);
    }
    else if (getFeatures().emulateTransformFeedback.enabled)
    {
        invalidateGraphicsDriverUniforms();
        invalidateCurrentTransformFeedbackBuffers();

        // Invalidate the graphics pipeline too.  On transform feedback state change, the current
        // program may be used again, and it should switch between outputting transform feedback and
        // not.
        invalidateCurrentGraphicsPipeline();
        resetCurrentGraphicsPipeline();
    }
}

angle::Result ContextVk::onBeginTransformFeedback(
    size_t bufferCount,
    const gl::TransformFeedbackBuffersArray<vk::BufferHelper *> &buffers,
    const gl::TransformFeedbackBuffersArray<vk::BufferHelper> &counterBuffers)
{
    onTransformFeedbackStateChanged();

    bool shouldEndRenderPass = false;

    if (hasActiveRenderPass())
    {
        // If any of the buffers were previously used in the render pass, break the render pass as a
        // barrier is needed.
        for (size_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
        {
            const vk::BufferHelper *buffer = buffers[bufferIndex];
            if (mRenderPassCommands->usesBuffer(*buffer))
            {
                shouldEndRenderPass = true;
                break;
            }
        }
    }

    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        // Break the render pass if the counter buffers are used too.  Note that Vulkan requires a
        // barrier on the counter buffer between pause and resume, so it cannot be resumed in the
        // same render pass.  Note additionally that we don't need to test all counters being used
        // in the render pass, as outside of the transform feedback object these buffers are
        // inaccessible and are therefore always used together.
        if (!shouldEndRenderPass && isRenderPassStartedAndUsesBuffer(counterBuffers[0]))
        {
            shouldEndRenderPass = true;
        }

        mGraphicsDirtyBits.set(DIRTY_BIT_TRANSFORM_FEEDBACK_RESUME);
    }

    if (shouldEndRenderPass)
    {
        ANGLE_TRY(flushCommandsAndEndRenderPass(RenderPassClosureReason::BufferUseThenXfbWrite));
    }

    return angle::Result::Continue;
}

void ContextVk::onEndTransformFeedback()
{
    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        if (mRenderPassCommands->isTransformFeedbackStarted())
        {
            mRenderPassCommands->endTransformFeedback();
        }
    }
    else if (getFeatures().emulateTransformFeedback.enabled)
    {
        onTransformFeedbackStateChanged();
    }
}

angle::Result ContextVk::onPauseTransformFeedback()
{
    if (getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        // If transform feedback was already active on this render pass, break it.  This
        // is for simplicity to avoid tracking multiple simultaneously active transform feedback
        // settings in the render pass.
        if (mRenderPassCommands->isTransformFeedbackActiveUnpaused())
        {
            return flushCommandsAndEndRenderPass(RenderPassClosureReason::XfbPause);
        }
    }
    onTransformFeedbackStateChanged();
    return angle::Result::Continue;
}

void ContextVk::invalidateGraphicsPipelineBinding()
{
    mGraphicsDirtyBits.set(DIRTY_BIT_PIPELINE_BINDING);
}

void ContextVk::invalidateComputePipelineBinding()
{
    mComputeDirtyBits.set(DIRTY_BIT_PIPELINE_BINDING);
}

void ContextVk::invalidateGraphicsDescriptorSet(DescriptorSetIndex usedDescriptorSet)
{
    // UtilsVk currently only uses set 0
    ASSERT(usedDescriptorSet == DescriptorSetIndex::Internal);
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();

    if (executable && executable->hasUniformBuffers())
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_DESCRIPTOR_SETS);
        return;
    }
}

void ContextVk::invalidateComputeDescriptorSet(DescriptorSetIndex usedDescriptorSet)
{
    // UtilsVk currently only uses set 0
    ASSERT(usedDescriptorSet == DescriptorSetIndex::Internal);
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();

    if (executable && executable->hasUniformBuffers())
    {
        mComputeDirtyBits.set(DIRTY_BIT_DESCRIPTOR_SETS);
        return;
    }
}

void ContextVk::invalidateAllDynamicState()
{
    mGraphicsDirtyBits |= mDynamicStateDirtyBits;
}

angle::Result ContextVk::dispatchCompute(const gl::Context *context,
                                         GLuint numGroupsX,
                                         GLuint numGroupsY,
                                         GLuint numGroupsZ)
{
    ANGLE_TRY(setupDispatch(context));

    mOutsideRenderPassCommands->getCommandBuffer().dispatch(numGroupsX, numGroupsY, numGroupsZ);
    // Track completion of compute.
    mOutsideRenderPassCommands->flushSetEvents(this);

    return angle::Result::Continue;
}

angle::Result ContextVk::dispatchComputeIndirect(const gl::Context *context, GLintptr indirect)
{
    gl::Buffer *glBuffer     = getState().getTargetBuffer(gl::BufferBinding::DispatchIndirect);
    vk::BufferHelper &buffer = vk::GetImpl(glBuffer)->getBuffer();

    // Break the render pass if the indirect buffer was previously used as the output from transform
    // feedback.
    if (mCurrentTransformFeedbackQueueSerial.valid() &&
        buffer.writtenByCommandBuffer(mCurrentTransformFeedbackQueueSerial))
    {
        ANGLE_TRY(flushCommandsAndEndRenderPass(
            RenderPassClosureReason::XfbWriteThenIndirectDispatchBuffer));
    }

    ANGLE_TRY(setupDispatch(context));

    // Process indirect buffer after command buffer has started.
    mOutsideRenderPassCommands->bufferRead(this, VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                           vk::PipelineStage::DrawIndirect, &buffer);

    mOutsideRenderPassCommands->getCommandBuffer().dispatchIndirect(buffer.getBuffer(),
                                                                    buffer.getOffset() + indirect);

    // Track completion of compute.
    mOutsideRenderPassCommands->flushSetEvents(this);

    return angle::Result::Continue;
}

angle::Result ContextVk::memoryBarrier(const gl::Context *context, GLbitfield barriers)
{
    // First, turn GL_ALL_BARRIER_BITS into a mask that has only the valid barriers set.
    constexpr GLbitfield kAllMemoryBarrierBits = kBufferMemoryBarrierBits | kImageMemoryBarrierBits;
    barriers &= kAllMemoryBarrierBits;

    // GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT specifies that a fence sync or glFinish must be used
    // after the barrier for the CPU to to see the shader writes.  Since host-visible buffer writes
    // always issue a barrier automatically for the sake of glMapBuffer() (see
    // comment on |mIsAnyHostVisibleBufferWritten|), there's nothing to do for
    // GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT.
    barriers &= ~GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT;

    // If no other barrier, early out.
    if (barriers == 0)
    {
        return angle::Result::Continue;
    }

    // glMemoryBarrier for barrier bit X_BARRIER_BIT implies:
    //
    // - An execution+memory barrier: shader writes are made visible to subsequent X accesses
    //
    // Additionally, SHADER_IMAGE_ACCESS_BARRIER_BIT and SHADER_STORAGE_BARRIER_BIT imply:
    //
    // - An execution+memory barrier: all accesses are finished before image/buffer writes
    //
    // For the first barrier, we can simplify the implementation by assuming that prior writes are
    // expected to be used right after this barrier, so we can close the render pass or flush the
    // outside render pass commands right away if they have had any writes.
    //
    // It's noteworthy that some barrier bits affect draw/dispatch calls only, while others affect
    // other commands.  For the latter, since storage buffer and images are not tracked in command
    // buffers, we can't rely on the command buffers being flushed in the usual way when recording
    // these commands (i.e. through |getOutsideRenderPassCommandBuffer()| and
    // |vk::CommandBufferAccess|).  Conservatively flushing command buffers with any storage output
    // simplifies this use case.  If this needs to be avoided in the future,
    // |getOutsideRenderPassCommandBuffer()| can be modified to flush the command buffers if they
    // have had any storage output.
    //
    // For the second barrier, we need to defer closing the render pass until there's a draw or
    // dispatch call that uses storage buffers or images that were previously used in the render
    // pass.  This allows the render pass to remain open in scenarios such as this:
    //
    // - Draw using resource X
    // - glMemoryBarrier
    // - Draw/dispatch with storage buffer/image Y
    //
    // To achieve this, a dirty bit is added that breaks the render pass if any storage
    // buffer/images are used in it.  Until the render pass breaks, changing the program or storage
    // buffer/image bindings should set this dirty bit again.

    if (mRenderPassCommands->hasShaderStorageOutput())
    {
        // Break the render pass if necessary as future non-draw commands can't know if they should.
        ANGLE_TRY(flushCommandsAndEndRenderPass(
            RenderPassClosureReason::StorageResourceUseThenGLMemoryBarrier));
    }
    else if (mOutsideRenderPassCommands->hasShaderStorageOutput())
    {
        // Otherwise flush the outside render pass commands if necessary.
        ANGLE_TRY(flushOutsideRenderPassCommands());
    }

    if ((barriers & kWriteAfterAccessMemoryBarriers) == 0)
    {
        return angle::Result::Continue;
    }

    // Accumulate unprocessed memoryBarrier bits
    mDeferredMemoryBarriers |= barriers;

    // Defer flushing the command buffers until a draw/dispatch with storage buffer/image is
    // encountered.
    mGraphicsDirtyBits.set(DIRTY_BIT_MEMORY_BARRIER);
    mComputeDirtyBits.set(DIRTY_BIT_MEMORY_BARRIER);

    // Make sure memory barrier is issued for future usages of storage buffers and images even if
    // there's no binding change.
    mGraphicsDirtyBits.set(DIRTY_BIT_SHADER_RESOURCES);
    mComputeDirtyBits.set(DIRTY_BIT_SHADER_RESOURCES);

    // Mark the command buffers as affected by glMemoryBarrier, so future program and storage
    // buffer/image binding changes can set DIRTY_BIT_MEMORY_BARRIER again.
    mOutsideRenderPassCommands->setGLMemoryBarrierIssued();
    mRenderPassCommands->setGLMemoryBarrierIssued();

    return angle::Result::Continue;
}

angle::Result ContextVk::memoryBarrierByRegion(const gl::Context *context, GLbitfield barriers)
{
    // Note: memoryBarrierByRegion is expected to affect only the fragment pipeline, but is
    // otherwise similar to memoryBarrier in function.
    //
    // TODO: Optimize memoryBarrierByRegion by issuing an in-subpass pipeline barrier instead of
    // breaking the render pass.  http://anglebug.com/42263695
    return memoryBarrier(context, barriers);
}

void ContextVk::framebufferFetchBarrier()
{
    // No need for a barrier with VK_EXT_rasterization_order_attachment_access.
    if (getFeatures().supportsRasterizationOrderAttachmentAccess.enabled)
    {
        return;
    }

    mGraphicsDirtyBits.set(DIRTY_BIT_FRAMEBUFFER_FETCH_BARRIER);
}

void ContextVk::blendBarrier()
{
    if (getFeatures().emulateAdvancedBlendEquations.enabled)
    {
        // When emulated, advanced blend is implemented through framebuffer fetch.
        framebufferFetchBarrier();
    }
    else
    {
        mGraphicsDirtyBits.set(DIRTY_BIT_BLEND_BARRIER);
    }
}

angle::Result ContextVk::acquireTextures(const gl::Context *context,
                                         const gl::TextureBarrierVector &textureBarriers)
{
    for (const gl::TextureAndLayout &textureBarrier : textureBarriers)
    {
        TextureVk *textureVk   = vk::GetImpl(textureBarrier.texture);
        vk::ImageHelper &image = textureVk->getImage();
        vk::ImageLayout layout = vk::GetImageLayoutFromGLImageLayout(this, textureBarrier.layout);
        // Image should not be accessed while unowned. Emulated formats may have staged updates
        // to clear the image after initialization.
        ASSERT(!image.hasStagedUpdatesInAllocatedLevels() || image.hasEmulatedImageChannels());
        image.setCurrentImageLayout(getRenderer(), layout);
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::releaseTextures(const gl::Context *context,
                                         gl::TextureBarrierVector *textureBarriers)
{
    for (gl::TextureAndLayout &textureBarrier : *textureBarriers)
    {
        TextureVk *textureVk = vk::GetImpl(textureBarrier.texture);

        ANGLE_TRY(textureVk->ensureImageInitialized(this, ImageMipLevels::EnabledLevels));

        vk::ImageHelper &image = textureVk->getImage();
        ANGLE_TRY(onImageReleaseToExternal(image));

        textureBarrier.layout =
            vk::ConvertImageLayoutToGLImageLayout(image.getCurrentImageLayout());
    }

    return flushAndSubmitCommands(nullptr, nullptr,
                                  RenderPassClosureReason::ImageUseThenReleaseToExternal);
}

vk::DynamicQueryPool *ContextVk::getQueryPool(gl::QueryType queryType)
{
    ASSERT(queryType == gl::QueryType::AnySamples ||
           queryType == gl::QueryType::AnySamplesConservative ||
           queryType == gl::QueryType::PrimitivesGenerated ||
           queryType == gl::QueryType::TransformFeedbackPrimitivesWritten ||
           queryType == gl::QueryType::Timestamp || queryType == gl::QueryType::TimeElapsed);

    // For PrimitivesGenerated queries:
    //
    // - If VK_EXT_primitives_generated_query is supported, use that.
    // - Otherwise, if pipelineStatisticsQuery is supported, use that,
    // - Otherwise, use the same pool as TransformFeedbackPrimitivesWritten and share the query as
    //   the Vulkan transform feedback query produces both results.  This option is non-conformant
    //   as the primitives generated query will not be functional without transform feedback.
    //
    if (queryType == gl::QueryType::PrimitivesGenerated &&
        !getFeatures().supportsPrimitivesGeneratedQuery.enabled &&
        !getFeatures().supportsPipelineStatisticsQuery.enabled)
    {
        queryType = gl::QueryType::TransformFeedbackPrimitivesWritten;
    }

    // Assert that timestamp extension is available if needed.
    ASSERT((queryType != gl::QueryType::Timestamp && queryType != gl::QueryType::TimeElapsed) ||
           mRenderer->getQueueFamilyProperties().timestampValidBits > 0);
    ASSERT(mQueryPools[queryType].isValid());
    return &mQueryPools[queryType];
}

const VkClearValue &ContextVk::getClearColorValue() const
{
    return mClearColorValue;
}

const VkClearValue &ContextVk::getClearDepthStencilValue() const
{
    return mClearDepthStencilValue;
}

gl::BlendStateExt::ColorMaskStorage::Type ContextVk::getClearColorMasks() const
{
    return mClearColorMasks;
}

void ContextVk::writeAtomicCounterBufferDriverUniformOffsets(uint32_t *offsetsOut,
                                                             size_t offsetsSize)
{
    const VkDeviceSize offsetAlignment =
        mRenderer->getPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
    size_t atomicCounterBufferCount = mState.getAtomicCounterBufferCount();

    ASSERT(atomicCounterBufferCount <= offsetsSize * 4);

    for (uint32_t bufferIndex = 0; bufferIndex < atomicCounterBufferCount; ++bufferIndex)
    {
        uint32_t offsetDiff = 0;

        const gl::OffsetBindingPointer<gl::Buffer> *atomicCounterBuffer =
            &mState.getIndexedAtomicCounterBuffer(bufferIndex);
        if (atomicCounterBuffer->get())
        {
            VkDeviceSize offset        = atomicCounterBuffer->getOffset();
            VkDeviceSize alignedOffset = (offset / offsetAlignment) * offsetAlignment;

            // GL requires the atomic counter buffer offset to be aligned with uint.
            ASSERT((offset - alignedOffset) % sizeof(uint32_t) == 0);
            offsetDiff = static_cast<uint32_t>((offset - alignedOffset) / sizeof(uint32_t));

            // We expect offsetDiff to fit in an 8-bit value.  The maximum difference is
            // minStorageBufferOffsetAlignment / 4, where minStorageBufferOffsetAlignment
            // currently has a maximum value of 256 on any device.
            ASSERT(offsetDiff < (1 << 8));
        }

        // The output array is already cleared prior to this call.
        ASSERT(bufferIndex % 4 != 0 || offsetsOut[bufferIndex / 4] == 0);

        offsetsOut[bufferIndex / 4] |= static_cast<uint8_t>(offsetDiff) << ((bufferIndex % 4) * 8);
    }
}

void ContextVk::pauseTransformFeedbackIfActiveUnpaused()
{
    if (mRenderPassCommands->isTransformFeedbackActiveUnpaused())
    {
        ASSERT(getFeatures().supportsTransformFeedbackExtension.enabled);
        mRenderPassCommands->pauseTransformFeedback();

        // Note that this function is called when render pass break is imminent
        // (flushCommandsAndEndRenderPass(), or UtilsVk::clearFramebuffer which will close the
        // render pass after the clear).  This dirty bit allows transform feedback to resume
        // automatically on next render pass.
        mGraphicsDirtyBits.set(DIRTY_BIT_TRANSFORM_FEEDBACK_RESUME);
    }
}

angle::Result ContextVk::handleDirtyGraphicsDriverUniforms(DirtyBits::Iterator *dirtyBitsIterator,
                                                           DirtyBits dirtyBitMask)
{
    FramebufferVk *drawFramebufferVk = getDrawFramebuffer();

    static_assert(gl::IMPLEMENTATION_MAX_FRAMEBUFFER_SIZE <= 0xFFFF,
                  "Not enough bits for render area");
    static_assert(gl::IMPLEMENTATION_MAX_RENDERBUFFER_SIZE <= 0xFFFF,
                  "Not enough bits for render area");
    uint16_t renderAreaWidth, renderAreaHeight;
    SetBitField(renderAreaWidth, drawFramebufferVk->getState().getDimensions().width);
    SetBitField(renderAreaHeight, drawFramebufferVk->getState().getDimensions().height);
    const uint32_t renderArea = renderAreaHeight << 16 | renderAreaWidth;

    bool flipX = false;
    bool flipY = false;
    // Y-axis flipping only comes into play with the default framebuffer (i.e. a swapchain
    // image). For 0-degree rotation, an FBO or pbuffer could be the draw framebuffer, and so we
    // must check whether flipY should be positive or negative.  All other rotations, will be to
    // the default framebuffer, and so the value of isViewportFlipEnabledForDrawFBO() is assumed
    // true; the appropriate flipY value is chosen such that gl_FragCoord is positioned at the
    // lower-left corner of the window.
    switch (mCurrentRotationDrawFramebuffer)
    {
        case SurfaceRotation::Identity:
            flipY = isViewportFlipEnabledForDrawFBO();
            break;
        case SurfaceRotation::Rotated90Degrees:
            ASSERT(isViewportFlipEnabledForDrawFBO());
            break;
        case SurfaceRotation::Rotated180Degrees:
            ASSERT(isViewportFlipEnabledForDrawFBO());
            flipX = true;
            break;
        case SurfaceRotation::Rotated270Degrees:
            ASSERT(isViewportFlipEnabledForDrawFBO());
            flipX = true;
            flipY = true;
            break;
        default:
            UNREACHABLE();
            break;
    }

    const bool invertViewport = isViewportFlipEnabledForDrawFBO();

    // Create the extended driver uniform, and populate the extended data fields if necessary.
    GraphicsDriverUniformsExtended driverUniformsExt = {};
    if (ShouldUseGraphicsDriverUniformsExtended(this))
    {
        if (mState.isTransformFeedbackActiveUnpaused())
        {
            TransformFeedbackVk *transformFeedbackVk =
                vk::GetImpl(mState.getCurrentTransformFeedback());
            transformFeedbackVk->getBufferOffsets(this, mXfbBaseVertex,
                                                  driverUniformsExt.xfbBufferOffsets.data(),
                                                  driverUniformsExt.xfbBufferOffsets.size());
        }
        driverUniformsExt.xfbVerticesPerInstance = static_cast<int32_t>(mXfbVertexCountPerInstance);
    }

    // Create the driver uniform object that will be used as push constant argument.
    GraphicsDriverUniforms *driverUniforms = &driverUniformsExt.common;
    uint32_t driverUniformSize             = GetDriverUniformSize(this, PipelineType::Graphics);

    const float depthRangeNear = mState.getNearPlane();
    const float depthRangeFar  = mState.getFarPlane();
    const uint32_t numSamples  = drawFramebufferVk->getSamples();
    const uint32_t isLayered   = drawFramebufferVk->getLayerCount() > 1;

    uint32_t advancedBlendEquation = 0;
    if (getFeatures().emulateAdvancedBlendEquations.enabled && mState.isBlendEnabled())
    {
        // Pass the advanced blend equation to shader as-is.  If the equation is not one of the
        // advanced ones, 0 is expected.
        const gl::BlendStateExt &blendStateExt = mState.getBlendStateExt();
        if (blendStateExt.getUsesAdvancedBlendEquationMask().test(0))
        {
            advancedBlendEquation =
                static_cast<uint32_t>(getState().getBlendStateExt().getEquationColorIndexed(0));
        }
    }

    const uint32_t swapXY               = IsRotatedAspectRatio(mCurrentRotationDrawFramebuffer);
    const uint32_t enabledClipDistances = mState.getEnabledClipDistances().bits();
    const uint32_t transformDepth =
        getFeatures().supportsDepthClipControl.enabled ? 0 : !mState.isClipDepthModeZeroToOne();

    static_assert(angle::BitMask<uint32_t>(gl::IMPLEMENTATION_MAX_CLIP_DISTANCES) <=
                      sh::vk::kDriverUniformsMiscEnabledClipPlanesMask,
                  "Not enough bits for enabled clip planes");

    ASSERT((swapXY & ~sh::vk::kDriverUniformsMiscSwapXYMask) == 0);
    ASSERT((advancedBlendEquation & ~sh::vk::kDriverUniformsMiscAdvancedBlendEquationMask) == 0);
    ASSERT((numSamples & ~sh::vk::kDriverUniformsMiscSampleCountMask) == 0);
    ASSERT((enabledClipDistances & ~sh::vk::kDriverUniformsMiscEnabledClipPlanesMask) == 0);
    ASSERT((transformDepth & ~sh::vk::kDriverUniformsMiscTransformDepthMask) == 0);

    const uint32_t misc =
        swapXY | advancedBlendEquation << sh::vk::kDriverUniformsMiscAdvancedBlendEquationOffset |
        numSamples << sh::vk::kDriverUniformsMiscSampleCountOffset |
        enabledClipDistances << sh::vk::kDriverUniformsMiscEnabledClipPlanesOffset |
        transformDepth << sh::vk::kDriverUniformsMiscTransformDepthOffset |
        isLayered << sh::vk::kDriverUniformsMiscLayeredFramebufferOffset;

    // Copy and flush to the device.
    *driverUniforms = {
        {},
        {depthRangeNear, depthRangeFar},
        renderArea,
        MakeFlipUniform(flipX, flipY, invertViewport),
        mGraphicsPipelineDesc->getEmulatedDitherControl(),
        misc,
    };

    if (mState.hasValidAtomicCounterBuffer())
    {
        writeAtomicCounterBufferDriverUniformOffsets(driverUniforms->acbBufferOffsets.data(),
                                                     driverUniforms->acbBufferOffsets.size());
    }

    // Update push constant driver uniforms.
    ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
    mRenderPassCommands->getCommandBuffer().pushConstants(
        executableVk->getPipelineLayout(), getRenderer()->getSupportedVulkanShaderStageMask(), 0,
        driverUniformSize, driverUniforms);

    return angle::Result::Continue;
}

angle::Result ContextVk::handleDirtyComputeDriverUniforms(DirtyBits::Iterator *dirtyBitsIterator)
{
    // Create the driver uniform object that will be used as push constant argument.
    ComputeDriverUniforms driverUniforms = {};
    uint32_t driverUniformSize           = GetDriverUniformSize(this, PipelineType::Compute);

    if (mState.hasValidAtomicCounterBuffer())
    {
        writeAtomicCounterBufferDriverUniformOffsets(driverUniforms.acbBufferOffsets.data(),
                                                     driverUniforms.acbBufferOffsets.size());
    }

    // Update push constant driver uniforms.
    ProgramExecutableVk *executableVk = vk::GetImpl(mState.getProgramExecutable());
    mOutsideRenderPassCommands->getCommandBuffer().pushConstants(
        executableVk->getPipelineLayout(), getRenderer()->getSupportedVulkanShaderStageMask(), 0,
        driverUniformSize, &driverUniforms);

    return angle::Result::Continue;
}

void ContextVk::handleError(VkResult errorCode,
                            const char *file,
                            const char *function,
                            unsigned int line)
{
    ASSERT(errorCode != VK_SUCCESS);

    GLenum glErrorCode = DefaultGLErrorCode(errorCode);

    std::stringstream errorStream;
    errorStream << "Internal Vulkan error (" << errorCode << "): " << VulkanResultString(errorCode)
                << ".";

    getRenderer()->getMemoryAllocationTracker()->logMemoryStatsOnError();

    if (errorCode == VK_ERROR_DEVICE_LOST)
    {
        WARN() << errorStream.str();
        handleDeviceLost();
    }

    mErrors->handleError(glErrorCode, errorStream.str().c_str(), file, function, line);
}

angle::Result ContextVk::initBufferAllocation(vk::BufferHelper *bufferHelper,
                                              uint32_t memoryTypeIndex,
                                              size_t allocationSize,
                                              size_t alignment,
                                              BufferUsageType bufferUsageType)
{
    vk::BufferPool *pool = getDefaultBufferPool(allocationSize, memoryTypeIndex, bufferUsageType);
    VkResult result      = bufferHelper->initSuballocation(this, memoryTypeIndex, allocationSize,
                                                           alignment, bufferUsageType, pool);
    if (ANGLE_LIKELY(result == VK_SUCCESS))
    {
        if (mRenderer->getFeatures().allocateNonZeroMemory.enabled)
        {
            ANGLE_TRY(bufferHelper->initializeNonZeroMemory(
                this, GetDefaultBufferUsageFlags(mRenderer), allocationSize));
        }

        return angle::Result::Continue;
    }

    // If the error is not OOM, we should stop and handle the error. In case of OOM, we can try
    // other options.
    bool shouldTryFallback = (result == VK_ERROR_OUT_OF_DEVICE_MEMORY);
    ANGLE_VK_CHECK(this, shouldTryFallback, result);

    // If memory allocation fails, it is possible to retry the allocation after cleaning the garbage
    // and waiting for submitted commands to finish if necessary.
    bool anyGarbageCleaned  = false;
    bool someGarbageCleaned = false;
    do
    {
        ANGLE_TRY(mRenderer->cleanupSomeGarbage(this, &anyGarbageCleaned));
        if (anyGarbageCleaned)
        {
            someGarbageCleaned = true;
            result = bufferHelper->initSuballocation(this, memoryTypeIndex, allocationSize,
                                                     alignment, bufferUsageType, pool);
        }
    } while (result != VK_SUCCESS && anyGarbageCleaned);

    if (someGarbageCleaned)
    {
        INFO() << "Initial allocation failed. Cleaned some garbage | Allocation result: "
               << ((result == VK_SUCCESS) ? "SUCCESS" : "FAIL");
    }

    // If memory allocation fails, it is possible retry after flushing the context and cleaning all
    // the garbage.
    if (result != VK_SUCCESS)
    {
        ANGLE_TRY(finishImpl(RenderPassClosureReason::OutOfMemory));
        INFO() << "Context flushed due to out-of-memory error.";
        result = bufferHelper->initSuballocation(this, memoryTypeIndex, allocationSize, alignment,
                                                 bufferUsageType, pool);
    }

    // If the allocation continues to fail despite all the fallback options, the error must be
    // returned.
    ANGLE_VK_CHECK(this, result == VK_SUCCESS, result);

    // Initialize with non-zero value if needed.
    if (mRenderer->getFeatures().allocateNonZeroMemory.enabled)
    {
        ANGLE_TRY(bufferHelper->initializeNonZeroMemory(this, GetDefaultBufferUsageFlags(mRenderer),
                                                        allocationSize));
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::initImageAllocation(vk::ImageHelper *imageHelper,
                                             bool hasProtectedContent,
                                             const vk::MemoryProperties &memoryProperties,
                                             VkMemoryPropertyFlags flags,
                                             vk::MemoryAllocationType allocationType)
{
    VkMemoryPropertyFlags oomExcludedFlags = 0;
    VkMemoryPropertyFlags outputFlags;
    VkDeviceSize outputSize;

    if (hasProtectedContent)
    {
        flags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }

    // Get memory requirements for the allocation.
    VkMemoryRequirements memoryRequirements;
    imageHelper->getImage().getMemoryRequirements(getDevice(), &memoryRequirements);
    bool allocateDedicatedMemory =
        mRenderer->getImageMemorySuballocator().needsDedicatedMemory(memoryRequirements.size);

    VkResult result = imageHelper->initMemory(this, memoryProperties, flags, oomExcludedFlags,
                                              &memoryRequirements, allocateDedicatedMemory,
                                              allocationType, &outputFlags, &outputSize);
    if (ANGLE_LIKELY(result == VK_SUCCESS))
    {
        if (mRenderer->getFeatures().allocateNonZeroMemory.enabled)
        {
            ANGLE_TRY(imageHelper->initializeNonZeroMemory(this, hasProtectedContent, outputFlags,
                                                           outputSize));
        }

        return angle::Result::Continue;
    }

    // If the error is not OOM, we should stop and handle the error. In case of OOM, we can try
    // other options.
    bool shouldTryFallback = (result == VK_ERROR_OUT_OF_DEVICE_MEMORY);
    ANGLE_VK_CHECK(this, shouldTryFallback, result);

    // If memory allocation fails, it is possible to retry the allocation after cleaning the garbage
    // and waiting for submitted commands to finish if necessary.
    bool anyGarbageCleaned  = false;
    bool someGarbageCleaned = false;
    do
    {
        ANGLE_TRY(mRenderer->cleanupSomeGarbage(this, &anyGarbageCleaned));
        if (anyGarbageCleaned)
        {
            someGarbageCleaned = true;
            result = imageHelper->initMemory(this, memoryProperties, flags, oomExcludedFlags,
                                             &memoryRequirements, allocateDedicatedMemory,
                                             allocationType, &outputFlags, &outputSize);
        }
    } while (result != VK_SUCCESS && anyGarbageCleaned);

    if (someGarbageCleaned)
    {
        INFO() << "Initial allocation failed. Cleaned some garbage | Allocation result: "
               << ((result == VK_SUCCESS) ? "SUCCESS" : "FAIL");
    }

    // If memory allocation fails, it is possible retry after flushing the context and cleaning all
    // the garbage.
    if (result != VK_SUCCESS)
    {
        ANGLE_TRY(finishImpl(RenderPassClosureReason::OutOfMemory));
        INFO() << "Context flushed due to out-of-memory error.";
        result = imageHelper->initMemory(this, memoryProperties, flags, oomExcludedFlags,
                                         &memoryRequirements, allocateDedicatedMemory,
                                         allocationType, &outputFlags, &outputSize);
    }

    // If no fallback has worked so far, we should record the failed allocation information in case
    // it needs to be logged.
    if (result != VK_SUCCESS)
    {
        uint32_t pendingMemoryTypeIndex;
        if (vma::FindMemoryTypeIndexForImageInfo(
                mRenderer->getAllocator().getHandle(), &imageHelper->getVkImageCreateInfo(), flags,
                flags, allocateDedicatedMemory, &pendingMemoryTypeIndex) == VK_SUCCESS)
        {
            mRenderer->getMemoryAllocationTracker()->setPendingMemoryAlloc(
                allocationType, memoryRequirements.size, pendingMemoryTypeIndex);
        }
    }

    // If there is no space for the new allocation and other fallbacks have proved ineffective, the
    // allocation may still be made outside the device from all other memory types, although it will
    // result in performance penalty. This is a last resort.
    if (result != VK_SUCCESS)
    {
        oomExcludedFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        result           = imageHelper->initMemory(this, memoryProperties, flags, oomExcludedFlags,
                                                   &memoryRequirements, allocateDedicatedMemory,
                                                   allocationType, &outputFlags, &outputSize);
        INFO()
            << "Allocation failed. Removed the DEVICE_LOCAL bit requirement | Allocation result: "
            << ((result == VK_SUCCESS) ? "SUCCESS" : "FAIL");

        if (result == VK_SUCCESS)
        {
            // For images allocated here, although allocation is preferred on the device, it is not
            // required.
            mRenderer->getMemoryAllocationTracker()->compareExpectedFlagsWithAllocatedFlags(
                flags & ~oomExcludedFlags, flags, outputFlags,
                reinterpret_cast<void *>(imageHelper->getAllocation().getHandle()));

            getPerfCounters().deviceMemoryImageAllocationFallbacks++;
        }
    }

    // If the allocation continues to fail despite all the fallback options, the error must be
    // returned.
    ANGLE_VK_CHECK(this, result == VK_SUCCESS, result);

    // Initialize with non-zero value if needed.
    if (mRenderer->getFeatures().allocateNonZeroMemory.enabled)
    {
        ANGLE_TRY(imageHelper->initializeNonZeroMemory(this, hasProtectedContent, outputFlags,
                                                       outputSize));
        imageHelper->getImage().getHandle();
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::releaseBufferAllocation(vk::BufferHelper *bufferHelper)
{
    bufferHelper->releaseBufferAndDescriptorSetCache(this);

    if (ANGLE_UNLIKELY(hasExcessPendingGarbage()))
    {
        ANGLE_TRY(flushAndSubmitCommands(nullptr, nullptr,
                                         RenderPassClosureReason::ExcessivePendingGarbage));
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::initBufferForBufferCopy(vk::BufferHelper *bufferHelper,
                                                 size_t size,
                                                 vk::MemoryCoherency coherency)
{
    uint32_t memoryTypeIndex = mRenderer->getStagingBufferMemoryTypeIndex(coherency);
    size_t alignment         = mRenderer->getStagingBufferAlignment();
    return initBufferAllocation(bufferHelper, memoryTypeIndex, size, alignment,
                                BufferUsageType::Dynamic);
}

angle::Result ContextVk::initBufferForImageCopy(vk::BufferHelper *bufferHelper,
                                                size_t size,
                                                vk::MemoryCoherency coherency,
                                                angle::FormatID formatId,
                                                VkDeviceSize *offset,
                                                uint8_t **dataPtr)
{
    // When a buffer is used in copyImage, the offset must be multiple of pixel bytes. This may
    // result in non-power of two alignment. VMA's virtual allocator can not handle non-power of two
    // alignment. We have to adjust offset manually.
    uint32_t memoryTypeIndex  = mRenderer->getStagingBufferMemoryTypeIndex(coherency);
    size_t imageCopyAlignment = vk::GetImageCopyBufferAlignment(formatId);

    // Add extra padding for potential offset alignment
    size_t allocationSize   = size + imageCopyAlignment;
    allocationSize          = roundUp(allocationSize, imageCopyAlignment);
    size_t stagingAlignment = static_cast<size_t>(mRenderer->getStagingBufferAlignment());

    ANGLE_TRY(initBufferAllocation(bufferHelper, memoryTypeIndex, allocationSize, stagingAlignment,
                                   BufferUsageType::Static));

    *offset  = roundUp(bufferHelper->getOffset(), static_cast<VkDeviceSize>(imageCopyAlignment));
    *dataPtr = bufferHelper->getMappedMemory() + (*offset) - bufferHelper->getOffset();

    return angle::Result::Continue;
}

angle::Result ContextVk::initBufferForVertexConversion(ConversionBuffer *conversionBuffer,
                                                       size_t size,
                                                       vk::MemoryHostVisibility hostVisibility)
{
    vk::BufferHelper *bufferHelper = conversionBuffer->getBuffer();
    if (bufferHelper->valid())
    {
        // If size is big enough and it is idle, then just reuse the existing buffer. Or if current
        // render pass uses the buffer, try to allocate a new one to avoid breaking the render pass.
        if (size <= bufferHelper->getSize() &&
            (hostVisibility == vk::MemoryHostVisibility::Visible) ==
                bufferHelper->isHostVisible() &&
            !isRenderPassStartedAndUsesBuffer(*bufferHelper))
        {
            if (mRenderer->hasResourceUseFinished(bufferHelper->getResourceUse()))
            {
                bufferHelper->initializeBarrierTracker(this);
                return angle::Result::Continue;
            }
            else if (hostVisibility == vk::MemoryHostVisibility::NonVisible)
            {
                // For device local buffer, we can reuse the buffer even if it is still GPU busy.
                // The memory barrier should take care of this.
                return angle::Result::Continue;
            }
        }

        bufferHelper->release(this);
    }

    //  Mark entire buffer dirty if we have to reallocate the buffer.
    conversionBuffer->setEntireBufferDirty();

    uint32_t memoryTypeIndex = mRenderer->getVertexConversionBufferMemoryTypeIndex(hostVisibility);
    size_t alignment         = static_cast<size_t>(mRenderer->getVertexConversionBufferAlignment());

    // The size is retrieved and used in descriptor set. The descriptor set wants aligned size,
    // otherwise there are test failures. Note that the underlying VMA allocation is always
    // allocated with an aligned size anyway.
    size_t sizeToAllocate = roundUp(size, alignment);

    return initBufferAllocation(bufferHelper, memoryTypeIndex, sizeToAllocate, alignment,
                                BufferUsageType::Static);
}

angle::Result ContextVk::updateActiveTextures(const gl::Context *context, gl::Command command)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ProgramExecutableVk *executableVk       = vk::GetImpl(executable);

    const gl::ActiveTexturesCache &textures        = mState.getActiveTexturesCache();
    const gl::ActiveTextureMask &activeTextures    = executable->getActiveSamplersMask();
    const gl::ActiveTextureTypeArray &textureTypes = executable->getActiveSamplerTypes();

    FillWithNullptr(&mActiveTextures);

    bool recreatePipelineLayout                                     = false;
    ImmutableSamplerIndexMap immutableSamplerIndexMap               = {};
    std::unordered_map<size_t, uint32_t> textureUnitSamplerIndexMap = {};
    for (size_t textureUnit : activeTextures)
    {
        gl::Texture *texture        = textures[textureUnit];
        gl::TextureType textureType = textureTypes[textureUnit];
        ASSERT(textureType != gl::TextureType::InvalidEnum);

        const bool isIncompleteTexture = texture == nullptr;

        // Null textures represent incomplete textures.
        if (isIncompleteTexture)
        {
            ANGLE_TRY(getIncompleteTexture(
                context, textureType, executable->getSamplerFormatForTextureUnitIndex(textureUnit),
                &texture));
        }

        TextureVk *textureVk = vk::GetImpl(texture);
        ASSERT(textureVk != nullptr);

        mActiveTextures[textureUnit] = textureVk;

        if (textureType == gl::TextureType::Buffer)
        {
            continue;
        }

        if (!isIncompleteTexture && texture->isDepthOrStencil())
        {
            const bool isStencilTexture = IsStencilSamplerBinding(*executable, textureUnit);
            ANGLE_TRY(switchToReadOnlyDepthStencilMode(texture, command, getDrawFramebuffer(),
                                                       isStencilTexture));
        }

        gl::Sampler *sampler = mState.getSampler(static_cast<uint32_t>(textureUnit));
        const gl::SamplerState &samplerState =
            sampler ? sampler->getSamplerState() : texture->getSamplerState();

        // GL_EXT_texture_sRGB_decode
        //   The new parameter, TEXTURE_SRGB_DECODE_EXT controls whether the
        //   decoding happens at sample time. It only applies to textures with an
        //   internal format that is sRGB and is ignored for all other textures.
        ANGLE_TRY(textureVk->updateSrgbDecodeState(this, samplerState));

        const vk::ImageHelper &image = textureVk->getImage();
        if (image.hasInefficientlyEmulatedImageFormat())
        {
            ANGLE_VK_PERF_WARNING(
                this, GL_DEBUG_SEVERITY_LOW,
                "The Vulkan driver does not support texture format 0x%04X, emulating with 0x%04X",
                image.getIntendedFormat().glInternalFormat,
                image.getActualFormat().glInternalFormat);
        }

        if (image.hasImmutableSampler())
        {
            if (textureUnitSamplerIndexMap.empty())
            {
                GenerateTextureUnitSamplerIndexMap(executable->getSamplerBoundTextureUnits(),
                                                   &textureUnitSamplerIndexMap);
            }
            immutableSamplerIndexMap[image.getYcbcrConversionDesc()] =
                textureUnitSamplerIndexMap[textureUnit];
        }

        if (textureVk->getAndResetImmutableSamplerDirtyState())
        {
            recreatePipelineLayout = true;
        }
    }

    if (!executableVk->areImmutableSamplersCompatible(immutableSamplerIndexMap))
    {
        recreatePipelineLayout = true;
    }

    // Recreate the pipeline layout, if necessary.
    if (recreatePipelineLayout)
    {
        executableVk->resetLayout(this);
        ANGLE_TRY(executableVk->createPipelineLayout(
            this, &getPipelineLayoutCache(), &getDescriptorSetLayoutCache(), &mActiveTextures));
        ANGLE_TRY(executableVk->initializeDescriptorPools(this, &getDescriptorSetLayoutCache(),
                                                          &getMetaDescriptorPools()));

        // The default uniforms descriptor set was reset during createPipelineLayout(), so mark them
        // dirty to get everything reallocated/rebound before the next draw.
        if (executable->hasDefaultUniforms())
        {
            executableVk->setAllDefaultUniformsDirty();
        }
    }

    return angle::Result::Continue;
}

template <typename CommandBufferHelperT>
angle::Result ContextVk::updateActiveImages(CommandBufferHelperT *commandBufferHelper)
{
    const gl::State &glState                = mState;
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    ASSERT(executable);

    // If there are memoryBarrier call being made that requires we insert barriers for images we
    // must do so.
    bool memoryBarrierRequired = false;
    if ((mDeferredMemoryBarriers & kWriteAfterAccessImageMemoryBarriers) != 0)
    {
        memoryBarrierRequired = true;
        mDeferredMemoryBarriers &= ~kWriteAfterAccessImageMemoryBarriers;
    }

    FillWithNullptr(&mActiveImages);

    const gl::ActiveTextureMask &activeImages = executable->getActiveImagesMask();
    const gl::ActiveTextureArray<gl::ShaderBitSet> &activeImageShaderBits =
        executable->getActiveImageShaderBits();

    // Note: currently, the image layout is transitioned entirely even if only one level or layer is
    // used.  This is an issue if one subresource of the image is used as framebuffer attachment and
    // the other as image.  This is a similar issue to http://anglebug.com/40096531.  Another issue
    // however is if multiple subresources of the same image are used at the same time.
    // Inefficiencies aside, setting write dependency on the same image multiple times is not
    // supported.  The following makes sure write dependencies are set only once per image.
    std::set<vk::ImageHelper *> alreadyProcessed;

    for (size_t imageUnitIndex : activeImages)
    {
        const gl::ImageUnit &imageUnit = glState.getImageUnit(imageUnitIndex);
        const gl::Texture *texture     = imageUnit.texture.get();
        if (texture == nullptr)
        {
            continue;
        }

        TextureVk *textureVk          = vk::GetImpl(texture);
        mActiveImages[imageUnitIndex] = textureVk;

        // The image should be flushed and ready to use at this point. There may still be
        // lingering staged updates in its staging buffer for unused texture mip levels or
        // layers. Therefore we can't verify it has no staged updates right here.
        gl::ShaderBitSet shaderStages = activeImageShaderBits[imageUnitIndex];
        ASSERT(shaderStages.any());

        // Special handling of texture buffers.  They have a buffer attached instead of an image.
        if (texture->getType() == gl::TextureType::Buffer)
        {
            BufferVk *bufferVk = vk::GetImpl(textureVk->getBuffer().get());

            OnImageBufferWrite(this, bufferVk, shaderStages, commandBufferHelper);

            textureVk->retainBufferViews(commandBufferHelper);
            continue;
        }

        vk::ImageHelper *image = &textureVk->getImage();

        if (alreadyProcessed.find(image) != alreadyProcessed.end())
        {
            continue;
        }
        alreadyProcessed.insert(image);

        gl::LevelIndex level;
        uint32_t layerStart               = 0;
        uint32_t layerCount               = 0;
        const vk::ImageLayout imageLayout = GetImageWriteLayoutAndSubresource(
            imageUnit, *image, shaderStages, &level, &layerStart, &layerCount);

        if (imageLayout == image->getCurrentImageLayout() && !memoryBarrierRequired)
        {
            // GL spec does not require implementation to do WAW barriers for shader image access.
            // If there is no layout change, we skip the barrier here unless there is prior
            // memoryBarrier call.
            commandBufferHelper->retainImageWithEvent(this, image);
        }
        else
        {
            commandBufferHelper->imageWrite(this, level, layerStart, layerCount,
                                            image->getAspectFlags(), imageLayout, image);
        }
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::flushAndSubmitCommands(const vk::Semaphore *signalSemaphore,
                                                const vk::SharedExternalFence *externalFence,
                                                RenderPassClosureReason renderPassClosureReason)
{
    // Even if render pass does not have any command, we may still need to submit it in case it has
    // CLEAR loadOp.
    bool someCommandsNeedFlush =
        !mOutsideRenderPassCommands->empty() || mRenderPassCommands->started();
    bool someCommandAlreadyFlushedNeedsSubmit =
        mLastFlushedQueueSerial != mLastSubmittedQueueSerial;
    bool someOtherReasonNeedsSubmit = signalSemaphore != nullptr || externalFence != nullptr ||
                                      mHasWaitSemaphoresPendingSubmission;

    if (!someCommandsNeedFlush && !someCommandAlreadyFlushedNeedsSubmit &&
        !someOtherReasonNeedsSubmit)
    {
        // We have nothing to submit.
        return angle::Result::Continue;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::flushAndSubmitCommands");
    if (someCommandsNeedFlush)
    {
        // If any of secondary command buffer not empty, we need to do flush
        // Avoid calling vkQueueSubmit() twice, since submitCommands() below will do that.
        ANGLE_TRY(flushCommandsAndEndRenderPassWithoutSubmit(renderPassClosureReason));
    }
    else if (someCommandAlreadyFlushedNeedsSubmit)
    {
        // This is when someone already called flushCommandsAndEndRenderPassWithoutQueueSubmit.
        // Nothing to flush but we have some command to submit.
        ASSERT(mLastFlushedQueueSerial.valid());
        ASSERT(QueueSerialsHaveDifferentIndexOrSmaller(mLastSubmittedQueueSerial,
                                                       mLastFlushedQueueSerial));
    }

    const bool outsideRenderPassWritesToBuffer =
        mOutsideRenderPassCommands->getAndResetHasHostVisibleBufferWrite();
    const bool renderPassWritesToBuffer =
        mRenderPassCommands->getAndResetHasHostVisibleBufferWrite();
    if (mIsAnyHostVisibleBufferWritten || outsideRenderPassWritesToBuffer ||
        renderPassWritesToBuffer)
    {
        // Make sure all writes to host-visible buffers are flushed.  We have no way of knowing
        // whether any buffer will be mapped for readback in the future, and we can't afford to
        // flush and wait on a one-pipeline-barrier command buffer on every map().
        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask   = VK_ACCESS_MEMORY_WRITE_BIT;
        memoryBarrier.dstAccessMask   = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;

        mOutsideRenderPassCommands->getCommandBuffer().memoryBarrier(
            mRenderer->getSupportedBufferWritePipelineStageMask(), VK_PIPELINE_STAGE_HOST_BIT,
            memoryBarrier);
        mIsAnyHostVisibleBufferWritten = false;
    }

    if (mGpuEventsEnabled)
    {
        EventName eventName = GetTraceEventName("Primary", mPrimaryBufferEventCounter);
        ANGLE_TRY(traceGpuEvent(&mOutsideRenderPassCommands->getCommandBuffer(),
                                TRACE_EVENT_PHASE_END, eventName));
    }

    // This will handle any commands that might be recorded above as well as flush any wait
    // semaphores.
    ANGLE_TRY(flushOutsideRenderPassCommands());

    if (mLastFlushedQueueSerial == mLastSubmittedQueueSerial)
    {
        // We have to do empty submission...
        ASSERT(!someCommandsNeedFlush);
        mLastFlushedQueueSerial = mOutsideRenderPassCommands->getQueueSerial();
        generateOutsideRenderPassCommandsQueueSerial();
    }

    // We must add the per context dynamic buffers into resourceUseList before submission so that
    // they get retained properly until GPU completes. We do not add current buffer into
    // resourceUseList since they never get reused or freed until context gets destroyed, at which
    // time we always wait for GPU to finish before destroying the dynamic buffers.
    mDefaultUniformStorage.updateQueueSerialAndReleaseInFlightBuffers(this,
                                                                      mLastFlushedQueueSerial);

    if (mHasInFlightStreamedVertexBuffers.any())
    {
        for (size_t attribIndex : mHasInFlightStreamedVertexBuffers)
        {
            mStreamedVertexBuffers[attribIndex].updateQueueSerialAndReleaseInFlightBuffers(
                this, mLastFlushedQueueSerial);
        }
        mHasInFlightStreamedVertexBuffers.reset();
    }

    ASSERT(mWaitSemaphores.empty());
    ASSERT(mWaitSemaphoreStageMasks.empty());

    ANGLE_TRY(submitCommands(signalSemaphore, externalFence, Submit::AllCommands));

    ASSERT(mOutsideRenderPassCommands->getQueueSerial() > mLastSubmittedQueueSerial);

    mHasAnyCommandsPendingSubmission    = false;
    mHasWaitSemaphoresPendingSubmission = false;
    onRenderPassFinished(RenderPassClosureReason::AlreadySpecifiedElsewhere);

    if (mGpuEventsEnabled)
    {
        EventName eventName = GetTraceEventName("Primary", ++mPrimaryBufferEventCounter);
        ANGLE_TRY(traceGpuEvent(&mOutsideRenderPassCommands->getCommandBuffer(),
                                TRACE_EVENT_PHASE_BEGIN, eventName));
    }

    // Since we just flushed, deferred flush is no longer deferred.
    mHasDeferredFlush = false;
    return angle::Result::Continue;
}

angle::Result ContextVk::finishImpl(RenderPassClosureReason renderPassClosureReason)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::finishImpl");

    ANGLE_TRY(flushAndSubmitCommands(nullptr, nullptr, renderPassClosureReason));

    // You must have to wait for all queue indices ever used to finish. Just wait for
    // mLastSubmittedQueueSerial (which only contains current index) to finish is not enough, if it
    // has ever became unCurrent and then Current again.
    ANGLE_TRY(mRenderer->finishResourceUse(this, mSubmittedResourceUse));

    clearAllGarbage();

    if (mGpuEventsEnabled)
    {
        // This loop should in practice execute once since the queue is already idle.
        while (mInFlightGpuEventQueries.size() > 0)
        {
            ANGLE_TRY(checkCompletedGpuEvents());
        }
        // Recalculate the CPU/GPU time difference to account for clock drifting.  Avoid
        // unnecessary synchronization if there is no event to be adjusted (happens when
        // finish() gets called multiple times towards the end of the application).
        if (mGpuEvents.size() > 0)
        {
            ANGLE_TRY(synchronizeCpuGpuTime());
        }
    }

    return angle::Result::Continue;
}

void ContextVk::addWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags stageMask)
{
    mWaitSemaphores.push_back(semaphore);
    mWaitSemaphoreStageMasks.push_back(stageMask);
    mHasWaitSemaphoresPendingSubmission = true;
}

angle::Result ContextVk::getCompatibleRenderPass(const vk::RenderPassDesc &desc,
                                                 const vk::RenderPass **renderPassOut)
{
    if (getFeatures().preferDynamicRendering.enabled)
    {
        *renderPassOut = &mNullRenderPass;
        return angle::Result::Continue;
    }

    // Note: Each context has it's own RenderPassCache so no locking needed.
    return mRenderPassCache.getCompatibleRenderPass(this, desc, renderPassOut);
}

angle::Result ContextVk::getRenderPassWithOps(const vk::RenderPassDesc &desc,
                                              const vk::AttachmentOpsArray &ops,
                                              const vk::RenderPass **renderPassOut)
{
    if (getFeatures().preferDynamicRendering.enabled)
    {
        if (mState.isPerfMonitorActive())
        {
            mRenderPassCommands->updatePerfCountersForDynamicRenderingInstance(this,
                                                                               &mPerfCounters);
        }
        return angle::Result::Continue;
    }

    // Note: Each context has it's own RenderPassCache so no locking needed.
    return mRenderPassCache.getRenderPassWithOps(this, desc, ops, renderPassOut);
}

angle::Result ContextVk::getTimestamp(uint64_t *timestampOut)
{
    // The intent of this function is to query the timestamp without stalling the GPU.
    // Currently, that seems impossible, so instead, we are going to make a small submission
    // with just a timestamp query.  First, the disjoint timer query extension says:
    //
    // > This will return the GL time after all previous commands have reached the GL server but
    // have not yet necessarily executed.
    //
    // The previous commands may be deferred at the moment and not yet flushed. The wording allows
    // us to make a submission to get the timestamp without flushing.
    //
    // Second:
    //
    // > By using a combination of this synchronous get command and the asynchronous timestamp
    // query object target, applications can measure the latency between when commands reach the
    // GL server and when they are realized in the framebuffer.
    //
    // This fits with the above strategy as well, although inevitably we are possibly
    // introducing a GPU bubble.  This function directly generates a command buffer and submits
    // it instead of using the other member functions.  This is to avoid changing any state,
    // such as the queue serial.

    // Create a query used to receive the GPU timestamp
    VkDevice device = getDevice();
    vk::DeviceScoped<vk::DynamicQueryPool> timestampQueryPool(device);
    vk::QueryHelper timestampQuery;
    ANGLE_TRY(timestampQueryPool.get().init(this, VK_QUERY_TYPE_TIMESTAMP, 1));
    ANGLE_TRY(timestampQueryPool.get().allocateQuery(this, &timestampQuery, 1));

    // Record the command buffer
    vk::ScopedPrimaryCommandBuffer scopedCommandBuffer(device);

    ANGLE_TRY(mRenderer->getCommandBufferOneOff(this, getProtectionType(), &scopedCommandBuffer));
    vk::PrimaryCommandBuffer &commandBuffer = scopedCommandBuffer.get();

    timestampQuery.writeTimestampToPrimary(this, &commandBuffer);
    ANGLE_VK_TRY(this, commandBuffer.end());

    QueueSerial submitQueueSerial;
    ANGLE_TRY(mRenderer->queueSubmitOneOff(this, std::move(scopedCommandBuffer),
                                           getProtectionType(), mContextPriority, VK_NULL_HANDLE, 0,
                                           &submitQueueSerial));
    // Track it with the submitSerial.
    timestampQuery.setQueueSerial(submitQueueSerial);

    // Wait for the submission to finish.  Given no semaphores, there is hope that it would execute
    // in parallel with what's already running on the GPU.
    ANGLE_TRY(mRenderer->finishQueueSerial(this, submitQueueSerial));

    // Get the query results
    vk::QueryResult result(1);
    ANGLE_TRY(timestampQuery.getUint64Result(this, &result));
    *timestampOut = result.getResult(vk::QueryResult::kDefaultResultIndex);
    timestampQueryPool.get().freeQuery(this, &timestampQuery);

    // Convert results to nanoseconds.
    *timestampOut = static_cast<uint64_t>(
        *timestampOut *
        static_cast<double>(getRenderer()->getPhysicalDeviceProperties().limits.timestampPeriod));

    return angle::Result::Continue;
}

void ContextVk::invalidateDefaultAttribute(size_t attribIndex)
{
    mDirtyDefaultAttribsMask.set(attribIndex);
    mGraphicsDirtyBits.set(DIRTY_BIT_DEFAULT_ATTRIBS);
}

void ContextVk::invalidateDefaultAttributes(const gl::AttributesMask &dirtyMask)
{
    if (dirtyMask.any())
    {
        mDirtyDefaultAttribsMask |= dirtyMask;
        mGraphicsDirtyBits.set(DIRTY_BIT_DEFAULT_ATTRIBS);
        mGraphicsDirtyBits.set(DIRTY_BIT_VERTEX_BUFFERS);
    }
}

angle::Result ContextVk::onBufferReleaseToExternal(const vk::BufferHelper &buffer)
{
    if (mRenderPassCommands->usesBuffer(buffer))
    {
        return flushCommandsAndEndRenderPass(
            RenderPassClosureReason::BufferUseThenReleaseToExternal);
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::onImageReleaseToExternal(const vk::ImageHelper &image)
{
    if (isRenderPassStartedAndUsesImage(image))
    {
        return flushCommandsAndEndRenderPass(
            RenderPassClosureReason::ImageUseThenReleaseToExternal);
    }
    return angle::Result::Continue;
}

void ContextVk::finalizeImageLayout(vk::ImageHelper *image, UniqueSerial imageSiblingSerial)
{
    if (mRenderPassCommands->started())
    {
        mRenderPassCommands->finalizeImageLayout(this, image, imageSiblingSerial);
    }

    if (image->isForeignImage() && !image->isReleasedToForeign())
    {
        finalizeForeignImage(image);
    }
}

angle::Result ContextVk::beginNewRenderPass(
    vk::RenderPassFramebuffer &&framebuffer,
    const gl::Rectangle &renderArea,
    const vk::RenderPassDesc &renderPassDesc,
    const vk::AttachmentOpsArray &renderPassAttachmentOps,
    const vk::PackedAttachmentCount colorAttachmentCount,
    const vk::PackedAttachmentIndex depthStencilAttachmentIndex,
    const vk::PackedClearValuesArray &clearValues,
    vk::RenderPassCommandBuffer **commandBufferOut)
{
    // End any currently outstanding render pass. The render pass is normally closed before reaching
    // here for various reasons, except typically when UtilsVk needs to start one.
    ANGLE_TRY(flushCommandsAndEndRenderPass(RenderPassClosureReason::NewRenderPass));

    // Now generate queueSerial for the renderPass.
    QueueSerial renderPassQueueSerial;
    generateRenderPassCommandsQueueSerial(&renderPassQueueSerial);

    mPerfCounters.renderPasses++;
    ANGLE_TRY(mRenderPassCommands->beginRenderPass(
        this, std::move(framebuffer), renderArea, renderPassDesc, renderPassAttachmentOps,
        colorAttachmentCount, depthStencilAttachmentIndex, clearValues, renderPassQueueSerial,
        commandBufferOut));

    // By default all render pass should allow to be reactivated.
    mAllowRenderPassToReactivate = true;

    if (mCurrentGraphicsPipeline)
    {
        ASSERT(mCurrentGraphicsPipeline->valid());
        mCurrentGraphicsPipeline->retainInRenderPass(mRenderPassCommands);
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::startRenderPass(gl::Rectangle renderArea,
                                         vk::RenderPassCommandBuffer **commandBufferOut,
                                         bool *renderPassDescChangedOut)
{
    FramebufferVk *drawFramebufferVk = getDrawFramebuffer();
    ASSERT(drawFramebufferVk == vk::GetImpl(mState.getDrawFramebuffer()));

    ANGLE_TRY(drawFramebufferVk->startNewRenderPass(this, renderArea, &mRenderPassCommandBuffer,
                                                    renderPassDescChangedOut));

    // For dynamic rendering, the FramebufferVk's render pass desc does not track whether
    // framebuffer fetch is in use.  In that case, ContextVk updates the command buffer's (and
    // graphics pipeline's) render pass desc only:
    //
    // - When the render pass starts
    // - When the program binding changes (see |invalidateProgramExecutableHelper|)
    if (getFeatures().preferDynamicRendering.enabled)
    {
        vk::FramebufferFetchMode framebufferFetchMode =
            vk::GetProgramFramebufferFetchMode(mState.getProgramExecutable());
        if (framebufferFetchMode != vk::FramebufferFetchMode::None)
        {
            // Note: this function sets a dirty bit through onColorAccessChange() not through
            // |dirtyBitsIterator|, but that dirty bit is always set on new render passes, so it
            // won't be missed.
            onFramebufferFetchUse(framebufferFetchMode);
        }
        else
        {
            // Reset framebuffer fetch mode.  Note that |onFramebufferFetchUse| _accumulates_
            // framebuffer fetch mode.
            mRenderPassCommands->setFramebufferFetchMode(vk::FramebufferFetchMode::None);
        }
    }

    // Make sure the render pass is not restarted if it is started by UtilsVk (as opposed to
    // setupDraw(), which clears this bit automatically).
    mGraphicsDirtyBits.reset(DIRTY_BIT_RENDER_PASS);

    ANGLE_TRY(resumeRenderPassQueriesIfActive());

    if (commandBufferOut)
    {
        *commandBufferOut = mRenderPassCommandBuffer;
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::startNextSubpass()
{
    ASSERT(hasActiveRenderPass());

    // The graphics pipelines are bound to a subpass, so update the subpass as well.
    mGraphicsPipelineDesc->nextSubpass(&mGraphicsPipelineTransition);

    return mRenderPassCommands->nextSubpass(this, &mRenderPassCommandBuffer);
}

uint32_t ContextVk::getCurrentSubpassIndex() const
{
    return mGraphicsPipelineDesc->getSubpass();
}

uint32_t ContextVk::getCurrentViewCount() const
{
    FramebufferVk *drawFBO = vk::GetImpl(mState.getDrawFramebuffer());
    return drawFBO->getRenderPassDesc().viewCount();
}

angle::Result ContextVk::flushCommandsAndEndRenderPassWithoutSubmit(RenderPassClosureReason reason)
{
    // Ensure we flush the RenderPass *after* the prior commands.
    ANGLE_TRY(flushOutsideRenderPassCommands());
    ASSERT(mOutsideRenderPassCommands->empty());

    if (!mRenderPassCommands->started())
    {
        onRenderPassFinished(RenderPassClosureReason::AlreadySpecifiedElsewhere);
        return angle::Result::Continue;
    }

    // Set dirty bits if render pass was open (and thus will be closed).
    mGraphicsDirtyBits |= mNewGraphicsCommandBufferDirtyBits;

    mCurrentTransformFeedbackQueueSerial = QueueSerial();

    onRenderPassFinished(reason);

    if (mGpuEventsEnabled)
    {
        EventName eventName = GetTraceEventName("RP", mPerfCounters.renderPasses);
        ANGLE_TRY(traceGpuEvent(&mOutsideRenderPassCommands->getCommandBuffer(),
                                TRACE_EVENT_PHASE_BEGIN, eventName));
        ANGLE_TRY(flushOutsideRenderPassCommands());
    }

    addOverlayUsedBuffersCount(mRenderPassCommands);

    pauseTransformFeedbackIfActiveUnpaused();

    ANGLE_TRY(mRenderPassCommands->endRenderPass(this));

    if (kEnableCommandStreamDiagnostics)
    {
        addCommandBufferDiagnostics(mRenderPassCommands->getCommandDiagnostics());
    }

    flushDescriptorSetUpdates();
    // Collect RefCountedEvent garbage before submitting to renderer
    mRenderPassCommands->collectRefCountedEventsGarbage(
        mRenderer, mShareGroupVk->getRefCountedEventsGarbageRecycler());

    // Save the queueSerial before calling flushRenderPassCommands, which may return a new
    // mRenderPassCommands
    ASSERT(QueueSerialsHaveDifferentIndexOrSmaller(mLastFlushedQueueSerial,
                                                   mRenderPassCommands->getQueueSerial()));
    mLastFlushedQueueSerial = mRenderPassCommands->getQueueSerial();

    const vk::RenderPass unusedRenderPass;
    const vk::RenderPass *renderPass  = &unusedRenderPass;
    VkFramebuffer framebufferOverride = VK_NULL_HANDLE;

    ANGLE_TRY(getRenderPassWithOps(mRenderPassCommands->getRenderPassDesc(),
                                   mRenderPassCommands->getAttachmentOps(), &renderPass));

    // If a new framebuffer is used to accommodate resolve attachments that have been added
    // after the fact, create a temp one now and add it to garbage list.
    if (!getFeatures().preferDynamicRendering.enabled &&
        mRenderPassCommands->getFramebuffer().needsNewFramebufferWithResolveAttachments())
    {
        vk::Framebuffer tempFramebuffer;
        ANGLE_TRY(mRenderPassCommands->getFramebuffer().packResolveViewsAndCreateFramebuffer(
            this, *renderPass, &tempFramebuffer));

        framebufferOverride = tempFramebuffer.getHandle();
        addGarbage(&tempFramebuffer);
    }

    if (mRenderPassCommands->getAndResetHasHostVisibleBufferWrite())
    {
        mIsAnyHostVisibleBufferWritten = true;
    }
    ANGLE_TRY(mRenderer->flushRenderPassCommands(this, getProtectionType(), mContextPriority,
                                                 *renderPass, framebufferOverride,
                                                 &mRenderPassCommands));

    // We just flushed outSideRenderPassCommands above, and any future use of
    // outsideRenderPassCommands must have a queueSerial bigger than renderPassCommands. To ensure
    // this ordering, we generate a new queueSerial for outsideRenderPassCommands here.
    mOutsideRenderPassSerialFactory.reset();

    // Generate a new serial for outside commands.
    generateOutsideRenderPassCommandsQueueSerial();

    if (mGpuEventsEnabled)
    {
        EventName eventName = GetTraceEventName("RP", mPerfCounters.renderPasses);
        ANGLE_TRY(traceGpuEvent(&mOutsideRenderPassCommands->getCommandBuffer(),
                                TRACE_EVENT_PHASE_END, eventName));
        ANGLE_TRY(flushOutsideRenderPassCommands());
    }

    mHasAnyCommandsPendingSubmission = true;
    return angle::Result::Continue;
}

angle::Result ContextVk::flushCommandsAndEndRenderPass(RenderPassClosureReason reason)
{
    // The main reason we have mHasDeferredFlush is not to break render pass just because we want
    // to issue a flush. So there must be a started RP if it is true. Otherwise we should just
    // issue a flushAndSubmitCommands immediately instead of set mHasDeferredFlush to true.
    ASSERT(!mHasDeferredFlush || mRenderPassCommands->started());

    ANGLE_TRY(flushCommandsAndEndRenderPassWithoutSubmit(reason));

    if (mHasDeferredFlush || hasExcessPendingGarbage())
    {
        // If we have deferred glFlush call in the middle of render pass, or if there is too much
        // pending garbage, perform a flush now.
        RenderPassClosureReason flushImplReason =
            (hasExcessPendingGarbage()) ? RenderPassClosureReason::ExcessivePendingGarbage
                                        : RenderPassClosureReason::AlreadySpecifiedElsewhere;
        ANGLE_TRY(flushAndSubmitCommands(nullptr, nullptr, flushImplReason));
    }
    return angle::Result::Continue;
}

angle::Result ContextVk::flushDirtyGraphicsRenderPass(DirtyBits::Iterator *dirtyBitsIterator,
                                                      DirtyBits dirtyBitMask,
                                                      RenderPassClosureReason reason)
{
    ASSERT(mRenderPassCommands->started());

    ANGLE_TRY(flushCommandsAndEndRenderPass(reason));

    // Set dirty bits that need processing on new render pass on the dirty bits iterator that's
    // being processed right now.
    dirtyBitsIterator->setLaterBits(mNewGraphicsCommandBufferDirtyBits & dirtyBitMask);

    // Additionally, make sure any dirty bits not included in the mask are left for future
    // processing.  Note that |dirtyBitMask| is removed from |mNewGraphicsCommandBufferDirtyBits|
    // after dirty bits are iterated, so there's no need to mask them out.
    mGraphicsDirtyBits |= mNewGraphicsCommandBufferDirtyBits;

    ASSERT(mGraphicsPipelineDesc->getSubpass() == 0);

    return angle::Result::Continue;
}

angle::Result ContextVk::syncExternalMemory()
{
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask   = VK_ACCESS_MEMORY_WRITE_BIT;
    memoryBarrier.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    mOutsideRenderPassCommands->getCommandBuffer().memoryBarrier(
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, memoryBarrier);
    return angle::Result::Continue;
}

angle::Result ContextVk::onSyncObjectInit(vk::SyncHelper *syncHelper, SyncFenceScope scope)
{
    // Submit the commands:
    //
    // - This breaks the current render pass to ensure the proper ordering of the sync object in the
    //   commands,
    // - The sync object has a valid serial when it's waited on later,
    // - After waiting on the sync object, every resource that's used so far (and is being synced)
    //   will also be aware that it's finished (based on the serial) and won't incur a further wait
    //   (for example when a buffer is mapped).
    //
    // The submission is done immediately for EGL sync objects, and when no render pass is open.  If
    // a render pass is open, the submission is deferred.  This is done to be able to optimize
    // scenarios such as sync object init followed by eglSwapBuffers() (that would otherwise incur
    // another submission, as well as not being able to optimize the render-to-swapchain render
    // pass).
    if (scope != SyncFenceScope::CurrentContextToShareGroup || !mRenderPassCommands->started())
    {
        ANGLE_TRY(
            flushAndSubmitCommands(nullptr, nullptr, RenderPassClosureReason::SyncObjectInit));
        // Even if no commands is generated, and flushAndSubmitCommands bails out, queueSerial is
        // valid since Context initialization. It will always test finished/signaled.
        ASSERT(mLastSubmittedQueueSerial.valid());

        // If src synchronization scope is all contexts (an ANGLE extension), set the syncHelper
        // serial to the last serial of all contexts, instead of just the current context.
        if (scope == SyncFenceScope::AllContextsToAllContexts)
        {
            const size_t maxIndex = mRenderer->getLargestQueueSerialIndexEverAllocated();
            for (SerialIndex index = 0; index <= maxIndex; ++index)
            {
                syncHelper->setSerial(index, mRenderer->getLastSubmittedSerial(index));
            }
        }
        else
        {
            syncHelper->setQueueSerial(mLastSubmittedQueueSerial);
        }

        return angle::Result::Continue;
    }

    // Otherwise we must have a started render pass. The sync object will track the completion of
    // this render pass.
    mRenderPassCommands->retainResource(syncHelper);

    onRenderPassFinished(RenderPassClosureReason::SyncObjectInit);

    // Mark the context as having a deferred flush.  This is later used to close the render pass and
    // cause a submission in this context if another context wants to wait on the fence while the
    // original context never issued a submission naturally.  Note that this also takes care of
    // contexts that think they issued a submission (through glFlush) but that the submission got
    // deferred.
    mHasDeferredFlush = true;

    return angle::Result::Continue;
}

angle::Result ContextVk::flushCommandsAndEndRenderPassIfDeferredSyncInit(
    RenderPassClosureReason reason)
{
    if (!mHasDeferredFlush)
    {
        return angle::Result::Continue;
    }

    // If we have deferred glFlush call in the middle of render pass, flush them now.
    return flushCommandsAndEndRenderPass(reason);
}

void ContextVk::addCommandBufferDiagnostics(const std::string &commandBufferDiagnostics)
{
    mCommandBufferDiagnostics.push_back(commandBufferDiagnostics);
}

void ContextVk::dumpCommandStreamDiagnostics()
{
    std::ostream &out = std::cout;

    if (mCommandBufferDiagnostics.empty())
        return;

    out << "digraph {\n" << "  node [shape=plaintext fontname=\"Consolas\"]\n";

    for (size_t index = 0; index < mCommandBufferDiagnostics.size(); ++index)
    {
        const std::string &payload = mCommandBufferDiagnostics[index];
        out << "  cb" << index << " [label =\"" << payload << "\"];\n";
    }

    for (size_t index = 0; index < mCommandBufferDiagnostics.size() - 1; ++index)
    {
        out << "  cb" << index << " -> cb" << index + 1 << "\n";
    }

    mCommandBufferDiagnostics.clear();

    out << "}\n";
}

void ContextVk::initIndexTypeMap()
{
    // Init gles-vulkan index type map
    mIndexTypeMap[gl::DrawElementsType::UnsignedByte] =
        mRenderer->getFeatures().supportsIndexTypeUint8.enabled ? VK_INDEX_TYPE_UINT8_EXT
                                                                : VK_INDEX_TYPE_UINT16;
    mIndexTypeMap[gl::DrawElementsType::UnsignedShort] = VK_INDEX_TYPE_UINT16;
    mIndexTypeMap[gl::DrawElementsType::UnsignedInt]   = VK_INDEX_TYPE_UINT32;
}

VkIndexType ContextVk::getVkIndexType(gl::DrawElementsType glIndexType) const
{
    return mIndexTypeMap[glIndexType];
}

size_t ContextVk::getVkIndexTypeSize(gl::DrawElementsType glIndexType) const
{
    gl::DrawElementsType elementsType = shouldConvertUint8VkIndexType(glIndexType)
                                            ? gl::DrawElementsType::UnsignedShort
                                            : glIndexType;
    ASSERT(elementsType < gl::DrawElementsType::EnumCount);

    // Use GetDrawElementsTypeSize() to get the size
    return static_cast<size_t>(gl::GetDrawElementsTypeSize(elementsType));
}

bool ContextVk::shouldConvertUint8VkIndexType(gl::DrawElementsType glIndexType) const
{
    return (glIndexType == gl::DrawElementsType::UnsignedByte &&
            !mRenderer->getFeatures().supportsIndexTypeUint8.enabled);
}

uint32_t GetDriverUniformSize(vk::ErrorContext *context, PipelineType pipelineType)
{
    if (pipelineType == PipelineType::Compute)
    {
        return sizeof(ComputeDriverUniforms);
    }

    ASSERT(pipelineType == PipelineType::Graphics);
    if (ShouldUseGraphicsDriverUniformsExtended(context))
    {
        return sizeof(GraphicsDriverUniformsExtended);
    }
    else
    {
        return sizeof(GraphicsDriverUniforms);
    }
}

angle::Result ContextVk::flushAndSubmitOutsideRenderPassCommands()
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ContextVk::flushAndSubmitOutsideRenderPassCommands");
    ANGLE_TRY(flushOutsideRenderPassCommands());
    return submitCommands(nullptr, nullptr, Submit::OutsideRenderPassCommandsOnly);
}

angle::Result ContextVk::flushOutsideRenderPassCommands()
{
    if (!mWaitSemaphores.empty())
    {
        ASSERT(mHasWaitSemaphoresPendingSubmission);
        ANGLE_TRY(mRenderer->flushWaitSemaphores(getProtectionType(), mContextPriority,
                                                 std::move(mWaitSemaphores),
                                                 std::move(mWaitSemaphoreStageMasks)));
    }
    ASSERT(mWaitSemaphores.empty());
    ASSERT(mWaitSemaphoreStageMasks.empty());

    if (mOutsideRenderPassCommands->empty())
    {
        return angle::Result::Continue;
    }
    ASSERT(mOutsideRenderPassCommands->getQueueSerial().valid());

    addOverlayUsedBuffersCount(mOutsideRenderPassCommands);

    if (kEnableCommandStreamDiagnostics)
    {
        addCommandBufferDiagnostics(mOutsideRenderPassCommands->getCommandDiagnostics());
    }

    flushDescriptorSetUpdates();

    // Track completion of this command buffer.
    mOutsideRenderPassCommands->flushSetEvents(this);
    mOutsideRenderPassCommands->collectRefCountedEventsGarbage(
        mShareGroupVk->getRefCountedEventsGarbageRecycler());

    // Save the queueSerial before calling flushOutsideRPCommands, which may return a new
    // mOutsideRenderPassCommands
    ASSERT(QueueSerialsHaveDifferentIndexOrSmaller(mLastFlushedQueueSerial,
                                                   mOutsideRenderPassCommands->getQueueSerial()));
    mLastFlushedQueueSerial = mOutsideRenderPassCommands->getQueueSerial();

    if (mOutsideRenderPassCommands->getAndResetHasHostVisibleBufferWrite())
    {
        mIsAnyHostVisibleBufferWritten = true;
    }
    ANGLE_TRY(mRenderer->flushOutsideRPCommands(this, getProtectionType(), mContextPriority,
                                                &mOutsideRenderPassCommands));

    // Make sure appropriate dirty bits are set, in case another thread makes a submission before
    // the next dispatch call.
    mComputeDirtyBits |= mNewComputeCommandBufferDirtyBits;
    mHasAnyCommandsPendingSubmission = true;
    mPerfCounters.flushedOutsideRenderPassCommandBuffers++;

    if (mRenderPassCommands->started() && mOutsideRenderPassSerialFactory.empty())
    {
        ANGLE_PERF_WARNING(
            getDebug(), GL_DEBUG_SEVERITY_HIGH,
            "Running out of reserved outsideRenderPass queueSerial. ending renderPass now.");
        // flushCommandsAndEndRenderPass will end up call back into this function again. We must
        // ensure mOutsideRenderPassCommands is empty so that it can early out.
        ASSERT(mOutsideRenderPassCommands->empty());
        // We used up all reserved serials. In order to maintain serial order (outsideRenderPass
        // must be smaller than render pass), we also endRenderPass here as well. This is not
        // expected to happen often in real world usage.
        return flushCommandsAndEndRenderPass(
            RenderPassClosureReason::OutOfReservedQueueSerialForOutsideCommands);
    }
    else
    {
        // Since queueSerial is used to decide if a resource is being used or not, we have to
        // generate a new queueSerial for outsideCommandBuffer since we just flushed
        // outsideRenderPassCommands.
        generateOutsideRenderPassCommandsQueueSerial();
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::beginRenderPassQuery(QueryVk *queryVk)
{
    gl::QueryType type = queryVk->getType();

    // Emit debug-util markers before calling the query command.
    ANGLE_TRY(handleGraphicsEventLog(rx::GraphicsEventCmdBuf::InRenderPassCmdBufQueryCmd));

    // To avoid complexity, we always start and end these queries inside the render pass.  If the
    // render pass has not yet started, the query is deferred until it does.
    if (mRenderPassCommandBuffer)
    {
        ANGLE_TRY(queryVk->getQueryHelper()->beginRenderPassQuery(this));
        // Remove the dirty bit since next draw call will have active query enabled
        if (getFeatures().preferSubmitOnAnySamplesPassedQueryEnd.enabled && IsAnySamplesQuery(type))
        {
            mGraphicsDirtyBits.reset(DIRTY_BIT_ANY_SAMPLE_PASSED_QUERY_END);
        }
    }

    // Update rasterizer discard emulation with primitives generated query if necessary.
    if (type == gl::QueryType::PrimitivesGenerated)
    {
        updateRasterizerDiscardEnabled(true);
    }

    ASSERT(mActiveRenderPassQueries[type] == nullptr);
    mActiveRenderPassQueries[type] = queryVk;

    return angle::Result::Continue;
}

angle::Result ContextVk::endRenderPassQuery(QueryVk *queryVk)
{
    gl::QueryType type = queryVk->getType();

    // Emit debug-util markers before calling the query command.
    ANGLE_TRY(handleGraphicsEventLog(rx::GraphicsEventCmdBuf::InRenderPassCmdBufQueryCmd));

    // End the query inside the render pass.  In some situations, the query may not have actually
    // been issued, so there is nothing to do there.  That is the case for transform feedback
    // queries which are deferred until a draw call with transform feedback active is issued, which
    // may have never happened.
    ASSERT(mRenderPassCommandBuffer == nullptr ||
           type == gl::QueryType::TransformFeedbackPrimitivesWritten || queryVk->hasQueryBegun());
    if (mRenderPassCommandBuffer && queryVk->hasQueryBegun())
    {
        queryVk->getQueryHelper()->endRenderPassQuery(this);
        // Set dirty bit so that we can detect and do something when a draw without active query is
        // issued.
        if (getFeatures().preferSubmitOnAnySamplesPassedQueryEnd.enabled && IsAnySamplesQuery(type))
        {
            mGraphicsDirtyBits.set(DIRTY_BIT_ANY_SAMPLE_PASSED_QUERY_END);
        }
    }

    // Update rasterizer discard emulation with primitives generated query if necessary.
    if (type == gl::QueryType::PrimitivesGenerated)
    {
        updateRasterizerDiscardEnabled(false);
    }

    ASSERT(mActiveRenderPassQueries[type] == queryVk);
    mActiveRenderPassQueries[type] = nullptr;

    return angle::Result::Continue;
}

void ContextVk::pauseRenderPassQueriesIfActive()
{
    for (QueryVk *activeQuery : mActiveRenderPassQueries)
    {
        if (activeQuery)
        {
            activeQuery->onRenderPassEnd(this);
            // No need to update rasterizer discard emulation with primitives generated query.  The
            // state will be updated when the next render pass starts.
        }
    }
}

angle::Result ContextVk::resumeRenderPassQueriesIfActive()
{
    // Note: these queries should be processed in order.  See comment in QueryVk::onRenderPassStart.
    for (QueryVk *activeQuery : mActiveRenderPassQueries)
    {
        if (activeQuery)
        {
            // Transform feedback queries are handled separately.
            if (activeQuery->getType() == gl::QueryType::TransformFeedbackPrimitivesWritten)
            {
                continue;
            }

            ANGLE_TRY(activeQuery->onRenderPassStart(this));

            // Update rasterizer discard emulation with primitives generated query if necessary.
            if (activeQuery->getType() == gl::QueryType::PrimitivesGenerated)
            {
                updateRasterizerDiscardEnabled(true);
            }
        }
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::resumeXfbRenderPassQueriesIfActive()
{
    // All other queries are handled separately.
    QueryVk *xfbQuery = mActiveRenderPassQueries[gl::QueryType::TransformFeedbackPrimitivesWritten];
    if (xfbQuery && mState.isTransformFeedbackActiveUnpaused())
    {
        ANGLE_TRY(xfbQuery->onRenderPassStart(this));
    }

    return angle::Result::Continue;
}

bool ContextVk::doesPrimitivesGeneratedQuerySupportRasterizerDiscard() const
{
    // If primitives generated is implemented with VK_EXT_primitives_generated_query, check the
    // corresponding feature bit.
    if (getFeatures().supportsPrimitivesGeneratedQuery.enabled)
    {
        return mRenderer->getPhysicalDevicePrimitivesGeneratedQueryFeatures()
                   .primitivesGeneratedQueryWithRasterizerDiscard == VK_TRUE;
    }

    // If primitives generated is emulated with pipeline statistics query, it's unknown on which
    // hardware rasterizer discard is supported.  Assume it's supported on none.
    if (getFeatures().supportsPipelineStatisticsQuery.enabled)
    {
        return false;
    }

    return true;
}

bool ContextVk::isEmulatingRasterizerDiscardDuringPrimitivesGeneratedQuery(
    bool isPrimitivesGeneratedQueryActive) const
{
    return isPrimitivesGeneratedQueryActive && mState.isRasterizerDiscardEnabled() &&
           !doesPrimitivesGeneratedQuerySupportRasterizerDiscard();
}

QueryVk *ContextVk::getActiveRenderPassQuery(gl::QueryType queryType) const
{
    return mActiveRenderPassQueries[queryType];
}

bool ContextVk::isRobustResourceInitEnabled() const
{
    return mState.isRobustResourceInitEnabled();
}

void ContextVk::setDefaultUniformBlocksMinSizeForTesting(size_t minSize)
{
    mDefaultUniformStorage.setMinimumSizeForTesting(minSize);
}

angle::Result ContextVk::initializeMultisampleTextureToBlack(const gl::Context *context,
                                                             gl::Texture *glTexture)
{
    ASSERT(glTexture->getType() == gl::TextureType::_2DMultisample);
    TextureVk *textureVk = vk::GetImpl(glTexture);

    return textureVk->initializeContentsWithBlack(context, GL_NONE,
                                                  gl::ImageIndex::Make2DMultisample());
}

void ContextVk::onProgramExecutableReset(ProgramExecutableVk *executableVk)
{
    // We can not check if executableVk deleted is what we was bound to, since by the time we get
    // here, the program executable in the context's state has already been updated.
    // Reset ContextVk::mCurrentGraphicsPipeline, since programInfo.release() freed the
    // PipelineHelper that it's currently pointing to.
    // TODO(http://anglebug.com/42264159): rework updateActiveTextures(), createPipelineLayout(),
    // handleDirtyGraphicsPipeline(), and ProgramPipelineVk::link().
    resetCurrentGraphicsPipeline();
    invalidateCurrentComputePipeline();
    invalidateCurrentGraphicsPipeline();
}

angle::Result ContextVk::switchToReadOnlyDepthStencilMode(gl::Texture *texture,
                                                          gl::Command command,
                                                          FramebufferVk *drawFramebuffer,
                                                          bool isStencilTexture)
{
    ASSERT(texture->isDepthOrStencil());

    // When running compute we don't have a draw FBO.
    if (command == gl::Command::Dispatch)
    {
        return angle::Result::Continue;
    }

    // The readOnlyDepth/StencilMode flag enables read-only depth-stencil feedback loops.  We only
    // switch to read-only mode when there's a loop.  The render pass tracks the depth and stencil
    // access modes, which indicates whether it's possible to retroactively go back and change the
    // attachment layouts to read-only.
    //
    // If there are any writes, the render pass needs to break, so that one using the read-only
    // layouts can start.
    FramebufferVk *drawFramebufferVk = getDrawFramebuffer();
    if (!texture->isBoundToFramebuffer(drawFramebufferVk->getState().getFramebufferSerial()))
    {
        return angle::Result::Continue;
    }

    if (isStencilTexture)
    {
        if (mState.isStencilWriteEnabled(mState.getDrawFramebuffer()->getStencilBitCount()))
        {
            // This looks like a feedback loop, but we don't issue a warning because the application
            // may have correctly used BASE and MAX levels to avoid it.  ANGLE doesn't track that.
            mDepthStencilAttachmentFlags.set(vk::RenderPassUsage::StencilFeedbackLoop);
        }
        else if (!mDepthStencilAttachmentFlags[vk::RenderPassUsage::StencilFeedbackLoop])
        {
            // If we are not in the actual feedback loop mode, switch to read-only stencil mode
            mDepthStencilAttachmentFlags.set(vk::RenderPassUsage::StencilReadOnlyAttachment);
        }
    }

    // Switch to read-only depth feedback loop if not already
    if (mState.isDepthWriteEnabled())
    {
        // This looks like a feedback loop, but we don't issue a warning because the application
        // may have correctly used BASE and MAX levels to avoid it.  ANGLE doesn't track that.
        mDepthStencilAttachmentFlags.set(vk::RenderPassUsage::DepthFeedbackLoop);
    }
    else if (!mDepthStencilAttachmentFlags[vk::RenderPassUsage::DepthFeedbackLoop])
    {
        // If we are not in the actual feedback loop mode, switch to read-only depth mode
        mDepthStencilAttachmentFlags.set(vk::RenderPassUsage::DepthReadOnlyAttachment);
    }

    if ((mDepthStencilAttachmentFlags & vk::kDepthStencilReadOnlyBits).none())
    {
        return angle::Result::Continue;
    }

    // If the aspect that's switching to read-only has a pending clear, it can't be done in the same
    // render pass (as the clear is a write operation).  In that case, flush the deferred clears for
    // the aspect that is turning read-only first.  The other deferred clears (such as color) can
    // stay deferred.
    if ((!isStencilTexture && drawFramebuffer->hasDeferredDepthClear()) ||
        (isStencilTexture && drawFramebuffer->hasDeferredStencilClear()))
    {
        ANGLE_TRY(drawFramebuffer->flushDepthStencilDeferredClear(
            this, isStencilTexture ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT));
    }

    // If the render pass needs closing, mark it as such.  Note that a write to depth/stencil may be
    // pending through a deferred clear.
    if (hasActiveRenderPass())
    {
        const vk::RenderPassUsage readOnlyAttachmentUsage =
            isStencilTexture ? vk::RenderPassUsage::StencilReadOnlyAttachment
                             : vk::RenderPassUsage::DepthReadOnlyAttachment;
        TextureVk *textureVk = vk::GetImpl(texture);

        if (!textureVk->getImage().hasRenderPassUsageFlag(readOnlyAttachmentUsage))
        {
            // If the render pass has written to this aspect, it needs to be closed.
            if ((!isStencilTexture && getStartedRenderPassCommands().hasDepthWriteOrClear()) ||
                (isStencilTexture && getStartedRenderPassCommands().hasStencilWriteOrClear()))
            {
                onRenderPassFinished(RenderPassClosureReason::DepthStencilUseInFeedbackLoop);

                // Don't let the render pass reactivate.
                mAllowRenderPassToReactivate = false;
            }
        }

        // Make sure to update the current render pass's tracking of read-only depth/stencil mode.
        mGraphicsDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_ACCESS);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::onResourceAccess(const vk::CommandBufferAccess &access)
{
    ANGLE_TRY(flushCommandBuffersIfNecessary(access));

    for (const vk::CommandBufferImageAccess &imageAccess : access.getReadImages())
    {
        vk::ImageHelper *image = imageAccess.image;
        ASSERT(!isRenderPassStartedAndUsesImage(*image));

        imageAccess.image->recordReadBarrier(this, imageAccess.aspectFlags, imageAccess.imageLayout,
                                             mOutsideRenderPassCommands);
        mOutsideRenderPassCommands->retainImage(image);
    }

    for (const vk::CommandBufferImageSubresourceAccess &imageReadAccess :
         access.getReadImageSubresources())
    {
        vk::ImageHelper *image = imageReadAccess.access.image;
        ASSERT(!isRenderPassStartedAndUsesImage(*image));

        image->recordReadSubresourceBarrier(
            this, imageReadAccess.access.aspectFlags, imageReadAccess.access.imageLayout,
            imageReadAccess.levelStart, imageReadAccess.levelCount, imageReadAccess.layerStart,
            imageReadAccess.layerCount, mOutsideRenderPassCommands);
        mOutsideRenderPassCommands->retainImage(image);
    }

    for (const vk::CommandBufferImageSubresourceAccess &imageWrite : access.getWriteImages())
    {
        vk::ImageHelper *image = imageWrite.access.image;
        ASSERT(!isRenderPassStartedAndUsesImage(*image));

        image->recordWriteBarrier(this, imageWrite.access.aspectFlags,
                                  imageWrite.access.imageLayout, imageWrite.levelStart,
                                  imageWrite.levelCount, imageWrite.layerStart,
                                  imageWrite.layerCount, mOutsideRenderPassCommands);
        mOutsideRenderPassCommands->retainImage(image);
        image->onWrite(imageWrite.levelStart, imageWrite.levelCount, imageWrite.layerStart,
                       imageWrite.layerCount, imageWrite.access.aspectFlags);
    }

    for (const vk::CommandBufferBufferAccess &bufferAccess : access.getReadBuffers())
    {
        ASSERT(!isRenderPassStartedAndUsesBufferForWrite(*bufferAccess.buffer));
        ASSERT(!mOutsideRenderPassCommands->usesBufferForWrite(*bufferAccess.buffer));

        mOutsideRenderPassCommands->bufferRead(this, bufferAccess.accessType, bufferAccess.stage,
                                               bufferAccess.buffer);
    }

    for (const vk::CommandBufferBufferAccess &bufferAccess : access.getWriteBuffers())
    {
        ASSERT(!isRenderPassStartedAndUsesBuffer(*bufferAccess.buffer));
        ASSERT(!mOutsideRenderPassCommands->usesBuffer(*bufferAccess.buffer));

        mOutsideRenderPassCommands->bufferWrite(this, bufferAccess.accessType, bufferAccess.stage,
                                                bufferAccess.buffer);
    }

    for (const vk::CommandBufferBufferExternalAcquireRelease &bufferAcquireRelease :
         access.getExternalAcquireReleaseBuffers())
    {
        mOutsideRenderPassCommands->retainResourceForWrite(bufferAcquireRelease.buffer);
    }

    for (const vk::CommandBufferResourceAccess &resourceAccess : access.getAccessResources())
    {
        mOutsideRenderPassCommands->retainResource(resourceAccess.resource);
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::flushCommandBuffersIfNecessary(const vk::CommandBufferAccess &access)
{
    // Go over resources and decide whether the render pass needs to close, whether the outside
    // render pass commands need to be flushed, or neither.  Note that closing the render pass
    // implies flushing the outside render pass as well, so if that needs to be done, we can close
    // the render pass and immediately return from this function.  Otherwise, this function keeps
    // track of whether the outside render pass commands need to be closed, and if so, it will do
    // that once at the end.

    // Read images only need to close the render pass if they need a layout transition.
    for (const vk::CommandBufferImageAccess &imageAccess : access.getReadImages())
    {
        // Note that different read methods are not compatible. A shader read uses a different
        // layout than a transfer read. So we cannot support simultaneous read usage as easily as
        // for Buffers.  TODO: Don't close the render pass if the image was only used read-only in
        // the render pass.  http://anglebug.com/42263557
        if (isRenderPassStartedAndUsesImage(*imageAccess.image))
        {
            return flushCommandsAndEndRenderPass(RenderPassClosureReason::ImageUseThenOutOfRPRead);
        }
    }

    // In cases where the image has both read and write permissions, the render pass should be
    // closed if there is a read from a previously written subresource (in a specific level/layer),
    // or a write to a previously read one.
    for (const vk::CommandBufferImageSubresourceAccess &imageSubresourceAccess :
         access.getReadImageSubresources())
    {
        if (isRenderPassStartedAndUsesImage(*imageSubresourceAccess.access.image))
        {
            return flushCommandsAndEndRenderPass(RenderPassClosureReason::ImageUseThenOutOfRPRead);
        }
    }

    // Write images only need to close the render pass if they need a layout transition.
    for (const vk::CommandBufferImageSubresourceAccess &imageWrite : access.getWriteImages())
    {
        if (isRenderPassStartedAndUsesImage(*imageWrite.access.image))
        {
            return flushCommandsAndEndRenderPass(RenderPassClosureReason::ImageUseThenOutOfRPWrite);
        }
    }

    bool shouldCloseOutsideRenderPassCommands = false;

    // Read buffers only need a new command buffer if previously used for write.
    for (const vk::CommandBufferBufferAccess &bufferAccess : access.getReadBuffers())
    {
        if (isRenderPassStartedAndUsesBufferForWrite(*bufferAccess.buffer))
        {
            return flushCommandsAndEndRenderPass(
                RenderPassClosureReason::BufferWriteThenOutOfRPRead);
        }
        else if (mOutsideRenderPassCommands->usesBufferForWrite(*bufferAccess.buffer))
        {
            shouldCloseOutsideRenderPassCommands = true;
        }
    }

    // Write buffers always need a new command buffer if previously used.
    for (const vk::CommandBufferBufferAccess &bufferAccess : access.getWriteBuffers())
    {
        if (isRenderPassStartedAndUsesBuffer(*bufferAccess.buffer))
        {
            return flushCommandsAndEndRenderPass(
                RenderPassClosureReason::BufferUseThenOutOfRPWrite);
        }
        else if (mOutsideRenderPassCommands->usesBuffer(*bufferAccess.buffer))
        {
            shouldCloseOutsideRenderPassCommands = true;
        }
    }

    if (shouldCloseOutsideRenderPassCommands)
    {
        return flushOutsideRenderPassCommands();
    }

    return angle::Result::Continue;
}

angle::Result ContextVk::endRenderPassIfComputeReadAfterTransformFeedbackWrite()
{
    // Similar to flushCommandBuffersIfNecessary(), but using uniform buffers currently bound and
    // used by the current (compute) program.  This is to handle read-after-write hazards where the
    // write originates from transform feedback.
    if (!mCurrentTransformFeedbackQueueSerial.valid())
    {
        return angle::Result::Continue;
    }

    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable && executable->hasLinkedShaderStage(gl::ShaderType::Compute));

    // Uniform buffers:
    const std::vector<gl::InterfaceBlock> &blocks = executable->getUniformBlocks();

    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
    {
        const GLuint binding = executable->getUniformBlockBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            mState.getIndexedUniformBuffer(binding);

        if (bufferBinding.get() == nullptr)
        {
            continue;
        }

        vk::BufferHelper &buffer = vk::GetImpl(bufferBinding.get())->getBuffer();
        if (buffer.writtenByCommandBuffer(mCurrentTransformFeedbackQueueSerial))
        {
            return flushCommandsAndEndRenderPass(RenderPassClosureReason::XfbWriteThenComputeRead);
        }
    }

    return angle::Result::Continue;
}

// When textures/images bound/used by current compute program and have been accessed
// as sampled texture in current render pass, need to take care the implicit layout
// transition of these textures/images in the render pass.
angle::Result ContextVk::endRenderPassIfComputeAccessAfterGraphicsImageAccess()
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    ASSERT(executable && executable->hasLinkedShaderStage(gl::ShaderType::Compute));

    for (size_t imageUnitIndex : executable->getActiveImagesMask())
    {
        const gl::Texture *texture = mState.getImageUnit(imageUnitIndex).texture.get();
        if (texture == nullptr)
        {
            continue;
        }

        TextureVk *textureVk = vk::GetImpl(texture);

        if (texture->getType() == gl::TextureType::Buffer)
        {
            continue;
        }
        else
        {
            vk::ImageHelper &image = textureVk->getImage();

            // This is to handle the implicit layout transition in render pass of this image,
            // while it currently be bound and used by current compute program.
            if (mRenderPassCommands->startedAndUsesImageWithBarrier(image))
            {
                return flushCommandsAndEndRenderPass(
                    RenderPassClosureReason::GraphicsTextureImageAccessThenComputeAccess);
            }
        }
    }

    const gl::ActiveTexturesCache &textures        = mState.getActiveTexturesCache();
    const gl::ActiveTextureTypeArray &textureTypes = executable->getActiveSamplerTypes();

    for (size_t textureUnit : executable->getActiveSamplersMask())
    {
        gl::Texture *texture        = textures[textureUnit];
        gl::TextureType textureType = textureTypes[textureUnit];

        if (texture == nullptr || textureType == gl::TextureType::Buffer)
        {
            continue;
        }

        TextureVk *textureVk = vk::GetImpl(texture);
        ASSERT(textureVk != nullptr);
        vk::ImageHelper &image = textureVk->getImage();

        // Similar to flushCommandBuffersIfNecessary(), but using textures currently bound and used
        // by the current (compute) program.  This is to handle read-after-write hazards where the
        // write originates from a framebuffer attachment.
        if (image.hasRenderPassUsageFlag(vk::RenderPassUsage::RenderTargetAttachment) &&
            isRenderPassStartedAndUsesImage(image))
        {
            return flushCommandsAndEndRenderPass(
                RenderPassClosureReason::ImageAttachmentThenComputeRead);
        }

        // Take care of the read image layout transition require implicit synchronization.
        if (mRenderPassCommands->startedAndUsesImageWithBarrier(image))
        {
            return flushCommandsAndEndRenderPass(
                RenderPassClosureReason::GraphicsTextureImageAccessThenComputeAccess);
        }
    }

    return angle::Result::Continue;
}

const angle::PerfMonitorCounterGroups &ContextVk::getPerfMonitorCounters()
{
    syncObjectPerfCounters(mRenderer->getCommandQueuePerfCounters());

    angle::PerfMonitorCounters &counters =
        angle::GetPerfMonitorCounterGroup(mPerfMonitorCounters, "vulkan").counters;

#define ANGLE_UPDATE_PERF_MAP(COUNTER) \
    angle::GetPerfMonitorCounter(counters, #COUNTER).value = mPerfCounters.COUNTER;

    ANGLE_VK_PERF_COUNTERS_X(ANGLE_UPDATE_PERF_MAP)

#undef ANGLE_UPDATE_PERF_MAP

    return mPerfMonitorCounters;
}

angle::Result ContextVk::switchToColorFramebufferFetchMode(bool hasColorFramebufferFetch)
{
    ASSERT(!getFeatures().preferDynamicRendering.enabled);

    // If framebuffer fetch is permanent, make sure we never switch out of it.
    if (getFeatures().permanentlySwitchToFramebufferFetchMode.enabled &&
        mIsInColorFramebufferFetchMode)
    {
        return angle::Result::Continue;
    }

    ASSERT(mIsInColorFramebufferFetchMode != hasColorFramebufferFetch);
    mIsInColorFramebufferFetchMode = hasColorFramebufferFetch;

    // If a render pass is already open, close it.
    if (mRenderPassCommands->started())
    {
        ANGLE_TRY(
            flushCommandsAndEndRenderPass(RenderPassClosureReason::FramebufferFetchEmulation));
    }

    // If there's a draw buffer bound, switch it to framebuffer fetch mode.  Every other framebuffer
    // will switch when bound.
    if (mState.getDrawFramebuffer() != nullptr)
    {
        getDrawFramebuffer()->switchToColorFramebufferFetchMode(this,
                                                                mIsInColorFramebufferFetchMode);
    }

    // Clear the render pass cache; all render passes will be incompatible from now on with the
    // old ones.
    if (getFeatures().permanentlySwitchToFramebufferFetchMode.enabled)
    {
        mRenderPassCache.clear(this);
    }

    mRenderer->onColorFramebufferFetchUse();

    return angle::Result::Continue;
}

void ContextVk::onFramebufferFetchUse(vk::FramebufferFetchMode framebufferFetchMode)
{
    ASSERT(getFeatures().preferDynamicRendering.enabled);

    if (mRenderPassCommands->started())
    {
        // Accumulate framebuffer fetch mode to allow multiple draw calls in the same render pass
        // where some use color framebuffer fetch and some depth/stencil
        const vk::FramebufferFetchMode mergedMode = vk::FramebufferFetchModeMerge(
            mRenderPassCommands->getRenderPassDesc().framebufferFetchMode(), framebufferFetchMode);

        mRenderPassCommands->setFramebufferFetchMode(mergedMode);

        // When framebuffer fetch is enabled, attachments can be read from even if output is
        // masked, so update their access.
        if (FramebufferFetchModeHasColor(framebufferFetchMode))
        {
            onColorAccessChange();
        }
        if (FramebufferFetchModeHasDepthStencil(framebufferFetchMode))
        {
            onDepthStencilAccessChange();
        }
    }

    if (FramebufferFetchModeHasColor(framebufferFetchMode))
    {
        mRenderer->onColorFramebufferFetchUse();
    }
}

ANGLE_INLINE angle::Result ContextVk::allocateQueueSerialIndex()
{
    ASSERT(mCurrentQueueSerialIndex == kInvalidQueueSerialIndex);
    // Make everything appears to be flushed and submitted
    ANGLE_TRY(mRenderer->allocateQueueSerialIndex(&mCurrentQueueSerialIndex));
    // Note queueSerial for render pass is deferred until begin time.
    generateOutsideRenderPassCommandsQueueSerial();
    return angle::Result::Continue;
}

ANGLE_INLINE void ContextVk::releaseQueueSerialIndex()
{
    ASSERT(mCurrentQueueSerialIndex != kInvalidQueueSerialIndex);
    mRenderer->releaseQueueSerialIndex(mCurrentQueueSerialIndex);
    mCurrentQueueSerialIndex = kInvalidQueueSerialIndex;
}

ANGLE_INLINE void ContextVk::generateOutsideRenderPassCommandsQueueSerial()
{
    ASSERT(mCurrentQueueSerialIndex != kInvalidQueueSerialIndex);

    // If there is reserved serial number, use that. Otherwise generate a new one.
    Serial serial;
    if (mOutsideRenderPassSerialFactory.generate(&serial))
    {
        ASSERT(mRenderPassCommands->getQueueSerial().valid());
        ASSERT(mRenderPassCommands->getQueueSerial().getSerial() > serial);
        mOutsideRenderPassCommands->setQueueSerial(mCurrentQueueSerialIndex, serial);
        return;
    }

    serial = mRenderer->generateQueueSerial(mCurrentQueueSerialIndex);
    mOutsideRenderPassCommands->setQueueSerial(mCurrentQueueSerialIndex, serial);
}

ANGLE_INLINE void ContextVk::generateRenderPassCommandsQueueSerial(QueueSerial *queueSerialOut)
{
    ASSERT(mCurrentQueueSerialIndex != kInvalidQueueSerialIndex);

    // We reserve some serial number for outsideRenderPassCommands in case we have to flush.
    ASSERT(mOutsideRenderPassCommands->getQueueSerial().valid());
    mRenderer->reserveQueueSerials(mCurrentQueueSerialIndex,
                                   kMaxReservedOutsideRenderPassQueueSerials,
                                   &mOutsideRenderPassSerialFactory);

    Serial serial   = mRenderer->generateQueueSerial(mCurrentQueueSerialIndex);
    *queueSerialOut = QueueSerial(mCurrentQueueSerialIndex, serial);
}

void ContextVk::resetPerFramePerfCounters()
{
    mPerfCounters.renderPasses                           = 0;
    mPerfCounters.writeDescriptorSets                    = 0;
    mPerfCounters.flushedOutsideRenderPassCommandBuffers = 0;
    mPerfCounters.resolveImageCommands                   = 0;
    mPerfCounters.descriptorSetAllocations               = 0;

    mRenderer->resetCommandQueuePerFrameCounters();

    mShareGroupVk->getMetaDescriptorPools()[DescriptorSetIndex::UniformsAndXfb]
        .resetDescriptorCacheStats();
    mShareGroupVk->getMetaDescriptorPools()[DescriptorSetIndex::Texture]
        .resetDescriptorCacheStats();
    mShareGroupVk->getMetaDescriptorPools()[DescriptorSetIndex::ShaderResource]
        .resetDescriptorCacheStats();
}

angle::Result ContextVk::ensureInterfacePipelineCache()
{
    if (!mInterfacePipelinesCache.valid())
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        ANGLE_VK_TRY(this, mInterfacePipelinesCache.init(getDevice(), pipelineCacheCreateInfo));
    }

    return angle::Result::Continue;
}
}  // namespace rx
