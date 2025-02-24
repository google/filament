//
//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Context.h: Defines the gl::Context class, managing all GL state and performing
// rendering operations. It is the GLES2 specific implementation of EGLContext.

#ifndef LIBANGLE_CONTEXT_H_
#define LIBANGLE_CONTEXT_H_

#include <mutex>
#include <set>
#include <string>

#include "angle_gl.h"
#include "common/MemoryBuffer.h"
#include "common/PackedEnums.h"
#include "common/SimpleMutex.h"
#include "common/angleutils.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Context_gles_1_0_autogen.h"
#include "libANGLE/Context_gles_2_0_autogen.h"
#include "libANGLE/Context_gles_3_0_autogen.h"
#include "libANGLE/Context_gles_3_1_autogen.h"
#include "libANGLE/Context_gles_3_2_autogen.h"
#include "libANGLE/Context_gles_ext_autogen.h"
#include "libANGLE/Error.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/HandleAllocator.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/ResourceMap.h"
#include "libANGLE/State.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/angletypes.h"

namespace angle
{
class Closure;
class FrameCapture;
class FrameCaptureShared;
struct FrontendFeatures;
class WaitableEvent;
}  // namespace angle

namespace rx
{
class ContextImpl;
class EGLImplFactory;
}  // namespace rx

namespace egl
{
class AttributeMap;
class Surface;
struct Config;
class Thread;
}  // namespace egl

namespace gl
{
class Buffer;
class Compiler;
class FenceNV;
class GLES1Renderer;
class MemoryProgramCache;
class MemoryShaderCache;
class MemoryObject;
class PixelLocalStoragePlane;
class Program;
class ProgramPipeline;
class Query;
class Renderbuffer;
class Sampler;
class Semaphore;
class Shader;
class Sync;
class Texture;
class TransformFeedback;
class VertexArray;
struct VertexAttribute;

class ErrorSet : angle::NonCopyable
{
  public:
    explicit ErrorSet(Debug *debug,
                      const angle::FrontendFeatures &frontendFeatures,
                      const egl::AttributeMap &attribs);
    ~ErrorSet();

    bool empty() const { return mHasAnyErrors.load(std::memory_order_relaxed) == 0; }
    GLenum popError();

    void handleError(GLenum errorCode,
                     const char *message,
                     const char *file,
                     const char *function,
                     unsigned int line);

    void validationError(angle::EntryPoint entryPoint, GLenum errorCode, const char *message);
    ANGLE_FORMAT_PRINTF(4, 5)
    void validationErrorF(angle::EntryPoint entryPoint, GLenum errorCode, const char *format, ...);

    bool skipValidation() const
    {
        // Ensure we don't skip validation when context becomes lost, since implementations
        // generally assume a non-lost context, non-null objects, etc.
        ASSERT(!isContextLost() || !mSkipValidation);
        return mSkipValidation.load(std::memory_order_relaxed) != 0;
    }
    void forceValidation() { mSkipValidation = 0; }

    void markContextLost(GraphicsResetStatus status);
    bool isContextLost() const { return mContextLost.load(std::memory_order_relaxed) != 0; }
    GLenum getGraphicsResetStatus(rx::ContextImpl *contextImpl);
    GLenum getResetStrategy() const { return mResetStrategy; }
    GLenum getErrorForCapture() const;

  private:
    void setContextLost();
    void pushError(GLenum errorCode);
    std::unique_lock<std::mutex> getLockIfNotAlready();

    // Non-atomic members of this class are protected by a mutex.  This is to allow errors to be
    // safely set by entry points that don't hold a lock.  Note that other contexts may end up
    // triggering an error on this context (through making failable calls on other contexts in the
    // share group).
    //
    // Note also that the functionality used through the Debug class is thread-safe.
    std::mutex mMutex;

    // Error handling and reporting
    Debug *mDebug;
    std::set<GLenum> mErrors;

    const GLenum mResetStrategy;
    const bool mLoseContextOnOutOfMemory;

    // Context-loss handling
    bool mContextLostForced;
    GraphicsResetStatus mResetStatus;

    // The following are atomic and lockless as they are very frequently accessed.
    std::atomic_int mSkipValidation;
    std::atomic_int mContextLost;
    std::atomic_int mHasAnyErrors;
};

enum class VertexAttribTypeCase
{
    Invalid        = 0,
    Valid          = 1,
    ValidSize4Only = 2,
    ValidSize3or4  = 3,
};

// Part of StateCache (see below) that is private to the context and is inaccessible to other
// contexts.
class PrivateStateCache final : angle::NonCopyable
{
  public:
    PrivateStateCache();
    ~PrivateStateCache();

    void onCapChange() { mIsCachedBasicDrawStatesErrorValid = false; }
    void onColorMaskChange() { mIsCachedBasicDrawStatesErrorValid = false; }
    void onDefaultVertexAttributeChange() { mIsCachedBasicDrawStatesErrorValid = false; }

