//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Context11:
//   D3D11-specific functionality associated with a GL Context.
//

#include "libANGLE/renderer/d3d/d3d11/Context11.h"

#include "common/entry_points_enum_autogen.h"
#include "common/string_utils.h"
#include "image_util/loadimage.h"
#include "libANGLE/Context.h"
#include "libANGLE/Context.inl.h"
#include "libANGLE/MemoryProgramCache.h"
#include "libANGLE/renderer/OverlayImpl.h"
#include "libANGLE/renderer/d3d/CompilerD3D.h"
#include "libANGLE/renderer/d3d/ProgramExecutableD3D.h"
#include "libANGLE/renderer/d3d/RenderbufferD3D.h"
#include "libANGLE/renderer/d3d/SamplerD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Fence11.h"
#include "libANGLE/renderer/d3d/d3d11/Framebuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/IndexBuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Program11.h"
#include "libANGLE/renderer/d3d/d3d11/ProgramPipeline11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/StateManager11.h"
#include "libANGLE/renderer/d3d/d3d11/TransformFeedback11.h"
#include "libANGLE/renderer/d3d/d3d11/VertexArray11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{

namespace
{
ANGLE_INLINE bool DrawCallHasDynamicAttribs(const gl::Context *context)
{
    VertexArray11 *vertexArray11 = GetImplAs<VertexArray11>(context->getState().getVertexArray());
    return vertexArray11->hasActiveDynamicAttrib(context);
}

bool DrawCallHasStreamingVertexArrays(const gl::Context *context, gl::PrimitiveMode mode)
{
    // Direct drawing doesn't support dynamic attribute storage since it needs the first and count
    // to translate when applyVertexBuffer. GL_LINE_LOOP and GL_TRIANGLE_FAN are not supported
    // either since we need to simulate them in D3D.
    if (DrawCallHasDynamicAttribs(context) || mode == gl::PrimitiveMode::LineLoop ||
        mode == gl::PrimitiveMode::TriangleFan)
    {
        return true;
    }

    return false;
}

bool DrawCallHasStreamingElementArray(const gl::Context *context, gl::DrawElementsType srcType)
{
    const gl::State &glState       = context->getState();
    gl::Buffer *elementArrayBuffer = glState.getVertexArray()->getElementArrayBuffer();

    bool primitiveRestartWorkaround =
        UsePrimitiveRestartWorkaround(glState.isPrimitiveRestartEnabled(), srcType);
    const gl::DrawElementsType dstType =
        (srcType == gl::DrawElementsType::UnsignedInt || primitiveRestartWorkaround)
            ? gl::DrawElementsType::UnsignedInt
            : gl::DrawElementsType::UnsignedShort;

    // Not clear where the offset comes from here.
    switch (ClassifyIndexStorage(glState, elementArrayBuffer, srcType, dstType, 0))
    {
        case IndexStorageType::Dynamic:
            return true;
        case IndexStorageType::Direct:
            return false;
        case IndexStorageType::Static:
        {
            BufferD3D *bufferD3D                     = GetImplAs<BufferD3D>(elementArrayBuffer);
            StaticIndexBufferInterface *staticBuffer = bufferD3D->getStaticIndexBuffer();
            return (staticBuffer->getBufferSize() == 0 || staticBuffer->getIndexType() != dstType);
        }
        default:
            UNREACHABLE();
            return true;
    }
}

template <typename IndirectBufferT>
angle::Result ReadbackIndirectBuffer(const gl::Context *context,
                                     const void *indirect,
                                     const IndirectBufferT **bufferPtrOut)
{
    const gl::State &glState       = context->getState();
    gl::Buffer *drawIndirectBuffer = glState.getTargetBuffer(gl::BufferBinding::DrawIndirect);
    ASSERT(drawIndirectBuffer);
    Buffer11 *storage = GetImplAs<Buffer11>(drawIndirectBuffer);
    uintptr_t offset  = reinterpret_cast<uintptr_t>(indirect);

    const uint8_t *bufferData = nullptr;
    ANGLE_TRY(storage->getData(context, &bufferData));
    ASSERT(bufferData);

    *bufferPtrOut = reinterpret_cast<const IndirectBufferT *>(bufferData + offset);
    return angle::Result::Continue;
}

bool IsSameExecutable(const gl::ProgramExecutable *a, const gl::ProgramExecutable *b)
{
    return GetImplAs<ProgramExecutableD3D>(a)->getSerial() ==
           GetImplAs<ProgramExecutableD3D>(b)->getSerial();
}
}  // anonymous namespace

Context11::Context11(const gl::State &state, gl::ErrorSet *errorSet, Renderer11 *renderer)
    : ContextD3D(state, errorSet),
      mRenderer(renderer),
      mDisjointQueryStarted(false),
      mDisjoint(false),
      mFrequency(0)
{}

Context11::~Context11() {}

angle::Result Context11::initialize(const angle::ImageLoadContext &imageLoadContext)
{
    mImageLoadContext = imageLoadContext;
    return angle::Result::Continue;
}

void Context11::onDestroy(const gl::Context *context)
{
    mIncompleteTextures.onDestroy(context);

    mImageLoadContext = {};
}

