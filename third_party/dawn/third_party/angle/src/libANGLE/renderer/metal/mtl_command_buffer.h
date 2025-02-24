//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_command_buffer.h:
//    Defines the wrapper classes for Metal's MTLCommandEncoder, MTLCommandQueue and
//    MTLCommandBuffer.
//

#ifndef LIBANGLE_RENDERER_METAL_COMMANDENBUFFERMTL_H_
#define LIBANGLE_RENDERER_METAL_COMMANDENBUFFERMTL_H_

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <cstdint>

#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "common/FixedVector.h"
#include "common/angleutils.h"
#include "libANGLE/renderer/metal/mtl_common.h"
#include "libANGLE/renderer/metal/mtl_resources.h"
#include "libANGLE/renderer/metal/mtl_state_cache.h"

namespace rx
{
namespace mtl
{

enum CommandBufferFinishOperation
{
    NoWait,
    WaitUntilScheduled,
    WaitUntilFinished
};

class CommandBuffer;
class CommandEncoder;
class RenderCommandEncoder;
class OcclusionQueryPool;

class AtomicSerial : angle::NonCopyable
{
  public:
    uint64_t load() const { return mValue.load(std::memory_order_consume); }
    void increment(uint64_t value) { mValue.fetch_add(1, std::memory_order_release); }
    void storeMaxValue(uint64_t value);

  private:
    std::atomic<uint64_t> mValue{0};
};

class AtomicCommandBufferError : angle::NonCopyable
{
  public:
    void store(MTLCommandBufferError value) { mValue.store(value, std::memory_order_release); }
    MTLCommandBufferError pop()
    {
        return mValue.exchange(MTLCommandBufferErrorNone, std::memory_order_acq_rel);
    }

  private:
    std::atomic<MTLCommandBufferError> mValue{MTLCommandBufferErrorNone};
};

class CommandQueue final : public WrappedObject<id<MTLCommandQueue>>, angle::NonCopyable
{
  public:
    void reset();
    void set(id<MTLCommandQueue> metalQueue);

    void finishAllCommands();

    // This method will ensure that every GPU command buffer using this resource will finish before
    // returning. Note: this doesn't include the "in-progress" command buffer, i.e. the one hasn't
    // been commmitted yet. It's the responsibility of caller to make sure that command buffer is
    // commited/flushed first before calling this method.
    void ensureResourceReadyForCPU(const ResourceRef &resource);
    void ensureResourceReadyForCPU(Resource *resource);

    // Check whether the resource is being used by any command buffer still running on GPU.
    // This must be called before attempting to read the content of resource on CPU side.
    bool isResourceBeingUsedByGPU(const ResourceRef &resource) const
    {
        return isResourceBeingUsedByGPU(resource.get());
    }
    bool isResourceBeingUsedByGPU(const Resource *resource) const;

    // Checks whether the last command buffer that uses the given resource has been committed or not
    bool resourceHasPendingWorks(const Resource *resource) const;
    // Checks whether the last command buffer that uses the given resource (in a render encoder) has
    // been committed or not
    bool resourceHasPendingRenderWorks(const Resource *resource) const;

    bool isSerialCompleted(uint64_t serial) const;
    bool waitUntilSerialCompleted(uint64_t serial, uint64_t timeoutNs) const;

    CommandQueue &operator=(id<MTLCommandQueue> metalQueue)
    {
        set(metalQueue);
        return *this;
    }

    AutoObjCPtr<id<MTLCommandBuffer>> makeMetalCommandBuffer(uint64_t *queueSerialOut);
    void onCommandBufferCommitted(id<MTLCommandBuffer> buf, uint64_t serial);

    uint64_t getNextRenderPassEncoderSerial();

    uint64_t allocateTimeElapsedEntry();
    bool deleteTimeElapsedEntry(uint64_t id);
    void setActiveTimeElapsedEntry(uint64_t id);
    bool isTimeElapsedEntryComplete(uint64_t id);
    double getTimeElapsedEntryInSeconds(uint64_t id);
    MTLCommandBufferError popCmdBufferError() { return mCmdBufferError.pop(); }

