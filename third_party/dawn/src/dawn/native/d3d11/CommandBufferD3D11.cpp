// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d11/CommandBufferD3D11.h"

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/WindowsUtils.h"
#include "dawn/native/ApplyClearColorValueWithDrawHelper.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BindGroupTrackerD3D11.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/ComputePipelineD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/Forward.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/QuerySetD3D11.h"
#include "dawn/native/d3d11/RenderPipelineD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d11 {
namespace {

DXGI_FORMAT DXGIIndexFormat(wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Uint16:
            return DXGI_FORMAT_R16_UINT;
        case wgpu::IndexFormat::Uint32:
            return DXGI_FORMAT_R32_UINT;
        default:
            DAWN_UNREACHABLE();
    }
}

class VertexBufferTracker {
  public:
    explicit VertexBufferTracker(const ScopedSwapStateCommandRecordingContext* commandContext)
        : mCommandContext(commandContext) {}

    ~VertexBufferTracker() {
        mD3D11Buffers = {};
        mStrides = {};
        mOffsets = {};
        mCommandContext->GetD3D11DeviceContext4()->IASetVertexBuffers(
            0, kMaxVertexBuffers, mD3D11Buffers.data(), mStrides.data(), mOffsets.data());
    }

    void OnSetVertexBuffer(VertexBufferSlot slot, ID3D11Buffer* buffer, uint64_t offset) {
        mD3D11Buffers[slot] = buffer;
        mOffsets[slot] = offset;
    }

    void Apply(const RenderPipeline* renderPipeline) {
        DAWN_ASSERT(renderPipeline != nullptr);

        // If the vertex state has changed, we need to update the strides.
        if (mLastAppliedRenderPipeline != renderPipeline) {
            mLastAppliedRenderPipeline = renderPipeline;
            for (VertexBufferSlot slot : IterateBitSet(renderPipeline->GetVertexBuffersUsed())) {
                mStrides[slot] = renderPipeline->GetVertexBuffer(slot).arrayStride;
            }
        }

        mCommandContext->GetD3D11DeviceContext4()->IASetVertexBuffers(
            0, kMaxVertexBuffers, mD3D11Buffers.data(), mStrides.data(), mOffsets.data());
    }

  private:
    raw_ptr<const ScopedSwapStateCommandRecordingContext> mCommandContext;
    raw_ptr<const RenderPipeline> mLastAppliedRenderPipeline = nullptr;
    PerVertexBuffer<ID3D11Buffer*> mD3D11Buffers = {};
    PerVertexBuffer<UINT> mStrides = {};
    PerVertexBuffer<UINT> mOffsets = {};
};

