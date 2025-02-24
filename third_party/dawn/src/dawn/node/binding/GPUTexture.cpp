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

#include "src/dawn/node/binding/GPUTexture.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/Errors.h"
#include "src/dawn/node/binding/GPUTextureView.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUTexture
////////////////////////////////////////////////////////////////////////////////
GPUTexture::GPUTexture(wgpu::Device device,
                       const wgpu::TextureDescriptor& desc,
                       wgpu::Texture texture)
    : device_(std::move(device)), texture_(std::move(texture)), label_(CopyLabel(desc.label)) {}

interop::Interface<interop::GPUTextureView> GPUTexture::createView(
    Napi::Env env,
    interop::GPUTextureViewDescriptor descriptor) {
    if (!texture_) {
        Errors::OperationError(env).ThrowAsJavaScriptException();
        return {};
    }

    wgpu::TextureViewDescriptor desc{};
    Converter conv(env, device_);
    if (!conv(desc.baseMipLevel, descriptor.baseMipLevel) ||        //
        !conv(desc.mipLevelCount, descriptor.mipLevelCount) ||      //
        !conv(desc.baseArrayLayer, descriptor.baseArrayLayer) ||    //
        !conv(desc.arrayLayerCount, descriptor.arrayLayerCount) ||  //
        !conv(desc.format, descriptor.format) ||                    //
        !conv(desc.dimension, descriptor.dimension) ||              //
        !conv(desc.aspect, descriptor.aspect) ||                    //
        !conv(desc.label, descriptor.label) ||                      //
        !conv(desc.usage, descriptor.usage)) {
        return {};
    }
    return interop::GPUTextureView::Create<GPUTextureView>(env, desc, texture_.CreateView(&desc));
}

void GPUTexture::destroy(Napi::Env) {
    texture_.Destroy();
}

interop::GPUIntegerCoordinateOut GPUTexture::getWidth(Napi::Env) {
    return texture_.GetWidth();
}

interop::GPUIntegerCoordinateOut GPUTexture::getHeight(Napi::Env) {
    return texture_.GetHeight();
}

interop::GPUIntegerCoordinateOut GPUTexture::getDepthOrArrayLayers(Napi::Env) {
    return texture_.GetDepthOrArrayLayers();
}

interop::GPUIntegerCoordinateOut GPUTexture::getMipLevelCount(Napi::Env) {
    return texture_.GetMipLevelCount();
}

interop::GPUSize32Out GPUTexture::getSampleCount(Napi::Env) {
    return texture_.GetSampleCount();
}

interop::GPUTextureDimension GPUTexture::getDimension(Napi::Env env) {
    interop::GPUTextureDimension result;

    Converter conv(env);
    if (!conv(result, texture_.GetDimension())) {
        Napi::Error::New(env, "Couldn't convert dimension to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return interop::GPUTextureDimension::k1D;  // Doesn't get used.
    }

    return result;
}

interop::GPUTextureFormat GPUTexture::getFormat(Napi::Env env) {
    interop::GPUTextureFormat result;

    Converter conv(env);
    if (!conv(result, texture_.GetFormat())) {
        Napi::Error::New(env, "Couldn't convert format to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return interop::GPUTextureFormat::kR32Float;  // Doesn't get used.
    }

    return result;
}

interop::GPUFlagsConstant GPUTexture::getUsage(Napi::Env env) {
    interop::GPUTextureUsageFlags result;

    Converter conv(env);
    if (!conv(result, texture_.GetUsage())) {
        Napi::Error::New(env, "Couldn't convert usage to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return 0u;  // Doesn't get used.
    }

    return result;
}

std::string GPUTexture::getLabel(Napi::Env) {
    return label_;
}

void GPUTexture::setLabel(Napi::Env, std::string value) {
    texture_.SetLabel(std::string_view(value));
    label_ = value;
}

}  // namespace wgpu::binding
