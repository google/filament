//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerGL.h: Defines a class for caching applied OpenGL state

#include "libANGLE/renderer/gl/StateManagerGL.h"

#include <string.h>
#include <algorithm>
#include <limits>

#include "anglebase/numerics/safe_conversions.h"
#include "common/bitset_utils.h"
#include "common/mathutil.h"
#include "common/matrix_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Query.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/QueryGL.h"
#include "libANGLE/renderer/gl/SamplerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/TransformFeedbackGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"
#include "platform/PlatformMethods.h"

namespace rx
{

namespace
{

static void ValidateStateHelper(const FunctionsGL *functions,
                                const GLuint localValue,
                                const GLenum pname,
                                const char *localName,
                                const char *driverName)
{
    GLint queryValue;
    functions->getIntegerv(pname, &queryValue);
    if (localValue != static_cast<GLuint>(queryValue))
    {
        WARN() << localName << " (" << localValue << ") != " << driverName << " (" << queryValue
               << ")";
        // Re-add ASSERT: http://anglebug.com/42262547
        // ASSERT(false);
    }
}

}  // anonymous namespace

VertexArrayStateGL::VertexArrayStateGL(size_t maxAttribs, size_t maxBindings)
    : attributes(std::min<size_t>(maxAttribs, gl::MAX_VERTEX_ATTRIBS)),
      bindings(std::min<size_t>(maxBindings, gl::MAX_VERTEX_ATTRIBS))
{
    // Set the cached vertex attribute array and vertex attribute binding array size
    for (GLuint i = 0; i < attributes.size(); i++)
    {
        attributes[i].bindingIndex = i;
    }
}

StateManagerGL::IndexedBufferBinding::IndexedBufferBinding() : offset(0), size(0), buffer(0) {}

StateManagerGL::StateManagerGL(const FunctionsGL *functions,
                               const gl::Caps &rendererCaps,
                               const gl::Extensions &extensions,
                               const angle::FeaturesGL &features)
    : mFunctions(functions),
      mFeatures(features),
      mProgram(0),
      mSupportsVertexArrayObjects(nativegl::SupportsVertexArrayObjects(functions)),
      mVAO(0),
      mVertexAttribCurrentValues(rendererCaps.maxVertexAttributes),
      mDefaultVAOState(rendererCaps.maxVertexAttributes, rendererCaps.maxVertexAttribBindings),
      mVAOState(&mDefaultVAOState),
      mBuffers(),
      mIndexedBuffers(),
      mTextureUnitIndex(0),
      mTextures{},
      mSamplers{},
      mImages(rendererCaps.maxImageUnits, ImageUnitBinding()),
      mTransformFeedback(0),
      mCurrentTransformFeedback(nullptr),
      mQueries(),
      mPrevDrawContext({0}),
      mUnpackAlignment(4),
      mUnpackRowLength(0),
      mUnpackSkipRows(0),
      mUnpackSkipPixels(0),
      mUnpackImageHeight(0),
      mUnpackSkipImages(0),
      mPackAlignment(4),
      mPackRowLength(0),
      mPackSkipRows(0),
      mPackSkipPixels(0),
      mFramebuffers(angle::FramebufferBindingSingletonMax, 0),
      mRenderbuffer(0),
      mPlaceholderFbo(0),
      mPlaceholderRbo(0),
      mScissorTestEnabled(false),
      mScissor(0, 0, 0, 0),
      mViewport(0, 0, 0, 0),
      mNear(0.0f),
      mFar(1.0f),
      mClipOrigin(gl::ClipOrigin::LowerLeft),
      mClipDepthMode(gl::ClipDepthMode::NegativeOneToOne),
      mBlendColor(0, 0, 0, 0),
      mBlendStateExt(rendererCaps.maxDrawBuffers),
      mBlendAdvancedCoherent(extensions.blendEquationAdvancedCoherentKHR),
      mIndependentBlendStates(extensions.drawBuffersIndexedAny()),
      mSampleAlphaToCoverageEnabled(false),
      mSampleCoverageEnabled(false),
      mSampleCoverageValue(1.0f),
      mSampleCoverageInvert(false),
      mSampleMaskEnabled(false),
      mDepthTestEnabled(false),
      mDepthFunc(GL_LESS),
      mDepthMask(true),
      mStencilTestEnabled(false),
      mStencilFrontFunc(GL_ALWAYS),
      mStencilFrontRef(0),
      mStencilFrontValueMask(static_cast<GLuint>(-1)),
      mStencilFrontStencilFailOp(GL_KEEP),
      mStencilFrontStencilPassDepthFailOp(GL_KEEP),
      mStencilFrontStencilPassDepthPassOp(GL_KEEP),
      mStencilFrontWritemask(static_cast<GLuint>(-1)),
      mStencilBackFunc(GL_ALWAYS),
      mStencilBackRef(0),
      mStencilBackValueMask(static_cast<GLuint>(-1)),
      mStencilBackStencilFailOp(GL_KEEP),
      mStencilBackStencilPassDepthFailOp(GL_KEEP),
      mStencilBackStencilPassDepthPassOp(GL_KEEP),
      mStencilBackWritemask(static_cast<GLuint>(-1)),
      mCullFaceEnabled(false),
      mCullFace(gl::CullFaceMode::Back),
      mFrontFace(GL_CCW),
      mPolygonMode(gl::PolygonMode::Fill),
      mPolygonOffsetPointEnabled(false),
      mPolygonOffsetLineEnabled(false),
      mPolygonOffsetFillEnabled(false),
      mPolygonOffsetFactor(0.0f),
      mPolygonOffsetUnits(0.0f),
      mPolygonOffsetClamp(0.0f),
      mDepthClampEnabled(false),
      mRasterizerDiscardEnabled(false),
      mLineWidth(1.0f),
      mPrimitiveRestartEnabled(false),
      mPrimitiveRestartIndex(0),
      mClearColor(0.0f, 0.0f, 0.0f, 0.0f),
      mClearDepth(1.0f),
      mClearStencil(0),
      mFramebufferSRGBAvailable(extensions.sRGBWriteControlEXT),
      mFramebufferSRGBEnabled(false),
      mHasSeparateFramebufferBindings(mFunctions->isAtLeastGL(gl::Version(3, 0)) ||
                                      mFunctions->isAtLeastGLES(gl::Version(3, 0))),
      mDitherEnabled(true),
      mTextureCubemapSeamlessEnabled(false),
      mMultisamplingEnabled(true),
      mSampleAlphaToOneEnabled(false),
      mCoverageModulation(GL_NONE),
      mIsMultiviewEnabled(extensions.multiviewOVR || extensions.multiview2OVR),
      mProvokingVertex(GL_LAST_VERTEX_CONVENTION),
      mMaxClipDistances(rendererCaps.maxClipDistances),
      mLogicOpEnabled(false),
      mLogicOp(gl::LogicalOperation::Copy)
{
    ASSERT(mFunctions);
    ASSERT(rendererCaps.maxViews >= 1u);

    mIndexedBuffers[gl::BufferBinding::Uniform].resize(rendererCaps.maxUniformBufferBindings);
    mIndexedBuffers[gl::BufferBinding::AtomicCounter].resize(
        rendererCaps.maxAtomicCounterBufferBindings);
    mIndexedBuffers[gl::BufferBinding::ShaderStorage].resize(
        rendererCaps.maxShaderStorageBufferBindings);

    mSampleMaskValues.fill(~GLbitfield(0));

    mQueries.fill(nullptr);
    mTemporaryPausedQueries.fill(nullptr);

    // Initialize point sprite state for desktop GL
    if (mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        mFunctions->enable(GL_PROGRAM_POINT_SIZE);

        // GL_POINT_SPRITE was deprecated in the core profile. Point rasterization is always
        // performed
        // as though POINT_SPRITE were enabled.
        if ((mFunctions->profile & GL_CONTEXT_CORE_PROFILE_BIT) == 0)
        {
            mFunctions->enable(GL_POINT_SPRITE);
        }
    }

    if (features.emulatePrimitiveRestartFixedIndex.enabled)
    {
        // There is no consistent default value for primitive restart index. Set it to UINT -1.
        constexpr GLuint primitiveRestartIndex = gl::GetPrimitiveRestartIndexFromType<GLuint>();
        mFunctions->primitiveRestartIndex(primitiveRestartIndex);
        mPrimitiveRestartIndex = primitiveRestartIndex;
    }

    // It's possible we've enabled the emulated VAO feature for testing but we're on a core profile.
    // Use a generated VAO as the default VAO so we can still test.
    if ((features.syncAllVertexArraysToDefault.enabled ||
         features.syncDefaultVertexArraysToDefault.enabled) &&
        !nativegl::CanUseDefaultVertexArrayObject(mFunctions))
    {
        ASSERT(nativegl::SupportsVertexArrayObjects(mFunctions));
        mFunctions->genVertexArrays(1, &mDefaultVAO);
        mFunctions->bindVertexArray(mDefaultVAO);
        mVAO = mDefaultVAO;
    }

    // By default, desktop GL clamps values read from normalized
    // color buffers to [0, 1], which does not match expected ES
    // behavior for signed normalized color buffers.
    if (mFunctions->clampColor)
    {
        mFunctions->clampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
    }
}

StateManagerGL::~StateManagerGL()
{
    if (mPlaceholderFbo != 0)
    {
        deleteFramebuffer(mPlaceholderFbo);
    }
    if (mPlaceholderRbo != 0)
    {
        deleteRenderbuffer(mPlaceholderRbo);
    }
    if (mDefaultVAO != 0)
    {
        mFunctions->deleteVertexArrays(1, &mDefaultVAO);
    }
}

void StateManagerGL::deleteProgram(GLuint program)
{
    if (program != 0)
    {
        if (mProgram == program)
        {
            useProgram(0);
        }

        mFunctions->deleteProgram(program);
    }
}

void StateManagerGL::deleteVertexArray(GLuint vao)
{
    if (vao != 0)
    {
        if (mVAO == vao)
        {
            bindVertexArray(0, &mDefaultVAOState);
        }
        mFunctions->deleteVertexArrays(1, &vao);
    }
}

void StateManagerGL::deleteTexture(GLuint texture)
{
    if (texture != 0)
    {
        for (gl::TextureType type : angle::AllEnums<gl::TextureType>())
        {
            const auto &textureVector = mTextures[type];
            for (size_t textureUnitIndex = 0; textureUnitIndex < textureVector.size();
                 textureUnitIndex++)
            {
                if (textureVector[textureUnitIndex] == texture)
                {
                    activeTexture(textureUnitIndex);
                    bindTexture(type, 0);
                }
            }
        }

        for (size_t imageUnitIndex = 0; imageUnitIndex < mImages.size(); imageUnitIndex++)
        {
            if (mImages[imageUnitIndex].texture == texture)
            {
                bindImageTexture(imageUnitIndex, 0, 0, false, 0, GL_READ_ONLY, GL_R32UI);
            }
        }

        mFunctions->deleteTextures(1, &texture);
    }
}

void StateManagerGL::deleteSampler(GLuint sampler)
{
    if (sampler != 0)
    {
        for (size_t unit = 0; unit < mSamplers.size(); unit++)
        {
            if (mSamplers[unit] == sampler)
            {
                bindSampler(unit, 0);
            }
        }

        mFunctions->deleteSamplers(1, &sampler);
    }
}

void StateManagerGL::deleteBuffer(GLuint buffer)
{
    if (buffer == 0)
    {
        return;
    }

    for (auto target : angle::AllEnums<gl::BufferBinding>())
    {
        if (mBuffers[target] == buffer)
        {
            bindBuffer(target, 0);
        }

        auto &indexedTarget = mIndexedBuffers[target];
        for (size_t bindIndex = 0; bindIndex < indexedTarget.size(); ++bindIndex)
        {
            if (indexedTarget[bindIndex].buffer == buffer)
            {
                bindBufferBase(target, bindIndex, 0);
            }
        }
    }

    if (mVAOState)
    {
        if (mVAOState->elementArrayBuffer == buffer)
        {
            mVAOState->elementArrayBuffer = 0;
        }

        for (VertexBindingGL &binding : mVAOState->bindings)
        {
            if (binding.buffer == buffer)
            {
                binding.buffer = 0;
            }
        }
    }

    mFunctions->deleteBuffers(1, &buffer);
}

void StateManagerGL::deleteFramebuffer(GLuint fbo)
{
    if (fbo != 0)
    {
        if (mHasSeparateFramebufferBindings)
        {
            for (size_t binding = 0; binding < mFramebuffers.size(); ++binding)
            {
                if (mFramebuffers[binding] == fbo)
                {
                    GLenum enumValue = angle::FramebufferBindingToEnum(
                        static_cast<angle::FramebufferBinding>(binding));
                    bindFramebuffer(enumValue, 0);
                }
            }
        }
        else
        {
            ASSERT(mFramebuffers[angle::FramebufferBindingRead] ==
                   mFramebuffers[angle::FramebufferBindingDraw]);
            if (mFramebuffers[angle::FramebufferBindingRead] == fbo)
            {
                bindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }
        mFunctions->deleteFramebuffers(1, &fbo);
    }
}

void StateManagerGL::deleteRenderbuffer(GLuint rbo)
{
    if (rbo != 0)
    {
        if (mRenderbuffer == rbo)
        {
            bindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        mFunctions->deleteRenderbuffers(1, &rbo);
    }
}

void StateManagerGL::deleteTransformFeedback(GLuint transformFeedback)
{
    if (transformFeedback != 0)
    {
        if (mTransformFeedback == transformFeedback)
        {
            bindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        }

        if (mCurrentTransformFeedback != nullptr &&
            mCurrentTransformFeedback->getTransformFeedbackID() == transformFeedback)
        {
            mCurrentTransformFeedback = nullptr;
        }

        mFunctions->deleteTransformFeedbacks(1, &transformFeedback);
    }
}

void StateManagerGL::useProgram(GLuint program)
{
    if (mProgram != program)
    {
        forceUseProgram(program);
    }
}

void StateManagerGL::forceUseProgram(GLuint program)
{
    mProgram = program;
    mFunctions->useProgram(mProgram);
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_PROGRAM_BINDING);
}

void StateManagerGL::bindVertexArray(GLuint vao, VertexArrayStateGL *vaoState)
{
    if (mVAO != vao)
    {
        ASSERT(!mFeatures.syncAllVertexArraysToDefault.enabled);
        forceBindVertexArray(vao, vaoState);
    }
}

void StateManagerGL::forceBindVertexArray(GLuint vao, VertexArrayStateGL *vaoState)
{
    mVAO                                      = vao;
    mVAOState                                 = vaoState;
    mBuffers[gl::BufferBinding::ElementArray] = vaoState ? vaoState->elementArrayBuffer : 0;

    mFunctions->bindVertexArray(vao);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING);
}

void StateManagerGL::bindBuffer(gl::BufferBinding target, GLuint buffer)
{
    // GL drivers differ in whether the transform feedback bind point is modified when
    // glBindTransformFeedback is called. To avoid these behavior differences we shouldn't try to
    // use it.
    ASSERT(target != gl::BufferBinding::TransformFeedback);
    if (mBuffers[target] != buffer)
    {
        mBuffers[target] = buffer;
        mFunctions->bindBuffer(gl::ToGLenum(target), buffer);
    }
}

void StateManagerGL::bindBufferBase(gl::BufferBinding target, size_t index, GLuint buffer)
{
    // Transform feedback buffer bindings are tracked in TransformFeedbackGL
    ASSERT(target != gl::BufferBinding::TransformFeedback);

    ASSERT(index < mIndexedBuffers[target].size());
    auto &binding = mIndexedBuffers[target][index];
    if (binding.buffer != buffer || binding.offset != static_cast<size_t>(-1) ||
        binding.size != static_cast<size_t>(-1))
    {
        binding.buffer   = buffer;
        binding.offset   = static_cast<size_t>(-1);
        binding.size     = static_cast<size_t>(-1);
        mBuffers[target] = buffer;
        mFunctions->bindBufferBase(gl::ToGLenum(target), static_cast<GLuint>(index), buffer);
    }
}

void StateManagerGL::bindBufferRange(gl::BufferBinding target,
                                     size_t index,
                                     GLuint buffer,
                                     size_t offset,
                                     size_t size)
{
    // Transform feedback buffer bindings are tracked in TransformFeedbackGL
    ASSERT(target != gl::BufferBinding::TransformFeedback);

    auto &binding = mIndexedBuffers[target][index];
    if (binding.buffer != buffer || binding.offset != offset || binding.size != size)
    {
        binding.buffer   = buffer;
        binding.offset   = offset;
        binding.size     = size;
        mBuffers[target] = buffer;
        mFunctions->bindBufferRange(gl::ToGLenum(target), static_cast<GLuint>(index), buffer,
                                    offset, size);
    }
}

void StateManagerGL::activeTexture(size_t unit)
{
    if (mTextureUnitIndex != unit)
    {
        mTextureUnitIndex = unit;
        mFunctions->activeTexture(GL_TEXTURE0 + static_cast<GLenum>(mTextureUnitIndex));
    }
}

void StateManagerGL::bindTexture(gl::TextureType type, GLuint texture)
{
    gl::TextureType nativeType = nativegl::GetNativeTextureType(type);
    if (mTextures[nativeType][mTextureUnitIndex] != texture)
    {
        mTextures[nativeType][mTextureUnitIndex] = texture;
        mFunctions->bindTexture(nativegl::GetTextureBindingTarget(type), texture);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_TEXTURE_BINDINGS);
    }
}

