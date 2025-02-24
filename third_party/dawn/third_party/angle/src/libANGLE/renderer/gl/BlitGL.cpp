//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BlitGL.cpp: Implements the BlitGL class, a helper for blitting textures

#include "libANGLE/renderer/gl/BlitGL.h"

#include "common/FixedVector.h"
#include "common/utilities.h"
#include "common/vector_utils.h"
#include "image_util/copyimage.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RenderbufferGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/formatutilsgl.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "platform/autogen/FeaturesGL_autogen.h"

using angle::Vector2;

namespace rx
{

namespace
{

angle::Result CheckCompileStatus(const gl::Context *context,
                                 const rx::FunctionsGL *functions,
                                 GLuint shader)
{
    GLint compileStatus = GL_FALSE;
    ANGLE_GL_TRY(context, functions->getShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));

    ASSERT(compileStatus == GL_TRUE);
    ANGLE_CHECK(GetImplAs<ContextGL>(context), compileStatus == GL_TRUE,
                "Failed to compile internal blit shader.", GL_OUT_OF_MEMORY);

    return angle::Result::Continue;
}

angle::Result CheckLinkStatus(const gl::Context *context,
                              const rx::FunctionsGL *functions,
                              GLuint program)
{
    GLint linkStatus = GL_FALSE;
    ANGLE_GL_TRY(context, functions->getProgramiv(program, GL_LINK_STATUS, &linkStatus));
    ASSERT(linkStatus == GL_TRUE);
    ANGLE_CHECK(GetImplAs<ContextGL>(context), linkStatus == GL_TRUE,
                "Failed to link internal blit program.", GL_OUT_OF_MEMORY);

    return angle::Result::Continue;
}

class [[nodiscard]] ScopedGLState : angle::NonCopyable
{
  public:
    enum
    {
        KEEP_SCISSOR = 1,
    };

    ScopedGLState() {}

    ~ScopedGLState() { ASSERT(mExited); }

    angle::Result enter(const gl::Context *context, gl::Rectangle viewport, int keepState = 0)
    {
        ContextGL *contextGL         = GetImplAs<ContextGL>(context);
        StateManagerGL *stateManager = contextGL->getStateManager();

        if (!(keepState & KEEP_SCISSOR))
        {
            stateManager->setScissorTestEnabled(false);
        }
        stateManager->setViewport(viewport);
        stateManager->setDepthRange(0.0f, 1.0f);
        stateManager->setClipControl(gl::ClipOrigin::LowerLeft,
                                     gl::ClipDepthMode::NegativeOneToOne);
        stateManager->setClipDistancesEnable(gl::ClipDistanceEnableBits());
        stateManager->setDepthClampEnabled(false);
        stateManager->setBlendEnabled(false);
        stateManager->setColorMask(true, true, true, true);
        stateManager->setSampleAlphaToCoverageEnabled(false);
        stateManager->setSampleCoverageEnabled(false);
        stateManager->setDepthTestEnabled(false);
        stateManager->setStencilTestEnabled(false);
        stateManager->setCullFaceEnabled(false);
        stateManager->setPolygonMode(gl::PolygonMode::Fill);
        stateManager->setPolygonOffsetPointEnabled(false);
        stateManager->setPolygonOffsetLineEnabled(false);
        stateManager->setPolygonOffsetFillEnabled(false);
        stateManager->setRasterizerDiscardEnabled(false);
        stateManager->setLogicOpEnabled(false);

        stateManager->pauseTransformFeedback();
        return stateManager->pauseAllQueries(context);
    }

    angle::Result exit(const gl::Context *context)
    {
        mExited = true;

        ContextGL *contextGL         = GetImplAs<ContextGL>(context);
        StateManagerGL *stateManager = contextGL->getStateManager();

        // XFB resuming will be done automatically
        return stateManager->resumeAllQueries(context);
    }

    void willUseTextureUnit(const gl::Context *context, int unit)
    {
        ContextGL *contextGL = GetImplAs<ContextGL>(context);

        if (contextGL->getFunctions()->bindSampler)
        {
            contextGL->getStateManager()->bindSampler(unit, 0);
        }
    }

  private:
    bool mExited = false;
};

angle::Result SetClearState(StateManagerGL *stateManager,
                            bool colorClear,
                            bool depthClear,
                            bool stencilClear,
                            GLbitfield *outClearMask)
{
    *outClearMask = 0;
    if (colorClear)
    {
        stateManager->setClearColor(gl::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
        stateManager->setColorMask(true, true, true, true);
        *outClearMask |= GL_COLOR_BUFFER_BIT;
    }
    if (depthClear)
    {
        stateManager->setDepthMask(true);
        stateManager->setClearDepth(1.0f);
        *outClearMask |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencilClear)
    {
        stateManager->setClearStencil(0);
        *outClearMask |= GL_STENCIL_BUFFER_BIT;
    }

    stateManager->setScissorTestEnabled(false);

    return angle::Result::Continue;
}

using ClearBindTargetVector = angle::FixedVector<GLenum, 3>;

angle::Result PrepareForClear(StateManagerGL *stateManager,
                              GLenum sizedInternalFormat,
                              ClearBindTargetVector *outBindtargets,
                              ClearBindTargetVector *outUnbindTargets,
                              GLbitfield *outClearMask)
{
    const gl::InternalFormat &internalFormatInfo =
        gl::GetSizedInternalFormatInfo(sizedInternalFormat);
    bool bindDepth   = internalFormatInfo.depthBits > 0;
    bool bindStencil = internalFormatInfo.stencilBits > 0;
    bool bindColor   = !bindDepth && !bindStencil;

    outBindtargets->clear();
    if (bindColor)
    {
        outBindtargets->push_back(GL_COLOR_ATTACHMENT0);
    }
    else
    {
        outUnbindTargets->push_back(GL_COLOR_ATTACHMENT0);
    }
    if (bindDepth)
    {
        outBindtargets->push_back(GL_DEPTH_ATTACHMENT);
    }
    else
    {
        outUnbindTargets->push_back(GL_DEPTH_ATTACHMENT);
    }
    if (bindStencil)
    {
        outBindtargets->push_back(GL_STENCIL_ATTACHMENT);
    }
    else
    {
        outUnbindTargets->push_back(GL_STENCIL_ATTACHMENT);
    }

    ANGLE_TRY(SetClearState(stateManager, bindColor, bindDepth, bindStencil, outClearMask));

    return angle::Result::Continue;
}

angle::Result UnbindAttachment(const gl::Context *context,
                               const FunctionsGL *functions,
                               GLenum framebufferTarget,
                               GLenum attachment)
{
    // Always use framebufferTexture2D as a workaround for an Nvidia driver bug. See
    // https://anglebug.com/42264072 and FeaturesGL.alwaysUnbindFramebufferTexture2D
    ANGLE_GL_TRY(context,
                 functions->framebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0));

    return angle::Result::Continue;
}

angle::Result UnbindAttachments(const gl::Context *context,
                                const FunctionsGL *functions,
                                GLenum framebufferTarget,
                                const ClearBindTargetVector &bindTargets)
{
    for (GLenum bindTarget : bindTargets)
    {
        ANGLE_TRY(UnbindAttachment(context, functions, framebufferTarget, bindTarget));
    }
    return angle::Result::Continue;
}

angle::Result CheckIfAttachmentNeedsClearing(const gl::Context *context,
                                             const gl::FramebufferAttachment *attachment,
                                             bool *needsClearInit)
{
    if (attachment->initState() == gl::InitState::Initialized)
    {
        *needsClearInit = false;
        return angle::Result::Continue;
    }

    // Special case for 2D array and 3D textures. The init state tracks initialization for all
    // layers but only one will be cleared by a clear call. Initialize those entire textures
    // here.
    if (attachment->type() == GL_TEXTURE &&
        (attachment->getTextureImageIndex().getTarget() == gl::TextureTarget::_2DArray ||
         attachment->getTextureImageIndex().getTarget() == gl::TextureTarget::_3D))
    {
        ANGLE_TRY(attachment->initializeContents(context));
        *needsClearInit = false;
        return angle::Result::Continue;
    }

    *needsClearInit = true;
    return angle::Result::Continue;
}

}  // anonymous namespace

BlitGL::BlitGL(const FunctionsGL *functions,
               const angle::FeaturesGL &features,
               StateManagerGL *stateManager)
    : mFunctions(functions),
      mFeatures(features),
      mStateManager(stateManager),
      mScratchFBO(0),
      mVAO(0),
      mVertexBuffer(0)
{
    for (size_t i = 0; i < ArraySize(mScratchTextures); i++)
    {
        mScratchTextures[i] = 0;
    }

    ASSERT(mFunctions);
    ASSERT(mStateManager);
}

