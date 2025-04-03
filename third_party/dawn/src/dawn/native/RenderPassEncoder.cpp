// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/RenderPassEncoder.h"

#include <math.h>
#include <cstring>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils.h"

namespace dawn::native {
namespace {

// Check the query at queryIndex is unavailable, otherwise it cannot be written.
MaybeError ValidateQueryIndexOverwrite(QuerySetBase* querySet,
                                       uint32_t queryIndex,
                                       const QueryAvailabilityMap& queryAvailabilityMap) {
    auto it = queryAvailabilityMap.find(querySet);
    DAWN_INVALID_IF(it != queryAvailabilityMap.end() && it->second[queryIndex],
                    "Query index %u of %s is written to twice in a render pass.", queryIndex,
                    querySet);

    return {};
}

}  // namespace

// The usage tracker is passed in here, because it is prepopulated with usages from the
// BeginRenderPassCmd. If we had RenderPassEncoder responsible for recording the
// command, then this wouldn't be necessary.
RenderPassEncoder::RenderPassEncoder(DeviceBase* device,
                                     const UnpackedPtr<RenderPassDescriptor>& descriptor,
                                     CommandEncoder* commandEncoder,
                                     EncodingContext* encodingContext,
                                     RenderPassResourceUsageTracker usageTracker,
                                     Ref<AttachmentState> attachmentState,
                                     uint32_t renderTargetWidth,
                                     uint32_t renderTargetHeight,
                                     bool depthReadOnly,
                                     bool stencilReadOnly,
                                     EndCallback endCallback)
    : RenderEncoderBase(device,
                        descriptor->label,
                        encodingContext,
                        std::move(attachmentState),
                        depthReadOnly,
                        stencilReadOnly),
      mCommandEncoder(commandEncoder),
      mRenderTargetWidth(renderTargetWidth),
      mRenderTargetHeight(renderTargetHeight),
      mOcclusionQuerySet(descriptor->occlusionQuerySet),
      mEndCallback(std::move(endCallback)) {
    mUsageTracker = std::move(usageTracker);
    if (auto* maxDrawCountInfo = descriptor.Get<RenderPassMaxDrawCount>()) {
        mMaxDrawCount = maxDrawCountInfo->maxDrawCount;
    }
    GetObjectTrackingList()->Track(this);
}

// static
Ref<RenderPassEncoder> RenderPassEncoder::Create(
    DeviceBase* device,
    const UnpackedPtr<RenderPassDescriptor>& descriptor,
    CommandEncoder* commandEncoder,
    EncodingContext* encodingContext,
    RenderPassResourceUsageTracker usageTracker,
    Ref<AttachmentState> attachmentState,
    uint32_t renderTargetWidth,
    uint32_t renderTargetHeight,
    bool depthReadOnly,
    bool stencilReadOnly,
    EndCallback endCallback) {
    return AcquireRef(new RenderPassEncoder(device, descriptor, commandEncoder, encodingContext,
                                            std::move(usageTracker), std::move(attachmentState),
                                            renderTargetWidth, renderTargetHeight, depthReadOnly,
                                            stencilReadOnly, std::move(endCallback)));
}

RenderPassEncoder::RenderPassEncoder(DeviceBase* device,
                                     CommandEncoder* commandEncoder,
                                     EncodingContext* encodingContext,
                                     ErrorTag errorTag,
                                     StringView label)
    : RenderEncoderBase(device, encodingContext, errorTag, label),
      mCommandEncoder(commandEncoder) {}

// static
Ref<RenderPassEncoder> RenderPassEncoder::MakeError(DeviceBase* device,
                                                    CommandEncoder* commandEncoder,
                                                    EncodingContext* encodingContext,
                                                    StringView label) {
    return AcquireRef(
        new RenderPassEncoder(device, commandEncoder, encodingContext, ObjectBase::kError, label));
}

RenderPassEncoder::~RenderPassEncoder() {
    mEncodingContext = nullptr;
}

void RenderPassEncoder::DestroyImpl() {
    mIndirectDrawMetadata.ClearIndexedIndirectBufferValidationInfo();
    mCommandBufferState.End();

    RenderEncoderBase::DestroyImpl();
    // Ensure that the pass has exited. This is done for passes only since validation requires
    // they exit before destruction while bundles do not.
    mEncodingContext->EnsurePassExited(this);
}

ObjectType RenderPassEncoder::GetType() const {
    return ObjectType::RenderPassEncoder;
}

void RenderPassEncoder::TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex) {
    DAWN_ASSERT(querySet != nullptr);

    // Track the query availability with true on render pass for rewrite validation and query
    // reset on render pass on Vulkan
    mUsageTracker.TrackQueryAvailability(querySet, queryIndex);

    // Track it again on command encoder for zero-initializing when resolving unused queries.
    mCommandEncoder->TrackQueryAvailability(querySet, queryIndex);
}