void StateManagerGL::invalidateTexture(gl::TextureType type)
{
    // Assume the tracked texture binding is incorrect, query the real bound texture from GL.
    GLint boundTexture = 0;
    mFunctions->getIntegerv(nativegl::GetTextureBindingQuery(type), &boundTexture);
    mTextures[type][mTextureUnitIndex] = static_cast<GLuint>(boundTexture);
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_TEXTURE_BINDINGS);
}

void StateManagerGL::bindSampler(size_t unit, GLuint sampler)
{
    if (mSamplers[unit] != sampler)
    {
        mSamplers[unit] = sampler;
        mFunctions->bindSampler(static_cast<GLuint>(unit), sampler);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLER_BINDINGS);
    }
}

void StateManagerGL::bindImageTexture(size_t unit,
                                      GLuint texture,
                                      GLint level,
                                      GLboolean layered,
                                      GLint layer,
                                      GLenum access,
                                      GLenum format)
{
    auto &binding = mImages[unit];
    if (binding.texture != texture || binding.level != level || binding.layered != layered ||
        binding.layer != layer || binding.access != access || binding.format != format)
    {
        binding.texture = texture;
        binding.level   = level;
        binding.layered = layered;
        binding.layer   = layer;
        binding.access  = access;
        binding.format  = format;
        mFunctions->bindImageTexture(angle::base::checked_cast<GLuint>(unit), texture, level,
                                     layered, layer, access, format);
    }
}

angle::Result StateManagerGL::setPixelUnpackState(const gl::Context *context,
                                                  const gl::PixelUnpackState &unpack)
{
    if (mUnpackAlignment != unpack.alignment)
    {
        mUnpackAlignment = unpack.alignment;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_UNPACK_ALIGNMENT, mUnpackAlignment));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }

    if (mUnpackRowLength != unpack.rowLength)
    {
        mUnpackRowLength = unpack.rowLength;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_UNPACK_ROW_LENGTH, mUnpackRowLength));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }

    if (mUnpackSkipRows != unpack.skipRows)
    {
        mUnpackSkipRows = unpack.skipRows;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_UNPACK_SKIP_ROWS, mUnpackSkipRows));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }

    if (mUnpackSkipPixels != unpack.skipPixels)
    {
        mUnpackSkipPixels = unpack.skipPixels;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_UNPACK_SKIP_PIXELS, mUnpackSkipPixels));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }

    if (mUnpackImageHeight != unpack.imageHeight)
    {
        mUnpackImageHeight = unpack.imageHeight;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_UNPACK_IMAGE_HEIGHT, mUnpackImageHeight));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }

    if (mUnpackSkipImages != unpack.skipImages)
    {
        mUnpackSkipImages = unpack.skipImages;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_UNPACK_SKIP_IMAGES, mUnpackSkipImages));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }

    return angle::Result::Continue;
}

angle::Result StateManagerGL::setPixelUnpackBuffer(const gl::Context *context,
                                                   const gl::Buffer *pixelBuffer)
{
    GLuint bufferID = 0;
    if (pixelBuffer != nullptr)
    {
        bufferID = GetImplAs<BufferGL>(pixelBuffer)->getBufferID();
    }
    bindBuffer(gl::BufferBinding::PixelUnpack, bufferID);

    return angle::Result::Continue;
}

angle::Result StateManagerGL::setPixelPackState(const gl::Context *context,
                                                const gl::PixelPackState &pack)
{
    if (mPackAlignment != pack.alignment)
    {
        mPackAlignment = pack.alignment;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_PACK_ALIGNMENT, mPackAlignment));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PACK_STATE);
    }

    if (mPackRowLength != pack.rowLength)
    {
        mPackRowLength = pack.rowLength;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_PACK_ROW_LENGTH, mPackRowLength));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PACK_STATE);
    }

    if (mPackSkipRows != pack.skipRows)
    {
        mPackSkipRows = pack.skipRows;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_PACK_SKIP_ROWS, mPackSkipRows));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PACK_STATE);
    }

    if (mPackSkipPixels != pack.skipPixels)
    {
        mPackSkipPixels = pack.skipPixels;
        ANGLE_GL_TRY(context, mFunctions->pixelStorei(GL_PACK_SKIP_PIXELS, mPackSkipPixels));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PACK_STATE);
    }

    return angle::Result::Continue;
}

angle::Result StateManagerGL::setPixelPackBuffer(const gl::Context *context,
                                                 const gl::Buffer *pixelBuffer)
{
    GLuint bufferID = 0;
    if (pixelBuffer != nullptr)
    {
        bufferID = GetImplAs<BufferGL>(pixelBuffer)->getBufferID();
    }
    bindBuffer(gl::BufferBinding::PixelPack, bufferID);

    return angle::Result::Continue;
}

void StateManagerGL::bindFramebuffer(GLenum type, GLuint framebuffer)
{
    bool framebufferChanged = false;
    switch (type)
    {
        case GL_FRAMEBUFFER:
            if (mFramebuffers[angle::FramebufferBindingRead] != framebuffer ||
                mFramebuffers[angle::FramebufferBindingDraw] != framebuffer)
            {
                mFramebuffers[angle::FramebufferBindingRead] = framebuffer;
                mFramebuffers[angle::FramebufferBindingDraw] = framebuffer;
                mFunctions->bindFramebuffer(GL_FRAMEBUFFER, framebuffer);

                mLocalDirtyBits.set(gl::state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING);
                mLocalDirtyBits.set(gl::state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING);

                framebufferChanged = true;
            }
            break;

        case GL_READ_FRAMEBUFFER:
            ASSERT(mHasSeparateFramebufferBindings);
            if (mFramebuffers[angle::FramebufferBindingRead] != framebuffer)
            {
                mFramebuffers[angle::FramebufferBindingRead] = framebuffer;
                mFunctions->bindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

                mLocalDirtyBits.set(gl::state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING);

                framebufferChanged = true;
            }
            break;

        case GL_DRAW_FRAMEBUFFER:
            ASSERT(mHasSeparateFramebufferBindings);
            if (mFramebuffers[angle::FramebufferBindingDraw] != framebuffer)
            {
                mFramebuffers[angle::FramebufferBindingDraw] = framebuffer;
                mFunctions->bindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

                mLocalDirtyBits.set(gl::state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING);

                framebufferChanged = true;
            }
            break;

        default:
            UNREACHABLE();
            break;
    }

    if (framebufferChanged && mFeatures.flushOnFramebufferChange.enabled)
    {
        mFunctions->flush();
    }
}

void StateManagerGL::bindRenderbuffer(GLenum type, GLuint renderbuffer)
{
    ASSERT(type == GL_RENDERBUFFER);
    if (mRenderbuffer != renderbuffer)
    {
        mRenderbuffer = renderbuffer;
        mFunctions->bindRenderbuffer(type, mRenderbuffer);
    }
}

void StateManagerGL::bindTransformFeedback(GLenum type, GLuint transformFeedback)
{
    ASSERT(type == GL_TRANSFORM_FEEDBACK);
    if (mTransformFeedback != transformFeedback)
    {
        // Pause the current transform feedback if one is active.
        // To handle virtualized contexts, StateManagerGL needs to be able to bind a new transform
        // feedback at any time, even if there is one active.
        if (mCurrentTransformFeedback != nullptr &&
            mCurrentTransformFeedback->getTransformFeedbackID() != transformFeedback)
        {
            mCurrentTransformFeedback->syncPausedState(true);
            mCurrentTransformFeedback = nullptr;
        }

        mTransformFeedback = transformFeedback;
        mFunctions->bindTransformFeedback(type, mTransformFeedback);
        onTransformFeedbackStateChange();
    }
}

void StateManagerGL::onTransformFeedbackStateChange()
{
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_TRANSFORM_FEEDBACK_BINDING);
}

void StateManagerGL::beginQuery(gl::QueryType type, QueryGL *queryObject, GLuint queryId)
{
    // Make sure this is a valid query type and there is no current active query of this type
    ASSERT(mQueries[type] == nullptr);
    ASSERT(queryId != 0);

    GLuint oldFramebufferBindingDraw = mFramebuffers[angle::FramebufferBindingDraw];
    if (mFeatures.bindCompleteFramebufferForTimerQueries.enabled &&
        (mFramebuffers[angle::FramebufferBindingDraw] == 0 ||
         mFunctions->checkFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) &&
        (type == gl::QueryType::TimeElapsed || type == gl::QueryType::Timestamp))
    {
        if (!mPlaceholderFbo)
        {
            mFunctions->genFramebuffers(1, &mPlaceholderFbo);
        }
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, mPlaceholderFbo);

        if (!mPlaceholderRbo)
        {
            GLuint oldRenderBufferBinding = mRenderbuffer;
            mFunctions->genRenderbuffers(1, &mPlaceholderRbo);
            bindRenderbuffer(GL_RENDERBUFFER, mPlaceholderRbo);
            mFunctions->renderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 2, 2);
            mFunctions->framebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                GL_RENDERBUFFER, mPlaceholderRbo);
            bindRenderbuffer(GL_RENDERBUFFER, oldRenderBufferBinding);

            // This ensures renderbuffer attachment is not lazy.
            mFunctions->checkFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        }
    }

    mQueries[type] = queryObject;
    mFunctions->beginQuery(ToGLenum(type), queryId);

    if (oldFramebufferBindingDraw != mPlaceholderFbo)
    {
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, oldFramebufferBindingDraw);
    }
}

void StateManagerGL::endQuery(gl::QueryType type, QueryGL *queryObject, GLuint queryId)
{
    ASSERT(queryObject != nullptr);
    ASSERT(mQueries[type] == queryObject);
    mQueries[type] = nullptr;
    mFunctions->endQuery(ToGLenum(type));
}

void StateManagerGL::updateDrawIndirectBufferBinding(const gl::Context *context)
{
    gl::Buffer *drawIndirectBuffer =
        context->getState().getTargetBuffer(gl::BufferBinding::DrawIndirect);
    if (drawIndirectBuffer != nullptr)
    {
        const BufferGL *bufferGL = GetImplAs<BufferGL>(drawIndirectBuffer);
        bindBuffer(gl::BufferBinding::DrawIndirect, bufferGL->getBufferID());
    }
}

void StateManagerGL::updateDispatchIndirectBufferBinding(const gl::Context *context)
{
    gl::Buffer *dispatchIndirectBuffer =
        context->getState().getTargetBuffer(gl::BufferBinding::DispatchIndirect);
    if (dispatchIndirectBuffer != nullptr)
    {
        const BufferGL *bufferGL = GetImplAs<BufferGL>(dispatchIndirectBuffer);
        bindBuffer(gl::BufferBinding::DispatchIndirect, bufferGL->getBufferID());
    }
}

void StateManagerGL::pauseTransformFeedback()
{
    if (mCurrentTransformFeedback != nullptr)
    {
        mCurrentTransformFeedback->syncPausedState(true);
        onTransformFeedbackStateChange();
    }
}

angle::Result StateManagerGL::pauseAllQueries(const gl::Context *context)
{
    for (gl::QueryType type : angle::AllEnums<gl::QueryType>())
    {
        QueryGL *previousQuery = mQueries[type];

        if (previousQuery != nullptr)
        {
            ANGLE_TRY(previousQuery->pause(context));
            mTemporaryPausedQueries[type] = previousQuery;
            mQueries[type]                = nullptr;
        }
    }

    return angle::Result::Continue;
}

angle::Result StateManagerGL::pauseQuery(const gl::Context *context, gl::QueryType type)
{
    QueryGL *previousQuery = mQueries[type];

    if (previousQuery)
    {
        ANGLE_TRY(previousQuery->pause(context));
        mTemporaryPausedQueries[type] = previousQuery;
        mQueries[type]                = nullptr;
    }

    return angle::Result::Continue;
}

angle::Result StateManagerGL::resumeAllQueries(const gl::Context *context)
{
    for (gl::QueryType type : angle::AllEnums<gl::QueryType>())
    {
        QueryGL *pausedQuery = mTemporaryPausedQueries[type];

        if (pausedQuery != nullptr)
        {
            ASSERT(mQueries[type] == nullptr);
            ANGLE_TRY(pausedQuery->resume(context));
            mTemporaryPausedQueries[type] = nullptr;
        }
    }

    return angle::Result::Continue;
}

angle::Result StateManagerGL::resumeQuery(const gl::Context *context, gl::QueryType type)
{
    QueryGL *pausedQuery = mTemporaryPausedQueries[type];

    if (pausedQuery != nullptr)
    {
        ANGLE_TRY(pausedQuery->resume(context));
        mTemporaryPausedQueries[type] = nullptr;
    }

    return angle::Result::Continue;
}

angle::Result StateManagerGL::onMakeCurrent(const gl::Context *context)
{
    const gl::State &glState = context->getState();

#if defined(ANGLE_ENABLE_ASSERTS)
    // Temporarily pausing queries during context switch is not supported
    for (QueryGL *pausedQuery : mTemporaryPausedQueries)
    {
        ASSERT(pausedQuery == nullptr);
    }
#endif

    // If the context has changed, pause the previous context's queries
    auto contextID = context->getState().getContextID();
    if (contextID != mPrevDrawContext)
    {
        for (gl::QueryType type : angle::AllEnums<gl::QueryType>())
        {
            QueryGL *currentQuery = mQueries[type];
            // Pause any old query object
            if (currentQuery != nullptr)
            {
                ANGLE_TRY(currentQuery->pause(context));
                mQueries[type] = nullptr;
            }

            // Check if this new context needs to resume a query
            gl::Query *newQuery = glState.getActiveQuery(type);
            if (newQuery != nullptr)
            {
                QueryGL *queryGL = GetImplAs<QueryGL>(newQuery);
                ANGLE_TRY(queryGL->resume(context));
            }
        }
    }
    onTransformFeedbackStateChange();
    mPrevDrawContext = contextID;

    // Seamless cubemaps are required for ES3 and higher contexts. It should be the cheapest to set
    // this state here since MakeCurrent is expected to be called less frequently than draw calls.
    setTextureCubemapSeamlessEnabled(context->getClientMajorVersion() >= 3);

    return angle::Result::Continue;
}

void StateManagerGL::updateProgramTextureBindings(const gl::Context *context)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();

    // It is possible there is no active program during a path operation.
    if (!executable)
        return;

    const gl::ActiveTexturesCache &textures        = glState.getActiveTexturesCache();
    const gl::ActiveTextureMask &activeTextures    = executable->getActiveSamplersMask();
    const gl::ActiveTextureTypeArray &textureTypes = executable->getActiveSamplerTypes();

    for (size_t textureUnitIndex : activeTextures)
    {
        gl::TextureType textureType = textureTypes[textureUnitIndex];
        gl::Texture *texture        = textures[textureUnitIndex];

        // A nullptr texture indicates incomplete.
        if (texture != nullptr)
        {
            const TextureGL *textureGL = GetImplAs<TextureGL>(texture);
            // The DIRTY_BIT_BOUND_AS_ATTACHMENT may get inserted when texture is attached to
            // FBO and if texture is already bound, Texture::syncState will not get called and dirty
            // bit not gets cleared. But this bit is not used by GL backend at all, so it is
            // harmless even though we expect texture is clean when reaching here. The bit will
            // still get cleared next time syncState been called.
            ASSERT(!texture->hasAnyDirtyBitExcludingBoundAsAttachmentBit());
            ASSERT(!textureGL->hasAnyDirtyBit());

            activeTexture(textureUnitIndex);
            bindTexture(textureType, textureGL->getTextureID());
        }
        else
        {
            activeTexture(textureUnitIndex);
            bindTexture(textureType, 0);
        }
    }
}