// Handle pixel local storage attachments and return a vector of all pixel local storage UAVs.
// - For implicit attachments, create the texture and clear it to 0.
// - For explicit attachments, clear them to the specified clear color if their load operation is
//   `clear`
ResultOrError<std::vector<ComPtr<ID3D11UnorderedAccessView>>>
HandlePixelLocalStorageAndGetPixelLocalStorageUAVs(
    DeviceBase* device,
    const BeginRenderPassCmd* renderPass,
    const ScopedSwapStateCommandRecordingContext* commandContext) {
    std::vector<ComPtr<ID3D11UnorderedAccessView>> pixelLocalStorageUAVs;
    auto d3d11DeviceContext = commandContext->GetD3D11DeviceContext4();

    const std::vector<wgpu::TextureFormat>& storageAttachmentSlots =
        renderPass->attachmentState->GetStorageAttachmentSlots();
    uint32_t nextImplicitAttachmentIndex = 0;
    for (size_t attachment = 0; attachment < storageAttachmentSlots.size(); attachment++) {
        ComPtr<ID3D11UnorderedAccessView> pixelLocalStorageUAV;
        if (storageAttachmentSlots[attachment] == wgpu::TextureFormat::Undefined) {
            // Get implicit pixel local storage attachment
            TextureViewBase* implicitPixelLocalStorageTextureView = nullptr;
            DAWN_TRY_ASSIGN(
                implicitPixelLocalStorageTextureView,
                ToBackend(device)->GetOrCreateCachedImplicitPixelLocalStorageAttachment(
                    renderPass->width, renderPass->height, nextImplicitAttachmentIndex));
            ++nextImplicitAttachmentIndex;

            // Get and clear the UAV of the implicit pixel local storage attachment
            DAWN_TRY_ASSIGN(pixelLocalStorageUAV, ToBackend(implicitPixelLocalStorageTextureView)
                                                      ->GetOrCreateD3D11UnorderedAccessView());

            // TODO(dawn:1704): investigate if we can only clear it when necessary.
            uint32_t clearValue[4] = {0, 0, 0, 0};
            d3d11DeviceContext->ClearUnorderedAccessViewUint(pixelLocalStorageUAV.Get(),
                                                             clearValue);
        } else {
            // Get the UAV of the explicit pixel local storage attachment
            auto& attachmentInfo = renderPass->storageAttachments[attachment];
            DAWN_TRY_ASSIGN(
                pixelLocalStorageUAV,
                ToBackend(attachmentInfo.storage.Get())->GetOrCreateD3D11UnorderedAccessView());

            // Execute the load operation of the pixel local storage attachment
            switch (attachmentInfo.loadOp) {
                case wgpu::LoadOp::Clear: {
                    switch (attachmentInfo.storage->GetFormat().format) {
                        case wgpu::TextureFormat::R32Float: {
                            float clearValue[4] = {static_cast<float>(attachmentInfo.clearColor.r),
                                                   0, 0, 0};
                            d3d11DeviceContext->ClearUnorderedAccessViewFloat(
                                pixelLocalStorageUAV.Get(), clearValue);
                            break;
                        }
                        case wgpu::TextureFormat::R32Sint: {
                            uint32_t clearValue[4] = {static_cast<uint32_t>(static_cast<int32_t>(
                                                          attachmentInfo.clearColor.r)),
                                                      0, 0, 0};
                            d3d11DeviceContext->ClearUnorderedAccessViewUint(
                                pixelLocalStorageUAV.Get(), clearValue);
                            break;
                        }
                        case wgpu::TextureFormat::R32Uint: {
                            uint32_t clearValue[4] = {
                                static_cast<uint32_t>(attachmentInfo.clearColor.r), 0, 0, 0};
                            d3d11DeviceContext->ClearUnorderedAccessViewUint(
                                pixelLocalStorageUAV.Get(), clearValue);
                            break;
                        }
                        default:
                            DAWN_UNREACHABLE();
                            break;
                    }
                    break;
                }
                case wgpu::LoadOp::Load:
                case wgpu::LoadOp::ExpandResolveTexture:
                    break;
                case wgpu::LoadOp::Undefined:
                    DAWN_UNREACHABLE();
                    break;
            }
        }
        pixelLocalStorageUAVs.push_back(pixelLocalStorageUAV);
    }

    return pixelLocalStorageUAVs;
}

}  // namespace

// Create CommandBuffer
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

