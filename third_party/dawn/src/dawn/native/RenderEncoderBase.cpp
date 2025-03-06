// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/RenderEncoderBase.h"

#include <math.h>
#include <cstring>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

RenderEncoderBase::RenderEncoderBase(DeviceBase* device,
                                     StringView label,
                                     EncodingContext* encodingContext,
                                     Ref<AttachmentState> attachmentState,
                                     bool depthReadOnly,
                                     bool stencilReadOnly)
    : ProgrammableEncoder(device, label, encodingContext),
      mIndirectDrawMetadata(device->GetLimits()),
      mAttachmentState(std::move(attachmentState)),
      mDisableBaseVertex(device->IsToggleEnabled(Toggle::DisableBaseVertex)),
      mDisableBaseInstance(device->IsToggleEnabled(Toggle::DisableBaseInstance)) {
    mDepthReadOnly = depthReadOnly;
    mStencilReadOnly = stencilReadOnly;
}

RenderEncoderBase::RenderEncoderBase(DeviceBase* device,
                                     EncodingContext* encodingContext,
                                     ErrorTag errorTag,
                                     StringView label)
    : ProgrammableEncoder(device, encodingContext, errorTag, label),
      mIndirectDrawMetadata(device->GetLimits()),
      mDisableBaseVertex(device->IsToggleEnabled(Toggle::DisableBaseVertex)),
      mDisableBaseInstance(device->IsToggleEnabled(Toggle::DisableBaseInstance)) {}

void RenderEncoderBase::DestroyImpl() {
    // Remove reference to the attachment state so that we don't have lingering references to
    // it preventing it from being uncached in the device.
    mAttachmentState = nullptr;
}

const AttachmentState* RenderEncoderBase::GetAttachmentState() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mAttachmentState != nullptr);
    return mAttachmentState.Get();
}

bool RenderEncoderBase::IsDepthReadOnly() const {
    DAWN_ASSERT(!IsError());
    return mDepthReadOnly;
}

bool RenderEncoderBase::IsStencilReadOnly() const {
    DAWN_ASSERT(!IsError());
    return mStencilReadOnly;
}

uint64_t RenderEncoderBase::GetDrawCount() const {
    DAWN_ASSERT(!IsError());
    return mDrawCount;
}

Ref<AttachmentState> RenderEncoderBase::AcquireAttachmentState() {
    return std::move(mAttachmentState);
}

void RenderEncoderBase::APIDraw(uint32_t vertexCount,
                                uint32_t instanceCount,
                                uint32_t firstVertex,
                                uint32_t firstInstance) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                if (vertexCount == 0) {
                    GetDevice()->EmitWarningOnce(absl::StrFormat(
                        "Calling %s.Draw with a vertex count of 0 is unusual.", this));
                }
                if (instanceCount == 0) {
                    GetDevice()->EmitWarningOnce(absl::StrFormat(
                        "Calling %s.Draw with an instance count of 0 is unusual.", this));
                }

                DAWN_TRY(mCommandBufferState.ValidateCanDraw());

                if (!GetDevice()->HasFlexibleTextureViews()) {
                    DAWN_TRY(mCommandBufferState.ValidateNoDifferentTextureViewsOnSameTexture());
                }

                DAWN_INVALID_IF(mDisableBaseInstance && firstInstance != 0,
                                "First instance (%u) must be zero.", firstInstance);

                DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForVertexBuffer(vertexCount,
                                                                                  firstVertex));
                DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForInstanceBuffer(instanceCount,
                                                                                    firstInstance));
            }

            DrawCmd* draw = allocator->Allocate<DrawCmd>(Command::Draw);
            draw->vertexCount = vertexCount;
            draw->instanceCount = instanceCount;
            draw->firstVertex = firstVertex;
            draw->firstInstance = firstInstance;

            mDrawCount++;

            return {};
        },
        "encoding %s.Draw(%u, %u, %u, %u).", this, vertexCount, instanceCount, firstVertex,
        firstInstance);
}