void StateManagerGL::updateProgramStorageBufferBindings(const gl::Context *context)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();

    for (size_t blockIndex = 0; blockIndex < executable->getShaderStorageBlocks().size();
         blockIndex++)
    {
        GLuint binding = executable->getShaderStorageBlockBinding(static_cast<GLuint>(blockIndex));
        const auto &shaderStorageBuffer = glState.getIndexedShaderStorageBuffer(binding);

        if (shaderStorageBuffer.get() != nullptr)
        {
            BufferGL *bufferGL = GetImplAs<BufferGL>(shaderStorageBuffer.get());

            if (shaderStorageBuffer.getSize() == 0)
            {
                bindBufferBase(gl::BufferBinding::ShaderStorage, binding, bufferGL->getBufferID());
            }
            else
            {
                bindBufferRange(gl::BufferBinding::ShaderStorage, binding, bufferGL->getBufferID(),
                                shaderStorageBuffer.getOffset(), shaderStorageBuffer.getSize());
            }
        }
    }
}

void StateManagerGL::updateProgramUniformBufferBindings(const gl::Context *context)
{
    // Sync the current program executable state
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    ProgramExecutableGL *executableGL       = GetImplAs<ProgramExecutableGL>(executable);

    // If any calls to glUniformBlockBinding have been made, make them effective.  Note that if PPOs
    // are ever supported in this backend, this needs to look at the Program's attached to PPOs
    // instead of the PPOs own executable.  This is because glUniformBlockBinding operates on
    // programs directly.
    executableGL->syncUniformBlockBindings();

    for (size_t uniformBlockIndex = 0; uniformBlockIndex < executable->getUniformBlocks().size();
         uniformBlockIndex++)
    {
        GLuint binding = executable->getUniformBlockBinding(static_cast<GLuint>(uniformBlockIndex));
        const auto &uniformBuffer = glState.getIndexedUniformBuffer(binding);

        if (uniformBuffer.get() != nullptr)
        {
            BufferGL *bufferGL = GetImplAs<BufferGL>(uniformBuffer.get());

            if (uniformBuffer.getSize() == 0)
            {
                bindBufferBase(gl::BufferBinding::Uniform, binding, bufferGL->getBufferID());
            }
            else
            {
                bindBufferRange(gl::BufferBinding::Uniform, binding, bufferGL->getBufferID(),
                                uniformBuffer.getOffset(), uniformBuffer.getSize());
            }
        }
    }
}

void StateManagerGL::updateProgramAtomicCounterBufferBindings(const gl::Context *context)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();

    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers =
        executable->getAtomicCounterBuffers();
    for (size_t index = 0; index < atomicCounterBuffers.size(); ++index)
    {
        const GLuint binding = executable->getAtomicCounterBufferBinding(index);
        const auto &buffer   = glState.getIndexedAtomicCounterBuffer(binding);

        if (buffer.get() != nullptr)
        {
            BufferGL *bufferGL = GetImplAs<BufferGL>(buffer.get());

            if (buffer.getSize() == 0)
            {
                bindBufferBase(gl::BufferBinding::AtomicCounter, binding, bufferGL->getBufferID());
            }
            else
            {
                bindBufferRange(gl::BufferBinding::AtomicCounter, binding, bufferGL->getBufferID(),
                                buffer.getOffset(), buffer.getSize());
            }
        }
    }
}

void StateManagerGL::updateProgramImageBindings(const gl::Context *context)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();

    // It is possible there is no active program during a path operation.
    if (!executable)
        return;

    ASSERT(context->getClientVersion() >= gl::ES_3_1 ||
           context->getExtensions().shaderPixelLocalStorageANGLE ||
           executable->getImageBindings().empty());
    for (size_t imageUnitIndex : executable->getActiveImagesMask())
    {
        const gl::ImageUnit &imageUnit = glState.getImageUnit(imageUnitIndex);
        const TextureGL *textureGL     = SafeGetImplAs<TextureGL>(imageUnit.texture.get());
        if (textureGL)
        {
            // Do not set layer parameters for non-layered texture types to avoid driver bugs.
            const bool layered = IsLayeredTextureType(textureGL->getType());
            bindImageTexture(imageUnitIndex, textureGL->getTextureID(), imageUnit.level,
                             layered && imageUnit.layered, layered ? imageUnit.layer : 0,
                             imageUnit.access, imageUnit.format);
        }
        else
        {
            bindImageTexture(imageUnitIndex, 0, imageUnit.level, imageUnit.layered, imageUnit.layer,
                             imageUnit.access, imageUnit.format);
        }
    }
}

void StateManagerGL::setAttributeCurrentData(size_t index,
                                             const gl::VertexAttribCurrentValueData &data)
{
    if (mVertexAttribCurrentValues[index] != data)
    {
        mVertexAttribCurrentValues[index] = data;
        switch (mVertexAttribCurrentValues[index].Type)
        {
            case gl::VertexAttribType::Float:
                mFunctions->vertexAttrib4fv(static_cast<GLuint>(index),
                                            mVertexAttribCurrentValues[index].Values.FloatValues);
                break;
            case gl::VertexAttribType::Int:
                mFunctions->vertexAttribI4iv(static_cast<GLuint>(index),
                                             mVertexAttribCurrentValues[index].Values.IntValues);
                break;
            case gl::VertexAttribType::UnsignedInt:
                mFunctions->vertexAttribI4uiv(
                    static_cast<GLuint>(index),
                    mVertexAttribCurrentValues[index].Values.UnsignedIntValues);
                break;
            default:
                UNREACHABLE();
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CURRENT_VALUES);
        mLocalDirtyCurrentValues.set(index);
    }
}

void StateManagerGL::setScissorTestEnabled(bool enabled)
{
    if (mScissorTestEnabled != enabled)
    {
        mScissorTestEnabled = enabled;
        if (mScissorTestEnabled)
        {
            mFunctions->enable(GL_SCISSOR_TEST);
        }
        else
        {
            mFunctions->disable(GL_SCISSOR_TEST);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SCISSOR_TEST_ENABLED);
    }
}

void StateManagerGL::setScissor(const gl::Rectangle &scissor)
{
    if (scissor != mScissor)
    {
        mScissor = scissor;
        mFunctions->scissor(mScissor.x, mScissor.y, mScissor.width, mScissor.height);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SCISSOR);
    }
}

void StateManagerGL::setViewport(const gl::Rectangle &viewport)
{
    if (viewport != mViewport)
    {
        mViewport = viewport;
        mFunctions->viewport(mViewport.x, mViewport.y, mViewport.width, mViewport.height);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_VIEWPORT);
    }
}

void StateManagerGL::setDepthRange(float near, float far)
{
    mNear = near;
    mFar  = far;

    // The glDepthRangef function isn't available until OpenGL 4.1.  Prefer it when it is
    // available because OpenGL ES only works in floats.
    if (mFunctions->depthRangef)
    {
        mFunctions->depthRangef(mNear, mFar);
    }
    else
    {
        ASSERT(mFunctions->depthRange);
        mFunctions->depthRange(mNear, mFar);
    }

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_RANGE);
}

void StateManagerGL::setClipControl(gl::ClipOrigin origin, gl::ClipDepthMode depth)
{
    if (mClipOrigin == origin && mClipDepthMode == depth)
    {
        return;
    }

    mClipOrigin    = origin;
    mClipDepthMode = depth;

    ASSERT(mFunctions->clipControl);
    mFunctions->clipControl(ToGLenum(mClipOrigin), ToGLenum(mClipDepthMode));

    if (mFeatures.resyncDepthRangeOnClipControl.enabled)
    {
        // Change and restore depth range to trigger internal transformation
        // state resync. This is needed to apply clip control on some drivers.
        const float near = mNear;
        setDepthRange(near == 0.0f ? 1.0f : 0.0f, mFar);
        setDepthRange(near, mFar);
    }

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
    mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_CLIP_CONTROL);
}

void StateManagerGL::setClipControlWithEmulatedClipOrigin(const gl::ProgramExecutable *executable,
                                                          GLenum frontFace,
                                                          gl::ClipOrigin origin,
                                                          gl::ClipDepthMode depth)
{
    ASSERT(mFeatures.emulateClipOrigin.enabled);
    if (executable)
    {
        updateEmulatedClipOriginUniform(executable, origin);
    }
    static_assert((GL_CW ^ GL_CCW) == static_cast<GLenum>(gl::ClipOrigin::UpperLeft));
    setFrontFace(frontFace ^ static_cast<GLenum>(origin));
    setClipControl(gl::ClipOrigin::LowerLeft, depth);
}

void StateManagerGL::setBlendEnabled(bool enabled)
{
    const gl::DrawBufferMask mask =
        enabled ? mBlendStateExt.getAllEnabledMask() : gl::DrawBufferMask::Zero();
    if (mBlendStateExt.getEnabledMask() == mask)
    {
        return;
    }

    if (enabled)
    {
        mFunctions->enable(GL_BLEND);
    }
    else
    {
        mFunctions->disable(GL_BLEND);
    }

    mBlendStateExt.setEnabled(enabled);
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_ENABLED);
}

void StateManagerGL::setBlendEnabledIndexed(const gl::DrawBufferMask enabledMask)
{
    if (mBlendStateExt.getEnabledMask() == enabledMask)
    {
        return;
    }

    // Get DrawBufferMask of buffers with different blend enable state
    gl::DrawBufferMask diffMask = mBlendStateExt.getEnabledMask() ^ enabledMask;
    const size_t diffCount      = diffMask.count();

    // Check if enabling or disabling blending for all buffers reduces the number of subsequent
    // indexed commands. Implicitly handles the case when the new blend enable state is the same for
    // all buffers.
    if (diffCount > 1)
    {
        // The number of indexed blend enable commands in case a mass disable is used.
        const size_t enabledCount = enabledMask.count();

        // The mask and the number of indexed blend disable commands in case a mass enable is used.
        const gl::DrawBufferMask disabledMask = enabledMask ^ mBlendStateExt.getAllEnabledMask();
        const size_t disabledCount            = disabledMask.count();

        if (enabledCount < diffCount && enabledCount <= disabledCount)
        {
            diffMask = enabledMask;
            mFunctions->disable(GL_BLEND);
        }
        else if (disabledCount < diffCount && disabledCount <= enabledCount)
        {
            diffMask = disabledMask;
            mFunctions->enable(GL_BLEND);
        }
    }

    for (size_t drawBufferIndex : diffMask)
    {
        if (enabledMask.test(drawBufferIndex))
        {
            mFunctions->enablei(GL_BLEND, static_cast<GLuint>(drawBufferIndex));
        }
        else
        {
            mFunctions->disablei(GL_BLEND, static_cast<GLuint>(drawBufferIndex));
        }
    }

    mBlendStateExt.setEnabledMask(enabledMask);
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_ENABLED);
}

void StateManagerGL::setBlendColor(const gl::ColorF &blendColor)
{
    if (mBlendColor != blendColor)
    {
        mBlendColor = blendColor;
        mFunctions->blendColor(mBlendColor.red, mBlendColor.green, mBlendColor.blue,
                               mBlendColor.alpha);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_COLOR);
    }
}

void StateManagerGL::setBlendAdvancedCoherent(bool enabled)
{
    if (mBlendAdvancedCoherent != enabled)
    {
        mBlendAdvancedCoherent = enabled;

        if (mBlendAdvancedCoherent)
        {
            mFunctions->enable(GL_BLEND_ADVANCED_COHERENT_KHR);
        }
        else
        {
            mFunctions->disable(GL_BLEND_ADVANCED_COHERENT_KHR);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
        mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_BLEND_ADVANCED_COHERENT);
    }
}

void StateManagerGL::setBlendFuncs(const gl::BlendStateExt &blendStateExt)
{
    if (mBlendStateExt.getSrcColorBits() == blendStateExt.getSrcColorBits() &&
        mBlendStateExt.getDstColorBits() == blendStateExt.getDstColorBits() &&
        mBlendStateExt.getSrcAlphaBits() == blendStateExt.getSrcAlphaBits() &&
        mBlendStateExt.getDstAlphaBits() == blendStateExt.getDstAlphaBits())
    {
        return;
    }

    if (!mIndependentBlendStates)
    {
        mFunctions->blendFuncSeparate(ToGLenum(blendStateExt.getSrcColorIndexed(0)),
                                      ToGLenum(blendStateExt.getDstColorIndexed(0)),
                                      ToGLenum(blendStateExt.getSrcAlphaIndexed(0)),
                                      ToGLenum(blendStateExt.getDstAlphaIndexed(0)));
    }
    else
    {
        // Get DrawBufferMask of buffers with different blend factors
        gl::DrawBufferMask diffMask = mBlendStateExt.compareFactors(blendStateExt);
        size_t diffCount            = diffMask.count();

        // Check if setting all buffers to the same value reduces the number of subsequent indexed
        // commands. Implicitly handles the case when the new blend function state is the same for
        // all buffers.
        if (diffCount > 1)
        {
            bool found                                            = false;
            gl::BlendStateExt::FactorStorage::Type commonSrcColor = 0;
            gl::BlendStateExt::FactorStorage::Type commonDstColor = 0;
            gl::BlendStateExt::FactorStorage::Type commonSrcAlpha = 0;
            gl::BlendStateExt::FactorStorage::Type commonDstAlpha = 0;
            for (size_t i = 0; i < mBlendStateExt.getDrawBufferCount() - 1; i++)
            {
                const gl::BlendStateExt::FactorStorage::Type tempCommonSrcColor =
                    blendStateExt.expandSrcColorIndexed(i);
                const gl::BlendStateExt::FactorStorage::Type tempCommonDstColor =
                    blendStateExt.expandDstColorIndexed(i);
                const gl::BlendStateExt::FactorStorage::Type tempCommonSrcAlpha =
                    blendStateExt.expandSrcAlphaIndexed(i);
                const gl::BlendStateExt::FactorStorage::Type tempCommonDstAlpha =
                    blendStateExt.expandDstAlphaIndexed(i);

                const gl::DrawBufferMask tempDiffMask = blendStateExt.compareFactors(
                    tempCommonSrcColor, tempCommonDstColor, tempCommonSrcAlpha, tempCommonDstAlpha);

                const size_t tempDiffCount = tempDiffMask.count();
                if (tempDiffCount < diffCount)
                {
                    found          = true;
                    diffMask       = tempDiffMask;
                    diffCount      = tempDiffCount;
                    commonSrcColor = tempCommonSrcColor;
                    commonDstColor = tempCommonDstColor;
                    commonSrcAlpha = tempCommonSrcAlpha;
                    commonDstAlpha = tempCommonDstAlpha;
                    if (tempDiffCount == 0)
                    {
                        break;  // the blend factors are the same for all buffers
                    }
                }
            }
            if (found)
            {
                mFunctions->blendFuncSeparate(
                    ToGLenum(gl::BlendStateExt::FactorStorage::GetValueIndexed(0, commonSrcColor)),
                    ToGLenum(gl::BlendStateExt::FactorStorage::GetValueIndexed(0, commonDstColor)),
                    ToGLenum(gl::BlendStateExt::FactorStorage::GetValueIndexed(0, commonSrcAlpha)),
                    ToGLenum(gl::BlendStateExt::FactorStorage::GetValueIndexed(0, commonDstAlpha)));
            }
        }

        for (size_t drawBufferIndex : diffMask)
        {
            mFunctions->blendFuncSeparatei(
                static_cast<GLuint>(drawBufferIndex),
                ToGLenum(blendStateExt.getSrcColorIndexed(drawBufferIndex)),
                ToGLenum(blendStateExt.getDstColorIndexed(drawBufferIndex)),
                ToGLenum(blendStateExt.getSrcAlphaIndexed(drawBufferIndex)),
                ToGLenum(blendStateExt.getDstAlphaIndexed(drawBufferIndex)));
        }
    }
    mBlendStateExt.setSrcColorBits(blendStateExt.getSrcColorBits());
    mBlendStateExt.setDstColorBits(blendStateExt.getDstColorBits());
    mBlendStateExt.setSrcAlphaBits(blendStateExt.getSrcAlphaBits());
    mBlendStateExt.setDstAlphaBits(blendStateExt.getDstAlphaBits());
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_FUNCS);
}

