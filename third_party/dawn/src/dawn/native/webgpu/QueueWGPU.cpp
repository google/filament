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

#include "dawn/native/webgpu/QueueWGPU.h"

#include <limits>
#include <vector>

#include "dawn/native/Error.h"
#include "dawn/native/Queue.h"
#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/CommandBufferWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/WebGPUError.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    return AcquireRef(new Queue(device, descriptor));
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor)
    : QueueBase(device, descriptor), ObjectWGPU(device->wgpu.queueRelease) {
    mInnerHandle = device->wgpu.deviceGetQueue(device->GetInnerHandle());
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    if (commandCount == 0 || commands == nullptr) {
        return {};
    }

    auto& wgpu = ToBackend(GetDevice())->wgpu;

    std::vector<WGPUCommandBuffer> innerCommandBuffers(commandCount);
    for (uint32_t i = 0; i < commandCount; ++i) {
        innerCommandBuffers[i] = ToBackend(commands[i])->Encode();
    }

    wgpu.queueSubmit(mInnerHandle, commandCount, innerCommandBuffers.data());

    for (uint32_t i = 0; i < commandCount; ++i) {
        wgpu.commandBufferRelease(innerCommandBuffers[i]);
    }

    DAWN_TRY(SubmitFutureSync());
    return {};
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    auto innerBuffer = ToBackend(buffer)->GetInnerHandle();
    ToBackend(GetDevice())
        ->wgpu.queueWriteBuffer(mInnerHandle, innerBuffer, bufferOffset, data, size);
    buffer->MarkUsedInPendingCommands();
    return {};
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    auto& wgpu = ToBackend(GetDevice())->wgpu;
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
            ->wgpu.queueOnSubmittedWorkDone(
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
                ->wgpu.instanceWaitAny(ToBackend(GetDevice())->GetInnerInstance(), 1, &waitInfo,
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

MaybeError Queue::WaitForIdleForDestruction() {
    auto& wgpu = ToBackend(GetDevice())->wgpu;
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

}  // namespace dawn::native::webgpu