void RenderEncoderBase::APIDrawIndexed(uint32_t indexCount,
                                       uint32_t instanceCount,
                                       uint32_t firstIndex,
                                       int32_t baseVertex,
                                       uint32_t firstInstance) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                if (indexCount == 0) {
                    GetDevice()->EmitWarningOnce(absl::StrFormat(
                        "Calling %s.Draw with an index count of 0 is unusual.", this));
                }
                if (instanceCount == 0) {
                    GetDevice()->EmitWarningOnce(absl::StrFormat(
                        "Calling %s.Draw with an instance count of 0 is unusual.", this));
                }

                DAWN_TRY(mCommandBufferState.ValidateCanDrawIndexed());

                if (!GetDevice()->HasFlexibleTextureViews()) {
                    DAWN_TRY(mCommandBufferState.ValidateNoDifferentTextureViewsOnSameTexture());
                }

                DAWN_INVALID_IF(mDisableBaseInstance && firstInstance != 0,
                                "First instance (%u) must be zero.", firstInstance);

                DAWN_INVALID_IF(mDisableBaseVertex && baseVertex != 0,
                                "Base vertex (%u) must be zero.", baseVertex);

                DAWN_TRY(mCommandBufferState.ValidateIndexBufferInRange(indexCount, firstIndex));

                // DrawIndexed only validate instance step mode vertex buffer
                DAWN_TRY(mCommandBufferState.ValidateBufferInRangeForInstanceBuffer(instanceCount,
                                                                                    firstInstance));
            }

            DrawIndexedCmd* draw = allocator->Allocate<DrawIndexedCmd>(Command::DrawIndexed);
            draw->indexCount = indexCount;
            draw->instanceCount = instanceCount;
            draw->firstIndex = firstIndex;
            draw->baseVertex = baseVertex;
            draw->firstInstance = firstInstance;

            mDrawCount++;

            return {};
        },
        "encoding %s.DrawIndexed(%u, %u, %u, %i, %u).", this, indexCount, instanceCount, firstIndex,
        baseVertex, firstInstance);
}

void RenderEncoderBase::APIDrawIndirect(BufferBase* indirectBuffer, uint64_t indirectOffset) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(indirectBuffer));
                DAWN_TRY(ValidateCanUseAs(indirectBuffer, wgpu::BufferUsage::Indirect));
                DAWN_TRY(mCommandBufferState.ValidateCanDraw());
                if (!GetDevice()->HasFlexibleTextureViews()) {
                    DAWN_TRY(mCommandBufferState.ValidateNoDifferentTextureViewsOnSameTexture());
                }

                DAWN_INVALID_IF(indirectOffset % 4 != 0,
                                "Indirect offset (%u) is not a multiple of 4.", indirectOffset);

                DAWN_INVALID_IF(
                    indirectOffset >= indirectBuffer->GetSize() ||
                        kDrawIndirectSize > indirectBuffer->GetSize() - indirectOffset,
                    "Indirect offset (%u) is out of bounds of indirect buffer %s size (%u).",
                    indirectOffset, indirectBuffer, indirectBuffer->GetSize());
            }

            DrawIndirectCmd* cmd = allocator->Allocate<DrawIndirectCmd>(Command::DrawIndirect);

            bool duplicateBaseVertexInstance =
                GetDevice()->ShouldDuplicateParametersForDrawIndirect(
                    mCommandBufferState.GetRenderPipeline());
            if (IsValidationEnabled() || duplicateBaseVertexInstance) {
                // Later, EncodeIndirectDrawValidationCommands will allocate a scratch storage
                // buffer which will store the validated or duplicated indirect data. The buffer
                // and offset will be updated to point to it.
                // |EncodeIndirectDrawValidationCommands| is called at the end of encoding the
                // render pass, while the |cmd| pointer is still valid.
                cmd->indirectBuffer = nullptr;

                mIndirectDrawMetadata.AddIndirectDraw(indirectBuffer, indirectOffset,
                                                      duplicateBaseVertexInstance, cmd);

                // We only set usage as `kIndirectBufferForFrontendValidation` so that the indirect
                // usage can be ignored in the backends because `indirectBuffer` is actually not
                // used as an indirect buffer.
                mUsageTracker.BufferUsedAs(indirectBuffer, kIndirectBufferForFrontendValidation);
            } else {
                cmd->indirectBuffer = indirectBuffer;
                cmd->indirectOffset = indirectOffset;

                mUsageTracker.BufferUsedAs(indirectBuffer,
                                           kIndirectBufferForFrontendValidation |
                                               kIndirectBufferForBackendResourceTracking);
            }

            mDrawCount++;

            return {};
        },
        "encoding %s.DrawIndirect(%s, %u).", this, indirectBuffer, indirectOffset);
}

