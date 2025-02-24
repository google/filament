//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextGL:
//   OpenGL-specific functionality associated with a GL Context.
//

#include "libANGLE/renderer/gl/ContextGL.h"

#include "libANGLE/Context.h"
#include "libANGLE/Context.inl.h"
#include "libANGLE/PixelLocalStorage.h"
#include "libANGLE/renderer/OverlayImpl.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/CompilerGL.h"
#include "libANGLE/renderer/gl/FenceNVGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/MemoryObjectGL.h"
#include "libANGLE/renderer/gl/ProgramExecutableGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/ProgramPipelineGL.h"
#include "libANGLE/renderer/gl/QueryGL.h"
#include "libANGLE/renderer/gl/RenderbufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/SamplerGL.h"
#include "libANGLE/renderer/gl/SemaphoreGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/SyncGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/TransformFeedbackGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"

namespace rx
{
namespace
{
GLsizei GetDrawAdjustedInstanceCount(const gl::ProgramExecutable *executable)
{
    const bool usesMultiview = executable->usesMultiview();
    return usesMultiview ? executable->getNumViews() : 0;
}

GLsizei GetInstancedDrawAdjustedInstanceCount(const gl::ProgramExecutable *executable,
                                              GLsizei instanceCount)
{
    GLsizei adjustedInstanceCount = instanceCount;
    if (executable->usesMultiview())
    {
        adjustedInstanceCount *= executable->getNumViews();
    }
    return adjustedInstanceCount;
}
}  // anonymous namespace

ContextGL::ContextGL(const gl::State &state,
                     gl::ErrorSet *errorSet,
                     const std::shared_ptr<RendererGL> &renderer,
                     RobustnessVideoMemoryPurgeStatus robustnessVideoMemoryPurgeStatus)
    : ContextImpl(state, errorSet),
      mRenderer(renderer),
      mRobustnessVideoMemoryPurgeStatus(robustnessVideoMemoryPurgeStatus)
{}

ContextGL::~ContextGL() {}

angle::Result ContextGL::initialize(const angle::ImageLoadContext &imageLoadContext)
{
    return angle::Result::Continue;
}

CompilerImpl *ContextGL::createCompiler()
{
    return new CompilerGL(this);
}

ShaderImpl *ContextGL::createShader(const gl::ShaderState &data)
{
    const FunctionsGL *functions = getFunctions();
    GLuint shader                = functions->createShader(ToGLenum(data.getShaderType()));

    return new ShaderGL(data, shader);
}

ProgramImpl *ContextGL::createProgram(const gl::ProgramState &data)
{
    return new ProgramGL(data, getFunctions(), getFeaturesGL(), getStateManager(), mRenderer);
}

ProgramExecutableImpl *ContextGL::createProgramExecutable(const gl::ProgramExecutable *executable)
{
    return new ProgramExecutableGL(executable);
}

FramebufferImpl *ContextGL::createFramebuffer(const gl::FramebufferState &data)
{
    const FunctionsGL *funcs = getFunctions();

    GLuint fbo = 0;
    if (!data.isDefault())
    {
        funcs->genFramebuffers(1, &fbo);
    }

    return new FramebufferGL(data, fbo, false);
}

TextureImpl *ContextGL::createTexture(const gl::TextureState &state)
{
    const FunctionsGL *functions = getFunctions();
    StateManagerGL *stateManager = getStateManager();

    GLuint texture = 0;
    functions->genTextures(1, &texture);
    stateManager->bindTexture(state.getType(), texture);

    return new TextureGL(state, texture);
}

RenderbufferImpl *ContextGL::createRenderbuffer(const gl::RenderbufferState &state)
{
    const FunctionsGL *functions = getFunctions();
    StateManagerGL *stateManager = getStateManager();

    GLuint renderbuffer = 0;
    functions->genRenderbuffers(1, &renderbuffer);
    stateManager->bindRenderbuffer(GL_RENDERBUFFER, renderbuffer);

    return new RenderbufferGL(state, renderbuffer);
}

BufferImpl *ContextGL::createBuffer(const gl::BufferState &state)
{
    const FunctionsGL *functions = getFunctions();

    GLuint buffer = 0;
    functions->genBuffers(1, &buffer);

    return new BufferGL(state, buffer);
}

VertexArrayImpl *ContextGL::createVertexArray(const gl::VertexArrayState &data)
{
    const FunctionsGL *functions      = getFunctions();
    const angle::FeaturesGL &features = getFeaturesGL();

    // Use the shared default vertex array when forced to for workarounds
    // (syncAllVertexArraysToDefault) or for the frontend default vertex array so that client data
    // can be used directly
    // Disable on external contexts so that the default VAO is not modified by both ANGLE and the
    // external user.
    if (features.syncAllVertexArraysToDefault.enabled ||
        (features.syncDefaultVertexArraysToDefault.enabled && data.isDefault() &&
         mState.areClientArraysEnabled()))
    {
        StateManagerGL *stateManager = getStateManager();

        return new VertexArrayGL(data, stateManager->getDefaultVAO(),
                                 stateManager->getDefaultVAOState());
    }
    else
    {
        GLuint vao = 0;
        functions->genVertexArrays(1, &vao);
        return new VertexArrayGL(data, vao);
    }
}

QueryImpl *ContextGL::createQuery(gl::QueryType type)
{
    switch (type)
    {
        case gl::QueryType::CommandsCompleted:
            return new SyncQueryGL(type, getFunctions());

        default:
            return new StandardQueryGL(type, getFunctions(), getStateManager());
    }
}

FenceNVImpl *ContextGL::createFenceNV()
{
    const FunctionsGL *functions = getFunctions();
    if (FenceNVGL::Supported(functions))
    {
        return new FenceNVGL(functions);
    }
    else
    {
        ASSERT(FenceNVSyncGL::Supported(functions));
        return new FenceNVSyncGL(functions);
    }
}

SyncImpl *ContextGL::createSync()
{
    return new SyncGL(getFunctions());
}

TransformFeedbackImpl *ContextGL::createTransformFeedback(const gl::TransformFeedbackState &state)
{
    return new TransformFeedbackGL(state, getFunctions(), getStateManager());
}

SamplerImpl *ContextGL::createSampler(const gl::SamplerState &state)
{
    return new SamplerGL(state, getFunctions(), getStateManager());
}

ProgramPipelineImpl *ContextGL::createProgramPipeline(const gl::ProgramPipelineState &data)
{
    return new ProgramPipelineGL(data, getFunctions());
}

MemoryObjectImpl *ContextGL::createMemoryObject()
{
    const FunctionsGL *functions = getFunctions();

    GLuint memoryObject = 0;
    functions->createMemoryObjectsEXT(1, &memoryObject);

    return new MemoryObjectGL(memoryObject);
}

SemaphoreImpl *ContextGL::createSemaphore()
{
    const FunctionsGL *functions = getFunctions();

    GLuint semaphore = 0;
    functions->genSemaphoresEXT(1, &semaphore);

    return new SemaphoreGL(semaphore);
}

OverlayImpl *ContextGL::createOverlay(const gl::OverlayState &state)
{
    // Not implemented.
    return new OverlayImpl(state);
}

angle::Result ContextGL::flush(const gl::Context *context)
{
    return mRenderer->flush();
}

angle::Result ContextGL::finish(const gl::Context *context)
{
    return mRenderer->finish();
}

ANGLE_INLINE angle::Result ContextGL::setDrawArraysState(const gl::Context *context,
                                                         GLint first,
                                                         GLsizei count,
                                                         GLsizei instanceCount)
{
    const angle::FeaturesGL &features = getFeaturesGL();
    if (context->getStateCache().hasAnyActiveClientAttrib() ||
        (features.shiftInstancedArrayDataWithOffset.enabled && first > 0))
    {
        const gl::State &glState                = context->getState();
        const gl::ProgramExecutable *executable = getState().getProgramExecutable();
        const gl::VertexArray *vao              = glState.getVertexArray();
        const VertexArrayGL *vaoGL              = GetImplAs<VertexArrayGL>(vao);

        ANGLE_TRY(vaoGL->syncClientSideData(context, executable->getActiveAttribLocationsMask(),
                                            first, count, instanceCount));

#if defined(ANGLE_STATE_VALIDATION_ENABLED)
        ANGLE_TRY(vaoGL->validateState(context));
#endif  // ANGLE_STATE_VALIDATION_ENABLED
    }
    else if (features.shiftInstancedArrayDataWithOffset.enabled && first == 0)
    {
        // There could be previous draw call that has modified the attributes
        // Instead of forcefully streaming attributes, we just rebind the original ones
        const gl::State &glState   = context->getState();
        const gl::VertexArray *vao = glState.getVertexArray();
        const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
        ANGLE_TRY(vaoGL->recoverForcedStreamingAttributesForDrawArraysInstanced(context));
    }

    if (features.setPrimitiveRestartFixedIndexForDrawArrays.enabled)
    {
        StateManagerGL *stateManager           = getStateManager();
        constexpr GLuint primitiveRestartIndex = gl::GetPrimitiveRestartIndexFromType<GLuint>();
        ANGLE_TRY(stateManager->setPrimitiveRestartIndex(context, primitiveRestartIndex));
    }

    return angle::Result::Continue;
}

ANGLE_INLINE angle::Result ContextGL::setDrawElementsState(const gl::Context *context,
                                                           GLsizei count,
                                                           gl::DrawElementsType type,
                                                           const void *indices,
                                                           GLsizei instanceCount,
                                                           const void **outIndices)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = getState().getProgramExecutable();
    const gl::VertexArray *vao              = glState.getVertexArray();
    const gl::StateCache &stateCache        = context->getStateCache();

