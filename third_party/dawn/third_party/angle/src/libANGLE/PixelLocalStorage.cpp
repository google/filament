//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixelLocalStorage.cpp: Defines the renderer-agnostic container classes
// gl::PixelLocalStorage and gl::PixelLocalStoragePlane for
// ANGLE_shader_pixel_local_storage.

#include "libANGLE/PixelLocalStorage.h"

#include <numeric>
#include "common/FixedVector.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/context_private_call.inl.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/TextureImpl.h"

namespace gl
{
// RAII utilities for working with GL state.
namespace
{
class ScopedBindTexture2D : angle::NonCopyable
{
  public:
    ScopedBindTexture2D(Context *context, TextureID texture)
        : mContext(context),
          mSavedTexBinding2D(
              mContext->getState().getSamplerTextureId(mContext->getState().getActiveSampler(),
                                                       TextureType::_2D))
    {
        mContext->bindTexture(TextureType::_2D, texture);
    }

    ~ScopedBindTexture2D() { mContext->bindTexture(TextureType::_2D, mSavedTexBinding2D); }

  private:
    Context *const mContext;
    TextureID mSavedTexBinding2D;
};

class ScopedRestoreDrawFramebuffer : angle::NonCopyable
{
  public:
    ScopedRestoreDrawFramebuffer(Context *context)
        : mContext(context), mSavedFramebuffer(mContext->getState().getDrawFramebuffer())
    {
        ASSERT(mSavedFramebuffer);
    }

    ~ScopedRestoreDrawFramebuffer() { mContext->bindDrawFramebuffer(mSavedFramebuffer->id()); }

  private:
    Context *const mContext;
    Framebuffer *const mSavedFramebuffer;
};

class ScopedDisableScissor : angle::NonCopyable
{
  public:
    ScopedDisableScissor(Context *context)
        : mContext(context), mScissorTestEnabled(mContext->getState().isScissorTestEnabled())
    {
        if (mScissorTestEnabled)
        {
            ContextPrivateDisable(mContext->getMutablePrivateState(),
                                  mContext->getMutablePrivateStateCache(), GL_SCISSOR_TEST);
        }
    }

    ~ScopedDisableScissor()
    {
        if (mScissorTestEnabled)
        {
            ContextPrivateEnable(mContext->getMutablePrivateState(),
                                 mContext->getMutablePrivateStateCache(), GL_SCISSOR_TEST);
        }
    }

  private:
    Context *const mContext;
    const GLint mScissorTestEnabled;
};

class ScopedEnableColorMask : angle::NonCopyable
{
  public:
    ScopedEnableColorMask(Context *context, int numDrawBuffers)
        : mContext(context), mNumDrawBuffers(numDrawBuffers)
    {
        const State &state = mContext->getState();
        if (!mContext->getExtensions().drawBuffersIndexedAny())
        {
            std::array<bool, 4> &mask = mSavedColorMasks[0];
            state.getBlendStateExt().getColorMaskIndexed(0, &mask[0], &mask[1], &mask[2], &mask[3]);
            ContextPrivateColorMask(mContext->getMutablePrivateState(),
                                    mContext->getMutablePrivateStateCache(), GL_TRUE, GL_TRUE,
                                    GL_TRUE, GL_TRUE);
        }
        else
        {
            for (int i = 0; i < mNumDrawBuffers; ++i)
            {
                std::array<bool, 4> &mask = mSavedColorMasks[i];
                state.getBlendStateExt().getColorMaskIndexed(i, &mask[0], &mask[1], &mask[2],
                                                             &mask[3]);
                ContextPrivateColorMaski(mContext->getMutablePrivateState(),
                                         mContext->getMutablePrivateStateCache(), i, GL_TRUE,
                                         GL_TRUE, GL_TRUE, GL_TRUE);
            }
        }
    }

    ~ScopedEnableColorMask()
    {
        if (!mContext->getExtensions().drawBuffersIndexedAny())
        {
            const std::array<bool, 4> &mask = mSavedColorMasks[0];
            ContextPrivateColorMask(mContext->getMutablePrivateState(),
                                    mContext->getMutablePrivateStateCache(), mask[0], mask[1],
                                    mask[2], mask[3]);
        }
        else
        {
            for (int i = 0; i < mNumDrawBuffers; ++i)
            {
                const std::array<bool, 4> &mask = mSavedColorMasks[i];
                ContextPrivateColorMaski(mContext->getMutablePrivateState(),
                                         mContext->getMutablePrivateStateCache(), i, mask[0],
                                         mask[1], mask[2], mask[3]);
            }
        }
    }

