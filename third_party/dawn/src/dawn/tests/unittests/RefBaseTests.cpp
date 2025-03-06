// Copyright 2021 The Dawn & Tint Authors
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

#include <utility>
#include <vector>

#include "dawn/common/RefBase.h"
#include "gmock/gmock.h"

namespace dawn {
namespace {

using Id = uint32_t;

enum class Action {
    kAddRef,
    kRelease,
    kAssign,
    kMarker,
};

struct Event {
    Action action;
    Id thisId = 0;
    Id otherId = 0;
};

std::ostream& operator<<(std::ostream& os, const Event& event) {
    switch (event.action) {
        case Action::kAddRef:
            os << "AddRef " << event.thisId;
            break;
        case Action::kRelease:
            os << "Release " << event.thisId;
            break;
        case Action::kAssign:
            os << "Assign " << event.thisId << " <- " << event.otherId;
            break;
        case Action::kMarker:
            os << "Marker " << event.thisId;
            break;
    }
    return os;
}

bool operator==(const Event& a, const Event& b) {
    return a.action == b.action && a.thisId == b.thisId && a.otherId == b.otherId;
}

using Events = std::vector<Event>;

struct RefTracker {
    explicit constexpr RefTracker(nullptr_t) : mId(0), mEvents(nullptr) {}

    constexpr RefTracker(const RefTracker& other) = default;

    RefTracker(Id id, Events* events) : mId(id), mEvents(events) {}

    void AddRef() const { mEvents->emplace_back(Event{Action::kAddRef, mId}); }

    void Release() const { mEvents->emplace_back(Event{Action::kRelease, mId}); }

    RefTracker& operator=(const RefTracker& other) {
        if (mEvents || other.mEvents) {
            Events* events = mEvents ? mEvents : other.mEvents;
            events->emplace_back(Event{Action::kAssign, mId, other.mId});
        }
        mId = other.mId;
        mEvents = other.mEvents;
        return *this;
    }

    bool operator==(const RefTracker& other) const { return mId == other.mId; }

    bool operator!=(const RefTracker& other) const { return mId != other.mId; }

    Id mId;
    Events* mEvents;
};

struct RefTrackerTraits {
    static constexpr RefTracker kNullValue{nullptr};

    static void AddRef(const RefTracker& handle) { handle.AddRef(); }

    static void Release(const RefTracker& handle) { handle.Release(); }
};

constexpr RefTracker RefTrackerTraits::kNullValue;

using Ref = RefBase<RefTracker, RefTrackerTraits>;

TEST(RefBase, Acquire) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref ref(tracker1);

    events.clear();
    { ref.Acquire(tracker2); }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kRelease, 1},   // release ref
                                             Event{Action::kAssign, 1, 2}  // acquire tracker2
                                             ));
}

TEST(RefBase, Detach) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref(tracker);

    events.clear();
    { [[maybe_unused]] auto ptr = ref.Detach(); }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kAssign, 1, 0}  // nullify ref
                                             ));
}

TEST(RefBase, Constructor) {
    Ref ref;
    EXPECT_EQ(ref.Get(), RefTrackerTraits::kNullValue);
}

TEST(RefBase, ConstructDestruct) {
    Events events;
    RefTracker tracker(1, &events);

    events.clear();
    {
        Ref ref(tracker);
        events.emplace_back(Event{Action::kMarker, 10});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kAddRef, 1},   // reference tracker
                                             Event{Action::kMarker, 10},  //
                                             Event{Action::kRelease, 1}   // destruct ref
                                             ));
}

TEST(RefBase, CopyConstruct) {
    Events events;
    RefTracker tracker(1, &events);
    Ref refA(tracker);

    events.clear();
    {
        Ref refB(refA);
        events.emplace_back(Event{Action::kMarker, 10});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kAddRef, 1},   // reference tracker
                                             Event{Action::kMarker, 10},  //
                                             Event{Action::kRelease, 1}   // destruct ref
                                             ));
}

TEST(RefBase, RefCopyAssignment) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref refA(tracker1);
    Ref refB(tracker2);

    events.clear();
    {
        Ref ref;
        events.emplace_back(Event{Action::kMarker, 10});
        ref = refA;
        events.emplace_back(Event{Action::kMarker, 20});
        ref = refB;
        events.emplace_back(Event{Action::kMarker, 30});
        ref = refA;
        events.emplace_back(Event{Action::kMarker, 40});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kMarker, 10},    //
                                             Event{Action::kAddRef, 1},     // reference tracker1
                                             Event{Action::kAssign, 0, 1},  // copy tracker1
                                             Event{Action::kMarker, 20},    //
                                             Event{Action::kAddRef, 2},     // reference tracker2
                                             Event{Action::kRelease, 1},    // release tracker1
                                             Event{Action::kAssign, 1, 2},  // copy tracker2
                                             Event{Action::kMarker, 30},    //
                                             Event{Action::kAddRef, 1},     // reference tracker1
                                             Event{Action::kRelease, 2},    // release tracker2
                                             Event{Action::kAssign, 2, 1},  // copy tracker1
                                             Event{Action::kMarker, 40},    //
                                             Event{Action::kRelease, 1}     // destruct ref
                                             ));
}

