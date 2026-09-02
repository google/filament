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

#include "src/dawn/native/Queue.h"

#include <webgpu/webgpu.h>

#include <algorithm>
#include <cstring>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/cleanup/cleanup.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/FutureUtils.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/native/BindGroup.h"
#include "src/dawn/native/BlitBufferToDepthStencil.h"
#include "src/dawn/native/Buffer.h"
#include "src/dawn/native/CommandBuffer.h"
#include "src/dawn/native/CommandEncoder.h"
#include "src/dawn/native/CommandValidation.h"
#include "src/dawn/native/Commands.h"
#include "src/dawn/native/CopyTextureForBrowserHelper.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/DynamicUploader.h"
#include "src/dawn/native/EventManager.h"
#include "src/dawn/native/ExternalTexture.h"
#include "src/dawn/native/Instance.h"
#include "src/dawn/native/QuerySet.h"
#include "src/dawn/native/RenderPassEncoder.h"
#include "src/dawn/native/RenderPipeline.h"
#include "src/dawn/native/ResourceTable.h"
#include "src/dawn/native/Texture.h"
#include "src/dawn/platform/tracing/TraceEvent.h"
#include "src/utils/compiler.h"
#include "src/utils/numeric.h"
#include "src/utils/span.h"

namespace dawn::native {

namespace {

void CopyTextureData(Span<std::byte> dst,
                     Span<const std::byte> src,
                     uint32_t depth,
                     uint32_t dstRowsPerImage,
                     uint64_t srcRowsPerImage,
                     uint32_t actualBytesPerRow,
                     uint32_t dstBytesPerRow,
                     uint32_t srcBytesPerRow) {
    uint64_t imageAdditionalStride = srcBytesPerRow * (srcRowsPerImage - dstRowsPerImage);
    bool copyWholeLayer = actualBytesPerRow == dstBytesPerRow && dstBytesPerRow == srcBytesPerRow;
    bool copyWholeData = copyWholeLayer && imageAdditionalStride == 0;

    size_t dstOffset = 0;
    size_t srcOffset = 0;
    if (!copyWholeLayer) {  // copy row by row
        for (uint32_t d = 0; d < depth; ++d) {
            for (uint32_t h = 0; h < dstRowsPerImage; ++h) {
                dst.subspan(dstOffset).CopyPrefixFrom(src.subspan(srcOffset, actualBytesPerRow));
                dstOffset += dstBytesPerRow;
                srcOffset += srcBytesPerRow;
            }
            srcOffset += imageAdditionalStride;
        }
    } else {
        size_t layerSize =
            checked_cast<size_t>(dstRowsPerImage) * checked_cast<size_t>(actualBytesPerRow);
        if (!copyWholeData) {  // copy layer by layer
            for (uint32_t d = 0; d < depth; ++d) {
                dst.subspan(dstOffset).CopyPrefixFrom(src.subspan(srcOffset, layerSize));
                dstOffset += layerSize;
                srcOffset += layerSize + imageAdditionalStride;
            }
        } else {  // do a single copy
            dst.CopyPrefixFrom(src.first(layerSize * depth));
        }
    }
}

class ErrorQueue : public QueueBase {
  public:
    explicit ErrorQueue(DeviceBase* device, StringView label)
        : QueueBase(device, ObjectBase::kError, label) {}

  private:
    MaybeError SubmitImpl(Span<CommandBufferBase* const> commands) override { DAWN_UNREACHABLE(); }
    bool HasPendingCommands() const override { DAWN_UNREACHABLE(); }
    MaybeError SubmitPendingCommandsImpl() override { DAWN_UNREACHABLE(); }
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override { DAWN_UNREACHABLE(); }
    void ForceEventualFlushOfCommands() override { DAWN_UNREACHABLE(); }
    ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                          Nanoseconds timeout) override {
        DAWN_UNREACHABLE();
    }
    MaybeError WaitForIdleForDestructionImpl() override { DAWN_UNREACHABLE(); }
};

}  // namespace

// TrackTaskCallback

void TrackTaskCallback::SetFinishedSerial(ExecutionSerial serial) {
    mSerial = serial;
}

// QueueBase

QueueBase::QueueBase(DeviceBase* device, const QueueDescriptor* descriptor)
    : ExecutionQueueBase(device, descriptor->label) {
    GetObjectTrackingList()->Track(this);
}

QueueBase::QueueBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ExecutionQueueBase(device, tag, label) {}

