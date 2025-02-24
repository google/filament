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

#include <functional>
#include <thread>

#include "dawn/common/WeakRef.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/utils/BinarySemaphore.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using utils::BinarySemaphore;

class RefCountedT : public RefCounted {};

class WeakRefBaseA : public RefCounted, public WeakRefSupport<WeakRefBaseA> {
  public:
    WeakRefBaseA() = default;
    explicit WeakRefBaseA(std::function<void(WeakRefBaseA*)> deleteFn) : mDeleteFn(deleteFn) {}

  protected:
    ~WeakRefBaseA() override { mDeleteFn(this); }

  private:
    std::function<void(WeakRefBaseA*)> mDeleteFn = [](WeakRefBaseA*) -> void {};
};

class WeakRefDerivedA : public WeakRefBaseA {
  public:
    WeakRefDerivedA() = default;
    explicit WeakRefDerivedA(std::function<void(WeakRefBaseA*)> deleteFn)
        : WeakRefBaseA(deleteFn) {}
};

class WeakRefBaseB : public RefCounted, public WeakRefSupport<WeakRefBaseB> {};
class WeakRefDerivedB : public WeakRefBaseB {};

// When the original refcounted object is destroyed, all WeakRefs are no longer able to Promote.
TEST(WeakRefTests, BasicPromote) {
    Ref<WeakRefBaseA> base = AcquireRef(new WeakRefBaseA());
    WeakRef<WeakRefBaseA> weak = GetWeakRef(base);
    EXPECT_EQ(weak.Promote().Get(), base.Get());

    base = nullptr;
    EXPECT_EQ(weak.Promote().Get(), nullptr);
}

// When the original refcounted object is destroyed, all WeakRefs, including upcasted ones, are no
// longer able to Promote.
TEST(WeakRefTests, DerivedPromote) {
    Ref<WeakRefDerivedA> base = AcquireRef(new WeakRefDerivedA());
    WeakRef<WeakRefDerivedA> weak1 = GetWeakRef(base);
    WeakRef<WeakRefBaseA> weak2 = weak1;
    WeakRef<WeakRefBaseA> weak3 = GetWeakRef(base);
    EXPECT_EQ(weak1.Promote().Get(), base.Get());
    EXPECT_EQ(weak2.Promote().Get(), base.Get());
    EXPECT_EQ(weak3.Promote().Get(), base.Get());

    base = nullptr;
    EXPECT_EQ(weak1.Promote().Get(), nullptr);
    EXPECT_EQ(weak2.Promote().Get(), nullptr);
    EXPECT_EQ(weak3.Promote().Get(), nullptr);
}

// Trying to promote a WeakRef to a Ref while the original value is being destroyed returns nullptr.
TEST(WeakRefTests, DeletingAndPromoting) {
    BinarySemaphore semA, semB;
    Ref<WeakRefBaseA> base = AcquireRef(new WeakRefBaseA([&](WeakRefBaseA*) {
        semB.Release();
        semA.Acquire();
    }));

    auto f = [&] {
        WeakRef<WeakRefBaseA> weak = GetWeakRef(base);
        semA.Release();
        semB.Acquire();
        EXPECT_EQ(weak.Promote().Get(), nullptr);
        semA.Release();
    };
    std::thread t(f);

    semA.Acquire();
    base = nullptr;
    t.join();
}

}  // anonymous namespace
}  // namespace dawn

// Special compilation tests that are only enabled when experimental headers are available.
#if __has_include(<experimental/type_traits>)
#include <experimental/type_traits>

