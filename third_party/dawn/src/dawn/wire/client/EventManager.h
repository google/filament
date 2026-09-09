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

#include <atomic>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "dawn/wire/client/wgpu_structs_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/FutureUtils.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/RefCounted.h"
#include "src/dawn/wire/WireResult.h"
#include "src/utils/non_movable.h"
#include "src/utils/span.h"

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
class TrackedEvent : public RefCounted {
  public:
    explicit TrackedEvent(WGPUCallbackMode mode);
    ~TrackedEvent() override;

    virtual EventType GetType() = 0;

    WGPUCallbackMode GetCallbackMode() const;

    // Returns true iff the event is not |Pending|.
    bool IsReady() const;

    void SetReady();
    void Complete(FutureID futureID, EventCompletionType type);

  protected:
    virtual void CompleteImpl(FutureID futureID, EventCompletionType type) = 0;

    const WGPUCallbackMode mMode;
    enum class EventState {
        Pending,
        Ready,
        Running,
        Complete,
    };
    std::atomic<EventState> mEventState = EventState::Pending;
    std::once_flag mFlag;
};

// Subcomponent which tracks callback events for the Future-based callback
// entrypoints. All events from this instance (regardless of whether from an adapter, device, queue,
// etc.) are tracked here, and used by the instance-wide ProcessEvents and WaitAny entrypoints.
class EventManager final : NonMovable {
  public:
    using EventMap = std::map<FutureID, Ref<TrackedEvent>>;

    explicit EventManager(size_t timedWaitAnyMaxCount);
    ~EventManager();

    // Returns a pair of the FutureID and a bool that is true iff the event was successfuly tracked,
    // false otherwise. Events may not be tracked if the client is already disconnected.
    std::pair<FutureID, bool> TrackEvent(Ref<TrackedEvent>&& event);

    // Destroys the EventManager. Any existing tracked events' callbacks are immediately called.
    void Destroy();

    template <typename Event, typename... ReadyArgs>
    WireResult SetFutureReady(FutureID futureID, ReadyArgs&&... readyArgs) {
        // If the future id is greater than what we have assigned, it must be invalid.
        if (futureID > mNextFutureID) {
            return WireResult::FatalError;
        }

        Ref<TrackedEvent> trackedEvent;
        WireResult result = mTrackedEvents.Use([&](auto trackedEvents) {
            auto it = trackedEvents->find(futureID);
            if (it == trackedEvents->end()) {
                // If the future is not found, it must've already been completed.
                return WireResult::Success;
            }
            trackedEvent = it->second;

            if (trackedEvent->GetType() != Event::kType) {
                // Assert here for debugging, before returning a fatal error that is handled upwards
                // in production.
                DAWN_ASSERT(trackedEvent->GetType() == Event::kType);
                return WireResult::FatalError;
            }
            return WireResult::Success;
        });

        if (result != WireResult::Success || !trackedEvent) {
            return result;
        }

        // The ReadyHook function is assumed to be thread-safe or only triggered by a server
        // response (which only happens in a single thread). The only events that can be triggered
        // from the client directly as of writing are MapAsync and DeviceLost.
        result = static_cast<Event*>(trackedEvent.Get())
                     ->ReadyHook(futureID, std::forward<ReadyArgs>(readyArgs)...);

        // We need to set the event ready within the scope of the cond-var to signal it.
        mTrackedEvents.Use([&](auto trackedEvents) { trackedEvent->SetReady(); });

        // Handle spontaneous completions.
        if (trackedEvent->GetCallbackMode() == WGPUCallbackMode_AllowSpontaneous) {
            trackedEvent->Complete(futureID, EventCompletionType::Ready);
            mTrackedEvents.Use([&](auto trackedEvents) { trackedEvents->erase(futureID); });
        }
        return result;
    }

    void ProcessPollEvents();
    wgpu::WaitStatus WaitAny(Span<FutureWaitInfo> infos, uint64_t timeoutNS);

  private:
    const size_t mTimedWaitAnyMaxCount = 0;

    bool mIsDestroyed = false;

    // Tracks all kinds of events (for both WaitAny and ProcessEvents). We use an ordered map so
    // that in most cases, event ordering is already implicit when we iterate the map. (Not true for
    // WaitAny though because the user could specify the FutureIDs out of order.) The condition
    // variable is used in order to implement timed WaitAny and should notify anytime that the map
    // or any event state inside the map has changed.
    MutexCondVarProtected<EventMap> mTrackedEvents;
    std::atomic<FutureID> mNextFutureID = 1;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_EVENTMANAGER_H_
