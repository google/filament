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

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <utility>

class Object : public wgpu::ObjectBase<Object, int*> {
  public:
    using ObjectBase::ObjectBase;
    using ObjectBase::operator=;

    static void WGPUAddRef(int* handle) {
        ASSERT_LE(0, *handle);
        *handle += 1;
    }
    static void WGPURelease(int* handle) {
        ASSERT_LT(0, *handle);
        *handle -= 1;
    }
};

// Test that creating an C++ object from a C object takes a ref.
// Also test that the C++ object destructor removes a ref.
TEST(ObjectBase, CTypeConstructor) {
    int refcount = 1;
    {
        Object obj(&refcount);
        ASSERT_EQ(2, refcount);
    }
    ASSERT_EQ(1, refcount);
}

// Test consuming a C object into a C++ object doesn't take a ref.
TEST(ObjectBase, AcquireConstruction) {
    int refcount = 1;
    {
        Object object = Object::Acquire(&refcount);
        ASSERT_EQ(1, refcount);
    }
    ASSERT_EQ(0, refcount);
}

// Test .Get().
TEST(ObjectBase, Get) {
    int refcount = 1;
    {
        Object obj1(&refcount);

        ASSERT_EQ(2, refcount);
        ASSERT_EQ(&refcount, obj1.Get());
    }
    ASSERT_EQ(1, refcount);
}

// Test that MoveToCHandle consumes the C++ object into a C object and doesn't release
TEST(ObjectBase, MoveToCHandle) {
    int refcount = 1;
    {
        Object obj(&refcount);
        ASSERT_EQ(2, refcount);

        ASSERT_EQ(&refcount, obj.MoveToCHandle());
        ASSERT_EQ(nullptr, obj.Get());
        ASSERT_EQ(2, refcount);
    }
    ASSERT_EQ(2, refcount);
}

// Test using C++ objects in conditions
TEST(ObjectBase, OperatorBool) {
    int refcount = 1;
    Object trueObj(&refcount);
    Object falseObj;

    if (falseObj || !trueObj) {
        ASSERT_TRUE(false);
    }
}

// Test the copy constructor of C++ objects
TEST(ObjectBase, CopyConstructor) {
    int refcount = 1;

    Object source(&refcount);
    Object destination(source);

    ASSERT_EQ(source.Get(), &refcount);
    ASSERT_EQ(destination.Get(), &refcount);
    ASSERT_EQ(3, refcount);

    destination = Object();
    ASSERT_EQ(refcount, 2);
}

// Test the copy assignment of C++ objects
TEST(ObjectBase, CopyAssignment) {
    int refcount = 1;
    Object source(&refcount);

    Object destination;
    destination = source;

    ASSERT_EQ(source.Get(), &refcount);
    ASSERT_EQ(destination.Get(), &refcount);
    ASSERT_EQ(3, refcount);

    destination = Object();
    ASSERT_EQ(refcount, 2);
}

// Test the repeated copy assignment of C++ objects
TEST(ObjectBase, RepeatedCopyAssignment) {
    int refcount = 1;
    Object source(&refcount);

    Object destination;
    for (int i = 0; i < 10; i++) {
        destination = source;
    }

    ASSERT_EQ(source.Get(), &refcount);
    ASSERT_EQ(destination.Get(), &refcount);
    ASSERT_EQ(3, refcount);

    destination = Object();
    ASSERT_EQ(refcount, 2);
}

// Test the copy assignment of C++ objects onto themselves
TEST(ObjectBase, CopyAssignmentSelf) {
    int refcount = 1;

    Object obj(&refcount);

    // Fool the compiler to avoid a -Wself-assign-overload
    Object* objPtr = &obj;
    obj = *objPtr;

    ASSERT_EQ(obj.Get(), &refcount);
    ASSERT_EQ(refcount, 2);
}

// Test the move constructor of C++ objects
TEST(ObjectBase, MoveConstructor) {
    int refcount = 1;
    Object source(&refcount);
    Object destination(std::move(source));

    ASSERT_EQ(destination.Get(), &refcount);
    ASSERT_EQ(2, refcount);

    destination = Object();
    ASSERT_EQ(refcount, 1);
}

// Test the move assignment of C++ objects
TEST(ObjectBase, MoveAssignment) {
    int refcount = 1;
    Object source(&refcount);

    Object destination;
    destination = std::move(source);

    ASSERT_EQ(destination.Get(), &refcount);
    ASSERT_EQ(2, refcount);

    destination = Object();
    ASSERT_EQ(refcount, 1);
}

// Test the move assignment of C++ objects onto themselves
TEST(ObjectBase, MoveAssignmentSelf) {
    int refcount = 1;

    Object obj(&refcount);

    // Fool the compiler to avoid a -Wself-move
    Object* objPtr = &obj;
    obj = std::move(*objPtr);

    ASSERT_EQ(obj.Get(), &refcount);
    ASSERT_EQ(refcount, 2);
}

// Test the constructor using nullptr
TEST(ObjectBase, NullptrConstructor) {
    Object obj(nullptr);
    ASSERT_EQ(obj.Get(), nullptr);
}

// Test assigning nullptr to the object
TEST(ObjectBase, AssignNullptr) {
    int refcount = 1;

    Object obj(&refcount);
    ASSERT_EQ(refcount, 2);

    obj = nullptr;
    ASSERT_EQ(refcount, 1);
}
