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

#ifndef SRC_DAWN_COMMON_NSREF_H_
#define SRC_DAWN_COMMON_NSREF_H_

#include "dawn/common/RefBase.h"

#import <Foundation/NSObject.h>

#if !defined(__OBJC__)
#error "NSRef can only be used in Objective C/C++ code."
#endif

namespace dawn {

// This file contains smart pointers that automatically reference and release Objective C objects
// and prototocals in a manner very similar to Ref<>. Note that NSRef<> and NSPRef's constructor add
// a reference to the object by default, so the pattern to get a reference for a newly created
// NSObject is the following:
//
//    NSRef<NSFoo> foo = AcquireNSRef([NSFoo alloc]);
//
// NSRef overloads -> and * but these operators don't work extremely well with Objective C's
// features. For example automatic dereferencing when doing the following doesn't work:
//
//    NSFoo* foo;
//    foo.member = 1;
//    someVar = foo.member;
//
// Instead use the message passing syntax:
//
//    NSRef<NSFoo> foo;
//    [*foo setMember: 1];
//    someVar = [*foo member];
//
// Also did you notive the extra '*' in the example above? That's because Objective C's message
// passing doesn't automatically call a C++ operator to dereference smart pointers (like -> does) so
// we have to dereference manually using '*'. In some cases the extra * or message passing syntax
// can get a bit annoying so instead a local "naked" pointer can be borrowed from the NSRef. This
// would change the syntax overload in the following:
//
//    NSRef<NSFoo> foo;
//    [*foo setA:1];
//    [*foo setB:2];
//    [*foo setC:3];
//
// Into (note access to members of ObjC classes referenced via pointer is done with . and not ->):
//
//    NSRef<NSFoo> fooRef;
//    NSFoo* foo = fooRef.Get();
//    foo.a = 1;
//    foo.b = 2;
//    boo.c = 3;
//
// Which can be subjectively easier to read.

template <typename T>
struct NSRefTraits {
    static constexpr T kNullValue = nullptr;
    static void AddRef(T value) { [value retain]; }
    static void Release(T value) { [value release]; }
};

template <typename T>
class NSRef : public RefBase<T*, NSRefTraits<T*>> {
  public:
    using RefBase<T*, NSRefTraits<T*>>::RefBase;

    const T* operator*() const { return this->Get(); }

    T* operator*() { return this->Get(); }
};

template <typename T>
NSRef<T> AcquireNSRef(T* pointee) {
    NSRef<T> ref;
    ref.Acquire(pointee);
    return ref;
}

// This is a RefBase<> for an Objective C protocol (hence the P). Objective C protocols must always
// be referenced with id<ProtocolName> and not just ProtocolName* so they cannot use NSRef<>
// itself. That's what the P in NSPRef stands for: Protocol.
template <typename T>
class NSPRef : public RefBase<T, NSRefTraits<T>> {
  public:
    using RefBase<T, NSRefTraits<T>>::RefBase;

    const T operator*() const { return this->Get(); }

    T operator*() { return this->Get(); }
};

template <typename T>
NSPRef<T> AcquireNSPRef(T pointee) {
    NSPRef<T> ref;
    ref.Acquire(pointee);
    return ref;
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_NSREF_H_
