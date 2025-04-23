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

#include "dawn/native/opengl/SharedTextureMemoryGL.h"

#include <utility>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/QueueGL.h"
#include "dawn/native/opengl/SharedFenceGL.h"
#include "dawn/native/opengl/TextureGL.h"

namespace dawn::native::opengl {

namespace {
ResultOrError<wgpu::SharedFenceType> ChooseFenceTypeFromFeatures(Device* device) {
    if (device->HasFeature(Feature::SharedFenceSyncFD)) {
        return wgpu::SharedFenceType::SyncFD;
    } else if (device->HasFeature(Feature::SharedFenceEGLSync)) {
        return wgpu::SharedFenceType::EGLSync;
    } else {
        return DAWN_VALIDATION_ERROR("No enabled features for SharedFence creation.");
    }
}
}  // namespace

SharedTextureMemory::SharedTextureMemory(Device* device,
                                         StringView label,
                                         const SharedTextureMemoryProperties& properties)
    : SharedTextureMemoryBase(device, label, properties) {}

ResultOrError<Ref<TextureBase>> SharedTextureMemory::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return Texture::CreateFromSharedTextureMemory(this, descriptor);
}

MaybeError SharedTextureMemory::BeginAccessImpl(
    TextureBase* texture,
    const UnpackedPtr<BeginAccessDescriptor>& descriptor) {
    DAWN_TRY(descriptor.ValidateSubset<>());
    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        SharedFenceBase* fence = descriptor->fences[i];

        SharedFenceExportInfo exportInfo;
        DAWN_TRY(fence->ExportInfo(&exportInfo));
        switch (exportInfo.type) {
            case wgpu::SharedFenceType::SyncFD:
                DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceSyncFD),
                                "Required feature (%s) for %s is missing.",
                                wgpu::FeatureName::SharedFenceSyncFD,
                                wgpu::SharedFenceType::SyncFD);
                break;
            case wgpu::SharedFenceType::EGLSync:
                DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceEGLSync),
                                "Required feature (%s) for %s is missing.",
                                wgpu::FeatureName::SharedFenceEGLSync,
                                wgpu::SharedFenceType::EGLSync);
                break;
            default:
                return DAWN_VALIDATION_ERROR("Unsupported fence type %s.", exportInfo.type);
        }

        // All GL sync objects are binary.
        DAWN_INVALID_IF(descriptor->signaledValues[i] != 1, "%s signaled value (%u) was not 1.",
                        fence, descriptor->signaledValues[i]);
    }

    return {};
}

ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(
    TextureBase* texture,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<EndAccessState>& state) {
    DAWN_TRY(state.ValidateSubset<>());

    wgpu::SharedFenceType fenceType;
    DAWN_TRY_ASSIGN(fenceType, ChooseFenceTypeFromFeatures(ToBackend(GetDevice())));

    Ref<SharedFence> fence;
    DAWN_TRY_ASSIGN(
        fence,
        ToBackend(GetDevice()->GetQueue())->GetOrCreateSharedFence(lastUsageSerial, fenceType));
    return FenceAndSignalValue{std::move(fence), 1};
}

}  // namespace dawn::native::opengl
