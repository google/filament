//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererGL.cpp: Implements the class methods for RendererGL.

#include "libANGLE/renderer/gl/RendererGL.h"

#include <EGL/eglext.h>
#include <thread>

#include "common/debug.h"
#include "common/system_utils.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/State.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/BlitGL.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/ClearMultiviewGL.h"
#include "libANGLE/renderer/gl/CompilerGL.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/DisplayGL.h"
#include "libANGLE/renderer/gl/FenceNVGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/QueryGL.h"
#include "libANGLE/renderer/gl/RenderbufferGL.h"
#include "libANGLE/renderer/gl/SamplerGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/SyncGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/TransformFeedbackGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace
{

void SetMaxShaderCompilerThreads(const rx::FunctionsGL *functions, GLuint count)
{
    if (functions->maxShaderCompilerThreadsKHR != nullptr)
    {
        functions->maxShaderCompilerThreadsKHR(count);
    }
    else
    {
        ASSERT(functions->maxShaderCompilerThreadsARB != nullptr);
        functions->maxShaderCompilerThreadsARB(count);
    }
}

#if defined(ANGLE_PLATFORM_ANDROID)
const char *kIgnoredErrors[] = {
    // Wrong error message on Android Q Pixel 2. http://anglebug.com/42262155
    "FreeAllocationOnTimestamp - Reference to buffer created from "
    "different context without a share list. Application failed to pass "
    "share_context to eglCreateContext. Results are undefined.",
    // http://crbug.com/1348684
    "UpdateTimestamp - Reference to buffer created from different context without a share list. "
    "Application failed to pass share_context to eglCreateContext. Results are undefined.",
    "Attempt to use resource over contexts without enabling context sharing. App must pass a "
    "share_context to eglCreateContext() to share resources.",
};
#endif  // defined(ANGLE_PLATFORM_ANDROID)

const char *kIgnoredWarnings[] = {
    // We always request GL_ARB_gpu_shader5 and GL_EXT_gpu_shader5 when compiling shaders but some
    // drivers warn when it is not present. This ends up spamming the console on every shader
    // compile.
    "extension `GL_ARB_gpu_shader5' unsupported in",
    "extension `GL_EXT_gpu_shader5' unsupported in",
};

}  // namespace

static void INTERNAL_GL_APIENTRY LogGLDebugMessage(GLenum source,
                                                   GLenum type,
                                                   GLuint id,
                                                   GLenum severity,
                                                   GLsizei length,
                                                   const GLchar *message,
                                                   const void *userParam)
{
    std::string sourceText   = gl::GetDebugMessageSourceString(source);
    std::string typeText     = gl::GetDebugMessageTypeString(type);
    std::string severityText = gl::GetDebugMessageSeverityString(severity);

#if defined(ANGLE_PLATFORM_ANDROID)
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        for (const char *&err : kIgnoredErrors)
        {
            if (strncmp(err, message, length) == 0)
            {
                // There is only one ignored message right now and it is quite spammy, around 3MB
                // for a complete end2end tests run, so don't print it even as a warning.
                return;
            }
        }
    }
#endif  // defined(ANGLE_PLATFORM_ANDROID)

    if (type == GL_DEBUG_TYPE_ERROR)
    {
        ERR() << std::endl
              << "\tSource: " << sourceText << std::endl
              << "\tType: " << typeText << std::endl
              << "\tID: " << gl::FmtHex(id) << std::endl
              << "\tSeverity: " << severityText << std::endl
              << "\tMessage: " << message;
    }
    else if (type != GL_DEBUG_TYPE_PERFORMANCE)
    {
        // Don't print performance warnings. They tend to be very spammy in the dEQP test suite and
        // there is very little we can do about them.

        for (const char *&warn : kIgnoredWarnings)
        {
            if (strstr(message, warn) != nullptr)
            {
                return;
            }
        }

        // TODO(ynovikov): filter into WARN and INFO if INFO is ever implemented
        WARN() << std::endl
               << "\tSource: " << sourceText << std::endl
               << "\tType: " << typeText << std::endl
               << "\tID: " << gl::FmtHex(id) << std::endl
               << "\tSeverity: " << severityText << std::endl
               << "\tMessage: " << message;
    }
}