void RenderEncoderBase::APIDrawIndexedIndirect(BufferBase* indirectBuffer,
                                               uint64_t indirectOffset) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(indirectBuffer));
                DAWN_TRY(ValidateCanUseAs(indirectBuffer, wgpu::BufferUsage::Indirect));
                DAWN_TRY(mCommandBufferState.ValidateCanDrawIndexed());
                if (!GetDevice()->HasFlexibleTextureViews()) {
                    DAWN_TRY(mCommandBufferState.ValidateNoDifferentTextureViewsOnSameTexture());
                }

                DAWN_INVALID_IF(indirectOffset % 4 != 0,
                                "Indirect offset (%u) is not a multiple of 4.", indirectOffset);

                DAWN_INVALID_IF(
                    (indirectOffset >= indirectBuffer->GetSize() ||
                     kDrawIndexedIndirectSize > indirectBuffer->GetSize() - indirectOffset),
                    "Indirect offset (%u) is out of bounds of indirect buffer %s size (%u).",
                    indirectOffset, indirectBuffer, indirectBuffer->GetSize());
            }

            DrawIndexedIndirectCmd* cmd =
                allocator->Allocate<DrawIndexedIndirectCmd>(Command::DrawIndexedIndirect);

            bool duplicateBaseVertexInstance =
                GetDevice()->ShouldDuplicateParametersForDrawIndirect(
                    mCommandBufferState.GetRenderPipeline());
            bool applyIndexBufferOffsetToFirstIndex =
                GetDevice()->ShouldApplyIndexBufferOffsetToFirstIndex();
            if (IsValidationEnabled() || duplicateBaseVertexInstance ||
                applyIndexBufferOffsetToFirstIndex) {
                // Later, EncodeIndirectDrawValidationCommands will allocate a scratch storage
                // buffer which will store the validated or duplicated indirect data. The buffer
                // and offset will be updated to point to it.
                // |EncodeIndirectDrawValidationCommands| is called at the end of encoding the
                // render pass, while the |cmd| pointer is still valid.
                cmd->indirectBuffer = nullptr;

                mIndirectDrawMetadata.AddIndexedIndirectDraw(
                    mCommandBufferState.GetIndexFormat(), mCommandBufferState.GetIndexBufferSize(),
                    mCommandBufferState.GetIndexBufferOffset(), indirectBuffer, indirectOffset,
                    duplicateBaseVertexInstance, cmd);

                // We only set usage as `kIndirectBufferForFrontendValidation` so that the indirect
                // usage can be ignored in the backends because `indirectBuffer` is actually not
                // used as an indirect buffer.
                mUsageTracker.BufferUsedAs(indirectBuffer, kIndirectBufferForFrontendValidation);
            } else {
                cmd->indirectBuffer = indirectBuffer;
                cmd->indirectOffset = indirectOffset;

                mUsageTracker.BufferUsedAs(indirectBuffer,
                                           kIndirectBufferForFrontendValidation |
                                               kIndirectBufferForBackendResourceTracking);
            }

            mDrawCount++;

            return {};
        },
        "encoding %s.DrawIndexedIndirect(%s, %u).", this, indirectBuffer, indirectOffset);
}

