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

#include "dawn/wire/client/ObjectBase.h"

#include "dawn/common/Assert.h"
#include "dawn/wire/client/Client.h"

namespace dawn::wire::client {

ObjectBase::ObjectBase(const ObjectBaseParams& params)
    : mClient(params.client), mHandle(params.handle) {
    DAWN_ASSERT(IsRegistered());
}

bool ObjectBase::IsRegistered() const {
    return mClient != nullptr;
}

const ObjectHandle& ObjectBase::GetWireHandle() const {
    DAWN_ASSERT(IsRegistered());
    return mHandle;
}

ObjectId ObjectBase::GetWireId() const {
    DAWN_ASSERT(IsRegistered());
    return mHandle.id;
}

ObjectGeneration ObjectBase::GetWireGeneration() const {
    DAWN_ASSERT(IsRegistered());
    return mHandle.generation;
}

Client* ObjectBase::GetClient() const {
    DAWN_ASSERT(IsRegistered());
    return mClient;
}

void ObjectBase::DeleteThis() {
    Unregister();
    RefCounted::DeleteThis();
}

void ObjectBase::Unregister() {
    if (!IsRegistered()) {
        return;
    }

    mClient->Unregister(this, GetObjectType());
    mClient = nullptr;
}

ObjectWithEventsBase::ObjectWithEventsBase(const ObjectBaseParams& params,
                                           const ObjectHandle& eventManagerHandle)
    : ObjectBase(params), mEventManagerHandle(eventManagerHandle) {}

const ObjectHandle& ObjectWithEventsBase::GetEventManagerHandle() const {
    return mEventManagerHandle;
}

EventManager& ObjectWithEventsBase::GetEventManager() const {
    return GetClient()->GetEventManager(mEventManagerHandle);
}

}  // namespace dawn::wire::client