  private:
    Context *const mContext;
    const int mNumDrawBuffers;
    DrawBuffersArray<std::array<bool, 4>> mSavedColorMasks;
};
}  // namespace

PixelLocalStoragePlane::PixelLocalStoragePlane() : mTextureObserver(this, 0) {}

PixelLocalStoragePlane::~PixelLocalStoragePlane()
{
    // Call deinitialize or onContextObjectsLost first!
    // (PixelLocalStorage::deleteContextObjects calls deinitialize.)
    ASSERT(isDeinitialized());
    // We can always expect to receive angle::SubjectMessage::TextureIDDeleted, even if our texture
    // isn't deleted until context teardown. For this reason, we don't need to hold a ref on the
    // underlying texture that is the subject of mTextureObserver.
    ASSERT(mTextureObserver.getSubject() == nullptr);
}

void PixelLocalStoragePlane::onContextObjectsLost()
{
    // We normally call deleteTexture on the memoryless plane texture ID, since we own it, but in
    // this case we can let it go.
    mTextureID = TextureID();
    deinitialize(nullptr);
}

void PixelLocalStoragePlane::deinitialize(Context *context)
{
    if (mMemoryless && mTextureID.value != 0)
    {
        ASSERT(context);
        context->deleteTexture(mTextureID);  // Will deinitialize the texture via observers.
    }
    else
    {
        mInternalformat = GL_NONE;
        mMemoryless     = false;
        mTextureID      = TextureID();
        mTextureObserver.reset();
    }
    ASSERT(isDeinitialized());
}

void PixelLocalStoragePlane::setMemoryless(Context *context, GLenum internalformat)
{
    deinitialize(context);
    mInternalformat = internalformat;
    mMemoryless     = true;
    // The backing texture will get allocated lazily, once we know what dimensions it should be.
    ASSERT(mTextureID.value == 0);
    mTextureImageIndex = ImageIndex::MakeFromType(TextureType::_2D, 0, 0);
}

void PixelLocalStoragePlane::setTextureBacked(Context *context, Texture *tex, int level, int layer)
{
    deinitialize(context);
    ASSERT(tex->getImmutableFormat());
    mInternalformat = tex->getState().getBaseLevelDesc().format.info->internalFormat;
    mMemoryless     = false;
    mTextureID      = tex->id();
    mTextureObserver.bind(tex);
    mTextureImageIndex = ImageIndex::MakeFromType(tex->getType(), level, layer);
}

void PixelLocalStoragePlane::onSubjectStateChange(angle::SubjectIndex index,
                                                  angle::SubjectMessage message)
{
    ASSERT(index == 0);
    switch (message)
    {
        case angle::SubjectMessage::TextureIDDeleted:
            // When a texture object is deleted, any pixel local storage plane to which it is bound
            // is automatically deinitialized.
            ASSERT(mTextureID.value != 0);
            mTextureID = TextureID();
            deinitialize(nullptr);
            break;
        default:
            break;
    }
}

bool PixelLocalStoragePlane::isDeinitialized() const
{
    if (mInternalformat == GL_NONE)
    {
        ASSERT(!isMemoryless());
        ASSERT(mTextureID.value == 0);
        ASSERT(mTextureObserver.getSubject() == nullptr);
        return true;
    }
    return false;
}

GLint PixelLocalStoragePlane::getIntegeri(GLenum target) const
{
    if (!isDeinitialized())
    {
        switch (target)
        {
            case GL_PIXEL_LOCAL_FORMAT_ANGLE:
                return mInternalformat;
            case GL_PIXEL_LOCAL_TEXTURE_NAME_ANGLE:
                return isMemoryless() ? 0 : mTextureID.value;
            case GL_PIXEL_LOCAL_TEXTURE_LEVEL_ANGLE:
                return isMemoryless() ? 0 : mTextureImageIndex.getLevelIndex();
            case GL_PIXEL_LOCAL_TEXTURE_LAYER_ANGLE:
                return isMemoryless() ? 0 : mTextureImageIndex.getLayerIndex();
        }
    }
    // Since GL_NONE == 0, PLS queries all return 0 when the plane is deinitialized.
    static_assert(GL_NONE == 0, "Expecting GL_NONE to be zero.");
    return 0;
}

bool PixelLocalStoragePlane::getTextureImageExtents(const Context *context, Extents *extents) const
{
    ASSERT(!isDeinitialized());
    if (isMemoryless())
    {
        return false;
    }
    Texture *tex = context->getTexture(mTextureID);
    ASSERT(tex != nullptr);
    *extents = tex->getExtents(mTextureImageIndex.getTarget(), mTextureImageIndex.getLevelIndex());
    extents->depth = 0;
    return true;
}

void PixelLocalStoragePlane::ensureBackingTextureIfMemoryless(Context *context, Extents plsExtents)
{
    ASSERT(!isDeinitialized());
    if (!isMemoryless())
    {
        ASSERT(mTextureID.value != 0);
        return;
    }

    // Internal textures backing memoryless planes are always 2D and not mipmapped.
    ASSERT(mTextureImageIndex.getType() == TextureType::_2D);
    ASSERT(mTextureImageIndex.getLevelIndex() == 0);
    ASSERT(mTextureImageIndex.getLayerIndex() == 0);

    Texture *tex = nullptr;
    if (mTextureID.value != 0)
    {
        tex = context->getTexture(mTextureID);
        ASSERT(tex != nullptr);
    }

    // Do we need to allocate a new backing texture?
    if (tex == nullptr ||
        static_cast<GLsizei>(tex->getWidth(TextureTarget::_2D, 0)) != plsExtents.width ||
        static_cast<GLsizei>(tex->getHeight(TextureTarget::_2D, 0)) != plsExtents.height)
    {
        // Call setMemoryless() to release our current data, if any.
        setMemoryless(context, mInternalformat);
        ASSERT(mTextureID.value == 0);

        // Create a new texture that backs the memoryless plane.
        mTextureID = context->createTexture();
        {
            ScopedBindTexture2D scopedBindTexture2D(context, mTextureID);
            context->bindTexture(TextureType::_2D, mTextureID);
            context->texStorage2D(TextureType::_2D, 1, mInternalformat, plsExtents.width,
                                  plsExtents.height);
        }

        tex = context->getTexture(mTextureID);
        ASSERT(tex != nullptr);
        ASSERT(tex->id() == mTextureID);
        mTextureObserver.bind(tex);
    }
}

void PixelLocalStoragePlane::attachToDrawFramebuffer(Context *context, GLenum colorAttachment) const
{
    ASSERT(!isDeinitialized());
    // Call ensureBackingTextureIfMemoryless() first!
    ASSERT(mTextureID.value != 0 && context->getTexture(mTextureID) != nullptr);
    if (mTextureImageIndex.usesTex3D())  // GL_TEXTURE_3D or GL_TEXTURE_2D_ARRAY.
    {
        context->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, colorAttachment, mTextureID,
                                         mTextureImageIndex.getLevelIndex(),
                                         mTextureImageIndex.getLayerIndex());
    }
    else
    {
        context->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, colorAttachment,
                                      mTextureImageIndex.getTarget(), mTextureID,
                                      mTextureImageIndex.getLevelIndex());
    }
}

