// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/native/SharedResourceMemory.h"

#include <algorithm>
#include <utility>

#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Queue.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

SharedResource::~SharedResource() = default;

SharedResourceMemoryContents* SharedResource::GetSharedResourceMemoryContents() const {
    return mSharedResourceMemoryContents.Get();
}

SharedResourceMemory::SharedResourceMemory(DeviceBase* device,
                                           ObjectBase::ErrorTag tag,
                                           StringView label)
    : ApiObjectBase(device, tag, label),
      mContents(new SharedResourceMemoryContents(GetWeakRef(this))) {}

SharedResourceMemory::~SharedResourceMemory() = default;

void SharedResourceMemory::DestroyImpl() {}

bool SharedResourceMemoryContents::HasWriteAccess() const {
    return mSharedResourceAccessState == SharedResourceAccessState::Write;
}

bool SharedResourceMemoryContents::HasExclusiveReadAccess() const {
    return mSharedResourceAccessState == SharedResourceAccessState::ExclusiveRead;
}

int SharedResourceMemoryContents::GetReadAccessCount() const {
    return mReadAccessCount;
}

bool SharedResourceMemoryContents::HasAccess() const {
    return mSharedResourceAccessState != SharedResourceAccessState::NotAccessed;
}

void SharedResourceMemory::Initialize() {
    DAWN_ASSERT(!IsError());
    mContents = CreateContents();
}

Ref<SharedResourceMemoryContents> SharedResourceMemory::CreateContents() {
    return AcquireRef(new SharedResourceMemoryContents(GetWeakRef(this)));
}

SharedResourceMemoryContents* SharedResourceMemory::GetContents() const {
    return mContents.Get();
}

MaybeError SharedResourceMemory::ValidateResourceCreatedFromSelf(SharedResource* resource) {
    auto* contents = resource->GetSharedResourceMemoryContents();
    DAWN_INVALID_IF(contents == nullptr, "%s was not created from %s.", resource, this);

    auto* sharedResourceMemory =
        resource->GetSharedResourceMemoryContents()->GetSharedResourceMemory().Promote().Get();
    DAWN_INVALID_IF(sharedResourceMemory != this, "%s created from %s cannot be used with %s.",
                    resource, sharedResourceMemory, this);
    return {};
}

wgpu::Status SharedResourceMemory::APIBeginAccess(
    TextureBase* texture,
    const SharedTextureMemoryBeginAccessDescriptor* descriptor) {
    return GetDevice()->ConsumedError(BeginAccess(texture, descriptor),
                                      "calling %s.BeginAccess(%s).", this, texture)
               ? wgpu::Status::Error
               : wgpu::Status::Success;
}

wgpu::Status SharedResourceMemory::APIBeginAccess(
    BufferBase* buffer,
    const SharedBufferMemoryBeginAccessDescriptor* descriptor) {
    return GetDevice()->ConsumedError(BeginAccess(buffer, descriptor),
                                      "calling %s.BeginAccess(%s).", this, buffer)
               ? wgpu::Status::Error
               : wgpu::Status::Success;
}

