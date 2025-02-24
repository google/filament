//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextNULL.cpp:
//    Implements the class methods for ContextNULL.
//

#include "libANGLE/renderer/null/ContextNULL.h"

#include "common/debug.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/OverlayImpl.h"
#include "libANGLE/renderer/null/BufferNULL.h"
#include "libANGLE/renderer/null/CompilerNULL.h"
#include "libANGLE/renderer/null/DisplayNULL.h"
#include "libANGLE/renderer/null/FenceNVNULL.h"
#include "libANGLE/renderer/null/FramebufferNULL.h"
#include "libANGLE/renderer/null/ImageNULL.h"
#include "libANGLE/renderer/null/ProgramExecutableNULL.h"
#include "libANGLE/renderer/null/ProgramNULL.h"
#include "libANGLE/renderer/null/ProgramPipelineNULL.h"
#include "libANGLE/renderer/null/QueryNULL.h"
#include "libANGLE/renderer/null/RenderbufferNULL.h"
#include "libANGLE/renderer/null/SamplerNULL.h"
#include "libANGLE/renderer/null/ShaderNULL.h"
#include "libANGLE/renderer/null/SyncNULL.h"
#include "libANGLE/renderer/null/TextureNULL.h"
#include "libANGLE/renderer/null/TransformFeedbackNULL.h"
#include "libANGLE/renderer/null/VertexArrayNULL.h"