// Clears the draw buffer at 0-based index 'drawBufferIdx' on the current framebuffer.
class ClearBufferCommands : public PixelLocalStoragePlane::ClearCommands
{
  public:
    ClearBufferCommands(Context *context) : mContext(context) {}

    void clearfv(int drawBufferIdx, const GLfloat value[]) const override
    {
        mContext->clearBufferfv(GL_COLOR, drawBufferIdx, value);
    }

    void cleariv(int drawBufferIdx, const GLint value[]) const override
    {
        mContext->clearBufferiv(GL_COLOR, drawBufferIdx, value);
    }

    void clearuiv(int drawBufferIdx, const GLuint value[]) const override
    {
        mContext->clearBufferuiv(GL_COLOR, drawBufferIdx, value);
    }

  private:
    Context *const mContext;
};

template <typename T, size_t N>
void ClampArray(std::array<T, N> &arr, T lo, T hi)
{
    for (T &x : arr)
    {
        x = std::clamp(x, lo, hi);
    }
}

void PixelLocalStoragePlane::issueClearCommand(ClearCommands *clearCommands,
                                               int target,
                                               GLenum loadop) const
{
    switch (mInternalformat)
    {
        case GL_RGBA8:
        case GL_R32F:
        {
            std::array<GLfloat, 4> clearValue = {0, 0, 0, 0};
            if (loadop == GL_LOAD_OP_CLEAR_ANGLE)
            {
                clearValue = mClearValuef;
                if (mInternalformat == GL_RGBA8)
                {
                    ClampArray(clearValue, 0.f, 1.f);
                }
            }
            clearCommands->clearfv(target, clearValue.data());
            break;
        }
        case GL_RGBA8I:
        {
            std::array<GLint, 4> clearValue = {0, 0, 0, 0};
            if (loadop == GL_LOAD_OP_CLEAR_ANGLE)
            {
                clearValue = mClearValuei;
                ClampArray(clearValue, -128, 127);
            }
            clearCommands->cleariv(target, clearValue.data());
            break;
        }
        case GL_RGBA8UI:
        case GL_R32UI:
        {
            std::array<GLuint, 4> clearValue = {0, 0, 0, 0};
            if (loadop == GL_LOAD_OP_CLEAR_ANGLE)
            {
                clearValue = mClearValueui;
                if (mInternalformat == GL_RGBA8UI)
                {
                    ClampArray(clearValue, 0u, 255u);
                }
            }
            clearCommands->clearuiv(target, clearValue.data());
            break;
        }
        default:
            // Invalid PLS internalformats should not have made it this far.
            UNREACHABLE();
    }
}