template <typename Resource, typename BeginAccessDescriptor>
MaybeError SharedResourceMemory::BeginAccess(Resource* resource,
                                             const BeginAccessDescriptor* rawDescriptor) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(resource));

    UnpackedPtr<BeginAccessDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(rawDescriptor));

    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        DAWN_TRY(GetDevice()->ValidateObject(descriptor->fences[i]));
    }

    DAWN_TRY(ValidateResourceCreatedFromSelf(resource));

    DAWN_INVALID_IF(resource->IsDestroyed(), "%s has been destroyed.", resource);
    DAWN_INVALID_IF(resource->HasAccess(), "%s is already used to access %s.", resource, this);

    if constexpr (std::is_same_v<Resource, TextureBase>) {
        DAWN_INVALID_IF(resource->GetFormat().IsMultiPlanar() && !descriptor->initialized,
                        "%s with multiplanar format (%s) must be initialized.", resource,
                        resource->GetFormat().format);

        DAWN_INVALID_IF(mContents->HasWriteAccess(), "%s is currently accessed for writing.", this);
        DAWN_INVALID_IF(mContents->HasExclusiveReadAccess(),
                        "%s is currently accessed for exclusive reading.", this);

        if (static_cast<TextureBase*>(resource)->IsReadOnly()) {
            if (descriptor->concurrentRead) {
                DAWN_ASSERT(!mExclusiveAccess);
                DAWN_INVALID_IF(!descriptor->initialized, "Concurrent reading an uninitialized %s.",
                                resource);
                ++mContents->mReadAccessCount;
                mContents->mSharedResourceAccessState = SharedResourceAccessState::SimultaneousRead;

            } else {
                DAWN_INVALID_IF(
                    mContents->mReadAccessCount != 0,
                    "Exclusive read access used while %s is currently accessed for reading.", this);
                mContents->mSharedResourceAccessState = SharedResourceAccessState::ExclusiveRead;
                mExclusiveAccess = resource;
            }
        } else {
            DAWN_INVALID_IF(descriptor->concurrentRead, "Concurrent reading read-write %s.",
                            resource);
            DAWN_INVALID_IF(mContents->mReadAccessCount != 0,
                            "Read-Write access used while %s is currently accessed for reading.",
                            this);
            mContents->mSharedResourceAccessState = SharedResourceAccessState::Write;
            mExclusiveAccess = resource;
        }
    } else if constexpr (std::is_same_v<Resource, BufferBase>) {
        DAWN_INVALID_IF(mExclusiveAccess != nullptr,
                        "Cannot begin access with %s on %s which is currently accessed by %s.",
                        resource, this, mExclusiveAccess.Get());
        mContents->mSharedResourceAccessState = SharedResourceAccessState::Write;
        mExclusiveAccess = resource;
    }

    DAWN_TRY(BeginAccessImpl(resource, descriptor));

    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        // Add the fences to mPendingFences if they are not already contained in the list.
        // This loop is O(n*m), but there shouldn't be very many fences.
        auto it = std::find_if(
            mContents->mPendingFences.begin(), mContents->mPendingFences.end(),
            [&](const auto& fence) { return fence.object.Get() == descriptor->fences[i]; });
        if (it != mContents->mPendingFences.end()) {
            it->signaledValue = std::max(it->signaledValue, descriptor->signaledValues[i]);
            continue;
        }
        mContents->mPendingFences.push_back({descriptor->fences[i], descriptor->signaledValues[i]});
    }

    DAWN_ASSERT(!resource->IsError());
    resource->OnBeginAccess();
    resource->SetInitialized(descriptor->initialized);
    return {};
}

wgpu::Status SharedResourceMemory::APIEndAccess(TextureBase* texture,
                                                SharedTextureMemoryEndAccessState* state) {
    return GetDevice()->ConsumedError(EndAccess(texture, state), "calling %s.EndAccess(%s).", this,
                                      texture)
               ? wgpu::Status::Error
               : wgpu::Status::Success;
}

wgpu::Status SharedResourceMemory::APIEndAccess(BufferBase* buffer,
                                                SharedBufferMemoryEndAccessState* state) {
    return GetDevice()->ConsumedError(EndAccess(buffer, state), "calling %s.EndAccess(%s).", this,
                                      buffer)
               ? wgpu::Status::Error
               : wgpu::Status::Success;
}

MaybeError SharedResourceMemory::BeginAccessImpl(
    TextureBase* texture,
    const UnpackedPtr<SharedTextureMemoryBeginAccessDescriptor>& descriptor) {
    DAWN_UNREACHABLE();
}

MaybeError SharedResourceMemory::BeginAccessImpl(
    BufferBase* buffer,
    const UnpackedPtr<SharedBufferMemoryBeginAccessDescriptor>& descriptor) {
    DAWN_UNREACHABLE();
}

ResultOrError<FenceAndSignalValue> SharedResourceMemory::EndAccessImpl(
    TextureBase* texture,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<SharedTextureMemoryEndAccessState>& state) {
    DAWN_UNREACHABLE();
}

ResultOrError<FenceAndSignalValue> SharedResourceMemory::EndAccessImpl(
    BufferBase* buffer,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<SharedBufferMemoryEndAccessState>& state) {
    DAWN_UNREACHABLE();
}

bool SharedResourceMemory::APIIsDeviceLost() const {
    return GetDevice()->IsLost();
}

