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

#ifndef SRC_DAWN_WIRE_CLIENT_SURFACE_H_
#define SRC_DAWN_WIRE_CLIENT_SURFACE_H_

#include <webgpu/webgpu.h>

#include <vector>

#include "src/dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Device;

void APISurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities capabilities);

class Surface final : public ObjectBase {
  public:
    explicit Surface(const ObjectBaseParams& params, const SurfaceCapabilities* capabilities);
    ~Surface() override;

    ObjectType GetObjectType() const override;

    // WebGPU API
    void APIConfigure(const SurfaceConfiguration* config);
    wgpu::Status APIPresent();
    void APIUnconfigure();
    wgpu::TextureFormat APIGetPreferredFormat(Adapter* adapter) const;
    wgpu::Status APIGetCapabilities(Adapter* adapter, SurfaceCapabilities* capabilities) const;
    void APIGetCurrentTexture(SurfaceTexture* surfaceTexture);

  private:
    wgpu::TextureUsage mSupportedUsages;
    std::vector<wgpu::TextureFormat> mSupportedFormats;
    std::vector<wgpu::PresentMode> mSupportedPresentModes;
    std::vector<wgpu::CompositeAlphaMode> mSupportedAlphaModes;

    Ref<Device> mConfiguredDevice;
    TextureDescriptor mTextureDescriptor = {};
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_SURFACE_H_
