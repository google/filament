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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include "dawn/wire/client/EventManager.h"

#include <map>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/common/Time.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

// TrackedEvent

TrackedEvent::TrackedEvent(WGPUCallbackMode mode) : mMode(mode) {}

TrackedEvent::~TrackedEvent() {
    DAWN_ASSERT(mEventState == EventState::Complete);
}

WGPUCallbackMode TrackedEvent::GetCallbackMode() const {
    return mMode;
}

bool TrackedEvent::IsReady() const {
    return mEventState == EventState::Ready;
}

void TrackedEvent::SetReady() {
    DAWN_ASSERT(mEventState != EventState::Complete);
    mEventState = EventState::Ready;
}

void TrackedEvent::Complete(FutureID futureID, EventCompletionType type) {
    // If the callback is already in a running state, that means that the std::call_once below is
    // already running on some thread. Normally |Complete| is not called re-entrantly, however, in
    // the case when a callback may drop the last reference to the Instance, because we only remove
    // the Event from the tracking list after the callback is completed, the teardown of the
    // EventManager will cause us to re-iterate all events and call |Complete| again on this one
    // with EventCompletionType::Shutdown. In that case, we don't want to call the std::call_once
    // below because it would cause a self-deadlock. During shutdown, it's not as important that
    // |Complete| waits until the callback completes, and instead we rely on the destructor's ASSERT
    // to ensure the invariant.
    if (type == EventCompletionType::Shutdown && mEventState == EventState::Running) {
        return;
    }

    std::call_once(mFlag, [&]() {
        mEventState = EventState::Running;
        CompleteImpl(futureID, type);
        mEventState = EventState::Complete;
    });
}

// EventManager

EventManager::EventManager(size_t timedWaitAnyMaxCount)
    : mTimedWaitAnyMaxCount(timedWaitAnyMaxCount) {}

EventManager::~EventManager() {
    TransitionTo(State::ClientDropped);
}

std::pair<FutureID, bool> EventManager::TrackEvent(Ref<TrackedEvent>&& event) {
    if (!ValidateCallbackMode(event->GetCallbackMode())) {
        dawn::ErrorLog() << "Invalid callback mode: " << event->GetCallbackMode();
        return {kNullFutureID, false};
    }

    FutureID futureID = mNextFutureID++;

    switch (mState) {
        case State::InstanceDropped: {
            if (event->GetCallbackMode() != WGPUCallbackMode_AllowSpontaneous) {
                event->Complete(futureID, EventCompletionType::Shutdown);
                return {futureID, false};
            }
            break;
        }
        case State::ClientDropped: {
            event->Complete(futureID, EventCompletionType::Shutdown);
            return {futureID, false};
        }
        case State::Nominal:
            break;
    }

    mTrackedEvents.Use([&](auto trackedEvents) {
        auto [it, inserted] = trackedEvents->emplace(futureID, std::move(event));
        DAWN_ASSERT(inserted);
    });

    return {futureID, true};
}

void EventManager::TransitionTo(EventManager::State state) {
    // If the client is disconnected, this becomes a no-op.
    if (mState == State::ClientDropped) {
        return;
    }

    // Only forward state transitions are allowed.
    DAWN_ASSERT(state > mState);
    mState = state;

    while (true) {
        EventMap events;
        switch (state) {
            case State::InstanceDropped: {
                mTrackedEvents.Use([&](auto trackedEvents) {
                    for (auto it = trackedEvents->begin(); it != trackedEvents->end();) {
                        auto& event = it->second;
                        if (event->GetCallbackMode() != WGPUCallbackMode_AllowSpontaneous) {
                            events.emplace(it->first, std::move(event));
                            it = trackedEvents->erase(it);
                        } else {
                            ++it;
                        }
                    }
                });
                break;
            }
            case State::ClientDropped: {
                mTrackedEvents.Use([&](auto trackedEvents) { events = std::move(*trackedEvents); });
                break;
            }
            case State::Nominal:
                // We always start in the nominal state so we should never be transitioning to it.
                DAWN_UNREACHABLE();
        }
        if (events.empty()) {
            break;
        }
        for (auto& [futureID, event] : events) {
            event->Complete(futureID, EventCompletionType::Shutdown);
        }
    }
}