QueueBase::~QueueBase() {
    DAWN_CHECK(mTasksInFlight->Empty());
}

void QueueBase::DestroyImpl(DestroyReason reason) {}

// static
Ref<QueueBase> QueueBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new ErrorQueue(device, label));
}

ObjectType QueueBase::GetType() const {
    return ObjectType::Queue;
}

// It doesn't make much sense right now to mark the default queue as "unlabeled", so this override
// prevents that. Consider removing when multiqueue is implemented.
void QueueBase::FormatLabel(absl::FormatSink* s) const {
    s->Append(ObjectTypeAsString(GetType()));
    const std::string& label = GetLabel();
    if (!label.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", label));
    }
}

void QueueBase::APISubmit(Span<CommandBufferBase* const> commands) {
    MaybeError result = SubmitInternal(commands);

    // Destroy the command buffers even if SubmitInternal failed. (crbug.com/dawn/1863)
    for (CommandBufferBase* commandBuffer : commands) {
        commandBuffer->Destroy();
    }

    [[maybe_unused]] bool hadError =
        GetDevice()->ConsumedError(std::move(result), "calling %s.Submit(%s)", this, commands);
}

Future QueueBase::APIOnSubmittedWorkDone(const WGPUQueueWorkDoneCallbackInfo& callbackInfo) {
    struct WorkDoneEvent final : public EventManager::TrackedEvent {
        std::optional<WGPUQueueWorkDoneStatus> mEarlyStatus;
        WGPUQueueWorkDoneCallback mCallback = nullptr;
        std::string mMessage = "";
        raw_ptr<void> mUserdata1 = nullptr;
        raw_ptr<void> mUserdata2 = nullptr;

        // Create an event backed by the given queue execution serial.
        WorkDoneEvent(const WGPUQueueWorkDoneCallbackInfo& callbackInfo,
                      QueueBase* queue,
                      ExecutionSerial serial)
            : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode), queue, serial),
              mCallback(callbackInfo.callback),
              mUserdata1(callbackInfo.userdata1),
              mUserdata2(callbackInfo.userdata2) {}

        // Create an event that's ready at creation (for errors, etc.)
        WorkDoneEvent(const WGPUQueueWorkDoneCallbackInfo& callbackInfo,
                      QueueBase* queue,
                      wgpu::QueueWorkDoneStatus earlyStatus)
            : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                           queue,
                           queue->GetCompletedCommandSerial()),
              mEarlyStatus(ToAPI(earlyStatus)),
              mCallback(callbackInfo.callback),
              mUserdata1(callbackInfo.userdata1),
              mUserdata2(callbackInfo.userdata2) {}

        ~WorkDoneEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

        void Complete(EventCompletionType completionType) override {
            // WorkDoneEvent has no error cases other than the mEarlyStatus ones.
            WGPUQueueWorkDoneStatus status = WGPUQueueWorkDoneStatus_Success;
            if (completionType == EventCompletionType::Shutdown) {
                status = WGPUQueueWorkDoneStatus_CallbackCancelled;
            } else if (mEarlyStatus) {
                status = mEarlyStatus.value();
            }

            mCallback(status, ToOutputStringView(mMessage), mUserdata1.ExtractAsDangling(),
                      mUserdata2.ExtractAsDangling());
        }
    };

    // TODO(crbug.com/dawn/2052): Once we always return a future, change this to log to the instance
    // (note, not raise a validation error to the device) and return the null future.
    DAWN_CHECK(callbackInfo.nextInChain == nullptr);

    Ref<EventManager::TrackedEvent> event;
    {
        // TODO(crbug.com/dawn/831) Manually acquire device lock instead of relying on code-gen for
        // re-entrancy.
        auto deviceGuard = GetDevice()->GetGuard();
        if (GetDevice()->ConsumedError(GetDevice()->ValidateIsAlive())) {
            event = AcquireRef(
                new WorkDoneEvent(callbackInfo, this, wgpu::QueueWorkDoneStatus::Success));
        } else if (GetDevice()->ConsumedError(ValidateOnSubmittedWorkDone())) {
            event =
                AcquireRef(new WorkDoneEvent(callbackInfo, this, wgpu::QueueWorkDoneStatus::Error));
        } else {
            event = AcquireRef(new WorkDoneEvent(callbackInfo, this, GetScheduledWorkDoneSerial()));
        }
    }

    // Note: if the callback is spontaneous, it may get called in here.
    FutureID futureID = GetInstance()->GetEventManager()->TrackEvent(std::move(event));

    return {futureID};
}

