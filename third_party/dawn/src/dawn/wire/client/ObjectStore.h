// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_OBJECTSTORE_H_
#define SRC_DAWN_WIRE_CLIENT_OBJECTSTORE_H_

#include <memory>
#include <vector>

#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class ClientBase;

// A helper class used in Client, ObjectStore owns the association of some ObjectBase and
// ObjectHandles. The lifetime of the ObjectBase is then owned by the ObjectStore, destruction
// happening when Free is called.
//
// Since the wire has one "ID" namespace per type of object, each ObjectStore should contain a
// single type of objects. However no templates are used because Client wraps ObjectStore and is
// type-generic, so ObjectStore is type-erased to only work on ObjectBase.
class ObjectStore {
  public:
    ObjectHandle ReserveHandle();
    void Insert(ObjectBase* obj, const ClientBase* forClient);
    void Remove(ObjectBase* obj, const ClientBase* forClient);

    ObjectBase* Get(ObjectId id) const;

    // This returns a copy of the list of all objects so that we can iterate the objects without
    // holding the lock. This is important because objects can call |Unregister| which will try to
    // remove itself from the source-of-truth list behind the lock.
    std::vector<raw_ptr<ObjectBase>> GetAllObjects() const;

  private:
    struct State {
        uint32_t currentId = 1;
        std::vector<ObjectHandle> freeHandles;
        std::vector<raw_ptr<ObjectBase>> objects = {nullptr};
    };
    MutexRWProtected<State> mState;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_OBJECTSTORE_H_
