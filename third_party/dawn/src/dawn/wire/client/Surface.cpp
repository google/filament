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

#include "src/dawn/wire/client/Surface.h"

#include <algorithm>
#include <utility>

#include "dawn/wire/client/webgpu.h"
#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/client/Device.h"
#include "src/dawn/wire/client/Texture.h"
#include "src/utils/compiler.h"
#include "src/utils/log.h"
#include "src/utils/platform.h"

namespace dawn::wire::client {

Surface::Surface(const ObjectBaseParams& params, const SurfaceCapabilities* capabilities)
    : ObjectBase(params) {
    // Copy over the capabilities.
    mSupportedUsages = capabilities->usages;
    mSupportedFormats.assign(capabilities->formats.begin(), capabilities->formats.end());
    mSupportedPresentModes.assign(capabilities->presentModes.begin(),
                                  capabilities->presentModes.end());
    mSupportedAlphaModes.assign(capabilities->alphaModes.begin(), capabilities->alphaModes.end());

    DAWN_ASSERT(!mSupportedFormats.empty() && !mSupportedPresentModes.empty() &&
                !mSupportedAlphaModes.empty());
}

Surface::~Surface() = default;

ObjectType Surface::GetObjectType() const {
    return ObjectType::Surface;
}

void Surface::APIConfigure(const SurfaceConfiguration* config) {
    mConfiguredDevice = config->device;

    mTextureDescriptor = {};
    mTextureDescriptor.size = {config->width, config->height, 1};
    mTextureDescriptor.format = config->format;
    mTextureDescriptor.usage = config->usage;
    mTextureDescriptor.dimension = wgpu::TextureDimension::e2D;
    mTextureDescriptor.mipLevelCount = 1;
    mTextureDescriptor.sampleCount = 1;

    SurfaceConfigureCmd cmd;
    cmd.self = ToAPI(this);
    cmd.config = ToAPI(config);
    GetClient()->SerializeCommand(cmd);
}

wgpu::Status Surface::APIPresent() {
    if (mConfiguredDevice == nullptr) {
        dawn::ErrorLog() << "Surface::Present on an unconfigured Surface.";
        return wgpu::Status::Error;
    }

    SurfacePresentCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);

    // The only synchronous error is if the surface isn't configured.
    // Otherwise, we let the server report errors via the device.
    return wgpu::Status::Success;
}

void Surface::APIUnconfigure() {
    mConfiguredDevice = nullptr;

    SurfaceUnconfigureCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);
}

wgpu::TextureFormat Surface::APIGetPreferredFormat([[maybe_unused]] Adapter* adapter) const {
    dawn::ErrorLog() << "Surface::GetPreferredFormat is deprecated, use "
                        "Surface::GetCapabilities().formats[0] instead.";
    return mSupportedFormats[0];
}

wgpu::Status Surface::APIGetCapabilities(Adapter* adapter,
                                         SurfaceCapabilities* capabilities) const {
    // Return the capabilities that were provided when injecting the surface.
    capabilities->nextInChain = nullptr;
    capabilities->usages = mSupportedUsages;

    // These will be freed by APISurfaceCapabilitiesFreeMembers.
    capabilities->presentModes = HeapArrayFrom(mSupportedPresentModes).MoveToSpan();
    capabilities->formats = HeapArrayFrom(mSupportedFormats).MoveToSpan();
    capabilities->alphaModes = HeapArrayFrom(mSupportedAlphaModes).MoveToSpan();

    return wgpu::Status::Success;
}

void Surface::APIGetCurrentTexture(SurfaceTexture* surfaceTexture) {
    // Handle error cases that return no textures first.
    surfaceTexture->texture = nullptr;

    surfaceTexture->status = wgpu::SurfaceGetCurrentTextureStatus::Error;
    if (mConfiguredDevice == nullptr) {
        return;
    }

    // Assume texture creation will work in the server and return a new texture proxy.
    Client* wireClient = GetClient();
    Ref<Texture> texture = wireClient->Make<Texture>(mConfiguredDevice.Get(), &mTextureDescriptor);

    SurfaceGetCurrentTextureCmd cmd;
    cmd.surfaceId = GetWireHandle(wireClient).id;
    cmd.textureHandle = texture->GetWireHandle(wireClient);
    cmd.configuredDeviceId = mConfiguredDevice->GetWireHandle(wireClient).id;
    wireClient->SerializeCommand(cmd);

    surfaceTexture->status = wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal;
    surfaceTexture->texture = ReturnToAPI2(std::move(texture));
}

void APISurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities capabilities) {
    delete[] capabilities.presentModes;
    delete[] capabilities.formats;
    delete[] capabilities.alphaModes;
}

}  // namespace dawn::wire::client
