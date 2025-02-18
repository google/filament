// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/metal/CommandBufferMTL.h"

#include "absl/container/flat_hash_map.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/native/BindGroupTracker.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Commands.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/metal/BindGroupMTL.h"
#include "dawn/native/metal/BufferMTL.h"
#include "dawn/native/metal/ComputePipelineMTL.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/PipelineLayoutMTL.h"
#include "dawn/native/metal/QuerySetMTL.h"
#include "dawn/native/metal/RenderPipelineMTL.h"
#include "dawn/native/metal/SamplerMTL.h"
#include "dawn/native/metal/TextureMTL.h"
#include "dawn/native/metal/UtilsMetal.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include <tint/tint.h>

namespace dawn::native::metal {

namespace {

MTLIndexType MTLIndexFormat(wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Uint16:
            return MTLIndexTypeUInt16;
        case wgpu::IndexFormat::Uint32:
            return MTLIndexTypeUInt32;
        case wgpu::IndexFormat::Undefined:
            DAWN_UNREACHABLE();
    }
}

template <typename PassDescriptor>
class SampleBufferAttachment {
  public:
    void SetSampleBuffer(PassDescriptor* descriptor, id<MTLCounterSampleBuffer> sampleBuffer);

    void SetStartSampleIndex(PassDescriptor* descriptor, NSUInteger sampleIndex);
    void SetEndSampleIndex(PassDescriptor* descriptor, NSUInteger sampleIndex);

  private:
    // Initialized to the maximum value, in order to start from 0 after the first increment.
    NSUInteger attachmentIndex = NSUIntegerMax;
    // TODO(dawn:1473): The maximum of sampleBufferAttachments depends on the length of MTLDevice's
    // counterSets, but Metal does not match the allowed maximum of sampleBufferAttachments with the
    // length of counterSets on AGX family. Hardcode as a constant and check this whenever Metal
    // could get the matched value.
    static constexpr NSUInteger kMaxSampleBufferAttachments = 4;
};

template <typename PassDescriptor>
void SampleBufferAttachment<PassDescriptor>::SetSampleBuffer(
    PassDescriptor* descriptor,
    id<MTLCounterSampleBuffer> sampleBuffer) {
    attachmentIndex++;
    DAWN_ASSERT(attachmentIndex < kMaxSampleBufferAttachments);
    descriptor.sampleBufferAttachments[attachmentIndex].sampleBuffer = sampleBuffer;
}

// Must be called after SetSampleBuffer
template <>
void SampleBufferAttachment<MTLRenderPassDescriptor>::SetStartSampleIndex(
    MTLRenderPassDescriptor* descriptor,
    NSUInteger sampleIndex) {
    DAWN_ASSERT(attachmentIndex < kMaxSampleBufferAttachments);
    descriptor.sampleBufferAttachments[attachmentIndex].startOfVertexSampleIndex = sampleIndex;
}

// Must be called after SetSampleBuffer
template <>
void SampleBufferAttachment<MTLRenderPassDescriptor>::SetEndSampleIndex(
    MTLRenderPassDescriptor* descriptor,
    NSUInteger sampleIndex) {
    DAWN_ASSERT(attachmentIndex < kMaxSampleBufferAttachments);
    descriptor.sampleBufferAttachments[attachmentIndex].endOfFragmentSampleIndex = sampleIndex;
}

// Must be called after SetSampleBuffer
template <>
void SampleBufferAttachment<MTLComputePassDescriptor>::SetStartSampleIndex(
    MTLComputePassDescriptor* descriptor,
    NSUInteger sampleIndex) {
    DAWN_ASSERT(attachmentIndex < kMaxSampleBufferAttachments);
    descriptor.sampleBufferAttachments[attachmentIndex].startOfEncoderSampleIndex = sampleIndex;
}

// Must be called after SetSampleBuffer
template <>
void SampleBufferAttachment<MTLComputePassDescriptor>::SetEndSampleIndex(
    MTLComputePassDescriptor* descriptor,
    NSUInteger sampleIndex) {
    // TODO(dawn:1473): Use MTLComputePassSampleBuffers or query method instead of the magic number
    // 4 when Metal could get the maximum of sampleBufferAttachments on compute pass
    DAWN_ASSERT(attachmentIndex < kMaxSampleBufferAttachments);
    descriptor.sampleBufferAttachments[attachmentIndex].endOfEncoderSampleIndex = sampleIndex;
}

template <typename PassDescriptor, typename BeginPass>
void SetSampleBufferAttachments(PassDescriptor* descriptor, BeginPass* cmd) {
    QuerySetBase* querySet = cmd->timestampWrites.querySet.Get();
    if (querySet == nullptr) {
        return;
    }
        SampleBufferAttachment<PassDescriptor> sampleBufferAttachment;
        sampleBufferAttachment.SetSampleBuffer(descriptor,
                                               ToBackend(querySet)->GetCounterSampleBuffer());
        uint32_t beginningOfPassWriteIndex = cmd->timestampWrites.beginningOfPassWriteIndex;
        sampleBufferAttachment.SetStartSampleIndex(
            descriptor, beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined
                            ? NSUInteger(beginningOfPassWriteIndex)
                            : MTLCounterDontSample);
        uint32_t endOfPassWriteIndex = cmd->timestampWrites.endOfPassWriteIndex;
        sampleBufferAttachment.SetEndSampleIndex(
            descriptor, endOfPassWriteIndex != wgpu::kQuerySetIndexUndefined
                            ? NSUInteger(endOfPassWriteIndex)
                            : MTLCounterDontSample);
}

NSRef<MTLComputePassDescriptor> CreateMTLComputePassDescriptor(BeginComputePassCmd* computePass) {
    // Note that this creates a descriptor that's autoreleased so we don't use AcquireNSRef
    NSRef<MTLComputePassDescriptor> descriptorRef =
        [MTLComputePassDescriptor computePassDescriptor];
    MTLComputePassDescriptor* descriptor = descriptorRef.Get();
    // MTLDispatchTypeSerial is the same dispatch type as the deafult MTLComputeCommandEncoder.
    // MTLDispatchTypeConcurrent requires memory barriers to ensure multiple commands synchronize
    // access to the same resources, which we may support it later.
    descriptor.dispatchType = MTLDispatchTypeSerial;

    SetSampleBufferAttachments(descriptor, computePass);

    return descriptorRef;
}

NSRef<MTLRenderPassDescriptor> CreateMTLRenderPassDescriptor(
    const Device* device,
    BeginRenderPassCmd* renderPass,
    bool useCounterSamplingAtStageBoundary) {
    // Note that this creates a descriptor that's autoreleased so we don't use AcquireNSRef
    NSRef<MTLRenderPassDescriptor> descriptorRef = [MTLRenderPassDescriptor renderPassDescriptor];
    MTLRenderPassDescriptor* descriptor = descriptorRef.Get();

    for (auto attachment : IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
        uint8_t i = static_cast<uint8_t>(attachment);
        auto& attachmentInfo = renderPass->colorAttachments[attachment];

        switch (attachmentInfo.loadOp) {
            case wgpu::LoadOp::Clear:
                descriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
                descriptor.colorAttachments[i].clearColor =
                    MTLClearColorMake(attachmentInfo.clearColor.r, attachmentInfo.clearColor.g,
                                      attachmentInfo.clearColor.b, attachmentInfo.clearColor.a);
                break;

            case wgpu::LoadOp::Load:
                descriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
                break;

            case wgpu::LoadOp::ExpandResolveTexture:
                // The loading of resolve texture -> MSAA attachment is inserted at the beginning of
                // the render pass. We don't care about the intial value of the MSAA attachment.
                descriptor.colorAttachments[i].loadAction = MTLLoadActionDontCare;
                break;

            case wgpu::LoadOp::Undefined:
                DAWN_UNREACHABLE();
                break;
        }

        auto colorAttachment = ToBackend(attachmentInfo.view)->GetAttachmentInfo();
        descriptor.colorAttachments[i].texture = colorAttachment.texture.Get();
        descriptor.colorAttachments[i].level = colorAttachment.baseMipLevel;
        descriptor.colorAttachments[i].slice = colorAttachment.baseArrayLayer;
        // We'd validated that depthSlice must be undefined and set to 0 for 2d texture views.
        descriptor.colorAttachments[i].depthPlane = attachmentInfo.depthSlice;

        bool hasResolveTarget = attachmentInfo.resolveTarget != nullptr;
        if (hasResolveTarget) {
            auto resolveAttachment = ToBackend(attachmentInfo.resolveTarget)->GetAttachmentInfo();
            descriptor.colorAttachments[i].resolveTexture = resolveAttachment.texture.Get();
            descriptor.colorAttachments[i].resolveLevel = resolveAttachment.baseMipLevel;
            descriptor.colorAttachments[i].resolveSlice = resolveAttachment.baseArrayLayer;

            switch (attachmentInfo.storeOp) {
                case wgpu::StoreOp::Store:
                    descriptor.colorAttachments[i].storeAction =
                        MTLStoreActionStoreAndMultisampleResolve;
                    break;
                case wgpu::StoreOp::Discard:
                    descriptor.colorAttachments[i].storeAction = MTLStoreActionMultisampleResolve;
                    break;
                case wgpu::StoreOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }
        } else {
            switch (attachmentInfo.storeOp) {
                case wgpu::StoreOp::Store:
                    descriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
                    break;
                case wgpu::StoreOp::Discard:
                    descriptor.colorAttachments[i].storeAction = MTLStoreActionDontCare;
                    break;
                case wgpu::StoreOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }
        }
    }

    if (renderPass->attachmentState->HasDepthStencilAttachment()) {
        auto& attachmentInfo = renderPass->depthStencilAttachment;

        auto depthStencilAttachment = ToBackend(attachmentInfo.view)->GetAttachmentInfo();
        const Format& format = attachmentInfo.view->GetFormat();

        if (format.HasDepth()) {
            descriptor.depthAttachment.texture = depthStencilAttachment.texture.Get();
            descriptor.depthAttachment.level = depthStencilAttachment.baseMipLevel;
            descriptor.depthAttachment.slice = depthStencilAttachment.baseArrayLayer;

            switch (attachmentInfo.depthStoreOp) {
                case wgpu::StoreOp::Store:
                    descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                    break;

                case wgpu::StoreOp::Discard:
                    descriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
                    break;

                case wgpu::StoreOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }

            switch (attachmentInfo.depthLoadOp) {
                case wgpu::LoadOp::Clear:
                    descriptor.depthAttachment.loadAction = MTLLoadActionClear;
                    descriptor.depthAttachment.clearDepth = attachmentInfo.clearDepth;
                    break;

                case wgpu::LoadOp::Load:
                    descriptor.depthAttachment.loadAction = MTLLoadActionLoad;
                    break;

                case wgpu::LoadOp::ExpandResolveTexture:
                case wgpu::LoadOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }
        }

        if (format.HasStencil()) {
            descriptor.stencilAttachment.texture = depthStencilAttachment.texture.Get();
            descriptor.stencilAttachment.level = depthStencilAttachment.baseMipLevel;
            descriptor.stencilAttachment.slice = depthStencilAttachment.baseArrayLayer;

            switch (attachmentInfo.stencilStoreOp) {
                case wgpu::StoreOp::Store:
                    descriptor.stencilAttachment.storeAction = MTLStoreActionStore;
                    break;

                case wgpu::StoreOp::Discard:
                    descriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
                    break;

                case wgpu::StoreOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }

            switch (attachmentInfo.stencilLoadOp) {
                case wgpu::LoadOp::Clear:
                    descriptor.stencilAttachment.loadAction = MTLLoadActionClear;
                    descriptor.stencilAttachment.clearStencil = attachmentInfo.clearStencil;
                    break;

                case wgpu::LoadOp::Load:
                    descriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
                    break;

                case wgpu::LoadOp::ExpandResolveTexture:
                case wgpu::LoadOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }
        }
    }

    if (renderPass->occlusionQuerySet.Get() != nullptr) {
        descriptor.visibilityResultBuffer =
            ToBackend(renderPass->occlusionQuerySet.Get())->GetVisibilityBuffer();
    }

    if (useCounterSamplingAtStageBoundary) {
        SetSampleBufferAttachments(descriptor, renderPass);
    }

    if (renderPass->attachmentState->HasPixelLocalStorage()) {
        const std::vector<wgpu::TextureFormat>& storageAttachmentSlots =
            renderPass->attachmentState->GetStorageAttachmentSlots();
        std::vector<ColorAttachmentIndex> storageAttachmentPacking =
            renderPass->attachmentState->ComputeStorageAttachmentPackingInColorAttachments();

        for (size_t attachment = 0; attachment < storageAttachmentSlots.size(); attachment++) {
            uint8_t i = static_cast<uint8_t>(storageAttachmentPacking[attachment]);
            MTLRenderPassColorAttachmentDescriptor* mtlAttachment = descriptor.colorAttachments[i];

            // For implicit pixel local storage slots use transient memoryless textures.
            if (storageAttachmentSlots[attachment] == wgpu::TextureFormat::Undefined) {
                NSRef<MTLTextureDescriptor> texDescRef = AcquireNSRef([MTLTextureDescriptor new]);
                MTLTextureDescriptor* texDesc = texDescRef.Get();
                texDesc.textureType = MTLTextureType2D;
                texDesc.width = renderPass->width;
                texDesc.height = renderPass->height;
                texDesc.usage = MTLTextureUsageRenderTarget;
                texDesc.storageMode = MTLStorageModeMemoryless;

                texDesc.pixelFormat =
                    MetalPixelFormat(device, RenderPipelineBase::kImplicitPLSSlotFormat);

                NSPRef<id<MTLTexture>> implicitAttachment =
                    AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:texDesc]);

                mtlAttachment.loadAction = MTLLoadActionClear;
                mtlAttachment.clearColor = MTLClearColorMake(0, 0, 0, 0);
                mtlAttachment.storeAction = MTLStoreActionDontCare;
                mtlAttachment.texture = *implicitAttachment;
                continue;
            }

            auto& attachmentInfo = renderPass->storageAttachments[attachment];

            switch (attachmentInfo.loadOp) {
                case wgpu::LoadOp::Clear:
                    mtlAttachment.loadAction = MTLLoadActionClear;
                    mtlAttachment.clearColor =
                        MTLClearColorMake(attachmentInfo.clearColor.r, attachmentInfo.clearColor.g,
                                          attachmentInfo.clearColor.b, attachmentInfo.clearColor.a);
                    break;

                case wgpu::LoadOp::Load:
                    mtlAttachment.loadAction = MTLLoadActionLoad;
                    break;

                case wgpu::LoadOp::ExpandResolveTexture:
                    // The loading of resolve texture -> MSAA attachment is inserted at the
                    // beginning of the render pass. We don't care about the intial value of the
                    // MSAA attachment.
                    mtlAttachment.loadAction = MTLLoadActionDontCare;
                    break;

                case wgpu::LoadOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }

            switch (attachmentInfo.storeOp) {
                case wgpu::StoreOp::Store:
                    mtlAttachment.storeAction = MTLStoreActionStore;
                    break;
                case wgpu::StoreOp::Discard:
                    mtlAttachment.storeAction = MTLStoreActionDontCare;
                    break;
                case wgpu::StoreOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }

            auto storageAttachment = ToBackend(attachmentInfo.storage)->GetAttachmentInfo();
            mtlAttachment.texture = storageAttachment.texture.Get();
            mtlAttachment.level = storageAttachment.baseMipLevel;
            mtlAttachment.slice = storageAttachment.baseArrayLayer;
        }
    }

    return descriptorRef;
}

