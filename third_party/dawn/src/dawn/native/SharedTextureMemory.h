// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_
#define SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_

#include "dawn/common/WeakRef.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/SharedResourceMemory.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class SharedTextureMemoryContents;
class SharedResourceMemoryContents;
struct SharedTextureMemoryDescriptor;
struct SharedTextureMemoryBeginAccessDescriptor;
struct SharedTextureMemoryEndAccessState;
struct SharedTextureMemoryProperties;
struct TextureDescriptor;

class SharedTextureMemoryBase : public SharedResourceMemory {
  public:
    using BeginAccessDescriptor = SharedTextureMemoryBeginAccessDescriptor;
    using EndAccessState = SharedTextureMemoryEndAccessState;

    static Ref<SharedTextureMemoryBase> MakeError(DeviceBase* device,
                                                  const SharedTextureMemoryDescriptor* descriptor);

    wgpu::Status APIGetProperties(SharedTextureMemoryProperties* properties) const;
    TextureBase* APICreateTexture(const TextureDescriptor* descriptor);

    ObjectType GetType() const override;

    SharedTextureMemoryContents* GetContents() const;

  protected:
    SharedTextureMemoryBase(DeviceBase* device,
                            StringView label,
                            const SharedTextureMemoryProperties& properties);
    SharedTextureMemoryBase(DeviceBase* device,
                            const SharedTextureMemoryDescriptor* descriptor,
                            ObjectBase::ErrorTag tag);

    MaybeError GetProperties(SharedTextureMemoryProperties* properties) const;

  private:
    ResultOrError<Ref<TextureBase>> CreateTexture(const TextureDescriptor* rawDescriptor);

    Ref<SharedResourceMemoryContents> CreateContents() override;

    virtual ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) = 0;

    virtual MaybeError GetChainedProperties(
        UnpackedPtr<SharedTextureMemoryProperties>& properties) const {
        return {};
    }

    SharedTextureMemoryProperties mProperties;
};

class SharedTextureMemoryContents : public SharedResourceMemoryContents {
  public:
    explicit SharedTextureMemoryContents(WeakRef<SharedTextureMemoryBase> sharedTextureMemory);

    SampleTypeBit GetExternalFormatSupportedSampleTypes() const;
    void SetExternalFormatSupportedSampleTypes(SampleTypeBit supportedSampleType);

  private:
    friend class SharedTextureMemoryBase;

    SampleTypeBit mSupportedExternalSampleTypes;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_
