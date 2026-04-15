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

#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/Time.h"
#include "dawn/utils/SystemUtils.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using ::testing::Test;
using ::testing::Types;

// Simple thread-unsafe counter class.
class CounterT : public RefCounted {
  public:
    CounterT() = default;
    explicit CounterT(int count) : mCount(count) {}

    int Get() const { return mCount; }

    void Increment() { mCount++; }
    void Decrement() { mCount--; }

  private:
    int mCount = 0;
};

template <typename T>
class MutexProtectedTest : public Test {
  protected:
    MutexProtected<T> CreateDefault() {
        if constexpr (IsRef<T>::value) {
            return MutexProtected<T>(AcquireRef(new typename UnwrapRef<T>::type()));
        } else {
            return MutexProtected<T>();
        }
    }

    template <typename... Args>
    MutexProtected<T> CreateCustom(Args&&... args) {
        if constexpr (IsRef<T>::value) {
            return MutexProtected<T>(
                AcquireRef(new typename UnwrapRef<T>::type(std::forward<Args>(args)...)));
        } else {
            return MutexProtected<T>(std::forward<Args>(args)...);
        }
    }
};

class MutexProtectedTestTypeNames {
  public:
    template <typename T>
    static std::string GetName(int) {
        if (std::is_same<T, CounterT>()) {
            return "CounterT";
        }
        if (std::is_same<T, Ref<CounterT>>()) {
            return "Ref<CounterT>";
        }
    }
};
using MutexProtectedTestTypes = Types<CounterT, Ref<CounterT>>;
TYPED_TEST_SUITE(MutexProtectedTest, MutexProtectedTestTypes, MutexProtectedTestTypeNames);

TYPED_TEST(MutexProtectedTest, DefaultCtor) {
    static constexpr int kIncrementCount = 100;
    static constexpr int kDecrementCount = 50;

    auto counter = this->CreateDefault();

    auto increment = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            counter->Increment();
        }
    };
    auto useIncrement = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            counter.Use([](auto c) { c->Increment(); });
        }
    };
    auto decrement = [&] {
        for (uint32_t i = 0; i < kDecrementCount; i++) {
            counter->Decrement();
        }
    };
    auto useDecrement = [&] {
        for (uint32_t i = 0; i < kDecrementCount; i++) {
            counter.Use([](auto c) { c->Decrement(); });
        }
    };

    std::thread incrementThread(increment);
    std::thread useIncrementThread(useIncrement);
    std::thread decrementThread(decrement);
    std::thread useDecrementThread(useDecrement);
    incrementThread.join();
    useIncrementThread.join();
    decrementThread.join();
    useDecrementThread.join();

    EXPECT_EQ(counter->Get(), 2 * (kIncrementCount - kDecrementCount));
}

TYPED_TEST(MutexProtectedTest, CustomCtor) {
    static constexpr int kIncrementCount = 100;
    static constexpr int kDecrementCount = 50;
    static constexpr int kStartingcount = -100;

    auto counter = this->CreateCustom(kStartingcount);

    auto increment = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            counter->Increment();
        }
    };
    auto useIncrement = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            counter.Use([](auto c) { c->Increment(); });
        }
    };
    auto decrement = [&] {
        for (uint32_t i = 0; i < kDecrementCount; i++) {
            counter->Decrement();
        }
    };
    auto useDecrement = [&] {
        for (uint32_t i = 0; i < kDecrementCount; i++) {
            counter.Use([](auto c) { c->Decrement(); });
        }
    };

    std::thread incrementThread(increment);
    std::thread useIncrementThread(useIncrement);
    std::thread decrementThread(decrement);
    std::thread useDecrementThread(useDecrement);
    incrementThread.join();
    useIncrementThread.join();
    decrementThread.join();
    useDecrementThread.join();

    EXPECT_EQ(counter->Get(), kStartingcount + 2 * (kIncrementCount - kDecrementCount));
}

TYPED_TEST(MutexProtectedTest, MultipleProtected) {
    static constexpr int kIncrementCount = 100;

    auto c1 = this->CreateDefault();
    auto c2 = this->CreateDefault();

    auto increment = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            c1.Use([&](auto x1) {
                c2.Use([&](auto x2) {
                    x1->Increment();
                    x2->Increment();
                });
            });
        }
    };
    auto validate = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            c1.Use([&](auto x1) { c2.Use([&](auto x2) { EXPECT_EQ(x1->Get(), x2->Get()); }); });
        }
    };
    std::thread incrementThread(increment);
    std::thread validateThread(validate);
    incrementThread.join();
    validateThread.join();
}