MaybeError CommandBuffer::Execute(const ScopedSwapStateCommandRecordingContext* commandContext) {
    auto LazyClearSyncScope = [&commandContext](const SyncScopeResourceUsage& scope) -> MaybeError {
        for (size_t i = 0; i < scope.textures.size(); i++) {
            Texture* texture = ToBackend(scope.textures[i]);

            // Clear subresources that are not render attachments or storage attachment. Render
            // attachments will be cleared in RecordBeginRenderPass by setting the loadop to clear
            // when the texture subresource has not been initialized before the render pass. Storage
            // attachments will also be cleared in RecordBeginRenderPass by
            // ClearUnorderedAccessView*() when the texture subresource has not been initialized
            // before the render pass.
            DAWN_TRY(scope.textureSyncInfos[i].Iterate([&](const SubresourceRange& range,
                                                           TextureSyncInfo syncInfo) -> MaybeError {
                if (syncInfo.usage & ~(wgpu::TextureUsage::RenderAttachment |
                                       wgpu::TextureUsage::StorageAttachment)) {
                    DAWN_TRY(texture->EnsureSubresourceContentInitialized(commandContext, range));
                }
                return {};
            }));
        }

        for (BufferBase* buffer : scope.buffers) {
            DAWN_TRY(ToBackend(buffer)->EnsureDataInitialized(commandContext));
            buffer->MarkUsedInPendingCommands();
        }

        return {};
    };

    size_t nextComputePassNumber = 0;
    size_t nextRenderPassNumber = 0;

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                mCommands.NextCommand<BeginComputePassCmd>();
                for (BufferBase* buffer :
                     GetResourceUsages().computePasses[nextComputePassNumber].referencedBuffers) {
                    buffer->MarkUsedInPendingCommands();
                }
                for (TextureBase* texture :
                     GetResourceUsages().computePasses[nextComputePassNumber].referencedTextures) {
                    DAWN_TRY(ToBackend(texture)->SynchronizeTextureBeforeUse(commandContext));
                }
                for (const SyncScopeResourceUsage& scope :
                     GetResourceUsages().computePasses[nextComputePassNumber].dispatchUsages) {
                    for (TextureBase* texture : scope.textures) {
                        DAWN_TRY(ToBackend(texture)->SynchronizeTextureBeforeUse(commandContext));
                    }
                    DAWN_TRY(LazyClearSyncScope(scope));
                }
                DAWN_TRY(ExecuteComputePass(commandContext));

                nextComputePassNumber++;
                break;
            }

            case Command::BeginRenderPass: {
                auto* cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                for (TextureBase* texture :
                     GetResourceUsages().renderPasses[nextRenderPassNumber].textures) {
                    DAWN_TRY(ToBackend(texture)->SynchronizeTextureBeforeUse(commandContext));
                }
                for (ExternalTextureBase* externalTexture :
                     GetResourceUsages().renderPasses[nextRenderPassNumber].externalTextures) {
                    for (auto& view : externalTexture->GetTextureViews()) {
                        if (view.Get()) {
                            DAWN_TRY(ToBackend(view->GetTexture())
                                         ->SynchronizeTextureBeforeUse(commandContext));
                        }
                    }
                }
                DAWN_TRY(
                    LazyClearSyncScope(GetResourceUsages().renderPasses[nextRenderPassNumber]));
                DAWN_TRY(ExecuteRenderPass(cmd, commandContext));

                nextRenderPassNumber++;
                break;
            }

            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                if (copy->size == 0) {
                    // Skip no-op copies.
                    break;
                }

                Buffer* source = ToBackend(copy->source.Get());
                Buffer* destination = ToBackend(copy->destination.Get());

                // Buffer::Copy() will ensure the source and destination buffers are initialized.
                DAWN_TRY(Buffer::Copy(commandContext, source, copy->sourceOffset, copy->size,
                                      destination, copy->destinationOffset));
                source->MarkUsedInPendingCommands();
                destination->MarkUsedInPendingCommands();
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

                Buffer* buffer = ToBackend(src.buffer.Get());
                uint64_t bufferOffset = src.offset;
                Ref<BufferBase> stagingBuffer;
                // If the buffer is not mappable, we need to create a staging buffer and copy the
                // data from the buffer to the staging buffer.
                if (!buffer->IsCPUReadable()) {
                    const TexelBlockInfo& blockInfo =
                        ToBackend(dst.texture)->GetFormat().GetAspectInfo(dst.aspect).block;
                    // TODO(dawn:1768): use compute shader to copy data from buffer to texture.
                    BufferDescriptor desc;
                    DAWN_TRY_ASSIGN(desc.size,
                                    ComputeRequiredBytesInCopy(blockInfo, copy->copySize,
                                                               src.bytesPerRow, src.rowsPerImage));
                    desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
                    DAWN_TRY_ASSIGN(stagingBuffer, Buffer::Create(ToBackend(GetDevice()),
                                                                  Unpack(&desc), commandContext));

                    DAWN_TRY(Buffer::Copy(commandContext, buffer, src.offset,
                                          stagingBuffer->GetSize(), ToBackend(stagingBuffer.Get()),
                                          0));
                    buffer = ToBackend(stagingBuffer.Get());
                    bufferOffset = 0;
                }

                Buffer::ScopedMap scopedMap;
                DAWN_TRY_ASSIGN(scopedMap, Buffer::ScopedMap::Create(commandContext, buffer,
                                                                     wgpu::MapMode::Read));
                DAWN_TRY(buffer->EnsureDataInitialized(commandContext));

                Texture* texture = ToBackend(dst.texture.Get());
                DAWN_TRY(texture->SynchronizeTextureBeforeUse(commandContext));
                SubresourceRange subresources = GetSubresourcesAffectedByCopy(dst, copy->copySize);

                DAWN_ASSERT(scopedMap.GetMappedData());
                const uint8_t* data = scopedMap.GetMappedData() + bufferOffset;
                DAWN_TRY(texture->Write(commandContext, subresources, dst.origin, copy->copySize,
                                        data, src.bytesPerRow, src.rowsPerImage));

                buffer->MarkUsedInPendingCommands();
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

                SubresourceRange subresources = GetSubresourcesAffectedByCopy(src, copy->copySize);
                Texture* texture = ToBackend(src.texture.Get());
                DAWN_TRY(texture->SynchronizeTextureBeforeUse(commandContext));
                DAWN_TRY(
                    texture->EnsureSubresourceContentInitialized(commandContext, subresources));

                Buffer* buffer = ToBackend(dst.buffer.Get());
                Buffer::ScopedMap scopedDstMap;
                DAWN_TRY_ASSIGN(scopedDstMap, Buffer::ScopedMap::Create(commandContext, buffer,
                                                                        wgpu::MapMode::Write));

                DAWN_TRY(buffer->EnsureDataInitializedAsDestination(commandContext, copy));

                Texture::ReadCallback callback = [&](const uint8_t* data, uint64_t offset,
                                                     uint64_t size) -> MaybeError {
                    DAWN_TRY(ToBackend(dst.buffer)
                                 ->Write(commandContext, dst.offset + offset, data, size));
                    return {};
                };

                DAWN_TRY(ToBackend(src.texture)
                             ->Read(commandContext, subresources, src.origin, copy->copySize,
                                    dst.bytesPerRow, dst.rowsPerImage, callback));

                dst.buffer->MarkUsedInPendingCommands();
                break;
            }

            case Command::CopyTextureToTexture: {
                CopyTextureToTextureCmd* copy = mCommands.NextCommand<CopyTextureToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }

                DAWN_TRY(ToBackend(copy->source.texture.Get())
                             ->SynchronizeTextureBeforeUse(commandContext));
                DAWN_TRY(ToBackend(copy->destination.texture.Get())
                             ->SynchronizeTextureBeforeUse(commandContext));
                DAWN_TRY(Texture::Copy(commandContext, copy));
                break;
            }

            case Command::ClearBuffer: {
                ClearBufferCmd* cmd = mCommands.NextCommand<ClearBufferCmd>();
                if (cmd->size == 0) {
                    // Skip no-op fills.
                    break;
                }
                Buffer* buffer = ToBackend(cmd->buffer.Get());
                DAWN_TRY(buffer->Clear(commandContext, 0, cmd->offset, cmd->size));
                buffer->MarkUsedInPendingCommands();
                break;
            }

            case Command::ResolveQuerySet: {
                ResolveQuerySetCmd* cmd = mCommands.NextCommand<ResolveQuerySetCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                uint32_t firstQuery = cmd->firstQuery;
                uint32_t queryCount = cmd->queryCount;
                Buffer* destination = ToBackend(cmd->destination.Get());
                uint64_t destinationOffset = cmd->destinationOffset;

                DAWN_TRY(querySet->Resolve(commandContext, firstQuery, queryCount, destination,
                                           destinationOffset));
                destination->MarkUsedInPendingCommands();
                break;
            }

            case Command::WriteTimestamp: {
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");
            }

            case Command::WriteBuffer: {
                WriteBufferCmd* cmd = mCommands.NextCommand<WriteBufferCmd>();
                if (cmd->size == 0) {
                    // Skip no-op writes.
                    continue;
                }

                Buffer* dstBuffer = ToBackend(cmd->buffer.Get());
                uint8_t* data = mCommands.NextData<uint8_t>(cmd->size);
                DAWN_TRY(dstBuffer->Write(commandContext, cmd->offset, data, cmd->size));
                dstBuffer->MarkUsedInPendingCommands();

                break;
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                HandleDebugCommands(commandContext, &mCommands, type);
                break;
            }

            default:
                return DAWN_FORMAT_INTERNAL_ERROR("Unknown command type: %d", type);
        }
    }

    return {};
}

