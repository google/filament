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

#include "dawn/native/EventManager.h"

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Atomic.h"
#include "dawn/common/FutureUtils.h"
#include "dawn/common/Log.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Queue.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/WaitAnySystemEvent.h"

namespace dawn::native {
namespace {

// Ref<TrackedEvent> plus a few extra fields needed for some implementations.
// Sometimes they'll be unused, but that's OK; it simplifies code reuse.
struct TrackedFutureWaitInfo {
    FutureID futureID;
    Ref<EventManager::TrackedEvent> event;
    // Used by EventManager::ProcessPollEvents
    size_t indexInInfos;
    // Used by EventManager::ProcessPollEvents and ::WaitAny
    bool ready;
};

// Wrapper around an iterator to yield event specific objects and a pointer
// to the ready bool. We pass this into helpers so that they can extract
// the event specific objects and get pointers to the ready status - without
// allocating duplicate storage to store the objects and ready bools.
template <typename Traits>
class WrappingIterator {
  public:
    // Specify required iterator traits.
    using value_type = typename Traits::value_type;
    using difference_type = typename Traits::WrappedIter::difference_type;
    using iterator_category = typename Traits::WrappedIter::iterator_category;
    using pointer = value_type*;
    using reference = value_type&;

    WrappingIterator() = default;
    WrappingIterator(const WrappingIterator&) = default;
    WrappingIterator& operator=(const WrappingIterator&) = default;

    explicit WrappingIterator(typename Traits::WrappedIter wrappedIt) : mWrappedIt(wrappedIt) {}

    bool operator==(const WrappingIterator& rhs) const = default;

    difference_type operator-(const WrappingIterator& rhs) const {
        return mWrappedIt - rhs.mWrappedIt;
    }

    WrappingIterator operator+(difference_type rhs) const {
        return WrappingIterator{mWrappedIt + rhs};
    }

    WrappingIterator& operator++() {
        ++mWrappedIt;
        return *this;
    }

    value_type operator*() { return Traits::Deref(mWrappedIt); }

  private:
    typename Traits::WrappedIter mWrappedIt;
};

struct ExtractSystemEventAndReadyStateTraits {
    using WrappedIter = std::vector<TrackedFutureWaitInfo>::iterator;
    using value_type = std::pair<const SystemEventReceiver&, bool*>;

    static value_type Deref(const WrappedIter& wrappedIt) {
        if (auto event = wrappedIt->event->GetIfWaitListEvent()) {
            return {event->WaitAsync(), &wrappedIt->ready};
        }
        DAWN_ASSERT(wrappedIt->event->GetIfSystemEvent());
        return {
            wrappedIt->event->GetIfSystemEvent()->GetOrCreateSystemEventReceiver(),
            &wrappedIt->ready,
        };
    }
};

using SystemEventAndReadyStateIterator = WrappingIterator<ExtractSystemEventAndReadyStateTraits>;

struct ExtractWaitListEventAndReadyStateTraits {
    using WrappedIter = std::vector<TrackedFutureWaitInfo>::iterator;
    using value_type = std::pair<Ref<WaitListEvent>, bool*>;