void EncodeEmptyBlitEncoderForWriteTimestamp(Device* device,
                                             CommandRecordingContext* commandContext,
                                             WriteTimestampCmd* cmd) {
    commandContext->EndBlit();

    auto scopedDescriptor = AcquireNSRef([[MTLBlitPassDescriptor alloc] init]);
    MTLBlitPassDescriptor* descriptor = scopedDescriptor.Get();
    if (cmd->querySet.Get() != nullptr) {
        descriptor.sampleBufferAttachments[0].sampleBuffer =
            ToBackend(cmd->querySet.Get())->GetCounterSampleBuffer();
        descriptor.sampleBufferAttachments[0].startOfEncoderSampleIndex = MTLCounterDontSample;
        descriptor.sampleBufferAttachments[0].endOfEncoderSampleIndex = NSUInteger(cmd->queryIndex);

        id<MTLBlitCommandEncoder> blit = commandContext->BeginBlit(descriptor);
        if (device->IsToggleEnabled(Toggle::MetalUseMockBlitEncoderForWriteTimestamp)) {
            [blit fillBuffer:device->GetMockBlitMtlBuffer() range:NSMakeRange(0, 1) value:0];
        }
        commandContext->EndBlit();
    }
}

// Metal uses a physical addressing mode which means buffers in the shading language are
// just pointers to the virtual address of their start. This means there is no way to know
// the length of a buffer to compute the length() of unsized arrays at the end of storage
// buffers. Tint implements the length() of unsized arrays by requiring an extra
// buffer that contains the length of other buffers. This structure that keeps track of the
// length of storage buffers and can apply them to the reserved "buffer length buffer" when
// needed for a draw or a dispatch.
struct StorageBufferLengthTracker {
    wgpu::ShaderStage dirtyStages = wgpu::ShaderStage::None;

    // The lengths of buffers are stored as 32bit integers because that is the width the
    // MSL code generated by Tint expects.
    // UBOs require we align the max buffer count to 4 elements (16 bytes).
    static constexpr size_t MaxBufferCount = ((kGenericMetalBufferSlots + 3) / 4) * 4;
    PerStage<std::array<uint32_t, MaxBufferCount>> data;

