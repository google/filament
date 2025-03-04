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

#include "dawn/common/MutexProtected.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
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
MutexProtected<T> CreateDefault() {
    if constexpr (IsRef<T>::value) {
        return MutexProtected<T>(AcquireRef(new typename UnwrapRef<T>::type()));
    } else {
        return MutexProtected<T>();
    }
}

template <typename T, typename... Args>
MutexProtected<T> CreateCustom(Args&&... args) {
    if constexpr (IsRef<T>::value) {
        return MutexProtected<T>(
            AcquireRef(new typename UnwrapRef<T>::type(std::forward<Args>(args)...)));
    } else {
        return MutexProtected<T>(std::forward<Args>(args)...);
    }
}

template <typename T>
class MutexProtectedTest : public Test {};

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

    MutexProtected<TypeParam> counter = CreateDefault<TypeParam>();

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

    MutexProtected<TypeParam> counter = CreateCustom<TypeParam>(kStartingcount);

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

    MutexProtected<TypeParam> c1 = CreateDefault<TypeParam>();
    MutexProtected<TypeParam> c2 = CreateDefault<TypeParam>();

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