BlitGL::~BlitGL()
{
    for (const auto &blitProgram : mBlitPrograms)
    {
        mStateManager->deleteProgram(blitProgram.second.program);
    }
    mBlitPrograms.clear();

    for (size_t i = 0; i < ArraySize(mScratchTextures); i++)
    {
        if (mScratchTextures[i] != 0)
        {
            mStateManager->deleteTexture(mScratchTextures[i]);
            mScratchTextures[i] = 0;
        }
    }

    if (mScratchFBO != 0)
    {
        mStateManager->deleteFramebuffer(mScratchFBO);
        mScratchFBO = 0;
    }

    if (mOwnsVAOState)
    {
        mStateManager->deleteVertexArray(mVAO);
        SafeDelete(mVAOState);
        mVAO = 0;
    }
}

angle::Result BlitGL::copyImageToLUMAWorkaroundTexture(const gl::Context *context,
                                                       GLuint texture,
                                                       gl::TextureType textureType,
                                                       gl::TextureTarget target,
                                                       GLenum lumaFormat,
                                                       size_t level,
                                                       const gl::Rectangle &sourceArea,
                                                       GLenum internalFormat,
                                                       gl::Framebuffer *source)
{
    mStateManager->bindTexture(textureType, texture);

    // Allocate the texture memory
    GLenum format   = gl::GetUnsizedFormat(internalFormat);
    GLenum readType = source->getImplementationColorReadType(context);

    // getImplementationColorReadType aligns the type with ES client version
    if (readType == GL_HALF_FLOAT_OES && mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        readType = GL_HALF_FLOAT;
    }

    gl::PixelUnpackState unpack;
    ANGLE_TRY(mStateManager->setPixelUnpackState(context, unpack));
    ANGLE_TRY(mStateManager->setPixelUnpackBuffer(
        context, context->getState().getTargetBuffer(gl::BufferBinding::PixelUnpack)));
    ANGLE_GL_TRY_ALWAYS_CHECK(
        context,
        mFunctions->texImage2D(ToGLenum(target), static_cast<GLint>(level), internalFormat,
                               sourceArea.width, sourceArea.height, 0, format, readType, nullptr));

    return copySubImageToLUMAWorkaroundTexture(context, texture, textureType, target, lumaFormat,
                                               level, gl::Offset(0, 0, 0), sourceArea, source);
}

angle::Result BlitGL::copySubImageToLUMAWorkaroundTexture(const gl::Context *context,
                                                          GLuint texture,
                                                          gl::TextureType textureType,
                                                          gl::TextureTarget target,
                                                          GLenum lumaFormat,
                                                          size_t level,
                                                          const gl::Offset &destOffset,
                                                          const gl::Rectangle &sourceArea,
                                                          gl::Framebuffer *source)
{
    ANGLE_TRY(initializeResources(context));

    BlitProgram *blitProgram = nullptr;
    ANGLE_TRY(getBlitProgram(context, gl::TextureType::_2D, GL_FLOAT, GL_FLOAT, &blitProgram));

    // Blit the framebuffer to the first scratch texture
    const FramebufferGL *sourceFramebufferGL = GetImplAs<FramebufferGL>(source);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, sourceFramebufferGL->getFramebufferID());

    GLenum readFormat = source->getImplementationColorReadFormat(context);
    GLenum readType   = source->getImplementationColorReadType(context);

    // getImplementationColorReadType aligns the type with ES client version
    if (readType == GL_HALF_FLOAT_OES && mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        readType = GL_HALF_FLOAT;
    }

    nativegl::CopyTexImageImageFormat copyTexImageFormat =
        nativegl::GetCopyTexImageImageFormat(mFunctions, mFeatures, readFormat, readType);

    mStateManager->bindTexture(gl::TextureType::_2D, mScratchTextures[0]);
    ANGLE_GL_TRY_ALWAYS_CHECK(
        context, mFunctions->copyTexImage2D(GL_TEXTURE_2D, 0, copyTexImageFormat.internalFormat,
                                            sourceArea.x, sourceArea.y, sourceArea.width,
                                            sourceArea.height, 0));

    // Set the swizzle of the scratch texture so that the channels sample into the correct emulated
    // LUMA channels.
    GLint swizzle[4] = {
        (lumaFormat == GL_ALPHA) ? GL_ALPHA : GL_RED,
        (lumaFormat == GL_LUMINANCE_ALPHA) ? GL_ALPHA : GL_ZERO,
        GL_ZERO,
        GL_ZERO,
    };
    ANGLE_GL_TRY(context,
                 mFunctions->texParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle));

    // Make a temporary framebuffer using the second scratch texture to render the swizzled result
    // to.
    mStateManager->bindTexture(gl::TextureType::_2D, mScratchTextures[1]);
    ANGLE_GL_TRY_ALWAYS_CHECK(
        context, mFunctions->texImage2D(GL_TEXTURE_2D, 0, copyTexImageFormat.internalFormat,
                                        sourceArea.width, sourceArea.height, 0,
                                        gl::GetUnsizedFormat(copyTexImageFormat.internalFormat),
                                        readType, nullptr));

    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                           GL_TEXTURE_2D, mScratchTextures[1], 0));

    // Render to the destination texture, sampling from the scratch texture
    ScopedGLState scopedState;
    ANGLE_TRY(scopedState.enter(context, gl::Rectangle(0, 0, sourceArea.width, sourceArea.height)));
    scopedState.willUseTextureUnit(context, 0);

    ANGLE_TRY(setScratchTextureParameter(context, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    ANGLE_TRY(setScratchTextureParameter(context, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    mStateManager->activeTexture(0);
    mStateManager->bindTexture(gl::TextureType::_2D, mScratchTextures[0]);

    mStateManager->useProgram(blitProgram->program);
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->sourceTextureLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform2f(blitProgram->scaleLocation, 1.0, 1.0));
    ANGLE_GL_TRY(context, mFunctions->uniform2f(blitProgram->offsetLocation, 0.0, 0.0));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->multiplyAlphaLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->unMultiplyAlphaLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->transformLinearToSrgbLocation, 0));

    ANGLE_TRY(setVAOState(context));
    ANGLE_GL_TRY(context, mFunctions->drawArrays(GL_TRIANGLES, 0, 3));

    // Copy the swizzled texture to the destination texture
    mStateManager->bindTexture(textureType, texture);

    if (nativegl::UseTexImage3D(textureType))
    {
        ANGLE_GL_TRY(context,
                     mFunctions->copyTexSubImage3D(ToGLenum(target), static_cast<GLint>(level),
                                                   destOffset.x, destOffset.y, destOffset.z, 0, 0,
                                                   sourceArea.width, sourceArea.height));
    }
    else
    {
        ASSERT(nativegl::UseTexImage2D(textureType));
        ANGLE_GL_TRY(context, mFunctions->copyTexSubImage2D(
                                  ToGLenum(target), static_cast<GLint>(level), destOffset.x,
                                  destOffset.y, 0, 0, sourceArea.width, sourceArea.height));
    }

    // Finally orphan the scratch textures so they can be GCed by the driver.
    ANGLE_TRY(orphanScratchTextures(context));
    ANGLE_TRY(UnbindAttachment(context, mFunctions, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));

    ANGLE_TRY(scopedState.exit(context));
    return angle::Result::Continue;
}

angle::Result BlitGL::blitColorBufferWithShader(const gl::Context *context,
                                                const gl::Framebuffer *source,
                                                const GLuint destTexture,
                                                const gl::TextureTarget destTarget,
                                                const size_t destLevel,
                                                const gl::Rectangle &sourceAreaIn,
                                                const gl::Rectangle &destAreaIn,
                                                GLenum filter,
                                                bool writeAlpha)
{
    ANGLE_TRY(initializeResources(context));
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                           ToGLenum(destTarget), destTexture,
                                                           static_cast<GLint>(destLevel)));
    GLenum status = ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        return angle::Result::Stop;
    }
    angle::Result result = blitColorBufferWithShader(context, source, mScratchFBO, sourceAreaIn,
                                                     destAreaIn, filter, writeAlpha);
    // Unbind the texture from the the scratch framebuffer.
    ANGLE_TRY(UnbindAttachment(context, mFunctions, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));
    return result;
}

angle::Result BlitGL::blitColorBufferWithShader(const gl::Context *context,
                                                const gl::Framebuffer *source,
                                                const gl::Framebuffer *dest,
                                                const gl::Rectangle &sourceAreaIn,
                                                const gl::Rectangle &destAreaIn,
                                                GLenum filter,
                                                bool writeAlpha)
{
    const FramebufferGL *destGL = GetImplAs<FramebufferGL>(dest);
    return blitColorBufferWithShader(context, source, destGL->getFramebufferID(), sourceAreaIn,
                                     destAreaIn, filter, writeAlpha);
}