void QueueBase::TrackTask(std::unique_ptr<TrackTaskCallback> task, ExecutionSerial serial) {
    // If the task depends on a serial which is not submitted yet, force a flush.
    if (serial > GetLastSubmittedCommandSerial()) {
        ForceEventualFlushOfCommands();
    }

    DAWN_CHECK(serial <= GetScheduledWorkDoneSerial());

    // If the serial indicated command has been completed, the task will be moved to callback task
    // manager.
    if (serial <= GetCompletedCommandSerial()) {
        task->SetFinishedSerial(GetCompletedCommandSerial());
        GetDevice()->GetCallbackTaskManager()->AddCallbackTask(std::move(task));
    } else {
        mTasksInFlight->Enqueue(std::move(task), serial);
    }
}

void QueueBase::TrackTaskAfterEventualFlush(std::unique_ptr<TrackTaskCallback> task) {
    ForceEventualFlushOfCommands();
    TrackTask(std::move(task), GetScheduledWorkDoneSerial());
}

void QueueBase::TrackPendingTask(std::unique_ptr<TrackTaskCallback> task) {
    mTasksInFlight->Enqueue(std::move(task), GetPendingCommandSerial());
}

void QueueBase::Tick(ExecutionSerial finishedSerial) {
    // If a user calls Queue::Submit inside a task, for example in a Buffer::MapAsync callback,
    // then the device will be ticked, which in turns ticks the queue, causing reentrance here.
    // To prevent the reentrant call from invalidating mTasksInFlight while in use by the first
    // call, we remove the tasks to finish from the queue, update mTasksInFlight, then run the
    // callbacks.
    TRACE_EVENT(DAWN_TRACE_CATEGORY(), "Queue::Tick", "finishedSerial", uint64_t{finishedSerial});

    std::vector<std::unique_ptr<TrackTaskCallback>> tasks;
    mTasksInFlight.Use([&](auto tasksInFlight) {
        for (auto& task : tasksInFlight->IterateUpTo(finishedSerial)) {
            tasks.push_back(std::move(task));
        }
        tasksInFlight->ClearUpTo(finishedSerial);
    });
    // Tasks' serials have passed. Move them to the callback task manager. They
    // are ready to be called.
    for (auto& task : tasks) {
        task->SetFinishedSerial(finishedSerial);
        GetDevice()->GetCallbackTaskManager()->AddCallbackTask(std::move(task));
    }
}

void QueueBase::HandleDeviceLoss() {
    mTasksInFlight.Use([&](auto tasksInFlight) {
        for (auto& task : tasksInFlight->IterateAll()) {
            task->OnDeviceLoss();
            GetDevice()->GetCallbackTaskManager()->AddCallbackTask(std::move(task));
        }
        tasksInFlight->Clear();
    });
}

void QueueBase::APIWriteBuffer(BufferBase* buffer,
                               uint64_t bufferOffset,
                               Span<const std::byte> data) {
    auto writeBuffer = [&]() -> MaybeError {
        DAWN_TRY(WriteBuffer(buffer, bufferOffset, data));
        return GetDevice()->GetDynamicUploader()->MaybeSubmitPendingCommands();
    };
    [[maybe_unused]] bool hadError = GetDevice()->ConsumedError(
        writeBuffer(), "calling %s.WriteBuffer(%s, (%d bytes), data, (%d bytes))", this, buffer,
        bufferOffset, data.size());
}

MaybeError QueueBase::WriteBuffer(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  Span<const std::byte> data) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(ValidateWriteBuffer(GetDevice(), buffer, bufferOffset, data.size()));
    BufferBase::ScopedUseBuffer scopedUseBuffer;
    DAWN_TRY_ASSIGN(scopedUseBuffer, buffer->ValidateCanUseOnQueueNow());
    return WriteBufferImpl(buffer, bufferOffset, data);
}

MaybeError QueueBase::WriteBufferImpl(BufferBase* buffer,
                                      uint64_t bufferOffset,
                                      Span<const std::byte> data) {
    return buffer->UploadData(bufferOffset, data);
}

void QueueBase::APIWriteTexture(const TexelCopyTextureInfo* destination,
                                Span<const std::byte> data,
                                const TexelCopyBufferLayout* dataLayout,
                                const Extent3D* writeSize) {
    auto writeTexture = [&]() -> MaybeError {
        DAWN_TRY(WriteTextureInternal(destination, data, *dataLayout, writeSize));
        return GetDevice()->GetDynamicUploader()->MaybeSubmitPendingCommands();
    };
    [[maybe_unused]] bool hadError = GetDevice()->ConsumedError(
        writeTexture(), "calling %s.WriteTexture(%s, (%u bytes), %s, %s)", this, destination,
        data.size(), dataLayout, writeSize);
}