void PixelLocalStoragePlane::bindToImage(Context *context, GLuint unit, bool needsR32Packing) const
{
    ASSERT(!isDeinitialized());
    // Call ensureBackingTextureIfMemoryless() first!
    ASSERT(mTextureID.value != 0 && context->getTexture(mTextureID) != nullptr);
    GLenum imageBindingFormat = mInternalformat;
    if (needsR32Packing)
    {
        // D3D and ES require us to pack all PLS formats into r32f, r32i, or r32ui images.
        switch (imageBindingFormat)
        {
            case GL_RGBA8:
            case GL_RGBA8UI:
                imageBindingFormat = GL_R32UI;
                break;
            case GL_RGBA8I:
                imageBindingFormat = GL_R32I;
                break;
        }
    }
    context->bindImageTexture(unit, mTextureID, mTextureImageIndex.getLevelIndex(), GL_FALSE,
                              mTextureImageIndex.getLayerIndex(), GL_READ_WRITE,
                              imageBindingFormat);
}

const Texture *PixelLocalStoragePlane::getBackingTexture(const Context *context) const
{
    ASSERT(!isDeinitialized());
    ASSERT(!isMemoryless());
    const Texture *tex = context->getTexture(mTextureID);
    ASSERT(tex != nullptr);
    return tex;
}

PixelLocalStorage::PixelLocalStorage(const ShPixelLocalStorageOptions &plsOptions, const Caps &caps)
    : mPLSOptions(plsOptions), mPlanes(caps.maxPixelLocalStoragePlanes)
{}

PixelLocalStorage::~PixelLocalStorage() {}

namespace
{
bool AllPlanesDeinitialized(
    const angle::FixedVector<PixelLocalStoragePlane, IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES>
        &planes,
    const Context *context)
{
    for (const PixelLocalStoragePlane &plane : planes)
    {
        if (!plane.isDeinitialized())
        {
            return false;
        }
    }
    return true;
}
}  // namespace

void PixelLocalStorage::onFramebufferDestroyed(const Context *context)
{
    if (!context->isReferenced())
    {
        // If the Context's refcount is zero, we know it's in a teardown state and we can just let
        // go of our GL objects -- they get cleaned up as part of context teardown. Otherwise, the
        // Context should have called deleteContextObjects before reaching this point.
        onContextObjectsLost();
        for (PixelLocalStoragePlane &plane : mPlanes)
        {
            plane.onContextObjectsLost();
        }
    }
    // Call deleteContextObjects() when a Framebuffer is destroyed outside of context teardown!
    ASSERT(AllPlanesDeinitialized(mPlanes, context));
}

void PixelLocalStorage::deleteContextObjects(Context *context)
{
    onDeleteContextObjects(context);
    for (PixelLocalStoragePlane &plane : mPlanes)
    {
        plane.deinitialize(context);
    }
}

void PixelLocalStorage::begin(Context *context, GLsizei n, const GLenum loadops[])
{
    // Find the pixel local storage rendering dimensions.
    Extents plsExtents;
    bool hasPLSExtents = false;
    for (GLsizei i = 0; i < n; ++i)
    {
        PixelLocalStoragePlane &plane = mPlanes[i];
        if (plane.getTextureImageExtents(context, &plsExtents))
        {
            hasPLSExtents = true;
            break;
        }
    }
    if (!hasPLSExtents)
    {
        // All PLS planes are memoryless. Use the rendering area of the framebuffer instead.
        plsExtents =
            context->getState().getDrawFramebuffer()->getState().getAttachmentExtentsIntersection();
        ASSERT(plsExtents.depth == 0);
    }
    for (GLsizei i = 0; i < n; ++i)
    {
        PixelLocalStoragePlane &plane = mPlanes[i];
        if (mPLSOptions.type == ShPixelLocalStorageType::ImageLoadStore ||
            mPLSOptions.type == ShPixelLocalStorageType::FramebufferFetch)
        {
            plane.ensureBackingTextureIfMemoryless(context, plsExtents);
        }
        plane.markActive(true);
    }

    // Disable blend and enable the full color mask on the draw buffers reserved for PLS.
    const Caps &caps           = context->getCaps();
    GLint firstPLSDrawBuffer   = FirstOverriddenDrawBuffer(caps, n);
    PrivateState *privateState = context->getMutablePrivateState();
    if (firstPLSDrawBuffer == 0)
    {
        privateState->setBlend(false);
        privateState->setColorMask(true, true, true, true);
    }
    else
    {
        ASSERT(context->getExtensions().drawBuffersIndexedAny());
        for (GLint i = firstPLSDrawBuffer; i < caps.maxDrawBuffers; ++i)
        {
            privateState->setBlendIndexed(false, i);
            privateState->setColorMaskIndexed(true, true, true, true, i);
        }
    }

    onBegin(context, n, loadops, plsExtents);
}