void StateManagerGL::setBlendEquations(const gl::BlendStateExt &blendStateExt)
{
    if (mBlendStateExt.getEquationColorBits() == blendStateExt.getEquationColorBits() &&
        mBlendStateExt.getEquationAlphaBits() == blendStateExt.getEquationAlphaBits())
    {
        return;
    }

    if (!mIndependentBlendStates)
    {
        mFunctions->blendEquationSeparate(ToGLenum(blendStateExt.getEquationColorIndexed(0)),
                                          ToGLenum(blendStateExt.getEquationAlphaIndexed(0)));
    }
    else
    {
        // Get DrawBufferMask of buffers with different blend equations
        gl::DrawBufferMask diffMask = mBlendStateExt.compareEquations(blendStateExt);
        size_t diffCount            = diffMask.count();

        // Check if setting all buffers to the same value reduces the number of subsequent indexed
        // commands. Implicitly handles the case when the new blend equation state is the same for
        // all buffers.
        if (diffCount > 1)
        {
            bool found                                                   = false;
            gl::BlendStateExt::EquationStorage::Type commonEquationColor = 0;
            gl::BlendStateExt::EquationStorage::Type commonEquationAlpha = 0;
            for (size_t i = 0; i < mBlendStateExt.getDrawBufferCount() - 1; i++)
            {
                const gl::BlendStateExt::EquationStorage::Type tempCommonEquationColor =
                    blendStateExt.expandEquationColorIndexed(i);
                const gl::BlendStateExt::EquationStorage::Type tempCommonEquationAlpha =
                    blendStateExt.expandEquationAlphaIndexed(i);

                const gl::DrawBufferMask tempDiffMask = blendStateExt.compareEquations(
                    tempCommonEquationColor, tempCommonEquationAlpha);

                const size_t tempDiffCount = tempDiffMask.count();
                if (tempDiffCount < diffCount)
                {
                    found               = true;
                    diffMask            = tempDiffMask;
                    diffCount           = tempDiffCount;
                    commonEquationColor = tempCommonEquationColor;
                    commonEquationAlpha = tempCommonEquationAlpha;
                    if (tempDiffCount == 0)
                    {
                        break;  // the new blend equations are the same for all buffers
                    }
                }
            }
            if (found)
            {
                if (commonEquationColor == commonEquationAlpha)
                {
                    mFunctions->blendEquation(
                        ToGLenum(gl::BlendStateExt::EquationStorage::GetValueIndexed(
                            0, commonEquationColor)));
                }
                else
                {
                    mFunctions->blendEquationSeparate(
                        ToGLenum(gl::BlendStateExt::EquationStorage::GetValueIndexed(
                            0, commonEquationColor)),
                        ToGLenum(gl::BlendStateExt::EquationStorage::GetValueIndexed(
                            0, commonEquationAlpha)));
                }
            }
        }

        for (size_t drawBufferIndex : diffMask)
        {
            gl::BlendEquationType equationColor =
                blendStateExt.getEquationColorIndexed(drawBufferIndex);
            gl::BlendEquationType equationAlpha =
                blendStateExt.getEquationAlphaIndexed(drawBufferIndex);
            if (equationColor == equationAlpha)
            {
                mFunctions->blendEquationi(static_cast<GLuint>(drawBufferIndex),
                                           ToGLenum(equationColor));
            }
            else
            {
                mFunctions->blendEquationSeparatei(static_cast<GLuint>(drawBufferIndex),
                                                   ToGLenum(equationColor),
                                                   ToGLenum(equationAlpha));
            }
        }
    }
    mBlendStateExt.setEquationColorBits(blendStateExt.getEquationColorBits());
    mBlendStateExt.setEquationAlphaBits(blendStateExt.getEquationAlphaBits());
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_COLOR_MASK);
}

void StateManagerGL::setColorMask(bool red, bool green, bool blue, bool alpha)
{
    const gl::BlendStateExt::ColorMaskStorage::Type mask =
        mBlendStateExt.expandColorMaskValue(red, green, blue, alpha);
    if (mBlendStateExt.getColorMaskBits() != mask)
    {
        mFunctions->colorMask(red, green, blue, alpha);
        mBlendStateExt.setColorMaskBits(mask);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_COLOR_MASK);
    }
}

void StateManagerGL::setSampleAlphaToCoverageEnabled(bool enabled)
{
    if (mSampleAlphaToCoverageEnabled != enabled)
    {
        mSampleAlphaToCoverageEnabled = enabled;
        if (mSampleAlphaToCoverageEnabled)
        {
            mFunctions->enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED);
    }
}

void StateManagerGL::setSampleCoverageEnabled(bool enabled)
{
    if (mSampleCoverageEnabled != enabled)
    {
        mSampleCoverageEnabled = enabled;
        if (mSampleCoverageEnabled)
        {
            mFunctions->enable(GL_SAMPLE_COVERAGE);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_COVERAGE);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED);
    }
}

void StateManagerGL::setSampleCoverage(float value, bool invert)
{
    if (mSampleCoverageValue != value || mSampleCoverageInvert != invert)
    {
        mSampleCoverageValue  = value;
        mSampleCoverageInvert = invert;
        mFunctions->sampleCoverage(mSampleCoverageValue, mSampleCoverageInvert);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_COVERAGE);
    }
}

void StateManagerGL::setSampleMaskEnabled(bool enabled)
{
    if (mSampleMaskEnabled != enabled)
    {
        mSampleMaskEnabled = enabled;
        if (mSampleMaskEnabled)
        {
            mFunctions->enable(GL_SAMPLE_MASK);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_MASK);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_MASK_ENABLED);
    }
}

void StateManagerGL::setSampleMaski(GLuint maskNumber, GLbitfield mask)
{
    ASSERT(maskNumber < mSampleMaskValues.size());
    if (mSampleMaskValues[maskNumber] != mask)
    {
        mSampleMaskValues[maskNumber] = mask;
        mFunctions->sampleMaski(maskNumber, mask);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_MASK);
    }
}

// Depth and stencil redundant state changes are guarded in the
// frontend so for related cases here just set the dirty bit
// and update backend states.
void StateManagerGL::setDepthTestEnabled(bool enabled)
{
    mDepthTestEnabled = enabled;
    if (mDepthTestEnabled)
    {
        mFunctions->enable(GL_DEPTH_TEST);
    }
    else
    {
        mFunctions->disable(GL_DEPTH_TEST);
    }

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_TEST_ENABLED);
}

void StateManagerGL::setDepthFunc(GLenum depthFunc)
{
    mDepthFunc = depthFunc;
    mFunctions->depthFunc(mDepthFunc);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_FUNC);
}

void StateManagerGL::setDepthMask(bool mask)
{
    mDepthMask = mask;
    mFunctions->depthMask(mDepthMask);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_MASK);
}

void StateManagerGL::setStencilTestEnabled(bool enabled)
{
    mStencilTestEnabled = enabled;
    if (mStencilTestEnabled)
    {
        mFunctions->enable(GL_STENCIL_TEST);
    }
    else
    {
        mFunctions->disable(GL_STENCIL_TEST);
    }

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_TEST_ENABLED);
}

void StateManagerGL::setStencilFrontWritemask(GLuint mask)
{
    mStencilFrontWritemask = mask;
    mFunctions->stencilMaskSeparate(GL_FRONT, mStencilFrontWritemask);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
}

void StateManagerGL::setStencilBackWritemask(GLuint mask)
{
    mStencilBackWritemask = mask;
    mFunctions->stencilMaskSeparate(GL_BACK, mStencilBackWritemask);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_WRITEMASK_BACK);
}

void StateManagerGL::setStencilFrontFuncs(GLenum func, GLint ref, GLuint mask)
{
    mStencilFrontFunc      = func;
    mStencilFrontRef       = ref;
    mStencilFrontValueMask = mask;
    mFunctions->stencilFuncSeparate(GL_FRONT, mStencilFrontFunc, mStencilFrontRef,
                                    mStencilFrontValueMask);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_FUNCS_FRONT);
}

void StateManagerGL::setStencilBackFuncs(GLenum func, GLint ref, GLuint mask)
{
    mStencilBackFunc      = func;
    mStencilBackRef       = ref;
    mStencilBackValueMask = mask;
    mFunctions->stencilFuncSeparate(GL_BACK, mStencilBackFunc, mStencilBackRef,
                                    mStencilBackValueMask);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_FUNCS_BACK);
}

void StateManagerGL::setStencilFrontOps(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    mStencilFrontStencilFailOp          = sfail;
    mStencilFrontStencilPassDepthFailOp = dpfail;
    mStencilFrontStencilPassDepthPassOp = dppass;
    mFunctions->stencilOpSeparate(GL_FRONT, mStencilFrontStencilFailOp,
                                  mStencilFrontStencilPassDepthFailOp,
                                  mStencilFrontStencilPassDepthPassOp);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_OPS_FRONT);
}

void StateManagerGL::setStencilBackOps(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    mStencilBackStencilFailOp          = sfail;
    mStencilBackStencilPassDepthFailOp = dpfail;
    mStencilBackStencilPassDepthPassOp = dppass;
    mFunctions->stencilOpSeparate(GL_BACK, mStencilBackStencilFailOp,
                                  mStencilBackStencilPassDepthFailOp,
                                  mStencilBackStencilPassDepthPassOp);

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_OPS_BACK);
}

void StateManagerGL::setCullFaceEnabled(bool enabled)
{
    if (mCullFaceEnabled != enabled)
    {
        mCullFaceEnabled = enabled;
        if (mCullFaceEnabled)
        {
            mFunctions->enable(GL_CULL_FACE);
        }
        else
        {
            mFunctions->disable(GL_CULL_FACE);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CULL_FACE_ENABLED);
    }
}

void StateManagerGL::setCullFace(gl::CullFaceMode cullFace)
{
    if (mCullFace != cullFace)
    {
        mCullFace = cullFace;
        mFunctions->cullFace(ToGLenum(mCullFace));

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CULL_FACE);
    }
}

void StateManagerGL::setFrontFace(GLenum frontFace)
{
    if (mFrontFace != frontFace)
    {
        mFrontFace = frontFace;
        mFunctions->frontFace(mFrontFace);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_FRONT_FACE);
    }
}

void StateManagerGL::setPolygonMode(gl::PolygonMode mode)
{
    if (mPolygonMode != mode)
    {
        mPolygonMode = mode;
        if (mFunctions->standard == STANDARD_GL_DESKTOP)
        {
            mFunctions->polygonMode(GL_FRONT_AND_BACK, ToGLenum(mPolygonMode));
        }
        else
        {
            ASSERT(mFunctions->polygonModeNV);
            mFunctions->polygonModeNV(GL_FRONT_AND_BACK, ToGLenum(mPolygonMode));
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
        mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_POLYGON_MODE);
    }
}

void StateManagerGL::setPolygonOffsetPointEnabled(bool enabled)
{
    if (mPolygonOffsetPointEnabled != enabled)
    {
        mPolygonOffsetPointEnabled = enabled;
        if (mPolygonOffsetPointEnabled)
        {
            mFunctions->enable(GL_POLYGON_OFFSET_POINT_NV);
        }
        else
        {
            mFunctions->disable(GL_POLYGON_OFFSET_POINT_NV);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
        mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_POINT_ENABLED);
    }
}

void StateManagerGL::setPolygonOffsetLineEnabled(bool enabled)
{
    if (mPolygonOffsetLineEnabled != enabled)
    {
        mPolygonOffsetLineEnabled = enabled;
        if (mPolygonOffsetLineEnabled)
        {
            mFunctions->enable(GL_POLYGON_OFFSET_LINE_NV);
        }
        else
        {
            mFunctions->disable(GL_POLYGON_OFFSET_LINE_NV);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
        mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED);
    }
}

void StateManagerGL::setPolygonOffsetFillEnabled(bool enabled)
{
    if (mPolygonOffsetFillEnabled != enabled)
    {
        mPolygonOffsetFillEnabled = enabled;
        if (mPolygonOffsetFillEnabled)
        {
            mFunctions->enable(GL_POLYGON_OFFSET_FILL);
        }
        else
        {
            mFunctions->disable(GL_POLYGON_OFFSET_FILL);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED);
    }
}

void StateManagerGL::setPolygonOffset(float factor, float units, float clamp)
{
    if (mPolygonOffsetFactor != factor || mPolygonOffsetUnits != units ||
        mPolygonOffsetClamp != clamp)
    {
        mPolygonOffsetFactor = factor;
        mPolygonOffsetUnits  = units;
        mPolygonOffsetClamp  = clamp;

        if (clamp == 0.0f)
        {
            mFunctions->polygonOffset(mPolygonOffsetFactor, mPolygonOffsetUnits);
        }
        else
        {
            ASSERT(mFunctions->polygonOffsetClampEXT);
            mFunctions->polygonOffsetClampEXT(mPolygonOffsetFactor, mPolygonOffsetUnits,
                                              mPolygonOffsetClamp);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_POLYGON_OFFSET);
    }
}

void StateManagerGL::setDepthClampEnabled(bool enabled)
{
    if (mDepthClampEnabled != enabled)
    {
        mDepthClampEnabled = enabled;
        if (mDepthClampEnabled)
        {
            mFunctions->enable(GL_DEPTH_CLAMP_EXT);
        }
        else
        {
            mFunctions->disable(GL_DEPTH_CLAMP_EXT);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
        mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_DEPTH_CLAMP_ENABLED);
    }
}

void StateManagerGL::setRasterizerDiscardEnabled(bool enabled)
{
    if (mRasterizerDiscardEnabled != enabled)
    {
        mRasterizerDiscardEnabled = enabled;
        if (mRasterizerDiscardEnabled)
        {
            mFunctions->enable(GL_RASTERIZER_DISCARD);
        }
        else
        {
            mFunctions->disable(GL_RASTERIZER_DISCARD);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED);
    }
}

void StateManagerGL::setLineWidth(float width)
{
    if (mLineWidth != width)
    {
        mLineWidth = width;
        mFunctions->lineWidth(mLineWidth);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_LINE_WIDTH);
    }
}

angle::Result StateManagerGL::setPrimitiveRestartEnabled(const gl::Context *context, bool enabled)
{
    if (mPrimitiveRestartEnabled != enabled)
    {
        GLenum cap = mFeatures.emulatePrimitiveRestartFixedIndex.enabled
                         ? GL_PRIMITIVE_RESTART
                         : GL_PRIMITIVE_RESTART_FIXED_INDEX;

        if (enabled)
        {
            ANGLE_GL_TRY(context, mFunctions->enable(cap));
        }
        else
        {
            ANGLE_GL_TRY(context, mFunctions->disable(cap));
        }
        mPrimitiveRestartEnabled = enabled;

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED);
    }

    return angle::Result::Continue;
}

angle::Result StateManagerGL::setPrimitiveRestartIndex(const gl::Context *context, GLuint index)
{
    if (mPrimitiveRestartIndex != index)
    {
        ANGLE_GL_TRY(context, mFunctions->primitiveRestartIndex(index));
        mPrimitiveRestartIndex = index;

        // No dirty bit for this state, it is not exposed to the frontend.
    }

    return angle::Result::Continue;
}

void StateManagerGL::setClearDepth(float clearDepth)
{
    if (mClearDepth != clearDepth)
    {
        mClearDepth = clearDepth;

        // The glClearDepthf function isn't available until OpenGL 4.1.  Prefer it when it is
        // available because OpenGL ES only works in floats.
        if (mFunctions->clearDepthf)
        {
            mFunctions->clearDepthf(mClearDepth);
        }
        else
        {
            ASSERT(mFunctions->clearDepth);
            mFunctions->clearDepth(mClearDepth);
        }

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CLEAR_DEPTH);
    }
}

void StateManagerGL::setClearColor(const gl::ColorF &clearColor)
{
    gl::ColorF modifiedClearColor = clearColor;
    if (mFeatures.clearToZeroOrOneBroken.enabled &&
        (clearColor.red == 1.0f || clearColor.red == 0.0f) &&
        (clearColor.green == 1.0f || clearColor.green == 0.0f) &&
        (clearColor.blue == 1.0f || clearColor.blue == 0.0f) &&
        (clearColor.alpha == 1.0f || clearColor.alpha == 0.0f))
    {
        if (clearColor.alpha == 1.0f)
        {
            modifiedClearColor.alpha = 2.0f;
        }
        else
        {
            modifiedClearColor.alpha = -1.0f;
        }
    }

    if (mClearColor != modifiedClearColor)
    {
        mClearColor = modifiedClearColor;
        mFunctions->clearColor(mClearColor.red, mClearColor.green, mClearColor.blue,
                               mClearColor.alpha);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CLEAR_COLOR);
    }
}