MaybeError CommandBuffer::ExecuteComputePass(
    const ScopedSwapStateCommandRecordingContext* commandContext) {
    ComputePipeline* lastPipeline = nullptr;
    ComputePassBindGroupTracker bindGroupTracker(commandContext);

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                mCommands.NextCommand<EndComputePassCmd>();
                return {};
            }

            case Command::Dispatch: {
                DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();

                DAWN_TRY(bindGroupTracker.Apply());

                DAWN_TRY(RecordNumWorkgroupsForDispatch(lastPipeline, commandContext, dispatch));
                commandContext->GetD3D11DeviceContext4()->Dispatch(dispatch->x, dispatch->y,
                                                                   dispatch->z);

                break;
            }

            case Command::DispatchIndirect: {
                DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();

                DAWN_TRY(bindGroupTracker.Apply());

                auto* indirectBuffer = ToGPUUsableBuffer(dispatch->indirectBuffer.Get());

                if (lastPipeline->UsesNumWorkgroups()) {
                    // Copy indirect args into the uniform buffer for built-in workgroup variables.
                    DAWN_TRY(Buffer::Copy(commandContext, indirectBuffer, dispatch->indirectOffset,
                                          sizeof(uint32_t) * 3, commandContext->GetUniformBuffer(),
                                          0));
                }

                ID3D11Buffer* d3dBuffer;
                DAWN_TRY_ASSIGN(d3dBuffer,
                                indirectBuffer->GetD3D11NonConstantBuffer(commandContext));
                commandContext->GetD3D11DeviceContext4()->DispatchIndirect(
                    d3dBuffer, dispatch->indirectOffset);
                break;
            }

            case Command::SetComputePipeline: {
                SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                lastPipeline = ToBackend(cmd->pipeline).Get();
                lastPipeline->ApplyNow(commandContext);
                bindGroupTracker.OnSetPipeline(lastPipeline);
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();

                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = mCommands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                bindGroupTracker.OnSetBindGroup(cmd->index, cmd->group.Get(),
                                                cmd->dynamicOffsetCount, dynamicOffsets);

                break;
            }

            case Command::WriteTimestamp: {
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                HandleDebugCommands(commandContext, &mCommands, type);
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
}