MaybeError QueueBase::WriteTextureInternal(const TexelCopyTextureInfo* destinationOrig,
                                           Span<const std::byte> data,
                                           const TexelCopyBufferLayout& dataLayout,
                                           const Extent3D* writeSize) {
    TexelCopyTextureInfo destination = WithTrivialFrontendDefaults(*destinationOrig);

    DAWN_TRY(ValidateWriteTexture(&destination, data.size(), dataLayout, writeSize));

    if (writeSize->width == 0 || writeSize->height == 0 || writeSize->depthOrArrayLayers == 0) {
        return {};
    }

    const TexelBlockInfo& blockInfo = GetBlockInfo(destination);
    TexelCopyBufferLayout layout = dataLayout;
    ApplyDefaultTexelCopyBufferLayoutOptions(&layout, blockInfo, *writeSize);
    return WriteTextureImpl(destination, data, layout, *writeSize);
}

MaybeError QueueBase::WriteTextureImpl(const TexelCopyTextureInfo& destination,
                                       Span<const std::byte> data,
                                       const TexelCopyBufferLayout& dataLayout,
                                       const Extent3D& writeSizePixel) {
    const TypedTexelBlockInfo& blockInfo = GetBlockInfo(destination);
    BlockExtent3D writeSize = blockInfo.ToBlock(writeSizePixel);

    // We are only copying the part of the data that will appear in the texture.
    // Note that validating texture copy range ensures that writeSizePixel->width and
    // writeSizePixel->height are multiples of blockWidth and blockHeight respectively.
    BlockCount rowsPerImage = writeSize.height;
    uint32_t bytesPerRow = uint32_t(blockInfo.ToBytes(writeSize.width));
    uint32_t alignedBytesPerRow = Align(bytesPerRow, GetDevice()->GetOptimalBytesPerRowAlignment());
    BlockCount alignedBlocksPerRow = blockInfo.BytesToBlocks(alignedBytesPerRow);

    uint64_t packedDataSize =
        ComputeRequiredBytesInCopy(blockInfo, writeSize, alignedBlocksPerRow, rowsPerImage);

    // We need the offset to be aligned to both the optimal offset for that device and
    // blockByteSize, since both of them are powers of two, we only need to align to the max value.
    DAWN_CHECK(IsPowerOfTwo(GetDevice()->GetOptimalBufferToTextureCopyOffsetAlignment()));
    DAWN_CHECK(IsPowerOfTwo(blockInfo.byteSize));
    uint64_t offsetAlignment = std::max(
        uint64_t{blockInfo.byteSize}, GetDevice()->GetOptimalBufferToTextureCopyOffsetAlignment());

    // Buffer offset alignments must follow additional restrictions for depth stencil formats.
    const Format& format = destination.texture->GetFormat();
    if (format.HasDepthOrStencil()) {
        offsetAlignment =
            std::max(offsetAlignment, GetDevice()->GetBufferCopyOffsetAlignmentForDepthStencil());
    }

    return GetDevice()->GetDynamicUploader()->WithUploadReservation(
        packedDataSize, offsetAlignment, [&](UploadReservation reservation) -> MaybeError {
            CopyTextureData(
                reservation.mappedData, data.subspan(checked_cast<size_t>(dataLayout.offset)),
                writeSizePixel.depthOrArrayLayers, dchecked_cast<uint32_t>(rowsPerImage),
                dataLayout.rowsPerImage, bytesPerRow, alignedBytesPerRow, dataLayout.bytesPerRow);

            TexelCopyBufferLayout passDataLayout = dataLayout;
            passDataLayout.offset = reservation.offsetInBuffer;
            passDataLayout.bytesPerRow = alignedBytesPerRow;
            passDataLayout.rowsPerImage = dchecked_cast<uint32_t>(rowsPerImage);

            TextureCopy textureCopy;
            textureCopy.texture = destination.texture;
            textureCopy.mipLevel = destination.mipLevel;
            textureCopy.origin = destination.origin;
            textureCopy.aspect = ConvertAspect(format, destination.aspect);

            return GetDevice()->CopyFromStagingToTexture(reservation.buffer.Get(), passDataLayout,
                                                         textureCopy, writeSizePixel);
        });
}

