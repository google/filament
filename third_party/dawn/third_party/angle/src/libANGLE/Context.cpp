//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Context.cpp: Implements the gl::Context class, managing all GL state and performing
// rendering operations. It is the GLES2 specific implementation of EGLContext.
#include "libANGLE/Context.inl.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <iterator>
#include <sstream>
#include <vector>

#include "common/PackedEnums.h"
#include "common/angle_version_info.h"
#include "common/hash_utils.h"
#include "common/matrix_utils.h"
#include "common/platform.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "common/tls.h"
#include "common/utilities.h"
#include "image_util/loadimage.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Display.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Fence.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/MemoryObject.h"
#include "libANGLE/PixelLocalStorage.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramPipeline.h"
#include "libANGLE/Query.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/Semaphore.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/capture/FrameCapture.h"
#include "libANGLE/capture/serialize.h"
#include "libANGLE/context_private_call.inl.h"
#include "libANGLE/context_private_call_autogen.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/renderer/DisplayImpl.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/trace.h"
#include "libANGLE/validationES.h"

#if defined(ANGLE_PLATFORM_APPLE)
#    include <dispatch/dispatch.h>
#    include "common/tls.h"
#endif

namespace gl
{
namespace
{
constexpr state::DirtyObjects kDrawDirtyObjectsBase{
    state::DIRTY_OBJECT_ACTIVE_TEXTURES,
    state::DIRTY_OBJECT_DRAW_FRAMEBUFFER,
    state::DIRTY_OBJECT_VERTEX_ARRAY,
    state::DIRTY_OBJECT_TEXTURES,
    state::DIRTY_OBJECT_PROGRAM_PIPELINE_OBJECT,
    state::DIRTY_OBJECT_SAMPLERS,
    state::DIRTY_OBJECT_IMAGES,
};

// TexImage uses the unpack state
constexpr state::DirtyBits kTexImageDirtyBits{
    state::DIRTY_BIT_UNPACK_STATE,
    state::DIRTY_BIT_UNPACK_BUFFER_BINDING,
};
constexpr state::ExtendedDirtyBits kTexImageExtendedDirtyBits{};
constexpr state::DirtyObjects kTexImageDirtyObjects{};

// Readpixels uses the pack state and read FBO
constexpr state::DirtyBits kReadPixelsDirtyBits{
    state::DIRTY_BIT_PACK_STATE,
    state::DIRTY_BIT_PACK_BUFFER_BINDING,
    state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING,
};
constexpr state::ExtendedDirtyBits kReadPixelsExtendedDirtyBits{};
constexpr state::DirtyObjects kReadPixelsDirtyObjectsBase{state::DIRTY_OBJECT_READ_FRAMEBUFFER};

// We sync the draw Framebuffer manually in prepareForClear to allow the clear calls to do
// more custom handling for robust resource init.
constexpr state::DirtyBits kClearDirtyBits{
    state::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED,
    state::DIRTY_BIT_SCISSOR_TEST_ENABLED,
    state::DIRTY_BIT_SCISSOR,
    state::DIRTY_BIT_VIEWPORT,
    state::DIRTY_BIT_CLEAR_COLOR,
    state::DIRTY_BIT_CLEAR_DEPTH,
    state::DIRTY_BIT_CLEAR_STENCIL,
    state::DIRTY_BIT_COLOR_MASK,
    state::DIRTY_BIT_DEPTH_MASK,
    state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT,
    state::DIRTY_BIT_STENCIL_WRITEMASK_BACK,
    state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING,
};
constexpr state::ExtendedDirtyBits kClearExtendedDirtyBits{};
constexpr state::DirtyObjects kClearDirtyObjects{state::DIRTY_OBJECT_DRAW_FRAMEBUFFER};

constexpr state::DirtyBits kBlitDirtyBits{
    state::DIRTY_BIT_SCISSOR_TEST_ENABLED,
    state::DIRTY_BIT_SCISSOR,
    state::DIRTY_BIT_FRAMEBUFFER_SRGB_WRITE_CONTROL_MODE,
    state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING,
    state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING,
};
constexpr state::ExtendedDirtyBits kBlitExtendedDirtyBits{};
constexpr state::DirtyObjects kBlitDirtyObjectsBase{
    state::DIRTY_OBJECT_READ_FRAMEBUFFER,
    state::DIRTY_OBJECT_DRAW_FRAMEBUFFER,
};

constexpr state::DirtyBits kComputeDirtyBits{
    state::DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING,
    state::DIRTY_BIT_UNIFORM_BUFFER_BINDINGS,
    state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING,
    state::DIRTY_BIT_PROGRAM_BINDING,
    state::DIRTY_BIT_PROGRAM_EXECUTABLE,
    state::DIRTY_BIT_TEXTURE_BINDINGS,
    state::DIRTY_BIT_SAMPLER_BINDINGS,
    state::DIRTY_BIT_IMAGE_BINDINGS,
    state::DIRTY_BIT_DISPATCH_INDIRECT_BUFFER_BINDING,
};
constexpr state::ExtendedDirtyBits kComputeExtendedDirtyBits{};
constexpr state::DirtyObjects kComputeDirtyObjectsBase{
    state::DIRTY_OBJECT_ACTIVE_TEXTURES,
    state::DIRTY_OBJECT_TEXTURES,
    state::DIRTY_OBJECT_PROGRAM_PIPELINE_OBJECT,
    state::DIRTY_OBJECT_IMAGES,
    state::DIRTY_OBJECT_SAMPLERS,
};

constexpr state::DirtyBits kCopyImageDirtyBitsBase{state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING};
constexpr state::ExtendedDirtyBits kCopyImageExtendedDirtyBits{};
constexpr state::DirtyObjects kCopyImageDirtyObjectsBase{state::DIRTY_OBJECT_READ_FRAMEBUFFER};

constexpr state::DirtyBits kReadInvalidateDirtyBits{state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING};
constexpr state::ExtendedDirtyBits kReadInvalidateExtendedDirtyBits{};

constexpr state::DirtyBits kDrawInvalidateDirtyBits{state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING};
constexpr state::ExtendedDirtyBits kDrawInvalidateExtendedDirtyBits{};

constexpr state::DirtyBits kTilingDirtyBits{state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING};
constexpr state::ExtendedDirtyBits kTilingExtendedDirtyBits{};
constexpr state::DirtyObjects kTilingDirtyObjects{state::DIRTY_OBJECT_DRAW_FRAMEBUFFER};

constexpr bool kEnableAEPRequirementLogging = false;

egl::ShareGroup *AllocateOrGetShareGroup(egl::Display *display, const gl::Context *shareContext)
{
    if (shareContext)
    {
        egl::ShareGroup *shareGroup = shareContext->getState().getShareGroup();
        shareGroup->addRef();
        return shareGroup;
    }
    else
    {
        return new egl::ShareGroup(display->getImplementation());
    }
}

egl::ContextMutex *AllocateOrUseContextMutex(egl::ContextMutex *sharedContextMutex)
{
    if (sharedContextMutex != nullptr)
    {
        ASSERT(egl::kIsContextMutexEnabled);
        ASSERT(sharedContextMutex->isReferenced());
        return sharedContextMutex;
    }
    return new egl::ContextMutex();
}

template <typename T>
angle::Result GetQueryObjectParameter(const Context *context, Query *query, GLenum pname, T *params)
{
    if (!query)
    {
        // Some applications call into glGetQueryObjectuiv(...) prior to calling glBeginQuery(...)
        // This wouldn't be an issue since the validation layer will handle such a usecases but when
        // the app enables EGL_KHR_create_context_no_error extension, we skip the validation layer.
        switch (pname)
        {
            case GL_QUERY_RESULT_EXT:
                *params = 0;
                break;
            case GL_QUERY_RESULT_AVAILABLE_EXT:
                *params = GL_FALSE;
                break;
            default:
                UNREACHABLE();
                return angle::Result::Stop;
        }
        return angle::Result::Continue;
    }

    switch (pname)
    {
        case GL_QUERY_RESULT_EXT:
            return query->getResult(context, params);
        case GL_QUERY_RESULT_AVAILABLE_EXT:
        {
            bool available = false;
            if (context->isContextLost())
            {
                available = true;
            }
            else
            {
                ANGLE_TRY(query->isResultAvailable(context, &available));
            }
            *params = CastFromStateValue<T>(pname, static_cast<GLuint>(available));
            return angle::Result::Continue;
        }
        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }
}

// Attribute map queries.
EGLint GetClientMajorVersion(const egl::AttributeMap &attribs)
{
    return static_cast<EGLint>(attribs.get(EGL_CONTEXT_CLIENT_VERSION, 1));
}

EGLint GetClientMinorVersion(const egl::AttributeMap &attribs)
{
    return static_cast<EGLint>(attribs.get(EGL_CONTEXT_MINOR_VERSION, 0));
}

bool GetBackwardCompatibleContext(const egl::AttributeMap &attribs)
{
    return attribs.get(EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE, EGL_TRUE) == EGL_TRUE;
}

bool GetWebGLContext(const egl::AttributeMap &attribs)
{
    return (attribs.get(EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE, EGL_FALSE) == EGL_TRUE);
}

Version GetClientVersion(egl::Display *display, const egl::AttributeMap &attribs)
{
    Version requestedVersion =
        Version(GetClientMajorVersion(attribs), GetClientMinorVersion(attribs));
    if (GetBackwardCompatibleContext(attribs))
    {
        if (requestedVersion.major == 1)
        {
            // If the user requests an ES1 context, we cannot return an ES 2+ context.
            return Version(1, 1);
        }
        else
        {
            // Always up the version to at least the max conformant version this display supports.
            // Only return a higher client version if requested.
            const Version conformantVersion = std::max(
                display->getImplementation()->getMaxConformantESVersion(), requestedVersion);
            // Limit the WebGL context to at most version 3.1
            const bool isWebGL = GetWebGLContext(attribs);
            return isWebGL ? std::min(conformantVersion, Version(3, 1)) : conformantVersion;
        }
    }
    else
    {
        return requestedVersion;
    }
}

GLenum GetResetStrategy(const egl::AttributeMap &attribs)
{
    EGLAttrib resetStrategyExt =
        attribs.get(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT, EGL_NO_RESET_NOTIFICATION);
    EGLAttrib resetStrategyCore =
        attribs.get(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY, resetStrategyExt);

    switch (resetStrategyCore)
    {
        case EGL_NO_RESET_NOTIFICATION:
            return GL_NO_RESET_NOTIFICATION_EXT;
        case EGL_LOSE_CONTEXT_ON_RESET:
            return GL_LOSE_CONTEXT_ON_RESET_EXT;
        default:
            UNREACHABLE();
            return GL_NONE;
    }
}

bool GetRobustAccess(const egl::AttributeMap &attribs)
{
    EGLAttrib robustAccessExt  = attribs.get(EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT, EGL_FALSE);
    EGLAttrib robustAccessCore = attribs.get(EGL_CONTEXT_OPENGL_ROBUST_ACCESS, robustAccessExt);

    bool attribRobustAccess = (robustAccessCore == EGL_TRUE);
    bool contextFlagsRobustAccess =
        ((attribs.get(EGL_CONTEXT_FLAGS_KHR, 0) & EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR) != 0);

    return (attribRobustAccess || contextFlagsRobustAccess);
}

bool GetDebug(const egl::AttributeMap &attribs)
{
    return (attribs.get(EGL_CONTEXT_OPENGL_DEBUG, EGL_FALSE) == EGL_TRUE) ||
           ((attribs.get(EGL_CONTEXT_FLAGS_KHR, 0) & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR) != 0);
}

bool GetNoError(const egl::AttributeMap &attribs)
{
    return (attribs.get(EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_FALSE) == EGL_TRUE);
}

bool GetExtensionsEnabled(const egl::AttributeMap &attribs, bool webGLContext)
{
    // If the context is WebGL, extensions are disabled by default
    EGLAttrib defaultValue = webGLContext ? EGL_FALSE : EGL_TRUE;
    return (attribs.get(EGL_EXTENSIONS_ENABLED_ANGLE, defaultValue) == EGL_TRUE);
}

bool GetBindGeneratesResource(const egl::AttributeMap &attribs)
{
    return (attribs.get(EGL_CONTEXT_BIND_GENERATES_RESOURCE_CHROMIUM, EGL_TRUE) == EGL_TRUE);
}

bool GetClientArraysEnabled(const egl::AttributeMap &attribs)
{
    return (attribs.get(EGL_CONTEXT_CLIENT_ARRAYS_ENABLED_ANGLE, EGL_TRUE) == EGL_TRUE);
}

bool GetRobustResourceInit(egl::Display *display, const egl::AttributeMap &attribs)
{
    const angle::FrontendFeatures &frontendFeatures = display->getFrontendFeatures();
    return (frontendFeatures.forceRobustResourceInit.enabled ||
            attribs.get(EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE, EGL_FALSE) == EGL_TRUE);
}

EGLenum GetContextPriority(const egl::AttributeMap &attribs)
{
    return static_cast<EGLenum>(
        attribs.getAsInt(EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_CONTEXT_PRIORITY_MEDIUM_IMG));
}

bool GetProtectedContent(const egl::AttributeMap &attribs)
{
    return static_cast<bool>(attribs.getAsInt(EGL_PROTECTED_CONTENT_EXT, EGL_FALSE));
}

std::string GetObjectLabelFromPointer(GLsizei length, const GLchar *label)
{
    std::string labelName;
    if (label != nullptr)
    {
        size_t labelLength = length < 0 ? strlen(label) : length;
        labelName          = std::string(label, labelLength);
    }
    return labelName;
}

void GetObjectLabelBase(const std::string &objectLabel,
                        GLsizei bufSize,
                        GLsizei *length,
                        GLchar *label)
{
    size_t writeLength = objectLabel.length();
    if (label != nullptr && bufSize > 0)
    {
        writeLength = std::min(static_cast<size_t>(bufSize) - 1, objectLabel.length());
        std::copy(objectLabel.begin(), objectLabel.begin() + writeLength, label);
        label[writeLength] = '\0';
    }

    if (length != nullptr)
    {
        *length = static_cast<GLsizei>(writeLength);
    }
}

enum SubjectIndexes : angle::SubjectIndex
{
    kTexture0SubjectIndex       = 0,
    kTextureMaxSubjectIndex     = kTexture0SubjectIndex + IMPLEMENTATION_MAX_ACTIVE_TEXTURES,
    kImage0SubjectIndex         = kTextureMaxSubjectIndex,
    kImageMaxSubjectIndex       = kImage0SubjectIndex + IMPLEMENTATION_MAX_IMAGE_UNITS,
    kUniformBuffer0SubjectIndex = kImageMaxSubjectIndex,
    kUniformBufferMaxSubjectIndex =
        kUniformBuffer0SubjectIndex + IMPLEMENTATION_MAX_UNIFORM_BUFFER_BINDINGS,
    kAtomicCounterBuffer0SubjectIndex = kUniformBufferMaxSubjectIndex,
    kAtomicCounterBufferMaxSubjectIndex =
        kAtomicCounterBuffer0SubjectIndex + IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS,
    kShaderStorageBuffer0SubjectIndex = kAtomicCounterBufferMaxSubjectIndex,
    kShaderStorageBufferMaxSubjectIndex =
        kShaderStorageBuffer0SubjectIndex + IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS,
    kSampler0SubjectIndex    = kShaderStorageBufferMaxSubjectIndex,
    kSamplerMaxSubjectIndex  = kSampler0SubjectIndex + IMPLEMENTATION_MAX_ACTIVE_TEXTURES,
    kVertexArraySubjectIndex = kSamplerMaxSubjectIndex,
    kReadFramebufferSubjectIndex,
    kDrawFramebufferSubjectIndex,
    kProgramSubjectIndex,
    kProgramPipelineSubjectIndex,
};

bool IsClearBufferEnabled(const FramebufferState &fbState, GLenum buffer, GLint drawbuffer)
{
    return buffer != GL_COLOR || fbState.getEnabledDrawBuffers()[drawbuffer];
}

bool IsColorMaskedOut(const BlendStateExt &blendStateExt, const GLint drawbuffer)
{
    ASSERT(static_cast<size_t>(drawbuffer) < blendStateExt.getDrawBufferCount());
    return blendStateExt.getColorMaskIndexed(static_cast<size_t>(drawbuffer)) == 0;
}

bool GetIsExternal(const egl::AttributeMap &attribs)
{
    return (attribs.get(EGL_EXTERNAL_CONTEXT_ANGLE, EGL_FALSE) == EGL_TRUE);
}

void GetPerfMonitorString(const std::string &name,
                          GLsizei bufSize,
                          GLsizei *length,
                          GLchar *stringOut)
{
    GLsizei numCharsWritten = std::min(bufSize, static_cast<GLsizei>(name.size()));

    if (length)
    {
        if (bufSize == 0)
        {
            *length = static_cast<GLsizei>(name.size());
        }
        else
        {
            // Excludes null terminator.
            ASSERT(numCharsWritten > 0);
            *length = numCharsWritten - 1;
        }
    }

    if (stringOut)
    {
        memcpy(stringOut, name.c_str(), numCharsWritten);
    }
}

bool CanSupportAEP(const gl::Version &version, const gl::Extensions &extensions)
{
    // From the GL_ANDROID_extension_pack_es31a extension spec:
    // OpenGL ES 3.1 and GLSL ES 3.10 are required.
    // The following extensions are required:
    // * KHR_debug
    // * KHR_texture_compression_astc_ldr
    // * KHR_blend_equation_advanced
    // * OES_sample_shading
    // * OES_sample_variables
    // * OES_shader_image_atomic
    // * OES_shader_multisample_interpolation
    // * OES_texture_stencil8
    // * OES_texture_storage_multisample_2d_array
    // * EXT_copy_image
    // * EXT_draw_buffers_indexed
    // * EXT_geometry_shader
    // * EXT_gpu_shader5
    // * EXT_primitive_bounding_box
    // * EXT_shader_io_blocks
    // * EXT_tessellation_shader
    // * EXT_texture_border_clamp
    // * EXT_texture_buffer
    // * EXT_texture_cube_map_array
    // * EXT_texture_sRGB_decode
    std::pair<const char *, bool> requirements[] = {
        {"version >= ES_3_1", version >= ES_3_1},
        {"extensions.debugKHR", extensions.debugKHR},
        {"extensions.textureCompressionAstcLdrKHR", extensions.textureCompressionAstcLdrKHR},
        {"extensions.blendEquationAdvancedKHR", extensions.blendEquationAdvancedKHR},
        {"extensions.sampleShadingOES", extensions.sampleShadingOES},
        {"extensions.sampleVariablesOES", extensions.sampleVariablesOES},
        {"extensions.shaderImageAtomicOES", extensions.shaderImageAtomicOES},
        {"extensions.shaderMultisampleInterpolationOES",
         extensions.shaderMultisampleInterpolationOES},
        {"extensions.textureStencil8OES", extensions.textureStencil8OES},
        {"extensions.textureStorageMultisample2dArrayOES",
         extensions.textureStorageMultisample2dArrayOES},
        {"extensions.copyImageEXT", extensions.copyImageEXT},
        {"extensions.drawBuffersIndexedEXT", extensions.drawBuffersIndexedEXT},
        {"extensions.geometryShaderEXT", extensions.geometryShaderEXT},
        {"extensions.gpuShader5EXT", extensions.gpuShader5EXT},
        {"extensions.primitiveBoundingBoxEXT", extensions.primitiveBoundingBoxEXT},
        {"extensions.shaderIoBlocksEXT", extensions.shaderIoBlocksEXT},
        {"extensions.tessellationShaderEXT", extensions.tessellationShaderEXT},
        {"extensions.textureBorderClampEXT", extensions.textureBorderClampEXT},
        {"extensions.textureBufferEXT", extensions.textureBufferEXT},
        {"extensions.textureCubeMapArrayEXT", extensions.textureCubeMapArrayEXT},
        {"extensions.textureSRGBDecodeEXT", extensions.textureSRGBDecodeEXT},
    };

    bool result = true;
    for (const auto &req : requirements)
    {
        result = result && req.second;
    }

    if (kEnableAEPRequirementLogging && !result)
    {
        INFO() << "CanSupportAEP() check failed for missing the following requirements:\n";
        for (const auto &req : requirements)
        {
            if (!req.second)
            {
                INFO() << "- " << req.first << "\n";
            }
        }
    }

    return result;
}
}  // anonymous namespace

#if defined(ANGLE_PLATFORM_APPLE)
// TODO(angleproject:6479): Due to a bug in Apple's dyld loader, `thread_local` will cause
// excessive memory use. Temporarily avoid it by using pthread's thread
// local storage instead.
static angle::TLSIndex GetCurrentValidContextTLSIndex()
{
    static angle::TLSIndex CurrentValidContextIndex = TLS_INVALID_INDEX;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
      ASSERT(CurrentValidContextIndex == TLS_INVALID_INDEX);
      CurrentValidContextIndex = angle::CreateTLSIndex(nullptr);
    });
    return CurrentValidContextIndex;
}
Context *GetCurrentValidContextTLS()
{
    angle::TLSIndex CurrentValidContextIndex = GetCurrentValidContextTLSIndex();
    ASSERT(CurrentValidContextIndex != TLS_INVALID_INDEX);
    return static_cast<Context *>(angle::GetTLSValue(CurrentValidContextIndex));
}
void SetCurrentValidContextTLS(Context *context)
{
    angle::TLSIndex CurrentValidContextIndex = GetCurrentValidContextTLSIndex();
    ASSERT(CurrentValidContextIndex != TLS_INVALID_INDEX);
    angle::SetTLSValue(CurrentValidContextIndex, context);
}
#elif defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
static thread_local Context *gCurrentValidContext = nullptr;
Context *GetCurrentValidContextTLS()
{
    return gCurrentValidContext;
}
void SetCurrentValidContextTLS(Context *context)
{
    gCurrentValidContext = context;
}
#else
thread_local Context *gCurrentValidContext = nullptr;
#endif

// Handle setting the current context in TLS on different platforms
extern void SetCurrentValidContext(Context *context)
{
#if defined(ANGLE_USE_ANDROID_TLS_SLOT)
    if (angle::gUseAndroidOpenGLTlsSlot)
    {
        ANGLE_ANDROID_GET_GL_TLS()[angle::kAndroidOpenGLTlsSlot] = static_cast<void *>(context);
        return;
    }
#endif

#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
    SetCurrentValidContextTLS(context);
#else
    gCurrentValidContext = context;
#endif
}

Context::Context(egl::Display *display,
                 const egl::Config *config,
                 const Context *shareContext,
                 TextureManager *shareTextures,
                 SemaphoreManager *shareSemaphores,
                 egl::ContextMutex *sharedContextMutex,
                 MemoryProgramCache *memoryProgramCache,
                 MemoryShaderCache *memoryShaderCache,
                 const egl::AttributeMap &attribs,
                 const egl::DisplayExtensions &displayExtensions,
                 const egl::ClientExtensions &clientExtensions)
    : mState(shareContext ? &shareContext->mState : nullptr,
             AllocateOrGetShareGroup(display, shareContext),
             shareTextures,
             shareSemaphores,
             AllocateOrUseContextMutex(sharedContextMutex),
             &mOverlay,
             GetClientVersion(display, attribs),
             GetDebug(attribs),
             GetBindGeneratesResource(attribs),
             GetClientArraysEnabled(attribs),
             GetRobustResourceInit(display, attribs),
             memoryProgramCache != nullptr,
             GetContextPriority(attribs),
             GetRobustAccess(attribs),
             GetProtectedContent(attribs),
             GetIsExternal(attribs)),
      mShared(shareContext != nullptr || shareTextures != nullptr || shareSemaphores != nullptr),
      mDisplayTextureShareGroup(shareTextures != nullptr),
      mDisplaySemaphoreShareGroup(shareSemaphores != nullptr),
      mErrors(&mState.getDebug(), display->getFrontendFeatures(), attribs),
      mImplementation(display->getImplementation()
                          ->createContext(mState, &mErrors, config, shareContext, attribs)),
      mLabel(nullptr),
      mCompiler(),
      mConfig(config),
      mHasBeenCurrent(false),
      mSurfacelessSupported(displayExtensions.surfacelessContext),
      mCurrentDrawSurface(static_cast<egl::Surface *>(EGL_NO_SURFACE)),
      mCurrentReadSurface(static_cast<egl::Surface *>(EGL_NO_SURFACE)),
      mDisplay(display),
      mWebGLContext(GetWebGLContext(attribs)),
      mBufferAccessValidationEnabled(false),
      mExtensionsEnabled(GetExtensionsEnabled(attribs, mWebGLContext)),
      mMemoryProgramCache(memoryProgramCache),
      mMemoryShaderCache(memoryShaderCache),
      mVertexArrayObserverBinding(this, kVertexArraySubjectIndex),
      mDrawFramebufferObserverBinding(this, kDrawFramebufferSubjectIndex),
      mReadFramebufferObserverBinding(this, kReadFramebufferSubjectIndex),
      mProgramObserverBinding(this, kProgramSubjectIndex),
      mProgramPipelineObserverBinding(this, kProgramPipelineSubjectIndex),
      mFrameCapture(new angle::FrameCapture),
      mRefCount(0),
      mOverlay(mImplementation.get()),
      mIsDestroyed(false)
{
    for (angle::SubjectIndex uboIndex = kUniformBuffer0SubjectIndex;
         uboIndex < kUniformBufferMaxSubjectIndex; ++uboIndex)
    {
        mUniformBufferObserverBindings.emplace_back(this, uboIndex);
    }

    for (angle::SubjectIndex acbIndex = kAtomicCounterBuffer0SubjectIndex;
         acbIndex < kAtomicCounterBufferMaxSubjectIndex; ++acbIndex)
    {
        mAtomicCounterBufferObserverBindings.emplace_back(this, acbIndex);
    }

    for (angle::SubjectIndex ssboIndex = kShaderStorageBuffer0SubjectIndex;
         ssboIndex < kShaderStorageBufferMaxSubjectIndex; ++ssboIndex)
    {
        mShaderStorageBufferObserverBindings.emplace_back(this, ssboIndex);
    }

    for (angle::SubjectIndex samplerIndex = kSampler0SubjectIndex;
         samplerIndex < kSamplerMaxSubjectIndex; ++samplerIndex)
    {
        mSamplerObserverBindings.emplace_back(this, samplerIndex);
    }

    for (angle::SubjectIndex imageIndex = kImage0SubjectIndex; imageIndex < kImageMaxSubjectIndex;
         ++imageIndex)
    {
        mImageObserverBindings.emplace_back(this, imageIndex);
    }

    // Implementations now require the display to be set at context creation.
    ASSERT(mDisplay);
}

egl::Error Context::initialize()
{
    if (!mImplementation)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "native context creation failed");
    }

    // If the final context version created (with backwards compatibility possibly added in),
    // generate an error if it's higher than the maximum supported version for the display. This
    // validation is always done even with EGL validation disabled because it's not possible to
    // detect ahead of time if an ES 3.1 context is supported (no ES_31_BIT) or if
    // KHR_no_config_context is used.
    if (getClientVersion() > getDisplay()->getMaxSupportedESVersion())
    {
        return egl::Error(EGL_BAD_ATTRIBUTE, "Requested version is not supported");
    }

    return egl::NoError();
}

void Context::initializeDefaultResources()
{
    mImplementation->setMemoryProgramCache(mMemoryProgramCache);

    initCaps();

    mState.initialize(this);

    mDefaultFramebuffer = std::make_unique<Framebuffer>(this, mImplementation.get());

    mFenceNVHandleAllocator.setBaseHandle(0);

    // [OpenGL ES 2.0.24] section 3.7 page 83:
    // In the initial state, TEXTURE_2D and TEXTURE_CUBE_MAP have two-dimensional
    // and cube map texture state vectors respectively associated with them.
    // In order that access to these initial textures not be lost, they are treated as texture
    // objects all of whose names are 0.

    Texture *zeroTexture2D = new Texture(mImplementation.get(), {0}, TextureType::_2D);
    mZeroTextures[TextureType::_2D].set(this, zeroTexture2D);

    Texture *zeroTextureCube = new Texture(mImplementation.get(), {0}, TextureType::CubeMap);
    mZeroTextures[TextureType::CubeMap].set(this, zeroTextureCube);

    if (getClientVersion() >= Version(3, 0) || mSupportedExtensions.texture3DOES)
    {
        Texture *zeroTexture3D = new Texture(mImplementation.get(), {0}, TextureType::_3D);
        mZeroTextures[TextureType::_3D].set(this, zeroTexture3D);
    }
    if (getClientVersion() >= Version(3, 0))
    {
        Texture *zeroTexture2DArray =
            new Texture(mImplementation.get(), {0}, TextureType::_2DArray);
        mZeroTextures[TextureType::_2DArray].set(this, zeroTexture2DArray);
    }
    if (getClientVersion() >= Version(3, 1) || mSupportedExtensions.textureMultisampleANGLE)
    {
        Texture *zeroTexture2DMultisample =
            new Texture(mImplementation.get(), {0}, TextureType::_2DMultisample);
        mZeroTextures[TextureType::_2DMultisample].set(this, zeroTexture2DMultisample);
    }
    if (getClientVersion() >= Version(3, 2) ||
        mSupportedExtensions.textureStorageMultisample2dArrayOES)
    {
        Texture *zeroTexture2DMultisampleArray =
            new Texture(mImplementation.get(), {0}, TextureType::_2DMultisampleArray);
        mZeroTextures[TextureType::_2DMultisampleArray].set(this, zeroTexture2DMultisampleArray);
    }

    if (getClientVersion() >= Version(3, 1))
    {
        for (int i = 0; i < mState.getCaps().maxAtomicCounterBufferBindings; i++)
        {
            bindBufferRange(BufferBinding::AtomicCounter, i, {0}, 0, 0);
        }

        for (int i = 0; i < mState.getCaps().maxShaderStorageBufferBindings; i++)
        {
            bindBufferRange(BufferBinding::ShaderStorage, i, {0}, 0, 0);
        }
    }

    if (getClientVersion() >= Version(3, 2) || mSupportedExtensions.textureCubeMapArrayAny())
    {
        Texture *zeroTextureCubeMapArray =
            new Texture(mImplementation.get(), {0}, TextureType::CubeMapArray);
        mZeroTextures[TextureType::CubeMapArray].set(this, zeroTextureCubeMapArray);
    }

    if (getClientVersion() >= Version(3, 2) || mSupportedExtensions.textureBufferAny())
    {
        Texture *zeroTextureBuffer = new Texture(mImplementation.get(), {0}, TextureType::Buffer);
        mZeroTextures[TextureType::Buffer].set(this, zeroTextureBuffer);
    }

    if (mSupportedExtensions.textureRectangleANGLE)
    {
        Texture *zeroTextureRectangle =
            new Texture(mImplementation.get(), {0}, TextureType::Rectangle);
        mZeroTextures[TextureType::Rectangle].set(this, zeroTextureRectangle);
    }

    if (mSupportedExtensions.EGLImageExternalOES ||
        mSupportedExtensions.EGLStreamConsumerExternalNV)
    {
        Texture *zeroTextureExternal =
            new Texture(mImplementation.get(), {0}, TextureType::External);
        mZeroTextures[TextureType::External].set(this, zeroTextureExternal);
    }

    // This may change native TEXTURE_2D, TEXTURE_EXTERNAL_OES and TEXTURE_RECTANGLE,
    // binding states. Ensure state manager is aware of this when binding
    // this texture type.
    if (mSupportedExtensions.videoTextureWEBGL)
    {
        Texture *zeroTextureVideoImage =
            new Texture(mImplementation.get(), {0}, TextureType::VideoImage);
        mZeroTextures[TextureType::VideoImage].set(this, zeroTextureVideoImage);
    }

    mState.initializeZeroTextures(this, mZeroTextures);

    ANGLE_CONTEXT_TRY(mImplementation->initialize(mDisplay->getImageLoadContext()));

    // Add context into the share group
    mState.getShareGroup()->addSharedContext(this);

    bindVertexArray({0});

    if (getClientVersion() >= Version(3, 0))
    {
        // [OpenGL ES 3.0.2] section 2.14.1 pg 85:
        // In the initial state, a default transform feedback object is bound and treated as
        // a transform feedback object with a name of zero. That object is bound any time
        // BindTransformFeedback is called with id of zero
        bindTransformFeedback(GL_TRANSFORM_FEEDBACK, {0});
    }

    for (auto type : angle::AllEnums<BufferBinding>())
    {
        bindBuffer(type, {0});
    }

    bindRenderbuffer(GL_RENDERBUFFER, {0});

    for (int i = 0; i < mState.getCaps().maxUniformBufferBindings; i++)
    {
        bindBufferRange(BufferBinding::Uniform, i, {0}, 0, -1);
    }

    // Initialize GLES1 renderer if appropriate.
    if (getClientVersion() < Version(2, 0))
    {
        mGLES1Renderer.reset(new GLES1Renderer());
    }

    // Initialize dirty bit masks (in addition to what updateCaps() might have set up).
    mDrawDirtyObjects |= kDrawDirtyObjectsBase;
    mTexImageDirtyObjects |= kTexImageDirtyObjects;
    mReadPixelsDirtyObjects |= kReadPixelsDirtyObjectsBase;
    mClearDirtyObjects |= kClearDirtyObjects;
    mBlitDirtyObjects |= kBlitDirtyObjectsBase;
    mComputeDirtyObjects |= kComputeDirtyObjectsBase;
    mCopyImageDirtyBits |= kCopyImageDirtyBitsBase;
    mCopyImageDirtyObjects |= kCopyImageDirtyObjectsBase;

    mOverlay.init();
}

egl::Error Context::onDestroy(const egl::Display *display)
{
    if (!mHasBeenCurrent)
    {
        // Shared objects and ShareGroup must be released regardless.
        releaseSharedObjects();
        mState.mShareGroup->release(display);
        // The context is never current, so default resources are not allocated.
        return egl::NoError();
    }

    mState.ensureNoPendingLink(this);

    // eglDestoryContext() must have been called for this Context and there must not be any Threads
    // that still have it current.
    ASSERT(mIsDestroyed == true && mRefCount == 0);

    // Dump frame capture if enabled.
    getShareGroup()->getFrameCaptureShared()->onDestroyContext(this);

    // Remove context from the capture share group
    getShareGroup()->removeSharedContext(this);

    if (mGLES1Renderer)
    {
        mGLES1Renderer->onDestroy(this, &mState);
    }

    ANGLE_TRY(unMakeCurrent(display));

    mDefaultFramebuffer->onDestroy(this);
    mDefaultFramebuffer.reset();

    for (auto fence : UnsafeResourceMapIter(mFenceNVMap))
    {
        if (fence.second)
        {
            fence.second->onDestroy(this);
        }
        SafeDelete(fence.second);
    }
    mFenceNVMap.clear();

    for (auto query : UnsafeResourceMapIter(mQueryMap))
    {
        if (query.second != nullptr)
        {
            query.second->release(this);
        }
    }
    mQueryMap.clear();

    for (auto vertexArray : UnsafeResourceMapIter(mVertexArrayMap))
    {
        if (vertexArray.second)
        {
            vertexArray.second->onDestroy(this);
        }
    }
    mVertexArrayMap.clear();

    for (auto transformFeedback : UnsafeResourceMapIter(mTransformFeedbackMap))
    {
        if (transformFeedback.second != nullptr)
        {
            transformFeedback.second->release(this);
        }
    }
    mTransformFeedbackMap.clear();

    for (BindingPointer<Texture> &zeroTexture : mZeroTextures)
    {
        if (zeroTexture.get() != nullptr)
        {
            zeroTexture.set(this, nullptr);
        }
    }

    releaseShaderCompiler();

    mState.reset(this);

    releaseSharedObjects();

    mImplementation->onDestroy(this);

    // Backend requires implementation to be destroyed first to close down all the objects
    mState.mShareGroup->release(display);

    mOverlay.destroy(this);

    return egl::NoError();
}

void Context::releaseSharedObjects()
{
    mState.mBufferManager->release(this);
    // mProgramPipelineManager must be before mShaderProgramManager to give each
    // PPO the chance to release any references they have to the Programs that
    // are bound to them before the Programs are released()'ed.
    mState.mProgramPipelineManager->release(this);
    mState.mShaderProgramManager->release(this);
    mState.mTextureManager->release(this);
    mState.mRenderbufferManager->release(this);
    mState.mSamplerManager->release(this);
    mState.mSyncManager->release(this);
    mState.mFramebufferManager->release(this);
    mState.mMemoryObjectManager->release(this);
    mState.mSemaphoreManager->release(this);
}

Context::~Context() {}

void Context::setLabel(EGLLabelKHR label)
{
    mLabel = label;
}

EGLLabelKHR Context::getLabel() const
{
    return mLabel;
}

egl::Error Context::makeCurrent(egl::Display *display,
                                egl::Surface *drawSurface,
                                egl::Surface *readSurface)
{
    mDisplay = display;

    if (!mHasBeenCurrent)
    {
        initializeDefaultResources();
        initRendererString();
        initVendorString();
        initVersionStrings();
        initExtensionStrings();

        int width  = 0;
        int height = 0;
        if (drawSurface != nullptr)
        {
            width  = drawSurface->getWidth();
            height = drawSurface->getHeight();
        }

        ContextPrivateViewport(getMutablePrivateState(), getMutablePrivateStateCache(), 0, 0, width,
                               height);
        ContextPrivateScissor(getMutablePrivateState(), getMutablePrivateStateCache(), 0, 0, width,
                              height);

        mHasBeenCurrent = true;
    }

    ANGLE_TRY(unsetDefaultFramebuffer());

    getShareGroup()->getFrameCaptureShared()->onMakeCurrent(this, drawSurface);

    // TODO(jmadill): Rework this when we support ContextImpl
    mState.setAllDirtyBits();
    mState.setAllDirtyObjects();

    ANGLE_TRY(setDefaultFramebuffer(drawSurface, readSurface));

    // Notify the renderer of a context switch.
    angle::Result implResult = mImplementation->onMakeCurrent(this);

    // If the implementation fails onMakeCurrent, unset the default framebuffer.
    if (implResult != angle::Result::Continue)
    {
        ANGLE_TRY(unsetDefaultFramebuffer());
        return angle::ResultToEGL(implResult);
    }

    return egl::NoError();
}

egl::Error Context::unMakeCurrent(const egl::Display *display)
{
    ANGLE_TRY(angle::ResultToEGL(mImplementation->onUnMakeCurrent(this)));

    ANGLE_TRY(unsetDefaultFramebuffer());

    // Return the scratch buffers to the display so they can be shared with other contexts while
    // this one is not current.
    if (mScratchBuffer.valid())
    {
        mDisplay->returnScratchBuffer(mScratchBuffer.release());
    }
    if (mZeroFilledBuffer.valid())
    {
        mDisplay->returnZeroFilledBuffer(mZeroFilledBuffer.release());
    }

    return egl::NoError();
}

BufferID Context::createBuffer()
{
    return mState.mBufferManager->createBuffer();
}

GLuint Context::createProgram()
{
    return mState.mShaderProgramManager->createProgram(mImplementation.get()).value;
}

GLuint Context::createShader(ShaderType type)
{
    return mState.mShaderProgramManager
        ->createShader(mImplementation.get(), mState.getLimitations(), type)
        .value;
}

TextureID Context::createTexture()
{
    return mState.mTextureManager->createTexture();
}

RenderbufferID Context::createRenderbuffer()
{
    return mState.mRenderbufferManager->createRenderbuffer();
}

// Returns an unused framebuffer name
FramebufferID Context::createFramebuffer()
{
    return mState.mFramebufferManager->createFramebuffer();
}

void Context::genFencesNV(GLsizei n, FenceNVID *fences)
{
    for (int i = 0; i < n; i++)
    {
        GLuint handle = mFenceNVHandleAllocator.allocate();
        mFenceNVMap.assign({handle}, new FenceNV(mImplementation.get()));
        fences[i] = {handle};
    }
}

ProgramPipelineID Context::createProgramPipeline()
{
    return mState.mProgramPipelineManager->createProgramPipeline();
}

GLuint Context::createShaderProgramv(ShaderType type, GLsizei count, const GLchar *const *strings)
{
    const ShaderProgramID shaderID = PackParam<ShaderProgramID>(createShader(type));
    if (shaderID.value)
    {
        Shader *shaderObject = getShaderNoResolveCompile(shaderID);
        ASSERT(shaderObject);
        shaderObject->setSource(this, count, strings, nullptr);
        shaderObject->compile(this, angle::JobResultExpectancy::Immediate);
        const ShaderProgramID programID = PackParam<ShaderProgramID>(createProgram());
        if (programID.value)
        {
            gl::Program *programObject = getProgramNoResolveLink(programID);
            ASSERT(programObject);

            // Note: this call serializes the compilation with the following link.  For backends
            // that prefer parallel compile and link, it's more efficient to remove this check, and
            // let link fail instead.
            if (shaderObject->isCompiled(this))
            {
                // As per Khronos issue 2261:
                // https://gitlab.khronos.org/Tracker/vk-gl-cts/issues/2261
                // We must wait to mark the program separable until it's successfully compiled.
                programObject->setSeparable(this, true);

                programObject->attachShader(this, shaderObject);

                // Note: the result expectancy of this link could be turned to Future if
                // |detachShader| below is made not to resolve the link.
                if (programObject->link(this, angle::JobResultExpectancy::Immediate) !=
                    angle::Result::Continue)
                {
                    deleteShader(shaderID);
                    deleteProgram(programID);
                    return 0u;
                }

                programObject->detachShader(this, shaderObject);
            }

            InfoLog &programInfoLog = programObject->getInfoLog();
            programInfoLog << shaderObject->getInfoLogString();
        }

        deleteShader(shaderID);

        return programID.value;
    }

    return 0u;
}

MemoryObjectID Context::createMemoryObject()
{
    return mState.mMemoryObjectManager->createMemoryObject(mImplementation.get());
}

SemaphoreID Context::createSemaphore()
{
    return mState.mSemaphoreManager->createSemaphore(mImplementation.get());
}

void Context::deleteBuffer(BufferID bufferName)
{
    Buffer *buffer = mState.mBufferManager->getBuffer(bufferName);
    if (buffer)
    {
        detachBuffer(buffer);
    }

    mState.mBufferManager->deleteObject(this, bufferName);
}

void Context::deleteShader(ShaderProgramID shader)
{
    mState.mShaderProgramManager->deleteShader(this, shader);
}

void Context::deleteProgram(ShaderProgramID program)
{
    mState.mShaderProgramManager->deleteProgram(this, program);
}

void Context::deleteTexture(TextureID textureID)
{
    // If a texture object is deleted while its image is bound to a pixel local storage plane on the
    // currently bound draw framebuffer, and pixel local storage is active, then it is as if
    // EndPixelLocalStorageANGLE() had been called with <n>=PIXEL_LOCAL_STORAGE_ACTIVE_PLANES_ANGLE
    // and <storeops> of STORE_OP_STORE_ANGLE.
    if (mState.getPixelLocalStorageActivePlanes() != 0)
    {
        PixelLocalStorage *pls = mState.getDrawFramebuffer()->peekPixelLocalStorage();
        // Even though there is a nonzero number of active PLS planes, peekPixelLocalStorage() may
        // still return null if we are in the middle of deleting the active framebuffer.
        if (pls != nullptr)
        {
            for (GLuint i = 0; i < mState.getCaps().maxPixelLocalStoragePlanes; ++i)
            {
                if (pls->getPlane(i).getTextureID() == textureID)
                {
                    endPixelLocalStorageImplicit();
                    break;
                }
            }
        }
    }

    Texture *texture = mState.mTextureManager->getTexture(textureID);
    if (texture != nullptr)
    {
        texture->onStateChange(angle::SubjectMessage::TextureIDDeleted);
        detachTexture(textureID);
    }

    mState.mTextureManager->deleteObject(this, textureID);
}

void Context::deleteRenderbuffer(RenderbufferID renderbuffer)
{
    if (mState.mRenderbufferManager->getRenderbuffer(renderbuffer))
    {
        detachRenderbuffer(renderbuffer);
    }

    mState.mRenderbufferManager->deleteObject(this, renderbuffer);
}

void Context::deleteSync(SyncID syncPacked)
{
    // The spec specifies the underlying Fence object is not deleted until all current
    // wait commands finish. However, since the name becomes invalid, we cannot query the fence,
    // and since our API is currently designed for being called from a single thread, we can delete
    // the fence immediately.
    mState.mSyncManager->deleteObject(this, syncPacked);
}

void Context::deleteProgramPipeline(ProgramPipelineID pipelineID)
{
    ProgramPipeline *pipeline = mState.mProgramPipelineManager->getProgramPipeline(pipelineID);
    if (pipeline)
    {
        detachProgramPipeline(pipelineID);
    }

    mState.mProgramPipelineManager->deleteObject(this, pipelineID);
}

void Context::deleteMemoryObject(MemoryObjectID memoryObject)
{
    mState.mMemoryObjectManager->deleteMemoryObject(this, memoryObject);
}

void Context::deleteSemaphore(SemaphoreID semaphore)
{
    mState.mSemaphoreManager->deleteSemaphore(this, semaphore);
}

// GL_CHROMIUM_lose_context
void Context::loseContext(GraphicsResetStatus current, GraphicsResetStatus other)
{
    // TODO(geofflang): mark the rest of the share group lost. Requires access to the entire share
    // group from a context. http://anglebug.com/42262046
    markContextLost(current);
}

void Context::deleteFramebuffer(FramebufferID framebufferID)
{
    // We are responsible for deleting the GL objects from the Framebuffer's pixel local storage.
    std::unique_ptr<PixelLocalStorage> plsToDelete;

    Framebuffer *framebuffer = mState.mFramebufferManager->getFramebuffer(framebufferID);
    if (framebuffer != nullptr)
    {
        if (mState.getPixelLocalStorageActivePlanes() != 0 &&
            framebuffer == mState.getDrawFramebuffer())
        {
            endPixelLocalStorageImplicit();
        }
        plsToDelete = framebuffer->detachPixelLocalStorage();
        detachFramebuffer(framebufferID);
    }

    mState.mFramebufferManager->deleteObject(this, framebufferID);

    // Delete the pixel local storage GL objects after the framebuffer, in order to avoid any
    // potential trickyness with orphaning.
    if (plsToDelete != nullptr)
    {
        plsToDelete->deleteContextObjects(this);
    }
}

void Context::deleteFencesNV(GLsizei n, const FenceNVID *fences)
{
    for (int i = 0; i < n; i++)
    {
        FenceNVID fence = fences[i];

        FenceNV *fenceObject = nullptr;
        if (mFenceNVMap.erase(fence, &fenceObject))
        {
            mFenceNVHandleAllocator.release(fence.value);
            if (fenceObject)
            {
                fenceObject->onDestroy(this);
            }
            delete fenceObject;
        }
    }
}

Buffer *Context::getBuffer(BufferID handle) const
{
    return mState.mBufferManager->getBuffer(handle);
}

Renderbuffer *Context::getRenderbuffer(RenderbufferID handle) const
{
    return mState.mRenderbufferManager->getRenderbuffer(handle);
}

EGLenum Context::getContextPriority() const
{
    return egl::ToEGLenum(mImplementation->getContextPriority());
}

Sync *Context::getSync(SyncID syncPacked) const
{
    return mState.mSyncManager->getSync(syncPacked);
}

VertexArray *Context::getVertexArray(VertexArrayID handle) const
{
    return mVertexArrayMap.query(handle);
}

Sampler *Context::getSampler(SamplerID handle) const
{
    return mState.mSamplerManager->getSampler(handle);
}

TransformFeedback *Context::getTransformFeedback(TransformFeedbackID handle) const
{
    return mTransformFeedbackMap.query(handle);
}

ProgramPipeline *Context::getProgramPipeline(ProgramPipelineID handle) const
{
    return mState.mProgramPipelineManager->getProgramPipeline(handle);
}

gl::LabeledObject *Context::getLabeledObject(GLenum identifier, GLuint name) const
{
    switch (identifier)
    {
        case GL_BUFFER:
        case GL_BUFFER_OBJECT_EXT:
            return getBuffer({name});
        case GL_SHADER:
        case GL_SHADER_OBJECT_EXT:
            return getShaderNoResolveCompile({name});
        case GL_PROGRAM:
        case GL_PROGRAM_OBJECT_EXT:
            return getProgramNoResolveLink({name});
        case GL_VERTEX_ARRAY:
        case GL_VERTEX_ARRAY_OBJECT_EXT:
            return getVertexArray({name});
        case GL_QUERY:
        case GL_QUERY_OBJECT_EXT:
            return getQuery({name});
        case GL_TRANSFORM_FEEDBACK:
            return getTransformFeedback({name});
        case GL_SAMPLER:
            return getSampler({name});
        case GL_TEXTURE:
            return getTexture({name});
        case GL_RENDERBUFFER:
            return getRenderbuffer({name});
        case GL_FRAMEBUFFER:
            return getFramebuffer({name});
        case GL_PROGRAM_PIPELINE:
        case GL_PROGRAM_PIPELINE_OBJECT_EXT:
            return getProgramPipeline({name});
        default:
            UNREACHABLE();
            return nullptr;
    }
}

gl::LabeledObject *Context::getLabeledObjectFromPtr(const void *ptr) const
{
    return getSync({unsafe_pointer_to_int_cast<uint32_t>(ptr)});
}

void Context::objectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    gl::LabeledObject *object = getLabeledObject(identifier, name);
    ASSERT(object != nullptr);

    std::string labelName = GetObjectLabelFromPointer(length, label);
    ANGLE_CONTEXT_TRY(object->setLabel(this, labelName));

    // TODO(jmadill): Determine if the object is dirty based on 'name'. Conservatively assume the
    // specified object is active until we do this.
    mState.setObjectDirty(identifier);
}

void Context::labelObject(GLenum type, GLuint object, GLsizei length, const GLchar *label)
{
    gl::LabeledObject *obj = getLabeledObject(type, object);
    ASSERT(obj != nullptr);

    std::string labelName = "";
    if (label != nullptr)
    {
        size_t labelLength = length == 0 ? strlen(label) : length;
        labelName          = std::string(label, labelLength);
    }
    ANGLE_CONTEXT_TRY(obj->setLabel(this, labelName));
    mState.setObjectDirty(type);
}

void Context::objectPtrLabel(const void *ptr, GLsizei length, const GLchar *label)
{
    gl::LabeledObject *object = getLabeledObjectFromPtr(ptr);
    ASSERT(object != nullptr);

    std::string labelName = GetObjectLabelFromPointer(length, label);
    ANGLE_CONTEXT_TRY(object->setLabel(this, labelName));
}

void Context::getObjectLabel(GLenum identifier,
                             GLuint name,
                             GLsizei bufSize,
                             GLsizei *length,
                             GLchar *label)
{
    gl::LabeledObject *object = getLabeledObject(identifier, name);
    ASSERT(object != nullptr);

    const std::string &objectLabel = object->getLabel();
    GetObjectLabelBase(objectLabel, bufSize, length, label);
}

void Context::getObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    gl::LabeledObject *object = getLabeledObjectFromPtr(ptr);
    ASSERT(object != nullptr);

    const std::string &objectLabel = object->getLabel();
    GetObjectLabelBase(objectLabel, bufSize, length, label);
}

GLboolean Context::isSampler(SamplerID samplerName) const
{
    return mState.mSamplerManager->isSampler(samplerName);
}

void Context::bindTexture(TextureType target, TextureID handle)
{
    // Some apps enable KHR_create_context_no_error but pass in an invalid texture type.
    // Workaround this by silently returning in such situations.
    if (target == TextureType::InvalidEnum)
    {
        return;
    }

    Texture *texture = nullptr;
    if (handle.value == 0)
    {
        texture = mZeroTextures[target].get();
    }
    else
    {
        texture =
            mState.mTextureManager->checkTextureAllocation(mImplementation.get(), handle, target);
    }

    ASSERT(texture);
    // Early return if rebinding the same texture
    if (texture == mState.getTargetTexture(target))
    {
        return;
    }

    mState.setSamplerTexture(this, target, texture);
    mStateCache.onActiveTextureChange(this);
}

void Context::bindReadFramebuffer(FramebufferID framebufferHandle)
{
    Framebuffer *framebuffer = mState.mFramebufferManager->checkFramebufferAllocation(
        mImplementation.get(), this, framebufferHandle);
    mState.setReadFramebufferBinding(framebuffer);
    mReadFramebufferObserverBinding.bind(framebuffer);
}

void Context::bindDrawFramebuffer(FramebufferID framebufferHandle)
{
    endTilingImplicit();
    if (mState.getPixelLocalStorageActivePlanes() != 0)
    {
        endPixelLocalStorageImplicit();
    }
    Framebuffer *framebuffer = mState.mFramebufferManager->checkFramebufferAllocation(
        mImplementation.get(), this, framebufferHandle);
    mState.setDrawFramebufferBinding(framebuffer);
    mDrawFramebufferObserverBinding.bind(framebuffer);
    mStateCache.onDrawFramebufferChange(this);
}

void Context::bindVertexArray(VertexArrayID vertexArrayHandle)
{
    VertexArray *vertexArray = checkVertexArrayAllocation(vertexArrayHandle);
    mState.setVertexArrayBinding(this, vertexArray);
    mVertexArrayObserverBinding.bind(vertexArray);
    mStateCache.onVertexArrayBindingChange(this);
}

void Context::bindVertexBuffer(GLuint bindingIndex,
                               BufferID bufferHandle,
                               GLintptr offset,
                               GLsizei stride)
{
    Buffer *buffer =
        mState.mBufferManager->checkBufferAllocation(mImplementation.get(), bufferHandle);
    mState.bindVertexBuffer(this, bindingIndex, buffer, offset, stride);
    mStateCache.onVertexArrayStateChange(this);
}

void Context::bindSampler(GLuint textureUnit, SamplerID samplerHandle)
{
    ASSERT(textureUnit < static_cast<GLuint>(mState.getCaps().maxCombinedTextureImageUnits));
    Sampler *sampler =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), samplerHandle);

    // Early return if rebinding the same sampler
    if (sampler == mState.getSampler(textureUnit))
    {
        return;
    }

    mState.setSamplerBinding(this, textureUnit, sampler);
    mSamplerObserverBindings[textureUnit].bind(sampler);
    mStateCache.onActiveTextureChange(this);
}

void Context::bindImageTexture(GLuint unit,
                               TextureID texture,
                               GLint level,
                               GLboolean layered,
                               GLint layer,
                               GLenum access,
                               GLenum format)
{
    Texture *tex = mState.mTextureManager->getTexture(texture);
    mState.setImageUnit(this, unit, tex, level, layered, layer, access, format);
    mImageObserverBindings[unit].bind(tex);
}

void Context::useProgram(ShaderProgramID program)
{
    Program *programObject = getProgramResolveLink(program);
    ANGLE_CONTEXT_TRY(mState.setProgram(this, programObject));
    mStateCache.onProgramExecutableChange(this);
    mProgramObserverBinding.bind(programObject);
}

void Context::useProgramStages(ProgramPipelineID pipeline,
                               GLbitfield stages,
                               ShaderProgramID program)
{
    Program *shaderProgram = getProgramNoResolveLink(program);
    ProgramPipeline *programPipeline =
        mState.mProgramPipelineManager->checkProgramPipelineAllocation(mImplementation.get(),
                                                                       pipeline);

    ASSERT(programPipeline);
    ANGLE_CONTEXT_TRY(programPipeline->useProgramStages(this, stages, shaderProgram));
}

void Context::bindTransformFeedback(GLenum target, TransformFeedbackID transformFeedbackHandle)
{
    ASSERT(target == GL_TRANSFORM_FEEDBACK);
    TransformFeedback *transformFeedback =
        checkTransformFeedbackAllocation(transformFeedbackHandle);
    mState.setTransformFeedbackBinding(this, transformFeedback);
    mStateCache.onActiveTransformFeedbackChange(this);
}

void Context::bindProgramPipeline(ProgramPipelineID pipelineHandle)
{
    ProgramPipeline *pipeline = mState.mProgramPipelineManager->checkProgramPipelineAllocation(
        mImplementation.get(), pipelineHandle);
    ANGLE_CONTEXT_TRY(mState.setProgramPipelineBinding(this, pipeline));
    mStateCache.onProgramExecutableChange(this);
    mProgramPipelineObserverBinding.bind(pipeline);
}

void Context::beginQuery(QueryType target, QueryID query)
{
    Query *queryObject = getOrCreateQuery(query, target);
    ASSERT(queryObject);

    // begin query
    ANGLE_CONTEXT_TRY(queryObject->begin(this));

    // set query as active for specified target only if begin succeeded
    mState.setActiveQuery(this, target, queryObject);
    mStateCache.onQueryChange(this);
}

void Context::endQuery(QueryType target)
{
    Query *queryObject = mState.getActiveQuery(target);
    ASSERT(queryObject);

    // Intentionally don't call try here. We don't want an early return.
    (void)(queryObject->end(this));

    // Always unbind the query, even if there was an error. This may delete the query object.
    mState.setActiveQuery(this, target, nullptr);
    mStateCache.onQueryChange(this);
}

void Context::queryCounter(QueryID id, QueryType target)
{
    ASSERT(target == QueryType::Timestamp);

    Query *queryObject = getOrCreateQuery(id, target);
    ASSERT(queryObject);

    ANGLE_CONTEXT_TRY(queryObject->queryCounter(this));
}

void Context::getQueryiv(QueryType target, GLenum pname, GLint *params)
{
    switch (pname)
    {
        case GL_CURRENT_QUERY_EXT:
            params[0] = mState.getActiveQueryId(target).value;
            break;
        case GL_QUERY_COUNTER_BITS_EXT:
            switch (target)
            {
                case QueryType::TimeElapsed:
                    params[0] = getCaps().queryCounterBitsTimeElapsed;
                    break;
                case QueryType::Timestamp:
                    params[0] = getCaps().queryCounterBitsTimestamp;
                    break;
                default:
                    UNREACHABLE();
                    params[0] = 0;
                    break;
            }
            break;
        default:
            UNREACHABLE();
            return;
    }
}

void Context::getQueryivRobust(QueryType target,
                               GLenum pname,
                               GLsizei bufSize,
                               GLsizei *length,
                               GLint *params)
{
    getQueryiv(target, pname, params);
}

void Context::getUnsignedBytev(GLenum pname, GLubyte *data)
{
    UNIMPLEMENTED();
}

void Context::getUnsignedBytei_v(GLenum target, GLuint index, GLubyte *data)
{
    UNIMPLEMENTED();
}

void Context::getQueryObjectiv(QueryID id, GLenum pname, GLint *params)
{
    ANGLE_CONTEXT_TRY(GetQueryObjectParameter(this, getQuery(id), pname, params));
}

void Context::getQueryObjectivRobust(QueryID id,
                                     GLenum pname,
                                     GLsizei bufSize,
                                     GLsizei *length,
                                     GLint *params)
{
    getQueryObjectiv(id, pname, params);
}

void Context::getQueryObjectuiv(QueryID id, GLenum pname, GLuint *params)
{
    ANGLE_CONTEXT_TRY(GetQueryObjectParameter(this, getQuery(id), pname, params));
}

void Context::getQueryObjectuivRobust(QueryID id,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      GLsizei *length,
                                      GLuint *params)
{
    getQueryObjectuiv(id, pname, params);
}

void Context::getQueryObjecti64v(QueryID id, GLenum pname, GLint64 *params)
{
    ANGLE_CONTEXT_TRY(GetQueryObjectParameter(this, getQuery(id), pname, params));
}

void Context::getQueryObjecti64vRobust(QueryID id,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       GLsizei *length,
                                       GLint64 *params)
{
    getQueryObjecti64v(id, pname, params);
}

void Context::getQueryObjectui64v(QueryID id, GLenum pname, GLuint64 *params)
{
    ANGLE_CONTEXT_TRY(GetQueryObjectParameter(this, getQuery(id), pname, params));
}

void Context::getQueryObjectui64vRobust(QueryID id,
                                        GLenum pname,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLuint64 *params)
{
    getQueryObjectui64v(id, pname, params);
}

Framebuffer *Context::getFramebuffer(FramebufferID handle) const
{
    return mState.mFramebufferManager->getFramebuffer(handle);
}

FenceNV *Context::getFenceNV(FenceNVID handle) const
{
    return mFenceNVMap.query(handle);
}

Query *Context::getOrCreateQuery(QueryID handle, QueryType type)
{
    if (!mQueryMap.contains(handle))
    {
        return nullptr;
    }

    Query *query = mQueryMap.query(handle);
    if (!query)
    {
        ASSERT(type != QueryType::InvalidEnum);
        query = new Query(mImplementation.get(), type, handle);
        query->addRef();
        mQueryMap.assign(handle, query);
    }
    return query;
}

Query *Context::getQuery(QueryID handle) const
{
    return mQueryMap.query(handle);
}

Texture *Context::getTextureByType(TextureType type) const
{
    ASSERT(ValidTextureTarget(this, type) || ValidTextureExternalTarget(this, type));
    return mState.getTargetTexture(type);
}

Texture *Context::getTextureByTarget(TextureTarget target) const
{
    return getTextureByType(TextureTargetToType(target));
}

Texture *Context::getSamplerTexture(unsigned int sampler, TextureType type) const
{
    return mState.getSamplerTexture(sampler, type);
}

Compiler *Context::getCompiler() const
{
    if (mCompiler.get() == nullptr)
    {
        mCompiler.set(this, new Compiler(mImplementation.get(), mState, mDisplay));
    }
    return mCompiler.get();
}

void Context::getBooleanvImpl(GLenum pname, GLboolean *params) const
{
    switch (pname)
    {
        case GL_SHADER_COMPILER:
            *params = GL_TRUE;
            break;
        case GL_CONTEXT_ROBUST_ACCESS_EXT:
            *params = ConvertToGLBoolean(mState.hasRobustAccess());
            break;

        default:
            mState.getBooleanv(pname, params);
            break;
    }
}

void Context::getFloatvImpl(GLenum pname, GLfloat *params) const
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.
    switch (pname)
    {
        case GL_ALIASED_LINE_WIDTH_RANGE:
            params[0] = mState.getCaps().minAliasedLineWidth;
            params[1] = mState.getCaps().maxAliasedLineWidth;
            break;
        case GL_ALIASED_POINT_SIZE_RANGE:
            params[0] = mState.getCaps().minAliasedPointSize;
            params[1] = mState.getCaps().maxAliasedPointSize;
            break;
        case GL_SMOOTH_POINT_SIZE_RANGE:
            params[0] = mState.getCaps().minSmoothPointSize;
            params[1] = mState.getCaps().maxSmoothPointSize;
            break;
        case GL_SMOOTH_LINE_WIDTH_RANGE:
            params[0] = mState.getCaps().minSmoothLineWidth;
            params[1] = mState.getCaps().maxSmoothLineWidth;
            break;
        case GL_MULTISAMPLE_LINE_WIDTH_RANGE:
            params[0] = mState.getCaps().minMultisampleLineWidth;
            params[1] = mState.getCaps().maxMultisampleLineWidth;
            break;
        case GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY:
            *params = mState.getCaps().lineWidthGranularity;
            break;
        case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
            ASSERT(mState.getExtensions().textureFilterAnisotropicEXT);
            *params = mState.getCaps().maxTextureAnisotropy;
            break;
        case GL_MAX_TEXTURE_LOD_BIAS:
            *params = mState.getCaps().maxLODBias;
            break;
        case GL_MIN_FRAGMENT_INTERPOLATION_OFFSET:
            *params = mState.getCaps().minInterpolationOffset;
            break;
        case GL_MAX_FRAGMENT_INTERPOLATION_OFFSET:
            *params = mState.getCaps().maxInterpolationOffset;
            break;
        case GL_PRIMITIVE_BOUNDING_BOX:
            params[0] = mState.getBoundingBoxMinX();
            params[1] = mState.getBoundingBoxMinY();
            params[2] = mState.getBoundingBoxMinZ();
            params[3] = mState.getBoundingBoxMinW();
            params[4] = mState.getBoundingBoxMaxX();
            params[5] = mState.getBoundingBoxMaxY();
            params[6] = mState.getBoundingBoxMaxZ();
            params[7] = mState.getBoundingBoxMaxW();
            break;
        default:
            mState.getFloatv(pname, params);
            break;
    }
}

void Context::getIntegervImpl(GLenum pname, GLint *params) const
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.

    switch (pname)
    {
        case GL_MAX_VERTEX_ATTRIBS:
            *params = mState.getCaps().maxVertexAttributes;
            break;
        case GL_MAX_VERTEX_UNIFORM_VECTORS:
            *params = mState.getCaps().maxVertexUniformVectors;
            break;
        case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
            *params = mState.getCaps().maxShaderUniformComponents[ShaderType::Vertex];
            break;
        case GL_MAX_VARYING_VECTORS:
            *params = mState.getCaps().maxVaryingVectors;
            break;
        case GL_MAX_VARYING_COMPONENTS:
            *params = mState.getCaps().maxVaryingVectors * 4;
            break;
        case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
            *params = mState.getCaps().maxCombinedTextureImageUnits;
            break;
        case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
            *params = mState.getCaps().maxShaderTextureImageUnits[ShaderType::Vertex];
            break;
        case GL_MAX_TEXTURE_IMAGE_UNITS:
            *params = mState.getCaps().maxShaderTextureImageUnits[ShaderType::Fragment];
            break;
        case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
            *params = mState.getCaps().maxFragmentUniformVectors;
            break;
        case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
            *params = mState.getCaps().maxShaderUniformComponents[ShaderType::Fragment];
            break;
        case GL_MAX_RENDERBUFFER_SIZE:
            *params = mState.getCaps().maxRenderbufferSize;
            break;
        case GL_MAX_COLOR_ATTACHMENTS_EXT:
            *params = mState.getCaps().maxColorAttachments;
            break;
        case GL_MAX_DRAW_BUFFERS_EXT:
            *params = mState.getCaps().maxDrawBuffers;
            break;
        case GL_SUBPIXEL_BITS:
            *params = mState.getCaps().subPixelBits;
            break;
        case GL_MAX_TEXTURE_SIZE:
            *params = mState.getCaps().max2DTextureSize;
            break;
        case GL_MAX_RECTANGLE_TEXTURE_SIZE_ANGLE:
            *params = mState.getCaps().maxRectangleTextureSize;
            break;
        case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
            *params = mState.getCaps().maxCubeMapTextureSize;
            break;
        case GL_MAX_3D_TEXTURE_SIZE:
            *params = mState.getCaps().max3DTextureSize;
            break;
        case GL_MAX_ARRAY_TEXTURE_LAYERS:
            *params = mState.getCaps().maxArrayTextureLayers;
            break;
        case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
            *params = mState.getCaps().uniformBufferOffsetAlignment;
            break;
        case GL_MAX_UNIFORM_BUFFER_BINDINGS:
            *params = mState.getCaps().maxUniformBufferBindings;
            break;
        case GL_MAX_VERTEX_UNIFORM_BLOCKS:
            *params = mState.getCaps().maxShaderUniformBlocks[ShaderType::Vertex];
            break;
        case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
            *params = mState.getCaps().maxShaderUniformBlocks[ShaderType::Fragment];
            break;
        case GL_MAX_COMBINED_UNIFORM_BLOCKS:
            *params = mState.getCaps().maxCombinedUniformBlocks;
            break;
        case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
            *params = mState.getCaps().maxVertexOutputComponents;
            break;
        case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
            *params = mState.getCaps().maxFragmentInputComponents;
            break;
        case GL_MIN_PROGRAM_TEXEL_OFFSET:
            *params = mState.getCaps().minProgramTexelOffset;
            break;
        case GL_MAX_PROGRAM_TEXEL_OFFSET:
            *params = mState.getCaps().maxProgramTexelOffset;
            break;
        case GL_MAJOR_VERSION:
            *params = getClientVersion().major;
            break;
        case GL_MINOR_VERSION:
            *params = getClientVersion().minor;
            break;
        case GL_MAX_ELEMENTS_INDICES:
            *params = mState.getCaps().maxElementsIndices;
            break;
        case GL_MAX_ELEMENTS_VERTICES:
            *params = mState.getCaps().maxElementsVertices;
            break;
        case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
            *params = mState.getCaps().maxTransformFeedbackInterleavedComponents;
            break;
        case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
            *params = mState.getCaps().maxTransformFeedbackSeparateAttributes;
            break;
        case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
            *params = mState.getCaps().maxTransformFeedbackSeparateComponents;
            break;
        case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
            *params = static_cast<GLint>(mState.getCaps().compressedTextureFormats.size());
            break;
        case GL_MAX_SAMPLES_ANGLE:
            *params = mState.getCaps().maxSamples;
            break;
        case GL_MAX_VIEWPORT_DIMS:
        {
            params[0] = mState.getCaps().maxViewportWidth;
            params[1] = mState.getCaps().maxViewportHeight;
        }
        break;
        case GL_COMPRESSED_TEXTURE_FORMATS:
            std::copy(mState.getCaps().compressedTextureFormats.begin(),
                      mState.getCaps().compressedTextureFormats.end(), params);
            break;
        case GL_RESET_NOTIFICATION_STRATEGY_EXT:
            *params = mErrors.getResetStrategy();
            break;
        case GL_NUM_SHADER_BINARY_FORMATS:
            *params = static_cast<GLint>(mState.getCaps().shaderBinaryFormats.size());
            break;
        case GL_SHADER_BINARY_FORMATS:
            std::copy(mState.getCaps().shaderBinaryFormats.begin(),
                      mState.getCaps().shaderBinaryFormats.end(), params);
            break;
        case GL_NUM_PROGRAM_BINARY_FORMATS:
            *params = static_cast<GLint>(mState.getCaps().programBinaryFormats.size());
            break;
        case GL_PROGRAM_BINARY_FORMATS:
            std::copy(mState.getCaps().programBinaryFormats.begin(),
                      mState.getCaps().programBinaryFormats.end(), params);
            break;
        case GL_NUM_EXTENSIONS:
            *params = static_cast<GLint>(mExtensionStrings.size());
            break;

        // GLES3.2 client flags
        case GL_CONTEXT_FLAGS:
        {
            GLint contextFlags = 0;
            if (mState.hasProtectedContent())
            {
                contextFlags |= GL_CONTEXT_FLAG_PROTECTED_CONTENT_BIT_EXT;
            }

            if (mState.isDebugContext())
            {
                contextFlags |= GL_CONTEXT_FLAG_DEBUG_BIT_KHR;
            }

            if (mState.hasRobustAccess())
            {
                contextFlags |= GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT;
            }
            *params = contextFlags;
        }
        break;

        // GL_ANGLE_request_extension
        case GL_NUM_REQUESTABLE_EXTENSIONS_ANGLE:
            *params = static_cast<GLint>(mRequestableExtensionStrings.size());
            break;

        // GL_KHR_debug
        case GL_MAX_DEBUG_MESSAGE_LENGTH:
            *params = mState.getCaps().maxDebugMessageLength;
            break;
        case GL_MAX_DEBUG_LOGGED_MESSAGES:
            *params = mState.getCaps().maxDebugLoggedMessages;
            break;
        case GL_MAX_DEBUG_GROUP_STACK_DEPTH:
            *params = mState.getCaps().maxDebugGroupStackDepth;
            break;
        case GL_MAX_LABEL_LENGTH:
            *params = mState.getCaps().maxLabelLength;
            break;

        // GL_OVR_multiview2
        case GL_MAX_VIEWS_OVR:
            *params = mState.getCaps().maxViews;
            break;

        // GL_EXT_disjoint_timer_query
        case GL_GPU_DISJOINT_EXT:
            *params = mImplementation->getGPUDisjoint();
            break;
        case GL_MAX_FRAMEBUFFER_WIDTH:
            *params = mState.getCaps().maxFramebufferWidth;
            break;
        case GL_MAX_FRAMEBUFFER_HEIGHT:
            *params = mState.getCaps().maxFramebufferHeight;
            break;
        case GL_MAX_FRAMEBUFFER_SAMPLES:
            *params = mState.getCaps().maxFramebufferSamples;
            break;
        case GL_MAX_SAMPLE_MASK_WORDS:
            *params = mState.getCaps().maxSampleMaskWords;
            break;
        case GL_MAX_COLOR_TEXTURE_SAMPLES:
            *params = mState.getCaps().maxColorTextureSamples;
            break;
        case GL_MAX_DEPTH_TEXTURE_SAMPLES:
            *params = mState.getCaps().maxDepthTextureSamples;
            break;
        case GL_MAX_INTEGER_SAMPLES:
            *params = mState.getCaps().maxIntegerSamples;
            break;
        case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
            *params = mState.getCaps().maxVertexAttribRelativeOffset;
            break;
        case GL_MAX_VERTEX_ATTRIB_BINDINGS:
            *params = mState.getCaps().maxVertexAttribBindings;
            break;
        case GL_MAX_VERTEX_ATTRIB_STRIDE:
            *params = mState.getCaps().maxVertexAttribStride;
            break;
        case GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS:
            *params = mState.getCaps().maxShaderAtomicCounterBuffers[ShaderType::Vertex];
            break;
        case GL_MAX_VERTEX_ATOMIC_COUNTERS:
            *params = mState.getCaps().maxShaderAtomicCounters[ShaderType::Vertex];
            break;
        case GL_MAX_VERTEX_IMAGE_UNIFORMS:
            *params = mState.getCaps().maxShaderImageUniforms[ShaderType::Vertex];
            break;
        case GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS:
            *params = mState.getCaps().maxShaderStorageBlocks[ShaderType::Vertex];
            break;
        case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
            *params = mState.getCaps().maxShaderAtomicCounterBuffers[ShaderType::Fragment];
            break;
        case GL_MAX_FRAGMENT_ATOMIC_COUNTERS:
            *params = mState.getCaps().maxShaderAtomicCounters[ShaderType::Fragment];
            break;
        case GL_MAX_FRAGMENT_IMAGE_UNIFORMS:
            *params = mState.getCaps().maxShaderImageUniforms[ShaderType::Fragment];
            break;
        case GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS:
            *params = mState.getCaps().maxShaderStorageBlocks[ShaderType::Fragment];
            break;
        case GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET:
            *params = mState.getCaps().minProgramTextureGatherOffset;
            break;
        case GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET:
            *params = mState.getCaps().maxProgramTextureGatherOffset;
            break;
        case GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
            *params = mState.getCaps().maxComputeWorkGroupInvocations;
            break;
        case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
            *params = mState.getCaps().maxShaderUniformBlocks[ShaderType::Compute];
            break;
        case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
            *params = mState.getCaps().maxShaderTextureImageUnits[ShaderType::Compute];
            break;
        case GL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
            *params = mState.getCaps().maxComputeSharedMemorySize;
            break;
        case GL_MAX_COMPUTE_UNIFORM_COMPONENTS:
            *params = mState.getCaps().maxShaderUniformComponents[ShaderType::Compute];
            break;
        case GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS:
            *params = mState.getCaps().maxShaderAtomicCounterBuffers[ShaderType::Compute];
            break;
        case GL_MAX_COMPUTE_ATOMIC_COUNTERS:
            *params = mState.getCaps().maxShaderAtomicCounters[ShaderType::Compute];
            break;
        case GL_MAX_COMPUTE_IMAGE_UNIFORMS:
            *params = mState.getCaps().maxShaderImageUniforms[ShaderType::Compute];
            break;
        case GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS:
            *params = static_cast<GLint>(
                mState.getCaps().maxCombinedShaderUniformComponents[ShaderType::Compute]);
            break;
        case GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS:
            *params = mState.getCaps().maxShaderStorageBlocks[ShaderType::Compute];
            break;
        case GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
            *params = mState.getCaps().maxCombinedShaderOutputResources;
            break;
        case GL_MAX_UNIFORM_LOCATIONS:
            *params = mState.getCaps().maxUniformLocations;
            break;
        case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
            *params = mState.getCaps().maxAtomicCounterBufferBindings;
            break;
        case GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE:
            *params = mState.getCaps().maxAtomicCounterBufferSize;
            break;
        case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
            *params = mState.getCaps().maxCombinedAtomicCounterBuffers;
            break;
        case GL_MAX_COMBINED_ATOMIC_COUNTERS:
            *params = mState.getCaps().maxCombinedAtomicCounters;
            break;
        case GL_MAX_IMAGE_UNITS:
            *params = mState.getCaps().maxImageUnits;
            break;
        case GL_MAX_COMBINED_IMAGE_UNIFORMS:
            *params = mState.getCaps().maxCombinedImageUniforms;
            break;
        case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS:
            *params = mState.getCaps().maxShaderStorageBufferBindings;
            break;
        case GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS:
            *params = mState.getCaps().maxCombinedShaderStorageBlocks;
            break;
        case GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT:
            *params = mState.getCaps().shaderStorageBufferOffsetAlignment;
            break;

        // GL_EXT_geometry_shader
        case GL_MAX_FRAMEBUFFER_LAYERS_EXT:
            *params = mState.getCaps().maxFramebufferLayers;
            break;
        case GL_LAYER_PROVOKING_VERTEX_EXT:
            *params = mState.getCaps().layerProvokingVertex;
            break;
        case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT:
            *params = mState.getCaps().maxShaderUniformComponents[ShaderType::Geometry];
            break;
        case GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT:
            *params = mState.getCaps().maxShaderUniformBlocks[ShaderType::Geometry];
            break;
        case GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT:
            *params = static_cast<GLint>(
                mState.getCaps().maxCombinedShaderUniformComponents[ShaderType::Geometry]);
            break;
        case GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxGeometryInputComponents;
            break;
        case GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxGeometryOutputComponents;
            break;
        case GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT:
            *params = mState.getCaps().maxGeometryOutputVertices;
            break;
        case GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxGeometryTotalOutputComponents;
            break;
        case GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT:
            *params = mState.getCaps().maxGeometryShaderInvocations;
            break;
        case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT:
            *params = mState.getCaps().maxShaderTextureImageUnits[ShaderType::Geometry];
            break;
        case GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT:
            *params = mState.getCaps().maxShaderAtomicCounterBuffers[ShaderType::Geometry];
            break;
        case GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT:
            *params = mState.getCaps().maxShaderAtomicCounters[ShaderType::Geometry];
            break;
        case GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT:
            *params = mState.getCaps().maxShaderImageUniforms[ShaderType::Geometry];
            break;
        case GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT:
            *params = mState.getCaps().maxShaderStorageBlocks[ShaderType::Geometry];
            break;
        // GL_EXT_tessellation_shader
        case GL_MAX_PATCH_VERTICES_EXT:
            *params = mState.getCaps().maxPatchVertices;
            break;
        case GL_MAX_TESS_GEN_LEVEL_EXT:
            *params = mState.getCaps().maxTessGenLevel;
            break;
        case GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_EXT:
            *params = mState.getCaps().maxShaderUniformComponents[ShaderType::TessControl];
            break;
        case GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT:
            *params = mState.getCaps().maxShaderUniformComponents[ShaderType::TessEvaluation];
            break;
        case GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_EXT:
            *params = mState.getCaps().maxShaderTextureImageUnits[ShaderType::TessControl];
            break;
        case GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_EXT:
            *params = mState.getCaps().maxShaderTextureImageUnits[ShaderType::TessEvaluation];
            break;
        case GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxTessControlOutputComponents;
            break;
        case GL_MAX_TESS_PATCH_COMPONENTS_EXT:
            *params = mState.getCaps().maxTessPatchComponents;
            break;
        case GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxTessControlTotalOutputComponents;
            break;
        case GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxTessEvaluationOutputComponents;
            break;
        case GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_EXT:
            *params = mState.getCaps().maxShaderUniformBlocks[ShaderType::TessControl];
            break;
        case GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_EXT:
            *params = mState.getCaps().maxShaderUniformBlocks[ShaderType::TessEvaluation];
            break;
        case GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxTessControlInputComponents;
            break;
        case GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_EXT:
            *params = mState.getCaps().maxTessEvaluationInputComponents;
            break;
        case GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_EXT:
            *params = static_cast<GLint>(
                mState.getCaps().maxCombinedShaderUniformComponents[ShaderType::TessControl]);
            break;
        case GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT:
            *params = static_cast<GLint>(
                mState.getCaps().maxCombinedShaderUniformComponents[ShaderType::TessEvaluation]);
            break;
        case GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_EXT:
            *params = mState.getCaps().maxShaderAtomicCounterBuffers[ShaderType::TessControl];
            break;
        case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_EXT:
            *params = mState.getCaps().maxShaderAtomicCounterBuffers[ShaderType::TessEvaluation];
            break;
        case GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_EXT:
            *params = mState.getCaps().maxShaderAtomicCounters[ShaderType::TessControl];
            break;
        case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_EXT:
            *params = mState.getCaps().maxShaderAtomicCounters[ShaderType::TessEvaluation];
            break;
        case GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_EXT:
            *params = mState.getCaps().maxShaderImageUniforms[ShaderType::TessControl];
            break;
        case GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_EXT:
            *params = mState.getCaps().maxShaderImageUniforms[ShaderType::TessEvaluation];
            break;
        case GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_EXT:
            *params = mState.getCaps().maxShaderStorageBlocks[ShaderType::TessControl];
            break;
        case GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_EXT:
            *params = mState.getCaps().maxShaderStorageBlocks[ShaderType::TessEvaluation];
            break;
        // GLES1 emulation: Caps queries
        case GL_MAX_TEXTURE_UNITS:
            *params = mState.getCaps().maxMultitextureUnits;
            break;
        case GL_MAX_MODELVIEW_STACK_DEPTH:
            *params = mState.getCaps().maxModelviewMatrixStackDepth;
            break;
        case GL_MAX_PROJECTION_STACK_DEPTH:
            *params = mState.getCaps().maxProjectionMatrixStackDepth;
            break;
        case GL_MAX_TEXTURE_STACK_DEPTH:
            *params = mState.getCaps().maxTextureMatrixStackDepth;
            break;
        case GL_MAX_LIGHTS:
            *params = mState.getCaps().maxLights;
            break;

        // case GL_MAX_CLIP_DISTANCES_EXT:  Conflict enum value
        case GL_MAX_CLIP_PLANES:
            if (getClientVersion().major >= 2)
            {
                // GL_APPLE_clip_distance / GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance
                *params = mState.getCaps().maxClipDistances;
            }
            else
            {
                *params = mState.getCaps().maxClipPlanes;
            }
            break;
        case GL_MAX_CULL_DISTANCES_EXT:
            *params = mState.getCaps().maxCullDistances;
            break;
        case GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES_EXT:
            *params = mState.getCaps().maxCombinedClipAndCullDistances;
            break;
        // GLES1 emulation: Vertex attribute queries
        case GL_VERTEX_ARRAY_BUFFER_BINDING:
        case GL_NORMAL_ARRAY_BUFFER_BINDING:
        case GL_COLOR_ARRAY_BUFFER_BINDING:
        case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
            getIntegerVertexAttribImpl(pname, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, params);
            break;
        case GL_VERTEX_ARRAY_STRIDE:
        case GL_NORMAL_ARRAY_STRIDE:
        case GL_COLOR_ARRAY_STRIDE:
        case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        case GL_TEXTURE_COORD_ARRAY_STRIDE:
            getIntegerVertexAttribImpl(pname, GL_VERTEX_ATTRIB_ARRAY_STRIDE, params);
            break;
        case GL_VERTEX_ARRAY_SIZE:
        case GL_COLOR_ARRAY_SIZE:
        case GL_TEXTURE_COORD_ARRAY_SIZE:
            getIntegerVertexAttribImpl(pname, GL_VERTEX_ATTRIB_ARRAY_SIZE, params);
            break;
        case GL_VERTEX_ARRAY_TYPE:
        case GL_COLOR_ARRAY_TYPE:
        case GL_NORMAL_ARRAY_TYPE:
        case GL_POINT_SIZE_ARRAY_TYPE_OES:
        case GL_TEXTURE_COORD_ARRAY_TYPE:
            getIntegerVertexAttribImpl(pname, GL_VERTEX_ATTRIB_ARRAY_TYPE, params);
            break;

        // GL_KHR_parallel_shader_compile
        case GL_MAX_SHADER_COMPILER_THREADS_KHR:
            *params = mState.getMaxShaderCompilerThreads();
            break;

        // GL_EXT_blend_func_extended
        case GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT:
            *params = mState.getCaps().maxDualSourceDrawBuffers;
            break;

        // OES_shader_multisample_interpolation
        case GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES:
            *params = mState.getCaps().subPixelInterpolationOffsetBits;
            break;

        // GL_OES_texture_buffer
        case GL_MAX_TEXTURE_BUFFER_SIZE:
            *params = mState.getCaps().maxTextureBufferSize;
            break;
        case GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
            *params = mState.getCaps().textureBufferOffsetAlignment;
            break;

        // ANGLE_shader_pixel_local_storage
        case GL_MAX_PIXEL_LOCAL_STORAGE_PLANES_ANGLE:
            *params = mState.getCaps().maxPixelLocalStoragePlanes;
            break;
        case GL_MAX_COLOR_ATTACHMENTS_WITH_ACTIVE_PIXEL_LOCAL_STORAGE_ANGLE:
            *params = mState.getCaps().maxColorAttachmentsWithActivePixelLocalStorage;
            break;
        case GL_MAX_COMBINED_DRAW_BUFFERS_AND_PIXEL_LOCAL_STORAGE_PLANES_ANGLE:
            *params = mState.getCaps().maxCombinedDrawBuffersAndPixelLocalStoragePlanes;
            break;

        case GL_QUERY_COUNTER_BITS_EXT:
            *params = mState.getCaps().queryCounterBitsTimestamp;
            break;

        default:
            ANGLE_CONTEXT_TRY(mState.getIntegerv(this, pname, params));
            break;
    }
}

void Context::getIntegerVertexAttribImpl(GLenum pname, GLenum attribpname, GLint *params) const
{
    getVertexAttribivImpl(static_cast<GLuint>(vertexArrayIndex(ParamToVertexArrayType(pname))),
                          attribpname, params);
}

void Context::getInteger64vImpl(GLenum pname, GLint64 *params) const
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.
    switch (pname)
    {
        case GL_MAX_ELEMENT_INDEX:
            *params = mState.getCaps().maxElementIndex;
            break;
        case GL_MAX_UNIFORM_BLOCK_SIZE:
            *params = mState.getCaps().maxUniformBlockSize;
            break;
        case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
            *params = mState.getCaps().maxCombinedShaderUniformComponents[ShaderType::Vertex];
            break;
        case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
            *params = mState.getCaps().maxCombinedShaderUniformComponents[ShaderType::Fragment];
            break;
        case GL_MAX_SERVER_WAIT_TIMEOUT:
            *params = mState.getCaps().maxServerWaitTimeout;
            break;

        // GL_EXT_disjoint_timer_query
        case GL_TIMESTAMP_EXT:
            *params = mImplementation->getTimestamp();
            break;

        case GL_MAX_SHADER_STORAGE_BLOCK_SIZE:
            *params = mState.getCaps().maxShaderStorageBlockSize;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void Context::getPointerv(GLenum pname, void **params)
{
    mState.getPointerv(this, pname, params);
}

void Context::getPointervRobustANGLERobust(GLenum pname,
                                           GLsizei bufSize,
                                           GLsizei *length,
                                           void **params)
{
    UNIMPLEMENTED();
}

void Context::getIntegeri_v(GLenum target, GLuint index, GLint *data)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.

    GLenum nativeType;
    unsigned int numParams;
    bool queryStatus = getIndexedQueryParameterInfo(target, &nativeType, &numParams);
    ASSERT(queryStatus);

    if (nativeType == GL_INT)
    {
        switch (target)
        {
            case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
                ASSERT(index < 3u);
                *data = mState.getCaps().maxComputeWorkGroupCount[index];
                break;
            case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
                ASSERT(index < 3u);
                *data = mState.getCaps().maxComputeWorkGroupSize[index];
                break;
            default:
                mState.getIntegeri_v(this, target, index, data);
        }
    }
    else
    {
        CastIndexedStateValues(this, nativeType, target, index, numParams, data);
    }
}

void Context::getIntegeri_vRobust(GLenum target,
                                  GLuint index,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLint *data)
{
    getIntegeri_v(target, index, data);
}

void Context::getInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.

    GLenum nativeType;
    unsigned int numParams;
    bool queryStatus = getIndexedQueryParameterInfo(target, &nativeType, &numParams);
    ASSERT(queryStatus);

    if (nativeType == GL_INT_64_ANGLEX)
    {
        mState.getInteger64i_v(target, index, data);
    }
    else
    {
        CastIndexedStateValues(this, nativeType, target, index, numParams, data);
    }
}

void Context::getInteger64i_vRobust(GLenum target,
                                    GLuint index,
                                    GLsizei bufSize,
                                    GLsizei *length,
                                    GLint64 *data)
{
    getInteger64i_v(target, index, data);
}

void Context::getBooleani_v(GLenum target, GLuint index, GLboolean *data)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.

    GLenum nativeType;
    unsigned int numParams;
    bool queryStatus = getIndexedQueryParameterInfo(target, &nativeType, &numParams);
    ASSERT(queryStatus);

    if (nativeType == GL_BOOL)
    {
        mState.getBooleani_v(target, index, data);
    }
    else
    {
        CastIndexedStateValues(this, nativeType, target, index, numParams, data);
    }
}

void Context::getBooleani_vRobust(GLenum target,
                                  GLuint index,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLboolean *data)
{
    getBooleani_v(target, index, data);
}

void Context::getBufferParameteriv(BufferBinding target, GLenum pname, GLint *params)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    QueryBufferParameteriv(buffer, pname, params);
}

void Context::getBufferParameterivRobust(BufferBinding target,
                                         GLenum pname,
                                         GLsizei bufSize,
                                         GLsizei *length,
                                         GLint *params)
{
    getBufferParameteriv(target, pname, params);
}

void Context::getFramebufferAttachmentParameteriv(GLenum target,
                                                  GLenum attachment,
                                                  GLenum pname,
                                                  GLint *params)
{
    const Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    QueryFramebufferAttachmentParameteriv(this, framebuffer, attachment, pname, params);
}

void Context::getFramebufferAttachmentParameterivRobust(GLenum target,
                                                        GLenum attachment,
                                                        GLenum pname,
                                                        GLsizei bufSize,
                                                        GLsizei *length,
                                                        GLint *params)
{
    getFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void Context::getRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    Renderbuffer *renderbuffer = mState.getCurrentRenderbuffer();
    QueryRenderbufferiv(this, renderbuffer, pname, params);
}

void Context::getRenderbufferParameterivRobust(GLenum target,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               GLsizei *length,
                                               GLint *params)
{
    getRenderbufferParameteriv(target, pname, params);
}

void Context::texBuffer(TextureType target, GLenum internalformat, BufferID buffer)
{
    ASSERT(target == TextureType::Buffer);

    Texture *texture  = getTextureByType(target);
    Buffer *bufferObj = mState.mBufferManager->getBuffer(buffer);
    ANGLE_CONTEXT_TRY(texture->setBuffer(this, bufferObj, internalformat));
}

void Context::texBufferRange(TextureType target,
                             GLenum internalformat,
                             BufferID buffer,
                             GLintptr offset,
                             GLsizeiptr size)
{
    ASSERT(target == TextureType::Buffer);

    Texture *texture  = getTextureByType(target);
    Buffer *bufferObj = mState.mBufferManager->getBuffer(buffer);
    ANGLE_CONTEXT_TRY(texture->setBufferRange(this, bufferObj, internalformat, offset, size));
}

void Context::getTexParameterfv(TextureType target, GLenum pname, GLfloat *params)
{
    const Texture *const texture = getTextureByType(target);
    QueryTexParameterfv(this, texture, pname, params);
}

void Context::getTexParameterfvRobust(TextureType target,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      GLsizei *length,
                                      GLfloat *params)
{
    getTexParameterfv(target, pname, params);
}

void Context::getTexParameteriv(TextureType target, GLenum pname, GLint *params)
{
    const Texture *const texture = getTextureByType(target);
    QueryTexParameteriv(this, texture, pname, params);
}

void Context::getTexParameterIiv(TextureType target, GLenum pname, GLint *params)
{
    const Texture *const texture = getTextureByType(target);
    QueryTexParameterIiv(this, texture, pname, params);
}

void Context::getTexParameterIuiv(TextureType target, GLenum pname, GLuint *params)
{
    const Texture *const texture = getTextureByType(target);
    QueryTexParameterIuiv(this, texture, pname, params);
}

void Context::getTexParameterivRobust(TextureType target,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      GLsizei *length,
                                      GLint *params)
{
    getTexParameteriv(target, pname, params);
}

void Context::getTexParameterIivRobust(TextureType target,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       GLsizei *length,
                                       GLint *params)
{
    UNIMPLEMENTED();
}

void Context::getTexParameterIuivRobust(TextureType target,
                                        GLenum pname,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLuint *params)
{
    UNIMPLEMENTED();
}

void Context::getTexLevelParameteriv(TextureTarget target, GLint level, GLenum pname, GLint *params)
{
    Texture *texture = getTextureByTarget(target);
    QueryTexLevelParameteriv(texture, target, level, pname, params);
}

void Context::getTexLevelParameterivRobust(TextureTarget target,
                                           GLint level,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           GLsizei *length,
                                           GLint *params)
{
    UNIMPLEMENTED();
}

void Context::getTexLevelParameterfv(TextureTarget target,
                                     GLint level,
                                     GLenum pname,
                                     GLfloat *params)
{
    Texture *texture = getTextureByTarget(target);
    QueryTexLevelParameterfv(texture, target, level, pname, params);
}

void Context::getTexLevelParameterfvRobust(TextureTarget target,
                                           GLint level,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           GLsizei *length,
                                           GLfloat *params)
{
    UNIMPLEMENTED();
}

void Context::texParameterf(TextureType target, GLenum pname, GLfloat param)
{
    Texture *const texture = getTextureByType(target);
    SetTexParameterf(this, texture, pname, param);
}

void Context::texParameterfv(TextureType target, GLenum pname, const GLfloat *params)
{
    Texture *const texture = getTextureByType(target);
    SetTexParameterfv(this, texture, pname, params);
}

void Context::texParameterfvRobust(TextureType target,
                                   GLenum pname,
                                   GLsizei bufSize,
                                   const GLfloat *params)
{
    texParameterfv(target, pname, params);
}

void Context::texParameteri(TextureType target, GLenum pname, GLint param)
{
    // Some apps enable KHR_create_context_no_error but pass in an invalid texture type.
    // Workaround this by silently returning in such situations.
    if (target == TextureType::InvalidEnum)
    {
        return;
    }

    Texture *const texture = getTextureByType(target);
    SetTexParameteri(this, texture, pname, param);
}

void Context::texParameteriv(TextureType target, GLenum pname, const GLint *params)
{
    Texture *const texture = getTextureByType(target);
    SetTexParameteriv(this, texture, pname, params);
}

void Context::texParameterIiv(TextureType target, GLenum pname, const GLint *params)
{
    Texture *const texture = getTextureByType(target);
    SetTexParameterIiv(this, texture, pname, params);
}

void Context::texParameterIuiv(TextureType target, GLenum pname, const GLuint *params)
{
    Texture *const texture = getTextureByType(target);
    SetTexParameterIuiv(this, texture, pname, params);
}

void Context::texParameterivRobust(TextureType target,
                                   GLenum pname,
                                   GLsizei bufSize,
                                   const GLint *params)
{
    texParameteriv(target, pname, params);
}

void Context::texParameterIivRobust(TextureType target,
                                    GLenum pname,
                                    GLsizei bufSize,
                                    const GLint *params)
{
    UNIMPLEMENTED();
}

void Context::texParameterIuivRobust(TextureType target,
                                     GLenum pname,
                                     GLsizei bufSize,
                                     const GLuint *params)
{
    UNIMPLEMENTED();
}

void Context::drawArraysInstanced(PrimitiveMode mode,
                                  GLint first,
                                  GLsizei count,
                                  GLsizei instanceCount)
{
    // No-op if count draws no primitives for given mode
    if (noopDrawInstanced(mode, count, instanceCount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(
        mImplementation->drawArraysInstanced(this, mode, first, count, instanceCount));
    MarkTransformFeedbackBufferUsage(this, count, instanceCount);
    MarkShaderStorageUsage(this);
}

void Context::drawElementsInstanced(PrimitiveMode mode,
                                    GLsizei count,
                                    DrawElementsType type,
                                    const void *indices,
                                    GLsizei instances)
{
    // No-op if count draws no primitives for given mode
    if (noopDrawInstanced(mode, count, instances))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(
        mImplementation->drawElementsInstanced(this, mode, count, type, indices, instances));
    MarkShaderStorageUsage(this);
}

void Context::drawElementsBaseVertex(PrimitiveMode mode,
                                     GLsizei count,
                                     DrawElementsType type,
                                     const void *indices,
                                     GLint basevertex)
{
    // No-op if count draws no primitives for given mode
    if (noopDraw(mode, count))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(
        mImplementation->drawElementsBaseVertex(this, mode, count, type, indices, basevertex));
    MarkShaderStorageUsage(this);
}

void Context::drawElementsInstancedBaseVertex(PrimitiveMode mode,
                                              GLsizei count,
                                              DrawElementsType type,
                                              const void *indices,
                                              GLsizei instancecount,
                                              GLint basevertex)
{
    // No-op if count draws no primitives for given mode
    if (noopDrawInstanced(mode, count, instancecount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->drawElementsInstancedBaseVertex(
        this, mode, count, type, indices, instancecount, basevertex));
    MarkShaderStorageUsage(this);
}

void Context::drawRangeElements(PrimitiveMode mode,
                                GLuint start,
                                GLuint end,
                                GLsizei count,
                                DrawElementsType type,
                                const void *indices)
{
    // No-op if count draws no primitives for given mode
    if (noopDraw(mode, count))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(
        mImplementation->drawRangeElements(this, mode, start, end, count, type, indices));
    MarkShaderStorageUsage(this);
}

void Context::drawRangeElementsBaseVertex(PrimitiveMode mode,
                                          GLuint start,
                                          GLuint end,
                                          GLsizei count,
                                          DrawElementsType type,
                                          const void *indices,
                                          GLint basevertex)
{
    // No-op if count draws no primitives for given mode
    if (noopDraw(mode, count))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->drawRangeElementsBaseVertex(this, mode, start, end, count,
                                                                   type, indices, basevertex));
    MarkShaderStorageUsage(this);
}

void Context::drawArraysIndirect(PrimitiveMode mode, const void *indirect)
{
    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->drawArraysIndirect(this, mode, indirect));
    MarkShaderStorageUsage(this);
}

void Context::drawElementsIndirect(PrimitiveMode mode, DrawElementsType type, const void *indirect)
{
    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->drawElementsIndirect(this, mode, type, indirect));
    MarkShaderStorageUsage(this);
}

void Context::flush()
{
    ANGLE_CONTEXT_TRY(mImplementation->flush(this));
}

void Context::finish()
{
    ANGLE_CONTEXT_TRY(mImplementation->finish(this));
}

void Context::insertEventMarker(GLsizei length, const char *marker)
{
    ASSERT(mImplementation);
    ANGLE_CONTEXT_TRY(mImplementation->insertEventMarker(length, marker));
}

void Context::pushGroupMarker(GLsizei length, const char *marker)
{
    ASSERT(mImplementation);

    if (marker == nullptr)
    {
        // From the EXT_debug_marker spec,
        // "If <marker> is null then an empty string is pushed on the stack."
        ANGLE_CONTEXT_TRY(mImplementation->pushGroupMarker(length, ""));
    }
    else
    {
        ANGLE_CONTEXT_TRY(mImplementation->pushGroupMarker(length, marker));
    }
}

void Context::popGroupMarker()
{
    ASSERT(mImplementation);
    ANGLE_CONTEXT_TRY(mImplementation->popGroupMarker());
}

void Context::bindUniformLocation(ShaderProgramID program,
                                  UniformLocation location,
                                  const GLchar *name)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);

    programObject->bindUniformLocation(this, location, name);
}

GLuint Context::getProgramResourceIndex(ShaderProgramID program,
                                        GLenum programInterface,
                                        const GLchar *name)
{
    const Program *programObject = getProgramResolveLink(program);
    return QueryProgramResourceIndex(programObject, programInterface, name);
}

void Context::getProgramResourceName(ShaderProgramID program,
                                     GLenum programInterface,
                                     GLuint index,
                                     GLsizei bufSize,
                                     GLsizei *length,
                                     GLchar *name)
{
    const Program *programObject = getProgramResolveLink(program);
    QueryProgramResourceName(this, programObject, programInterface, index, bufSize, length, name);
}

GLint Context::getProgramResourceLocation(ShaderProgramID program,
                                          GLenum programInterface,
                                          const GLchar *name)
{
    const Program *programObject = getProgramResolveLink(program);
    return QueryProgramResourceLocation(programObject, programInterface, name);
}

void Context::getProgramResourceiv(ShaderProgramID program,
                                   GLenum programInterface,
                                   GLuint index,
                                   GLsizei propCount,
                                   const GLenum *props,
                                   GLsizei bufSize,
                                   GLsizei *length,
                                   GLint *params)
{
    const Program *programObject = getProgramResolveLink(program);
    QueryProgramResourceiv(programObject, programInterface, {index}, propCount, props, bufSize,
                           length, params);
}

void Context::getProgramInterfaceiv(ShaderProgramID program,
                                    GLenum programInterface,
                                    GLenum pname,
                                    GLint *params)
{
    const Program *programObject = getProgramResolveLink(program);
    QueryProgramInterfaceiv(programObject, programInterface, pname, params);
}

void Context::getProgramInterfaceivRobust(ShaderProgramID program,
                                          GLenum programInterface,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLint *params)
{
    UNIMPLEMENTED();
}

void Context::handleError(GLenum errorCode,
                          const char *message,
                          const char *file,
                          const char *function,
                          unsigned int line)
{
    mErrors.handleError(errorCode, message, file, function, line);
}

// Get one of the recorded errors and clear its flag, if any.
// [OpenGL ES 2.0.24] section 2.5 page 13.
GLenum Context::getError()
{
    if (mErrors.empty())
    {
        return GL_NO_ERROR;
    }
    else
    {
        return mErrors.popError();
    }
}

GLenum Context::getGraphicsResetStatus()
{
    return mErrors.getGraphicsResetStatus(mImplementation.get());
}

bool Context::isResetNotificationEnabled() const
{
    return mErrors.getResetStrategy() == GL_LOSE_CONTEXT_ON_RESET_EXT;
}

EGLenum Context::getRenderBuffer() const
{
    const Framebuffer *framebuffer =
        mState.mFramebufferManager->getFramebuffer(Framebuffer::kDefaultDrawFramebufferHandle);
    if (framebuffer == nullptr)
    {
        return EGL_NONE;
    }

    const FramebufferAttachment *backAttachment = framebuffer->getAttachment(this, GL_BACK);
    ASSERT(backAttachment != nullptr);
    return backAttachment->getSurface()->getRenderBuffer();
}

VertexArray *Context::checkVertexArrayAllocation(VertexArrayID vertexArrayHandle)
{
    // Only called after a prior call to Gen.
    VertexArray *vertexArray = getVertexArray(vertexArrayHandle);
    if (!vertexArray)
    {
        vertexArray = new VertexArray(mImplementation.get(), vertexArrayHandle,
                                      mState.getCaps().maxVertexAttributes,
                                      mState.getCaps().maxVertexAttribBindings);
        vertexArray->setBufferAccessValidationEnabled(mBufferAccessValidationEnabled);

        mVertexArrayMap.assign(vertexArrayHandle, vertexArray);
    }

    return vertexArray;
}

TransformFeedback *Context::checkTransformFeedbackAllocation(
    TransformFeedbackID transformFeedbackHandle)
{
    // Only called after a prior call to Gen.
    TransformFeedback *transformFeedback = getTransformFeedback(transformFeedbackHandle);
    if (!transformFeedback)
    {
        transformFeedback =
            new TransformFeedback(mImplementation.get(), transformFeedbackHandle, mState.getCaps());
        transformFeedback->addRef();
        mTransformFeedbackMap.assign(transformFeedbackHandle, transformFeedback);
    }

    return transformFeedback;
}

bool Context::isVertexArrayGenerated(VertexArrayID vertexArray) const
{
    ASSERT(mVertexArrayMap.contains({0}));
    return mVertexArrayMap.contains(vertexArray);
}

bool Context::isTransformFeedbackGenerated(TransformFeedbackID transformFeedback) const
{
    ASSERT(mTransformFeedbackMap.contains({0}));
    return mTransformFeedbackMap.contains(transformFeedback);
}

void Context::detachTexture(TextureID texture)
{
    // The State cannot unbind image observers itself, they are owned by the Context
    Texture *tex = mState.mTextureManager->getTexture(texture);
    for (auto &imageBinding : mImageObserverBindings)
    {
        if (imageBinding.getSubject() == tex)
        {
            imageBinding.reset();
        }
    }

    // Simple pass-through to State's detachTexture method, as textures do not require
    // allocation map management either here or in the resource manager at detach time.
    // Zero textures are held by the Context, and we don't attempt to request them from
    // the State.
    mState.detachTexture(this, mZeroTextures, texture);
}

void Context::detachBuffer(Buffer *buffer)
{
    // Simple pass-through to State's detachBuffer method, since
    // only buffer attachments to container objects that are bound to the current context
    // should be detached. And all those are available in State.

    // [OpenGL ES 3.2] section 5.1.2 page 45:
    // Attachments to unbound container objects, such as
    // deletion of a buffer attached to a vertex array object which is not bound to the context,
    // are not affected and continue to act as references on the deleted object
    ANGLE_CONTEXT_TRY(mState.detachBuffer(this, buffer));
}

void Context::detachFramebuffer(FramebufferID framebuffer)
{
    // Framebuffer detachment is handled by Context, because 0 is a valid
    // Framebuffer object, and a pointer to it must be passed from Context
    // to State at binding time.

    // [OpenGL ES 2.0.24] section 4.4 page 107:
    // If a framebuffer that is currently bound to the target FRAMEBUFFER is deleted, it is as
    // though BindFramebuffer had been executed with the target of FRAMEBUFFER and framebuffer of
    // zero.

    if (mState.removeReadFramebufferBinding(framebuffer) && framebuffer.value != 0)
    {
        bindReadFramebuffer({0});
    }

    if (mState.removeDrawFramebufferBinding(framebuffer) && framebuffer.value != 0)
    {
        bindDrawFramebuffer({0});
    }
}

void Context::detachRenderbuffer(RenderbufferID renderbuffer)
{
    mState.detachRenderbuffer(this, renderbuffer);
}

void Context::detachVertexArray(VertexArrayID vertexArray)
{
    // Vertex array detachment is handled by Context, because 0 is a valid
    // VAO, and a pointer to it must be passed from Context to State at
    // binding time.

    // [OpenGL ES 3.0.2] section 2.10 page 43:
    // If a vertex array object that is currently bound is deleted, the binding
    // for that object reverts to zero and the default vertex array becomes current.
    if (mState.removeVertexArrayBinding(this, vertexArray))
    {
        bindVertexArray({0});
    }
}

void Context::detachTransformFeedback(TransformFeedbackID transformFeedback)
{
    // Transform feedback detachment is handled by Context, because 0 is a valid
    // transform feedback, and a pointer to it must be passed from Context to State at
    // binding time.

    // The OpenGL specification doesn't mention what should happen when the currently bound
    // transform feedback object is deleted. Since it is a container object, we treat it like
    // VAOs and FBOs and set the current bound transform feedback back to 0.
    if (mState.removeTransformFeedbackBinding(this, transformFeedback))
    {
        bindTransformFeedback(GL_TRANSFORM_FEEDBACK, {0});
        mStateCache.onActiveTransformFeedbackChange(this);
    }
}

void Context::detachSampler(SamplerID sampler)
{
    mState.detachSampler(this, sampler);
}

void Context::detachProgramPipeline(ProgramPipelineID pipeline)
{
    mState.detachProgramPipeline(this, pipeline);
}

void Context::vertexAttribDivisor(GLuint index, GLuint divisor)
{
    mState.setVertexAttribDivisor(this, index, divisor);
    mStateCache.onVertexArrayStateChange(this);
}

void Context::samplerParameteri(SamplerID sampler, GLenum pname, GLint param)
{
    Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    SetSamplerParameteri(this, samplerObject, pname, param);
}

void Context::samplerParameteriv(SamplerID sampler, GLenum pname, const GLint *param)
{
    Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    SetSamplerParameteriv(this, samplerObject, pname, param);
}

void Context::samplerParameterIiv(SamplerID sampler, GLenum pname, const GLint *param)
{
    Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    SetSamplerParameterIiv(this, samplerObject, pname, param);
}

void Context::samplerParameterIuiv(SamplerID sampler, GLenum pname, const GLuint *param)
{
    Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    SetSamplerParameterIuiv(this, samplerObject, pname, param);
}

void Context::samplerParameterivRobust(SamplerID sampler,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       const GLint *param)
{
    samplerParameteriv(sampler, pname, param);
}

void Context::samplerParameterIivRobust(SamplerID sampler,
                                        GLenum pname,
                                        GLsizei bufSize,
                                        const GLint *param)
{
    UNIMPLEMENTED();
}

void Context::samplerParameterIuivRobust(SamplerID sampler,
                                         GLenum pname,
                                         GLsizei bufSize,
                                         const GLuint *param)
{
    UNIMPLEMENTED();
}

void Context::samplerParameterf(SamplerID sampler, GLenum pname, GLfloat param)
{
    Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    SetSamplerParameterf(this, samplerObject, pname, param);
}

void Context::samplerParameterfv(SamplerID sampler, GLenum pname, const GLfloat *param)
{
    Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    SetSamplerParameterfv(this, samplerObject, pname, param);
}

void Context::samplerParameterfvRobust(SamplerID sampler,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       const GLfloat *param)
{
    samplerParameterfv(sampler, pname, param);
}

void Context::getSamplerParameteriv(SamplerID sampler, GLenum pname, GLint *params)
{
    const Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    QuerySamplerParameteriv(samplerObject, pname, params);
}

void Context::getSamplerParameterIiv(SamplerID sampler, GLenum pname, GLint *params)
{
    const Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    QuerySamplerParameterIiv(samplerObject, pname, params);
}

void Context::getSamplerParameterIuiv(SamplerID sampler, GLenum pname, GLuint *params)
{
    const Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    QuerySamplerParameterIuiv(samplerObject, pname, params);
}

void Context::getSamplerParameterivRobust(SamplerID sampler,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLint *params)
{
    getSamplerParameteriv(sampler, pname, params);
}

void Context::getSamplerParameterIivRobust(SamplerID sampler,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           GLsizei *length,
                                           GLint *params)
{
    UNIMPLEMENTED();
}

void Context::getSamplerParameterIuivRobust(SamplerID sampler,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            GLsizei *length,
                                            GLuint *params)
{
    UNIMPLEMENTED();
}

void Context::getSamplerParameterfv(SamplerID sampler, GLenum pname, GLfloat *params)
{
    const Sampler *const samplerObject =
        mState.mSamplerManager->checkSamplerAllocation(mImplementation.get(), sampler);
    QuerySamplerParameterfv(samplerObject, pname, params);
}

void Context::getSamplerParameterfvRobust(SamplerID sampler,
                                          GLenum pname,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLfloat *params)
{
    getSamplerParameterfv(sampler, pname, params);
}

void Context::programParameteri(ShaderProgramID program, GLenum pname, GLint value)
{
    gl::Program *programObject = getProgramResolveLink(program);
    SetProgramParameteri(this, programObject, pname, value);
}

void Context::initRendererString()
{
    std::ostringstream frontendRendererString;

    constexpr char kRendererString[]        = "ANGLE_GL_RENDERER";
    constexpr char kAndroidRendererString[] = "debug.angle.gl_renderer";

    std::string overrideRenderer =
        angle::GetEnvironmentVarOrAndroidProperty(kRendererString, kAndroidRendererString);
    if (!overrideRenderer.empty())
    {
        frontendRendererString << overrideRenderer;
    }
    else
    {
        std::string vendorString(mDisplay->getBackendVendorString());
        std::string rendererString(mDisplay->getBackendRendererDescription());
        std::string versionString(mDisplay->getBackendVersionString(!isWebGL()));
        // Commas are used as a separator in ANGLE's renderer string, so remove commas from each
        // element.
        vendorString.erase(std::remove(vendorString.begin(), vendorString.end(), ','),
                           vendorString.end());
        rendererString.erase(std::remove(rendererString.begin(), rendererString.end(), ','),
                             rendererString.end());
        versionString.erase(std::remove(versionString.begin(), versionString.end(), ','),
                            versionString.end());
        frontendRendererString << "ANGLE (";
        frontendRendererString << vendorString;
        frontendRendererString << ", ";
        frontendRendererString << rendererString;
        frontendRendererString << ", ";
        frontendRendererString << versionString;
        frontendRendererString << ")";
    }

    mRendererString = MakeStaticString(frontendRendererString.str());
}

void Context::initVendorString()
{
    std::ostringstream vendorString;

    constexpr char kVendorString[]        = "ANGLE_GL_VENDOR";
    constexpr char kAndroidVendorString[] = "debug.angle.gl_vendor";

    std::string overrideVendor =
        angle::GetEnvironmentVarOrAndroidProperty(kVendorString, kAndroidVendorString);

    if (!overrideVendor.empty())
    {
        vendorString << overrideVendor;
    }
    else
    {
        vendorString << mDisplay->getVendorString();
    }

    mVendorString = MakeStaticString(vendorString.str());
}

void Context::initVersionStrings()
{
    const Version &clientVersion = getClientVersion();

    std::ostringstream versionString;

    constexpr char kVersionString[]        = "ANGLE_GL_VERSION";
    constexpr char kAndroidVersionString[] = "debug.angle.gl_version";

    std::string overrideVersion =
        angle::GetEnvironmentVarOrAndroidProperty(kVersionString, kAndroidVersionString);

    if (!overrideVersion.empty())
    {
        versionString << overrideVersion;
    }
    else
    {
        versionString << "OpenGL ES ";
        versionString << clientVersion.major << "." << clientVersion.minor << ".0 (ANGLE "
                      << angle::GetANGLEVersionString() << ")";
    }

    mVersionString = MakeStaticString(versionString.str());

    std::ostringstream shadingLanguageVersionString;
    shadingLanguageVersionString << "OpenGL ES GLSL ES ";
    shadingLanguageVersionString << (clientVersion.major == 2 ? 1 : clientVersion.major) << "."
                                 << clientVersion.minor << "0 (ANGLE "
                                 << angle::GetANGLEVersionString() << ")";
    mShadingLanguageString = MakeStaticString(shadingLanguageVersionString.str());
}

void Context::initExtensionStrings()
{
    auto mergeExtensionStrings = [](const std::vector<const char *> &strings) {
        std::ostringstream combinedStringStream;
        std::copy(strings.begin(), strings.end(),
                  std::ostream_iterator<const char *>(combinedStringStream, " "));
        return MakeStaticString(combinedStringStream.str());
    };

    mExtensionStrings.clear();
    for (const auto &extensionString : mState.getExtensions().getStrings())
    {
        mExtensionStrings.push_back(MakeStaticString(extensionString));
    }
    mExtensionString = mergeExtensionStrings(mExtensionStrings);

    mRequestableExtensionStrings.clear();
    for (const auto &extensionInfo : GetExtensionInfoMap())
    {
        if (extensionInfo.second.Requestable &&
            !(mState.getExtensions().*(extensionInfo.second.ExtensionsMember)) &&
            mSupportedExtensions.*(extensionInfo.second.ExtensionsMember))
        {
            mRequestableExtensionStrings.push_back(MakeStaticString(extensionInfo.first));
        }
    }
    mRequestableExtensionString = mergeExtensionStrings(mRequestableExtensionStrings);
}

const GLubyte *Context::getString(GLenum name)
{
    return static_cast<const Context *>(this)->getString(name);
}

const GLubyte *Context::getStringi(GLenum name, GLuint index)
{
    return static_cast<const Context *>(this)->getStringi(name, index);
}

const GLubyte *Context::getString(GLenum name) const
{
    switch (name)
    {
        case GL_VENDOR:
            return reinterpret_cast<const GLubyte *>(mVendorString);

        case GL_RENDERER:
            return reinterpret_cast<const GLubyte *>(mRendererString);

        case GL_VERSION:
            return reinterpret_cast<const GLubyte *>(mVersionString);

        case GL_SHADING_LANGUAGE_VERSION:
            return reinterpret_cast<const GLubyte *>(mShadingLanguageString);

        case GL_EXTENSIONS:
            return reinterpret_cast<const GLubyte *>(mExtensionString);

        case GL_REQUESTABLE_EXTENSIONS_ANGLE:
            return reinterpret_cast<const GLubyte *>(mRequestableExtensionString);

        case GL_SERIALIZED_CONTEXT_STRING_ANGLE:
            if (angle::SerializeContextToString(this, &mCachedSerializedStateString) ==
                angle::Result::Continue)
            {
                return reinterpret_cast<const GLubyte *>(mCachedSerializedStateString.c_str());
            }
            else
            {
                return nullptr;
            }

        default:
            UNREACHABLE();
            return nullptr;
    }
}

const GLubyte *Context::getStringi(GLenum name, GLuint index) const
{
    switch (name)
    {
        case GL_EXTENSIONS:
            return reinterpret_cast<const GLubyte *>(mExtensionStrings[index]);

        case GL_REQUESTABLE_EXTENSIONS_ANGLE:
            return reinterpret_cast<const GLubyte *>(mRequestableExtensionStrings[index]);

        default:
            UNREACHABLE();
            return nullptr;
    }
}

size_t Context::getExtensionStringCount() const
{
    return mExtensionStrings.size();
}

bool Context::isExtensionRequestable(const char *name) const
{
    const ExtensionInfoMap &extensionInfos = GetExtensionInfoMap();
    auto extension                         = extensionInfos.find(name);

    return extension != extensionInfos.end() && extension->second.Requestable &&
           mSupportedExtensions.*(extension->second.ExtensionsMember);
}

bool Context::isExtensionDisablable(const char *name) const
{
    const ExtensionInfoMap &extensionInfos = GetExtensionInfoMap();
    auto extension                         = extensionInfos.find(name);

    return extension != extensionInfos.end() && extension->second.Disablable &&
           mSupportedExtensions.*(extension->second.ExtensionsMember);
}

void Context::requestExtension(const char *name)
{
    setExtensionEnabled(name, true);
}
void Context::disableExtension(const char *name)
{
    setExtensionEnabled(name, false);
}

void Context::setExtensionEnabled(const char *name, bool enabled)
{
    const ExtensionInfoMap &extensionInfos = GetExtensionInfoMap();
    ASSERT(extensionInfos.find(name) != extensionInfos.end());
    const auto &extension = extensionInfos.at(name);
    ASSERT(extension.Requestable);
    ASSERT(isExtensionRequestable(name));

    if (mState.getExtensions().*(extension.ExtensionsMember) == enabled)
    {
        // No change
        return;
    }

    mState.getMutableExtensions()->*(extension.ExtensionsMember) = enabled;

    if (enabled)
    {
        if (strcmp(name, "GL_OVR_multiview2") == 0)
        {
            // OVR_multiview is implicitly enabled when OVR_multiview2 is enabled
            requestExtension("GL_OVR_multiview");
        }
        else if (strcmp(name, "GL_OES_texture_storage_multisample_2d_array") == 0)
        {
            // This extension implies that the context supports multisample 2D textures
            // so ANGLE_texture_multisample must be enabled implicitly here.
            requestExtension("GL_ANGLE_texture_multisample");
        }
        else if (strcmp(name, "GL_ANGLE_shader_pixel_local_storage") == 0 ||
                 strcmp(name, "GL_ANGLE_shader_pixel_local_storage_coherent") == 0)
        {
            // ANGLE_shader_pixel_local_storage/ANGLE_shader_pixel_local_storage_coherent have
            // various dependency extensions, including each other.
            const auto enableIfRequestable = [this](const char *extensionName) {
                for (const char *requestableExtension : mRequestableExtensionStrings)
                {
                    if (strcmp(extensionName, requestableExtension) == 0)
                    {
                        requestExtension(extensionName);
                        return;
                    }
                }
            };
            enableIfRequestable("GL_OES_draw_buffers_indexed");
            enableIfRequestable("GL_EXT_draw_buffers_indexed");
            enableIfRequestable("GL_EXT_color_buffer_float");
            enableIfRequestable("GL_EXT_color_buffer_half_float");
            enableIfRequestable("GL_ANGLE_shader_pixel_local_storage_coherent");
            enableIfRequestable("GL_ANGLE_shader_pixel_local_storage");
        }
    }

    reinitializeAfterExtensionsChanged();
}

void Context::reinitializeAfterExtensionsChanged()
{
    updateCaps();
    initExtensionStrings();

    // Release the shader compiler so it will be re-created with the requested extensions enabled.
    releaseShaderCompiler();

    // Invalidate all textures and framebuffer. Some extensions make new formats renderable or
    // sampleable.
    mState.mTextureManager->signalAllTexturesDirty();
    for (auto &zeroTexture : mZeroTextures)
    {
        if (zeroTexture.get() != nullptr)
        {
            zeroTexture->signalDirtyStorage(InitState::Initialized);
        }
    }

    mState.mFramebufferManager->invalidateFramebufferCompletenessCache();
}

size_t Context::getRequestableExtensionStringCount() const
{
    return mRequestableExtensionStrings.size();
}

void Context::beginTransformFeedback(PrimitiveMode primitiveMode)
{
    TransformFeedback *transformFeedback = mState.getCurrentTransformFeedback();
    ASSERT(transformFeedback != nullptr);
    ASSERT(!transformFeedback->isPaused());

    // TODO: http://anglebug.com/42265705: Handle PPOs
    ANGLE_CONTEXT_TRY(transformFeedback->begin(this, primitiveMode, mState.getProgram()));
    mStateCache.onActiveTransformFeedbackChange(this);
}

bool Context::hasActiveTransformFeedback(ShaderProgramID program) const
{
    // Note: transform feedback objects are private to context and so the map doesn't need locking
    for (auto pair : UnsafeResourceMapIter(mTransformFeedbackMap))
    {
        if (pair.second != nullptr && pair.second->hasBoundProgram(program))
        {
            return true;
        }
    }
    return false;
}

Extensions Context::generateSupportedExtensions() const
{
    Extensions supportedExtensions = mImplementation->getNativeExtensions();

    if (getClientVersion() < ES_2_0)
    {
        // Default extensions for GLES1
        supportedExtensions.blendSubtractOES         = true;
        supportedExtensions.pointSizeArrayOES        = true;
        supportedExtensions.textureCubeMapOES        = true;
        supportedExtensions.textureMirroredRepeatOES = true;
        supportedExtensions.pointSpriteOES           = true;
        supportedExtensions.drawTextureOES           = true;
        supportedExtensions.framebufferObjectOES     = true;
        supportedExtensions.parallelShaderCompileKHR = false;
        supportedExtensions.texture3DOES             = false;
        supportedExtensions.clipDistanceAPPLE        = false;
    }

    if (getClientVersion() < ES_3_0)
    {
        // Disable ES3+ extensions
        supportedExtensions.colorBufferFloatEXT          = false;
        supportedExtensions.EGLImageExternalEssl3OES     = false;
        supportedExtensions.multiviewOVR                 = false;
        supportedExtensions.multiview2OVR                = false;
        supportedExtensions.multiviewMultisampleANGLE    = false;
        supportedExtensions.copyTexture3dANGLE           = false;
        supportedExtensions.textureMultisampleANGLE      = false;
        supportedExtensions.textureQueryLodEXT           = false;
        supportedExtensions.textureShadowLodEXT          = false;
        supportedExtensions.textureStencil8OES           = false;
        supportedExtensions.conservativeDepthEXT         = false;
        supportedExtensions.drawBuffersIndexedEXT        = false;
        supportedExtensions.drawBuffersIndexedOES        = false;
        supportedExtensions.EGLImageArrayEXT             = false;
        supportedExtensions.stencilTexturingANGLE        = false;
        supportedExtensions.textureFormatSRGBOverrideEXT = false;
        supportedExtensions.renderSharedExponentQCOM     = false;
        supportedExtensions.renderSnormEXT               = false;

        // Support GL_EXT_texture_norm16 on non-WebGL ES2 contexts. This is needed for R16/RG16
        // texturing for HDR video playback in Chromium which uses ES2 for compositor contexts.
        // Remove this workaround after Chromium migrates to ES3 for compositor contexts.
        if (mWebGLContext || getClientVersion() < ES_2_0)
        {
            supportedExtensions.textureNorm16EXT = false;
        }

        // Requires immutable textures
        supportedExtensions.yuvInternalFormatANGLE = false;

        // Require ESSL 3.0
        supportedExtensions.shaderMultisampleInterpolationOES  = false;
        supportedExtensions.shaderNoperspectiveInterpolationNV = false;
        supportedExtensions.sampleVariablesOES                 = false;

        // Require ES 3.1 but could likely be exposed on 3.0
        supportedExtensions.textureCubeMapArrayEXT = false;
        supportedExtensions.textureCubeMapArrayOES = false;

        // Require RED and RG formats
        supportedExtensions.textureSRGBR8EXT  = false;
        supportedExtensions.textureSRGBRG8EXT = false;

        // Requires glCompressedTexImage3D
        supportedExtensions.textureCompressionAstcOES = false;

        // Don't expose GL_EXT_texture_sRGB_decode without sRGB texture support
        if (!supportedExtensions.sRGBEXT)
        {
            supportedExtensions.textureSRGBDecodeEXT = false;
        }

        // Don't expose GL_OES_texture_float_linear without full legacy float texture support
        // The renderer may report OES_texture_float_linear without OES_texture_float
        // This is valid in a GLES 3.0 context, but not in a GLES 2.0 context
        if (!(supportedExtensions.textureFloatOES && supportedExtensions.textureHalfFloatOES))
        {
            supportedExtensions.textureFloatLinearOES     = false;
            supportedExtensions.textureHalfFloatLinearOES = false;
        }

        // Because of the difference in the SNORM to FLOAT conversion formula
        // between GLES 2.0 and 3.0, vertex type 10_10_10_2 is disabled
        // when the context version is lower than 3.0
        supportedExtensions.vertexType1010102OES = false;

        // GL_EXT_EGL_image_storage requires ESSL3
        supportedExtensions.EGLImageStorageEXT = false;

        // GL_EXT_YUV_target requires ESSL3
        supportedExtensions.YUVTargetEXT = false;

        // GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance require ESSL3
        supportedExtensions.clipCullDistanceEXT   = false;
        supportedExtensions.clipCullDistanceANGLE = false;

        // ANGLE_shader_pixel_local_storage requires ES3
        supportedExtensions.shaderPixelLocalStorageANGLE         = false;
        supportedExtensions.shaderPixelLocalStorageCoherentANGLE = false;

        // Multisample arrays could be supported on ES 3.0
        // although the extension spec requires ES 3.1.
        supportedExtensions.textureStorageMultisample2dArrayOES = false;
    }

    if (getClientVersion() < ES_3_1)
    {
        // Disable ES3.1+ extensions
        supportedExtensions.geometryShaderEXT       = false;
        supportedExtensions.geometryShaderOES       = false;
        supportedExtensions.gpuShader5EXT           = false;
        supportedExtensions.gpuShader5OES           = false;
        supportedExtensions.primitiveBoundingBoxEXT = false;
        supportedExtensions.shaderImageAtomicOES    = false;
        supportedExtensions.shaderIoBlocksEXT       = false;
        supportedExtensions.shaderIoBlocksOES       = false;
        supportedExtensions.tessellationShaderEXT   = false;
        supportedExtensions.tessellationShaderOES   = false;
        supportedExtensions.textureBufferEXT        = false;
        supportedExtensions.textureBufferOES        = false;
    }

    if (getClientVersion() > ES_2_0)
    {
        // FIXME(geofflang): Don't support EXT_sRGB in non-ES2 contexts
        // supportedExtensions.sRGB = false;

        // If colorBufferFloatEXT is disabled but colorBufferHalfFloatEXT is enabled, then we will
        // expose some floating-point formats as color buffer targets but reject blits between
        // fixed-point and floating-point formats (this behavior is only enabled in
        // colorBufferFloatEXT, and must be rejected if only colorBufferHalfFloatEXT is enabled).
        // dEQP does not check for this, and will assume that floating-point and fixed-point formats
        // can be blit onto each other if the format is available.
        // We require colorBufferFloatEXT to be present in order to enable colorBufferHalfFloatEXT,
        // so that blitting is always allowed if the requested formats are exposed and have the
        // correct feature capabilities. WebGL 2 wants to support colorBufferHalfFloatEXT without
        // colorBufferFloatEXT.
        if (!supportedExtensions.colorBufferFloatEXT && !mWebGLContext)
        {
            supportedExtensions.colorBufferHalfFloatEXT = false;
        }

        // Disable support for CHROMIUM_color_buffer_float_rgb[a] in ES 3.0+, these extensions are
        // non-conformant in ES 3.0 and superseded by EXT_color_buffer_float.
        supportedExtensions.colorBufferFloatRgbCHROMIUM  = false;
        supportedExtensions.colorBufferFloatRgbaCHROMIUM = false;
    }

    if (getClientVersion() >= ES_3_0)
    {
        // Enable this extension for GLES3+.
        supportedExtensions.renderabilityValidationANGLE = true;
    }

    if (getFrontendFeatures().disableDrawBuffersIndexed.enabled)
    {
        supportedExtensions.drawBuffersIndexedEXT = false;
        supportedExtensions.drawBuffersIndexedOES = false;
    }

    if (getFrontendFeatures().disableAnisotropicFiltering.enabled)
    {
        supportedExtensions.textureFilterAnisotropicEXT = false;
    }

    if (!getFrontendFeatures().emulatePixelLocalStorage.enabled)
    {
        supportedExtensions.shaderPixelLocalStorageANGLE         = false;
        supportedExtensions.shaderPixelLocalStorageCoherentANGLE = false;
    }

    // Some extensions are always available because they are implemented in the GL layer.
    supportedExtensions.bindUniformLocationCHROMIUM      = true;
    supportedExtensions.vertexArrayObjectOES             = true;
    supportedExtensions.bindGeneratesResourceCHROMIUM    = true;
    supportedExtensions.clientArraysANGLE                = true;
    supportedExtensions.requestExtensionANGLE            = true;
    supportedExtensions.multiDrawANGLE                   = true;
    supportedExtensions.programBinaryReadinessQueryANGLE = true;

    const Limitations &limitations                  = getLimitations();
    const angle::FrontendFeatures &frontendFeatures = mDisplay->getFrontendFeatures();

    if (limitations.multidrawEmulated &&
        !frontendFeatures.alwaysEnableEmulatedMultidrawExtensions.enabled && !mWebGLContext)
    {
        supportedExtensions.multiDrawANGLE       = false;
        supportedExtensions.multiDrawIndirectEXT = false;
    }

    if (limitations.baseInstanceBaseVertexEmulated &&
        !frontendFeatures.alwaysEnableEmulatedMultidrawExtensions.enabled && !mWebGLContext)
    {
        supportedExtensions.baseVertexBaseInstanceANGLE = false;
    }

    if (limitations.baseInstanceEmulated &&
        !frontendFeatures.alwaysEnableEmulatedMultidrawExtensions.enabled && !mWebGLContext)
    {
        supportedExtensions.baseInstanceEXT = false;
    }

    // Enable the no error extension if the context was created with the flag.
    supportedExtensions.noErrorKHR = skipValidation();

    // Enable surfaceless to advertise we'll have the correct behavior when there is no default FBO
    supportedExtensions.surfacelessContextOES = mSurfacelessSupported;

    // Explicitly enable GL_KHR_debug
    supportedExtensions.debugKHR = true;

    // Explicitly enable GL_EXT_debug_label
    supportedExtensions.debugLabelEXT = true;

    // Explicitly enable GL_ANGLE_robust_client_memory if the context supports validation.
    supportedExtensions.robustClientMemoryANGLE = !skipValidation();

    // Determine robust resource init availability from EGL.
    supportedExtensions.robustResourceInitializationANGLE = mState.isRobustResourceInitEnabled();

    // mState.getExtensions().robustBufferAccessBehaviorKHR is true only if robust access is true
    // and the backend supports it.
    supportedExtensions.robustBufferAccessBehaviorKHR =
        mState.hasRobustAccess() && supportedExtensions.robustBufferAccessBehaviorKHR;

    // Enable the cache control query unconditionally.
    supportedExtensions.programCacheControlANGLE = true;

    // If EGL_KHR_fence_sync is not enabled, don't expose GL_OES_EGL_sync.
    ASSERT(mDisplay);
    if (!mDisplay->getExtensions().fenceSync)
    {
        supportedExtensions.EGLSyncOES = false;
    }

    if (mDisplay->getExtensions().robustnessVideoMemoryPurgeNV)
    {
        supportedExtensions.robustnessVideoMemoryPurgeNV = true;
    }

    supportedExtensions.memorySizeANGLE = true;

    // GL_CHROMIUM_lose_context is implemented in the frontend
    supportedExtensions.loseContextCHROMIUM = true;

    // The ASTC texture extensions have dependency requirements.
    if (supportedExtensions.textureCompressionAstcHdrKHR ||
        supportedExtensions.textureCompressionAstcSliced3dKHR)
    {
        // GL_KHR_texture_compression_astc_hdr cannot be exposed without also exposing
        // GL_KHR_texture_compression_astc_ldr
        ASSERT(supportedExtensions.textureCompressionAstcLdrKHR);
    }

    if (supportedExtensions.textureCompressionAstcOES)
    {
        // GL_OES_texture_compression_astc cannot be exposed without also exposing
        // GL_KHR_texture_compression_astc_ldr and GL_KHR_texture_compression_astc_hdr
        ASSERT(supportedExtensions.textureCompressionAstcLdrKHR);
        ASSERT(supportedExtensions.textureCompressionAstcHdrKHR);
    }

    // GL_KHR_protected_textures
    // If EGL_KHR_protected_content is not supported then GL_EXT_protected_texture
    // can not be supported.
    if (!mDisplay->getExtensions().protectedContentEXT)
    {
        supportedExtensions.protectedTexturesEXT = false;
    }

    // GL_ANGLE_get_tex_level_parameter is implemented in the front-end
    supportedExtensions.getTexLevelParameterANGLE = true;

    // Always enabled. Will return a default string if capture is not enabled.
    supportedExtensions.getSerializedContextStringANGLE = true;

    // Performance counter queries are always supported. Different groups exist on each back-end.
    supportedExtensions.performanceMonitorAMD = true;

    // GL_ANDROID_extension_pack_es31a
    supportedExtensions.extensionPackEs31aANDROID =
        CanSupportAEP(getClientVersion(), supportedExtensions);

    // Blob cache extension is provided by the ANGLE frontend
    supportedExtensions.blobCacheANGLE = true;

    return supportedExtensions;
}

void Context::initCaps()
{
    Caps *caps = mState.getMutableCaps();
    *caps      = mImplementation->getNativeCaps();

    // Update limitations before evaluating extension support
    *mState.getMutableLimitations() = mImplementation->getNativeLimitations();

    // TODO (http://anglebug.com/42264543): mSupportedExtensions should not be modified here
    mSupportedExtensions = generateSupportedExtensions();

    if (!mDisplay->getFrontendFeatures().allowCompressedFormats.enabled)
    {
        INFO() << "Limiting compressed format support.\n";

        mSupportedExtensions.compressedEACR11SignedTextureOES                = false;
        mSupportedExtensions.compressedEACR11UnsignedTextureOES              = false;
        mSupportedExtensions.compressedEACRG11SignedTextureOES               = false;
        mSupportedExtensions.compressedEACRG11UnsignedTextureOES             = false;
        mSupportedExtensions.compressedETC1RGB8SubTextureEXT                 = false;
        mSupportedExtensions.compressedETC1RGB8TextureOES                    = false;
        mSupportedExtensions.compressedETC2PunchthroughARGBA8TextureOES      = false;
        mSupportedExtensions.compressedETC2PunchthroughASRGB8AlphaTextureOES = false;
        mSupportedExtensions.compressedETC2RGB8TextureOES                    = false;
        mSupportedExtensions.compressedETC2RGBA8TextureOES                   = false;
        mSupportedExtensions.compressedETC2SRGB8Alpha8TextureOES             = false;
        mSupportedExtensions.compressedETC2SRGB8TextureOES                   = false;
        mSupportedExtensions.compressedTextureEtcANGLE                       = false;
        mSupportedExtensions.textureCompressionPvrtcIMG                      = false;
        mSupportedExtensions.pvrtcSRGBEXT                                    = false;
        mSupportedExtensions.copyCompressedTextureCHROMIUM                   = false;
        mSupportedExtensions.textureCompressionAstcHdrKHR                    = false;
        mSupportedExtensions.textureCompressionAstcLdrKHR                    = false;
        mSupportedExtensions.textureCompressionAstcOES                       = false;
        mSupportedExtensions.textureCompressionBptcEXT                       = false;
        mSupportedExtensions.textureCompressionDxt1EXT                       = false;
        mSupportedExtensions.textureCompressionDxt3ANGLE                     = false;
        mSupportedExtensions.textureCompressionDxt5ANGLE                     = false;
        mSupportedExtensions.textureCompressionRgtcEXT                       = false;
        mSupportedExtensions.textureCompressionS3tcSrgbEXT                   = false;
        mSupportedExtensions.textureCompressionAstcSliced3dKHR               = false;

        caps->compressedTextureFormats.clear();
    }

    Extensions *extensions = mState.getMutableExtensions();
    *extensions            = mSupportedExtensions;

    // GLES1 emulation: Initialize caps (Table 6.20 / 6.22 in the ES 1.1 spec)
    if (getClientVersion() < Version(2, 0))
    {
        caps->maxMultitextureUnits          = 4;
        caps->maxClipPlanes                 = 6;
        caps->maxLights                     = 8;
        caps->maxModelviewMatrixStackDepth  = Caps::GlobalMatrixStackDepth;
        caps->maxProjectionMatrixStackDepth = Caps::GlobalMatrixStackDepth;
        caps->maxTextureMatrixStackDepth    = Caps::GlobalMatrixStackDepth;
        caps->minSmoothPointSize            = 1.0f;
        caps->maxSmoothPointSize            = 1.0f;
        caps->minSmoothLineWidth            = 1.0f;
        caps->maxSmoothLineWidth            = 1.0f;
    }

    caps->maxDebugMessageLength   = 1024;
    caps->maxDebugLoggedMessages  = 1024;
    caps->maxDebugGroupStackDepth = 1024;
    caps->maxLabelLength          = 1024;

    if (getClientVersion() < Version(3, 0))
    {
        caps->maxViews = 1u;
    }

#if 0
// This logging can generate a lot of spam in test suites that create many contexts
#    define ANGLE_LOG_LIMITED_CAP(cap, limit)                                               \
        INFO() << "Limiting " << #cap << " to implementation limit " << (limit) << " (was " \
               << (cap) << ")."
#else
#    define ANGLE_LOG_LIMITED_CAP(cap, limit)
#endif

#define ANGLE_LIMIT_CAP(cap, limit)            \
    do                                         \
    {                                          \
        if ((cap) > (limit))                   \
        {                                      \
            ANGLE_LOG_LIMITED_CAP(cap, limit); \
            (cap) = (limit);                   \
        }                                      \
    } while (0)

    // Apply/Verify implementation limits
    ANGLE_LIMIT_CAP(caps->maxDrawBuffers, IMPLEMENTATION_MAX_DRAW_BUFFERS);
    ANGLE_LIMIT_CAP(caps->maxFramebufferWidth, IMPLEMENTATION_MAX_FRAMEBUFFER_SIZE);
    ANGLE_LIMIT_CAP(caps->maxFramebufferHeight, IMPLEMENTATION_MAX_FRAMEBUFFER_SIZE);
    ANGLE_LIMIT_CAP(caps->maxRenderbufferSize, IMPLEMENTATION_MAX_RENDERBUFFER_SIZE);
    ANGLE_LIMIT_CAP(caps->maxColorAttachments, IMPLEMENTATION_MAX_DRAW_BUFFERS);
    ANGLE_LIMIT_CAP(caps->maxVertexAttributes, MAX_VERTEX_ATTRIBS);
    if (mDisplay->getFrontendFeatures().forceMinimumMaxVertexAttributes.enabled &&
        getClientVersion() <= Version(2, 0))
    {
        // Only limit GL_MAX_VERTEX_ATTRIBS on ES2 or lower, the ES3+ cap is already at the minimum
        // (16)
        static_assert(MAX_VERTEX_ATTRIBS == 16);
        ANGLE_LIMIT_CAP(caps->maxVertexAttributes, 8);
    }
    ANGLE_LIMIT_CAP(caps->maxVertexAttribStride,
                    static_cast<GLint>(limits::kMaxVertexAttribStride));

    ASSERT(caps->minAliasedPointSize >= 1.0f);

    if (getClientVersion() < ES_3_1)
    {
        caps->maxVertexAttribBindings = caps->maxVertexAttributes;
    }
    else
    {
        ANGLE_LIMIT_CAP(caps->maxVertexAttribBindings, MAX_VERTEX_ATTRIB_BINDINGS);
    }

    const Limitations &limitations = getLimitations();

    if (mWebGLContext && limitations.webGLTextureSizeLimit > 0)
    {
        ANGLE_LIMIT_CAP(caps->max2DTextureSize, limitations.webGLTextureSizeLimit);
        ANGLE_LIMIT_CAP(caps->max3DTextureSize, limitations.webGLTextureSizeLimit);
        ANGLE_LIMIT_CAP(caps->maxCubeMapTextureSize, limitations.webGLTextureSizeLimit);
        ANGLE_LIMIT_CAP(caps->maxArrayTextureLayers, limitations.webGLTextureSizeLimit);
        ANGLE_LIMIT_CAP(caps->maxRectangleTextureSize, limitations.webGLTextureSizeLimit);
    }

    ANGLE_LIMIT_CAP(caps->max2DTextureSize, IMPLEMENTATION_MAX_2D_TEXTURE_SIZE);
    ANGLE_LIMIT_CAP(caps->maxCubeMapTextureSize, IMPLEMENTATION_MAX_CUBE_MAP_TEXTURE_SIZE);
    ANGLE_LIMIT_CAP(caps->max3DTextureSize, IMPLEMENTATION_MAX_3D_TEXTURE_SIZE);
    ANGLE_LIMIT_CAP(caps->maxArrayTextureLayers, IMPLEMENTATION_MAX_2D_ARRAY_TEXTURE_LAYERS);
    ANGLE_LIMIT_CAP(caps->maxRectangleTextureSize, IMPLEMENTATION_MAX_2D_TEXTURE_SIZE);

    ANGLE_LIMIT_CAP(caps->maxShaderUniformBlocks[ShaderType::Vertex],
                    IMPLEMENTATION_MAX_VERTEX_SHADER_UNIFORM_BUFFERS);
    ANGLE_LIMIT_CAP(caps->maxShaderUniformBlocks[ShaderType::Geometry],
                    IMPLEMENTATION_MAX_GEOMETRY_SHADER_UNIFORM_BUFFERS);
    ANGLE_LIMIT_CAP(caps->maxShaderUniformBlocks[ShaderType::Fragment],
                    IMPLEMENTATION_MAX_FRAGMENT_SHADER_UNIFORM_BUFFERS);
    ANGLE_LIMIT_CAP(caps->maxShaderUniformBlocks[ShaderType::Compute],
                    IMPLEMENTATION_MAX_COMPUTE_SHADER_UNIFORM_BUFFERS);
    ANGLE_LIMIT_CAP(caps->maxCombinedUniformBlocks,
                    IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS);
    ANGLE_LIMIT_CAP(caps->maxUniformBufferBindings, IMPLEMENTATION_MAX_UNIFORM_BUFFER_BINDINGS);

    ANGLE_LIMIT_CAP(caps->maxVertexOutputComponents, IMPLEMENTATION_MAX_VARYING_VECTORS * 4);
    ANGLE_LIMIT_CAP(caps->maxFragmentInputComponents, IMPLEMENTATION_MAX_VARYING_VECTORS * 4);

    ANGLE_LIMIT_CAP(caps->maxTransformFeedbackInterleavedComponents,
                    IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS);
    ANGLE_LIMIT_CAP(caps->maxTransformFeedbackSeparateAttributes,
                    IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
    ANGLE_LIMIT_CAP(caps->maxTransformFeedbackSeparateComponents,
                    IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS);

    if (getClientVersion() < ES_3_2 && !extensions->tessellationShaderAny())
    {
        ANGLE_LIMIT_CAP(caps->maxCombinedTextureImageUnits,
                        IMPLEMENTATION_MAX_ES31_ACTIVE_TEXTURES);
    }
    else
    {
        ANGLE_LIMIT_CAP(caps->maxCombinedTextureImageUnits, IMPLEMENTATION_MAX_ACTIVE_TEXTURES);
    }

    for (ShaderType shaderType : AllShaderTypes())
    {
        ANGLE_LIMIT_CAP(caps->maxShaderTextureImageUnits[shaderType],
                        IMPLEMENTATION_MAX_SHADER_TEXTURES);
    }

    ANGLE_LIMIT_CAP(caps->maxImageUnits, IMPLEMENTATION_MAX_IMAGE_UNITS);
    ANGLE_LIMIT_CAP(caps->maxCombinedImageUniforms, IMPLEMENTATION_MAX_IMAGE_UNITS);
    for (ShaderType shaderType : AllShaderTypes())
    {
        ANGLE_LIMIT_CAP(caps->maxShaderImageUniforms[shaderType], IMPLEMENTATION_MAX_IMAGE_UNITS);
    }

    for (ShaderType shaderType : AllShaderTypes())
    {
        ANGLE_LIMIT_CAP(caps->maxShaderAtomicCounterBuffers[shaderType],
                        IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
    }
    ANGLE_LIMIT_CAP(caps->maxAtomicCounterBufferBindings,
                    IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
    ANGLE_LIMIT_CAP(caps->maxCombinedAtomicCounterBuffers,
                    IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);

    for (ShaderType shaderType : AllShaderTypes())
    {
        ANGLE_LIMIT_CAP(caps->maxShaderStorageBlocks[shaderType],
                        IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
    }
    ANGLE_LIMIT_CAP(caps->maxShaderStorageBufferBindings,
                    IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
    ANGLE_LIMIT_CAP(caps->maxCombinedShaderStorageBlocks,
                    IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS);

    ANGLE_LIMIT_CAP(caps->maxClipDistances, IMPLEMENTATION_MAX_CLIP_DISTANCES);

    ANGLE_LIMIT_CAP(caps->maxFramebufferLayers, IMPLEMENTATION_MAX_FRAMEBUFFER_LAYERS);

    ANGLE_LIMIT_CAP(caps->maxSampleMaskWords, IMPLEMENTATION_MAX_SAMPLE_MASK_WORDS);
    ANGLE_LIMIT_CAP(caps->maxSamples, IMPLEMENTATION_MAX_SAMPLES);
    ANGLE_LIMIT_CAP(caps->maxFramebufferSamples, IMPLEMENTATION_MAX_SAMPLES);
    ANGLE_LIMIT_CAP(caps->maxColorTextureSamples, IMPLEMENTATION_MAX_SAMPLES);
    ANGLE_LIMIT_CAP(caps->maxDepthTextureSamples, IMPLEMENTATION_MAX_SAMPLES);
    ANGLE_LIMIT_CAP(caps->maxIntegerSamples, IMPLEMENTATION_MAX_SAMPLES);

    ANGLE_LIMIT_CAP(caps->maxViews, IMPLEMENTATION_ANGLE_MULTIVIEW_MAX_VIEWS);

    ANGLE_LIMIT_CAP(caps->maxDualSourceDrawBuffers, IMPLEMENTATION_MAX_DUAL_SOURCE_DRAW_BUFFERS);

    // WebGL compatibility
    extensions->webglCompatibilityANGLE = mWebGLContext;
    for (const auto &extensionInfo : GetExtensionInfoMap())
    {
        // If the user has requested that extensions start disabled and they are requestable,
        // disable them.
        if (!mExtensionsEnabled && extensionInfo.second.Requestable)
        {
            extensions->*(extensionInfo.second.ExtensionsMember) = false;
        }
    }

    // Hide emulated ETC1 extension from WebGL contexts.
    if (mWebGLContext && limitations.emulatedEtc1)
    {
        mSupportedExtensions.compressedETC1RGB8SubTextureEXT = false;
        mSupportedExtensions.compressedETC1RGB8TextureOES    = false;
    }

    if (limitations.emulatedAstc)
    {
        // Hide emulated ASTC extension from WebGL contexts.
        if (mWebGLContext)
        {
            mSupportedExtensions.textureCompressionAstcLdrKHR = false;
            extensions->textureCompressionAstcLdrKHR          = false;
        }
#if !defined(ANGLE_HAS_ASTCENC)
        // Don't expose emulated ASTC when it's not built.
        mSupportedExtensions.textureCompressionAstcLdrKHR = false;
        extensions->textureCompressionAstcLdrKHR          = false;
#endif
    }

    // If we're capturing application calls for replay, apply some feature limits to increase
    // portability of the trace.
    if (getShareGroup()->getFrameCaptureShared()->enabled() ||
        getFrontendFeatures().enableCaptureLimits.enabled)
    {
        INFO() << "Limit some features because "
               << (getShareGroup()->getFrameCaptureShared()->enabled()
                       ? "FrameCapture is enabled"
                       : "FrameCapture limits were forced")
               << std::endl;

        if (!getFrontendFeatures().enableProgramBinaryForCapture.enabled)
        {
            // Some apps insist on being able to use glProgramBinary. For those, we'll allow the
            // extension to remain on. Otherwise, force the extension off.
            INFO() << "Disabling GL_OES_get_program_binary for trace portability";
            mDisplay->overrideFrontendFeatures({"disable_program_binary"}, true);
        }

        // Set to the most common limit per gpuinfo.org. Required for several platforms we test.
        constexpr GLint maxImageUnits = 8;
        INFO() << "Limiting image unit count to " << maxImageUnits;
        ANGLE_LIMIT_CAP(caps->maxImageUnits, maxImageUnits);
        for (ShaderType shaderType : AllShaderTypes())
        {
            ANGLE_LIMIT_CAP(caps->maxShaderImageUniforms[shaderType], maxImageUnits);
        }

        // Set a large uniform buffer offset alignment that works on multiple platforms.
        // The offset used by the trace needs to be divisible by the device's actual value.
        // Values seen during development: ARM (16), Intel (32), Qualcomm (128), Nvidia (256)
        constexpr GLint uniformBufferOffsetAlignment = 256;
        ASSERT(uniformBufferOffsetAlignment % caps->uniformBufferOffsetAlignment == 0);
        INFO() << "Setting uniform buffer offset alignment to " << uniformBufferOffsetAlignment;
        caps->uniformBufferOffsetAlignment = uniformBufferOffsetAlignment;

        // Also limit texture buffer offset alignment, if enabled
        if (extensions->textureBufferAny())
        {
            constexpr GLint textureBufferOffsetAlignment =
                gl::limits::kMinTextureBufferOffsetAlignment;
            ASSERT(textureBufferOffsetAlignment % caps->textureBufferOffsetAlignment == 0);
            INFO() << "Setting texture buffer offset alignment to " << textureBufferOffsetAlignment;
            caps->textureBufferOffsetAlignment = textureBufferOffsetAlignment;
        }

        INFO() << "Disabling GL_EXT_map_buffer_range and GL_OES_mapbuffer during capture, which "
                  "are not supported on some native drivers";
        extensions->mapBufferRangeEXT = false;
        extensions->mapbufferOES      = false;

        INFO() << "Disabling GL_CHROMIUM_bind_uniform_location during capture, which is not "
                  "supported on native drivers";
        extensions->bindUniformLocationCHROMIUM = false;

        INFO() << "Disabling GL_NV_shader_noperspective_interpolation during capture, which is not "
                  "supported on some native drivers";
        extensions->shaderNoperspectiveInterpolationNV = false;

        INFO() << "Disabling GL_NV_framebuffer_blit during capture, which is not "
                  "supported on some native drivers";
        extensions->framebufferBlitNV = false;

        INFO() << "Disabling GL_EXT_texture_mirror_clamp_to_edge during capture, which is not "
                  "supported on some native drivers";
        extensions->textureMirrorClampToEdgeEXT = false;

        // NVIDIA's Vulkan driver only supports 4 draw buffers
        constexpr GLint maxDrawBuffers = 4;
        INFO() << "Limiting draw buffer count to " << maxDrawBuffers;
        ANGLE_LIMIT_CAP(caps->maxDrawBuffers, maxDrawBuffers);

        // Unity based applications are sending down GL streams with undefined behavior.
        // Disabling EGL_KHR_create_context_no_error (which enables a new EGL attrib) prevents that,
        // but we don't have the infrastructure for disabling EGL extensions yet.
        // Instead, disable GL_KHR_no_error (which disables exposing the GL extension), which
        // prevents writing invalid calls to the capture.
        INFO() << "Enabling validation to prevent invalid calls from being captured. This "
                  "effectively disables GL_KHR_no_error and enables GL_ANGLE_robust_client_memory.";
        mErrors.forceValidation();
        extensions->noErrorKHR              = skipValidation();
        extensions->robustClientMemoryANGLE = !skipValidation();

        INFO() << "Disabling GL_OES_depth32 during capture, which is not widely supported on "
                  "mobile";
        extensions->depth32OES = false;

        // Pixel 4 (Qualcomm) only supports 6 atomic counter buffer bindings.
        constexpr GLint maxAtomicCounterBufferBindings = 6;
        INFO() << "Limiting max atomic counter buffer bindings to "
               << maxAtomicCounterBufferBindings;
        ANGLE_LIMIT_CAP(caps->maxAtomicCounterBufferBindings, maxAtomicCounterBufferBindings);
        for (gl::ShaderType shaderType : gl::AllShaderTypes())
        {
            ANGLE_LIMIT_CAP(caps->maxShaderAtomicCounterBuffers[shaderType],
                            maxAtomicCounterBufferBindings);
        }

        // SwiftShader only supports 12 shader storage buffer bindings.
        constexpr GLint maxShaderStorageBufferBindings = 12;
        INFO() << "Limiting max shader storage buffer bindings to "
               << maxShaderStorageBufferBindings;
        ANGLE_LIMIT_CAP(caps->maxShaderStorageBufferBindings, maxShaderStorageBufferBindings);
        for (gl::ShaderType shaderType : gl::AllShaderTypes())
        {
            ANGLE_LIMIT_CAP(caps->maxShaderStorageBlocks[shaderType],
                            maxShaderStorageBufferBindings);
        }

        // Pixel 7 MAX_TEXTURE_SIZE is 16K
        constexpr GLint max2DTextureSize = 16383;
        INFO() << "Limiting GL_MAX_TEXTURE_SIZE to " << max2DTextureSize;
        ANGLE_LIMIT_CAP(caps->max2DTextureSize, max2DTextureSize);

        // Pixel 4 only supports GL_MAX_SAMPLES of 4
        constexpr GLint maxSamples = 4;
        INFO() << "Limiting GL_MAX_SAMPLES to " << maxSamples;
        ANGLE_LIMIT_CAP(caps->maxSamples, maxSamples);

        // Pixel 4/5 only supports GL_MAX_VERTEX_UNIFORM_VECTORS of 256
        constexpr GLint maxVertexUniformVectors = 256;
        INFO() << "Limiting GL_MAX_VERTEX_UNIFORM_VECTORS to " << maxVertexUniformVectors;
        ANGLE_LIMIT_CAP(caps->maxVertexUniformVectors, maxVertexUniformVectors);

        // Test if we require shadow memory for coherent buffer tracking
        getShareGroup()->getFrameCaptureShared()->determineMemoryProtectionSupport(this);
    }

    // Disable support for OES_get_program_binary
    if (mDisplay->getFrontendFeatures().disableProgramBinary.enabled)
    {
        extensions->getProgramBinaryOES = false;
        caps->shaderBinaryFormats.clear();
        caps->programBinaryFormats.clear();
        mMemoryProgramCache = nullptr;
    }

    // Initialize ANGLE_shader_pixel_local_storage caps based on frontend GL queries.
    //
    // The backend may have already initialized these caps with its own custom values, in which case
    // maxPixelLocalStoragePlanes will already be nonzero and we can skip this step.
    if (mSupportedExtensions.shaderPixelLocalStorageANGLE && caps->maxPixelLocalStoragePlanes == 0)
    {
        int maxDrawableAttachments = std::min(caps->maxDrawBuffers, caps->maxColorAttachments);
        switch (mImplementation->getNativePixelLocalStorageOptions().type)
        {
            case ShPixelLocalStorageType::ImageLoadStore:
                caps->maxPixelLocalStoragePlanes =
                    caps->maxShaderImageUniforms[ShaderType::Fragment];
                ANGLE_LIMIT_CAP(caps->maxPixelLocalStoragePlanes,
                                IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);
                caps->maxColorAttachmentsWithActivePixelLocalStorage = caps->maxColorAttachments;
                caps->maxCombinedDrawBuffersAndPixelLocalStoragePlanes =
                    std::min<GLint>(caps->maxPixelLocalStoragePlanes +
                                        std::min(caps->maxDrawBuffers, caps->maxColorAttachments),
                                    caps->maxCombinedShaderOutputResources);
                break;

            case ShPixelLocalStorageType::FramebufferFetch:
                caps->maxPixelLocalStoragePlanes = maxDrawableAttachments;
                ANGLE_LIMIT_CAP(caps->maxPixelLocalStoragePlanes,
                                IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);
                if (!mSupportedExtensions.drawBuffersIndexedAny())
                {
                    // When pixel local storage is implemented as framebuffer attachments, we need
                    // to disable color masks and blending to its attachments. If the backend
                    // context doesn't have indexed blend and color mask support, then we will have
                    // have to disable them globally. This also means the application can't have its
                    // own draw buffers while PLS is active.
                    caps->maxColorAttachmentsWithActivePixelLocalStorage = 0;
                }
                else
                {
                    caps->maxColorAttachmentsWithActivePixelLocalStorage =
                        maxDrawableAttachments - 1;
                }
                caps->maxCombinedDrawBuffersAndPixelLocalStoragePlanes = maxDrawableAttachments;
                break;

            case ShPixelLocalStorageType::NotSupported:
                UNREACHABLE();
                break;
        }
    }
    // Validate that pixel local storage caps were initialized within implementation limits. We
    // can't just clamp this value here since it would potentially impact other caps.
    ASSERT(caps->maxPixelLocalStoragePlanes <= IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);

#undef ANGLE_LIMIT_CAP
#undef ANGLE_LOG_CAP_LIMIT

    // Generate texture caps
    updateCaps();
}

void Context::updateCaps()
{
    Caps *caps                  = mState.getMutableCaps();
    TextureCapsMap *textureCaps = mState.getMutableTextureCaps();

    caps->compressedTextureFormats.clear();
    textureCaps->clear();

    for (GLenum sizedInternalFormat : GetAllSizedInternalFormats())
    {
        TextureCaps formatCaps = mImplementation->getNativeTextureCaps().get(sizedInternalFormat);
        const InternalFormat &formatInfo = GetSizedInternalFormatInfo(sizedInternalFormat);

        // Update the format caps based on the client version and extensions.
        // Caps are AND'd with the renderer caps because some core formats are still unsupported in
        // ES3.
        formatCaps.texturable =
            formatCaps.texturable &&
            formatInfo.textureSupport(getClientVersion(), mState.getExtensions());
        formatCaps.filterable =
            formatCaps.filterable &&
            formatInfo.filterSupport(getClientVersion(), mState.getExtensions());
        formatCaps.textureAttachment =
            formatCaps.textureAttachment &&
            formatInfo.textureAttachmentSupport(getClientVersion(), mState.getExtensions());
        formatCaps.renderbuffer =
            formatCaps.renderbuffer &&
            formatInfo.renderbufferSupport(getClientVersion(), mState.getExtensions());
        formatCaps.blendable = formatCaps.blendable &&
                               formatInfo.blendSupport(getClientVersion(), mState.getExtensions());

        // OpenGL ES does not support multisampling with non-rendererable formats
        // OpenGL ES 3.0 or prior does not support multisampling with integer formats
        if (!formatCaps.renderbuffer ||
            (getClientVersion() < ES_3_1 && !mState.getExtensions().textureMultisampleANGLE &&
             formatInfo.isInt()))
        {
            formatCaps.sampleCounts.clear();
        }
        else
        {
            // We may have limited the max samples for some required renderbuffer formats due to
            // non-conformant formats. In this case MAX_SAMPLES needs to be lowered accordingly.
            GLuint formatMaxSamples = formatCaps.getMaxSamples();

            // GLES 3.0.5 section 4.4.2.2: "Implementations must support creation of renderbuffers
            // in these required formats with up to the value of MAX_SAMPLES multisamples, with the
            // exception of signed and unsigned integer formats."
            if (!formatInfo.isInt() && formatInfo.isRequiredRenderbufferFormat(getClientVersion()))
            {
                ASSERT(getClientVersion() < ES_3_0 || formatMaxSamples >= 4);
                caps->maxSamples =
                    std::min(static_cast<GLuint>(caps->maxSamples), formatMaxSamples);
            }

            // Handle GLES 3.1 MAX_*_SAMPLES values similarly to MAX_SAMPLES.
            if (getClientVersion() >= ES_3_1 || mState.getExtensions().textureMultisampleANGLE)
            {
                // GLES 3.1 section 9.2.5: "Implementations must support creation of renderbuffers
                // in these required formats with up to the value of MAX_SAMPLES multisamples, with
                // the exception that the signed and unsigned integer formats are required only to
                // support creation of renderbuffers with up to the value of MAX_INTEGER_SAMPLES
                // multisamples, which must be at least one."
                if (formatInfo.isInt())
                {
                    caps->maxIntegerSamples =
                        std::min(static_cast<GLuint>(caps->maxIntegerSamples), formatMaxSamples);
                }

                // GLES 3.1 section 19.3.1.
                if (formatCaps.texturable)
                {
                    if (formatInfo.depthBits > 0)
                    {
                        caps->maxDepthTextureSamples = std::min(
                            static_cast<GLuint>(caps->maxDepthTextureSamples), formatMaxSamples);
                    }
                    else if (formatInfo.redBits > 0)
                    {
                        caps->maxColorTextureSamples = std::min(
                            static_cast<GLuint>(caps->maxColorTextureSamples), formatMaxSamples);
                    }
                }
            }
        }

        if (formatCaps.texturable && (formatInfo.compressed || formatInfo.paletted))
        {
            caps->compressedTextureFormats.push_back(sizedInternalFormat);
        }

        textureCaps->insert(sizedInternalFormat, formatCaps);
    }

    // If program binary is disabled, blank out the memory cache pointer.
    if (!mSupportedExtensions.getProgramBinaryOES)
    {
        mMemoryProgramCache = nullptr;
    }

    // Compute which buffer types are allowed
    mValidBufferBindings.reset();
    mValidBufferBindings.set(BufferBinding::ElementArray);
    mValidBufferBindings.set(BufferBinding::Array);

    if (mState.getExtensions().pixelBufferObjectNV || getClientVersion() >= ES_3_0)
    {
        mValidBufferBindings.set(BufferBinding::PixelPack);
        mValidBufferBindings.set(BufferBinding::PixelUnpack);
    }

    if (getClientVersion() >= ES_3_0)
    {
        mValidBufferBindings.set(BufferBinding::CopyRead);
        mValidBufferBindings.set(BufferBinding::CopyWrite);
        mValidBufferBindings.set(BufferBinding::TransformFeedback);
        mValidBufferBindings.set(BufferBinding::Uniform);
    }

    if (getClientVersion() >= ES_3_1)
    {
        mValidBufferBindings.set(BufferBinding::AtomicCounter);
        mValidBufferBindings.set(BufferBinding::ShaderStorage);
        mValidBufferBindings.set(BufferBinding::DrawIndirect);
        mValidBufferBindings.set(BufferBinding::DispatchIndirect);
    }

    if (getClientVersion() >= ES_3_2 || mState.getExtensions().textureBufferAny())
    {
        mValidBufferBindings.set(BufferBinding::Texture);
    }

    // Reinitialize some dirty bits that depend on extensions.
    if (mState.isRobustResourceInitEnabled())
    {
        mDrawDirtyObjects.set(state::DIRTY_OBJECT_DRAW_ATTACHMENTS);
        mDrawDirtyObjects.set(state::DIRTY_OBJECT_TEXTURES_INIT);
        mDrawDirtyObjects.set(state::DIRTY_OBJECT_IMAGES_INIT);
        mBlitDirtyObjects.set(state::DIRTY_OBJECT_DRAW_ATTACHMENTS);
        mBlitDirtyObjects.set(state::DIRTY_OBJECT_READ_ATTACHMENTS);
        mComputeDirtyObjects.set(state::DIRTY_OBJECT_TEXTURES_INIT);
        mComputeDirtyObjects.set(state::DIRTY_OBJECT_IMAGES_INIT);
        mReadPixelsDirtyObjects.set(state::DIRTY_OBJECT_READ_ATTACHMENTS);
        mCopyImageDirtyBits.set(state::DIRTY_BIT_READ_FRAMEBUFFER_BINDING);
        mCopyImageDirtyObjects.set(state::DIRTY_OBJECT_READ_ATTACHMENTS);
    }

    // We need to validate buffer bounds if we are in a WebGL or robust access context and the
    // back-end does not support robust buffer access behaviour.
    mBufferAccessValidationEnabled = (!mSupportedExtensions.robustBufferAccessBehaviorKHR &&
                                      (mState.isWebGL() || mState.hasRobustAccess()));

    // Cache this in the VertexArrays. They need to check it in state change notifications.
    // Note: vertex array objects are private to context and so the map doesn't need locking
    for (auto vaoIter : UnsafeResourceMapIter(mVertexArrayMap))
    {
        VertexArray *vao = vaoIter.second;
        vao->setBufferAccessValidationEnabled(mBufferAccessValidationEnabled);
    }

    // Reinitialize state cache after extension changes.
    mStateCache.initialize(this);
}

bool Context::noopDrawInstanced(PrimitiveMode mode, GLsizei count, GLsizei instanceCount) const
{
    return (instanceCount == 0) || noopDraw(mode, count);
}

angle::Result Context::prepareForClear(GLbitfield mask)
{
    // Sync the draw framebuffer manually after the clear attachments.
    ANGLE_TRY(mState.getDrawFramebuffer()->ensureClearAttachmentsInitialized(this, mask));
    return syncStateForClear();
}

angle::Result Context::prepareForClearBuffer(GLenum buffer, GLint drawbuffer)
{
    // Sync the draw framebuffer manually after the clear attachments.
    ANGLE_TRY(mState.getDrawFramebuffer()->ensureClearBufferAttachmentsInitialized(this, buffer,
                                                                                   drawbuffer));
    return syncStateForClear();
}

ANGLE_INLINE angle::Result Context::prepareForCopyImage()
{
    ANGLE_TRY(syncDirtyObjects(mCopyImageDirtyObjects, Command::CopyImage));
    return syncDirtyBits(mCopyImageDirtyBits, kCopyImageExtendedDirtyBits, Command::CopyImage);
}

ANGLE_INLINE angle::Result Context::prepareForDispatch()
{
    // Converting a PPO from graphics to compute requires re-linking it.
    // The compute shader must have successfully linked before being included in the PPO, so no link
    // errors that would have been caught during validation should be possible when re-linking the
    // PPO with the compute shader.
    Program *program          = mState.getProgram();
    ProgramPipeline *pipeline = mState.getProgramPipeline();
    if (!program && pipeline)
    {
        // Linking the PPO can't fail due to a validation error within the compute program,
        // since it successfully linked already in order to become part of the PPO in the first
        // place.
        pipeline->resolveLink(this);
        ANGLE_CHECK(this, pipeline->isLinked(), err::kProgramPipelineLinkFailed,
                    GL_INVALID_OPERATION);
    }

    ANGLE_TRY(syncDirtyObjects(mComputeDirtyObjects, Command::Dispatch));
    return syncDirtyBits(kComputeDirtyBits, kComputeExtendedDirtyBits, Command::Dispatch);
}

angle::Result Context::prepareForInvalidate(GLenum target)
{
    // Only sync the FBO that's being invalidated.  Per the GLES3 spec, GL_FRAMEBUFFER is equivalent
    // to GL_DRAW_FRAMEBUFFER for the purposes of invalidation.
    GLenum effectiveTarget = target;
    if (effectiveTarget == GL_FRAMEBUFFER)
    {
        effectiveTarget = GL_DRAW_FRAMEBUFFER;
    }
    ANGLE_TRY(mState.syncDirtyObject(this, effectiveTarget));
    const state::DirtyBits dirtyBits                 = effectiveTarget == GL_READ_FRAMEBUFFER
                                                           ? kReadInvalidateDirtyBits
                                                           : kDrawInvalidateDirtyBits;
    const state::ExtendedDirtyBits extendedDirtyBits = effectiveTarget == GL_READ_FRAMEBUFFER
                                                           ? kReadInvalidateExtendedDirtyBits
                                                           : kDrawInvalidateExtendedDirtyBits;
    return syncDirtyBits(dirtyBits, extendedDirtyBits, Command::Invalidate);
}

angle::Result Context::syncState(const state::DirtyBits bitMask,
                                 const state::ExtendedDirtyBits extendedBitMask,
                                 const state::DirtyObjects &objectMask,
                                 Command command)
{
    ANGLE_TRY(syncDirtyObjects(objectMask, command));
    ANGLE_TRY(syncDirtyBits(bitMask, extendedBitMask, command));
    return angle::Result::Continue;
}

void Context::blitFramebuffer(GLint srcX0,
                              GLint srcY0,
                              GLint srcX1,
                              GLint srcY1,
                              GLint dstX0,
                              GLint dstY0,
                              GLint dstX1,
                              GLint dstY1,
                              GLbitfield mask,
                              GLenum filter)
{
    if (mask == 0)
    {
        // ES3.0 spec, section 4.3.2 specifies that a mask of zero is valid and no
        // buffers are copied.
        return;
    }

    Framebuffer *drawFramebuffer = mState.getDrawFramebuffer();
    Framebuffer *readFramebuffer = mState.getReadFramebuffer();
    ASSERT(drawFramebuffer);
    ASSERT(readFramebuffer);

    // Note that blitting is called against draw framebuffer.
    // See the code in gl::Context::blitFramebuffer.
    if ((mask & GL_COLOR_BUFFER_BIT) && (!drawFramebuffer->hasEnabledDrawBuffer() ||
                                         readFramebuffer->getReadColorAttachment() == nullptr))
    {
        mask &= ~GL_COLOR_BUFFER_BIT;
    }

    if ((mask & GL_STENCIL_BUFFER_BIT) &&
        (drawFramebuffer->getState().getStencilAttachment() == nullptr ||
         readFramebuffer->getState().getStencilAttachment() == nullptr))
    {
        mask &= ~GL_STENCIL_BUFFER_BIT;
    }

    if ((mask & GL_DEPTH_BUFFER_BIT) &&
        (drawFramebuffer->getState().getDepthAttachment() == nullptr ||
         readFramebuffer->getState().getDepthAttachment() == nullptr))
    {
        mask &= ~GL_DEPTH_BUFFER_BIT;
    }

    // Early out if none of the specified attachments exist or are enabled.
    if (mask == 0)
    {
        ANGLE_PERF_WARNING(mState.getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "BlitFramebuffer called for non-existing buffers");
        return;
    }

    Rectangle srcArea(srcX0, srcY0, srcX1 - srcX0, srcY1 - srcY0);
    Rectangle dstArea(dstX0, dstY0, dstX1 - dstX0, dstY1 - dstY0);

    if (dstArea.width == 0 || dstArea.height == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(syncStateForBlit(mask));
    ANGLE_CONTEXT_TRY(drawFramebuffer->blit(this, srcArea, dstArea, mask, filter));
}

void Context::blitFramebufferNV(GLint srcX0,
                                GLint srcY0,
                                GLint srcX1,
                                GLint srcY1,
                                GLint dstX0,
                                GLint dstY0,
                                GLint dstX1,
                                GLint dstY1,
                                GLbitfield mask,
                                GLenum filter)
{
    blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void Context::clear(GLbitfield mask)
{
    if (mState.isRasterizerDiscardEnabled())
    {
        return;
    }

    // Remove clear bits that are ineffective. An effective clear changes at least one fragment. If
    // color/depth/stencil masks make the clear ineffective we skip it altogether.

    // If all color channels in all draw buffers are masked, don't attempt to clear color.
    if (mState.allActiveDrawBufferChannelsMasked())
    {
        mask &= ~GL_COLOR_BUFFER_BIT;
    }

    // If depth write is disabled, don't attempt to clear depth.
    if (mState.getDrawFramebuffer()->getDepthAttachment() == nullptr ||
        mState.getDepthStencilState().isDepthMaskedOut())
    {
        mask &= ~GL_DEPTH_BUFFER_BIT;
    }

    // If all stencil bits are masked, don't attempt to clear stencil.
    if (mState.getDepthStencilState().isStencilMaskedOut(
            mState.getDrawFramebuffer()->getStencilBitCount()))
    {
        mask &= ~GL_STENCIL_BUFFER_BIT;
    }

    if (mask == 0)
    {
        ANGLE_PERF_WARNING(mState.getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Clear called for non-existing buffers");
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForClear(mask));
    ANGLE_CONTEXT_TRY(mState.getDrawFramebuffer()->clear(this, mask));
}

bool Context::isClearBufferMaskedOut(GLenum buffer,
                                     GLint drawbuffer,
                                     GLuint framebufferStencilSize) const
{
    switch (buffer)
    {
        case GL_COLOR:
            return IsColorMaskedOut(mState.getBlendStateExt(), drawbuffer);
        case GL_DEPTH:
            return mState.getDepthStencilState().isDepthMaskedOut();
        case GL_STENCIL:
            return mState.getDepthStencilState().isStencilMaskedOut(framebufferStencilSize);
        case GL_DEPTH_STENCIL:
            return mState.getDepthStencilState().isDepthMaskedOut() &&
                   mState.getDepthStencilState().isStencilMaskedOut(framebufferStencilSize);
        default:
            UNREACHABLE();
            return true;
    }
}

bool Context::noopClearBuffer(GLenum buffer, GLint drawbuffer) const
{
    Framebuffer *framebufferObject = mState.getDrawFramebuffer();

    if (buffer == GL_COLOR && getPrivateState().isActivelyOverriddenPLSDrawBuffer(drawbuffer))
    {
        // If pixel local storage is active and currently overriding the drawbuffer, do nothing.
        // From the client's perspective, there is effectively no buffer bound.
        return true;
    }

    return !IsClearBufferEnabled(framebufferObject->getState(), buffer, drawbuffer) ||
           mState.isRasterizerDiscardEnabled() ||
           isClearBufferMaskedOut(buffer, drawbuffer, framebufferObject->getStencilBitCount());
}

void Context::clearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *values)
{
    if (noopClearBuffer(buffer, drawbuffer))
    {
        return;
    }

    Framebuffer *framebufferObject          = mState.getDrawFramebuffer();
    const FramebufferAttachment *attachment = nullptr;
    if (buffer == GL_DEPTH)
    {
        attachment = framebufferObject->getDepthAttachment();
    }
    else if (buffer == GL_COLOR &&
             static_cast<size_t>(drawbuffer) < framebufferObject->getNumColorAttachments())
    {
        attachment = framebufferObject->getColorAttachment(drawbuffer);
    }
    // It's not an error to try to clear a non-existent buffer, but it's a no-op. We early out so
    // that the backend doesn't need to take this case into account.
    if (!attachment)
    {
        return;
    }
    ANGLE_CONTEXT_TRY(prepareForClearBuffer(buffer, drawbuffer));
    ANGLE_CONTEXT_TRY(framebufferObject->clearBufferfv(this, buffer, drawbuffer, values));
}

void Context::clearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *values)
{
    if (noopClearBuffer(buffer, drawbuffer))
    {
        return;
    }

    Framebuffer *framebufferObject          = mState.getDrawFramebuffer();
    const FramebufferAttachment *attachment = nullptr;
    if (buffer == GL_COLOR &&
        static_cast<size_t>(drawbuffer) < framebufferObject->getNumColorAttachments())
    {
        attachment = framebufferObject->getColorAttachment(drawbuffer);
    }
    // It's not an error to try to clear a non-existent buffer, but it's a no-op. We early out so
    // that the backend doesn't need to take this case into account.
    if (!attachment)
    {
        return;
    }
    ANGLE_CONTEXT_TRY(prepareForClearBuffer(buffer, drawbuffer));
    ANGLE_CONTEXT_TRY(framebufferObject->clearBufferuiv(this, buffer, drawbuffer, values));
}

void Context::clearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *values)
{
    if (noopClearBuffer(buffer, drawbuffer))
    {
        return;
    }

    Framebuffer *framebufferObject          = mState.getDrawFramebuffer();
    const FramebufferAttachment *attachment = nullptr;
    if (buffer == GL_STENCIL)
    {
        attachment = framebufferObject->getStencilAttachment();
    }
    else if (buffer == GL_COLOR &&
             static_cast<size_t>(drawbuffer) < framebufferObject->getNumColorAttachments())
    {
        attachment = framebufferObject->getColorAttachment(drawbuffer);
    }
    // It's not an error to try to clear a non-existent buffer, but it's a no-op. We early out so
    // that the backend doesn't need to take this case into account.
    if (!attachment)
    {
        return;
    }
    ANGLE_CONTEXT_TRY(prepareForClearBuffer(buffer, drawbuffer));
    ANGLE_CONTEXT_TRY(framebufferObject->clearBufferiv(this, buffer, drawbuffer, values));
}

void Context::clearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    if (noopClearBuffer(buffer, drawbuffer))
    {
        return;
    }

    Framebuffer *framebufferObject = mState.getDrawFramebuffer();
    ASSERT(framebufferObject);

    // If a buffer is not present, the clear has no effect
    if (framebufferObject->getDepthAttachment() == nullptr &&
        framebufferObject->getStencilAttachment() == nullptr)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForClearBuffer(buffer, drawbuffer));
    ANGLE_CONTEXT_TRY(framebufferObject->clearBufferfi(this, buffer, drawbuffer, depth, stencil));
}

void Context::readPixels(GLint x,
                         GLint y,
                         GLsizei width,
                         GLsizei height,
                         GLenum format,
                         GLenum type,
                         void *pixels)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(syncStateForReadPixels());

    Framebuffer *readFBO = mState.getReadFramebuffer();
    ASSERT(readFBO);

    Rectangle area(x, y, width, height);
    PixelPackState packState = mState.getPackState();
    Buffer *packBuffer       = mState.getTargetBuffer(gl::BufferBinding::PixelPack);
    ANGLE_CONTEXT_TRY(readFBO->readPixels(this, area, format, type, packState, packBuffer, pixels));
}

void Context::readPixelsRobust(GLint x,
                               GLint y,
                               GLsizei width,
                               GLsizei height,
                               GLenum format,
                               GLenum type,
                               GLsizei bufSize,
                               GLsizei *length,
                               GLsizei *columns,
                               GLsizei *rows,
                               void *pixels)
{
    readPixels(x, y, width, height, format, type, pixels);
}

void Context::readnPixelsRobust(GLint x,
                                GLint y,
                                GLsizei width,
                                GLsizei height,
                                GLenum format,
                                GLenum type,
                                GLsizei bufSize,
                                GLsizei *length,
                                GLsizei *columns,
                                GLsizei *rows,
                                void *data)
{
    readPixels(x, y, width, height, format, type, data);
}

void Context::copyTexImage2D(TextureTarget target,
                             GLint level,
                             GLenum internalformat,
                             GLint x,
                             GLint y,
                             GLsizei width,
                             GLsizei height,
                             GLint border)
{
    ANGLE_CONTEXT_TRY(prepareForCopyImage());

    Rectangle sourceArea(x, y, width, height);

    Framebuffer *framebuffer = mState.getReadFramebuffer();
    Texture *texture         = getTextureByTarget(target);
    ANGLE_CONTEXT_TRY(
        texture->copyImage(this, target, level, sourceArea, internalformat, framebuffer));
}

void Context::copyTexSubImage2D(TextureTarget target,
                                GLint level,
                                GLint xoffset,
                                GLint yoffset,
                                GLint x,
                                GLint y,
                                GLsizei width,
                                GLsizei height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForCopyImage());

    Offset destOffset(xoffset, yoffset, 0);
    Rectangle sourceArea(x, y, width, height);

    ImageIndex index = ImageIndex::MakeFromTarget(target, level, 1);

    Framebuffer *framebuffer = mState.getReadFramebuffer();
    Texture *texture         = getTextureByTarget(target);
    ANGLE_CONTEXT_TRY(texture->copySubImage(this, index, destOffset, sourceArea, framebuffer));
}

void Context::copyTexSubImage3D(TextureTarget target,
                                GLint level,
                                GLint xoffset,
                                GLint yoffset,
                                GLint zoffset,
                                GLint x,
                                GLint y,
                                GLsizei width,
                                GLsizei height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForCopyImage());

    Offset destOffset(xoffset, yoffset, zoffset);
    Rectangle sourceArea(x, y, width, height);

    ImageIndex index = ImageIndex::MakeFromType(TextureTargetToType(target), level, zoffset);

    Framebuffer *framebuffer = mState.getReadFramebuffer();
    Texture *texture         = getTextureByTarget(target);
    ANGLE_CONTEXT_TRY(texture->copySubImage(this, index, destOffset, sourceArea, framebuffer));
}

void Context::copyImageSubData(GLuint srcName,
                               GLenum srcTarget,
                               GLint srcLevel,
                               GLint srcX,
                               GLint srcY,
                               GLint srcZ,
                               GLuint dstName,
                               GLenum dstTarget,
                               GLint dstLevel,
                               GLint dstX,
                               GLint dstY,
                               GLint dstZ,
                               GLsizei srcWidth,
                               GLsizei srcHeight,
                               GLsizei srcDepth)
{
    // if copy region is zero, the copy is a successful no-op
    if ((srcWidth == 0) || (srcHeight == 0) || (srcDepth == 0))
    {
        return;
    }

    if (srcTarget == GL_RENDERBUFFER)
    {
        // Source target is a Renderbuffer
        Renderbuffer *readBuffer = getRenderbuffer(PackParam<RenderbufferID>(srcName));
        if (dstTarget == GL_RENDERBUFFER)
        {
            // Destination target is a Renderbuffer
            Renderbuffer *writeBuffer = getRenderbuffer(PackParam<RenderbufferID>(dstName));

            // Copy Renderbuffer to Renderbuffer
            ANGLE_CONTEXT_TRY(writeBuffer->copyRenderbufferSubData(
                this, readBuffer, srcLevel, srcX, srcY, srcZ, dstLevel, dstX, dstY, dstZ, srcWidth,
                srcHeight, srcDepth));
        }
        else
        {
            // Destination target is a Texture
            ASSERT(dstTarget == GL_TEXTURE_2D || dstTarget == GL_TEXTURE_2D_ARRAY ||
                   dstTarget == GL_TEXTURE_3D || dstTarget == GL_TEXTURE_CUBE_MAP);

            Texture *writeTexture = getTexture(PackParam<TextureID>(dstName));
            ANGLE_CONTEXT_TRY(syncTextureForCopy(writeTexture));

            // Copy Renderbuffer to Texture
            ANGLE_CONTEXT_TRY(writeTexture->copyRenderbufferSubData(
                this, readBuffer, srcLevel, srcX, srcY, srcZ, dstLevel, dstX, dstY, dstZ, srcWidth,
                srcHeight, srcDepth));
        }
    }
    else
    {
        // Source target is a Texture
        ASSERT(srcTarget == GL_TEXTURE_2D || srcTarget == GL_TEXTURE_2D_ARRAY ||
               srcTarget == GL_TEXTURE_3D || srcTarget == GL_TEXTURE_CUBE_MAP ||
               srcTarget == GL_TEXTURE_EXTERNAL_OES || srcTarget == GL_TEXTURE_2D_MULTISAMPLE ||
               srcTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES);

        Texture *readTexture = getTexture(PackParam<TextureID>(srcName));
        ANGLE_CONTEXT_TRY(syncTextureForCopy(readTexture));

        if (dstTarget == GL_RENDERBUFFER)
        {
            // Destination target is a Renderbuffer
            Renderbuffer *writeBuffer = getRenderbuffer(PackParam<RenderbufferID>(dstName));

            // Copy Texture to Renderbuffer
            ANGLE_CONTEXT_TRY(writeBuffer->copyTextureSubData(this, readTexture, srcLevel, srcX,
                                                              srcY, srcZ, dstLevel, dstX, dstY,
                                                              dstZ, srcWidth, srcHeight, srcDepth));
        }
        else
        {
            // Destination target is a Texture
            ASSERT(dstTarget == GL_TEXTURE_2D || dstTarget == GL_TEXTURE_2D_ARRAY ||
                   dstTarget == GL_TEXTURE_3D || dstTarget == GL_TEXTURE_CUBE_MAP ||
                   dstTarget == GL_TEXTURE_EXTERNAL_OES || dstTarget == GL_TEXTURE_2D_MULTISAMPLE ||
                   dstTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES);

            Texture *writeTexture = getTexture(PackParam<TextureID>(dstName));
            ANGLE_CONTEXT_TRY(syncTextureForCopy(writeTexture));

            // Copy Texture to Texture
            ANGLE_CONTEXT_TRY(writeTexture->copyTextureSubData(
                this, readTexture, srcLevel, srcX, srcY, srcZ, dstLevel, dstX, dstY, dstZ, srcWidth,
                srcHeight, srcDepth));
        }
    }
}

void Context::framebufferTexture2D(GLenum target,
                                   GLenum attachment,
                                   TextureTarget textarget,
                                   TextureID texture,
                                   GLint level)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }

    if (texture.value != 0)
    {
        Texture *textureObj = getTexture(texture);
        ImageIndex index    = ImageIndex::MakeFromTarget(textarget, level, 1);
        framebuffer->setAttachment(this, GL_TEXTURE, attachment, index, textureObj);
    }
    else
    {
        framebuffer->resetAttachment(this, attachment);
    }

    mState.setObjectDirty(target);
}

void Context::framebufferTexture3D(GLenum target,
                                   GLenum attachment,
                                   TextureTarget textargetPacked,
                                   TextureID texture,
                                   GLint level,
                                   GLint zoffset)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }

    if (texture.value != 0)
    {
        Texture *textureObj = getTexture(texture);
        ImageIndex index    = ImageIndex::Make3D(level, zoffset);
        framebuffer->setAttachment(this, GL_TEXTURE, attachment, index, textureObj);
    }
    else
    {
        framebuffer->resetAttachment(this, attachment);
    }

    mState.setObjectDirty(target);
}

void Context::framebufferRenderbuffer(GLenum target,
                                      GLenum attachment,
                                      GLenum renderbuffertarget,
                                      RenderbufferID renderbuffer)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }

    if (renderbuffer.value != 0)
    {
        Renderbuffer *renderbufferObject = getRenderbuffer(renderbuffer);
        GLsizei rbSamples                = renderbufferObject->getState().getSamples();

        framebuffer->setAttachmentMultisample(this, GL_RENDERBUFFER, attachment, gl::ImageIndex(),
                                              renderbufferObject, rbSamples);
    }
    else
    {
        framebuffer->resetAttachment(this, attachment);
    }

    mState.setObjectDirty(target);
}

void Context::framebufferTextureLayer(GLenum target,
                                      GLenum attachment,
                                      TextureID texture,
                                      GLint level,
                                      GLint layer)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }

    if (texture.value != 0)
    {
        Texture *textureObject = getTexture(texture);
        ImageIndex index       = ImageIndex::MakeFromType(textureObject->getType(), level, layer);
        framebuffer->setAttachment(this, GL_TEXTURE, attachment, index, textureObject);
    }
    else
    {
        framebuffer->resetAttachment(this, attachment);
    }

    mState.setObjectDirty(target);
}

void Context::framebufferTextureMultiview(GLenum target,
                                          GLenum attachment,
                                          TextureID texture,
                                          GLint level,
                                          GLint baseViewIndex,
                                          GLsizei numViews)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }

    if (texture.value != 0)
    {
        Texture *textureObj = getTexture(texture);

        ImageIndex index;
        if (textureObj->getType() == TextureType::_2DArray)
        {
            index = ImageIndex::Make2DArrayRange(level, baseViewIndex, numViews);
        }
        else
        {
            ASSERT(textureObj->getType() == TextureType::_2DMultisampleArray);
            ASSERT(level == 0);
            index = ImageIndex::Make2DMultisampleArrayRange(baseViewIndex, numViews);
        }
        framebuffer->setAttachmentMultiview(this, GL_TEXTURE, attachment, index, textureObj,
                                            numViews, baseViewIndex);
    }
    else
    {
        framebuffer->resetAttachment(this, attachment);
    }

    mState.setObjectDirty(target);
}

void Context::framebufferTexture(GLenum target, GLenum attachment, TextureID texture, GLint level)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }

    if (texture.value != 0)
    {
        Texture *textureObj = getTexture(texture);

        ImageIndex index = ImageIndex::MakeFromType(
            textureObj->getType(), level, ImageIndex::kEntireLevel, ImageIndex::kEntireLevel);
        framebuffer->setAttachment(this, GL_TEXTURE, attachment, index, textureObj);
    }
    else
    {
        framebuffer->resetAttachment(this, attachment);
    }

    mState.setObjectDirty(target);
}

void Context::drawBuffers(GLsizei n, const GLenum *bufs)
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    if (mState.getPixelLocalStorageActivePlanes() != 0)
    {
        endPixelLocalStorageImplicit();
    }
    framebuffer->setDrawBuffers(n, bufs);
    mState.setDrawFramebufferDirty();
    mStateCache.onDrawFramebufferChange(this);
}

void Context::readBuffer(GLenum mode)
{
    Framebuffer *readFBO = mState.getReadFramebuffer();
    readFBO->setReadBuffer(mode);
    mState.setObjectDirty(GL_READ_FRAMEBUFFER);
}

void Context::discardFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    // The specification isn't clear what should be done when the framebuffer isn't complete.
    // We threat it the same way as GLES3 glInvalidateFramebuffer.
    invalidateFramebuffer(target, numAttachments, attachments);
}

void Context::invalidateFramebuffer(GLenum target,
                                    GLsizei numAttachments,
                                    const GLenum *attachments)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    // No-op incomplete FBOs.
    if (!framebuffer->isComplete(this))
    {
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForInvalidate(target));
    ANGLE_CONTEXT_TRY(framebuffer->invalidate(this, numAttachments, attachments));
}

void Context::invalidateSubFramebuffer(GLenum target,
                                       GLsizei numAttachments,
                                       const GLenum *attachments,
                                       GLint x,
                                       GLint y,
                                       GLsizei width,
                                       GLsizei height)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (!framebuffer->isComplete(this))
    {
        return;
    }

    Rectangle area(x, y, width, height);
    ANGLE_CONTEXT_TRY(prepareForInvalidate(target));
    ANGLE_CONTEXT_TRY(framebuffer->invalidateSub(this, numAttachments, attachments, area));
}

void Context::texImage2D(TextureTarget target,
                         GLint level,
                         GLint internalformat,
                         GLsizei width,
                         GLsizei height,
                         GLint border,
                         GLenum format,
                         GLenum type,
                         const void *pixels)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    gl::Buffer *unpackBuffer = mState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    Extents size(width, height, 1);
    Texture *texture = getTextureByTarget(target);
    ANGLE_CONTEXT_TRY(texture->setImage(this, mState.getUnpackState(), unpackBuffer, target, level,
                                        internalformat, size, format, type,
                                        static_cast<const uint8_t *>(pixels)));
}

void Context::texImage2DRobust(TextureTarget target,
                               GLint level,
                               GLint internalformat,
                               GLsizei width,
                               GLsizei height,
                               GLint border,
                               GLenum format,
                               GLenum type,
                               GLsizei bufSize,
                               const void *pixels)
{
    texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void Context::texImage3D(TextureTarget target,
                         GLint level,
                         GLint internalformat,
                         GLsizei width,
                         GLsizei height,
                         GLsizei depth,
                         GLint border,
                         GLenum format,
                         GLenum type,
                         const void *pixels)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    gl::Buffer *unpackBuffer = mState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    Extents size(width, height, depth);
    Texture *texture = getTextureByTarget(target);
    ANGLE_CONTEXT_TRY(texture->setImage(this, mState.getUnpackState(), unpackBuffer, target, level,
                                        internalformat, size, format, type,
                                        static_cast<const uint8_t *>(pixels)));
}

void Context::texImage3DRobust(TextureTarget target,
                               GLint level,
                               GLint internalformat,
                               GLsizei width,
                               GLsizei height,
                               GLsizei depth,
                               GLint border,
                               GLenum format,
                               GLenum type,
                               GLsizei bufSize,
                               const void *pixels)
{
    texImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void Context::texSubImage2D(TextureTarget target,
                            GLint level,
                            GLint xoffset,
                            GLint yoffset,
                            GLsizei width,
                            GLsizei height,
                            GLenum format,
                            GLenum type,
                            const void *pixels)
{
    // Zero sized uploads are valid but no-ops
    if (width == 0 || height == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Box area(xoffset, yoffset, 0, width, height, 1);
    Texture *texture = getTextureByTarget(target);

    gl::Buffer *unpackBuffer = mState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    ANGLE_CONTEXT_TRY(texture->setSubImage(this, mState.getUnpackState(), unpackBuffer, target,
                                           level, area, format, type,
                                           static_cast<const uint8_t *>(pixels)));
}

void Context::texSubImage2DRobust(TextureTarget target,
                                  GLint level,
                                  GLint xoffset,
                                  GLint yoffset,
                                  GLsizei width,
                                  GLsizei height,
                                  GLenum format,
                                  GLenum type,
                                  GLsizei bufSize,
                                  const void *pixels)
{
    texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void Context::texSubImage3D(TextureTarget target,
                            GLint level,
                            GLint xoffset,
                            GLint yoffset,
                            GLint zoffset,
                            GLsizei width,
                            GLsizei height,
                            GLsizei depth,
                            GLenum format,
                            GLenum type,
                            const void *pixels)
{
    // Zero sized uploads are valid but no-ops
    if (width == 0 || height == 0 || depth == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Box area(xoffset, yoffset, zoffset, width, height, depth);
    Texture *texture = getTextureByTarget(target);

    gl::Buffer *unpackBuffer = mState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    ANGLE_CONTEXT_TRY(texture->setSubImage(this, mState.getUnpackState(), unpackBuffer, target,
                                           level, area, format, type,
                                           static_cast<const uint8_t *>(pixels)));
}

void Context::texSubImage3DRobust(TextureTarget target,
                                  GLint level,
                                  GLint xoffset,
                                  GLint yoffset,
                                  GLint zoffset,
                                  GLsizei width,
                                  GLsizei height,
                                  GLsizei depth,
                                  GLenum format,
                                  GLenum type,
                                  GLsizei bufSize,
                                  const void *pixels)
{
    texSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type,
                  pixels);
}

void Context::compressedTexImage2D(TextureTarget target,
                                   GLint level,
                                   GLenum internalformat,
                                   GLsizei width,
                                   GLsizei height,
                                   GLint border,
                                   GLsizei imageSize,
                                   const void *data)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Extents size(width, height, 1);
    Texture *texture = getTextureByTarget(target);
    // From OpenGL ES 3 spec: All pixel storage modes are ignored when decoding a compressed texture
    // image. So we use an empty PixelUnpackState.
    ANGLE_CONTEXT_TRY(texture->setCompressedImage(this, PixelUnpackState(), target, level,
                                                  internalformat, size, imageSize,
                                                  static_cast<const uint8_t *>(data)));
}

void Context::compressedTexImage2DRobust(TextureTarget target,
                                         GLint level,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLint border,
                                         GLsizei imageSize,
                                         GLsizei dataSize,
                                         const GLvoid *data)
{
    compressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void Context::compressedTexImage3D(TextureTarget target,
                                   GLint level,
                                   GLenum internalformat,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLint border,
                                   GLsizei imageSize,
                                   const void *data)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Extents size(width, height, depth);
    Texture *texture = getTextureByTarget(target);
    // From OpenGL ES 3 spec: All pixel storage modes are ignored when decoding a compressed texture
    // image. So we use an empty PixelUnpackState.
    ANGLE_CONTEXT_TRY(texture->setCompressedImage(this, PixelUnpackState(), target, level,
                                                  internalformat, size, imageSize,
                                                  static_cast<const uint8_t *>(data)));
}

void Context::compressedTexImage3DRobust(TextureTarget target,
                                         GLint level,
                                         GLenum internalformat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth,
                                         GLint border,
                                         GLsizei imageSize,
                                         GLsizei dataSize,
                                         const GLvoid *data)
{
    compressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize,
                         data);
}

void Context::compressedTexSubImage2D(TextureTarget target,
                                      GLint level,
                                      GLint xoffset,
                                      GLint yoffset,
                                      GLsizei width,
                                      GLsizei height,
                                      GLenum format,
                                      GLsizei imageSize,
                                      const void *data)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Box area(xoffset, yoffset, 0, width, height, 1);
    Texture *texture = getTextureByTarget(target);
    // From OpenGL ES 3 spec: All pixel storage modes are ignored when decoding a compressed texture
    // image. So we use an empty PixelUnpackState.
    ANGLE_CONTEXT_TRY(texture->setCompressedSubImage(this, PixelUnpackState(), target, level, area,
                                                     format, imageSize,
                                                     static_cast<const uint8_t *>(data)));
}

void Context::compressedTexSubImage2DRobust(TextureTarget target,
                                            GLint level,
                                            GLint xoffset,
                                            GLint yoffset,
                                            GLsizei width,
                                            GLsizei height,
                                            GLenum format,
                                            GLsizei imageSize,
                                            GLsizei dataSize,
                                            const GLvoid *data)
{
    compressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize,
                            data);
}

void Context::compressedTexSubImage3D(TextureTarget target,
                                      GLint level,
                                      GLint xoffset,
                                      GLint yoffset,
                                      GLint zoffset,
                                      GLsizei width,
                                      GLsizei height,
                                      GLsizei depth,
                                      GLenum format,
                                      GLsizei imageSize,
                                      const void *data)
{
    // Zero sized uploads are valid but no-ops
    if (width == 0 || height == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Box area(xoffset, yoffset, zoffset, width, height, depth);
    Texture *texture = getTextureByTarget(target);
    // From OpenGL ES 3 spec: All pixel storage modes are ignored when decoding a compressed texture
    // image. So we use an empty PixelUnpackState.
    ANGLE_CONTEXT_TRY(texture->setCompressedSubImage(this, PixelUnpackState(), target, level, area,
                                                     format, imageSize,
                                                     static_cast<const uint8_t *>(data)));
}

void Context::compressedTexSubImage3DRobust(TextureTarget target,
                                            GLint level,
                                            GLint xoffset,
                                            GLint yoffset,
                                            GLint zoffset,
                                            GLsizei width,
                                            GLsizei height,
                                            GLsizei depth,
                                            GLenum format,
                                            GLsizei imageSize,
                                            GLsizei dataSize,
                                            const GLvoid *data)
{
    compressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format,
                            imageSize, data);
}

void Context::generateMipmap(TextureType target)
{
    Texture *texture = getTextureByType(target);
    ANGLE_CONTEXT_TRY(texture->generateMipmap(this));
}

void Context::copyTexture(TextureID sourceId,
                          GLint sourceLevel,
                          TextureTarget destTarget,
                          TextureID destId,
                          GLint destLevel,
                          GLint internalFormat,
                          GLenum destType,
                          GLboolean unpackFlipY,
                          GLboolean unpackPremultiplyAlpha,
                          GLboolean unpackUnmultiplyAlpha)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    gl::Texture *sourceTexture = getTexture(sourceId);
    gl::Texture *destTexture   = getTexture(destId);
    ANGLE_CONTEXT_TRY(
        destTexture->copyTexture(this, destTarget, destLevel, internalFormat, destType, sourceLevel,
                                 ConvertToBool(unpackFlipY), ConvertToBool(unpackPremultiplyAlpha),
                                 ConvertToBool(unpackUnmultiplyAlpha), sourceTexture));
}

void Context::copySubTexture(TextureID sourceId,
                             GLint sourceLevel,
                             TextureTarget destTarget,
                             TextureID destId,
                             GLint destLevel,
                             GLint xoffset,
                             GLint yoffset,
                             GLint x,
                             GLint y,
                             GLsizei width,
                             GLsizei height,
                             GLboolean unpackFlipY,
                             GLboolean unpackPremultiplyAlpha,
                             GLboolean unpackUnmultiplyAlpha)
{
    // Zero sized copies are valid but no-ops
    if (width == 0 || height == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    gl::Texture *sourceTexture = getTexture(sourceId);
    gl::Texture *destTexture   = getTexture(destId);
    Offset offset(xoffset, yoffset, 0);
    Box box(x, y, 0, width, height, 1);
    ANGLE_CONTEXT_TRY(destTexture->copySubTexture(
        this, destTarget, destLevel, offset, sourceLevel, box, ConvertToBool(unpackFlipY),
        ConvertToBool(unpackPremultiplyAlpha), ConvertToBool(unpackUnmultiplyAlpha),
        sourceTexture));
}

void Context::copyTexture3D(TextureID sourceId,
                            GLint sourceLevel,
                            TextureTarget destTarget,
                            TextureID destId,
                            GLint destLevel,
                            GLint internalFormat,
                            GLenum destType,
                            GLboolean unpackFlipY,
                            GLboolean unpackPremultiplyAlpha,
                            GLboolean unpackUnmultiplyAlpha)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Texture *sourceTexture = getTexture(sourceId);
    Texture *destTexture   = getTexture(destId);
    ANGLE_CONTEXT_TRY(
        destTexture->copyTexture(this, destTarget, destLevel, internalFormat, destType, sourceLevel,
                                 ConvertToBool(unpackFlipY), ConvertToBool(unpackPremultiplyAlpha),
                                 ConvertToBool(unpackUnmultiplyAlpha), sourceTexture));
}

void Context::copySubTexture3D(TextureID sourceId,
                               GLint sourceLevel,
                               TextureTarget destTarget,
                               TextureID destId,
                               GLint destLevel,
                               GLint xoffset,
                               GLint yoffset,
                               GLint zoffset,
                               GLint x,
                               GLint y,
                               GLint z,
                               GLsizei width,
                               GLsizei height,
                               GLsizei depth,
                               GLboolean unpackFlipY,
                               GLboolean unpackPremultiplyAlpha,
                               GLboolean unpackUnmultiplyAlpha)
{
    // Zero sized copies are valid but no-ops
    if (width == 0 || height == 0 || depth == 0)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    Texture *sourceTexture = getTexture(sourceId);
    Texture *destTexture   = getTexture(destId);
    Offset offset(xoffset, yoffset, zoffset);
    Box box(x, y, z, width, height, depth);
    ANGLE_CONTEXT_TRY(destTexture->copySubTexture(
        this, destTarget, destLevel, offset, sourceLevel, box, ConvertToBool(unpackFlipY),
        ConvertToBool(unpackPremultiplyAlpha), ConvertToBool(unpackUnmultiplyAlpha),
        sourceTexture));
}

void Context::compressedCopyTexture(TextureID sourceId, TextureID destId)
{
    ANGLE_CONTEXT_TRY(syncStateForTexImage());

    gl::Texture *sourceTexture = getTexture(sourceId);
    gl::Texture *destTexture   = getTexture(destId);
    ANGLE_CONTEXT_TRY(destTexture->copyCompressedTexture(this, sourceTexture));
}

void Context::getBufferPointerv(BufferBinding target, GLenum pname, void **params)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);

    QueryBufferPointerv(buffer, pname, params);
}

void Context::getBufferPointervRobust(BufferBinding target,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      GLsizei *length,
                                      void **params)
{
    getBufferPointerv(target, pname, params);
}

void *Context::mapBuffer(BufferBinding target, GLenum access)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);

    if (buffer->map(this, access) == angle::Result::Stop)
    {
        return nullptr;
    }

    return buffer->getMapPointer();
}

GLboolean Context::unmapBuffer(BufferBinding target)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);

    GLboolean result;
    if (buffer->unmap(this, &result) == angle::Result::Stop)
    {
        return GL_FALSE;
    }

    return result;
}

void *Context::mapBufferRange(BufferBinding target,
                              GLintptr offset,
                              GLsizeiptr length,
                              GLbitfield access)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);

    if (buffer->mapRange(this, offset, length, access) == angle::Result::Stop)
    {
        return nullptr;
    }

    // TODO: (anglebug.com/42266294): Modify return value in entry point layer
    angle::FrameCaptureShared *frameCaptureShared = getShareGroup()->getFrameCaptureShared();
    if (frameCaptureShared->enabled())
    {
        return frameCaptureShared->maybeGetShadowMemoryPointer(buffer, length, access);
    }
    else
    {
        return buffer->getMapPointer();
    }
}

void Context::flushMappedBufferRange(BufferBinding /*target*/,
                                     GLintptr /*offset*/,
                                     GLsizeiptr /*length*/)
{
    // We do not currently support a non-trivial implementation of FlushMappedBufferRange
}

angle::Result Context::syncStateForReadPixels()
{
    return syncState(kReadPixelsDirtyBits, kReadPixelsExtendedDirtyBits, mReadPixelsDirtyObjects,
                     Command::ReadPixels);
}

angle::Result Context::syncStateForTexImage()
{
    return syncState(kTexImageDirtyBits, kTexImageExtendedDirtyBits, mTexImageDirtyObjects,
                     Command::TexImage);
}

angle::Result Context::syncStateForBlit(GLbitfield mask)
{
    uint32_t commandMask = 0;
    if ((mask & GL_COLOR_BUFFER_BIT) != 0)
    {
        commandMask |= CommandBlitBufferColor;
    }
    if ((mask & GL_DEPTH_BUFFER_BIT) != 0)
    {
        commandMask |= CommandBlitBufferDepth;
    }
    if ((mask & GL_STENCIL_BUFFER_BIT) != 0)
    {
        commandMask |= CommandBlitBufferStencil;
    }

    Command command = static_cast<Command>(static_cast<uint32_t>(Command::Blit) + commandMask);

    return syncState(kBlitDirtyBits, kBlitExtendedDirtyBits, mBlitDirtyObjects, command);
}

angle::Result Context::syncStateForClear()
{
    return syncState(kClearDirtyBits, kClearExtendedDirtyBits, mClearDirtyObjects, Command::Clear);
}

angle::Result Context::syncTextureForCopy(Texture *texture)
{
    ASSERT(texture);
    // Sync texture not active but scheduled for a copy
    if (texture->hasAnyDirtyBit())
    {
        return texture->syncState(this, Command::Other);
    }

    return angle::Result::Continue;
}

void Context::activeShaderProgram(ProgramPipelineID pipeline, ShaderProgramID program)
{
    Program *shaderProgram = getProgramNoResolveLink(program);
    ProgramPipeline *programPipeline =
        mState.mProgramPipelineManager->checkProgramPipelineAllocation(mImplementation.get(),
                                                                       pipeline);
    ASSERT(programPipeline);

    programPipeline->activeShaderProgram(shaderProgram);
}

void Context::blendBarrier()
{
    mImplementation->blendBarrier();
}

void Context::disableVertexAttribArray(GLuint index)
{
    mState.setEnableVertexAttribArray(index, false);
    mStateCache.onVertexArrayStateChange(this);
}

void Context::enableVertexAttribArray(GLuint index)
{
    mState.setEnableVertexAttribArray(index, true);
    mStateCache.onVertexArrayStateChange(this);
}

void Context::vertexAttribPointer(GLuint index,
                                  GLint size,
                                  VertexAttribType type,
                                  GLboolean normalized,
                                  GLsizei stride,
                                  const void *ptr)
{
    mState.setVertexAttribPointer(this, index, mState.getTargetBuffer(BufferBinding::Array), size,
                                  type, ConvertToBool(normalized), stride, ptr);
    mStateCache.onVertexArrayStateChange(this);
}

void Context::vertexAttribFormat(GLuint attribIndex,
                                 GLint size,
                                 VertexAttribType type,
                                 GLboolean normalized,
                                 GLuint relativeOffset)
{
    mState.setVertexAttribFormat(attribIndex, size, type, ConvertToBool(normalized), false,
                                 relativeOffset);
    mStateCache.onVertexArrayFormatChange(this);
}

void Context::vertexAttribIFormat(GLuint attribIndex,
                                  GLint size,
                                  VertexAttribType type,
                                  GLuint relativeOffset)
{
    mState.setVertexAttribFormat(attribIndex, size, type, false, true, relativeOffset);
    mStateCache.onVertexArrayFormatChange(this);
}

void Context::vertexAttribBinding(GLuint attribIndex, GLuint bindingIndex)
{
    mState.setVertexAttribBinding(this, attribIndex, bindingIndex);
    mStateCache.onVertexArrayStateChange(this);
}

void Context::vertexBindingDivisor(GLuint bindingIndex, GLuint divisor)
{
    mState.setVertexBindingDivisor(this, bindingIndex, divisor);
    mStateCache.onVertexArrayFormatChange(this);
}

void Context::vertexAttribIPointer(GLuint index,
                                   GLint size,
                                   VertexAttribType type,
                                   GLsizei stride,
                                   const void *pointer)
{
    mState.setVertexAttribIPointer(this, index, mState.getTargetBuffer(BufferBinding::Array), size,
                                   type, stride, pointer);
    mStateCache.onVertexArrayStateChange(this);
}

void Context::getVertexAttribivImpl(GLuint index, GLenum pname, GLint *params) const
{
    const VertexAttribCurrentValueData &currentValues =
        getState().getVertexAttribCurrentValue(index);
    const VertexArray *vao = getState().getVertexArray();
    QueryVertexAttribiv(vao->getVertexAttribute(index), vao->getBindingFromAttribIndex(index),
                        currentValues, pname, params);
}

void Context::getVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    return getVertexAttribivImpl(index, pname, params);
}

void Context::getVertexAttribivRobust(GLuint index,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      GLsizei *length,
                                      GLint *params)
{
    getVertexAttribiv(index, pname, params);
}

void Context::getVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    const VertexAttribCurrentValueData &currentValues =
        getState().getVertexAttribCurrentValue(index);
    const VertexArray *vao = getState().getVertexArray();
    QueryVertexAttribfv(vao->getVertexAttribute(index), vao->getBindingFromAttribIndex(index),
                        currentValues, pname, params);
}

void Context::getVertexAttribfvRobust(GLuint index,
                                      GLenum pname,
                                      GLsizei bufSize,
                                      GLsizei *length,
                                      GLfloat *params)
{
    getVertexAttribfv(index, pname, params);
}

void Context::getVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
    const VertexAttribCurrentValueData &currentValues =
        getState().getVertexAttribCurrentValue(index);
    const VertexArray *vao = getState().getVertexArray();
    QueryVertexAttribIiv(vao->getVertexAttribute(index), vao->getBindingFromAttribIndex(index),
                         currentValues, pname, params);
}

void Context::getVertexAttribIivRobust(GLuint index,
                                       GLenum pname,
                                       GLsizei bufSize,
                                       GLsizei *length,
                                       GLint *params)
{
    getVertexAttribIiv(index, pname, params);
}

void Context::getVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
    const VertexAttribCurrentValueData &currentValues =
        getState().getVertexAttribCurrentValue(index);
    const VertexArray *vao = getState().getVertexArray();
    QueryVertexAttribIuiv(vao->getVertexAttribute(index), vao->getBindingFromAttribIndex(index),
                          currentValues, pname, params);
}

void Context::getVertexAttribIuivRobust(GLuint index,
                                        GLenum pname,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLuint *params)
{
    getVertexAttribIuiv(index, pname, params);
}

void Context::getVertexAttribPointerv(GLuint index, GLenum pname, void **pointer)
{
    const VertexAttribute &attrib = getState().getVertexArray()->getVertexAttribute(index);
    QueryVertexAttribPointerv(attrib, pname, pointer);
}

void Context::getVertexAttribPointervRobust(GLuint index,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            GLsizei *length,
                                            void **pointer)
{
    getVertexAttribPointerv(index, pname, pointer);
}

void Context::debugMessageControl(GLenum source,
                                  GLenum type,
                                  GLenum severity,
                                  GLsizei count,
                                  const GLuint *ids,
                                  GLboolean enabled)
{
    std::vector<GLuint> idVector(ids, ids + count);
    mState.getDebug().setMessageControl(source, type, severity, std::move(idVector),
                                        ConvertToBool(enabled));
}

void Context::debugMessageInsert(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 const GLchar *buf)
{
    std::string msg(buf, (length > 0) ? static_cast<size_t>(length) : strlen(buf));
    mState.getDebug().insertMessage(source, type, id, severity, std::move(msg), gl::LOG_INFO,
                                    angle::EntryPoint::GLDebugMessageInsert);
}

void Context::debugMessageCallback(GLDEBUGPROCKHR callback, const void *userParam)
{
    mState.getDebug().setCallback(callback, userParam);
}

GLuint Context::getDebugMessageLog(GLuint count,
                                   GLsizei bufSize,
                                   GLenum *sources,
                                   GLenum *types,
                                   GLuint *ids,
                                   GLenum *severities,
                                   GLsizei *lengths,
                                   GLchar *messageLog)
{
    return static_cast<GLuint>(mState.getDebug().getMessages(count, bufSize, sources, types, ids,
                                                             severities, lengths, messageLog));
}

void Context::pushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message)
{
    std::string msg(message, (length > 0) ? static_cast<size_t>(length) : strlen(message));
    ANGLE_CONTEXT_TRY(mImplementation->pushDebugGroup(this, source, id, msg));
    mState.getDebug().pushGroup(source, id, std::move(msg));
}

angle::Result Context::handleNoopDrawEvent()
{
    return (mImplementation->handleNoopDrawEvent());
}

void Context::popDebugGroup()
{
    mState.getDebug().popGroup();
    ANGLE_CONTEXT_TRY(mImplementation->popDebugGroup(this));
}

void Context::bufferStorage(BufferBinding target,
                            GLsizeiptr size,
                            const void *data,
                            GLbitfield flags)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);
    ANGLE_CONTEXT_TRY(buffer->bufferStorage(this, target, size, data, flags));
}

void Context::bufferStorageExternal(BufferBinding target,
                                    GLintptr offset,
                                    GLsizeiptr size,
                                    GLeglClientBufferEXT clientBuffer,
                                    GLbitfield flags)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);

    ANGLE_CONTEXT_TRY(buffer->bufferStorageExternal(this, target, size, clientBuffer, flags));
}

void Context::namedBufferStorageExternal(GLuint buffer,
                                         GLintptr offset,
                                         GLsizeiptr size,
                                         GLeglClientBufferEXT clientBuffer,
                                         GLbitfield flags)
{
    UNIMPLEMENTED();
}

void Context::bufferData(BufferBinding target, GLsizeiptr size, const void *data, BufferUsage usage)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);
    ANGLE_CONTEXT_TRY(buffer->bufferData(this, target, data, size, usage));
}

void Context::bufferSubData(BufferBinding target,
                            GLintptr offset,
                            GLsizeiptr size,
                            const void *data)
{
    if (data == nullptr || size == 0)
    {
        return;
    }

    Buffer *buffer = mState.getTargetBuffer(target);
    ASSERT(buffer);
    ANGLE_CONTEXT_TRY(buffer->bufferSubData(this, target, data, size, offset));
}

void Context::attachShader(ShaderProgramID program, ShaderProgramID shader)
{
    Program *programObject = mState.mShaderProgramManager->getProgram(program);
    Shader *shaderObject   = mState.mShaderProgramManager->getShader(shader);
    ASSERT(programObject && shaderObject);
    programObject->attachShader(this, shaderObject);
}

void Context::copyBufferSubData(BufferBinding readTarget,
                                BufferBinding writeTarget,
                                GLintptr readOffset,
                                GLintptr writeOffset,
                                GLsizeiptr size)
{
    // if size is zero, the copy is a successful no-op
    if (size == 0)
    {
        return;
    }

    // TODO(jmadill): cache these.
    Buffer *readBuffer  = mState.getTargetBuffer(readTarget);
    Buffer *writeBuffer = mState.getTargetBuffer(writeTarget);

    ANGLE_CONTEXT_TRY(
        writeBuffer->copyBufferSubData(this, readBuffer, readOffset, writeOffset, size));
}

void Context::bindAttribLocation(ShaderProgramID program, GLuint index, const GLchar *name)
{
    // Ideally we could share the program query with the validation layer if possible.
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->bindAttributeLocation(this, index, name);
}

void Context::bindBufferBase(BufferBinding target, GLuint index, BufferID buffer)
{
    bindBufferRange(target, index, buffer, 0, 0);
}

void Context::bindBufferRange(BufferBinding target,
                              GLuint index,
                              BufferID buffer,
                              GLintptr offset,
                              GLsizeiptr size)
{
    Buffer *object = mState.mBufferManager->checkBufferAllocation(mImplementation.get(), buffer);
    ANGLE_CONTEXT_TRY(mState.setIndexedBufferBinding(this, target, index, object, offset, size));
    if (target == BufferBinding::Uniform)
    {
        mUniformBufferObserverBindings[index].bind(object);
        mState.onUniformBufferStateChange(index);
        mStateCache.onUniformBufferStateChange(this);
    }
    else if (target == BufferBinding::AtomicCounter)
    {
        mAtomicCounterBufferObserverBindings[index].bind(object);
        mStateCache.onAtomicCounterBufferStateChange(this);
    }
    else if (target == BufferBinding::ShaderStorage)
    {
        mShaderStorageBufferObserverBindings[index].bind(object);
        mStateCache.onShaderStorageBufferStateChange(this);
    }
    else
    {
        mStateCache.onBufferBindingChange(this);
    }

    if (object)
    {
        object->onBind(this, target);
    }
}

void Context::bindFramebuffer(GLenum target, FramebufferID framebuffer)
{
    if (target == GL_READ_FRAMEBUFFER || target == GL_FRAMEBUFFER)
    {
        bindReadFramebuffer(framebuffer);
    }

    if (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
    {
        bindDrawFramebuffer(framebuffer);
    }
}

void Context::bindRenderbuffer(GLenum target, RenderbufferID renderbuffer)
{
    ASSERT(target == GL_RENDERBUFFER);
    Renderbuffer *object = mState.mRenderbufferManager->checkRenderbufferAllocation(
        mImplementation.get(), renderbuffer);
    mState.setRenderbufferBinding(this, object);
}

void Context::texStorage2DMultisample(TextureType target,
                                      GLsizei samples,
                                      GLenum internalformat,
                                      GLsizei width,
                                      GLsizei height,
                                      GLboolean fixedsamplelocations)
{
    Extents size(width, height, 1);
    Texture *texture = getTextureByType(target);
    ANGLE_CONTEXT_TRY(texture->setStorageMultisample(this, target, samples, internalformat, size,
                                                     ConvertToBool(fixedsamplelocations)));
}

void Context::texStorage3DMultisample(TextureType target,
                                      GLsizei samples,
                                      GLenum internalformat,
                                      GLsizei width,
                                      GLsizei height,
                                      GLsizei depth,
                                      GLboolean fixedsamplelocations)
{
    Extents size(width, height, depth);
    Texture *texture = getTextureByType(target);
    ANGLE_CONTEXT_TRY(texture->setStorageMultisample(this, target, samples, internalformat, size,
                                                     ConvertToBool(fixedsamplelocations)));
}

void Context::texImage2DExternal(TextureTarget target,
                                 GLint level,
                                 GLint internalformat,
                                 GLsizei width,
                                 GLsizei height,
                                 GLint border,
                                 GLenum format,
                                 GLenum type)
{
    Extents size(width, height, 1);
    Texture *texture = getTextureByTarget(target);
    ANGLE_CONTEXT_TRY(
        texture->setImageExternal(this, target, level, internalformat, size, format, type));
}

void Context::invalidateTexture(TextureType target)
{
    mImplementation->invalidateTexture(target);
    mState.invalidateTextureBindings(target);
}

void Context::getMultisamplefv(GLenum pname, GLuint index, GLfloat *val)
{
    // According to spec 3.1 Table 20.49: Framebuffer Dependent Values,
    // the sample position should be queried by DRAW_FRAMEBUFFER.
    ANGLE_CONTEXT_TRY(mState.syncDirtyObject(this, GL_DRAW_FRAMEBUFFER));
    const Framebuffer *framebuffer = mState.getDrawFramebuffer();

    switch (pname)
    {
        case GL_SAMPLE_POSITION:
            ANGLE_CONTEXT_TRY(framebuffer->getSamplePosition(this, index, val));
            break;
        default:
            UNREACHABLE();
    }
}

void Context::getMultisamplefvRobust(GLenum pname,
                                     GLuint index,
                                     GLsizei bufSize,
                                     GLsizei *length,
                                     GLfloat *val)
{
    UNIMPLEMENTED();
}

void Context::renderbufferStorage(GLenum target,
                                  GLenum internalformat,
                                  GLsizei width,
                                  GLsizei height)
{
    // Hack for the special WebGL 1 "DEPTH_STENCIL" internal format.
    GLenum convertedInternalFormat = getConvertedRenderbufferFormat(internalformat);

    Renderbuffer *renderbuffer = mState.getCurrentRenderbuffer();
    ANGLE_CONTEXT_TRY(renderbuffer->setStorage(this, convertedInternalFormat, width, height));
}

void Context::renderbufferStorageMultisample(GLenum target,
                                             GLsizei samples,
                                             GLenum internalformat,
                                             GLsizei width,
                                             GLsizei height)
{
    renderbufferStorageMultisampleImpl(target, samples, internalformat, width, height,
                                       MultisamplingMode::Regular);
}

void Context::renderbufferStorageMultisampleEXT(GLenum target,
                                                GLsizei samples,
                                                GLenum internalformat,
                                                GLsizei width,
                                                GLsizei height)
{
    renderbufferStorageMultisampleImpl(target, samples, internalformat, width, height,
                                       MultisamplingMode::MultisampledRenderToTexture);
}

void Context::renderbufferStorageMultisampleImpl(GLenum target,
                                                 GLsizei samples,
                                                 GLenum internalformat,
                                                 GLsizei width,
                                                 GLsizei height,
                                                 MultisamplingMode mode)
{
    // Hack for the special WebGL 1 "DEPTH_STENCIL" internal format.
    GLenum convertedInternalFormat = getConvertedRenderbufferFormat(internalformat);

    Renderbuffer *renderbuffer = mState.getCurrentRenderbuffer();
    ANGLE_CONTEXT_TRY(renderbuffer->setStorageMultisample(this, samples, convertedInternalFormat,
                                                          width, height, mode));
}

void Context::framebufferTexture2DMultisample(GLenum target,
                                              GLenum attachment,
                                              TextureTarget textarget,
                                              TextureID texture,
                                              GLint level,
                                              GLsizei samples)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }

    if (texture.value != 0)
    {
        Texture *textureObj = getTexture(texture);
        ImageIndex index    = ImageIndex::MakeFromTarget(textarget, level, 1);
        framebuffer->setAttachmentMultisample(this, GL_TEXTURE, attachment, index, textureObj,
                                              samples);
        textureObj->onBindToMSRTTFramebuffer();
    }
    else
    {
        framebuffer->resetAttachment(this, attachment);
    }

    mState.setObjectDirty(target);
}

void Context::getSynciv(SyncID syncPacked,
                        GLenum pname,
                        GLsizei bufSize,
                        GLsizei *length,
                        GLint *values)
{
    const Sync *syncObject = nullptr;
    if (!isContextLost())
    {
        syncObject = getSync(syncPacked);
    }
    ANGLE_CONTEXT_TRY(QuerySynciv(this, syncObject, pname, bufSize, length, values));
}

void Context::getFramebufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    QueryFramebufferParameteriv(framebuffer, pname, params);
}

void Context::getFramebufferParameterivRobust(GLenum target,
                                              GLenum pname,
                                              GLsizei bufSize,
                                              GLsizei *length,
                                              GLint *params)
{
    UNIMPLEMENTED();
}

void Context::framebufferParameteri(GLenum target, GLenum pname, GLint param)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    if (mState.getPixelLocalStorageActivePlanes() != 0 &&
        framebuffer == mState.getDrawFramebuffer())
    {
        endPixelLocalStorageImplicit();
    }
    SetFramebufferParameteri(this, framebuffer, pname, param);
}

bool Context::getScratchBuffer(size_t requstedSizeBytes,
                               angle::MemoryBuffer **scratchBufferOut) const
{
    if (!mScratchBuffer.valid())
    {
        mScratchBuffer = mDisplay->requestScratchBuffer();
    }

    ASSERT(mScratchBuffer.valid());
    return mScratchBuffer.value().get(requstedSizeBytes, scratchBufferOut);
}

angle::ScratchBuffer *Context::getScratchBuffer() const
{
    if (!mScratchBuffer.valid())
    {
        mScratchBuffer = mDisplay->requestScratchBuffer();
    }

    ASSERT(mScratchBuffer.valid());
    return &mScratchBuffer.value();
}

bool Context::getZeroFilledBuffer(size_t requstedSizeBytes,
                                  angle::MemoryBuffer **zeroBufferOut) const
{
    if (!mZeroFilledBuffer.valid())
    {
        mZeroFilledBuffer = mDisplay->requestZeroFilledBuffer();
    }

    ASSERT(mZeroFilledBuffer.valid());
    return mZeroFilledBuffer.value().getInitialized(requstedSizeBytes, zeroBufferOut, 0);
}

void Context::dispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ)
{
    if (numGroupsX == 0u || numGroupsY == 0u || numGroupsZ == 0u)
    {
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDispatch());

    angle::Result result =
        mImplementation->dispatchCompute(this, numGroupsX, numGroupsY, numGroupsZ);

    // This must be called before convertPpoToComputeOrDraw() so it uses the PPO's compute values
    // before convertPpoToComputeOrDraw() reverts the PPO back to graphics.
    MarkShaderStorageUsage(this);

    if (ANGLE_UNLIKELY(IsError(result)))
    {
        return;
    }
}

void Context::dispatchComputeIndirect(GLintptr indirect)
{
    ANGLE_CONTEXT_TRY(prepareForDispatch());
    ANGLE_CONTEXT_TRY(mImplementation->dispatchComputeIndirect(this, indirect));

    MarkShaderStorageUsage(this);
}

void Context::texStorage2D(TextureType target,
                           GLsizei levels,
                           GLenum internalFormat,
                           GLsizei width,
                           GLsizei height)
{
    Extents size(width, height, 1);
    Texture *texture = getTextureByType(target);
    ANGLE_CONTEXT_TRY(texture->setStorage(this, target, levels, internalFormat, size));
}

void Context::texStorage3D(TextureType target,
                           GLsizei levels,
                           GLenum internalFormat,
                           GLsizei width,
                           GLsizei height,
                           GLsizei depth)
{
    Extents size(width, height, depth);
    Texture *texture = getTextureByType(target);
    ANGLE_CONTEXT_TRY(texture->setStorage(this, target, levels, internalFormat, size));
}

void Context::memoryBarrier(GLbitfield barriers)
{
    ANGLE_CONTEXT_TRY(mImplementation->memoryBarrier(this, barriers));
}

void Context::memoryBarrierByRegion(GLbitfield barriers)
{
    ANGLE_CONTEXT_TRY(mImplementation->memoryBarrierByRegion(this, barriers));
}

void Context::multiDrawArrays(PrimitiveMode mode,
                              const GLint *firsts,
                              const GLsizei *counts,
                              GLsizei drawcount)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->multiDrawArrays(this, mode, firsts, counts, drawcount));
}

void Context::multiDrawArraysInstanced(PrimitiveMode mode,
                                       const GLint *firsts,
                                       const GLsizei *counts,
                                       const GLsizei *instanceCounts,
                                       GLsizei drawcount)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->multiDrawArraysInstanced(this, mode, firsts, counts,
                                                                instanceCounts, drawcount));
}

void Context::multiDrawArraysIndirect(PrimitiveMode mode,
                                      const void *indirect,
                                      GLsizei drawcount,
                                      GLsizei stride)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(
        mImplementation->multiDrawArraysIndirect(this, mode, indirect, drawcount, stride));
    MarkShaderStorageUsage(this);
}

void Context::multiDrawElements(PrimitiveMode mode,
                                const GLsizei *counts,
                                DrawElementsType type,
                                const GLvoid *const *indices,
                                GLsizei drawcount)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(
        mImplementation->multiDrawElements(this, mode, counts, type, indices, drawcount));
}

void Context::multiDrawElementsInstanced(PrimitiveMode mode,
                                         const GLsizei *counts,
                                         DrawElementsType type,
                                         const GLvoid *const *indices,
                                         const GLsizei *instanceCounts,
                                         GLsizei drawcount)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->multiDrawElementsInstanced(this, mode, counts, type, indices,
                                                                  instanceCounts, drawcount));
}

void Context::multiDrawElementsIndirect(PrimitiveMode mode,
                                        DrawElementsType type,
                                        const void *indirect,
                                        GLsizei drawcount,
                                        GLsizei stride)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(
        mImplementation->multiDrawElementsIndirect(this, mode, type, indirect, drawcount, stride));
    MarkShaderStorageUsage(this);
}

void Context::drawArraysInstancedBaseInstance(PrimitiveMode mode,
                                              GLint first,
                                              GLsizei count,
                                              GLsizei instanceCount,
                                              GLuint baseInstance)
{
    if (noopDrawInstanced(mode, count, instanceCount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ProgramExecutable *executable = mState.getLinkedProgramExecutable(this);

    const bool hasBaseInstance = executable->hasBaseInstanceUniform();
    if (hasBaseInstance)
    {
        executable->setBaseInstanceUniform(baseInstance);
    }

    rx::ResetBaseVertexBaseInstance resetUniforms(executable, false, hasBaseInstance);

    // The input gl_InstanceID does not follow the baseinstance. gl_InstanceID always falls on
    // the half-open range [0, instancecount). No need to set other stuff. Except for Vulkan.

    ANGLE_CONTEXT_TRY(mImplementation->drawArraysInstancedBaseInstance(
        this, mode, first, count, instanceCount, baseInstance));
    MarkTransformFeedbackBufferUsage(this, count, 1);
}

void Context::drawArraysInstancedBaseInstanceANGLE(PrimitiveMode mode,
                                                   GLint first,
                                                   GLsizei count,
                                                   GLsizei instanceCount,
                                                   GLuint baseInstance)
{
    drawArraysInstancedBaseInstance(mode, first, count, instanceCount, baseInstance);
}

void Context::drawElementsInstancedBaseInstance(PrimitiveMode mode,
                                                GLsizei count,
                                                DrawElementsType type,
                                                const void *indices,
                                                GLsizei instanceCount,
                                                GLuint baseInstance)
{
    drawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instanceCount, 0,
                                                baseInstance);
}

void Context::drawElementsInstancedBaseVertexBaseInstance(PrimitiveMode mode,
                                                          GLsizei count,
                                                          DrawElementsType type,
                                                          const GLvoid *indices,
                                                          GLsizei instanceCount,
                                                          GLint baseVertex,
                                                          GLuint baseInstance)
{
    if (noopDrawInstanced(mode, count, instanceCount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ProgramExecutable *executable = mState.getLinkedProgramExecutable(this);

    const bool hasBaseVertex   = executable->hasBaseVertexUniform();
    const bool hasBaseInstance = executable->hasBaseInstanceUniform();

    if (hasBaseVertex)
    {
        executable->setBaseVertexUniform(baseVertex);
    }

    if (hasBaseInstance)
    {
        executable->setBaseInstanceUniform(baseInstance);
    }

    rx::ResetBaseVertexBaseInstance resetUniforms(executable, hasBaseVertex, hasBaseInstance);

    ANGLE_CONTEXT_TRY(mImplementation->drawElementsInstancedBaseVertexBaseInstance(
        this, mode, count, type, indices, instanceCount, baseVertex, baseInstance));
}

void Context::drawElementsInstancedBaseVertexBaseInstanceANGLE(PrimitiveMode mode,
                                                               GLsizei count,
                                                               DrawElementsType type,
                                                               const GLvoid *indices,
                                                               GLsizei instanceCount,
                                                               GLint baseVertex,
                                                               GLuint baseInstance)
{
    drawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instanceCount,
                                                baseVertex, baseInstance);
}

void Context::multiDrawArraysInstancedBaseInstance(PrimitiveMode mode,
                                                   const GLint *firsts,
                                                   const GLsizei *counts,
                                                   const GLsizei *instanceCounts,
                                                   const GLuint *baseInstances,
                                                   GLsizei drawcount)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->multiDrawArraysInstancedBaseInstance(
        this, mode, firsts, counts, instanceCounts, baseInstances, drawcount));
}

void Context::multiDrawElementsBaseVertex(PrimitiveMode mode,
                                          const GLsizei *count,
                                          DrawElementsType type,
                                          const void *const *indices,
                                          GLsizei drawcount,
                                          const GLint *basevertex)
{
    UNIMPLEMENTED();
}

void Context::multiDrawElementsInstancedBaseVertexBaseInstance(PrimitiveMode mode,
                                                               const GLsizei *counts,
                                                               DrawElementsType type,
                                                               const GLvoid *const *indices,
                                                               const GLsizei *instanceCounts,
                                                               const GLint *baseVertices,
                                                               const GLuint *baseInstances,
                                                               GLsizei drawcount)
{
    if (noopMultiDraw(drawcount))
    {
        ANGLE_CONTEXT_TRY(mImplementation->handleNoopDrawEvent());
        return;
    }

    ANGLE_CONTEXT_TRY(prepareForDraw(mode));
    ANGLE_CONTEXT_TRY(mImplementation->multiDrawElementsInstancedBaseVertexBaseInstance(
        this, mode, counts, type, indices, instanceCounts, baseVertices, baseInstances, drawcount));
}

GLenum Context::checkFramebufferStatus(GLenum target)
{
    Framebuffer *framebuffer = mState.getTargetFramebuffer(target);
    ASSERT(framebuffer);
    return framebuffer->checkStatus(this).status;
}

void Context::compileShader(ShaderProgramID shader)
{
    Shader *shaderObject = GetValidShader(this, angle::EntryPoint::GLCompileShader, shader);
    if (!shaderObject)
    {
        return;
    }
    shaderObject->compile(this, angle::JobResultExpectancy::Future);
}

void Context::deleteBuffers(GLsizei n, const BufferID *buffers)
{
    for (int i = 0; i < n; i++)
    {
        deleteBuffer(buffers[i]);
    }
}

void Context::deleteFramebuffers(GLsizei n, const FramebufferID *framebuffers)
{
    for (int i = 0; i < n; i++)
    {
        if (framebuffers[i].value != 0)
        {
            deleteFramebuffer(framebuffers[i]);
        }
    }
}

void Context::deleteRenderbuffers(GLsizei n, const RenderbufferID *renderbuffers)
{
    for (int i = 0; i < n; i++)
    {
        deleteRenderbuffer(renderbuffers[i]);
    }
}

void Context::deleteTextures(GLsizei n, const TextureID *textures)
{
    for (int i = 0; i < n; i++)
    {
        if (textures[i].value != 0)
        {
            deleteTexture(textures[i]);
        }
    }
}

void Context::detachShader(ShaderProgramID program, ShaderProgramID shader)
{
    Program *programObject = getProgramNoResolveLink(program);
    ASSERT(programObject);

    Shader *shaderObject = getShaderNoResolveCompile(shader);
    ASSERT(shaderObject);

    programObject->detachShader(this, shaderObject);
}

void Context::genBuffers(GLsizei n, BufferID *buffers)
{
    for (int i = 0; i < n; i++)
    {
        buffers[i] = createBuffer();
    }
}

void Context::genFramebuffers(GLsizei n, FramebufferID *framebuffers)
{
    for (int i = 0; i < n; i++)
    {
        framebuffers[i] = createFramebuffer();
    }
}

void Context::genRenderbuffers(GLsizei n, RenderbufferID *renderbuffers)
{
    for (int i = 0; i < n; i++)
    {
        renderbuffers[i] = createRenderbuffer();
    }
}

void Context::genTextures(GLsizei n, TextureID *textures)
{
    for (int i = 0; i < n; i++)
    {
        textures[i] = createTexture();
    }
}

void Context::getActiveAttrib(ShaderProgramID program,
                              GLuint index,
                              GLsizei bufsize,
                              GLsizei *length,
                              GLint *size,
                              GLenum *type,
                              GLchar *name)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().getActiveAttribute(index, bufsize, length, size, type, name);
}

void Context::getActiveUniform(ShaderProgramID program,
                               GLuint index,
                               GLsizei bufsize,
                               GLsizei *length,
                               GLint *size,
                               GLenum *type,
                               GLchar *name)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().getActiveUniform(index, bufsize, length, size, type, name);
}

void Context::getAttachedShaders(ShaderProgramID program,
                                 GLsizei maxcount,
                                 GLsizei *count,
                                 ShaderProgramID *shaders)
{
    Program *programObject = getProgramNoResolveLink(program);
    ASSERT(programObject);
    programObject->getAttachedShaders(maxcount, count, shaders);
}

GLint Context::getAttribLocation(ShaderProgramID program, const GLchar *name)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    return programObject->getExecutable().getAttributeLocation(name);
}

void Context::getBooleanv(GLenum pname, GLboolean *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;
    getQueryParameterInfo(pname, &nativeType, &numParams);

    if (nativeType == GL_BOOL)
    {
        getBooleanvImpl(pname, params);
    }
    else
    {
        CastStateValues(this, nativeType, pname, numParams, params);
    }
}

void Context::getBooleanvRobust(GLenum pname, GLsizei bufSize, GLsizei *length, GLboolean *params)
{
    getBooleanv(pname, params);
}

void Context::getFloatv(GLenum pname, GLfloat *params)
{
    GLenum nativeType;
    unsigned int numParams = 0;
    getQueryParameterInfo(pname, &nativeType, &numParams);

    if (nativeType == GL_FLOAT)
    {
        getFloatvImpl(pname, params);
    }
    else
    {
        CastStateValues(this, nativeType, pname, numParams, params);
    }
}

void Context::getFloatvRobust(GLenum pname, GLsizei bufSize, GLsizei *length, GLfloat *params)
{
    getFloatv(pname, params);
}

void Context::getIntegerv(GLenum pname, GLint *params)
{
    GLenum nativeType      = GL_NONE;
    unsigned int numParams = 0;
    getQueryParameterInfo(pname, &nativeType, &numParams);

    if (nativeType == GL_INT)
    {
        getIntegervImpl(pname, params);
    }
    else
    {
        CastStateValues(this, nativeType, pname, numParams, params);
    }
}

void Context::getIntegervRobust(GLenum pname, GLsizei bufSize, GLsizei *length, GLint *data)
{
    getIntegerv(pname, data);
}

void Context::getProgramiv(ShaderProgramID program, GLenum pname, GLint *params)
{
    // Don't resolve link if checking the link completion status.
    Program *programObject = getProgramNoResolveLink(program);
    if (!isContextLost() && pname != GL_COMPLETION_STATUS_KHR)
    {
        programObject = getProgramResolveLink(program);
    }
    ASSERT(programObject);
    QueryProgramiv(this, programObject, pname, params);
}

void Context::getProgramivRobust(ShaderProgramID program,
                                 GLenum pname,
                                 GLsizei bufSize,
                                 GLsizei *length,
                                 GLint *params)
{
    getProgramiv(program, pname, params);
}

void Context::getProgramPipelineiv(ProgramPipelineID pipeline, GLenum pname, GLint *params)
{
    ProgramPipeline *programPipeline = nullptr;
    if (!isContextLost())
    {
        programPipeline = getProgramPipeline(pipeline);
    }
    QueryProgramPipelineiv(this, programPipeline, pname, params);
}

MemoryObject *Context::getMemoryObject(MemoryObjectID handle) const
{
    return mState.mMemoryObjectManager->getMemoryObject(handle);
}

Semaphore *Context::getSemaphore(SemaphoreID handle) const
{
    return mState.mSemaphoreManager->getSemaphore(handle);
}

void Context::getProgramInfoLog(ShaderProgramID program,
                                GLsizei bufsize,
                                GLsizei *length,
                                GLchar *infolog)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getInfoLog(bufsize, length, infolog);
}

void Context::getProgramPipelineInfoLog(ProgramPipelineID pipeline,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLchar *infoLog)
{
    ProgramPipeline *programPipeline = getProgramPipeline(pipeline);
    if (programPipeline)
    {
        programPipeline->getInfoLog(bufSize, length, infoLog);
    }
    else
    {
        *length  = 0;
        *infoLog = '\0';
    }
}

void Context::getShaderiv(ShaderProgramID shader, GLenum pname, GLint *params)
{
    Shader *shaderObject = nullptr;
    if (!isContextLost())
    {
        shaderObject = getShaderNoResolveCompile(shader);
        ASSERT(shaderObject);
    }
    QueryShaderiv(this, shaderObject, pname, params);
}

void Context::getShaderivRobust(ShaderProgramID shader,
                                GLenum pname,
                                GLsizei bufSize,
                                GLsizei *length,
                                GLint *params)
{
    getShaderiv(shader, pname, params);
}

void Context::getShaderInfoLog(ShaderProgramID shader,
                               GLsizei bufsize,
                               GLsizei *length,
                               GLchar *infolog)
{
    Shader *shaderObject = getShaderNoResolveCompile(shader);
    ASSERT(shaderObject);
    shaderObject->getInfoLog(this, bufsize, length, infolog);
}

void Context::getShaderPrecisionFormat(GLenum shadertype,
                                       GLenum precisiontype,
                                       GLint *range,
                                       GLint *precision)
{
    switch (shadertype)
    {
        case GL_VERTEX_SHADER:
            switch (precisiontype)
            {
                case GL_LOW_FLOAT:
                    mState.getCaps().vertexLowpFloat.get(range, precision);
                    break;
                case GL_MEDIUM_FLOAT:
                    mState.getCaps().vertexMediumpFloat.get(range, precision);
                    break;
                case GL_HIGH_FLOAT:
                    mState.getCaps().vertexHighpFloat.get(range, precision);
                    break;

                case GL_LOW_INT:
                    mState.getCaps().vertexLowpInt.get(range, precision);
                    break;
                case GL_MEDIUM_INT:
                    mState.getCaps().vertexMediumpInt.get(range, precision);
                    break;
                case GL_HIGH_INT:
                    mState.getCaps().vertexHighpInt.get(range, precision);
                    break;

                default:
                    UNREACHABLE();
                    return;
            }
            break;

        case GL_FRAGMENT_SHADER:
            switch (precisiontype)
            {
                case GL_LOW_FLOAT:
                    mState.getCaps().fragmentLowpFloat.get(range, precision);
                    break;
                case GL_MEDIUM_FLOAT:
                    mState.getCaps().fragmentMediumpFloat.get(range, precision);
                    break;
                case GL_HIGH_FLOAT:
                    mState.getCaps().fragmentHighpFloat.get(range, precision);
                    break;

                case GL_LOW_INT:
                    mState.getCaps().fragmentLowpInt.get(range, precision);
                    break;
                case GL_MEDIUM_INT:
                    mState.getCaps().fragmentMediumpInt.get(range, precision);
                    break;
                case GL_HIGH_INT:
                    mState.getCaps().fragmentHighpInt.get(range, precision);
                    break;

                default:
                    UNREACHABLE();
                    return;
            }
            break;

        default:
            UNREACHABLE();
            return;
    }
}

void Context::getShaderSource(ShaderProgramID shader,
                              GLsizei bufsize,
                              GLsizei *length,
                              GLchar *source)
{
    Shader *shaderObject = getShaderNoResolveCompile(shader);
    ASSERT(shaderObject);
    shaderObject->getSource(bufsize, length, source);
}

void Context::getUniformfv(ShaderProgramID program, UniformLocation location, GLfloat *params)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().getUniformfv(this, location, params);
}

void Context::getUniformfvRobust(ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei bufSize,
                                 GLsizei *length,
                                 GLfloat *params)
{
    getUniformfv(program, location, params);
}

void Context::getUniformiv(ShaderProgramID program, UniformLocation location, GLint *params)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().getUniformiv(this, location, params);
}

void Context::getUniformivRobust(ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei bufSize,
                                 GLsizei *length,
                                 GLint *params)
{
    getUniformiv(program, location, params);
}

GLint Context::getUniformLocation(ShaderProgramID program, const GLchar *name)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    return programObject->getExecutable().getUniformLocation(name).value;
}

GLboolean Context::isBuffer(BufferID buffer) const
{
    if (buffer.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getBuffer(buffer));
}

GLboolean Context::isFramebuffer(FramebufferID framebuffer) const
{
    if (framebuffer.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getFramebuffer(framebuffer));
}

GLboolean Context::isProgram(ShaderProgramID program) const
{
    if (program.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getProgramNoResolveLink(program));
}

GLboolean Context::isRenderbuffer(RenderbufferID renderbuffer) const
{
    if (renderbuffer.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getRenderbuffer(renderbuffer));
}

GLboolean Context::isShader(ShaderProgramID shader) const
{
    if (shader.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getShaderNoResolveCompile(shader));
}

GLboolean Context::isTexture(TextureID texture) const
{
    if (texture.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getTexture(texture));
}

void Context::linkProgram(ShaderProgramID program)
{
    Program *programObject = getProgramNoResolveLink(program);
    ASSERT(programObject);
    ANGLE_CONTEXT_TRY(programObject->link(this, angle::JobResultExpectancy::Future));
}

void Context::releaseShaderCompiler()
{
    mCompiler.set(this, nullptr);
}

void Context::shaderBinary(GLsizei n,
                           const ShaderProgramID *shaders,
                           GLenum binaryformat,
                           const void *binary,
                           GLsizei length)
{
    Shader *shaderObject = getShaderNoResolveCompile(*shaders);
    ASSERT(shaderObject != nullptr);
    ANGLE_CONTEXT_TRY(
        shaderObject->loadShaderBinary(this, binary, length, angle::JobResultExpectancy::Future));
}

void Context::bindFragDataLocationIndexed(ShaderProgramID program,
                                          GLuint colorNumber,
                                          GLuint index,
                                          const char *name)
{
    Program *programObject = getProgramNoResolveLink(program);
    programObject->bindFragmentOutputLocation(this, colorNumber, name);
    programObject->bindFragmentOutputIndex(this, index, name);
}

void Context::bindFragDataLocation(ShaderProgramID program, GLuint colorNumber, const char *name)
{
    bindFragDataLocationIndexed(program, colorNumber, 0u, name);
}

int Context::getFragDataIndex(ShaderProgramID program, const char *name)
{
    Program *programObject = getProgramResolveLink(program);
    return programObject->getExecutable().getFragDataIndex(name);
}

int Context::getProgramResourceLocationIndex(ShaderProgramID program,
                                             GLenum programInterface,
                                             const char *name)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programInterface == GL_PROGRAM_OUTPUT);
    return programObject->getExecutable().getFragDataIndex(name);
}

void Context::shaderSource(ShaderProgramID shader,
                           GLsizei count,
                           const GLchar *const *string,
                           const GLint *length)
{
    Shader *shaderObject = getShaderNoResolveCompile(shader);
    ASSERT(shaderObject);
    shaderObject->setSource(this, count, string, length);
}

Program *Context::getActiveLinkedProgramPPO() const
{
    ProgramPipeline *programPipelineObject = mState.getProgramPipeline();
    if (programPipelineObject)
    {
        return programPipelineObject->getLinkedActiveShaderProgram(this);
    }

    return nullptr;
}

void Context::uniform1f(UniformLocation location, GLfloat x)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform1fv(location, 1, &x);
}

void Context::uniform1fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform1fv(location, count, v);
}

void Context::setUniform1iImpl(Program *program,
                               UniformLocation location,
                               GLsizei count,
                               const GLint *v)
{
    program->getExecutable().setUniform1iv(this, location, count, v);
}

void Context::onSamplerUniformChange(size_t textureUnitIndex)
{
    mState.onActiveTextureChange(this, textureUnitIndex);
    mStateCache.onActiveTextureChange(this);
}

void Context::uniform1i(UniformLocation location, GLint x)
{
    Program *program = getActiveLinkedProgram();
    setUniform1iImpl(program, location, 1, &x);
}

void Context::uniform1iv(UniformLocation location, GLsizei count, const GLint *v)
{
    Program *program = getActiveLinkedProgram();
    setUniform1iImpl(program, location, count, v);
}

void Context::uniform2f(UniformLocation location, GLfloat x, GLfloat y)
{
    GLfloat xy[2]    = {x, y};
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform2fv(location, 1, xy);
}

void Context::uniform2fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform2fv(location, count, v);
}

void Context::uniform2i(UniformLocation location, GLint x, GLint y)
{
    GLint xy[2]      = {x, y};
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform2iv(location, 1, xy);
}

void Context::uniform2iv(UniformLocation location, GLsizei count, const GLint *v)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform2iv(location, count, v);
}

void Context::uniform3f(UniformLocation location, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat xyz[3]   = {x, y, z};
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform3fv(location, 1, xyz);
}

void Context::uniform3fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform3fv(location, count, v);
}

void Context::uniform3i(UniformLocation location, GLint x, GLint y, GLint z)
{
    GLint xyz[3]     = {x, y, z};
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform3iv(location, 1, xyz);
}

void Context::uniform3iv(UniformLocation location, GLsizei count, const GLint *v)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform3iv(location, count, v);
}

void Context::uniform4f(UniformLocation location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat xyzw[4]  = {x, y, z, w};
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform4fv(location, 1, xyzw);
}

void Context::uniform4fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform4fv(location, count, v);
}

void Context::uniform4i(UniformLocation location, GLint x, GLint y, GLint z, GLint w)
{
    GLint xyzw[4]    = {x, y, z, w};
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform4iv(location, 1, xyzw);
}

void Context::uniform4iv(UniformLocation location, GLsizei count, const GLint *v)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform4iv(location, count, v);
}

void Context::uniformMatrix2fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix2fv(location, count, transpose, value);
}

void Context::uniformMatrix3fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix3fv(location, count, transpose, value);
}

void Context::uniformMatrix4fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix4fv(location, count, transpose, value);
}

void Context::validateProgram(ShaderProgramID program)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->validate(mState.getCaps());
}

void Context::validateProgramPipeline(ProgramPipelineID pipeline)
{
    // GLES spec 3.2, Section 7.4 "Program Pipeline Objects"
    // If pipeline is a name that has been generated (without subsequent deletion) by
    // GenProgramPipelines, but refers to a program pipeline object that has not been
    // previously bound, the GL first creates a new state vector in the same manner as
    // when BindProgramPipeline creates a new program pipeline object.
    //
    // void BindProgramPipeline( uint pipeline );
    // pipeline is the program pipeline object name. The resulting program pipeline
    // object is a new state vector, comprising all the state and with the same initial values
    // listed in table 21.20.
    //
    // If we do not have a pipeline object that's been created with glBindProgramPipeline, we leave
    // VALIDATE_STATUS at it's default false value without generating a pipeline object.
    if (!getProgramPipeline(pipeline))
    {
        return;
    }

    ProgramPipeline *programPipeline =
        mState.mProgramPipelineManager->checkProgramPipelineAllocation(mImplementation.get(),
                                                                       pipeline);
    ASSERT(programPipeline);

    programPipeline->validate(this);
}

void Context::getProgramBinary(ShaderProgramID program,
                               GLsizei bufSize,
                               GLsizei *length,
                               GLenum *binaryFormat,
                               void *binary)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject != nullptr);

    ANGLE_CONTEXT_TRY(programObject->getBinary(this, binaryFormat, binary, bufSize, length));
}

void Context::programBinary(ShaderProgramID program,
                            GLenum binaryFormat,
                            const void *binary,
                            GLsizei length)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject != nullptr);

    ANGLE_CONTEXT_TRY(programObject->setBinary(this, binaryFormat, binary, length));
}

void Context::uniform1ui(UniformLocation location, GLuint v0)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform1uiv(location, 1, &v0);
}

void Context::uniform2ui(UniformLocation location, GLuint v0, GLuint v1)
{
    Program *program  = getActiveLinkedProgram();
    const GLuint xy[] = {v0, v1};
    program->getExecutable().setUniform2uiv(location, 1, xy);
}

void Context::uniform3ui(UniformLocation location, GLuint v0, GLuint v1, GLuint v2)
{
    Program *program   = getActiveLinkedProgram();
    const GLuint xyz[] = {v0, v1, v2};
    program->getExecutable().setUniform3uiv(location, 1, xyz);
}

void Context::uniform4ui(UniformLocation location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    Program *program    = getActiveLinkedProgram();
    const GLuint xyzw[] = {v0, v1, v2, v3};
    program->getExecutable().setUniform4uiv(location, 1, xyzw);
}

void Context::uniform1uiv(UniformLocation location, GLsizei count, const GLuint *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform1uiv(location, count, value);
}
void Context::uniform2uiv(UniformLocation location, GLsizei count, const GLuint *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform2uiv(location, count, value);
}

void Context::uniform3uiv(UniformLocation location, GLsizei count, const GLuint *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform3uiv(location, count, value);
}

void Context::uniform4uiv(UniformLocation location, GLsizei count, const GLuint *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniform4uiv(location, count, value);
}

void Context::genQueries(GLsizei n, QueryID *ids)
{
    for (GLsizei i = 0; i < n; i++)
    {
        QueryID handle = QueryID{mQueryHandleAllocator.allocate()};
        mQueryMap.assign(handle, nullptr);
        ids[i] = handle;
    }
}

void Context::deleteQueries(GLsizei n, const QueryID *ids)
{
    for (int i = 0; i < n; i++)
    {
        QueryID query = ids[i];

        Query *queryObject = nullptr;
        if (mQueryMap.erase(query, &queryObject))
        {
            mQueryHandleAllocator.release(query.value);
            if (queryObject)
            {
                queryObject->release(this);
            }
        }
    }
}

bool Context::isQueryGenerated(QueryID query) const
{
    return mQueryMap.contains(query);
}

GLboolean Context::isQuery(QueryID id) const
{
    return ConvertToGLBoolean(getQuery(id) != nullptr);
}

void Context::uniformMatrix2x3fv(UniformLocation location,
                                 GLsizei count,
                                 GLboolean transpose,
                                 const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix2x3fv(location, count, transpose, value);
}

void Context::uniformMatrix3x2fv(UniformLocation location,
                                 GLsizei count,
                                 GLboolean transpose,
                                 const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix3x2fv(location, count, transpose, value);
}

void Context::uniformMatrix2x4fv(UniformLocation location,
                                 GLsizei count,
                                 GLboolean transpose,
                                 const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix2x4fv(location, count, transpose, value);
}

void Context::uniformMatrix4x2fv(UniformLocation location,
                                 GLsizei count,
                                 GLboolean transpose,
                                 const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix4x2fv(location, count, transpose, value);
}

void Context::uniformMatrix3x4fv(UniformLocation location,
                                 GLsizei count,
                                 GLboolean transpose,
                                 const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix3x4fv(location, count, transpose, value);
}

void Context::uniformMatrix4x3fv(UniformLocation location,
                                 GLsizei count,
                                 GLboolean transpose,
                                 const GLfloat *value)
{
    Program *program = getActiveLinkedProgram();
    program->getExecutable().setUniformMatrix4x3fv(location, count, transpose, value);
}

void Context::deleteVertexArrays(GLsizei n, const VertexArrayID *arrays)
{
    for (int arrayIndex = 0; arrayIndex < n; arrayIndex++)
    {
        VertexArrayID vertexArray = arrays[arrayIndex];

        if (arrays[arrayIndex].value != 0)
        {
            VertexArray *vertexArrayObject = nullptr;
            if (mVertexArrayMap.erase(vertexArray, &vertexArrayObject))
            {
                if (vertexArrayObject != nullptr)
                {
                    detachVertexArray(vertexArray);
                    vertexArrayObject->onDestroy(this);
                }

                mVertexArrayHandleAllocator.release(vertexArray.value);
            }
        }
    }
}

void Context::genVertexArrays(GLsizei n, VertexArrayID *arrays)
{
    for (int arrayIndex = 0; arrayIndex < n; arrayIndex++)
    {
        VertexArrayID vertexArray = {mVertexArrayHandleAllocator.allocate()};
        mVertexArrayMap.assign(vertexArray, nullptr);
        arrays[arrayIndex] = vertexArray;
    }
}

GLboolean Context::isVertexArray(VertexArrayID array) const
{
    if (array.value == 0)
    {
        return GL_FALSE;
    }

    VertexArray *vao = getVertexArray(array);
    return ConvertToGLBoolean(vao != nullptr);
}

void Context::endTransformFeedback()
{
    TransformFeedback *transformFeedback = mState.getCurrentTransformFeedback();
    ANGLE_CONTEXT_TRY(transformFeedback->end(this));
    mStateCache.onActiveTransformFeedbackChange(this);
}

void Context::transformFeedbackVaryings(ShaderProgramID program,
                                        GLsizei count,
                                        const GLchar *const *varyings,
                                        GLenum bufferMode)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->setTransformFeedbackVaryings(this, count, varyings, bufferMode);
}

void Context::getTransformFeedbackVarying(ShaderProgramID program,
                                          GLuint index,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLsizei *size,
                                          GLenum *type,
                                          GLchar *name)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().getTransformFeedbackVarying(index, bufSize, length, size, type,
                                                               name);
}

void Context::deleteTransformFeedbacks(GLsizei n, const TransformFeedbackID *ids)
{
    for (int i = 0; i < n; i++)
    {
        TransformFeedbackID transformFeedback = ids[i];
        if (transformFeedback.value == 0)
        {
            continue;
        }

        TransformFeedback *transformFeedbackObject = nullptr;
        if (mTransformFeedbackMap.erase(transformFeedback, &transformFeedbackObject))
        {
            if (transformFeedbackObject != nullptr)
            {
                detachTransformFeedback(transformFeedback);
                transformFeedbackObject->release(this);
            }

            mTransformFeedbackHandleAllocator.release(transformFeedback.value);
        }
    }
}

void Context::genTransformFeedbacks(GLsizei n, TransformFeedbackID *ids)
{
    for (int i = 0; i < n; i++)
    {
        TransformFeedbackID transformFeedback = {mTransformFeedbackHandleAllocator.allocate()};
        mTransformFeedbackMap.assign(transformFeedback, nullptr);
        ids[i] = transformFeedback;
    }
}

GLboolean Context::isTransformFeedback(TransformFeedbackID id) const
{
    if (id.value == 0)
    {
        // The 3.0.4 spec [section 6.1.11] states that if ID is zero, IsTransformFeedback
        // returns FALSE
        return GL_FALSE;
    }

    const TransformFeedback *transformFeedback = getTransformFeedback(id);
    return ConvertToGLBoolean(transformFeedback != nullptr);
}

void Context::pauseTransformFeedback()
{
    TransformFeedback *transformFeedback = mState.getCurrentTransformFeedback();
    ANGLE_CONTEXT_TRY(transformFeedback->pause(this));
    mStateCache.onActiveTransformFeedbackChange(this);
}

void Context::resumeTransformFeedback()
{
    TransformFeedback *transformFeedback = mState.getCurrentTransformFeedback();
    ANGLE_CONTEXT_TRY(transformFeedback->resume(this));
    mStateCache.onActiveTransformFeedbackChange(this);
}

void Context::getUniformuiv(ShaderProgramID program, UniformLocation location, GLuint *params)
{
    const Program *programObject = getProgramResolveLink(program);
    programObject->getExecutable().getUniformuiv(this, location, params);
}

void Context::getUniformuivRobust(ShaderProgramID program,
                                  UniformLocation location,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLuint *params)
{
    getUniformuiv(program, location, params);
}

GLint Context::getFragDataLocation(ShaderProgramID program, const GLchar *name)
{
    const Program *programObject = getProgramResolveLink(program);
    return programObject->getExecutable().getFragDataLocation(name);
}

void Context::getUniformIndices(ShaderProgramID program,
                                GLsizei uniformCount,
                                const GLchar *const *uniformNames,
                                GLuint *uniformIndices)
{
    const Program *programObject = getProgramResolveLink(program);
    if (!programObject->isLinked())
    {
        for (int uniformId = 0; uniformId < uniformCount; uniformId++)
        {
            uniformIndices[uniformId] = GL_INVALID_INDEX;
        }
    }
    else
    {
        for (int uniformId = 0; uniformId < uniformCount; uniformId++)
        {
            uniformIndices[uniformId] =
                programObject->getExecutable().getUniformIndex(uniformNames[uniformId]);
        }
    }
}

void Context::getActiveUniformsiv(ShaderProgramID program,
                                  GLsizei uniformCount,
                                  const GLuint *uniformIndices,
                                  GLenum pname,
                                  GLint *params)
{
    const Program *programObject = getProgramResolveLink(program);
    for (int uniformId = 0; uniformId < uniformCount; uniformId++)
    {
        const GLuint index = uniformIndices[uniformId];
        params[uniformId]  = GetUniformResourceProperty(programObject, index, pname);
    }
}

GLuint Context::getUniformBlockIndex(ShaderProgramID program, const GLchar *uniformBlockName)
{
    const Program *programObject = getProgramResolveLink(program);
    return programObject->getExecutable().getUniformBlockIndex(uniformBlockName);
}

void Context::getActiveUniformBlockiv(ShaderProgramID program,
                                      UniformBlockIndex uniformBlockIndex,
                                      GLenum pname,
                                      GLint *params)
{
    const Program *programObject = getProgramResolveLink(program);
    QueryActiveUniformBlockiv(programObject, uniformBlockIndex, pname, params);
}

void Context::getActiveUniformBlockivRobust(ShaderProgramID program,
                                            UniformBlockIndex uniformBlockIndex,
                                            GLenum pname,
                                            GLsizei bufSize,
                                            GLsizei *length,
                                            GLint *params)
{
    getActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

void Context::getActiveUniformBlockName(ShaderProgramID program,
                                        UniformBlockIndex uniformBlockIndex,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLchar *uniformBlockName)
{
    const Program *programObject = getProgramResolveLink(program);
    programObject->getExecutable().getActiveUniformBlockName(this, uniformBlockIndex, bufSize,
                                                             length, uniformBlockName);
}

void Context::uniformBlockBinding(ShaderProgramID program,
                                  UniformBlockIndex uniformBlockIndex,
                                  GLuint uniformBlockBinding)
{
    Program *programObject = getProgramResolveLink(program);
    programObject->bindUniformBlock(uniformBlockIndex, uniformBlockBinding);
}

GLsync Context::fenceSync(GLenum condition, GLbitfield flags)
{
    SyncID syncHandle = mState.mSyncManager->createSync(mImplementation.get());
    Sync *syncObject  = getSync(syncHandle);
    if (syncObject->set(this, condition, flags) == angle::Result::Stop)
    {
        deleteSync(syncHandle);
        return nullptr;
    }

    return unsafe_int_to_pointer_cast<GLsync>(syncHandle.value);
}

GLboolean Context::isSync(SyncID syncPacked) const
{
    return (getSync(syncPacked) != nullptr);
}

GLenum Context::clientWaitSync(SyncID syncPacked, GLbitfield flags, GLuint64 timeout)
{
    Sync *syncObject = getSync(syncPacked);

    GLenum result = GL_WAIT_FAILED;
    if (syncObject->clientWait(this, flags, timeout, &result) == angle::Result::Stop)
    {
        return GL_WAIT_FAILED;
    }
    return result;
}

void Context::waitSync(SyncID syncPacked, GLbitfield flags, GLuint64 timeout)
{
    Sync *syncObject = getSync(syncPacked);
    ANGLE_CONTEXT_TRY(syncObject->serverWait(this, flags, timeout));
}

void Context::getInteger64v(GLenum pname, GLint64 *params)
{
    GLenum nativeType      = GL_NONE;
    unsigned int numParams = 0;
    getQueryParameterInfo(pname, &nativeType, &numParams);

    if (nativeType == GL_INT_64_ANGLEX)
    {
        getInteger64vImpl(pname, params);
    }
    else
    {
        CastStateValues(this, nativeType, pname, numParams, params);
    }
}

void Context::getInteger64vRobust(GLenum pname, GLsizei bufSize, GLsizei *length, GLint64 *data)
{
    getInteger64v(pname, data);
}

void Context::getBufferParameteri64v(BufferBinding target, GLenum pname, GLint64 *params)
{
    Buffer *buffer = mState.getTargetBuffer(target);
    QueryBufferParameteri64v(buffer, pname, params);
}

void Context::getBufferParameteri64vRobust(BufferBinding target,
                                           GLenum pname,
                                           GLsizei bufSize,
                                           GLsizei *length,
                                           GLint64 *params)
{
    getBufferParameteri64v(target, pname, params);
}

void Context::genSamplers(GLsizei count, SamplerID *samplers)
{
    for (int i = 0; i < count; i++)
    {
        samplers[i] = mState.mSamplerManager->createSampler();
    }
}

void Context::deleteSamplers(GLsizei count, const SamplerID *samplers)
{
    for (int i = 0; i < count; i++)
    {
        SamplerID sampler = samplers[i];

        if (mState.mSamplerManager->getSampler(sampler))
        {
            detachSampler(sampler);
        }

        mState.mSamplerManager->deleteObject(this, sampler);
    }
}

void Context::getInternalformativ(GLenum target,
                                  GLenum internalformat,
                                  GLenum pname,
                                  GLsizei bufSize,
                                  GLint *params)
{
    Texture *texture    = nullptr;
    TextureType textype = FromGLenum<TextureType>(target);
    if (textype != TextureType::InvalidEnum)
    {
        texture = getTextureByType(textype);
    }
    const TextureCaps &formatCaps = mState.getTextureCap(internalformat);
    QueryInternalFormativ(this, texture, internalformat, formatCaps, pname, bufSize, params);
}

void Context::getInternalformativRobust(GLenum target,
                                        GLenum internalformat,
                                        GLenum pname,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLint *params)
{
    getInternalformativ(target, internalformat, pname, bufSize, params);
}

void Context::programUniform1i(ShaderProgramID program, UniformLocation location, GLint v0)
{
    programUniform1iv(program, location, 1, &v0);
}

void Context::programUniform2i(ShaderProgramID program,
                               UniformLocation location,
                               GLint v0,
                               GLint v1)
{
    GLint xy[2] = {v0, v1};
    programUniform2iv(program, location, 1, xy);
}

void Context::programUniform3i(ShaderProgramID program,
                               UniformLocation location,
                               GLint v0,
                               GLint v1,
                               GLint v2)
{
    GLint xyz[3] = {v0, v1, v2};
    programUniform3iv(program, location, 1, xyz);
}

void Context::programUniform4i(ShaderProgramID program,
                               UniformLocation location,
                               GLint v0,
                               GLint v1,
                               GLint v2,
                               GLint v3)
{
    GLint xyzw[4] = {v0, v1, v2, v3};
    programUniform4iv(program, location, 1, xyzw);
}

void Context::programUniform1ui(ShaderProgramID program, UniformLocation location, GLuint v0)
{
    programUniform1uiv(program, location, 1, &v0);
}

void Context::programUniform2ui(ShaderProgramID program,
                                UniformLocation location,
                                GLuint v0,
                                GLuint v1)
{
    GLuint xy[2] = {v0, v1};
    programUniform2uiv(program, location, 1, xy);
}

void Context::programUniform3ui(ShaderProgramID program,
                                UniformLocation location,
                                GLuint v0,
                                GLuint v1,
                                GLuint v2)
{
    GLuint xyz[3] = {v0, v1, v2};
    programUniform3uiv(program, location, 1, xyz);
}

void Context::programUniform4ui(ShaderProgramID program,
                                UniformLocation location,
                                GLuint v0,
                                GLuint v1,
                                GLuint v2,
                                GLuint v3)
{
    GLuint xyzw[4] = {v0, v1, v2, v3};
    programUniform4uiv(program, location, 1, xyzw);
}

void Context::programUniform1f(ShaderProgramID program, UniformLocation location, GLfloat v0)
{
    programUniform1fv(program, location, 1, &v0);
}

void Context::programUniform2f(ShaderProgramID program,
                               UniformLocation location,
                               GLfloat v0,
                               GLfloat v1)
{
    GLfloat xy[2] = {v0, v1};
    programUniform2fv(program, location, 1, xy);
}

void Context::programUniform3f(ShaderProgramID program,
                               UniformLocation location,
                               GLfloat v0,
                               GLfloat v1,
                               GLfloat v2)
{
    GLfloat xyz[3] = {v0, v1, v2};
    programUniform3fv(program, location, 1, xyz);
}

void Context::programUniform4f(ShaderProgramID program,
                               UniformLocation location,
                               GLfloat v0,
                               GLfloat v1,
                               GLfloat v2,
                               GLfloat v3)
{
    GLfloat xyzw[4] = {v0, v1, v2, v3};
    programUniform4fv(program, location, 1, xyzw);
}

void Context::programUniform1iv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    setUniform1iImpl(programObject, location, count, value);
}

void Context::programUniform2iv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform2iv(location, count, value);
}

void Context::programUniform3iv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform3iv(location, count, value);
}

void Context::programUniform4iv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform4iv(location, count, value);
}

void Context::programUniform1uiv(ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei count,
                                 const GLuint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform1uiv(location, count, value);
}

void Context::programUniform2uiv(ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei count,
                                 const GLuint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform2uiv(location, count, value);
}

void Context::programUniform3uiv(ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei count,
                                 const GLuint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform3uiv(location, count, value);
}

void Context::programUniform4uiv(ShaderProgramID program,
                                 UniformLocation location,
                                 GLsizei count,
                                 const GLuint *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform4uiv(location, count, value);
}

void Context::programUniform1fv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform1fv(location, count, value);
}

void Context::programUniform2fv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform2fv(location, count, value);
}

void Context::programUniform3fv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform3fv(location, count, value);
}

void Context::programUniform4fv(ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count,
                                const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniform4fv(location, count, value);
}

void Context::programUniformMatrix2fv(ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix2fv(location, count, transpose, value);
}

void Context::programUniformMatrix3fv(ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix3fv(location, count, transpose, value);
}

void Context::programUniformMatrix4fv(ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix4fv(location, count, transpose, value);
}

void Context::programUniformMatrix2x3fv(ShaderProgramID program,
                                        UniformLocation location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix2x3fv(location, count, transpose, value);
}

void Context::programUniformMatrix3x2fv(ShaderProgramID program,
                                        UniformLocation location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix3x2fv(location, count, transpose, value);
}

void Context::programUniformMatrix2x4fv(ShaderProgramID program,
                                        UniformLocation location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix2x4fv(location, count, transpose, value);
}

void Context::programUniformMatrix4x2fv(ShaderProgramID program,
                                        UniformLocation location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix4x2fv(location, count, transpose, value);
}

void Context::programUniformMatrix3x4fv(ShaderProgramID program,
                                        UniformLocation location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix3x4fv(location, count, transpose, value);
}

void Context::programUniformMatrix4x3fv(ShaderProgramID program,
                                        UniformLocation location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);
    programObject->getExecutable().setUniformMatrix4x3fv(location, count, transpose, value);
}

bool Context::isCurrentTransformFeedback(const TransformFeedback *tf) const
{
    return mState.isCurrentTransformFeedback(tf);
}

void Context::genProgramPipelines(GLsizei count, ProgramPipelineID *pipelines)
{
    for (int i = 0; i < count; i++)
    {
        pipelines[i] = createProgramPipeline();
    }
}

void Context::deleteProgramPipelines(GLsizei count, const ProgramPipelineID *pipelines)
{
    for (int i = 0; i < count; i++)
    {
        if (pipelines[i].value != 0)
        {
            deleteProgramPipeline(pipelines[i]);
        }
    }
}

GLboolean Context::isProgramPipeline(ProgramPipelineID pipeline) const
{
    if (pipeline.value == 0)
    {
        return GL_FALSE;
    }

    if (getProgramPipeline(pipeline))
    {
        return GL_TRUE;
    }

    return GL_FALSE;
}

void Context::finishFenceNV(FenceNVID fence)
{
    FenceNV *fenceObject = getFenceNV(fence);

    ASSERT(fenceObject && fenceObject->isSet());
    ANGLE_CONTEXT_TRY(fenceObject->finish(this));
}

void Context::getFenceivNV(FenceNVID fence, GLenum pname, GLint *params)
{
    FenceNV *fenceObject = getFenceNV(fence);

    ASSERT(fenceObject && fenceObject->isSet());

    switch (pname)
    {
        case GL_FENCE_STATUS_NV:
        {
            // GL_NV_fence spec:
            // Once the status of a fence has been finished (via FinishFenceNV) or tested and
            // the returned status is TRUE (via either TestFenceNV or GetFenceivNV querying the
            // FENCE_STATUS_NV), the status remains TRUE until the next SetFenceNV of the fence.
            GLboolean status = GL_TRUE;
            if (fenceObject->getStatus() != GL_TRUE)
            {
                ANGLE_CONTEXT_TRY(fenceObject->test(this, &status));
            }
            *params = status;
            break;
        }

        case GL_FENCE_CONDITION_NV:
        {
            *params = static_cast<GLint>(fenceObject->getCondition());
            break;
        }

        default:
            UNREACHABLE();
    }
}

void Context::getTranslatedShaderSource(ShaderProgramID shader,
                                        GLsizei bufsize,
                                        GLsizei *length,
                                        GLchar *source)
{
    Shader *shaderObject = getShaderNoResolveCompile(shader);
    ASSERT(shaderObject);
    shaderObject->getTranslatedSourceWithDebugInfo(this, bufsize, length, source);
}

void Context::getnUniformfv(ShaderProgramID program,
                            UniformLocation location,
                            GLsizei bufSize,
                            GLfloat *params)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);

    programObject->getExecutable().getUniformfv(this, location, params);
}

void Context::getnUniformfvRobust(ShaderProgramID program,
                                  UniformLocation location,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLfloat *params)
{
    UNIMPLEMENTED();
}

void Context::getnUniformiv(ShaderProgramID program,
                            UniformLocation location,
                            GLsizei bufSize,
                            GLint *params)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);

    programObject->getExecutable().getUniformiv(this, location, params);
}

void Context::getnUniformuiv(ShaderProgramID program,
                             UniformLocation location,
                             GLsizei bufSize,
                             GLuint *params)
{
    Program *programObject = getProgramResolveLink(program);
    ASSERT(programObject);

    programObject->getExecutable().getUniformuiv(this, location, params);
}

void Context::getnUniformivRobust(ShaderProgramID program,
                                  UniformLocation location,
                                  GLsizei bufSize,
                                  GLsizei *length,
                                  GLint *params)
{
    UNIMPLEMENTED();
}

void Context::getnUniformuivRobust(ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei bufSize,
                                   GLsizei *length,
                                   GLuint *params)
{
    UNIMPLEMENTED();
}

GLboolean Context::isFenceNV(FenceNVID fence) const
{
    FenceNV *fenceObject = getFenceNV(fence);

    if (fenceObject == nullptr)
    {
        return GL_FALSE;
    }

    // GL_NV_fence spec:
    // A name returned by GenFencesNV, but not yet set via SetFenceNV, is not the name of an
    // existing fence.
    return fenceObject->isSet();
}

void Context::readnPixels(GLint x,
                          GLint y,
                          GLsizei width,
                          GLsizei height,
                          GLenum format,
                          GLenum type,
                          GLsizei bufSize,
                          void *data)
{
    return readPixels(x, y, width, height, format, type, data);
}

void Context::setFenceNV(FenceNVID fence, GLenum condition)
{
    ASSERT(condition == GL_ALL_COMPLETED_NV);

    FenceNV *fenceObject = getFenceNV(fence);
    ASSERT(fenceObject != nullptr);
    ANGLE_CONTEXT_TRY(fenceObject->set(this, condition));
}

GLboolean Context::testFenceNV(FenceNVID fence)
{
    FenceNV *fenceObject = getFenceNV(fence);

    ASSERT(fenceObject != nullptr);
    ASSERT(fenceObject->isSet() == GL_TRUE);

    GLboolean result = GL_TRUE;
    if (fenceObject->test(this, &result) == angle::Result::Stop)
    {
        return GL_TRUE;
    }

    return result;
}

void Context::deleteMemoryObjects(GLsizei n, const MemoryObjectID *memoryObjects)
{
    for (int i = 0; i < n; i++)
    {
        deleteMemoryObject(memoryObjects[i]);
    }
}

GLboolean Context::isMemoryObject(MemoryObjectID memoryObject) const
{
    if (memoryObject.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getMemoryObject(memoryObject));
}

void Context::createMemoryObjects(GLsizei n, MemoryObjectID *memoryObjects)
{
    for (int i = 0; i < n; i++)
    {
        memoryObjects[i] = createMemoryObject();
    }
}

void Context::memoryObjectParameteriv(MemoryObjectID memory, GLenum pname, const GLint *params)
{
    MemoryObject *memoryObject = getMemoryObject(memory);
    ASSERT(memoryObject);
    ANGLE_CONTEXT_TRY(SetMemoryObjectParameteriv(this, memoryObject, pname, params));
}

void Context::getMemoryObjectParameteriv(MemoryObjectID memory, GLenum pname, GLint *params)
{
    const MemoryObject *memoryObject = getMemoryObject(memory);
    ASSERT(memoryObject);
    QueryMemoryObjectParameteriv(memoryObject, pname, params);
}

void Context::texStorageMem2D(TextureType target,
                              GLsizei levels,
                              GLenum internalFormat,
                              GLsizei width,
                              GLsizei height,
                              MemoryObjectID memory,
                              GLuint64 offset)
{
    texStorageMemFlags2D(target, levels, internalFormat, width, height, memory, offset,
                         std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(),
                         nullptr);
}

void Context::texStorageMem2DMultisample(TextureType target,
                                         GLsizei samples,
                                         GLenum internalFormat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLboolean fixedSampleLocations,
                                         MemoryObjectID memory,
                                         GLuint64 offset)
{
    UNIMPLEMENTED();
}

void Context::texStorageMem3D(TextureType target,
                              GLsizei levels,
                              GLenum internalFormat,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              MemoryObjectID memory,
                              GLuint64 offset)
{
    UNIMPLEMENTED();
}

void Context::texStorageMem3DMultisample(TextureType target,
                                         GLsizei samples,
                                         GLenum internalFormat,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth,
                                         GLboolean fixedSampleLocations,
                                         MemoryObjectID memory,
                                         GLuint64 offset)
{
    UNIMPLEMENTED();
}

void Context::bufferStorageMem(TextureType target,
                               GLsizeiptr size,
                               MemoryObjectID memory,
                               GLuint64 offset)
{
    UNIMPLEMENTED();
}

void Context::importMemoryFd(MemoryObjectID memory, GLuint64 size, HandleType handleType, GLint fd)
{
    MemoryObject *memoryObject = getMemoryObject(memory);
    ASSERT(memoryObject != nullptr);
    ANGLE_CONTEXT_TRY(memoryObject->importFd(this, size, handleType, fd));
}

void Context::texStorageMemFlags2D(TextureType target,
                                   GLsizei levels,
                                   GLenum internalFormat,
                                   GLsizei width,
                                   GLsizei height,
                                   MemoryObjectID memory,
                                   GLuint64 offset,
                                   GLbitfield createFlags,
                                   GLbitfield usageFlags,
                                   const void *imageCreateInfoPNext)
{
    MemoryObject *memoryObject = getMemoryObject(memory);
    ASSERT(memoryObject);
    Extents size(width, height, 1);
    Texture *texture = getTextureByType(target);
    ANGLE_CONTEXT_TRY(texture->setStorageExternalMemory(this, target, levels, internalFormat, size,
                                                        memoryObject, offset, createFlags,
                                                        usageFlags, imageCreateInfoPNext));
}

void Context::texStorageMemFlags2DMultisample(TextureType target,
                                              GLsizei samples,
                                              GLenum internalFormat,
                                              GLsizei width,
                                              GLsizei height,
                                              GLboolean fixedSampleLocations,
                                              MemoryObjectID memory,
                                              GLuint64 offset,
                                              GLbitfield createFlags,
                                              GLbitfield usageFlags,
                                              const void *imageCreateInfoPNext)
{
    UNIMPLEMENTED();
}

void Context::texStorageMemFlags3D(TextureType target,
                                   GLsizei levels,
                                   GLenum internalFormat,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   MemoryObjectID memory,
                                   GLuint64 offset,
                                   GLbitfield createFlags,
                                   GLbitfield usageFlags,
                                   const void *imageCreateInfoPNext)
{
    UNIMPLEMENTED();
}

void Context::texStorageMemFlags3DMultisample(TextureType target,
                                              GLsizei samples,
                                              GLenum internalFormat,
                                              GLsizei width,
                                              GLsizei height,
                                              GLsizei depth,
                                              GLboolean fixedSampleLocations,
                                              MemoryObjectID memory,
                                              GLuint64 offset,
                                              GLbitfield createFlags,
                                              GLbitfield usageFlags,
                                              const void *imageCreateInfoPNext)
{
    UNIMPLEMENTED();
}

void Context::importMemoryZirconHandle(MemoryObjectID memory,
                                       GLuint64 size,
                                       HandleType handleType,
                                       GLuint handle)
{
    MemoryObject *memoryObject = getMemoryObject(memory);
    ASSERT(memoryObject != nullptr);
    ANGLE_CONTEXT_TRY(memoryObject->importZirconHandle(this, size, handleType, handle));
}

void Context::genSemaphores(GLsizei n, SemaphoreID *semaphores)
{
    for (int i = 0; i < n; i++)
    {
        semaphores[i] = createSemaphore();
    }
}

void Context::deleteSemaphores(GLsizei n, const SemaphoreID *semaphores)
{
    for (int i = 0; i < n; i++)
    {
        deleteSemaphore(semaphores[i]);
    }
}

GLboolean Context::isSemaphore(SemaphoreID semaphore) const
{
    if (semaphore.value == 0)
    {
        return GL_FALSE;
    }

    return ConvertToGLBoolean(getSemaphore(semaphore));
}

void Context::semaphoreParameterui64v(SemaphoreID semaphore, GLenum pname, const GLuint64 *params)
{
    UNIMPLEMENTED();
}

void Context::getSemaphoreParameterui64v(SemaphoreID semaphore, GLenum pname, GLuint64 *params)
{
    UNIMPLEMENTED();
}

void Context::acquireTextures(GLuint numTextures,
                              const TextureID *textureIds,
                              const GLenum *layouts)
{
    TextureBarrierVector textureBarriers(numTextures);
    for (size_t i = 0; i < numTextures; i++)
    {
        textureBarriers[i].texture = getTexture(textureIds[i]);
        textureBarriers[i].layout  = layouts[i];
    }
    ANGLE_CONTEXT_TRY(mImplementation->acquireTextures(this, textureBarriers));
}

void Context::releaseTextures(GLuint numTextures, const TextureID *textureIds, GLenum *layouts)
{
    TextureBarrierVector textureBarriers(numTextures);
    for (size_t i = 0; i < numTextures; i++)
    {
        textureBarriers[i].texture = getTexture(textureIds[i]);
    }
    ANGLE_CONTEXT_TRY(mImplementation->releaseTextures(this, &textureBarriers));
    for (size_t i = 0; i < numTextures; i++)
    {
        layouts[i] = textureBarriers[i].layout;
    }
}

void Context::waitSemaphore(SemaphoreID semaphoreHandle,
                            GLuint numBufferBarriers,
                            const BufferID *buffers,
                            GLuint numTextureBarriers,
                            const TextureID *textures,
                            const GLenum *srcLayouts)
{
    Semaphore *semaphore = getSemaphore(semaphoreHandle);
    ASSERT(semaphore);

    BufferBarrierVector bufferBarriers(numBufferBarriers);
    for (GLuint bufferBarrierIdx = 0; bufferBarrierIdx < numBufferBarriers; bufferBarrierIdx++)
    {
        bufferBarriers[bufferBarrierIdx] = getBuffer(buffers[bufferBarrierIdx]);
    }

    TextureBarrierVector textureBarriers(numTextureBarriers);
    for (GLuint textureBarrierIdx = 0; textureBarrierIdx < numTextureBarriers; textureBarrierIdx++)
    {
        textureBarriers[textureBarrierIdx].texture = getTexture(textures[textureBarrierIdx]);
        textureBarriers[textureBarrierIdx].layout  = srcLayouts[textureBarrierIdx];
    }

    ANGLE_CONTEXT_TRY(semaphore->wait(this, bufferBarriers, textureBarriers));
}

void Context::signalSemaphore(SemaphoreID semaphoreHandle,
                              GLuint numBufferBarriers,
                              const BufferID *buffers,
                              GLuint numTextureBarriers,
                              const TextureID *textures,
                              const GLenum *dstLayouts)
{
    Semaphore *semaphore = getSemaphore(semaphoreHandle);
    ASSERT(semaphore);

    BufferBarrierVector bufferBarriers(numBufferBarriers);
    for (GLuint bufferBarrierIdx = 0; bufferBarrierIdx < numBufferBarriers; bufferBarrierIdx++)
    {
        bufferBarriers[bufferBarrierIdx] = getBuffer(buffers[bufferBarrierIdx]);
    }

    TextureBarrierVector textureBarriers(numTextureBarriers);
    for (GLuint textureBarrierIdx = 0; textureBarrierIdx < numTextureBarriers; textureBarrierIdx++)
    {
        textureBarriers[textureBarrierIdx].texture = getTexture(textures[textureBarrierIdx]);
        textureBarriers[textureBarrierIdx].layout  = dstLayouts[textureBarrierIdx];
    }

    ANGLE_CONTEXT_TRY(semaphore->signal(this, bufferBarriers, textureBarriers));
}

void Context::importSemaphoreFd(SemaphoreID semaphore, HandleType handleType, GLint fd)
{
    Semaphore *semaphoreObject = getSemaphore(semaphore);
    ASSERT(semaphoreObject != nullptr);
    ANGLE_CONTEXT_TRY(semaphoreObject->importFd(this, handleType, fd));
}

void Context::importSemaphoreZirconHandle(SemaphoreID semaphore,
                                          HandleType handleType,
                                          GLuint handle)
{
    Semaphore *semaphoreObject = getSemaphore(semaphore);
    ASSERT(semaphoreObject != nullptr);
    ANGLE_CONTEXT_TRY(semaphoreObject->importZirconHandle(this, handleType, handle));
}

void Context::framebufferMemorylessPixelLocalStorage(GLint plane, GLenum internalformat)
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0)
    {
        endPixelLocalStorageImplicit();
    }

    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);

    if (internalformat == GL_NONE)
    {
        pls.deinitialize(this, plane);
    }
    else
    {
        pls.setMemoryless(this, plane, internalformat);
    }
}

void Context::framebufferTexturePixelLocalStorage(GLint plane,
                                                  TextureID backingtexture,
                                                  GLint level,
                                                  GLint layer)
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);

    if (mState.getPixelLocalStorageActivePlanes() != 0)
    {
        endPixelLocalStorageImplicit();
    }

    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);

    if (backingtexture.value == 0)
    {
        pls.deinitialize(this, plane);
    }
    else
    {
        Texture *tex = getTexture(backingtexture);
        ASSERT(tex);  // Validation guarantees this.
        pls.setTextureBacked(this, plane, tex, level, layer);
    }
}

void Context::framebufferPixelLocalClearValuefv(GLint plane, const GLfloat value[])
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);
    pls.setClearValuef(plane, value);
}

void Context::framebufferPixelLocalClearValueiv(GLint plane, const GLint value[])
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);
    pls.setClearValuei(plane, value);
}

void Context::framebufferPixelLocalClearValueuiv(GLint plane, const GLuint value[])
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);
    pls.setClearValueui(plane, value);
}

void Context::beginPixelLocalStorage(GLsizei n, const GLenum loadops[])
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);

    pls.begin(this, n, loadops);
    mState.setPixelLocalStorageActivePlanes(n);
}

void Context::endPixelLocalStorage(GLsizei n, const GLenum storeops[])
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);

    ASSERT(n == mState.getPixelLocalStorageActivePlanes());
    mState.setPixelLocalStorageActivePlanes(0);
    pls.end(this, n, storeops);
}

void Context::endPixelLocalStorageImplicit()
{
    GLsizei n = mState.getPixelLocalStorageActivePlanes();
    ASSERT(n != 0);
    angle::FixedVector<GLenum, IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES> storeops(
        n, GL_STORE_OP_STORE_ANGLE);
    endPixelLocalStorage(n, storeops.data());
}

bool Context::areBlobCacheFuncsSet() const
{
    return mState.getBlobCacheCallbacks().getFunction && mState.getBlobCacheCallbacks().setFunction;
}

void Context::pixelLocalStorageBarrier()
{
    if (getExtensions().shaderPixelLocalStorageCoherentANGLE)
    {
        return;
    }

    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);

    pls.barrier(this);
}

void Context::framebufferPixelLocalStorageInterrupt()
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    if (framebuffer->id().value != 0)
    {
        PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);
        pls.interrupt(this);
    }
}

void Context::framebufferPixelLocalStorageRestore()
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    if (framebuffer->id().value != 0)
    {
        PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);
        pls.restore(this);
    }
}

void Context::getFramebufferPixelLocalStorageParameterfv(GLint plane, GLenum pname, GLfloat *params)
{
    getFramebufferPixelLocalStorageParameterfvRobust(
        plane, pname, std::numeric_limits<GLsizei>::max(), nullptr, params);
}

void Context::getFramebufferPixelLocalStorageParameteriv(GLint plane, GLenum pname, GLint *params)
{
    getFramebufferPixelLocalStorageParameterivRobust(
        plane, pname, std::numeric_limits<GLsizei>::max(), nullptr, params);
}

void Context::getFramebufferPixelLocalStorageParameterfvRobust(GLint plane,
                                                               GLenum pname,
                                                               GLsizei bufSize,
                                                               GLsizei *length,
                                                               GLfloat *params)
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);

    switch (pname)
    {
        case GL_PIXEL_LOCAL_CLEAR_VALUE_FLOAT_ANGLE:
            if (length != nullptr)
            {
                *length = 4;
            }
            pls.getPlane(plane).getClearValuef(params);
            break;
    }
}

void Context::getFramebufferPixelLocalStorageParameterivRobust(GLint plane,
                                                               GLenum pname,
                                                               GLsizei bufSize,
                                                               GLsizei *length,
                                                               GLint *params)
{
    Framebuffer *framebuffer = mState.getDrawFramebuffer();
    ASSERT(framebuffer);
    PixelLocalStorage &pls = framebuffer->getPixelLocalStorage(this);

    switch (pname)
    {
        // GL_ANGLE_shader_pixel_local_storage.
        case GL_PIXEL_LOCAL_FORMAT_ANGLE:
        case GL_PIXEL_LOCAL_TEXTURE_NAME_ANGLE:
        case GL_PIXEL_LOCAL_TEXTURE_LEVEL_ANGLE:
        case GL_PIXEL_LOCAL_TEXTURE_LAYER_ANGLE:
            if (length != nullptr)
            {
                *length = 1;
            }
            *params = pls.getPlane(plane).getIntegeri(pname);
            break;
        case GL_PIXEL_LOCAL_CLEAR_VALUE_INT_ANGLE:
            if (length != nullptr)
            {
                *length = 4;
            }
            pls.getPlane(plane).getClearValuei(params);
            break;
        case GL_PIXEL_LOCAL_CLEAR_VALUE_UNSIGNED_INT_ANGLE:
        {
            if (length != nullptr)
            {
                *length = 4;
            }
            GLuint valueui[4];
            pls.getPlane(plane).getClearValueui(valueui);
            memcpy(params, valueui, sizeof(valueui));
            break;
        }
    }
}

void Context::eGLImageTargetTexStorage(GLenum target, egl::ImageID image, const GLint *attrib_list)
{
    Texture *texture        = getTextureByType(FromGLenum<TextureType>(target));
    egl::Image *imageObject = mDisplay->getImage(image);
    ANGLE_CONTEXT_TRY(texture->setStorageEGLImageTarget(this, FromGLenum<TextureType>(target),
                                                        imageObject, attrib_list));
}

void Context::eGLImageTargetTextureStorage(GLuint texture,
                                           egl::ImageID image,
                                           const GLint *attrib_list)
{}

void Context::eGLImageTargetTexture2D(TextureType target, egl::ImageID image)
{
    Texture *texture        = getTextureByType(target);
    egl::Image *imageObject = mDisplay->getImage(image);
    ANGLE_CONTEXT_TRY(texture->setEGLImageTarget(this, target, imageObject));
}

void Context::eGLImageTargetRenderbufferStorage(GLenum target, egl::ImageID image)
{
    Renderbuffer *renderbuffer = mState.getCurrentRenderbuffer();
    egl::Image *imageObject    = mDisplay->getImage(image);
    ANGLE_CONTEXT_TRY(renderbuffer->setStorageEGLImageTarget(this, imageObject));
}

void Context::framebufferFetchBarrier()
{
    mImplementation->framebufferFetchBarrier();
}

void Context::texStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    UNIMPLEMENTED();
}

bool Context::getQueryParameterInfo(GLenum pname, GLenum *type, unsigned int *numParams) const
{
    return GetQueryParameterInfo(mState, pname, type, numParams);
}

bool Context::getIndexedQueryParameterInfo(GLenum target,
                                           GLenum *type,
                                           unsigned int *numParams) const
{
    return GetIndexedQueryParameterInfo(mState, target, type, numParams);
}

Program *Context::getProgramNoResolveLink(ShaderProgramID handle) const
{
    return mState.mShaderProgramManager->getProgram(handle);
}

Shader *Context::getShaderResolveCompile(ShaderProgramID handle) const
{
    Shader *shader = getShaderNoResolveCompile(handle);
    if (shader)
    {
        shader->resolveCompile(this);
    }
    return shader;
}

Shader *Context::getShaderNoResolveCompile(ShaderProgramID handle) const
{
    return mState.mShaderProgramManager->getShader(handle);
}

const angle::FrontendFeatures &Context::getFrontendFeatures() const
{
    return mDisplay->getFrontendFeatures();
}

bool Context::isRenderbufferGenerated(RenderbufferID renderbuffer) const
{
    return mState.mRenderbufferManager->isHandleGenerated(renderbuffer);
}

bool Context::isFramebufferGenerated(FramebufferID framebuffer) const
{
    return mState.mFramebufferManager->isHandleGenerated(framebuffer);
}

bool Context::isProgramPipelineGenerated(ProgramPipelineID pipeline) const
{
    return mState.mProgramPipelineManager->isHandleGenerated(pipeline);
}

bool Context::usingDisplayTextureShareGroup() const
{
    return mDisplayTextureShareGroup;
}

bool Context::usingDisplaySemaphoreShareGroup() const
{
    return mDisplaySemaphoreShareGroup;
}

GLenum Context::getConvertedRenderbufferFormat(GLenum internalformat) const
{
    if (isWebGL() && mState.getClientMajorVersion() == 2 && internalformat == GL_DEPTH_STENCIL)
    {
        return GL_DEPTH24_STENCIL8;
    }
    return internalformat;
}

void Context::maxShaderCompilerThreads(GLuint count)
{
    // A count of zero specifies a request for no parallel compiling or linking.  This is handled in
    // getShaderCompileThreadPool.  Otherwise the count itself has no effect as the pool is shared
    // between contexts.
    mState.setMaxShaderCompilerThreads(count);
    mImplementation->setMaxShaderCompilerThreads(count);
}

void Context::framebufferParameteriMESA(GLenum target, GLenum pname, GLint param)
{
    framebufferParameteri(target, pname, param);
}

void Context::getFramebufferParameterivMESA(GLenum target, GLenum pname, GLint *params)
{
    getFramebufferParameteriv(target, pname, params);
}

bool Context::isGLES1() const
{
    return mState.isGLES1();
}

std::shared_ptr<angle::WorkerThreadPool> Context::getShaderCompileThreadPool() const
{
    if (mState.getExtensions().parallelShaderCompileKHR && mState.getMaxShaderCompilerThreads() > 0)
    {
        return mDisplay->getMultiThreadPool();
    }
    return mDisplay->getSingleThreadPool();
}

std::shared_ptr<angle::WorkerThreadPool> Context::getLinkSubTaskThreadPool() const
{
    return getFrontendFeatures().alwaysRunLinkSubJobsThreaded.enabled
               ? getWorkerThreadPool()
               : getShaderCompileThreadPool();
}

std::shared_ptr<angle::WaitableEvent> Context::postCompileLinkTask(
    const std::shared_ptr<angle::Closure> &task,
    angle::JobThreadSafety safety,
    angle::JobResultExpectancy resultExpectancy) const
{
    // If the compile/link job is not thread safe, use the single-thread pool.  Otherwise, the pool
    // that is configured by the application (through GL_KHR_parallel_shader_compile) is used.
    const bool isThreadSafe = safety == angle::JobThreadSafety::Safe;
    std::shared_ptr<angle::WorkerThreadPool> workerPool =
        isThreadSafe ? getShaderCompileThreadPool() : getSingleThreadPool();

    // If the job is thread-safe, but it's still not going to be threaded, then it's performed as an
    // unlocked tail call to allow other threads to proceed.  This is only possible if the results
    // of the call are not immediately needed in the same entry point call.
    if (isThreadSafe && !workerPool->isAsync() &&
        resultExpectancy == angle::JobResultExpectancy::Future &&
        !getShareGroup()->getFrameCaptureShared()->enabled())
    {
        std::shared_ptr<angle::AsyncWaitableEvent> event =
            std::make_shared<angle::AsyncWaitableEvent>();
        auto unlockedTask = [task, event](void *resultOut) {
            ANGLE_TRACE_EVENT0("gpu.angle", "Compile/Link (unlocked)");
            (*task)();
            event->markAsReady();
        };
        egl::Display::GetCurrentThreadUnlockedTailCall()->add(unlockedTask);
        return event;
    }

    // Otherwise, just schedule the task on the pool
    return workerPool->postWorkerTask(task);
}

std::shared_ptr<angle::WorkerThreadPool> Context::getSingleThreadPool() const
{
    return mDisplay->getSingleThreadPool();
}

std::shared_ptr<angle::WorkerThreadPool> Context::getWorkerThreadPool() const
{
    return mDisplay->getMultiThreadPool();
}

void Context::onUniformBlockBindingUpdated(GLuint uniformBlockIndex)
{
    mState.mDirtyBits.set(state::DIRTY_BIT_UNIFORM_BUFFER_BINDINGS);
    mState.mDirtyUniformBlocks.set(uniformBlockIndex);
    mStateCache.onUniformBufferStateChange(this);
}

void Context::endTilingImplicit()
{
    if (getMutablePrivateState()->isTiledRendering())
    {
        ANGLE_PERF_WARNING(getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Implicitly ending tiled rendering due to framebuffer state change");
        getMutablePrivateState()->setTiledRendering(false);
    }
}

void Context::onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message)
{
    switch (index)
    {
        case kVertexArraySubjectIndex:
            switch (message)
            {
                case angle::SubjectMessage::ContentsChanged:
                    mState.setObjectDirty(GL_VERTEX_ARRAY);
                    mStateCache.onVertexArrayBufferContentsChange(this);
                    break;
                case angle::SubjectMessage::SubjectMapped:
                case angle::SubjectMessage::SubjectUnmapped:
                case angle::SubjectMessage::BindingChanged:
                    mStateCache.onVertexArrayBufferStateChange(this);
                    break;
                default:
                    break;
            }
            break;

        case kReadFramebufferSubjectIndex:
            switch (message)
            {
                case angle::SubjectMessage::DirtyBitsFlagged:
                    mState.setReadFramebufferDirty();
                    break;
                case angle::SubjectMessage::SurfaceChanged:
                    mState.setReadFramebufferBindingDirty();
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            break;

        case kDrawFramebufferSubjectIndex:
            switch (message)
            {
                case angle::SubjectMessage::DirtyBitsFlagged:
                    mState.setDrawFramebufferDirty();
                    mStateCache.onDrawFramebufferChange(this);
                    break;
                case angle::SubjectMessage::SurfaceChanged:
                    mState.setDrawFramebufferBindingDirty();
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            break;

        case kProgramSubjectIndex:
            switch (message)
            {
                case angle::SubjectMessage::ProgramUnlinked:
                    mStateCache.onProgramExecutableChange(this);
                    break;
                case angle::SubjectMessage::ProgramRelinked:
                {
                    Program *program = mState.getProgram();
                    ASSERT(program->isLinked());
                    ANGLE_CONTEXT_TRY(mState.installProgramExecutable(this));
                    mStateCache.onProgramExecutableChange(this);
                    break;
                }
                default:
                    if (angle::IsProgramUniformBlockBindingUpdatedMessage(message))
                    {
                        onUniformBlockBindingUpdated(
                            angle::ProgramUniformBlockBindingUpdatedMessageToIndex(message));
                        break;
                    }
                    // Ignore all the other notifications
                    break;
            }
            break;

        case kProgramPipelineSubjectIndex:
            switch (message)
            {
                case angle::SubjectMessage::ProgramUnlinked:
                    mStateCache.onProgramExecutableChange(this);
                    break;
                case angle::SubjectMessage::ProgramRelinked:
                    ANGLE_CONTEXT_TRY(mState.installProgramPipelineExecutable(this));
                    mStateCache.onProgramExecutableChange(this);
                    break;
                default:
                    if (angle::IsProgramUniformBlockBindingUpdatedMessage(message))
                    {
                        // Note: if there's a program bound, its executable is used (and not the
                        // PPO's)
                        if (mState.getProgram() == nullptr)
                        {
                            onUniformBlockBindingUpdated(
                                angle::ProgramUniformBlockBindingUpdatedMessageToIndex(message));
                        }
                        break;
                    }
                    UNREACHABLE();
                    break;
            }
            break;

        default:
            if (index < kTextureMaxSubjectIndex)
            {
                if (message != angle::SubjectMessage::ContentsChanged &&
                    message != angle::SubjectMessage::BindingChanged)
                {
                    mState.onActiveTextureStateChange(this, index);
                    mStateCache.onActiveTextureChange(this);
                }
            }
            else if (index < kImageMaxSubjectIndex)
            {
                mState.onImageStateChange(this, index - kImage0SubjectIndex);
                if (message == angle::SubjectMessage::ContentsChanged)
                {
                    mState.mDirtyBits.set(state::DirtyBitType::DIRTY_BIT_IMAGE_BINDINGS);
                }
            }
            else if (index < kUniformBufferMaxSubjectIndex)
            {
                mState.onUniformBufferStateChange(index - kUniformBuffer0SubjectIndex);
                mStateCache.onUniformBufferStateChange(this);
            }
            else if (index < kAtomicCounterBufferMaxSubjectIndex)
            {
                mState.onAtomicCounterBufferStateChange(index - kAtomicCounterBuffer0SubjectIndex);
                mStateCache.onAtomicCounterBufferStateChange(this);
            }
            else if (index < kShaderStorageBufferMaxSubjectIndex)
            {
                mState.onShaderStorageBufferStateChange(index - kShaderStorageBuffer0SubjectIndex);
                mStateCache.onShaderStorageBufferStateChange(this);
            }
            else
            {
                ASSERT(index < kSamplerMaxSubjectIndex);
                mState.setSamplerDirty(index - kSampler0SubjectIndex);
                mState.onActiveTextureStateChange(this, index - kSampler0SubjectIndex);
            }
            break;
    }
}

egl::Error Context::setDefaultFramebuffer(egl::Surface *drawSurface, egl::Surface *readSurface)
{
    ASSERT(mCurrentDrawSurface == nullptr);
    ASSERT(mCurrentReadSurface == nullptr);

    mCurrentDrawSurface = drawSurface;
    mCurrentReadSurface = readSurface;

    if (drawSurface != nullptr)
    {
        ANGLE_TRY(drawSurface->makeCurrent(this));
    }

    ANGLE_TRY(mDefaultFramebuffer->setSurfaces(this, drawSurface, readSurface));

    if (readSurface && (drawSurface != readSurface))
    {
        ANGLE_TRY(readSurface->makeCurrent(this));
    }

    // Update default framebuffer, the binding of the previous default
    // framebuffer (or lack of) will have a nullptr.
    mState.mFramebufferManager->setDefaultFramebuffer(mDefaultFramebuffer.get());
    if (mState.getDrawFramebuffer() == nullptr)
    {
        bindDrawFramebuffer(mDefaultFramebuffer->id());
    }
    if (mState.getReadFramebuffer() == nullptr)
    {
        bindReadFramebuffer(mDefaultFramebuffer->id());
    }

    return egl::NoError();
}

egl::Error Context::unsetDefaultFramebuffer()
{
    Framebuffer *defaultFramebuffer =
        mState.mFramebufferManager->getFramebuffer(Framebuffer::kDefaultDrawFramebufferHandle);

    if (defaultFramebuffer)
    {
        // Remove the default framebuffer
        if (defaultFramebuffer == mState.getReadFramebuffer())
        {
            mState.setReadFramebufferBinding(nullptr);
            mReadFramebufferObserverBinding.bind(nullptr);
        }

        if (defaultFramebuffer == mState.getDrawFramebuffer())
        {
            mState.setDrawFramebufferBinding(nullptr);
            mDrawFramebufferObserverBinding.bind(nullptr);
        }

        ANGLE_TRY(defaultFramebuffer->unsetSurfaces(this));
        mState.mFramebufferManager->setDefaultFramebuffer(nullptr);
    }

    // Always unset the current surface, even if setIsCurrent fails.
    egl::Surface *drawSurface = mCurrentDrawSurface;
    egl::Surface *readSurface = mCurrentReadSurface;
    mCurrentDrawSurface       = nullptr;
    mCurrentReadSurface       = nullptr;
    if (drawSurface)
    {
        ANGLE_TRY(drawSurface->unMakeCurrent(this));
    }
    if (drawSurface != readSurface)
    {
        ANGLE_TRY(readSurface->unMakeCurrent(this));
    }

    return egl::NoError();
}

void Context::onPreSwap()
{
    // Dump frame capture if enabled.
    getShareGroup()->getFrameCaptureShared()->onEndFrame(this);
}

void Context::getTexImage(TextureTarget target,
                          GLint level,
                          GLenum format,
                          GLenum type,
                          void *pixels)
{
    Texture *texture   = getTextureByTarget(target);
    Buffer *packBuffer = mState.getTargetBuffer(BufferBinding::PixelPack);
    ANGLE_CONTEXT_TRY(texture->getTexImage(this, mState.getPackState(), packBuffer, target, level,
                                           format, type, pixels));
}

void Context::getCompressedTexImage(TextureTarget target, GLint level, void *pixels)
{
    Texture *texture   = getTextureByTarget(target);
    Buffer *packBuffer = mState.getTargetBuffer(BufferBinding::PixelPack);
    ANGLE_CONTEXT_TRY(texture->getCompressedTexImage(this, mState.getPackState(), packBuffer,
                                                     target, level, pixels));
}

void Context::getRenderbufferImage(GLenum target, GLenum format, GLenum type, void *pixels)
{
    Renderbuffer *renderbuffer = mState.getCurrentRenderbuffer();
    Buffer *packBuffer         = mState.getTargetBuffer(BufferBinding::PixelPack);
    ANGLE_CONTEXT_TRY(renderbuffer->getRenderbufferImage(this, mState.getPackState(), packBuffer,
                                                         format, type, pixels));
}

void Context::setLogicOpEnabledForGLES1(bool enabled)
{
    // Same implementation as ContextPrivateEnable(GL_COLOR_LOGIC_OP), without the GLES1 forwarding.
    getMutablePrivateState()->setLogicOpEnabled(enabled);
    getMutablePrivateStateCache()->onCapChange();
}

egl::Error Context::releaseHighPowerGPU()
{
    return mImplementation->releaseHighPowerGPU(this);
}

egl::Error Context::reacquireHighPowerGPU()
{
    return mImplementation->reacquireHighPowerGPU(this);
}

void Context::onGPUSwitch()
{
    // Re-initialize the renderer string, which just changed, and
    // which must be visible to applications.
    initRendererString();
}

egl::Error Context::acquireExternalContext(egl::Surface *drawAndReadSurface)
{
    mImplementation->acquireExternalContext(this);

    if (drawAndReadSurface != mCurrentDrawSurface || drawAndReadSurface != mCurrentReadSurface)
    {
        ANGLE_TRY(unsetDefaultFramebuffer());
        ANGLE_TRY(setDefaultFramebuffer(drawAndReadSurface, drawAndReadSurface));
    }

    return egl::NoError();
}

egl::Error Context::releaseExternalContext()
{
    mImplementation->releaseExternalContext(this);
    return egl::NoError();
}

angle::SimpleMutex &Context::getProgramCacheMutex() const
{
    return mDisplay->getProgramCacheMutex();
}

bool Context::supportsGeometryOrTesselation() const
{
    return mState.getClientVersion() == ES_3_2 || mState.getExtensions().geometryShaderAny() ||
           mState.getExtensions().tessellationShaderAny();
}

void Context::dirtyAllState()
{
    mState.setAllDirtyBits();
    mState.setAllDirtyObjects();
    getMutableGLES1State()->setAllDirty();
}

void Context::finishImmutable() const
{
    ANGLE_CONTEXT_TRY(mImplementation->finish(this));
}

void Context::beginPerfMonitor(GLuint monitor)
{
    getMutablePrivateState()->setPerfMonitorActive(true);
}

void Context::deletePerfMonitors(GLsizei n, GLuint *monitors) {}

void Context::endPerfMonitor(GLuint monitor)
{
    getMutablePrivateState()->setPerfMonitorActive(false);
}

void Context::genPerfMonitors(GLsizei n, GLuint *monitors)
{
    for (GLsizei monitorIndex = 0; monitorIndex < n; ++monitorIndex)
    {
        monitors[n] = static_cast<GLuint>(monitorIndex);
    }
}

void Context::getPerfMonitorCounterData(GLuint monitor,
                                        GLenum pname,
                                        GLsizei dataSize,
                                        GLuint *data,
                                        GLint *bytesWritten)
{
    using namespace angle;
    const PerfMonitorCounterGroups &perfMonitorGroups = mImplementation->getPerfMonitorCounters();
    GLint byteCount                                   = 0;
    switch (pname)
    {
        case GL_PERFMON_RESULT_AVAILABLE_AMD:
        {
            *data = GL_TRUE;
            byteCount += sizeof(GLuint);
            break;
        }
        case GL_PERFMON_RESULT_SIZE_AMD:
        {
            GLuint resultSize = 0;
            for (const PerfMonitorCounterGroup &group : perfMonitorGroups)
            {
                resultSize += sizeof(PerfMonitorTriplet) * group.counters.size();
            }
            *data = resultSize;
            byteCount += sizeof(GLuint);
            break;
        }
        case GL_PERFMON_RESULT_AMD:
        {
            PerfMonitorTriplet *resultsOut = reinterpret_cast<PerfMonitorTriplet *>(data);
            GLsizei maxResults             = dataSize / sizeof(PerfMonitorTriplet);
            GLsizei resultCount            = 0;
            for (size_t groupIndex = 0;
                 groupIndex < perfMonitorGroups.size() && resultCount < maxResults; ++groupIndex)
            {
                const PerfMonitorCounterGroup &group = perfMonitorGroups[groupIndex];
                for (size_t counterIndex = 0;
                     counterIndex < group.counters.size() && resultCount < maxResults;
                     ++counterIndex)
                {
                    const PerfMonitorCounter &counter = group.counters[counterIndex];
                    PerfMonitorTriplet &triplet       = resultsOut[resultCount++];
                    triplet.counter                   = static_cast<GLuint>(counterIndex);
                    triplet.group                     = static_cast<GLuint>(groupIndex);
                    triplet.value                     = counter.value;
                }
            }
            byteCount += sizeof(PerfMonitorTriplet) * resultCount;
            break;
        }
        default:
            UNREACHABLE();
    }

    if (bytesWritten)
    {
        *bytesWritten = byteCount;
    }
}

void Context::getPerfMonitorCounterInfo(GLuint group, GLuint counter, GLenum pname, void *data)
{
    using namespace angle;
    const PerfMonitorCounterGroups &perfMonitorGroups = mImplementation->getPerfMonitorCounters();
    ASSERT(group < perfMonitorGroups.size());
    const PerfMonitorCounters &counters = perfMonitorGroups[group].counters;
    ASSERT(counter < counters.size());

    switch (pname)
    {
        case GL_COUNTER_TYPE_AMD:
        {
            GLenum *dataOut = reinterpret_cast<GLenum *>(data);
            *dataOut        = GL_UNSIGNED_INT;
            break;
        }
        case GL_COUNTER_RANGE_AMD:
        {
            GLuint *dataOut = reinterpret_cast<GLuint *>(data);
            dataOut[0]      = 0;
            dataOut[1]      = std::numeric_limits<GLuint>::max();
            break;
        }
        default:
            UNREACHABLE();
    }
}

void Context::getPerfMonitorCounterString(GLuint group,
                                          GLuint counter,
                                          GLsizei bufSize,
                                          GLsizei *length,
                                          GLchar *counterString)
{
    using namespace angle;
    const PerfMonitorCounterGroups &perfMonitorGroups = mImplementation->getPerfMonitorCounters();
    ASSERT(group < perfMonitorGroups.size());
    const PerfMonitorCounters &counters = perfMonitorGroups[group].counters;
    ASSERT(counter < counters.size());
    GetPerfMonitorString(counters[counter].name, bufSize, length, counterString);
}

void Context::getPerfMonitorCounters(GLuint group,
                                     GLint *numCounters,
                                     GLint *maxActiveCounters,
                                     GLsizei counterSize,
                                     GLuint *counters)
{
    using namespace angle;
    const PerfMonitorCounterGroups &perfMonitorGroups = mImplementation->getPerfMonitorCounters();
    ASSERT(group < perfMonitorGroups.size());
    const PerfMonitorCounters &groupCounters = perfMonitorGroups[group].counters;

    if (numCounters)
    {
        *numCounters = static_cast<GLint>(groupCounters.size());
    }

    if (maxActiveCounters)
    {
        *maxActiveCounters = static_cast<GLint>(groupCounters.size());
    }

    if (counters)
    {
        GLsizei maxCounterIndex = std::min(counterSize, static_cast<GLsizei>(groupCounters.size()));
        for (GLsizei counterIndex = 0; counterIndex < maxCounterIndex; ++counterIndex)
        {
            counters[counterIndex] = static_cast<GLuint>(counterIndex);
        }
    }
}

void Context::getPerfMonitorGroupString(GLuint group,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLchar *groupString)
{
    using namespace angle;
    const PerfMonitorCounterGroups &perfMonitorGroups = mImplementation->getPerfMonitorCounters();
    ASSERT(group < perfMonitorGroups.size());
    GetPerfMonitorString(perfMonitorGroups[group].name, bufSize, length, groupString);
}

void Context::getPerfMonitorGroups(GLint *numGroups, GLsizei groupsSize, GLuint *groups)
{
    using namespace angle;
    const PerfMonitorCounterGroups &perfMonitorGroups = mImplementation->getPerfMonitorCounters();

    if (numGroups)
    {
        *numGroups = static_cast<GLint>(perfMonitorGroups.size());
    }

    GLuint maxGroupIndex =
        std::min<GLuint>(groupsSize, static_cast<GLuint>(perfMonitorGroups.size()));
    for (GLuint groupIndex = 0; groupIndex < maxGroupIndex; ++groupIndex)
    {
        groups[groupIndex] = groupIndex;
    }
}

void Context::selectPerfMonitorCounters(GLuint monitor,
                                        GLboolean enable,
                                        GLuint group,
                                        GLint numCounters,
                                        GLuint *counterList)
{}

const angle::PerfMonitorCounterGroups &Context::getPerfMonitorCounterGroups() const
{
    return mImplementation->getPerfMonitorCounters();
}

void Context::framebufferFoveationConfig(FramebufferID framebufferPacked,
                                         GLuint numLayers,
                                         GLuint focalPointsPerLayer,
                                         GLuint requestedFeatures,
                                         GLuint *providedFeatures)
{
    ASSERT(numLayers <= gl::IMPLEMENTATION_MAX_NUM_LAYERS);
    ASSERT(focalPointsPerLayer <= gl::IMPLEMENTATION_MAX_FOCAL_POINTS);
    ASSERT(providedFeatures);

    Framebuffer *framebuffer = getFramebuffer(framebufferPacked);
    ASSERT(!framebuffer->isFoveationConfigured());

    *providedFeatures = 0;
    // We only support GL_FOVEATION_ENABLE_BIT_QCOM feature, for now.
    // If requestedFeatures == 0 return without configuring the framebuffer.
    if (requestedFeatures != 0)
    {
        framebuffer->configureFoveation();
        *providedFeatures = framebuffer->getSupportedFoveationFeatures();
    }
}

void Context::framebufferFoveationParameters(FramebufferID framebufferPacked,
                                             GLuint layer,
                                             GLuint focalPoint,
                                             GLfloat focalX,
                                             GLfloat focalY,
                                             GLfloat gainX,
                                             GLfloat gainY,
                                             GLfloat foveaArea)
{
    Framebuffer *framebuffer = getFramebuffer(framebufferPacked);
    ASSERT(framebuffer);
    framebuffer->setFocalPoint(layer, focalPoint, focalX, focalY, gainX, gainY, foveaArea);
}

void Context::textureFoveationParameters(TextureID texturePacked,
                                         GLuint layer,
                                         GLuint focalPoint,
                                         GLfloat focalX,
                                         GLfloat focalY,
                                         GLfloat gainX,
                                         GLfloat gainY,
                                         GLfloat foveaArea)
{
    Texture *texture = getTexture(texturePacked);
    ASSERT(texture);
    texture->setFocalPoint(layer, focalPoint, focalX, focalY, gainX, gainY, foveaArea);
}

void Context::endTiling(GLbitfield preserveMask)
{
    ANGLE_CONTEXT_TRY(mImplementation->endTiling(this, preserveMask));
    getMutablePrivateState()->setTiledRendering(false);
}

void Context::startTiling(GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask)
{
    ANGLE_CONTEXT_TRY(syncDirtyObjects(kTilingDirtyObjects, Command::Other));
    ANGLE_CONTEXT_TRY(syncDirtyBits(kTilingDirtyBits, kTilingExtendedDirtyBits, Command::Other));
    ANGLE_CONTEXT_TRY(
        mImplementation->startTiling(this, Rectangle(x, y, width, height), preserveMask));
    getMutablePrivateState()->setTiledRendering(true);
}

void Context::clearTexImage(TextureID texturePacked,
                            GLint level,
                            GLenum format,
                            GLenum type,
                            const void *data)
{
    Texture *texture = getTexture(texturePacked);

    // Sync the texture's state directly. EXT_clear_texture does not require that the texture is
    // bound.
    if (texture->hasAnyDirtyBit())
    {
        ANGLE_CONTEXT_TRY(texture->syncState(this, Command::ClearTexture));
    }

    ANGLE_CONTEXT_TRY(
        texture->clearImage(this, level, format, type, static_cast<const uint8_t *>(data)));
}

void Context::clearTexSubImage(TextureID texturePacked,
                               GLint level,
                               GLint xoffset,
                               GLint yoffset,
                               GLint zoffset,
                               GLsizei width,
                               GLsizei height,
                               GLsizei depth,
                               GLenum format,
                               GLenum type,
                               const void *data)
{
    Texture *texture = getTexture(texturePacked);

    // It is allowed to use extents of 0 as input args. In this case, the function should return
    // with no changes to the texture.
    if (width == 0 || height == 0 || depth == 0)
    {
        return;
    }

    // Sync the texture's state directly. EXT_clear_texture does not require that the texture is
    // bound.
    if (texture->hasAnyDirtyBit())
    {
        ANGLE_CONTEXT_TRY(texture->syncState(this, Command::ClearTexture));
    }

    Box area(xoffset, yoffset, zoffset, width, height, depth);
    ANGLE_CONTEXT_TRY(texture->clearSubImage(this, level, area, format, type,
                                             static_cast<const uint8_t *>(data)));
}

void Context::blobCacheCallbacks(GLSETBLOBPROCANGLE set,
                                 GLGETBLOBPROCANGLE get,
                                 const void *userParam)
{
    mState.getBlobCacheCallbacks() = {set, get, userParam};
}

void Context::texStorageAttribs2D(GLenum target,
                                  GLsizei levels,
                                  GLenum internalFormat,
                                  GLsizei width,
                                  GLsizei height,
                                  const GLint *attribList)
{
    Extents size(width, height, 1);
    TextureType textype = FromGLenum<TextureType>(target);
    Texture *texture    = getTextureByType(textype);
    ANGLE_CONTEXT_TRY(
        texture->setStorageAttribs(this, textype, levels, internalFormat, size, attribList));
}

void Context::texStorageAttribs3D(GLenum target,
                                  GLsizei levels,
                                  GLenum internalFormat,
                                  GLsizei width,
                                  GLsizei height,
                                  GLsizei depth,
                                  const GLint *attribList)
{
    Extents size(width, height, depth);
    TextureType textype = FromGLenum<TextureType>(target);
    Texture *texture    = getTextureByType(textype);
    ANGLE_CONTEXT_TRY(
        texture->setStorageAttribs(this, textype, levels, internalFormat, size, attribList));
}

size_t Context::getMemoryUsage() const
{
    size_t memoryUsage = 0;

    memoryUsage += mState.mBufferManager->getTotalMemorySize();
    memoryUsage += mState.mRenderbufferManager->getTotalMemorySize();
    memoryUsage += mState.mTextureManager->getTotalMemorySize();

    return memoryUsage;
}

// ErrorSet implementation.
ErrorSet::ErrorSet(Debug *debug,
                   const angle::FrontendFeatures &frontendFeatures,
                   const egl::AttributeMap &attribs)
    : mDebug(debug),
      mResetStrategy(GetResetStrategy(attribs)),
      mLoseContextOnOutOfMemory(frontendFeatures.loseContextOnOutOfMemory.enabled),
      mContextLostForced(false),
      mResetStatus(GraphicsResetStatus::NoError),
      mSkipValidation(GetNoError(attribs)),
      mContextLost(0),
      mHasAnyErrors(0)
{}

ErrorSet::~ErrorSet() = default;

void ErrorSet::handleError(GLenum errorCode,
                           const char *message,
                           const char *file,
                           const char *function,
                           unsigned int line)
{
    if (errorCode == GL_OUT_OF_MEMORY && mResetStrategy == GL_LOSE_CONTEXT_ON_RESET_EXT &&
        mLoseContextOnOutOfMemory)
    {
        markContextLost(GraphicsResetStatus::UnknownContextReset);
    }

    std::stringstream errorStream;
    errorStream << "Error: " << gl::FmtHex(errorCode) << ", in " << file << ", " << function << ':'
                << line << ". " << message;

    std::string formattedMessage = errorStream.str();

    // Process the error, but log it with WARN severity so it shows up in logs.
    mDebug->insertMessage(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, errorCode,
                          GL_DEBUG_SEVERITY_HIGH, std::move(formattedMessage), gl::LOG_WARN,
                          angle::EntryPoint::Invalid);

    pushError(errorCode);
}

void ErrorSet::validationError(angle::EntryPoint entryPoint, GLenum errorCode, const char *message)
{
    mDebug->insertMessage(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, errorCode,
                          GL_DEBUG_SEVERITY_HIGH, message, gl::LOG_INFO, entryPoint);

    pushError(errorCode);
}

void ErrorSet::validationErrorF(angle::EntryPoint entryPoint,
                                GLenum errorCode,
                                const char *format,
                                ...)
{
    va_list vargs;
    va_start(vargs, format);
    constexpr size_t kMessageSize = 256;
    char message[kMessageSize];
    int r = vsnprintf(message, kMessageSize, format, vargs);
    va_end(vargs);

    if (r > 0)
    {
        validationError(entryPoint, errorCode, message);
    }
    else
    {
        validationError(entryPoint, errorCode, format);
    }
}

std::unique_lock<std::mutex> ErrorSet::getLockIfNotAlready()
{
    // Avoid mutex recursion and return the lock only if it is not already locked.  This can happen
    // if device loss is generated while it is being queried.
    if (mMutex.try_lock())
    {
        return std::unique_lock<std::mutex>(mMutex, std::adopt_lock);
    }
    return std::unique_lock<std::mutex>();
}

void ErrorSet::pushError(GLenum errorCode)
{
    ASSERT(errorCode != GL_NO_ERROR);
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mErrors.insert(errorCode);
        mHasAnyErrors = 1;
    }
}

GLenum ErrorSet::popError()
{
    std::lock_guard<std::mutex> lock(mMutex);

    ASSERT(!empty());
    GLenum error = *mErrors.begin();
    mErrors.erase(mErrors.begin());
    if (mErrors.empty())
    {
        mHasAnyErrors = 0;
    }
    return error;
}

// NOTE: this function should not assume that this context is current!
void ErrorSet::markContextLost(GraphicsResetStatus status)
{
    // This function may be called indirectly through ErrorSet::getGraphicsResetStatus() from the
    // backend, in which case mMutex is already held.
    std::unique_lock<std::mutex> lock = getLockIfNotAlready();

    ASSERT(status != GraphicsResetStatus::NoError);
    if (mResetStrategy == GL_LOSE_CONTEXT_ON_RESET_EXT)
    {
        mResetStatus       = status;
        mContextLostForced = true;
    }
    setContextLost();
}

void ErrorSet::setContextLost()
{
    // Always called with the mutex held.
    ASSERT(mMutex.try_lock() == false);

    mContextLost = 1;

    // Stop skipping validation, since many implementation entrypoint assume they can't
    // be called when lost, or with null object arguments, etc.
    mSkipValidation = 0;

    // Make sure we update TLS.
    SetCurrentValidContext(nullptr);
}

GLenum ErrorSet::getGraphicsResetStatus(rx::ContextImpl *contextImpl)
{
    std::lock_guard<std::mutex> lock(mMutex);

    // Even if the application doesn't want to know about resets, we want to know
    // as it will allow us to skip all the calls.
    if (mResetStrategy == GL_NO_RESET_NOTIFICATION_EXT)
    {
        if (!isContextLost() && contextImpl->getResetStatus() != GraphicsResetStatus::NoError)
        {
            setContextLost();
        }

        // EXT_robustness, section 2.6: If the reset notification behavior is
        // NO_RESET_NOTIFICATION_EXT, then the implementation will never deliver notification of
        // reset events, and GetGraphicsResetStatusEXT will always return NO_ERROR.
        return GL_NO_ERROR;
    }

    // The GL_EXT_robustness spec says that if a reset is encountered, a reset
    // status should be returned at least once, and GL_NO_ERROR should be returned
    // once the device has finished resetting.
    if (!isContextLost())
    {
        ASSERT(mResetStatus == GraphicsResetStatus::NoError);
        mResetStatus = contextImpl->getResetStatus();

        if (mResetStatus != GraphicsResetStatus::NoError)
        {
            setContextLost();
        }
    }
    else if (!mContextLostForced && mResetStatus != GraphicsResetStatus::NoError)
    {
        // If markContextLost was used to mark the context lost then
        // assume that is not recoverable, and continue to report the
        // lost reset status for the lifetime of this context.
        mResetStatus = contextImpl->getResetStatus();
    }

    return ToGLenum(mResetStatus);
}

GLenum ErrorSet::getErrorForCapture() const
{
    if (mErrors.empty())
    {
        return GL_NO_ERROR;
    }
    else
    {
        // Return the error without clearing it
        return *mErrors.begin();
    }
}

// StateCache implementation.
StateCache::StateCache()
    : mCachedNonInstancedVertexElementLimit(0),
      mCachedInstancedVertexElementLimit(0),
      mCachedBasicDrawStatesErrorString(kInvalidPointer),
      mCachedBasicDrawStatesErrorCode(GL_NO_ERROR),
      mCachedBasicDrawElementsError(kInvalidPointer),
      mCachedProgramPipelineError(kInvalidPointer),
      mCachedHasAnyEnabledClientAttrib(false),
      mCachedTransformFeedbackActiveUnpaused(false),
      mCachedCanDraw(false)
{
    mCachedValidDrawModes.fill(false);
}

StateCache::~StateCache() = default;

ANGLE_INLINE void StateCache::updateVertexElementLimits(Context *context)
{
    if (context->isBufferAccessValidationEnabled())
    {
        updateVertexElementLimitsImpl(context);
    }
}

void StateCache::initialize(Context *context)
{
    updateValidDrawModes(context);
    updateValidBindTextureTypes(context);
    updateValidDrawElementsTypes(context);
    updateBasicDrawStatesError();
    updateBasicDrawElementsError();
    updateVertexAttribTypesValidation(context);
    updateCanDraw(context);
}

void StateCache::updateActiveAttribsMask(Context *context)
{
    bool isGLES1         = context->isGLES1();
    const State &glState = context->getState();

    if (!isGLES1 && !glState.getProgramExecutable())
    {
        mCachedActiveBufferedAttribsMask = AttributesMask();
        mCachedActiveClientAttribsMask   = AttributesMask();
        mCachedActiveDefaultAttribsMask  = AttributesMask();
        return;
    }

    AttributesMask activeAttribs =
        isGLES1 ? glState.gles1().getActiveAttributesMask()
                : glState.getProgramExecutable()->getActiveAttribLocationsMask();

    const VertexArray *vao = glState.getVertexArray();
    ASSERT(vao);

    const AttributesMask &clientAttribs  = vao->getClientAttribsMask();
    const AttributesMask &enabledAttribs = vao->getEnabledAttributesMask();
    const AttributesMask &activeEnabled  = activeAttribs & enabledAttribs;

    mCachedActiveClientAttribsMask   = activeEnabled & clientAttribs;
    mCachedActiveBufferedAttribsMask = activeEnabled & ~clientAttribs;
    mCachedActiveDefaultAttribsMask  = activeAttribs & ~enabledAttribs;
    mCachedHasAnyEnabledClientAttrib = (clientAttribs & enabledAttribs).any();
}

void StateCache::updateVertexElementLimitsImpl(Context *context)
{
    ASSERT(context->isBufferAccessValidationEnabled());

    const VertexArray *vao = context->getState().getVertexArray();

    mCachedNonInstancedVertexElementLimit = std::numeric_limits<GLint64>::max();
    mCachedInstancedVertexElementLimit    = std::numeric_limits<GLint64>::max();

    // VAO can be null on Context startup. If we make this computation lazier we could ASSERT.
    // If there are no buffered attributes then we should not limit the draw call count.
    if (!vao || !mCachedActiveBufferedAttribsMask.any())
    {
        return;
    }

    const auto &vertexAttribs  = vao->getVertexAttributes();
    const auto &vertexBindings = vao->getVertexBindings();

    for (size_t attributeIndex : mCachedActiveBufferedAttribsMask)
    {
        const VertexAttribute &attrib = vertexAttribs[attributeIndex];

        const VertexBinding &binding = vertexBindings[attrib.bindingIndex];
        ASSERT(context->isGLES1() ||
               context->getState().getProgramExecutable()->isAttribLocationActive(attributeIndex));

        GLint64 limit = attrib.getCachedElementLimit();
        if (binding.getDivisor() > 0)
        {
            // For instanced draw calls, |divisor| times this limit is the limit for instance count
            // (because every |divisor| instances accesses the same attribute)
            angle::CheckedNumeric<GLint64> checkedLimit = limit;
            checkedLimit *= binding.getDivisor();

            mCachedInstancedVertexElementLimit =
                std::min<GLint64>(mCachedInstancedVertexElementLimit,
                                  checkedLimit.ValueOrDefault(VertexAttribute::kIntegerOverflow));
        }
        else
        {
            mCachedNonInstancedVertexElementLimit =
                std::min(mCachedNonInstancedVertexElementLimit, limit);
        }
    }
}

intptr_t StateCache::getBasicDrawStatesErrorImpl(const Context *context,
                                                 const PrivateStateCache *privateStateCache) const
{
    ASSERT(mCachedBasicDrawStatesErrorString == kInvalidPointer ||
           !privateStateCache->isCachedBasicDrawStatesErrorValid());
    ASSERT(mCachedBasicDrawStatesErrorCode == GL_NO_ERROR ||
           !privateStateCache->isCachedBasicDrawStatesErrorValid());

    // Only assign the error code after ValidateDrawStates has completed. ValidateDrawStates calls
    // updateBasicDrawStatesError in some cases and resets the value mid-call.
    GLenum errorCode = GL_NO_ERROR;
    mCachedBasicDrawStatesErrorString =
        reinterpret_cast<intptr_t>(ValidateDrawStates(context, &errorCode));
    mCachedBasicDrawStatesErrorCode = errorCode;

    // Ensure that if an error is set mCachedBasicDrawStatesErrorCode must be GL_NO_ERROR and if no
    // error is set mCachedBasicDrawStatesErrorCode must be an error.
    ASSERT((mCachedBasicDrawStatesErrorString == 0) ==
           (mCachedBasicDrawStatesErrorCode == GL_NO_ERROR));

    privateStateCache->setCachedBasicDrawStatesErrorValid();
    return mCachedBasicDrawStatesErrorString;
}

intptr_t StateCache::getProgramPipelineErrorImpl(const Context *context) const
{
    ASSERT(mCachedProgramPipelineError == kInvalidPointer);
    mCachedProgramPipelineError = reinterpret_cast<intptr_t>(ValidateProgramPipeline(context));
    return mCachedProgramPipelineError;
}

intptr_t StateCache::getBasicDrawElementsErrorImpl(const Context *context) const
{
    ASSERT(mCachedBasicDrawElementsError == kInvalidPointer);
    mCachedBasicDrawElementsError = reinterpret_cast<intptr_t>(ValidateDrawElementsStates(context));
    return mCachedBasicDrawElementsError;
}

void StateCache::onVertexArrayBindingChange(Context *context)
{
    updateActiveAttribsMask(context);
    updateVertexElementLimits(context);
    updateBasicDrawStatesError();
    updateBasicDrawElementsError();
}

void StateCache::onProgramExecutableChange(Context *context)
{
    updateActiveAttribsMask(context);
    updateVertexElementLimits(context);
    updateBasicDrawStatesError();
    updateProgramPipelineError();
    updateValidDrawModes(context);
    updateActiveShaderStorageBufferIndices(context);
    updateActiveImageUnitIndices(context);
    updateCanDraw(context);
}

void StateCache::onVertexArrayFormatChange(Context *context)
{
    updateVertexElementLimits(context);
}

void StateCache::onVertexArrayBufferContentsChange(Context *context)
{
    updateVertexElementLimits(context);
    updateBasicDrawStatesError();
}

void StateCache::onVertexArrayStateChange(Context *context)
{
    updateActiveAttribsMask(context);
    updateVertexElementLimits(context);
    updateBasicDrawStatesError();
    updateBasicDrawElementsError();
}

void StateCache::onVertexArrayBufferStateChange(Context *context)
{
    updateBasicDrawStatesError();
    updateBasicDrawElementsError();
}

void StateCache::onGLES1ClientStateChange(Context *context)
{
    updateActiveAttribsMask(context);
}

void StateCache::onGLES1TextureStateChange(Context *context)
{
    updateActiveAttribsMask(context);
}

void StateCache::onDrawFramebufferChange(Context *context)
{
    updateBasicDrawStatesError();
}

void StateCache::onActiveTextureChange(Context *context)
{
    updateBasicDrawStatesError();
}

void StateCache::onQueryChange(Context *context)
{
    updateBasicDrawStatesError();
}

void StateCache::onActiveTransformFeedbackChange(Context *context)
{
    updateTransformFeedbackActiveUnpaused(context);
    updateBasicDrawStatesError();
    updateBasicDrawElementsError();
    updateValidDrawModes(context);
}

void StateCache::onUniformBufferStateChange(Context *context)
{
    updateBasicDrawStatesError();
}

void StateCache::onAtomicCounterBufferStateChange(Context *context)
{
    updateBasicDrawStatesError();
}

void StateCache::onShaderStorageBufferStateChange(Context *context)
{
    updateBasicDrawStatesError();
}

void StateCache::setValidDrawModes(bool pointsOK,
                                   bool linesOK,
                                   bool trisOK,
                                   bool lineAdjOK,
                                   bool triAdjOK,
                                   bool patchOK)
{
    mCachedValidDrawModes[PrimitiveMode::Points]                 = pointsOK;
    mCachedValidDrawModes[PrimitiveMode::Lines]                  = linesOK;
    mCachedValidDrawModes[PrimitiveMode::LineLoop]               = linesOK;
    mCachedValidDrawModes[PrimitiveMode::LineStrip]              = linesOK;
    mCachedValidDrawModes[PrimitiveMode::Triangles]              = trisOK;
    mCachedValidDrawModes[PrimitiveMode::TriangleStrip]          = trisOK;
    mCachedValidDrawModes[PrimitiveMode::TriangleFan]            = trisOK;
    mCachedValidDrawModes[PrimitiveMode::LinesAdjacency]         = lineAdjOK;
    mCachedValidDrawModes[PrimitiveMode::LineStripAdjacency]     = lineAdjOK;
    mCachedValidDrawModes[PrimitiveMode::TrianglesAdjacency]     = triAdjOK;
    mCachedValidDrawModes[PrimitiveMode::TriangleStripAdjacency] = triAdjOK;
    mCachedValidDrawModes[PrimitiveMode::Patches]                = patchOK;
}

void StateCache::updateValidDrawModes(Context *context)
{
    const State &state = context->getState();

    const ProgramExecutable *programExecutable = context->getState().getProgramExecutable();

    // If tessellation is active primitive mode must be GL_PATCHES.
    if (programExecutable && programExecutable->hasLinkedTessellationShader())
    {
        setValidDrawModes(false, false, false, false, false, true);
        return;
    }

    if (mCachedTransformFeedbackActiveUnpaused)
    {
        TransformFeedback *curTransformFeedback = state.getCurrentTransformFeedback();

        // ES Spec 3.0 validation text:
        // When transform feedback is active and not paused, all geometric primitives generated must
        // match the value of primitiveMode passed to BeginTransformFeedback. The error
        // INVALID_OPERATION is generated by DrawArrays and DrawArraysInstanced if mode is not
        // identical to primitiveMode. The error INVALID_OPERATION is also generated by
        // DrawElements, DrawElementsInstanced, and DrawRangeElements while transform feedback is
        // active and not paused, regardless of mode. Any primitive type may be used while transform
        // feedback is paused.
        if (!context->getExtensions().geometryShaderAny() &&
            !context->getExtensions().tessellationShaderAny() &&
            context->getClientVersion() < ES_3_2)
        {
            mCachedValidDrawModes.fill(false);
            mCachedValidDrawModes[curTransformFeedback->getPrimitiveMode()] = true;
            return;
        }
    }

    if (!programExecutable || !programExecutable->hasLinkedShaderStage(ShaderType::Geometry))
    {
        bool adjacencyOK =
            (context->getExtensions().geometryShaderAny() || context->getClientVersion() >= ES_3_2);

        // All draw modes are valid, since drawing without a program does not generate an error and
        // operations requiring a GS will trigger other validation errors.
        // `patchOK = false` due to checking above already enabling it if a TS is present.
        setValidDrawModes(true, true, true, adjacencyOK, adjacencyOK, false);
        return;
    }

    PrimitiveMode gsMode = programExecutable->getGeometryShaderInputPrimitiveType();
    bool pointsOK        = gsMode == PrimitiveMode::Points;
    bool linesOK         = gsMode == PrimitiveMode::Lines;
    bool trisOK          = gsMode == PrimitiveMode::Triangles;
    bool lineAdjOK       = gsMode == PrimitiveMode::LinesAdjacency;
    bool triAdjOK        = gsMode == PrimitiveMode::TrianglesAdjacency;

    setValidDrawModes(pointsOK, linesOK, trisOK, lineAdjOK, triAdjOK, false);
}

void StateCache::updateValidBindTextureTypes(Context *context)
{
    const Extensions &exts = context->getExtensions();
    bool isGLES3           = context->getClientMajorVersion() >= 3;
    bool isGLES31          = context->getClientVersion() >= Version(3, 1);
    bool isGLES32          = context->getClientVersion() >= Version(3, 2);

    mCachedValidBindTextureTypes = {{
        {TextureType::_2D, true},
        {TextureType::_2DArray, isGLES3},
        {TextureType::_2DMultisample, isGLES31 || exts.textureMultisampleANGLE},
        {TextureType::_2DMultisampleArray, isGLES32 || exts.textureStorageMultisample2dArrayOES},
        {TextureType::_3D, isGLES3 || exts.texture3DOES},
        {TextureType::External, exts.EGLImageExternalOES || exts.EGLStreamConsumerExternalNV},
        {TextureType::Rectangle, exts.textureRectangleANGLE},
        {TextureType::CubeMap, true},
        {TextureType::CubeMapArray, isGLES32 || exts.textureCubeMapArrayAny()},
        {TextureType::VideoImage, exts.videoTextureWEBGL},
        {TextureType::Buffer, isGLES32 || exts.textureBufferAny()},
    }};
}

void StateCache::updateValidDrawElementsTypes(Context *context)
{
    bool supportsUint =
        (context->getClientMajorVersion() >= 3 || context->getExtensions().elementIndexUintOES);

    mCachedValidDrawElementsTypes = {{
        {DrawElementsType::UnsignedByte, true},
        {DrawElementsType::UnsignedShort, true},
        {DrawElementsType::UnsignedInt, supportsUint},
    }};
}

void StateCache::updateTransformFeedbackActiveUnpaused(Context *context)
{
    TransformFeedback *xfb                 = context->getState().getCurrentTransformFeedback();
    mCachedTransformFeedbackActiveUnpaused = xfb && xfb->isActive() && !xfb->isPaused();
}

void StateCache::updateVertexAttribTypesValidation(Context *context)
{
    VertexAttribTypeCase halfFloatValidity = (context->getExtensions().vertexHalfFloatOES)
                                                 ? VertexAttribTypeCase::Valid
                                                 : VertexAttribTypeCase::Invalid;

    VertexAttribTypeCase vertexType1010102Validity = (context->getExtensions().vertexType1010102OES)
                                                         ? VertexAttribTypeCase::ValidSize3or4
                                                         : VertexAttribTypeCase::Invalid;

    if (context->getClientMajorVersion() <= 2)
    {
        mCachedVertexAttribTypesValidation = {{
            {VertexAttribType::Byte, VertexAttribTypeCase::Valid},
            {VertexAttribType::Short, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedByte, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedShort, VertexAttribTypeCase::Valid},
            {VertexAttribType::Float, VertexAttribTypeCase::Valid},
            {VertexAttribType::Fixed, VertexAttribTypeCase::Valid},
            {VertexAttribType::HalfFloatOES, halfFloatValidity},
        }};
    }
    else
    {
        mCachedVertexAttribTypesValidation = {{
            {VertexAttribType::Byte, VertexAttribTypeCase::Valid},
            {VertexAttribType::Short, VertexAttribTypeCase::Valid},
            {VertexAttribType::Int, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedByte, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedShort, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedInt, VertexAttribTypeCase::Valid},
            {VertexAttribType::Float, VertexAttribTypeCase::Valid},
            {VertexAttribType::HalfFloat, VertexAttribTypeCase::Valid},
            {VertexAttribType::Fixed, VertexAttribTypeCase::Valid},
            {VertexAttribType::Int2101010, VertexAttribTypeCase::ValidSize4Only},
            {VertexAttribType::HalfFloatOES, halfFloatValidity},
            {VertexAttribType::UnsignedInt2101010, VertexAttribTypeCase::ValidSize4Only},
            {VertexAttribType::Int1010102, vertexType1010102Validity},
            {VertexAttribType::UnsignedInt1010102, vertexType1010102Validity},
        }};

        mCachedIntegerVertexAttribTypesValidation = {{
            {VertexAttribType::Byte, VertexAttribTypeCase::Valid},
            {VertexAttribType::Short, VertexAttribTypeCase::Valid},
            {VertexAttribType::Int, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedByte, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedShort, VertexAttribTypeCase::Valid},
            {VertexAttribType::UnsignedInt, VertexAttribTypeCase::Valid},
        }};
    }
}

void StateCache::updateActiveShaderStorageBufferIndices(Context *context)
{
    mCachedActiveShaderStorageBufferIndices.reset();
    const ProgramExecutable *executable = context->getState().getProgramExecutable();
    if (executable)
    {
        const std::vector<InterfaceBlock> &blocks = executable->getShaderStorageBlocks();
        for (size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex)
        {
            const GLuint binding = executable->getShaderStorageBlockBinding(blockIndex);
            mCachedActiveShaderStorageBufferIndices.set(binding);
        }
    }
}

void StateCache::updateActiveImageUnitIndices(Context *context)
{
    mCachedActiveImageUnitIndices.reset();
    const ProgramExecutable *executable = context->getState().getProgramExecutable();
    if (executable)
    {
        for (const ImageBinding &imageBinding : executable->getImageBindings())
        {
            for (GLuint binding : imageBinding.boundImageUnits)
            {
                mCachedActiveImageUnitIndices.set(binding);
            }
        }
    }
}

void StateCache::updateCanDraw(Context *context)
{
    // Can draw if:
    //
    // - Is GLES1: GLES1 always creates programs as needed
    // - There is an installed executable with a vertex shader
    // - A program pipeline is to be used: Program pipelines don't have a specific link function, so
    //   the pipeline might just be waiting to be linked at draw time (in which case there won't
    //   necessarily be an executable installed yet).
    mCachedCanDraw =
        context->isGLES1() || (context->getState().getProgramExecutable() &&
                               context->getState().getProgramExecutable()->hasVertexShader());
}

bool StateCache::isCurrentContext(const Context *context,
                                  const PrivateStateCache *privateStateCache) const
{
    // Ensure that the state cache is not queried by any context other than the one that owns it.
    return &context->getStateCache() == this &&
           &context->getPrivateStateCache() == privateStateCache;
}

PrivateStateCache::PrivateStateCache() : mIsCachedBasicDrawStatesErrorValid(true) {}

PrivateStateCache::~PrivateStateCache() = default;
}  // namespace gl
