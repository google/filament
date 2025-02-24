//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextWgpu.h:
//    Defines the class interface for ContextWgpu, implementing ContextImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_CONTEXTWGPU_H_
#define LIBANGLE_RENDERER_WGPU_CONTEXTWGPU_H_

#include <dawn/webgpu_cpp.h>

#include "image_util/loadimage.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_command_buffer.h"
#include "libANGLE/renderer/wgpu/wgpu_format_utils.h"
#include "libANGLE/renderer/wgpu/wgpu_helpers.h"
#include "libANGLE/renderer/wgpu/wgpu_pipeline_state.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{

class ContextWgpu : public ContextImpl
{
  public:
    ContextWgpu(const gl::State &state, gl::ErrorSet *errorSet, DisplayWgpu *display);
    ~ContextWgpu() override;

    angle::Result initialize(const angle::ImageLoadContext &imageLoadContext) override;

    void onDestroy(const gl::Context *context) override;

    // Flush and finish.
    angle::Result flush(const gl::Context *context) override;
    angle::Result finish(const gl::Context *context) override;

    // Drawing methods.
    angle::Result drawArrays(const gl::Context *context,
                             gl::PrimitiveMode mode,
                             GLint first,
                             GLsizei count) override;
    angle::Result drawArraysInstanced(const gl::Context *context,
                                      gl::PrimitiveMode mode,
                                      GLint first,
                                      GLsizei count,
                                      GLsizei instanceCount) override;
    angle::Result drawArraysInstancedBaseInstance(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  GLint first,
                                                  GLsizei count,
                                                  GLsizei instanceCount,
                                                  GLuint baseInstance) override;

