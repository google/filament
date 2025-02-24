//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SecondaryCommandBuffer:
//    Lightweight, CPU-Side command buffers used to hold command state until
//    it has to be submitted to GPU.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDBUFFERVK_H_
#define LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDBUFFERVK_H_

#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/vk_command_buffer_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

#if ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC
#    include "libANGLE/renderer/vulkan/AllocatorHelperRing.h"
#else
#    include "libANGLE/renderer/vulkan/AllocatorHelperPool.h"
#endif

namespace rx
{
class ContextVk;

namespace vk
{
class ErrorContext;
class RenderPassDesc;
class SecondaryCommandPool;

#if ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC
using SecondaryCommandMemoryAllocator = SharedCommandMemoryAllocator;
using SecondaryCommandBlockPool       = SharedCommandBlockPool;
using SecondaryCommandBlockAllocator  = SharedCommandBlockAllocator;
#else
using SecondaryCommandMemoryAllocator = DedicatedCommandMemoryAllocator;
using SecondaryCommandBlockPool       = DedicatedCommandBlockPool;
using SecondaryCommandBlockAllocator  = DedicatedCommandBlockAllocator;
#endif

namespace priv
{

// NOTE: Please keep command-related enums, structs, functions and other code dealing with commands
// in alphabetical order.
// This simplifies searching and updating commands.
enum class CommandID : uint16_t
{
    // Invalid cmd used to mark end of sequence of commands
    Invalid = 0,
    BeginDebugUtilsLabel,
    BeginQuery,
    BeginTransformFeedback,
    BindComputePipeline,
    BindDescriptorSets,
    BindGraphicsPipeline,
    BindIndexBuffer,
    BindTransformFeedbackBuffers,
    BindVertexBuffers,
    BindVertexBuffers2,
    BlitImage,
    BufferBarrier,
    BufferBarrier2,
    ClearAttachments,
    ClearColorImage,
    ClearDepthStencilImage,
    CopyBuffer,
    CopyBufferToImage,
    CopyImage,
    CopyImageToBuffer,
    Dispatch,
    DispatchIndirect,
    Draw,
    DrawIndexed,
    DrawIndexedBaseVertex,
    DrawIndexedIndirect,
    DrawIndexedInstanced,
    DrawIndexedInstancedBaseVertex,
    DrawIndexedInstancedBaseVertexBaseInstance,
    DrawIndirect,
    DrawInstanced,
    DrawInstancedBaseInstance,
    EndDebugUtilsLabel,
    EndQuery,
    EndTransformFeedback,
    FillBuffer,
    ImageBarrier,
    ImageBarrier2,
    ImageWaitEvent,
    InsertDebugUtilsLabel,
    MemoryBarrier,
    MemoryBarrier2,
    NextSubpass,
    PipelineBarrier,
    PipelineBarrier2,
    PushConstants,
    ResetEvent,
    ResetQueryPool,
    ResolveImage,
    SetBlendConstants,
    SetCullMode,
    SetDepthBias,
    SetDepthBiasEnable,
    SetDepthCompareOp,
    SetDepthTestEnable,
    SetDepthWriteEnable,
    SetEvent,
    SetFragmentShadingRate,
    SetFrontFace,
    SetLineWidth,
    SetLogicOp,
    SetPrimitiveRestartEnable,
    SetRasterizerDiscardEnable,
    SetScissor,
    SetStencilCompareMask,
    SetStencilOp,
    SetStencilReference,
    SetStencilTestEnable,
    SetStencilWriteMask,
    SetVertexInput,
    SetViewport,
    WaitEvents,
    WriteTimestamp,
    WriteTimestamp2,
};

// Header for every cmd in custom cmd buffer
struct CommandHeader
{
    CommandID id;
    uint16_t size;
};
static_assert(sizeof(CommandHeader) == 4, "Check CommandHeader size");

#define VERIFY_8_BYTE_ALIGNMENT(StructName) \
    static_assert((sizeof(StructName) % 8) == 0, "Check " #StructName " alignment");

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

// Structs to encapsulate parameters for different commands.  This makes it easy to know the size of
// params & to copy params.  Each struct must be prefixed by a CommandHeader (which is 4 bytes) and
// must be aligned to 8 bytes.
struct BeginQueryParams
{
    CommandHeader header;

    uint32_t query;
    VkQueryPool queryPool;
};
VERIFY_8_BYTE_ALIGNMENT(BeginQueryParams)

struct BeginTransformFeedbackParams
{
    CommandHeader header;

    uint32_t bufferCount;
};
VERIFY_8_BYTE_ALIGNMENT(BeginTransformFeedbackParams)

struct BindDescriptorSetParams
{
    CommandHeader header;

    VkPipelineBindPoint pipelineBindPoint : 8;
    uint32_t firstSet : 8;
    uint32_t descriptorSetCount : 8;
    uint32_t dynamicOffsetCount : 8;

    VkPipelineLayout layout;
};
VERIFY_8_BYTE_ALIGNMENT(BindDescriptorSetParams)

struct BindIndexBufferParams
{
    CommandHeader header;

    VkIndexType indexType;
    VkBuffer buffer;
    VkDeviceSize offset;
};
VERIFY_8_BYTE_ALIGNMENT(BindIndexBufferParams)

struct BindPipelineParams
{
    CommandHeader header;

    uint32_t padding;
    VkPipeline pipeline;
};
VERIFY_8_BYTE_ALIGNMENT(BindPipelineParams)

struct BindTransformFeedbackBuffersParams
{
    CommandHeader header;

    // ANGLE always has firstBinding of 0 so not storing that currently
    uint32_t bindingCount;
};
VERIFY_8_BYTE_ALIGNMENT(BindTransformFeedbackBuffersParams)

using BindVertexBuffersParams  = BindTransformFeedbackBuffersParams;
using BindVertexBuffers2Params = BindVertexBuffersParams;

struct BlitImageParams
{
    CommandHeader header;

    VkFilter filter;
    VkImage srcImage;
    VkImage dstImage;
    VkImageBlit region;
};
VERIFY_8_BYTE_ALIGNMENT(BlitImageParams)

struct BufferBarrierParams
{
    CommandHeader header;

    uint32_t padding;
    VkBufferMemoryBarrier bufferMemoryBarrier;
};
VERIFY_8_BYTE_ALIGNMENT(BufferBarrierParams)

struct BufferBarrier2Params
{
    CommandHeader header;
    uint32_t padding;
    VkBufferMemoryBarrier2 bufferMemoryBarrier2;
};
VERIFY_8_BYTE_ALIGNMENT(BufferBarrier2Params)

struct ClearAttachmentsParams
{
    CommandHeader header;

    uint32_t attachmentCount;
    VkClearRect rect;
};
VERIFY_8_BYTE_ALIGNMENT(ClearAttachmentsParams)

struct ClearColorImageParams
{
    CommandHeader header;

    VkImageLayout imageLayout;
    VkImage image;
    VkClearColorValue color;
    VkImageSubresourceRange range;
    uint32_t padding;
};
VERIFY_8_BYTE_ALIGNMENT(ClearColorImageParams)

struct ClearDepthStencilImageParams
{
    CommandHeader header;

    VkImageLayout imageLayout;
    VkImage image;
    VkClearDepthStencilValue depthStencil;
    VkImageSubresourceRange range;
    uint32_t padding;
};
VERIFY_8_BYTE_ALIGNMENT(ClearDepthStencilImageParams)

struct CopyBufferParams
{
    CommandHeader header;

    uint32_t regionCount;
    VkBuffer srcBuffer;
    VkBuffer destBuffer;
};
VERIFY_8_BYTE_ALIGNMENT(CopyBufferParams)

struct CopyBufferToImageParams
{
    CommandHeader header;

    VkImageLayout dstImageLayout;
    VkBuffer srcBuffer;
    VkImage dstImage;
    VkBufferImageCopy region;
};
VERIFY_8_BYTE_ALIGNMENT(CopyBufferToImageParams)

struct CopyImageParams
{
    CommandHeader header;

    VkImageCopy region;
    VkImageLayout srcImageLayout;
    VkImageLayout dstImageLayout;
    VkImage srcImage;
    VkImage dstImage;
};
VERIFY_8_BYTE_ALIGNMENT(CopyImageParams)

struct CopyImageToBufferParams
{
    CommandHeader header;

    VkImageLayout srcImageLayout;
    VkImage srcImage;
    VkBuffer dstBuffer;
    VkBufferImageCopy region;
};
VERIFY_8_BYTE_ALIGNMENT(CopyImageToBufferParams)

// This is a common struct used by both begin & insert DebugUtilsLabelEXT() functions
struct DebugUtilsLabelParams
{
    CommandHeader header;

    uint32_t padding;
    float color[4];
};
VERIFY_8_BYTE_ALIGNMENT(DebugUtilsLabelParams)

struct DispatchParams
{
    CommandHeader header;

    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
};
VERIFY_8_BYTE_ALIGNMENT(DispatchParams)

struct DispatchIndirectParams
{
    CommandHeader header;

    uint32_t padding;
    VkBuffer buffer;
    VkDeviceSize offset;
};
VERIFY_8_BYTE_ALIGNMENT(DispatchIndirectParams)

struct DrawParams
{
    CommandHeader header;

    uint32_t padding;
    uint32_t vertexCount;
    uint32_t firstVertex;
};
VERIFY_8_BYTE_ALIGNMENT(DrawParams)

struct DrawIndexedParams
{
    CommandHeader header;

    uint32_t indexCount;
};
VERIFY_8_BYTE_ALIGNMENT(DrawIndexedParams)

struct DrawIndexedBaseVertexParams
{
    CommandHeader header;

    uint32_t padding;
    uint32_t indexCount;
    uint32_t vertexOffset;
};
VERIFY_8_BYTE_ALIGNMENT(DrawIndexedBaseVertexParams)

struct DrawIndexedIndirectParams
{
    CommandHeader header;

    uint32_t padding;
    uint32_t drawCount;
    uint32_t stride;
    VkBuffer buffer;
    VkDeviceSize offset;
};
VERIFY_8_BYTE_ALIGNMENT(DrawIndexedIndirectParams)

struct DrawIndexedInstancedParams
{
    CommandHeader header;

    uint32_t padding;
    uint32_t indexCount;
    uint32_t instanceCount;
};
VERIFY_8_BYTE_ALIGNMENT(DrawIndexedInstancedParams)

struct DrawIndexedInstancedBaseVertexParams
{
    CommandHeader header;

    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t vertexOffset;
};
VERIFY_8_BYTE_ALIGNMENT(DrawIndexedInstancedBaseVertexParams)

struct DrawIndexedInstancedBaseVertexBaseInstanceParams
{
    CommandHeader header;

    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
    uint32_t firstInstance;
};
VERIFY_8_BYTE_ALIGNMENT(DrawIndexedInstancedBaseVertexBaseInstanceParams)

struct DrawIndirectParams
{
    CommandHeader header;

    uint32_t padding;
    uint32_t drawCount;
    uint32_t stride;
    VkBuffer buffer;
    VkDeviceSize offset;
};
VERIFY_8_BYTE_ALIGNMENT(DrawIndirectParams)

struct DrawInstancedParams
{
    CommandHeader header;

    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
};
VERIFY_8_BYTE_ALIGNMENT(DrawInstancedParams)

struct DrawInstancedBaseInstanceParams
{
    CommandHeader header;

    uint32_t padding;
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};
VERIFY_8_BYTE_ALIGNMENT(DrawInstancedBaseInstanceParams)

// A special struct used with commands that don't have params
struct EmptyParams
{
    CommandHeader header;
    uint32_t padding;
};
VERIFY_8_BYTE_ALIGNMENT(EmptyParams)

struct EndQueryParams
{
    CommandHeader header;

    uint32_t query;
    VkQueryPool queryPool;
};
VERIFY_8_BYTE_ALIGNMENT(EndQueryParams)

struct EndTransformFeedbackParams
{
    CommandHeader header;

    uint32_t bufferCount;
};
VERIFY_8_BYTE_ALIGNMENT(EndTransformFeedbackParams)

struct FillBufferParams
{
    CommandHeader header;

    uint32_t data;
    VkBuffer dstBuffer;
    VkDeviceSize dstOffset;
    VkDeviceSize size;
};
VERIFY_8_BYTE_ALIGNMENT(FillBufferParams)

struct ImageBarrierParams
{
    CommandHeader header;

    uint32_t padding;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
};
VERIFY_8_BYTE_ALIGNMENT(ImageBarrierParams)

struct ImageBarrier2Params
{
    CommandHeader header;

    uint32_t padding;
};
VERIFY_8_BYTE_ALIGNMENT(ImageBarrier2Params)

struct ImageWaitEventParams
{
    CommandHeader header;

    uint32_t padding;
    VkEvent event;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
};
VERIFY_8_BYTE_ALIGNMENT(ImageWaitEventParams)

struct MemoryBarrierParams
{
    CommandHeader header;

    uint32_t padding;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
};
VERIFY_8_BYTE_ALIGNMENT(MemoryBarrierParams)

struct MemoryBarrier2Params
{
    CommandHeader header;

    uint32_t padding;
};
VERIFY_8_BYTE_ALIGNMENT(MemoryBarrierParams)

struct PipelineBarrierParams
{
    CommandHeader header;

    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    VkDependencyFlags dependencyFlags;
    uint32_t memoryBarrierCount;
    uint32_t imageMemoryBarrierCount;
};
VERIFY_8_BYTE_ALIGNMENT(PipelineBarrierParams)

struct PipelineBarrierParams2
{
    CommandHeader header;
    VkDependencyFlags dependencyFlags;
    uint32_t memoryBarrierCount;
    uint32_t imageMemoryBarrierCount;
};
VERIFY_8_BYTE_ALIGNMENT(PipelineBarrierParams2)

struct PushConstantsParams
{
    CommandHeader header;

    VkShaderStageFlags flag;
    VkPipelineLayout layout;
    uint32_t offset;
    uint32_t size;
};
VERIFY_8_BYTE_ALIGNMENT(PushConstantsParams)

struct ResetEventParams
{
    CommandHeader header;

    VkPipelineStageFlags stageMask;
    VkEvent event;
};
VERIFY_8_BYTE_ALIGNMENT(ResetEventParams)

struct ResetQueryPoolParams
{
    CommandHeader header;

    uint32_t firstQuery : 24;
    uint32_t queryCount : 8;
    VkQueryPool queryPool;
};
VERIFY_8_BYTE_ALIGNMENT(ResetQueryPoolParams)

struct ResolveImageParams
{
    CommandHeader header;

    VkImageResolve region;
    VkImage srcImage;
    VkImage dstImage;
};
VERIFY_8_BYTE_ALIGNMENT(ResolveImageParams)

struct SetBlendConstantsParams
{
    CommandHeader header;

    uint32_t padding;
    float blendConstants[4];
};
VERIFY_8_BYTE_ALIGNMENT(SetBlendConstantsParams)

struct SetCullModeParams
{
    CommandHeader header;

    VkCullModeFlags cullMode;
};
VERIFY_8_BYTE_ALIGNMENT(SetCullModeParams)

struct SetDepthBiasParams
{
    CommandHeader header;

    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
};
VERIFY_8_BYTE_ALIGNMENT(SetDepthBiasParams)

struct SetDepthBiasEnableParams
{
    CommandHeader header;

    VkBool32 depthBiasEnable;
};
VERIFY_8_BYTE_ALIGNMENT(SetDepthBiasEnableParams)

struct SetDepthCompareOpParams
{
    CommandHeader header;

    VkCompareOp depthCompareOp;
};
VERIFY_8_BYTE_ALIGNMENT(SetDepthCompareOpParams)

struct SetDepthTestEnableParams
{
    CommandHeader header;

    VkBool32 depthTestEnable;
};
VERIFY_8_BYTE_ALIGNMENT(SetDepthTestEnableParams)

struct SetDepthWriteEnableParams
{
    CommandHeader header;

    VkBool32 depthWriteEnable;
};
VERIFY_8_BYTE_ALIGNMENT(SetDepthWriteEnableParams)

struct SetEventParams
{
    CommandHeader header;

    VkPipelineStageFlags stageMask;
    VkEvent event;
};
VERIFY_8_BYTE_ALIGNMENT(SetEventParams)

struct SetFragmentShadingRateParams
{
    CommandHeader header;

    uint32_t fragmentWidth : 8;
    uint32_t fragmentHeight : 8;
    uint32_t vkFragmentShadingRateCombinerOp1 : 16;
};
VERIFY_8_BYTE_ALIGNMENT(SetFragmentShadingRateParams)

struct SetFrontFaceParams
{
    CommandHeader header;

    VkFrontFace frontFace;
};
VERIFY_8_BYTE_ALIGNMENT(SetFrontFaceParams)

struct SetLineWidthParams
{
    CommandHeader header;

    float lineWidth;
};
VERIFY_8_BYTE_ALIGNMENT(SetLineWidthParams)

struct SetLogicOpParams
{
    CommandHeader header;

    VkLogicOp logicOp;
};
VERIFY_8_BYTE_ALIGNMENT(SetLogicOpParams)

struct SetPrimitiveRestartEnableParams
{
    CommandHeader header;

    VkBool32 primitiveRestartEnable;
};
VERIFY_8_BYTE_ALIGNMENT(SetPrimitiveRestartEnableParams)

struct SetRasterizerDiscardEnableParams
{
    CommandHeader header;

    VkBool32 rasterizerDiscardEnable;
};
VERIFY_8_BYTE_ALIGNMENT(SetRasterizerDiscardEnableParams)

struct SetScissorParams
{
    CommandHeader header;

    uint32_t padding;
    VkRect2D scissor;
};
VERIFY_8_BYTE_ALIGNMENT(SetScissorParams)

struct SetStencilCompareMaskParams
{
    CommandHeader header;

    uint16_t compareFrontMask;
    uint16_t compareBackMask;
};
VERIFY_8_BYTE_ALIGNMENT(SetStencilCompareMaskParams)

struct SetStencilOpParams
{
    CommandHeader header;

    uint32_t faceMask : 4;
    uint32_t failOp : 3;
    uint32_t passOp : 3;
    uint32_t depthFailOp : 3;
    uint32_t compareOp : 3;
    uint32_t padding : 16;
};
VERIFY_8_BYTE_ALIGNMENT(SetStencilOpParams)

struct SetStencilReferenceParams
{
    CommandHeader header;

    uint16_t frontReference;
    uint16_t backReference;
};
VERIFY_8_BYTE_ALIGNMENT(SetStencilReferenceParams)

struct SetStencilTestEnableParams
{
    CommandHeader header;

    VkBool32 stencilTestEnable;
};
VERIFY_8_BYTE_ALIGNMENT(SetStencilTestEnableParams)

struct SetStencilWriteMaskParams
{
    CommandHeader header;

    uint16_t writeFrontMask;
    uint16_t writeBackMask;
};
VERIFY_8_BYTE_ALIGNMENT(SetStencilWriteMaskParams)

struct SetVertexInputParams
{
    CommandHeader header;

    uint16_t vertexBindingDescriptionCount;
    uint16_t vertexAttributeDescriptionCount;
};
VERIFY_8_BYTE_ALIGNMENT(SetVertexInputParams)

struct SetViewportParams
{
    CommandHeader header;

    uint32_t padding;
    VkViewport viewport;
};
VERIFY_8_BYTE_ALIGNMENT(SetViewportParams)

struct WaitEventsParams
{
    CommandHeader header;

    uint32_t eventCount;
    VkPipelineStageFlags srcStageMask;
    VkPipelineStageFlags dstStageMask;
    uint32_t memoryBarrierCount;
    uint32_t imageMemoryBarrierCount;
};
VERIFY_8_BYTE_ALIGNMENT(WaitEventsParams)

struct WriteTimestampParams
{
    CommandHeader header;

    uint32_t query;
    VkQueryPool queryPool;
};
VERIFY_8_BYTE_ALIGNMENT(WriteTimestampParams)

#undef VERIFY_8_BYTE_ALIGNMENT

ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

template <typename DestT, typename T>
ANGLE_INLINE DestT *Offset(T *ptr, size_t bytes)
{
    return reinterpret_cast<DestT *>((reinterpret_cast<uint8_t *>(ptr) + bytes));
}

template <typename DestT, typename T>
ANGLE_INLINE const DestT *Offset(const T *ptr, size_t bytes)
{
    return reinterpret_cast<const DestT *>((reinterpret_cast<const uint8_t *>(ptr) + bytes));
}

class SecondaryCommandBuffer final : angle::NonCopyable
{
  public:
    SecondaryCommandBuffer();
    ~SecondaryCommandBuffer();

    static bool SupportsQueries(const VkPhysicalDeviceFeatures &features) { return true; }

    // SecondaryCommandBuffer replays its commands inline when executed on the primary command
    // buffer.
    static constexpr bool ExecutesInline() { return true; }

    static angle::Result InitializeCommandPool(ErrorContext *context,
                                               SecondaryCommandPool *pool,
                                               uint32_t queueFamilyIndex,
                                               ProtectionType protectionType)
    {
        return angle::Result::Continue;
    }
    static angle::Result InitializeRenderPassInheritanceInfo(
        ContextVk *contextVk,
        const Framebuffer &framebuffer,
        const RenderPassDesc &renderPassDesc,
        VkCommandBufferInheritanceInfo *inheritanceInfoOut,
        VkCommandBufferInheritanceRenderingInfo *renderingInfoOut,
        gl::DrawBuffersArray<VkFormat> *colorFormatStorageOut)
    {
        *inheritanceInfoOut = {};
        return angle::Result::Continue;
    }

    // Add commands
    void beginDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT &label);

    void beginQuery(const QueryPool &queryPool, uint32_t query, VkQueryControlFlags flags);

    void beginTransformFeedback(uint32_t firstCounterBuffer,
                                uint32_t bufferCount,
                                const VkBuffer *counterBuffers,
                                const VkDeviceSize *counterBufferOffsets);

    void bindComputePipeline(const Pipeline &pipeline);

    void bindDescriptorSets(const PipelineLayout &layout,
                            VkPipelineBindPoint pipelineBindPoint,
                            DescriptorSetIndex firstSet,
                            uint32_t descriptorSetCount,
                            const VkDescriptorSet *descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t *dynamicOffsets);

    void bindGraphicsPipeline(const Pipeline &pipeline);

    void bindIndexBuffer(const Buffer &buffer, VkDeviceSize offset, VkIndexType indexType);

    void bindTransformFeedbackBuffers(uint32_t firstBinding,
                                      uint32_t bindingCount,
                                      const VkBuffer *buffers,
                                      const VkDeviceSize *offsets,
                                      const VkDeviceSize *sizes);

    void bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const VkBuffer *buffers,
                           const VkDeviceSize *offsets);