    // Blending updates invalidate draw
    // state in the following cases:
    //
    // * Blend equations have been changed and the context
    //   supports KHR_blend_equation_advanced. The number
    //   of enabled draw buffers may need to be checked
    //   to not be greater than 1.
    //
    // * Blend funcs have been changed with indexed
    //   commands. The D3D11 backend cannot support
    //   constant color and alpha blend funcs together
    //   so a check is needed across all draw buffers.
    //
    // * Blend funcs have been changed and the context
    //   supports EXT_blend_func_extended. The number
    //   of enabled draw buffers may need to be checked
    //   against MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT limit.
    void onBlendEquationOrFuncChange() { mIsCachedBasicDrawStatesErrorValid = false; }

    void onStencilStateChange() { mIsCachedBasicDrawStatesErrorValid = false; }

    bool isCachedBasicDrawStatesErrorValid() const { return mIsCachedBasicDrawStatesErrorValid; }
    void setCachedBasicDrawStatesErrorValid() const { mIsCachedBasicDrawStatesErrorValid = true; }

  private:
    // StateCache::mCachedBasicDrawStatesError* may be invalidated through numerous calls (see the
    // comment on getBasicDrawStatesErrorString), some of which may originate from other contexts
    // (through the observer interface).  However, ContextPrivate* helpers may also need to
    // invalidate the draw states, but they are called without holding the share group lock.  The
    // following tracks whether StateCache::mCachedBasicDrawStatesError* values are valid and is
    // accessed only by the context itself.
    mutable bool mIsCachedBasicDrawStatesErrorValid;
};

// Helper class for managing cache variables and state changes.
class StateCache final : angle::NonCopyable
{
  public:
    StateCache();
    ~StateCache();

    void initialize(Context *context);

    // Places that can trigger updateActiveAttribsMask:
    // 1. onVertexArrayBindingChange.
    // 2. onProgramExecutableChange.
    // 3. onVertexArrayStateChange.
    // 4. onGLES1ClientStateChange.
    // 5. onGLES1TextureStateChange.
    AttributesMask getActiveBufferedAttribsMask() const { return mCachedActiveBufferedAttribsMask; }
    AttributesMask getActiveClientAttribsMask() const { return mCachedActiveClientAttribsMask; }
    AttributesMask getActiveDefaultAttribsMask() const { return mCachedActiveDefaultAttribsMask; }
    bool hasAnyEnabledClientAttrib() const { return mCachedHasAnyEnabledClientAttrib; }
    bool hasAnyActiveClientAttrib() const { return mCachedActiveClientAttribsMask.any(); }

    // Places that can trigger updateVertexElementLimits:
    // 1. onVertexArrayBindingChange.
    // 2. onProgramExecutableChange.
    // 3. onVertexArrayFormatChange.
    // 4. onVertexArrayBufferChange.
    // 5. onVertexArrayStateChange.
    GLint64 getNonInstancedVertexElementLimit() const
    {
        return mCachedNonInstancedVertexElementLimit;
    }
    GLint64 getInstancedVertexElementLimit() const { return mCachedInstancedVertexElementLimit; }

    // Places that can trigger updateBasicDrawStatesError:
    // 1. onVertexArrayBindingChange.
    // 2. onProgramExecutableChange.
    // 3. onVertexArrayBufferContentsChange.
    // 4. onVertexArrayStateChange.
    // 5. onVertexArrayBufferStateChange.
    // 6. onDrawFramebufferChange.
    // 7. onActiveTextureChange.
    // 8. onQueryChange.
    // 9. onActiveTransformFeedbackChange.
    // 10. onUniformBufferStateChange.
    // 11. onBufferBindingChange.
    //
    // Additionally, the following in PrivateStateCache can lead to updateBasicDrawStatesError:
    // 1. onCapChange.
    // 2. onStencilStateChange.
    // 3. onDefaultVertexAttributeChange.
    // 4. onColorMaskChange.
    // 5. onBlendEquationOrFuncChange.
    intptr_t getBasicDrawStatesErrorString(const Context *context,
                                           const PrivateStateCache *privateStateCache) const
    {
        // This is only ever called with the context that owns this state cache
        ASSERT(isCurrentContext(context, privateStateCache));
        if (privateStateCache->isCachedBasicDrawStatesErrorValid() &&
            mCachedBasicDrawStatesErrorString != kInvalidPointer)
        {
            return mCachedBasicDrawStatesErrorString;
        }

        return getBasicDrawStatesErrorImpl(context, privateStateCache);
    }

    // The GL error enum to use when generating errors due to failed draw states. Only valid if
    // getBasicDrawStatesErrorString returns non-zero.
    GLenum getBasicDrawElementsErrorCode() const
    {
        ASSERT(mCachedBasicDrawStatesErrorString != kInvalidPointer);
        ASSERT(mCachedBasicDrawStatesErrorCode != GL_NO_ERROR);
        return mCachedBasicDrawStatesErrorCode;
    }

    // Places that can trigger updateProgramPipelineError:
    // 1. onProgramExecutableChange.
    intptr_t getProgramPipelineError(const Context *context) const
    {
        if (mCachedProgramPipelineError != kInvalidPointer)
        {
            return mCachedProgramPipelineError;
        }

        return getProgramPipelineErrorImpl(context);
    }

    // Places that can trigger updateBasicDrawElementsError:
    // 1. onActiveTransformFeedbackChange.
    // 2. onVertexArrayBufferStateChange.
    // 3. onBufferBindingChange.
    // 4. onVertexArrayStateChange.
    // 5. onVertexArrayBindingStateChange.
    intptr_t getBasicDrawElementsError(const Context *context) const
    {
        if (mCachedBasicDrawElementsError != kInvalidPointer)
        {
            return mCachedBasicDrawElementsError;
        }

        return getBasicDrawElementsErrorImpl(context);
    }

