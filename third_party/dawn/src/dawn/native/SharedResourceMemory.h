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

#ifndef SRC_DAWN_NATIVE_SHAREDRESOURCEMEMORY_H_
#define SRC_DAWN_NATIVE_SHAREDRESOURCEMEMORY_H_

#include "absl/container/inlined_vector.h"
#include "dawn/common/WeakRef.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class SharedResourceMemoryContents;

enum SharedResourceAccessState { NotAccessed, ExclusiveRead, SimultaneousRead, Write };

// The interface for the resources that can be created from a SharedResourceMemory (SRM).
// Note that in practice this is only Buffer and Texture and that some internals of SRM assume that
// with static polymorphism.
class SharedResource : public ApiObjectBase {
  public:
    using ApiObjectBase::ApiObjectBase;
    ~SharedResource() override;

    SharedResourceMemoryContents* GetSharedResourceMemoryContents() const;

    // Set the resource state to allow access.
    virtual void OnBeginAccess() = 0;
    // Set the resource state to disallow access, and return the last usage serial.
    virtual ExecutionSerial OnEndAccess() = 0;
    // Check whether the resource may be accessed.
    virtual bool HasAccess() const = 0;
    virtual bool IsDestroyed() const = 0;
    virtual void SetInitialized(bool initialized) = 0;
    virtual bool IsInitialized() const = 0;

  protected:
    // The shared contents the resource was created from. May be null.
    Ref<SharedResourceMemoryContents> mSharedResourceMemoryContents;
};

class SharedResourceMemory : public ApiObjectBase, public WeakRefSupport<SharedResourceMemory> {
  public:
    using PendingFenceList = absl::InlinedVector<FenceAndSignalValue, 1>;

    ~SharedResourceMemory() override;
    void Initialize();
    void DestroyImpl() override;

    // Returns true if access was acquired. If it returns true, then APIEndAccess must
    // be called to release access. Other errors may occur even if `true` is returned.
    // Use an error scope to catch them.
    wgpu::Status APIBeginAccess(TextureBase* texture,
                                const SharedTextureMemoryBeginAccessDescriptor* descriptor);
    // Returns true if access was released.
    wgpu::Status APIEndAccess(TextureBase* texture, SharedTextureMemoryEndAccessState* state);

    // Returns true if access was acquired. If it returns true, then APIEndAccess must
    // be called to release access. Other errors may occur even if `true` is returned.
    // Use an error scope to catch them.
    wgpu::Status APIBeginAccess(BufferBase* buffer,
                                const SharedBufferMemoryBeginAccessDescriptor* descriptor);
    // Returns true if access was released.
    wgpu::Status APIEndAccess(BufferBase* buffer, SharedBufferMemoryEndAccessState* state);

    // Returns true iff the device passed to this object on creation is now lost.
    // TODO(crbug.com/1506468): Eliminate this API once Chromium has been
    // transitioned away from using it in favor of observing device lost events.
    bool APIIsDeviceLost() const;

    SharedResourceMemoryContents* GetContents() const;

  protected:
    SharedResourceMemory(DeviceBase* device, ObjectBase::ErrorTag, StringView label);
    using ApiObjectBase::ApiObjectBase;

  private:
    virtual Ref<SharedResourceMemoryContents> CreateContents();

    // Validate that the resource was created from this SharedResourceMemory.
    MaybeError ValidateResourceCreatedFromSelf(SharedResource* resource);

    template <typename Resource, typename BeginAccessDescriptor>
    MaybeError BeginAccess(Resource* resource, const BeginAccessDescriptor* rawDescriptor);

    template <typename Resource, typename EndAccessState>
    MaybeError EndAccess(Resource* resource, EndAccessState* state);

    template <typename Resource, typename EndAccessState>
    ResultOrError<FenceAndSignalValue> EndAccessInternal(ExecutionSerial lastUsageSerial,
                                                         Resource* resource,
                                                         EndAccessState* rawState);

    // BeginAccessImpl validates the operation is valid on the backend, and performs any
    // backend specific operations. It does NOT need to acquire begin fences; that is done in the
    // frontend in BeginAccess.
    virtual MaybeError BeginAccessImpl(
        TextureBase* texture,
        const UnpackedPtr<SharedTextureMemoryBeginAccessDescriptor>& descriptor);
    virtual MaybeError BeginAccessImpl(
        BufferBase* buffer,
        const UnpackedPtr<SharedBufferMemoryBeginAccessDescriptor>& descriptor);
    // EndAccessImpl validates the operation is valid on the backend, and returns the end fence.
    // It should also write out any backend specific state in chained out structs of EndAccessState.
    virtual ResultOrError<FenceAndSignalValue> EndAccessImpl(
        TextureBase* texture,
        ExecutionSerial lastUsageSerial,
        UnpackedPtr<SharedTextureMemoryEndAccessState>& state);
    virtual ResultOrError<FenceAndSignalValue> EndAccessImpl(
        BufferBase* buffer,
        ExecutionSerial lastUsageSerial,
        UnpackedPtr<SharedBufferMemoryEndAccessState>& state);

    // If non-null, the SRM is exclusively accessed from that SR, used for validation.
    Ref<SharedResource> mExclusiveAccess;
    Ref<SharedResourceMemoryContents> mContents;
};

// SharedResourceMemoryContents is a separate object because it needs to live as long as
// the SharedResourceMemory or any resources created from the SharedResourceMemory. This
// allows state and objects needed by the resource to persist after the
// SharedResourceMemory itself has been dropped.
class SharedResourceMemoryContents : public RefCounted {
  public:
    using PendingFenceList = SharedResourceMemory::PendingFenceList;

    explicit SharedResourceMemoryContents(WeakRef<SharedResourceMemory> sharedResourceMemory);

    void AcquirePendingFences(PendingFenceList* fences);

    const WeakRef<SharedResourceMemory>& GetSharedResourceMemory() const;

    bool HasWriteAccess() const;
    bool HasExclusiveReadAccess() const;
    int GetReadAccessCount() const;

  private:
    friend class SharedResourceMemory;

    // The fences that must be waited on before the next use of the resource, whether that use is
    // internal to Dawn or external (when exporting on EndAccess).
    PendingFenceList mPendingFences;

    SharedResourceAccessState mSharedResourceAccessState = SharedResourceAccessState::NotAccessed;
    int mReadAccessCount = 0;

    // A pointer to the parent SRM that's a weak to prevent potential ref cycles.
    WeakRef<SharedResourceMemory> mSharedResourceMemory;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDRESOURCEMEMORY_H_
