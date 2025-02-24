//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererGL.h: Defines the class interface for RendererGL.

#ifndef LIBANGLE_RENDERER_GL_RENDERERGL_H_
#define LIBANGLE_RENDERER_GL_RENDERERGL_H_

#include <list>
#include <mutex>

#include "libANGLE/Caps.h"
#include "libANGLE/Error.h"
#include "libANGLE/Version.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "platform/autogen/FeaturesGL_autogen.h"

namespace angle
{
struct FrontendFeatures;
}  // namespace angle

namespace gl
{
struct IndexRange;
class Path;
class State;
}  // namespace gl

namespace egl
{
class AttributeMap;
}  // namespace egl

namespace sh
{
struct BlockMemberInfo;
}  // namespace sh

namespace rx
{
class BlitGL;
class ClearMultiviewGL;
class ContextImpl;
class DisplayGL;
class FunctionsGL;
class RendererGL;
class StateManagerGL;

class RendererGL : angle::NonCopyable
{
  public:
    RendererGL(std::unique_ptr<FunctionsGL> functions,
               const egl::AttributeMap &attribMap,
               DisplayGL *display);
    virtual ~RendererGL();

    angle::Result flush();
    angle::Result finish();

    gl::GraphicsResetStatus getResetStatus();

    // EXT_debug_marker
    void insertEventMarker(GLsizei length, const char *marker);
    void pushGroupMarker(GLsizei length, const char *marker);
    void popGroupMarker();

    // KHR_debug
    void pushDebugGroup(GLenum source, GLuint id, const std::string &message);
    void popDebugGroup();

    GLint getGPUDisjoint();
    GLint64 getTimestamp();

    const gl::Version &getMaxSupportedESVersion() const;
    const FunctionsGL *getFunctions() const { return mFunctions.get(); }
    StateManagerGL *getStateManager() const { return mStateManager; }
    const angle::FeaturesGL &getFeatures() const { return mFeatures; }
    BlitGL *getBlitter() const { return mBlitter; }
    ClearMultiviewGL *getMultiviewClearer() const { return mMultiviewClearer; }

    MultiviewImplementationTypeGL getMultiviewImplementationType() const;
    const gl::Caps &getNativeCaps() const;
    const gl::TextureCapsMap &getNativeTextureCaps() const;
    const gl::Extensions &getNativeExtensions() const;
    const gl::Limitations &getNativeLimitations() const;
    const ShPixelLocalStorageOptions &getNativePixelLocalStorageOptions() const;
    void initializeFrontendFeatures(angle::FrontendFeatures *features) const;

    angle::Result dispatchCompute(const gl::Context *context,
                                  GLuint numGroupsX,
                                  GLuint numGroupsY,
                                  GLuint numGroupsZ);
    angle::Result dispatchComputeIndirect(const gl::Context *context, GLintptr indirect);

    angle::Result memoryBarrier(GLbitfield barriers);
    angle::Result memoryBarrierByRegion(GLbitfield barriers);

    void framebufferFetchBarrier();

    // Checks if the driver has the KHR_parallel_shader_compile or ARB_parallel_shader_compile
    // extension.
    bool hasNativeParallelCompile();
    void setMaxShaderCompilerThreads(GLuint count);

    void setNeedsFlushBeforeDeleteTextures();
    void flushIfNecessaryBeforeDeleteTextures();

    void markWorkSubmitted();

    void handleGPUSwitch();

  private:
    void ensureCapsInitialized() const;
    void generateCaps(gl::Caps *outCaps,
                      gl::TextureCapsMap *outTextureCaps,
                      gl::Extensions *outExtensions,
                      gl::Limitations *outLimitations) const;

    mutable gl::Version mMaxSupportedESVersion;

    std::unique_ptr<FunctionsGL> mFunctions;
    StateManagerGL *mStateManager;

    BlitGL *mBlitter;
    ClearMultiviewGL *mMultiviewClearer;

    bool mUseDebugOutput;

    mutable bool mCapsInitialized;
    mutable gl::Caps mNativeCaps;
    mutable gl::TextureCapsMap mNativeTextureCaps;
    mutable gl::Extensions mNativeExtensions;
    mutable gl::Limitations mNativeLimitations;
    mutable ShPixelLocalStorageOptions mNativePLSOptions;
    mutable MultiviewImplementationTypeGL mMultiviewImplementationType;

    bool mWorkDoneSinceLastFlush = false;

    bool mNativeParallelCompileEnabled;

    angle::FeaturesGL mFeatures;

    // Workaround for anglebug.com/40644715
    bool mNeedsFlushBeforeDeleteTextures;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_RENDERERGL_H_
