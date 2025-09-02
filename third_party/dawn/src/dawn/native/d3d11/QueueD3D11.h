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

#ifndef SRC_DAWN_NATIVE_D3D11_QUEUED3D11_H_
#define SRC_DAWN_NATIVE_D3D11_QUEUED3D11_H_

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialMap.h"
#include "dawn/native/d3d/QueueD3D.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/Forward.h"

namespace dawn::native::d3d11 {

class Device;
class SharedFence;

class Queue : public d3d::Queue {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device, const QueueDescriptor* descriptor);

    ScopedCommandRecordingContext GetScopedPendingCommandContext(SubmitMode submitMode);
    ScopedSwapStateCommandRecordingContext GetScopedSwapStatePendingCommandContext(
        SubmitMode submitMode);
    virtual MaybeError NextSerial() = 0;

    // Separated from creation because it creates resources, which is not valid before the
    // DeviceBase is fully created.
    MaybeError InitializePendingContext();

    // Register the pending map buffer to be checked.
    void TrackPendingMapBuffer(Ref<Buffer>&& buffer,
                               wgpu::MapMode mode,
                               ExecutionSerial readySerial);

    const Ref<SharedFence>& GetSharedFence() const { return mSharedFence; }

  protected:
    using d3d::Queue::Queue;

    ~Queue() override = default;

    MaybeError Initialize(bool useMonitoredFence);
    MaybeError InitializeD3DFence(bool useMonitoredFence);

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

    void DestroyImpl() override;
    bool HasPendingCommands() const override;
    void ForceEventualFlushOfCommands() override;
    MaybeError WaitForIdleForDestruction() override;
    MaybeError SubmitPendingCommandsImpl() override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;

    ResultOrError<Ref<d3d::SharedFence>> GetOrCreateSharedFence() override;

    virtual ResultOrError<ExecutionSerial> CheckCompletedSerialsImpl() = 0;

    // Check all pending map buffers, and actually map the ready ones.
    MaybeError CheckAndMapReadyBuffers(ExecutionSerial completedSerial);

    ComPtr<ID3D11Fence> mFence;
    Ref<SharedFence> mSharedFence;
    MutexProtected<CommandRecordingContext, CommandRecordingContextGuard> mPendingCommands;
    std::atomic<bool> mPendingCommandsNeedSubmit = false;

    struct BufferMapEntry {
        Ref<Buffer> buffer;
        wgpu::MapMode mode;
    };
    SerialMap<ExecutionSerial, BufferMapEntry> mPendingMapBuffers;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_QUEUED3D11_H_