    static value_type Deref(const WrappedIter& wrappedIt) {
        DAWN_ASSERT(wrappedIt->event->GetIfWaitListEvent());
        return {wrappedIt->event->GetIfWaitListEvent(), &wrappedIt->ready};
    }
};

using WaitListEventAndReadyStateIterator =
    WrappingIterator<ExtractWaitListEventAndReadyStateTraits>;

// Returns true if at least one future is ready.
bool PollFutures(std::vector<TrackedFutureWaitInfo>& futures) {
    bool success = false;
    for (auto& future : futures) {
        if (future.event->IsReadyToComplete()) {
            success = true;
            future.ready = true;
        }
    }
    return success;
}

// Wait/poll queues with given `timeout`. `queueWaitSerials` should contain per queue, the serial up
// to which we should flush the queue if needed. Note that keys are WeakRef<QueueBase> which
// actually means the keys are not based on the QueueBase pointer, but a pointer to metadata that is
// guaranteed to be unique and alive. This ensures that each queue will be represented for multi
// source validation.
using QueueWaitSerialsMap = absl::flat_hash_map<WeakRef<QueueBase>, ExecutionSerial>;
void WaitQueueSerials(const QueueWaitSerialsMap& queueWaitSerials, Nanoseconds timeout) {
    // Poll/wait on queues up to the lowest wait serial, but do this once per queue instead of
    // per event so that events with same serial complete at the same time instead of racing.
    for (const auto& queueAndSerial : queueWaitSerials) {
        auto queue = queueAndSerial.first.Promote();
        if (queue == nullptr) {
            // If we can't promote the queue, then all the work is already done.
            continue;
        }
        auto waitSerial = queueAndSerial.second;

        auto* device = queue->GetDevice();
        [[maybe_unused]] bool error;
        error = device->ConsumedError(
            [&]() -> MaybeError {
                auto deviceGuard = device->GetGuard();

                if (waitSerial > queue->GetLastSubmittedCommandSerial()) {
                    // Serial has not been submitted yet. Submit it now.
                    DAWN_TRY(queue->EnsureCommandsFlushed(waitSerial));
                }
                // Check the completed serial.
                if (waitSerial > queue->GetCompletedCommandSerial()) {
                    if (timeout > Nanoseconds(0)) {
                        // Wait on the serial if it hasn't passed yet.
                        [[maybe_unused]] bool waitResult = false;
                        DAWN_TRY_ASSIGN(waitResult, queue->WaitForQueueSerial(waitSerial, timeout));
                    }
                }
                return {};
            }(),
            "waiting for work in %s.", queue.Get());

        // Updating completed serial cannot hold the device-wide lock because it may cause user
        // callbacks to fire.
        error = device->ConsumedError(queue->UpdateCompletedSerial(),
                                      "updating completed serial in %s", queue.Get());
    }
}

// We can replace the std::vector& when std::span is available via C++20.
wgpu::WaitStatus WaitImpl(const InstanceBase* instance,
                          std::vector<TrackedFutureWaitInfo>& futures,
                          Nanoseconds timeout) {
    bool foundSystemEvent = false;
    bool foundWaitListEvent = false;

    QueueWaitSerialsMap queueLowestWaitSerials;
    for (const auto& future : futures) {
        if (future.event->GetIfSystemEvent()) {
            foundSystemEvent = true;
        }
        if (future.event->GetIfWaitListEvent()) {
            foundWaitListEvent = true;
        }
        if (const auto* queueAndSerial = future.event->GetIfQueueAndSerial()) {
            auto [it, inserted] = queueLowestWaitSerials.insert(
                {queueAndSerial->queue,
                 queueAndSerial->completionSerial.load(std::memory_order_acquire)});
            if (!inserted) {
                it->second = std::min(
                    it->second, queueAndSerial->completionSerial.load(std::memory_order_acquire));
            }
        }
    }

    if (timeout == Nanoseconds(0)) {
        // This is a no-op if `queueLowestWaitSerials` is empty.
        WaitQueueSerials(queueLowestWaitSerials, timeout);
        return PollFutures(futures) ? wgpu::WaitStatus::Success : wgpu::WaitStatus::TimedOut;
    }

    // We can't have a mix of system/wait-list events and queue-serial events or queue-serial events
    // from multiple queues with a non-zero timeout.
    if (queueLowestWaitSerials.size() > 1 ||
        (!queueLowestWaitSerials.empty() && (foundWaitListEvent || foundSystemEvent))) {
        // Multi-source wait is unsupported.
        // TODO(dawn:2062): Implement support for this when the device supports it.
        // It should eventually gather the lowest serial from the queue(s), transform them
        // into completion events, and wait on all of the events. Then for any queues that
        // saw a completion, poll all futures related to that queue for completion.
        instance->EmitLog(WGPULoggingType_Error,
                          "Mixed source waits with timeouts are not currently supported.");
        return wgpu::WaitStatus::Error;
    }

    bool success = false;
    if (foundSystemEvent) {
        // Can upgrade wait list events to system events.
        success = WaitAnySystemEvent(SystemEventAndReadyStateIterator{futures.begin()},
                                     SystemEventAndReadyStateIterator{futures.end()}, timeout);
    } else if (foundWaitListEvent) {
        success =
            WaitListEvent::WaitAny(WaitListEventAndReadyStateIterator{futures.begin()},
                                   WaitListEventAndReadyStateIterator{futures.end()}, timeout);
    } else {
        // This is a no-op if `queueLowestWaitSerials` is empty.
        WaitQueueSerials(queueLowestWaitSerials, timeout);
        success = PollFutures(futures);
    }
    return success ? wgpu::WaitStatus::Success : wgpu::WaitStatus::TimedOut;
}

// Reorder callbacks to enforce callback ordering required by the spec.
// Returns an iterator just past the last ready callback.
auto PrepareReadyCallbacks(std::vector<TrackedFutureWaitInfo>& futures) {
    // Partition the futures so the following sort looks at fewer elements.
    auto endOfReady =
        std::partition(futures.begin(), futures.end(),
                       [](const TrackedFutureWaitInfo& future) { return future.ready; });

    // Enforce the following rules from https://gpuweb.github.io/gpuweb/#promise-ordering:
    // 1. For some GPUQueue q, if p1 = q.onSubmittedWorkDone() is called before
    //    p2 = q.onSubmittedWorkDone(), then p1 must settle before p2.
    // 2. For some GPUQueue q and GPUBuffer b on the same GPUDevice,
    //    if p1 = b.mapAsync() is called before p2 = q.onSubmittedWorkDone(),
    //    then p1 must settle before p2.
    //
    // To satisfy the rules, we need only put lower future ids before higher future
    // ids. Lower future ids were created first.
    std::sort(futures.begin(), endOfReady,
              [](const TrackedFutureWaitInfo& a, const TrackedFutureWaitInfo& b) {
                  return a.futureID < b.futureID;
              });

    return endOfReady;
}

}  // namespace

// EventManager

EventManager::EventManager(InstanceBase* instance) : mInstance(instance) {
    // Construct the non-movable inner struct.
    mEvents.Use([&](auto events) { (*events).emplace(); });
}

EventManager::~EventManager() {
    DAWN_ASSERT(IsShutDown());
}

MaybeError EventManager::Initialize(const UnpackedPtr<InstanceDescriptor>& descriptor) {
    if (descriptor) {
        for (auto feature :
             std::span(descriptor->requiredFeatures, descriptor->requiredFeatureCount)) {
            if (feature == wgpu::InstanceFeatureName::TimedWaitAny) {
                mTimedWaitAnyEnable = true;
                break;
            }
        }
        if (descriptor->requiredLimits) {
            mTimedWaitAnyMaxCount = std::max(kTimedWaitAnyMaxCountDefault,
                                             descriptor->requiredLimits->timedWaitAnyMaxCount);
        }
    }
    if (mTimedWaitAnyMaxCount > kTimedWaitAnyMaxCountDefault) {
        // We don't yet support a higher timedWaitAnyMaxCount because it would be complicated
        // to implement on Windows, and it isn't that useful to implement only on non-Windows.
        return DAWN_VALIDATION_ERROR("Requested timedWaitAnyMaxCount is not supported");
    }

    return {};
}

void EventManager::ShutDown() {
    mEvents.Use([&](auto events) {
        // For all non-spontaneous events, call their callbacks now.
        for (auto& [futureID, event] : **events) {
            if (event->mCallbackMode != wgpu::CallbackMode::AllowSpontaneous) {
                event->EnsureComplete(EventCompletionType::Shutdown);
            }
        }
        (*events).reset();
    });
}

bool EventManager::IsShutDown() const {
    return mEvents.Use([](auto events) { return !events->has_value(); });
}

FutureID EventManager::TrackEvent(Ref<TrackedEvent>&& event) {
    if (!ValidateCallbackMode(ToAPI(event->mCallbackMode))) {
        mInstance->EmitLog(WGPULoggingType_Error,
                           absl::StrFormat("Invalid callback mode: %d",
                                           static_cast<uint32_t>(event->mCallbackMode)));
        return kNullFutureID;
    }

    FutureID futureID = mNextFutureID++;
    event->mFutureID = futureID;

    // Handle the event now if it's spontaneous and ready.
    if (event->mCallbackMode == wgpu::CallbackMode::AllowSpontaneous) {
        if (event->IsReadyToComplete()) {
            event->EnsureComplete(EventCompletionType::Ready);
            return futureID;
        }
    }

    if (const auto* queueAndSerial = event->GetIfQueueAndSerial()) {
        if (auto q = queueAndSerial->queue.Promote()) {
            q->TrackSerialTask(queueAndSerial->completionSerial, [this, event]() {
                // If this is executed, we can be sure that the raw pointer to this EventManager is
                // valid because the Queue is alive and:
                //   Queue -[refs]->
                //     Device -[refs]->
                //       Adapter -[refs]->
                //         Instance -[owns]->
                //           EventManager.
                SetFutureReady(event.Get());
            });
        }
    }

    mEvents.Use([&](auto events) {
        if (!events->has_value()) {
            // We are shutting down, so if the event isn't spontaneous, call the callback now.
            if (event->mCallbackMode != wgpu::CallbackMode::AllowSpontaneous) {
                event->EnsureComplete(EventCompletionType::Shutdown);
            }
            // Otherwise, in native, the event manager is not in charge of tracking the event, so
            // just return early now.
            return;
        }
        if (event->mCallbackMode != wgpu::CallbackMode::WaitAnyOnly) {
            FetchMax(mLastProcessEventID, futureID);
        }
        (*events)->emplace(futureID, std::move(event));
    });
    return futureID;
}

void EventManager::SetFutureReady(TrackedEvent* event) {
    event->SetReadyToComplete();

    // Sometimes, events might become ready before they are even tracked. This can happen because
    // tracking is ordered to uphold callback ordering, but events may become ready in any order. If
    // the event is spontaneous, it will be completed when it is tracked.
    if (event->mFutureID == kNullFutureID) {
        return;
    }

    // Handle spontaneous completion now.
    if (event->mCallbackMode == wgpu::CallbackMode::AllowSpontaneous) {
        // Since we use the presence of the event to indicate whether the callback has already been
        // called in WaitAny when searching for the matching FutureID, untrack the event after
        // calling the callbacks to ensure that we can't race on two different threads waiting on
        // the same future. Note that only one thread will actually call the callback since
        // EnsureComplete is thread safe.
        event->EnsureComplete(EventCompletionType::Ready);
        mEvents.Use([&](auto events) {
            if (!events->has_value()) {
                return;
            }
            (*events)->erase(event->mFutureID);
        });
    }
}

bool EventManager::ProcessPollEvents() {
    DAWN_ASSERT(!IsShutDown());

    std::vector<TrackedFutureWaitInfo> futures;
    wgpu::WaitStatus waitStatus;
    bool hasProgressingEvents = false;
    FutureID lastProcessEventID;
    mEvents.Use([&](auto events) {
        // Iterate all events and record poll events and spontaneous events since they are both
        // allowed to be completed in the ProcessPoll call. Note that spontaneous events are allowed
        // to trigger anywhere which is why we include them in the call.
        lastProcessEventID = mLastProcessEventID.load(std::memory_order_acquire);
        futures.reserve((*events)->size());
        for (auto& [futureID, event] : **events) {
            if (event->mCallbackMode != wgpu::CallbackMode::WaitAnyOnly) {
                // Figure out if there are any progressing events. If we only have non-progressing
                // events, we need to return false to indicate that there isn't any polling work to
                // be done.
                if (event->IsProgressing()) {
                    hasProgressingEvents = true;
                }

                futures.push_back(TrackedFutureWaitInfo{futureID, event, 0, false});
            }
        }
    });

    // If there wasn't anything to wait on, we can skip the wait and just return.
    if (futures.size() == 0) {
        return false;
    }

    // Wait and enforce callback ordering.
    waitStatus = WaitImpl(mInstance, futures, Nanoseconds(0));
    if (waitStatus == wgpu::WaitStatus::TimedOut) {
        return hasProgressingEvents;
    }
    DAWN_ASSERT(waitStatus == wgpu::WaitStatus::Success);

    // Enforce callback ordering.
    auto readyEnd = PrepareReadyCallbacks(futures);
    bool hasIncompleteEvents = readyEnd != futures.end();

    // Call all the callbacks.
    for (auto it = futures.begin(); it != readyEnd; ++it) {
        it->event->EnsureComplete(EventCompletionType::Ready);
    }

    // Since we use the presence of the event to indicate whether the callback has already been
    // called in WaitAny when searching for the matching FutureID, untrack the event after calling
    // the callbacks to ensure that we can't race on two different threads waiting on the same
    // future. Note that only one thread will actually call the callback since EnsureComplete is
    // thread safe.
    mEvents.Use([&](auto events) {
        for (auto it = futures.begin(); it != readyEnd; ++it) {
            (*events)->erase(it->futureID);
        }
    });

    // Note that in the event of all progressing events completing, but there exists non-progressing
    // events, we will return true one extra time.
    return hasIncompleteEvents ||
           (lastProcessEventID != mLastProcessEventID.load(std::memory_order_acquire));
}

wgpu::WaitStatus EventManager::WaitAny(size_t count, FutureWaitInfo* infos, Nanoseconds timeout) {
    DAWN_ASSERT(!IsShutDown());

    // Validate for feature support.
    if (timeout > Nanoseconds(0)) {
        if (!mTimedWaitAnyEnable) {
            mInstance->EmitLog(WGPULoggingType_Error,
                               "Timeout waits are either not enabled or not supported.");
            return wgpu::WaitStatus::Error;
        }
        if (count > mTimedWaitAnyMaxCount) {
            mInstance->EmitLog(
                WGPULoggingType_Error,
                absl::StrFormat("Number of futures to wait on (%d) exceeds maximum (%d).", count,
                                mTimedWaitAnyMaxCount));
            return wgpu::WaitStatus::Error;
        }
        // UnsupportedMixedSources is validated later, in WaitImpl.
    }

    if (count == 0) {
        return wgpu::WaitStatus::Success;
    }

    // Look up all of the futures and build a list of `TrackedFutureWaitInfo`s.
    std::vector<TrackedFutureWaitInfo> futures;
    futures.reserve(count);
    bool anyCompleted = false;
    mEvents.Use([&](auto events) {
        FutureID firstInvalidFutureID = mNextFutureID;
        for (size_t i = 0; i < count; ++i) {
            FutureID futureID = infos[i].future.id;

            // Check for cases that are undefined behavior in the API contract.
            DAWN_ASSERT(futureID != 0);
            DAWN_ASSERT(futureID < firstInvalidFutureID);

            // Try to find the event, if we don't find it, we can assume that it has already been
            // completed.
            auto it = (*events)->find(futureID);
            if (it == (*events)->end()) {
                infos[i].completed = true;
                anyCompleted = true;
            } else {
                infos[i].completed = false;
                TrackedEvent* event = it->second.Get();
                futures.push_back(TrackedFutureWaitInfo{futureID, event, i, false});
            }
        }
    });
    // If any completed, return immediately.
    if (anyCompleted) {
        return wgpu::WaitStatus::Success;
    }
    // Otherwise, we should have successfully looked up all of them.
    DAWN_ASSERT(futures.size() == count);

    wgpu::WaitStatus waitStatus = WaitImpl(mInstance, futures, timeout);
    if (waitStatus != wgpu::WaitStatus::Success) {
        return waitStatus;
    }

    // Enforce callback ordering
    auto readyEnd = PrepareReadyCallbacks(futures);

    // Call callbacks and update return values.
    for (auto it = futures.begin(); it != readyEnd; ++it) {
        // Set completed before calling the callback.
        infos[it->indexInInfos].completed = true;
        it->event->EnsureComplete(EventCompletionType::Ready);
    }

    // Since we use the presence of the event to indicate whether the callback has already been
    // called in WaitAny when searching for the matching FutureID, untrack the event after calling
    // the callbacks to ensure that we can't race on two different threads waiting on the same
    // future. Note that only one thread will actually call the callback since EnsureComplete is
    // thread safe.
    mEvents.Use([&](auto events) {
        for (auto it = futures.begin(); it != readyEnd; ++it) {
            (*events)->erase(it->futureID);
        }
    });

    return wgpu::WaitStatus::Success;
}

// QueueAndSerial

QueueAndSerial::QueueAndSerial(QueueBase* q, ExecutionSerial serial)
    : queue(GetWeakRef(q)), completionSerial(serial) {}

ExecutionSerial QueueAndSerial::GetCompletedSerial() const {
    if (auto q = queue.Promote()) {
        return q->GetCompletedCommandSerial();
    }
    return completionSerial;
}

// EventManager::TrackedEvent

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode,
                                         Ref<WaitListEvent> completionEvent)
    : mCallbackMode(callbackMode), mCompletionData(std::move(completionEvent)) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode,
                                         Ref<SystemEvent> completionEvent)
    : mCallbackMode(callbackMode), mCompletionData(std::move(completionEvent)) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode,
                                         QueueBase* queue,
                                         ExecutionSerial completionSerial)
    : mCallbackMode(callbackMode),
      mCompletionData(std::in_place_type_t<QueueAndSerial>(), queue, completionSerial) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode, Completed tag)
    : mCallbackMode(callbackMode), mCompletionData(AcquireRef(new WaitListEvent())) {
    GetIfWaitListEvent()->Signal();
}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode, NonProgressing tag)
    : mCallbackMode(callbackMode),
      mCompletionData(AcquireRef(new WaitListEvent())),
      mIsProgressing(false) {}

