//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Context9:
//   D3D9-specific functionality associated with a GL Context.
//

#include "libANGLE/renderer/d3d/d3d9/Context9.h"

#include "common/entry_points_enum_autogen.h"
#include "common/string_utils.h"
#include "image_util/loadimage.h"
#include "libANGLE/renderer/OverlayImpl.h"
#include "libANGLE/renderer/d3d/CompilerD3D.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/ProgramExecutableD3D.h"
#include "libANGLE/renderer/d3d/RenderbufferD3D.h"
#include "libANGLE/renderer/d3d/SamplerD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/d3d9/Buffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Fence9.h"
#include "libANGLE/renderer/d3d/d3d9/Framebuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/Query9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/StateManager9.h"
#include "libANGLE/renderer/d3d/d3d9/VertexArray9.h"

namespace rx
{

Context9::Context9(const gl::State &state, gl::ErrorSet *errorSet, Renderer9 *renderer)
    : ContextD3D(state, errorSet), mRenderer(renderer)
{}

Context9::~Context9() {}

angle::Result Context9::initialize(const angle::ImageLoadContext &imageLoadContext)
{
    mImageLoadContext = imageLoadContext;
    return angle::Result::Continue;
}

void Context9::onDestroy(const gl::Context *context)
{
    mIncompleteTextures.onDestroy(context);

    mImageLoadContext = {};
}

CompilerImpl *Context9::createCompiler()
{
    return new CompilerD3D(SH_HLSL_3_0_OUTPUT);
}

ShaderImpl *Context9::createShader(const gl::ShaderState &data)
{
    return new ShaderD3D(data, mRenderer);
}

ProgramImpl *Context9::createProgram(const gl::ProgramState &data)
{
    return new ProgramD3D(data, mRenderer);
}

ProgramExecutableImpl *Context9::createProgramExecutable(const gl::ProgramExecutable *executable)
{
    return new ProgramExecutableD3D(executable);
}

FramebufferImpl *Context9::createFramebuffer(const gl::FramebufferState &data)
{
    return new Framebuffer9(data, mRenderer);
}

TextureImpl *Context9::createTexture(const gl::TextureState &state)
{
    switch (state.getType())
    {
        case gl::TextureType::_2D:
        // GL_TEXTURE_VIDEO_IMAGE_WEBGL maps to 2D texture on Windows platform.
        case gl::TextureType::VideoImage:
            return new TextureD3D_2D(state, mRenderer);
        case gl::TextureType::CubeMap:
            return new TextureD3D_Cube(state, mRenderer);
        case gl::TextureType::External:
            return new TextureD3D_External(state, mRenderer);
        default:
            UNREACHABLE();
    }
    return nullptr;
}

RenderbufferImpl *Context9::createRenderbuffer(const gl::RenderbufferState &state)
{
    return new RenderbufferD3D(state, mRenderer);
}

BufferImpl *Context9::createBuffer(const gl::BufferState &state)
{
    return new Buffer9(state, mRenderer);
}

VertexArrayImpl *Context9::createVertexArray(const gl::VertexArrayState &data)
{
    return new VertexArray9(data);
}

QueryImpl *Context9::createQuery(gl::QueryType type)
{
    return new Query9(mRenderer, type);
}

FenceNVImpl *Context9::createFenceNV()
{
    return new FenceNV9(mRenderer);
}

SyncImpl *Context9::createSync()
{
    // D3D9 doesn't support ES 3.0 and its sync objects.
    UNREACHABLE();
    return nullptr;
}

TransformFeedbackImpl *Context9::createTransformFeedback(const gl::TransformFeedbackState &state)
{
    UNREACHABLE();
    return nullptr;
}

SamplerImpl *Context9::createSampler(const gl::SamplerState &state)
{
    return new SamplerD3D(state);
}

ProgramPipelineImpl *Context9::createProgramPipeline(const gl::ProgramPipelineState &data)
{
    UNREACHABLE();
    return nullptr;
}

MemoryObjectImpl *Context9::createMemoryObject()
{
    UNREACHABLE();
    return nullptr;
}

SemaphoreImpl *Context9::createSemaphore()
{
    UNREACHABLE();
    return nullptr;
}

OverlayImpl *Context9::createOverlay(const gl::OverlayState &state)
{
    // Not implemented.
    return new OverlayImpl(state);
}

angle::Result Context9::flush(const gl::Context *context)
{
    return mRenderer->flush(context);
}

angle::Result Context9::finish(const gl::Context *context)
{
    return mRenderer->finish(context);
}

angle::Result Context9::drawArrays(const gl::Context *context,
                                   gl::PrimitiveMode mode,
                                   GLint first,
                                   GLsizei count)
{
    return mRenderer->genericDrawArrays(context, mode, first, count, 0);
}

angle::Result Context9::drawArraysInstanced(const gl::Context *context,
                                            gl::PrimitiveMode mode,
                                            GLint first,
                                            GLsizei count,
                                            GLsizei instanceCount)
{
    return mRenderer->genericDrawArrays(context, mode, first, count, instanceCount);
}

angle::Result Context9::drawArraysInstancedBaseInstance(const gl::Context *context,
                                                        gl::PrimitiveMode mode,
                                                        GLint first,
                                                        GLsizei count,
                                                        GLsizei instanceCount,
                                                        GLuint baseInstance)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Continue;
}

