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
#include <memory>
#include <mutex>
#include <optional>
#include <variant>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/container/flat_hash_map.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Atomic.h"
#include "src/dawn/common/FutureUtils.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/Time.h"
#include "src/dawn/common/WeakRef.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/native/Forward.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/utils/non_movable.h"
#include "src/utils/span.h"

namespace dawn::native {

struct InstanceDescriptor;
class QueueBase;

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
    class TrackedEvent;

    explicit EventManager(InstanceBase* instance);
    ~EventManager();

    // Track a TrackedEvent and give it a FutureID.
    FutureID TrackEvent(Ref<TrackedEvent>&&);

    // Note that SetFutureReady takes a Ref<TrackedEvent> instead of a TrackedEvent* because TSAN
    // found that this function uses the event after setting it ready meaning another thread can
    // race to complete the event as soon as it's ready, i.e. via a WaitAny call, potentially
    // completing and freeing the event leaving the remainder of this call referencing a freed
    // event.
    void SetFutureReady(Ref<TrackedEvent> event);

    // Returns true if future ProcessEvents is needed.
    bool ProcessPollEvents();
    [[nodiscard]] wgpu::WaitStatus WaitAny(Span<FutureWaitInfo> infos, Nanoseconds timeout);

  private:
    // Internal waiter class that's signaled when specific events become ready.
    class Waiter;

    using EventMap = absl::flat_hash_map<FutureID, Ref<TrackedEvent>>;
    using SortedEventMap = absl::btree_map<FutureID, Ref<TrackedEvent>>;

    // Raw pointer to the Instance to allow for logging. The Instance owns the EventManager, so a
    // raw pointer here is always safe.
    raw_ptr<const InstanceBase> mInstance;
    std::atomic<FutureID> mNextFutureID = 1;

    struct EventState {
        EventMap events;
        // The Waiter's always live on the stack of a calling thread to WaitAny and are removed as
        // soon as that thread yields from the wait. Therefore the pointers are guaranteed to be
        // valid if they are in the list.
        absl::flat_hash_map<FutureID, std::vector<raw_ptr<Waiter>>> waiters;
    };
    MutexProtected<EventState> mEventState;

    // Records last process event id in order to properly return whether or not there are still
    // events to process when we have re-entrant callbacks.
    std::atomic<FutureID> mLastProcessEventID = 0;
};

struct QueueAndSerial {
    WeakRef<QueueBase> queue;
    std::atomic<ExecutionSerial> completionSerial;

    QueueAndSerial(QueueBase* q, ExecutionSerial serial);

    // Returns the most recently completed serial on |queue|. Otherwise, returns |completionSerial|.
    ExecutionSerial GetCompletedSerial() const;
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
  public:
    // Subclasses must implement this to complete the event (if not completed) with
    // EventCompletionType::Shutdown.
    ~TrackedEvent() override;

    // Create and track a TrackedEvent that is already completed.
    // Note: if callbackMode is set to other value, the caller need to make sure to wait on it to
    // avoid leakage.
    static Ref<TrackedEvent> CreateAlreadyCompletedEvent(
        EventManager* eventManager,
        wgpu::CallbackMode callbackMode = wgpu::CallbackMode::AllowSpontaneous);

    Future GetFuture() const;

    bool IsProgressing() const { return mIsProgressing; }

    bool IsReadyToComplete() const;

    QueueAndSerial* GetIfQueueAndSerial();
    const QueueAndSerial* GetIfQueueAndSerial() const;

  protected:
    friend class EventManager;

    // Create an event that can be manually signaled.
    explicit TrackedEvent(wgpu::CallbackMode callbackMode, bool readyToComplete = false);

    // Create a TrackedEvent from a queue completion serial.
    TrackedEvent(wgpu::CallbackMode callbackMode,
                 QueueBase* queue,
                 ExecutionSerial completionSerial);

    // Create a TrackedEvent that is already completed.
    struct Completed {};
    TrackedEvent(wgpu::CallbackMode callbackMode, Completed tag);

    // Some events may be non-progressing, i.e. DeviceLost. We tag these events so that we can
    // correctly return whether there is progressing work when users are polling.
    struct NonProgressing {};
    TrackedEvent(wgpu::CallbackMode callbackMode, NonProgressing tag);

    void EnsureComplete(EventCompletionType);
    virtual void Complete(EventCompletionType) = 0;

    wgpu::CallbackMode mCallbackMode = wgpu::CallbackMode::WaitAnyOnly;
    FutureID mFutureID = kNullFutureID;

  private:
    // The EventManager is the only place that should ever call |SetReadyToComplete| because it is
    // the one responsible for notifying/waking threads up for newly completed events.
    friend class EventManager;
    void SetReadyToComplete();

    const bool mIsProgressing = true;

    // Events may be one of two types:
    // - A boolean which will be updated from our code.
    // - A queue and the ExecutionSerial after which the event will be completed. Note that the
    //   atomic bool above can override the status of the queue and serial, i.e. if the bool is
    //   set to true, the event will be marked as ready to complete regardless of whether the
    //   queue has actually reached the corresponding serial.
    Atomic<bool, std::memory_order_acquire, std::memory_order_release> mIsReadyToComplete{false};

    // Note that currently, the queue ref creates a temporary ref cycle
    // (Queue->Device->Instance->EventManager->TrackedEvent). This is OK because the instance will
    // clear out the EventManager on shutdown.
    // TODO(crbug.com/dawn/2067): This is a bit fragile. Is it possible to remove the ref cycle?
    std::optional<QueueAndSerial> mQueueAndSerial;

    // Flag used to ensure that the callback is only completed once.
    std::once_flag mFlag;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EVENTMANAGER_H_
