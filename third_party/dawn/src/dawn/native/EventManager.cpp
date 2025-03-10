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
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "dawn/common/Assert.h"
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

// Wrapper around an iterator to yield system event receiver and a pointer
// to the ready bool. We pass this into WaitAnySystemEvent so it can extract
// the receivers and get pointers to the ready status - without allocating
// duplicate storage to store the receivers and ready bools.
class SystemEventAndReadyStateIterator {
  public:
    using WrappedIter = std::vector<TrackedFutureWaitInfo>::iterator;

    // Specify required iterator traits.
    using value_type = std::pair<const SystemEventReceiver&, bool*>;
    using difference_type = typename WrappedIter::difference_type;
    using iterator_category = typename WrappedIter::iterator_category;
    using pointer = value_type*;
    using reference = value_type&;

    SystemEventAndReadyStateIterator() = default;
    SystemEventAndReadyStateIterator(const SystemEventAndReadyStateIterator&) = default;
    SystemEventAndReadyStateIterator& operator=(const SystemEventAndReadyStateIterator&) = default;

    explicit SystemEventAndReadyStateIterator(WrappedIter wrappedIt) : mWrappedIt(wrappedIt) {}

    bool operator!=(const SystemEventAndReadyStateIterator& rhs) const {
        return rhs.mWrappedIt != mWrappedIt;
    }
    bool operator==(const SystemEventAndReadyStateIterator& rhs) const {
        return rhs.mWrappedIt == mWrappedIt;
    }
    difference_type operator-(const SystemEventAndReadyStateIterator& rhs) const {
        return mWrappedIt - rhs.mWrappedIt;
    }

    SystemEventAndReadyStateIterator operator+(difference_type rhs) const {
        return SystemEventAndReadyStateIterator{mWrappedIt + rhs};
    }

    SystemEventAndReadyStateIterator& operator++() {
        ++mWrappedIt;
        return *this;
    }

    value_type operator*() {
        return {
            std::get<Ref<SystemEvent>>(mWrappedIt->event->GetCompletionData())
                ->GetOrCreateSystemEventReceiver(),
            &mWrappedIt->ready,
        };
    }

  private:
    WrappedIter mWrappedIt;
};

// Wait/poll the queue for futures in range [begin, end). `waitSerial` should be
// the serial after which at least one future should be complete. All futures must
// have completion data of type QueueAndSerial.
// Returns true if at least one future is ready. If no futures are ready or the wait
// timed out, returns false.
bool WaitQueueSerialsImpl(DeviceBase* device,
                          QueueBase* queue,
                          ExecutionSerial waitSerial,
                          std::vector<TrackedFutureWaitInfo>::iterator begin,
                          std::vector<TrackedFutureWaitInfo>::iterator end,
                          Nanoseconds timeout) {
    bool success = false;
    // TODO(dawn:1662): Make error handling thread-safe.
    auto deviceLock(device->GetScopedLock());
    if (device->ConsumedError([&]() -> MaybeError {
            if (waitSerial > queue->GetLastSubmittedCommandSerial()) {
                // Serial has not been submitted yet. Submit it now.
                // TODO(dawn:1413): This doesn't need to be a full tick. It just needs to
                // flush work up to `waitSerial`. This should be done after the
                // ExecutionQueue / ExecutionContext refactor.
                queue->ForceEventualFlushOfCommands();
                DAWN_TRY(device->Tick());
            }
            // Check the completed serial.
            ExecutionSerial completedSerial = queue->GetCompletedCommandSerial();
            if (completedSerial < waitSerial) {
                if (timeout > Nanoseconds(0)) {
                    // Wait on the serial if it hasn't passed yet.
                    DAWN_TRY_ASSIGN(success, queue->WaitForQueueSerial(waitSerial, timeout));
                }
                // Update completed serials.
                DAWN_TRY(queue->CheckPassedSerials());
                completedSerial = queue->GetCompletedCommandSerial();
            }
            // Poll futures for completion.
            for (auto it = begin; it != end; ++it) {
                ExecutionSerial serial =
                    std::get<QueueAndSerial>(it->event->GetCompletionData()).completionSerial;
                if (serial <= completedSerial) {
                    success = true;
                    it->ready = true;
                }
            }
            return {};
        }())) {
        // There was an error. Pending submit may have failed or waiting for fences
        // may have lost the device. The device is lost inside ConsumedError.
        // Mark all futures as ready.
        for (auto it = begin; it != end; ++it) {
            it->ready = true;
        }
        success = true;
    }
    return success;
}