  private:
    void onCommandBufferCompleted(id<MTLCommandBuffer> buf,
                                  uint64_t serial,
                                  uint64_t timeElapsedEntry);
    using ParentClass = WrappedObject<id<MTLCommandQueue>>;

    struct CmdBufferQueueEntry
    {
        AutoObjCPtr<id<MTLCommandBuffer>> buffer;
        uint64_t serial;
    };
    std::deque<CmdBufferQueueEntry> mMetalCmdBuffers;

    uint64_t mQueueSerialCounter = 1;
    AtomicSerial mCommittedBufferSerial;
    AtomicSerial mCompletedBufferSerial;
    uint64_t mRenderEncoderCounter = 1;

    // The bookkeeping for TIME_ELAPSED queries must be managed under
    // the cover of a lock because it's accessed by multiple threads:
    // the application, and the internal thread which dispatches the
    // command buffer completed handlers. The QueryMtl object
    // allocates and deallocates the IDs and associated storage.
    // In-flight CommandBuffers might refer to IDs that have been
    // deallocated. ID 0 is used as a sentinel.
    struct TimeElapsedEntry
    {
        double elapsed_seconds          = 0.0;
        int32_t pending_command_buffers = 0;
        uint64_t id                     = 0;
    };
    angle::HashMap<uint64_t, TimeElapsedEntry> mTimeElapsedEntries;
    uint64_t mTimeElapsedNextId   = 1;
    uint64_t mActiveTimeElapsedId = 0;

    mutable std::mutex mLock;
    mutable std::condition_variable mCompletedBufferSerialCv;

    AtomicCommandBufferError mCmdBufferError;

    void addCommandBufferToTimeElapsedEntry(std::lock_guard<std::mutex> &lg, uint64_t id);
    void recordCommandBufferTimeElapsed(std::lock_guard<std::mutex> &lg,
                                        uint64_t id,
                                        double seconds);
};

class CommandBuffer final : public WrappedObject<id<MTLCommandBuffer>>, angle::NonCopyable
{
  public:
    CommandBuffer(CommandQueue *cmdQueue);
    ~CommandBuffer();

    // This method must be called so that command encoder can be used.
    void restart();

    // Return true if command buffer can be encoded into. Return false if it has been committed
    // and hasn't been restarted.
    bool ready() const;
    void commit(CommandBufferFinishOperation operation);
    void wait(CommandBufferFinishOperation operation);

    void present(id<CAMetalDrawable> presentationDrawable);

    void setWriteDependency(const ResourceRef &resource, bool isRenderCommand);
    void setReadDependency(const ResourceRef &resource, bool isRenderCommand);
    void setReadDependency(Resource *resourcePtr, bool isRenderCommand);

    // Queues the event and returns the current command buffer queue serial.
    uint64_t queueEventSignal(id<MTLEvent> event, uint64_t value);
    void serverWaitEvent(id<MTLEvent> event, uint64_t value);

    void insertDebugSign(const std::string &marker);
    void pushDebugGroup(const std::string &marker);
    void popDebugGroup();

    CommandQueue &cmdQueue() { return mCmdQueue; }
    const CommandQueue &cmdQueue() const { return mCmdQueue; }

    // Private use only
    void setActiveCommandEncoder(CommandEncoder *encoder);
    void invalidateActiveCommandEncoder(CommandEncoder *encoder);

    bool needsFlushForDrawCallLimits() const;

    uint64_t getQueueSerial() const;

  private:
    void set(id<MTLCommandBuffer> metalBuffer);

    // This function returns either blit/compute encoder (if active) or render encoder.
    // If both types of encoders are active (blit/compute and render), the former will be returned.
    CommandEncoder *getPendingCommandEncoder();

    void cleanup();

    bool readyImpl() const;
    bool commitImpl();
    void forceEndingAllEncoders();

    void setPendingEvents();
    void setEventImpl(id<MTLEvent> event, uint64_t value);

    void pushDebugGroupImpl(const std::string &marker);
    void popDebugGroupImpl();

    void setResourceUsedByCommandBuffer(const ResourceRef &resource);
    void clearResourceListAndSize();

    using ParentClass = WrappedObject<id<MTLCommandBuffer>>;