    void bindVertexBuffers2(uint32_t firstBinding,
                            uint32_t bindingCount,
                            const VkBuffer *buffers,
                            const VkDeviceSize *offsets,
                            const VkDeviceSize *sizes,
                            const VkDeviceSize *strides);

    void blitImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageBlit *regions,
                   VkFilter filter);

    void bufferBarrier(VkPipelineStageFlags srcStageMask,
                       VkPipelineStageFlags dstStageMask,
                       const VkBufferMemoryBarrier *bufferMemoryBarrier);

    void bufferBarrier2(const VkBufferMemoryBarrier2 *bufferMemoryBarrier);

    void clearAttachments(uint32_t attachmentCount,
                          const VkClearAttachment *attachments,
                          uint32_t rectCount,
                          const VkClearRect *rects);

    void clearColorImage(const Image &image,
                         VkImageLayout imageLayout,
                         const VkClearColorValue &color,
                         uint32_t rangeCount,
                         const VkImageSubresourceRange *ranges);

    void clearDepthStencilImage(const Image &image,
                                VkImageLayout imageLayout,
                                const VkClearDepthStencilValue &depthStencil,
                                uint32_t rangeCount,
                                const VkImageSubresourceRange *ranges);

    void copyBuffer(const Buffer &srcBuffer,
                    const Buffer &destBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy *regions);

