//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextImpl:
//   Implementation-specific functionality associated with a GL Context.
//

#ifndef LIBANGLE_RENDERER_CONTEXTIMPL_H_
#define LIBANGLE_RENDERER_CONTEXTIMPL_H_

#include <vector>

#include "common/angleutils.h"
#include "libANGLE/State.h"
#include "libANGLE/renderer/GLImplFactory.h"

namespace gl
{
class ErrorSet;
class MemoryProgramCache;
class Path;
class PixelLocalStoragePlane;
class Semaphore;
struct Workarounds;
}  // namespace gl

namespace angle
{
struct ImageLoadContext;
}

namespace rx
{
class ContextImpl : public GLImplFactory
{
  public:
    ContextImpl(const gl::State &state, gl::ErrorSet *errorSet);
    ~ContextImpl() override;

    virtual void onDestroy(const gl::Context *context) {}

    virtual angle::Result initialize(const angle::ImageLoadContext &imageLoadContext) = 0;

    // Flush and finish.
    virtual angle::Result flush(const gl::Context *context)  = 0;
    virtual angle::Result finish(const gl::Context *context) = 0;

    // Drawing methods.
    virtual angle::Result drawArrays(const gl::Context *context,
                                     gl::PrimitiveMode mode,
                                     GLint first,
                                     GLsizei count)                  = 0;
    virtual angle::Result drawArraysInstanced(const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              GLint first,
                                              GLsizei count,
                                              GLsizei instanceCount) = 0;
    // Necessary for Vulkan since gl_InstanceIndex includes baseInstance
    virtual angle::Result drawArraysInstancedBaseInstance(const gl::Context *context,
                                                          gl::PrimitiveMode mode,
                                                          GLint first,
                                                          GLsizei count,
                                                          GLsizei instanceCount,
                                                          GLuint baseInstance) = 0;

    virtual angle::Result drawElements(const gl::Context *context,
                                       gl::PrimitiveMode mode,
                                       GLsizei count,
                                       gl::DrawElementsType type,
                                       const void *indices)                                = 0;
    virtual angle::Result drawElementsBaseVertex(const gl::Context *context,
                                                 gl::PrimitiveMode mode,
                                                 GLsizei count,
                                                 gl::DrawElementsType type,
                                                 const void *indices,
                                                 GLint baseVertex)                         = 0;
    virtual angle::Result drawElementsInstanced(const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                GLsizei count,
                                                gl::DrawElementsType type,
                                                const void *indices,
                                                GLsizei instances)                         = 0;
    virtual angle::Result drawElementsInstancedBaseVertex(const gl::Context *context,
                                                          gl::PrimitiveMode mode,
                                                          GLsizei count,
                                                          gl::DrawElementsType type,
                                                          const void *indices,
                                                          GLsizei instances,
                                                          GLint baseVertex)                = 0;
    virtual angle::Result drawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                                      gl::PrimitiveMode mode,
                                                                      GLsizei count,
                                                                      gl::DrawElementsType type,
                                                                      const void *indices,
                                                                      GLsizei instances,
                                                                      GLint baseVertex,
                                                                      GLuint baseInstance) = 0;
    virtual angle::Result drawRangeElements(const gl::Context *context,
                                            gl::PrimitiveMode mode,
                                            GLuint start,
                                            GLuint end,
                                            GLsizei count,
                                            gl::DrawElementsType type,
                                            const void *indices)                           = 0;
    virtual angle::Result drawRangeElementsBaseVertex(const gl::Context *context,
                                                      gl::PrimitiveMode mode,
                                                      GLuint start,
                                                      GLuint end,
                                                      GLsizei count,
                                                      gl::DrawElementsType type,
                                                      const void *indices,
                                                      GLint baseVertex)                    = 0;

    virtual angle::Result drawArraysIndirect(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             const void *indirect)   = 0;
    virtual angle::Result drawElementsIndirect(const gl::Context *context,
                                               gl::PrimitiveMode mode,
                                               gl::DrawElementsType type,
                                               const void *indirect) = 0;

    // MultiDraw* impl added as we need workaround for promoting dynamic attributes in D3D backend
    virtual angle::Result multiDrawArrays(const gl::Context *context,
                                          gl::PrimitiveMode mode,
                                          const GLint *firsts,
                                          const GLsizei *counts,
                                          GLsizei drawcount)                      = 0;
    virtual angle::Result multiDrawArraysIndirect(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  const void *indirect,
                                                  GLsizei drawcount,
                                                  GLsizei stride)                 = 0;
    virtual angle::Result multiDrawArraysInstanced(const gl::Context *context,
                                                   gl::PrimitiveMode mode,
                                                   const GLint *firsts,
                                                   const GLsizei *counts,
                                                   const GLsizei *instanceCounts,
                                                   GLsizei drawcount)             = 0;
    virtual angle::Result multiDrawElements(const gl::Context *context,
                                            gl::PrimitiveMode mode,
                                            const GLsizei *counts,
                                            gl::DrawElementsType type,
                                            const GLvoid *const *indices,
                                            GLsizei drawcount)                    = 0;
    virtual angle::Result multiDrawElementsInstanced(const gl::Context *context,
                                                     gl::PrimitiveMode mode,
                                                     const GLsizei *counts,
                                                     gl::DrawElementsType type,
                                                     const GLvoid *const *indices,
                                                     const GLsizei *instanceCounts,
                                                     GLsizei drawcount)           = 0;
    virtual angle::Result multiDrawElementsIndirect(const gl::Context *context,
                                                    gl::PrimitiveMode mode,
                                                    gl::DrawElementsType type,
                                                    const void *indirect,
                                                    GLsizei drawcount,
                                                    GLsizei stride)               = 0;
    virtual angle::Result multiDrawArraysInstancedBaseInstance(const gl::Context *context,
                                                               gl::PrimitiveMode mode,
                                                               const GLint *firsts,
                                                               const GLsizei *counts,
                                                               const GLsizei *instanceCounts,
                                                               const GLuint *baseInstances,
                                                               GLsizei drawcount) = 0;
    virtual angle::Result multiDrawElementsInstancedBaseVertexBaseInstance(
        const gl::Context *context,
        gl::PrimitiveMode mode,
        const GLsizei *counts,
        gl::DrawElementsType type,
        const GLvoid *const *indices,
        const GLsizei *instanceCounts,
        const GLint *baseVertices,
        const GLuint *baseInstances,
        GLsizei drawcount) = 0;