    void Apply(id<MTLRenderCommandEncoder> render,
               RenderPipeline* pipeline,
               bool enableVertexPulling) {
        wgpu::ShaderStage stagesToApply =
            dirtyStages & pipeline->GetStagesRequiringStorageBufferLength();

        if (stagesToApply == wgpu::ShaderStage::None) {
            return;
        }

        if (stagesToApply & wgpu::ShaderStage::Vertex) {
            uint32_t bufferCount =
                ToBackend(pipeline->GetLayout())->GetBufferBindingCount(SingleShaderStage::Vertex);

            if (enableVertexPulling) {
                bufferCount += pipeline->GetVertexBufferCount();
            }

            bufferCount = Align(bufferCount, 4);
            DAWN_ASSERT(bufferCount <= data[SingleShaderStage::Vertex].size());

            [render setVertexBytes:data[SingleShaderStage::Vertex].data()
                            length:sizeof(uint32_t) * bufferCount
                           atIndex:kBufferLengthBufferSlot];
        }

        if (stagesToApply & wgpu::ShaderStage::Fragment) {
            uint32_t bufferCount = ToBackend(pipeline->GetLayout())
                                       ->GetBufferBindingCount(SingleShaderStage::Fragment);
            bufferCount = Align(bufferCount, 4);
            DAWN_ASSERT(bufferCount <= data[SingleShaderStage::Fragment].size());

            [render setFragmentBytes:data[SingleShaderStage::Fragment].data()
                              length:sizeof(uint32_t) * bufferCount
                             atIndex:kBufferLengthBufferSlot];
        }

        // Only mark clean stages that were actually applied.
        dirtyStages ^= stagesToApply;
    }

    void Apply(id<MTLComputeCommandEncoder> compute, ComputePipeline* pipeline) {
        if (!(dirtyStages & wgpu::ShaderStage::Compute)) {
            return;
        }

        if (!pipeline->RequiresStorageBufferLength()) {
            return;
        }

        uint32_t bufferCount =
            ToBackend(pipeline->GetLayout())->GetBufferBindingCount(SingleShaderStage::Compute);
        bufferCount = Align(bufferCount, 4);
        DAWN_ASSERT(bufferCount <= data[SingleShaderStage::Compute].size());

        [compute setBytes:data[SingleShaderStage::Compute].data()
                   length:sizeof(uint32_t) * bufferCount
                  atIndex:kBufferLengthBufferSlot];

        dirtyStages ^= wgpu::ShaderStage::Compute;
    }
};

// Keeps track of the dirty bind groups so they can be lazily applied when we know the
// pipeline state.
// Bind groups may be inherited because bind groups are packed in the buffer /
// texture tables in contiguous order.
class BindGroupTracker : public BindGroupTrackerBase<true, uint64_t> {
  public:
    explicit BindGroupTracker(StorageBufferLengthTracker* lengthTracker)
        : BindGroupTrackerBase(), mLengthTracker(lengthTracker) {}

    template <typename Encoder>
    void Apply(Encoder encoder) {
        BeforeApply();
        for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
            ApplyBindGroup(encoder, index, ToBackend(mBindGroups[index]), mDynamicOffsets[index],
                           ToBackend(mPipelineLayout));
        }
        AfterApply();
    }

  private:
    // Handles a call to SetBindGroup, directing the commands to the correct encoder.
    // There is a single function that takes both encoders to factor code. Other approaches
    // like templates wouldn't work because the name of methods are different between the
    // two encoder types.
    void ApplyBindGroupImpl(id<MTLRenderCommandEncoder> render,
                            id<MTLComputeCommandEncoder> compute,
                            BindGroupIndex index,
                            BindGroup* group,
                            const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets,
                            PipelineLayout* pipelineLayout) {
        // TODO(crbug.com/dawn/854): Maintain buffers and offsets arrays in BindGroup
        // so that we only have to do one setVertexBuffers and one setFragmentBuffers
        // call here.
        for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
            const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);

            bool hasVertStage =
                bindingInfo.visibility & wgpu::ShaderStage::Vertex && render != nullptr;
            bool hasFragStage =
                bindingInfo.visibility & wgpu::ShaderStage::Fragment && render != nullptr;
            bool hasComputeStage =
                bindingInfo.visibility & wgpu::ShaderStage::Compute && compute != nullptr;

            uint32_t vertIndex = 0;
            uint32_t fragIndex = 0;
            uint32_t computeIndex = 0;

            if (hasVertStage) {
                vertIndex = pipelineLayout->GetBindingIndexInfo(
                    SingleShaderStage::Vertex)[index][bindingIndex];
            }
            if (hasFragStage) {
                fragIndex = pipelineLayout->GetBindingIndexInfo(
                    SingleShaderStage::Fragment)[index][bindingIndex];
            }
            if (hasComputeStage) {
                computeIndex = pipelineLayout->GetBindingIndexInfo(
                    SingleShaderStage::Compute)[index][bindingIndex];
            }

            auto HandleTextureBinding = [&]() {
                auto textureView = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                id<MTLTexture> texture = textureView->GetMTLTexture();
                if (hasVertStage &&
                    mBoundTextures[SingleShaderStage::Vertex][vertIndex] != texture) {
                    mBoundTextures[SingleShaderStage::Vertex][vertIndex] = texture;
                    [render setVertexTexture:texture atIndex:vertIndex];
                }
                if (hasFragStage &&
                    mBoundTextures[SingleShaderStage::Fragment][fragIndex] != texture) {
                    mBoundTextures[SingleShaderStage::Fragment][fragIndex] = texture;
                    [render setFragmentTexture:texture atIndex:fragIndex];
                }
                if (hasComputeStage &&
                    mBoundTextures[SingleShaderStage::Compute][computeIndex] != texture) {
                    mBoundTextures[SingleShaderStage::Compute][computeIndex] = texture;
                    [compute setTexture:texture atIndex:computeIndex];
                }
            };

            MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& layout) {
                    const BufferBinding& binding = group->GetBindingAsBufferBinding(bindingIndex);
                    ToBackend(binding.buffer)->TrackUsage();
                    const id<MTLBuffer> buffer = ToBackend(binding.buffer)->GetMTLBuffer();
                    NSUInteger offset = binding.offset;

                    // TODO(crbug.com/dawn/854): Record bound buffer status to use
                    // setBufferOffset to achieve better performance.
                    if (layout.hasDynamicOffset) {
                        // Dynamic buffers are packed at the front of BindingIndices.
                        offset += dynamicOffsets[bindingIndex];
                    }

                    if (hasVertStage) {
                        mLengthTracker->data[SingleShaderStage::Vertex][vertIndex] = binding.size;
                        mLengthTracker->dirtyStages |= wgpu::ShaderStage::Vertex;
                        [render setVertexBuffers:&buffer
                                         offsets:&offset
                                       withRange:NSMakeRange(vertIndex, 1)];
                    }
                    if (hasFragStage) {
                        mLengthTracker->data[SingleShaderStage::Fragment][fragIndex] = binding.size;
                        mLengthTracker->dirtyStages |= wgpu::ShaderStage::Fragment;
                        [render setFragmentBuffers:&buffer
                                           offsets:&offset
                                         withRange:NSMakeRange(fragIndex, 1)];
                    }
                    if (hasComputeStage) {
                        mLengthTracker->data[SingleShaderStage::Compute][computeIndex] =
                            binding.size;
                        mLengthTracker->dirtyStages |= wgpu::ShaderStage::Compute;
                        [compute setBuffers:&buffer
                                    offsets:&offset
                                  withRange:NSMakeRange(computeIndex, 1)];
                    }
                },
                [&](const SamplerBindingInfo&) {
                    auto sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                    id<MTLSamplerState> samplerState = sampler->GetMTLSamplerState();
                    if (hasVertStage &&
                        mBoundSamplers[SingleShaderStage::Vertex][vertIndex] != samplerState) {
                        mBoundSamplers[SingleShaderStage::Vertex][vertIndex] = samplerState;
                        [render setVertexSamplerState:samplerState atIndex:vertIndex];
                    }
                    if (hasFragStage &&
                        mBoundSamplers[SingleShaderStage::Fragment][fragIndex] != samplerState) {
                        mBoundSamplers[SingleShaderStage::Fragment][fragIndex] = samplerState;
                        [render setFragmentSamplerState:samplerState atIndex:fragIndex];
                    }
                    if (hasComputeStage &&
                        mBoundSamplers[SingleShaderStage::Compute][computeIndex] != samplerState) {
                        mBoundSamplers[SingleShaderStage::Compute][computeIndex] = samplerState;
                        [compute setSamplerState:sampler->GetMTLSamplerState()
                                         atIndex:computeIndex];
                    }
                },
                [&](const StaticSamplerBindingInfo&) {
                    // Static samplers are handled in the frontend.
                    // TODO(crbug.com/dawn/2482): Implement static samplers in the
                    // Metal backend.
                    DAWN_UNREACHABLE();
                },
                [&](const TextureBindingInfo&) { HandleTextureBinding(); },
                [&](const StorageTextureBindingInfo&) { HandleTextureBinding(); },
                [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); });
        }
    }

    template <typename... Args>
    void ApplyBindGroup(id<MTLRenderCommandEncoder> encoder, Args&&... args) {
        ApplyBindGroupImpl(encoder, nullptr, std::forward<Args&&>(args)...);
    }

    template <typename... Args>
    void ApplyBindGroup(id<MTLComputeCommandEncoder> encoder, Args&&... args) {
        ApplyBindGroupImpl(nullptr, encoder, std::forward<Args&&>(args)...);
    }

    raw_ptr<StorageBufferLengthTracker> mLengthTracker;

    // Keep track of current texture and sampler bindings to minimize state changes even when bind
    // groups are dirtied. It's safe to keep raw pointers here since if an entry is set here, the
    // texture/sampler is bound in Metal and the Metal runtime will keep them alive.
    PerStage<absl::flat_hash_map<uint32_t, id<MTLTexture>>> mBoundTextures;
    PerStage<absl::flat_hash_map<uint32_t, id<MTLSamplerState>>> mBoundSamplers;
};