TEST(RefBase, RefMoveAssignment) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref refA(tracker1);
    Ref refB(tracker2);

    events.clear();
    {
        Ref ref;
        events.emplace_back(Event{Action::kMarker, 10});
        ref = std::move(refA);
        events.emplace_back(Event{Action::kMarker, 20});
        ref = std::move(refB);
        events.emplace_back(Event{Action::kMarker, 30});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kMarker, 10},    //
                                             Event{Action::kAssign, 1, 0},  // nullify refA
                                             Event{Action::kAssign, 0, 1},  // move into ref
                                             Event{Action::kMarker, 20},    //
                                             Event{Action::kRelease, 1},    // release tracker1
                                             Event{Action::kAssign, 2, 0},  // nullify refB
                                             Event{Action::kAssign, 1, 2},  // move into ref
                                             Event{Action::kMarker, 30},    //
                                             Event{Action::kRelease, 2}     // destruct ref
                                             ));
}

TEST(RefBase, RefCopyAssignmentSelf) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref(tracker);
    Ref& self = ref;

    events.clear();
    {
        ref = self;
        ref = self;
        ref = self;
    }
    EXPECT_THAT(events, testing::ElementsAre());
}

TEST(RefBase, RefMoveAssignmentSelf) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref(tracker);
    Ref& self = ref;

    events.clear();
    { ref = std::move(self); }
    EXPECT_THAT(events, testing::ElementsAre());
}

TEST(RefBase, TCopyAssignment) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref;

    events.clear();
    {
        ref = tracker;
        ref = tracker;
        ref = tracker;
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kAddRef, 1},  //
                                             Event{Action::kAssign, 0, 1}));
}

TEST(RefBase, TMoveAssignment) {
    Events events;
    RefTracker tracker(1, &events);
    Ref ref;

    events.clear();
    { ref = std::move(tracker); }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kAddRef, 1},  //
                                             Event{Action::kAssign, 0, 1}));
}

TEST(RefBase, TCopyAssignmentAlternate) {
    Events events;
    RefTracker tracker1(1, &events);
    RefTracker tracker2(2, &events);
    Ref ref;

    events.clear();
    {
        ref = tracker1;
        events.emplace_back(Event{Action::kMarker, 10});
        ref = tracker2;
        events.emplace_back(Event{Action::kMarker, 20});
        ref = tracker1;
        events.emplace_back(Event{Action::kMarker, 30});
    }
    EXPECT_THAT(events, testing::ElementsAre(Event{Action::kAddRef, 1},     // reference tracker1
                                             Event{Action::kAssign, 0, 1},  // copy tracker1
                                             Event{Action::kMarker, 10},    //
                                             Event{Action::kAddRef, 2},     // reference tracker2
                                             Event{Action::kRelease, 1},    // release tracker1
                                             Event{Action::kAssign, 1, 2},  // copy tracker2
                                             Event{Action::kMarker, 20},    //
                                             Event{Action::kAddRef, 1},     // reference tracker1
                                             Event{Action::kRelease, 2},    // release tracker2
                                             Event{Action::kAssign, 2, 1},  // copy tracker1
                                             Event{Action::kMarker, 30}));
}

// Regression test for an issue where RefBase<T*> comparison would end up using operator bool
// depending on the order in which the compiler did implicit conversions.
struct FakePtrRefTraits {
    static constexpr int* kNullValue{nullptr};
    static void AddRef(int*) {}
    static void Release(int*) {}
};
TEST(RefBase, MissingExplicitOnOperatorBool) {
    using MyRef = RefBase<int*, FakePtrRefTraits>;
    int a = 0;
    int b = 1;
    MyRef refA(&a);
    MyRef refB(&b);

    EXPECT_TRUE(refA == refA);
    EXPECT_FALSE(refA != refA);
    EXPECT_TRUE((refA) == (refA));
    EXPECT_FALSE((refA) != (refA));

    EXPECT_FALSE(refA == refB);
    EXPECT_TRUE(refA != refB);
    EXPECT_FALSE((refA) == (refB));
    EXPECT_TRUE((refA) != (refB));
}

}  // anonymous namespace
}  // namespace dawn