CompilerImpl *Context11::createCompiler()
{
    return new CompilerD3D(SH_HLSL_4_1_OUTPUT);
}

ShaderImpl *Context11::createShader(const gl::ShaderState &data)
{
    return new ShaderD3D(data, mRenderer);
}

ProgramImpl *Context11::createProgram(const gl::ProgramState &data)
{
    return new Program11(data, mRenderer);
}

ProgramExecutableImpl *Context11::createProgramExecutable(const gl::ProgramExecutable *executable)
{
    return new ProgramExecutableD3D(executable);
}

FramebufferImpl *Context11::createFramebuffer(const gl::FramebufferState &data)
{
    return new Framebuffer11(data, mRenderer);
}

TextureImpl *Context11::createTexture(const gl::TextureState &state)
{
    switch (state.getType())
    {
        case gl::TextureType::_2D:
        // GL_TEXTURE_VIDEO_IMAGE_WEBGL maps to native 2D texture on Windows platform
        case gl::TextureType::VideoImage:
            return new TextureD3D_2D(state, mRenderer);
        case gl::TextureType::CubeMap:
            return new TextureD3D_Cube(state, mRenderer);
        case gl::TextureType::_3D:
            return new TextureD3D_3D(state, mRenderer);
        case gl::TextureType::_2DArray:
            return new TextureD3D_2DArray(state, mRenderer);
        case gl::TextureType::External:
            return new TextureD3D_External(state, mRenderer);
        case gl::TextureType::_2DMultisample:
            return new TextureD3D_2DMultisample(state, mRenderer);
        case gl::TextureType::_2DMultisampleArray:
            return new TextureD3D_2DMultisampleArray(state, mRenderer);
        case gl::TextureType::Buffer:
            return new TextureD3D_Buffer(state, mRenderer);
        default:
            UNREACHABLE();
    }

    return nullptr;
}

RenderbufferImpl *Context11::createRenderbuffer(const gl::RenderbufferState &state)
{
    return new RenderbufferD3D(state, mRenderer);
}

BufferImpl *Context11::createBuffer(const gl::BufferState &state)
{
    Buffer11 *buffer = new Buffer11(state, mRenderer);
    mRenderer->onBufferCreate(buffer);
    return buffer;
}

VertexArrayImpl *Context11::createVertexArray(const gl::VertexArrayState &data)
{
    return new VertexArray11(data);
}

QueryImpl *Context11::createQuery(gl::QueryType type)
{
    return new Query11(mRenderer, type);
}

FenceNVImpl *Context11::createFenceNV()
{
    return new FenceNV11(mRenderer);
}

SyncImpl *Context11::createSync()
{
    return new Sync11(mRenderer);
}

TransformFeedbackImpl *Context11::createTransformFeedback(const gl::TransformFeedbackState &state)
{
    return new TransformFeedback11(state, mRenderer);
}

SamplerImpl *Context11::createSampler(const gl::SamplerState &state)
{
    return new SamplerD3D(state);
}

ProgramPipelineImpl *Context11::createProgramPipeline(const gl::ProgramPipelineState &data)
{
    return new ProgramPipeline11(data);
}

MemoryObjectImpl *Context11::createMemoryObject()
{
    UNREACHABLE();
    return nullptr;
}

SemaphoreImpl *Context11::createSemaphore()
{
    UNREACHABLE();
    return nullptr;
}

OverlayImpl *Context11::createOverlay(const gl::OverlayState &state)
{
    // Not implemented.
    return new OverlayImpl(state);
}

angle::Result Context11::flush(const gl::Context *context)
{
    return mRenderer->flush(this);
}

angle::Result Context11::finish(const gl::Context *context)
{
    return mRenderer->finish(this);
}

angle::Result Context11::drawArrays(const gl::Context *context,
                                    gl::PrimitiveMode mode,
                                    GLint first,
                                    GLsizei count)
{
    ASSERT(count > 0);
    ANGLE_TRY(mRenderer->getStateManager()->updateState(
        context, mode, first, count, gl::DrawElementsType::InvalidEnum, nullptr, 0, 0, 0, true));
    return mRenderer->drawArrays(context, mode, first, count, 0, 0, false);
}

angle::Result Context11::drawArraysInstanced(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             GLint first,
                                             GLsizei count,
                                             GLsizei instanceCount)
{
    ASSERT(count > 0);
    ANGLE_TRY(mRenderer->getStateManager()->updateState(context, mode, first, count,
                                                        gl::DrawElementsType::InvalidEnum, nullptr,
                                                        instanceCount, 0, 0, true));
    return mRenderer->drawArrays(context, mode, first, count, instanceCount, 0, true);
}

angle::Result Context11::drawArraysInstancedBaseInstance(const gl::Context *context,
                                                         gl::PrimitiveMode mode,
                                                         GLint first,
                                                         GLsizei count,
                                                         GLsizei instanceCount,
                                                         GLuint baseInstance)
{
    ASSERT(count > 0);
    ANGLE_TRY(mRenderer->getStateManager()->updateState(context, mode, first, count,
                                                        gl::DrawElementsType::InvalidEnum, nullptr,
                                                        instanceCount, 0, baseInstance, true));
    return mRenderer->drawArrays(context, mode, first, count, instanceCount, baseInstance, true);
}