void RenderEncoderBase::APIMultiDrawIndirect(BufferBase* indirectBuffer,
                                             uint64_t indirectOffset,
                                             uint32_t maxDrawCount,
                                             BufferBase* drawCountBuffer,
                                             uint64_t drawCountBufferOffset) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::MultiDrawIndirect),
                                "%s is not enabled.", wgpu::FeatureName::MultiDrawIndirect);

                DAWN_TRY(GetDevice()->ValidateObject(indirectBuffer));
                DAWN_TRY(ValidateCanUseAs(indirectBuffer, wgpu::BufferUsage::Indirect));

                DAWN_INVALID_IF(indirectOffset % 4 != 0,
                                "Indirect offset (%u) is not a multiple of 4.", indirectOffset);

                DAWN_INVALID_IF(indirectOffset >= indirectBuffer->GetSize(),
                                "Indirect offset (%u) is larger than or equal to the size (%u) of "
                                "the indirect buffer.",
                                indirectOffset, indirectBuffer->GetSize());

                // maxDrawCount * kDrawIndirectSize can't overflow because
                // kDrawIndirectSize is a uint64_t which results in a uint64_t
                DAWN_INVALID_IF(
                    indirectBuffer->GetSize() - indirectOffset < maxDrawCount * kDrawIndirectSize,
                    "Indirect Buffer size (%u) and offset (%u) can not hold maxDrawCount "
                    "(%u) draw commands.",
                    indirectBuffer->GetSize(), indirectOffset, maxDrawCount);

                // draw count buffer is optional
                if (drawCountBuffer != nullptr) {
                    DAWN_TRY(GetDevice()->ValidateObject(drawCountBuffer));
                    DAWN_TRY(ValidateCanUseAs(drawCountBuffer, wgpu::BufferUsage::Indirect));

                    DAWN_INVALID_IF(drawCountBufferOffset % 4 != 0,
                                    "Draw count buffer offset (%u) is not a multiple of 4.",
                                    drawCountBufferOffset);

                    DAWN_INVALID_IF(
                        drawCountBufferOffset >= drawCountBuffer->GetSize(),
                        "Draw count buffer offset (%u) is larger than or equal to the size "
                        "(%u) of the draw count buffer.",
                        drawCountBufferOffset, drawCountBuffer->GetSize());

                    // Can't underflow because the offset is checked to be smaller than the buffer.
                    DAWN_INVALID_IF(drawCountBuffer->GetSize() - drawCountBufferOffset < 4,
                                    "Draw count buffer offset (%u) is out of bounds of the draw "
                                    "count buffer %s size (%u).",
                                    drawCountBufferOffset, drawCountBuffer,
                                    drawCountBuffer->GetSize());
                }

                DAWN_TRY(mCommandBufferState.ValidateCanDraw());

                if (!GetDevice()->HasFlexibleTextureViews()) {
                    DAWN_TRY(mCommandBufferState.ValidateNoDifferentTextureViewsOnSameTexture());
                }
            }

            MultiDrawIndirectCmd* cmd =
                allocator->Allocate<MultiDrawIndirectCmd>(Command::MultiDrawIndirect);

            cmd->indirectBuffer = indirectBuffer;
            cmd->indirectOffset = indirectOffset;
            cmd->maxDrawCount = maxDrawCount;
            cmd->drawCountBuffer = drawCountBuffer;
            cmd->drawCountOffset = drawCountBufferOffset;

            bool duplicateBaseVertexInstance =
                GetDevice()->ShouldDuplicateParametersForDrawIndirect(
                    mCommandBufferState.GetRenderPipeline());

            mIndirectDrawMetadata.AddMultiDrawIndirect(
                mCommandBufferState.GetRenderPipeline()->GetPrimitiveTopology(),
                duplicateBaseVertexInstance, cmd);

            if (GetDevice()->IsValidationEnabled() ||
                GetDevice()->MayRequireDuplicationOfIndirectParameters()) {
                // We only set usage as `kIndirectBufferForFrontendValidation` because
                // `indirectBuffer` may not be used as an indirect buffer. The usage of
                // `indirectBuffer` may be updated in `EncodeIndirectDrawValidationCommands()` in
                // `EncodingContext::ExitRenderPass()`, which is only called when above conditions
                // are both met.
                mUsageTracker.BufferUsedAs(indirectBuffer, kIndirectBufferForFrontendValidation);
            } else {
                mUsageTracker.BufferUsedAs(indirectBuffer,
                                           kIndirectBufferForFrontendValidation |
                                               kIndirectBufferForBackendResourceTracking);
            }
            if (drawCountBuffer != nullptr) {
                mUsageTracker.BufferUsedAs(drawCountBuffer,
                                           kIndirectBufferForFrontendValidation |
                                               kIndirectBufferForBackendResourceTracking);
            }

            mDrawCount += maxDrawCount;

            return {};
        },
        "encoding %s.MultiDrawIndirect(%s, %u, %u, %s, %u).", this, indirectBuffer, indirectOffset,
        maxDrawCount, drawCountBuffer, drawCountBufferOffset);
}