// Keeps track of the dirty vertex buffer values so they can be lazily applied when we know
// all the relevant state.
class VertexBufferTracker {
  public:
    explicit VertexBufferTracker(StorageBufferLengthTracker* lengthTracker)
        : mLengthTracker(lengthTracker) {}

    void OnSetVertexBuffer(VertexBufferSlot slot, Buffer* buffer, uint64_t offset) {
        id<MTLBuffer> mtlBuffer = buffer->GetMTLBuffer();
        if (mVertexBuffers[slot] == mtlBuffer && mVertexBufferOffsets[slot] == offset) {
            return;
        }
        buffer->TrackUsage();
        mVertexBuffers[slot] = mtlBuffer;
        mVertexBufferOffsets[slot] = offset;

        DAWN_ASSERT(buffer->GetSize() < std::numeric_limits<uint32_t>::max());
        mVertexBufferBindingSizes[slot] =
            static_cast<uint32_t>(buffer->GetAllocatedSize() - offset);
        mDirtyVertexBuffers.set(slot);
    }

    void OnSetPipeline(RenderPipeline* lastPipeline, RenderPipeline* pipeline) {
        // When a new pipeline is bound we must set all the vertex buffers again because
        // they might have been offset by the pipeline layout, and they might be packed
        // differently from the previous pipeline.
        mDirtyVertexBuffers |= pipeline->GetVertexBuffersUsed();
    }

    void Apply(id<MTLRenderCommandEncoder> encoder,
               RenderPipeline* pipeline,
               bool enableVertexPulling) {
        const auto& vertexBuffersToApply = mDirtyVertexBuffers & pipeline->GetVertexBuffersUsed();

        for (VertexBufferSlot slot : IterateBitSet(vertexBuffersToApply)) {
            uint32_t metalIndex = pipeline->GetMtlVertexBufferIndex(slot);

            if (enableVertexPulling) {
                // Insert lengths for vertex buffers bound as storage buffers
                mLengthTracker->data[SingleShaderStage::Vertex][metalIndex] =
                    mVertexBufferBindingSizes[slot];
                mLengthTracker->dirtyStages |= wgpu::ShaderStage::Vertex;
            }

            [encoder setVertexBuffers:&mVertexBuffers[slot]
                              offsets:&mVertexBufferOffsets[slot]
                            withRange:NSMakeRange(metalIndex, 1)];
        }

        mDirtyVertexBuffers.reset();
    }

  private:
    // All the indices in these arrays are Dawn vertex buffer indices
    VertexBufferMask mDirtyVertexBuffers;
    PerVertexBuffer<id<MTLBuffer>> mVertexBuffers;
    PerVertexBuffer<NSUInteger> mVertexBufferOffsets;
    PerVertexBuffer<uint32_t> mVertexBufferBindingSizes;

    raw_ptr<StorageBufferLengthTracker> mLengthTracker;
};

}  // anonymous namespace

void RecordCopyBufferToTexture(CommandRecordingContext* commandContext,
                               id<MTLBuffer> mtlBuffer,
                               uint64_t bufferSize,
                               uint64_t offset,
                               uint32_t bytesPerRow,
                               uint32_t rowsPerImage,
                               Texture* texture,
                               uint32_t mipLevel,
                               const Origin3D& origin,
                               Aspect aspect,
                               const Extent3D& copySize) {
    TextureBufferCopySplit splitCopies = ComputeTextureBufferCopySplit(
        texture, mipLevel, origin, copySize, bufferSize, offset, bytesPerRow, rowsPerImage, aspect);

    MTLBlitOption blitOption = texture->ComputeMTLBlitOption(aspect);

    for (const auto& copyInfo : splitCopies) {
        uint64_t bufferOffset = copyInfo.bufferOffset;
        switch (texture->GetDimension()) {
            case wgpu::TextureDimension::Undefined:
                DAWN_UNREACHABLE();
            case wgpu::TextureDimension::e1D: {
                [commandContext->EnsureBlit()
                         copyFromBuffer:mtlBuffer
                           sourceOffset:bufferOffset
                      sourceBytesPerRow:copyInfo.bytesPerRow
                    sourceBytesPerImage:copyInfo.bytesPerImage
                             sourceSize:MTLSizeMake(copyInfo.copyExtent.width, 1, 1)
                              toTexture:texture->GetMTLTexture(aspect)
                       destinationSlice:0
                       destinationLevel:mipLevel
                      destinationOrigin:MTLOriginMake(copyInfo.textureOrigin.x, 0, 0)
                                options:blitOption];
                break;
            }
            case wgpu::TextureDimension::e2D: {
                const MTLOrigin textureOrigin =
                    MTLOriginMake(copyInfo.textureOrigin.x, copyInfo.textureOrigin.y, 0);
                const MTLSize copyExtent =
                    MTLSizeMake(copyInfo.copyExtent.width, copyInfo.copyExtent.height, 1);

                for (uint32_t z = copyInfo.textureOrigin.z;
                     z < copyInfo.textureOrigin.z + copyInfo.copyExtent.depthOrArrayLayers; ++z) {
                    [commandContext->EnsureBlit() copyFromBuffer:mtlBuffer
                                                    sourceOffset:bufferOffset
                                               sourceBytesPerRow:copyInfo.bytesPerRow
                                             sourceBytesPerImage:copyInfo.bytesPerImage
                                                      sourceSize:copyExtent
                                                       toTexture:texture->GetMTLTexture(aspect)
                                                destinationSlice:z
                                                destinationLevel:mipLevel
                                               destinationOrigin:textureOrigin
                                                         options:blitOption];
                    bufferOffset += copyInfo.bytesPerImage;
                }
                break;
            }
            case wgpu::TextureDimension::e3D: {
                [commandContext->EnsureBlit()
                         copyFromBuffer:mtlBuffer
                           sourceOffset:bufferOffset
                      sourceBytesPerRow:copyInfo.bytesPerRow
                    sourceBytesPerImage:copyInfo.bytesPerImage
                             sourceSize:MTLSizeMake(copyInfo.copyExtent.width,
                                                    copyInfo.copyExtent.height,
                                                    copyInfo.copyExtent.depthOrArrayLayers)
                              toTexture:texture->GetMTLTexture(aspect)
                       destinationSlice:0
                       destinationLevel:mipLevel
                      destinationOrigin:MTLOriginMake(copyInfo.textureOrigin.x,
                                                      copyInfo.textureOrigin.y,
                                                      copyInfo.textureOrigin.z)
                                options:blitOption];
                break;
            }
        }
    }
}

// static
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

CommandBuffer::CommandBuffer(CommandEncoder* enc, const CommandBufferDescriptor* desc)
    : CommandBufferBase(enc, desc) {}

CommandBuffer::~CommandBuffer() = default;