TEST(MutexCondVarProtectedTest, Nominal) {
    static constexpr int kIncrementCount = 100;
    auto counter = MutexCondVarProtected<CounterT>();

    auto increment = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            counter->Increment();
        }
    };
    auto useIncrement = [&] {
        for (uint32_t i = 0; i < kIncrementCount; i++) {
            counter.Use([](auto c) { c->Increment(); });
        }
    };
    std::thread incrementThread(increment);
    std::thread useIncrementThread(useIncrement);

    auto expected = 2 * kIncrementCount;
    counter.Use([&](auto c) {
        c.WaitFor(kMaxDurationNanos, [&](auto& count) { return count.Get() == expected; });
        EXPECT_EQ(c->Get(), expected);
    });
    EXPECT_EQ(counter->Get(), expected);

    incrementThread.join();
    useIncrementThread.join();
}

// WaitFor should timeout and fail if the condition is never met.
TEST(MutexCondVarProtectedTest, WaitForTimeout) {
    auto counter = MutexCondVarProtected<CounterT>();
    counter.Use([](auto c) {
        EXPECT_FALSE(c.WaitFor(Nanoseconds(5), [](auto& x) { return x.Get() == 1; }));
    });
}

// Test that Wait releases the lock, otherwise this test would deadlock.
TEST(MutexCondVarProtectedTest, WaitDeadlock) {
    auto c1 = MutexCondVarProtected<CounterT>();
    auto c2 = MutexCondVarProtected<CounterT>();

    auto t1 = [&] {
        c1.Use([&](auto x1) {
            x1.Wait([](auto& x) { return x.Get() == 1; });
            c2->Increment();
        });
    };
    auto t2 = [&] {
        c2.Use([&](auto x2) {
            c1->Increment();
            x2.Wait([](auto& x) { return x.Get() == 1; });
        });
    };

    std::thread thread1(t1);
    std::thread thread2(t2);
    thread1.join();
    thread2.join();
}

// Test that if we specifically ask for only one thread to be notified, then only one thread should
// wake up from waiting.
TEST(MutexCondVarProtectedTest, NotifyTypes) {
    auto counter = MutexCondVarProtected<CounterT>();
    std::atomic<int> woken = 0;

    // Multiple threads both waiting on the condition variable, only one of them should actually be
    // woken up on the first increment.
    static constexpr int kNumThreads = 5;
    std::vector<std::thread> threads;
    threads.reserve(kNumThreads);
    for (auto i = 0; i < kNumThreads; i++) {
        threads.emplace_back([&] {
            counter.ConstUse([&](auto c) {
                c.Wait([](auto& x) { return x.Get() >= 1; });
                woken += 1;
            });
        });
    }

    // Don't notify any threads.
    counter.Use<NotifyType::None>([](auto c) { EXPECT_EQ(c->Get(), 0); });
    EXPECT_EQ(woken, 0);

    // Notify one of the threads only. This is currently racy w.r.t to the increment below in that
    // it's possible that the increment happens before the threads start waiting. As a result, we
    // only verify that at least once thread was woken. In practice, it is very difficult to verify
    // that exactly one thread is woken.
    counter.Use<NotifyType::One>([](auto c) { c->Increment(); });
    while (woken == 0) {
        utils::USleep(1000);
    }

    // Notify the rest of the threads via a NotifyAll, then wait for all the threads to join.
    counter.Use<NotifyType::All>([](auto c) { c->Increment(); });
    for (auto& t : threads) {
        t.join();
    }
    EXPECT_EQ(woken, kNumThreads);
}

}  // anonymous namespace
}  // namespace dawn

// Special compilation tests that are only enabled when experimental headers are available.
#if __has_include(<experimental/type_traits>)
#include <experimental/type_traits>

namespace dawn {
namespace {

// MutexProtected types are only copyable when they are wrapping a Ref type.
template <typename T>
using mutexprotected_copyable_t =
    decltype(std::declval<MutexProtected<T>&>() = std::declval<const MutexProtected<T>&>());
TYPED_TEST(MutexProtectedTest, Copyable) {
    static_assert(IsRef<TypeParam>::value ==
                      std::experimental::is_detected_v<mutexprotected_copyable_t, TypeParam>,
                  "Copy assignment is only allowed when the wrapping type is a Ref.");
}

}  // anonymous namespace
}  // namespace dawn

#endif
