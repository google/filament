// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NODE_BINDING_GPUTEXTURE_H_
#define SRC_DAWN_NODE_BINDING_GPUTEXTURE_H_

#include <webgpu/webgpu_cpp.h>

#include <string>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUTexture is an implementation of interop::GPUTexture that wraps a wgpu::Texture.
class GPUTexture final : public interop::GPUTexture {
  public:
    GPUTexture(wgpu::Device device, const wgpu::TextureDescriptor& desc, wgpu::Texture texture);

    // Implicit cast operator to Dawn GPU object
    inline operator const wgpu::Texture&() const { return texture_; }

    // interop::GPUTexture interface compliance
    interop::Interface<interop::GPUTextureView> createView(
        Napi::Env,
        interop::GPUTextureViewDescriptor descriptor) override;
    void destroy(Napi::Env) override;
    interop::GPUIntegerCoordinateOut getWidth(Napi::Env) override;
    interop::GPUIntegerCoordinateOut getHeight(Napi::Env) override;
    interop::GPUIntegerCoordinateOut getDepthOrArrayLayers(Napi::Env) override;
    interop::GPUIntegerCoordinateOut getMipLevelCount(Napi::Env) override;
    interop::GPUSize32Out getSampleCount(Napi::Env) override;
    interop::GPUTextureDimension getDimension(Napi::Env) override;
    interop::GPUTextureFormat getFormat(Napi::Env) override;
    interop::GPUFlagsConstant getUsage(Napi::Env) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::Device device_;
    wgpu::Texture texture_;
    std::string label_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUTEXTURE_H_
