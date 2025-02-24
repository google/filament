//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextMtl.h:
//    Defines the class interface for ContextMtl, implementing ContextImpl.
//

#ifndef LIBANGLE_RENDERER_METAL_CONTEXTMTL_H_
#define LIBANGLE_RENDERER_METAL_CONTEXTMTL_H_

#import <Metal/Metal.h>
#import <mach/mach_types.h>

#include "common/Optional.h"
#include "image_util/loadimage.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/metal/ProvokingVertexHelper.h"
#include "libANGLE/renderer/metal/mtl_buffer_manager.h"
#include "libANGLE/renderer/metal/mtl_buffer_pool.h"
#include "libANGLE/renderer/metal/mtl_command_buffer.h"
#include "libANGLE/renderer/metal/mtl_context_device.h"
#include "libANGLE/renderer/metal/mtl_occlusion_query_pool.h"
#include "libANGLE/renderer/metal/mtl_pipeline_cache.h"
#include "libANGLE/renderer/metal/mtl_resources.h"
#include "libANGLE/renderer/metal/mtl_state_cache.h"
#include "libANGLE/renderer/metal/mtl_utils.h"
namespace rx
{
class DisplayMtl;
class FramebufferMtl;
class VertexArrayMtl;
class ProgramMtl;
class ProgramExecutableMtl;
class RenderTargetMtl;
class WindowSurfaceMtl;
class TransformFeedbackMtl;

class ContextMtl : public ContextImpl, public mtl::Context
{
  public:
    ContextMtl(const gl::State &state,
               gl::ErrorSet *errorSet,
               const egl::AttributeMap &attribs,
               DisplayMtl *display);
    ~ContextMtl() override;

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
                                        GLsizei instanceCount) override;
    angle::Result drawElementsInstancedBaseVertex(const gl::Context *context,
                                                  gl::PrimitiveMode mode,
                                                  GLsizei count,
                                                  gl::DrawElementsType type,
                                                  const void *indices,
                                                  GLsizei instanceCount,
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
    angle::Result onUnMakeCurrent(const gl::Context *context) override;

    // Native capabilities, unmodified by gl::Context.
    gl::Caps getNativeCaps() const override;
    const gl::TextureCapsMap &getNativeTextureCaps() const override;
    const gl::Extensions &getNativeExtensions() const override;
    const gl::Limitations &getNativeLimitations() const override;
    const ShPixelLocalStorageOptions &getNativePixelLocalStorageOptions() const override;

    const ProgramExecutableMtl *getProgramExecutable() const { return mExecutable; }

    // Shader creation
    CompilerImpl *createCompiler() override;
    ShaderImpl *createShader(const gl::ShaderState &state) override;
    ProgramImpl *createProgram(const gl::ProgramState &state) override;
    ProgramExecutableImpl *createProgramExecutable(
        const gl::ProgramExecutable *executable) override;

    // Framebuffer creation
    FramebufferImpl *createFramebuffer(const gl::FramebufferState &state) override;

    // Texture creation
    TextureImpl *createTexture(const gl::TextureState &state) override;

    // Renderbuffer creation
    RenderbufferImpl *createRenderbuffer(const gl::RenderbufferState &state) override;

    // Buffer creation
    BufferImpl *createBuffer(const gl::BufferState &state) override;

    // Vertex Array creation
    VertexArrayImpl *createVertexArray(const gl::VertexArrayState &state) override;

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

    // override mtl::ErrorHandler
    void handleError(GLenum error,
                     const char *message,
                     const char *file,
                     const char *function,
                     unsigned int line) override;

    using ContextImpl::handleError;

    void invalidateState(const gl::Context *context);
    void invalidateDefaultAttribute(size_t attribIndex);
    void invalidateDefaultAttributes(const gl::AttributesMask &dirtyMask);
    void invalidateCurrentTextures();
    void invalidateDriverUniforms();
    void invalidateRenderPipeline();

    void updateIncompatibleAttachments(const gl::State &glState);

    // Call this to notify ContextMtl whenever FramebufferMtl's state changed
    void onDrawFrameBufferChangedState(const gl::Context *context,
                                       FramebufferMtl *framebuffer,
                                       bool renderPassChanged);
    void onBackbufferResized(const gl::Context *context, WindowSurfaceMtl *backbuffer);

    // Invoke by QueryMtl
    angle::Result onOcclusionQueryBegin(const gl::Context *context, QueryMtl *query);
    void onOcclusionQueryEnd(const gl::Context *context, QueryMtl *query);
    void onOcclusionQueryDestroy(const gl::Context *context, QueryMtl *query);

    // Useful for temporarily pause then restart occlusion query during clear/blit with draw.
    bool hasActiveOcclusionQuery() const { return mOcclusionQuery; }
    // Disable the occlusion query in the current render pass.
    // The render pass must already started.
    void disableActiveOcclusionQueryInRenderPass();
    // Re-enable the occlusion query in the current render pass.
    // The render pass must already started.
    // NOTE: the old query's result will be retained and combined with the new result.
    angle::Result restartActiveOcclusionQueryInRenderPass();

    // Invoke by TransformFeedbackMtl
    void onTransformFeedbackActive(const gl::Context *context, TransformFeedbackMtl *xfb);
    void onTransformFeedbackInactive(const gl::Context *context, TransformFeedbackMtl *xfb);

    // Invoked by multiple classes in SyncMtl.mm
    // Enqueue an event and return the command queue serial that the event was or will be placed in.
    uint64_t queueEventSignal(id<MTLEvent> event, uint64_t value);
    void serverWaitEvent(id<MTLEvent> event, uint64_t value);

    const mtl::ClearColorValue &getClearColorValue() const;
    const mtl::WriteMaskArray &getWriteMaskArray() const;
    float getClearDepthValue() const;
    uint32_t getClearStencilValue() const;
    // Return front facing stencil write mask
    uint32_t getStencilMask() const;
    bool getDepthMask() const;

    const mtl::Format &getPixelFormat(angle::FormatID angleFormatId) const;
    const mtl::FormatCaps &getNativeFormatCaps(MTLPixelFormat mtlFormat) const;
    // See mtl::FormatTable::getVertexFormat()
    const mtl::VertexFormat &getVertexFormat(angle::FormatID angleFormatId,
                                             bool tightlyPacked) const;

    angle::Result getIncompleteTexture(const gl::Context *context,
                                       gl::TextureType type,
                                       gl::SamplerFormat format,
                                       gl::Texture **textureOut);

    // Recommended to call these methods to end encoding instead of invoking the encoder's
    // endEncoding() directly.
    void endRenderEncoding(mtl::RenderCommandEncoder *encoder);
    // Ends any active command encoder
    void endEncoding(bool forceSaveRenderPassContent);

    void flushCommandBuffer(mtl::CommandBufferFinishOperation operation);
    void present(const gl::Context *context, id<CAMetalDrawable> presentationDrawable);
    angle::Result finishCommandBuffer();

    // Check whether compatible render pass has been started. Compatible render pass is a render
    // pass having the same attachments, and possibly having different load/store options.
    bool hasStartedRenderPass(const mtl::RenderPassDesc &desc);

    // Get current render encoder. May be nullptr if no render pass has been started.
    mtl::RenderCommandEncoder *getRenderCommandEncoder();

    // Will end current command encoder if it is valid, then start new encoder.
    // Unless hasStartedRenderPass(desc) returns true.
    // Note: passing a compatible render pass with different load/store options won't end the
    // current render pass. If a new render pass is desired, call endEncoding() prior to this.
    mtl::RenderCommandEncoder *getRenderPassCommandEncoder(const mtl::RenderPassDesc &desc);

    // Utilities to quickly create render command encoder to a specific texture:
    // The previous content of texture will be loaded
    mtl::RenderCommandEncoder *getTextureRenderCommandEncoder(const mtl::TextureRef &textureTarget,
                                                              const mtl::ImageNativeIndex &index);
    // The previous content of texture will be loaded if clearColor is not provided
    mtl::RenderCommandEncoder *getRenderTargetCommandEncoderWithClear(
        const RenderTargetMtl &renderTarget,
        const Optional<MTLClearColor> &clearColor);
    // The previous content of texture will be loaded
    mtl::RenderCommandEncoder *getRenderTargetCommandEncoder(const RenderTargetMtl &renderTarget);

    // Will end current command encoder and start new blit command encoder. Unless a blit comamnd
    // encoder is already started.
    mtl::BlitCommandEncoder *getBlitCommandEncoder();

    // Will end current command encoder and start new compute command encoder. Unless a compute
    // command encoder is already started.
    mtl::ComputeCommandEncoder *getComputeCommandEncoder();

    // Because this backend uses an intermediate representation for the rendering
    // commands, a render encoder can coexist with blit/compute command encoders.
    // Note: the blit/compute commands will run before the pending render commands.
    mtl::BlitCommandEncoder *getBlitCommandEncoderWithoutEndingRenderEncoder();
    mtl::ComputeCommandEncoder *getComputeCommandEncoderWithoutEndingRenderEncoder();

    // Get the provoking vertex command encoder.
    mtl::ComputeCommandEncoder *getIndexPreprocessingCommandEncoder();

    bool isCurrentRenderEncoderSerial(uint64_t serial);

    const mtl::ContextDevice &getMetalDevice() const { return mContextDevice; }

    mtl::BufferManager &getBufferManager() { return mBufferManager; }

    mtl::PipelineCache &getPipelineCache() { return mPipelineCache; }

    const angle::ImageLoadContext &getImageLoadContext() const { return mImageLoadContext; }

    bool getForceResyncDrawFramebuffer() const { return mForceResyncDrawFramebuffer; }
    gl::DrawBufferMask getIncompatibleAttachments() const { return mIncompatibleAttachments; }

  private:
    void ensureCommandBufferReady();
    void endBlitAndComputeEncoding();
    angle::Result resyncDrawFramebufferIfNeeded(const gl::Context *context);
    angle::Result setupDraw(const gl::Context *context,
                            gl::PrimitiveMode mode,
                            GLint firstVertex,
                            GLsizei vertexOrIndexCount,
                            GLsizei instanceCount,
                            gl::DrawElementsType indexTypeOrNone,
                            const void *indices,
                            bool xfbPass,
                            bool *isNoOp);

    angle::Result setupDrawImpl(const gl::Context *context,
                                gl::PrimitiveMode mode,
                                GLint firstVertex,
                                GLsizei vertexOrIndexCount,
                                GLsizei instanceCount,
                                gl::DrawElementsType indexTypeOrNone,
                                const void *indices,
                                bool xfbPass,
                                bool *isNoOp);

    angle::Result drawTriFanArrays(const gl::Context *context,
                                   GLint first,
                                   GLsizei count,
                                   GLsizei instances,
                                   GLuint baseInstance);
    angle::Result drawTriFanArraysWithBaseVertex(const gl::Context *context,
                                                 GLint first,
                                                 GLsizei count,
                                                 GLsizei instances,
                                                 GLuint baseInstance);
    angle::Result drawTriFanArraysLegacy(const gl::Context *context,
                                         GLint first,
                                         GLsizei count,
                                         GLsizei instances);
    angle::Result drawTriFanElements(const gl::Context *context,
                                     GLsizei count,
                                     gl::DrawElementsType type,
                                     const void *indices,
                                     GLsizei instances,
                                     GLint baseVertex,
                                     GLuint baseInstance);

    angle::Result drawLineLoopArraysNonInstanced(const gl::Context *context,
                                                 GLint first,
                                                 GLsizei count);
    angle::Result drawLineLoopArrays(const gl::Context *context,
                                     GLint first,
                                     GLsizei count,
                                     GLsizei instances,
                                     GLuint baseInstance);
    angle::Result drawLineLoopElementsNonInstancedNoPrimitiveRestart(const gl::Context *context,
                                                                     GLsizei count,
                                                                     gl::DrawElementsType type,
                                                                     const void *indices);
    angle::Result drawLineLoopElements(const gl::Context *context,
                                       GLsizei count,
                                       gl::DrawElementsType type,
                                       const void *indices,
                                       GLsizei instances,
                                       GLint baseVertex,
                                       GLuint baseInstance);

    angle::Result drawArraysProvokingVertexImpl(const gl::Context *context,
                                                gl::PrimitiveMode mode,
                                                GLsizei first,
                                                GLsizei count,
                                                GLsizei instances,
                                                GLuint baseInstance);

    angle::Result drawArraysImpl(const gl::Context *context,
                                 gl::PrimitiveMode mode,
                                 GLint first,
                                 GLsizei count,
                                 GLsizei instanceCount,
                                 GLuint baseInstance);

    angle::Result drawElementsImpl(const gl::Context *context,
                                   gl::PrimitiveMode mode,
                                   GLsizei count,
                                   gl::DrawElementsType type,
                                   const void *indices,
                                   GLsizei instanceCount,
                                   GLint baseVertex,
                                   GLuint baseInstance);
    void flushCommandBufferIfNeeded();
    void updateExtendedState(const gl::State &glState,
                             const gl::state::ExtendedDirtyBits extendedDirtyBits);

    void updateViewport(FramebufferMtl *framebufferMtl,
                        const gl::Rectangle &viewport,
                        float nearPlane,
                        float farPlane);
    void updateDepthRange(float nearPlane, float farPlane);
    void updateBlendDescArray(const gl::BlendStateExt &blendStateExt);
    void updateScissor(const gl::State &glState);
    void updateCullMode(const gl::State &glState);
    void updateFrontFace(const gl::State &glState);
    void updateDrawFrameBufferBinding(const gl::Context *context);
    void updateProgramExecutable(const gl::Context *context);
    void updateVertexArray(const gl::Context *context);
    bool requiresIndexRewrite(const gl::State &state, gl::PrimitiveMode mode);
    angle::Result updateDefaultAttribute(size_t attribIndex);
    void filterOutXFBOnlyDirtyBits(const gl::Context *context);
    angle::Result handleDirtyActiveTextures(const gl::Context *context);
    angle::Result handleDirtyDefaultAttribs(const gl::Context *context);
    angle::Result handleDirtyDriverUniforms(const gl::Context *context,
                                            GLint drawCallFirstVertex,
                                            uint32_t verticesPerInstance);
    angle::Result fillDriverXFBUniforms(GLint drawCallFirstVertex,
                                        uint32_t verticesPerInstance,
                                        uint32_t skippedInstances);
    angle::Result handleDirtyDepthStencilState(const gl::Context *context);
    angle::Result handleDirtyDepthBias(const gl::Context *context);
    angle::Result handleDirtyRenderPass(const gl::Context *context);
    angle::Result checkIfPipelineChanged(const gl::Context *context,
                                         gl::PrimitiveMode primitiveMode,
                                         bool xfbPass,
                                         bool *pipelineDescChanged);

    angle::Result startOcclusionQueryInRenderPass(QueryMtl *query, bool clearOldValue);

    angle::Result checkCommandBufferError();

    // Dirty bits.
    enum DirtyBitType : size_t
    {
        DIRTY_BIT_DEFAULT_ATTRIBS,
        DIRTY_BIT_TEXTURES,
        DIRTY_BIT_DRIVER_UNIFORMS,
        DIRTY_BIT_DEPTH_STENCIL_DESC,
        DIRTY_BIT_DEPTH_BIAS,
        DIRTY_BIT_DEPTH_CLIP_MODE,
        DIRTY_BIT_STENCIL_REF,
        DIRTY_BIT_BLEND_COLOR,
        DIRTY_BIT_VIEWPORT,
        DIRTY_BIT_SCISSOR,
        DIRTY_BIT_DRAW_FRAMEBUFFER,
        DIRTY_BIT_CULL_MODE,
        DIRTY_BIT_FILL_MODE,
        DIRTY_BIT_WINDING,
        DIRTY_BIT_RENDER_PIPELINE,
        DIRTY_BIT_UNIFORM_BUFFERS_BINDING,
        DIRTY_BIT_RASTERIZER_DISCARD,

        DIRTY_BIT_INVALID,
        DIRTY_BIT_MAX = DIRTY_BIT_INVALID,
    };

    // Must keep this in sync with DriverUniform::createUniformFields in:
    // src/compiler/translator/tree_util/DriverUniform.cpp
    // and DriverUniformMetal::createUniformFields in:
    // src/compiler/translator/DriverUniformMetal.cpp
    struct DriverUniforms
    {
        uint32_t acbBufferOffsets[2];
        float depthRange[2];
        uint32_t renderArea;
        uint32_t flipXY;
        uint32_t unused;
        uint32_t misc;

        int32_t xfbBufferOffsets[4];
        int32_t xfbVerticesPerInstance;
        uint32_t coverageMask;  // Metal specific
        uint32_t unused2[2];
    };
    static_assert(sizeof(DriverUniforms) % (sizeof(uint32_t) * 4) == 0,
                  "DriverUniforms should be 16 bytes aligned");

    struct DefaultAttribute
    {
        uint8_t values[sizeof(float) * 4];
    };

    angle::ImageLoadContext mImageLoadContext;

    mtl::OcclusionQueryPool mOcclusionQueryPool;

    mtl::CommandBuffer mCmdBuffer;
    mtl::RenderCommandEncoder mRenderEncoder;
    mtl::BlitCommandEncoder mBlitEncoder;
    mtl::ComputeCommandEncoder mComputeEncoder;

    mtl::PipelineCache mPipelineCache;

    // Cached back-end objects
    FramebufferMtl *mDrawFramebuffer  = nullptr;
    VertexArrayMtl *mVertexArray      = nullptr;
    ProgramExecutableMtl *mExecutable = nullptr;
    QueryMtl *mOcclusionQuery         = nullptr;

    using DirtyBits = angle::BitSet<DIRTY_BIT_MAX>;

    gl::AttributesMask mDirtyDefaultAttribsMask;
    DirtyBits mDirtyBits;

    uint32_t mRenderPassesSinceFlush = 0;

    // State
    mtl::RenderPipelineDesc mRenderPipelineDesc;
    mtl::DepthStencilDesc mDepthStencilDesc;
    mtl::BlendDescArray mBlendDescArray;
    mtl::WriteMaskArray mWriteMaskArray;
    mtl::ClearColorValue mClearColor;
    uint32_t mClearStencil    = 0;
    uint32_t mStencilRefFront = 0;
    uint32_t mStencilRefBack  = 0;
    MTLViewport mViewport;
    MTLScissorRect mScissorRect;
    MTLWinding mWinding;
    MTLCullMode mCullMode;
    bool mCullAllPolygons = false;

    // Cached state to handle attachments incompatible with the current program
    bool mForceResyncDrawFramebuffer = false;
    gl::DrawBufferMask mIncompatibleAttachments;

    mtl::BufferManager mBufferManager;

    // Lineloop and TriFan index buffer
    mtl::BufferPool mLineLoopIndexBuffer;
    mtl::BufferPool mLineLoopLastSegmentIndexBuffer;
    mtl::BufferPool mTriFanIndexBuffer;
    // one buffer can be reused for any starting vertex in DrawArrays()
    mtl::BufferRef mTriFanArraysIndexBuffer;

    // Dummy texture to be used for transform feedback only pass.
    mtl::TextureRef mDummyXFBRenderTexture;

    DriverUniforms mDriverUniforms;

    DefaultAttribute mDefaultAttributes[mtl::kMaxVertexAttribs];

    IncompleteTextureSet mIncompleteTextures;
    ProvokingVertexHelper mProvokingVertexHelper;

    mtl::ContextDevice mContextDevice;
};

}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_CONTEXTMTL_H_ */