void PixelLocalStorage::end(Context *context, GLsizei n, const GLenum storeops[])
{
    onEnd(context, n, storeops);

    for (GLsizei i = 0; i < n; ++i)
    {
        mPlanes[i].markActive(false);
    }
}

void PixelLocalStorage::barrier(Context *context)
{
    ASSERT(!context->getExtensions().shaderPixelLocalStorageCoherentANGLE);
    onBarrier(context);
}

void PixelLocalStorage::interrupt(Context *context)
{
    if (mInterruptCount == 0)
    {
        mActivePlanesAtInterrupt = context->getState().getPixelLocalStorageActivePlanes();
        ASSERT(0 <= mActivePlanesAtInterrupt &&
               mActivePlanesAtInterrupt <= IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);
        if (mActivePlanesAtInterrupt != 0)
        {
            context->endPixelLocalStorageImplicit();
        }
    }
    ++mInterruptCount;
    ASSERT(mInterruptCount > 0);
}

void PixelLocalStorage::restore(Context *context)
{
    ASSERT(mInterruptCount > 0);
    --mInterruptCount;
    ASSERT(0 <= mActivePlanesAtInterrupt &&
           mActivePlanesAtInterrupt <= IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);
    if (mInterruptCount == 0 && mActivePlanesAtInterrupt >= 1)
    {
        angle::FixedVector<GLenum, IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES> loadops(
            mActivePlanesAtInterrupt);
        for (GLsizei i = 0; i < mActivePlanesAtInterrupt; ++i)
        {
            loadops[i] = mPlanes[i].isMemoryless() ? GL_DONT_CARE : GL_LOAD_OP_LOAD_ANGLE;
        }
        context->beginPixelLocalStorage(mActivePlanesAtInterrupt, loadops.data());
    }
}

namespace
{
// Implements pixel local storage with image load/store shader operations.
class PixelLocalStorageImageLoadStore : public PixelLocalStorage
{
  public:
    PixelLocalStorageImageLoadStore(const ShPixelLocalStorageOptions &plsOptions, const Caps &caps)
        : PixelLocalStorage(plsOptions, caps)
    {
        ASSERT(mPLSOptions.type == ShPixelLocalStorageType::ImageLoadStore);
    }

    // Call deleteContextObjects or onContextObjectsLost first!
    ~PixelLocalStorageImageLoadStore() override
    {
        ASSERT(mScratchFramebufferForClearing.value == 0);
    }

    void onContextObjectsLost() override
    {
        mScratchFramebufferForClearing = FramebufferID();  // Let go of GL objects.
    }

    void onDeleteContextObjects(Context *context) override
    {
        if (mScratchFramebufferForClearing.value != 0)
        {
            context->deleteFramebuffer(mScratchFramebufferForClearing);
            mScratchFramebufferForClearing = FramebufferID();
        }
    }

    void onBegin(Context *context, GLsizei n, const GLenum loadops[], Extents plsExtents) override
    {
        // Save the image bindings so we can restore them during onEnd().
        const State &state = context->getState();
        ASSERT(static_cast<size_t>(n) <= state.getImageUnits().size());
        mSavedImageBindings.clear();
        mSavedImageBindings.reserve(n);
        for (GLsizei i = 0; i < n; ++i)
        {
            mSavedImageBindings.emplace_back(state.getImageUnit(i));
        }

        Framebuffer *framebuffer = state.getDrawFramebuffer();
        if (mPLSOptions.renderPassNeedsAMDRasterOrderGroupsWorkaround)
        {
            // anglebug.com/42266263 -- Metal [[raster_order_group()]] does not work for read_write
            // textures on AMD when the render pass doesn't have a color attachment on slot 0. To
            // work around this we attach one of the PLS textures to GL_COLOR_ATTACHMENT0, if there
            // isn't one already.
            // It's important to keep the attachment enabled so that it's set in the corresponding
            // MTLRenderPassAttachmentDescriptor. As the fragment shader does not have any output
            // bound to this attachment, set the color write mask to all-disabled.
            // Note that the PLS extension disallows simultaneously binding a single texture image
            // to a PLS plane and attaching it to the draw framebuffer. Enabling this workaround on
            // any other platform would yield incorrect results.
            // This flag is set to true iff the framebuffer has an attachment 0 and it is enabled.
            mHadColorAttachment0 = framebuffer->getDrawBufferMask().test(0);
            if (!mHadColorAttachment0)
            {
                // Indexed color masks are always available on Metal.
                ASSERT(context->getExtensions().drawBuffersIndexedAny());
                // Remember the current draw buffer 0 color mask and set it to all-disabled.
                state.getBlendStateExt().getColorMaskIndexed(
                    0, &mSavedColorMask[0], &mSavedColorMask[1], &mSavedColorMask[2],
                    &mSavedColorMask[3]);
                ContextPrivateColorMaski(context->getMutablePrivateState(),
                                         context->getMutablePrivateStateCache(), 0, false, false,
                                         false, false);

                // Remember the current draw buffer state so we can restore it during onEnd().
                const DrawBuffersVector<GLenum> &appDrawBuffers =
                    framebuffer->getDrawBufferStates();
                mSavedDrawBuffers.resize(appDrawBuffers.size());
                std::copy(appDrawBuffers.begin(), appDrawBuffers.end(), mSavedDrawBuffers.begin());

                // Turn on draw buffer 0.
                if (mSavedDrawBuffers[0] != GL_COLOR_ATTACHMENT0)
                {
                    GLenum drawBuffer0   = mSavedDrawBuffers[0];
                    mSavedDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
                    context->drawBuffers(static_cast<GLsizei>(mSavedDrawBuffers.size()),
                                         mSavedDrawBuffers.data());
                    mSavedDrawBuffers[0] = drawBuffer0;
                }

                // Attach one of the PLS textures to GL_COLOR_ATTACHMENT0.
                getPlane(0).attachToDrawFramebuffer(context, GL_COLOR_ATTACHMENT0);
            }
        }
        else
        {
            // Save the default framebuffer width/height so we can restore it during onEnd().
            mSavedFramebufferDefaultWidth  = framebuffer->getDefaultWidth();
            mSavedFramebufferDefaultHeight = framebuffer->getDefaultHeight();

            // Specify the framebuffer width/height explicitly in case we end up rendering
            // exclusively to shader images.
            context->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
                                           plsExtents.width);
            context->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,
                                           plsExtents.height);
        }