template <typename Resource, typename EndAccessState>
MaybeError SharedResourceMemory::EndAccess(Resource* resource, EndAccessState* state) {
    DAWN_TRY(GetDevice()->ValidateObject(resource));
    DAWN_TRY(ValidateResourceCreatedFromSelf(resource));

    DAWN_INVALID_IF(!resource->HasAccess(), "%s is not currently being accessed.", resource);
    if constexpr (std::is_same_v<Resource, TextureBase>) {
        if (static_cast<TextureBase*>(resource)->IsReadOnly()) {
            DAWN_ASSERT(!mContents->HasWriteAccess());
            if (mContents->HasExclusiveReadAccess()) {
                DAWN_ASSERT(mContents->mReadAccessCount == 0);
                mContents->mSharedResourceAccessState = SharedResourceAccessState::NotAccessed;
                mExclusiveAccess = nullptr;
            } else {
                DAWN_ASSERT(mContents->mSharedResourceAccessState ==
                            SharedResourceAccessState::SimultaneousRead);
                DAWN_ASSERT(mExclusiveAccess == nullptr);
                --mContents->mReadAccessCount;
                if (mContents->mReadAccessCount == 0) {
                    mContents->mSharedResourceAccessState = SharedResourceAccessState::NotAccessed;
                }
            }
        } else {
            DAWN_ASSERT(mContents->mSharedResourceAccessState == SharedResourceAccessState::Write);
            DAWN_ASSERT(mContents->mReadAccessCount == 0);
            mContents->mSharedResourceAccessState = SharedResourceAccessState::NotAccessed;
            mExclusiveAccess = nullptr;
        }
    } else if constexpr (std::is_same_v<Resource, BufferBase>) {
        DAWN_INVALID_IF(
            static_cast<BufferBase*>(resource)->APIGetMapState() != wgpu::BufferMapState::Unmapped,
            "%s is currently mapped or pending map.", resource);
        DAWN_INVALID_IF(mExclusiveAccess != resource,
                        "Cannot end access with %s on %s which is currently accessed by %s.",
                        resource, this, mExclusiveAccess.Get());
        mContents->mSharedResourceAccessState = SharedResourceAccessState::NotAccessed;
        mExclusiveAccess = nullptr;
    }

    PendingFenceList fenceList;
    // The state transitions to NotAccessed if the exclusive access ends, or the last concurrent
    // read ends. When this occurs, acquire any pending fences that may remain. This occurs if
    // the accesses never acquired them.
    if (mContents->mSharedResourceAccessState == SharedResourceAccessState::NotAccessed) {
        mContents->AcquirePendingFences(&fenceList);
    }

    DAWN_ASSERT(!resource->IsError());
    ExecutionSerial lastUsageSerial = resource->OnEndAccess();

    // If the last usage serial is non-zero, the texture was used.
    // Call the error-generating part of the EndAccess implementation to export a fence.
    // This is separated out because writing the output state must happen regardless of whether
    // or not EndAccessInternal succeeds.
    MaybeError err;
    if (lastUsageSerial != kBeginningOfGPUTime) {
        ResultOrError<FenceAndSignalValue> result =
            EndAccessInternal(lastUsageSerial, resource, state);
        if (result.IsSuccess()) {
            FenceAndSignalValue fence = result.AcquireSuccess();
            // Some backends might not support fence, in those case, a null object might be
            // returned. So skip it.
            if (fence.object) {
                fenceList.push_back(fence);
            }
        } else {
            err = result.AcquireError();
        }
    }

    // Copy the fences to the output state.
    if (size_t fenceCount = fenceList.size()) {
        auto* fences = new SharedFenceBase*[fenceCount];
        uint64_t* signaledValues = new uint64_t[fenceCount];
        for (size_t i = 0; i < fenceCount; ++i) {
            fences[i] = ReturnToAPI(std::move(fenceList[i].object));
            signaledValues[i] = fenceList[i].signaledValue;
        }

        state->fenceCount = fenceCount;
        state->fences = fences;
        state->signaledValues = signaledValues;
    } else {
        state->fenceCount = 0;
        state->fences = nullptr;
        state->signaledValues = nullptr;
    }
    state->initialized = resource->IsInitialized();
    return err;
}

template <typename Resource, typename EndAccessState>
ResultOrError<FenceAndSignalValue> SharedResourceMemory::EndAccessInternal(
    ExecutionSerial lastUsageSerial,
    Resource* resource,
    EndAccessState* rawState) {
    UnpackedPtr<EndAccessState> state;
    DAWN_TRY_ASSIGN(state, ValidateAndUnpack(rawState));
    // Ensure that commands are submitted before exporting fences with the last usage serial.
    DAWN_TRY(GetDevice()->GetQueue()->EnsureCommandsFlushed(lastUsageSerial));
    return EndAccessImpl(resource, lastUsageSerial, state);
}

// SharedResourceMemoryContents

SharedResourceMemoryContents::SharedResourceMemoryContents(
    WeakRef<SharedResourceMemory> sharedResourceMemory)
    : mSharedResourceMemory(std::move(sharedResourceMemory)) {}

const WeakRef<SharedResourceMemory>& SharedResourceMemoryContents::GetSharedResourceMemory() const {
    return mSharedResourceMemory;
}

void SharedResourceMemoryContents::AcquirePendingFences(PendingFenceList* fences) {
    *fences = mPendingFences;
    mPendingFences.clear();
}

}  // namespace dawn::native
