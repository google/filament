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

#include "src/dawn/node/binding/GPUAdapterInfo.h"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUAdapterInfo
////////////////////////////////////////////////////////////////////////////////

namespace {

interop::GPUSubgroupMatrixComponentType SubgroupMatrixComponentType(
    wgpu::SubgroupMatrixComponentType c) {
    switch (c) {
        case SubgroupMatrixComponentType::F32:
            return interop::GPUSubgroupMatrixComponentType::kF32;
        case SubgroupMatrixComponentType::F16:
            return interop::GPUSubgroupMatrixComponentType::kF16;
        case SubgroupMatrixComponentType::U32:
            return interop::GPUSubgroupMatrixComponentType::kU32;
        case SubgroupMatrixComponentType::I32:
            return interop::GPUSubgroupMatrixComponentType::kI32;
    }
}

struct GPUSubgroupMatrixConfig : public interop::GPUSubgroupMatrixConfig {
    interop::GPUSubgroupMatrixComponentType componentType;
    interop::GPUSubgroupMatrixComponentType resultComponentType;
    uint32_t M;
    uint32_t N;
    uint32_t K;

    explicit GPUSubgroupMatrixConfig(const wgpu::SubgroupMatrixConfig& config)
        : componentType(SubgroupMatrixComponentType(config.componentType)),
          resultComponentType(SubgroupMatrixComponentType(config.resultComponentType)),
          M(config.M),
          N(config.N),
          K(config.K) {}

    interop::GPUSubgroupMatrixComponentType getComponentType(Napi::Env) override {
        return componentType;
    }
    interop::GPUSubgroupMatrixComponentType getResultComponentType(Napi::Env) override {
        return resultComponentType;
    }
    uint32_t getM(Napi::Env) override { return M; }
    uint32_t getN(Napi::Env) override { return N; }
    uint32_t getK(Napi::Env) override { return K; }
};

// Normalize according to https://gpuweb.github.io/gpuweb/#normalized-identifier-string
std::string NormalizeIdentifierString(wgpu::StringView s) {
    std::ostringstream o;

    // Used to concatenate multiple non-alnum into a single dash.
    bool lastWasDash = false;
    // Used to start adding dashes only after we had one alnum.
    bool hadAlnum = false;

    for (char c : std::string_view(s)) {
        if (std::isalnum(c)) {
            o << std::tolower(c);
            lastWasDash = false;
            hadAlnum = true;
        } else if (!lastWasDash && hadAlnum) {
            o << '-';
            lastWasDash = true;
        }
    }

    return o.str();
}

}  // namespace

GPUAdapterInfo::GPUAdapterInfo(const wgpu::AdapterInfo& info)
    : vendor_(NormalizeIdentifierString(info.vendor)),
      architecture_(NormalizeIdentifierString(info.architecture)),
      device_(NormalizeIdentifierString(info.device)),
      description_(info.description),
      subgroup_min_size_(info.subgroupMinSize),
      subgroup_max_size_(info.subgroupMaxSize),
      is_fallback_adapter_(info.adapterType == wgpu::AdapterType::CPU) {
    auto* next = info.nextInChain;
    while (next) {
        if (next->sType == SType::AdapterPropertiesSubgroupMatrixConfigs) {
            auto* configs = static_cast<wgpu::AdapterPropertiesSubgroupMatrixConfigs*>(next);
            subgroup_matrix_configs_.reserve(configs->configCount);
            for (uint32_t i = 0; i < configs->configCount; i++) {
                subgroup_matrix_configs_.push_back(configs->configs[i]);
            }
        }
        next = next->nextInChain;
    }
}

std::string GPUAdapterInfo::getVendor(Napi::Env) {
    return vendor_;
}

std::string GPUAdapterInfo::getArchitecture(Napi::Env) {
    return architecture_;
}

std::string GPUAdapterInfo::getDevice(Napi::Env) {
    return device_;
}

std::string GPUAdapterInfo::getDescription(Napi::Env) {
    return description_;
}

uint32_t GPUAdapterInfo::getSubgroupMinSize(Napi::Env) {
    return subgroup_min_size_;
}

uint32_t GPUAdapterInfo::getSubgroupMaxSize(Napi::Env) {
    return subgroup_max_size_;
}

bool GPUAdapterInfo::getIsFallbackAdapter(Napi::Env) {
    return is_fallback_adapter_;
}

GPUAdapterInfo::SubgroupMatrixConfigs GPUAdapterInfo::getSubgroupMatrixConfigs(Napi::Env env) {
    SubgroupMatrixConfigs out;
    out.reserve(subgroup_matrix_configs_.size());
    for (auto& config : subgroup_matrix_configs_) {
        out.emplace_back(
            interop::GPUSubgroupMatrixConfig::Create<GPUSubgroupMatrixConfig>(env, config));
    }
    return out;
}

}  // namespace wgpu::binding