angle::Result BlitGL::blitColorBufferWithShader(const gl::Context *context,
                                                const gl::Framebuffer *source,
                                                const GLuint destFramebuffer,
                                                const gl::Rectangle &sourceAreaIn,
                                                const gl::Rectangle &destAreaIn,
                                                GLenum filter,
                                                bool writeAlpha)
{
    ANGLE_TRY(initializeResources(context));

    BlitProgram *blitProgram = nullptr;
    ANGLE_TRY(getBlitProgram(context, gl::TextureType::_2D, GL_FLOAT, GL_FLOAT, &blitProgram));

    // We'll keep things simple by removing reversed coordinates from the rectangles. In the end
    // we'll apply the reversal to the source texture coordinates if needed. The destination
    // rectangle will be set to the gl viewport, which can't be reversed.
    bool reverseX            = sourceAreaIn.isReversedX() != destAreaIn.isReversedX();
    bool reverseY            = sourceAreaIn.isReversedY() != destAreaIn.isReversedY();
    gl::Rectangle sourceArea = sourceAreaIn.removeReversal();
    gl::Rectangle destArea   = destAreaIn.removeReversal();

    const gl::FramebufferAttachment *readAttachment = source->getReadColorAttachment();
    ASSERT(readAttachment->getSamples() <= 1);

    // Compute the part of the source that will be sampled.
    gl::Rectangle inBoundsSource;
    {
        gl::Extents sourceSize = readAttachment->getSize();
        gl::Rectangle sourceBounds(0, 0, sourceSize.width, sourceSize.height);
        if (!gl::ClipRectangle(sourceArea, sourceBounds, &inBoundsSource))
        {
            // Early out when the sampled part is empty as the blit will be a noop,
            // and it prevents a division by zero in later computations.
            return angle::Result::Continue;
        }
    }

    // The blit will be emulated by getting the source of the blit in a texture and sampling it
    // with CLAMP_TO_EDGE.

    GLuint textureId;

    // TODO(cwallez) once texture dirty bits are landed, reuse attached texture instead of using
    // CopyTexImage2D
    {
        textureId = mScratchTextures[0];

        const gl::InternalFormat &sourceInternalFormat       = *readAttachment->getFormat().info;
        nativegl::CopyTexImageImageFormat copyTexImageFormat = nativegl::GetCopyTexImageImageFormat(
            mFunctions, mFeatures, sourceInternalFormat.internalFormat, sourceInternalFormat.type);
        const FramebufferGL *sourceGL = GetImplAs<FramebufferGL>(source);
        mStateManager->bindFramebuffer(GL_READ_FRAMEBUFFER, sourceGL->getFramebufferID());
        mStateManager->bindTexture(gl::TextureType::_2D, textureId);

        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, mFunctions->copyTexImage2D(GL_TEXTURE_2D, 0, copyTexImageFormat.internalFormat,
                                                inBoundsSource.x, inBoundsSource.y,
                                                inBoundsSource.width, inBoundsSource.height, 0));

        // Translate sourceArea to be relative to the copied image.
        sourceArea.x -= inBoundsSource.x;
        sourceArea.y -= inBoundsSource.y;

        ANGLE_TRY(setScratchTextureParameter(context, GL_TEXTURE_MIN_FILTER, filter));
        ANGLE_TRY(setScratchTextureParameter(context, GL_TEXTURE_MAG_FILTER, filter));
        ANGLE_TRY(setScratchTextureParameter(context, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        ANGLE_TRY(setScratchTextureParameter(context, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    }

    // Transform the source area to the texture coordinate space (where 0.0 and 1.0 correspond to
    // the edges of the texture).
    Vector2 texCoordOffset(
        static_cast<float>(sourceArea.x) / static_cast<float>(inBoundsSource.width),
        static_cast<float>(sourceArea.y) / static_cast<float>(inBoundsSource.height));
    // texCoordScale is equal to the size of the source area in texture coordinates.
    Vector2 texCoordScale(
        static_cast<float>(sourceArea.width) / static_cast<float>(inBoundsSource.width),
        static_cast<float>(sourceArea.height) / static_cast<float>(inBoundsSource.height));

    if (reverseX)
    {
        texCoordOffset.x() = texCoordOffset.x() + texCoordScale.x();
        texCoordScale.x()  = -texCoordScale.x();
    }
    if (reverseY)
    {
        texCoordOffset.y() = texCoordOffset.y() + texCoordScale.y();
        texCoordScale.y()  = -texCoordScale.y();
    }

    // Reset all the state except scissor and use the viewport to draw exactly to the destination
    // rectangle
    ScopedGLState scopedState;
    ANGLE_TRY(scopedState.enter(context, destArea, ScopedGLState::KEEP_SCISSOR));
    scopedState.willUseTextureUnit(context, 0);

    // Set the write color mask to potentially not write alpha
    mStateManager->setColorMask(true, true, true, writeAlpha);

    // Set uniforms
    mStateManager->activeTexture(0);
    mStateManager->bindTexture(gl::TextureType::_2D, textureId);

    mStateManager->useProgram(blitProgram->program);
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->sourceTextureLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform2f(blitProgram->scaleLocation, texCoordScale.x(),
                                                texCoordScale.y()));
    ANGLE_GL_TRY(context, mFunctions->uniform2f(blitProgram->offsetLocation, texCoordOffset.x(),
                                                texCoordOffset.y()));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->multiplyAlphaLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->unMultiplyAlphaLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->transformLinearToSrgbLocation, 0));

    mStateManager->bindFramebuffer(GL_DRAW_FRAMEBUFFER, destFramebuffer);

    ANGLE_TRY(setVAOState(context));
    ANGLE_GL_TRY(context, mFunctions->drawArrays(GL_TRIANGLES, 0, 3));

    ANGLE_TRY(scopedState.exit(context));
    return angle::Result::Continue;
}

angle::Result BlitGL::copySubTexture(const gl::Context *context,
                                     TextureGL *source,
                                     size_t sourceLevel,
                                     GLenum sourceComponentType,
                                     GLuint destID,
                                     gl::TextureTarget destTarget,
                                     size_t destLevel,
                                     GLenum destComponentType,
                                     const gl::Extents &sourceSize,
                                     const gl::Rectangle &sourceArea,
                                     const gl::Offset &destOffset,
                                     bool needsLumaWorkaround,
                                     GLenum lumaFormat,
                                     bool unpackFlipY,
                                     bool unpackPremultiplyAlpha,
                                     bool unpackUnmultiplyAlpha,
                                     bool transformLinearToSrgb,
                                     bool *copySucceededOut)
{
    ASSERT(source->getType() == gl::TextureType::_2D ||
           source->getType() == gl::TextureType::External ||
           source->getType() == gl::TextureType::Rectangle);
    ANGLE_TRY(initializeResources(context));

    // Make sure the destination texture can be rendered to before setting anything else up.  Some
    // cube maps may not be renderable until all faces have been filled.
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                           ToGLenum(destTarget), destID,
                                                           static_cast<GLint>(destLevel)));
    GLenum status = ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        *copySucceededOut = false;
        return angle::Result::Continue;
    }

    BlitProgram *blitProgram = nullptr;
    ANGLE_TRY(getBlitProgram(context, source->getType(), sourceComponentType, destComponentType,
                             &blitProgram));

    // Setup the source texture
    if (needsLumaWorkaround)
    {
        GLint luminance = (lumaFormat == GL_ALPHA) ? GL_ZERO : GL_RED;

        GLint alpha = GL_RED;
        if (lumaFormat == GL_LUMINANCE)
        {
            alpha = GL_ONE;
        }
        else if (lumaFormat == GL_LUMINANCE_ALPHA)
        {
            alpha = GL_GREEN;
        }
        else
        {
            ASSERT(lumaFormat == GL_ALPHA);
        }

        GLint swizzle[4] = {luminance, luminance, luminance, alpha};
        ANGLE_TRY(source->setSwizzle(context, swizzle));
    }
    ANGLE_TRY(source->setMinFilter(context, GL_NEAREST));
    ANGLE_TRY(source->setMagFilter(context, GL_NEAREST));
    ANGLE_TRY(source->setBaseLevel(context, static_cast<GLuint>(sourceLevel)));

    // Render to the destination texture, sampling from the source texture
    ScopedGLState scopedState;
    ANGLE_TRY(scopedState.enter(
        context, gl::Rectangle(destOffset.x, destOffset.y, sourceArea.width, sourceArea.height)));
    scopedState.willUseTextureUnit(context, 0);

    mStateManager->activeTexture(0);
    mStateManager->bindTexture(source->getType(), source->getTextureID());

    Vector2 scale(sourceArea.width, sourceArea.height);
    Vector2 offset(sourceArea.x, sourceArea.y);
    if (source->getType() != gl::TextureType::Rectangle)
    {
        scale.x() /= static_cast<float>(sourceSize.width);
        scale.y() /= static_cast<float>(sourceSize.height);
        offset.x() /= static_cast<float>(sourceSize.width);
        offset.y() /= static_cast<float>(sourceSize.height);
    }
    if (unpackFlipY)
    {
        offset.y() += scale.y();
        scale.y() = -scale.y();
    }

    mStateManager->useProgram(blitProgram->program);
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->sourceTextureLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform2f(blitProgram->scaleLocation, scale.x(), scale.y()));
    ANGLE_GL_TRY(context,
                 mFunctions->uniform2f(blitProgram->offsetLocation, offset.x(), offset.y()));
    if (unpackPremultiplyAlpha == unpackUnmultiplyAlpha)
    {
        ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->multiplyAlphaLocation, 0));
        ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->unMultiplyAlphaLocation, 0));
    }
    else
    {
        ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->multiplyAlphaLocation,
                                                    unpackPremultiplyAlpha));
        ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->unMultiplyAlphaLocation,
                                                    unpackUnmultiplyAlpha));
    }
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->transformLinearToSrgbLocation,
                                                transformLinearToSrgb));

    ANGLE_TRY(setVAOState(context));
    ANGLE_GL_TRY(context, mFunctions->drawArrays(GL_TRIANGLES, 0, 3));
    ANGLE_TRY(UnbindAttachment(context, mFunctions, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));

    *copySucceededOut = true;
    ANGLE_TRY(scopedState.exit(context));
    return angle::Result::Continue;
}

