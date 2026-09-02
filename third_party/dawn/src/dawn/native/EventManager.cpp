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

#include "src/dawn/native/EventManager.h"

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/cleanup/cleanup.h"
#include "absl/strings/str_format.h"
#include "src/dawn/common/Atomic.h"
#include "src/dawn/common/FutureUtils.h"
#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/Instance.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/Queue.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/log.h"
#include "src/utils/non_movable.h"

namespace dawn::native {
namespace {

// Wait/poll queues with given `timeout`. `queueWaitSerials` should contain per queue, the serial up
// to which we should flush the queue if needed. Note that keys are WeakRef<QueueBase> which
// actually means the keys are not based on the QueueBase pointer, but a pointer to metadata that is
// guaranteed to be unique and alive. This ensures that each queue will be represented for multi
// source validation.
struct QueueWaitSerialsMaps {
    // A map from the queue to strong ref of the queue if possible. This avoids promoting
    // (acquire/releasing locks) for the same queue multiple times.
    absl::flat_hash_map<WeakRef<QueueBase>, Ref<QueueBase>> strongRefs;
    absl::flat_hash_map<Ref<QueueBase>, ExecutionSerial> minWaitSerials;
};

// Helper that updates the serial maps with the new `queueAndSerial`.
void UpdateQueueWaitSerialsMap(QueueWaitSerialsMaps& queueWaitSerialMaps,
                               const QueueAndSerial* queueAndSerial) {
    DAWN_ASSERT(queueAndSerial);

    auto refIt = queueWaitSerialMaps.strongRefs.find(queueAndSerial->queue);
    bool refInserted = false;
    if (refIt == queueWaitSerialMaps.strongRefs.end()) {
        std::tie(refIt, refInserted) = queueWaitSerialMaps.strongRefs.insert(
            {queueAndSerial->queue, queueAndSerial->queue.Promote()});
        DAWN_ASSERT(refInserted);
    }
    auto queue = refIt->second;

    // If the queue failed promotion, then it means that the work is all already done, so we don't
    // need to do anything here.
    if (queue == nullptr) {
        return;
    }

    // Otherwise, track the queue and the minimum wait serial to satisfy the wait condition.
    ExecutionSerial completionSerial =
        queueAndSerial->completionSerial.load(std::memory_order_acquire);
    auto [serialIt, serialInserted] =
        queueWaitSerialMaps.minWaitSerials.insert({queue, completionSerial});
    if (!serialInserted) {
        serialIt->second = std::min(serialIt->second, completionSerial);
    }
}

void WaitQueueSerials(const QueueWaitSerialsMaps& queueWaitSerialMaps, Nanoseconds timeout) {
    // Poll/wait on queues up to the lowest wait serial, but do this once per queue instead of
    // per event so that events with same serial complete at the same time instead of racing.
    for (const auto& [queue, waitSerial] : queueWaitSerialMaps.minWaitSerials) {
        [[maybe_unused]] bool hadError = queue->GetDevice()->ConsumedError(
            queue->WaitForQueueSerial(waitSerial, timeout), "waiting for work in %s.", queue.Get());
    }
}

struct AlreadyCompletedEvent final : public EventManager::TrackedEvent {
    explicit AlreadyCompletedEvent(wgpu::CallbackMode callbackMode)
        : TrackedEvent(callbackMode, TrackedEvent::Completed{}) {}
    ~AlreadyCompletedEvent() override { EnsureComplete(EventCompletionType::Shutdown); }
    void Complete(EventCompletionType) override {}
};

}  // namespace

class EventManager::Waiter : public NonMovable {
  public:
    void Signal() {
        return mSignaled.Use([](auto signaled) { *signaled = true; });
    }
    bool Wait(Nanoseconds timeout) {
        return mSignaled.Use([&](auto signaled) {
            return signaled.WaitFor(timeout, [](const bool& isSignaled) { return isSignaled; });
        });
    }

