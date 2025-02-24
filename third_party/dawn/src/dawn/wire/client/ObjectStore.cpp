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

#include "dawn/wire/client/ObjectStore.h"

#include <limits>
#include <utility>

namespace dawn::wire::client {

ObjectStore::ObjectStore() {
    // ID 0 is nullptr
    mObjects.emplace_back(nullptr);
    mCurrentId = 1;
}

ObjectHandle ObjectStore::ReserveHandle() {
    if (mFreeHandles.empty()) {
        return {mCurrentId++, 0};
    }
    ObjectHandle handle = mFreeHandles.back();
    mFreeHandles.pop_back();
    return handle;
}

void ObjectStore::Insert(ObjectBase* obj) {
    ObjectId id = obj->GetWireId();

    if (id >= mObjects.size()) {
        DAWN_ASSERT(id == mObjects.size());
        mObjects.emplace_back(obj);
    } else {
        // The generation should never overflow. We don't recycle ObjectIds that would
        // overflow their next generation.
        DAWN_ASSERT(obj->GetWireGeneration() != 0);
        DAWN_ASSERT(mObjects[id] == nullptr);
        mObjects[id] = obj;
    }
}

void ObjectStore::Remove(ObjectBase* obj) {
    // The wire reuses ID for objects to keep them in a packed array starting from 0.
    // To avoid issues with asynchronous server->client communication referring to an ID that's
    // already reused, each handle also has a generation that's increment by one on each reuse.
    // Avoid overflows by only reusing the ID if the increment of the generation won't overflow.
    const ObjectHandle& currentHandle = obj->GetWireHandle();
    if (DAWN_LIKELY(currentHandle.generation != std::numeric_limits<ObjectGeneration>::max())) {
        mFreeHandles.push_back({currentHandle.id, currentHandle.generation + 1});
    }
    mObjects[currentHandle.id] = nullptr;
}

const std::vector<raw_ptr<ObjectBase>>& ObjectStore::GetAllObjects() const {
    return mObjects;
}

ObjectBase* ObjectStore::Get(ObjectId id) const {
    if (id >= mObjects.size()) {
        return nullptr;
    }
    return mObjects[id];
}

}  // namespace dawn::wire::client