angle::Result Context9::drawElements(const gl::Context *context,
                                     gl::PrimitiveMode mode,
                                     GLsizei count,
                                     gl::DrawElementsType type,
                                     const void *indices)
{
    return mRenderer->genericDrawElements(context, mode, count, type, indices, 0);
}

angle::Result Context9::drawElementsBaseVertex(const gl::Context *context,
                                               gl::PrimitiveMode mode,
                                               GLsizei count,
                                               gl::DrawElementsType type,
                                               const void *indices,
                                               GLint baseVertex)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Continue;
}

angle::Result Context9::drawElementsInstanced(const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              GLsizei count,
                                              gl::DrawElementsType type,
                                              const void *indices,
                                              GLsizei instances)
{
    return mRenderer->genericDrawElements(context, mode, count, type, indices, instances);
}

angle::Result Context9::drawElementsInstancedBaseVertex(const gl::Context *context,
                                                        gl::PrimitiveMode mode,
                                                        GLsizei count,
                                                        gl::DrawElementsType type,
                                                        const void *indices,
                                                        GLsizei instances,
                                                        GLint baseVertex)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Continue;
}

angle::Result Context9::drawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                                    gl::PrimitiveMode mode,
                                                                    GLsizei count,
                                                                    gl::DrawElementsType type,
                                                                    const void *indices,
                                                                    GLsizei instances,
                                                                    GLint baseVertex,
                                                                    GLuint baseInstance)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Continue;
}

angle::Result Context9::drawRangeElements(const gl::Context *context,
                                          gl::PrimitiveMode mode,
                                          GLuint start,
                                          GLuint end,
                                          GLsizei count,
                                          gl::DrawElementsType type,
                                          const void *indices)
{
    return mRenderer->genericDrawElements(context, mode, count, type, indices, 0);
}

angle::Result Context9::drawRangeElementsBaseVertex(const gl::Context *context,
                                                    gl::PrimitiveMode mode,
                                                    GLuint start,
                                                    GLuint end,
                                                    GLsizei count,
                                                    gl::DrawElementsType type,
                                                    const void *indices,
                                                    GLint baseVertex)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Continue;
}

angle::Result Context9::drawArraysIndirect(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           const void *indirect)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

angle::Result Context9::drawElementsIndirect(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             gl::DrawElementsType type,
                                             const void *indirect)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

angle::Result Context9::multiDrawArrays(const gl::Context *context,
                                        gl::PrimitiveMode mode,
                                        const GLint *firsts,
                                        const GLsizei *counts,
                                        GLsizei drawcount)
{
    return rx::MultiDrawArraysGeneral(this, context, mode, firsts, counts, drawcount);
}

angle::Result Context9::multiDrawArraysInstanced(const gl::Context *context,
                                                 gl::PrimitiveMode mode,
                                                 const GLint *firsts,
                                                 const GLsizei *counts,
                                                 const GLsizei *instanceCounts,
                                                 GLsizei drawcount)
{
    return rx::MultiDrawArraysInstancedGeneral(this, context, mode, firsts, counts, instanceCounts,
                                               drawcount);
}

angle::Result Context9::multiDrawArraysIndirect(const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                const void *indirect,
                                                GLsizei drawcount,
                                                GLsizei stride)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result Context9::multiDrawElements(const gl::Context *context,
                                          gl::PrimitiveMode mode,
                                          const GLsizei *counts,
                                          gl::DrawElementsType type,
                                          const GLvoid *const *indices,
                                          GLsizei drawcount)
{
    return rx::MultiDrawElementsGeneral(this, context, mode, counts, type, indices, drawcount);
}

angle::Result Context9::multiDrawElementsInstanced(const gl::Context *context,
                                                   gl::PrimitiveMode mode,
                                                   const GLsizei *counts,
                                                   gl::DrawElementsType type,
                                                   const GLvoid *const *indices,
                                                   const GLsizei *instanceCounts,
                                                   GLsizei drawcount)
{
    return rx::MultiDrawElementsInstancedGeneral(this, context, mode, counts, type, indices,
                                                 instanceCounts, drawcount);
}

angle::Result Context9::multiDrawElementsIndirect(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  gl::DrawElementsType type,
                                                  const void *indirect,
                                                  GLsizei drawcount,
                                                  GLsizei stride)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result Context9::multiDrawArraysInstancedBaseInstance(const gl::Context *context,
                                                             gl::PrimitiveMode mode,
                                                             const GLint *firsts,
                                                             const GLsizei *counts,
                                                             const GLsizei *instanceCounts,
                                                             const GLuint *baseInstances,
                                                             GLsizei drawcount)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