ANGLE_INLINE angle::Result Context11::drawElementsImpl(const gl::Context *context,
                                                       gl::PrimitiveMode mode,
                                                       GLsizei indexCount,
                                                       gl::DrawElementsType indexType,
                                                       const void *indices,
                                                       GLsizei instanceCount,
                                                       GLint baseVertex,
                                                       GLuint baseInstance,
                                                       bool promoteDynamic,
                                                       bool isInstancedDraw)
{
    ASSERT(indexCount > 0);

    if (DrawCallHasDynamicAttribs(context))
    {
        gl::IndexRange indexRange;
        ANGLE_TRY(context->getState().getVertexArray()->getIndexRange(
            context, indexType, indexCount, indices, &indexRange));
        GLint startVertex;
        ANGLE_TRY(ComputeStartVertex(GetImplAs<Context11>(context), indexRange, baseVertex,
                                     &startVertex));
        ANGLE_TRY(mRenderer->getStateManager()->updateState(
            context, mode, startVertex, indexCount, indexType, indices, instanceCount, baseVertex,
            baseInstance, promoteDynamic));
        return mRenderer->drawElements(context, mode, startVertex, indexCount, indexType, indices,
                                       instanceCount, baseVertex, baseInstance, isInstancedDraw);
    }
    else
    {
        ANGLE_TRY(mRenderer->getStateManager()->updateState(context, mode, 0, indexCount, indexType,
                                                            indices, instanceCount, baseVertex,
                                                            baseInstance, promoteDynamic));
        return mRenderer->drawElements(context, mode, 0, indexCount, indexType, indices,
                                       instanceCount, baseVertex, baseInstance, isInstancedDraw);
    }
}

angle::Result Context11::drawElements(const gl::Context *context,
                                      gl::PrimitiveMode mode,
                                      GLsizei count,
                                      gl::DrawElementsType type,
                                      const void *indices)
{
    return drawElementsImpl(context, mode, count, type, indices, 0, 0, 0, true, false);
}

angle::Result Context11::drawElementsBaseVertex(const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                GLsizei count,
                                                gl::DrawElementsType type,
                                                const void *indices,
                                                GLint baseVertex)
{
    return drawElementsImpl(context, mode, count, type, indices, 0, baseVertex, 0, true, false);
}

angle::Result Context11::drawElementsInstanced(const gl::Context *context,
                                               gl::PrimitiveMode mode,
                                               GLsizei count,
                                               gl::DrawElementsType type,
                                               const void *indices,
                                               GLsizei instances)
{
    return drawElementsImpl(context, mode, count, type, indices, instances, 0, 0, true, true);
}

angle::Result Context11::drawElementsInstancedBaseVertex(const gl::Context *context,
                                                         gl::PrimitiveMode mode,
                                                         GLsizei count,
                                                         gl::DrawElementsType type,
                                                         const void *indices,
                                                         GLsizei instances,
                                                         GLint baseVertex)
{
    return drawElementsImpl(context, mode, count, type, indices, instances, baseVertex, 0, true,
                            true);
}

angle::Result Context11::drawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                                     gl::PrimitiveMode mode,
                                                                     GLsizei count,
                                                                     gl::DrawElementsType type,
                                                                     const void *indices,
                                                                     GLsizei instances,
                                                                     GLint baseVertex,
                                                                     GLuint baseInstance)
{
    return drawElementsImpl(context, mode, count, type, indices, instances, baseVertex,
                            baseInstance, true, true);
}

angle::Result Context11::drawRangeElements(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           GLuint start,
                                           GLuint end,
                                           GLsizei count,
                                           gl::DrawElementsType type,
                                           const void *indices)
{
    return drawElementsImpl(context, mode, count, type, indices, 0, 0, 0, true, false);
}

angle::Result Context11::drawRangeElementsBaseVertex(const gl::Context *context,
                                                     gl::PrimitiveMode mode,
                                                     GLuint start,
                                                     GLuint end,
                                                     GLsizei count,
                                                     gl::DrawElementsType type,
                                                     const void *indices,
                                                     GLint baseVertex)
{
    return drawElementsImpl(context, mode, count, type, indices, 0, baseVertex, 0, true, false);
}

angle::Result Context11::drawArraysIndirect(const gl::Context *context,
                                            gl::PrimitiveMode mode,
                                            const void *indirect)
{
    if (DrawCallHasStreamingVertexArrays(context, mode))
    {
        const gl::DrawArraysIndirectCommand *cmd = nullptr;
        ANGLE_TRY(ReadbackIndirectBuffer(context, indirect, &cmd));

        if (cmd->count == 0)
        {
            return angle::Result::Continue;
        }

        ANGLE_TRY(mRenderer->getStateManager()->updateState(
            context, mode, cmd->first, cmd->count, gl::DrawElementsType::InvalidEnum, nullptr,
            cmd->instanceCount, 0, 0, true));
        return mRenderer->drawArrays(context, mode, cmd->first, cmd->count, cmd->instanceCount,
                                     cmd->baseInstance, true);
    }
    else
    {
        ANGLE_TRY(mRenderer->getStateManager()->updateState(
            context, mode, 0, 0, gl::DrawElementsType::InvalidEnum, nullptr, 0, 0, 0, true));
        return mRenderer->drawArraysIndirect(context, indirect);
    }
}