MaybeError CommandBuffer::ExecuteRenderPass(
    BeginRenderPassCmd* renderPass,
    const ScopedSwapStateCommandRecordingContext* commandContext) {
    // For the color attachments that the clear_color_with_draw workaround has applied, we can skip
    // the clear for them.
    for (auto i :
         IterateBitSet(ClearWithDrawHelper::GetAppliedColorAttachments(GetDevice(), renderPass))) {
        auto& colorAttachment = renderPass->colorAttachments[i];
        DAWN_ASSERT(colorAttachment.loadOp == wgpu::LoadOp::Clear);
        // Skip the clear as it will be handled by the workaround.
        colorAttachment.loadOp = wgpu::LoadOp::Load;
        // Mark the resource as initialized to avoid the lazy clear.
        SubresourceRange range = colorAttachment.view->GetSubresourceRange();
        colorAttachment.view->GetTexture()->SetIsSubresourceContentInitialized(true, range);
    }
    LazyClearRenderPassAttachments(renderPass);

    auto* d3d11DeviceContext = commandContext->GetD3D11DeviceContext4();
    // Hold ID3D11RenderTargetView ComPtr to make attachments alive.
    PerColorAttachment<ID3D11RenderTargetView*> d3d11RenderTargetViews = {};
    ColorAttachmentIndex attachmentCount{};
    // TODO(dawn:1815): Shrink the sparse attachments to accommodate more UAVs.
    for (auto i : IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
        TextureView* colorTextureView = ToBackend(renderPass->colorAttachments[i].view.Get());
        DAWN_TRY_ASSIGN(d3d11RenderTargetViews[i],
                        colorTextureView->GetOrCreateD3D11RenderTargetView(
                            renderPass->colorAttachments[i].depthSlice));
        if (renderPass->colorAttachments[i].loadOp == wgpu::LoadOp::Clear) {
            std::array<float, 4> clearColor =
                ConvertToFloatColor(renderPass->colorAttachments[i].clearColor);
            d3d11DeviceContext->ClearRenderTargetView(d3d11RenderTargetViews[i], clearColor.data());
        }
        attachmentCount = ityp::PlusOne(i);
    }

    ID3D11DepthStencilView* d3d11DepthStencilView = nullptr;
    if (renderPass->attachmentState->HasDepthStencilAttachment()) {
        auto* attachmentInfo = &renderPass->depthStencilAttachment;
        const Format& attachmentFormat = attachmentInfo->view->GetTexture()->GetFormat();

        TextureView* depthStencilTextureView =
            ToBackend(renderPass->depthStencilAttachment.view.Get());
        DAWN_TRY_ASSIGN(d3d11DepthStencilView,
                        depthStencilTextureView->GetOrCreateD3D11DepthStencilView(
                            attachmentInfo->depthReadOnly, attachmentInfo->stencilReadOnly));
        UINT clearFlags = 0;
        if (attachmentFormat.HasDepth() &&
            renderPass->depthStencilAttachment.depthLoadOp == wgpu::LoadOp::Clear) {
            clearFlags |= D3D11_CLEAR_DEPTH;
        }

        if (attachmentFormat.HasStencil() &&
            renderPass->depthStencilAttachment.stencilLoadOp == wgpu::LoadOp::Clear) {
            clearFlags |= D3D11_CLEAR_STENCIL;
        }

        d3d11DeviceContext->ClearDepthStencilView(d3d11DepthStencilView, clearFlags,
                                                  attachmentInfo->clearDepth,
                                                  attachmentInfo->clearStencil);
    }

    d3d11DeviceContext->OMSetRenderTargets(static_cast<uint8_t>(attachmentCount),
                                           d3d11RenderTargetViews.data(), d3d11DepthStencilView);

    std::vector<ComPtr<ID3D11UnorderedAccessView>> pixelLocalStorageUAVs;
    if (renderPass->attachmentState->HasPixelLocalStorage()) {
        DAWN_TRY_ASSIGN(pixelLocalStorageUAVs, HandlePixelLocalStorageAndGetPixelLocalStorageUAVs(
                                                   GetDevice(), renderPass, commandContext));
    }

    // Set viewport
    D3D11_VIEWPORT defautViewport;
    defautViewport.TopLeftX = 0;
    defautViewport.TopLeftY = 0;
    defautViewport.Width = renderPass->width;
    defautViewport.Height = renderPass->height;
    defautViewport.MinDepth = 0.0f;
    defautViewport.MaxDepth = 1.0f;
    d3d11DeviceContext->RSSetViewports(1, &defautViewport);

    // Set scissor
    D3D11_RECT scissor;
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = renderPass->width;
    scissor.bottom = renderPass->height;
    d3d11DeviceContext->RSSetScissorRects(1, &scissor);

    RenderPipeline* lastPipeline = nullptr;
    RenderPassBindGroupTracker bindGroupTracker(commandContext, std::move(pixelLocalStorageUAVs));
    VertexBufferTracker vertexBufferTracker(commandContext);
    std::array<float, 4> blendColor = {0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t stencilReference = 0;

    auto DoRenderBundleCommand = [&](CommandIterator* iter, Command type) -> MaybeError {
        switch (type) {
            case Command::Draw: {
                DrawCmd* draw = iter->NextCommand<DrawCmd>();

                DAWN_TRY(bindGroupTracker.Apply());
                vertexBufferTracker.Apply(lastPipeline);
                DAWN_TRY(RecordFirstIndexOffset(lastPipeline, commandContext, draw->firstVertex,
                                                draw->firstInstance));
                commandContext->GetD3D11DeviceContext4()->DrawInstanced(
                    draw->vertexCount, draw->instanceCount, draw->firstVertex, draw->firstInstance);

                break;
            }

            case Command::DrawIndexed: {
                DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();

                DAWN_TRY(bindGroupTracker.Apply());
                vertexBufferTracker.Apply(lastPipeline);
                DAWN_TRY(RecordFirstIndexOffset(lastPipeline, commandContext, draw->baseVertex,
                                                draw->firstInstance));
                commandContext->GetD3D11DeviceContext4()->DrawIndexedInstanced(
                    draw->indexCount, draw->instanceCount, draw->firstIndex, draw->baseVertex,
                    draw->firstInstance);

                break;
            }

            case Command::DrawIndirect: {
                DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();

                auto* indirectBuffer = ToGPUUsableBuffer(draw->indirectBuffer.Get());
                DAWN_ASSERT(indirectBuffer != nullptr);

                DAWN_TRY(bindGroupTracker.Apply());
                vertexBufferTracker.Apply(lastPipeline);

                if (lastPipeline->UsesVertexIndex() || lastPipeline->UsesInstanceIndex()) {
                    // Copy StartVertexLocation and StartInstanceLocation into the uniform buffer
                    // for built-in variables.
                    uint64_t offset =
                        draw->indirectOffset +
                        offsetof(D3D11_DRAW_INSTANCED_INDIRECT_ARGS, StartVertexLocation);
                    DAWN_TRY(Buffer::Copy(commandContext, indirectBuffer, offset,
                                          sizeof(uint32_t) * 2, commandContext->GetUniformBuffer(),
                                          0));
                }

                ID3D11Buffer* d3dBuffer;
                DAWN_TRY_ASSIGN(d3dBuffer,
                                indirectBuffer->GetD3D11NonConstantBuffer(commandContext));
                commandContext->GetD3D11DeviceContext4()->DrawInstancedIndirect(
                    d3dBuffer, draw->indirectOffset);

                break;
            }

            case Command::DrawIndexedIndirect: {
                DrawIndexedIndirectCmd* draw = iter->NextCommand<DrawIndexedIndirectCmd>();

                auto* indirectBuffer = ToGPUUsableBuffer(draw->indirectBuffer.Get());
                DAWN_ASSERT(indirectBuffer != nullptr);

                DAWN_TRY(bindGroupTracker.Apply());
                vertexBufferTracker.Apply(lastPipeline);

                if (lastPipeline->UsesVertexIndex() || lastPipeline->UsesInstanceIndex()) {
                    // Copy StartVertexLocation and StartInstanceLocation into the uniform buffer
                    // for built-in variables.
                    uint64_t offset =
                        draw->indirectOffset +
                        offsetof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS, BaseVertexLocation);
                    DAWN_TRY(Buffer::Copy(commandContext, indirectBuffer, offset,
                                          sizeof(uint32_t) * 2, commandContext->GetUniformBuffer(),
                                          0));
                }

                ID3D11Buffer* d3dBuffer;
                DAWN_TRY_ASSIGN(d3dBuffer,
                                indirectBuffer->GetD3D11NonConstantBuffer(commandContext));
                commandContext->GetD3D11DeviceContext4()->DrawIndexedInstancedIndirect(
                    d3dBuffer, draw->indirectOffset);

                break;
            }

            case Command::SetRenderPipeline: {
                SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();

                lastPipeline = ToBackend(cmd->pipeline.Get());
                lastPipeline->ApplyNow(commandContext, blendColor, stencilReference);
                bindGroupTracker.OnSetPipeline(lastPipeline);

                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();

                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = iter->NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                bindGroupTracker.OnSetBindGroup(cmd->index, cmd->group.Get(),
                                                cmd->dynamicOffsetCount, dynamicOffsets);

                break;
            }

            case Command::SetIndexBuffer: {
                SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();

                UINT indexBufferBaseOffset = cmd->offset;
                DXGI_FORMAT indexBufferFormat = DXGIIndexFormat(cmd->format);

                ID3D11Buffer* d3dBuffer;
                DAWN_TRY_ASSIGN(d3dBuffer, ToGPUUsableBuffer(cmd->buffer.Get())
                                               ->GetD3D11NonConstantBuffer(commandContext));
                commandContext->GetD3D11DeviceContext4()->IASetIndexBuffer(
                    d3dBuffer, indexBufferFormat, indexBufferBaseOffset);

                break;
            }

            case Command::SetVertexBuffer: {
                SetVertexBufferCmd* cmd = iter->NextCommand<SetVertexBufferCmd>();
                ID3D11Buffer* d3dBuffer;
                DAWN_TRY_ASSIGN(d3dBuffer, ToGPUUsableBuffer(cmd->buffer.Get())
                                               ->GetD3D11NonConstantBuffer(commandContext));
                vertexBufferTracker.OnSetVertexBuffer(cmd->slot, d3dBuffer, cmd->offset);
                break;
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                HandleDebugCommands(commandContext, iter, type);
                break;
            }

            default:
                DAWN_UNREACHABLE();
                break;
        }

        return {};
    };

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                mCommands.NextCommand<EndRenderPassCmd>();
                d3d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

                if (renderPass->attachmentState->GetSampleCount() <= 1) {
                    return {};
                }

                // Resolve multisampled textures.
                for (auto i :
                     IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                    const auto& attachment = renderPass->colorAttachments[i];
                    if (!attachment.resolveTarget.Get()) {
                        continue;
                    }

                    DAWN_ASSERT(attachment.view->GetAspects() == Aspect::Color);
                    DAWN_ASSERT(attachment.resolveTarget->GetAspects() == Aspect::Color);

                    Texture* resolveTexture = ToBackend(attachment.resolveTarget->GetTexture());
                    Texture* colorTexture = ToBackend(attachment.view->GetTexture());
                    uint32_t dstSubresource = resolveTexture->GetSubresourceIndex(
                        attachment.resolveTarget->GetBaseMipLevel(),
                        attachment.resolveTarget->GetBaseArrayLayer(), Aspect::Color);
                    uint32_t srcSubresource = colorTexture->GetSubresourceIndex(
                        attachment.view->GetBaseMipLevel(), attachment.view->GetBaseArrayLayer(),
                        Aspect::Color);
                    d3d11DeviceContext->ResolveSubresource(
                        resolveTexture->GetD3D11Resource(), dstSubresource,
                        colorTexture->GetD3D11Resource(), srcSubresource,
                        d3d::DXGITextureFormat(GetDevice(),
                                               attachment.resolveTarget->GetFormat().format));
                }

                return {};
            }

            case Command::SetStencilReference: {
                SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                stencilReference = cmd->reference;
                if (lastPipeline) {
                    lastPipeline->ApplyDepthStencilState(commandContext, stencilReference);
                }
                break;
            }

            case Command::SetViewport: {
                SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();

                D3D11_VIEWPORT viewport;
                viewport.TopLeftX = cmd->x;
                viewport.TopLeftY = cmd->y;
                viewport.Width = cmd->width;
                viewport.Height = cmd->height;
                viewport.MinDepth = cmd->minDepth;
                viewport.MaxDepth = cmd->maxDepth;
                commandContext->GetD3D11DeviceContext4()->RSSetViewports(1, &viewport);
                break;
            }

            case Command::SetScissorRect: {
                SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();

                D3D11_RECT scissorRect = {static_cast<LONG>(cmd->x), static_cast<LONG>(cmd->y),
                                          static_cast<LONG>(cmd->x + cmd->width),
                                          static_cast<LONG>(cmd->y + cmd->height)};
                commandContext->GetD3D11DeviceContext4()->RSSetScissorRects(1, &scissorRect);
                break;
            }

            case Command::SetBlendConstant: {
                SetBlendConstantCmd* cmd = mCommands.NextCommand<SetBlendConstantCmd>();
                blendColor = ConvertToFloatColor(cmd->color);
                if (lastPipeline) {
                    lastPipeline->ApplyBlendState(commandContext, blendColor);
                }
                break;
            }

            case Command::ExecuteBundles: {
                ExecuteBundlesCmd* cmd = mCommands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = mCommands.NextData<Ref<RenderBundleBase>>(cmd->count);
                for (uint32_t i = 0; i < cmd->count; ++i) {
                    CommandIterator* iter = bundles[i]->GetCommands();
                    iter->Reset();
                    while (iter->NextCommandId(&type)) {
                        DAWN_TRY(DoRenderBundleCommand(iter, type));
                    }
                }
                break;
            }

            case Command::BeginOcclusionQuery: {
                BeginOcclusionQueryCmd* cmd = mCommands.NextCommand<BeginOcclusionQueryCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                querySet->BeginQuery(commandContext->GetD3D11DeviceContext4(), cmd->queryIndex);
                break;
            }

            case Command::EndOcclusionQuery: {
                EndOcclusionQueryCmd* cmd = mCommands.NextCommand<EndOcclusionQueryCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                querySet->EndQuery(commandContext->GetD3D11DeviceContext4(), cmd->queryIndex);
                break;
            }

            case Command::WriteTimestamp:
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");

            default: {
                DAWN_TRY(DoRenderBundleCommand(&mCommands, type));
            }
        }
    }

    // EndRenderPass should have been called
    DAWN_UNREACHABLE();
}