// We can replace the std::vector& when std::span is available via C++20.
wgpu::WaitStatus WaitImpl(const InstanceBase* instance,
                          std::vector<TrackedFutureWaitInfo>& futures,
                          Nanoseconds timeout) {
    auto begin = futures.begin();
    const auto end = futures.end();
    bool anySuccess = false;
    // The following loop will partition [begin, end) based on the type of wait is required.
    // After each partition, it will wait/poll on the first partition, then advance `begin`
    // to the start of the next partition. Note that for timeout > 0 and unsupported mixed
    // sources, we validate that there is a single partition. If there is only one, then the
    // loop runs only once and the timeout does not stack.
    while (begin != end) {
        const auto& first = begin->event->GetCompletionData();

        DeviceBase* waitDevice;
        ExecutionSerial lowestWaitSerial;
        if (std::holds_alternative<Ref<SystemEvent>>(first)) {
            waitDevice = nullptr;
        } else {
            const auto& queueAndSerial = std::get<QueueAndSerial>(first);
            waitDevice = queueAndSerial.queue->GetDevice();
            lowestWaitSerial = queueAndSerial.completionSerial;
        }
        // Partition the remaining futures based on whether they match the same completion
        // data type as the first. Also keep track of the lowest wait serial.
        const auto mid =
            std::partition(std::next(begin), end, [&](const TrackedFutureWaitInfo& info) {
                const auto& completionData = info.event->GetCompletionData();
                if (std::holds_alternative<Ref<SystemEvent>>(completionData)) {
                    return waitDevice == nullptr;
                } else {
                    const auto& queueAndSerial = std::get<QueueAndSerial>(completionData);
                    if (waitDevice == queueAndSerial.queue->GetDevice()) {
                        lowestWaitSerial =
                            std::min(lowestWaitSerial, queueAndSerial.completionSerial);
                        return true;
                    } else {
                        return false;
                    }
                }
            });

        // There's a mix of wait sources if partition yielded an iterator that is not at the end.
        if (mid != end) {
            if (timeout > Nanoseconds(0)) {
                // Multi-source wait is unsupported.
                // TODO(dawn:2062): Implement support for this when the device supports it.
                // It should eventually gather the lowest serial from the queue(s), transform them
                // into completion events, and wait on all of the events. Then for any queues that
                // saw a completion, poll all futures related to that queue for completion.
                instance->EmitLog(WGPULoggingType_Error,
                                  "Mixed source waits with timeouts are not currently supported.");
                return wgpu::WaitStatus::Error;
            }
        }

        bool success;
        if (waitDevice) {
            success = WaitQueueSerialsImpl(waitDevice, std::get<QueueAndSerial>(first).queue.Get(),
                                           lowestWaitSerial, begin, mid, timeout);
        } else {
            if (timeout > Nanoseconds(0)) {
                success = WaitAnySystemEvent(SystemEventAndReadyStateIterator{begin},
                                             SystemEventAndReadyStateIterator{mid}, timeout);
            } else {
                // Poll the completion events.
                success = false;
                for (auto it = begin; it != mid; ++it) {
                    if (std::get<Ref<SystemEvent>>(it->event->GetCompletionData())->IsSignaled()) {
                        it->ready = true;
                        success = true;
                    }
                }
            }
        }
        anySuccess |= success;

        // Advance the iterator to the next partition.
        begin = mid;
    }
    if (!anySuccess) {
        return wgpu::WaitStatus::TimedOut;
    }
    return wgpu::WaitStatus::Success;
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
        mTimedWaitAnyEnable = descriptor->capabilities.timedWaitAnyEnable;
        mTimedWaitAnyMaxCount =
            std::max(kTimedWaitAnyMaxCountDefault, descriptor->capabilities.timedWaitAnyMaxCount);
    }
    if (mTimedWaitAnyMaxCount > kTimedWaitAnyMaxCountDefault) {
        // We don't yet support a higher timedWaitAnyMaxCount because it would be complicated
        // to implement on Windows, and it isn't that useful to implement only on non-Windows.
        return DAWN_VALIDATION_ERROR("Requested timedWaitAnyMaxCount is not supported");
    }

    return {};
}

