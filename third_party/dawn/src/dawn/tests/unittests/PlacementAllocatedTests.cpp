// Copyright 2020 The Dawn & Tint Authors
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

#include <memory>

#include "dawn/common/PlacementAllocated.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using testing::InSequence;
using testing::StrictMock;

enum class DestructedClass {
    Foo,
    Bar,
};

class MockDestructor {
  public:
    MOCK_METHOD(void, Call, (void*, DestructedClass));
};

std::unique_ptr<StrictMock<MockDestructor>> mockDestructor;

class PlacementAllocatedTests : public testing::Test {
    void SetUp() override { mockDestructor = std::make_unique<StrictMock<MockDestructor>>(); }

    void TearDown() override { mockDestructor = nullptr; }
};

struct Foo : PlacementAllocated {
    virtual ~Foo() { mockDestructor->Call(this, DestructedClass::Foo); }
};

struct Bar : Foo {
    ~Bar() override { mockDestructor->Call(this, DestructedClass::Bar); }
};

// Test that deletion calls the destructor and does not free memory.
TEST_F(PlacementAllocatedTests, DeletionDoesNotFreeMemory) {
    void* ptr = malloc(sizeof(Foo));

    Foo* foo = new (ptr) Foo();

    EXPECT_CALL(*mockDestructor, Call(foo, DestructedClass::Foo));
    delete foo;

    // Touch the memory, this shouldn't crash.
    static_assert(sizeof(Foo) >= sizeof(uint32_t));
    *reinterpret_cast<uint32_t*>(foo) = 42;

    free(ptr);
}

// Test that destructing an instance of a derived class calls the derived, then base destructor, and
// does not free memory.
TEST_F(PlacementAllocatedTests, DeletingDerivedClassCallsBaseDestructor) {
    void* ptr = malloc(sizeof(Bar));

    Bar* bar = new (ptr) Bar();

    {
        InSequence s;
        EXPECT_CALL(*mockDestructor, Call(bar, DestructedClass::Bar));
        EXPECT_CALL(*mockDestructor, Call(bar, DestructedClass::Foo));
        delete bar;
    }

    // Touch the memory, this shouldn't crash.
    static_assert(sizeof(Bar) >= sizeof(uint32_t));
    *reinterpret_cast<uint32_t*>(bar) = 42;

    free(ptr);
}

// Test that destructing an instance of a base class calls the derived, then base destructor, and
// does not free memory.
TEST_F(PlacementAllocatedTests, DeletingBaseClassCallsDerivedDestructor) {
    void* ptr = malloc(sizeof(Bar));

    Foo* foo = new (ptr) Bar();

    {
        InSequence s;
        EXPECT_CALL(*mockDestructor, Call(foo, DestructedClass::Bar));
        EXPECT_CALL(*mockDestructor, Call(foo, DestructedClass::Foo));
        delete foo;
    }

    // Touch the memory, this shouldn't crash.
    static_assert(sizeof(Bar) >= sizeof(uint32_t));
    *reinterpret_cast<uint32_t*>(foo) = 42;

    free(ptr);
}

}  // anonymous namespace
}  // namespace dawn