    // Places that can trigger updateValidDrawModes:
    // 1. onProgramExecutableChange.
    // 2. onActiveTransformFeedbackChange.
    bool isValidDrawMode(PrimitiveMode primitiveMode) const
    {
        return mCachedValidDrawModes[primitiveMode];
    }

    // Cannot change except on Context/Extension init.
    bool isValidBindTextureType(TextureType type) const
    {
        return mCachedValidBindTextureTypes[type];
    }

    // Cannot change except on Context/Extension init.
    bool isValidDrawElementsType(DrawElementsType type) const
    {
        return mCachedValidDrawElementsTypes[type];
    }

    // Places that can trigger updateTransformFeedbackActiveUnpaused:
    // 1. onActiveTransformFeedbackChange.
    bool isTransformFeedbackActiveUnpaused() const
    {
        return mCachedTransformFeedbackActiveUnpaused;
    }

    // Cannot change except on Context/Extension init.
    VertexAttribTypeCase getVertexAttribTypeValidation(VertexAttribType type) const
    {
        return mCachedVertexAttribTypesValidation[type];
    }

    VertexAttribTypeCase getIntegerVertexAttribTypeValidation(VertexAttribType type) const
    {
        return mCachedIntegerVertexAttribTypesValidation[type];
    }

    // Places that can trigger updateActiveShaderStorageBufferIndices:
    // 1. onProgramExecutableChange.
    StorageBuffersMask getActiveShaderStorageBufferIndices() const
    {
        return mCachedActiveShaderStorageBufferIndices;
    }

    // Places that can trigger updateActiveImageUnitIndices:
    // 1. onProgramExecutableChange.
    const ImageUnitMask &getActiveImageUnitIndices() const { return mCachedActiveImageUnitIndices; }

    // Places that can trigger updateCanDraw:
    // 1. onProgramExecutableChange.
    bool getCanDraw() const { return mCachedCanDraw; }

    // State change notifications.
    void onVertexArrayBindingChange(Context *context);
    void onProgramExecutableChange(Context *context);
    void onVertexArrayFormatChange(Context *context);
    void onVertexArrayBufferContentsChange(Context *context);
    void onVertexArrayStateChange(Context *context);
    void onVertexArrayBufferStateChange(Context *context);
    void onGLES1TextureStateChange(Context *context);
    void onGLES1ClientStateChange(Context *context);
    void onDrawFramebufferChange(Context *context);
    void onActiveTextureChange(Context *context);
    void onQueryChange(Context *context);
    void onActiveTransformFeedbackChange(Context *context);
    void onUniformBufferStateChange(Context *context);
    void onAtomicCounterBufferStateChange(Context *context);
    void onShaderStorageBufferStateChange(Context *context);
    void onBufferBindingChange(Context *context);

  private:
    bool isCurrentContext(const Context *context, const PrivateStateCache *privateStateCache) const;

    // Cache update functions.
    void updateActiveAttribsMask(Context *context);
    void updateVertexElementLimits(Context *context);
    void updateVertexElementLimitsImpl(Context *context);
    void updateValidDrawModes(Context *context);
    void updateValidBindTextureTypes(Context *context);
    void updateValidDrawElementsTypes(Context *context);
    void updateBasicDrawStatesError()
    {
        mCachedBasicDrawStatesErrorString = kInvalidPointer;
        mCachedBasicDrawStatesErrorCode   = GL_NO_ERROR;
    }
    void updateProgramPipelineError() { mCachedProgramPipelineError = kInvalidPointer; }
    void updateBasicDrawElementsError() { mCachedBasicDrawElementsError = kInvalidPointer; }
    void updateTransformFeedbackActiveUnpaused(Context *context);
    void updateVertexAttribTypesValidation(Context *context);
    void updateActiveShaderStorageBufferIndices(Context *context);
    void updateActiveImageUnitIndices(Context *context);
    void updateCanDraw(Context *context);

    void setValidDrawModes(bool pointsOK,
                           bool linesOK,
                           bool trisOK,
                           bool lineAdjOK,
                           bool triAdjOK,
                           bool patchOK);

    intptr_t getBasicDrawStatesErrorImpl(const Context *context,
                                         const PrivateStateCache *privateStateCache) const;
    intptr_t getProgramPipelineErrorImpl(const Context *context) const;
    intptr_t getBasicDrawElementsErrorImpl(const Context *context) const;

    static constexpr intptr_t kInvalidPointer = 1;

    AttributesMask mCachedActiveBufferedAttribsMask;
    AttributesMask mCachedActiveClientAttribsMask;
    AttributesMask mCachedActiveDefaultAttribsMask;