    // Device loss
    virtual gl::GraphicsResetStatus getResetStatus() = 0;

    // EXT_debug_marker
    virtual angle::Result insertEventMarker(GLsizei length, const char *marker) = 0;
    virtual angle::Result pushGroupMarker(GLsizei length, const char *marker)   = 0;
    virtual angle::Result popGroupMarker()                                      = 0;

    // KHR_debug
    virtual angle::Result pushDebugGroup(const gl::Context *context,
                                         GLenum source,
                                         GLuint id,
                                         const std::string &message) = 0;
    virtual angle::Result popDebugGroup(const gl::Context *context)  = 0;
    virtual angle::Result handleNoopDrawEvent();

    // KHR_parallel_shader_compile
    virtual void setMaxShaderCompilerThreads(GLuint count) {}

    // GL_ANGLE_texture_storage_external
    virtual void invalidateTexture(gl::TextureType target);

    // EXT_shader_framebuffer_fetch_non_coherent
    virtual void framebufferFetchBarrier() {}

    // KHR_blend_equation_advanced
    virtual void blendBarrier() {}

    // QCOM_tiled_rendering
    virtual angle::Result startTiling(const gl::Context *context,
                                      const gl::Rectangle &area,
                                      GLbitfield preserveMask);
    virtual angle::Result endTiling(const gl::Context *context, GLbitfield preserveMask);

    // State sync with dirty bits.
    virtual angle::Result syncState(const gl::Context *context,
                                    const gl::state::DirtyBits dirtyBits,
                                    const gl::state::DirtyBits bitMask,
                                    const gl::state::ExtendedDirtyBits extendedDirtyBits,
                                    const gl::state::ExtendedDirtyBits extendedBitMask,
                                    gl::Command command) = 0;

    // Disjoint timer queries
    virtual GLint getGPUDisjoint() = 0;
    virtual GLint64 getTimestamp() = 0;

    // Context switching
    virtual angle::Result onMakeCurrent(const gl::Context *context) = 0;
    virtual angle::Result onUnMakeCurrent(const gl::Context *context);

    // Native capabilities, unmodified by gl::Context.
    virtual gl::Caps getNativeCaps() const                                              = 0;
    virtual const gl::TextureCapsMap &getNativeTextureCaps() const                      = 0;
    virtual const gl::Extensions &getNativeExtensions() const                           = 0;
    virtual const gl::Limitations &getNativeLimitations() const                         = 0;
    virtual const ShPixelLocalStorageOptions &getNativePixelLocalStorageOptions() const = 0;

    virtual angle::Result dispatchCompute(const gl::Context *context,
                                          GLuint numGroupsX,
                                          GLuint numGroupsY,
                                          GLuint numGroupsZ)         = 0;
    virtual angle::Result dispatchComputeIndirect(const gl::Context *context,
                                                  GLintptr indirect) = 0;

    virtual angle::Result memoryBarrier(const gl::Context *context, GLbitfield barriers) = 0;
    virtual angle::Result memoryBarrierByRegion(const gl::Context *context,
                                                GLbitfield barriers)                     = 0;

    const gl::State &getState() const { return mState; }
    int getClientMajorVersion() const { return mState.getClientMajorVersion(); }
    int getClientMinorVersion() const { return mState.getClientMinorVersion(); }
    const gl::Caps &getCaps() const { return mState.getCaps(); }
    const gl::TextureCapsMap &getTextureCaps() const { return mState.getTextureCaps(); }
    const gl::Extensions &getExtensions() const { return mState.getExtensions(); }
    const gl::Limitations &getLimitations() const { return mState.getLimitations(); }

    // A common GL driver behaviour is to trigger dynamic shader recompilation on a draw call,
    // based on the current render states. We store a mutable pointer to the program cache so
    // on draw calls we can store the refreshed shaders in the cache.
    void setMemoryProgramCache(gl::MemoryProgramCache *memoryProgramCache);

    void handleError(GLenum errorCode,
                     const char *message,
                     const char *file,
                     const char *function,
                     unsigned int line);

    virtual egl::ContextPriority getContextPriority() const;

    // EGL_ANGLE_power_preference implementation.
    virtual egl::Error releaseHighPowerGPU(gl::Context *context);
    virtual egl::Error reacquireHighPowerGPU(gl::Context *context);

    // EGL_ANGLE_external_context_and_surface
    virtual void acquireExternalContext(const gl::Context *context);
    virtual void releaseExternalContext(const gl::Context *context);

    // GL_ANGLE_vulkan_image
    virtual angle::Result acquireTextures(const gl::Context *context,
                                          const gl::TextureBarrierVector &textureBarriers);
    virtual angle::Result releaseTextures(const gl::Context *context,
                                          gl::TextureBarrierVector *textureBarriers);

    // AMD_performance_monitor
    virtual const angle::PerfMonitorCounterGroups &getPerfMonitorCounters();

  protected:
    const gl::State &mState;
    gl::MemoryProgramCache *mMemoryProgramCache;
    gl::ErrorSet *mErrors;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CONTEXTIMPL_H_