EventManager::TrackedEvent::~TrackedEvent() {
    DAWN_ASSERT(mFutureID != kNullFutureID);
    DAWN_ASSERT(mCompleted.Use([](auto completed) { return *completed; }));
}

Future EventManager::TrackedEvent::GetFuture() const {
    return {mFutureID};
}

bool EventManager::TrackedEvent::IsReadyToComplete() const {
    bool isReady = false;
    if (auto event = GetIfSystemEvent()) {
        isReady = event->IsSignaled();
    }
    if (auto event = GetIfWaitListEvent()) {
        isReady = event->IsSignaled();
    }
    if (const auto* queueAndSerial = GetIfQueueAndSerial()) {
        isReady = queueAndSerial->completionSerial <= queueAndSerial->GetCompletedSerial();
    }
    return isReady;
}

void EventManager::TrackedEvent::SetReadyToComplete() {
    if (auto event = GetIfSystemEvent()) {
        event->Signal();
    }
    if (auto event = GetIfWaitListEvent()) {
        event->Signal();
    }
    if (auto* queueAndSerial = GetIfQueueAndSerial()) {
        ExecutionSerial current = queueAndSerial->completionSerial.load(std::memory_order_acquire);
        for (auto completed = queueAndSerial->GetCompletedSerial();
             current > completed && !queueAndSerial->completionSerial.compare_exchange_weak(
                                        current, completed, std::memory_order_acq_rel);) {
        }
    }
}

void EventManager::TrackedEvent::EnsureComplete(EventCompletionType completionType) {
    mCompleted.Use([&](auto completed) {
        if (!*completed) {
            Complete(completionType);
            *completed = true;
        }
    });
}

}  // namespace dawn::native