    // Given a vertex attribute's stride, the corresponding vertex buffer can fit a number of such
    // attributes.  A draw call that attempts to use more vertex attributes thus needs to fail (when
    // robust access is enabled).  The following variables help implement this limit given the
    // following situations:
    //
    // Assume:
    //
    // Ni = Number of vertex attributes that can fit in buffer bound to attribute i.
    // Di = Vertex attribute divisor set for attribute i.
    // F = Draw calls "first" vertex index
    // C = Draw calls vertex "count"
    // B = Instanced draw calls "baseinstance"
    // P = Instanced draw calls "primcount"
    //
    // Then, for each attribute i:
    //
    //   If Di == 0 (i.e. non-instanced)
    //     Vertices [F, F+C) are accessed
    //     Draw call should fail if F+C > Ni
    //
    //   If Di != 0 (i.e. instanced), in a non-instanced draw call:
    //     Only vertex 0 is accessed - note that a non-zero divisor in a non-instanced draw call
    //       implies that F is ignored and the vertex index is not incremented.
    //     Draw call should fail if Ni < 1
    //
    //   If Di != 0, in an instanced draw call:
    //     Vertices [B, B+ceil(P/Di)) are accessed
    //     Draw call should fail if B+ceil(P/Di) > Ni
    //
    // To avoid needing to iterate over all attributes in the hot paths, the following is
    // calculated:
    //
    // Non-instanced limit: min(Ni) for all non-instanced attributes.  At draw time F+C <= min(Ni)
    // is validated.
    // Instanced limit: min(Ni*Di) for all instanced attributes.  At draw time, B+P <= min(Ni*Di) is
    // validated (the math works out, try with an example!)
    //
    // For instanced attributes in a non-instanced draw call, need to check that min(Ni) > 0.
    // Evaluating min(Ni*DI) > 0 produces the same result though, so the instanced limit is used
    // there too.
    //
    // If there are no instanced attributes, the non-instanced limit is set to infinity.  If there
    // are no instanced attributes, the instanced limits are set to infinity.
    GLint64 mCachedNonInstancedVertexElementLimit;
    GLint64 mCachedInstancedVertexElementLimit;

    mutable intptr_t mCachedBasicDrawStatesErrorString;
    mutable GLenum mCachedBasicDrawStatesErrorCode;
    mutable intptr_t mCachedBasicDrawElementsError;
    // mCachedProgramPipelineError checks only the
    // current-program-exists subset of mCachedBasicDrawStatesError.
    // Therefore, mCachedProgramPipelineError follows
    // mCachedBasicDrawStatesError in that if mCachedBasicDrawStatesError is
    // no-error, so is mCachedProgramPipelineError.  Otherwise, if
    // mCachedBasicDrawStatesError is in error, the state of
    // mCachedProgramPipelineError can be no-error or also in error, or
    // unknown due to early exiting.
    mutable intptr_t mCachedProgramPipelineError;
    bool mCachedHasAnyEnabledClientAttrib;
    bool mCachedTransformFeedbackActiveUnpaused;
    StorageBuffersMask mCachedActiveShaderStorageBufferIndices;
    ImageUnitMask mCachedActiveImageUnitIndices;

    // Reserve an extra slot at the end of these maps for invalid enum.
    angle::PackedEnumMap<PrimitiveMode, bool, angle::EnumSize<PrimitiveMode>() + 1>
        mCachedValidDrawModes;
    angle::PackedEnumMap<TextureType, bool, angle::EnumSize<TextureType>() + 1>
        mCachedValidBindTextureTypes;
    angle::PackedEnumMap<DrawElementsType, bool, angle::EnumSize<DrawElementsType>() + 1>
        mCachedValidDrawElementsTypes;
    angle::PackedEnumMap<VertexAttribType,
                         VertexAttribTypeCase,
                         angle::EnumSize<VertexAttribType>() + 1>
        mCachedVertexAttribTypesValidation;
    angle::PackedEnumMap<VertexAttribType,
                         VertexAttribTypeCase,
                         angle::EnumSize<VertexAttribType>() + 1>
        mCachedIntegerVertexAttribTypesValidation;

    bool mCachedCanDraw;
};

using VertexArrayMap       = ResourceMap<VertexArray, VertexArrayID>;
using QueryMap             = ResourceMap<Query, QueryID>;
using TransformFeedbackMap = ResourceMap<TransformFeedback, TransformFeedbackID>;

class Context final : public egl::LabeledObject, angle::NonCopyable, public angle::ObserverInterface
{
  public:
    Context(egl::Display *display,
            const egl::Config *config,
            const Context *shareContext,
            TextureManager *shareTextures,
            SemaphoreManager *shareSemaphores,
            egl::ContextMutex *sharedContextMutex,
            MemoryProgramCache *memoryProgramCache,
            MemoryShaderCache *memoryShaderCache,
            const egl::AttributeMap &attribs,
            const egl::DisplayExtensions &displayExtensions,
            const egl::ClientExtensions &clientExtensions);

    // Use for debugging.
    ContextID id() const { return mState.getContextID(); }

    egl::Error initialize();

    egl::Error onDestroy(const egl::Display *display);
    ~Context() override;

    void setLabel(EGLLabelKHR label) override;
    EGLLabelKHR getLabel() const override;

    egl::Error makeCurrent(egl::Display *display,
                           egl::Surface *drawSurface,
                           egl::Surface *readSurface);
    egl::Error unMakeCurrent(const egl::Display *display);

    // These create and destroy methods pass through to ResourceManager, which owns these objects.
    BufferID createBuffer();
    TextureID createTexture();
    RenderbufferID createRenderbuffer();
    ProgramPipelineID createProgramPipeline();
    MemoryObjectID createMemoryObject();
    SemaphoreID createSemaphore();