void RenderPassEncoder::APIEnd() {
    // The encoding context might create additional resources, so we need to lock the device.
    auto deviceLock(GetDevice()->GetScopedLock());
    End();
}

void RenderPassEncoder::End() {
    DAWN_ASSERT(GetDevice()->IsLockedByCurrentThreadIfNeeded());

    if (mEnded && IsValidationEnabled()) {
        GetDevice()->HandleError(DAWN_VALIDATION_ERROR("%s was already ended.", this));
        return;
    }

    mEnded = true;
    mCommandBufferState.End();

    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(ValidateProgrammableEncoderEnd());

                DAWN_INVALID_IF(
                    mOcclusionQueryActive,
                    "Render pass %s ended with incomplete occlusion query index %u of %s.", this,
                    mCurrentOcclusionQueryIndex, mOcclusionQuerySet.Get());

                DAWN_INVALID_IF(mDrawCount > mMaxDrawCount,
                                "The drawCount (%u) of %s is greater than the maxDrawCount (%u).",
                                mDrawCount, this, mMaxDrawCount);
            }

            allocator->Allocate<EndRenderPassCmd>(Command::EndRenderPass);
            DAWN_TRY(mEncodingContext->ExitRenderPass(this, std::move(mUsageTracker),
                                                      mCommandEncoder.Get(),
                                                      std::move(mIndirectDrawMetadata)));
            if (mEndCallback) {
                mEncodingContext->ConsumedError(mEndCallback());
            }

            return {};
        },
        "encoding %s.End().", this);
}

void RenderPassEncoder::APISetStencilReference(uint32_t reference) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            SetStencilReferenceCmd* cmd =
                allocator->Allocate<SetStencilReferenceCmd>(Command::SetStencilReference);
            cmd->reference = reference & 255;

            return {};
        },
        "encoding %s.SetStencilReference(%u).", this, reference);
}

void RenderPassEncoder::APISetBlendConstant(const Color* color) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(ValidateColor("color", *color));
            }
            SetBlendConstantCmd* cmd =
                allocator->Allocate<SetBlendConstantCmd>(Command::SetBlendConstant);
            cmd->color = *color;

            return {};
        },
        "encoding %s.SetBlendConstant(%s).", this, color);
}

void RenderPassEncoder::APISetViewport(float x,
                                       float y,
                                       float width,
                                       float height,
                                       float minDepth,
                                       float maxDepth) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(ValidateFloat("x", x));
                DAWN_TRY(ValidateFloat("y", y));
                DAWN_TRY(ValidateFloat("width", width));
                DAWN_TRY(ValidateFloat("height", height));
                DAWN_TRY(ValidateFloat("minDepth", minDepth));
                DAWN_TRY(ValidateFloat("maxDepth", maxDepth));

                const CombinedLimits& limits = GetDevice()->GetLimits();
                uint32_t maxViewportSize = limits.v1.maxTextureDimension2D;
                float maxViewportBounds = maxViewportSize * 2.0;

                DAWN_INVALID_IF(
                    width < 0 || height < 0,
                    "Viewport bounds (width: %f, height: %f) contains a negative value.", width,
                    height);

                DAWN_INVALID_IF(
                    width > maxViewportSize, "Viewport width (%f) exceeds the maximum (%u).%s",
                    width, maxViewportSize,
                    DAWN_INCREASE_LIMIT_MESSAGE(GetDevice()->GetAdapter()->GetLimits().v1,
                                                maxTextureDimension2D, width));

                DAWN_INVALID_IF(
                    height > maxViewportSize,
                    "Viewport size height (%f) exceeds the maximum (%u).%s", height,
                    maxViewportSize,
                    DAWN_INCREASE_LIMIT_MESSAGE(GetDevice()->GetAdapter()->GetLimits().v1,
                                                maxTextureDimension2D, height));

                DAWN_INVALID_IF(x < -maxViewportBounds || y < -maxViewportBounds,
                                "Viewport offset (x: %f, y: %f) is less than the minimum "
                                "supported bounds (%f x %f).",
                                x, y, -maxViewportBounds, -maxViewportBounds);

                DAWN_INVALID_IF(
                    x + width > maxViewportBounds - 1 || y + height > maxViewportBounds - 1,
                    "Viewport bounds (x: %f, y: %f, width: %f, height: %f) exceed "
                    "the maximum supported bounds (%f x %f).",
                    x, y, width, height, maxViewportBounds - 1, maxViewportBounds - 1);

                // Check for depths being in [0, 1] and min <= max in 3 checks instead of 5.
                DAWN_INVALID_IF(minDepth < 0 || minDepth > maxDepth || maxDepth > 1,
                                "Viewport minDepth (%f) and maxDepth (%f) are not in [0, 1] or "
                                "minDepth was "
                                "greater than maxDepth.",
                                minDepth, maxDepth);
            }

            SetViewportCmd* cmd = allocator->Allocate<SetViewportCmd>(Command::SetViewport);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;
            cmd->minDepth = minDepth;
            cmd->maxDepth = maxDepth;

            return {};
        },
        "encoding %s.SetViewport(%f, %f, %f, %f, %f, %f).", this, x, y, width, height, minDepth,
        maxDepth);
}

