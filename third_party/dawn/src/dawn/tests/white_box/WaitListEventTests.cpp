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

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#include "dawn/common/Ref.h"
#include "dawn/native/WaitAnySystemEvent.h"
#include "dawn/native/WaitListEvent.h"
#include "dawn/tests/DawnTest.h"

namespace dawn::native {
namespace {

constexpr uint64_t kZeroDurationNs = 0;
constexpr uint64_t kShortDurationNs = 1000000;
constexpr uint64_t kMediumDurationNs = 50000000;

// Helper to wait on a SystemEventReceiver with a timeout
bool WaitOnReceiver(const SystemEventReceiver& receiver, Nanoseconds timeout) {
    bool ready = false;
    std::pair<const SystemEventReceiver&, bool*> event = {receiver, &ready};
    return WaitAnySystemEvent(&event, &event + 1, timeout);
}

class WaitListEventTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// Test basic signaling and state checking
TEST(WaitListEventTests, SignalAndCheck) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());
    EXPECT_FALSE(event->IsSignaled());
    event->Signal();
    EXPECT_TRUE(event->IsSignaled());
}

// Test waiting on an already signaled event
TEST(WaitListEventTests, WaitAlreadySignaled) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());
    event->Signal();
    EXPECT_TRUE(event->IsSignaled());
    // Wait with zero timeout should return true immediately
    EXPECT_TRUE(event->Wait(Nanoseconds(kZeroDurationNs)));
    // Wait with non-zero timeout should return true immediately
    EXPECT_TRUE(event->Wait(Nanoseconds(kShortDurationNs)));
}

// Test waiting on an event that gets signaled later
TEST(WaitListEventTests, WaitThenSignal) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());
    EXPECT_FALSE(event->IsSignaled());

    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(kShortDurationNs));
        event->Signal();
    });

    // Wait for longer than the signal delay
    EXPECT_TRUE(event->Wait(Nanoseconds(kMediumDurationNs)));
    EXPECT_TRUE(event->IsSignaled());

    signaler.join();
}

// Test waiting with a timeout that expires
TEST(WaitListEventTests, WaitTimeout) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());
    EXPECT_FALSE(event->IsSignaled());

    // Wait for a short duration, expect timeout
    EXPECT_FALSE(event->Wait(Nanoseconds(kShortDurationNs)));
    EXPECT_FALSE(event->IsSignaled());
}

// Test waiting with a zero timeout
TEST(WaitListEventTests, WaitZeroTimeout) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());
    EXPECT_FALSE(event->IsSignaled());
    // Wait with zero timeout should return false immediately
    EXPECT_FALSE(event->Wait(Nanoseconds(kZeroDurationNs)));
    EXPECT_FALSE(event->IsSignaled());

    event->Signal();
    EXPECT_TRUE(event->IsSignaled());
    // Wait with zero timeout should return true immediately
    EXPECT_TRUE(event->Wait(Nanoseconds(kZeroDurationNs)));
}

// Test WaitAsync on an already signaled event
TEST(WaitListEventTests, WaitAsyncAlreadySignaled) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());
    event->Signal();
    EXPECT_TRUE(event->IsSignaled());

    SystemEventReceiver receiver = event->WaitAsync();
    // The receiver should be immediately ready
    EXPECT_TRUE(WaitOnReceiver(receiver, Nanoseconds(kZeroDurationNs)));
}

// Test WaitAsync, signaling the event later
TEST(WaitListEventTests, WaitAsyncThenSignal) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());
    EXPECT_FALSE(event->IsSignaled());

    SystemEventReceiver receiver = event->WaitAsync();

    // Check it's not ready yet
    EXPECT_FALSE(WaitOnReceiver(receiver, Nanoseconds(kZeroDurationNs)));

    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(kShortDurationNs));
        event->Signal();
    });

    // Wait for the receiver to become signaled
    EXPECT_TRUE(WaitOnReceiver(receiver, Nanoseconds(kMediumDurationNs)));
    EXPECT_TRUE(event->IsSignaled());

    signaler.join();
}

// Test WaitAny with an empty list
TEST(WaitListEventTests, WaitAnyEmpty) {
    std::array<std::pair<Ref<WaitListEvent>, bool*>, 0> events;
    EXPECT_FALSE(
        WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kShortDurationNs)));
}

// Test WaitAny where one event is already signaled
TEST(WaitListEventTests, WaitAnyOneAlreadySignaled) {
    Ref<WaitListEvent> event1 = AcquireRef(new WaitListEvent());
    Ref<WaitListEvent> event2 = AcquireRef(new WaitListEvent());
    event1->Signal();

    bool ready1 = false;
    bool ready2 = false;
    std::array<std::pair<Ref<WaitListEvent>, bool*>, 2> events = {
        {{event1, &ready1}, {event2, &ready2}}};

    EXPECT_TRUE(
        WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kShortDurationNs)));
    EXPECT_TRUE(ready1);
    EXPECT_FALSE(ready2);
}

// Test WaitAny where one event is signaled while waiting
TEST(WaitListEventTests, WaitAnySignalDuringWait) {
    Ref<WaitListEvent> event1 = AcquireRef(new WaitListEvent());
    Ref<WaitListEvent> event2 = AcquireRef(new WaitListEvent());

    bool ready1 = false;
    bool ready2 = false;
    std::array<std::pair<Ref<WaitListEvent>, bool*>, 2> events = {
        {{event1, &ready1}, {event2, &ready2}}};

    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(kShortDurationNs));
        event2->Signal();  // Signal the second event
    });

    EXPECT_TRUE(
        WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kMediumDurationNs)));
    EXPECT_FALSE(ready1);
    EXPECT_TRUE(ready2);  // Expect the second event to be ready

    signaler.join();
}