    CommandQueue &mCmdQueue;

    // Note: due to render command encoder being a deferred encoder, it can coexist with
    // blit/compute encoder. When submitting, blit/compute encoder will be executed before the
    // render encoder.
    CommandEncoder *mActiveRenderEncoder        = nullptr;
    CommandEncoder *mActiveBlitOrComputeEncoder = nullptr;

    uint64_t mQueueSerial = 0;

    mutable std::mutex mLock;

    std::vector<std::string> mPendingDebugSigns;
    struct PendingEvent
    {
        AutoObjCPtr<id<MTLEvent>> event;
        uint64_t signalValue = 0;
    };
    std::vector<PendingEvent> mPendingSignalEvents;
    std::vector<std::string> mDebugGroups;

    angle::HashSet<id> mResourceList;
    size_t mWorkingResourceSize              = 0;
    bool mCommitted                          = false;
    CommandBufferFinishOperation mLastWaitOp = mtl::NoWait;
};

class CommandEncoder : public WrappedObject<id<MTLCommandEncoder>>, angle::NonCopyable
{
  public:
    enum Type
    {
        RENDER,
        BLIT,
        COMPUTE,
    };

    virtual ~CommandEncoder();

    virtual void endEncoding();

    virtual void reset();
    Type getType() const { return mType; }

    CommandEncoder &markResourceBeingWrittenByGPU(const BufferRef &buffer);
    CommandEncoder &markResourceBeingWrittenByGPU(const TextureRef &texture);

    void insertDebugSign(NSString *label);

    virtual void pushDebugGroup(NSString *label);
    virtual void popDebugGroup();

  protected:
    using ParentClass = WrappedObject<id<MTLCommandEncoder>>;

    CommandEncoder(CommandBuffer *cmdBuffer, Type type);

    CommandBuffer &cmdBuffer() { return mCmdBuffer; }
    CommandQueue &cmdQueue() { return mCmdBuffer.cmdQueue(); }

    void set(id<MTLCommandEncoder> metalCmdEncoder);

    virtual void insertDebugSignImpl(NSString *marker);

  private:
    bool isRenderEncoder() const { return getType() == Type::RENDER; }

    const Type mType;
    CommandBuffer &mCmdBuffer;
};

// Stream to store commands before encoding them into the real MTLCommandEncoder
class IntermediateCommandStream
{
  public:
    template <typename T>
    inline IntermediateCommandStream &push(const T &val)
    {
        auto ptr = reinterpret_cast<const uint8_t *>(&val);
        mBuffer.insert(mBuffer.end(), ptr, ptr + sizeof(T));
        return *this;
    }

    inline IntermediateCommandStream &push(const uint8_t *bytes, size_t len)
    {
        mBuffer.insert(mBuffer.end(), bytes, bytes + len);
        return *this;
    }

    template <typename T>
    inline T peek()
    {
        ASSERT(mReadPtr <= mBuffer.size() - sizeof(T));
        T re;
        auto ptr = reinterpret_cast<uint8_t *>(&re);
        std::copy(mBuffer.data() + mReadPtr, mBuffer.data() + mReadPtr + sizeof(T), ptr);
        return re;
    }

    template <typename T>
    inline T fetch()
    {
        auto re = peek<T>();
        mReadPtr += sizeof(T);
        return re;
    }

    inline const uint8_t *fetch(size_t bytes)
    {
        ASSERT(mReadPtr <= mBuffer.size() - bytes);
        auto cur = mReadPtr;
        mReadPtr += bytes;
        return mBuffer.data() + cur;
    }

    inline void clear()
    {
        mBuffer.clear();
        mReadPtr = 0;
    }

    inline void resetReadPtr(size_t readPtr)
    {
        ASSERT(readPtr <= mBuffer.size());
        mReadPtr = readPtr;
    }

    inline bool good() const { return mReadPtr < mBuffer.size(); }

  private:
    std::vector<uint8_t> mBuffer;
    size_t mReadPtr = 0;
};

// Per shader stage's states
struct RenderCommandEncoderShaderStates
{
    RenderCommandEncoderShaderStates();

    void reset();