angle::Result Context11::drawElementsIndirect(const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              gl::DrawElementsType type,
                                              const void *indirect)
{
    if (DrawCallHasStreamingVertexArrays(context, mode) ||
        DrawCallHasStreamingElementArray(context, type))
    {
        const gl::DrawElementsIndirectCommand *cmd = nullptr;
        ANGLE_TRY(ReadbackIndirectBuffer(context, indirect, &cmd));

        if (cmd->count == 0)
        {
            return angle::Result::Continue;
        }

        const GLuint typeBytes = gl::GetDrawElementsTypeSize(type);
        const void *indices =
            reinterpret_cast<const void *>(static_cast<uintptr_t>(cmd->firstIndex * typeBytes));

        // We must explicitly resolve the index range for the slow-path indirect drawElements to
        // make sure we are using the correct 'baseVertex'. This parameter does not exist for the
        // direct drawElements.
        gl::IndexRange indexRange;
        ANGLE_TRY(context->getState().getVertexArray()->getIndexRange(context, type, cmd->count,
                                                                      indices, &indexRange));

        GLint startVertex;
        ANGLE_TRY(ComputeStartVertex(GetImplAs<Context11>(context), indexRange, cmd->baseVertex,
                                     &startVertex));

        ANGLE_TRY(mRenderer->getStateManager()->updateState(
            context, mode, startVertex, cmd->count, type, indices, cmd->primCount, cmd->baseVertex,
            cmd->baseInstance, true));
        return mRenderer->drawElements(context, mode, static_cast<GLint>(indexRange.start),
                                       cmd->count, type, indices, cmd->primCount, 0,
                                       cmd->baseInstance, true);
    }
    else
    {
        ANGLE_TRY(mRenderer->getStateManager()->updateState(context, mode, 0, 0, type, nullptr, 0,
                                                            0, 0, true));
        return mRenderer->drawElementsIndirect(context, indirect);
    }
}

#define DRAW_ARRAYS__                                                                           \
    do                                                                                          \
    {                                                                                           \
        ANGLE_TRY(mRenderer->getStateManager()->updateState(                                    \
            context, mode, firsts[drawID], counts[drawID], gl::DrawElementsType::InvalidEnum,   \
            nullptr, 0, 0, 0, false));                                                          \
        ANGLE_TRY(                                                                              \
            mRenderer->drawArrays(context, mode, firsts[drawID], counts[drawID], 0, 0, false)); \
    } while (0)
#define DRAW_ARRAYS_INSTANCED_                                                                \
    do                                                                                        \
    {                                                                                         \
        ANGLE_TRY(mRenderer->getStateManager()->updateState(                                  \
            context, mode, firsts[drawID], counts[drawID], gl::DrawElementsType::InvalidEnum, \
            nullptr, instanceCounts[drawID], 0, 0, false));                                   \
        ANGLE_TRY(mRenderer->drawArrays(context, mode, firsts[drawID], counts[drawID],        \
                                        instanceCounts[drawID], 0, true));                    \
    } while (0)
#define DRAW_ARRAYS_INSTANCED_BASE_INSTANCE                                                    \
    do                                                                                         \
    {                                                                                          \
        ANGLE_TRY(mRenderer->getStateManager()->updateState(                                   \
            context, mode, firsts[drawID], counts[drawID], gl::DrawElementsType::InvalidEnum,  \
            nullptr, instanceCounts[drawID], 0, baseInstances[drawID], false));                \
        ANGLE_TRY(mRenderer->drawArrays(context, mode, firsts[drawID], counts[drawID],         \
                                        instanceCounts[drawID], baseInstances[drawID], true)); \
    } while (0)
#define DRAW_ELEMENTS__                                                                       \
    ANGLE_TRY(drawElementsImpl(context, mode, counts[drawID], type, indices[drawID], 0, 0, 0, \
                               false, false))
#define DRAW_ELEMENTS_INSTANCED_                                                     \
    ANGLE_TRY(drawElementsImpl(context, mode, counts[drawID], type, indices[drawID], \
                               instanceCounts[drawID], 0, 0, false, true))
#define DRAW_ELEMENTS_INSTANCED_BASE_VERTEX_BASE_INSTANCE                            \
    ANGLE_TRY(drawElementsImpl(context, mode, counts[drawID], type, indices[drawID], \
                               instanceCounts[drawID], baseVertices[drawID],         \
                               baseInstances[drawID], false, true))

#define DRAW_CALL(drawType, instanced, bvbi) DRAW_##drawType##instanced##bvbi

