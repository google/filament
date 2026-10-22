// Copyright 2025 The Dawn & Tint Authors
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

#include "src/dawn/native/webgpu/QueueWGPU.h"

#include <limits>
#include <vector>

#include "src/dawn/common/Enumerator.h"
#include "src/dawn/native/EventManager.h"
#include "src/dawn/native/Instance.h"
#include "src/dawn/native/Queue.h"
#include "src/dawn/native/webgpu/BufferWGPU.h"
#include "src/dawn/native/webgpu/CaptureContext.h"
#include "src/dawn/native/webgpu/CommandBufferWGPU.h"
#include "src/dawn/native/webgpu/DeviceWGPU.h"
#include "src/dawn/native/webgpu/SharedFenceWGPU.h"
#include "src/dawn/native/webgpu/TextureWGPU.h"
#include "src/dawn/native/webgpu/ToWGPU.h"
#include "src/dawn/native/webgpu/WebGPUError.h"
#include "src/utils/compiler.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    return AcquireRef(new Queue(device, descriptor));
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor)
    : QueueBase(device, descriptor), ObjectWGPU(device->wgpu->queueRelease) {
    mInnerHandle = device->wgpu->deviceGetQueue(device->GetInnerHandle());
}

ResultOrError<Ref<SharedFence>> Queue::GetOrCreateSharedFence(WGPUSharedFence innerFence) {
    if (mSharedFence) {
        return mSharedFence;
    }

    mSharedFence = SharedFence::CreateFromHandle(ToBackend(GetDevice()),
                                                 "WebGPU SharedFence Wrapper", innerFence);
    return mSharedFence;
}

MaybeError Queue::SubmitImpl(Span<CommandBufferBase* const> commands) {
    if (commands.empty()) {
        return {};
    }

    if (IsCapturing()) {
        std::vector<schema::ObjectId> commandBufferIds;
        commandBufferIds.reserve(commands.size());

        for (CommandBufferBase* commandBuffer : commands) {
            schema::ObjectId id;
            DAWN_TRY_ASSIGN(id, mCaptureContext->AddResourceAndGetId(ToBackend(commandBuffer)));
            commandBufferIds.emplace_back(id);
        }

        schema::RootCommandQueueSubmitCmd cmd{{
            .data = {{
                .commandBuffers = commandBufferIds,
            }},
        }};
        Serialize(*mCaptureContext, cmd);
    }

    std::vector<WGPUCommandBuffer> innerCommandBuffers(commands.size());
    for (auto [i, commandBuffer] : Enumerate(commands)) {
        DAWN_TRY_ASSIGN(innerCommandBuffers[i], ToBackend(commandBuffer)->Encode());
    }

    auto& wgpu = ToBackend(GetDevice())->wgpu.get();
    wgpu.queueSubmit(mInnerHandle, innerCommandBuffers.size(), innerCommandBuffers.data());

    for (WGPUCommandBuffer commandBuffer : innerCommandBuffers) {
        wgpu.commandBufferRelease(commandBuffer);
    }

    DAWN_TRY(SubmitFutureSync());
    return {};
}

CaptureContext* Queue::GetCaptureContext() const {
    return mCaptureContext.get();
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  Span<const std::byte> data) {
    if (IsCapturing()) {
        DAWN_TRY(mCaptureContext->CaptureQueueWriteBuffer(ToBackend(buffer), bufferOffset, data));
    }

    auto innerBuffer = ToBackend(buffer)->GetInnerHandle();
    ToBackend(GetDevice())
        ->wgpu->queueWriteBuffer(mInnerHandle, innerBuffer, bufferOffset, data.data(), data.size());
    buffer->MarkUsedInPendingCommands();

    return {};
}

MaybeError Queue::WriteTextureImpl(const TexelCopyTextureInfo& destination,
                                   Span<const std::byte> data,
                                   const TexelCopyBufferLayout& dataLayout,
                                   const Extent3D& writeSizePixel) {
    if (IsCapturing()) {
        DAWN_TRY(mCaptureContext->CaptureQueueWriteTexture(destination, data, dataLayout,
                                                           writeSizePixel));
    }

    TextureCopy copy;
    copy.texture = destination.texture;
    copy.mipLevel = destination.mipLevel;
    copy.origin = destination.origin;
    copy.aspect = ConvertAspect(destination.texture->GetFormat(), destination.aspect);

    WGPUTexelCopyTextureInfo dest = ToWGPU(copy);
    WGPUTexelCopyBufferLayout layout = {
        .offset = dataLayout.offset,
        .bytesPerRow = dataLayout.bytesPerRow,
        .rowsPerImage = dataLayout.rowsPerImage,
    };
    WGPUExtent3D writeSize = ToWGPU(writeSizePixel);
    ToBackend(destination.texture)->SynchronizeTextureBeforeUse();
    ToBackend(GetDevice())
        ->wgpu->queueWriteTexture(mInnerHandle, &dest, data.data(), data.size(), &layout,
                                  &writeSize);
    destination.texture->SetIsSubresourceContentInitialized(
        true, GetSubresourcesAffectedByCopy(copy, writeSizePixel));

    return {};
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    auto& wgpu = ToBackend(GetDevice())->wgpu.get();
    return mFuturesInFlight.Use([&](auto futuresInFlight) -> ResultOrError<ExecutionSerial> {
        ExecutionSerial fenceSerial(GetCompletedCommandSerial());
        while (!futuresInFlight->empty()) {
            auto [future, tentativeSerial] = futuresInFlight->front();

            WGPUFutureWaitInfo waitInfo = {future, false};
            WGPUWaitStatus status =
                wgpu.instanceWaitAny(ToBackend(GetDevice())->GetInnerInstance(), 1, &waitInfo, 0);

            if (status == WGPUWaitStatus_TimedOut) {
                return fenceSerial;
            }
            DAWN_TRY(CheckWGPUSuccess(status, "instanceWaitAny"));

            // Update fenceSerial since future is ready.
            fenceSerial = tentativeSerial;

            futuresInFlight->pop_front();
        }

        return fenceSerial;
    });
}