    void deleteBuffer(BufferID buffer);
    void deleteTexture(TextureID texture);
    void deleteRenderbuffer(RenderbufferID renderbuffer);
    void deleteProgramPipeline(ProgramPipelineID pipeline);
    void deleteMemoryObject(MemoryObjectID memoryObject);
    void deleteSemaphore(SemaphoreID semaphore);

    void bindReadFramebuffer(FramebufferID framebufferHandle);
    void bindDrawFramebuffer(FramebufferID framebufferHandle);

    Buffer *getBuffer(BufferID handle) const;
    FenceNV *getFenceNV(FenceNVID handle) const;
    Sync *getSync(SyncID syncPacked) const;
    ANGLE_INLINE Texture *getTexture(TextureID handle) const
    {
        return mState.mTextureManager->getTexture(handle);
    }

    Framebuffer *getFramebuffer(FramebufferID handle) const;
    Renderbuffer *getRenderbuffer(RenderbufferID handle) const;
    VertexArray *getVertexArray(VertexArrayID handle) const;
    Sampler *getSampler(SamplerID handle) const;
    Query *getOrCreateQuery(QueryID handle, QueryType type);
    Query *getQuery(QueryID handle) const;
    TransformFeedback *getTransformFeedback(TransformFeedbackID handle) const;
    ProgramPipeline *getProgramPipeline(ProgramPipelineID handle) const;
    MemoryObject *getMemoryObject(MemoryObjectID handle) const;
    Semaphore *getSemaphore(SemaphoreID handle) const;

    Texture *getTextureByType(TextureType type) const;
    Texture *getTextureByTarget(TextureTarget target) const;
    Texture *getSamplerTexture(unsigned int sampler, TextureType type) const;

    Compiler *getCompiler() const;

    bool isVertexArrayGenerated(VertexArrayID vertexArray) const;
    bool isTransformFeedbackGenerated(TransformFeedbackID transformFeedback) const;

    bool isExternal() const { return mState.isExternal(); }

    void getBooleanvImpl(GLenum pname, GLboolean *params) const;
    void getFloatvImpl(GLenum pname, GLfloat *params) const;
    void getIntegervImpl(GLenum pname, GLint *params) const;
    void getInteger64vImpl(GLenum pname, GLint64 *params) const;
    void getIntegerVertexAttribImpl(GLenum pname, GLenum attribpname, GLint *params) const;
    void getVertexAttribivImpl(GLuint index, GLenum pname, GLint *params) const;

    // Framebuffers are owned by the Context, so these methods do not pass through
    FramebufferID createFramebuffer();
    void deleteFramebuffer(FramebufferID framebuffer);

    bool hasActiveTransformFeedback(ShaderProgramID program) const;

    // GLES entry point interface
    ANGLE_GLES_1_0_CONTEXT_API
    ANGLE_GLES_2_0_CONTEXT_API
    ANGLE_GLES_3_0_CONTEXT_API
    ANGLE_GLES_3_1_CONTEXT_API
    ANGLE_GLES_3_2_CONTEXT_API
    ANGLE_GLES_EXT_CONTEXT_API

    angle::Result handleNoopDrawEvent();

    // Consumes an error.
    void handleError(GLenum errorCode,
                     const char *message,
                     const char *file,
                     const char *function,
                     unsigned int line);

    bool isResetNotificationEnabled() const;

    bool isRobustnessEnabled() const { return mState.hasRobustAccess(); }

    const egl::Config *getConfig() const { return mConfig; }
    EGLenum getRenderBuffer() const;
    EGLenum getContextPriority() const;

    const GLubyte *getString(GLenum name) const;
    const GLubyte *getStringi(GLenum name, GLuint index) const;

    size_t getExtensionStringCount() const;

    bool isExtensionRequestable(const char *name) const;
    bool isExtensionDisablable(const char *name) const;
    size_t getRequestableExtensionStringCount() const;
    void setExtensionEnabled(const char *name, bool enabled);
    void reinitializeAfterExtensionsChanged();

    rx::ContextImpl *getImplementation() const { return mImplementation.get(); }

    [[nodiscard]] bool getScratchBuffer(size_t requestedSizeBytes,
                                        angle::MemoryBuffer **scratchBufferOut) const;
    [[nodiscard]] bool getZeroFilledBuffer(size_t requstedSizeBytes,
                                           angle::MemoryBuffer **zeroBufferOut) const;
    angle::ScratchBuffer *getScratchBuffer() const;

    angle::Result prepareForCopyImage();
    angle::Result prepareForDispatch();
    angle::Result prepareForInvalidate(GLenum target);

    MemoryProgramCache *getMemoryProgramCache() const { return mMemoryProgramCache; }
    MemoryShaderCache *getMemoryShaderCache() const { return mMemoryShaderCache; }

    angle::SimpleMutex &getProgramCacheMutex() const;

    bool hasBeenCurrent() const { return mHasBeenCurrent; }
    egl::Display *getDisplay() const { return mDisplay; }
    egl::Surface *getCurrentDrawSurface() const { return mCurrentDrawSurface; }
    egl::Surface *getCurrentReadSurface() const { return mCurrentReadSurface; }

