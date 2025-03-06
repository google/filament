// Copyright 2017 The Dawn & Tint Authors
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

#include <type_traits>

#include "gtest/gtest.h"

#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/ToBackend.h"

// Make our own Base - Backend object pair, reusing the MyObjectBase name
namespace dawn::native {

class MyObjectBase : public RefCounted {};

class MyObject : public MyObjectBase {};

struct MyBackendTraits {
    using MyObjectType = MyObject;
};

template <typename BackendTraits>
struct ToBackendTraits<MyObjectBase, BackendTraits> {
    using BackendType = typename BackendTraits::MyObjectType;
};

// Instantiate ToBackend for our "backend"
template <typename T>
auto ToBackend(T&& common) -> decltype(ToBackendBase<MyBackendTraits>(common)) {
    return ToBackendBase<MyBackendTraits>(common);
}

// Test that ToBackend correctly converts pointers to base classes.
TEST(ToBackend, Pointers) {
    {
        MyObject* myObject = new MyObject;
        const MyObjectBase* base = myObject;

        auto* backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(backendAdapter), const MyObject*>::value);
        ASSERT_EQ(myObject, backendAdapter);

        myObject->Release();
    }
    {
        MyObject* myObject = new MyObject;
        MyObjectBase* base = myObject;

        auto* backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(backendAdapter), MyObject*>::value);
        ASSERT_EQ(myObject, backendAdapter);

        myObject->Release();
    }
}

// Test that ToBackend correctly converts Refs to base classes.
TEST(ToBackend, Ref) {
    {
        MyObject* myObject = new MyObject;
        const Ref<MyObjectBase> base(myObject);

        const auto& backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(ToBackend(base)), const Ref<MyObject>&>::value);
        ASSERT_EQ(myObject, backendAdapter.Get());

        myObject->Release();
    }
    {
        MyObject* myObject = new MyObject;
        Ref<MyObjectBase> base(myObject);

        auto backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(ToBackend(base)), Ref<MyObject>&>::value);
        ASSERT_EQ(myObject, backendAdapter.Get());

        myObject->Release();
    }
}
}  // namespace dawn::native
