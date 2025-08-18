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
#include "dawn/common/WeakRef.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/WaitListEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

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

    Future GetFuture() const;

    bool IsProgressing() const { return mIsProgressing; }

    bool IsReadyToComplete() const;

    QueueAndSerial* GetIfQueueAndSerial() { return std::get_if<QueueAndSerial>(&mCompletionData); }
    const QueueAndSerial* GetIfQueueAndSerial() const {
        return std::get_if<QueueAndSerial>(&mCompletionData);
    }

    Ref<SystemEvent> GetIfSystemEvent() const {
        if (auto* event = std::get_if<Ref<SystemEvent>>(&mCompletionData)) {
            return *event;
        }
        return nullptr;
    }

    Ref<WaitListEvent> GetIfWaitListEvent() const {
        if (auto* event = std::get_if<Ref<WaitListEvent>>(&mCompletionData)) {
            return *event;
        }
        return nullptr;
    }

    // Events may be one of three types:
    // - A queue and the ExecutionSerial after which the event will be completed.
    //   Used for queue completion.
    // - A SystemEvent which will be signaled usually by the OS / GPU driver. It stores a boolean
    //   that we can check instead of polling with the OS, or it can be transformed lazily into a
    //   SystemEventReceiver.
    // - A WaitListEvent which will be signaled from our code, usually on a separate thread. It also
    //   stores an atomic boolean that we can check instead of waiting synchronously, or it can be
    //   transformed into a SystemEventReceiver for asynchronous waits.
    // The queue ref creates a temporary ref cycle
    // (Queue->Device->Instance->EventManager->TrackedEvent). This is OK because the instance will
    // clear out the EventManager on shutdown.
    // TODO(crbug.com/dawn/2067): This is a bit fragile. Is it possible to remove the ref cycle?
  protected:
    friend class EventManager;

    using CompletionData = std::variant<QueueAndSerial, Ref<SystemEvent>, Ref<WaitListEvent>>;

    // Create an event from a WaitListEvent that can be signaled and waited-on in user-space only in
    // the current process. Note that events like RequestAdapter and RequestDevice complete
    // immediately in dawn native, and may use an already-completed event.
    TrackedEvent(wgpu::CallbackMode callbackMode, Ref<WaitListEvent> completionEvent);

    // Create an event from a SystemEvent. Note that events like RequestAdapter and
    // RequestDevice complete immediately in dawn native, and may use an already-completed event.
    TrackedEvent(wgpu::CallbackMode callbackMode, Ref<SystemEvent> completionEvent);

    // Create a TrackedEvent from a queue completion serial.
    TrackedEvent(wgpu::CallbackMode callbackMode,
                 QueueBase* queue,
                 ExecutionSerial completionSerial);

    // Create a TrackedEvent that is already completed.
    struct Completed {};
    TrackedEvent(wgpu::CallbackMode callbackMode, Completed tag);

    // Some SystemEvents may be non-progressing, i.e. DeviceLost. We tag these events so that we can
    // correctly return whether there is progressing work when users are polling.
    struct NonProgressing {};
    TrackedEvent(wgpu::CallbackMode callbackMode, NonProgressing tag);

    void SetReadyToComplete();

    void EnsureComplete(EventCompletionType);
    virtual void Complete(EventCompletionType) = 0;

    wgpu::CallbackMode mCallbackMode;
    FutureID mFutureID = kNullFutureID;

  private:
    CompletionData mCompletionData;
    const bool mIsProgressing = true;
    // Whether the callback has been called. Note that this is a MutexProtected<bool> because for
    // spontaneous events, multiple threads may call |EnsureComplete| and that function should only
    // return after the actual callback is completed. Without the lock, previous to this change we
    // just had an std::atomic<bool>, two threads could race, and the thread that does not run the
    // callback can make forward progress even though the callback hasn't completed on the other
    // thread yet.
    MutexProtected<bool> mCompleted;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EVENTMANAGER_H_