MaybeError CommandBuffer::FillCommands(CommandRecordingContext* commandContext) {
    size_t nextComputePassNumber = 0;
    size_t nextRenderPassNumber = 0;

    auto LazyClearSyncScope = [](const SyncScopeResourceUsage& scope,
                                 CommandRecordingContext* commandContext) -> MaybeError {
        for (size_t i = 0; i < scope.textures.size(); ++i) {
            Texture* texture = ToBackend(scope.textures[i]);

            // Clear subresources that are not attachments. Attachments will be cleared in
            // RecordBeginRenderPass by setting the loadop to clear when the texture subresource
            // has not been initialized before the render pass.
            DAWN_TRY(scope.textureSyncInfos[i].Iterate(
                [&](const SubresourceRange& range, const TextureSyncInfo& syncInfo) -> MaybeError {
                    if (syncInfo.usage & ~(wgpu::TextureUsage::RenderAttachment |
                                           wgpu::TextureUsage::StorageAttachment)) {
                        DAWN_TRY(
                            texture->EnsureSubresourceContentInitialized(commandContext, range));
                    }
                    return {};
                }));
        }
        for (BufferBase* bufferBase : scope.buffers) {
            ToBackend(bufferBase)->EnsureDataInitialized(commandContext);
        }
        return {};
    };

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                BeginComputePassCmd* cmd = mCommands.NextCommand<BeginComputePassCmd>();

                for (TextureBase* texture :
                     GetResourceUsages().computePasses[nextComputePassNumber].referencedTextures) {
                    ToBackend(texture)->SynchronizeTextureBeforeUse(commandContext);
                }
                for (const SyncScopeResourceUsage& scope :
                     GetResourceUsages().computePasses[nextComputePassNumber].dispatchUsages) {
                    DAWN_TRY(LazyClearSyncScope(scope, commandContext));
                }
                commandContext->EndBlit();

                DAWN_TRY(EncodeComputePass(commandContext, cmd));

                nextComputePassNumber++;
                break;
            }

            case Command::BeginRenderPass: {
                BeginRenderPassCmd* cmd = mCommands.NextCommand<BeginRenderPassCmd>();

                for (TextureBase* texture :
                     this->GetResourceUsages().renderPasses[nextRenderPassNumber].textures) {
                    ToBackend(texture)->SynchronizeTextureBeforeUse(commandContext);
                }
                for (ExternalTextureBase* externalTexture : this->GetResourceUsages()
                                                                .renderPasses[nextRenderPassNumber]
                                                                .externalTextures) {
                    for (auto& view : externalTexture->GetTextureViews()) {
                        if (view.Get()) {
                            Texture* texture = ToBackend(view->GetTexture());
                            texture->SynchronizeTextureBeforeUse(commandContext);
                        }
                    }
                }
                DAWN_TRY(LazyClearSyncScope(GetResourceUsages().renderPasses[nextRenderPassNumber],
                                            commandContext));
                commandContext->EndBlit();

                // Before beginning, we encode a compute pass that converts multi draws into an ICB
                // if they exist.
                auto& indirectMetadata = GetIndirectDrawMetadata();
                const IndirectDrawMetadata& metadata = indirectMetadata[nextRenderPassNumber];
                const auto& multiDraws = metadata.GetIndirectMultiDraws();

                std::vector<MultiDrawExecutionData> multiDrawExecutions;

                if (!multiDraws.empty()) {
                    id<MTLComputeCommandEncoder> computeEnc = commandContext->BeginCompute();

                    DAWN_TRY_ASSIGN(multiDrawExecutions,
                                    PrepareMultiDraws(GetDevice(), computeEnc, multiDraws));

                    commandContext->EndCompute();
                }

                LazyClearRenderPassAttachments(cmd);
                if (cmd->attachmentState->HasDepthStencilAttachment() &&
                    ToBackend(cmd->depthStencilAttachment.view->GetTexture())
                        ->ShouldKeepInitialized()) {
                    // Ensure that depth and stencil never become uninitialized again if
                    // the attachment must stay initialized.
                    // For Toggle MetalKeepMultisubresourceDepthStencilTexturesInitialized.
                    cmd->depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
                    cmd->depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
                }
                Device* device = ToBackend(GetDevice());
                NSRef<MTLRenderPassDescriptor> descriptor = CreateMTLRenderPassDescriptor(
                    device, cmd, device->UseCounterSamplingAtStageBoundary());

                EmptyOcclusionQueries emptyOcclusionQueries;
                DAWN_TRY(EncodeMetalRenderPass(
                    device, commandContext, descriptor.Get(), cmd->width, cmd->height,
                    [&](id<MTLRenderCommandEncoder> encoder,
                        BeginRenderPassCmd* cmd) -> MaybeError {
                        return this->EncodeRenderPass(
                            encoder, cmd,
                            device->IsToggleEnabled(Toggle::MetalFillEmptyOcclusionQueriesWithZero)
                                ? &emptyOcclusionQueries
                                : nullptr,
                            multiDrawExecutions);
                    },
                    cmd));
                for (const auto& [querySet, queryIndex] : emptyOcclusionQueries) {
                    [commandContext->EnsureBlit()
                        fillBuffer:querySet->GetVisibilityBuffer()
                             range:NSMakeRange(queryIndex * sizeof(uint64_t), sizeof(uint64_t))
                             value:0u];
                }
                nextRenderPassNumber++;
                break;
            }

            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                if (copy->size == 0) {
                    // Skip no-op copies.
                    break;
                }

                auto srcBuffer = ToBackend(copy->source);
                srcBuffer->EnsureDataInitialized(commandContext);
                srcBuffer->TrackUsage();
                auto dstBuffer = ToBackend(copy->destination);
                dstBuffer->EnsureDataInitializedAsDestination(commandContext,
                                                              copy->destinationOffset, copy->size);
                dstBuffer->TrackUsage();

                [commandContext->EnsureBlit()
                       copyFromBuffer:ToBackend(copy->source)->GetMTLBuffer()
                         sourceOffset:copy->sourceOffset
                             toBuffer:ToBackend(copy->destination)->GetMTLBuffer()
                    destinationOffset:copy->destinationOffset
                                 size:copy->size];
                break;
            }

            case Command::CopyBufferToTexture: {
                CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                auto& src = copy->source;
                auto& dst = copy->destination;
                auto& copySize = copy->copySize;
                Buffer* buffer = ToBackend(src.buffer.Get());
                Texture* texture = ToBackend(dst.texture.Get());

                buffer->EnsureDataInitialized(commandContext);
                DAWN_TRY(
                    EnsureDestinationTextureInitialized(commandContext, texture, dst, copySize));

                buffer->TrackUsage();
                texture->SynchronizeTextureBeforeUse(commandContext);
                RecordCopyBufferToTexture(commandContext, buffer->GetMTLBuffer(), buffer->GetSize(),
                                          src.offset, src.bytesPerRow, src.rowsPerImage, texture,
                                          dst.mipLevel, dst.origin, dst.aspect, copySize);
                break;
            }

            case Command::CopyTextureToBuffer: {
                CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                auto& src = copy->source;
                auto& dst = copy->destination;
                auto& copySize = copy->copySize;
                Texture* texture = ToBackend(src.texture.Get());
                Buffer* buffer = ToBackend(dst.buffer.Get());

                buffer->EnsureDataInitializedAsDestination(commandContext, copy);

                texture->SynchronizeTextureBeforeUse(commandContext);
                DAWN_TRY(texture->EnsureSubresourceContentInitialized(
                    commandContext, GetSubresourcesAffectedByCopy(src, copySize)));
                buffer->TrackUsage();

                TextureBufferCopySplit splitCopies = ComputeTextureBufferCopySplit(
                    texture, src.mipLevel, src.origin, copySize, buffer->GetSize(), dst.offset,
                    dst.bytesPerRow, dst.rowsPerImage, src.aspect);

                for (const auto& copyInfo : splitCopies) {
                    MTLBlitOption blitOption = texture->ComputeMTLBlitOption(src.aspect);
                    uint64_t bufferOffset = copyInfo.bufferOffset;

                    switch (texture->GetDimension()) {
                        case wgpu::TextureDimension::Undefined:
                            DAWN_UNREACHABLE();
                        case wgpu::TextureDimension::e1D: {
                            [commandContext->EnsureBlit()
                                         copyFromTexture:texture->GetMTLTexture(src.aspect)
                                             sourceSlice:0
                                             sourceLevel:src.mipLevel
                                            sourceOrigin:MTLOriginMake(copyInfo.textureOrigin.x, 0,
                                                                       0)
                                              sourceSize:MTLSizeMake(copyInfo.copyExtent.width, 1,
                                                                     1)
                                                toBuffer:buffer->GetMTLBuffer()
                                       destinationOffset:bufferOffset
                                  destinationBytesPerRow:copyInfo.bytesPerRow
                                destinationBytesPerImage:copyInfo.bytesPerImage
                                                 options:blitOption];
                            break;
                        }

                        case wgpu::TextureDimension::e2D: {
                            const MTLOrigin textureOrigin = MTLOriginMake(
                                copyInfo.textureOrigin.x, copyInfo.textureOrigin.y, 0);
                            const MTLSize copyExtent = MTLSizeMake(copyInfo.copyExtent.width,
                                                                   copyInfo.copyExtent.height, 1);

                            for (uint32_t z = copyInfo.textureOrigin.z;
                                 z <
                                 copyInfo.textureOrigin.z + copyInfo.copyExtent.depthOrArrayLayers;
                                 ++z) {
                                [commandContext->EnsureBlit()
                                             copyFromTexture:texture->GetMTLTexture(src.aspect)
                                                 sourceSlice:z
                                                 sourceLevel:src.mipLevel
                                                sourceOrigin:textureOrigin
                                                  sourceSize:copyExtent
                                                    toBuffer:buffer->GetMTLBuffer()
                                           destinationOffset:bufferOffset
                                      destinationBytesPerRow:copyInfo.bytesPerRow
                                    destinationBytesPerImage:copyInfo.bytesPerImage
                                                     options:blitOption];
                                bufferOffset += copyInfo.bytesPerImage;
                            }
                            break;
                        }
                        case wgpu::TextureDimension::e3D: {
                            [commandContext->EnsureBlit()
                                         copyFromTexture:texture->GetMTLTexture(src.aspect)
                                             sourceSlice:0
                                             sourceLevel:src.mipLevel
                                            sourceOrigin:MTLOriginMake(copyInfo.textureOrigin.x,
                                                                       copyInfo.textureOrigin.y,
                                                                       copyInfo.textureOrigin.z)
                                              sourceSize:MTLSizeMake(
                                                             copyInfo.copyExtent.width,
                                                             copyInfo.copyExtent.height,
                                                             copyInfo.copyExtent.depthOrArrayLayers)
                                                toBuffer:buffer->GetMTLBuffer()
                                       destinationOffset:bufferOffset
                                  destinationBytesPerRow:copyInfo.bytesPerRow
                                destinationBytesPerImage:copyInfo.bytesPerImage
                                                 options:blitOption];
                            break;
                        }
                    }
                }
                break;
            }

            case Command::CopyTextureToTexture: {
                CopyTextureToTextureCmd* copy = mCommands.NextCommand<CopyTextureToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                Texture* srcTexture = ToBackend(copy->source.texture.Get());
                Texture* dstTexture = ToBackend(copy->destination.texture.Get());

                srcTexture->SynchronizeTextureBeforeUse(commandContext);
                dstTexture->SynchronizeTextureBeforeUse(commandContext);
                DAWN_TRY(srcTexture->EnsureSubresourceContentInitialized(
                    commandContext, GetSubresourcesAffectedByCopy(copy->source, copy->copySize)));
                DAWN_TRY(EnsureDestinationTextureInitialized(commandContext, dstTexture,
                                                             copy->destination, copy->copySize));

                const MTLSize sizeOneSlice =
                    MTLSizeMake(copy->copySize.width, copy->copySize.height, 1);

                uint32_t sourceLayer = 0;
                uint32_t sourceOriginZ = 0;

                uint32_t destinationLayer = 0;
                uint32_t destinationOriginZ = 0;

                uint32_t* sourceZPtr;
                if (srcTexture->GetDimension() == wgpu::TextureDimension::e2D) {
                    sourceZPtr = &sourceLayer;
                } else {
                    sourceZPtr = &sourceOriginZ;
                }

                uint32_t* destinationZPtr;
                if (dstTexture->GetDimension() == wgpu::TextureDimension::e2D) {
                    destinationZPtr = &destinationLayer;
                } else {
                    destinationZPtr = &destinationOriginZ;
                }

                // TODO(crbug.com/dawn/782): Do a single T2T copy if both are 1D or 3D.
                for (uint32_t z = 0; z < copy->copySize.depthOrArrayLayers; ++z) {
                    *sourceZPtr = copy->source.origin.z + z;
                    *destinationZPtr = copy->destination.origin.z + z;

                    // Hold the ref until out of scope
                    NSPRef<id<MTLTexture>> dstTextureView =
                        dstTexture->CreateFormatView(srcTexture->GetFormat().format);

                    [commandContext->EnsureBlit()
                          copyFromTexture:srcTexture->GetMTLTexture(copy->source.aspect)
                              sourceSlice:sourceLayer
                              sourceLevel:copy->source.mipLevel
                             sourceOrigin:MTLOriginMake(copy->source.origin.x,
                                                        copy->source.origin.y, sourceOriginZ)
                               sourceSize:sizeOneSlice
                                toTexture:dstTextureView.Get()
                         destinationSlice:destinationLayer
                         destinationLevel:copy->destination.mipLevel
                        destinationOrigin:MTLOriginMake(copy->destination.origin.x,
                                                        copy->destination.origin.y,
                                                        destinationOriginZ)];
                }
                break;
            }

            case Command::ClearBuffer: {
                ClearBufferCmd* cmd = mCommands.NextCommand<ClearBufferCmd>();
                if (cmd->size == 0) {
                    // Skip no-op copies.
                    break;
                }
                Buffer* dstBuffer = ToBackend(cmd->buffer.Get());

                bool clearedToZero = dstBuffer->EnsureDataInitializedAsDestination(
                    commandContext, cmd->offset, cmd->size);

                if (!clearedToZero) {
                    dstBuffer->TrackUsage();
                    [commandContext->EnsureBlit() fillBuffer:dstBuffer->GetMTLBuffer()
                                                       range:NSMakeRange(cmd->offset, cmd->size)
                                                       value:0u];
                }

                break;
            }

            case Command::ResolveQuerySet: {
                ResolveQuerySetCmd* cmd = mCommands.NextCommand<ResolveQuerySetCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                Buffer* destination = ToBackend(cmd->destination.Get());

                destination->EnsureDataInitializedAsDestination(
                    commandContext, cmd->destinationOffset, cmd->queryCount * sizeof(uint64_t));

                if (querySet->GetQueryType() == wgpu::QueryType::Occlusion) {
                    destination->TrackUsage();
                    [commandContext->EnsureBlit()
                           copyFromBuffer:querySet->GetVisibilityBuffer()
                             sourceOffset:NSUInteger(cmd->firstQuery * sizeof(uint64_t))
                                 toBuffer:destination->GetMTLBuffer()
                        destinationOffset:NSUInteger(cmd->destinationOffset)
                                     size:NSUInteger(cmd->queryCount * sizeof(uint64_t))];
                } else {
                    destination->TrackUsage();
                    if (GetDevice()->IsToggleEnabled(
                            Toggle::MetalSerializeTimestampGenerationAndResolution)) {
                        DAWN_TRY(commandContext->EncodeSharedEventWorkaround());
                    }
                    [commandContext->EnsureBlit()
                          resolveCounters:querySet->GetCounterSampleBuffer()
                                  inRange:NSMakeRange(cmd->firstQuery, cmd->queryCount)
                        destinationBuffer:destination->GetMTLBuffer()
                        destinationOffset:NSUInteger(cmd->destinationOffset)];
                }
                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();

                if (ToBackend(GetDevice())->UseCounterSamplingAtStageBoundary()) {
                    // Simulate writeTimestamp cmd between blit commands on the devices which
                    // supports counter sampling at stage boundary.
                    EncodeEmptyBlitEncoderForWriteTimestamp(ToBackend(GetDevice()), commandContext,
                                                            cmd);

                } else {
                        DAWN_ASSERT(ToBackend(GetDevice())->UseCounterSamplingAtCommandBoundary());
                        [commandContext->EnsureBlit()
                            sampleCountersInBuffer:ToBackend(cmd->querySet.Get())
                                                       ->GetCounterSampleBuffer()
                                     atSampleIndex:NSUInteger(cmd->queryIndex)
                                       withBarrier:YES];
                }

                break;
            }

            case Command::InsertDebugMarker: {
                // MTLCommandBuffer does not implement insertDebugSignpost
                SkipCommand(&mCommands, type);
                break;
            }

            case Command::PopDebugGroup: {
                mCommands.NextCommand<PopDebugGroupCmd>();
                [commandContext->GetCommands() popDebugGroup];
                break;
            }

            case Command::PushDebugGroup: {
                PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                char* label = mCommands.NextData<char>(cmd->length + 1);
                NSRef<NSString> mtlLabel =
                    AcquireNSRef([[NSString alloc] initWithUTF8String:label]);
                [commandContext->GetCommands() pushDebugGroup:mtlLabel.Get()];
                break;
            }

            case Command::WriteBuffer: {
                WriteBufferCmd* write = mCommands.NextCommand<WriteBufferCmd>();
                const uint64_t offset = write->offset;
                const uint64_t size = write->size;
                if (size == 0) {
                    continue;
                }

                Buffer* dstBuffer = ToBackend(write->buffer.Get());
                uint8_t* data = mCommands.NextData<uint8_t>(size);
                Device* device = ToBackend(GetDevice());

                UploadHandle uploadHandle;
                DAWN_TRY_ASSIGN(uploadHandle,
                                device->GetDynamicUploader()->Allocate(
                                    size, device->GetQueue()->GetPendingCommandSerial(),
                                    kCopyBufferToBufferOffsetAlignment));
                DAWN_ASSERT(uploadHandle.mappedBuffer != nullptr);
                memcpy(uploadHandle.mappedBuffer, data, size);

                dstBuffer->EnsureDataInitializedAsDestination(commandContext, offset, size);

                dstBuffer->TrackUsage();
                [commandContext->EnsureBlit()
                       copyFromBuffer:ToBackend(uploadHandle.stagingBuffer)->GetMTLBuffer()
                         sourceOffset:uploadHandle.startOffset
                             toBuffer:dstBuffer->GetMTLBuffer()
                    destinationOffset:offset
                                 size:size];
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }
    }

    commandContext->EndBlit();

    return {};
}