void QueueBase::APICopyTextureForBrowser(const TexelCopyTextureInfo* source,
                                         const TexelCopyTextureInfo* destination,
                                         const Extent3D* copySize,
                                         const CopyTextureForBrowserOptions* options) {
    [[maybe_unused]] bool hadError = GetDevice()->ConsumedError(
        CopyTextureForBrowserInternal(source, destination, copySize, options));
}

void QueueBase::APICopyExternalTextureForBrowser(const ImageCopyExternalTexture* source,
                                                 const TexelCopyTextureInfo* destination,
                                                 const Extent3D* copySize,
                                                 const CopyTextureForBrowserOptions* options) {
    [[maybe_unused]] bool hadError = GetDevice()->ConsumedError(
        CopyExternalTextureForBrowserInternal(source, destination, copySize, options));
}

MaybeError QueueBase::CopyTextureForBrowserInternal(const TexelCopyTextureInfo* sourceOrig,
                                                    const TexelCopyTextureInfo* destinationOrig,
                                                    const Extent3D* copySize,
                                                    const CopyTextureForBrowserOptions* options) {
    TexelCopyTextureInfo source = WithTrivialFrontendDefaults(*sourceOrig);
    TexelCopyTextureInfo destination = WithTrivialFrontendDefaults(*destinationOrig);

    if (GetDevice()->IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(
            ValidateCopyTextureForBrowser(GetDevice(), &source, &destination, copySize, options),
            "validating CopyTextureForBrowser from %s to %s", source.texture, destination.texture);
    }

    return DoCopyTextureForBrowser(GetDevice(), &source, &destination, copySize, options);
}

MaybeError QueueBase::CopyExternalTextureForBrowserInternal(
    const ImageCopyExternalTexture* source,
    const TexelCopyTextureInfo* destinationOrig,
    const Extent3D* copySize,
    const CopyTextureForBrowserOptions* options) {
    TexelCopyTextureInfo destination = WithTrivialFrontendDefaults(*destinationOrig);

    if (GetDevice()->IsValidationEnabled()) {
        DAWN_TRY_CONTEXT(ValidateCopyExternalTextureForBrowser(GetDevice(), source, &destination,
                                                               copySize, options),
                         "validating CopyExternalTextureForBrowser from %s to %s",
                         source->externalTexture, destination.texture);
    }

    return DoCopyExternalTextureForBrowser(GetDevice(), source, &destination, copySize, options);
}

MaybeError QueueBase::ValidateSubmit(Span<CommandBufferBase* const> commands,
                                     BufferSet& buffersFromCommands) const {
    TRACE_EVENT(DAWN_TRACE_CATEGORY("validation"), "Queue::ValidateSubmit");
    DAWN_TRY(GetDevice()->ValidateObject(this));

    std::set<CommandBufferBase*> uniqueCommandBuffers;

    for (CommandBufferBase* commandBuffer : commands) {
        DAWN_TRY(GetDevice()->ValidateObject(commandBuffer));
        DAWN_TRY(commandBuffer->ValidateCanUseInSubmitNow());

        auto insertResult = uniqueCommandBuffers.insert(commandBuffer);
        DAWN_INVALID_IF(!insertResult.second, "Submit contains duplicates of %s.", commandBuffer);

        const CommandBufferResourceUsage& usages = commandBuffer->GetResourceUsages();

        auto ValidateBuffer = [&buffersFromCommands](BufferBase* buffer) -> MaybeError {
            if (auto [iter, inserted] = buffersFromCommands.insert(buffer); inserted) {
                BufferBase::ScopedUseBuffer use;
                DAWN_TRY_ASSIGN_WITH_CLEANUP(use, buffer->ValidateCanUseOnQueueNow(),
                                             { buffersFromCommands.erase(iter); });
                // FinishUse() will be called on the buffers in `buffersFromCommands` explicitly.
                use.Release();
            }
            return {};
        };

        // Maybe track last usage for other resources, and use it to release resources earlier?
        for (const SyncScopeResourceUsage& scope : usages.renderPasses) {
            for (BufferBase* buffer : scope.buffers) {
                DAWN_TRY(ValidateBuffer(buffer));
            }
            for (const TextureBase* texture : scope.textures) {
                DAWN_TRY(texture->ValidateCanUseInSubmitNow());
            }
            for (const ExternalTextureBase* externalTexture : scope.externalTextures) {
                DAWN_TRY(externalTexture->ValidateCanUseInSubmitNow());
            }
            if (auto table = scope.resourceTable) {
                DAWN_TRY(table->ValidateCanUseInSubmitNow());
            }
        }

        for (const ComputePassResourceUsage& pass : usages.computePasses) {
            for (BufferBase* buffer : pass.referencedBuffers) {
                DAWN_TRY(ValidateBuffer(buffer));
            }
            for (const TextureBase* texture : pass.referencedTextures) {
                DAWN_TRY(texture->ValidateCanUseInSubmitNow());
            }
            for (const ExternalTextureBase* externalTexture : pass.referencedExternalTextures) {
                DAWN_TRY(externalTexture->ValidateCanUseInSubmitNow());
            }
            for (const ResourceTableBase* resourceTable : pass.referencedResourceTables) {
                DAWN_TRY(resourceTable->ValidateCanUseInSubmitNow());
            }
        }

        for (BufferBase* buffer : usages.topLevelBuffers) {
            DAWN_TRY(ValidateBuffer(buffer));
        }
        for (const TextureBase* texture : usages.topLevelTextures) {
            DAWN_TRY(texture->ValidateCanUseInSubmitNow());
        }
        for (const QuerySetBase* querySet : usages.usedQuerySets) {
            DAWN_TRY(querySet->ValidateCanUseInSubmitNow());
        }
    }

    return {};
}

