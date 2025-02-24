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

#ifndef SRC_DAWN_NATIVE_EVENTMANAGER_H_
#define SRC_DAWN_NATIVE_EVENTMANAGER_H_

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <variant>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/FutureUtils.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/SystemEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

struct InstanceDescriptor;

// Subcomponent of the Instance which tracks callback events for the Future-based callback
// entrypoints. All events from this instance (regardless of whether from an adapter, device, queue,
// etc.) are tracked here, and used by the instance-wide ProcessEvents and WaitAny entrypoints.
//
// TODO(crbug.com/dawn/2050): Can this eventually replace CallbackTaskManager?
//
// There are various ways to optimize ProcessEvents/WaitAny:
// - TODO(crbug.com/dawn/2059) Spontaneously set events as "early-ready" in other places when we see
//   serials advance, e.g. Submit, or when checking a later wait before an earlier wait.
class EventManager final : NonMovable {
  public:
    explicit EventManager(InstanceBase* instance);
    ~EventManager();

    MaybeError Initialize(const UnpackedPtr<InstanceDescriptor>& descriptor);
    // Called by WillDropLastExternalRef. Once shut down, the EventManager stops tracking anything.
    // It drops any refs to TrackedEvents, to break reference cycles. If doing so frees the last ref
    // of any uncompleted TrackedEvents, they'll get completed with EventCompletionType::Shutdown.
    void ShutDown();

    class TrackedEvent;
    // Track a TrackedEvent and give it a FutureID.
    FutureID TrackEvent(Ref<TrackedEvent>&&);
    void SetFutureReady(TrackedEvent* event);

    // Returns true if future ProcessEvents is needed.
    bool ProcessPollEvents();
    [[nodiscard]] wgpu::WaitStatus WaitAny(size_t count,
                                           FutureWaitInfo* infos,
                                           Nanoseconds timeout);

  private:
    bool IsShutDown() const;

    // Raw pointer to the Instance to allow for logging. The Instance owns the EventManager, so a
    // raw pointer here is always safe.
    raw_ptr<const InstanceBase> mInstance;

    bool mTimedWaitAnyEnable = false;
    size_t mTimedWaitAnyMaxCount = kTimedWaitAnyMaxCountDefault;
    std::atomic<FutureID> mNextFutureID = 1;

    // Freed once the user has dropped their last ref to the Instance, so can't call WaitAny or
    // ProcessEvents anymore. This breaks reference cycles.
    using EventMap = absl::flat_hash_map<FutureID, Ref<TrackedEvent>>;
    MutexProtected<std::optional<EventMap>> mEvents;

    // Records last process event id in order to properly return whether or not there are still
    // events to process when we have re-entrant callbacks.
    std::atomic<FutureID> mLastProcessEventID = 0;
};

struct QueueAndSerial {
    Ref<QueueBase> queue;
    ExecutionSerial completionSerial;
};

// Base class for the objects that back WGPUFutures. TrackedEvent is responsible for the lifetime
// the callback it contains. If TrackedEvent gets destroyed before it completes, it's responsible
// for cleaning up (by calling the callback with an "Unknown" status).
//
// For Future-based and ProcessEvents-based TrackedEvents, the EventManager will track them for
// completion in WaitAny or ProcessEvents. However, once the Instance has lost all its external
// refs, the user can't call either of those methods anymore, so EventManager will stop holding refs
// to any TrackedEvents. Any which are not ref'd elsewhere (in order to be `Spontaneous`ly
// completed) will be cleaned up at that time.
class EventManager::TrackedEvent : public RefCounted {
  protected:
    // Create an event from a SystemEvent. Note that events like RequestAdapter and
    // RequestDevice complete immediately in dawn native, and may use an already-completed event.
    TrackedEvent(wgpu::CallbackMode callbackMode, Ref<SystemEvent> completionEvent);

    // Create a TrackedEvent from a queue completion serial.
    TrackedEvent(wgpu::CallbackMode callbackMode,
                 QueueBase* queue,
                 ExecutionSerial completionSerial);

    struct Completed {};
    // Create a TrackedEvent that is already completed.
    TrackedEvent(wgpu::CallbackMode callbackMode, Completed tag);

  public:
    // Subclasses must implement this to complete the event (if not completed) with
    // EventCompletionType::Shutdown.
    ~TrackedEvent() override;

    Future GetFuture() const;

    // Events may be one of two types:
    // - A queue and the ExecutionSerial after which the event will be completed.
    //   Used for queue completion.
    // - A SystemEvent which will be signaled from our code, usually on a separate thread.
    //   It stores a boolean that we can check instead of polling with the OS, or it can be
    //   transformed lazily into a SystemEventReceiver. Used for async pipeline creation, and Metal
    //   queue completion.
    // The queue ref creates a temporary ref cycle
    // (Queue->Device->Instance->EventManager->TrackedEvent). This is OK because the instance will
    // clear out the EventManager on shutdown.
    // TODO(crbug.com/dawn/2067): This is a bit fragile. Is it possible to remove the ref cycle?
    using CompletionData = std::variant<QueueAndSerial, Ref<SystemEvent>>;

    const CompletionData& GetCompletionData() const;

  protected:
    void EnsureComplete(EventCompletionType);
    virtual void Complete(EventCompletionType) = 0;

    wgpu::CallbackMode mCallbackMode;
    FutureID mFutureID = kNullFutureID;

#if DAWN_ENABLE_ASSERTS
    std::atomic<bool> mCurrentlyBeingWaited = false;
#endif

  private:
    friend class EventManager;

    CompletionData mCompletionData;
    // Callback has been called.
    std::atomic<bool> mCompleted = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EVENTMANAGER_H_