    std::array<id<MTLBuffer>, kMaxShaderBuffers> buffers;
    std::array<uint32_t, kMaxShaderBuffers> bufferOffsets;
    std::array<id<MTLSamplerState>, kMaxShaderSamplers> samplers;
    std::array<Optional<std::pair<float, float>>, kMaxShaderSamplers> samplerLodClamps;
    std::array<id<MTLTexture>, kMaxShaderSamplers> textures;
};

// Per render pass's states
struct RenderCommandEncoderStates
{
    RenderCommandEncoderStates();

    void reset();

    id<MTLRenderPipelineState> renderPipeline;

    MTLTriangleFillMode triangleFillMode;
    MTLWinding winding;
    MTLCullMode cullMode;

    id<MTLDepthStencilState> depthStencilState;
    float depthBias, depthSlopeScale, depthClamp;

    MTLDepthClipMode depthClipMode;

    uint32_t stencilFrontRef, stencilBackRef;

    Optional<MTLViewport> viewport;
    Optional<MTLScissorRect> scissorRect;

    std::array<float, 4> blendColor;

    gl::ShaderMap<RenderCommandEncoderShaderStates> perShaderStates;

    MTLVisibilityResultMode visibilityResultMode;
    size_t visibilityResultBufferOffset;
};

// Encoder for encoding render commands
class RenderCommandEncoder final : public CommandEncoder
{
  public:
    RenderCommandEncoder(CommandBuffer *cmdBuffer,
                         const OcclusionQueryPool &queryPool,
                         bool emulateDontCareLoadOpWithRandomClear);
    ~RenderCommandEncoder() override;

    // override CommandEncoder
    bool valid() const { return mRecording; }
    void reset() override;
    void endEncoding() override;

    // Restart the encoder so that new commands can be encoded.
    // NOTE: parent CommandBuffer's restart() must be called before this.
    RenderCommandEncoder &restart(const RenderPassDesc &desc, uint32_t deviceMaxRenderTargets);

    RenderCommandEncoder &setRenderPipelineState(id<MTLRenderPipelineState> state);
    RenderCommandEncoder &setTriangleFillMode(MTLTriangleFillMode mode);
    RenderCommandEncoder &setFrontFacingWinding(MTLWinding winding);
    RenderCommandEncoder &setCullMode(MTLCullMode mode);

    RenderCommandEncoder &setDepthStencilState(id<MTLDepthStencilState> state);
    RenderCommandEncoder &setDepthBias(float depthBias, float slopeScale, float clamp);
    RenderCommandEncoder &setDepthClipMode(MTLDepthClipMode depthClipMode);
    RenderCommandEncoder &setStencilRefVals(uint32_t frontRef, uint32_t backRef);
    RenderCommandEncoder &setStencilRefVal(uint32_t ref);

    RenderCommandEncoder &setViewport(const MTLViewport &viewport);
    RenderCommandEncoder &setScissorRect(const MTLScissorRect &rect);

    RenderCommandEncoder &setBlendColor(float r, float g, float b, float a);

    RenderCommandEncoder &setVertexBuffer(const BufferRef &buffer, uint32_t offset, uint32_t index)
    {
        return setBuffer(gl::ShaderType::Vertex, buffer, offset, index);
    }
    RenderCommandEncoder &setVertexBytes(const uint8_t *bytes, size_t size, uint32_t index)
    {
        return setBytes(gl::ShaderType::Vertex, bytes, size, index);
    }
    template <typename T>
    RenderCommandEncoder &setVertexData(const T &data, uint32_t index)
    {
        return setVertexBytes(reinterpret_cast<const uint8_t *>(&data), sizeof(T), index);
    }
    RenderCommandEncoder &setVertexSamplerState(id<MTLSamplerState> state,
                                                float lodMinClamp,
                                                float lodMaxClamp,
                                                uint32_t index)
    {
        return setSamplerState(gl::ShaderType::Vertex, state, lodMinClamp, lodMaxClamp, index);
    }
    RenderCommandEncoder &setVertexTexture(const TextureRef &texture, uint32_t index)
    {
        return setTexture(gl::ShaderType::Vertex, texture, index);
    }