  private:
    MutexCondVarProtected<bool> mSignaled = false;
};

// EventManager

EventManager::EventManager(InstanceBase* instance) : mInstance(instance) {}

EventManager::~EventManager() {
    mEventState.Use([&](auto state) {
        // If the event manager is being destroyed, it shouldn't be possible to have waits in
        // flight.
        DAWN_CHECK(state->waiters.empty());

        // For all non-spontaneous events, call their callbacks now.
        for (auto& [futureID, event] : state->events) {
            if (event->mCallbackMode != wgpu::CallbackMode::AllowSpontaneous) {
                event->EnsureComplete(EventCompletionType::Shutdown);
            }
        }
    });
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
            q->TrackSerialTask(QueuePriority::UserVisible, queueAndSerial->completionSerial,
                               [this, event]() {
                                   // If this is executed, we can be sure that the raw pointer
                                   // to this EventManager is valid because the task is ran by
                                   // the Queue and:
                                   //   Queue -[refs]->
                                   //     Device -[refs]->
                                   //       Adapter -[refs]->
                                   //         Instance -[owns]->
                                   //           EventManager.
                                   SetFutureReady(event.Get());
                               });
        }
    }

    mEventState.Use([&](auto state) {
        if (event->mCallbackMode != wgpu::CallbackMode::WaitAnyOnly) {
            FetchMax(mLastProcessEventID, futureID);
        }
        state->events.emplace(futureID, std::move(event));
    });
    return futureID;
}

void EventManager::SetFutureReady(Ref<TrackedEvent> event) {
    event->SetReadyToComplete();

    // Sometimes, events might become ready before they are even tracked. This can happen
    // because tracking is ordered to uphold callback ordering, but events may become ready in
    // any order. If the event is spontaneous, it will be completed when it is tracked.
    if (event->mFutureID == kNullFutureID) {
        return;
    }

    // Signal all relevant waiters that the event has become ready.
    mEventState.Use([&](auto state) {
        if (auto node = state->waiters.extract(event->mFutureID)) {
            for (Waiter* waiter : node.mapped()) {
                waiter->Signal();
            }
        }
    });

    // Handle spontaneous completion now.
    if (event->mCallbackMode == wgpu::CallbackMode::AllowSpontaneous) {
        // Since we use the presence of the event to indicate whether the callback has already
        // been called in WaitAny when searching for the matching FutureID, untrack the event
        // after calling the callbacks to ensure that we can't race on two different threads
        // waiting on the same future. Note that only one thread will actually call the callback
        // since EnsureComplete is thread safe.
        event->EnsureComplete(EventCompletionType::Ready);
        mEventState.Use([&](auto state) { state->events.erase(event->mFutureID); });
    }
}

bool EventManager::ProcessPollEvents() {
    QueueWaitSerialsMaps queueWaitSerialMaps;
    SortedEventMap readyEvents;
    bool hasProgressingEvents = false;
    FutureID lastProcessEventID;

    mEventState.Use([&](auto state) {
        // Iterate all events and record poll events and spontaneous events since they are both
        // allowed to be completed in the ProcessPoll call. Note that spontaneous events are
        // allowed to trigger anywhere which is why we include them in the call.
        lastProcessEventID = mLastProcessEventID.load(std::memory_order_acquire);
        for (const auto& [futureID, event] : state->events) {
            if (event->mCallbackMode == wgpu::CallbackMode::WaitAnyOnly) {
                continue;
            }

            if (event->IsReadyToComplete()) {
                readyEvents.emplace(futureID, event);
                continue;
            }
            if (event->IsProgressing()) {
                hasProgressingEvents = true;
            }

            // Record queue's and their min serial to force a submit if applicable.
            if (const auto* queueAndSerial = event->GetIfQueueAndSerial()) {
                UpdateQueueWaitSerialsMap(queueWaitSerialMaps, queueAndSerial);
            }
        }
    });

    // This call is a no-op if `queueWaitSerialMaps` is empty, otherwise, it ensures that the
    // lowest serial work is submitted on each queue.
    WaitQueueSerials(queueWaitSerialMaps, Nanoseconds(0u));

    // Complete the events that are completable.
    for (auto& [_, event] : readyEvents) {
        if (event) {
            event->EnsureComplete(EventCompletionType::Ready);
        }
    }

    // Since we use the presence of the event to indicate whether the callback has already been
    // called in WaitAny when searching for the matching FutureID, untrack the event after
    // calling the callbacks to ensure that we can't race on two different threads waiting on
    // the same future. Note that only one thread will actually call the callback since
    // EnsureComplete is thread safe.
    mEventState.Use([&](auto state) {
        for (auto& [futureID, _] : readyEvents) {
            state->events.erase(futureID);
        }
    });

    return hasProgressingEvents ||
           (lastProcessEventID != mLastProcessEventID.load(std::memory_order_acquire));
}