angle::Result BlitGL::copySubTextureCPUReadback(const gl::Context *context,
                                                TextureGL *source,
                                                size_t sourceLevel,
                                                GLenum sourceSizedInternalFormat,
                                                TextureGL *dest,
                                                gl::TextureTarget destTarget,
                                                size_t destLevel,
                                                GLenum destFormat,
                                                GLenum destType,
                                                const gl::Extents &sourceSize,
                                                const gl::Rectangle &sourceArea,
                                                const gl::Offset &destOffset,
                                                bool needsLumaWorkaround,
                                                GLenum lumaFormat,
                                                bool unpackFlipY,
                                                bool unpackPremultiplyAlpha,
                                                bool unpackUnmultiplyAlpha)
{
    ANGLE_TRY(initializeResources(context));

    ContextGL *contextGL = GetImplAs<ContextGL>(context);

    ASSERT(source->getType() == gl::TextureType::_2D ||
           source->getType() == gl::TextureType::External ||
           source->getType() == gl::TextureType::Rectangle);
    const auto &destInternalFormatInfo = gl::GetInternalFormatInfo(destFormat, destType);
    const gl::InternalFormat &sourceInternalFormatInfo =
        gl::GetSizedInternalFormatInfo(sourceSizedInternalFormat);

    gl::Rectangle readPixelsArea = sourceArea;

    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    bool supportExternalTarget =
        source->getType() == gl::TextureType::External && context->getExtensions().YUVTargetEXT;
    GLenum status = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    if (supportExternalTarget || source->getType() != gl::TextureType::External)
    {
        ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(
                                  GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ToGLenum(source->getType()),
                                  source->getTextureID(), static_cast<GLint>(sourceLevel)));
        status = ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
    }
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        // The source texture cannot be read with glReadPixels. Copy it into another RGBA texture
        // and read that back instead.
        nativegl::TexImageFormat texImageFormat = nativegl::GetTexImageFormat(
            mFunctions, mFeatures, sourceInternalFormatInfo.internalFormat,
            sourceInternalFormatInfo.format, sourceInternalFormatInfo.type);

        gl::TextureType scratchTextureType = gl::TextureType::_2D;
        mStateManager->bindTexture(scratchTextureType, mScratchTextures[0]);
        ANGLE_GL_TRY_ALWAYS_CHECK(
            context,
            mFunctions->texImage2D(ToGLenum(scratchTextureType), 0, texImageFormat.internalFormat,
                                   sourceArea.width, sourceArea.height, 0, texImageFormat.format,
                                   texImageFormat.type, nullptr));

        bool copySucceeded = false;
        ANGLE_TRY(copySubTexture(
            context, source, sourceLevel, sourceInternalFormatInfo.componentType,
            mScratchTextures[0], NonCubeTextureTypeToTarget(scratchTextureType), 0,
            sourceInternalFormatInfo.componentType, sourceSize, sourceArea, gl::Offset(0, 0, 0),
            needsLumaWorkaround, lumaFormat, false, false, false, false, &copySucceeded));
        if (!copySucceeded)
        {
            // No fallback options if we can't render to the scratch texture.
            return angle::Result::Stop;
        }

        // Bind the scratch texture as the readback texture
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
        ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                               ToGLenum(scratchTextureType),
                                                               mScratchTextures[0], 0));

        // The scratch texture sized to sourceArea so adjust the readpixels area
        readPixelsArea.x = 0;
        readPixelsArea.y = 0;

        // Recheck the status
        status = ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
    }

    ASSERT(status == GL_FRAMEBUFFER_COMPLETE);

    // Create a buffer for holding the source and destination memory
    const size_t sourcePixelSize = 4;
    size_t sourceBufferSize      = readPixelsArea.width * readPixelsArea.height * sourcePixelSize;
    size_t destBufferSize =
        readPixelsArea.width * readPixelsArea.height * destInternalFormatInfo.pixelBytes;
    angle::MemoryBuffer *buffer = nullptr;
    ANGLE_CHECK_GL_ALLOC(contextGL,
                         context->getScratchBuffer(sourceBufferSize + destBufferSize, &buffer));

    uint8_t *sourceMemory = buffer->data();
    uint8_t *destMemory   = buffer->data() + sourceBufferSize;

    GLenum readPixelsFormat        = GL_NONE;
    PixelReadFunction readFunction = nullptr;
    if (sourceInternalFormatInfo.componentType == GL_UNSIGNED_INT)
    {
        readPixelsFormat = GL_RGBA_INTEGER;
        readFunction     = angle::ReadColor<angle::R8G8B8A8, GLuint>;
    }
    else
    {
        ASSERT(sourceInternalFormatInfo.componentType != GL_INT);
        readPixelsFormat = GL_RGBA;
        readFunction     = angle::ReadColor<angle::R8G8B8A8, GLfloat>;
    }

    gl::PixelUnpackState unpack;
    unpack.alignment = 1;
    ANGLE_TRY(mStateManager->setPixelUnpackState(context, unpack));
    ANGLE_TRY(mStateManager->setPixelUnpackBuffer(context, nullptr));
    ANGLE_GL_TRY(context, mFunctions->readPixels(readPixelsArea.x, readPixelsArea.y,
                                                 readPixelsArea.width, readPixelsArea.height,
                                                 readPixelsFormat, GL_UNSIGNED_BYTE, sourceMemory));

    angle::FormatID destFormatID =
        angle::Format::InternalFormatToID(destInternalFormatInfo.sizedInternalFormat);
    const auto &destFormatInfo = angle::Format::Get(destFormatID);
    CopyImageCHROMIUM(
        sourceMemory, readPixelsArea.width * sourcePixelSize, sourcePixelSize, 0, readFunction,
        destMemory, readPixelsArea.width * destInternalFormatInfo.pixelBytes,
        destInternalFormatInfo.pixelBytes, 0, destFormatInfo.pixelWriteFunction,
        destInternalFormatInfo.format, destInternalFormatInfo.componentType, readPixelsArea.width,
        readPixelsArea.height, 1, unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha);

    gl::PixelPackState pack;
    pack.alignment = 1;
    ANGLE_TRY(mStateManager->setPixelPackState(context, pack));
    ANGLE_TRY(mStateManager->setPixelPackBuffer(context, nullptr));

    nativegl::TexSubImageFormat texSubImageFormat =
        nativegl::GetTexSubImageFormat(mFunctions, mFeatures, destFormat, destType);

    mStateManager->bindTexture(dest->getType(), dest->getTextureID());
    ANGLE_GL_TRY(context, mFunctions->texSubImage2D(
                              ToGLenum(destTarget), static_cast<GLint>(destLevel), destOffset.x,
                              destOffset.y, readPixelsArea.width, readPixelsArea.height,
                              texSubImageFormat.format, texSubImageFormat.type, destMemory));

    ANGLE_TRY(UnbindAttachment(context, mFunctions, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));

    return angle::Result::Continue;
}