    void copyBufferToImage(VkBuffer srcBuffer,
                           const Image &dstImage,
                           VkImageLayout dstImageLayout,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);

    void copyImage(const Image &srcImage,
                   VkImageLayout srcImageLayout,
                   const Image &dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageCopy *regions);

    void copyImageToBuffer(const Image &srcImage,
                           VkImageLayout srcImageLayout,
                           VkBuffer dstBuffer,
                           uint32_t regionCount,
                           const VkBufferImageCopy *regions);

    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    void dispatchIndirect(const Buffer &buffer, VkDeviceSize offset);

    void draw(uint32_t vertexCount, uint32_t firstVertex);

    void drawIndexed(uint32_t indexCount);
    void drawIndexedBaseVertex(uint32_t indexCount, uint32_t vertexOffset);
    void drawIndexedIndirect(const Buffer &buffer,
                             VkDeviceSize offset,
                             uint32_t drawCount,
                             uint32_t stride);
    void drawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount);
    void drawIndexedInstancedBaseVertex(uint32_t indexCount,
                                        uint32_t instanceCount,
                                        uint32_t vertexOffset);
    void drawIndexedInstancedBaseVertexBaseInstance(uint32_t indexCount,
                                                    uint32_t instanceCount,
                                                    uint32_t firstIndex,
                                                    int32_t vertexOffset,
                                                    uint32_t firstInstance);

    void drawIndirect(const Buffer &buffer,
                      VkDeviceSize offset,
                      uint32_t drawCount,
                      uint32_t stride);