#define MULTI_DRAW_BLOCK(drawType, instanced, bvbi, hasDrawID, hasBaseVertex, hasBaseInstance) \
    do                                                                                         \
    {                                                                                          \
        for (GLsizei drawID = 0; drawID < drawcount; ++drawID)                                 \
        {                                                                                      \
            if (ANGLE_NOOP_DRAW(instanced))                                                    \
            {                                                                                  \
                continue;                                                                      \
            }                                                                                  \
            ANGLE_SET_DRAW_ID_UNIFORM(hasDrawID)(drawID);                                      \
            ANGLE_SET_BASE_VERTEX_UNIFORM(hasBaseVertex)(baseVertices[drawID]);                \
            ANGLE_SET_BASE_INSTANCE_UNIFORM(hasBaseInstance)(baseInstances[drawID]);           \
            ASSERT(counts[drawID] > 0);                                                        \
            DRAW_CALL(drawType, instanced, bvbi);                                              \
            ANGLE_MARK_TRANSFORM_FEEDBACK_USAGE(instanced);                                    \
            gl::MarkShaderStorageUsage(context);                                               \
        }                                                                                      \
        /* reset the uniform to zero for non-multi-draw uses of the program */                 \
        ANGLE_SET_DRAW_ID_UNIFORM(hasDrawID)(0);                                               \
    } while (0)

angle::Result Context11::multiDrawArrays(const gl::Context *context,
                                         gl::PrimitiveMode mode,
                                         const GLint *firsts,
                                         const GLsizei *counts,
                                         GLsizei drawcount)
{
    gl::ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const bool hasDrawID              = executable->hasDrawIDUniform();
    if (hasDrawID)
    {
        MULTI_DRAW_BLOCK(ARRAYS, _, _, 1, 0, 0);
    }
    else
    {
        MULTI_DRAW_BLOCK(ARRAYS, _, _, 0, 0, 0);
    }

    return angle::Result::Continue;
}

angle::Result Context11::multiDrawArraysInstanced(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  const GLint *firsts,
                                                  const GLsizei *counts,
                                                  const GLsizei *instanceCounts,
                                                  GLsizei drawcount)
{
    gl::ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const bool hasDrawID              = executable->hasDrawIDUniform();
    if (hasDrawID)
    {
        MULTI_DRAW_BLOCK(ARRAYS, _INSTANCED, _, 1, 0, 0);
    }
    else
    {
        MULTI_DRAW_BLOCK(ARRAYS, _INSTANCED, _, 0, 0, 0);
    }

    return angle::Result::Continue;
}

angle::Result Context11::multiDrawArraysIndirect(const gl::Context *context,
                                                 gl::PrimitiveMode mode,
                                                 const void *indirect,
                                                 GLsizei drawcount,
                                                 GLsizei stride)
{
    return rx::MultiDrawArraysIndirectGeneral(this, context, mode, indirect, drawcount, stride);
}

angle::Result Context11::multiDrawElements(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           const GLsizei *counts,
                                           gl::DrawElementsType type,
                                           const GLvoid *const *indices,
                                           GLsizei drawcount)
{
    gl::ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const bool hasDrawID              = executable->hasDrawIDUniform();
    if (hasDrawID)
    {
        MULTI_DRAW_BLOCK(ELEMENTS, _, _, 1, 0, 0);
    }
    else
    {
        MULTI_DRAW_BLOCK(ELEMENTS, _, _, 0, 0, 0);
    }

    return angle::Result::Continue;
}

angle::Result Context11::multiDrawElementsInstanced(const gl::Context *context,
                                                    gl::PrimitiveMode mode,
                                                    const GLsizei *counts,
                                                    gl::DrawElementsType type,
                                                    const GLvoid *const *indices,
                                                    const GLsizei *instanceCounts,
                                                    GLsizei drawcount)
{
    gl::ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const bool hasDrawID              = executable->hasDrawIDUniform();
    if (hasDrawID)
    {
        MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _, 1, 0, 0);
    }
    else
    {
        MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _, 0, 0, 0);
    }

    return angle::Result::Continue;
}

angle::Result Context11::multiDrawElementsIndirect(const gl::Context *context,
                                                   gl::PrimitiveMode mode,
                                                   gl::DrawElementsType type,
                                                   const void *indirect,
                                                   GLsizei drawcount,
                                                   GLsizei stride)
{
    return rx::MultiDrawElementsIndirectGeneral(this, context, mode, type, indirect, drawcount,
                                                stride);
}

angle::Result Context11::multiDrawArraysInstancedBaseInstance(const gl::Context *context,
                                                              gl::PrimitiveMode mode,
                                                              const GLint *firsts,
                                                              const GLsizei *counts,
                                                              const GLsizei *instanceCounts,
                                                              const GLuint *baseInstances,
                                                              GLsizei drawcount)
{
    gl::ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const bool hasDrawID              = executable->hasDrawIDUniform();
    const bool hasBaseInstance        = executable->hasBaseInstanceUniform();
    ResetBaseVertexBaseInstance resetUniforms(executable, false, hasBaseInstance);

    if (hasDrawID && hasBaseInstance)
    {
        MULTI_DRAW_BLOCK(ARRAYS, _INSTANCED, _BASE_INSTANCE, 1, 0, 1);
    }
    else if (hasDrawID)
    {
        MULTI_DRAW_BLOCK(ARRAYS, _INSTANCED, _BASE_INSTANCE, 1, 0, 0);
    }
    else if (hasBaseInstance)
    {
        MULTI_DRAW_BLOCK(ARRAYS, _INSTANCED, _BASE_INSTANCE, 0, 0, 1);
    }
    else
    {
        MULTI_DRAW_BLOCK(ARRAYS, _INSTANCED, _BASE_INSTANCE, 0, 0, 0);
    }

    return angle::Result::Continue;
}

