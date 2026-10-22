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

#ifndef SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_
#define SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_

#include <webgpu/webgpu.h>

#include "dawn/wire/ObjectType_autogen.h"
#include "dawn/wire/client/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/RefCounted.h"
#include "src/dawn/wire/ObjectHandle.h"
#include "src/dawn/wire/client/EventManager.h"

namespace dawn::wire::client {

class Client;
class ClientBase;

struct ObjectBaseParams {
    raw_ptr<Client> client;
    ObjectHandle handle;
};

// All objects on the client side have:
//  - A pointer to the Client to get where to serialize commands
//  - The external reference count, starting at 1.
//  - An ID that is used to refer to this object when talking with the server side
class ObjectBase : public RefCounted {
  public:
    explicit ObjectBase(const ObjectBaseParams& params);

    virtual ObjectType GetObjectType() const = 0;

    // Objects are assumed to be registered with the wire on creation but can be unregistered
    // with Unregister(). After that is done, other ObjectBase getters are invalid to use.
    bool IsRegistered() const;
    void Unregister();

    const ObjectHandle& GetWireHandle(const ClientBase* forClient) const;
    Client* GetClient() const;

  protected:
    void DeleteThis() override;

  private:
    raw_ptr<Client> mClient;
    const ObjectHandle mHandle;
};

class Instance;

// Compositable functionality for objects on the client side that need to have access to the event
// manager.
class ObjectWithEventsBase : public ObjectBase {
  public:
    ObjectWithEventsBase(const ObjectBaseParams& params, Ref<Instance> instance);

    Ref<Instance> GetInstance() const;
    EventManager& GetEventManager() const;

  private:
    Ref<Instance> mInstance;
};

// TODO(https://crbug.com/526537254): Remove ReturnToAPI2 and replace ReturnToAPI with it once all
// callsites are updated.
template <class T>
auto ReturnToAPI(Ref<T>&& object) {
    if constexpr (T::HasExternalRefCount) {
        // For an object which has external ref count, just need to increase the external ref count,
        // and keep the total ref count unchanged.
        object->IncrementExternalRefCount();
    }
    return ToAPI(object.Detach());
}
template <class T>
T* ReturnToAPI2(Ref<T>&& object) {
    if constexpr (T::HasExternalRefCount) {
        // For an object which has external ref count, just need to increase the external ref count,
        // and keep the total ref count unchanged.
        object->IncrementExternalRefCount();
    }
    return object.Detach();
}

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_OBJECTBASE_H_