    void drawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex);
    void drawInstancedBaseInstance(uint32_t vertexCount,
                                   uint32_t instanceCount,
                                   uint32_t firstVertex,
                                   uint32_t firstInstance);

    void endDebugUtilsLabelEXT();

    void endQuery(const QueryPool &queryPool, uint32_t query);

    void endTransformFeedback(uint32_t firstCounterBuffer,
                              uint32_t counterBufferCount,
                              const VkBuffer *counterBuffers,
                              const VkDeviceSize *counterBufferOffsets);

    void fillBuffer(const Buffer &dstBuffer,
                    VkDeviceSize dstOffset,
                    VkDeviceSize size,
                    uint32_t data);

    void imageBarrier(VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags dstStageMask,
                      const VkImageMemoryBarrier &imageMemoryBarrier);

    void imageBarrier2(const VkImageMemoryBarrier2 &imageMemoryBarrier2);

    void imageWaitEvent(const VkEvent &event,
                        VkPipelineStageFlags srcStageMask,
                        VkPipelineStageFlags dstStageMask,
                        const VkImageMemoryBarrier &imageMemoryBarrier);
    void imageWaitEvent2(const VkEvent &event, const VkImageMemoryBarrier2 &imageMemoryBarrier2);

    void insertDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT &label);

    void memoryBarrier(VkPipelineStageFlags srcStageMask,
                       VkPipelineStageFlags dstStageMask,
                       const VkMemoryBarrier &memoryBarrier);

    void memoryBarrier2(const VkMemoryBarrier2 &memoryBarrier2);

    void nextSubpass(VkSubpassContents subpassContents);

    void pipelineBarrier(VkPipelineStageFlags srcStageMask,
                         VkPipelineStageFlags dstStageMask,
                         VkDependencyFlags dependencyFlags,
                         uint32_t memoryBarrierCount,
                         const VkMemoryBarrier *memoryBarriers,
                         uint32_t bufferMemoryBarrierCount,
                         const VkBufferMemoryBarrier *bufferMemoryBarriers,
                         uint32_t imageMemoryBarrierCount,
                         const VkImageMemoryBarrier *imageMemoryBarriers);

    void pipelineBarrier2(VkDependencyFlags dependencyFlags,
                          uint32_t memoryBarrierCount,
                          const VkMemoryBarrier2 *memoryBarriers2,
                          uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier2 *bufferMemoryBarriers2,
                          uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier2 *imageMemoryBarriers2);

    void pushConstants(const PipelineLayout &layout,
                       VkShaderStageFlags flag,
                       uint32_t offset,
                       uint32_t size,
                       const void *data);

    void resetEvent(VkEvent event, VkPipelineStageFlags stageMask);

    void resetQueryPool(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount);

    void resolveImage(const Image &srcImage,
                      VkImageLayout srcImageLayout,
                      const Image &dstImage,
                      VkImageLayout dstImageLayout,
                      uint32_t regionCount,
                      const VkImageResolve *regions);

    void setBlendConstants(const float blendConstants[4]);
    void setCullMode(VkCullModeFlags cullMode);
    void setDepthBias(float depthBiasConstantFactor,
                      float depthBiasClamp,
                      float depthBiasSlopeFactor);
    void setDepthBiasEnable(VkBool32 depthBiasEnable);
    void setDepthCompareOp(VkCompareOp depthCompareOp);
    void setDepthTestEnable(VkBool32 depthTestEnable);
    void setDepthWriteEnable(VkBool32 depthWriteEnable);
    void setEvent(VkEvent event, VkPipelineStageFlags stageMask);
    void setFragmentShadingRate(const VkExtent2D *fragmentSize,
                                VkFragmentShadingRateCombinerOpKHR ops[2]);
    void setFrontFace(VkFrontFace frontFace);
    void setLineWidth(float lineWidth);
    void setLogicOp(VkLogicOp logicOp);
    void setPrimitiveRestartEnable(VkBool32 primitiveRestartEnable);
    void setRasterizerDiscardEnable(VkBool32 rasterizerDiscardEnable);
    void setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *scissors);
    void setStencilCompareMask(uint32_t compareFrontMask, uint32_t compareBackMask);
    void setStencilOp(VkStencilFaceFlags faceMask,
                      VkStencilOp failOp,
                      VkStencilOp passOp,
                      VkStencilOp depthFailOp,
                      VkCompareOp compareOp);
    void setStencilReference(uint32_t frontReference, uint32_t backReference);
    void setStencilTestEnable(VkBool32 stencilTestEnable);
    void setStencilWriteMask(uint32_t writeFrontMask, uint32_t writeBackMask);
    void setVertexInput(uint32_t vertexBindingDescriptionCount,
                        const VkVertexInputBindingDescription2EXT *vertexBindingDescriptions,
                        uint32_t vertexAttributeDescriptionCount,
                        const VkVertexInputAttributeDescription2EXT *vertexAttributeDescriptions);
    void setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *viewports);

    void waitEvents(uint32_t eventCount,
                    const VkEvent *events,
                    VkPipelineStageFlags srcStageMask,
                    VkPipelineStageFlags dstStageMask,
                    uint32_t memoryBarrierCount,
                    const VkMemoryBarrier *memoryBarriers,
                    uint32_t bufferMemoryBarrierCount,
                    const VkBufferMemoryBarrier *bufferMemoryBarriers,
                    uint32_t imageMemoryBarrierCount,
                    const VkImageMemoryBarrier *imageMemoryBarriers);

    void writeTimestamp(VkPipelineStageFlagBits pipelineStage,
                        const QueryPool &queryPool,
                        uint32_t query);

    void writeTimestamp2(VkPipelineStageFlagBits2 pipelineStage,
                         const QueryPool &queryPool,
                         uint32_t query);

    // No-op for compatibility
    VkResult end() { return VK_SUCCESS; }

    // Parse the cmds in this cmd buffer into given primary cmd buffer for execution
    void executeCommands(PrimaryCommandBuffer *primary);

    // Calculate memory usage of this command buffer for diagnostics.
    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;
    void getMemoryUsageStatsForPoolAlloc(size_t blockSize,
                                         size_t *usedMemoryOut,
                                         size_t *allocatedMemoryOut) const;

    // Traverse the list of commands and build a summary for diagnostics.
    std::string dumpCommands(const char *separator) const;

    // Initialize the SecondaryCommandBuffer by setting the allocator it will use
    angle::Result initialize(vk::ErrorContext *context,
                             vk::SecondaryCommandPool *pool,
                             bool isRenderPassCommandBuffer,
                             SecondaryCommandMemoryAllocator *allocator)
    {
        return mCommandAllocator.initialize(allocator);
    }

    void attachAllocator(vk::SecondaryCommandMemoryAllocator *source)
    {
        mCommandAllocator.attachAllocator(source);
    }

    void detachAllocator(vk::SecondaryCommandMemoryAllocator *destination)
    {
        mCommandAllocator.detachAllocator(destination);
    }

    angle::Result begin(ErrorContext *context,
                        const VkCommandBufferInheritanceInfo &inheritanceInfo)
    {
        return angle::Result::Continue;
    }
    angle::Result end(ErrorContext *context) { return angle::Result::Continue; }

    void open() { mIsOpen = true; }
    void close() { mIsOpen = false; }

    void reset()
    {
        mCommands.clear();
        mCommandAllocator.reset(&mCommandTracker);
    }

    // The SecondaryCommandBuffer is valid if it's been initialized
    bool valid() const { return mCommandAllocator.valid(); }

    bool empty() const { return mCommandAllocator.empty(); }
    bool checkEmptyForPoolAlloc() const
    {
        return mCommands.size() == 0 || mCommands[0]->id == CommandID::Invalid;
    }

    uint32_t getRenderPassWriteCommandCount() const
    {
        return mCommandTracker.getRenderPassWriteCommandCount();
    }

    void clearCommands() { mCommands.clear(); }
    bool hasEmptyCommands() { return mCommands.empty(); }
    void pushToCommands(uint8_t *command)
    {
        mCommands.push_back(reinterpret_cast<priv::CommandHeader *>(command));
    }

  private:
    void commonDebugUtilsLabel(CommandID cmd, const VkDebugUtilsLabelEXT &label);
    template <class StructType>
    ANGLE_INLINE StructType *commonInit(CommandID cmdID,
                                        size_t allocationSize,
                                        uint8_t *commandMemory)
    {
        ASSERT(mIsOpen);
        ASSERT(allocationSize <= std::numeric_limits<uint16_t>::max());

        StructType *command  = reinterpret_cast<StructType *>(commandMemory);
        command->header.id   = cmdID;
        command->header.size = static_cast<uint16_t>(allocationSize);

        return command;
    }

    // Allocate and initialize memory for given commandID & variable param size, setting
    // variableDataOut to the byte following fixed cmd data where variable-sized ptr data will
    // be written and returning a pointer to the start of the command's parameter data
    template <class StructType>
    ANGLE_INLINE StructType *initCommand(CommandID cmdID,
                                         size_t variableSize,
                                         uint8_t **variableDataOut)
    {
        constexpr size_t fixedAllocationSize = sizeof(StructType);
        const size_t allocationSize          = fixedAllocationSize + variableSize;

        // Make sure we have enough room to mark follow-on header "Invalid"
        const size_t requiredSize = allocationSize + sizeof(CommandHeader);
        uint8_t *commandMemory    = nullptr;

        mCommandAllocator.onNewVariableSizedCommand(requiredSize, allocationSize, &commandMemory);
        StructType *command = commonInit<StructType>(cmdID, allocationSize, commandMemory);
        *variableDataOut    = Offset<uint8_t>(commandMemory, fixedAllocationSize);
        return command;
    }

    // Initialize a command that doesn't have variable-sized ptr data
    template <class StructType>
    ANGLE_INLINE StructType *initCommand(CommandID cmdID)
    {
        constexpr size_t allocationSize = sizeof(StructType);

        // Make sure we have enough room to mark follow-on header "Invalid"
        const size_t requiredSize = allocationSize + sizeof(CommandHeader);
        uint8_t *commandMemory    = nullptr;
        mCommandAllocator.onNewCommand(requiredSize, allocationSize, &commandMemory);

        return commonInit<StructType>(cmdID, allocationSize, commandMemory);
    }

    // Return a pointer to the parameter type.  Note that every param struct has the header as its
    // first member, so in fact the parameter type pointer is identical to the header pointer.
    template <class StructType>
    const StructType *getParamPtr(const CommandHeader *header) const
    {
        return reinterpret_cast<const StructType *>(header);
    }
    struct ArrayParamSize
    {
        // Given an array of N elements of type T, N*sizeof(T) bytes need to be copied, but due to
        // alignment requirements, roundUp(size, 8) bytes need to be allocated for the array.
        size_t copyBytes;
        size_t allocateBytes;
    };
    template <class T>
    ANGLE_INLINE ArrayParamSize calculateArrayParameterSize(size_t count)
    {
        ArrayParamSize size;
        size.copyBytes     = sizeof(T) * count;
        size.allocateBytes = roundUpPow2<size_t>(size.copyBytes, 8u);
        return size;
    }
    // Copy size.copyBytes data from paramData to writePointer and return writePointer plus
    // size.allocateBytes.
    template <class T>
    ANGLE_INLINE uint8_t *storeArrayParameter(uint8_t *writePointer,
                                              const T *data,
                                              const ArrayParamSize &size)
    {
        // See |PointerParamSize|.  The size should always be calculated with
        // |calculatePointerParameterSize|, and that satisfies this condition.
        ASSERT(size.allocateBytes == roundUpPow2<size_t>(size.copyBytes, 8u));

        memcpy(writePointer, data, size.copyBytes);
        return writePointer + size.allocateBytes;
    }

    // Flag to indicate that commandBuffer is open for new commands. Initially open.
    bool mIsOpen;

    std::vector<CommandHeader *> mCommands;

    // Allocator used by this class. If non-null then the class is valid.
    SecondaryCommandBlockPool mCommandAllocator;

    CommandBufferCommandTracker mCommandTracker;
};

ANGLE_INLINE SecondaryCommandBuffer::SecondaryCommandBuffer() : mIsOpen(true)
{
    mCommandAllocator.setCommandBuffer(this);
}

ANGLE_INLINE SecondaryCommandBuffer::~SecondaryCommandBuffer()
{
    mCommandAllocator.resetCommandBuffer();
}