    angle::Result drawElements(const gl::Context *context,
                               gl::PrimitiveMode mode,
                               GLsizei count,
                               gl::DrawElementsType type,
                               const void *indices) override;
    angle::Result drawElementsBaseVertex(const gl::Context *context,
                                         gl::PrimitiveMode mode,
                                         GLsizei count,
                                         gl::DrawElementsType type,
                                         const void *indices,
                                         GLint baseVertex) override;
    angle::Result drawElementsInstanced(const gl::Context *context,
                                        gl::PrimitiveMode mode,
                                        GLsizei count,
                                        gl::DrawElementsType type,
                                        const void *indices,
                                        GLsizei instances) override;
    angle::Result drawElementsInstancedBaseVertex(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  GLsizei count,
                                                  gl::DrawElementsType type,
                                                  const void *indices,
                                                  GLsizei instances,
                                                  GLint baseVertex) override;
    angle::Result drawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                              gl::PrimitiveMode mode,
                                                              GLsizei count,
                                                              gl::DrawElementsType type,
                                                              const void *indices,
                                                              GLsizei instances,
                                                              GLint baseVertex,
                                                              GLuint baseInstance) override;
    angle::Result drawRangeElements(const gl::Context *context,
                                    gl::PrimitiveMode mode,
                                    GLuint start,
                                    GLuint end,
                                    GLsizei count,
                                    gl::DrawElementsType type,
                                    const void *indices) override;
    angle::Result drawRangeElementsBaseVertex(const gl::Context *context,
                                              gl::PrimitiveMode mode,
                                              GLuint start,
                                              GLuint end,
                                              GLsizei count,
                                              gl::DrawElementsType type,
                                              const void *indices,
                                              GLint baseVertex) override;
    angle::Result drawArraysIndirect(const gl::Context *context,
                                     gl::PrimitiveMode mode,
                                     const void *indirect) override;
    angle::Result drawElementsIndirect(const gl::Context *context,
                                       gl::PrimitiveMode mode,
                                       gl::DrawElementsType type,
                                       const void *indirect) override;

    angle::Result multiDrawArrays(const gl::Context *context,
                                  gl::PrimitiveMode mode,
                                  const GLint *firsts,
                                  const GLsizei *counts,
                                  GLsizei drawcount) override;
    angle::Result multiDrawArraysInstanced(const gl::Context *context,
                                           gl::PrimitiveMode mode,
                                           const GLint *firsts,
                                           const GLsizei *counts,
                                           const GLsizei *instanceCounts,
                                           GLsizei drawcount) override;
    angle::Result multiDrawArraysIndirect(const gl::Context *context,
                                          gl::PrimitiveMode mode,
                                          const void *indirect,
                                          GLsizei drawcount,
                                          GLsizei stride) override;
    angle::Result multiDrawElements(const gl::Context *context,
                                    gl::PrimitiveMode mode,
                                    const GLsizei *counts,
                                    gl::DrawElementsType type,
                                    const GLvoid *const *indices,
                                    GLsizei drawcount) override;
    angle::Result multiDrawElementsInstanced(const gl::Context *context,
                                             gl::PrimitiveMode mode,
                                             const GLsizei *counts,
                                             gl::DrawElementsType type,
                                             const GLvoid *const *indices,
                                             const GLsizei *instanceCounts,
                                             GLsizei drawcount) override;
    angle::Result multiDrawElementsIndirect(const gl::Context *context,
                                            gl::PrimitiveMode mode,
                                            gl::DrawElementsType type,
                                            const void *indirect,
                                            GLsizei drawcount,
                                            GLsizei stride) override;
    angle::Result multiDrawArraysInstancedBaseInstance(const gl::Context *context,
                                                       gl::PrimitiveMode mode,
                                                       const GLint *firsts,
                                                       const GLsizei *counts,
                                                       const GLsizei *instanceCounts,
                                                       const GLuint *baseInstances,
                                                       GLsizei drawcount) override;
    angle::Result multiDrawElementsInstancedBaseVertexBaseInstance(const gl::Context *context,
                                                                   gl::PrimitiveMode mode,
                                                                   const GLsizei *counts,
                                                                   gl::DrawElementsType type,
                                                                   const GLvoid *const *indices,
                                                                   const GLsizei *instanceCounts,
                                                                   const GLint *baseVertices,
                                                                   const GLuint *baseInstances,
                                                                   GLsizei drawcount) override;

    // Device loss
    gl::GraphicsResetStatus getResetStatus() override;

    // EXT_debug_marker
    angle::Result insertEventMarker(GLsizei length, const char *marker) override;
    angle::Result pushGroupMarker(GLsizei length, const char *marker) override;
    angle::Result popGroupMarker() override;

    // KHR_debug
    angle::Result pushDebugGroup(const gl::Context *context,
                                 GLenum source,
                                 GLuint id,
                                 const std::string &message) override;
    angle::Result popDebugGroup(const gl::Context *context) override;

    // State sync with dirty bits.
    angle::Result syncState(const gl::Context *context,
                            const gl::state::DirtyBits dirtyBits,
                            const gl::state::DirtyBits bitMask,
                            const gl::state::ExtendedDirtyBits extendedDirtyBits,
                            const gl::state::ExtendedDirtyBits extendedBitMask,
                            gl::Command command) override;

    // Disjoint timer queries
    GLint getGPUDisjoint() override;
    GLint64 getTimestamp() override;

    // Context switching
    angle::Result onMakeCurrent(const gl::Context *context) override;

    // Native capabilities, unmodified by gl::Context.
    gl::Caps getNativeCaps() const override;
    const gl::TextureCapsMap &getNativeTextureCaps() const override;
    const gl::Extensions &getNativeExtensions() const override;
    const gl::Limitations &getNativeLimitations() const override;
    const ShPixelLocalStorageOptions &getNativePixelLocalStorageOptions() const override;

    // Shader creation
    CompilerImpl *createCompiler() override;
    ShaderImpl *createShader(const gl::ShaderState &data) override;
    ProgramImpl *createProgram(const gl::ProgramState &data) override;
    ProgramExecutableImpl *createProgramExecutable(
        const gl::ProgramExecutable *executable) override;

    // Framebuffer creation
    FramebufferImpl *createFramebuffer(const gl::FramebufferState &data) override;

    // Texture creation
    TextureImpl *createTexture(const gl::TextureState &state) override;

    // Renderbuffer creation
    RenderbufferImpl *createRenderbuffer(const gl::RenderbufferState &state) override;

    // Buffer creation
    BufferImpl *createBuffer(const gl::BufferState &state) override;

    // Vertex Array creation
    VertexArrayImpl *createVertexArray(const gl::VertexArrayState &data) override;

    // Query and Fence creation
    QueryImpl *createQuery(gl::QueryType type) override;
    FenceNVImpl *createFenceNV() override;
    SyncImpl *createSync() override;

    // Transform Feedback creation
    TransformFeedbackImpl *createTransformFeedback(
        const gl::TransformFeedbackState &state) override;

    // Sampler object creation
    SamplerImpl *createSampler(const gl::SamplerState &state) override;

    // Program Pipeline object creation
    ProgramPipelineImpl *createProgramPipeline(const gl::ProgramPipelineState &data) override;

    // Memory object creation.
    MemoryObjectImpl *createMemoryObject() override;

    // Semaphore creation.
    SemaphoreImpl *createSemaphore() override;

    // Overlay creation.
    OverlayImpl *createOverlay(const gl::OverlayState &state) override;

    angle::Result dispatchCompute(const gl::Context *context,
                                  GLuint numGroupsX,
                                  GLuint numGroupsY,
                                  GLuint numGroupsZ) override;
    angle::Result dispatchComputeIndirect(const gl::Context *context, GLintptr indirect) override;

    angle::Result memoryBarrier(const gl::Context *context, GLbitfield barriers) override;
    angle::Result memoryBarrierByRegion(const gl::Context *context, GLbitfield barriers) override;

    void handleError(GLenum errorCode,
                     const char *message,
                     const char *file,
                     const char *function,
                     unsigned int line);

    const angle::ImageLoadContext &getImageLoadContext() const { return mImageLoadContext; }

    DisplayWgpu *getDisplay() { return mDisplay; }
    wgpu::Device &getDevice() { return mDisplay->getDevice(); }
    wgpu::Queue &getQueue() { return mDisplay->getQueue(); }
    wgpu::Instance &getInstance() { return mDisplay->getInstance(); }
    angle::ImageLoadContext &getImageLoadContext() { return mImageLoadContext; }
    const webgpu::Format &getFormat(GLenum internalFormat) const
    {
        return mDisplay->getFormat(internalFormat);
    }
    angle::Result startRenderPass(const wgpu::RenderPassDescriptor &desc);
    angle::Result endRenderPass(webgpu::RenderPassClosureReason closureReason);

    bool hasActiveRenderPass() { return mCurrentRenderPass != nullptr; }

    angle::Result onFramebufferChange(FramebufferWgpu *framebufferWgpu, gl::Command command);

    angle::Result flush(webgpu::RenderPassClosureReason);

    void setColorAttachmentFormat(size_t colorIndex, wgpu::TextureFormat format);
    void setColorAttachmentFormats(const gl::DrawBuffersArray<wgpu::TextureFormat> &formats);
    void setDepthStencilFormat(wgpu::TextureFormat format);
    void setVertexAttribute(size_t attribIndex, webgpu::PackedVertexAttribute newAttrib);

    void invalidateVertexBuffer(size_t slot);
    void invalidateVertexBuffers();
    void invalidateIndexBuffer();

    void ensureCommandEncoderCreated();
    wgpu::CommandEncoder &getCurrentCommandEncoder();

  private:
    // Dirty bits.
    enum DirtyBitType : size_t
    {
        // The pipeline has changed and needs to be recreated.
        DIRTY_BIT_RENDER_PIPELINE_DESC,

        DIRTY_BIT_RENDER_PASS,

        DIRTY_BIT_RENDER_PIPELINE_BINDING,
        DIRTY_BIT_VIEWPORT,
        DIRTY_BIT_SCISSOR,

        DIRTY_BIT_VERTEX_BUFFERS,
        DIRTY_BIT_INDEX_BUFFER,

        DIRTY_BIT_BIND_GROUPS,

        DIRTY_BIT_MAX,
    };

    static_assert(DIRTY_BIT_RENDER_PIPELINE_BINDING > DIRTY_BIT_RENDER_PIPELINE_DESC,
                  "Pipeline binding must be handled after the pipeline desc dirty bit");

    // Dirty bit handlers that record commands or otherwise expect to manipulate the render pass
    // that will be used for the draw call must be specified after DIRTY_BIT_RENDER_PASS.
    static_assert(DIRTY_BIT_RENDER_PIPELINE_BINDING > DIRTY_BIT_RENDER_PASS,
                  "Render pass using dirty bit must be handled after the render pass dirty bit");

    using DirtyBits = angle::BitSet<DIRTY_BIT_MAX>;

    DirtyBits mDirtyBits;
    gl::AttributesMask mDirtyVertexBuffers;

    DirtyBits mNewRenderPassDirtyBits;

    ANGLE_INLINE void invalidateCurrentRenderPipeline()
    {
        mDirtyBits.set(DIRTY_BIT_RENDER_PIPELINE_DESC);
    }

    angle::Result setupDraw(const gl::Context *context,
                            gl::PrimitiveMode mode,
                            GLint firstVertexOrInvalid,
                            GLsizei vertexOrIndexCount,
                            GLsizei instanceCount,
                            gl::DrawElementsType indexTypeOrInvalid,
                            const void *indices,
                            GLint baseVertex,
                            uint32_t *outFirstIndex,
                            uint32_t *indexCountOut);

    angle::Result handleDirtyRenderPipelineDesc(DirtyBits::Iterator *dirtyBitsIterator);
    angle::Result handleDirtyRenderPipelineBinding(DirtyBits::Iterator *dirtyBitsIterator);
    angle::Result handleDirtyViewport(DirtyBits::Iterator *dirtyBitsIterator);
    angle::Result handleDirtyScissor(DirtyBits::Iterator *dirtyBitsIterator);
    angle::Result handleDirtyVertexBuffers(const gl::AttributesMask &slots,
                                           DirtyBits::Iterator *dirtyBitsIterator);
    angle::Result handleDirtyIndexBuffer(gl::DrawElementsType indexType,
                                         DirtyBits::Iterator *dirtyBitsIterator);
    angle::Result handleDirtyBindGroups(DirtyBits::Iterator *dirtyBitsIterator);

    angle::Result handleDirtyRenderPass(DirtyBits::Iterator *dirtyBitsIterator);

    angle::ImageLoadContext mImageLoadContext;

    DisplayWgpu *mDisplay;

    wgpu::CommandEncoder mCurrentCommandEncoder;
    wgpu::RenderPassEncoder mCurrentRenderPass;

    webgpu::CommandBuffer mCommandBuffer;

    webgpu::RenderPipelineDesc mRenderPipelineDesc;
    wgpu::RenderPipeline mCurrentGraphicsPipeline;
    gl::AttributesMask mCurrentRenderPipelineAllAttributes;

    gl::DrawElementsType mCurrentIndexBufferType = gl::DrawElementsType::InvalidEnum;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_CONTEXTWGPU_H_