        // Guard GL state and bind a scratch framebuffer in case we need to reallocate or clear any
        // PLS planes.
        const size_t maxDrawBuffers = context->getCaps().maxDrawBuffers;
        ScopedRestoreDrawFramebuffer ScopedRestoreDrawFramebuffer(context);
        if (mScratchFramebufferForClearing.value == 0)
        {
            context->genFramebuffers(1, &mScratchFramebufferForClearing);
            context->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mScratchFramebufferForClearing);
            // Turn on all draw buffers on the scratch framebuffer for clearing.
            DrawBuffersVector<GLenum> drawBuffers(maxDrawBuffers);
            std::iota(drawBuffers.begin(), drawBuffers.end(), GL_COLOR_ATTACHMENT0);
            context->drawBuffers(static_cast<int>(drawBuffers.size()), drawBuffers.data());
        }
        else
        {
            context->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mScratchFramebufferForClearing);
        }
        ScopedDisableScissor scopedDisableScissor(context);

        // Bind and clear the PLS planes.
        size_t maxClearedAttachments = 0;
        for (GLsizei i = 0; i < n;)
        {
            DrawBuffersVector<int> pendingClears;
            for (; pendingClears.size() < maxDrawBuffers && i < n; ++i)
            {
                GLenum loadop                       = loadops[i];
                const PixelLocalStoragePlane &plane = getPlane(i);
                plane.bindToImage(context, i, !mPLSOptions.supportsNativeRGBA8ImageFormats);
                if (loadop == GL_LOAD_OP_ZERO_ANGLE || loadop == GL_LOAD_OP_CLEAR_ANGLE)
                {
                    plane.attachToDrawFramebuffer(
                        context, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(pendingClears.size()));
                    pendingClears.push_back(i);  // Defer the clear for later.
                }
            }
            // Clear in batches in order to be more efficient with GL state.
            ScopedEnableColorMask scopedEnableColorMask(context,
                                                        static_cast<int>(pendingClears.size()));
            ClearBufferCommands clearBufferCommands(context);
            for (size_t drawBufferIdx = 0; drawBufferIdx < pendingClears.size(); ++drawBufferIdx)
            {
                int plsIdx = pendingClears[drawBufferIdx];
                getPlane(plsIdx).issueClearCommand(
                    &clearBufferCommands, static_cast<int>(drawBufferIdx), loadops[plsIdx]);
            }
            maxClearedAttachments = std::max(maxClearedAttachments, pendingClears.size());
        }

        // Detach the cleared PLS textures from the scratch framebuffer.
        for (size_t i = 0; i < maxClearedAttachments; ++i)
        {
            context->framebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                                          GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                                          TextureTarget::_2D, TextureID(), 0);
        }

        // Unlike other barriers, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT also synchronizes all types of
        // memory accesses that happened before the barrier:
        //
        //   SHADER_IMAGE_ACCESS_BARRIER_BIT: Memory accesses using shader built-in image load and
        //   store functions issued after the barrier will reflect data written by shaders prior to
        //   the barrier. Additionally, image stores issued after the barrier will not execute until
        //   all memory accesses (e.g., loads, stores, texture fetches, vertex fetches) initiated
        //   prior to the barrier complete.
        //
        // So we don't any barriers other than GL_SHADER_IMAGE_ACCESS_BARRIER_BIT during begin().
        context->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void onEnd(Context *context, GLsizei n, const GLenum storeops[]) override
    {
        // Restore the image bindings. Since glBindImageTexture and any commands that modify
        // textures are banned while PLS is active, these will all still be alive and valid.
        ASSERT(mSavedImageBindings.size() == static_cast<size_t>(n));
        for (GLuint unit = 0; unit < mSavedImageBindings.size(); ++unit)
        {
            ImageUnit &binding = mSavedImageBindings[unit];
            context->bindImageTexture(unit, binding.texture.id(), binding.level, binding.layered,
                                      binding.layer, binding.access, binding.format);

            // BindingPointers have to be explicitly cleaned up.
            binding.texture.set(context, nullptr);
        }
        mSavedImageBindings.clear();

        if (mPLSOptions.renderPassNeedsAMDRasterOrderGroupsWorkaround)
        {
            if (!mHadColorAttachment0)
            {
                // Detach the PLS texture we attached to GL_COLOR_ATTACHMENT0.
                context->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              TextureTarget::_2D, TextureID(), 0);

                // Restore the draw buffer state from before PLS was enabled.
                if (mSavedDrawBuffers[0] != GL_COLOR_ATTACHMENT0)
                {
                    context->drawBuffers(static_cast<GLsizei>(mSavedDrawBuffers.size()),
                                         mSavedDrawBuffers.data());
                }
                mSavedDrawBuffers.clear();

                // Restore the draw buffer 0 color mask.
                ContextPrivateColorMaski(
                    context->getMutablePrivateState(), context->getMutablePrivateStateCache(), 0,
                    mSavedColorMask[0], mSavedColorMask[1], mSavedColorMask[2], mSavedColorMask[3]);
            }
        }
        else
        {
            // Restore the default framebuffer width/height.
            context->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
                                           mSavedFramebufferDefaultWidth);
            context->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,
                                           mSavedFramebufferDefaultHeight);
        }

        // We need ALL_BARRIER_BITS during end() because GL_SHADER_IMAGE_ACCESS_BARRIER_BIT doesn't
        // synchronize all types of memory accesses that can happen after the barrier.
        context->memoryBarrier(GL_ALL_BARRIER_BITS);
    }

    void onBarrier(Context *context) override
    {
        context->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

  private:
    // D3D and ES require us to pack all PLS formats into r32f, r32i, or r32ui images.
    FramebufferID mScratchFramebufferForClearing{};

    // Saved values to restore during onEnd().
    std::vector<ImageUnit> mSavedImageBindings;
    // If mPLSOptions.plsRenderPassNeedsColorAttachmentWorkaround.
    bool mHadColorAttachment0;
    std::array<bool, 4> mSavedColorMask;
    DrawBuffersVector<GLenum> mSavedDrawBuffers;
    // If !mPLSOptions.plsRenderPassNeedsColorAttachmentWorkaround.
    GLint mSavedFramebufferDefaultWidth;
    GLint mSavedFramebufferDefaultHeight;
};

