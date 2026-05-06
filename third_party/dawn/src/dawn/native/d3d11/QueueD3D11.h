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

#include <map>
#include <optional>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"
#include "dawn/common/LinkedList.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/native/d3d/QueueD3D.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/Forward.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d11 {

class Device;
class SharedFence;

class Queue : public d3d::Queue {
  public:
    static ResultOrError<Ref<Queue>> Create(Device* device, const QueueDescriptor* descriptor);

    ScopedCommandRecordingContext GetScopedPendingCommandContext(SubmitMode submitMode,
                                                                 bool lockD3D11Scope = true);
    std::optional<ScopedCommandRecordingContext> TryGetScopedPendingCommandContext(
        SubmitMode submitMode,
        bool lockD3D11Scope = true);
    ScopedSwapStateCommandRecordingContext GetScopedSwapStatePendingCommandContext(
        SubmitMode submitMode);
    virtual MaybeError NextSerial() = 0;

    // Separated from creation because it creates resources, which is not valid before the
    // DeviceBase is fully created.
    MaybeError InitializePendingContext();

    // The request for buffer mapping.
    // We make it a linked list node to support O(1) removal when cancelling.
    struct BufferMapRequest : public LinkNode<BufferMapRequest> {
        BufferMapRequest(Buffer* b, wgpu::MapMode m) : buffer(b), mode(m) {}

        raw_ptr<Buffer> buffer = nullptr;
        wgpu::MapMode mode;
    };

    // Schedule a buffer for mapping.
    // The request is expected to be allocated by the caller. Any existing schedule will be
    // canceled before the new one is added.
    void ScheduleBufferMapping(BufferMapRequest* request, ExecutionSerial readySerial);

    // Cancel a scheduled buffer mapping by removing it from the pending list.
    void CancelScheduledBufferMapping(Buffer* buffer);

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

    void DestroyImpl(DestroyReason reason) override;
    bool HasPendingCommands() const override;
    void ForceEventualFlushOfCommands() override;
    MaybeError WaitForIdleForDestructionImpl() override;
    MaybeError SubmitPendingCommandsImpl() override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;

    ResultOrError<Ref<d3d::SharedFence>> GetOrCreateSharedFence() override;

    virtual ResultOrError<ExecutionSerial> CheckCompletedSerialsImpl() = 0;

    // Check and process scheduled buffer mappings.
    MaybeError CheckScheduledBufferMappings(ExecutionSerial completedSerial);

    // Helper template to create scoped command contexts with common logic
    template <typename ScopedContextType, typename... Args>
    ScopedContextType CreateScopedCommandContext(SubmitMode submitMode,
                                                 CommandRecordingContext::Guard&& commands,
                                                 Args&&... args);

    ComPtr<ID3D11Fence> mFence;
    Ref<SharedFence> mSharedFence;
    MutexProtected<CommandRecordingContext, CommandRecordingContextGuard> mPendingCommands;
    std::atomic<bool> mPendingCommandsNeedSubmit = false;

    struct BufferRefHash {
        using is_transparent = void;
        size_t operator()(const Ref<Buffer>& buffer) const {
            return absl::DefaultHashContainerHash<const Buffer*>()(buffer.Get());
        }
        size_t operator()(const Buffer* buffer) const {
            return absl::DefaultHashContainerHash<const Buffer*>()(buffer);
        }
    };

    struct BufferRefEq {
        using is_transparent = void;
        bool operator()(const Ref<Buffer>& a, const Ref<Buffer>& b) const {
            return a.Get() == b.Get();
        }
        bool operator()(const Ref<Buffer>& a, const Buffer* b) const { return a.Get() == b; }
        bool operator()(const Buffer* a, const Ref<Buffer>& b) const { return a == b.Get(); }
    };

    struct BufferMapList {
        // Map from ExecutionSerial to a list of buffers to be mapped when the serial passes.
        // The map is used to process the requests in order of serial.
        // We use LinkedList to allow O(1) removal of an entry when it's cancelled.
        std::map<ExecutionSerial, LinkedList<BufferMapRequest>> serialQueue;
        // Map from Buffer* to BufferMapRequest*.
        // This map table serves 2 purposes: first is to keep the buffer alive via Ref.
        // 2nd is fast lookup for the existing schedule.
        absl::flat_hash_map<Ref<Buffer>, BufferMapRequest*, BufferRefHash, BufferRefEq> requestMap;
    };
    MutexProtected<BufferMapList> mPendingMapBuffers;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_QUEUED3D11_H_
