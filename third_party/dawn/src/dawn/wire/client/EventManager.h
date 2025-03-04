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

#ifndef SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_
#define SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_

#include <webgpu/webgpu.h>

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/wire/WireResult.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {

class Client;

enum class EventType {
    CompilationInfo,
    CreateComputePipeline,
    CreateRenderPipeline,
    DeviceLost,
    MapAsync,
    PopErrorScope,
    RequestAdapter,
    RequestDevice,
    WorkDone,
};

// Implementations of TrackedEvents must implement the GetType, CompleteImpl, and ReadyHook
// functions. In most scenarios, the CompleteImpl function should call the callbacks while the
// ReadyHook should process and copy memory (if necessary) from the wire deserialization buffer
// into a local copy that can be readily used by the user callback. Specifically, the wire
// deserialization data is guaranteed to be alive when the ReadyHook is called, but not when
// CompleteImpl is called.
class TrackedEvent : NonMovable {
  public:
    explicit TrackedEvent(WGPUCallbackMode mode);
    virtual ~TrackedEvent();

    virtual EventType GetType() = 0;

    WGPUCallbackMode GetCallbackMode() const;
    bool IsReady() const;

    void SetReady();
    void Complete(FutureID futureID, EventCompletionType type);

  protected:
    virtual void CompleteImpl(FutureID futureID, EventCompletionType type) = 0;

    const WGPUCallbackMode mMode;
    enum class EventState {
        Pending,
        Ready,
        Complete,
    };
    EventState mEventState = EventState::Pending;
};

// Subcomponent which tracks callback events for the Future-based callback
// entrypoints. All events from this instance (regardless of whether from an adapter, device, queue,
// etc.) are tracked here, and used by the instance-wide ProcessEvents and WaitAny entrypoints.
//
// TODO(crbug.com/dawn/2060): This should probably be merged together with RequestTracker.
class EventManager final : NonMovable {
  public:
    ~EventManager();

    // See mState for breakdown of these states.
    enum class State { Nominal, InstanceDropped, ClientDropped };

    // Returns a pair of the FutureID and a bool that is true iff the event was successfuly tracked,
    // false otherwise. Events may not be tracked if the client is already disconnected.
    std::pair<FutureID, bool> TrackEvent(std::unique_ptr<TrackedEvent> event);

    // Transitions the EventManager to the given state. Note that states can only go in one
    // direction, i.e. once the EventManager transitions to InstanceDropped, it cannot transition
    // back to Nominal, though it may transition to ClientDropped later on.
    void TransitionTo(State state);

    template <typename Event, typename... ReadyArgs>
    WireResult SetFutureReady(FutureID futureID, ReadyArgs&&... readyArgs) {
        DAWN_ASSERT(futureID > 0);

        // If the future id is greater than what we have assigned, it must be invalid.
        if (futureID > mNextFutureID) {
            return WireResult::FatalError;
        }

        std::unique_ptr<TrackedEvent> spontaneousEvent;
        WireResult result = mTrackedEvents.Use([&](auto trackedEvents) {
            auto it = trackedEvents->find(futureID);
            if (it == trackedEvents->end()) {
                // If the future is not found, it must've already been completed.
                return WireResult::Success;
            }
            auto& trackedEvent = it->second;

            if (trackedEvent->GetType() != Event::kType) {
                // Assert here for debugging, before returning a fatal error that is handled upwards
                // in production.
                DAWN_ASSERT(trackedEvent->GetType() == Event::kType);
                return WireResult::FatalError;
            }

            WireResult result = static_cast<Event*>(trackedEvent.get())
                                    ->ReadyHook(futureID, std::forward<ReadyArgs>(readyArgs)...);
            trackedEvent->SetReady();

            // If the event can be spontaneously completed, prepare to do so now.
            if (trackedEvent->GetCallbackMode() == WGPUCallbackMode_AllowSpontaneous) {
                spontaneousEvent = std::move(trackedEvent);
                trackedEvents->erase(futureID);
            }
            return result;
        });

        // Handle spontaneous completions.
        if (spontaneousEvent) {
            spontaneousEvent->Complete(futureID, EventCompletionType::Ready);
        }
        return result;
    }

    void ProcessPollEvents();
    WGPUWaitStatus WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS);

  private:
    // Different states of the EventManager dictate how new incoming events are handled.
    //   Nominal: Usual state of the manager. All events are tracked and callbacks are fired
    //     depending on the callback modes.
    //   InstanceDropped: Transitioned to this state if the last external reference of the Instance
    //     is dropped. In this mode, any non-spontaneous events are no longer tracked and their
    //     callbacks are immediately called since the user cannot call WaitAny or ProcessEvents
    //     anymore. Any existing non-spontaneous events' callbacks are also called on transition.
    //   ClientDropped: Transitioned to this state once the client is dropped. In this mode, no new
    //     events are tracked and callbacks are all immediately fired. Any existing tracked events'
    //     callbacks are also called on transition.
    State mState = State::Nominal;

    // Tracks all kinds of events (for both WaitAny and ProcessEvents). We use an ordered map so
    // that in most cases, event ordering is already implicit when we iterate the map. (Not true for
    // WaitAny though because the user could specify the FutureIDs out of order.)
    MutexProtected<std::map<FutureID, std::unique_ptr<TrackedEvent>>> mTrackedEvents;
    std::atomic<FutureID> mNextFutureID = 1;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_