    RenderCommandEncoder &setFragmentBuffer(const BufferRef &buffer,
                                            uint32_t offset,
                                            uint32_t index)
    {
        return setBuffer(gl::ShaderType::Fragment, buffer, offset, index);
    }
    RenderCommandEncoder &setFragmentBytes(const uint8_t *bytes, size_t size, uint32_t index)
    {
        return setBytes(gl::ShaderType::Fragment, bytes, size, index);
    }
    template <typename T>
    RenderCommandEncoder &setFragmentData(const T &data, uint32_t index)
    {
        return setFragmentBytes(reinterpret_cast<const uint8_t *>(&data), sizeof(T), index);
    }
    RenderCommandEncoder &setFragmentSamplerState(id<MTLSamplerState> state,
                                                  float lodMinClamp,
                                                  float lodMaxClamp,
                                                  uint32_t index)
    {
        return setSamplerState(gl::ShaderType::Fragment, state, lodMinClamp, lodMaxClamp, index);
    }
    RenderCommandEncoder &setFragmentTexture(const TextureRef &texture, uint32_t index)
    {
        return setTexture(gl::ShaderType::Fragment, texture, index);
    }

    RenderCommandEncoder &setBuffer(gl::ShaderType shaderType,
                                    const BufferRef &buffer,
                                    uint32_t offset,
                                    uint32_t index);
    RenderCommandEncoder &setBufferForWrite(gl::ShaderType shaderType,
                                            const BufferRef &buffer,
                                            uint32_t offset,
                                            uint32_t index);
    RenderCommandEncoder &setBytes(gl::ShaderType shaderType,
                                   const uint8_t *bytes,
                                   size_t size,
                                   uint32_t index);
    template <typename T>
    RenderCommandEncoder &setData(gl::ShaderType shaderType, const T &data, uint32_t index)
    {
        return setBytes(shaderType, reinterpret_cast<const uint8_t *>(&data), sizeof(T), index);
    }
    RenderCommandEncoder &setSamplerState(gl::ShaderType shaderType,
                                          id<MTLSamplerState> state,
                                          float lodMinClamp,
                                          float lodMaxClamp,
                                          uint32_t index);
    RenderCommandEncoder &setTexture(gl::ShaderType shaderType,
                                     const TextureRef &texture,
                                     uint32_t index);
    RenderCommandEncoder &setRWTexture(gl::ShaderType, const TextureRef &, uint32_t index);

    RenderCommandEncoder &draw(MTLPrimitiveType primitiveType,
                               uint32_t vertexStart,
                               uint32_t vertexCount);
    RenderCommandEncoder &drawInstanced(MTLPrimitiveType primitiveType,
                                        uint32_t vertexStart,
                                        uint32_t vertexCount,
                                        uint32_t instances);
    RenderCommandEncoder &drawInstancedBaseInstance(MTLPrimitiveType primitiveType,
                                                    uint32_t vertexStart,
                                                    uint32_t vertexCount,
                                                    uint32_t instances,
                                                    uint32_t baseInstance);
    RenderCommandEncoder &drawIndexed(MTLPrimitiveType primitiveType,
                                      uint32_t indexCount,
                                      MTLIndexType indexType,
                                      const BufferRef &indexBuffer,
                                      size_t bufferOffset);
    RenderCommandEncoder &drawIndexedInstanced(MTLPrimitiveType primitiveType,
                                               uint32_t indexCount,
                                               MTLIndexType indexType,
                                               const BufferRef &indexBuffer,
                                               size_t bufferOffset,
                                               uint32_t instances);
    RenderCommandEncoder &drawIndexedInstancedBaseVertexBaseInstance(MTLPrimitiveType primitiveType,
                                                                     uint32_t indexCount,
                                                                     MTLIndexType indexType,
                                                                     const BufferRef &indexBuffer,
                                                                     size_t bufferOffset,
                                                                     uint32_t instances,
                                                                     uint32_t baseVertex,
                                                                     uint32_t baseInstance);

    RenderCommandEncoder &setVisibilityResultMode(MTLVisibilityResultMode mode, size_t offset);