angle::Result Context9::multiDrawElementsInstancedBaseVertexBaseInstance(
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
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

gl::GraphicsResetStatus Context9::getResetStatus()
{
    return mRenderer->getResetStatus();
}

angle::Result Context9::insertEventMarker(GLsizei length, const char *marker)
{
    mRenderer->getAnnotator()->setMarker(/*context=*/nullptr, marker);
    return angle::Result::Continue;
}

angle::Result Context9::pushGroupMarker(GLsizei length, const char *marker)
{
    mRenderer->getAnnotator()->beginEvent(nullptr, angle::EntryPoint::GLPushGroupMarkerEXT, marker,
                                          marker);
    mMarkerStack.push(std::string(marker));
    return angle::Result::Continue;
}

angle::Result Context9::popGroupMarker()
{
    const char *marker = nullptr;
    if (!mMarkerStack.empty())
    {
        marker = mMarkerStack.top().c_str();
        mMarkerStack.pop();
        mRenderer->getAnnotator()->endEvent(nullptr, marker,
                                            angle::EntryPoint::GLPopGroupMarkerEXT);
    }
    return angle::Result::Continue;
}

angle::Result Context9::pushDebugGroup(const gl::Context *context,
                                       GLenum source,
                                       GLuint id,
                                       const std::string &message)
{
    // Fall through to the EXT_debug_marker functions
    return pushGroupMarker(static_cast<GLsizei>(message.size()), message.c_str());
}

angle::Result Context9::popDebugGroup(const gl::Context *context)
{
    // Fall through to the EXT_debug_marker functions
    return popGroupMarker();
}

angle::Result Context9::syncState(const gl::Context *context,
                                  const gl::state::DirtyBits dirtyBits,
                                  const gl::state::DirtyBits bitMask,
                                  const gl::state::ExtendedDirtyBits extendedDirtyBits,
                                  const gl::state::ExtendedDirtyBits extendedBitMask,
                                  gl::Command command)
{
    mRenderer->getStateManager()->syncState(mState, dirtyBits, extendedDirtyBits);
    return angle::Result::Continue;
}

GLint Context9::getGPUDisjoint()
{
    // Disjoint timer queries are not supported.
    return false;
}

GLint64 Context9::getTimestamp()
{
    return mRenderer->getTimestamp();
}

angle::Result Context9::onMakeCurrent(const gl::Context *context)
{
    mRenderer->getStateManager()->setAllDirtyBits();
    return mRenderer->ensureVertexDataManagerInitialized(context);
}

gl::Caps Context9::getNativeCaps() const
{
    return mRenderer->getNativeCaps();
}

const gl::TextureCapsMap &Context9::getNativeTextureCaps() const
{
    return mRenderer->getNativeTextureCaps();
}

const gl::Extensions &Context9::getNativeExtensions() const
{
    return mRenderer->getNativeExtensions();
}

const gl::Limitations &Context9::getNativeLimitations() const
{
    return mRenderer->getNativeLimitations();
}

const ShPixelLocalStorageOptions &Context9::getNativePixelLocalStorageOptions() const
{
    return mRenderer->getNativePixelLocalStorageOptions();
}

angle::Result Context9::dispatchCompute(const gl::Context *context,
                                        GLuint numGroupsX,
                                        GLuint numGroupsY,
                                        GLuint numGroupsZ)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

angle::Result Context9::dispatchComputeIndirect(const gl::Context *context, GLintptr indirect)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

angle::Result Context9::memoryBarrier(const gl::Context *context, GLbitfield barriers)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

angle::Result Context9::memoryBarrierByRegion(const gl::Context *context, GLbitfield barriers)
{
    ANGLE_HR_UNREACHABLE(this);
    return angle::Result::Stop;
}

angle::Result Context9::getIncompleteTexture(const gl::Context *context,
                                             gl::TextureType type,
                                             gl::Texture **textureOut)
{
    return mIncompleteTextures.getIncompleteTexture(context, type, gl::SamplerFormat::Float,
                                                    nullptr, textureOut);
}

void Context9::handleResult(HRESULT hr,
                            const char *message,
                            const char *file,
                            const char *function,
                            unsigned int line)
{
    ASSERT(FAILED(hr));

    if (d3d9::isDeviceLostError(hr))
    {
        mRenderer->notifyDeviceLost();
    }

    GLenum glErrorCode = DefaultGLErrorCode(hr);

    std::stringstream errorStream;
    errorStream << "Internal D3D9 error: " << gl::FmtHR(hr) << ": " << message;

    mErrors->handleError(glErrorCode, errorStream.str().c_str(), file, function, line);
}
}  // namespace rx