angle::Result BlitGL::copyTexSubImage(const gl::Context *context,
                                      TextureGL *source,
                                      size_t sourceLevel,
                                      TextureGL *dest,
                                      gl::TextureTarget destTarget,
                                      size_t destLevel,
                                      const gl::Rectangle &sourceArea,
                                      const gl::Offset &destOffset,
                                      bool *copySucceededOut)
{
    ANGLE_TRY(initializeResources(context));

    // Make sure the source texture can create a complete framebuffer before continuing.
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(
                              GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ToGLenum(source->getType()),
                              source->getTextureID(), static_cast<GLint>(sourceLevel)));
    GLenum status = ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        *copySucceededOut = false;
        return angle::Result::Continue;
    }

    mStateManager->bindTexture(dest->getType(), dest->getTextureID());

    // Handle GL errors during copyTexSubImage2D manually since this can fail for certain formats on
    // Pixel 2 and 4 and we have fallback paths (blit via shader) in the caller.
    ClearErrors(context, __FILE__, __FUNCTION__, __LINE__);
    mFunctions->copyTexSubImage2D(ToGLenum(destTarget), static_cast<GLint>(destLevel), destOffset.x,
                                  destOffset.y, sourceArea.x, sourceArea.y, sourceArea.width,
                                  sourceArea.height);
    // Use getError to retrieve the error directly instead of using CheckError so that we don't
    // propagate the error to the client and also so that we can handle INVALID_OPERATION specially.
    const GLenum copyError = mFunctions->getError();
    // Any error other than NO_ERROR or INVALID_OPERATION is propagated to the client as a failure.
    // INVALID_OPERATION is ignored and instead copySucceeded is set to false so that the caller can
    // fallback to another copy/blit implementation.
    if (ANGLE_UNLIKELY(copyError != GL_NO_ERROR && copyError != GL_INVALID_OPERATION))
    {
        // Propagate the error to the client and check for other unexpected errors.
        ANGLE_TRY(
            HandleError(context, copyError, "copyTexSubImage2D", __FILE__, __FUNCTION__, __LINE__));
    }
    // Even if copyTexSubImage2D fails with GL_INVALID_OPERATION, check for other unexpected errors.
    ANGLE_TRY(CheckError(context, "copyTexSubImage2D", __FILE__, __FUNCTION__, __LINE__));

    ANGLE_TRY(UnbindAttachment(context, mFunctions, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));
    *copySucceededOut = copyError == GL_NO_ERROR;
    return angle::Result::Continue;
}

angle::Result BlitGL::clearRenderableTexture(const gl::Context *context,
                                             TextureGL *source,
                                             GLenum sizedInternalFormat,
                                             int numTextureLayers,
                                             const gl::ImageIndex &imageIndex,
                                             bool *clearSucceededOut)
{
    ANGLE_TRY(initializeResources(context));

    ClearBindTargetVector bindTargets;
    ClearBindTargetVector unbindTargets;
    GLbitfield clearMask = 0;
    ANGLE_TRY(PrepareForClear(mStateManager, sizedInternalFormat, &bindTargets, &unbindTargets,
                              &clearMask));

    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    ANGLE_TRY(UnbindAttachments(context, mFunctions, GL_FRAMEBUFFER, unbindTargets));

    if (nativegl::UseTexImage2D(source->getType()))
    {
        ASSERT(numTextureLayers == 1);
        for (GLenum bindTarget : bindTargets)
        {
            ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(
                                      GL_FRAMEBUFFER, bindTarget, ToGLenum(imageIndex.getTarget()),
                                      source->getTextureID(), imageIndex.getLevelIndex()));
        }

        GLenum status = ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
        if (status == GL_FRAMEBUFFER_COMPLETE)
        {
            ANGLE_GL_TRY(context, mFunctions->clear(clearMask));
        }
        else
        {
            ANGLE_TRY(UnbindAttachments(context, mFunctions, GL_FRAMEBUFFER, bindTargets));
            *clearSucceededOut = false;
            return angle::Result::Continue;
        }
    }
    else
    {
        ASSERT(nativegl::UseTexImage3D(source->getType()));

        // Check if it's possible to bind all layers of the texture at once
        if (mFunctions->framebufferTexture && !imageIndex.hasLayer())
        {
            for (GLenum bindTarget : bindTargets)
            {
                ANGLE_GL_TRY(context, mFunctions->framebufferTexture(GL_FRAMEBUFFER, bindTarget,
                                                                     source->getTextureID(),
                                                                     imageIndex.getLevelIndex()));
            }

            GLenum status =
                ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
            if (status == GL_FRAMEBUFFER_COMPLETE)
            {
                ANGLE_GL_TRY(context, mFunctions->clear(clearMask));
            }
            else
            {
                ANGLE_TRY(UnbindAttachments(context, mFunctions, GL_FRAMEBUFFER, bindTargets));
                *clearSucceededOut = false;
                return angle::Result::Continue;
            }
        }
        else
        {
            GLint firstLayer = 0;
            GLint layerCount = numTextureLayers;
            if (imageIndex.hasLayer())
            {
                firstLayer = imageIndex.getLayerIndex();
                layerCount = imageIndex.getLayerCount();
            }

            for (GLint layer = 0; layer < layerCount; layer++)
            {
                for (GLenum bindTarget : bindTargets)
                {
                    ANGLE_GL_TRY(context, mFunctions->framebufferTextureLayer(
                                              GL_FRAMEBUFFER, bindTarget, source->getTextureID(),
                                              imageIndex.getLevelIndex(), layer + firstLayer));
                }

                GLenum status =
                    ANGLE_GL_TRY(context, mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER));
                if (status == GL_FRAMEBUFFER_COMPLETE)
                {
                    ANGLE_GL_TRY(context, mFunctions->clear(clearMask));
                }
                else
                {
                    ANGLE_TRY(UnbindAttachments(context, mFunctions, GL_FRAMEBUFFER, bindTargets));
                    *clearSucceededOut = false;
                    return angle::Result::Continue;
                }
            }
        }
    }

    ANGLE_TRY(UnbindAttachments(context, mFunctions, GL_FRAMEBUFFER, bindTargets));
    *clearSucceededOut = true;
    return angle::Result::Continue;
}

angle::Result BlitGL::clearRenderbuffer(const gl::Context *context,
                                        RenderbufferGL *source,
                                        GLenum sizedInternalFormat)
{
    ANGLE_TRY(initializeResources(context));

    ClearBindTargetVector bindTargets;
    ClearBindTargetVector unbindTargets;
    GLbitfield clearMask = 0;
    ANGLE_TRY(PrepareForClear(mStateManager, sizedInternalFormat, &bindTargets, &unbindTargets,
                              &clearMask));

    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    ANGLE_TRY(UnbindAttachments(context, mFunctions, GL_FRAMEBUFFER, unbindTargets));

    for (GLenum bindTarget : bindTargets)
    {
        ANGLE_GL_TRY(context,
                     mFunctions->framebufferRenderbuffer(
                         GL_FRAMEBUFFER, bindTarget, GL_RENDERBUFFER, source->getRenderbufferID()));
    }
    ANGLE_GL_TRY(context, mFunctions->clear(clearMask));

    // Unbind
    for (GLenum bindTarget : bindTargets)
    {
        ANGLE_GL_TRY(context, mFunctions->framebufferRenderbuffer(GL_FRAMEBUFFER, bindTarget,
                                                                  GL_RENDERBUFFER, 0));
    }

    return angle::Result::Continue;
}