void EventManager::ShutDown() {
    mEvents.Use([&](auto events) { (*events).reset(); });
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
        bool isReady = false;
        auto completionData = event->GetCompletionData();
        if (std::holds_alternative<Ref<SystemEvent>>(completionData)) {
            isReady = std::get<Ref<SystemEvent>>(completionData)->IsSignaled();
        }
        if (std::holds_alternative<QueueAndSerial>(completionData)) {
            auto& queueAndSerial = std::get<QueueAndSerial>(completionData);
            isReady = queueAndSerial.completionSerial <=
                      queueAndSerial.queue->GetCompletedCommandSerial();
        }
        if (isReady) {
            event->EnsureComplete(EventCompletionType::Ready);
            return futureID;
        }
    }

    mEvents.Use([&](auto events) {
        if (!events->has_value()) {
            return;
        }
        if (event->mCallbackMode != wgpu::CallbackMode::WaitAnyOnly) {
            FutureID lastProcessedEventID = mLastProcessEventID.load(std::memory_order_acquire);
            while (lastProcessedEventID < futureID &&
                   !mLastProcessEventID.compare_exchange_weak(lastProcessedEventID, futureID,
                                                              std::memory_order_acq_rel)) {
            }
        }
        (*events)->emplace(futureID, std::move(event));
    });
    return futureID;
}

void EventManager::SetFutureReady(TrackedEvent* event) {
    auto completionData = event->GetCompletionData();
    if (std::holds_alternative<Ref<SystemEvent>>(completionData)) {
        std::get<Ref<SystemEvent>>(completionData)->Signal();
    }
    if (std::holds_alternative<QueueAndSerial>(completionData)) {
        auto& queueAndSerial = std::get<QueueAndSerial>(completionData);
        queueAndSerial.completionSerial = queueAndSerial.queue->GetCompletedCommandSerial();
    }

    // Sometimes, events might become ready before they are even tracked. This can happen because
    // tracking is ordered to uphold callback ordering, but events may become ready in any order. If
    // the event is spontaneous, it will be completed when it is tracked.
    if (event->mFutureID == kNullFutureID) {
        return;
    }

    // Handle spontaneous completion now.
    if (event->mCallbackMode == wgpu::CallbackMode::AllowSpontaneous) {
        mEvents.Use([&](auto events) {
            if (!events->has_value()) {
                return;
            }
            (*events)->erase(event->mFutureID);
        });
        event->EnsureComplete(EventCompletionType::Ready);
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
                auto completionData = event->GetCompletionData();
                if (std::holds_alternative<Ref<SystemEvent>>(completionData)) {
                    hasProgressingEvents |=
                        std::get<Ref<SystemEvent>>(completionData)->IsProgressing();
                } else {
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

    // For all the futures we are about to complete, first ensure they're untracked.
    mEvents.Use([&](auto events) {
        for (auto it = futures.begin(); it != readyEnd; ++it) {
            (*events)->erase(it->futureID);
        }
    });

    // Finally, call callbacks while comparing the last process event id with any new ones that may
    // have been created via the callbacks.
    for (auto it = futures.begin(); it != readyEnd; ++it) {
        it->event->EnsureComplete(EventCompletionType::Ready);
    }
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

            // Try to find the event.
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

    // For any futures that we're about to complete, first ensure they're untracked. It's OK if
    // something actually isn't tracked anymore (because it completed elsewhere while waiting.)
    mEvents.Use([&](auto events) {
        for (auto it = futures.begin(); it != readyEnd; ++it) {
            (*events)->erase(it->futureID);
        }
    });

    // Finally, call callbacks and update return values.
    for (auto it = futures.begin(); it != readyEnd; ++it) {
        // Set completed before calling the callback.
        infos[it->indexInInfos].completed = true;
        it->event->EnsureComplete(EventCompletionType::Ready);
    }

    return wgpu::WaitStatus::Success;
}

// EventManager::TrackedEvent

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode,
                                         Ref<SystemEvent> completionEvent)
    : mCallbackMode(callbackMode), mCompletionData(std::move(completionEvent)) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode,
                                         QueueBase* queue,
                                         ExecutionSerial completionSerial)
    : mCallbackMode(callbackMode), mCompletionData(QueueAndSerial{queue, completionSerial}) {}

EventManager::TrackedEvent::TrackedEvent(wgpu::CallbackMode callbackMode, Completed tag)
    : TrackedEvent(callbackMode, SystemEvent::CreateSignaled()) {}

EventManager::TrackedEvent::~TrackedEvent() {
    DAWN_ASSERT(mFutureID != kNullFutureID);
    DAWN_ASSERT(mCompleted);
}

Future EventManager::TrackedEvent::GetFuture() const {
    return {mFutureID};
}

const EventManager::TrackedEvent::CompletionData& EventManager::TrackedEvent::GetCompletionData()
    const {
    return mCompletionData;
}

void EventManager::TrackedEvent::EnsureComplete(EventCompletionType completionType) {
    bool alreadyComplete = mCompleted.exchange(true);
    if (!alreadyComplete) {
        Complete(completionType);
    }
}

}  // namespace dawn::native