wgpu::WaitStatus EventManager::WaitAny(Span<FutureWaitInfo> infos, Nanoseconds timeout) {
    bool foundNonQueueEvent = false;
    QueueWaitSerialsMaps queueWaitSerialMaps;
    SortedEventMap readyEvents;

    auto PreProcessWaits = [&](Waiter* waiter) {
        FutureID firstInvalidFutureID = mNextFutureID;
        mEventState.Use([&](auto state) {
            for (auto& info : infos) {
                FutureID futureID = info.future.id;
                info.completed = false;

                // Check for cases that are undefined behavior in the API contract.
                DAWN_CHECK(futureID != 0);
                DAWN_CHECK(futureID < firstInvalidFutureID);

                // Try to find the event, if we don't find it, we can assume that it has already
                // been completed so we can signal the waiter here.
                auto eventIt = state->events.find(futureID);
                if (eventIt == state->events.end()) {
                    if (waiter) {
                        waiter->Signal();
                    }
                    continue;
                }

                // Otherwise, we want to add the waiter for it.
                if (waiter) {
                    state->waiters[futureID].push_back(waiter);
                }

                // We also handle coalescing the queue events per queue on the lowest serial,
                // and tracking the different types of events for multi-source validation later.
                TrackedEvent* event = eventIt->second.Get();
                if (event->IsReadyToComplete() && waiter) {
                    waiter->Signal();
                }
                if (const auto* queueAndSerial = event->GetIfQueueAndSerial()) {
                    UpdateQueueWaitSerialsMap(queueWaitSerialMaps, queueAndSerial);
                } else {
                    foundNonQueueEvent = true;
                }
            }
        });
    };

    auto PostProcessWaits = [&](bool shouldComplete, Waiter* waiter) {
        mEventState.Use([&](auto state) {
            for (auto& info : infos) {
                FutureID futureID = info.future.id;

                if (shouldComplete) {
                    auto eventIt = state->events.find(futureID);
                    Ref<TrackedEvent> event =
                        (eventIt != state->events.end()) ? eventIt->second : nullptr;
                    if (!event || event->IsReadyToComplete()) {
                        info.completed = true;
                        readyEvents.emplace(futureID, std::move(event));
                    }
                }

                // Remove the waiter if relevant.
                if (waiter) {
                    std::erase(state->waiters[futureID], waiter);
                    if (state->waiters[futureID].empty()) {
                        state->waiters.erase(futureID);
                    }
                }
            }
        });
    };

    if (timeout == Nanoseconds(0u)) {
        PreProcessWaits(/*waiter=*/nullptr);
        WaitQueueSerials(queueWaitSerialMaps, Nanoseconds(0u));
        PostProcessWaits(/*shouldComplete=*/true, /*waiter=*/nullptr);
    } else {
        // Add the waiter to each of the events and initialize a cleanup routine to ensure that we
        // always remove the waiter at the end of this scope, optionally completing events if
        // `shouldComplete` is set to true.
        Waiter waiter;
        PreProcessWaits(&waiter);
        bool shouldComplete = true;
        absl::Cleanup removeWaiter = [&]() { PostProcessWaits(shouldComplete, &waiter); };

        // There's currently a couple of different cases to handle here depending on the
        // capabilities of the backends. Each case is documented in their respective clause below.
        if (queueWaitSerialMaps.minWaitSerials.empty()) {
            // When `queueWaitSerialMaps` is empty there are no queue events so all events are
            // waitable via the waiter.
            shouldComplete = waiter.Wait(timeout);
        } else if (queueWaitSerialMaps.minWaitSerials.size() == 1 && !foundNonQueueEvent) {
            // When there is exactly 1 queue in `queueWaitSerialMaps` and there are no other
            // non-queue events, we can just rely on `WaitQueueSerials` to handle a timed wait on
            // that queue.
            WaitQueueSerials(queueWaitSerialMaps, timeout);
        } else {
            // Otherwise, we either have multiple queues and/or a mix of queue and non-queue events.
            // We first verify the device capabilities across all the events before trying to
            // process them.
            for (const auto& [queue, waitSerial] : queueWaitSerialMaps.minWaitSerials) {
                if (!queue->GetDevice()->IsToggleEnabled(Toggle::SpontaneousQueueEvents)) {
                    mInstance->EmitLog(
                        WGPULoggingType_Error,
                        "Mixed source waits with timeouts is not supported for the set of events.");
                    shouldComplete = false;
                    return wgpu::WaitStatus::Error;
                }
            }

            // For queue events, we still need to ensure that the `waitSerial` has been
            // submitted via a call to `WaitForQueueSerial` with a timeout=0.
            WaitQueueSerials(queueWaitSerialMaps, Nanoseconds(0u));

            // Now that we've handled the queue events, we can wait on the events via the waiter.
            shouldComplete = waiter.Wait(timeout);
        }
    }

    // Complete the events that are completable.
    for (auto& [_, event] : readyEvents) {
        if (event) {
            event->EnsureComplete(EventCompletionType::Ready);
        }
    }

    // Since we use the presence of the event to indicate whether the callback has already been
    // called in WaitAny when searching for the matching FutureID, untrack the event after
    // calling the callbacks to ensure that we can't race on two different threads waiting on
    // the same future. Note that only one thread will actually call the callback since
    // EnsureComplete is thread safe.
    mEventState.Use([&](auto state) {
        for (auto& [futureID, _] : readyEvents) {
            state->events.erase(futureID);
        }
    });

    return !readyEvents.empty() ? wgpu::WaitStatus::Success : wgpu::WaitStatus::TimedOut;
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

Ref<EventManager::TrackedEvent> EventManager::TrackedEvent::CreateAlreadyCompletedEvent(
    EventManager* eventManager,
    wgpu::CallbackMode callbackMode) {
    Ref<TrackedEvent> event = AcquireRef(new AlreadyCompletedEvent(callbackMode));
    eventManager->TrackEvent(Ref<TrackedEvent>(event));
    return event;
}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode, bool readyToComplete)
    : mCallbackMode(callbackMode), mIsReadyToComplete(readyToComplete) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode,
                                         QueueBase* queue,
                                         ExecutionSerial completionSerial)
    : mCallbackMode(callbackMode), mQueueAndSerial(std::in_place, queue, completionSerial) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode, Completed tag)
    : mCallbackMode(callbackMode), mIsReadyToComplete(true) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode, NonProgressing tag)
    : mCallbackMode(callbackMode), mIsProgressing(false) {}