angle::Result BlitGL::clearFramebuffer(const gl::Context *context,
                                       const gl::DrawBufferMask &colorAttachments,
                                       bool depthClear,
                                       bool stencilClear,
                                       FramebufferGL *source)
{
    // initializeResources skipped because no local state is used

    bool hasIntegerColorAttachments = false;

    // Filter the color attachments for ones that actually have an init state of uninitialized.
    gl::DrawBufferMask uninitializedColorAttachments;
    for (size_t colorAttachmentIdx : colorAttachments)
    {
        bool needsInit = false;
        const gl::FramebufferAttachment *attachment =
            source->getState().getColorAttachment(colorAttachmentIdx);
        ANGLE_TRY(CheckIfAttachmentNeedsClearing(context, attachment, &needsInit));
        uninitializedColorAttachments[colorAttachmentIdx] = needsInit;
        if (needsInit && (attachment->getComponentType() == GL_INT ||
                          attachment->getComponentType() == GL_UNSIGNED_INT))
        {
            hasIntegerColorAttachments = true;
        }
    }

    bool depthNeedsInit = false;
    if (depthClear)
    {
        ANGLE_TRY(CheckIfAttachmentNeedsClearing(context, source->getState().getDepthAttachment(),
                                                 &depthNeedsInit));
    }

    bool stencilNeedsInit = false;
    if (stencilClear)
    {
        ANGLE_TRY(CheckIfAttachmentNeedsClearing(context, source->getState().getStencilAttachment(),
                                                 &stencilNeedsInit));
    }

    // Clear all attachments
    GLbitfield clearMask = 0;
    ANGLE_TRY(SetClearState(mStateManager, uninitializedColorAttachments.any(), depthNeedsInit,
                            stencilNeedsInit, &clearMask));

    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, source->getFramebufferID());

    // If we're not clearing all attached color attachments, we need to clear them individually with
    // glClearBuffer*
    if ((clearMask & GL_COLOR_BUFFER_BIT) &&
        (uninitializedColorAttachments != source->getState().getColorAttachmentsMask() ||
         uninitializedColorAttachments != source->getState().getEnabledDrawBuffers() ||
         hasIntegerColorAttachments))
    {
        for (size_t colorAttachmentIdx : uninitializedColorAttachments)
        {
            const gl::FramebufferAttachment *attachment =
                source->getState().getColorAttachment(colorAttachmentIdx);
            if (attachment->initState() == gl::InitState::Initialized)
            {
                continue;
            }

            switch (attachment->getComponentType())
            {
                case GL_UNSIGNED_NORMALIZED:
                case GL_SIGNED_NORMALIZED:
                case GL_FLOAT:
                {
                    constexpr GLfloat clearValue[] = {0, 0, 0, 0};
                    ANGLE_GL_TRY(context,
                                 mFunctions->clearBufferfv(
                                     GL_COLOR, static_cast<GLint>(colorAttachmentIdx), clearValue));
                }
                break;

                case GL_INT:
                {
                    constexpr GLint clearValue[] = {0, 0, 0, 0};
                    ANGLE_GL_TRY(context,
                                 mFunctions->clearBufferiv(
                                     GL_COLOR, static_cast<GLint>(colorAttachmentIdx), clearValue));
                }
                break;

                case GL_UNSIGNED_INT:
                {
                    constexpr GLuint clearValue[] = {0, 0, 0, 0};
                    ANGLE_GL_TRY(context,
                                 mFunctions->clearBufferuiv(
                                     GL_COLOR, static_cast<GLint>(colorAttachmentIdx), clearValue));
                }
                break;

                default:
                    UNREACHABLE();
                    break;
            }
        }

        // Remove color buffer bit and clear the rest of the attachments with glClear
        clearMask = clearMask & ~GL_COLOR_BUFFER_BIT;
    }

    if (clearMask != 0)
    {
        ANGLE_GL_TRY(context, mFunctions->clear(clearMask));
    }

    return angle::Result::Continue;
}

angle::Result BlitGL::clearRenderableTextureAlphaToOne(const gl::Context *context,
                                                       GLuint texture,
                                                       gl::TextureTarget target,
                                                       size_t level)
{
    // Clearing the alpha of 3D textures is not supported/needed yet.
    ASSERT(nativegl::UseTexImage2D(TextureTargetToType(target)));

    ANGLE_TRY(initializeResources(context));

    mStateManager->setClearColor(gl::ColorF(0.0f, 0.0f, 0.0f, 1.0f));
    mStateManager->setColorMask(false, false, false, true);
    mStateManager->setScissorTestEnabled(false);

    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                           ToGLenum(target), texture,
                                                           static_cast<GLint>(level)));
    ANGLE_GL_TRY(context, mFunctions->clear(GL_COLOR_BUFFER_BIT));

    // Unbind the texture from the the scratch framebuffer
    ANGLE_TRY(UnbindAttachment(context, mFunctions, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));

    return angle::Result::Continue;
}

angle::Result BlitGL::generateMipmap(const gl::Context *context,
                                     TextureGL *source,
                                     GLuint baseLevel,
                                     GLuint levelCount,
                                     const gl::Extents &sourceBaseLevelSize,
                                     const nativegl::TexImageFormat &format)
{
    ANGLE_TRY(initializeResources(context));

    const gl::TextureType sourceType     = gl::TextureType::_2D;
    const gl::TextureTarget sourceTarget = gl::TextureTarget::_2D;

    ScopedGLState scopedState;
    ANGLE_TRY(scopedState.enter(
        context, gl::Rectangle(0, 0, sourceBaseLevelSize.width, sourceBaseLevelSize.height)));
    scopedState.willUseTextureUnit(context, 0);
    mStateManager->activeTexture(0);

    // Copy source to an intermediate texture.
    GLuint intermediateTexture = mScratchTextures[0];
    mStateManager->bindTexture(sourceType, intermediateTexture);
    mStateManager->bindBuffer(gl::BufferBinding::PixelUnpack, 0);
    ANGLE_GL_TRY(context, mFunctions->texParameteri(ToGLenum(sourceTarget), GL_TEXTURE_MIN_FILTER,
                                                    GL_NEAREST));
    ANGLE_GL_TRY(context, mFunctions->texParameteri(ToGLenum(sourceTarget), GL_TEXTURE_MAG_FILTER,
                                                    GL_NEAREST));

    // Use a shader to copy the source to intermediate texture. glBlitFramebuffer does not always do
    // sRGB to linear conversions for us.
    BlitProgram *blitProgram = nullptr;
    ANGLE_TRY(getBlitProgram(context, sourceType, GL_FLOAT, GL_FLOAT, &blitProgram));

    mStateManager->useProgram(blitProgram->program);
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->sourceTextureLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform2f(blitProgram->scaleLocation, 1.0f, 1.0f));
    ANGLE_GL_TRY(context, mFunctions->uniform2f(blitProgram->offsetLocation, 0.0f, 0.0f));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->multiplyAlphaLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->unMultiplyAlphaLocation, 0));
    ANGLE_GL_TRY(context, mFunctions->uniform1i(blitProgram->transformLinearToSrgbLocation, 0));

    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mScratchFBO);
    mStateManager->setFramebufferSRGBEnabled(context, true);

    ANGLE_TRY(setVAOState(context));

    ANGLE_TRY(source->setMinFilter(context, GL_LINEAR));
    ANGLE_TRY(source->setMagFilter(context, GL_LINEAR));

    // Copy back to the source texture from the mips generated in the intermediate texture
    for (GLuint levelIdx = 1; levelIdx < levelCount; levelIdx++)
    {
        gl::Extents levelSize(std::max(sourceBaseLevelSize.width >> levelIdx, 1),
                              std::max(sourceBaseLevelSize.height >> levelIdx, 1), 1);

        // Downsample from the source texture into the intermediate texture
        mStateManager->bindTexture(sourceType, intermediateTexture);
        ANGLE_GL_TRY(context, mFunctions->texImage2D(
                                  ToGLenum(sourceTarget), 0, format.internalFormat, levelSize.width,
                                  levelSize.height, 0, format.format, format.type, nullptr));

        ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                               ToGLenum(sourceTarget),
                                                               intermediateTexture, 0));
        mStateManager->setViewport(gl::Rectangle(0, 0, levelSize.width, levelSize.height));

        GLuint sourceTextureReadLevel = baseLevel + levelIdx - 1;
        mStateManager->bindTexture(sourceType, source->getTextureID());
        ANGLE_TRY(source->setBaseLevel(context, sourceTextureReadLevel));
        ANGLE_GL_TRY(context, mFunctions->drawArrays(GL_TRIANGLES, 0, 3));

        // Copy back to the source texture
        GLuint sourceTextureWriteLevel = sourceTextureReadLevel + 1;
        ANGLE_GL_TRY(context, mFunctions->framebufferTexture2D(
                                  GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ToGLenum(sourceTarget),
                                  source->getTextureID(), sourceTextureWriteLevel));
        mStateManager->bindTexture(sourceType, intermediateTexture);
        ANGLE_GL_TRY(context, mFunctions->drawArrays(GL_TRIANGLES, 0, 3));
    }

    ANGLE_TRY(source->setBaseLevel(context, baseLevel));

    ANGLE_TRY(orphanScratchTextures(context));
    ANGLE_TRY(UnbindAttachment(context, mFunctions, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));

    ANGLE_TRY(scopedState.exit(context));
    return angle::Result::Continue;
}

angle::Result BlitGL::generateSRGBMipmap(const gl::Context *context,
                                         TextureGL *source,
                                         GLuint baseLevel,
                                         GLuint levelCount,
                                         const gl::Extents &sourceBaseLevelSize)
{
    return generateMipmap(context, source, baseLevel, levelCount, sourceBaseLevelSize,
                          mSRGBMipmapGenerationFormat);
}

