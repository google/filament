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

#include "dawn/native/SharedTextureMemory.h"

#include <utility>

#include "dawn/common/WeakRef.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Queue.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

namespace {

class ErrorSharedTextureMemory : public SharedTextureMemoryBase {
  public:
    ErrorSharedTextureMemory(DeviceBase* device, const SharedTextureMemoryDescriptor* descriptor)
        : SharedTextureMemoryBase(device, descriptor, ObjectBase::kError) {}

    Ref<SharedResourceMemoryContents> CreateContents() override { DAWN_UNREACHABLE(); }
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) override {
        DAWN_UNREACHABLE();
    }
    MaybeError BeginAccessImpl(TextureBase* texture,
                               const UnpackedPtr<BeginAccessDescriptor>& descriptor) override {
        DAWN_UNREACHABLE();
    }
    ResultOrError<FenceAndSignalValue> EndAccessImpl(TextureBase* texture,
                                                     ExecutionSerial lastUsageSerial,
                                                     UnpackedPtr<EndAccessState>& state) override {
        DAWN_UNREACHABLE();
    }
    void DestroyImpl() override {}
};

}  // namespace

// static
Ref<SharedTextureMemoryBase> SharedTextureMemoryBase::MakeError(
    DeviceBase* device,
    const SharedTextureMemoryDescriptor* descriptor) {
    return AcquireRef(new ErrorSharedTextureMemory(device, descriptor));
}

SharedTextureMemoryBase::SharedTextureMemoryBase(DeviceBase* device,
                                                 const SharedTextureMemoryDescriptor* descriptor,
                                                 ObjectBase::ErrorTag tag)
    : SharedResourceMemory(device, tag, descriptor->label),
      mProperties{
          nullptr,
          wgpu::TextureUsage::None,
          {0, 0, 0},
          wgpu::TextureFormat::Undefined,
      } {}

SharedTextureMemoryBase::SharedTextureMemoryBase(DeviceBase* device,
                                                 StringView label,
                                                 const SharedTextureMemoryProperties& properties)
    : SharedResourceMemory(device, label), mProperties(properties) {
    // Reify properties to ensure we don't expose capabilities not supported by the device.
    const Format& internalFormat = device->GetValidInternalFormat(mProperties.format);
    if (internalFormat.format != wgpu::TextureFormat::External) {
        if (!internalFormat.supportsStorageUsage || internalFormat.IsMultiPlanar()) {
            mProperties.usage = mProperties.usage & ~wgpu::TextureUsage::StorageBinding;
        }
        if (!internalFormat.isRenderable ||
            (internalFormat.IsMultiPlanar() &&
             !device->HasFeature(Feature::MultiPlanarRenderTargets))) {
            mProperties.usage = mProperties.usage & ~wgpu::TextureUsage::RenderAttachment;
        }
        if (internalFormat.IsMultiPlanar() &&
            !device->HasFeature(Feature::MultiPlanarFormatExtendedUsages)) {
            mProperties.usage = mProperties.usage & ~wgpu::TextureUsage::CopyDst;
        }
    }

    GetObjectTrackingList()->Track(this);
}

ObjectType SharedTextureMemoryBase::GetType() const {
    return ObjectType::SharedTextureMemory;
}

wgpu::Status SharedTextureMemoryBase::APIGetProperties(
    SharedTextureMemoryProperties* properties) const {
    if (GetDevice()->ConsumedError(GetProperties(properties), "calling %s.GetProperties", this)) {
        return wgpu::Status::Error;
    }
    return wgpu::Status::Success;
}

MaybeError SharedTextureMemoryBase::GetProperties(SharedTextureMemoryProperties* properties) const {
    properties->usage = mProperties.usage;
    properties->size = mProperties.size;
    properties->format = mProperties.format;

    UnpackedPtr<SharedTextureMemoryProperties> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(properties));

    if (unpacked.Get<SharedTextureMemoryAHardwareBufferProperties>()) {
        DAWN_INVALID_IF(
            !GetDevice()->HasFeature(Feature::SharedTextureMemoryAHardwareBuffer),
            "SharedTextureMemory properties (%s) have a chained "
            "SharedTextureMemoryAHardwareBufferProperties without the %s feature being set.",
            this, ToAPI(Feature::SharedTextureMemoryAHardwareBuffer));
    }

    DAWN_TRY(GetChainedProperties(unpacked));

    return {};
}