void RenderPassEncoder::APISetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(
                    width > mRenderTargetWidth || height > mRenderTargetHeight ||
                        x > mRenderTargetWidth - width || y > mRenderTargetHeight - height,
                    "Scissor rect (x: %u, y: %u, width: %u, height: %u) is not contained in "
                    "the render target dimensions (%u x %u).",
                    x, y, width, height, mRenderTargetWidth, mRenderTargetHeight);
            }

            SetScissorRectCmd* cmd =
                allocator->Allocate<SetScissorRectCmd>(Command::SetScissorRect);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;

            return {};
        },
        "encoding %s.SetScissorRect(%u, %u, %u, %u).", this, x, y, width, height);
}

void RenderPassEncoder::APIExecuteBundles(uint32_t count, RenderBundleBase* const* renderBundles) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                const AttachmentState* attachmentState = GetAttachmentState();
                bool depthReadOnlyInPass = IsDepthReadOnly();
                bool stencilReadOnlyInPass = IsStencilReadOnly();
                for (uint32_t i = 0; i < count; ++i) {
                    DAWN_TRY(GetDevice()->ValidateObject(renderBundles[i]));

                    DAWN_INVALID_IF(attachmentState != renderBundles[i]->GetAttachmentState(),
                                    "Attachment state of renderBundles[%i] (%s) is not "
                                    "compatible with %s.\n"
                                    "%s expects an attachment state of %s.\n"
                                    "renderBundles[%i] (%s) has an attachment state of %s.",
                                    i, renderBundles[i], this, this, attachmentState, i,
                                    renderBundles[i], renderBundles[i]->GetAttachmentState());

                    bool depthReadOnlyInBundle = renderBundles[i]->IsDepthReadOnly();
                    DAWN_INVALID_IF(depthReadOnlyInPass && !depthReadOnlyInBundle,
                                    "DepthReadOnly (%u) of renderBundle[%i] (%s) is not compatible "
                                    "with DepthReadOnly (%u) of %s.",
                                    depthReadOnlyInBundle, i, renderBundles[i], depthReadOnlyInPass,
                                    this);

                    bool stencilReadOnlyInBundle = renderBundles[i]->IsStencilReadOnly();
                    DAWN_INVALID_IF(stencilReadOnlyInPass && !stencilReadOnlyInBundle,
                                    "StencilReadOnly (%u) of renderBundle[%i] (%s) is not "
                                    "compatible with StencilReadOnly (%u) of %s.",
                                    stencilReadOnlyInBundle, i, renderBundles[i],
                                    stencilReadOnlyInPass, this);
                }
            }

            mCommandBufferState = CommandBufferStateTracker{};

            ExecuteBundlesCmd* cmd =
                allocator->Allocate<ExecuteBundlesCmd>(Command::ExecuteBundles);
            cmd->count = count;

            Ref<RenderBundleBase>* bundles = allocator->AllocateData<Ref<RenderBundleBase>>(count);
            for (uint32_t i = 0; i < count; ++i) {
                bundles[i] = renderBundles[i];

                const RenderPassResourceUsage& usages = bundles[i]->GetResourceUsage();
                for (uint32_t j = 0; j < usages.buffers.size(); ++j) {
                    mUsageTracker.BufferUsedAs(usages.buffers[j], usages.bufferSyncInfos[j].usage,
                                               usages.bufferSyncInfos[j].shaderStages);
                }

                for (uint32_t j = 0; j < usages.textures.size(); ++j) {
                    mUsageTracker.AddRenderBundleTextureUsage(usages.textures[j],
                                                              usages.textureSyncInfos[j]);
                }

                if (IsValidationEnabled()) {
                    mIndirectDrawMetadata.AddBundle(renderBundles[i]);
                }

                mDrawCount += bundles[i]->GetDrawCount();
            }

            return {};
        },
        "encoding %s.ExecuteBundles(%u, ...).", this, count);
}

