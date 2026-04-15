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

#ifndef SRC_DAWN_NATIVE_WEBGPU_QUEUEWGPU_H_
#define SRC_DAWN_NATIVE_WEBGPU_QUEUEWGPU_H_

#include <deque>
#include <memory>
#include <utility>

#include "dawn/common/MutexProtected.h"
#include "dawn/native/Queue.h"
#include "dawn/native/webgpu/Forward.h"
#include "dawn/native/webgpu/ObjectWGPU.h"

namespace dawn::native::webgpu {

class CaptureContext;

class Queue final : public QueueBase, public ObjectWGPU<WGPUQueue> {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device, const QueueDescriptor* descriptor);

    bool IsCapturing() const;
    MaybeError SetCaptureContext(std::unique_ptr<CaptureContext> captureContext);
    CaptureContext* GetCaptureContext() const;

    // Returns a SharedFence wrapping the inner SharedFence handle.
    ResultOrError<Ref<SharedFence>> GetOrCreateSharedFence(WGPUSharedFence innerFence);

  private:
    Queue(Device* device, const QueueDescriptor* descriptor);
    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override;
    MaybeError WriteBufferImpl(BufferBase* buffer,
                               uint64_t bufferOffset,
                               const void* data,
                               size_t size) override;
    MaybeError WriteTextureImpl(const TexelCopyTextureInfo& destination,
                                const void* data,
                                size_t dataSize,
                                const TexelCopyBufferLayout& dataLayout,
                                const Extent3D& writeSizePixel) override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void ForceEventualFlushOfCommands() override;
    bool HasPendingCommands() const override;
    MaybeError SubmitPendingCommandsImpl() override;
    ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                          Nanoseconds timeout) override;
    MaybeError WaitForIdleForDestructionImpl() override;
    void DestroyImpl(DestroyReason reason) override;
    MaybeError SubmitFutureSync();

    MutexProtected<std::deque<std::pair<WGPUFuture, ExecutionSerial>>> mFuturesInFlight;
    Ref<SharedFence> mSharedFence;
    bool mHasPendingCommands = false;

    std::unique_ptr<CaptureContext> mCaptureContext;
};

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_QUEUEWGPU_H_