void RenderEncoderBase::APIMultiDrawIndexedIndirect(BufferBase* indirectBuffer,
                                                    uint64_t indirectOffset,
                                                    uint32_t maxDrawCount,
                                                    BufferBase* drawCountBuffer,
                                                    uint64_t drawCountBufferOffset) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::MultiDrawIndirect),
                                "%s is not enabled.", wgpu::FeatureName::MultiDrawIndirect);

                DAWN_TRY(GetDevice()->ValidateObject(indirectBuffer));
                DAWN_TRY(ValidateCanUseAs(indirectBuffer, wgpu::BufferUsage::Indirect));

                DAWN_INVALID_IF(indirectOffset % 4 != 0,
                                "Indirect offset (%u) is not a multiple of 4.", indirectOffset);

                DAWN_INVALID_IF(indirectOffset >= indirectBuffer->GetSize(),
                                "Indirect offset (%u) is larger than or equal to the size (%u) of "
                                "the indirect buffer.",
                                indirectOffset, indirectBuffer->GetSize());

                // maxDrawCount * kDrawIndexedIndirectSize can't overflow because
                // kDrawIndexedIndirectSize is a uint64_t which results in a uint64_t
                DAWN_INVALID_IF(
                    indirectBuffer->GetSize() - indirectOffset <
                        maxDrawCount * kDrawIndexedIndirectSize,
                    "Indirect Buffer size (%u) and offset (%u) can not hold maxDrawCount "
                    "(%u) draw commands.",
                    indirectBuffer->GetSize(), indirectOffset, maxDrawCount);

                // draw count buffer is optional
                if (drawCountBuffer != nullptr) {
                    DAWN_TRY(GetDevice()->ValidateObject(drawCountBuffer));
                    DAWN_TRY(ValidateCanUseAs(drawCountBuffer, wgpu::BufferUsage::Indirect));

                    DAWN_INVALID_IF(drawCountBufferOffset % 4 != 0,
                                    "Draw count buffer offset (%u) is not a multiple of 4.",
                                    drawCountBufferOffset);

                    DAWN_INVALID_IF(
                        drawCountBufferOffset >= drawCountBuffer->GetSize(),
                        "Draw count buffer offset (%u) is larger than or equal to the size "
                        "(%u) of the draw count buffer.",
                        drawCountBufferOffset, drawCountBuffer->GetSize());

                    // Can't underflow because the offset is checked to be smaller than the buffer.
                    DAWN_INVALID_IF(drawCountBuffer->GetSize() - drawCountBufferOffset < 4,
                                    "Draw count buffer offset (%u) is out of bounds of the draw "
                                    "count buffer %s size (%u).",
                                    drawCountBufferOffset, drawCountBuffer,
                                    drawCountBuffer->GetSize());
                }

                DAWN_TRY(mCommandBufferState.ValidateCanDrawIndexed());

                if (!GetDevice()->HasFlexibleTextureViews()) {
                    DAWN_TRY(mCommandBufferState.ValidateNoDifferentTextureViewsOnSameTexture());
                }
            }

            MultiDrawIndexedIndirectCmd* cmd =
                allocator->Allocate<MultiDrawIndexedIndirectCmd>(Command::MultiDrawIndexedIndirect);

            cmd->indirectBuffer = indirectBuffer;
            cmd->indirectOffset = indirectOffset;
            cmd->maxDrawCount = maxDrawCount;
            cmd->drawCountBuffer = drawCountBuffer;
            cmd->drawCountOffset = drawCountBufferOffset;

            bool duplicateBaseVertexInstance =
                GetDevice()->ShouldDuplicateParametersForDrawIndirect(
                    mCommandBufferState.GetRenderPipeline());

            mIndirectDrawMetadata.AddMultiDrawIndexedIndirect(
                mCommandBufferState.GetIndexBuffer(), mCommandBufferState.GetIndexFormat(),
                mCommandBufferState.GetIndexBufferSize(),
                mCommandBufferState.GetIndexBufferOffset(),
                mCommandBufferState.GetRenderPipeline()->GetPrimitiveTopology(),
                duplicateBaseVertexInstance, cmd);

            if (GetDevice()->IsValidationEnabled() ||
                GetDevice()->MayRequireDuplicationOfIndirectParameters()) {
                // We only set usage as `kIndirectBufferForFrontendValidation` because
                // `indirectBuffer` may not be used as an indirect buffer. The usage of
                // `indirectBuffer` may be updated in `EncodeIndirectDrawValidationCommands()` in
                // `EncodingContext::ExitRenderPass()`, which is only called when above conditions
                // are both met.
                mUsageTracker.BufferUsedAs(indirectBuffer, kIndirectBufferForFrontendValidation);
            } else {
                mUsageTracker.BufferUsedAs(indirectBuffer,
                                           kIndirectBufferForFrontendValidation |
                                               kIndirectBufferForBackendResourceTracking);
            }
            if (drawCountBuffer != nullptr) {
                mUsageTracker.BufferUsedAs(drawCountBuffer,
                                           kIndirectBufferForFrontendValidation |
                                               kIndirectBufferForBackendResourceTracking);
            }

            mDrawCount += maxDrawCount;

            return {};
        },
        "encoding %s.MultiDrawIndexedIndirect(%s, %u, %u, %s, %u).", this, indirectBuffer,
        indirectOffset, maxDrawCount, drawCountBuffer, drawCountBufferOffset);
}