TextureBase* SharedTextureMemoryBase::APICreateTexture(const TextureDescriptor* descriptor) {
    Ref<TextureBase> result;

    // Provide the defaults if no descriptor is provided.
    TextureDescriptor defaultDescriptor;
    if (descriptor == nullptr) {
        defaultDescriptor = {};
        defaultDescriptor.format = mProperties.format;
        defaultDescriptor.size = mProperties.size;
        defaultDescriptor.usage = mProperties.usage;
        descriptor = &defaultDescriptor;
    }

    if (GetDevice()->ConsumedError(CreateTexture(descriptor), &result,
                                   InternalErrorType::OutOfMemory, "calling %s.CreateTexture(%s).",
                                   this, descriptor)) {
        result = TextureBase::MakeError(GetDevice(), descriptor);
    }
    return ReturnToAPI(std::move(result));
}

ResultOrError<Ref<TextureBase>> SharedTextureMemoryBase::CreateTexture(
    const TextureDescriptor* rawDescriptor) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    UnpackedPtr<TextureDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(rawDescriptor));

    // Validate that there is one 2D, single-sampled subresource
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);
    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);
    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    // Validate that the texture size exactly matches the shared texture memory's size.
    DAWN_INVALID_IF(
        (descriptor->size.width != mProperties.size.width) ||
            (descriptor->size.height != mProperties.size.height) ||
            (descriptor->size.depthOrArrayLayers != mProperties.size.depthOrArrayLayers),
        "SharedTextureMemory size (%s) doesn't match descriptor size (%s).", &mProperties.size,
        &descriptor->size);

    // Validate that the texture format exactly matches the shared texture memory's format.
    DAWN_INVALID_IF(descriptor->format != mProperties.format,
                    "SharedTextureMemory format (%s) doesn't match descriptor format (%s).",
                    mProperties.format, descriptor->format);

    // Validate the texture descriptor, and require its usage to be a subset of the shared texture
    // memory's usage.
    DAWN_TRY(ValidateTextureDescriptor(GetDevice(), descriptor, AllowMultiPlanarTextureFormat::Yes,
                                       mProperties.usage));

    Ref<TextureBase> texture;
    DAWN_TRY_ASSIGN(texture, CreateTextureImpl(descriptor));
    // Access is started on memory.BeginAccess.
    texture->OnEndAccess();
    return texture;
}

Ref<SharedResourceMemoryContents> SharedTextureMemoryBase::CreateContents() {
    return AcquireRef(new SharedTextureMemoryContents(GetWeakRef(this)));
}

SharedTextureMemoryContents* SharedTextureMemoryBase::GetContents() const {
    return static_cast<SharedTextureMemoryContents*>(SharedResourceMemory::GetContents());
}

void APISharedTextureMemoryEndAccessStateFreeMembers(WGPUSharedTextureMemoryEndAccessState cState) {
    auto* state = reinterpret_cast<SharedTextureMemoryBase::EndAccessState*>(&cState);
    for (size_t i = 0; i < state->fenceCount; ++i) {
        state->fences[i]->APIRelease();
    }
    delete[] state->fences;
    delete[] state->signaledValues;
}

// SharedTextureMemoryContents

SharedTextureMemoryContents::SharedTextureMemoryContents(
    WeakRef<SharedTextureMemoryBase> sharedTextureMemory)
    : SharedResourceMemoryContents(sharedTextureMemory),
      mSupportedExternalSampleTypes(SampleTypeBit::None) {}

SampleTypeBit SharedTextureMemoryContents::GetExternalFormatSupportedSampleTypes() const {
    return mSupportedExternalSampleTypes;
}

void SharedTextureMemoryContents::SetExternalFormatSupportedSampleTypes(
    SampleTypeBit supportedSampleType) {
    mSupportedExternalSampleTypes = supportedSampleType;
}

}  // namespace dawn::native