angle::Result Context11::multiDrawElementsInstancedBaseVertexBaseInstance(
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
    gl::ProgramExecutable *executable = context->getState().getLinkedProgramExecutable(context);
    const bool hasDrawID              = executable->hasDrawIDUniform();
    const bool hasBaseVertex          = executable->hasBaseVertexUniform();
    const bool hasBaseInstance        = executable->hasBaseInstanceUniform();
    ResetBaseVertexBaseInstance resetUniforms(executable, hasBaseVertex, hasBaseInstance);

    if (hasDrawID)
    {
        if (hasBaseVertex)
        {
            if (hasBaseInstance)
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 1, 1, 1);
            }
            else
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 1, 1, 0);
            }
        }
        else
        {
            if (hasBaseInstance)
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 1, 0, 1);
            }
            else
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 1, 0, 0);
            }
        }
    }
    else
    {
        if (hasBaseVertex)
        {
            if (hasBaseInstance)
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 0, 1, 1);
            }
            else
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 0, 1, 0);
            }
        }
        else
        {
            if (hasBaseInstance)
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 0, 0, 1);
            }
            else
            {
                MULTI_DRAW_BLOCK(ELEMENTS, _INSTANCED, _BASE_VERTEX_BASE_INSTANCE, 0, 0, 0);
            }
        }
    }

    return angle::Result::Continue;
}

gl::GraphicsResetStatus Context11::getResetStatus()
{
    return mRenderer->getResetStatus();
}

angle::Result Context11::insertEventMarker(GLsizei length, const char *marker)
{
    mRenderer->getDebugAnnotatorContext()->setMarker(marker);
    return angle::Result::Continue;
}

angle::Result Context11::pushGroupMarker(GLsizei length, const char *marker)
{
    mRenderer->getDebugAnnotatorContext()->beginEvent(angle::EntryPoint::GLPushGroupMarkerEXT,
                                                      marker, marker);
    mMarkerStack.push(std::string(marker));
    return angle::Result::Continue;
}

angle::Result Context11::popGroupMarker()
{
    const char *marker = nullptr;
    if (!mMarkerStack.empty())
    {
        marker = mMarkerStack.top().c_str();
        mMarkerStack.pop();
        mRenderer->getDebugAnnotatorContext()->endEvent(marker,
                                                        angle::EntryPoint::GLPopGroupMarkerEXT);
    }
    return angle::Result::Continue;
}

angle::Result Context11::pushDebugGroup(const gl::Context *context,
                                        GLenum source,
                                        GLuint id,
                                        const std::string &message)
{
    // Fall through to the EXT_debug_marker functions
    return pushGroupMarker(static_cast<GLsizei>(message.size()), message.c_str());
}

angle::Result Context11::popDebugGroup(const gl::Context *context)
{
    // Fall through to the EXT_debug_marker functions
    return popGroupMarker();
}

angle::Result Context11::syncState(const gl::Context *context,
                                   const gl::state::DirtyBits dirtyBits,
                                   const gl::state::DirtyBits bitMask,
                                   const gl::state::ExtendedDirtyBits extendedDirtyBits,
                                   const gl::state::ExtendedDirtyBits extendedBitMask,
                                   gl::Command command)
{
    mRenderer->getStateManager()->syncState(context, dirtyBits, extendedDirtyBits, command);
    return angle::Result::Continue;
}

angle::Result Context11::checkDisjointQuery()
{
    if (!mDisjointQuery.valid())
    {
        D3D11_QUERY_DESC queryDesc;
        queryDesc.Query     = gl_d3d11::ConvertQueryType(gl::QueryType::Timestamp);
        queryDesc.MiscFlags = 0;

        ANGLE_TRY(mRenderer->allocateResource(this, queryDesc, &mDisjointQuery));
        mRenderer->getDeviceContext()->Begin(mDisjointQuery.get());
        mDisjointQueryStarted = true;
    }
    return angle::Result::Continue;
}

HRESULT Context11::checkDisjointQueryStatus()
{
    HRESULT result = S_OK;
    if (mDisjointQuery.valid())
    {
        ID3D11DeviceContext *context = mRenderer->getDeviceContext();
        if (mDisjointQueryStarted)
        {
            context->End(mDisjointQuery.get());
            mDisjointQueryStarted = false;
        }
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timeStats = {};
        result = context->GetData(mDisjointQuery.get(), &timeStats, sizeof(timeStats), 0);
        if (result == S_OK)
        {
            mFrequency = timeStats.Frequency;
            mDisjoint  = timeStats.Disjoint;
            mDisjointQuery.reset();
        }
    }
    return result;
}

UINT64 Context11::getDisjointFrequency()
{
    return mFrequency;
}

void Context11::setDisjointFrequency(UINT64 frequency)
{
    mFrequency = frequency;
}

void Context11::setGPUDisjoint()
{
    mDisjoint = true;
}