void RenderEncoderBase::APISetPipeline(RenderPipelineBase* pipeline) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(pipeline));

                DAWN_INVALID_IF(pipeline->GetAttachmentState() != mAttachmentState.Get(),
                                "Attachment state of %s is not compatible with %s.\n"
                                "%s expects an attachment state of %s.\n"
                                "%s has an attachment state of %s.",
                                pipeline, this, this, mAttachmentState.Get(), pipeline,
                                pipeline->GetAttachmentState());

                DAWN_INVALID_IF(pipeline->WritesDepth() && mDepthReadOnly,
                                "%s writes depth while %s's depthReadOnly is true", pipeline, this);

                DAWN_INVALID_IF(pipeline->WritesStencil() && mStencilReadOnly,
                                "%s writes stencil while %s's stencilReadOnly is true", pipeline,
                                this);
            }

            mCommandBufferState.SetRenderPipeline(pipeline);

            SetRenderPipelineCmd* cmd =
                allocator->Allocate<SetRenderPipelineCmd>(Command::SetRenderPipeline);
            cmd->pipeline = pipeline;

            return {};
        },
        "encoding %s.SetPipeline(%s).", this, pipeline);
}

void RenderEncoderBase::APISetIndexBuffer(BufferBase* buffer,
                                          wgpu::IndexFormat format,
                                          uint64_t offset,
                                          uint64_t size) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(buffer));
                DAWN_TRY(ValidateCanUseAs(buffer, wgpu::BufferUsage::Index));

                DAWN_TRY(ValidateIndexFormat(format));

                DAWN_INVALID_IF(format == wgpu::IndexFormat::Undefined,
                                "Index format must be specified");

                DAWN_INVALID_IF(offset % uint64_t(IndexFormatSize(format)) != 0,
                                "Index buffer offset (%u) is not a multiple of the size (%u) "
                                "of %s.",
                                offset, IndexFormatSize(format), format);

                uint64_t bufferSize = buffer->GetSize();
                DAWN_INVALID_IF(offset > bufferSize,
                                "Index buffer offset (%u) is larger than the size (%u) of %s.",
                                offset, bufferSize, buffer);

                uint64_t remainingSize = bufferSize - offset;

                if (size == wgpu::kWholeSize) {
                    size = remainingSize;
                } else {
                    DAWN_INVALID_IF(size > remainingSize,
                                    "Index buffer range (offset: %u, size: %u) doesn't fit in "
                                    "the size (%u) of "
                                    "%s.",
                                    offset, size, bufferSize, buffer);
                }
            } else {
                if (size == wgpu::kWholeSize) {
                    DAWN_ASSERT(buffer->GetSize() >= offset);
                    size = buffer->GetSize() - offset;
                }
            }

            mCommandBufferState.SetIndexBuffer(buffer, format, offset, size);

            SetIndexBufferCmd* cmd =
                allocator->Allocate<SetIndexBufferCmd>(Command::SetIndexBuffer);
            cmd->buffer = buffer;
            cmd->format = format;
            cmd->offset = offset;
            cmd->size = size;

            mUsageTracker.BufferUsedAs(buffer, wgpu::BufferUsage::Index);

            return {};
        },
        "encoding %s.SetIndexBuffer(%s, %s, %u, %u).", this, buffer, format, offset, size);
}