    bool isRobustResourceInitEnabled() const { return mState.isRobustResourceInitEnabled(); }

    bool isCurrentTransformFeedback(const TransformFeedback *tf) const;

    bool isCurrentVertexArray(const VertexArray *va) const
    {
        return mState.isCurrentVertexArray(va);
    }

    ANGLE_INLINE bool isShared() const { return mShared; }
    // Once a context is setShared() it cannot be undone
    void setShared() { mShared = true; }

    const State &getState() const { return mState; }
    const PrivateState &getPrivateState() const { return mState.privateState(); }
    GLint getClientMajorVersion() const { return mState.getClientMajorVersion(); }
    GLint getClientMinorVersion() const { return mState.getClientMinorVersion(); }
    const Version &getClientVersion() const { return mState.getClientVersion(); }
    const Caps &getCaps() const { return mState.getCaps(); }
    const TextureCapsMap &getTextureCaps() const { return mState.getTextureCaps(); }
    const Extensions &getExtensions() const { return mState.getExtensions(); }
    const Limitations &getLimitations() const { return mState.getLimitations(); }
    bool isGLES1() const;

    // To be used **only** directly by the entry points.
    PrivateState *getMutablePrivateState() { return mState.getMutablePrivateState(); }
    GLES1State *getMutableGLES1State() { return mState.getMutableGLES1State(); }

    bool skipValidation() const { return mErrors.skipValidation(); }
    void markContextLost(GraphicsResetStatus status) { mErrors.markContextLost(status); }
    bool isContextLost() const { return mErrors.isContextLost(); }

    ErrorSet *getMutableErrorSetForValidation() const { return &mErrors; }

    // Specific methods needed for validation.
    bool getQueryParameterInfo(GLenum pname, GLenum *type, unsigned int *numParams) const;
    bool getIndexedQueryParameterInfo(GLenum target, GLenum *type, unsigned int *numParams) const;

    ANGLE_INLINE Program *getProgramResolveLink(ShaderProgramID handle) const
    {
        Program *program = mState.mShaderProgramManager->getProgram(handle);
        if (ANGLE_LIKELY(program))
        {
            program->resolveLink(this);
        }
        return program;
    }

    Program *getProgramNoResolveLink(ShaderProgramID handle) const;
    Shader *getShaderResolveCompile(ShaderProgramID handle) const;
    Shader *getShaderNoResolveCompile(ShaderProgramID handle) const;

    ANGLE_INLINE bool isTextureGenerated(TextureID texture) const
    {
        return mState.mTextureManager->isHandleGenerated(texture);
    }

    ANGLE_INLINE bool isBufferGenerated(BufferID buffer) const
    {
        return mState.mBufferManager->isHandleGenerated(buffer);
    }

    bool isRenderbufferGenerated(RenderbufferID renderbuffer) const;
    bool isFramebufferGenerated(FramebufferID framebuffer) const;
    bool isProgramPipelineGenerated(ProgramPipelineID pipeline) const;
    bool isQueryGenerated(QueryID query) const;

    bool usingDisplayTextureShareGroup() const;
    bool usingDisplaySemaphoreShareGroup() const;

    // Hack for the special WebGL 1 "DEPTH_STENCIL" internal format.
    GLenum getConvertedRenderbufferFormat(GLenum internalformat) const;

    bool isWebGL() const { return mState.isWebGL(); }
    bool isWebGL1() const { return mState.isWebGL1(); }
    const char *getRendererString() const { return mRendererString; }

    bool isValidBufferBinding(BufferBinding binding) const { return mValidBufferBindings[binding]; }

    // GLES1 emulation: Renderer level (for validation)
    int vertexArrayIndex(ClientVertexArrayType type) const;
    static int TexCoordArrayIndex(unsigned int unit);

    // GL_KHR_parallel_shader_compile
    std::shared_ptr<angle::WorkerThreadPool> getShaderCompileThreadPool() const;
    std::shared_ptr<angle::WorkerThreadPool> getLinkSubTaskThreadPool() const;
    std::shared_ptr<angle::WaitableEvent> postCompileLinkTask(
        const std::shared_ptr<angle::Closure> &task,
        angle::JobThreadSafety safety,
        angle::JobResultExpectancy resultExpectancy) const;

    // Single-threaded pool; runs everything instantly
    std::shared_ptr<angle::WorkerThreadPool> getSingleThreadPool() const;

    // Generic multithread pool.
    std::shared_ptr<angle::WorkerThreadPool> getWorkerThreadPool() const;

    const StateCache &getStateCache() const { return mStateCache; }
    StateCache &getStateCache() { return mStateCache; }

    const PrivateStateCache &getPrivateStateCache() const { return mPrivateStateCache; }
    PrivateStateCache *getMutablePrivateStateCache() { return &mPrivateStateCache; }

    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    void onSamplerUniformChange(size_t textureUnitIndex);

    bool isBufferAccessValidationEnabled() const { return mBufferAccessValidationEnabled; }

    const angle::FrontendFeatures &getFrontendFeatures() const;

    angle::FrameCapture *getFrameCapture() const { return mFrameCapture.get(); }