void RenderPassEncoder::APIBeginOcclusionQuery(uint32_t queryIndex) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(mOcclusionQuerySet.Get() == nullptr,
                                "The occlusionQuerySet in RenderPassDescriptor is not set.");

                // The type of querySet has been validated by ValidateRenderPassDescriptor

                DAWN_INVALID_IF(queryIndex >= mOcclusionQuerySet->GetQueryCount(),
                                "Query index (%u) exceeds the number of queries (%u) in %s.",
                                queryIndex, mOcclusionQuerySet->GetQueryCount(),
                                mOcclusionQuerySet.Get());

                DAWN_INVALID_IF(mOcclusionQueryActive,
                                "An occlusion query (%u) in %s is already active.",
                                mCurrentOcclusionQueryIndex, mOcclusionQuerySet.Get());

                DAWN_TRY_CONTEXT(
                    ValidateQueryIndexOverwrite(mOcclusionQuerySet.Get(), queryIndex,
                                                mUsageTracker.GetQueryAvailabilityMap()),
                    "validating the occlusion query index (%u) in %s", queryIndex,
                    mOcclusionQuerySet.Get());
            }

            // Record the current query index for endOcclusionQuery.
            mCurrentOcclusionQueryIndex = queryIndex;
            mOcclusionQueryActive = true;

            BeginOcclusionQueryCmd* cmd =
                allocator->Allocate<BeginOcclusionQueryCmd>(Command::BeginOcclusionQuery);
            cmd->querySet = mOcclusionQuerySet.Get();
            cmd->queryIndex = queryIndex;

            return {};
        },
        "encoding %s.BeginOcclusionQuery(%u).", this, queryIndex);
}

void RenderPassEncoder::APIEndOcclusionQuery() {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(!mOcclusionQueryActive, "No occlusion queries are active.");
            }

            TrackQueryAvailability(mOcclusionQuerySet.Get(), mCurrentOcclusionQueryIndex);

            mOcclusionQueryActive = false;

            EndOcclusionQueryCmd* cmd =
                allocator->Allocate<EndOcclusionQueryCmd>(Command::EndOcclusionQuery);
            cmd->querySet = mOcclusionQuerySet.Get();
            cmd->queryIndex = mCurrentOcclusionQueryIndex;

            return {};
        },
        "encoding %s.EndOcclusionQuery().", this);
}

void RenderPassEncoder::APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex) {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_TRY(ValidateTimestampQuery(
                    GetDevice(), querySet, queryIndex,
                    Feature::ChromiumExperimentalTimestampQueryInsidePasses));
                DAWN_TRY_CONTEXT(ValidateQueryIndexOverwrite(
                                     querySet, queryIndex, mUsageTracker.GetQueryAvailabilityMap()),
                                 "validating the timestamp query index (%u) of %s", queryIndex,
                                 querySet);
            }

            TrackQueryAvailability(querySet, queryIndex);

            WriteTimestampCmd* cmd =
                allocator->Allocate<WriteTimestampCmd>(Command::WriteTimestamp);
            cmd->querySet = querySet;
            cmd->queryIndex = queryIndex;

            return {};
        },
        "encoding %s.WriteTimestamp(%s, %u).", this, querySet, queryIndex);
}

void RenderPassEncoder::APIPixelLocalStorageBarrier() {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(!GetAttachmentState()->HasPixelLocalStorage(),
                                "%s does not define any pixel local storage.", this);
            }

            allocator->Allocate<PixelLocalStorageBarrierCmd>(Command::PixelLocalStorageBarrier);
            return {};
        },
        "encoding %s.PixelLocalStorageBarrier().", this);
}

}  // namespace dawn::native
