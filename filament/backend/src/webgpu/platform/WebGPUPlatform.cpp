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
#include "webgpu/WebGPUStrings.h"

#include <backend/DriverEnums.h>
#include <backend/Platform.h>
#include <utils/Hash.h>
#include <utils/Panic.h>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <sstream>// for one-time-ish setup string concatenation, namely error messaging
#include <unordered_set>
#include <utility>
#include <vector>
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
#include <string>
#include <string_view>
#include <variant>
#endif

/**
 * WebGPU Backend implementation common across platforms or operating systems (at least for now).
 * Some of these functions may likely be refactored to platform/OS-specific implementations
 * over time as needed. The caller of the WebGPUPlatform doesn't need to care which is the case.
 */

namespace filament::backend {

namespace {

// The SPD Algorithm can make use of up to 12 storage texture attachments
constexpr uint32_t MAX_MIPMAP_STORAGE_TEXTURES_PER_STAGE = 12u;

constexpr std::array REQUIRED_FEATURES = {
    wgpu::FeatureName::TransientAttachments,
    // Qualcomm 500 and 600 GPUs do not support this so it is not part of core webgpu spec. To
    // support such devices, we will either need Filament to not attempt this, or find another
    // workaround. https://github.com/gpuweb/gpuweb/issues/2648
    wgpu::FeatureName::RG11B10UfloatRenderable,
    // necessary for blit conversions of formats like RGBA32Float...
    wgpu::FeatureName::Float32Filterable,
};

constexpr std::array OPTIONAL_FEATURES = {
    wgpu::FeatureName::DepthClipControl,
    wgpu::FeatureName::Depth32FloatStencil8,
    wgpu::FeatureName::CoreFeaturesAndLimits };

enum class LimitToValidate : uint8_t {
    begin = 0,// needs to be first for iterating through all possible values in the enum
    MAX_BIND_GROUPS,
    MAX_BINDINGS_PER_BIND_GROUP,
    MAX_SAMPLERS_PER_SHADER_STAGE,
    MAX_STORAGE_BUFFERS_PER_SHADER_STAGE,
    MAX_VERTEX_BUFFERS,
    MAX_VERTEX_ATTRIBUTES,
    end// needs to be last for iterating through all possible values in the enum
       // (Sentinel value)
};

// Only the attributes identified by the LimitToValidate enum above will actually be validated
// at runtime. Thus, if you add a limit here add the associated enum value in LimitToValidate!
constexpr wgpu::Limits REQUIRED_LIMITS = {
    .maxBindGroups = filament::backend::MAX_DESCRIPTOR_SET_COUNT,
    .maxBindingsPerBindGroup = filament::backend::MAX_DESCRIPTOR_COUNT,
    .maxSamplersPerShaderStage = 16, // TODO should be set to filament::backend::MAX_SAMPLER_COUNT,
    .maxStorageBuffersPerShaderStage = filament::backend::MAX_SSBO_COUNT,
    .maxVertexBuffers = 8, // TODO should be set to filament::backend::MAX_VERTEX_BUFFER_COUNT,
    .maxVertexAttributes = filament::backend::MAX_VERTEX_ATTRIBUTE_COUNT,
};

constexpr void forEachLimitToValidate(std::function<bool(LimitToValidate const)> const& func) {
    for (auto limit = static_cast<uint8_t>(LimitToValidate::begin) + 1;
            limit < static_cast<uint8_t>(LimitToValidate::end); limit++) {
        auto castedLimit = static_cast<LimitToValidate>(limit);
#ifndef NDEBUG
        bool castedToValidValue = false;
        switch (castedLimit) {
            case LimitToValidate::begin:
            case LimitToValidate::MAX_BIND_GROUPS:
            case LimitToValidate::MAX_BINDINGS_PER_BIND_GROUP:
            case LimitToValidate::MAX_SAMPLERS_PER_SHADER_STAGE:
            case LimitToValidate::MAX_STORAGE_BUFFERS_PER_SHADER_STAGE:
            case LimitToValidate::MAX_VERTEX_BUFFERS:
            case LimitToValidate::MAX_VERTEX_ATTRIBUTES:
            case LimitToValidate::end:
                castedToValidValue = true;
                break;
        }
        assert_invariant(castedToValidValue &&
                         "LimitToValidate enum values are not sequentially incremented by 1? "
                         "Compilers normally do this by default; which compiler was used here?");
#endif
        if (func(castedLimit)) {
            break;
        }
    }
}

constexpr bool isDefined(uint32_t limit) { return limit < wgpu::kLimitU32Undefined; }

[[maybe_unused]] constexpr bool isDefined(uint64_t limit) {
    return limit < wgpu::kLimitU64Undefined;
}

void assertLimitsAreExpressedInRequirementsStruct() {
    forEachLimitToValidate([](LimitToValidate const limit) {
        switch (limit) {
            case LimitToValidate::MAX_BIND_GROUPS:
                static_assert(isDefined(REQUIRED_LIMITS.maxBindGroups));
                break;
            case LimitToValidate::MAX_BINDINGS_PER_BIND_GROUP:
                static_assert(isDefined(REQUIRED_LIMITS.maxBindingsPerBindGroup));
                break;
            case LimitToValidate::MAX_SAMPLERS_PER_SHADER_STAGE:
                static_assert(isDefined(REQUIRED_LIMITS.maxSamplersPerShaderStage));
                break;
            case LimitToValidate::MAX_STORAGE_BUFFERS_PER_SHADER_STAGE:
                static_assert(isDefined(REQUIRED_LIMITS.maxStorageBuffersPerShaderStage));
                break;
            case LimitToValidate::MAX_VERTEX_BUFFERS:
                static_assert(isDefined(REQUIRED_LIMITS.maxVertexBuffers));
                break;
            case LimitToValidate::MAX_VERTEX_ATTRIBUTES:
                static_assert(isDefined(REQUIRED_LIMITS.maxVertexAttributes));
                break;
            case LimitToValidate::begin:
            case LimitToValidate::end:
                break;
        }
        return false; // should not break
    });
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
        case LimitToValidate::begin:
        case LimitToValidate::end:
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
        case LimitToValidate::begin:
        case LimitToValidate::end:
            return 0;
    }
}

[[nodiscard]] bool satisfiesLimits(wgpu::Limits const& limits) {
    size_t failedLimitCount = 0;
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    std::stringstream failedLimitsStream;
#endif
    forEachLimitToValidate([&](LimitToValidate const limit) {
        uint64_t supportedValue = valueForLimit(limits, limit);
        uint64_t requiredValue = valueForLimit(REQUIRED_LIMITS, limit);
        if (supportedValue < requiredValue) {
            failedLimitCount++;
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
            failedLimitsStream << toString(limit) << " required " << requiredValue << " but found "
                               << supportedValue << ". ";
#else
            return true;// break early if not printing anything
#endif
        }
        return false;// don't break, keep going
    });
    if (failedLimitCount == 0) {
        return true;
    }
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    FWGPU_LOGI << "Failed to satisfy " << failedLimitCount
               << " limit(s): " << failedLimitsStream.str();
#endif
    return false;
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printInstanceDetails(wgpu::Instance const& instance) {
    wgpu::SupportedWGSLLanguageFeatures supportedWGSLLanguageFeatures{};
    if (!instance.GetWGSLLanguageFeatures(&supportedWGSLLanguageFeatures)) {
        FWGPU_LOGW << "Failed to get WebGPU instance supported WGSL language features";
    } else {
        FWGPU_LOGI << "WebGPU instance supported WGSL language features ("
                   << supportedWGSLLanguageFeatures.featureCount << "):";
        if (supportedWGSLLanguageFeatures.featureCount > 0 &&
                supportedWGSLLanguageFeatures.features != nullptr) {
            std::for_each(supportedWGSLLanguageFeatures.features,
                    supportedWGSLLanguageFeatures.features +
                            supportedWGSLLanguageFeatures.featureCount,
                    [](wgpu::WGSLLanguageFeatureName const featureName) {
                        FWGPU_LOGI << "  " << webGPUPrintableToString(featureName);
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
    FWGPU_LOGI << "setting on toggle enable_immediate_error_handling";
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
    const wgpu::InstanceFeatureName features[] = {wgpu::InstanceFeatureName::TimedWaitAny};
    wgpu::InstanceDescriptor instanceDescriptor{
        .nextInChain = &dawnTogglesDescriptor,
        .requiredFeatures = features,
//        .requiredFeatureCount = std::size(features),
    };
//    instanceDescriptor.requiredFeatures = features;
    instanceDescriptor.requiredFeatureCount = 1;
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(instance != nullptr) << "Unable to create WebGPU instance.";
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printInstanceDetails(instance);
#endif
    return instance;
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printLimit(std::string_view name, const std::variant<uint32_t, uint64_t> value) {
    static constexpr std::string_view indent = "  ";
    bool undefined = true;
    if (std::holds_alternative<uint32_t>(value)) {
        if (std::get<uint32_t>(value) != WGPU_LIMIT_U32_UNDEFINED) {
            undefined = false;
            FWGPU_LOGI << indent << name.data() << ": " << std::get<uint32_t>(value);
        }
    } else if (std::holds_alternative<uint64_t>(value)) {
        if (std::get<uint64_t>(value) != WGPU_LIMIT_U64_UNDEFINED) {
            undefined = false;
            FWGPU_LOGI << indent << name.data() << ": " << std::get<uint64_t>(value);
        }
    }
    if (undefined) {
        FWGPU_LOGI << indent << name.data() << ": UNDEFINED";
    }
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

[[nodiscard]] std::string toString(AdapterDetails const& details) {
    std::stringstream out;
    out << adapterInfoToString(details.info)
        << " power preference " << powerPreferenceToString(details.powerPreference);
    return out.str();
}

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printAdapterDetails(AdapterDetails const& details) {
    FWGPU_LOGI << "Selected WebGPU adapter info: " << toString(details);
    wgpu::SupportedFeatures supportedFeatures{};
    details.adapter.GetFeatures(&supportedFeatures);
    FWGPU_LOGI << "WebGPU adapter supported features (" << supportedFeatures.featureCount
               << "):";
    if (supportedFeatures.featureCount > 0 && supportedFeatures.features != nullptr) {
        std::for_each(supportedFeatures.features,
                supportedFeatures.features + supportedFeatures.featureCount,
                [](wgpu::FeatureName const featureName) {
                    FWGPU_LOGI << "  " << webGPUPrintableToString(featureName);
                });
    }
    wgpu::Limits supportedLimits{};
    if (!details.adapter.GetLimits(&supportedLimits)) {
        FWGPU_LOGW << "Failed to get WebGPU adapter supported limits";
    } else {
        FWGPU_LOGI << "WebGPU adapter supported limits:";
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
        FWGPU_LOGI << "WebGPU adapter " << toString(details)
                   << " does not have the minimum required features.";
        FWGPU_LOGI << "Missing required feature(s): ";
        for (wgpu::FeatureName const& requiredFeature: REQUIRED_FEATURES) {
            if (!details.adapter.HasFeature(requiredFeature)) {
                FWGPU_LOGI << webGPUPrintableToString(requiredFeature);
            }
        }
        FWGPU_LOGI << "";
#endif
        return false;
    }
    wgpu::Limits supportedLimits {};
    FILAMENT_CHECK_POSTCONDITION(details.adapter.GetLimits(&supportedLimits))
            << "Failed to get limits for WebGPU adapter: " << toString(details);
    if (!satisfiesLimits(supportedLimits)) {
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
        FWGPU_LOGI << " (for WebGPU adapter " << toString(details) << ")";
#endif
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
                            << adapterOptionsToString(options) << " " << message.data;
                    FILAMENT_CHECK_POSTCONDITION(status != wgpu::RequestAdapterStatus::Error)
                            << "Failed to request a WebGPU adapter due to an error. Options: "
                            << adapterOptionsToString(options) << " Error: " << message.data;
                    if (status == wgpu::RequestAdapterStatus::Success) {
                        AdapterDetails details = AdapterDetails(readyAdapter);
                        FILAMENT_CHECK_POSTCONDITION(readyAdapter.GetInfo(&details.info))
                                << "Failed to get info for adapter (options: "
                                << adapterOptionsToString(options) << ")";
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
        wgpu::WaitStatus status =
                instance.WaitAny(future, FILAMENT_WEBGPU_REQUEST_ADAPTER_TIMEOUT_NANOSECONDS);
        FILAMENT_CHECK_POSTCONDITION(status != wgpu::WaitStatus::TimedOut)
                << "Timed out requesting a WebGPU adapter with options "
                << adapterOptionsToString(options);
        FILAMENT_CHECK_POSTCONDITION(status != wgpu::WaitStatus::Error)
                << "Failed to request a WebGPU adapter with options "
                << adapterOptionsToString(options)
                << " due to an error (as request was made synchronous)";
        assert_invariant(status == wgpu::WaitStatus::Success);
    }
    FILAMENT_CHECK_POSTCONDITION(!compatibleAdapters.empty()) << "No WebGPU adapters found!";
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    FWGPU_LOGI << compatibleAdapters.size() << " WebGPU adapter(s) found:";
    for (auto& details: compatibleAdapters) {
        FWGPU_LOGI << "  WebGPU adapter: " << toString(details);
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
        // NOTE: we can make this selection logic more dynamic, configurable,
        // and robust in the future as needed, such as the app preferring lower power
        // (prioritizing battery life) or weighting specific optional features in its decision, etc.
        // This is just a good start that works for now.
        //
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

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printDeviceDetails(wgpu::Device const& device) {
    wgpu::SupportedFeatures supportedFeatures{};
    device.GetFeatures(&supportedFeatures);
    FWGPU_LOGI << "WebGPU device supported features (" << supportedFeatures.featureCount
               << "):";
    if (supportedFeatures.featureCount > 0 && supportedFeatures.features != nullptr) {
        std::for_each(supportedFeatures.features,
                supportedFeatures.features + supportedFeatures.featureCount,
                [](wgpu::FeatureName const featureName) {
                    FWGPU_LOGI << "  " << webGPUPrintableToString(featureName);
                });
    }
    wgpu::Limits supportedLimits{};
    if (!device.GetLimits(&supportedLimits)) {
        FWGPU_LOGW << "Failed to get WebGPU supported device limits";
    } else {
        FWGPU_LOGI << "WebGPU device supported limits:";
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

    // It is helpful to increase maxStorageTexturesPerShaderStage if the hardware supports it, but
    // not required. This allows our mipmap generation to perform as well as possible for the given
    // hardware. If we have more cases of "Nice to increase" limits like this we can generalize it
    // in the future.
    wgpu::Limits supportedLimits{};
    FILAMENT_CHECK_POSTCONDITION(
            adapter.GetLimits(&supportedLimits).status == wgpu::Status::Success)
            << "Failed to get limits for WebGPU adapter";
    auto limitsToRequest = REQUIRED_LIMITS;
    limitsToRequest.maxStorageTexturesPerShaderStage =
            std::min(MAX_MIPMAP_STORAGE_TEXTURES_PER_STAGE,
                    supportedLimits.maxStorageTexturesPerShaderStage);
    deviceDescriptor.requiredLimits = &limitsToRequest;

    deviceDescriptor.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
            [](wgpu::Device const&, wgpu::DeviceLostReason const& reason,
                    wgpu::StringView message) {
                if (reason == wgpu::DeviceLostReason::Destroyed) {
#if FWGPU_ENABLED(FWGPU_DEBUG_VALIDATION)
                    FWGPU_LOGD << "WebGPU device lost due to being destroyed (expected)";
#endif
                    return;
                }
                // TODO try recreating the device instead of just panicking
                FILAMENT_CHECK_POSTCONDITION(reason != wgpu::DeviceLostReason::Destroyed)
                        << "WebGPU device lost: " << deviceLostReasonToString(reason) << " "
                        << message.data;
            });
    deviceDescriptor.SetUncapturedErrorCallback(
            [](wgpu::Device const&, wgpu::ErrorType errorType, wgpu::StringView message) {
                FWGPU_LOGE << "WebGPU device error: " << errorTypeToString(errorType) << " "
                           << message.data;
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
            FILAMENT_WEBGPU_REQUEST_DEVICE_TIMEOUT_NANOSECONDS);
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
                      "sharedContext was provided, but it will be ignored.";
    }
    return WebGPUDriver::create(*this, driverConfig);
}

WebGPUPlatform::WebGPUPlatform()
    : mInstance(createInstance()) {}

}// namespace filament::backend