angle::Result BlitGL::initializeResources(const gl::Context *context)
{
    if (mResourcesInitialized)
    {
        return angle::Result::Continue;
    }

    for (size_t i = 0; i < ArraySize(mScratchTextures); i++)
    {
        ANGLE_GL_TRY(context, mFunctions->genTextures(1, &mScratchTextures[i]));
    }

    ANGLE_GL_TRY(context, mFunctions->genFramebuffers(1, &mScratchFBO));

    ANGLE_GL_TRY(context, mFunctions->genBuffers(1, &mVertexBuffer));
    mStateManager->bindBuffer(gl::BufferBinding::Array, mVertexBuffer);

    // Use a single, large triangle, to avoid arithmetic precision issues where fragments
    // with the same Y coordinate don't get exactly the same interpolated texcoord Y.
    float vertexData[] = {
        -0.5f, 0.0f, 1.5f, 0.0f, 0.5f, 2.0f,
    };

    ANGLE_GL_TRY(context, mFunctions->bufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, vertexData,
                                                 GL_STATIC_DRAW));

    VertexArrayStateGL *defaultVAOState = mStateManager->getDefaultVAOState();
    if (!mFeatures.syncAllVertexArraysToDefault.enabled)
    {
        ANGLE_GL_TRY(context, mFunctions->genVertexArrays(1, &mVAO));
        mVAOState     = new VertexArrayStateGL(defaultVAOState->attributes.size(),
                                               defaultVAOState->bindings.size());
        mOwnsVAOState = true;
        ANGLE_TRY(setVAOState(context));
        ANGLE_TRY(initializeVAOState(context));
    }
    else
    {
        mVAO          = mStateManager->getDefaultVAO();
        mVAOState     = defaultVAOState;
        mOwnsVAOState = false;
    }

    constexpr GLenum potentialSRGBMipmapGenerationFormats[] = {
        GL_RGBA16, GL_RGBA16F, GL_RGBA32F,
        GL_RGBA8,  // RGBA8 can have precision loss when generating mipmaps of a sRGBA8 texture
    };
    for (GLenum internalFormat : potentialSRGBMipmapGenerationFormats)
    {
        if (nativegl::SupportsNativeRendering(mFunctions, gl::TextureType::_2D, internalFormat))
        {
            const gl::InternalFormat &internalFormatInfo =
                gl::GetSizedInternalFormatInfo(internalFormat);

            // Pass the 'format' instead of 'internalFormat' to make sure we use unsized formats
            // when available to increase support.
            mSRGBMipmapGenerationFormat =
                nativegl::GetTexImageFormat(mFunctions, mFeatures, internalFormatInfo.format,
                                            internalFormatInfo.format, internalFormatInfo.type);
            break;
        }
    }
    ASSERT(mSRGBMipmapGenerationFormat.internalFormat != GL_NONE);

    mResourcesInitialized = true;
    return angle::Result::Continue;
}

angle::Result BlitGL::orphanScratchTextures(const gl::Context *context)
{
    for (auto texture : mScratchTextures)
    {
        mStateManager->bindTexture(gl::TextureType::_2D, texture);
        gl::PixelUnpackState unpack;
        ANGLE_TRY(mStateManager->setPixelUnpackState(context, unpack));
        ANGLE_TRY(mStateManager->setPixelUnpackBuffer(context, nullptr));
        if (mFunctions->isAtLeastGL(gl::Version(3, 3)))
        {
            constexpr GLint swizzle[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
            ANGLE_GL_TRY(context, mFunctions->texParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA,
                                                             swizzle));
        }
        else if (mFunctions->isAtLeastGLES(gl::Version(3, 0)))
        {
            ANGLE_GL_TRY(context,
                         mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED));
            ANGLE_GL_TRY(context,
                         mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN));
            ANGLE_GL_TRY(context,
                         mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE));
            ANGLE_GL_TRY(context,
                         mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA));
        }

        ANGLE_GL_TRY(context, mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
        ANGLE_GL_TRY(context, mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000));
        ANGLE_GL_TRY(context, mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                                        GL_NEAREST_MIPMAP_LINEAR));
        ANGLE_GL_TRY(context,
                     mFunctions->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        ANGLE_GL_TRY(context, mFunctions->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA,
                                                     GL_UNSIGNED_BYTE, nullptr));
    }

    return angle::Result::Continue;
}

angle::Result BlitGL::setScratchTextureParameter(const gl::Context *context,
                                                 GLenum param,
                                                 GLenum value)
{
    for (auto texture : mScratchTextures)
    {
        mStateManager->bindTexture(gl::TextureType::_2D, texture);
        ANGLE_GL_TRY(context, mFunctions->texParameteri(GL_TEXTURE_2D, param, value));
        ANGLE_GL_TRY(context, mFunctions->texParameteri(GL_TEXTURE_2D, param, value));
    }
    return angle::Result::Continue;
}

angle::Result BlitGL::setVAOState(const gl::Context *context)
{
    mStateManager->bindVertexArray(mVAO, mVAOState);
    if (mFeatures.syncAllVertexArraysToDefault.enabled)
    {
        ANGLE_TRY(initializeVAOState(context));
    }

    return angle::Result::Continue;
}

angle::Result BlitGL::initializeVAOState(const gl::Context *context)
{
    mStateManager->bindBuffer(gl::BufferBinding::Array, mVertexBuffer);

    ANGLE_GL_TRY(context, mFunctions->enableVertexAttribArray(mTexcoordAttribLocation));
    ANGLE_GL_TRY(context, mFunctions->vertexAttribPointer(mTexcoordAttribLocation, 2, GL_FLOAT,
                                                          GL_FALSE, 0, nullptr));

    VertexAttributeGL &attribute = mVAOState->attributes[mTexcoordAttribLocation];
    attribute.enabled            = true;
    attribute.format             = &angle::Format::Get(angle::FormatID::R32G32_FLOAT);
    attribute.pointer            = nullptr;

    VertexBindingGL &binding = mVAOState->bindings[mTexcoordAttribLocation];
    binding.stride           = 8;
    binding.offset           = 0;
    binding.buffer           = mVertexBuffer;

    if (mFeatures.syncAllVertexArraysToDefault.enabled)
    {
        mStateManager->setDefaultVAOStateDirty();
    }

    return angle::Result::Continue;
}