GLint Context11::getGPUDisjoint()
{
    if (mRenderer->getFeatures().enableTimestampQueries.enabled)
    {
        checkDisjointQueryStatus();
    }
    bool disjoint = mDisjoint;

    // Disjoint flag is cleared when read
    mDisjoint = false;

    return disjoint;
}

GLint64 Context11::getTimestamp()
{
    return mRenderer->getTimestamp();
}

angle::Result Context11::onMakeCurrent(const gl::Context *context)
{
    // Immediately return if the device has been lost.
    if (!mRenderer->getDevice())
    {
        return angle::Result::Continue;
    }

    return mRenderer->getStateManager()->onMakeCurrent(context);
}

gl::Caps Context11::getNativeCaps() const
{
    gl::Caps caps = mRenderer->getNativeCaps();

    // For pixel shaders, the render targets and unordered access views share the same resource
    // slots, so the maximum number of fragment shader outputs depends on the current context
    // version:
    // - If current context is ES 3.0 and below, we use D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT(8)
    //   as the value of max draw buffers because UAVs are not used.
    // - If current context is ES 3.1 and the feature level is 11_0, the RTVs and UAVs share 8
    //   slots. As ES 3.1 requires at least 1 atomic counter buffer in compute shaders, the value
    //   of max combined shader output resources is limited to 7, thus only 7 RTV slots can be
    //   used simultaneously.
    // - If current context is ES 3.1 and the feature level is 11_1, the RTVs and UAVs share 64
    //   slots. Currently we allocate 60 slots for combined shader output resources, so we can use
    //   at most D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT(8) RTVs simultaneously.
    if (mState.getClientVersion() >= gl::ES_3_1 &&
        mRenderer->getRenderer11DeviceCaps().featureLevel == D3D_FEATURE_LEVEL_11_0)
    {
        caps.maxDrawBuffers      = caps.maxCombinedShaderOutputResources;
        caps.maxColorAttachments = caps.maxCombinedShaderOutputResources;
    }

    return caps;
}

const gl::TextureCapsMap &Context11::getNativeTextureCaps() const
{
    return mRenderer->getNativeTextureCaps();
}

const gl::Extensions &Context11::getNativeExtensions() const
{
    return mRenderer->getNativeExtensions();
}

const gl::Limitations &Context11::getNativeLimitations() const
{
    return mRenderer->getNativeLimitations();
}

const ShPixelLocalStorageOptions &Context11::getNativePixelLocalStorageOptions() const
{
    return mRenderer->getNativePixelLocalStorageOptions();
}

angle::Result Context11::dispatchCompute(const gl::Context *context,
                                         GLuint numGroupsX,
                                         GLuint numGroupsY,
                                         GLuint numGroupsZ)
{
    return mRenderer->dispatchCompute(context, numGroupsX, numGroupsY, numGroupsZ);
}

angle::Result Context11::dispatchComputeIndirect(const gl::Context *context, GLintptr indirect)
{
    return mRenderer->dispatchComputeIndirect(context, indirect);
}

angle::Result Context11::triggerDrawCallProgramRecompilation(const gl::Context *context,
                                                             gl::PrimitiveMode drawMode)
{
    const auto &glState                 = context->getState();
    const auto *va11                    = GetImplAs<VertexArray11>(glState.getVertexArray());
    const auto *drawFBO                 = glState.getDrawFramebuffer();
    gl::ProgramExecutable *executable   = glState.getProgramExecutable();
    ProgramExecutableD3D *executableD3D = GetImplAs<ProgramExecutableD3D>(executable);

    executableD3D->updateCachedInputLayout(mRenderer, va11->getCurrentStateSerial(), glState);
    executableD3D->updateCachedOutputLayout(context, drawFBO);
    executableD3D->updateCachedImage2DBindLayout(context, gl::ShaderType::Fragment);

    bool recompileVS = !executableD3D->hasVertexExecutableForCachedInputLayout();
    bool recompileGS =
        !executableD3D->hasGeometryExecutableForPrimitiveType(mRenderer, glState, drawMode);
    bool recompilePS = !executableD3D->hasPixelExecutableForCachedOutputLayout();

    if (!recompileVS && !recompileGS && !recompilePS)
    {
        return angle::Result::Continue;
    }

    // Load the compiler if necessary and recompile the programs.
    ANGLE_TRY(mRenderer->ensureHLSLCompilerInitialized(this));

    gl::InfoLog infoLog;

    if (recompileVS)
    {
        ShaderExecutableD3D *vertexExe = nullptr;
        ANGLE_TRY(executableD3D->getVertexExecutableForCachedInputLayout(this, mRenderer,
                                                                         &vertexExe, &infoLog));
        if (!executableD3D->hasVertexExecutableForCachedInputLayout())
        {
            ASSERT(infoLog.getLength() > 0);
            ERR() << "Error compiling dynamic vertex executable: " << infoLog.str();
            ANGLE_TRY_HR(this, E_FAIL, "Error compiling dynamic vertex executable");
        }
    }

    if (recompileGS)
    {
        ShaderExecutableD3D *geometryExe = nullptr;
        ANGLE_TRY(executableD3D->getGeometryExecutableForPrimitiveType(
            this, mRenderer, glState.getCaps(), glState.getProvokingVertex(), drawMode,
            &geometryExe, &infoLog));
        if (!executableD3D->hasGeometryExecutableForPrimitiveType(mRenderer, glState, drawMode))
        {
            ASSERT(infoLog.getLength() > 0);
            ERR() << "Error compiling dynamic geometry executable: " << infoLog.str();
            ANGLE_TRY_HR(this, E_FAIL, "Error compiling dynamic geometry executable");
        }
    }

    if (recompilePS)
    {
        ShaderExecutableD3D *pixelExe = nullptr;
        ANGLE_TRY(executableD3D->getPixelExecutableForCachedOutputLayout(this, mRenderer, &pixelExe,
                                                                         &infoLog));
        if (!executableD3D->hasPixelExecutableForCachedOutputLayout())
        {
            ASSERT(infoLog.getLength() > 0);
            ERR() << "Error compiling dynamic pixel executable: " << infoLog.str();
            ANGLE_TRY_HR(this, E_FAIL, "Error compiling dynamic pixel executable");
        }
    }

    // Refresh the program cache entry.
    gl::Program *program = glState.getProgram();
    if (mMemoryProgramCache && IsSameExecutable(&program->getExecutable(), executable))
    {
        ANGLE_TRY(mMemoryProgramCache->updateProgram(context, program));
    }

    return angle::Result::Continue;
}

