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

#include "dawn/wire/client/Surface.h"

#include <algorithm>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"
#include "dawn/wire/client/Texture.h"
#include "dawn/wire/client/webgpu.h"

namespace dawn::wire::client {

Surface::Surface(const ObjectBaseParams& params, const WGPUSurfaceCapabilities* capabilities)
    : ObjectBase(params) {
    // Copy over the capabilities.
    mSupportedUsages = capabilities->usages;
    mSupportedFormats.assign(capabilities->formats,
                             capabilities->formats + capabilities->formatCount);
    mSupportedPresentModes.assign(capabilities->presentModes,
                                  capabilities->presentModes + capabilities->presentModeCount);
    mSupportedAlphaModes.assign(capabilities->alphaModes,
                                capabilities->alphaModes + capabilities->alphaModeCount);

    DAWN_ASSERT(!mSupportedFormats.empty() && !mSupportedPresentModes.empty() &&
                !mSupportedAlphaModes.empty());
}

Surface::~Surface() = default;

ObjectType Surface::GetObjectType() const {
    return ObjectType::Surface;
}

void Surface::APIConfigure(const WGPUSurfaceConfiguration* config) {
    mConfiguredDevice = FromAPI(config->device);

    mTextureDescriptor = {};
    mTextureDescriptor.size = {config->width, config->height, 1};
    mTextureDescriptor.format = config->format;
    mTextureDescriptor.usage = config->usage;
    mTextureDescriptor.dimension = WGPUTextureDimension_2D;
    mTextureDescriptor.mipLevelCount = 1;
    mTextureDescriptor.sampleCount = 1;

    SurfaceConfigureCmd cmd;
    cmd.self = ToAPI(this);
    cmd.config = config;
    GetClient()->SerializeCommand(cmd);
}

WGPUStatus Surface::APIPresent() {
    if (mConfiguredDevice == nullptr) {
        dawn::ErrorLog() << "Surface::Present on an unconfigured Surface.";
        return WGPUStatus_Error;
    }

    SurfacePresentCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);

    // The only synchronous error is if the surface isn't configured.
    // Otherwise, we let the server report errors via the device.
    return WGPUStatus_Success;
}

void Surface::APIUnconfigure() {
    mConfiguredDevice = nullptr;

    SurfaceUnconfigureCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);
}

WGPUTextureFormat Surface::APIGetPreferredFormat([[maybe_unused]] WGPUAdapter adapter) const {
    dawn::ErrorLog() << "Surface::GetPreferredFormat is deprecated, use "
                        "Surface::GetCapabilities().formats[0] instead.";
    return mSupportedFormats[0];
}

WGPUStatus Surface::APIGetCapabilities(WGPUAdapter adapter,
                                       WGPUSurfaceCapabilities* capabilities) const {
    // Return the capabilities that were provided when injecting the surface.
    capabilities->nextInChain = nullptr;
    capabilities->usages = mSupportedUsages;

    capabilities->presentModeCount = mSupportedPresentModes.size();
    WGPUPresentMode* presentModes = new WGPUPresentMode[capabilities->presentModeCount];
    std::copy(mSupportedPresentModes.begin(), mSupportedPresentModes.end(), presentModes);
    capabilities->presentModes = presentModes;

    capabilities->formatCount = mSupportedFormats.size();
    WGPUTextureFormat* formats = new WGPUTextureFormat[capabilities->formatCount];
    std::copy(mSupportedFormats.begin(), mSupportedFormats.end(), formats);
    capabilities->formats = formats;

    capabilities->alphaModeCount = mSupportedAlphaModes.size();
    WGPUCompositeAlphaMode* alphaModes = new WGPUCompositeAlphaMode[capabilities->alphaModeCount];
    std::copy(mSupportedAlphaModes.begin(), mSupportedAlphaModes.end(), alphaModes);
    capabilities->alphaModes = alphaModes;

    return WGPUStatus_Success;
}

void Surface::APIGetCurrentTexture(WGPUSurfaceTexture* surfaceTexture) {
    // Handle error cases that return no textures first.
    surfaceTexture->texture = nullptr;

    surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_Error;
    if (mConfiguredDevice == nullptr) {
        return;
    }

    if (!mConfiguredDevice->IsAlive()) {
        surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal;
        surfaceTexture->texture =
            Texture::CreateError(mConfiguredDevice.Get(), &mTextureDescriptor);
        return;
    }

    // Assume texture creation will work in the server and return a new texture proxy.
    Client* wireClient = GetClient();
    Ref<Texture> texture = wireClient->Make<Texture>(&mTextureDescriptor);

    SurfaceGetCurrentTextureCmd cmd;
    cmd.surfaceId = GetWireId();
    cmd.textureHandle = texture->GetWireHandle();
    cmd.configuredDeviceId = mConfiguredDevice->GetWireId();
    wireClient->SerializeCommand(cmd);

    surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal;
    surfaceTexture->texture = ReturnToAPI(std::move(texture));
}

void APIFreeMembers(WGPUSurfaceCapabilities capabilities) {
    delete[] capabilities.presentModes;
    delete[] capabilities.formats;
    delete[] capabilities.alphaModes;
}

}  // namespace dawn::wire::client