angle::Result BlitGL::getBlitProgram(const gl::Context *context,
                                     gl::TextureType sourceTextureType,
                                     GLenum sourceComponentType,
                                     GLenum destComponentType,
                                     BlitProgram **program)
{

    BlitProgramType programType(sourceTextureType, sourceComponentType, destComponentType);
    BlitProgram &result = mBlitPrograms[programType];
    if (result.program == 0)
    {
        result.program = ANGLE_GL_TRY(context, mFunctions->createProgram());

        // Depending on what types need to be output by the shaders, different versions need to be
        // used.
        constexpr const char *texcoordAttribName = "a_texcoord";
        std::string version;
        std::string vsInputVariableQualifier;
        std::string vsOutputVariableQualifier;
        std::string fsInputVariableQualifier;
        std::string fsOutputVariableQualifier;
        std::string sampleFunction;
        if (sourceComponentType != GL_UNSIGNED_INT && destComponentType != GL_UNSIGNED_INT &&
            sourceTextureType != gl::TextureType::Rectangle)
        {
            // Simple case, float-to-float with 2D or external textures.  Only needs ESSL/GLSL 100
            version                   = "100";
            vsInputVariableQualifier  = "attribute";
            vsOutputVariableQualifier = "varying";
            fsInputVariableQualifier  = "varying";
            fsOutputVariableQualifier = "";
            sampleFunction            = "texture2D";
        }
        else
        {
            // Need to use a higher version to support non-float output types
            if (mFunctions->standard == STANDARD_GL_DESKTOP)
            {
                version = "330";
            }
            else
            {
                ASSERT(mFunctions->standard == STANDARD_GL_ES);
                version = "300 es";
            }
            vsInputVariableQualifier  = "in";
            vsOutputVariableQualifier = "out";
            fsInputVariableQualifier  = "in";
            fsOutputVariableQualifier = "out";
            sampleFunction            = "texture";
        }

        {
            // Compile the vertex shader
            std::ostringstream vsSourceStream;
            vsSourceStream << "#version " << version << "\n";
            vsSourceStream << vsInputVariableQualifier << " vec2 " << texcoordAttribName << ";\n";
            vsSourceStream << "uniform vec2 u_scale;\n";
            vsSourceStream << "uniform vec2 u_offset;\n";
            vsSourceStream << vsOutputVariableQualifier << " vec2 v_texcoord;\n";
            vsSourceStream << "\n";
            vsSourceStream << "void main()\n";
            vsSourceStream << "{\n";
            vsSourceStream << "    gl_Position = vec4((" << texcoordAttribName
                           << " * 2.0) - 1.0, 0.0, 1.0);\n";
            vsSourceStream << "    v_texcoord = " << texcoordAttribName
                           << " * u_scale + u_offset;\n";
            vsSourceStream << "}\n";

            std::string vsSourceStr  = vsSourceStream.str();
            const char *vsSourceCStr = vsSourceStr.c_str();

            GLuint vs = ANGLE_GL_TRY(context, mFunctions->createShader(GL_VERTEX_SHADER));
            ANGLE_GL_TRY(context, mFunctions->shaderSource(vs, 1, &vsSourceCStr, nullptr));
            ANGLE_GL_TRY(context, mFunctions->compileShader(vs));
            ANGLE_TRY(CheckCompileStatus(context, mFunctions, vs));

            ANGLE_GL_TRY(context, mFunctions->attachShader(result.program, vs));
            ANGLE_GL_TRY(context, mFunctions->deleteShader(vs));
        }

        {
            // Sampling texture uniform changes depending on source texture type.
            std::string samplerType;
            switch (sourceTextureType)
            {
                case gl::TextureType::_2D:
                    switch (sourceComponentType)
                    {
                        case GL_UNSIGNED_INT:
                            samplerType = "usampler2D";
                            break;

                        default:  // Float type
                            samplerType = "sampler2D";
                            break;
                    }
                    break;

                case gl::TextureType::External:
                    ASSERT(sourceComponentType != GL_UNSIGNED_INT);
                    samplerType = "samplerExternalOES";
                    break;

                case gl::TextureType::Rectangle:
                    ASSERT(sourceComponentType != GL_UNSIGNED_INT);
                    samplerType = "sampler2DRect";
                    break;

                default:
                    UNREACHABLE();
                    break;
            }

            std::string samplerResultType;
            switch (sourceComponentType)
            {
                case GL_UNSIGNED_INT:
                    samplerResultType = "uvec4";
                    break;

                default:  // Float type
                    samplerResultType = "vec4";
                    break;
            }

            std::string extensionRequirements;
            switch (sourceTextureType)
            {
                case gl::TextureType::External:
                    extensionRequirements = "#extension GL_OES_EGL_image_external : require";
                    break;

                case gl::TextureType::Rectangle:
                    if (mFunctions->hasGLExtension("GL_ARB_texture_rectangle"))
                    {
                        extensionRequirements = "#extension GL_ARB_texture_rectangle : require";
                    }
                    else
                    {
                        ASSERT(mFunctions->isAtLeastGL(gl::Version(3, 1)));
                    }
                    break;

                default:
                    break;
            }

            // Output variables depend on the output type
            std::string outputType;
            std::string outputVariableName;
            std::string outputMultiplier;
            switch (destComponentType)
            {
                case GL_UNSIGNED_INT:
                    outputType         = "uvec4";
                    outputVariableName = "outputUint";
                    outputMultiplier   = "255.0";
                    break;

                default:  //  float type
                    if (version == "100")
                    {
                        outputType         = "";
                        outputVariableName = "gl_FragColor";
                        outputMultiplier   = "1.0";
                    }
                    else
                    {
                        outputType         = "vec4";
                        outputVariableName = "outputFloat";
                        outputMultiplier   = "1.0";
                    }
                    break;
            }

            // Compile the fragment shader
            std::ostringstream fsSourceStream;
            fsSourceStream << "#version " << version << "\n";
            fsSourceStream << extensionRequirements << "\n";
            fsSourceStream << "precision highp float;\n";
            fsSourceStream << "uniform " << samplerType << " u_source_texture;\n";

            // Write the rest of the uniforms and varyings
            fsSourceStream << "uniform bool u_multiply_alpha;\n";
            fsSourceStream << "uniform bool u_unmultiply_alpha;\n";
            fsSourceStream << "uniform bool u_transform_linear_to_srgb;\n";
            fsSourceStream << fsInputVariableQualifier << " vec2 v_texcoord;\n";
            if (!outputType.empty())
            {
                fsSourceStream << fsOutputVariableQualifier << " " << outputType << " "
                               << outputVariableName << ";\n";
            }

            // Write the linear to sRGB function.
            fsSourceStream << "\n";
            fsSourceStream << "float transformLinearToSrgb(float cl)\n";
            fsSourceStream << "{\n";
            fsSourceStream << "    if (cl <= 0.0)\n";
            fsSourceStream << "        return 0.0;\n";
            fsSourceStream << "    if (cl < 0.0031308)\n";
            fsSourceStream << "        return 12.92 * cl;\n";
            fsSourceStream << "    if (cl < 1.0)\n";
            fsSourceStream << "        return 1.055 * pow(cl, 0.41666) - 0.055;\n";
            fsSourceStream << "    return 1.0;\n";
            fsSourceStream << "}\n";

            // Write the main body
            fsSourceStream << "\n";
            fsSourceStream << "void main()\n";
            fsSourceStream << "{\n";

            std::string maxTexcoord;
            switch (sourceTextureType)
            {
                case gl::TextureType::Rectangle:
                    // Valid texcoords are within source texture size
                    maxTexcoord = "vec2(textureSize(u_source_texture))";
                    break;

                default:
                    // Valid texcoords are in [0, 1]
                    maxTexcoord = "vec2(1.0)";
                    break;
            }

            // discard if the texcoord is invalid so the blitframebuffer workaround doesn't
            // write when the point sampled is outside of the source framebuffer.
            fsSourceStream << "    if (clamp(v_texcoord, vec2(0.0), " << maxTexcoord
                           << ") != v_texcoord)\n";
            fsSourceStream << "    {\n";
            fsSourceStream << "        discard;\n";
            fsSourceStream << "    }\n";

            // Sampling code depends on the input data type
            fsSourceStream << "    " << samplerResultType << " color = " << sampleFunction
                           << "(u_source_texture, v_texcoord);\n";

            // Perform transformation from linear to sRGB encoding.
            fsSourceStream << "    if (u_transform_linear_to_srgb)\n";
            fsSourceStream << "    {\n";
            fsSourceStream << "        color.x = transformLinearToSrgb(color.x);\n";
            fsSourceStream << "        color.y = transformLinearToSrgb(color.y);\n";
            fsSourceStream << "        color.z = transformLinearToSrgb(color.z);\n";
            fsSourceStream << "    }\n";

            // Perform unmultiply-alpha if requested.
            fsSourceStream << "    if (u_unmultiply_alpha && color.a != 0.0)\n";
            fsSourceStream << "    {\n";
            fsSourceStream << "         color.xyz = color.xyz / color.a;\n";
            fsSourceStream << "    }\n";

            // Perform premultiply-alpha if requested.
            fsSourceStream << "    if (u_multiply_alpha)\n";
            fsSourceStream << "    {\n";
            fsSourceStream << "        color.xyz = color.xyz * color.a;\n";
            fsSourceStream << "    }\n";

            // Write the conversion to the destionation type
            fsSourceStream << "    color = color * " << outputMultiplier << ";\n";

            // Write the output assignment code
            fsSourceStream << "    " << outputVariableName << " = " << outputType << "(color);\n";
            fsSourceStream << "}\n";

            std::string fsSourceStr  = fsSourceStream.str();
            const char *fsSourceCStr = fsSourceStr.c_str();

            GLuint fs = ANGLE_GL_TRY(context, mFunctions->createShader(GL_FRAGMENT_SHADER));
            ANGLE_GL_TRY(context, mFunctions->shaderSource(fs, 1, &fsSourceCStr, nullptr));
            ANGLE_GL_TRY(context, mFunctions->compileShader(fs));
            ANGLE_TRY(CheckCompileStatus(context, mFunctions, fs));

            ANGLE_GL_TRY(context, mFunctions->attachShader(result.program, fs));
            ANGLE_GL_TRY(context, mFunctions->deleteShader(fs));
        }

        ANGLE_GL_TRY(context, mFunctions->bindAttribLocation(
                                  result.program, mTexcoordAttribLocation, texcoordAttribName));
        ANGLE_GL_TRY(context, mFunctions->linkProgram(result.program));
        ANGLE_TRY(CheckLinkStatus(context, mFunctions, result.program));

        result.sourceTextureLocation = ANGLE_GL_TRY(
            context, mFunctions->getUniformLocation(result.program, "u_source_texture"));
        result.scaleLocation =
            ANGLE_GL_TRY(context, mFunctions->getUniformLocation(result.program, "u_scale"));
        result.offsetLocation =
            ANGLE_GL_TRY(context, mFunctions->getUniformLocation(result.program, "u_offset"));
        result.multiplyAlphaLocation = ANGLE_GL_TRY(
            context, mFunctions->getUniformLocation(result.program, "u_multiply_alpha"));
        result.unMultiplyAlphaLocation = ANGLE_GL_TRY(
            context, mFunctions->getUniformLocation(result.program, "u_unmultiply_alpha"));
        result.transformLinearToSrgbLocation = ANGLE_GL_TRY(
            context, mFunctions->getUniformLocation(result.program, "u_transform_linear_to_srgb"));
    }

    *program = &result;
    return angle::Result::Continue;
}

}  // namespace rx