MaybeError QueueBase::ValidateOnSubmittedWorkDone() const {
    DAWN_TRY(GetDevice()->ValidateObject(this));
    return {};
}

MaybeError QueueBase::ValidateWriteTexture(const TexelCopyTextureInfo* destination,
                                           size_t dataSize,
                                           const TexelCopyBufferLayout& dataLayout,
                                           const Extent3D* writeSize) const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(GetDevice()->ValidateObject(destination->texture));

    DAWN_TRY(ValidateTexelCopyTextureInfo(GetDevice(), *destination, *writeSize));

    DAWN_INVALID_IF(dataLayout.offset > dataSize,
                    "Data offset (%u) is greater than the data size (%u).", dataLayout.offset,
                    dataSize);

    DAWN_INVALID_IF(!(destination->texture->GetUsage() & wgpu::TextureUsage::CopyDst),
                    "Usage (%s) of %s does not include %s.", destination->texture->GetUsage(),
                    destination->texture, wgpu::TextureUsage::CopyDst);

    DAWN_INVALID_IF(destination->texture->GetSampleCount() > 1, "Sample count (%u) of %s is not 1",
                    destination->texture->GetSampleCount(), destination->texture);

    DAWN_TRY(ValidateLinearToDepthStencilCopyRestrictions(*destination));
    // We validate texture copy range before validating linear texture data,
    // because in the latter we divide copyExtent.width by blockWidth and
    // copyExtent.height by blockHeight while the divisibility conditions are
    // checked in validating texture copy range.
    DAWN_TRY(ValidateTextureCopyRange(GetDevice(), *destination, *writeSize));

    const TexelBlockInfo& blockInfo = GetBlockInfo(*destination);

    DAWN_TRY(ValidateLinearTextureData(dataLayout, dataSize, blockInfo, *writeSize));

    DAWN_TRY(destination->texture->ValidateCanUseInSubmitNow());

    return {};
}

MaybeError QueueBase::SubmitInternal(Span<CommandBufferBase* const> commands) {
    DeviceBase* device = GetDevice();

    // If device is lost, don't let any commands be submitted
    DAWN_TRY(device->ValidateIsAlive());

    TRACE_EVENT(DAWN_TRACE_CATEGORY(), "Queue::Submit");

    BufferSet buffersUsedInSubmit;
    absl::Cleanup finishUseBuffers = [&buffersUsedInSubmit]() {
        for (BufferBase* buffer : buffersUsedInSubmit) {
            buffer->FinishUse();
        }
    };
    if (device->IsValidationEnabled()) {
        // TODO(crbug.com/425472913): Keep a rolling average of set size so this can reserve a
        // sufficiently large set for max of last N submits.
        DAWN_TRY(ValidateSubmit(commands, buffersUsedInSubmit));
    }
    DAWN_CHECK(!IsError());

    DAWN_TRY(SubmitImpl(commands));

    // Switch the buffer state back to unmapped before Tick().
    std::move(finishUseBuffers).Invoke();

    // Call Tick() to flush pending work.
    DAWN_TRY(device->Tick());

    return {};
}

}  // namespace dawn::native