// Implements pixel local storage via framebuffer fetch.
class PixelLocalStorageFramebufferFetch : public PixelLocalStorage
{
  public:
    PixelLocalStorageFramebufferFetch(const ShPixelLocalStorageOptions &plsOptions,
                                      const Caps &caps)
        : PixelLocalStorage(plsOptions, caps)
    {
        ASSERT(mPLSOptions.type == ShPixelLocalStorageType::FramebufferFetch);
    }

    void onContextObjectsLost() override {}

    void onDeleteContextObjects(Context *) override {}

    void onBegin(Context *context, GLsizei n, const GLenum loadops[], Extents plsExtents) override
    {
        const Caps &caps                                = context->getCaps();
        Framebuffer *framebuffer                        = context->getState().getDrawFramebuffer();
        const DrawBuffersVector<GLenum> &appDrawBuffers = framebuffer->getDrawBufferStates();

        // Remember the current draw buffer state so we can restore it during onEnd().
        mSavedDrawBuffers.resize(appDrawBuffers.size());
        std::copy(appDrawBuffers.begin(), appDrawBuffers.end(), mSavedDrawBuffers.begin());

        // Set up new draw buffers for PLS.
        int firstPLSDrawBuffer = caps.maxCombinedDrawBuffersAndPixelLocalStoragePlanes - n;
        int numAppDrawBuffers =
            std::min(static_cast<int>(appDrawBuffers.size()), firstPLSDrawBuffer);
        DrawBuffersArray<GLenum> plsDrawBuffers;
        std::copy(appDrawBuffers.begin(), appDrawBuffers.begin() + numAppDrawBuffers,
                  plsDrawBuffers.begin());
        std::fill(plsDrawBuffers.begin() + numAppDrawBuffers,
                  plsDrawBuffers.begin() + firstPLSDrawBuffer, GL_NONE);

        bool needsClear = false;
        for (GLsizei i = 0; i < n; ++i)
        {
            GLuint drawBufferIdx                = GetDrawBufferIdx(caps, i);
            GLenum loadop                       = loadops[i];
            const PixelLocalStoragePlane &plane = getPlane(i);
            ASSERT(!plane.isDeinitialized());

            // Attach our PLS texture to the framebuffer. Validation should have already ensured
            // nothing else was attached at this point.
            GLenum colorAttachment = GL_COLOR_ATTACHMENT0 + drawBufferIdx;
            ASSERT(!framebuffer->getAttachment(context, colorAttachment));
            plane.attachToDrawFramebuffer(context, colorAttachment);
            plsDrawBuffers[drawBufferIdx] = colorAttachment;

            needsClear = needsClear || (loadop != GL_LOAD_OP_LOAD_ANGLE);
        }

        // Turn on the PLS draw buffers.
        context->drawBuffers(caps.maxCombinedDrawBuffersAndPixelLocalStoragePlanes,
                             plsDrawBuffers.data());

        // Clear the non-LOAD_OP_LOAD PLS planes now that their draw buffers are turned on.
        if (needsClear)
        {
            ScopedDisableScissor scopedDisableScissor(context);
            ClearBufferCommands clearBufferCommands(context);
            for (GLsizei i = 0; i < n; ++i)
            {
                GLenum loadop = loadops[i];
                if (loadop != GL_LOAD_OP_LOAD_ANGLE)
                {
                    GLuint drawBufferIdx = GetDrawBufferIdx(caps, i);
                    getPlane(i).issueClearCommand(&clearBufferCommands, drawBufferIdx, loadop);
                }
            }
        }

        if (!context->getExtensions().shaderPixelLocalStorageCoherentANGLE)
        {
            // Insert a barrier if we aren't coherent, since the textures may have been rendered to
            // previously.
            barrier(context);
        }
    }