void StateManagerGL::setClearStencil(GLint clearStencil)
{
    if (mClearStencil != clearStencil)
    {
        mClearStencil = clearStencil;
        mFunctions->clearStencil(mClearStencil);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CLEAR_STENCIL);
    }
}

angle::Result StateManagerGL::syncState(const gl::Context *context,
                                        const gl::state::DirtyBits &glDirtyBits,
                                        const gl::state::DirtyBits &bitMask,
                                        const gl::state::ExtendedDirtyBits &extendedDirtyBits,
                                        const gl::state::ExtendedDirtyBits &extendedBitMask)
{
    const gl::State &state = context->getState();

    const gl::state::DirtyBits glAndLocalDirtyBits = (glDirtyBits | mLocalDirtyBits) & bitMask;
    if (!glAndLocalDirtyBits.any())
    {
        return angle::Result::Continue;
    }

    // TODO(jmadill): Investigate only syncing vertex state for active attributes
    for (auto iter = glAndLocalDirtyBits.begin(), endIter = glAndLocalDirtyBits.end();
         iter != endIter; ++iter)
    {
        switch (*iter)
        {
            case gl::state::DIRTY_BIT_SCISSOR_TEST_ENABLED:
                setScissorTestEnabled(state.isScissorTestEnabled());
                break;
            case gl::state::DIRTY_BIT_SCISSOR:
            {
                const gl::Rectangle &scissor = state.getScissor();
                setScissor(scissor);
            }
            break;
            case gl::state::DIRTY_BIT_VIEWPORT:
            {
                const gl::Rectangle &viewport = state.getViewport();
                setViewport(viewport);
            }
            break;
            case gl::state::DIRTY_BIT_DEPTH_RANGE:
                setDepthRange(state.getNearPlane(), state.getFarPlane());
                break;
            case gl::state::DIRTY_BIT_BLEND_ENABLED:
                if (mIndependentBlendStates)
                {
                    setBlendEnabledIndexed(state.getBlendEnabledDrawBufferMask());
                }
                else
                {
                    setBlendEnabled(state.isBlendEnabled());
                }
                break;
            case gl::state::DIRTY_BIT_BLEND_COLOR:
                setBlendColor(state.getBlendColor());
                break;
            case gl::state::DIRTY_BIT_BLEND_FUNCS:
            {
                setBlendFuncs(state.getBlendStateExt());
                break;
            }
            case gl::state::DIRTY_BIT_BLEND_EQUATIONS:
            {
                setBlendEquations(state.getBlendStateExt());
                break;
            }
            case gl::state::DIRTY_BIT_COLOR_MASK:
            {
                const gl::Framebuffer *framebuffer = state.getDrawFramebuffer();
                const FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
                const bool disableAlphaWrite =
                    framebufferGL->hasEmulatedAlphaChannelTextureAttachment();

                setColorMaskForFramebuffer(state.getBlendStateExt(), disableAlphaWrite);
                break;
            }
            case gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
                setSampleAlphaToCoverageEnabled(state.isSampleAlphaToCoverageEnabled());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED:
                setSampleCoverageEnabled(state.isSampleCoverageEnabled());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_COVERAGE:
                setSampleCoverage(state.getSampleCoverageValue(), state.getSampleCoverageInvert());
                break;
            case gl::state::DIRTY_BIT_DEPTH_TEST_ENABLED:
                setDepthTestEnabled(state.isDepthTestEnabled());
                break;
            case gl::state::DIRTY_BIT_DEPTH_FUNC:
                setDepthFunc(state.getDepthStencilState().depthFunc);
                break;
            case gl::state::DIRTY_BIT_DEPTH_MASK:
                setDepthMask(state.getDepthStencilState().depthMask);
                break;
            case gl::state::DIRTY_BIT_STENCIL_TEST_ENABLED:
                setStencilTestEnabled(state.isStencilTestEnabled());
                break;
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_FRONT:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilFrontFuncs(depthStencilState.stencilFunc, state.getStencilRef(),
                                     depthStencilState.stencilMask);
                break;
            }
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_BACK:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilBackFuncs(depthStencilState.stencilBackFunc, state.getStencilBackRef(),
                                    depthStencilState.stencilBackMask);
                break;
            }
            case gl::state::DIRTY_BIT_STENCIL_OPS_FRONT:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilFrontOps(depthStencilState.stencilFail,
                                   depthStencilState.stencilPassDepthFail,
                                   depthStencilState.stencilPassDepthPass);
                break;
            }
            case gl::state::DIRTY_BIT_STENCIL_OPS_BACK:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilBackOps(depthStencilState.stencilBackFail,
                                  depthStencilState.stencilBackPassDepthFail,
                                  depthStencilState.stencilBackPassDepthPass);
                break;
            }
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                setStencilFrontWritemask(state.getDepthStencilState().stencilWritemask);
                break;
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                setStencilBackWritemask(state.getDepthStencilState().stencilBackWritemask);
                break;
            case gl::state::DIRTY_BIT_CULL_FACE_ENABLED:
                setCullFaceEnabled(state.isCullFaceEnabled());
                break;
            case gl::state::DIRTY_BIT_CULL_FACE:
                setCullFace(state.getRasterizerState().cullMode);
                break;
            case gl::state::DIRTY_BIT_FRONT_FACE:
                if (mFeatures.emulateClipOrigin.enabled)
                {
                    static_assert((GL_CW ^ GL_CCW) ==
                                  static_cast<GLenum>(gl::ClipOrigin::UpperLeft));
                    setFrontFace(state.getRasterizerState().frontFace ^
                                 static_cast<GLenum>(state.getClipOrigin()));
                    break;
                }
                setFrontFace(state.getRasterizerState().frontFace);
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED:
                setPolygonOffsetFillEnabled(state.isPolygonOffsetFillEnabled());
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET:
            {
                const auto &rasterizerState = state.getRasterizerState();
                setPolygonOffset(rasterizerState.polygonOffsetFactor,
                                 rasterizerState.polygonOffsetUnits,
                                 rasterizerState.polygonOffsetClamp);
                break;
            }
            case gl::state::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED:
                setRasterizerDiscardEnabled(state.isRasterizerDiscardEnabled());
                break;
            case gl::state::DIRTY_BIT_LINE_WIDTH:
                setLineWidth(state.getLineWidth());
                break;
            case gl::state::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED:
                ANGLE_TRY(setPrimitiveRestartEnabled(context, state.isPrimitiveRestartEnabled()));
                break;
            case gl::state::DIRTY_BIT_CLEAR_COLOR:
                setClearColor(state.getColorClearValue());
                break;
            case gl::state::DIRTY_BIT_CLEAR_DEPTH:
                setClearDepth(state.getDepthClearValue());
                break;
            case gl::state::DIRTY_BIT_CLEAR_STENCIL:
                setClearStencil(state.getStencilClearValue());
                break;
            case gl::state::DIRTY_BIT_UNPACK_STATE:
                ANGLE_TRY(setPixelUnpackState(context, state.getUnpackState()));
                break;
            case gl::state::DIRTY_BIT_UNPACK_BUFFER_BINDING:
                ANGLE_TRY(setPixelUnpackBuffer(
                    context, state.getTargetBuffer(gl::BufferBinding::PixelUnpack)));
                break;
            case gl::state::DIRTY_BIT_PACK_STATE:
                ANGLE_TRY(setPixelPackState(context, state.getPackState()));
                break;
            case gl::state::DIRTY_BIT_PACK_BUFFER_BINDING:
                ANGLE_TRY(setPixelPackBuffer(context,
                                             state.getTargetBuffer(gl::BufferBinding::PixelPack)));
                break;
            case gl::state::DIRTY_BIT_DITHER_ENABLED:
                setDitherEnabled(state.isDitherEnabled());
                break;
            case gl::state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING:
            {
                gl::Framebuffer *framebuffer = state.getReadFramebuffer();

                // Necessary for an Intel TexImage workaround.
                if (!framebuffer)
                    continue;

                FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
                bindFramebuffer(
                    mHasSeparateFramebufferBindings ? GL_READ_FRAMEBUFFER : GL_FRAMEBUFFER,
                    framebufferGL->getFramebufferID());
                break;
            }
            case gl::state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING:
            {
                gl::Framebuffer *framebuffer = state.getDrawFramebuffer();

                // Necessary for an Intel TexImage workaround.
                if (!framebuffer)
                    continue;

                FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
                bindFramebuffer(
                    mHasSeparateFramebufferBindings ? GL_DRAW_FRAMEBUFFER : GL_FRAMEBUFFER,
                    framebufferGL->getFramebufferID());

                const gl::ProgramExecutable *executable = state.getProgramExecutable();
                if (executable)
                {
                    updateMultiviewBaseViewLayerIndexUniform(executable, framebufferGL->getState());
                }

                // Changing the draw framebuffer binding sometimes requires resetting srgb blending.
                iter.setLaterBit(gl::state::DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE);

                // If the framebuffer is emulating RGB on top of RGBA, the color mask has to be
                // updated
                iter.setLaterBit(gl::state::DIRTY_BIT_COLOR_MASK);
                break;
            }
            case gl::state::DIRTY_BIT_RENDERBUFFER_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING:
            {
                VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(state.getVertexArray());
                bindVertexArray(vaoGL->getVertexArrayID(), vaoGL->getNativeState());

                ANGLE_TRY(propagateProgramToVAO(context, state.getProgramExecutable(),
                                                GetImplAs<VertexArrayGL>(state.getVertexArray())));

                if (vaoGL->syncsToSharedState())
                {
                    // Re-sync the vertex array because all frontend VAOs share the same backend
                    // state. Only sync bits that can be set in ES2.0 or 3.0
                    gl::VertexArray::DirtyBits dirtyBits;
                    gl::VertexArray::DirtyAttribBitsArray dirtyAttribBits;
                    gl::VertexArray::DirtyBindingBitsArray dirtBindingBits;

                    dirtyBits.set(gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER);
                    for (GLint attrib = 0; attrib < context->getCaps().maxVertexAttributes;
                         attrib++)
                    {
                        dirtyBits.set(gl::VertexArray::DIRTY_BIT_ATTRIB_0 + attrib);
                        dirtyAttribBits[attrib].set(gl::VertexArray::DIRTY_ATTRIB_ENABLED);
                        dirtyAttribBits[attrib].set(gl::VertexArray::DIRTY_ATTRIB_POINTER);
                        dirtyAttribBits[attrib].set(gl::VertexArray::DIRTY_ATTRIB_POINTER_BUFFER);
                    }
                    for (GLint binding = 0; binding < context->getCaps().maxVertexAttribBindings;
                         binding++)
                    {
                        dirtyBits.set(gl::VertexArray::DIRTY_BIT_BINDING_0 + binding);
                        dirtBindingBits[binding].set(gl::VertexArray::DIRTY_BINDING_DIVISOR);
                    }

                    ANGLE_TRY(
                        vaoGL->syncState(context, dirtyBits, &dirtyAttribBits, &dirtBindingBits));
                }
                break;
            }
            case gl::state::DIRTY_BIT_DRAW_INDIRECT_BUFFER_BINDING:
                updateDrawIndirectBufferBinding(context);
                break;
            case gl::state::DIRTY_BIT_DISPATCH_INDIRECT_BUFFER_BINDING:
                updateDispatchIndirectBufferBinding(context);
                break;
            case gl::state::DIRTY_BIT_PROGRAM_BINDING:
            {
                gl::Program *program = state.getProgram();
                if (program != nullptr)
                {
                    useProgram(GetImplAs<ProgramGL>(program)->getProgramID());
                }
                break;
            }
            case gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE:
            {
                const gl::ProgramExecutable *executable = state.getProgramExecutable();

                if (executable)
                {
                    iter.setLaterBit(gl::state::DIRTY_BIT_TEXTURE_BINDINGS);

                    if (executable->getActiveImagesMask().any())
                    {
                        iter.setLaterBit(gl::state::DIRTY_BIT_IMAGE_BINDINGS);
                    }

                    if (executable->getShaderStorageBlocks().size() > 0)
                    {
                        iter.setLaterBit(gl::state::DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING);
                    }

                    if (executable->getUniformBlocks().size() > 0)
                    {
                        iter.setLaterBit(gl::state::DIRTY_BIT_UNIFORM_BUFFER_BINDINGS);
                    }

                    if (executable->getAtomicCounterBuffers().size() > 0)
                    {
                        iter.setLaterBit(gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING);
                    }

                    if (mIsMultiviewEnabled && executable->usesMultiview())
                    {
                        updateMultiviewBaseViewLayerIndexUniform(
                            executable,
                            state.getDrawFramebuffer()->getImplementation()->getState());
                    }

                    // If the current executable does not use clip distances, the related API
                    // state has to be disabled to avoid runtime failures on certain drivers.
                    // On other drivers, that state is always emulated via a special uniform,
                    // which needs to be updated when switching programs.
                    if (mMaxClipDistances > 0)
                    {
                        iter.setLaterBit(gl::state::DIRTY_BIT_EXTENDED);
                        mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_CLIP_DISTANCES);
                    }

                    if (mFeatures.emulateClipOrigin.enabled)
                    {
                        updateEmulatedClipOriginUniform(executable, state.getClipOrigin());
                    }
                }

                if (!executable || !executable->hasLinkedShaderStage(gl::ShaderType::Compute))
                {
                    ANGLE_TRY(propagateProgramToVAO(
                        context, executable, GetImplAs<VertexArrayGL>(state.getVertexArray())));
                }
                break;
            }
            case gl::state::DIRTY_BIT_TEXTURE_BINDINGS:
                updateProgramTextureBindings(context);
                break;
            case gl::state::DIRTY_BIT_SAMPLER_BINDINGS:
                syncSamplersState(context);
                break;
            case gl::state::DIRTY_BIT_IMAGE_BINDINGS:
                updateProgramImageBindings(context);
                break;
            case gl::state::DIRTY_BIT_TRANSFORM_FEEDBACK_BINDING:
                syncTransformFeedbackState(context);
                break;
            case gl::state::DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING:
                updateProgramStorageBufferBindings(context);
                break;
            case gl::state::DIRTY_BIT_UNIFORM_BUFFER_BINDINGS:
                updateProgramUniformBufferBindings(context);
                break;
            case gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING:
                updateProgramAtomicCounterBufferBindings(context);
                break;
            case gl::state::DIRTY_BIT_MULTISAMPLING:
                setMultisamplingStateEnabled(state.isMultisamplingEnabled());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_ONE:
                setSampleAlphaToOneStateEnabled(state.isSampleAlphaToOneEnabled());
                break;
            case gl::state::DIRTY_BIT_COVERAGE_MODULATION:
                setCoverageModulation(state.getCoverageModulation());
                break;
            case gl::state::DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE:
                setFramebufferSRGBEnabledForFramebuffer(
                    context, state.getFramebufferSRGB(),
                    GetImplAs<FramebufferGL>(state.getDrawFramebuffer()));
                break;
            case gl::state::DIRTY_BIT_SAMPLE_MASK_ENABLED:
                setSampleMaskEnabled(state.isSampleMaskEnabled());
                break;
            case gl::state::DIRTY_BIT_SAMPLE_MASK:
            {
                for (GLuint maskNumber = 0; maskNumber < state.getMaxSampleMaskWords();
                     ++maskNumber)
                {
                    setSampleMaski(maskNumber, state.getSampleMaskWord(maskNumber));
                }
                break;
            }
            case gl::state::DIRTY_BIT_CURRENT_VALUES:
            {
                gl::AttributesMask combinedMask =
                    (state.getAndResetDirtyCurrentValues() | mLocalDirtyCurrentValues);

                for (auto attribIndex : combinedMask)
                {
                    setAttributeCurrentData(attribIndex,
                                            state.getVertexAttribCurrentValue(attribIndex));
                }

                mLocalDirtyCurrentValues.reset();
                break;
            }
            case gl::state::DIRTY_BIT_PROVOKING_VERTEX:
                setProvokingVertex(ToGLenum(state.getProvokingVertex()));
                break;
            case gl::state::DIRTY_BIT_EXTENDED:
            {
                const gl::state::ExtendedDirtyBits glAndLocalExtendedDirtyBits =
                    (extendedDirtyBits | mLocalExtendedDirtyBits) & extendedBitMask;
                for (size_t extendedDirtyBit : glAndLocalExtendedDirtyBits)
                {
                    switch (extendedDirtyBit)
                    {
                        case gl::state::EXTENDED_DIRTY_BIT_CLIP_CONTROL:
                            if (mFeatures.emulateClipOrigin.enabled)
                            {
                                setClipControlWithEmulatedClipOrigin(
                                    state.getProgramExecutable(),
                                    state.getRasterizerState().frontFace, state.getClipOrigin(),
                                    state.getClipDepthMode());
                                break;
                            }
                            setClipControl(state.getClipOrigin(), state.getClipDepthMode());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_CLIP_DISTANCES:
                        {
                            const gl::ProgramExecutable *executable = state.getProgramExecutable();
                            if (executable && executable->hasClipDistance())
                            {
                                setClipDistancesEnable(state.getEnabledClipDistances());
                                if (mFeatures.emulateClipDistanceState.enabled)
                                {
                                    updateEmulatedClipDistanceState(
                                        executable, state.getEnabledClipDistances());
                                }
                            }
                            else
                            {
                                setClipDistancesEnable(gl::ClipDistanceEnableBits());
                            }
                            break;
                        }
                        case gl::state::EXTENDED_DIRTY_BIT_DEPTH_CLAMP_ENABLED:
                            setDepthClampEnabled(state.isDepthClampEnabled());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_LOGIC_OP_ENABLED:
                            setLogicOpEnabled(state.isLogicOpEnabled());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_LOGIC_OP:
                            setLogicOp(state.getLogicOp());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_MIPMAP_GENERATION_HINT:
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_MODE:
                            setPolygonMode(state.getPolygonMode());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_POINT_ENABLED:
                            setPolygonOffsetPointEnabled(state.isPolygonOffsetPointEnabled());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED:
                            setPolygonOffsetLineEnabled(state.isPolygonOffsetLineEnabled());
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_SHADER_DERIVATIVE_HINT:
                            // These hints aren't forwarded to GL yet.
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_SHADING_RATE:
                            // Unimplemented extensions.
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_BLEND_ADVANCED_COHERENT:
                            setBlendAdvancedCoherent(state.isBlendAdvancedCoherentEnabled());
                            break;
                        default:
                            UNREACHABLE();
                            break;
                    }
                    mLocalExtendedDirtyBits &= ~extendedBitMask;
                }
                break;
            }
            case gl::state::DIRTY_BIT_SAMPLE_SHADING:
                // Nothing to do until OES_sample_shading is implemented.
                break;
            case gl::state::DIRTY_BIT_PATCH_VERTICES:
                // Nothing to do until EXT_tessellation_shader is implemented.
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    mLocalDirtyBits &= ~bitMask;

    return angle::Result::Continue;
}

