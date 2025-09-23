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

#ifndef SRC_DAWN_NATIVE_VULKAN_QUEUEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_QUEUEVK_H_

#include <deque>
#include <utility>
#include <vector>

#include "dawn/common/SerialQueue.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Device.h"
#include "dawn/native/Queue.h"
#include "dawn/native/vulkan/CommandRecordingContextVk.h"

namespace dawn::native::vulkan {

class Device;

class Queue final : public QueueBase {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device,
                                            const QueueDescriptor* descriptor,
                                            uint32_t family);

    VkQueue GetVkQueue() const;

    CommandRecordingContext* GetPendingRecordingContext(SubmitMode submitMode = SubmitMode::Normal);
    MaybeError SplitRecordingContext(CommandRecordingContext* recordingContext);
    void RecycleCompletedCommands(ExecutionSerial completedSerial);

    ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                          Nanoseconds timeout) override;

  private:
    Queue(Device* device, const QueueDescriptor* descriptor, uint32_t family);
    ~Queue() override;
    using QueueBase::QueueBase;

    MaybeError Initialize();

    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override;
    bool HasPendingCommands() const override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void ForceEventualFlushOfCommands() override;
    MaybeError WaitForIdleForDestruction() override;
    MaybeError SubmitPendingCommandsImpl() override;
    void DestroyImpl() override;

    // Dawn API
    void SetLabelImpl() override;

    ResultOrError<VkFence> GetUnusedFence();

    // We track which operations are in flight on the GPU with an increasing serial.
    // This works only because we have a single queue. Each submit to a queue is associated
    // to a serial and a fence, such that when the fence is "ready" we know the operations
    // have finished.
    MutexProtected<std::deque<std::pair<VkFence, ExecutionSerial>>> mFencesInFlight;
    // Fences in the unused list aren't reset yet.
    MutexProtected<std::vector<VkFence>> mUnusedFences;

    MaybeError PrepareRecordingContext();
    ResultOrError<CommandPoolAndBuffer> BeginVkCommandBuffer();

    SerialQueue<ExecutionSerial, CommandPoolAndBuffer> mCommandsInFlight;
    // Command pools in the unused list haven't been reset yet.
    std::vector<CommandPoolAndBuffer> mUnusedCommands;
    // There is always a valid recording context stored in mRecordingContext
    CommandRecordingContext mRecordingContext;

    uint32_t mQueueFamily = 0;
    VkQueue mQueue = VK_NULL_HANDLE;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_QUEUEVK_H_