    const VertexArrayMap &getVertexArraysForCapture() const { return mVertexArrayMap; }
    const QueryMap &getQueriesForCapture() const { return mQueryMap; }
    const TransformFeedbackMap &getTransformFeedbacksForCapture() const
    {
        return mTransformFeedbackMap;
    }
    GLenum getErrorForCapture() const { return mErrors.getErrorForCapture(); }

    void onPreSwap();

    ANGLE_INLINE Program *getActiveLinkedProgram() const
    {
        Program *program = mState.getLinkedProgram(this);
        if (ANGLE_LIKELY(program))
        {
            return program;
        }
        return getActiveLinkedProgramPPO();
    }

    ANGLE_NOINLINE Program *getActiveLinkedProgramPPO() const;

    // EGL_ANGLE_power_preference implementation.
    egl::Error releaseHighPowerGPU();
    egl::Error reacquireHighPowerGPU();
    void onGPUSwitch();

    // EGL_ANGLE_external_context_and_surface implementation.
    egl::Error acquireExternalContext(egl::Surface *drawAndReadSurface);
    egl::Error releaseExternalContext();

    bool noopDraw(PrimitiveMode mode, GLsizei count) const;
    bool noopDrawInstanced(PrimitiveMode mode, GLsizei count, GLsizei instanceCount) const;
    bool noopMultiDraw(GLsizei drawcount) const;

    bool isClearBufferMaskedOut(GLenum buffer,
                                GLint drawbuffer,
                                GLuint framebufferStencilSize) const;
    bool noopClearBuffer(GLenum buffer, GLint drawbuffer) const;

    void addRef() const { mRefCount++; }
    void release() const { mRefCount--; }
    bool isReferenced() const { return mRefCount > 0; }

    egl::ShareGroup *getShareGroup() const { return mState.getShareGroup(); }

    // Warning! When need to store pointer to the mutex in other object use `getRoot()` pointer, do
    // NOT get pointer of the `getContextMutex()` reference.
    egl::ContextMutex &getContextMutex() const { return mState.mContextMutex; }

    bool supportsGeometryOrTesselation() const;
    void dirtyAllState();

    bool isDestroyed() const { return mIsDestroyed; }
    void setIsDestroyed() { mIsDestroyed = true; }

    // This function acts as glEnable(GL_COLOR_LOGIC_OP), but it's called from the GLES1 emulation
    // code to implement logicOp using the non-GLES1 functionality (i.e. GL_ANGLE_logic_op).  The
    // ContextPrivateEnable() entry point implementation cannot be used (as ContextPrivate*
    // functions are typically used by other frontend-emulated features) because it forwards this
    // back to GLES1.
    void setLogicOpEnabledForGLES1(bool enabled);

    // Needed by capture serialization logic that works with a "const" Context pointer.
    void finishImmutable() const;

    const angle::PerfMonitorCounterGroups &getPerfMonitorCounterGroups() const;

    // Ends the currently active pixel local storage session with GL_STORE_OP_STORE on all planes.
    void endPixelLocalStorageImplicit();

    bool areBlobCacheFuncsSet() const;

    size_t getMemoryUsage() const;

  private:
    void initializeDefaultResources();
    void releaseSharedObjects();

    angle::Result prepareForDraw(PrimitiveMode mode);
    angle::Result prepareForClear(GLbitfield mask);
    angle::Result prepareForClearBuffer(GLenum buffer, GLint drawbuffer);
    angle::Result syncState(const state::DirtyBits bitMask,
                            const state::ExtendedDirtyBits extendedBitMask,
                            const state::DirtyObjects &objectMask,
                            Command command);
    angle::Result syncAllDirtyBits(Command command);
    angle::Result syncDirtyBits(const state::DirtyBits bitMask,
                                const state::ExtendedDirtyBits extendedBitMask,
                                Command command);
    angle::Result syncDirtyObjects(const state::DirtyObjects &objectMask, Command command);
    angle::Result syncStateForReadPixels();
    angle::Result syncStateForTexImage();
    angle::Result syncStateForBlit(GLbitfield mask);
    angle::Result syncStateForClear();
    angle::Result syncTextureForCopy(Texture *texture);

    VertexArray *checkVertexArrayAllocation(VertexArrayID vertexArrayHandle);
    TransformFeedback *checkTransformFeedbackAllocation(TransformFeedbackID transformFeedback);

    void detachBuffer(Buffer *buffer);
    void detachTexture(TextureID texture);
    void detachFramebuffer(FramebufferID framebuffer);
    void detachRenderbuffer(RenderbufferID renderbuffer);
    void detachVertexArray(VertexArrayID vertexArray);
    void detachTransformFeedback(TransformFeedbackID transformFeedback);
    void detachSampler(SamplerID sampler);
    void detachProgramPipeline(ProgramPipelineID pipeline);

    egl::Error setDefaultFramebuffer(egl::Surface *drawSurface, egl::Surface *readSurface);
    egl::Error unsetDefaultFramebuffer();

    void initRendererString();
    void initVendorString();
    void initVersionStrings();
    void initExtensionStrings();

    Extensions generateSupportedExtensions() const;
    void initCaps();
    void updateCaps();

    gl::LabeledObject *getLabeledObject(GLenum identifier, GLuint name) const;
    gl::LabeledObject *getLabeledObjectFromPtr(const void *ptr) const;