    const angle::FeaturesGL &features = getFeaturesGL();
    if (features.shiftInstancedArrayDataWithOffset.enabled)
    {
        // There might be instanced arrays that are forced streaming for drawArraysInstanced
        // They cannot be ELEMENT_ARRAY_BUFFER
        const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
        ANGLE_TRY(vaoGL->recoverForcedStreamingAttributesForDrawArraysInstanced(context));
    }

    if (stateCache.hasAnyActiveClientAttrib() || vao->getElementArrayBuffer() == nullptr)
    {
        const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
        ANGLE_TRY(vaoGL->syncDrawElementsState(context, executable->getActiveAttribLocationsMask(),
                                               count, type, indices, instanceCount,
                                               glState.isPrimitiveRestartEnabled(), outIndices));
    }
    else
    {
        *outIndices = indices;
    }

    if (glState.isPrimitiveRestartEnabled() && features.emulatePrimitiveRestartFixedIndex.enabled)
    {
        StateManagerGL *stateManager = getStateManager();

        GLuint primitiveRestartIndex = gl::GetPrimitiveRestartIndex(type);
        ANGLE_TRY(stateManager->setPrimitiveRestartIndex(context, primitiveRestartIndex));
    }

#if defined(ANGLE_STATE_VALIDATION_ENABLED)
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
    ANGLE_TRY(vaoGL->validateState(context));
#endif  // ANGLE_STATE_VALIDATION_ENABLED