    RenderCommandEncoder &useResource(const BufferRef &resource,
                                      MTLResourceUsage usage,
                                      MTLRenderStages stages);

    RenderCommandEncoder &memoryBarrier(MTLBarrierScope scope,
                                        MTLRenderStages after,
                                        MTLRenderStages before);

    RenderCommandEncoder &memoryBarrierWithResource(const BufferRef &resource,
                                                    MTLRenderStages after,
                                                    MTLRenderStages before);

    RenderCommandEncoder &setColorStoreAction(MTLStoreAction action, uint32_t colorAttachmentIndex);
    // Set store action for every color attachment.
    RenderCommandEncoder &setColorStoreAction(MTLStoreAction action);

    RenderCommandEncoder &setDepthStencilStoreAction(MTLStoreAction depthStoreAction,
                                                     MTLStoreAction stencilStoreAction);
    RenderCommandEncoder &setDepthStoreAction(MTLStoreAction action);
    RenderCommandEncoder &setStencilStoreAction(MTLStoreAction action);

    // Set storeaction for every color & depth & stencil attachment.
    RenderCommandEncoder &setStoreAction(MTLStoreAction action);

    // Change the render pass's loadAction. Note that this operation is only allowed when there
    // is no draw call recorded yet.
    RenderCommandEncoder &setColorLoadAction(MTLLoadAction action,
                                             const MTLClearColor &clearValue,
                                             uint32_t colorAttachmentIndex);
    RenderCommandEncoder &setDepthLoadAction(MTLLoadAction action, double clearValue);
    RenderCommandEncoder &setStencilLoadAction(MTLLoadAction action, uint32_t clearValue);

    void setLabel(NSString *label);

    void pushDebugGroup(NSString *label) override;
    void popDebugGroup() override;

    const RenderPassDesc &renderPassDesc() const { return mRenderPassDesc; }
    bool hasDrawCalls() const { return mHasDrawCalls; }

    uint64_t getSerial() const { return mSerial; }

  private:
    // Override CommandEncoder
    id<MTLRenderCommandEncoder> get()
    {
        return static_cast<id<MTLRenderCommandEncoder>>(CommandEncoder::get());
    }
    void insertDebugSignImpl(NSString *label) override;

    void initAttachmentWriteDependencyAndScissorRect(const RenderPassAttachmentDesc &attachment);
    void initWriteDependency(const TextureRef &texture);

    template <typename ObjCAttachmentDescriptor>
    bool finalizeLoadStoreAction(const RenderPassAttachmentDesc &cppRenderPassAttachment,
                                 ObjCAttachmentDescriptor *objCRenderPassAttachment);

    void encodeMetalEncoder();
    void simulateDiscardFramebuffer();
    void endEncodingImpl(bool considerDiscardSimulation);

    RenderCommandEncoder &commonSetBuffer(gl::ShaderType shaderType,
                                          id<MTLBuffer> mtlBuffer,
                                          uint32_t offset,
                                          uint32_t index);

    RenderPassDesc mRenderPassDesc;
    // Cached Objective-C render pass desc to avoid re-allocate every frame.
    mtl::AutoObjCPtr<MTLRenderPassDescriptor *> mCachedRenderPassDescObjC;

    mtl::AutoObjCPtr<NSString *> mLabel;

    MTLScissorRect mRenderPassMaxScissorRect;

    const OcclusionQueryPool &mOcclusionQueryPool;
    bool mRecording    = false;
    bool mHasDrawCalls = false;
    IntermediateCommandStream mCommands;

    gl::ShaderMap<uint8_t> mSetBufferCmds;
    gl::ShaderMap<uint8_t> mSetBufferOffsetCmds;
    gl::ShaderMap<uint8_t> mSetBytesCmds;
    gl::ShaderMap<uint8_t> mSetTextureCmds;
    gl::ShaderMap<uint8_t> mSetSamplerCmds;

    RenderCommandEncoderStates mStateCache = {};

    bool mPipelineStateSet = false;
    uint64_t mSerial       = 0;

    const bool mEmulateDontCareLoadOpWithRandomClear;
};

class BlitCommandEncoder final : public CommandEncoder
{
  public:
    BlitCommandEncoder(CommandBuffer *cmdBuffer);
    ~BlitCommandEncoder() override;