namespace rx
{

RendererGL::RendererGL(std::unique_ptr<FunctionsGL> functions,
                       const egl::AttributeMap &attribMap,
                       DisplayGL *display)
    : mMaxSupportedESVersion(0, 0),
      mFunctions(std::move(functions)),
      mStateManager(nullptr),
      mBlitter(nullptr),
      mMultiviewClearer(nullptr),
      mUseDebugOutput(false),
      mCapsInitialized(false),
      mMultiviewImplementationType(MultiviewImplementationTypeGL::UNSPECIFIED),
      mNativeParallelCompileEnabled(false),
      mNeedsFlushBeforeDeleteTextures(false)
{
    ASSERT(mFunctions);
    ApplyFeatureOverrides(&mFeatures, display->getState().featureOverrides);
    if (!display->getState().featureOverrides.allDisabled)
    {
        nativegl_gl::InitializeFeatures(mFunctions.get(), &mFeatures);
    }
    mStateManager =
        new StateManagerGL(mFunctions.get(), getNativeCaps(), getNativeExtensions(), mFeatures);
    mBlitter          = new BlitGL(mFunctions.get(), mFeatures, mStateManager);
    mMultiviewClearer = new ClearMultiviewGL(mFunctions.get(), mStateManager);

    bool hasDebugOutput = mFunctions->isAtLeastGL(gl::Version(4, 3)) ||
                          mFunctions->hasGLExtension("GL_KHR_debug") ||
                          mFunctions->isAtLeastGLES(gl::Version(3, 2)) ||
                          mFunctions->hasGLESExtension("GL_KHR_debug");

    mUseDebugOutput = hasDebugOutput && ShouldUseDebugLayers(attribMap);

    if (mUseDebugOutput)
    {
        mFunctions->enable(GL_DEBUG_OUTPUT);
        mFunctions->enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0,
                                        nullptr, GL_TRUE);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0,
                                        nullptr, GL_TRUE);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0,
                                        nullptr, GL_FALSE);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
                                        0, nullptr, GL_FALSE);
        mFunctions->debugMessageCallback(&LogGLDebugMessage, nullptr);
    }

    if (mFeatures.initializeCurrentVertexAttributes.enabled)
    {
        GLint maxVertexAttribs = 0;
        mFunctions->getIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);

        for (GLint i = 0; i < maxVertexAttribs; ++i)
        {
            mFunctions->vertexAttrib4f(i, 0.0f, 0.0f, 0.0f, 1.0f);
        }
    }

    if (hasNativeParallelCompile() && !mNativeParallelCompileEnabled)
    {
        SetMaxShaderCompilerThreads(mFunctions.get(), 0xffffffff);
        mNativeParallelCompileEnabled = true;
    }
}

RendererGL::~RendererGL()
{
    SafeDelete(mBlitter);
    SafeDelete(mMultiviewClearer);
    SafeDelete(mStateManager);
}

angle::Result RendererGL::flush()
{
    if (!mWorkDoneSinceLastFlush && !mNeedsFlushBeforeDeleteTextures)
    {
        return angle::Result::Continue;
    }

    mFunctions->flush();
    mNeedsFlushBeforeDeleteTextures = false;
    mWorkDoneSinceLastFlush         = false;
    return angle::Result::Continue;
}

angle::Result RendererGL::finish()
{
    if (mFeatures.finishDoesNotCauseQueriesToBeAvailable.enabled && mUseDebugOutput)
    {
        mFunctions->enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

    mFunctions->finish();
    mNeedsFlushBeforeDeleteTextures = false;
    mWorkDoneSinceLastFlush         = false;

    if (mFeatures.finishDoesNotCauseQueriesToBeAvailable.enabled && mUseDebugOutput)
    {
        mFunctions->disable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

    return angle::Result::Continue;
}

gl::GraphicsResetStatus RendererGL::getResetStatus()
{
    return gl::FromGLenum<gl::GraphicsResetStatus>(mFunctions->getGraphicsResetStatus());
}

void RendererGL::insertEventMarker(GLsizei length, const char *marker) {}

void RendererGL::pushGroupMarker(GLsizei length, const char *marker) {}

void RendererGL::popGroupMarker() {}

void RendererGL::pushDebugGroup(GLenum source, GLuint id, const std::string &message) {}

void RendererGL::popDebugGroup() {}

const gl::Version &RendererGL::getMaxSupportedESVersion() const
{
    // Force generation of caps
    getNativeCaps();

    return mMaxSupportedESVersion;
}

void RendererGL::generateCaps(gl::Caps *outCaps,
                              gl::TextureCapsMap *outTextureCaps,
                              gl::Extensions *outExtensions,
                              gl::Limitations *outLimitations) const
{
    nativegl_gl::GenerateCaps(mFunctions.get(), mFeatures, outCaps, outTextureCaps, outExtensions,
                              outLimitations, &mMaxSupportedESVersion,
                              &mMultiviewImplementationType, &mNativePLSOptions);
}

GLint RendererGL::getGPUDisjoint()
{
    // TODO(ewell): On GLES backends we should find a way to reliably query disjoint events
    return 0;
}

GLint64 RendererGL::getTimestamp()
{
    GLint64 result = 0;
    mFunctions->getInteger64v(GL_TIMESTAMP, &result);
    return result;
}

void RendererGL::ensureCapsInitialized() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mNativeCaps, &mNativeTextureCaps, &mNativeExtensions, &mNativeLimitations);
        mCapsInitialized = true;
    }
}

