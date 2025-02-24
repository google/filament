//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferD3D.cpp: Implements the DefaultAttachmentD3D and FramebufferD3D classes.

#include "libANGLE/renderer/d3d/FramebufferD3D.h"

#include "common/bitset_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Surface.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/RenderbufferD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/SurfaceD3D.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"

namespace rx
{

namespace
{

ClearParameters GetClearParameters(const gl::State &state, GLbitfield mask)
{
    ClearParameters clearParams;
    memset(&clearParams, 0, sizeof(ClearParameters));

    clearParams.colorF           = state.getColorClearValue();
    clearParams.colorType        = GL_FLOAT;
    clearParams.clearDepth       = false;
    clearParams.depthValue       = state.getDepthClearValue();
    clearParams.clearStencil     = false;
    clearParams.stencilValue     = state.getStencilClearValue();
    clearParams.stencilWriteMask = state.getDepthStencilState().stencilWritemask;

    const auto *framebufferObject      = state.getDrawFramebuffer();
    const gl::Extents &framebufferSize = framebufferObject->getFirstNonNullAttachment()->getSize();
    const gl::Offset &surfaceTextureOffset = framebufferObject->getSurfaceTextureOffset();
    if (state.isScissorTestEnabled())
    {
        clearParams.scissorEnabled = true;
        clearParams.scissor        = state.getScissor();
        clearParams.scissor.x      = clearParams.scissor.x + surfaceTextureOffset.x;
        clearParams.scissor.y      = clearParams.scissor.y + surfaceTextureOffset.y;
    }
    else if (surfaceTextureOffset != gl::kOffsetZero)
    {
        clearParams.scissorEnabled = true;
        clearParams.scissor        = gl::Rectangle(surfaceTextureOffset.x, surfaceTextureOffset.y,
                                                   framebufferSize.width, framebufferSize.height);
    }

    const bool clearColor =
        (mask & GL_COLOR_BUFFER_BIT) && framebufferObject->hasEnabledDrawBuffer();
    if (clearColor)
    {
        clearParams.clearColor.set();
    }
    else
    {
        clearParams.clearColor.reset();
    }
    clearParams.colorMask = state.getBlendStateExt().getColorMaskBits();

    if (mask & GL_DEPTH_BUFFER_BIT)
    {
        if (state.getDepthStencilState().depthMask &&
            framebufferObject->getDepthAttachment() != nullptr)
        {
            clearParams.clearDepth = true;
        }
    }

    if (mask & GL_STENCIL_BUFFER_BIT)
    {
        if (framebufferObject->getStencilAttachment() != nullptr &&
            framebufferObject->getStencilAttachment()->getStencilSize() > 0)
        {
            clearParams.clearStencil = true;
        }
    }

    return clearParams;
}
}  // namespace

ClearParameters::ClearParameters() = default;

ClearParameters::ClearParameters(const ClearParameters &other) = default;

FramebufferD3D::FramebufferD3D(const gl::FramebufferState &data, RendererD3D *renderer)
    : FramebufferImpl(data), mRenderer(renderer), mMockAttachment()
{}

FramebufferD3D::~FramebufferD3D() {}

angle::Result FramebufferD3D::clear(const gl::Context *context, GLbitfield mask)
{
    ClearParameters clearParams = GetClearParameters(context->getState(), mask);
    return clearImpl(context, clearParams);
}

angle::Result FramebufferD3D::clearBufferfv(const gl::Context *context,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            const GLfloat *values)
{
    // glClearBufferfv can be called to clear the color buffer or depth buffer
    ClearParameters clearParams = GetClearParameters(context->getState(), 0);

    if (buffer == GL_COLOR)
    {
        for (unsigned int i = 0; i < clearParams.clearColor.size(); i++)
        {
            clearParams.clearColor[i] = (drawbuffer == static_cast<int>(i));
        }
        clearParams.colorF    = gl::ColorF(values[0], values[1], values[2], values[3]);
        clearParams.colorType = GL_FLOAT;
    }

    if (buffer == GL_DEPTH)
    {
        clearParams.clearDepth = true;
        clearParams.depthValue = values[0];
    }

    return clearImpl(context, clearParams);
}

angle::Result FramebufferD3D::clearBufferuiv(const gl::Context *context,
                                             GLenum buffer,
                                             GLint drawbuffer,
                                             const GLuint *values)
{
    // glClearBufferuiv can only be called to clear a color buffer
    ClearParameters clearParams = GetClearParameters(context->getState(), 0);
    for (unsigned int i = 0; i < clearParams.clearColor.size(); i++)
    {
        clearParams.clearColor[i] = (drawbuffer == static_cast<int>(i));
    }
    clearParams.colorUI   = gl::ColorUI(values[0], values[1], values[2], values[3]);
    clearParams.colorType = GL_UNSIGNED_INT;

    return clearImpl(context, clearParams);
}

angle::Result FramebufferD3D::clearBufferiv(const gl::Context *context,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            const GLint *values)
{
    // glClearBufferiv can be called to clear the color buffer or stencil buffer
    ClearParameters clearParams = GetClearParameters(context->getState(), 0);

    if (buffer == GL_COLOR)
    {
        for (unsigned int i = 0; i < clearParams.clearColor.size(); i++)
        {
            clearParams.clearColor[i] = (drawbuffer == static_cast<int>(i));
        }
        clearParams.colorI    = gl::ColorI(values[0], values[1], values[2], values[3]);
        clearParams.colorType = GL_INT;
    }

    if (buffer == GL_STENCIL)
    {
        clearParams.clearStencil = true;
        clearParams.stencilValue = values[0];
    }

    return clearImpl(context, clearParams);
}

angle::Result FramebufferD3D::clearBufferfi(const gl::Context *context,
                                            GLenum buffer,
                                            GLint drawbuffer,
                                            GLfloat depth,
                                            GLint stencil)
{
    // glClearBufferfi can only be called to clear a depth stencil buffer
    ClearParameters clearParams = GetClearParameters(context->getState(), 0);
    clearParams.clearDepth      = true;
    clearParams.depthValue      = depth;
    clearParams.clearStencil    = true;
    clearParams.stencilValue    = stencil;

    return clearImpl(context, clearParams);
}

angle::Result FramebufferD3D::readPixels(const gl::Context *context,
                                         const gl::Rectangle &area,
                                         GLenum format,
                                         GLenum type,
                                         const gl::PixelPackState &pack,
                                         gl::Buffer *packBuffer,
                                         void *pixels)
{
    // Clip read area to framebuffer.
    const gl::Extents fbSize = getState().getReadPixelsAttachment(format)->getSize();
    const gl::Rectangle fbRect(0, 0, fbSize.width, fbSize.height);
    gl::Rectangle clippedArea;
    if (!ClipRectangle(area, fbRect, &clippedArea))
    {
        // nothing to read
        return angle::Result::Continue;
    }

    const gl::InternalFormat &sizedFormatInfo = gl::GetInternalFormatInfo(format, type);

    ContextD3D *contextD3D = GetImplAs<ContextD3D>(context);

    GLuint outputPitch = 0;
    ANGLE_CHECK_GL_MATH(contextD3D,
                        sizedFormatInfo.computeRowPitch(type, area.width, pack.alignment,
                                                        pack.rowLength, &outputPitch));

    GLuint outputSkipBytes = 0;
    ANGLE_CHECK_GL_MATH(contextD3D, sizedFormatInfo.computeSkipBytes(type, outputPitch, 0, pack,
                                                                     false, &outputSkipBytes));
    outputSkipBytes += (clippedArea.x - area.x) * sizedFormatInfo.pixelBytes +
                       (clippedArea.y - area.y) * outputPitch;

    return readPixelsImpl(context, clippedArea, format, type, outputPitch, pack, packBuffer,
                          static_cast<uint8_t *>(pixels) + outputSkipBytes);
}

angle::Result FramebufferD3D::blit(const gl::Context *context,
                                   const gl::Rectangle &sourceArea,
                                   const gl::Rectangle &destArea,
                                   GLbitfield mask,
                                   GLenum filter)
{
    const auto &glState                      = context->getState();
    const gl::Framebuffer *sourceFramebuffer = glState.getReadFramebuffer();
    const gl::Rectangle *scissor = glState.isScissorTestEnabled() ? &glState.getScissor() : nullptr;
    ANGLE_TRY(blitImpl(context, sourceArea, destArea, scissor, (mask & GL_COLOR_BUFFER_BIT) != 0,
                       (mask & GL_DEPTH_BUFFER_BIT) != 0, (mask & GL_STENCIL_BUFFER_BIT) != 0,
                       filter, sourceFramebuffer));

    return angle::Result::Continue;
}

gl::FramebufferStatus FramebufferD3D::checkStatus(const gl::Context *context) const
{
    // if we have both a depth and stencil buffer, they must refer to the same object
    // since we only support packed_depth_stencil and not separate depth and stencil
    if (mState.hasSeparateDepthAndStencilAttachments())
    {
        return gl::FramebufferStatus::Incomplete(
            GL_FRAMEBUFFER_UNSUPPORTED,
            gl::err::kFramebufferIncompleteUnsupportedSeparateDepthStencilBuffers);
    }

    // D3D11 does not allow for overlapping RenderTargetViews.
    // If WebGL compatibility is enabled, this has already been checked at a higher level.
    ASSERT(!context->isWebGL() || mState.colorAttachmentsAreUniqueImages());
    if (!context->isWebGL())
    {
        if (!mState.colorAttachmentsAreUniqueImages())
        {
            return gl::FramebufferStatus::Incomplete(
                GL_FRAMEBUFFER_UNSUPPORTED,
                gl::err::kFramebufferIncompleteUnsupportedNonUniqueAttachments);
        }
    }

    // D3D requires all render targets to have the same dimensions.
    if (!mState.attachmentsHaveSameDimensions())
    {
        return gl::FramebufferStatus::Incomplete(
            GL_FRAMEBUFFER_UNSUPPORTED,
            gl::err::kFramebufferIncompleteUnsupportedMissmatchedDimensions);
    }

    return gl::FramebufferStatus::Complete();
}

angle::Result FramebufferD3D::syncState(const gl::Context *context,
                                        GLenum binding,
                                        const gl::Framebuffer::DirtyBits &dirtyBits,
                                        gl::Command command)
{
    if (!mColorAttachmentsForRender.valid())
    {
        return angle::Result::Continue;
    }

    for (auto dirtyBit : dirtyBits)
    {
        if ((dirtyBit >= gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_0 &&
             dirtyBit < gl::Framebuffer::DIRTY_BIT_COLOR_ATTACHMENT_MAX) ||
            dirtyBit == gl::Framebuffer::DIRTY_BIT_DRAW_BUFFERS)
        {
            mColorAttachmentsForRender.reset();
        }
    }

    return angle::Result::Continue;
}

const gl::AttachmentList &FramebufferD3D::getColorAttachmentsForRender(const gl::Context *context)
{
    gl::DrawBufferMask activeProgramOutputs =
        context->getState().getProgramExecutable()->getActiveOutputVariablesMask();

    if (mColorAttachmentsForRender.valid() && mCurrentActiveProgramOutputs == activeProgramOutputs)
    {
        return mColorAttachmentsForRender.value();
    }

    // Does not actually free memory
    gl::AttachmentList colorAttachmentsForRender;
    mColorAttachmentsForRenderMask.reset();

    const auto &colorAttachments = mState.getColorAttachments();
    const auto &drawBufferStates = mState.getDrawBufferStates();
    const auto &features         = mRenderer->getFeatures();

    for (size_t attachmentIndex = 0; attachmentIndex < colorAttachments.size(); ++attachmentIndex)
    {
        GLenum drawBufferState                           = drawBufferStates[attachmentIndex];
        const gl::FramebufferAttachment &colorAttachment = colorAttachments[attachmentIndex];

        if (colorAttachment.isAttached() && drawBufferState != GL_NONE &&
            activeProgramOutputs[attachmentIndex])
        {
            ASSERT(drawBufferState == GL_BACK ||
                   drawBufferState == (GL_COLOR_ATTACHMENT0_EXT + attachmentIndex));
            colorAttachmentsForRender.push_back(&colorAttachment);
            mColorAttachmentsForRenderMask.set(attachmentIndex);
        }
        else if (!features.mrtPerfWorkaround.enabled)
        {
            colorAttachmentsForRender.push_back(nullptr);
            mColorAttachmentsForRenderMask.set(attachmentIndex);
        }
    }

    // When rendering with no render target on D3D, two bugs lead to incorrect behavior on Intel
    // drivers < 4815. The rendering samples always pass neglecting discard statements in pixel
    // shader. We add a mock texture as render target in such case.
    if (mRenderer->getFeatures().addMockTextureNoRenderTarget.enabled &&
        colorAttachmentsForRender.empty() && activeProgramOutputs.any())
    {
        static_assert(static_cast<size_t>(activeProgramOutputs.size()) <= 32,
                      "Size of active program outputs should less or equal than 32.");
        const GLuint activeProgramLocation = static_cast<GLuint>(
            gl::ScanForward(static_cast<uint32_t>(activeProgramOutputs.bits())));

        if (mMockAttachment.isAttached() &&
            (mMockAttachment.getBinding() - GL_COLOR_ATTACHMENT0) == activeProgramLocation)
        {
            colorAttachmentsForRender.push_back(&mMockAttachment);
        }
        else
        {
            // Remove mock attachment to prevents us from leaking it, and the program may require
            // it to be attached to a new binding point.
            if (mMockAttachment.isAttached())
            {
                mMockAttachment.detach(context, UniqueSerial());
            }

            gl::Texture *mockTex = nullptr;
            // TODO(jmadill): Handle error if mock texture can't be created.
            (void)mRenderer->getIncompleteTexture(context, gl::TextureType::_2D, &mockTex);
            if (mockTex)
            {
                gl::ImageIndex index = gl::ImageIndex::Make2D(0);
                mMockAttachment      = gl::FramebufferAttachment(
                    context, GL_TEXTURE, GL_COLOR_ATTACHMENT0_EXT + activeProgramLocation, index,
                    mockTex, UniqueSerial());
                colorAttachmentsForRender.push_back(&mMockAttachment);
            }
        }
    }

    mColorAttachmentsForRender   = std::move(colorAttachmentsForRender);
    mCurrentActiveProgramOutputs = activeProgramOutputs;

    return mColorAttachmentsForRender.value();
}

void FramebufferD3D::destroy(const gl::Context *context)
{
    if (mMockAttachment.isAttached())
    {
        mMockAttachment.detach(context, UniqueSerial());
    }
}

}  // namespace rx
