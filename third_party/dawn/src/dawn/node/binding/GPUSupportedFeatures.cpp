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

#include "src/dawn/node/binding/GPUSupportedFeatures.h"

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/IteratorHelper.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUSupportedFeatures
////////////////////////////////////////////////////////////////////////////////

GPUSupportedFeatures::GPUSupportedFeatures(Napi::Env env,
                                           const wgpu::SupportedFeatures& supportedFeatures) {
    Converter conv(env);

    // Add all known GPUFeatureNames that are known by dawn.node and skip the other ones are they
    // may be native-only extension, Dawn-specific or other special cases.
    for (uint32_t i = 0; i < supportedFeatures.featureCount; ++i) {
        wgpu::FeatureName feature = supportedFeatures.features[i];
        interop::GPUFeatureName gpuFeature;
        if (conv(gpuFeature, feature)) {
            enabled_.emplace(gpuFeature);
        }
    }
}

bool GPUSupportedFeatures::has(Napi::Env, std::string name) {
    interop::GPUFeatureName feature;
    if (!interop::Converter<interop::GPUFeatureName>::FromString(name, feature)) {
        return false;
    }

    return enabled_.count(feature) != 0u;
}

std::vector<std::string> GPUSupportedFeatures::keys(Napi::Env) {
    std::vector<std::string> out;

    out.reserve(enabled_.size());
    for (auto feature : enabled_) {
        out.push_back(interop::Converter<interop::GPUFeatureName>::ToString(feature));
    }
    return out;
}

size_t GPUSupportedFeatures::getSize(Napi::Env) {
    return enabled_.size();
}

Napi::Value GPUSupportedFeatures::iterator(const Napi::CallbackInfo& info) {
    return CreateIterator(info, this->enabled_);
}

}  // namespace wgpu::binding