const gl::Caps &RendererGL::getNativeCaps() const
{
    ensureCapsInitialized();
    return mNativeCaps;
}

const gl::TextureCapsMap &RendererGL::getNativeTextureCaps() const
{
    ensureCapsInitialized();
    return mNativeTextureCaps;
}

const gl::Extensions &RendererGL::getNativeExtensions() const
{
    ensureCapsInitialized();
    return mNativeExtensions;
}

const gl::Limitations &RendererGL::getNativeLimitations() const
{
    ensureCapsInitialized();
    return mNativeLimitations;
}

const ShPixelLocalStorageOptions &RendererGL::getNativePixelLocalStorageOptions() const
{
    return mNativePLSOptions;
}

MultiviewImplementationTypeGL RendererGL::getMultiviewImplementationType() const
{
    ensureCapsInitialized();
    return mMultiviewImplementationType;
}

void RendererGL::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    ensureCapsInitialized();
    nativegl_gl::InitializeFrontendFeatures(mFunctions.get(), features);
}

angle::Result RendererGL::dispatchCompute(const gl::Context *context,
                                          GLuint numGroupsX,
                                          GLuint numGroupsY,
                                          GLuint numGroupsZ)
{
    mFunctions->dispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
    mWorkDoneSinceLastFlush = true;
    return angle::Result::Continue;
}

angle::Result RendererGL::dispatchComputeIndirect(const gl::Context *context, GLintptr indirect)
{
    mFunctions->dispatchComputeIndirect(indirect);
    mWorkDoneSinceLastFlush = true;
    return angle::Result::Continue;
}

angle::Result RendererGL::memoryBarrier(GLbitfield barriers)
{
    mFunctions->memoryBarrier(barriers);
    mWorkDoneSinceLastFlush = true;
    return angle::Result::Continue;
}
angle::Result RendererGL::memoryBarrierByRegion(GLbitfield barriers)
{
    mFunctions->memoryBarrierByRegion(barriers);
    mWorkDoneSinceLastFlush = true;
    return angle::Result::Continue;
}

void RendererGL::framebufferFetchBarrier()
{
    mFunctions->framebufferFetchBarrierEXT();
    mWorkDoneSinceLastFlush = true;
}

bool RendererGL::hasNativeParallelCompile()
{
    if (mFeatures.disableNativeParallelCompile.enabled)
    {
        return false;
    }
    return mFunctions->maxShaderCompilerThreadsKHR != nullptr ||
           mFunctions->maxShaderCompilerThreadsARB != nullptr;
}

void RendererGL::setMaxShaderCompilerThreads(GLuint count)
{
    if (hasNativeParallelCompile())
    {
        SetMaxShaderCompilerThreads(mFunctions.get(), count);
    }
}

void RendererGL::setNeedsFlushBeforeDeleteTextures()
{
    mNeedsFlushBeforeDeleteTextures = true;
}

void RendererGL::markWorkSubmitted()
{
    mWorkDoneSinceLastFlush = true;
}

void RendererGL::flushIfNecessaryBeforeDeleteTextures()
{
    if (mNeedsFlushBeforeDeleteTextures)
    {
        (void)flush();
    }
}

void RendererGL::handleGPUSwitch()
{
    nativegl_gl::ReInitializeFeaturesAtGPUSwitch(mFunctions.get(), &mFeatures);
}

}  // namespace rx
