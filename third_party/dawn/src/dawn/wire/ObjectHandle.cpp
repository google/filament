// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/wire/ObjectHandle.h"

#include "dawn/common/Assert.h"

namespace dawn::wire {

ObjectHandle::ObjectHandle() = default;
ObjectHandle::ObjectHandle(ObjectId objId, ObjectGeneration objGeneration)
    : Handle{objId, objGeneration} {
    DAWN_ASSERT(id != 0);
}

ObjectHandle::ObjectHandle(const volatile ObjectHandle& rhs) : Handle{rhs.id, rhs.generation} {}
ObjectHandle& ObjectHandle::operator=(const volatile ObjectHandle& rhs) {
    id = rhs.id;
    generation = rhs.generation;
    return *this;
}

ObjectHandle::ObjectHandle(const ObjectHandle& rhs) = default;
ObjectHandle& ObjectHandle::operator=(const ObjectHandle& rhs) = default;

ObjectHandle::ObjectHandle(const Handle& rhs) : Handle{rhs.id, rhs.generation} {}

ObjectHandle& ObjectHandle::AssignFrom(const ObjectHandle& rhs) {
    id = rhs.id;
    generation = rhs.generation;
    return *this;
}
ObjectHandle& ObjectHandle::AssignFrom(const volatile ObjectHandle& rhs) {
    id = rhs.id;
    generation = rhs.generation;
    return *this;
}

bool ObjectHandle::IsValid() const {
    return id > 0;
}

}  // namespace dawn::wire