namespace rx
{

AllocationTrackerNULL::AllocationTrackerNULL(size_t maxTotalAllocationSize)
    : mAllocatedBytes(0), mMaxBytes(maxTotalAllocationSize)
{}

AllocationTrackerNULL::~AllocationTrackerNULL()
{
    // ASSERT that all objects with the NULL renderer clean up after themselves
    ASSERT(mAllocatedBytes == 0);
}

bool AllocationTrackerNULL::updateMemoryAllocation(size_t oldSize, size_t newSize)
{
    ASSERT(mAllocatedBytes >= oldSize);

    size_t sizeAfterRelease    = mAllocatedBytes - oldSize;
    size_t sizeAfterReallocate = sizeAfterRelease + newSize;
    if (sizeAfterReallocate < sizeAfterRelease || sizeAfterReallocate > mMaxBytes)
    {
        // Overflow or allocation would be too large
        return false;
    }

    mAllocatedBytes = sizeAfterReallocate;
    return true;
}

ContextNULL::ContextNULL(const gl::State &state,
                         gl::ErrorSet *errorSet,
                         AllocationTrackerNULL *allocationTracker)
    : ContextImpl(state, errorSet), mAllocationTracker(allocationTracker)
{
    ASSERT(mAllocationTracker != nullptr);

    mExtensions                               = gl::Extensions();
    mExtensions.blendEquationAdvancedKHR      = true;
    mExtensions.blendFuncExtendedEXT          = true;
    mExtensions.copyCompressedTextureCHROMIUM = true;
    mExtensions.copyTextureCHROMIUM           = true;
    mExtensions.debugMarkerEXT                = true;
    mExtensions.drawBuffersIndexedOES         = true;
    mExtensions.fenceNV                       = true;
    mExtensions.framebufferBlitANGLE          = true;
    mExtensions.framebufferBlitNV             = true;
    mExtensions.instancedArraysANGLE          = true;
    mExtensions.instancedArraysEXT            = true;
    mExtensions.mapBufferRangeEXT             = true;
    mExtensions.mapbufferOES                  = true;
    mExtensions.pixelBufferObjectNV           = true;
    mExtensions.shaderPixelLocalStorageANGLE  = state.getClientVersion() >= gl::Version(3, 0);
    mExtensions.shaderPixelLocalStorageCoherentANGLE = mExtensions.shaderPixelLocalStorageANGLE;
    mExtensions.textureRectangleANGLE                = true;
    mExtensions.textureUsageANGLE                    = true;
    mExtensions.translatedShaderSourceANGLE          = true;
    mExtensions.vertexArrayObjectOES                 = true;

    mExtensions.textureStorageEXT               = true;
    mExtensions.rgb8Rgba8OES                    = true;
    mExtensions.textureCompressionDxt1EXT       = true;
    mExtensions.textureCompressionDxt3ANGLE     = true;
    mExtensions.textureCompressionDxt5ANGLE     = true;
    mExtensions.textureCompressionS3tcSrgbEXT   = true;
    mExtensions.textureCompressionAstcHdrKHR    = true;
    mExtensions.textureCompressionAstcLdrKHR    = true;
    mExtensions.textureCompressionAstcOES       = true;
    mExtensions.compressedETC1RGB8TextureOES    = true;
    mExtensions.compressedETC1RGB8SubTextureEXT = true;
    mExtensions.lossyEtcDecodeANGLE             = true;
    mExtensions.geometryShaderEXT               = true;
    mExtensions.geometryShaderOES               = true;
    mExtensions.multiDrawIndirectEXT            = true;

    mExtensions.EGLImageOES                 = true;
    mExtensions.EGLImageExternalOES         = true;
    mExtensions.EGLImageExternalEssl3OES    = true;
    mExtensions.EGLImageArrayEXT            = true;
    mExtensions.EGLStreamConsumerExternalNV = true;

    const gl::Version maxClientVersion(3, 1);
    mCaps = GenerateMinimumCaps(maxClientVersion, mExtensions);

    InitMinimumTextureCapsMap(maxClientVersion, mExtensions, &mTextureCaps);

    if (mExtensions.shaderPixelLocalStorageANGLE)
    {
        mPLSOptions.type             = ShPixelLocalStorageType::FramebufferFetch;
        mPLSOptions.fragmentSyncType = ShFragmentSynchronizationType::Automatic;
    }
}

ContextNULL::~ContextNULL() {}

angle::Result ContextNULL::initialize(const angle::ImageLoadContext &imageLoadContext)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::flush(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::finish(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawArrays(const gl::Context *context,
                                      gl::PrimitiveMode mode,
                                      GLint first,
                                      GLsizei count)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawArraysInstanced(const gl::Context *context,
                                               gl::PrimitiveMode mode,
                                               GLint first,
                                               GLsizei count,
                                               GLsizei instanceCount)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawArraysInstancedBaseInstance(const gl::Context *context,
                                                           gl::PrimitiveMode mode,
                                                           GLint first,
                                                           GLsizei count,
                                                           GLsizei instanceCount,
                                                           GLuint baseInstance)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawElements(const gl::Context *context,
                                        gl::PrimitiveMode mode,
                                        GLsizei count,
                                        gl::DrawElementsType type,
                                        const void *indices)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawElementsBaseVertex(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  GLsizei count,
                                                  gl::DrawElementsType type,
                                                  const void *indices,
                                                  GLint baseVertex)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawElementsInstanced(const gl::Context *context,
                                                 gl::PrimitiveMode mode,
                                                 GLsizei count,
                                                 gl::DrawElementsType type,
                                                 const void *indices,
                                                 GLsizei instances)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawElementsInstancedBaseVertex(const gl::Context *context,
                                                           gl::PrimitiveMode mode,
                                                           GLsizei count,
                                                           gl::DrawElementsType type,
                                                           const void *indices,
                                                           GLsizei instances,
                                                           GLint baseVertex)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                                       gl::PrimitiveMode mode,
                                                                       GLsizei count,
                                                                       gl::DrawElementsType type,
                                                                       const void *indices,
                                                                       GLsizei instances,
                                                                       GLint baseVertex,
                                                                       GLuint baseInstance)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawRangeElements(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             GLuint start,
                                             GLuint end,
                                             GLsizei count,
                                             gl::DrawElementsType type,
                                             const void *indices)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawRangeElementsBaseVertex(const gl::Context *context,
                                                       gl::PrimitiveMode mode,
                                                       GLuint start,
                                                       GLuint end,
                                                       GLsizei count,
                                                       gl::DrawElementsType type,
                                                       const void *indices,
                                                       GLint baseVertex)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawArraysIndirect(const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              const void *indirect)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::drawElementsIndirect(const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                gl::DrawElementsType type,
                                                const void *indirect)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawArrays(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           const GLint *firsts,
                                           const GLsizei *counts,
                                           GLsizei drawcount)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawArraysInstanced(const gl::Context *context,
                                                    gl::PrimitiveMode mode,
                                                    const GLint *firsts,
                                                    const GLsizei *counts,
                                                    const GLsizei *instanceCounts,
                                                    GLsizei drawcount)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawArraysIndirect(const gl::Context *context,
                                                   gl::PrimitiveMode mode,
                                                   const void *indirect,
                                                   GLsizei drawcount,
                                                   GLsizei stride)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawElements(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             const GLsizei *counts,
                                             gl::DrawElementsType type,
                                             const GLvoid *const *indices,
                                             GLsizei drawcount)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawElementsInstanced(const gl::Context *context,
                                                      gl::PrimitiveMode mode,
                                                      const GLsizei *counts,
                                                      gl::DrawElementsType type,
                                                      const GLvoid *const *indices,
                                                      const GLsizei *instanceCounts,
                                                      GLsizei drawcount)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawElementsIndirect(const gl::Context *context,
                                                     gl::PrimitiveMode mode,
                                                     gl::DrawElementsType type,
                                                     const void *indirect,
                                                     GLsizei drawcount,
                                                     GLsizei stride)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawArraysInstancedBaseInstance(const gl::Context *context,
                                                                gl::PrimitiveMode mode,
                                                                const GLint *firsts,
                                                                const GLsizei *counts,
                                                                const GLsizei *instanceCounts,
                                                                const GLuint *baseInstances,
                                                                GLsizei drawcount)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::multiDrawElementsInstancedBaseVertexBaseInstance(
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
    return angle::Result::Continue;
}

gl::GraphicsResetStatus ContextNULL::getResetStatus()
{
    return gl::GraphicsResetStatus::NoError;
}

angle::Result ContextNULL::insertEventMarker(GLsizei length, const char *marker)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::pushGroupMarker(GLsizei length, const char *marker)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::popGroupMarker()
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::pushDebugGroup(const gl::Context *context,
                                          GLenum source,
                                          GLuint id,
                                          const std::string &message)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::popDebugGroup(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::syncState(const gl::Context *context,
                                     const gl::state::DirtyBits dirtyBits,
                                     const gl::state::DirtyBits bitMask,
                                     const gl::state::ExtendedDirtyBits extendedDirtyBits,
                                     const gl::state::ExtendedDirtyBits extendedBitMask,
                                     gl::Command command)
{
    return angle::Result::Continue;
}

GLint ContextNULL::getGPUDisjoint()
{
    return 0;
}

GLint64 ContextNULL::getTimestamp()
{
    return 0;
}

angle::Result ContextNULL::onMakeCurrent(const gl::Context *context)
{
    return angle::Result::Continue;
}

gl::Caps ContextNULL::getNativeCaps() const
{
    return mCaps;
}

const gl::TextureCapsMap &ContextNULL::getNativeTextureCaps() const
{
    return mTextureCaps;
}

const gl::Extensions &ContextNULL::getNativeExtensions() const
{
    return mExtensions;
}

const gl::Limitations &ContextNULL::getNativeLimitations() const
{
    return mLimitations;
}

const ShPixelLocalStorageOptions &ContextNULL::getNativePixelLocalStorageOptions() const
{
    return mPLSOptions;
}

CompilerImpl *ContextNULL::createCompiler()
{
    return new CompilerNULL();
}

ShaderImpl *ContextNULL::createShader(const gl::ShaderState &data)
{
    return new ShaderNULL(data);
}

ProgramImpl *ContextNULL::createProgram(const gl::ProgramState &data)
{
    return new ProgramNULL(data);
}

ProgramExecutableImpl *ContextNULL::createProgramExecutable(const gl::ProgramExecutable *executable)
{
    return new ProgramExecutableNULL(executable);
}

FramebufferImpl *ContextNULL::createFramebuffer(const gl::FramebufferState &data)
{
    return new FramebufferNULL(data);
}

TextureImpl *ContextNULL::createTexture(const gl::TextureState &state)
{
    return new TextureNULL(state);
}

RenderbufferImpl *ContextNULL::createRenderbuffer(const gl::RenderbufferState &state)
{
    return new RenderbufferNULL(state);
}

BufferImpl *ContextNULL::createBuffer(const gl::BufferState &state)
{
    return new BufferNULL(state, mAllocationTracker);
}

VertexArrayImpl *ContextNULL::createVertexArray(const gl::VertexArrayState &data)
{
    return new VertexArrayNULL(data);
}

QueryImpl *ContextNULL::createQuery(gl::QueryType type)
{
    return new QueryNULL(type);
}

FenceNVImpl *ContextNULL::createFenceNV()
{
    return new FenceNVNULL();
}

SyncImpl *ContextNULL::createSync()
{
    return new SyncNULL();
}

TransformFeedbackImpl *ContextNULL::createTransformFeedback(const gl::TransformFeedbackState &state)
{
    return new TransformFeedbackNULL(state);
}

SamplerImpl *ContextNULL::createSampler(const gl::SamplerState &state)
{
    return new SamplerNULL(state);
}

ProgramPipelineImpl *ContextNULL::createProgramPipeline(const gl::ProgramPipelineState &state)
{
    return new ProgramPipelineNULL(state);
}

MemoryObjectImpl *ContextNULL::createMemoryObject()
{
    UNREACHABLE();
    return nullptr;
}

SemaphoreImpl *ContextNULL::createSemaphore()
{
    UNREACHABLE();
    return nullptr;
}

OverlayImpl *ContextNULL::createOverlay(const gl::OverlayState &state)
{
    return new OverlayImpl(state);
}

angle::Result ContextNULL::dispatchCompute(const gl::Context *context,
                                           GLuint numGroupsX,
                                           GLuint numGroupsY,
                                           GLuint numGroupsZ)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::dispatchComputeIndirect(const gl::Context *context, GLintptr indirect)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::memoryBarrier(const gl::Context *context, GLbitfield barriers)
{
    return angle::Result::Continue;
}

angle::Result ContextNULL::memoryBarrierByRegion(const gl::Context *context, GLbitfield barriers)
{
    return angle::Result::Continue;
}

void ContextNULL::handleError(GLenum errorCode,
                              const char *message,
                              const char *file,
                              const char *function,
                              unsigned int line)
{
    std::stringstream errorStream;
    errorStream << "Internal NULL back-end error: " << message << ".";
    mErrors->handleError(errorCode, errorStream.str().c_str(), file, function, line);
}
}  // namespace rx