void EventManager::ProcessPollEvents() {
    std::vector<std::pair<FutureID, Ref<TrackedEvent>>> eventsToComplete;
    mTrackedEvents.ConstUse([&](auto trackedEvents) {
        for (auto& [futureID, event] : *trackedEvents) {
            WGPUCallbackMode callbackMode = event->GetCallbackMode();
            if ((callbackMode == WGPUCallbackMode_AllowProcessEvents ||
                 callbackMode == WGPUCallbackMode_AllowSpontaneous) &&
                event->IsReady()) {
                eventsToComplete.emplace_back(futureID, event);
            }
        }
    });

    // Since events were initially stored and iterated from an ordered map, they must be ordered.
    for (auto& [futureID, event] : eventsToComplete) {
        event->Complete(futureID, EventCompletionType::Ready);
    }

    mTrackedEvents.Use([&](auto trackedEvents) {
        for (auto& [futureID, _] : eventsToComplete) {
            trackedEvents->erase(futureID);
        }
    });
}

namespace {
bool UpdateAnyCompletedOrReady(std::span<WGPUFutureWaitInfo> waitInfos,
                               const EventManager::EventMap& allEvents,
                               EventManager::EventMap* eventsToComplete) {
    DAWN_ASSERT(eventsToComplete->empty());

    bool anyCompleted = false;
    for (auto& waitInfo : waitInfos) {
        auto it = allEvents.find(waitInfo.future.id);
        if (it == allEvents.end()) {
            waitInfo.completed = true;
            anyCompleted = true;
            continue;
        }

        auto& event = it->second;
        if (event->IsReady()) {
            waitInfo.completed = true;
            anyCompleted = true;
            eventsToComplete->emplace(it->first, event);
        }
    }

    DAWN_ASSERT(eventsToComplete->empty() || anyCompleted);
    return anyCompleted;
}
}  // anonymous namespace

WGPUWaitStatus EventManager::WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS) {
    if (timeoutNS > 0) {
        if (mTimedWaitAnyMaxCount == 0) {
            dawn::ErrorLog() << "Instance only supports timed wait anys if "
                                "WGPUInstanceFeatureName_TimedWaitAny is enabled.";
            return WGPUWaitStatus_Error;
        }
        if (count > mTimedWaitAnyMaxCount) {
            dawn::ErrorLog() << "Instance only supports up to (" << mTimedWaitAnyMaxCount
                             << ") timed wait anys.";
            return WGPUWaitStatus_Error;
        }
    }

    if (count == 0) {
        return WGPUWaitStatus_Success;
    }

    // Since the user can specify the FutureIDs in any order, we need to use another ordered map
    // here to ensure that the result is ordered for JS event ordering.
    auto waitInfos = std::span(infos, count);
    EventMap eventsToComplete;
    bool anyCompleted = mTrackedEvents.ConstUse([&](auto trackedEvents) {
        if (UpdateAnyCompletedOrReady(waitInfos, *trackedEvents, &eventsToComplete)) {
            return true;
        }
        if (timeoutNS > 0) {
            return trackedEvents.WaitFor(Nanoseconds(timeoutNS), [&](const EventMap& events) {
                return UpdateAnyCompletedOrReady(waitInfos, events, &eventsToComplete);
            });
        }
        return false;
    });

    for (auto& [futureID, event] : eventsToComplete) {
        // .completed has already been set to true (before the callback, per API contract).
        event->Complete(futureID, EventCompletionType::Ready);
    }

    mTrackedEvents.Use([&](auto trackedEvents) {
        for (auto& [futureID, _] : eventsToComplete) {
            trackedEvents->erase(futureID);
        }
    });

    return anyCompleted ? WGPUWaitStatus_Success : WGPUWaitStatus_TimedOut;
}

}  // namespace dawn::wire::client