namespace dawn {
namespace {

// Helper detection utilities for verifying that unintended assignments are not allowed.
template <typename L, typename R>
using weakref_copyable_t =
    decltype(std::declval<WeakRef<L>&>() = std::declval<const WeakRef<R>&>());
template <typename L, typename R>
using weakref_movable_t =
    decltype(std::declval<WeakRef<L>&>() = std::declval<const WeakRef<R>&&>());
TEST(WeakRefTests, CrossTypesAssignments) {
    // Same type and upcasting is allowed.
    static_assert(std::experimental::is_detected_v<weakref_copyable_t, WeakRefBaseA, WeakRefBaseA>,
                  "Same type copy assignment is allowed.");
    static_assert(std::experimental::is_detected_v<weakref_movable_t, WeakRefBaseA, WeakRefBaseA>,
                  "Same type move assignment is allowed.");

    static_assert(
        std::experimental::is_detected_v<weakref_copyable_t, WeakRefBaseA, WeakRefDerivedA>,
        "Upcasting type copy assignment is allowed.");
    static_assert(
        std::experimental::is_detected_v<weakref_movable_t, WeakRefBaseA, WeakRefDerivedA>,
        "Upcasting type move assignment is allowed.");

    // Same type, but down casting is not allowed.
    static_assert(
        !std::experimental::is_detected_v<weakref_copyable_t, WeakRefDerivedA, WeakRefBaseA>,
        "Downcasting type copy assignment is not allowed.");
    static_assert(
        !std::experimental::is_detected_v<weakref_movable_t, WeakRefDerivedA, WeakRefBaseA>,
        "Downcasting type move assignment is not allowed.");

    // Cross types are not allowed.
    static_assert(!std::experimental::is_detected_v<weakref_copyable_t, WeakRefBaseA, WeakRefBaseB>,
                  "Cross type copy assignment is not allowed.");
    static_assert(!std::experimental::is_detected_v<weakref_movable_t, WeakRefBaseA, WeakRefBaseB>,
                  "Cross type move assignment is not allowed.");
    static_assert(
        !std::experimental::is_detected_v<weakref_copyable_t, WeakRefBaseA, WeakRefDerivedB>,
        "Cross type upcasting copy assignment is not allowed.");
    static_assert(
        !std::experimental::is_detected_v<weakref_movable_t, WeakRefBaseA, WeakRefDerivedB>,
        "Cross type upcasting move assignment is not allowed.");
}

// Helper detection utilty to verify whether GetWeakRef is enabled.
template <typename T>
using can_get_weakref_t = decltype(GetWeakRef(std::declval<T*>()));
TEST(WeakRefTests, GetWeakRefFromPtr) {
    // The GetWeakRef function is only available on types that extend WeakRefSupport.
    static_assert(std::experimental::is_detected_v<can_get_weakref_t, WeakRefBaseA>,
                  "GetWeakRef is enabled on classes that directly extend WeakRefSupport.");
    static_assert(std::experimental::is_detected_v<can_get_weakref_t, WeakRefDerivedA>,
                  "GetWeakRef is enabled on classes that indirectly extend WeakRefSupport.");

    static_assert(!std::experimental::is_detected_v<can_get_weakref_t, RefCountedT>,
                  "GetWeakRef is disabled on classes that do not extend WeakRefSupport.");
}

// Helper detection utilty to verify whether GetWeakRef is enabled.
template <typename T>
using can_get_weakref_from_ref_t = decltype(GetWeakRef(std::declval<Ref<T>>()));
TEST(WeakRefTests, GetWeakRefFromRef) {
    // The GetWeakRef function is only available on types that extend WeakRefSupport.
    static_assert(std::experimental::is_detected_v<can_get_weakref_from_ref_t, WeakRefBaseA>,
                  "GetWeakRef is enabled on classes that directly extend WeakRefSupport.");
    static_assert(std::experimental::is_detected_v<can_get_weakref_from_ref_t, WeakRefDerivedA>,
                  "GetWeakRef is enabled on classes that indirectly extend WeakRefSupport.");

    static_assert(!std::experimental::is_detected_v<can_get_weakref_from_ref_t, RefCountedT>,
                  "GetWeakRef is disabled on classes that do not extend WeakRefSupport.");
}

}  // anonymous namespace
}  // namespace dawn

#endif