// Test WaitAny with a timeout
TEST(WaitListEventTests, WaitAnyTimeout) {
    Ref<WaitListEvent> event1 = AcquireRef(new WaitListEvent());
    Ref<WaitListEvent> event2 = AcquireRef(new WaitListEvent());

    bool ready1 = false;
    bool ready2 = false;
    std::array<std::pair<Ref<WaitListEvent>, bool*>, 2> events = {
        {{event1, &ready1}, {event2, &ready2}}};

    EXPECT_FALSE(
        WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kShortDurationNs)));
    EXPECT_FALSE(ready1);
    EXPECT_FALSE(ready2);
}

// Test WaitAny with zero timeout
TEST(WaitListEventTests, WaitAnyZeroTimeout) {
    Ref<WaitListEvent> event1 = AcquireRef(new WaitListEvent());
    Ref<WaitListEvent> event2 = AcquireRef(new WaitListEvent());

    bool ready1 = false;
    bool ready2 = false;
    std::array<std::pair<Ref<WaitListEvent>, bool*>, 2> events = {
        {{event1, &ready1}, {event2, &ready2}}};

    // No events signaled
    EXPECT_FALSE(
        WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kZeroDurationNs)));
    EXPECT_FALSE(ready1);
    EXPECT_FALSE(ready2);

    // Signal one event
    event1->Signal();
    EXPECT_TRUE(WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kZeroDurationNs)));
    EXPECT_TRUE(ready1);
    EXPECT_FALSE(ready2);
}

// Test WaitAny with the same event multiple times
TEST(WaitListEventTests, WaitAnyDuplicateEvents) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());

    bool ready1 = false;
    bool ready2 = false;
    std::vector<std::pair<Ref<WaitListEvent>, bool*>> events = {
        {event, &ready1}, {event, &ready2}  // Same event again
    };

    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(kShortDurationNs));
        event->Signal();
    });

    EXPECT_TRUE(
        WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kMediumDurationNs)));
    // Both ready flags corresponding to the same event should be true
    EXPECT_TRUE(ready1);
    EXPECT_TRUE(ready2);

    signaler.join();
}

// Test WaitAny with the same event multiple times, already signaled
TEST(WaitListEventTests, WaitAnyDuplicateEventsAlreadySignaled) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());

    bool ready1 = false;
    bool ready2 = false;
    std::vector<std::pair<Ref<WaitListEvent>, bool*>> events = {
        {event, &ready1}, {event, &ready2}  // Same event again
    };

    // Signal the event *before* waiting
    event->Signal();
    EXPECT_TRUE(event->IsSignaled());

    // WaitAny should return immediately since the event is already signaled
    EXPECT_TRUE(
        WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kMediumDurationNs)));

    // Both ready flags corresponding to the same event should be true
    EXPECT_TRUE(ready1);
    EXPECT_TRUE(ready2);
}

// Test multiple threads waiting on the same event
TEST(WaitListEventTests, WaitMultiThreadedSingleEvent) {
    Ref<WaitListEvent> event = AcquireRef(new WaitListEvent());

    constexpr size_t kNumWaiters = 5;
    std::array<std::thread, kNumWaiters> waiters;
    std::array<std::optional<bool>, kNumWaiters> results;

    for (size_t i = 0; i < kNumWaiters; ++i) {
        waiters[i] = std::thread(
            [&results, &event, i]() { results[i] = event->Wait(Nanoseconds(kMediumDurationNs)); });
    }

    // Give waiters time to start waiting
    std::this_thread::sleep_for(std::chrono::nanoseconds(kShortDurationNs));
    event->Signal();

    // Check all waiters returned true
    for (size_t i = 0; i < kNumWaiters; ++i) {
        waiters[i].join();
        EXPECT_TRUE(results[i].has_value());
        EXPECT_TRUE(results[i].value());
    }
    EXPECT_TRUE(event->IsSignaled());
}

// Test multiple threads waiting on different events via WaitAny
TEST(WaitListEventTests, WaitAnyMultiThreaded) {
    Ref<WaitListEvent> event1 = AcquireRef(new WaitListEvent());
    Ref<WaitListEvent> event2 = AcquireRef(new WaitListEvent());
    Ref<WaitListEvent> event3 = AcquireRef(new WaitListEvent());

    bool ready1 = false;
    bool ready2 = false;
    bool ready3 = false;
    std::array<std::pair<Ref<WaitListEvent>, bool*>, 3> events = {
        {{event1, &ready1}, {event2, &ready2}, {event3, &ready3}}};

    // Start a thread that waits on any of the events
    bool waitResult = false;
    std::thread waiter([&]() {
        waitResult =
            WaitListEvent::WaitAny(events.begin(), events.end(), Nanoseconds(kMediumDurationNs));
    });

    // Start another thread that signals one of the events
    std::thread signaler([&]() {
        std::this_thread::sleep_for(std::chrono::nanoseconds(kShortDurationNs));
        event2->Signal();  // Signal the middle event
    });

    waiter.join();

    // Check that the waiting thread completes successfully
    EXPECT_TRUE(waitResult);

    // Check that the correct ready flag was set
    EXPECT_FALSE(ready1);
    EXPECT_TRUE(ready2);
    EXPECT_FALSE(ready3);

    signaler.join();
}

}  // namespace
}  // namespace dawn::native