MaybeError CommandBuffer::EncodeComputePass(CommandRecordingContext* commandContext,
                                            BeginComputePassCmd* computePassCmd) {
    ComputePipeline* lastPipeline = nullptr;
    StorageBufferLengthTracker storageBufferLengths = {};
    BindGroupTracker bindGroups(&storageBufferLengths);

    id<MTLComputeCommandEncoder> encoder;
    // When counter sampling is supported at stage boundary, begin a configurable compute pass
    // encoder which is supported since macOS 11.0+ and iOS 14.0+ and set timestamp writes to
    // compute pass descriptor, otherwise begin a default compute pass encoder, and simulate
    // timestamp writes using sampleCountersInBuffer API at the beginning and end of compute pass.
    if (ToBackend(GetDevice())->UseCounterSamplingAtStageBoundary()) {
        NSRef<MTLComputePassDescriptor> descriptor = CreateMTLComputePassDescriptor(computePassCmd);
        encoder = commandContext->BeginCompute(descriptor.Get());

    } else {
        encoder = commandContext->BeginCompute();

            if (computePassCmd->timestampWrites.beginningOfPassWriteIndex !=
                wgpu::kQuerySetIndexUndefined) {
                DAWN_ASSERT(ToBackend(GetDevice())->UseCounterSamplingAtCommandBoundary());

                [encoder
                    sampleCountersInBuffer:ToBackend(computePassCmd->timestampWrites.querySet.Get())
                                               ->GetCounterSampleBuffer()
                             atSampleIndex:NSUInteger(computePassCmd->timestampWrites
                                                          .beginningOfPassWriteIndex)
                               withBarrier:YES];
            }
    }
    SetDebugName(GetDevice(), encoder, "Dawn_ComputePassEncoder", computePassCmd->label);

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                mCommands.NextCommand<EndComputePassCmd>();

                // Simulate timestamp write at the end of render pass if it does not support
                // counter sampling at stage boundary.
                if (ToBackend(GetDevice())->UseCounterSamplingAtCommandBoundary() &&
                    computePassCmd->timestampWrites.endOfPassWriteIndex !=
                        wgpu::kQuerySetIndexUndefined) {
                    DAWN_ASSERT(!ToBackend(GetDevice())->UseCounterSamplingAtStageBoundary());

                    [encoder
                        sampleCountersInBuffer:ToBackend(
                                                   computePassCmd->timestampWrites.querySet.Get())
                                                   ->GetCounterSampleBuffer()
                                 atSampleIndex:NSUInteger(computePassCmd->timestampWrites
                                                              .endOfPassWriteIndex)
                                   withBarrier:YES];
                }

                commandContext->EndCompute();
                return {};
            }

            case Command::Dispatch: {
                DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();

                // Skip noop dispatches, it can causes issues on some systems.
                if (dispatch->x == 0 || dispatch->y == 0 || dispatch->z == 0) {
                    break;
                }

                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline);

                [encoder dispatchThreadgroups:MTLSizeMake(dispatch->x, dispatch->y, dispatch->z)
                        threadsPerThreadgroup:lastPipeline->GetLocalWorkGroupSize()];
                break;
            }

            case Command::DispatchIndirect: {
                DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();

                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline);

                Buffer* buffer = ToBackend(dispatch->indirectBuffer.Get());
                buffer->TrackUsage();
                id<MTLBuffer> indirectBuffer = buffer->GetMTLBuffer();
                [encoder
                    dispatchThreadgroupsWithIndirectBuffer:indirectBuffer
                                      indirectBufferOffset:dispatch->indirectOffset
                                     threadsPerThreadgroup:lastPipeline->GetLocalWorkGroupSize()];
                break;
            }

            case Command::SetComputePipeline: {
                SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                lastPipeline = ToBackend(cmd->pipeline).Get();

                bindGroups.OnSetPipeline(lastPipeline);

                lastPipeline->Encode(encoder);
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = mCommands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                bindGroups.OnSetBindGroup(cmd->index, ToBackend(cmd->group.Get()),
                                          cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }

            case Command::InsertDebugMarker: {
                InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                char* label = mCommands.NextData<char>(cmd->length + 1);
                NSRef<NSString> mtlLabel =
                    AcquireNSRef([[NSString alloc] initWithUTF8String:label]);
                [encoder insertDebugSignpost:mtlLabel.Get()];
                break;
            }

            case Command::PopDebugGroup: {
                mCommands.NextCommand<PopDebugGroupCmd>();

                [encoder popDebugGroup];
                break;
            }

            case Command::PushDebugGroup: {
                PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                char* label = mCommands.NextData<char>(cmd->length + 1);
                NSRef<NSString> mtlLabel =
                    AcquireNSRef([[NSString alloc] initWithUTF8String:label]);
                [encoder pushDebugGroup:mtlLabel.Get()];
                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());

                [encoder sampleCountersInBuffer:querySet->GetCounterSampleBuffer()
                                  atSampleIndex:NSUInteger(cmd->queryIndex)
                                    withBarrier:YES];

                break;
            }

            default: {
                DAWN_UNREACHABLE();
                break;
            }
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
}