    void setUniform1iImpl(Program *program,
                          UniformLocation location,
                          GLsizei count,
                          const GLint *v);
    void renderbufferStorageMultisampleImpl(GLenum target,
                                            GLsizei samples,
                                            GLenum internalformat,
                                            GLsizei width,
                                            GLsizei height,
                                            MultisamplingMode mode);

    void onUniformBlockBindingUpdated(GLuint uniformBlockIndex);

    void endTilingImplicit();

    State mState;
    bool mShared;
    bool mDisplayTextureShareGroup;
    bool mDisplaySemaphoreShareGroup;

    // Recorded errors
    mutable ErrorSet mErrors;

    // Stores for each buffer binding type whether is it allowed to be used in this context.
    angle::PackedEnumBitSet<BufferBinding> mValidBufferBindings;

    std::unique_ptr<rx::ContextImpl> mImplementation;

    EGLLabelKHR mLabel;

    // Extensions supported by the implementation plus extensions that are implemented entirely
    // within the frontend.
    Extensions mSupportedExtensions;

    // Shader compiler. Lazily initialized hence the mutable value.
    mutable BindingPointer<Compiler> mCompiler;

    const egl::Config *mConfig;

    TextureMap mZeroTextures;

    ResourceMap<FenceNV, FenceNVID> mFenceNVMap;
    HandleAllocator mFenceNVHandleAllocator;

    QueryMap mQueryMap;
    HandleAllocator mQueryHandleAllocator;

    VertexArrayMap mVertexArrayMap;
    HandleAllocator mVertexArrayHandleAllocator;

    TransformFeedbackMap mTransformFeedbackMap;
    HandleAllocator mTransformFeedbackHandleAllocator;

    const char *mVendorString;
    const char *mVersionString;
    const char *mShadingLanguageString;
    const char *mRendererString;
    const char *mExtensionString;
    std::vector<const char *> mExtensionStrings;
    const char *mRequestableExtensionString;
    std::vector<const char *> mRequestableExtensionStrings;

    // GLES1 renderer state
    std::unique_ptr<GLES1Renderer> mGLES1Renderer;

    // Current/lost context flags
    bool mHasBeenCurrent;
    const bool mSurfacelessSupported;
    egl::Surface *mCurrentDrawSurface;
    egl::Surface *mCurrentReadSurface;
    egl::Display *mDisplay;
    const bool mWebGLContext;
    bool mBufferAccessValidationEnabled;
    const bool mExtensionsEnabled;
    MemoryProgramCache *mMemoryProgramCache;
    MemoryShaderCache *mMemoryShaderCache;

    state::DirtyObjects mDrawDirtyObjects;

    StateCache mStateCache;
    PrivateStateCache mPrivateStateCache;

    state::DirtyObjects mTexImageDirtyObjects;
    state::DirtyObjects mReadPixelsDirtyObjects;
    state::DirtyObjects mClearDirtyObjects;
    state::DirtyObjects mBlitDirtyObjects;
    state::DirtyObjects mComputeDirtyObjects;
    state::DirtyBits mCopyImageDirtyBits;
    state::DirtyObjects mCopyImageDirtyObjects;

    // Binding to container objects that use dependent state updates.
    angle::ObserverBinding mVertexArrayObserverBinding;
    angle::ObserverBinding mDrawFramebufferObserverBinding;
    angle::ObserverBinding mReadFramebufferObserverBinding;
    angle::ObserverBinding mProgramObserverBinding;
    angle::ObserverBinding mProgramPipelineObserverBinding;
    std::vector<angle::ObserverBinding> mUniformBufferObserverBindings;
    std::vector<angle::ObserverBinding> mAtomicCounterBufferObserverBindings;
    std::vector<angle::ObserverBinding> mShaderStorageBufferObserverBindings;
    std::vector<angle::ObserverBinding> mSamplerObserverBindings;
    std::vector<angle::ObserverBinding> mImageObserverBindings;

    // Not really a property of context state. The size and contexts change per-api-call.
    mutable Optional<angle::ScratchBuffer> mScratchBuffer;
    mutable Optional<angle::ScratchBuffer> mZeroFilledBuffer;

    // Note: we use a raw pointer here so we can exclude frame capture sources from the build.
    std::unique_ptr<angle::FrameCapture> mFrameCapture;

    // Cache representation of the serialized context string.
    mutable std::string mCachedSerializedStateString;

    mutable size_t mRefCount;

    OverlayType mOverlay;

    bool mIsDestroyed;

    std::unique_ptr<Framebuffer> mDefaultFramebuffer;
};

class [[nodiscard]] ScopedContextRef
{
  public:
    ScopedContextRef(Context *context) : mContext(context)
    {
        if (mContext)
        {
            mContext->addRef();
        }
    }
    ~ScopedContextRef()
    {
        if (mContext)
        {
            mContext->release();
        }
    }

  private:
    Context *const mContext;
};

// Thread-local current valid context bound to the thread.
#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
extern Context *GetCurrentValidContextTLS();
extern void SetCurrentValidContextTLS(Context *context);
#else
extern thread_local Context *gCurrentValidContext;
#endif

extern void SetCurrentValidContext(Context *context);

}  // namespace gl

#endif  // LIBANGLE_CONTEXT_H_
