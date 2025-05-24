/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <backend/platforms/WebGPUPlatform.h>

#include "webgpu/WebGPUConstants.h"
#include "webgpu/WebGPUDriver.h"

#include <backend/DriverEnums.h>
#include <backend/Platform.h>
#include <utils/Hash.h>
#include <utils/Panic.h>
#include <utils/ostream.h>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <mutex>
#include <sstream>// for one-time-ish setup string concatenation, namely error messaging
#include <string_view>
#include <unordered_set>
#include <utility>
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
#include <variant>
#endif
#include <vector>

/**
 * WebGPU Backend implementation common across platforms or operating systems (at least for now).
 * Some of these functions may likely be refactored to platform/OS-specific implementations
 * over time as needed. The caller of the WebGPUPlatform doesn't need to care which is the case.
 */

namespace filament::backend {

namespace {

constexpr std::array REQUIRED_FEATURES = {
    wgpu::FeatureName::TransientAttachments };

constexpr std::array OPTIONAL_FEATURES = {
    wgpu::FeatureName::DepthClipControl,
    wgpu::FeatureName::Depth32FloatStencil8,
    wgpu::FeatureName::CoreFeaturesAndLimits };

constexpr wgpu::Limits REQUIRED_LIMITS = wgpu::Limits{
    .maxBindGroups = MAX_DESCRIPTOR_SET_COUNT,
    .maxBindingsPerBindGroup = MAX_DESCRIPTOR_COUNT,
    .maxSamplersPerShaderStage = 16, // TODO should be set to MAX_SAMPLER_COUNT,
    .maxStorageBuffersPerShaderStage = MAX_SSBO_COUNT,
    .maxVertexBuffers = 8, // TODO should be set to MAX_VERTEX_BUFFER_COUNT,
    .maxVertexAttributes = MAX_VERTEX_ATTRIBUTE_COUNT,
};

constexpr uint64_t REQUEST_ADAPTER_TIMEOUT_NANOSECONDS =
        /* milliseconds */ 1000u * /* converted to ns */ 1000000u;

constexpr uint64_t REQUEST_DEVICE_TIMEOUT_NANOSECONDS =
        /* milliseconds */ 1000u * /* converted to ns */ 1000000u;

enum class LimitToValidate : uint8_t {
    MAX_BIND_GROUPS = 0,
    MAX_BINDINGS_PER_BIND_GROUP = 1,
    MAX_SAMPLERS_PER_SHADER_STAGE = 2,
    MAX_STORAGE_BUFFERS_PER_SHADER_STAGE = 3,
    MAX_VERTEX_BUFFERS = 4,
    MAX_VERTEX_ATTRIBUTES = 5,
    UNKNOWN = 6
};

void assertLimitsAreExpressedInRequirementsStruct() {
    for (uint8_t limit = 0; limit < static_cast<uint8_t>(LimitToValidate::UNKNOWN); limit++) {
        switch (static_cast<LimitToValidate>(limit)) {
            case LimitToValidate::MAX_BIND_GROUPS:
                static_assert(REQUIRED_LIMITS.maxBindGroups < wgpu::kLimitU32Undefined);
                break;
            case LimitToValidate::MAX_BINDINGS_PER_BIND_GROUP:
                static_assert(REQUIRED_LIMITS.maxBindingsPerBindGroup < wgpu::kLimitU32Undefined);
                break;
            case LimitToValidate::MAX_SAMPLERS_PER_SHADER_STAGE:
                static_assert(REQUIRED_LIMITS.maxSamplersPerShaderStage < wgpu::kLimitU32Undefined);
                break;
            case LimitToValidate::MAX_STORAGE_BUFFERS_PER_SHADER_STAGE:
                static_assert(
                        REQUIRED_LIMITS.maxStorageBuffersPerShaderStage < wgpu::kLimitU32Undefined);
                break;
            case LimitToValidate::MAX_VERTEX_BUFFERS:
                static_assert(REQUIRED_LIMITS.maxVertexBuffers < wgpu::kLimitU32Undefined);
                break;
            case LimitToValidate::MAX_VERTEX_ATTRIBUTES:
                static_assert(REQUIRED_LIMITS.maxVertexAttributes < wgpu::kLimitU32Undefined);
                break;
            case LimitToValidate::UNKNOWN:
                break;
        }
    }
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
[[nodiscard]] std::string_view toString(LimitToValidate limit) {
    switch (limit) {
        case LimitToValidate::MAX_BIND_GROUPS:
            return "MAX_BIND_GROUPS";
        case LimitToValidate::MAX_BINDINGS_PER_BIND_GROUP:
            return "MAX_BINDINGS_PER_BIND_GROUP";
        case LimitToValidate::MAX_SAMPLERS_PER_SHADER_STAGE:
            return "MAX_SAMPLERS_PER_SHADER_STAGE";
        case LimitToValidate::MAX_STORAGE_BUFFERS_PER_SHADER_STAGE:
            return "MAX_STORAGE_BUFFERS_PER_SHADER_STAGE";
        case LimitToValidate::MAX_VERTEX_BUFFERS:
            return "MAX_VERTEX_BUFFERS";
        case LimitToValidate::MAX_VERTEX_ATTRIBUTES:
            return "MAX_VERTEX_ATTRIBUTES";
        case LimitToValidate::UNKNOWN:
            return "UNKNOWN";
    }
}
#endif

[[nodiscard]] uint64_t valueForLimit(wgpu::Limits const& limits, LimitToValidate limit) {
    switch (limit) {
        case LimitToValidate::MAX_BIND_GROUPS:
            return limits.maxBindGroups;
        case LimitToValidate::MAX_BINDINGS_PER_BIND_GROUP:
            return limits.maxBindingsPerBindGroup;
        case LimitToValidate::MAX_SAMPLERS_PER_SHADER_STAGE:
            return limits.maxSamplersPerShaderStage;
        case LimitToValidate::MAX_STORAGE_BUFFERS_PER_SHADER_STAGE:
            return limits.maxStorageBuffersPerShaderStage;
        case LimitToValidate::MAX_VERTEX_BUFFERS:
            return limits.maxVertexBuffers;
        case LimitToValidate::MAX_VERTEX_ATTRIBUTES:
            return limits.maxVertexAttributes;
        case LimitToValidate::UNKNOWN:
            return 0;
    }
}

[[nodiscard]] bool satisfiesLimits(wgpu::Limits const& limits) {
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    std::stringstream failedLimitsStream;
    size_t failedLimitCount = 0;
#endif
    for (uint8_t limitIndex = 0; limitIndex < static_cast<uint8_t>(LimitToValidate::UNKNOWN);
            limitIndex++) {
        LimitToValidate limit = static_cast<LimitToValidate>(limitIndex);
        uint64_t supportedValue = valueForLimit(limits, limit);
        uint64_t requiredValue = valueForLimit(REQUIRED_LIMITS, limit);
        if (supportedValue < requiredValue) {
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
            failedLimitCount++;
            failedLimitsStream << toString(limit) << " required " << requiredValue << " but found "
                               << supportedValue << ". ";
#else
            return false;
#endif
        }
    }
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    if (failedLimitCount < 1) {
        return true;
    }
    FWGPU_LOGI << "Failed to satisfy " << failedLimitCount
               << " limit(s): " << failedLimitsStream.str();
    return false;
#else
    return true;
#endif
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
utils::io::ostream& operator<<(utils::io::ostream& out,
        wgpu::WGSLLanguageFeatureName const languageFeatureName) noexcept {
    std::stringstream nameStream;
    nameStream << languageFeatureName;
    out << nameStream.str();
    return out;
}
#endif

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printInstanceDetails(wgpu::Instance const& instance) {
    wgpu::SupportedWGSLLanguageFeatures supportedWGSLLanguageFeatures{};
    if (!instance.GetWGSLLanguageFeatures(&supportedWGSLLanguageFeatures)) {
        FWGPU_LOGW << "Failed to get WebGPU instance supported WGSL language features"
                   << utils::io::endl;
    } else {
        FWGPU_LOGI << "WebGPU instance supported WGSL language features ("
                   << supportedWGSLLanguageFeatures.featureCount << "):" << utils::io::endl;
        if (supportedWGSLLanguageFeatures.featureCount > 0 &&
                supportedWGSLLanguageFeatures.features != nullptr) {
            std::for_each(supportedWGSLLanguageFeatures.features,
                    supportedWGSLLanguageFeatures.features +
                            supportedWGSLLanguageFeatures.featureCount,
                    [](wgpu::WGSLLanguageFeatureName const featureName) {
                        FWGPU_LOGI << "  " << featureName << utils::io::endl;
                    });
        }
    }
}
#endif

//either returns a valid instance or panics
[[nodiscard]] wgpu::Instance createInstance() {
    wgpu::DawnTogglesDescriptor dawnTogglesDescriptor{};
#if defined(FILAMENT_WEBGPU_IMMEDIATE_ERROR_HANDLING)
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    FWGPU_LOGI << "setting on toggle enable_immediate_error_handling" << utils::io::endl;
#endif
    /**
     * Have the un-captured error callback invoked immediately when an error occurs, rather than
     * waiting for the next Tick. This enables using the stack trace in which the un-captured error
     * occurred when breaking into the un-captured error callback.
     * https://crbug.com/dawn/1789
     */
    static const char* toggleName = "enable_immediate_error_handling";
    dawnTogglesDescriptor.enabledToggleCount = 1;
    dawnTogglesDescriptor.enabledToggles = &toggleName;
#endif
    wgpu::InstanceDescriptor instanceDescriptor{
        .nextInChain = &dawnTogglesDescriptor,
        .capabilities = {
            .timedWaitAnyEnable = true// TODO consider using pure async instead
        }
    };
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(instance != nullptr) << "Unable to create WebGPU instance.";
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printInstanceDetails(instance);
#endif
    return instance;
}

[[nodiscard]] constexpr std::string_view toString(const wgpu::ErrorType errorType) {
    switch (errorType) {
        case wgpu::ErrorType::NoError:     return "NO_ERROR";
        case wgpu::ErrorType::Validation:  return "VALIDATION";
        case wgpu::ErrorType::OutOfMemory: return "OUT_OF_MEMORY";
        case wgpu::ErrorType::Internal:    return "INTERNAL";
        case wgpu::ErrorType::Unknown:     return "UNKNOWN";
    }
}

[[nodiscard]] constexpr std::string_view toString(const wgpu::PowerPreference powerPreference) {
    switch (powerPreference) {
        case wgpu::PowerPreference::Undefined:       return "UNDEFINED";
        case wgpu::PowerPreference::LowPower:        return "LOW_POWER";
        case wgpu::PowerPreference::HighPerformance: return "HIGH_PERFORMANCE";
    }
}

[[nodiscard]] constexpr std::string_view toString(wgpu::BackendType backendType) {
    switch (backendType) {
        case wgpu::BackendType::Undefined: return "UNDEFINED";
        case wgpu::BackendType::Null:      return "NULL";
        case wgpu::BackendType::WebGPU:    return "WEBGPU";
        case wgpu::BackendType::D3D11:     return "D3D11";
        case wgpu::BackendType::D3D12:     return "D3D12";
        case wgpu::BackendType::Metal:     return "METAL";
        case wgpu::BackendType::Vulkan:    return "VULKAN";
        case wgpu::BackendType::OpenGL:    return "OPENGL";
        case wgpu::BackendType::OpenGLES:  return "OPENGLES";
    }
}

[[nodiscard]] constexpr std::string_view toString(wgpu::AdapterType adapterType) {
    switch (adapterType) {
        case wgpu::AdapterType::DiscreteGPU:   return "DISCRETE_GPU";
        case wgpu::AdapterType::IntegratedGPU: return "INTEGRATED_GPU";
        case wgpu::AdapterType::CPU:           return "CPU";
        case wgpu::AdapterType::Unknown:       return "UNKNOWN";
    }
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
template<class T>
struct Verbose final {
    T const& it;
};
#endif

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
utils::io::ostream& operator<<(utils::io::ostream& out,
        wgpu::FeatureName const featureName) noexcept {
    std::stringstream nameStream;
    nameStream << featureName;
    out << nameStream.str();
    return out;
}
#endif

template<class STREAM_TYPE>
STREAM_TYPE& operator<<(STREAM_TYPE& out, wgpu::RequestAdapterOptions const& options) noexcept {
    out << "power preference " << toString(options.powerPreference)
        << " force fallback adapter " << options.forceFallbackAdapter
        << " backend type " << toString(options.backendType);
    return out;
}

template<class STREAM_TYPE>
STREAM_TYPE& operator<<(STREAM_TYPE& out, wgpu::AdapterInfo const& info) noexcept {
    out << "vendor (" << info.vendorID << ") '" << info.vendor
        << "' device (" << info.deviceID << ") '" << info.device
        << "' adapter " << toString(info.adapterType)
        << " backend " << toString(info.backendType);
    return out;
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
utils::io::ostream& operator<<(utils::io::ostream& out,
        const Verbose<wgpu::AdapterInfo> verboseInfo) noexcept {
    wgpu::AdapterInfo const& info = verboseInfo.it;
    out << info << " architecture '" << info.architecture
        << "' subgroupMinSize " << info.subgroupMinSize
        << " subgroupMaxSize " << info.subgroupMaxSize;
    return out;
}
#endif

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
struct Limit final {
    std::string_view name;
    const std::variant<uint32_t, uint64_t> value;
};

utils::io::ostream& operator<<(utils::io::ostream& out, const Limit limit) noexcept {
    out << limit.name.data() << ": ";
    bool undefined = true;
    if (std::holds_alternative<uint32_t>(limit.value)) {
        if (std::get<uint32_t>(limit.value) != WGPU_LIMIT_U32_UNDEFINED) {
            undefined = false;
            out << std::get<uint32_t>(limit.value);
        }
    } else if (std::holds_alternative<uint64_t>(limit.value)) {
        if (std::get<uint64_t>(limit.value) != WGPU_LIMIT_U64_UNDEFINED) {
            undefined = false;
            out << std::get<uint64_t>(limit.value);
        }
    }
    if (undefined) {
        out << "UNDEFINED";
    }
    return out;
}

void printLimit(std::string_view name, const std::variant<uint32_t, uint64_t> value) {
    FWGPU_LOGI << "  " << Limit{ name, value } << utils::io::endl;
}
#endif// FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printLimits(wgpu::Limits const& limits) {
    printLimit("maxTextureDimension1D", limits.maxTextureDimension1D);
    printLimit("maxTextureDimension2D", limits.maxTextureDimension2D);
    printLimit("maxTextureDimension3D", limits.maxTextureDimension3D);
    printLimit("maxTextureArrayLayers", limits.maxTextureArrayLayers);
    printLimit("maxBindGroups", limits.maxBindGroups);
    printLimit("maxBindGroupsPlusVertexBuffers", limits.maxBindGroupsPlusVertexBuffers);
    printLimit("maxBindingsPerBindGroup", limits.maxBindingsPerBindGroup);
    printLimit("maxDynamicUniformBuffersPerPipelineLayout",
            limits.maxDynamicUniformBuffersPerPipelineLayout);
    printLimit("maxDynamicStorageBuffersPerPipelineLayout",
            limits.maxDynamicStorageBuffersPerPipelineLayout);
    printLimit("maxSampledTexturesPerShaderStage", limits.maxSampledTexturesPerShaderStage);
    printLimit("maxSamplersPerShaderStage", limits.maxSamplersPerShaderStage);
    printLimit("maxStorageBuffersPerShaderStage", limits.maxStorageBuffersPerShaderStage);
    printLimit("maxStorageTexturesPerShaderStage", limits.maxStorageTexturesPerShaderStage);
    printLimit("maxUniformBuffersPerShaderStage", limits.maxUniformBuffersPerShaderStage);
    printLimit("maxUniformBufferBindingSize", limits.maxUniformBufferBindingSize);
    printLimit("maxStorageBufferBindingSize", limits.maxStorageBufferBindingSize);
    printLimit("minUniformBufferOffsetAlignment", limits.minUniformBufferOffsetAlignment);
    printLimit("minStorageBufferOffsetAlignment", limits.minStorageBufferOffsetAlignment);
    printLimit("maxVertexBuffers", limits.maxVertexBuffers);
    printLimit("maxBufferSize", limits.maxBufferSize);
    printLimit("maxVertexAttributes", limits.maxVertexAttributes);
    printLimit("maxVertexBufferArrayStride", limits.maxVertexBufferArrayStride);
    printLimit("maxInterStageShaderVariables", limits.maxInterStageShaderVariables);
    printLimit("maxColorAttachments", limits.maxColorAttachments);
    printLimit("maxColorAttachmentBytesPerSample", limits.maxColorAttachmentBytesPerSample);
    printLimit("maxComputeWorkgroupStorageSize", limits.maxComputeWorkgroupStorageSize);
    printLimit("maxComputeInvocationsPerWorkgroup", limits.maxComputeInvocationsPerWorkgroup);
    printLimit("maxComputeWorkgroupSizeX", limits.maxComputeWorkgroupSizeX);
    printLimit("maxComputeWorkgroupSizeY", limits.maxComputeWorkgroupSizeY);
    printLimit("maxComputeWorkgroupSizeZ", limits.maxComputeWorkgroupSizeZ);
    printLimit("maxComputeWorkgroupsPerDimension", limits.maxComputeWorkgroupsPerDimension);
    printLimit("maxStorageBuffersInVertexStage", limits.maxStorageBuffersInVertexStage);
    printLimit("maxStorageTexturesInVertexStage", limits.maxStorageTexturesInVertexStage);
    printLimit("maxStorageBuffersInFragmentStage", limits.maxStorageBuffersInFragmentStage);
    printLimit("maxStorageTexturesInFragmentStage", limits.maxStorageTexturesInFragmentStage);
}
#endif

struct AdapterDetails final {
    AdapterDetails()
        : AdapterDetails(nullptr) {}
    explicit AdapterDetails(wgpu::Adapter adapter)
        : adapter(std::move(adapter)) {
        info.nextInChain = &powerPreference;
    }
    AdapterDetails(wgpu::AdapterInfo&& info,
            wgpu::DawnAdapterPropertiesPowerPreference& powerPreference, wgpu::Adapter&& adapter)
        : info(std::move(info)),
          powerPreference(powerPreference),
          adapter(std::move(adapter)) {}
    AdapterDetails& operator=(AdapterDetails&& other) noexcept {
        adapter = std::exchange(other.adapter, nullptr);
        info = std::exchange(other.info, {});
        powerPreference = std::exchange(other.powerPreference, {});
        return *this;
    }

    wgpu::AdapterInfo info{};
    wgpu::DawnAdapterPropertiesPowerPreference powerPreference{};
    wgpu::Adapter adapter = nullptr;

    bool operator==(AdapterDetails const& other) const {
        return info.vendorID == other.info.vendorID &&
               info.deviceID == other.info.deviceID &&
               info.backendType == other.info.backendType &&
               info.adapterType == other.info.adapterType &&
               powerPreference.powerPreference == other.powerPreference.powerPreference;
    }
};

template<class STREAM_TYPE>
STREAM_TYPE& operator<<(STREAM_TYPE& out, AdapterDetails const& details) noexcept {
    out << details.info
        << " power preference " << toString(details.powerPreference.powerPreference);
    return out;
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
utils::io::ostream& operator<<(utils::io::ostream& out,
        Verbose<AdapterDetails> const& verboseDetails) noexcept {
    AdapterDetails const& details = verboseDetails.it;
    out << Verbose<wgpu::AdapterInfo>{ details.info }
        << " power preference " << toString(details.powerPreference.powerPreference);
    return out;
}
#endif

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printAdapterDetails(AdapterDetails const& details) {
    FWGPU_LOGI << "Selected WebGPU adapter info: " << Verbose<AdapterDetails>{ details }
               << utils::io::endl;
    wgpu::SupportedFeatures supportedFeatures{};
    details.adapter.GetFeatures(&supportedFeatures);
    FWGPU_LOGI << "WebGPU adapter supported features (" << supportedFeatures.featureCount
               << "):" << utils::io::endl;
    if (supportedFeatures.featureCount > 0 && supportedFeatures.features != nullptr) {
        std::for_each(supportedFeatures.features,
                supportedFeatures.features + supportedFeatures.featureCount,
                [](wgpu::FeatureName const featureName) {
                    FWGPU_LOGI << "  " << featureName << utils::io::endl;
                });
    }
    wgpu::Limits supportedLimits{};
    if (!details.adapter.GetLimits(&supportedLimits)) {
        FWGPU_LOGW << "Failed to get WebGPU adapter supported limits" << utils::io::endl;
    } else {
        FWGPU_LOGI << "WebGPU adapter supported limits:" << utils::io::endl;
        printLimits(supportedLimits);
    }
}
#endif

struct AdapterDetailsHash final {
    size_t operator()(AdapterDetails const& details) const {
        using utils::hash::combine;
        size_t hash = 0;
        combine(hash, details.info.vendorID);
        combine(hash, details.info.deviceID);
        combine(hash, static_cast<uint32_t>(details.info.backendType));
        combine(hash, static_cast<uint32_t>(details.info.adapterType));
        combine(hash, static_cast<uint32_t>(details.powerPreference.powerPreference));
        return hash;
    }
};

[[nodiscard]] bool adapterMeetsMinimumRequirements(AdapterDetails const& details) {
    // check if the adapter has all required features...
    if (!std::all_of(REQUIRED_FEATURES.begin(), REQUIRED_FEATURES.end(),
                [&details](auto const& featureName) {
                    return details.adapter.HasFeature(featureName);
                })) {
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
        FWGPU_LOGI << "WebGPU adapter " << details
                   << " does not have the minimum required features." << utils::io::endl;
        FWGPU_LOGI << "  Missing required feature(s): ";
        for (wgpu::FeatureName const& requiredFeature: REQUIRED_FEATURES) {
            if (!details.adapter.HasFeature(requiredFeature)) {
                FWGPU_LOGI << requiredFeature << " ";
            }
        }
        FWGPU_LOGI << utils::io::endl;
#endif
        return false;
    }
    wgpu::Limits supportedLimits {};
    FILAMENT_CHECK_POSTCONDITION(details.adapter.GetLimits(&supportedLimits))
            << "Failed to get limits for WebGPU adapter: " << details;
    if (!satisfiesLimits(supportedLimits)) {
        FWGPU_LOGI << " (for WebGPU adapter " << details << ")" << utils::io::endl;
        return false;
    }
    return true;
}

// trys to get a set of compatible adapters (whether the meet minimum requirements or not).
// It panics if none are found.
[[nodiscard]] std::unordered_set<AdapterDetails, AdapterDetailsHash> requestCompatibleAdapters(
        wgpu::Instance const& instance, std::vector<wgpu::RequestAdapterOptions> const& requests) {
    // make the series of requests asynchronously, collecting compatible adapter results...
    std::unordered_set<AdapterDetails, AdapterDetailsHash> compatibleAdapters;
    compatibleAdapters.reserve(requests.size());
    std::mutex adaptersMutex;
    std::vector<wgpu::Future> futures(requests.size());
    for (size_t i = 0; i < requests.size(); i++) {
        wgpu::RequestAdapterOptions const& options = requests[i];
        futures[i] = instance.RequestAdapter(&options, wgpu::CallbackMode::WaitAnyOnly,
                [&options, &compatibleAdapters,
                        &adaptersMutex](wgpu::RequestAdapterStatus const status,
                        wgpu::Adapter const& readyAdapter, wgpu::StringView const message) {
                    FILAMENT_CHECK_POSTCONDITION(
                            status != wgpu::RequestAdapterStatus::CallbackCancelled)
                            << "Failed to request a WebGPU adapter due to the request callback "
                               "being cancelled? Options: "
                            << options << " " << message.data;
                    FILAMENT_CHECK_POSTCONDITION(status != wgpu::RequestAdapterStatus::Error)
                            << "Failed to request a WebGPU adapter due to an error. Options: "
                            << options << " Error: " << message.data;
                    if (status == wgpu::RequestAdapterStatus::Success) {
                        AdapterDetails details = AdapterDetails(readyAdapter);
                        FILAMENT_CHECK_POSTCONDITION(readyAdapter.GetInfo(&details.info))
                                << "Failed to get info for adapter (options: " << options << ")";
                        const std::lock_guard<std::mutex> lock(adaptersMutex);
                        compatibleAdapters.emplace(std::move(details.info), details.powerPreference,
                                std::move(details.adapter));
                        return;
                    }
                    assert_invariant(status == wgpu::RequestAdapterStatus::Unavailable);
                });
    }
    // wait for all the results to return...
    for (size_t i = 0; i < futures.size(); i++) {
        wgpu::RequestAdapterOptions const& options = requests[i];
        wgpu::Future& future = futures[i];
        wgpu::WaitStatus status = instance.WaitAny(future, REQUEST_ADAPTER_TIMEOUT_NANOSECONDS);
        FILAMENT_CHECK_POSTCONDITION(status != wgpu::WaitStatus::TimedOut)
                << "Timed out requesting a WebGPU adapter with options " << options;
        FILAMENT_CHECK_POSTCONDITION(status != wgpu::WaitStatus::Error)
                << "Failed to request a WebGPU adapter with options " << options
                << " due to an error (as request was made synchronous)";
        assert_invariant(status == wgpu::WaitStatus::Success);
    }
    FILAMENT_CHECK_POSTCONDITION(!compatibleAdapters.empty()) << "No WebGPU adapters found!";
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    FWGPU_LOGI << compatibleAdapters.size() << " WebGPU adapter(s) found:" << utils::io::endl;
    for (auto& details: compatibleAdapters) {
        FWGPU_LOGI << "  WebGPU adapter: " << details << utils::io::endl;
    }
#endif
    return compatibleAdapters;
}

// selects one preferred adapter or panics if none can be found
wgpu::Adapter selectPreferredAdapter(
        std::unordered_set<AdapterDetails, AdapterDetailsHash> const& compatibleAdapters) {
    // for each unique adapter...
    AdapterDetails const* selectedAdapter = nullptr;
    size_t selectedOptionalFeaturesCount = 0;

    // choose the most desirable adapter that meets the minimum requirements...
    for (AdapterDetails const& details: compatibleAdapters) {
        if (!adapterMeetsMinimumRequirements(details)) {
            continue;
        }
        size_t supportedOptionalFeaturesCount = std::count_if(OPTIONAL_FEATURES.begin(),
                OPTIONAL_FEATURES.end(), [&details](auto const& featureName) {
                    return details.adapter.HasFeature(featureName);
                });
        // select this one if it is the first (perhaps it is the only one anyway)...
        if (selectedAdapter == nullptr) {
            selectedAdapter = &details;
            selectedOptionalFeaturesCount = supportedOptionalFeaturesCount;
            continue;
        }
        // first, prefer higher performance...
        if (details.powerPreference.powerPreference >
                selectedAdapter->powerPreference.powerPreference) {
            selectedAdapter = &details;
            selectedOptionalFeaturesCount = supportedOptionalFeaturesCount;
            continue;
        }
        // second, prefer more optional features supported...
        if (supportedOptionalFeaturesCount > selectedOptionalFeaturesCount) {
            selectedAdapter = &details;
            selectedOptionalFeaturesCount = supportedOptionalFeaturesCount;
            continue;
        }
        // otherwise keep the selected adapter
        assert_invariant(selectedAdapter != nullptr);
    }
    FILAMENT_CHECK_POSTCONDITION(selectedAdapter != nullptr)
            << "Could not find a WebGPU adapter that meets the minimum requirements.";
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printAdapterDetails(*selectedAdapter);
#endif
    return selectedAdapter->adapter;
}

[[nodiscard]] std::string_view toString(const wgpu::DeviceLostReason reason) {
    switch (reason) {
        case wgpu::DeviceLostReason::Unknown:           return "UNKNOWN";
        case wgpu::DeviceLostReason::Destroyed:         return "DESTROYED";
        case wgpu::DeviceLostReason::CallbackCancelled: return "CALLBACK_CANCELLED";
        case wgpu::DeviceLostReason::FailedCreation:    return "FAILED_CREATION";
    }
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printDeviceDetails(wgpu::Device const& device) {
    wgpu::SupportedFeatures supportedFeatures{};
    device.GetFeatures(&supportedFeatures);
    FWGPU_LOGI << "WebGPU device supported features (" << supportedFeatures.featureCount
               << "):" << utils::io::endl;
    if (supportedFeatures.featureCount > 0 && supportedFeatures.features != nullptr) {
        std::for_each(supportedFeatures.features,
                supportedFeatures.features + supportedFeatures.featureCount,
                [](wgpu::FeatureName const featureName) {
                    FWGPU_LOGI << "  " << featureName << utils::io::endl;
                });
    }
    wgpu::Limits supportedLimits{};
    if (!device.GetLimits(&supportedLimits)) {
        FWGPU_LOGW << "Failed to get WebGPU supported device limits" << utils::io::endl;
    } else {
        FWGPU_LOGI << "WebGPU device supported limits:" << utils::io::endl;
        printLimits(supportedLimits);
    }
}
#endif

}// namespace

wgpu::Adapter WebGPUPlatform::requestAdapter(wgpu::Surface const& surface) {
    assertLimitsAreExpressedInRequirementsStruct();
    std::vector<wgpu::RequestAdapterOptions> requests = getAdapterOptions();
    for (auto& request: requests) {
        request.compatibleSurface = surface;
    }
    const std::unordered_set<AdapterDetails, AdapterDetailsHash> compatibleAdapters =
            requestCompatibleAdapters(mInstance, requests);
    return selectPreferredAdapter(compatibleAdapters);
}

wgpu::Device WebGPUPlatform::requestDevice(wgpu::Adapter const& adapter) {
    std::vector<wgpu::FeatureName> enabledFeatures;
    enabledFeatures.reserve(REQUIRED_FEATURES.size() + OPTIONAL_FEATURES.size());
    for (auto const& requiredFeature : REQUIRED_FEATURES) {
        enabledFeatures.push_back(requiredFeature);
    }
    for (auto const& optionalFeature : OPTIONAL_FEATURES) {
        if (adapter.HasFeature(optionalFeature)) {
            enabledFeatures.push_back(optionalFeature);
        }
    }
    wgpu::DeviceDescriptor deviceDescriptor{};
    deviceDescriptor.label = "graphics_device";
    deviceDescriptor.defaultQueue.label = "default_queue";
    deviceDescriptor.requiredFeatureCount = enabledFeatures.size();
    deviceDescriptor.requiredFeatures = enabledFeatures.data();
    deviceDescriptor.requiredLimits = &REQUIRED_LIMITS;
    deviceDescriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
            [](wgpu::Device const&, wgpu::DeviceLostReason const& reason,
                    wgpu::StringView message) {
                if (reason == wgpu::DeviceLostReason::Destroyed) {
#if FWGPU_ENABLED(FWGPU_DEBUG_VALIDATION)
                    FWGPU_LOGD << "WebGPU device lost due to being destroyed (expected)"
                               << utils::io::endl;
#endif
                    return;
                }
                // TODO try recreating the device instead of just panicking
                FILAMENT_CHECK_POSTCONDITION(reason != wgpu::DeviceLostReason::Destroyed)
                        << "WebGPU device lost: " << toString(reason) << " " << message.data;
            });
    deviceDescriptor.SetUncapturedErrorCallback(
            [](wgpu::Device const&, wgpu::ErrorType errorType, wgpu::StringView message) {
                FWGPU_LOGE << "WebGPU device error: " << toString(errorType) << " " << message.data
                           << utils::io::endl;
            });
    wgpu::Device device = nullptr;
    wgpu::WaitStatus status = mInstance.WaitAny(
            adapter.RequestDevice(&deviceDescriptor, wgpu::CallbackMode::WaitAnyOnly,
                    [&device](wgpu::RequestDeviceStatus const status,
                            wgpu::Device const& readyDevice, wgpu::StringView const message) {
                        FILAMENT_CHECK_POSTCONDITION(
                                status != wgpu::RequestDeviceStatus::CallbackCancelled)
                                << "Failed to request a WebGPU device due to the callback being "
                                   "cancelled? "
                                << message;
                        FILAMENT_CHECK_POSTCONDITION(status != wgpu::RequestDeviceStatus::Error)
                                << "Failed to request a WebGPU device due to en error: " << message;
                        assert_invariant(status == wgpu::RequestDeviceStatus::Success);
                        device = readyDevice;
                    }),
            REQUEST_DEVICE_TIMEOUT_NANOSECONDS);
    FILAMENT_CHECK_POSTCONDITION(status != wgpu::WaitStatus::TimedOut)
            << "Failed to request a WebGPU device due to a timeout.";
    FILAMENT_CHECK_POSTCONDITION(status != wgpu::WaitStatus::Error)
            << "Failed to request a WebGPU device due to an error.";
    assert_invariant(status == wgpu::WaitStatus::Success);
    FILAMENT_CHECK_POSTCONDITION(device != nullptr)
            << "Failed to get a WebGPU device for the platform. null device returned?";
    size_t missingFeatures = 0;
    std::stringstream featureNamesStream;
    for (wgpu::FeatureName const& enabledFeature : enabledFeatures) {
        if (!device.HasFeature(enabledFeature)) {
            missingFeatures += 1;
            featureNamesStream << enabledFeature << " ";
        }
    }
    if (missingFeatures > 0) {
        PANIC_POSTCONDITION("WebGPU device is missing %d requested feature(s) even though the "
                            "adapter should support them: %s\n",
                missingFeatures, featureNamesStream.str().data());
    }
    wgpu::Limits supportedLimits {};
    FILAMENT_CHECK_POSTCONDITION(device.GetLimits(&supportedLimits))
            << "Failed to get limits for the device?";
    FILAMENT_CHECK_POSTCONDITION(satisfiesLimits(supportedLimits))
            << "WebGPU device failed to statify required limits.";
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printDeviceDetails(device);
#endif
    return device;
}

Driver* WebGPUPlatform::createDriver(void* sharedContext,
        const Platform::DriverConfig& driverConfig) noexcept {
    if (sharedContext) {
        FWGPU_LOGW << "sharedContext is ignored/unused in the WebGPU backend. A non-null "
                      "sharedContext was provided, but it will be ignored."
                   << utils::io::endl;
    }
    return WebGPUDriver::create(*this, driverConfig);
}

WebGPUPlatform::WebGPUPlatform()
    : mInstance(createInstance()) {}

}// namespace filament::backend