void StateManagerGL::setFramebufferSRGBEnabled(const gl::Context *context, bool enabled)
{
    if (!mFramebufferSRGBAvailable)
    {
        return;
    }

    if (mFramebufferSRGBEnabled != enabled)
    {
        mFramebufferSRGBEnabled = enabled;
        if (mFramebufferSRGBEnabled)
        {
            mFunctions->enable(GL_FRAMEBUFFER_SRGB);
        }
        else
        {
            mFunctions->disable(GL_FRAMEBUFFER_SRGB);
        }
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE);
    }
}

void StateManagerGL::setFramebufferSRGBEnabledForFramebuffer(const gl::Context *context,
                                                             bool enabled,
                                                             const FramebufferGL *framebuffer)
{
    if (framebuffer->isDefault())
    {
        // Obey the framebuffer sRGB state for blending on all framebuffers except the default
        // framebuffer.
        // When SRGB blending is enabled, only SRGB capable formats will use it but the default
        // framebuffer will always use it if it is enabled.
        // TODO(geofflang): Update this when the framebuffer binding dirty changes, when it exists.
        setFramebufferSRGBEnabled(context, false);
    }
    else
    {
        setFramebufferSRGBEnabled(context, enabled);
    }
}

void StateManagerGL::setColorMaskForFramebuffer(const gl::BlendStateExt &blendStateExt,
                                                const bool disableAlpha)
{
    bool r, g, b, a;

    // Given that disableAlpha can be true only on macOS backbuffers and color mask is re-synced on
    // bound draw framebuffer change, switch all draw buffers color masks to avoid special case
    // later.
    if (!mIndependentBlendStates || disableAlpha)
    {
        blendStateExt.getColorMaskIndexed(0, &r, &g, &b, &a);
        setColorMask(r, g, b, disableAlpha ? false : a);
        return;
    }

    // Check if the current mask already matches the new state
    if (mBlendStateExt.getColorMaskBits() == blendStateExt.getColorMaskBits())
    {
        return;
    }

    // Get DrawBufferMask of buffers with different color masks
    gl::DrawBufferMask diffMask = mBlendStateExt.compareColorMask(blendStateExt.getColorMaskBits());
    size_t diffCount            = diffMask.count();

    // Check if setting all buffers to the same value reduces the number of subsequent indexed
    // commands. Implicitly handles the case when the new mask is the same for all buffers.
    // For instance, let's say that previously synced mask is ccccff00 and the new state is
    // ffeeeeee. Instead of calling colorMaski 8 times, ANGLE can set all buffers to `e` and then
    // use colorMaski only twice. On the other hand, if the new state is cceeee00, a non-indexed
    // call will increase the total number of GL commands.
    if (diffCount > 1)
    {
        bool found                                                = false;
        gl::BlendStateExt::ColorMaskStorage::Type commonColorMask = 0;
        for (size_t i = 0; i < mBlendStateExt.getDrawBufferCount() - 1; i++)
        {
            const gl::BlendStateExt::ColorMaskStorage::Type tempCommonColorMask =
                blendStateExt.expandColorMaskIndexed(i);
            const gl::DrawBufferMask tempDiffMask =
                blendStateExt.compareColorMask(tempCommonColorMask);
            const size_t tempDiffCount = tempDiffMask.count();
            if (tempDiffCount < diffCount)
            {
                found           = true;
                diffMask        = tempDiffMask;
                diffCount       = tempDiffCount;
                commonColorMask = tempCommonColorMask;
                if (tempDiffCount == 0)
                {
                    break;  // the new mask is the same for all buffers
                }
            }
        }
        if (found)
        {
            gl::BlendStateExt::UnpackColorMask(commonColorMask, &r, &g, &b, &a);
            mFunctions->colorMask(r, g, b, a);
        }
    }

    for (size_t drawBufferIndex : diffMask)
    {
        blendStateExt.getColorMaskIndexed(drawBufferIndex, &r, &g, &b, &a);
        mFunctions->colorMaski(static_cast<GLuint>(drawBufferIndex), r, g, b, a);
    }

    mBlendStateExt.setColorMaskBits(blendStateExt.getColorMaskBits());
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_COLOR_MASK);
}

void StateManagerGL::setDitherEnabled(bool enabled)
{
    if (mDitherEnabled != enabled)
    {
        mDitherEnabled = enabled;
        if (mDitherEnabled)
        {
            mFunctions->enable(GL_DITHER);
        }
        else
        {
            mFunctions->disable(GL_DITHER);
        }
    }
}

void StateManagerGL::setMultisamplingStateEnabled(bool enabled)
{
    if (mMultisamplingEnabled != enabled)
    {
        mMultisamplingEnabled = enabled;
        if (mMultisamplingEnabled)
        {
            mFunctions->enable(GL_MULTISAMPLE_EXT);
        }
        else
        {
            mFunctions->disable(GL_MULTISAMPLE_EXT);
        }
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_MULTISAMPLING);
    }
}

void StateManagerGL::setSampleAlphaToOneStateEnabled(bool enabled)
{
    if (mSampleAlphaToOneEnabled != enabled)
    {
        mSampleAlphaToOneEnabled = enabled;
        if (mSampleAlphaToOneEnabled)
        {
            mFunctions->enable(GL_SAMPLE_ALPHA_TO_ONE);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_ALPHA_TO_ONE);
        }
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_ONE);
    }
}

void StateManagerGL::setCoverageModulation(GLenum components)
{
    if (mCoverageModulation != components)
    {
        mCoverageModulation = components;
        mFunctions->coverageModulationNV(components);

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_COVERAGE_MODULATION);
    }
}

void StateManagerGL::setProvokingVertex(GLenum mode)
{
    if (mode != mProvokingVertex)
    {
        mFunctions->provokingVertex(mode);
        mProvokingVertex = mode;

        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PROVOKING_VERTEX);
    }
}

void StateManagerGL::setClipDistancesEnable(const gl::ClipDistanceEnableBits &enables)
{
    if (enables == mEnabledClipDistances)
    {
        return;
    }
    ASSERT(mMaxClipDistances <= gl::IMPLEMENTATION_MAX_CLIP_DISTANCES);

    gl::ClipDistanceEnableBits diff = enables ^ mEnabledClipDistances;
    for (size_t i : diff)
    {
        if (enables.test(i))
        {
            mFunctions->enable(GL_CLIP_DISTANCE0_EXT + static_cast<uint32_t>(i));
        }
        else
        {
            mFunctions->disable(GL_CLIP_DISTANCE0_EXT + static_cast<uint32_t>(i));
        }
    }

    mEnabledClipDistances = enables;
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
    mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_CLIP_DISTANCES);
}

void StateManagerGL::setLogicOpEnabled(bool enabled)
{
    if (enabled == mLogicOpEnabled)
    {
        return;
    }
    mLogicOpEnabled = enabled;

    if (enabled)
    {
        mFunctions->enable(GL_COLOR_LOGIC_OP);
    }
    else
    {
        mFunctions->disable(GL_COLOR_LOGIC_OP);
    }

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
    mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_LOGIC_OP_ENABLED);
}

void StateManagerGL::setLogicOp(gl::LogicalOperation opcode)
{
    if (opcode == mLogicOp)
    {
        return;
    }
    mLogicOp = opcode;

    mFunctions->logicOp(ToGLenum(opcode));

    mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
    mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_LOGIC_OP_ENABLED);
}

