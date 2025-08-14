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

#include "src/dawn/node/binding/GPU.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/webgpu_cpp_print.h"
#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPUAdapter.h"
#include "src/dawn/node/binding/IteratorHelper.h"
#include "src/dawn/node/binding/TogglesLoader.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace {
std::string GetEnvVar(const char* varName) {
#if defined(_WIN32)
    // Use _dupenv_s to avoid unsafe warnings about std::getenv
    char* value = nullptr;
    _dupenv_s(&value, nullptr, varName);
    if (value) {
        std::string result = value;
        free(value);
        return result;
    }
    return "";
#else
    if (auto* val = std::getenv(varName)) {
        return val;
    }
    return "";
#endif
}

void SetDllDir(const char* dir) {
    (void)dir;
#if defined(_WIN32)
    ::SetDllDirectory(dir);
#endif
}

struct BackendInfo {
    const char* const name;
    const char* const alias;  // may be nullptr
    wgpu::BackendType const backend;
};

constexpr BackendInfo kBackends[] = {
    {"null", nullptr, wgpu::BackendType::Null},         //
    {"webgpu", nullptr, wgpu::BackendType::WebGPU},     //
    {"d3d11", nullptr, wgpu::BackendType::D3D11},       //
    {"d3d12", "d3d", wgpu::BackendType::D3D12},         //
    {"metal", nullptr, wgpu::BackendType::Metal},       //
    {"vulkan", "vk", wgpu::BackendType::Vulkan},        //
    {"opengl", "gl", wgpu::BackendType::OpenGL},        //
    {"opengles", "gles", wgpu::BackendType::OpenGLES},  //
};

std::optional<wgpu::BackendType> ParseBackend(std::string_view name) {
    for (auto& info : kBackends) {
        if (info.name == name || (info.alias && info.alias == name)) {
            return info.backend;
        }
    }
    return std::nullopt;
}

const char* BackendName(wgpu::BackendType backend) {
    for (auto& info : kBackends) {
        if (info.backend == backend) {
            return info.name;
        }
    }
    return "<unknown>";
}

}  // namespace

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPU
////////////////////////////////////////////////////////////////////////////////
GPU::GPU(Flags flags) : flags_(std::move(flags)) {
    // Setting the DllDir changes where we load adapter DLLs from (e.g. d3dcompiler_47.dll)
    if (auto dir = flags_.Get("dlldir")) {
        SetDllDir(dir->c_str());
    }

    // Set up the chained descriptor for the various instance extensions.
    dawn::native::DawnInstanceDescriptor dawnDesc;
    if (auto validate = flags_.Get("validate"); validate == "1" || validate == "true") {
        dawnDesc.backendValidationLevel = dawn::native::BackendValidationLevel::Full;
    }

    TogglesLoader togglesLoader(flags_);
    DawnTogglesDescriptor togglesDesc = togglesLoader.GetDescriptor();
    togglesDesc.nextInChain = &dawnDesc;

    wgpu::InstanceDescriptor desc;
    desc.nextInChain = &togglesDesc;
    instance_ = std::make_unique<dawn::native::Instance>(
        reinterpret_cast<const WGPUInstanceDescriptor*>(&desc));
    async_ = AsyncRunner::Create(instance_.get());
}

interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>> GPU::requestAdapter(
    Napi::Env env,
    interop::GPURequestAdapterOptions options) {
    auto promise =
        interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>>(env, PROMISE_INFO);

    RequestAdapterOptions nativeOptions;
    nativeOptions.forceFallbackAdapter = options.forceFallbackAdapter;

    // Convert the feature level.
    nativeOptions.featureLevel = FeatureLevel::Undefined;
    if (options.featureLevel == "compatibility") {
        nativeOptions.featureLevel = FeatureLevel::Compatibility;
    } else if (options.featureLevel == "core") {
        nativeOptions.featureLevel = FeatureLevel::Core;
    } else {
        promise.Resolve({});
        return promise;
    }

    // Convert the power preference.
    nativeOptions.powerPreference = PowerPreference::Undefined;
    if (options.powerPreference.has_value()) {
        switch (options.powerPreference.value()) {
            case interop::GPUPowerPreference::kLowPower:
                nativeOptions.powerPreference = PowerPreference::LowPower;
                break;
            case interop::GPUPowerPreference::kHighPerformance:
                nativeOptions.powerPreference = PowerPreference::HighPerformance;
                break;
        }
    }

    // Choose the backend to use.
#if defined(_WIN32)
    constexpr auto kDefaultBackendType = wgpu::BackendType::D3D12;
#elif defined(__linux__)
    constexpr auto kDefaultBackendType = wgpu::BackendType::Vulkan;
#elif defined(__APPLE__)
    constexpr auto kDefaultBackendType = wgpu::BackendType::Metal;
#else
#error "Unsupported platform"
#endif
    nativeOptions.backendType = kDefaultBackendType;

    // Check for backend override from env var / flag.
    std::string forceBackend;
    if (auto f = flags_.Get("backend")) {
        forceBackend = *f;
    } else if (std::string envVar = GetEnvVar("DAWNNODE_BACKEND"); !envVar.empty()) {
        forceBackend = envVar;
    }
    std::transform(forceBackend.begin(), forceBackend.end(), forceBackend.begin(),
                   [](char c) { return std::tolower(c); });

    if (!forceBackend.empty()) {
        if (auto parsed = ParseBackend(forceBackend)) {
            nativeOptions.backendType = parsed.value();
        } else {
            std::stringstream msg;
            msg << "unrecognised backend '" << forceBackend << "'\nPossible backends : ";
            for (auto& info : kBackends) {
                if (&info != &kBackends[0]) {
                    msg << ", ";
                }
                msg << "'" << info.name << "'";
            }
            promise.Reject(msg.str());
            return promise;
        }
    }

    // Propagate toggles.
    TogglesLoader togglesLoader(flags_);
    DawnTogglesDescriptor togglesDescriptor = togglesLoader.GetDescriptor();
    nativeOptions.nextInChain = &togglesDescriptor;

    auto nativeAdapters = instance_->EnumerateAdapters(&nativeOptions);
    if (nativeAdapters.empty()) {
        promise.Resolve({});
        return promise;
    }

    std::vector<wgpu::Adapter> adapters(nativeAdapters.size());
    for (uint32_t i = 0; i < nativeAdapters.size(); ++i) {
        adapters[i] = wgpu::Adapter(nativeAdapters[i].Get());
    }

    // Check for specific adapter device name.
    // This was AdapterProperties.name, now it is AdapterInfo.device.
    std::string deviceName;
    if (auto f = flags_.Get("adapter")) {
        deviceName = *f;
    }

    wgpu::Adapter* adapter = nullptr;
    wgpu::AdapterInfo adapterInfo;
    for (auto& a : adapters) {
        a.GetInfo(&adapterInfo);

        if (!deviceName.empty() &&
            std::string_view(adapterInfo.device).find(deviceName) == std::string::npos) {
            continue;
        }

        adapter = &a;
        break;
    }

    if (!adapter) {
        std::stringstream msg;
        if (!forceBackend.empty() || deviceName.empty()) {
            msg << "no adapter ";
            if (!forceBackend.empty()) {
                msg << "with backend '" << forceBackend << "'";
                if (!deviceName.empty()) {
                    msg << " and name '" << deviceName << "'";
                }
            } else {
                msg << " with name '" << deviceName << "'";
            }
            msg << " found";
        } else {
            msg << "no suitable backends found";
        }
        msg << "\nAvailable adapters:";
        for (auto& a : adapters) {
            wgpu::AdapterInfo info;
            a.GetInfo(&info);
            msg << "\n * backend: '" << BackendName(info.backendType) << "', name: '" << info.device
                << "'";
        }
        promise.Reject(msg.str());
        return promise;
    }

    if (flags_.Get("verbose")) {
        std::cout << "using GPU adapter: " << adapterInfo.device << "\n";
    }

    auto gpuAdapter = GPUAdapter::Create<GPUAdapter>(env, *adapter, flags_, async_);
    promise.Resolve(std::optional<interop::Interface<interop::GPUAdapter>>(gpuAdapter));
    return promise;
}

interop::GPUTextureFormat GPU::getPreferredCanvasFormat(Napi::Env) {
#if defined(__ANDROID__)
    return interop::GPUTextureFormat::kRgba8Unorm;
#else
    return interop::GPUTextureFormat::kBgra8Unorm;
#endif  // defined(__ANDROID__)
}

interop::Interface<interop::WGSLLanguageFeatures> GPU::getWgslLanguageFeatures(Napi::Env env) {
    using InteropWGSLFeatureSet = std::unordered_set<interop::WGSLLanguageFeatureName>;

    struct Features : public interop::WGSLLanguageFeatures {
        explicit Features(InteropWGSLFeatureSet features) : features_(features) {}
        ~Features() override = default;

        bool has(Napi::Env env, std::string name) override {
            interop::WGSLLanguageFeatureName feature;
            if (!interop::Converter<interop::WGSLLanguageFeatureName>::FromString(name, feature)) {
                return false;
            }
            return features_.count(feature) != 0u;
        }
        std::vector<std::string> keys(Napi::Env env) override {
            std::vector<std::string> out;
            out.reserve(features_.size());
            for (auto feature : features_) {
                out.push_back(
                    interop::Converter<interop::WGSLLanguageFeatureName>::ToString(feature));
            }
            return out;
        }
        size_t getSize(Napi::Env env) override { return features_.size(); }
        Napi::Value iterator(const Napi::CallbackInfo& info) override {
            return CreateIterator<InteropWGSLFeatureSet>(info, this->features_);
        }

        InteropWGSLFeatureSet features_;
    };

    wgpu::Instance instance = instance_->Get();
    wgpu::SupportedWGSLLanguageFeatures supportedFeatures = {};
    instance.GetWGSLLanguageFeatures(&supportedFeatures);

    // Add all known WGSLLangaugeFeatures known by dawn.node but warn loudly when there are unknown
    // ones.
    InteropWGSLFeatureSet featureSet;
    Converter conv(env);
    for (size_t i = 0; i < supportedFeatures.featureCount; i++) {
        wgpu::WGSLLanguageFeatureName feature = supportedFeatures.features[i];
        interop::WGSLLanguageFeatureName wgslFeature;
        if (conv(wgslFeature, feature)) {
            featureSet.emplace(wgslFeature);
        } else {
            LOG("Unknown WGSLLanguageFeatureName ", feature);
        }
    }

    return interop::WGSLLanguageFeatures::Create<Features>(env, std::move(featureSet));
}

}  // namespace wgpu::binding