    void onEnd(Context *context, GLsizei n, const GLenum storeops[]) override
    {
        const Caps &caps = context->getCaps();

        // Invalidate the non-preserved PLS attachments.
        DrawBuffersVector<GLenum> invalidateList;
        for (GLsizei i = n - 1; i >= 0; --i)
        {
            if (!getPlane(i).isActive())
            {
                continue;
            }
            if (storeops[i] != GL_STORE_OP_STORE_ANGLE || getPlane(i).isMemoryless())
            {
                int drawBufferIdx = GetDrawBufferIdx(caps, i);
                invalidateList.push_back(GL_COLOR_ATTACHMENT0 + drawBufferIdx);
            }
        }
        if (!invalidateList.empty())
        {
            context->invalidateFramebuffer(GL_DRAW_FRAMEBUFFER,
                                           static_cast<GLsizei>(invalidateList.size()),
                                           invalidateList.data());
        }

        for (GLsizei i = 0; i < n; ++i)
        {
            // Reset color attachments where PLS was attached. Validation should have already
            // ensured nothing was attached at these points when we activated pixel local storage,
            // and that nothing got attached during.
            GLuint drawBufferIdx   = GetDrawBufferIdx(caps, i);
            GLenum colorAttachment = GL_COLOR_ATTACHMENT0 + drawBufferIdx;
            context->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, colorAttachment, TextureTarget::_2D,
                                          TextureID(), 0);
        }

        // Restore the draw buffer state from before PLS was enabled.
        context->drawBuffers(static_cast<GLsizei>(mSavedDrawBuffers.size()),
                             mSavedDrawBuffers.data());
        mSavedDrawBuffers.clear();
    }

    void onBarrier(Context *context) override { context->framebufferFetchBarrier(); }

  private:
    static GLuint GetDrawBufferIdx(const Caps &caps, GLuint plsPlaneIdx)
    {
        // Bind the PLS attachments in reverse order from the rear. This way, the shader translator
        // doesn't need to know how many planes are going to be active in order to figure out plane
        // indices.
        return caps.maxCombinedDrawBuffersAndPixelLocalStoragePlanes - plsPlaneIdx - 1;
    }

    DrawBuffersVector<GLenum> mSavedDrawBuffers;
};

}  // namespace

std::unique_ptr<PixelLocalStorage> PixelLocalStorage::Make(const Context *context)
{
    const ShPixelLocalStorageOptions &plsOptions =
        context->getImplementation()->getNativePixelLocalStorageOptions();
    const Caps &caps = context->getState().getCaps();
    switch (plsOptions.type)
    {
        case ShPixelLocalStorageType::ImageLoadStore:
            return std::make_unique<PixelLocalStorageImageLoadStore>(plsOptions, caps);
        case ShPixelLocalStorageType::FramebufferFetch:
            return std::make_unique<PixelLocalStorageFramebufferFetch>(plsOptions, caps);
        default:
            UNREACHABLE();
            return nullptr;
    }
}
}  // namespace gl