void RenderEncoderBase::APISetVertexBuffer(uint32_t slot,
                                           BufferBase* buffer,
                                           uint64_t offset,
                                           uint64_t size) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(slot >= kMaxVertexBuffers,
                                "Vertex buffer slot (%u) is larger the maximum (%u)", slot,
                                kMaxVertexBuffers - 1);

                if (buffer == nullptr) {
                    DAWN_INVALID_IF(offset != 0, "Offset (%u) must be 0 if buffer is null", offset);
                    DAWN_INVALID_IF(
                        size != 0 && size != wgpu::kWholeSize,
                        "Size (%u) must be either 0 or wgpu::kWholeSize if buffer is null", size);
                } else {
                    DAWN_TRY(GetDevice()->ValidateObject(buffer));
                    DAWN_TRY(ValidateCanUseAs(buffer, wgpu::BufferUsage::Vertex));
                    DAWN_INVALID_IF(offset % 4 != 0,
                                    "Vertex buffer offset (%u) is not a multiple of 4", offset);

                    uint64_t bufferSize = buffer->GetSize();
                    DAWN_INVALID_IF(offset > bufferSize,
                                    "Vertex buffer offset (%u) is larger than the size (%u) of %s.",
                                    offset, bufferSize, buffer);

                    uint64_t remainingSize = bufferSize - offset;

                    if (size == wgpu::kWholeSize) {
                        size = remainingSize;
                    } else {
                        DAWN_INVALID_IF(size > remainingSize,
                                        "Vertex buffer range (offset: %u, size: %u) doesn't fit in "
                                        "the size (%u) "
                                        "of %s.",
                                        offset, size, bufferSize, buffer);
                    }
                }
            } else {
                if (size == wgpu::kWholeSize && buffer != nullptr) {
                    DAWN_ASSERT(buffer->GetSize() >= offset);
                    size = buffer->GetSize() - offset;
                }
            }

            VertexBufferSlot vbSlot = VertexBufferSlot(static_cast<uint8_t>(slot));
            if (buffer == nullptr) {
                mCommandBufferState.UnsetVertexBuffer(vbSlot);
            } else {
                mCommandBufferState.SetVertexBuffer(vbSlot, size);

                SetVertexBufferCmd* cmd =
                    allocator->Allocate<SetVertexBufferCmd>(Command::SetVertexBuffer);
                cmd->slot = vbSlot;
                cmd->buffer = buffer;
                cmd->offset = offset;
                cmd->size = size;

                mUsageTracker.BufferUsedAs(buffer, wgpu::BufferUsage::Vertex);
            }
            return {};
        },
        "encoding %s.SetVertexBuffer(%u, %s, %u, %u).", this, slot, buffer, offset, size);
}

void RenderEncoderBase::APISetBindGroup(uint32_t groupIndexIn,
                                        BindGroupBase* group,
                                        uint32_t dynamicOffsetCount,
                                        const uint32_t* dynamicOffsets) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            BindGroupIndex groupIndex(groupIndexIn);

            if (IsValidationEnabled()) {
                DAWN_TRY(
                    ValidateSetBindGroup(groupIndex, group, dynamicOffsetCount, dynamicOffsets));
            }

            if (group == nullptr) {
                mCommandBufferState.UnsetBindGroup(groupIndex);
            } else {
                RecordSetBindGroup(allocator, groupIndex, group, dynamicOffsetCount,
                                   dynamicOffsets);
                mCommandBufferState.SetBindGroup(groupIndex, group, dynamicOffsetCount,
                                                 dynamicOffsets);
                mUsageTracker.AddBindGroup(group);
            }

            return {};
        },
        "encoding %s.SetBindGroup(%u, %s, %u, ...).", this, groupIndexIn, group,
        dynamicOffsetCount);
}

}  // namespace dawn::native
