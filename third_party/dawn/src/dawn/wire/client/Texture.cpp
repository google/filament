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

#include "dawn/wire/client/Texture.h"

#include <utility>

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"

namespace dawn::wire::client {

// static
WGPUTexture Texture::CreateError(Device* device, const WGPUTextureDescriptor* descriptor) {
    Client* client = device->GetClient();
    Ref<Texture> texture = client->Make<Texture>(descriptor);

    DeviceCreateErrorTextureCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = descriptor;
    cmd.result = texture->GetWireHandle();
    client->SerializeCommand(cmd);

    return ReturnToAPI(std::move(texture));
}

Texture::Texture(const ObjectBaseParams& params, const WGPUTextureDescriptor* descriptor)
    : ObjectBase(params),
      mSize(descriptor->size),
      mMipLevelCount(descriptor->mipLevelCount),
      mSampleCount(descriptor->sampleCount),
      mDimension(descriptor->dimension == WGPUTextureDimension_Undefined ? WGPUTextureDimension_2D
                                                                         : descriptor->dimension),
      mFormat(descriptor->format),
      mUsage(static_cast<WGPUTextureUsage>(descriptor->usage)) {}

Texture::~Texture() = default;

ObjectType Texture::GetObjectType() const {
    return ObjectType::Texture;
}

uint32_t Texture::APIGetWidth() const {
    return mSize.width;
}

uint32_t Texture::APIGetHeight() const {
    return mSize.height;
}

uint32_t Texture::APIGetDepthOrArrayLayers() const {
    return mSize.depthOrArrayLayers;
}

uint32_t Texture::APIGetMipLevelCount() const {
    return mMipLevelCount;
}

uint32_t Texture::APIGetSampleCount() const {
    return mSampleCount;
}

WGPUTextureDimension Texture::APIGetDimension() const {
    return mDimension;
}

WGPUTextureFormat Texture::APIGetFormat() const {
    return mFormat;
}

WGPUTextureUsage Texture::APIGetUsage() const {
    return mUsage;
}

}  // namespace dawn::wire::client