void Queue::ForceEventualFlushOfCommands() {
    mHasPendingCommands = true;
}

bool Queue::HasPendingCommands() const {
    return mHasPendingCommands;
}

MaybeError Queue::SubmitPendingCommandsImpl() {
    if (mHasPendingCommands) {
        DAWN_TRY(SubmitFutureSync());
    }
    return {};
}

MaybeError Queue::SubmitFutureSync() {
    // Call queueOnSubmittedWorkDone to get a future and maintain in mFuturesFlight.
    // TODO(crbug.com/413053623): Essentially track only via callbacks spontaneously, move content
    // from CheckAndUpdateCompletedSerials to WGPUQueueWorkDoneCallbackInfo::callback
    WGPUFuture future =
        ToBackend(GetDevice())
            ->wgpu->queueOnSubmittedWorkDone(
                mInnerHandle,
                {nullptr, WGPUCallbackMode_AllowSpontaneous,
                 [](WGPUQueueWorkDoneStatus, WGPUStringView, void*, void*) {}, nullptr, nullptr});
    if (future.id == kNullFutureID) {
        return DAWN_INTERNAL_ERROR("inner queueOnSubmittedWorkDone returned a null future.");
    }
    IncrementLastSubmittedCommandSerial();
    mFuturesInFlight.Use([&](auto futuresInFlight) {
        futuresInFlight->emplace_back(future, GetLastSubmittedCommandSerial());
    });
    mHasPendingCommands = false;
    return {};
}

ResultOrError<ExecutionSerial> Queue::WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                             Nanoseconds timeout) {
    return mFuturesInFlight.Use([&](auto futuresInFlight) -> ResultOrError<ExecutionSerial> {
        WGPUFuture future = {kNullFutureID};
        ExecutionSerial completedSerial = kWaitSerialTimeout;
        for (const auto& f : *futuresInFlight) {
            if (f.second >= waitSerial) {
                future = f.first;
                completedSerial = f.second;
                break;
            }
        }
        if (future.id == kNullFutureID) {
            return waitSerial;
        }

        WGPUFutureWaitInfo waitInfo = {future, false};
        WGPUWaitStatus status =
            ToBackend(GetDevice())
                ->wgpu->instanceWaitAny(ToBackend(GetDevice())->GetInnerInstance(), 1, &waitInfo,
                                        static_cast<uint64_t>(timeout));

        switch (status) {
            case WGPUWaitStatus_TimedOut:
                return kWaitSerialTimeout;
            case WGPUWaitStatus_Success:
                return completedSerial;
            default:
                return DAWN_FORMAT_INTERNAL_ERROR("inner instanceWaitAny status is (%s).",
                                                  FromAPI(status));
        }
    });
}

MaybeError Queue::WaitForIdleForDestructionImpl() {
    auto& wgpu = ToBackend(GetDevice())->wgpu.get();
    mFuturesInFlight.Use([&](auto futuresInFlight) {
        while (!futuresInFlight->empty()) {
            WGPUFuture future = futuresInFlight->front().first;
            WGPUFutureWaitInfo waitInfo = {future, false};
            wgpu.instanceWaitAny(ToBackend(GetDevice())->GetInnerInstance(), 1, &waitInfo,
                                 UINT64_MAX);

            futuresInFlight->pop_front();
        }
    });
    mHasPendingCommands = false;
    return {};
}

void Queue::DestroyImpl(DestroyReason reason) {
    mSharedFence = nullptr;
}

bool Queue::IsCapturing() const {
    return mCaptureContext != nullptr;
}

MaybeError Queue::SetCaptureContext(std::unique_ptr<CaptureContext> captureContext) {
    if (captureContext) {
        DAWN_INVALID_IF(mCaptureContext != nullptr, "A capture is already in progress.");
        mCaptureContext = std::move(captureContext);
    } else {
        DAWN_INVALID_IF(!mCaptureContext, "No capture is in progress.");
        mCaptureContext.reset();
    }
    return {};
}

}  // namespace dawn::native::webgpu