MaybeError CommandBuffer::EncodeRenderPass(
    id<MTLRenderCommandEncoder> encoder,
    BeginRenderPassCmd* renderPassCmd,
    EmptyOcclusionQueries* emptyOcclusionQueries,
    const std::vector<MultiDrawExecutionData>& multiDrawExecutions) {
    bool enableVertexPulling = GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling);
    RenderPipeline* lastPipeline = nullptr;
    id<MTLBuffer> indexBuffer = nullptr;
    uint32_t indexBufferBaseOffset = 0;
    MTLIndexType indexBufferType;
    uint64_t indexFormatSize = 0;
    uint32_t multiDrawIndex = 0;

    bool didDrawInCurrentOcclusionQuery = false;

    StorageBufferLengthTracker storageBufferLengths = {};
    VertexBufferTracker vertexBuffers(&storageBufferLengths);
    BindGroupTracker bindGroups(&storageBufferLengths);

    // Simulate timestamp write at the beginning of render pass by
    // sampleCountersInBuffer if it does not support counter sampling at stage boundary.
    if (ToBackend(GetDevice())->UseCounterSamplingAtCommandBoundary() &&
        renderPassCmd->timestampWrites.beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
        DAWN_ASSERT(!ToBackend(GetDevice())->UseCounterSamplingAtStageBoundary());

        [encoder
            sampleCountersInBuffer:ToBackend(renderPassCmd->timestampWrites.querySet.Get())
                                       ->GetCounterSampleBuffer()
                     atSampleIndex:NSUInteger(
                                       renderPassCmd->timestampWrites.beginningOfPassWriteIndex)
                       withBarrier:YES];
    }

    SetDebugName(GetDevice(), encoder, "Dawn_RenderPassEncoder", renderPassCmd->label);

    auto EncodeRenderBundleCommand = [&](CommandIterator* iter, Command type) {
        switch (type) {
            case Command::Draw: {
                DrawCmd* draw = iter->NextCommand<DrawCmd>();

                vertexBuffers.Apply(encoder, lastPipeline, enableVertexPulling);
                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline, enableVertexPulling);

                // The instance count must be non-zero, otherwise no-op
                if (draw->instanceCount != 0) {
                    // MTLFeatureSet_iOS_GPUFamily3_v1 does not support baseInstance
                    if (draw->firstInstance == 0) {
                        [encoder drawPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                    vertexStart:draw->firstVertex
                                    vertexCount:draw->vertexCount
                                  instanceCount:draw->instanceCount];
                        didDrawInCurrentOcclusionQuery = true;
                    } else {
                        [encoder drawPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                    vertexStart:draw->firstVertex
                                    vertexCount:draw->vertexCount
                                  instanceCount:draw->instanceCount
                                   baseInstance:draw->firstInstance];
                        didDrawInCurrentOcclusionQuery = true;
                    }
                }
                break;
            }

            case Command::DrawIndexed: {
                DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();

                vertexBuffers.Apply(encoder, lastPipeline, enableVertexPulling);
                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline, enableVertexPulling);

                // The index and instance count must be non-zero, otherwise no-op
                if (draw->indexCount != 0 && draw->instanceCount != 0) {
                    // MTLFeatureSet_iOS_GPUFamily3_v1 does not support baseInstance and
                    // baseVertex.
                    if (draw->baseVertex == 0 && draw->firstInstance == 0) {
                        [encoder drawIndexedPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                            indexCount:draw->indexCount
                                             indexType:indexBufferType
                                           indexBuffer:indexBuffer
                                     indexBufferOffset:indexBufferBaseOffset +
                                                       draw->firstIndex * indexFormatSize
                                         instanceCount:draw->instanceCount];
                        didDrawInCurrentOcclusionQuery = true;
                    } else {
                        [encoder drawIndexedPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                            indexCount:draw->indexCount
                                             indexType:indexBufferType
                                           indexBuffer:indexBuffer
                                     indexBufferOffset:indexBufferBaseOffset +
                                                       draw->firstIndex * indexFormatSize
                                         instanceCount:draw->instanceCount
                                            baseVertex:draw->baseVertex
                                          baseInstance:draw->firstInstance];
                        didDrawInCurrentOcclusionQuery = true;
                    }
                }
                break;
            }

            case Command::DrawIndirect: {
                DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();

                vertexBuffers.Apply(encoder, lastPipeline, enableVertexPulling);
                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline, enableVertexPulling);

                Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                buffer->TrackUsage();
                id<MTLBuffer> indirectBuffer = buffer->GetMTLBuffer();
                [encoder drawPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                          indirectBuffer:indirectBuffer
                    indirectBufferOffset:draw->indirectOffset];
                didDrawInCurrentOcclusionQuery = true;
                break;
            }

            case Command::DrawIndexedIndirect: {
                DrawIndexedIndirectCmd* draw = iter->NextCommand<DrawIndexedIndirectCmd>();

                vertexBuffers.Apply(encoder, lastPipeline, enableVertexPulling);
                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline, enableVertexPulling);

                Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                DAWN_ASSERT(buffer != nullptr);

                buffer->TrackUsage();
                id<MTLBuffer> indirectBuffer = buffer->GetMTLBuffer();
                [encoder drawIndexedPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                     indexType:indexBufferType
                                   indexBuffer:indexBuffer
                             indexBufferOffset:indexBufferBaseOffset
                                indirectBuffer:indirectBuffer
                          indirectBufferOffset:draw->indirectOffset];
                didDrawInCurrentOcclusionQuery = true;
                break;
            }

            case Command::MultiDrawIndirect: {
                iter->NextCommand<MultiDrawIndirectCmd>();

                vertexBuffers.Apply(encoder, lastPipeline, enableVertexPulling);
                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline, enableVertexPulling);

                ExecuteMultiDraw(multiDrawExecutions[multiDrawIndex], encoder);
                multiDrawIndex++;
                break;
            }
            case Command::MultiDrawIndexedIndirect: {
                iter->NextCommand<MultiDrawIndexedIndirectCmd>();

                vertexBuffers.Apply(encoder, lastPipeline, enableVertexPulling);
                bindGroups.Apply(encoder);
                storageBufferLengths.Apply(encoder, lastPipeline, enableVertexPulling);

                ExecuteMultiDraw(multiDrawExecutions[multiDrawIndex], encoder);
                multiDrawIndex++;
                break;
            }

            case Command::InsertDebugMarker: {
                InsertDebugMarkerCmd* cmd = iter->NextCommand<InsertDebugMarkerCmd>();
                char* label = iter->NextData<char>(cmd->length + 1);
                NSRef<NSString> mtlLabel =
                    AcquireNSRef([[NSString alloc] initWithUTF8String:label]);
                [encoder insertDebugSignpost:mtlLabel.Get()];
                break;
            }

            case Command::PopDebugGroup: {
                iter->NextCommand<PopDebugGroupCmd>();

                [encoder popDebugGroup];
                break;
            }

            case Command::PushDebugGroup: {
                PushDebugGroupCmd* cmd = iter->NextCommand<PushDebugGroupCmd>();
                char* label = iter->NextData<char>(cmd->length + 1);
                NSRef<NSString> mtlLabel =
                    AcquireNSRef([[NSString alloc] initWithUTF8String:label]);
                [encoder pushDebugGroup:mtlLabel.Get()];
                break;
            }

            case Command::SetRenderPipeline: {
                SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();
                RenderPipeline* newPipeline = ToBackend(cmd->pipeline).Get();
                if (newPipeline == lastPipeline) {
                    break;
                }

                vertexBuffers.OnSetPipeline(lastPipeline, newPipeline);
                bindGroups.OnSetPipeline(newPipeline);

                [encoder setDepthStencilState:newPipeline->GetMTLDepthStencilState()];
                [encoder setFrontFacingWinding:newPipeline->GetMTLFrontFace()];
                [encoder setCullMode:newPipeline->GetMTLCullMode()];
                [encoder setDepthBias:newPipeline->GetDepthBias()
                           slopeScale:newPipeline->GetDepthBiasSlopeScale()
                                clamp:newPipeline->GetDepthBiasClamp()];

                // When using @builtin(frag_depth) we need to clamp to the viewport, otherwise
                // Metal writes the raw value to the depth buffer, which doesn't match other
                // APIs.
                MTLDepthClipMode clipMode =
                    (newPipeline->UsesFragDepth() || newPipeline->HasUnclippedDepth())
                        ? MTLDepthClipModeClamp
                        : MTLDepthClipModeClip;
                [encoder setDepthClipMode:clipMode];

                newPipeline->Encode(encoder);

                lastPipeline = newPipeline;
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = iter->NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                bindGroups.OnSetBindGroup(cmd->index, ToBackend(cmd->group.Get()),
                                          cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }

            case Command::SetIndexBuffer: {
                SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();
                auto b = ToBackend(cmd->buffer.Get());
                b->TrackUsage();
                indexBuffer = b->GetMTLBuffer();
                indexBufferBaseOffset = cmd->offset;
                indexBufferType = MTLIndexFormat(cmd->format);
                indexFormatSize = IndexFormatSize(cmd->format);
                break;
            }

            case Command::SetVertexBuffer: {
                SetVertexBufferCmd* cmd = iter->NextCommand<SetVertexBufferCmd>();

                vertexBuffers.OnSetVertexBuffer(cmd->slot, ToBackend(cmd->buffer.Get()),
                                                cmd->offset);
                break;
            }

            default:
                DAWN_UNREACHABLE();
                break;
        }
    };

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                mCommands.NextCommand<EndRenderPassCmd>();

                // Simulate timestamp write at the end of render pass if it does not support
                // counter sampling at stage boundary.
                if (ToBackend(GetDevice())->UseCounterSamplingAtCommandBoundary() &&
                    renderPassCmd->timestampWrites.endOfPassWriteIndex !=
                        wgpu::kQuerySetIndexUndefined) {
                    DAWN_ASSERT(!ToBackend(GetDevice())->UseCounterSamplingAtStageBoundary());

                    [encoder
                        sampleCountersInBuffer:ToBackend(
                                                   renderPassCmd->timestampWrites.querySet.Get())
                                                   ->GetCounterSampleBuffer()
                                 atSampleIndex:NSUInteger(renderPassCmd->timestampWrites
                                                              .endOfPassWriteIndex)
                                   withBarrier:YES];
                }

                return {};
            }

            case Command::SetStencilReference: {
                SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                [encoder setStencilReferenceValue:cmd->reference];
                break;
            }

            case Command::SetViewport: {
                SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();
                MTLViewport viewport;
                viewport.originX = cmd->x;
                viewport.originY = cmd->y;
                viewport.width = cmd->width;
                viewport.height = cmd->height;
                viewport.znear = cmd->minDepth;
                viewport.zfar = cmd->maxDepth;

                [encoder setViewport:viewport];
                break;
            }

            case Command::SetScissorRect: {
                SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                MTLScissorRect rect;
                rect.x = cmd->x;
                rect.y = cmd->y;
                rect.width = cmd->width;
                rect.height = cmd->height;

                [encoder setScissorRect:rect];
                break;
            }

            case Command::SetBlendConstant: {
                SetBlendConstantCmd* cmd = mCommands.NextCommand<SetBlendConstantCmd>();
                [encoder setBlendColorRed:cmd->color.r
                                    green:cmd->color.g
                                     blue:cmd->color.b
                                    alpha:cmd->color.a];
                break;
            }

            case Command::ExecuteBundles: {
                ExecuteBundlesCmd* cmd = mCommands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = mCommands.NextData<Ref<RenderBundleBase>>(cmd->count);

                for (uint32_t i = 0; i < cmd->count; ++i) {
                    CommandIterator* iter = bundles[i]->GetCommands();
                    iter->Reset();
                    while (iter->NextCommandId(&type)) {
                        EncodeRenderBundleCommand(iter, type);
                    }
                }
                break;
            }

            case Command::BeginOcclusionQuery: {
                BeginOcclusionQueryCmd* cmd = mCommands.NextCommand<BeginOcclusionQueryCmd>();

                [encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean
                                          offset:cmd->queryIndex * sizeof(uint64_t)];
                didDrawInCurrentOcclusionQuery = false;
                break;
            }

            case Command::EndOcclusionQuery: {
                EndOcclusionQueryCmd* cmd = mCommands.NextCommand<EndOcclusionQueryCmd>();

                [encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled
                                          offset:cmd->queryIndex * sizeof(uint64_t)];
                if (emptyOcclusionQueries) {
                    // Empty occlusion queries aren't filled to zero on Apple GPUs.
                    // Keep track of them so we can clear them if necessary.
                    auto key = std::make_pair(ToBackend(renderPassCmd->occlusionQuerySet.Get()),
                                              cmd->queryIndex);
                    if (!didDrawInCurrentOcclusionQuery) {
                        emptyOcclusionQueries->insert(std::move(key));
                    } else {
                        auto it = emptyOcclusionQueries->find(std::move(key));
                        if (it != emptyOcclusionQueries->end()) {
                            emptyOcclusionQueries->erase(it);
                        }
                    }
                }
                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());

                [encoder sampleCountersInBuffer:querySet->GetCounterSampleBuffer()
                                  atSampleIndex:NSUInteger(cmd->queryIndex)
                                    withBarrier:YES];

                break;
            }

            default: {
                EncodeRenderBundleCommand(&mCommands, type);
                break;
            }
        }
    }

    // EndRenderPass should have been called
    DAWN_UNREACHABLE();
}

}  // namespace dawn::native::metal