    // Restart the encoder so that new commands can be encoded.
    // NOTE: parent CommandBuffer's restart() must be called before this.
    BlitCommandEncoder &restart();

    BlitCommandEncoder &copyBuffer(const BufferRef &src,
                                   size_t srcOffset,
                                   const BufferRef &dst,
                                   size_t dstOffset,
                                   size_t size);

    BlitCommandEncoder &copyBufferToTexture(const BufferRef &src,
                                            size_t srcOffset,
                                            size_t srcBytesPerRow,
                                            size_t srcBytesPerImage,
                                            MTLSize srcSize,
                                            const TextureRef &dst,
                                            uint32_t dstSlice,
                                            MipmapNativeLevel dstLevel,
                                            MTLOrigin dstOrigin,
                                            MTLBlitOption blitOption);

    BlitCommandEncoder &copyTextureToBuffer(const TextureRef &src,
                                            uint32_t srcSlice,
                                            MipmapNativeLevel srcLevel,
                                            MTLOrigin srcOrigin,
                                            MTLSize srcSize,
                                            const BufferRef &dst,
                                            size_t dstOffset,
                                            size_t dstBytesPerRow,
                                            size_t dstBytesPerImage,
                                            MTLBlitOption blitOption);

    BlitCommandEncoder &copyTexture(const TextureRef &src,
                                    uint32_t srcSlice,
                                    MipmapNativeLevel srcLevel,
                                    const TextureRef &dst,
                                    uint32_t dstSlice,
                                    MipmapNativeLevel dstLevel,
                                    uint32_t sliceCount,
                                    uint32_t levelCount);

    BlitCommandEncoder &fillBuffer(const BufferRef &buffer, NSRange range, uint8_t value);

    BlitCommandEncoder &generateMipmapsForTexture(const TextureRef &texture);
    BlitCommandEncoder &synchronizeResource(Buffer *bufferPtr);
    BlitCommandEncoder &synchronizeResource(Texture *texturePtr);

  private:
    id<MTLBlitCommandEncoder> get()
    {
        return static_cast<id<MTLBlitCommandEncoder>>(CommandEncoder::get());
    }
};

class ComputeCommandEncoder final : public CommandEncoder
{
  public:
    ComputeCommandEncoder(CommandBuffer *cmdBuffer);
    ~ComputeCommandEncoder() override;

    // Restart the encoder so that new commands can be encoded.
    // NOTE: parent CommandBuffer's restart() must be called before this.
    ComputeCommandEncoder &restart();

    ComputeCommandEncoder &setComputePipelineState(id<MTLComputePipelineState> state);

    ComputeCommandEncoder &setBuffer(const BufferRef &buffer, uint32_t offset, uint32_t index);
    ComputeCommandEncoder &setBufferForWrite(const BufferRef &buffer,
                                             uint32_t offset,
                                             uint32_t index);
    ComputeCommandEncoder &setBytes(const uint8_t *bytes, size_t size, uint32_t index);
    template <typename T>
    ComputeCommandEncoder &setData(const T &data, uint32_t index)
    {
        return setBytes(reinterpret_cast<const uint8_t *>(&data), sizeof(T), index);
    }
    ComputeCommandEncoder &setSamplerState(id<MTLSamplerState> state,
                                           float lodMinClamp,
                                           float lodMaxClamp,
                                           uint32_t index);
    ComputeCommandEncoder &setTexture(const TextureRef &texture, uint32_t index);
    ComputeCommandEncoder &setTextureForWrite(const TextureRef &texture, uint32_t index);

    ComputeCommandEncoder &dispatch(const MTLSize &threadGroupsPerGrid,
                                    const MTLSize &threadsPerGroup);

    ComputeCommandEncoder &dispatchNonUniform(const MTLSize &threadsPerGrid,
                                              const MTLSize &threadsPerGroup);

  private:
    id<MTLComputeCommandEncoder> get()
    {
        return static_cast<id<MTLComputeCommandEncoder>>(CommandEncoder::get());
    }
};

}  // namespace mtl
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_COMMANDENBUFFERMTL_H_ */
