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

#ifndef SRC_DAWN_NATIVE_WAITLISTEVENT_H_
#define SRC_DAWN_NATIVE_WAITLISTEVENT_H_

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <utility>
#include <vector>

#include "dawn/common/RefCounted.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/SystemEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

class WaitListEvent : public RefCounted {
  public:
    WaitListEvent();

    bool IsSignaled() const;
    void Signal();
    bool Wait(Nanoseconds timeout);
    SystemEventReceiver WaitAsync();

    template <typename It>
    static bool WaitAny(It eventAndReadyStateBegin, It eventAndReadyStateEnd, Nanoseconds timeout);

  private:
    ~WaitListEvent() override;

    struct SyncWaiter {
        std::condition_variable cv;
        std::mutex mutex;
        bool waitDone = false;
    };

    mutable std::mutex mMutex;
    std::atomic_bool mSignaled{false};
    std::vector<raw_ptr<SyncWaiter>> mSyncWaiters;
    std::vector<SystemEventPipeSender> mAsyncWaiters;
};

template <typename It>
bool WaitListEvent::WaitAny(It eventAndReadyStateBegin,
                            It eventAndReadyStateEnd,
                            Nanoseconds timeout) {
    static_assert(std::is_base_of_v<std::random_access_iterator_tag,
                                    typename std::iterator_traits<It>::iterator_category>);
    static_assert(std::is_same_v<typename std::iterator_traits<It>::value_type,
                                 std::pair<Ref<WaitListEvent>, bool*>>);

    const size_t count = std::distance(eventAndReadyStateBegin, eventAndReadyStateEnd);
    if (count == 0) {
        return false;
    }

    struct EventState {
        WaitListEvent* event = nullptr;
        size_t origIndex;
        bool isReady = false;
    };
    std::vector<EventState> events(count);

    for (size_t i = 0; i < count; i++) {
        const auto& event = (*(eventAndReadyStateBegin + i)).first;
        events[i].event = event.Get();
        events[i].origIndex = i;
    }
    // Sort the events by address to get a globally consistent order.
    std::sort(events.begin(), events.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.event < rhs.event; });

    // Acquire locks in order and enqueue our waiter.
    bool foundSignaled = false;
    for (size_t i = 0; i < count; i++) {
        WaitListEvent* event = events[i].event;
        // Skip over multiple waits on the same event, but ensure that we store the same ready state
        // for duplicates.
        if (i > 0 && event == events[i - 1].event) {
            events[i].isReady = events[i - 1].isReady;
            continue;
        }
        event->mMutex.lock();
        // Check `IsSignaled()` after acquiring the lock so that it doesn't become true immediately
        // before we acquire the lock - we assume that it is safe to enqueue our waiter after this
        // point if the event is not already signaled.
        if (event->IsSignaled()) {
            events[i].isReady = true;
            foundSignaled = true;
        }
    }

    // If any of the events were already signaled, early out after unlocking the events in reverse
    // order - note that unlocking in reverse order is not strictly needed for avoiding deadlocks.
    if (foundSignaled) {
        for (size_t i = 0; i < count; i++) {
            WaitListEvent* event = events[count - 1 - i].event;
            // Use the cached value of `IsSignaled()` because we might have unlocked the event
            // already if it was a duplicate and checking `IsSignaled()` without the lock is racy
            // and can cause different values of isReady for multiple waits on the same event.
            if (events[count - 1 - i].isReady) {
                bool* isReady =
                    (*(eventAndReadyStateBegin + events[count - 1 - i].origIndex)).second;
                *isReady = true;
            }
            // Skip over multiple waits on the same event.
            if (i > 0 && event == events[count - i].event) {
                continue;
            }
            event->mMutex.unlock();
        }
        return true;
    }

    // We have acquired locks for all the events we're going to wait on - enqueue our waiter now
    // after locking it since it could be woken up as soon as any of the events are unlocked. Unlock
    // the events in reverse order - note that this is not strictly needed for avoiding deadlocks.
    SyncWaiter waiter;
    std::unique_lock<std::mutex> waiterLock(waiter.mutex);
    for (size_t i = 0; i < count; i++) {
        WaitListEvent* event = events[count - 1 - i].event;
        // Skip over multiple waits on the same event.
        if (i > 0 && event == events[count - i].event) {
            continue;
        }
        event->mSyncWaiters.push_back(&waiter);
        event->mMutex.unlock();
    }

    // Any values larger than those representatable by std::chrono::nanoseconds will be treated as
    // infinite waits - in particular this covers values greater than INT64_MAX.
    static constexpr uint64_t kMaxDurationNanos = std::chrono::nanoseconds::max().count();
    [[maybe_unused]] bool waitDone = false;
    if (timeout > Nanoseconds(kMaxDurationNanos)) {
        waiter.cv.wait(waiterLock, [&waiter]() { return waiter.waitDone; });
        DAWN_ASSERT(waiter.waitDone);
        waitDone = true;
    } else {
        waitDone =
            waiter.cv.wait_for(waiterLock, std::chrono::nanoseconds(static_cast<uint64_t>(timeout)),
                               [&waiter]() { return waiter.waitDone; });
    }
    // Release the `waiterLock` so that TSAN doesn't complain about a benign lock order inversion
    // between `waiter.mutex` and `event.mMutex`.
    waiterLock.unlock();

    // Remove our waiter from the events.
    for (size_t i = 0; i < count; i++) {
        WaitListEvent* event = events[i].event;
        // Skip over multiple waits on the same event, but ensure that we store the same ready state
        // for duplicates.
        if (i > 0 && event == events[i - 1].event) {
            events[i].isReady = events[i - 1].isReady;
        } else {
            // We could be woken by the condition variable before the atomic release store to
            // `mSignaled` is visible - locking the mutex ensures that the atomic acquire load in
            // `IsSignaled()` sees the correct value.
            std::lock_guard<std::mutex> eventLock(event->mMutex);
            if (event->IsSignaled()) {
                events[i].isReady = true;
            }
            std::erase(event->mSyncWaiters, &waiter);
        }
        if (events[i].isReady) {
            bool* isReady = (*(eventAndReadyStateBegin + events[i].origIndex)).second;
            *isReady = true;
            foundSignaled = true;
        }
    }

    DAWN_ASSERT(!waitDone || foundSignaled);
    return foundSignaled;
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_WAITLISTEVENT_H_