void CommandBuffer::HandleDebugCommands(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    CommandIterator* iter,
    Command command) {
    switch (command) {
        case Command::InsertDebugMarker: {
            InsertDebugMarkerCmd* cmd = iter->NextCommand<InsertDebugMarkerCmd>();
            std::wstring label = UTF8ToWStr(iter->NextData<char>(cmd->length + 1));
            commandContext->GetD3DUserDefinedAnnotation()->SetMarker(label.c_str());
            break;
        }

        case Command::PopDebugGroup: {
            [[maybe_unused]] auto cmd = iter->NextCommand<PopDebugGroupCmd>();
            commandContext->GetD3DUserDefinedAnnotation()->EndEvent();
            break;
        }

        case Command::PushDebugGroup: {
            PushDebugGroupCmd* cmd = iter->NextCommand<PushDebugGroupCmd>();
            std::wstring label = UTF8ToWStr(iter->NextData<char>(cmd->length + 1));
            commandContext->GetD3DUserDefinedAnnotation()->BeginEvent(label.c_str());
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError CommandBuffer::RecordFirstIndexOffset(
    RenderPipeline* renderPipeline,
    const ScopedSwapStateCommandRecordingContext* commandContext,
    uint32_t firstVertex,
    uint32_t firstInstance) {
    constexpr uint32_t kFirstVertexOffset = 0;
    constexpr uint32_t kFirstInstanceOffset = 1;

    if (renderPipeline->UsesVertexIndex()) {
        commandContext->WriteUniformBuffer(kFirstVertexOffset, firstVertex);
    }
    if (renderPipeline->UsesInstanceIndex()) {
        commandContext->WriteUniformBuffer(kFirstInstanceOffset, firstInstance);
    }

    return commandContext->FlushUniformBuffer();
}

MaybeError CommandBuffer::RecordNumWorkgroupsForDispatch(
    ComputePipeline* computePipeline,
    const ScopedSwapStateCommandRecordingContext* commandContext,
    DispatchCmd* dispatchCmd) {
    if (!computePipeline->UsesNumWorkgroups()) {
        // Workgroup size is not used in shader, so we don't need to update the uniform buffer. The
        // original value in the uniform buffer will not be used, so we don't need to clear it.
        return {};
    }

    commandContext->WriteUniformBuffer(/*offset=*/0, dispatchCmd->x);
    commandContext->WriteUniformBuffer(/*offset=*/1, dispatchCmd->y);
    commandContext->WriteUniformBuffer(/*offset=*/2, dispatchCmd->z);
    return commandContext->FlushUniformBuffer();
}

}  // namespace dawn::native::d3d11