EventManager::TrackedEvent::~TrackedEvent() {
    DAWN_CHECK(mFutureID != kNullFutureID);
#if defined(DAWN_ENABLE_ASSERTS)
    std::call_once(mFlag, []() { DAWN_ASSERT(false); });
#endif
}

Future EventManager::TrackedEvent::GetFuture() const {
    return {mFutureID};
}

QueueAndSerial* EventManager::TrackedEvent::GetIfQueueAndSerial() {
    return mQueueAndSerial ? &*mQueueAndSerial : nullptr;
}

const QueueAndSerial* EventManager::TrackedEvent::GetIfQueueAndSerial() const {
    return mQueueAndSerial ? &*mQueueAndSerial : nullptr;
}

bool EventManager::TrackedEvent::IsReadyToComplete() const {
    // Currently there are only two types of events, queue events and simple state events.
    if (mIsReadyToComplete) {
        return true;
    }
    if (const auto* queueAndSerial = GetIfQueueAndSerial()) {
        return queueAndSerial->completionSerial <= queueAndSerial->GetCompletedSerial();
    }

    return false;
}

void EventManager::TrackedEvent::SetReadyToComplete() {
    mIsReadyToComplete = true;
}

void EventManager::TrackedEvent::EnsureComplete(EventCompletionType completionType) {
    std::call_once(mFlag, [&]() { Complete(completionType); });
}

}  // namespace dawn::native