void StateManagerGL::setTextureCubemapSeamlessEnabled(bool enabled)
{
    // TODO(jmadill): Also check for seamless extension.
    if (!mFunctions->isAtLeastGL(gl::Version(3, 2)))
    {
        return;
    }

    if (mTextureCubemapSeamlessEnabled != enabled)
    {
        mTextureCubemapSeamlessEnabled = enabled;
        if (mTextureCubemapSeamlessEnabled)
        {
            mFunctions->enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
        else
        {
            mFunctions->disable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    }
}

angle::Result StateManagerGL::propagateProgramToVAO(const gl::Context *context,
                                                    const gl::ProgramExecutable *executable,
                                                    VertexArrayGL *vao)
{
    if (vao == nullptr)
    {
        return angle::Result::Continue;
    }

    // Number of views:
    if (mIsMultiviewEnabled)
    {
        int numViews = 1;
        if (executable && executable->usesMultiview())
        {
            numViews = executable->getNumViews();
        }
        ANGLE_TRY(vao->applyNumViewsToDivisor(context, numViews));
    }

    // Attribute enabled mask:
    if (executable)
    {
        ANGLE_TRY(vao->applyActiveAttribLocationsMask(context,
                                                      executable->getActiveAttribLocationsMask()));
    }

    return angle::Result::Continue;
}

void StateManagerGL::updateMultiviewBaseViewLayerIndexUniformImpl(
    const gl::ProgramExecutable *executable,
    const gl::FramebufferState &drawFramebufferState) const
{
    ASSERT(mIsMultiviewEnabled && executable && executable->usesMultiview());
    const ProgramExecutableGL *executableGL = GetImplAs<ProgramExecutableGL>(executable);
    if (drawFramebufferState.isMultiview())
    {
        executableGL->enableLayeredRenderingPath(drawFramebufferState.getBaseViewIndex());
    }
}

void StateManagerGL::updateEmulatedClipDistanceState(const gl::ProgramExecutable *executable,
                                                     const gl::ClipDistanceEnableBits enables) const
{
    ASSERT(mFeatures.emulateClipDistanceState.enabled);
    ASSERT(executable && executable->hasClipDistance());
    const ProgramExecutableGL *executableGL = GetImplAs<ProgramExecutableGL>(executable);
    executableGL->updateEnabledClipDistances(static_cast<uint8_t>(enables.bits()));
}

void StateManagerGL::syncSamplersState(const gl::Context *context)
{
    const gl::SamplerBindingVector &samplers = context->getState().getSamplers();

    // This could be optimized by using a separate binding dirty bit per sampler.
    for (size_t samplerIndex = 0; samplerIndex < samplers.size(); ++samplerIndex)
    {
        const gl::Sampler *sampler = samplers[samplerIndex].get();
        if (sampler != nullptr)
        {
            SamplerGL *samplerGL = GetImplAs<SamplerGL>(sampler);
            bindSampler(samplerIndex, samplerGL->getSamplerID());
        }
        else
        {
            bindSampler(samplerIndex, 0);
        }
    }
}

void StateManagerGL::syncTransformFeedbackState(const gl::Context *context)
{
    // Set the current transform feedback state
    gl::TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
    if (transformFeedback)
    {
        TransformFeedbackGL *transformFeedbackGL =
            GetImplAs<TransformFeedbackGL>(transformFeedback);
        bindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbackGL->getTransformFeedbackID());
        transformFeedbackGL->syncActiveState(context, transformFeedback->isActive(),
                                             transformFeedback->getPrimitiveMode());
        transformFeedbackGL->syncPausedState(transformFeedback->isPaused());
        mCurrentTransformFeedback = transformFeedbackGL;
    }
    else
    {
        bindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        mCurrentTransformFeedback = nullptr;
    }
}

GLuint StateManagerGL::getDefaultVAO() const
{
    return mDefaultVAO;
}

VertexArrayStateGL *StateManagerGL::getDefaultVAOState()
{
    return &mDefaultVAOState;
}

void StateManagerGL::validateState() const
{
    // Current program
    ValidateStateHelper(mFunctions, mProgram, GL_CURRENT_PROGRAM, "mProgram", "GL_CURRENT_PROGRAM");

    // Buffers
    for (gl::BufferBinding bindingType : angle::AllEnums<gl::BufferBinding>())
    {
        // These binding types need compute support to be queried
        if (bindingType == gl::BufferBinding::AtomicCounter ||
            bindingType == gl::BufferBinding::DispatchIndirect ||
            bindingType == gl::BufferBinding::ShaderStorage)
        {
            if (!nativegl::SupportsCompute(mFunctions))
            {
                continue;
            }
        }

        // Transform feedback buffer bindings are tracked in TransformFeedbackGL
        if (bindingType == gl::BufferBinding::TransformFeedback)
        {
            continue;
        }

        GLenum bindingTypeGL  = nativegl::GetBufferBindingQuery(bindingType);
        std::string localName = "mBuffers[" + ToString(bindingType) + "]";
        ValidateStateHelper(mFunctions, mBuffers[bindingType], bindingTypeGL, localName.c_str(),
                            nativegl::GetBufferBindingString(bindingType).c_str());
    }

    // Vertex array object
    ValidateStateHelper(mFunctions, mVAO, GL_VERTEX_ARRAY_BINDING, "mVAO",
                        "GL_VERTEX_ARRAY_BINDING");
}

template <>
void StateManagerGL::get(GLenum name, GLboolean *value)
{
    mFunctions->getBooleanv(name, value);
    ASSERT(mFunctions->getError() == GL_NO_ERROR);
}

template <>
void StateManagerGL::get(GLenum name, bool *value)
{
    GLboolean v;
    get(name, &v);
    *value = (v == GL_TRUE);
}

template <>
void StateManagerGL::get(GLenum name, std::array<bool, 4> *values)
{
    GLboolean v[4];
    get(name, v);
    for (size_t i = 0; i < 4; i++)
    {
        (*values)[i] = (v[i] == GL_TRUE);
    }
}

template <>
void StateManagerGL::get(GLenum name, GLint *value)
{
    mFunctions->getIntegerv(name, value);
    ASSERT(mFunctions->getError() == GL_NO_ERROR);
}

template <>
void StateManagerGL::get(GLenum name, GLenum *value)
{
    GLint v;
    get(name, &v);
    *value = static_cast<GLenum>(v);
}

template <>
void StateManagerGL::get(GLenum name, gl::Rectangle *rect)
{
    GLint v[4];
    get(name, v);
    *rect = gl::Rectangle(v[0], v[1], v[2], v[3]);
}

template <>
void StateManagerGL::get(GLenum name, GLfloat *value)
{
    mFunctions->getFloatv(name, value);
    ASSERT(mFunctions->getError() == GL_NO_ERROR);
}

template <>
void StateManagerGL::get(GLenum name, gl::ColorF *color)
{
    GLfloat v[4];
    get(name, v);
    *color = gl::ColorF(v[0], v[1], v[2], v[3]);
}

void StateManagerGL::syncFromNativeContext(const gl::Extensions &extensions,
                                           ExternalContextState *state)
{
    ASSERT(mFunctions->getError() == GL_NO_ERROR);

    auto *platform   = ANGLEPlatformCurrent();
    double startTime = platform->currentTime(platform);

    get(GL_VIEWPORT, &state->viewport);
    if (mViewport != state->viewport)
    {
        mViewport = state->viewport;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_VIEWPORT);
    }

    if (extensions.clipControlEXT)
    {
        get(GL_CLIP_ORIGIN, &state->clipOrigin);
        get(GL_CLIP_DEPTH_MODE, &state->clipDepthMode);
        if (mClipOrigin != gl::FromGLenum<gl::ClipOrigin>(state->clipOrigin) ||
            mClipDepthMode != gl::FromGLenum<gl::ClipDepthMode>(state->clipDepthMode))
        {
            mClipOrigin    = gl::FromGLenum<gl::ClipOrigin>(state->clipOrigin);
            mClipDepthMode = gl::FromGLenum<gl::ClipDepthMode>(state->clipDepthMode);
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
            mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_CLIP_CONTROL);
        }
    }

    get(GL_SCISSOR_TEST, &state->scissorTest);
    if (mScissorTestEnabled != static_cast<bool>(state->scissorTest))
    {
        mScissorTestEnabled = state->scissorTest;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SCISSOR_TEST_ENABLED);
    }

    get(GL_SCISSOR_BOX, &state->scissorBox);
    if (mScissor != state->scissorBox)
    {
        mScissor = state->scissorBox;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SCISSOR);
    }

    get(GL_DEPTH_TEST, &state->depthTest);
    if (mDepthTestEnabled != state->depthTest)
    {
        mDepthTestEnabled = state->depthTest;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_TEST_ENABLED);
    }

    get(GL_CULL_FACE, &state->cullFace);
    if (mCullFaceEnabled != state->cullFace)
    {
        mCullFaceEnabled = state->cullFace;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CULL_FACE_ENABLED);
    }

    get(GL_CULL_FACE_MODE, &state->cullFaceMode);
    if (mCullFace != gl::FromGLenum<gl::CullFaceMode>(state->cullFaceMode))
    {
        mCullFace = gl::FromGLenum<gl::CullFaceMode>(state->cullFaceMode);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CULL_FACE);
    }

    get(GL_COLOR_WRITEMASK, &state->colorMask);
    auto colorMask = mBlendStateExt.expandColorMaskValue(state->colorMask[0], state->colorMask[1],
                                                         state->colorMask[2], state->colorMask[3]);
    if (mBlendStateExt.getColorMaskBits() != colorMask)
    {
        mBlendStateExt.setColorMaskBits(colorMask);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_COLOR_MASK);
    }

    get(GL_CURRENT_PROGRAM, &state->currentProgram);
    if (mProgram != static_cast<GLuint>(state->currentProgram))
    {
        mProgram = state->currentProgram;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PROGRAM_BINDING);
    }

    get(GL_COLOR_CLEAR_VALUE, &state->colorClear);
    if (mClearColor != state->colorClear)
    {
        mClearColor = state->colorClear;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CLEAR_COLOR);
    }

    get(GL_DEPTH_CLEAR_VALUE, &state->depthClear);
    if (mClearDepth != state->depthClear)
    {
        mClearDepth = state->depthClear;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CLEAR_DEPTH);
    }

    get(GL_DEPTH_FUNC, &state->depthFunc);
    if (mDepthFunc != static_cast<GLenum>(state->depthFunc))
    {
        mDepthFunc = state->depthFunc;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_FUNC);
    }

    get(GL_DEPTH_WRITEMASK, &state->depthMask);
    if (mDepthMask != state->depthMask)
    {
        mDepthMask = state->depthMask;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_MASK);
    }

    get(GL_DEPTH_RANGE, state->depthRage);
    if (mNear != state->depthRage[0] || mFar != state->depthRage[1])
    {
        mNear = state->depthRage[0];
        mFar  = state->depthRage[1];
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_DEPTH_RANGE);
    }

    get(GL_FRONT_FACE, &state->frontFace);
    if (mFrontFace != static_cast<GLenum>(state->frontFace))
    {
        mFrontFace = state->frontFace;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_FRONT_FACE);
    }

    get(GL_LINE_WIDTH, &state->lineWidth);
    if (mLineWidth != state->lineWidth)
    {
        mLineWidth = state->lineWidth;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_LINE_WIDTH);
    }

    get(GL_POLYGON_OFFSET_FACTOR, &state->polygonOffsetFactor);
    get(GL_POLYGON_OFFSET_UNITS, &state->polygonOffsetUnits);
    if (mPolygonOffsetFactor != state->polygonOffsetFactor ||
        mPolygonOffsetUnits != state->polygonOffsetUnits)
    {
        mPolygonOffsetFactor = state->polygonOffsetFactor;
        mPolygonOffsetUnits  = state->polygonOffsetUnits;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_POLYGON_OFFSET);
    }

    if (extensions.polygonOffsetClampEXT)
    {
        get(GL_POLYGON_OFFSET_CLAMP_EXT, &state->polygonOffsetClamp);
        if (mPolygonOffsetClamp != state->polygonOffsetClamp)
        {
            mPolygonOffsetClamp = state->polygonOffsetClamp;
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_POLYGON_OFFSET);
        }
    }

    if (extensions.depthClampEXT)
    {
        get(GL_DEPTH_CLAMP_EXT, &state->enableDepthClamp);
        if (mDepthClampEnabled != state->enableDepthClamp)
        {
            mDepthClampEnabled = state->enableDepthClamp;
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
            mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_DEPTH_CLAMP_ENABLED);
        }
    }

    get(GL_SAMPLE_COVERAGE_VALUE, &state->sampleCoverageValue);
    get(GL_SAMPLE_COVERAGE_INVERT, &state->sampleCoverageInvert);
    if (mSampleCoverageValue != state->sampleCoverageValue ||
        mSampleCoverageInvert != state->sampleCoverageInvert)
    {
        mSampleCoverageValue  = state->sampleCoverageValue;
        mSampleCoverageInvert = state->sampleCoverageInvert;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_COVERAGE);
    }

    get(GL_DITHER, &state->enableDither);
    if (mDitherEnabled != state->enableDither)
    {
        mDitherEnabled = state->enableDither;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_DITHER_ENABLED);
    }

    if (extensions.polygonModeAny())
    {
        get(GL_POLYGON_MODE_NV, &state->polygonMode);
        if (mPolygonMode != gl::FromGLenum<gl::PolygonMode>(state->polygonMode))
        {
            mPolygonMode = gl::FromGLenum<gl::PolygonMode>(state->polygonMode);
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
            mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_POLYGON_MODE);
        }

        if (extensions.polygonModeNV)
        {
            get(GL_POLYGON_OFFSET_POINT_NV, &state->enablePolygonOffsetPoint);
            if (mPolygonOffsetPointEnabled != state->enablePolygonOffsetPoint)
            {
                mPolygonOffsetPointEnabled = state->enablePolygonOffsetPoint;
                mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
                mLocalExtendedDirtyBits.set(
                    gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_POINT_ENABLED);
            }
        }

        get(GL_POLYGON_OFFSET_LINE_NV, &state->enablePolygonOffsetLine);
        if (mPolygonOffsetLineEnabled != state->enablePolygonOffsetLine)
        {
            mPolygonOffsetLineEnabled = state->enablePolygonOffsetLine;
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
            mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED);
        }
    }

    get(GL_POLYGON_OFFSET_FILL, &state->enablePolygonOffsetFill);
    if (mPolygonOffsetFillEnabled != state->enablePolygonOffsetFill)
    {
        mPolygonOffsetFillEnabled = state->enablePolygonOffsetFill;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED);
    }

    get(GL_SAMPLE_ALPHA_TO_COVERAGE, &state->enableSampleAlphaToCoverage);
    if (mSampleAlphaToOneEnabled != state->enableSampleAlphaToCoverage)
    {
        mSampleAlphaToOneEnabled = state->enableSampleAlphaToCoverage;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_ONE);
    }

    get(GL_SAMPLE_COVERAGE, &state->enableSampleCoverage);
    if (mSampleCoverageEnabled != state->enableSampleCoverage)
    {
        mSampleCoverageEnabled = state->enableSampleCoverage;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED);
    }

    if (extensions.multisampleCompatibilityEXT)
    {
        get(GL_MULTISAMPLE, &state->multisampleEnabled);
        if (mMultisamplingEnabled != state->multisampleEnabled)
        {
            mMultisamplingEnabled = state->multisampleEnabled;
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_MULTISAMPLING);
        }
    }

    syncBlendFromNativeContext(extensions, state);
    syncFramebufferFromNativeContext(extensions, state);
    syncPixelPackUnpackFromNativeContext(extensions, state);
    syncStencilFromNativeContext(extensions, state);
    syncVertexArraysFromNativeContext(extensions, state);
    syncBufferBindingsFromNativeContext(extensions, state);
    syncTextureUnitsFromNativeContext(extensions, state);

    ASSERT(mFunctions->getError() == GL_NO_ERROR);

    double delta = platform->currentTime(platform) - startTime;
    int us       = static_cast<int>(delta * 1000000.0);
    ANGLE_HISTOGRAM_COUNTS("GPU.ANGLE.SyncFromNativeContextMicroseconds", us);
}

void StateManagerGL::restoreNativeContext(const gl::Extensions &extensions,
                                          const ExternalContextState *state)
{
    ASSERT(mFunctions->getError() == GL_NO_ERROR);

    setViewport(state->viewport);
    if (extensions.clipControlEXT)
    {
        setClipControl(gl::FromGLenum<gl::ClipOrigin>(state->clipOrigin),
                       gl::FromGLenum<gl::ClipDepthMode>(state->clipDepthMode));
    }

    setScissorTestEnabled(state->scissorTest);
    setScissor(state->scissorBox);

    setDepthTestEnabled(state->depthTest);

    setCullFaceEnabled(state->cullFace);
    setCullFace(gl::FromGLenum<gl::CullFaceMode>(state->cullFaceMode));

    setColorMask(state->colorMask[0], state->colorMask[1], state->colorMask[2],
                 state->colorMask[3]);

    forceUseProgram(state->currentProgram);

    setClearColor(state->colorClear);

    setClearDepth(state->depthClear);
    setDepthFunc(state->depthFunc);
    setDepthMask(state->depthMask);
    setDepthRange(state->depthRage[0], state->depthRage[1]);

    setFrontFace(state->frontFace);

    setLineWidth(state->lineWidth);

    setPolygonOffset(state->polygonOffsetFactor, state->polygonOffsetUnits,
                     state->polygonOffsetClamp);

    if (extensions.depthClampEXT)
    {
        setDepthClampEnabled(state->enableDepthClamp);
    }

    setSampleCoverage(state->sampleCoverageValue, state->sampleCoverageInvert);

    setDitherEnabled(state->enableDither);

    if (extensions.polygonModeAny())
    {
        setPolygonMode(gl::FromGLenum<gl::PolygonMode>(state->polygonMode));
        if (extensions.polygonModeNV)
        {
            setPolygonOffsetPointEnabled(state->enablePolygonOffsetPoint);
        }
        setPolygonOffsetLineEnabled(state->enablePolygonOffsetLine);
    }

    setPolygonOffsetFillEnabled(state->enablePolygonOffsetFill);

    setSampleAlphaToOneStateEnabled(state->enableSampleAlphaToCoverage);

    setSampleCoverageEnabled(state->enableSampleCoverage);

    if (extensions.multisampleCompatibilityEXT)
        setMultisamplingStateEnabled(state->multisampleEnabled);

    restoreBlendNativeContext(extensions, state);
    restoreFramebufferNativeContext(extensions, state);
    restorePixelPackUnpackNativeContext(extensions, state);
    restoreStencilNativeContext(extensions, state);
    restoreVertexArraysNativeContext(extensions, state);
    restoreBufferBindingsNativeContext(extensions, state);
    restoreTextureUnitsNativeContext(extensions, state);

    // if (mFunctions->coverageModulationNV) ?
    setCoverageModulation(GL_NONE);

    ASSERT(mFunctions->getError() == GL_NO_ERROR);
}

void StateManagerGL::syncBlendFromNativeContext(const gl::Extensions &extensions,
                                                ExternalContextState *state)
{
    get(GL_BLEND, &state->blendEnabled);
    if (mBlendStateExt.getEnabledMask() !=
        (state->blendEnabled ? mBlendStateExt.getAllEnabledMask() : gl::DrawBufferMask::Zero()))
    {
        mBlendStateExt.setEnabled(state->blendEnabled);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_ENABLED);
    }

    get(GL_BLEND_SRC_RGB, &state->blendSrcRgb);
    get(GL_BLEND_DST_RGB, &state->blendDestRgb);
    get(GL_BLEND_SRC_ALPHA, &state->blendSrcAlpha);
    get(GL_BLEND_DST_ALPHA, &state->blendDestAlpha);
    if (mBlendStateExt.getSrcColorBits() != mBlendStateExt.expandFactorValue(state->blendSrcRgb) ||
        mBlendStateExt.getDstColorBits() != mBlendStateExt.expandFactorValue(state->blendDestRgb) ||
        mBlendStateExt.getSrcAlphaBits() !=
            mBlendStateExt.expandFactorValue(state->blendSrcAlpha) ||
        mBlendStateExt.getDstAlphaBits() != mBlendStateExt.expandFactorValue(state->blendDestAlpha))
    {
        mBlendStateExt.setFactors(state->blendSrcRgb, state->blendDestRgb, state->blendSrcAlpha,
                                  state->blendDestAlpha);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_FUNCS);
    }

    get(GL_BLEND_COLOR, &state->blendColor);
    if (mBlendColor != state->blendColor)
    {
        mBlendColor = state->blendColor;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_COLOR);
    }

    get(GL_BLEND_EQUATION_RGB, &state->blendEquationRgb);
    get(GL_BLEND_EQUATION_ALPHA, &state->blendEquationAlpha);
    if (mBlendStateExt.getEquationColorBits() !=
            mBlendStateExt.expandEquationValue(state->blendEquationRgb) ||
        mBlendStateExt.getEquationAlphaBits() !=
            mBlendStateExt.expandEquationValue(state->blendEquationAlpha))
    {
        mBlendStateExt.setEquations(state->blendEquationRgb, state->blendEquationAlpha);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_EQUATIONS);
    }

    if (extensions.blendEquationAdvancedCoherentKHR)
    {
        get(GL_BLEND_ADVANCED_COHERENT_KHR, &state->enableBlendEquationAdvancedCoherent);
        if (mBlendAdvancedCoherent != state->enableBlendEquationAdvancedCoherent)
        {
            setBlendAdvancedCoherent(state->enableBlendEquationAdvancedCoherent);
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
            mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_BLEND_ADVANCED_COHERENT);
        }
    }
}

void StateManagerGL::restoreBlendNativeContext(const gl::Extensions &extensions,
                                               const ExternalContextState *state)
{
    setBlendEnabled(state->blendEnabled);

    mFunctions->blendFuncSeparate(state->blendSrcRgb, state->blendDestRgb, state->blendSrcAlpha,
                                  state->blendDestAlpha);
    mBlendStateExt.setFactors(state->blendSrcRgb, state->blendDestRgb, state->blendSrcAlpha,
                              state->blendDestAlpha);
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_FUNCS);

    setBlendColor(state->blendColor);

    mFunctions->blendEquationSeparate(state->blendEquationRgb, state->blendEquationAlpha);
    mBlendStateExt.setEquations(state->blendEquationRgb, state->blendEquationAlpha);
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_BLEND_EQUATIONS);

    if (extensions.blendEquationAdvancedCoherentKHR)
    {
        setBlendAdvancedCoherent(state->enableBlendEquationAdvancedCoherent);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_EXTENDED);
        mLocalExtendedDirtyBits.set(gl::state::EXTENDED_DIRTY_BIT_BLEND_ADVANCED_COHERENT);
    }
}