    return angle::Result::Continue;
}

angle::Result ContextGL::drawArrays(const gl::Context *context,
                                    gl::PrimitiveMode mode,
                                    GLint first,
                                    GLsizei count)
{
    const gl::ProgramExecutable *executable = context->getState().getProgramExecutable();
    const GLsizei instanceCount             = GetDrawAdjustedInstanceCount(executable);

#if defined(ANGLE_STATE_VALIDATION_ENABLED)
    validateState();
#endif

    ANGLE_TRY(setDrawArraysState(context, first, count, instanceCount));
    if (!executable->usesMultiview())
    {
        ANGLE_GL_TRY(context, getFunctions()->drawArrays(ToGLenum(mode), first, count));
    }
    else
    {
        ANGLE_GL_TRY(context, getFunctions()->drawArraysInstanced(ToGLenum(mode), first, count,
                                                                  instanceCount));
    }
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawArraysInstanced(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             GLint first,
                                             GLsizei count,
                                             GLsizei instanceCount)
{
    const gl::ProgramExecutable *executable = context->getState().getProgramExecutable();
    const GLsizei adjustedInstanceCount =
        GetInstancedDrawAdjustedInstanceCount(executable, instanceCount);

    ANGLE_TRY(setDrawArraysState(context, first, count, adjustedInstanceCount));
    ANGLE_GL_TRY(context, getFunctions()->drawArraysInstanced(ToGLenum(mode), first, count,
                                                              adjustedInstanceCount));
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

gl::AttributesMask ContextGL::updateAttributesForBaseInstance(GLuint baseInstance)
{
    const gl::ProgramExecutable *executable = getState().getProgramExecutable();
    gl::AttributesMask attribToUpdateMask;

    if (baseInstance != 0)
    {
        const FunctionsGL *functions = getFunctions();
        const auto &attribs          = mState.getVertexArray()->getVertexAttributes();
        const auto &bindings         = mState.getVertexArray()->getVertexBindings();
        for (GLuint attribIndex = 0; attribIndex < attribs.size(); attribIndex++)
        {
            const gl::VertexAttribute &attrib = attribs[attribIndex];
            const gl::VertexBinding &binding  = bindings[attrib.bindingIndex];
            if (executable->isAttribLocationActive(attribIndex) && binding.getDivisor() != 0)
            {
                attribToUpdateMask.set(attribIndex);
                const char *p             = static_cast<const char *>(attrib.pointer);
                const size_t sourceStride = gl::ComputeVertexAttributeStride(attrib, binding);
                const void *newPointer    = p + sourceStride * baseInstance;

                const BufferGL *buffer = GetImplAs<BufferGL>(binding.getBuffer().get());
                // We often stream data from scratch buffers when client side data is being used
                // and that information is in VertexArrayGL.
                // Assert that the buffer is non-null because this case isn't handled.
                ASSERT(buffer);
                getStateManager()->bindBuffer(gl::BufferBinding::Array, buffer->getBufferID());
                if (attrib.format->isPureInt())
                {
                    functions->vertexAttribIPointer(attribIndex, attrib.format->channelCount,
                                                    gl::ToGLenum(attrib.format->vertexAttribType),
                                                    attrib.vertexAttribArrayStride, newPointer);
                }
                else
                {
                    functions->vertexAttribPointer(attribIndex, attrib.format->channelCount,
                                                   gl::ToGLenum(attrib.format->vertexAttribType),
                                                   attrib.format->isNorm(),
                                                   attrib.vertexAttribArrayStride, newPointer);
                }
            }
        }
    }

    return attribToUpdateMask;
}

void ContextGL::resetUpdatedAttributes(gl::AttributesMask attribMask)
{
    const FunctionsGL *functions = getFunctions();
    for (size_t attribIndex : attribMask)
    {
        const gl::VertexAttribute &attrib =
            mState.getVertexArray()->getVertexAttributes()[attribIndex];
        const gl::VertexBinding &binding =
            (mState.getVertexArray()->getVertexBindings())[attrib.bindingIndex];
        getStateManager()->bindBuffer(
            gl::BufferBinding::Array,
            GetImplAs<BufferGL>(binding.getBuffer().get())->getBufferID());
        if (attrib.format->isPureInt())
        {
            functions->vertexAttribIPointer(static_cast<GLuint>(attribIndex),
                                            attrib.format->channelCount,
                                            gl::ToGLenum(attrib.format->vertexAttribType),
                                            attrib.vertexAttribArrayStride, attrib.pointer);
        }
        else
        {
            functions->vertexAttribPointer(
                static_cast<GLuint>(attribIndex), attrib.format->channelCount,
                gl::ToGLenum(attrib.format->vertexAttribType), attrib.format->isNorm(),
                attrib.vertexAttribArrayStride, attrib.pointer);
        }
    }
}

angle::Result ContextGL::drawArraysInstancedBaseInstance(const gl::Context *context,
                                                         gl::PrimitiveMode mode,
                                                         GLint first,
                                                         GLsizei count,
                                                         GLsizei instanceCount,
                                                         GLuint baseInstance)
{
    const gl::ProgramExecutable *executable = context->getState().getProgramExecutable();
    const GLsizei adjustedInstanceCount =
        GetInstancedDrawAdjustedInstanceCount(executable, instanceCount);

    ANGLE_TRY(setDrawArraysState(context, first, count, adjustedInstanceCount));

    const FunctionsGL *functions = getFunctions();

    if (functions->drawArraysInstancedBaseInstance)
    {
        // GL 4.2+ or GL_EXT_base_instance
        ANGLE_GL_TRY(context,
                     functions->drawArraysInstancedBaseInstance(
                         ToGLenum(mode), first, count, adjustedInstanceCount, baseInstance));
    }
    else
    {
        // GL 3.3+ or GLES 3.2+
        // TODO(http://anglebug.com/42262554): This is a temporary solution by setting and resetting
        // pointer offset calling vertexAttribPointer Will refactor stateCache and pass baseInstance
        // to setDrawArraysState to set pointer offset

        gl::AttributesMask attribToResetMask = updateAttributesForBaseInstance(baseInstance);

        ANGLE_GL_TRY(context, functions->drawArraysInstanced(ToGLenum(mode), first, count,
                                                             adjustedInstanceCount));

        resetUpdatedAttributes(attribToResetMask);
    }

    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawElements(const gl::Context *context,
                                      gl::PrimitiveMode mode,
                                      GLsizei count,
                                      gl::DrawElementsType type,
                                      const void *indices)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    const GLsizei instanceCount             = GetDrawAdjustedInstanceCount(executable);
    const void *drawIndexPtr                = nullptr;

#if defined(ANGLE_STATE_VALIDATION_ENABLED)
    validateState();
#endif  // ANGLE_STATE_VALIDATION_ENABLED

    ANGLE_TRY(setDrawElementsState(context, count, type, indices, instanceCount, &drawIndexPtr));
    if (!executable->usesMultiview())
    {
        ANGLE_GL_TRY(context, getFunctions()->drawElements(ToGLenum(mode), count, ToGLenum(type),
                                                           drawIndexPtr));
    }
    else
    {
        ANGLE_GL_TRY(context,
                     getFunctions()->drawElementsInstanced(ToGLenum(mode), count, ToGLenum(type),
                                                           drawIndexPtr, instanceCount));
    }
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawElementsBaseVertex(const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                GLsizei count,
                                                gl::DrawElementsType type,
                                                const void *indices,
                                                GLint baseVertex)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    const GLsizei instanceCount             = GetDrawAdjustedInstanceCount(executable);
    const void *drawIndexPtr                = nullptr;

#if defined(ANGLE_STATE_VALIDATION_ENABLED)
    validateState();
#endif  // ANGLE_STATE_VALIDATION_ENABLED

    ANGLE_TRY(setDrawElementsState(context, count, type, indices, instanceCount, &drawIndexPtr));
    if (!executable->usesMultiview())
    {
        ANGLE_GL_TRY(context, getFunctions()->drawElementsBaseVertex(
                                  ToGLenum(mode), count, ToGLenum(type), drawIndexPtr, baseVertex));
    }
    else
    {
        ANGLE_GL_TRY(context, getFunctions()->drawElementsInstancedBaseVertex(
                                  ToGLenum(mode), count, ToGLenum(type), drawIndexPtr,
                                  instanceCount, baseVertex));
    }
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawElementsInstanced(const gl::Context *context,
                                               gl::PrimitiveMode mode,
                                               GLsizei count,
                                               gl::DrawElementsType type,
                                               const void *indices,
                                               GLsizei instances)
{
    const gl::ProgramExecutable *executable = context->getState().getProgramExecutable();
    const GLsizei adjustedInstanceCount =
        GetInstancedDrawAdjustedInstanceCount(executable, instances);
    const void *drawIndexPointer = nullptr;

    ANGLE_TRY(setDrawElementsState(context, count, type, indices, adjustedInstanceCount,
                                   &drawIndexPointer));
    ANGLE_GL_TRY(context,
                 getFunctions()->drawElementsInstanced(ToGLenum(mode), count, ToGLenum(type),
                                                       drawIndexPointer, adjustedInstanceCount));
    return angle::Result::Continue;
}

angle::Result ContextGL::drawElementsInstancedBaseVertex(const gl::Context *context,
                                                         gl::PrimitiveMode mode,
                                                         GLsizei count,
                                                         gl::DrawElementsType type,
                                                         const void *indices,
                                                         GLsizei instances,
                                                         GLint baseVertex)
{
    const gl::ProgramExecutable *executable = context->getState().getProgramExecutable();
    const GLsizei adjustedInstanceCount =
        GetInstancedDrawAdjustedInstanceCount(executable, instances);
    const void *drawIndexPointer = nullptr;

    ANGLE_TRY(setDrawElementsState(context, count, type, indices, adjustedInstanceCount,
                                   &drawIndexPointer));
    ANGLE_GL_TRY(context, getFunctions()->drawElementsInstancedBaseVertex(
                              ToGLenum(mode), count, ToGLenum(type), drawIndexPointer,
                              adjustedInstanceCount, baseVertex));
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                                     gl::PrimitiveMode mode,
                                                                     GLsizei count,
                                                                     gl::DrawElementsType type,
                                                                     const void *indices,
                                                                     GLsizei instances,
                                                                     GLint baseVertex,
                                                                     GLuint baseInstance)
{
    const gl::ProgramExecutable *executable = context->getState().getProgramExecutable();
    const GLsizei adjustedInstanceCount =
        GetInstancedDrawAdjustedInstanceCount(executable, instances);
    const void *drawIndexPointer = nullptr;

    ANGLE_TRY(setDrawElementsState(context, count, type, indices, adjustedInstanceCount,
                                   &drawIndexPointer));

    const FunctionsGL *functions = getFunctions();

    if (functions->drawElementsInstancedBaseVertexBaseInstance)
    {
        // GL 4.2+ or GL_EXT_base_instance
        ANGLE_GL_TRY(context, functions->drawElementsInstancedBaseVertexBaseInstance(
                                  ToGLenum(mode), count, ToGLenum(type), drawIndexPointer,
                                  adjustedInstanceCount, baseVertex, baseInstance));
    }
    else
    {
        // GL 3.3+ or GLES 3.2+
        // TODO(http://anglebug.com/42262554): same as above
        gl::AttributesMask attribToResetMask = updateAttributesForBaseInstance(baseInstance);

        ANGLE_GL_TRY(context, functions->drawElementsInstancedBaseVertex(
                                  ToGLenum(mode), count, ToGLenum(type), drawIndexPointer,
                                  adjustedInstanceCount, baseVertex));

        resetUpdatedAttributes(attribToResetMask);
    }

    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawRangeElements(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           GLuint start,
                                           GLuint end,
                                           GLsizei count,
                                           gl::DrawElementsType type,
                                           const void *indices)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    const GLsizei instanceCount             = GetDrawAdjustedInstanceCount(executable);
    const void *drawIndexPointer            = nullptr;

    ANGLE_TRY(
        setDrawElementsState(context, count, type, indices, instanceCount, &drawIndexPointer));
    if (!executable->usesMultiview())
    {
        ANGLE_GL_TRY(context, getFunctions()->drawRangeElements(ToGLenum(mode), start, end, count,
                                                                ToGLenum(type), drawIndexPointer));
    }
    else
    {
        ANGLE_GL_TRY(context,
                     getFunctions()->drawElementsInstanced(ToGLenum(mode), count, ToGLenum(type),
                                                           drawIndexPointer, instanceCount));
    }
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawRangeElementsBaseVertex(const gl::Context *context,
                                                     gl::PrimitiveMode mode,
                                                     GLuint start,
                                                     GLuint end,
                                                     GLsizei count,
                                                     gl::DrawElementsType type,
                                                     const void *indices,
                                                     GLint baseVertex)
{
    const gl::ProgramExecutable *executable = mState.getProgramExecutable();
    const GLsizei instanceCount             = GetDrawAdjustedInstanceCount(executable);
    const void *drawIndexPointer            = nullptr;

    ANGLE_TRY(
        setDrawElementsState(context, count, type, indices, instanceCount, &drawIndexPointer));
    if (!executable->usesMultiview())
    {
        ANGLE_GL_TRY(context, getFunctions()->drawRangeElementsBaseVertex(
                                  ToGLenum(mode), start, end, count, ToGLenum(type),
                                  drawIndexPointer, baseVertex));
    }
    else
    {
        ANGLE_GL_TRY(context, getFunctions()->drawElementsInstancedBaseVertex(
                                  ToGLenum(mode), count, ToGLenum(type), drawIndexPointer,
                                  instanceCount, baseVertex));
    }
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawArraysIndirect(const gl::Context *context,
                                            gl::PrimitiveMode mode,
                                            const void *indirect)
{
    ANGLE_GL_TRY(context, getFunctions()->drawArraysIndirect(ToGLenum(mode), indirect));
    mRenderer->markWorkSubmitted();

    return angle::Result::Continue;
}

angle::Result ContextGL::drawElementsIndirect(const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              gl::DrawElementsType type,
                                              const void *indirect)
{
    ANGLE_GL_TRY(context,
                 getFunctions()->drawElementsIndirect(ToGLenum(mode), ToGLenum(type), indirect));
    return angle::Result::Continue;
}

angle::Result ContextGL::multiDrawArrays(const gl::Context *context,
                                         gl::PrimitiveMode mode,
                                         const GLint *firsts,
                                         const GLsizei *counts,
                                         GLsizei drawcount)
{
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawArraysGeneral(this, context, mode, firsts, counts, drawcount);
}

angle::Result ContextGL::multiDrawArraysInstanced(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  const GLint *firsts,
                                                  const GLsizei *counts,
                                                  const GLsizei *instanceCounts,
                                                  GLsizei drawcount)
{
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawArraysInstancedGeneral(this, context, mode, firsts, counts, instanceCounts,
                                               drawcount);
}

angle::Result ContextGL::multiDrawArraysIndirect(const gl::Context *context,
                                                 gl::PrimitiveMode mode,
                                                 const void *indirect,
                                                 GLsizei drawcount,
                                                 GLsizei stride)
{
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawArraysIndirectGeneral(this, context, mode, indirect, drawcount, stride);
}

angle::Result ContextGL::multiDrawElements(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           const GLsizei *counts,
                                           gl::DrawElementsType type,
                                           const GLvoid *const *indices,
                                           GLsizei drawcount)
{
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawElementsGeneral(this, context, mode, counts, type, indices, drawcount);
}

angle::Result ContextGL::multiDrawElementsInstanced(const gl::Context *context,
                                                    gl::PrimitiveMode mode,
                                                    const GLsizei *counts,
                                                    gl::DrawElementsType type,
                                                    const GLvoid *const *indices,
                                                    const GLsizei *instanceCounts,
                                                    GLsizei drawcount)
{
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawElementsInstancedGeneral(this, context, mode, counts, type, indices,
                                                 instanceCounts, drawcount);
}

angle::Result ContextGL::multiDrawElementsIndirect(const gl::Context *context,
                                                   gl::PrimitiveMode mode,
                                                   gl::DrawElementsType type,
                                                   const void *indirect,
                                                   GLsizei drawcount,
                                                   GLsizei stride)
{
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawElementsIndirectGeneral(this, context, mode, type, indirect, drawcount,
                                                stride);
}

angle::Result ContextGL::multiDrawArraysInstancedBaseInstance(const gl::Context *context,
                                                              gl::PrimitiveMode mode,
                                                              const GLint *firsts,
                                                              const GLsizei *counts,
                                                              const GLsizei *instanceCounts,
                                                              const GLuint *baseInstances,
                                                              GLsizei drawcount)
{
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawArraysInstancedBaseInstanceGeneral(
        this, context, mode, firsts, counts, instanceCounts, baseInstances, drawcount);
}

angle::Result ContextGL::multiDrawElementsInstancedBaseVertexBaseInstance(
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
    mRenderer->markWorkSubmitted();

    return rx::MultiDrawElementsInstancedBaseVertexBaseInstanceGeneral(
        this, context, mode, counts, type, indices, instanceCounts, baseVertices, baseInstances,
        drawcount);
}

gl::GraphicsResetStatus ContextGL::getResetStatus()
{
    gl::GraphicsResetStatus resetStatus = mRenderer->getResetStatus();
    if (resetStatus == gl::GraphicsResetStatus::PurgedContextResetNV)
    {
        if (mRobustnessVideoMemoryPurgeStatus == RobustnessVideoMemoryPurgeStatus::NOT_REQUESTED)
        {
            resetStatus = gl::GraphicsResetStatus::UnknownContextReset;
        }
    }
    return resetStatus;
}

angle::Result ContextGL::insertEventMarker(GLsizei length, const char *marker)
{
    mRenderer->insertEventMarker(length, marker);
    return angle::Result::Continue;
}

angle::Result ContextGL::pushGroupMarker(GLsizei length, const char *marker)
{
    mRenderer->pushGroupMarker(length, marker);
    return angle::Result::Continue;
}

angle::Result ContextGL::popGroupMarker()
{
    mRenderer->popGroupMarker();
    return angle::Result::Continue;
}

angle::Result ContextGL::pushDebugGroup(const gl::Context *context,
                                        GLenum source,
                                        GLuint id,
                                        const std::string &message)
{
    mRenderer->pushDebugGroup(source, id, message);
    return angle::Result::Continue;
}

angle::Result ContextGL::popDebugGroup(const gl::Context *context)
{
    mRenderer->popDebugGroup();
    return angle::Result::Continue;
}

angle::Result ContextGL::syncState(const gl::Context *context,
                                   const gl::state::DirtyBits dirtyBits,
                                   const gl::state::DirtyBits bitMask,
                                   const gl::state::ExtendedDirtyBits extendedDirtyBits,
                                   const gl::state::ExtendedDirtyBits extendedBitMask,
                                   gl::Command command)
{
    return mRenderer->getStateManager()->syncState(context, dirtyBits, bitMask, extendedDirtyBits,
                                                   extendedBitMask);
}

GLint ContextGL::getGPUDisjoint()
{
    return mRenderer->getGPUDisjoint();
}

GLint64 ContextGL::getTimestamp()
{
    return mRenderer->getTimestamp();
}

angle::Result ContextGL::onMakeCurrent(const gl::Context *context)
{
    // Queries need to be paused/resumed on context switches
    return mRenderer->getStateManager()->onMakeCurrent(context);
}

angle::Result ContextGL::onUnMakeCurrent(const gl::Context *context)
{
    ANGLE_TRY(flush(context));
    if (getFeaturesGL().unbindFBOBeforeSwitchingContext.enabled)
    {
        mRenderer->getStateManager()->bindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    return ContextImpl::onUnMakeCurrent(context);
}

gl::Caps ContextGL::getNativeCaps() const
{
    return mRenderer->getNativeCaps();
}

const gl::TextureCapsMap &ContextGL::getNativeTextureCaps() const
{
    return mRenderer->getNativeTextureCaps();
}

const gl::Extensions &ContextGL::getNativeExtensions() const
{
    return mRenderer->getNativeExtensions();
}

const gl::Limitations &ContextGL::getNativeLimitations() const
{
    return mRenderer->getNativeLimitations();
}

const ShPixelLocalStorageOptions &ContextGL::getNativePixelLocalStorageOptions() const
{
    return mRenderer->getNativePixelLocalStorageOptions();
}

StateManagerGL *ContextGL::getStateManager()
{
    return mRenderer->getStateManager();
}

const angle::FeaturesGL &ContextGL::getFeaturesGL() const
{
    return mRenderer->getFeatures();
}

BlitGL *ContextGL::getBlitter() const
{
    return mRenderer->getBlitter();
}

ClearMultiviewGL *ContextGL::getMultiviewClearer() const
{
    return mRenderer->getMultiviewClearer();
}

angle::Result ContextGL::dispatchCompute(const gl::Context *context,
                                         GLuint numGroupsX,
                                         GLuint numGroupsY,
                                         GLuint numGroupsZ)
{
    return mRenderer->dispatchCompute(context, numGroupsX, numGroupsY, numGroupsZ);
}

angle::Result ContextGL::dispatchComputeIndirect(const gl::Context *context, GLintptr indirect)
{
    return mRenderer->dispatchComputeIndirect(context, indirect);
}

angle::Result ContextGL::memoryBarrier(const gl::Context *context, GLbitfield barriers)
{
    return mRenderer->memoryBarrier(barriers);
}
angle::Result ContextGL::memoryBarrierByRegion(const gl::Context *context, GLbitfield barriers)
{
    return mRenderer->memoryBarrierByRegion(barriers);
}

void ContextGL::framebufferFetchBarrier()
{
    mRenderer->framebufferFetchBarrier();
}

angle::Result ContextGL::startTiling(const gl::Context *context,
                                     const gl::Rectangle &area,
                                     GLbitfield preserveMask)
{
    const FunctionsGL *functions = getFunctions();
    ANGLE_GL_TRY(context,
                 functions->startTilingQCOM(area.x, area.y, area.width, area.height, preserveMask));
    return angle::Result::Continue;
}

angle::Result ContextGL::endTiling(const gl::Context *context, GLbitfield preserveMask)
{
    const FunctionsGL *functions = getFunctions();
    ANGLE_GL_TRY(context, functions->endTilingQCOM(preserveMask));
    return angle::Result::Continue;
}

void ContextGL::setMaxShaderCompilerThreads(GLuint count)
{
    mRenderer->setMaxShaderCompilerThreads(count);
}

void ContextGL::invalidateTexture(gl::TextureType target)
{
    mRenderer->getStateManager()->invalidateTexture(target);
}

void ContextGL::validateState() const
{
    const StateManagerGL *stateManager = mRenderer->getStateManager();
    stateManager->validateState();
}

void ContextGL::setNeedsFlushBeforeDeleteTextures()
{
    mRenderer->setNeedsFlushBeforeDeleteTextures();
}

void ContextGL::flushIfNecessaryBeforeDeleteTextures()
{
    mRenderer->flushIfNecessaryBeforeDeleteTextures();
}

void ContextGL::markWorkSubmitted()
{
    mRenderer->markWorkSubmitted();
}

MultiviewImplementationTypeGL ContextGL::getMultiviewImplementationType() const
{
    return mRenderer->getMultiviewImplementationType();
}

bool ContextGL::hasNativeParallelCompile()
{
    return mRenderer->hasNativeParallelCompile();
}

}  // namespace rx