// begin and insert DebugUtilsLabelEXT funcs share this same function body
ANGLE_INLINE void SecondaryCommandBuffer::commonDebugUtilsLabel(CommandID cmd,
                                                                const VkDebugUtilsLabelEXT &label)
{
    uint8_t *writePtr;
    const ArrayParamSize stringSize =
        calculateArrayParameterSize<char>(strlen(label.pLabelName) + 1);
    DebugUtilsLabelParams *paramStruct =
        initCommand<DebugUtilsLabelParams>(cmd, stringSize.allocateBytes, &writePtr);
    paramStruct->color[0] = label.color[0];
    paramStruct->color[1] = label.color[1];
    paramStruct->color[2] = label.color[2];
    paramStruct->color[3] = label.color[3];
    storeArrayParameter(writePtr, label.pLabelName, stringSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::beginDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT &label)
{
    commonDebugUtilsLabel(CommandID::BeginDebugUtilsLabel, label);
}

ANGLE_INLINE void SecondaryCommandBuffer::beginQuery(const QueryPool &queryPool,
                                                     uint32_t query,
                                                     VkQueryControlFlags flags)
{
    ASSERT(flags == 0);
    BeginQueryParams *paramStruct = initCommand<BeginQueryParams>(CommandID::BeginQuery);
    paramStruct->queryPool        = queryPool.getHandle();
    paramStruct->query            = query;
}

ANGLE_INLINE void SecondaryCommandBuffer::beginTransformFeedback(
    uint32_t firstCounterBuffer,
    uint32_t bufferCount,
    const VkBuffer *counterBuffers,
    const VkDeviceSize *counterBufferOffsets)
{
    ASSERT(firstCounterBuffer == 0);
    uint8_t *writePtr;
    const ArrayParamSize bufferSize = calculateArrayParameterSize<VkBuffer>(bufferCount);
    const ArrayParamSize offsetSize = calculateArrayParameterSize<VkDeviceSize>(bufferCount);
    BeginTransformFeedbackParams *paramStruct = initCommand<BeginTransformFeedbackParams>(
        CommandID::BeginTransformFeedback, bufferSize.allocateBytes + offsetSize.allocateBytes,
        &writePtr);
    paramStruct->bufferCount = bufferCount;
    writePtr                 = storeArrayParameter(writePtr, counterBuffers, bufferSize);
    storeArrayParameter(writePtr, counterBufferOffsets, offsetSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::bindComputePipeline(const Pipeline &pipeline)
{
    BindPipelineParams *paramStruct =
        initCommand<BindPipelineParams>(CommandID::BindComputePipeline);
    paramStruct->pipeline = pipeline.getHandle();
}

ANGLE_INLINE void SecondaryCommandBuffer::bindDescriptorSets(const PipelineLayout &layout,
                                                             VkPipelineBindPoint pipelineBindPoint,
                                                             DescriptorSetIndex firstSet,
                                                             uint32_t descriptorSetCount,
                                                             const VkDescriptorSet *descriptorSets,
                                                             uint32_t dynamicOffsetCount,
                                                             const uint32_t *dynamicOffsets)
{
    const ArrayParamSize descSize =
        calculateArrayParameterSize<VkDescriptorSet>(descriptorSetCount);
    const ArrayParamSize offsetSize = calculateArrayParameterSize<uint32_t>(dynamicOffsetCount);
    uint8_t *writePtr;
    BindDescriptorSetParams *paramStruct = initCommand<BindDescriptorSetParams>(
        CommandID::BindDescriptorSets, descSize.allocateBytes + offsetSize.allocateBytes,
        &writePtr);
    // Copy params into memory
    paramStruct->layout = layout.getHandle();
    SetBitField(paramStruct->pipelineBindPoint, pipelineBindPoint);
    SetBitField(paramStruct->firstSet, ToUnderlying(firstSet));
    SetBitField(paramStruct->descriptorSetCount, descriptorSetCount);
    SetBitField(paramStruct->dynamicOffsetCount, dynamicOffsetCount);
    // Copy variable sized data
    writePtr = storeArrayParameter(writePtr, descriptorSets, descSize);
    if (dynamicOffsetCount > 0)
    {
        storeArrayParameter(writePtr, dynamicOffsets, offsetSize);
    }
}

ANGLE_INLINE void SecondaryCommandBuffer::bindGraphicsPipeline(const Pipeline &pipeline)
{
    BindPipelineParams *paramStruct =
        initCommand<BindPipelineParams>(CommandID::BindGraphicsPipeline);
    paramStruct->pipeline = pipeline.getHandle();
}

ANGLE_INLINE void SecondaryCommandBuffer::bindIndexBuffer(const Buffer &buffer,
                                                          VkDeviceSize offset,
                                                          VkIndexType indexType)
{
    BindIndexBufferParams *paramStruct =
        initCommand<BindIndexBufferParams>(CommandID::BindIndexBuffer);
    paramStruct->buffer    = buffer.getHandle();
    paramStruct->offset    = offset;
    paramStruct->indexType = indexType;
}

ANGLE_INLINE void SecondaryCommandBuffer::bindTransformFeedbackBuffers(uint32_t firstBinding,
                                                                       uint32_t bindingCount,
                                                                       const VkBuffer *buffers,
                                                                       const VkDeviceSize *offsets,
                                                                       const VkDeviceSize *sizes)
{
    ASSERT(firstBinding == 0);
    uint8_t *writePtr;
    const ArrayParamSize buffersSize = calculateArrayParameterSize<VkBuffer>(bindingCount);
    const ArrayParamSize offsetsSize = calculateArrayParameterSize<VkDeviceSize>(bindingCount);
    const ArrayParamSize sizesSize   = offsetsSize;
    BindTransformFeedbackBuffersParams *paramStruct =
        initCommand<BindTransformFeedbackBuffersParams>(
            CommandID::BindTransformFeedbackBuffers,
            buffersSize.allocateBytes + offsetsSize.allocateBytes + sizesSize.allocateBytes,
            &writePtr);
    // Copy params
    paramStruct->bindingCount = bindingCount;
    writePtr                  = storeArrayParameter(writePtr, buffers, buffersSize);
    writePtr                  = storeArrayParameter(writePtr, offsets, offsetsSize);
    storeArrayParameter(writePtr, sizes, sizesSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::bindVertexBuffers(uint32_t firstBinding,
                                                            uint32_t bindingCount,
                                                            const VkBuffer *buffers,
                                                            const VkDeviceSize *offsets)
{
    ASSERT(firstBinding == 0);
    uint8_t *writePtr;
    const ArrayParamSize buffersSize     = calculateArrayParameterSize<VkBuffer>(bindingCount);
    const ArrayParamSize offsetsSize     = calculateArrayParameterSize<VkDeviceSize>(bindingCount);
    BindVertexBuffersParams *paramStruct = initCommand<BindVertexBuffersParams>(
        CommandID::BindVertexBuffers, buffersSize.allocateBytes + offsetsSize.allocateBytes,
        &writePtr);
    // Copy params
    paramStruct->bindingCount = bindingCount;
    writePtr                  = storeArrayParameter(writePtr, buffers, buffersSize);
    storeArrayParameter(writePtr, offsets, offsetsSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::bindVertexBuffers2(uint32_t firstBinding,
                                                             uint32_t bindingCount,
                                                             const VkBuffer *buffers,
                                                             const VkDeviceSize *offsets,
                                                             const VkDeviceSize *sizes,
                                                             const VkDeviceSize *strides)
{
    ASSERT(firstBinding == 0);
    ASSERT(sizes == nullptr);
    uint8_t *writePtr;
    const ArrayParamSize buffersSize      = calculateArrayParameterSize<VkBuffer>(bindingCount);
    const ArrayParamSize offsetsSize      = calculateArrayParameterSize<VkDeviceSize>(bindingCount);
    const ArrayParamSize stridesSize      = offsetsSize;
    BindVertexBuffers2Params *paramStruct = initCommand<BindVertexBuffers2Params>(
        CommandID::BindVertexBuffers2,
        buffersSize.allocateBytes + offsetsSize.allocateBytes + stridesSize.allocateBytes,
        &writePtr);
    // Copy params
    paramStruct->bindingCount = bindingCount;
    writePtr                  = storeArrayParameter(writePtr, buffers, buffersSize);
    writePtr                  = storeArrayParameter(writePtr, offsets, offsetsSize);
    storeArrayParameter(writePtr, strides, stridesSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::blitImage(const Image &srcImage,
                                                    VkImageLayout srcImageLayout,
                                                    const Image &dstImage,
                                                    VkImageLayout dstImageLayout,
                                                    uint32_t regionCount,
                                                    const VkImageBlit *regions,
                                                    VkFilter filter)
{
    // Currently ANGLE uses limited params so verify those assumptions and update if they change
    ASSERT(srcImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    ASSERT(dstImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ASSERT(regionCount == 1);
    BlitImageParams *paramStruct = initCommand<BlitImageParams>(CommandID::BlitImage);
    paramStruct->srcImage        = srcImage.getHandle();
    paramStruct->dstImage        = dstImage.getHandle();
    paramStruct->filter          = filter;
    paramStruct->region          = regions[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::bufferBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkBufferMemoryBarrier *bufferMemoryBarrier)
{
    // Used only during queue family ownership transfers
    ASSERT(srcStageMask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    ASSERT(dstStageMask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    BufferBarrierParams *paramStruct = initCommand<BufferBarrierParams>(CommandID::BufferBarrier);
    paramStruct->bufferMemoryBarrier = *bufferMemoryBarrier;
}

ANGLE_INLINE void SecondaryCommandBuffer::bufferBarrier2(
    const VkBufferMemoryBarrier2 *bufferMemoryBarrier2)
{
    ASSERT(bufferMemoryBarrier2->srcStageMask == VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
    ASSERT(bufferMemoryBarrier2->dstStageMask == VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);

    BufferBarrier2Params *paramStruct =
        initCommand<BufferBarrier2Params>(CommandID::BufferBarrier2);
    paramStruct->bufferMemoryBarrier2 = *bufferMemoryBarrier2;
}

ANGLE_INLINE void SecondaryCommandBuffer::clearAttachments(uint32_t attachmentCount,
                                                           const VkClearAttachment *attachments,
                                                           uint32_t rectCount,
                                                           const VkClearRect *rects)
{
    ASSERT(rectCount == 1);
    uint8_t *writePtr;
    const ArrayParamSize attachmentSize =
        calculateArrayParameterSize<VkClearAttachment>(attachmentCount);
    ClearAttachmentsParams *paramStruct = initCommand<ClearAttachmentsParams>(
        CommandID::ClearAttachments, attachmentSize.allocateBytes, &writePtr);
    paramStruct->attachmentCount = attachmentCount;
    paramStruct->rect            = rects[0];
    // Copy variable sized data
    storeArrayParameter(writePtr, attachments, attachmentSize);

    mCommandTracker.onClearAttachments();
}

ANGLE_INLINE void SecondaryCommandBuffer::clearColorImage(const Image &image,
                                                          VkImageLayout imageLayout,
                                                          const VkClearColorValue &color,
                                                          uint32_t rangeCount,
                                                          const VkImageSubresourceRange *ranges)
{
    ASSERT(rangeCount == 1);
    ClearColorImageParams *paramStruct =
        initCommand<ClearColorImageParams>(CommandID::ClearColorImage);
    paramStruct->image       = image.getHandle();
    paramStruct->imageLayout = imageLayout;
    paramStruct->color       = color;
    paramStruct->range       = ranges[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::clearDepthStencilImage(
    const Image &image,
    VkImageLayout imageLayout,
    const VkClearDepthStencilValue &depthStencil,
    uint32_t rangeCount,
    const VkImageSubresourceRange *ranges)
{
    ASSERT(rangeCount == 1);
    ClearDepthStencilImageParams *paramStruct =
        initCommand<ClearDepthStencilImageParams>(CommandID::ClearDepthStencilImage);
    paramStruct->image        = image.getHandle();
    paramStruct->imageLayout  = imageLayout;
    paramStruct->depthStencil = depthStencil;
    paramStruct->range        = ranges[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::copyBuffer(const Buffer &srcBuffer,
                                                     const Buffer &destBuffer,
                                                     uint32_t regionCount,
                                                     const VkBufferCopy *regions)
{
    uint8_t *writePtr;
    const ArrayParamSize regionSize = calculateArrayParameterSize<VkBufferCopy>(regionCount);
    CopyBufferParams *paramStruct =
        initCommand<CopyBufferParams>(CommandID::CopyBuffer, regionSize.allocateBytes, &writePtr);
    paramStruct->srcBuffer   = srcBuffer.getHandle();
    paramStruct->destBuffer  = destBuffer.getHandle();
    paramStruct->regionCount = regionCount;
    // Copy variable sized data
    storeArrayParameter(writePtr, regions, regionSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::copyBufferToImage(VkBuffer srcBuffer,
                                                            const Image &dstImage,
                                                            VkImageLayout dstImageLayout,
                                                            uint32_t regionCount,
                                                            const VkBufferImageCopy *regions)
{
    ASSERT(regionCount == 1);
    CopyBufferToImageParams *paramStruct =
        initCommand<CopyBufferToImageParams>(CommandID::CopyBufferToImage);
    paramStruct->srcBuffer      = srcBuffer;
    paramStruct->dstImage       = dstImage.getHandle();
    paramStruct->dstImageLayout = dstImageLayout;
    paramStruct->region         = regions[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::copyImage(const Image &srcImage,
                                                    VkImageLayout srcImageLayout,
                                                    const Image &dstImage,
                                                    VkImageLayout dstImageLayout,
                                                    uint32_t regionCount,
                                                    const VkImageCopy *regions)
{
    ASSERT(regionCount == 1);
    CopyImageParams *paramStruct = initCommand<CopyImageParams>(CommandID::CopyImage);
    paramStruct->srcImage        = srcImage.getHandle();
    paramStruct->srcImageLayout  = srcImageLayout;
    paramStruct->dstImage        = dstImage.getHandle();
    paramStruct->dstImageLayout  = dstImageLayout;
    paramStruct->region          = regions[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::copyImageToBuffer(const Image &srcImage,
                                                            VkImageLayout srcImageLayout,
                                                            VkBuffer dstBuffer,
                                                            uint32_t regionCount,
                                                            const VkBufferImageCopy *regions)
{
    ASSERT(regionCount == 1);
    CopyImageToBufferParams *paramStruct =
        initCommand<CopyImageToBufferParams>(CommandID::CopyImageToBuffer);
    paramStruct->srcImage       = srcImage.getHandle();
    paramStruct->srcImageLayout = srcImageLayout;
    paramStruct->dstBuffer      = dstBuffer;
    paramStruct->region         = regions[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::dispatch(uint32_t groupCountX,
                                                   uint32_t groupCountY,
                                                   uint32_t groupCountZ)
{
    DispatchParams *paramStruct = initCommand<DispatchParams>(CommandID::Dispatch);
    paramStruct->groupCountX    = groupCountX;
    paramStruct->groupCountY    = groupCountY;
    paramStruct->groupCountZ    = groupCountZ;
}

ANGLE_INLINE void SecondaryCommandBuffer::dispatchIndirect(const Buffer &buffer,
                                                           VkDeviceSize offset)
{
    DispatchIndirectParams *paramStruct =
        initCommand<DispatchIndirectParams>(CommandID::DispatchIndirect);
    paramStruct->buffer = buffer.getHandle();
    paramStruct->offset = offset;
}

ANGLE_INLINE void SecondaryCommandBuffer::draw(uint32_t vertexCount, uint32_t firstVertex)
{
    DrawParams *paramStruct  = initCommand<DrawParams>(CommandID::Draw);
    paramStruct->vertexCount = vertexCount;
    paramStruct->firstVertex = firstVertex;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawIndexed(uint32_t indexCount)
{
    DrawIndexedParams *paramStruct = initCommand<DrawIndexedParams>(CommandID::DrawIndexed);
    paramStruct->indexCount        = indexCount;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawIndexedBaseVertex(uint32_t indexCount,
                                                                uint32_t vertexOffset)
{
    DrawIndexedBaseVertexParams *paramStruct =
        initCommand<DrawIndexedBaseVertexParams>(CommandID::DrawIndexedBaseVertex);
    paramStruct->indexCount   = indexCount;
    paramStruct->vertexOffset = vertexOffset;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawIndexedIndirect(const Buffer &buffer,
                                                              VkDeviceSize offset,
                                                              uint32_t drawCount,
                                                              uint32_t stride)
{
    DrawIndexedIndirectParams *paramStruct =
        initCommand<DrawIndexedIndirectParams>(CommandID::DrawIndexedIndirect);
    paramStruct->buffer    = buffer.getHandle();
    paramStruct->offset    = offset;
    paramStruct->drawCount = drawCount;
    paramStruct->stride    = stride;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawIndexedInstanced(uint32_t indexCount,
                                                               uint32_t instanceCount)
{
    DrawIndexedInstancedParams *paramStruct =
        initCommand<DrawIndexedInstancedParams>(CommandID::DrawIndexedInstanced);
    paramStruct->indexCount    = indexCount;
    paramStruct->instanceCount = instanceCount;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawIndexedInstancedBaseVertex(uint32_t indexCount,
                                                                         uint32_t instanceCount,
                                                                         uint32_t vertexOffset)
{
    DrawIndexedInstancedBaseVertexParams *paramStruct =
        initCommand<DrawIndexedInstancedBaseVertexParams>(
            CommandID::DrawIndexedInstancedBaseVertex);
    paramStruct->indexCount    = indexCount;
    paramStruct->instanceCount = instanceCount;
    paramStruct->vertexOffset  = vertexOffset;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawIndexedInstancedBaseVertexBaseInstance(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance)
{
    DrawIndexedInstancedBaseVertexBaseInstanceParams *paramStruct =
        initCommand<DrawIndexedInstancedBaseVertexBaseInstanceParams>(
            CommandID::DrawIndexedInstancedBaseVertexBaseInstance);
    paramStruct->indexCount    = indexCount;
    paramStruct->instanceCount = instanceCount;
    paramStruct->firstIndex    = firstIndex;
    paramStruct->vertexOffset  = vertexOffset;
    paramStruct->firstInstance = firstInstance;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawIndirect(const Buffer &buffer,
                                                       VkDeviceSize offset,
                                                       uint32_t drawCount,
                                                       uint32_t stride)
{
    DrawIndirectParams *paramStruct = initCommand<DrawIndirectParams>(CommandID::DrawIndirect);
    paramStruct->buffer             = buffer.getHandle();
    paramStruct->offset             = offset;
    paramStruct->drawCount          = drawCount;
    paramStruct->stride             = stride;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawInstanced(uint32_t vertexCount,
                                                        uint32_t instanceCount,
                                                        uint32_t firstVertex)
{
    DrawInstancedParams *paramStruct = initCommand<DrawInstancedParams>(CommandID::DrawInstanced);
    paramStruct->vertexCount         = vertexCount;
    paramStruct->instanceCount       = instanceCount;
    paramStruct->firstVertex         = firstVertex;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::drawInstancedBaseInstance(uint32_t vertexCount,
                                                                    uint32_t instanceCount,
                                                                    uint32_t firstVertex,
                                                                    uint32_t firstInstance)
{
    DrawInstancedBaseInstanceParams *paramStruct =
        initCommand<DrawInstancedBaseInstanceParams>(CommandID::DrawInstancedBaseInstance);
    paramStruct->vertexCount   = vertexCount;
    paramStruct->instanceCount = instanceCount;
    paramStruct->firstVertex   = firstVertex;
    paramStruct->firstInstance = firstInstance;

    mCommandTracker.onDraw();
}

ANGLE_INLINE void SecondaryCommandBuffer::endDebugUtilsLabelEXT()
{
    initCommand<EmptyParams>(CommandID::EndDebugUtilsLabel);
}

ANGLE_INLINE void SecondaryCommandBuffer::endQuery(const QueryPool &queryPool, uint32_t query)
{
    EndQueryParams *paramStruct = initCommand<EndQueryParams>(CommandID::EndQuery);
    paramStruct->queryPool      = queryPool.getHandle();
    paramStruct->query          = query;
}

ANGLE_INLINE void SecondaryCommandBuffer::endTransformFeedback(
    uint32_t firstCounterBuffer,
    uint32_t counterBufferCount,
    const VkBuffer *counterBuffers,
    const VkDeviceSize *counterBufferOffsets)
{
    ASSERT(firstCounterBuffer == 0);
    uint8_t *writePtr;
    const ArrayParamSize buffersSize = calculateArrayParameterSize<VkBuffer>(counterBufferCount);
    const ArrayParamSize offsetsSize =
        calculateArrayParameterSize<VkDeviceSize>(counterBufferCount);
    EndTransformFeedbackParams *paramStruct = initCommand<EndTransformFeedbackParams>(
        CommandID::EndTransformFeedback, buffersSize.allocateBytes + offsetsSize.allocateBytes,
        &writePtr);
    paramStruct->bufferCount = counterBufferCount;
    writePtr                 = storeArrayParameter(writePtr, counterBuffers, buffersSize);
    storeArrayParameter(writePtr, counterBufferOffsets, offsetsSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::fillBuffer(const Buffer &dstBuffer,
                                                     VkDeviceSize dstOffset,
                                                     VkDeviceSize size,
                                                     uint32_t data)
{
    FillBufferParams *paramStruct = initCommand<FillBufferParams>(CommandID::FillBuffer);
    paramStruct->dstBuffer        = dstBuffer.getHandle();
    paramStruct->dstOffset        = dstOffset;
    paramStruct->size             = size;
    paramStruct->data             = data;
}

ANGLE_INLINE void SecondaryCommandBuffer::imageBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkImageMemoryBarrier &imageMemoryBarrier)
{
    ASSERT(imageMemoryBarrier.pNext == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize imgBarrierSize = calculateArrayParameterSize<VkImageMemoryBarrier>(1);
    ImageBarrierParams *paramStruct     = initCommand<ImageBarrierParams>(
        CommandID::ImageBarrier, imgBarrierSize.allocateBytes, &writePtr);
    paramStruct->srcStageMask = srcStageMask;
    paramStruct->dstStageMask = dstStageMask;
    storeArrayParameter(writePtr, &imageMemoryBarrier, imgBarrierSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::imageBarrier2(
    const VkImageMemoryBarrier2 &imageMemoryBarrier2)
{
    ASSERT(imageMemoryBarrier2.pNext == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize imgBarrier2Size = calculateArrayParameterSize<VkImageMemoryBarrier2>(1);
    initCommand<ImageBarrier2Params>(CommandID::ImageBarrier2, imgBarrier2Size.allocateBytes,
                                     &writePtr);
    storeArrayParameter(writePtr, &imageMemoryBarrier2, imgBarrier2Size);
}

ANGLE_INLINE void SecondaryCommandBuffer::imageWaitEvent(
    const VkEvent &event,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkImageMemoryBarrier &imageMemoryBarrier)
{
    ASSERT(imageMemoryBarrier.pNext == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize imgBarrierSize = calculateArrayParameterSize<VkImageMemoryBarrier>(1);

    ImageWaitEventParams *paramStruct = initCommand<ImageWaitEventParams>(
        CommandID::ImageWaitEvent, imgBarrierSize.allocateBytes, &writePtr);
    paramStruct->event        = event;
    paramStruct->srcStageMask = srcStageMask;
    paramStruct->dstStageMask = dstStageMask;
    storeArrayParameter(writePtr, &imageMemoryBarrier, imgBarrierSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::insertDebugUtilsLabelEXT(
    const VkDebugUtilsLabelEXT &label)
{
    commonDebugUtilsLabel(CommandID::InsertDebugUtilsLabel, label);
}

ANGLE_INLINE void SecondaryCommandBuffer::memoryBarrier(VkPipelineStageFlags srcStageMask,
                                                        VkPipelineStageFlags dstStageMask,
                                                        const VkMemoryBarrier &memoryBarrier)
{
    ASSERT(memoryBarrier.pNext == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize memBarrierSize = calculateArrayParameterSize<VkMemoryBarrier>(1);
    MemoryBarrierParams *paramStruct    = initCommand<MemoryBarrierParams>(
        CommandID::MemoryBarrier, memBarrierSize.allocateBytes, &writePtr);
    paramStruct->srcStageMask = srcStageMask;
    paramStruct->dstStageMask = dstStageMask;
    storeArrayParameter(writePtr, &memoryBarrier, memBarrierSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::memoryBarrier2(const VkMemoryBarrier2 &memoryBarrier2)
{
    ASSERT(memoryBarrier2.pNext == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize memBarrierSize = calculateArrayParameterSize<VkMemoryBarrier2>(1);
    initCommand<MemoryBarrier2Params>(CommandID::MemoryBarrier2, memBarrierSize.allocateBytes,
                                      &writePtr);
    storeArrayParameter(writePtr, &memoryBarrier2, memBarrierSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::nextSubpass(VkSubpassContents subpassContents)
{
    ASSERT(subpassContents == VK_SUBPASS_CONTENTS_INLINE);
    initCommand<EmptyParams>(CommandID::NextSubpass);
}

ANGLE_INLINE void SecondaryCommandBuffer::pipelineBarrier(
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier *memoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier *bufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *imageMemoryBarriers)
{
    // Other than for QFOT (where bufferBarrier() is used), ANGLE doesn't use buffer barriers.
    // Memory barriers are used instead.
    ASSERT(bufferMemoryBarrierCount == 0);
    ASSERT(bufferMemoryBarriers == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize memBarrierSize =
        calculateArrayParameterSize<VkMemoryBarrier>(memoryBarrierCount);
    const ArrayParamSize imgBarrierSize =
        calculateArrayParameterSize<VkImageMemoryBarrier>(imageMemoryBarrierCount);
    PipelineBarrierParams *paramStruct = initCommand<PipelineBarrierParams>(
        CommandID::PipelineBarrier, memBarrierSize.allocateBytes + imgBarrierSize.allocateBytes,
        &writePtr);
    paramStruct->srcStageMask            = srcStageMask;
    paramStruct->dstStageMask            = dstStageMask;
    paramStruct->dependencyFlags         = dependencyFlags;
    paramStruct->memoryBarrierCount      = memoryBarrierCount;
    paramStruct->imageMemoryBarrierCount = imageMemoryBarrierCount;
    // Copy variable sized data
    if (memoryBarrierCount > 0)
    {
        writePtr = storeArrayParameter(writePtr, memoryBarriers, memBarrierSize);
    }
    if (imageMemoryBarrierCount > 0)
    {
        storeArrayParameter(writePtr, imageMemoryBarriers, imgBarrierSize);
    }
}

ANGLE_INLINE void SecondaryCommandBuffer::pipelineBarrier2(
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier2 *memoryBarriers2,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier2 *bufferMemoryBarriers2,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier2 *imageMemoryBarriers2)
{
    // Other than for QFOT (where bufferBarrier() is used), ANGLE doesn't use buffer barriers.
    // Memory barriers are used instead.
    ASSERT(bufferMemoryBarrierCount == 0);
    ASSERT(bufferMemoryBarriers2 == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize memBarrier2Size =
        calculateArrayParameterSize<VkMemoryBarrier2>(memoryBarrierCount);
    const ArrayParamSize imgBarrier2Size =
        calculateArrayParameterSize<VkImageMemoryBarrier2>(imageMemoryBarrierCount);

    PipelineBarrierParams2 *paramStruct = initCommand<PipelineBarrierParams2>(
        CommandID::PipelineBarrier2, memBarrier2Size.allocateBytes + imgBarrier2Size.allocateBytes,
        &writePtr);

    paramStruct->dependencyFlags         = dependencyFlags;
    paramStruct->memoryBarrierCount      = memoryBarrierCount;
    paramStruct->imageMemoryBarrierCount = imageMemoryBarrierCount;
    // Copy variable sized data
    if (memoryBarrierCount > 0)
    {
        writePtr = storeArrayParameter(writePtr, memoryBarriers2, memBarrier2Size);
    }
    if (imageMemoryBarrierCount > 0)
    {
        storeArrayParameter(writePtr, imageMemoryBarriers2, imgBarrier2Size);
    }
}

ANGLE_INLINE void SecondaryCommandBuffer::pushConstants(const PipelineLayout &layout,
                                                        VkShaderStageFlags flag,
                                                        uint32_t offset,
                                                        uint32_t size,
                                                        const void *data)
{
    ASSERT(size == static_cast<size_t>(size));
    uint8_t *writePtr;
    const ArrayParamSize dataSize    = calculateArrayParameterSize<uint8_t>(size);
    PushConstantsParams *paramStruct = initCommand<PushConstantsParams>(
        CommandID::PushConstants, dataSize.allocateBytes, &writePtr);
    paramStruct->layout = layout.getHandle();
    paramStruct->flag   = flag;
    paramStruct->offset = offset;
    paramStruct->size   = size;
    // Copy variable sized data
    storeArrayParameter(writePtr, data, dataSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::resetEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    ResetEventParams *paramStruct = initCommand<ResetEventParams>(CommandID::ResetEvent);
    paramStruct->event            = event;
    paramStruct->stageMask        = stageMask;
}

ANGLE_INLINE void SecondaryCommandBuffer::resetQueryPool(const QueryPool &queryPool,
                                                         uint32_t firstQuery,
                                                         uint32_t queryCount)
{
    ResetQueryPoolParams *paramStruct =
        initCommand<ResetQueryPoolParams>(CommandID::ResetQueryPool);
    paramStruct->queryPool = queryPool.getHandle();
    SetBitField(paramStruct->firstQuery, firstQuery);
    SetBitField(paramStruct->queryCount, queryCount);
}

ANGLE_INLINE void SecondaryCommandBuffer::resolveImage(const Image &srcImage,
                                                       VkImageLayout srcImageLayout,
                                                       const Image &dstImage,
                                                       VkImageLayout dstImageLayout,
                                                       uint32_t regionCount,
                                                       const VkImageResolve *regions)
{
    // Currently ANGLE uses limited params so verify those assumptions and update if they change.
    ASSERT(srcImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    ASSERT(dstImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    ASSERT(regionCount == 1);
    ResolveImageParams *paramStruct = initCommand<ResolveImageParams>(CommandID::ResolveImage);
    paramStruct->srcImage           = srcImage.getHandle();
    paramStruct->dstImage           = dstImage.getHandle();
    paramStruct->region             = regions[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::setBlendConstants(const float blendConstants[4])
{
    SetBlendConstantsParams *paramStruct =
        initCommand<SetBlendConstantsParams>(CommandID::SetBlendConstants);
    for (uint32_t channel = 0; channel < 4; ++channel)
    {
        paramStruct->blendConstants[channel] = blendConstants[channel];
    }
}

ANGLE_INLINE void SecondaryCommandBuffer::setCullMode(VkCullModeFlags cullMode)
{
    SetCullModeParams *paramStruct = initCommand<SetCullModeParams>(CommandID::SetCullMode);
    paramStruct->cullMode          = cullMode;
}

ANGLE_INLINE void SecondaryCommandBuffer::setDepthBias(float depthBiasConstantFactor,
                                                       float depthBiasClamp,
                                                       float depthBiasSlopeFactor)
{
    SetDepthBiasParams *paramStruct      = initCommand<SetDepthBiasParams>(CommandID::SetDepthBias);
    paramStruct->depthBiasConstantFactor = depthBiasConstantFactor;
    paramStruct->depthBiasClamp          = depthBiasClamp;
    paramStruct->depthBiasSlopeFactor    = depthBiasSlopeFactor;
}

ANGLE_INLINE void SecondaryCommandBuffer::setDepthBiasEnable(VkBool32 depthBiasEnable)
{
    SetDepthBiasEnableParams *paramStruct =
        initCommand<SetDepthBiasEnableParams>(CommandID::SetDepthBiasEnable);
    paramStruct->depthBiasEnable = depthBiasEnable;
}

ANGLE_INLINE void SecondaryCommandBuffer::setDepthCompareOp(VkCompareOp depthCompareOp)
{
    SetDepthCompareOpParams *paramStruct =
        initCommand<SetDepthCompareOpParams>(CommandID::SetDepthCompareOp);
    paramStruct->depthCompareOp = depthCompareOp;
}

ANGLE_INLINE void SecondaryCommandBuffer::setDepthTestEnable(VkBool32 depthTestEnable)
{
    SetDepthTestEnableParams *paramStruct =
        initCommand<SetDepthTestEnableParams>(CommandID::SetDepthTestEnable);
    paramStruct->depthTestEnable = depthTestEnable;
}

ANGLE_INLINE void SecondaryCommandBuffer::setDepthWriteEnable(VkBool32 depthWriteEnable)
{
    SetDepthWriteEnableParams *paramStruct =
        initCommand<SetDepthWriteEnableParams>(CommandID::SetDepthWriteEnable);
    paramStruct->depthWriteEnable = depthWriteEnable;
}

ANGLE_INLINE void SecondaryCommandBuffer::setEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    SetEventParams *paramStruct = initCommand<SetEventParams>(CommandID::SetEvent);
    paramStruct->event          = event;
    paramStruct->stageMask      = stageMask;
}

ANGLE_INLINE void SecondaryCommandBuffer::setFragmentShadingRate(
    const VkExtent2D *fragmentSize,
    VkFragmentShadingRateCombinerOpKHR ops[2])
{
    ASSERT(fragmentSize != nullptr);

    // Supported parameter values -
    // 1. CombinerOp for ops[0] needs to be VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR
    //    as there are no current usecases in ANGLE to use primitive fragment shading rates
    // 2. The largest fragment size supported is 4x4
    ASSERT(ops[0] == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR);
    ASSERT(fragmentSize->width <= 4);
    ASSERT(fragmentSize->height <= 4);

    SetFragmentShadingRateParams *paramStruct =
        initCommand<SetFragmentShadingRateParams>(CommandID::SetFragmentShadingRate);
    paramStruct->fragmentWidth                    = static_cast<uint16_t>(fragmentSize->width);
    paramStruct->fragmentHeight                   = static_cast<uint16_t>(fragmentSize->height);
    paramStruct->vkFragmentShadingRateCombinerOp1 = static_cast<uint16_t>(ops[1]);
}

ANGLE_INLINE void SecondaryCommandBuffer::setFrontFace(VkFrontFace frontFace)
{
    SetFrontFaceParams *paramStruct = initCommand<SetFrontFaceParams>(CommandID::SetFrontFace);
    paramStruct->frontFace          = frontFace;
}

ANGLE_INLINE void SecondaryCommandBuffer::setLineWidth(float lineWidth)
{
    SetLineWidthParams *paramStruct = initCommand<SetLineWidthParams>(CommandID::SetLineWidth);
    paramStruct->lineWidth          = lineWidth;
}

ANGLE_INLINE void SecondaryCommandBuffer::setLogicOp(VkLogicOp logicOp)
{
    SetLogicOpParams *paramStruct = initCommand<SetLogicOpParams>(CommandID::SetLogicOp);
    paramStruct->logicOp          = logicOp;
}

ANGLE_INLINE void SecondaryCommandBuffer::setPrimitiveRestartEnable(VkBool32 primitiveRestartEnable)
{
    SetPrimitiveRestartEnableParams *paramStruct =
        initCommand<SetPrimitiveRestartEnableParams>(CommandID::SetPrimitiveRestartEnable);
    paramStruct->primitiveRestartEnable = primitiveRestartEnable;
}

ANGLE_INLINE void SecondaryCommandBuffer::setRasterizerDiscardEnable(
    VkBool32 rasterizerDiscardEnable)
{
    SetRasterizerDiscardEnableParams *paramStruct =
        initCommand<SetRasterizerDiscardEnableParams>(CommandID::SetRasterizerDiscardEnable);
    paramStruct->rasterizerDiscardEnable = rasterizerDiscardEnable;
}

ANGLE_INLINE void SecondaryCommandBuffer::setScissor(uint32_t firstScissor,
                                                     uint32_t scissorCount,
                                                     const VkRect2D *scissors)
{
    ASSERT(firstScissor == 0);
    ASSERT(scissorCount == 1);
    ASSERT(scissors != nullptr);
    SetScissorParams *paramStruct = initCommand<SetScissorParams>(CommandID::SetScissor);
    paramStruct->scissor          = scissors[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::setStencilCompareMask(uint32_t compareFrontMask,
                                                                uint32_t compareBackMask)
{
    SetStencilCompareMaskParams *paramStruct =
        initCommand<SetStencilCompareMaskParams>(CommandID::SetStencilCompareMask);
    paramStruct->compareFrontMask = static_cast<uint16_t>(compareFrontMask);
    paramStruct->compareBackMask  = static_cast<uint16_t>(compareBackMask);
}

ANGLE_INLINE void SecondaryCommandBuffer::setStencilOp(VkStencilFaceFlags faceMask,
                                                       VkStencilOp failOp,
                                                       VkStencilOp passOp,
                                                       VkStencilOp depthFailOp,
                                                       VkCompareOp compareOp)
{
    SetStencilOpParams *paramStruct = initCommand<SetStencilOpParams>(CommandID::SetStencilOp);
    SetBitField(paramStruct->faceMask, faceMask);
    SetBitField(paramStruct->failOp, failOp);
    SetBitField(paramStruct->passOp, passOp);
    SetBitField(paramStruct->depthFailOp, depthFailOp);
    SetBitField(paramStruct->compareOp, compareOp);
}

ANGLE_INLINE void SecondaryCommandBuffer::setStencilReference(uint32_t frontReference,
                                                              uint32_t backReference)
{
    SetStencilReferenceParams *paramStruct =
        initCommand<SetStencilReferenceParams>(CommandID::SetStencilReference);
    paramStruct->frontReference = static_cast<uint16_t>(frontReference);
    paramStruct->backReference  = static_cast<uint16_t>(backReference);
}

ANGLE_INLINE void SecondaryCommandBuffer::setStencilTestEnable(VkBool32 stencilTestEnable)
{
    SetStencilTestEnableParams *paramStruct =
        initCommand<SetStencilTestEnableParams>(CommandID::SetStencilTestEnable);
    paramStruct->stencilTestEnable = stencilTestEnable;
}

ANGLE_INLINE void SecondaryCommandBuffer::setStencilWriteMask(uint32_t writeFrontMask,
                                                              uint32_t writeBackMask)
{
    SetStencilWriteMaskParams *paramStruct =
        initCommand<SetStencilWriteMaskParams>(CommandID::SetStencilWriteMask);
    paramStruct->writeFrontMask = static_cast<uint16_t>(writeFrontMask);
    paramStruct->writeBackMask  = static_cast<uint16_t>(writeBackMask);
}

ANGLE_INLINE void SecondaryCommandBuffer::setVertexInput(
    uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *vertexBindingDescriptions,
    uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *vertexAttributeDescriptions)
{
    uint8_t *writePtr;
    const ArrayParamSize vertexBindingDescriptionSize =
        calculateArrayParameterSize<VkVertexInputBindingDescription2EXT>(
            vertexBindingDescriptionCount);
    const ArrayParamSize vertexAttributeDescriptionSize =
        calculateArrayParameterSize<VkVertexInputAttributeDescription2EXT>(
            vertexAttributeDescriptionCount);

    SetVertexInputParams *paramStruct = initCommand<SetVertexInputParams>(
        CommandID::SetVertexInput,
        vertexBindingDescriptionSize.allocateBytes + vertexAttributeDescriptionSize.allocateBytes,
        &writePtr);

    // Copy params
    SetBitField(paramStruct->vertexBindingDescriptionCount, vertexBindingDescriptionCount);
    SetBitField(paramStruct->vertexAttributeDescriptionCount, vertexAttributeDescriptionCount);

    if (vertexBindingDescriptionSize.copyBytes)
    {
        writePtr =
            storeArrayParameter(writePtr, vertexBindingDescriptions, vertexBindingDescriptionSize);
    }
    if (vertexAttributeDescriptionSize.copyBytes)
    {
        storeArrayParameter(writePtr, vertexAttributeDescriptions, vertexAttributeDescriptionSize);
    }
}

ANGLE_INLINE void SecondaryCommandBuffer::setViewport(uint32_t firstViewport,
                                                      uint32_t viewportCount,
                                                      const VkViewport *viewports)
{
    ASSERT(firstViewport == 0);
    ASSERT(viewportCount == 1);
    ASSERT(viewports != nullptr);
    SetViewportParams *paramStruct = initCommand<SetViewportParams>(CommandID::SetViewport);
    paramStruct->viewport          = viewports[0];
}

ANGLE_INLINE void SecondaryCommandBuffer::waitEvents(
    uint32_t eventCount,
    const VkEvent *events,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier *memoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier *bufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *imageMemoryBarriers)
{
    // Other than for QFOT (where bufferBarrier() is used), ANGLE doesn't use buffer barriers.
    // Memory barriers are used instead.
    ASSERT(bufferMemoryBarrierCount == 0);
    ASSERT(bufferMemoryBarriers == nullptr);

    uint8_t *writePtr;
    const ArrayParamSize eventSize = calculateArrayParameterSize<VkEvent>(eventCount);
    const ArrayParamSize memBarrierSize =
        calculateArrayParameterSize<VkMemoryBarrier>(memoryBarrierCount);
    const ArrayParamSize imgBarrierSize =
        calculateArrayParameterSize<VkImageMemoryBarrier>(imageMemoryBarrierCount);
    WaitEventsParams *paramStruct = initCommand<WaitEventsParams>(
        CommandID::WaitEvents,
        eventSize.allocateBytes + memBarrierSize.allocateBytes + imgBarrierSize.allocateBytes,
        &writePtr);
    paramStruct->eventCount              = eventCount;
    paramStruct->srcStageMask            = srcStageMask;
    paramStruct->dstStageMask            = dstStageMask;
    paramStruct->memoryBarrierCount      = memoryBarrierCount;
    paramStruct->imageMemoryBarrierCount = imageMemoryBarrierCount;
    // Copy variable sized data
    writePtr = storeArrayParameter(writePtr, events, eventSize);
    writePtr = storeArrayParameter(writePtr, memoryBarriers, memBarrierSize);
    storeArrayParameter(writePtr, imageMemoryBarriers, imgBarrierSize);
}

ANGLE_INLINE void SecondaryCommandBuffer::writeTimestamp(VkPipelineStageFlagBits pipelineStage,
                                                         const QueryPool &queryPool,
                                                         uint32_t query)
{
    ASSERT(pipelineStage == VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    WriteTimestampParams *paramStruct =
        initCommand<WriteTimestampParams>(CommandID::WriteTimestamp);
    paramStruct->queryPool = queryPool.getHandle();
    paramStruct->query     = query;
}

ANGLE_INLINE void SecondaryCommandBuffer::writeTimestamp2(VkPipelineStageFlagBits2 pipelineStage,
                                                          const QueryPool &queryPool,
                                                          uint32_t query)
{
    ASSERT(pipelineStage == VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

    WriteTimestampParams *paramStruct =
        initCommand<WriteTimestampParams>(CommandID::WriteTimestamp2);
    paramStruct->queryPool = queryPool.getHandle();
    paramStruct->query     = query;
}
}  // namespace priv
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SECONDARYCOMMANDBUFFERVK_H_