angle::Result Context11::triggerDispatchCallProgramRecompilation(const gl::Context *context)
{
    const auto &glState                 = context->getState();
    gl::ProgramExecutable *executable   = glState.getProgramExecutable();
    ProgramExecutableD3D *executableD3D = GetImplAs<ProgramExecutableD3D>(executable);

    executableD3D->updateCachedImage2DBindLayout(context, gl::ShaderType::Compute);

    bool recompileCS = !executableD3D->hasComputeExecutableForCachedImage2DBindLayout();

    if (!recompileCS)
    {
        return angle::Result::Continue;
    }

    // Load the compiler if necessary and recompile the programs.
    ANGLE_TRY(mRenderer->ensureHLSLCompilerInitialized(this));

    gl::InfoLog infoLog;

    ShaderExecutableD3D *computeExe = nullptr;
    ANGLE_TRY(executableD3D->getComputeExecutableForImage2DBindLayout(this, mRenderer, &computeExe,
                                                                      &infoLog));
    if (!executableD3D->hasComputeExecutableForCachedImage2DBindLayout())
    {
        ASSERT(infoLog.getLength() > 0);
        ERR() << "Dynamic recompilation error log: " << infoLog.str();
        ANGLE_TRY_HR(this, E_FAIL, "Error compiling dynamic compute executable");
    }

    // Refresh the program cache entry.
    gl::Program *program = glState.getProgram();
    if (mMemoryProgramCache && IsSameExecutable(&program->getExecutable(), executable))
    {
        ANGLE_TRY(mMemoryProgramCache->updateProgram(context, program));
    }

    return angle::Result::Continue;
}

angle::Result Context11::memoryBarrier(const gl::Context *context, GLbitfield barriers)
{
    return angle::Result::Continue;
}

angle::Result Context11::memoryBarrierByRegion(const gl::Context *context, GLbitfield barriers)
{
    return angle::Result::Continue;
}

angle::Result Context11::getIncompleteTexture(const gl::Context *context,
                                              gl::TextureType type,
                                              gl::Texture **textureOut)
{
    return mIncompleteTextures.getIncompleteTexture(context, type, gl::SamplerFormat::Float, this,
                                                    textureOut);
}

angle::Result Context11::initializeMultisampleTextureToBlack(const gl::Context *context,
                                                             gl::Texture *glTexture)
{
    ASSERT(glTexture->getType() == gl::TextureType::_2DMultisample);
    TextureD3D *textureD3D        = GetImplAs<TextureD3D>(glTexture);
    gl::ImageIndex index          = gl::ImageIndex::Make2DMultisample();
    RenderTargetD3D *renderTarget = nullptr;
    GLsizei texSamples            = textureD3D->getRenderToTextureSamples();
    ANGLE_TRY(textureD3D->getRenderTarget(context, index, texSamples, &renderTarget));
    return mRenderer->clearRenderTarget(context, renderTarget, gl::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                                        1.0f, 0);
}

void Context11::handleResult(HRESULT hr,
                             const char *message,
                             const char *file,
                             const char *function,
                             unsigned int line)
{
    ASSERT(FAILED(hr));

    GLenum glErrorCode = DefaultGLErrorCode(hr);

    std::stringstream errorStream;
    errorStream << "Internal D3D11 error: " << gl::FmtHR(hr);

    if (d3d11::isDeviceLostError(hr))
    {
        HRESULT removalReason = mRenderer->getDevice()->GetDeviceRemovedReason();
        errorStream << " (removal reason: " << gl::FmtHR(removalReason) << ")";
        mRenderer->notifyDeviceLost();
    }

    errorStream << ": " << message;

    mErrors->handleError(glErrorCode, errorStream.str().c_str(), file, function, line);
}
}  // namespace rx
