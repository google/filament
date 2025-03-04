// Copyright 2018 The Dawn & Tint Authors
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

#include <atomic>
#include <mutex>
#include <utility>

#include "absl/strings/str_format.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

static constexpr uint64_t kErrorPayload = 0;
static constexpr uint64_t kNotErrorPayload = 1;

ErrorMonad::ErrorMonad() : RefCounted(kNotErrorPayload) {}
ErrorMonad::ErrorMonad(ErrorTag) : RefCounted(kErrorPayload) {}

bool ErrorMonad::IsError() const {
    return GetRefCountPayload() == kErrorPayload;
}

ObjectBase::ObjectBase(DeviceBase* device) : ErrorMonad(), mDevice(device) {}

ObjectBase::ObjectBase(DeviceBase* device, ErrorTag) : ErrorMonad(kError), mDevice(device) {}

InstanceBase* ObjectBase::GetInstance() const {
    return mDevice->GetInstance();
}

DeviceBase* ObjectBase::GetDevice() const {
    return mDevice.Get();
}

void ApiObjectList::Track(ApiObjectBase* object) {
    if (mMarkedDestroyed.load(std::memory_order_acquire)) {
        object->DestroyImpl();
        return;
    }
    mObjects.Use([&object](auto lockedObjects) { lockedObjects->Prepend(object); });
}

bool ApiObjectList::Untrack(ApiObjectBase* object) {
    return mObjects.Use([&object](auto lockedObjects) { return object->RemoveFromList(); });
}

void ApiObjectList::Destroy() {
    LinkedList<ApiObjectBase> objects;
    mObjects.Use([&objects, this](auto lockedObjects) {
        mMarkedDestroyed.store(true, std::memory_order_release);
        lockedObjects->MoveInto(&objects);
    });
    while (!objects.empty()) {
        auto* head = objects.head();
        bool removed = head->RemoveFromList();
        DAWN_ASSERT(removed);
        head->value()->DestroyImpl();
    }
}

ApiObjectBase::ApiObjectBase(DeviceBase* device, StringView label) : ObjectBase(device) {
    mLabel = std::string(label);
}

ApiObjectBase::ApiObjectBase(DeviceBase* device, ErrorTag tag, StringView label)
    : ObjectBase(device, tag) {
    mLabel = std::string(label);
}

ApiObjectBase::ApiObjectBase(DeviceBase* device, LabelNotImplementedTag tag) : ObjectBase(device) {}

ApiObjectBase::~ApiObjectBase() {
    DAWN_ASSERT(!IsAlive());
}

void ApiObjectBase::APISetLabel(StringView label) {
    SetLabel(std::string(utils::NormalizeMessageString(label)));
}

void ApiObjectBase::SetLabel(std::string label) {
    mLabel = std::move(label);
    SetLabelImpl();
}

const std::string& ApiObjectBase::GetLabel() const {
    return mLabel;
}

void ApiObjectBase::FormatLabel(absl::FormatSink* s) const {
    s->Append(ObjectTypeAsString(GetType()));
    if (!mLabel.empty()) {
        s->Append(absl::StrFormat(" \"%s\"", mLabel));
    } else {
        s->Append(" (unlabeled)");
    }
}

void ApiObjectBase::SetLabelImpl() {}

bool ApiObjectBase::IsAlive() const {
    return IsInList();
}

void ApiObjectBase::DeleteThis() {
    Destroy();
    RefCounted::DeleteThis();
}

void ApiObjectBase::LockAndDeleteThis() {
    auto deviceLock(GetDevice()->GetScopedLockSafeForDelete());
    DeleteThis();
}

ApiObjectList* ApiObjectBase::GetObjectTrackingList() {
    DAWN_ASSERT(GetDevice() != nullptr);
    return GetDevice()->GetObjectTrackingList(GetType());
}

void ApiObjectBase::Destroy() {
    ApiObjectList* list = GetObjectTrackingList();
    DAWN_ASSERT(list != nullptr);
    if (list->Untrack(this)) {
        DestroyImpl();
    }
}

}  // namespace dawn::native