void StateManagerGL::syncFramebufferFromNativeContext(const gl::Extensions &extensions,
                                                      ExternalContextState *state)
{
    // TODO: wrap fbo into an EGLSurface
    get(GL_FRAMEBUFFER_BINDING, &state->framebufferBinding);
    if (mFramebuffers[angle::FramebufferBindingDraw] !=
        static_cast<GLenum>(state->framebufferBinding))
    {
        mFramebuffers[angle::FramebufferBindingDraw] =
            static_cast<GLenum>(state->framebufferBinding);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING);
    }
    if (mFramebuffers[angle::FramebufferBindingRead] !=
        static_cast<GLenum>(state->framebufferBinding))
    {
        mFramebuffers[angle::FramebufferBindingRead] =
            static_cast<GLenum>(state->framebufferBinding);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING);
    }
}

void StateManagerGL::restoreFramebufferNativeContext(const gl::Extensions &extensions,
                                                     const ExternalContextState *state)
{
    bindFramebuffer(GL_FRAMEBUFFER, state->framebufferBinding);
}

void StateManagerGL::syncPixelPackUnpackFromNativeContext(const gl::Extensions &extensions,
                                                          ExternalContextState *state)
{
    get(GL_PACK_ALIGNMENT, &state->packAlignment);
    if (mPackAlignment != state->packAlignment)
    {
        mPackAlignment = state->packAlignment;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PACK_STATE);
    }

    get(GL_UNPACK_ALIGNMENT, &state->unpackAlignment);
    if (mUnpackAlignment != state->unpackAlignment)
    {
        mUnpackAlignment = state->unpackAlignment;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }
}

void StateManagerGL::restorePixelPackUnpackNativeContext(const gl::Extensions &extensions,
                                                         const ExternalContextState *state)
{
    if (mPackAlignment != state->packAlignment)
    {
        mFunctions->pixelStorei(GL_PACK_ALIGNMENT, state->packAlignment);
        mPackAlignment = state->packAlignment;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_PACK_STATE);
    }

    if (mUnpackAlignment != state->unpackAlignment)
    {
        mFunctions->pixelStorei(GL_UNPACK_ALIGNMENT, state->unpackAlignment);
        mUnpackAlignment = state->unpackAlignment;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_UNPACK_STATE);
    }
}

void StateManagerGL::syncStencilFromNativeContext(const gl::Extensions &extensions,
                                                  ExternalContextState *state)
{
    get(GL_STENCIL_TEST, &state->stencilState.stencilTestEnabled);
    if (state->stencilState.stencilTestEnabled != mStencilTestEnabled)
    {
        mStencilTestEnabled = state->stencilState.stencilTestEnabled;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_TEST_ENABLED);
    }

    get(GL_STENCIL_FUNC, &state->stencilState.stencilFrontFunc);
    get(GL_STENCIL_VALUE_MASK, &state->stencilState.stencilFrontMask);
    get(GL_STENCIL_REF, &state->stencilState.stencilFrontRef);
    if (state->stencilState.stencilFrontFunc != mStencilFrontFunc ||
        state->stencilState.stencilFrontMask != mStencilFrontValueMask ||
        state->stencilState.stencilFrontRef != mStencilFrontRef)
    {
        mStencilFrontFunc      = state->stencilState.stencilFrontFunc;
        mStencilFrontValueMask = state->stencilState.stencilFrontMask;
        mStencilFrontRef       = state->stencilState.stencilFrontRef;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_FUNCS_FRONT);
    }

    get(GL_STENCIL_BACK_FUNC, &state->stencilState.stencilBackFunc);
    get(GL_STENCIL_BACK_VALUE_MASK, &state->stencilState.stencilBackMask);
    get(GL_STENCIL_BACK_REF, &state->stencilState.stencilBackRef);
    if (state->stencilState.stencilBackFunc != mStencilBackFunc ||
        state->stencilState.stencilBackMask != mStencilBackValueMask ||
        state->stencilState.stencilBackRef != mStencilBackRef)
    {
        mStencilBackFunc      = state->stencilState.stencilBackFunc;
        mStencilBackValueMask = state->stencilState.stencilBackMask;
        mStencilBackRef       = state->stencilState.stencilBackRef;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_FUNCS_BACK);
    }

    get(GL_STENCIL_CLEAR_VALUE, &state->stencilState.stencilClear);
    if (mClearStencil != state->stencilState.stencilClear)
    {
        mClearStencil = state->stencilState.stencilClear;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_CLEAR_STENCIL);
    }

    get(GL_STENCIL_WRITEMASK, &state->stencilState.stencilFrontWritemask);
    if (mStencilFrontWritemask != static_cast<GLenum>(state->stencilState.stencilFrontWritemask))
    {
        mStencilFrontWritemask = state->stencilState.stencilFrontWritemask;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
    }

    get(GL_STENCIL_BACK_WRITEMASK, &state->stencilState.stencilBackWritemask);
    if (mStencilBackWritemask != static_cast<GLenum>(state->stencilState.stencilBackWritemask))
    {
        mStencilBackWritemask = state->stencilState.stencilBackWritemask;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
    }

    get(GL_STENCIL_FAIL, &state->stencilState.stencilFrontFailOp);
    get(GL_STENCIL_PASS_DEPTH_FAIL, &state->stencilState.stencilFrontZFailOp);
    get(GL_STENCIL_PASS_DEPTH_PASS, &state->stencilState.stencilFrontZPassOp);
    if (mStencilFrontStencilFailOp != static_cast<GLenum>(state->stencilState.stencilFrontFailOp) ||
        mStencilFrontStencilPassDepthFailOp !=
            static_cast<GLenum>(state->stencilState.stencilFrontZFailOp) ||
        mStencilFrontStencilPassDepthPassOp !=
            static_cast<GLenum>(state->stencilState.stencilFrontZPassOp))
    {
        mStencilFrontStencilFailOp = static_cast<GLenum>(state->stencilState.stencilFrontFailOp);
        mStencilFrontStencilPassDepthFailOp =
            static_cast<GLenum>(state->stencilState.stencilFrontZFailOp);
        mStencilFrontStencilPassDepthPassOp =
            static_cast<GLenum>(state->stencilState.stencilFrontZPassOp);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_OPS_FRONT);
    }

    get(GL_STENCIL_BACK_FAIL, &state->stencilState.stencilBackFailOp);
    get(GL_STENCIL_BACK_PASS_DEPTH_FAIL, &state->stencilState.stencilBackZFailOp);
    get(GL_STENCIL_BACK_PASS_DEPTH_PASS, &state->stencilState.stencilBackZPassOp);
    if (mStencilBackStencilFailOp != static_cast<GLenum>(state->stencilState.stencilBackFailOp) ||
        mStencilBackStencilPassDepthFailOp !=
            static_cast<GLenum>(state->stencilState.stencilBackZFailOp) ||
        mStencilBackStencilPassDepthPassOp !=
            static_cast<GLenum>(state->stencilState.stencilBackZPassOp))
    {
        mStencilBackStencilFailOp = static_cast<GLenum>(state->stencilState.stencilBackFailOp);
        mStencilBackStencilPassDepthFailOp =
            static_cast<GLenum>(state->stencilState.stencilBackZFailOp);
        mStencilBackStencilPassDepthPassOp =
            static_cast<GLenum>(state->stencilState.stencilBackZPassOp);
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_STENCIL_OPS_BACK);
    }
}

void StateManagerGL::restoreStencilNativeContext(const gl::Extensions &extensions,
                                                 const ExternalContextState *state)
{
    setStencilTestEnabled(state->stencilState.stencilTestEnabled);
    setStencilFrontFuncs(state->stencilState.stencilFrontFunc, state->stencilState.stencilFrontMask,
                         state->stencilState.stencilFrontRef);
    setStencilBackFuncs(state->stencilState.stencilBackFunc, state->stencilState.stencilBackMask,
                        state->stencilState.stencilBackRef);
    setClearStencil(state->stencilState.stencilClear);
    setStencilFrontWritemask(state->stencilState.stencilFrontWritemask);
    setStencilBackWritemask(state->stencilState.stencilBackWritemask);
    setStencilFrontOps(state->stencilState.stencilFrontFailOp,
                       state->stencilState.stencilFrontZFailOp,
                       state->stencilState.stencilFrontZPassOp);
    setStencilBackOps(state->stencilState.stencilBackFailOp, state->stencilState.stencilBackZFailOp,
                      state->stencilState.stencilBackZPassOp);
}

void StateManagerGL::syncBufferBindingsFromNativeContext(const gl::Extensions &extensions,
                                                         ExternalContextState *state)
{
    get(GL_ARRAY_BUFFER_BINDING, &state->vertexArrayBufferBinding);
    mBuffers[gl::BufferBinding::Array] = state->vertexArrayBufferBinding;

    get(GL_ELEMENT_ARRAY_BUFFER_BINDING, &state->elementArrayBufferBinding);
    mBuffers[gl::BufferBinding::ElementArray] = state->elementArrayBufferBinding;

    if (mVAOState && mVAOState->elementArrayBuffer != state->elementArrayBufferBinding)
    {
        mVAOState->elementArrayBuffer = state->elementArrayBufferBinding;
        mLocalDirtyBits.set(gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING);
    }
}

void StateManagerGL::restoreBufferBindingsNativeContext(const gl::Extensions &extensions,
                                                        const ExternalContextState *state)
{
    bindBuffer(gl::BufferBinding::Array, state->vertexArrayBufferBinding);
    bindBuffer(gl::BufferBinding::ElementArray, state->elementArrayBufferBinding);
}

void StateManagerGL::syncTextureUnitsFromNativeContext(const gl::Extensions &extensions,
                                                       ExternalContextState *state)
{
    get(GL_ACTIVE_TEXTURE, &state->activeTexture);

    for (size_t i = 0; i < state->textureBindings.size(); ++i)
    {
        auto &bindings = state->textureBindings[i];
        activeTexture(i);
        get(GL_TEXTURE_BINDING_2D, &bindings.texture2d);
        get(GL_TEXTURE_BINDING_CUBE_MAP, &bindings.textureCubeMap);
        get(GL_TEXTURE_BINDING_EXTERNAL_OES, &bindings.textureExternalOES);
        if (mTextures[gl::TextureType::_2D][i] != static_cast<GLuint>(bindings.texture2d) ||
            mTextures[gl::TextureType::CubeMap][i] !=
                static_cast<GLuint>(bindings.textureCubeMap) ||
            mTextures[gl::TextureType::External][i] !=
                static_cast<GLuint>(bindings.textureExternalOES))
        {
            mTextures[gl::TextureType::_2D][i]      = bindings.texture2d;
            mTextures[gl::TextureType::CubeMap][i]  = bindings.textureCubeMap;
            mTextures[gl::TextureType::External][i] = bindings.textureExternalOES;
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_TEXTURE_BINDINGS);
        }
    }
}

void StateManagerGL::restoreTextureUnitsNativeContext(const gl::Extensions &extensions,
                                                      const ExternalContextState *state)
{
    for (size_t i = 0; i < state->textureBindings.size(); ++i)
    {
        const auto &bindings = state->textureBindings[i];
        activeTexture(i);
        bindTexture(gl::TextureType::_2D, bindings.texture2d);
        bindTexture(gl::TextureType::CubeMap, bindings.textureCubeMap);
        bindTexture(gl::TextureType::External, bindings.textureExternalOES);
        bindSampler(i, 0);
    }
    activeTexture(state->activeTexture - GL_TEXTURE0);
}

void StateManagerGL::syncVertexArraysFromNativeContext(const gl::Extensions &extensions,
                                                       ExternalContextState *state)
{
    if (mSupportsVertexArrayObjects)
    {
        get(GL_VERTEX_ARRAY_BINDING, &state->vertexArrayBinding);

        if (state->vertexArrayBinding != 0 || mVAO != 0)
        {
            // Force-bind VAO 0 if it's either not already bound or StateManagerGL thinks it's not
            // bound.
            forceBindVertexArray(0, &mDefaultVAOState);
        }
    }

    // Save the state of the default VAO
    state->defaultVertexArrayAttributes.resize(mDefaultVAOState.attributes.size());
    for (GLint i = 0; i < static_cast<GLint>(state->defaultVertexArrayAttributes.size()); i++)
    {
        ExternalContextVertexAttribute &externalAttrib = state->defaultVertexArrayAttributes[i];

        GLint enabled = 0;
        mFunctions->getVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        externalAttrib.enabled = (enabled != GL_FALSE);

        GLint size = 0;
        mFunctions->getVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
        GLint type = 0;
        mFunctions->getVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
        GLint normalized = 0;
        mFunctions->getVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);
        externalAttrib.format = &angle::Format::Get(gl::GetVertexFormatID(
            gl::FromGLenum<gl::VertexAttribType>(type), normalized != GL_FALSE, size, false));

        GLint stride = 0;
        mFunctions->getVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
        externalAttrib.stride = stride;

        mFunctions->getVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER,
                                            &externalAttrib.pointer);

        GLint buffer = 0;
        mFunctions->getVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
        externalAttrib.buffer = buffer;

        GLfloat currentData[4] = {0};
        mFunctions->getVertexAttribfv(i, GL_CURRENT_VERTEX_ATTRIB, currentData);
        externalAttrib.currentData.setFloatValues(currentData);

        // Update our local state to reflect the external context state
        VertexAttributeGL &localAttribute = mDefaultVAOState.attributes[i];
        localAttribute.enabled            = externalAttrib.enabled;
        localAttribute.format             = externalAttrib.format;
        localAttribute.pointer            = externalAttrib.pointer;
        localAttribute.relativeOffset     = 0;
        localAttribute.bindingIndex       = i;

        VertexBindingGL &localBinding = mDefaultVAOState.bindings[i];
        localBinding.stride           = externalAttrib.stride;
        localBinding.buffer           = externalAttrib.buffer;
        localBinding.divisor          = 0;
        localBinding.offset           = 0;

        gl::VertexAttribCurrentValueData &localCurrentData = mVertexAttribCurrentValues[i];
        if (localCurrentData != externalAttrib.currentData)
        {
            localCurrentData = externalAttrib.currentData;
            mLocalDirtyBits.set(gl::state::DIRTY_BIT_CURRENT_VALUES);
            mLocalDirtyCurrentValues.set(i);
        }
    }

    // Mark VAO state dirty and force it to be re-synced on the next draw
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING);
}

void StateManagerGL::restoreVertexArraysNativeContext(const gl::Extensions &extensions,
                                                      const ExternalContextState *state)
{
    if (mSupportsVertexArrayObjects)
    {
        // Restore the default VAO state first.
        bindVertexArray(0, &mDefaultVAOState);
    }

    for (GLint i = 0; i < static_cast<GLint>(state->defaultVertexArrayAttributes.size()); i++)
    {
        const ExternalContextVertexAttribute &externalAttrib =
            state->defaultVertexArrayAttributes[i];
        VertexAttributeGL &localAttribute = mDefaultVAOState.attributes[i];
        VertexBindingGL &localBinding     = mDefaultVAOState.bindings[i];

        if (externalAttrib.format != localAttribute.format ||
            externalAttrib.stride != localBinding.stride ||
            externalAttrib.pointer != localAttribute.pointer ||
            externalAttrib.buffer != localBinding.buffer)
        {
            bindBuffer(gl::BufferBinding::Array, externalAttrib.buffer);
            mFunctions->vertexAttribPointer(i, externalAttrib.format->channelCount,
                                            gl::ToGLenum(externalAttrib.format->vertexAttribType),
                                            externalAttrib.format->isNorm(), externalAttrib.stride,
                                            externalAttrib.pointer);
            if (mFunctions->vertexAttribDivisor)
            {
                mFunctions->vertexAttribDivisor(i, 0);
            }

            localAttribute.format         = externalAttrib.format;
            localAttribute.pointer        = externalAttrib.pointer;
            localAttribute.relativeOffset = 0;
            localAttribute.bindingIndex   = i;

            localBinding.stride  = externalAttrib.stride;
            localBinding.buffer  = externalAttrib.buffer;
            localBinding.divisor = 0;
            localBinding.offset  = 0;
        }

        if (externalAttrib.enabled != localAttribute.enabled)
        {
            if (externalAttrib.enabled)
            {
                mFunctions->enableVertexAttribArray(i);
            }
            else
            {
                mFunctions->disableVertexAttribArray(i);
            }

            localAttribute.enabled = externalAttrib.enabled;
        }

        setAttributeCurrentData(i, externalAttrib.currentData);
    }

    if (mSupportsVertexArrayObjects)
    {
        // Restore the VAO binding
        bindVertexArray(state->vertexArrayBinding, nullptr);
    }

    // Mark VAO state dirty and force it to be re-synced on the next draw
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING);
}

void StateManagerGL::setDefaultVAOStateDirty()
{
    mLocalDirtyBits.set(gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING);
}

}  // namespace rx
