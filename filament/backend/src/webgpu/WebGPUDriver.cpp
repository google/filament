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

#include "webgpu/WebGPUDriver.h"

#include "WebGPUSwapChain.h"
#include "webgpu/WebGPUConstants.h"
#include <backend/platforms/WebGPUPlatform.h>

#include "CommandStreamDispatcher.h"
#include "DriverBase.h"
#include "private/backend/Dispatcher.h"
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <math/mat3.h>
#include <utils/CString.h>
#include <utils/ostream.h>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>
#include <variant>

#define GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define FWGPU_PRINT_DRIVER_CALL(...)                                                               \
    GET_MACRO(_0, ##__VA_ARGS__,                                                                   \
            FWGPU_PRINT_DRIVER_CALL_10,                                                            \
            FWGPU_PRINT_DRIVER_CALL_9,                                                             \
            FWGPU_PRINT_DRIVER_CALL_8,                                                             \
            FWGPU_PRINT_DRIVER_CALL_7,                                                             \
            FWGPU_PRINT_DRIVER_CALL_6,                                                             \
            FWGPU_PRINT_DRIVER_CALL_5,                                                             \
            FWGPU_PRINT_DRIVER_CALL_4,                                                             \
            FWGPU_PRINT_DRIVER_CALL_3,                                                             \
            FWGPU_PRINT_DRIVER_CALL_2,                                                             \
            FWGPU_PRINT_DRIVER_CALL_1,                                                             \
            FWGPU_PRINT_DRIVER_CALL_0)(__VA_ARGS__)

#define FWGPU_PRINT_DRIVER_CALL_0()                                                                  \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "()" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_1(arg1)                                                             \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ")"          \
                << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_2(arg1, arg2)                                                      \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ")" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_3(arg1, arg2, arg3)                                                \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ")" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_4(arg1, arg2, arg3, arg4)                                          \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ", " << #arg4 << "="    \
                << arg4 << ")" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_5(arg1, arg2, arg3, arg4, arg5)                                    \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ", " << #arg4 << "="    \
                << arg4 << ", " << #arg5 << "=" << arg5 << ")" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_6(arg1, arg2, arg3, arg4, arg5, arg6)                              \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ", " << #arg4 << "="    \
                << arg4 << ", " << #arg5 << "=" << arg5 << ", " << #arg6 << "=" << arg6 << ")"        \
                << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_7(arg1, arg2, arg3, arg4, arg5, arg6, arg7)                        \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ", " << #arg4 << "="    \
                << arg4 << ", " << #arg5 << "=" << arg5 << ", " << #arg6 << "=" << arg6 << ", "    \
                << #arg7 << "=" << arg7 << ")" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_8(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)                  \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ", " << #arg4 << "="    \
                << arg4 << ", " << #arg5 << "=" << arg5 << ", " << #arg6 << "=" << arg6 << ", "    \
                << #arg7 << "=" << arg7 << ", " << #arg8 << "=" << arg8 << ")" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_9(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)            \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ", " << #arg4 << "="    \
                << arg4 << ", " << #arg5 << "=" << arg5 << ", " << #arg6 << "=" << arg6 << ", "    \
                << #arg7 << "=" << arg7 << ", " << #arg8 << "=" << arg8 << ", " << #arg9 << "="    \
                << arg9 << ")" << utils::io::endl)

#define FWGPU_PRINT_DRIVER_CALL_10(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)    \
    (FWGPU_LOGD << "WebGPUDriver::" << __FUNCTION__ << "(" << #arg1 << "=" << arg1 << ", "         \
                << #arg2 << "=" << arg2 << ", " << #arg3 << "=" << arg3 << ", " << #arg4 << "="    \
                << arg4 << ", " << #arg5 << "=" << arg5 << ", " << #arg6 << "=" << arg6 << ", "    \
                << #arg7 << "=" << arg7 << ", " << #arg8 << "=" << arg8 << ", " << #arg9 << "="    \
                << arg9 << ", " << #arg10 << "=" << arg10 << ")" << utils::io::endl)

namespace filament::backend {

namespace {

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
                        std::stringstream nameStream{};
                        nameStream << featureName;
                        FWGPU_LOGI << "  " << nameStream.str() << utils::io::endl;
                    });
        }
    }
}
#endif

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printLimit(std::string_view name, const std::variant<uint32_t, uint64_t> value) {
    FWGPU_LOGI << "  " << name.data() << ": ";
    bool undefined = true;
    if (std::holds_alternative<uint32_t>(value)) {
        if (std::get<uint32_t>(value) != WGPU_LIMIT_U32_UNDEFINED) {
            undefined = false;
            FWGPU_LOGI << std::get<uint32_t>(value);
        }
    } else if (std::holds_alternative<uint64_t>(value)) {
        if (std::get<uint64_t>(value) != WGPU_LIMIT_U64_UNDEFINED) {
            undefined = false;
            FWGPU_LOGI << std::get<uint64_t>(value);
        }
    }
    if (undefined) {
        FWGPU_LOGI << "UNDEFINED";
    }
    FWGPU_LOGI << utils::io::endl;
}
#endif

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

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printAdapterDetails(wgpu::Adapter const& adapter) {
    wgpu::DawnAdapterPropertiesPowerPreference powerPreferenceProperties{};
    wgpu::AdapterInfo adapterInfo{};
    adapterInfo.nextInChain = &powerPreferenceProperties;
    if (!adapter.GetInfo(&adapterInfo)) {
        FWGPU_LOGW << "Failed to get WebGPU adapter info" << utils::io::endl;
    } else {
        std::stringstream backendTypeStream{};
        backendTypeStream << adapterInfo.backendType;
        std::stringstream adapterTypeStream{};
        adapterTypeStream << adapterInfo.adapterType;
        std::stringstream powerPreferenceStream{};
        powerPreferenceStream << powerPreferenceProperties.powerPreference;
        FWGPU_LOGI << "WebGPU adapter info:" << utils::io::endl;
        FWGPU_LOGI << "  vendor: " << adapterInfo.vendor.data << utils::io::endl;
        FWGPU_LOGI << "  architecture: " << adapterInfo.architecture.data << utils::io::endl;
        FWGPU_LOGI << "  device: " << adapterInfo.device.data << utils::io::endl;
        FWGPU_LOGI << "  description: " << adapterInfo.description.data << utils::io::endl;
        FWGPU_LOGI << "  backend type: " << backendTypeStream.str().data() << utils::io::endl;
        FWGPU_LOGI << "  adapter type: " << adapterTypeStream.str().data() << utils::io::endl;
        FWGPU_LOGI << "  device ID: " << adapterInfo.deviceID << utils::io::endl;
        FWGPU_LOGI << "  vendor ID: " << adapterInfo.vendorID << utils::io::endl;
        FWGPU_LOGI << "  subgroup min size: " << adapterInfo.subgroupMinSize << utils::io::endl;
        FWGPU_LOGI << "  subgroup max size: " << adapterInfo.subgroupMaxSize << utils::io::endl;
        FWGPU_LOGI << "  power preference: " << powerPreferenceStream.str() << utils::io::endl;
    }
    wgpu::SupportedFeatures supportedFeatures{};
    adapter.GetFeatures(&supportedFeatures);
    FWGPU_LOGI << "WebGPU adapter supported features (" << supportedFeatures.featureCount
               << "):" << utils::io::endl;
    if (supportedFeatures.featureCount > 0 && supportedFeatures.features != nullptr) {
        std::for_each(supportedFeatures.features,
                supportedFeatures.features + supportedFeatures.featureCount,
                [](wgpu::FeatureName const name) {
                    std::stringstream nameStream{};
                    nameStream << name;
                    FWGPU_LOGI << "  " << nameStream.str().data() << utils::io::endl;
                });
    }
    wgpu::Limits supportedLimits{};
    if (!adapter.GetLimits(&supportedLimits)) {
        FWGPU_LOGW << "Failed to get WebGPU adapter supported limits" << utils::io::endl;
    } else {
        FWGPU_LOGI << "WebGPU adapter supported limits:" << utils::io::endl;
        printLimits(supportedLimits);
    }
}
#endif



#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printDeviceDetails(wgpu::Device const& device) {
    wgpu::SupportedFeatures supportedFeatures{};
    device.GetFeatures(&supportedFeatures);
    FWGPU_LOGI << "WebGPU device supported features (" << supportedFeatures.featureCount
               << "):" << utils::io::endl;
    if (supportedFeatures.featureCount > 0 && supportedFeatures.features != nullptr) {
        std::for_each(supportedFeatures.features,
                supportedFeatures.features + supportedFeatures.featureCount,
                [](wgpu::FeatureName const name) {
                    std::stringstream nameStream{};
                    nameStream << name;
                    FWGPU_LOGI << "  " << nameStream.str().data() << utils::io::endl;
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

#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
void driverConfigString(filament::backend::Platform::DriverConfig const& config,
        std::stringstream& s) {
    std::string_view stereoscopicType;
    switch (config.stereoscopicType) {
        case StereoscopicType::NONE:
            stereoscopicType = "NONE";
            break;
        case StereoscopicType::INSTANCED:
            stereoscopicType = "INSTANCED";
            break;
        case StereoscopicType::MULTIVIEW:
            stereoscopicType = "MULTIVIEW";
            break;
    }
    const auto boolString = [](bool b) { return b ? "true" : "false"; };
    s << "{" << "handleArenaSize:" << config.handleArenaSize
      << " metalUploadBufferSizeBytes:" << config.metalUploadBufferSizeBytes
      << " disableParallelShaderCompile:" << boolString(config.disableParallelShaderCompile)
      << " disableHandleUseAfterFreeCheck:" << boolString(config.disableHandleUseAfterFreeCheck)
      << " disableHeapHandleTags:" << boolString(config.disableHeapHandleTags)
      << " forceGLES2Context:" << boolString(config.forceGLES2Context)
      << " stereoscopicType:" << stereoscopicType
      << " assertNativeWindowIsValid: " << boolString(config.assertNativeWindowIsValid) << " }";
}
#endif

}// namespace

Driver* WebGPUDriver::create(WebGPUPlatform& platform, const Platform::DriverConfig& driverConfig) noexcept {
    constexpr size_t defaultSize = FILAMENT_WEBGPU_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U;
    Platform::DriverConfig validConfig {driverConfig};
    validConfig.handleArenaSize = std::max(driverConfig.handleArenaSize, defaultSize);
    return new WebGPUDriver(platform, validConfig);
}

WebGPUDriver::WebGPUDriver(WebGPUPlatform& platform, const Platform::DriverConfig& driverConfig) noexcept
    : mPlatform(platform),
      mHandleAllocator("Handles",
              driverConfig.handleArenaSize,
              driverConfig.disableHandleUseAfterFreeCheck,
              driverConfig.disableHeapHandleTags) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    std::stringstream config;
    driverConfigString(driverConfig, config);
    FWGPU_PRINT_DRIVER_CALL("platform", config.str());
#endif
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printInstanceDetails(mPlatform.getInstance());
#endif
}

WebGPUDriver::~WebGPUDriver() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL();
#endif
}

Dispatcher WebGPUDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<WebGPUDriver>::make();
}

ShaderModel WebGPUDriver::getShaderModel() const noexcept {
#if defined(__ANDROID__) || defined(FILAMENT_IOS) || defined(__EMSCRIPTEN__)
    return ShaderModel::MOBILE;
#else
    return ShaderModel::DESKTOP;
#endif
}

ShaderLanguage WebGPUDriver::getShaderLanguage() const noexcept {
    return ShaderLanguage::WGSL;
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<WebGPUDriver>;


void WebGPUDriver::terminate() {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL();
#endif
}

void WebGPUDriver::tick(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
    mDevice.Tick();
}

void WebGPUDriver::beginFrame(int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs, uint32_t frameId) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(monotonic_clock_ns, refreshIntervalNs, frameId);
#endif
}

void WebGPUDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, FrameScheduledCallback&& callback, uint64_t flags) {

}

void WebGPUDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, utils::Invocable<void(void)>&& callback) {

}

void WebGPUDriver::setPresentationTime(int64_t monotonic_clock_ns) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(monotonic_clock_ns);
#endif
}

void WebGPUDriver::endFrame(uint32_t frameId) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(frameId);
#endif
}

void WebGPUDriver::flush(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
}

void WebGPUDriver::finish(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
}

void WebGPUDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(rph);
#endif
   if (rph) {
        destructHandle<WGPURenderPrimitive>(rph);
    }
}

void WebGPUDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(vbih);
#endif
   if (vbih) {
        destructHandle<WGPUVertexBufferInfo>(vbih);
    }
}

void WebGPUDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(vbh);
#endif
    if (vbh) {
        destructHandle<WGPUVertexBuffer>(vbh);
    }
}

void WebGPUDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(ibh);
#endif
    if (ibh) {
        destructHandle<WGPUIndexBuffer>(ibh);
    }
}

void WebGPUDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(boh);
#endif
}

void WebGPUDriver::destroyTexture(Handle<HwTexture> th) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th);
#endif
}

void WebGPUDriver::destroyProgram(Handle<HwProgram> ph) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(ph);
#endif
}

void WebGPUDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(rth);
#endif
}

void WebGPUDriver::destroySwapChain(Handle<HwSwapChain> sch) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sch);
#endif
    if (sch) {
        destructHandle<WebGPUSwapChain>(sch);
    }
    mSwapChain = nullptr;
}

void WebGPUDriver::destroyStream(Handle<HwStream> sh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sh);
#endif
}

void WebGPUDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(tqh);
#endif
}

void WebGPUDriver::destroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> tqh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(tqh);
#endif
}

void WebGPUDriver::destroyDescriptorSet(Handle<HwDescriptorSet> tqh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(tqh);
#endif
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WebGPUSwapChain>();
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainHeadlessS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwSwapChain>((Handle<HwSwapChain>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WGPUTexture>();
}

Handle<HwTexture> WebGPUDriver::importTextureS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwProgram> WebGPUDriver::createProgramS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwProgram>((Handle<HwProgram>::HandleId) mNextFakeHandle++);
}

Handle<HwFence> WebGPUDriver::createFenceS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwFence>((Handle<HwFence>::HandleId) mNextFakeHandle++);
}

Handle<HwTimerQuery> WebGPUDriver::createTimerQueryS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwTimerQuery>((Handle<HwTimerQuery>::HandleId) mNextFakeHandle++);
}

Handle<HwIndexBuffer> WebGPUDriver::createIndexBufferS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<HwIndexBuffer>();
}

Handle<HwTexture> WebGPUDriver::createTextureViewS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwBufferObject> WebGPUDriver::createBufferObjectS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WGPUBufferObject>();
}

Handle<HwRenderTarget> WebGPUDriver::createRenderTargetS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WGPURenderTarget>();
}

Handle<HwVertexBuffer> WebGPUDriver::createVertexBufferS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WGPUVertexBuffer>();
}

Handle<HwDescriptorSet> WebGPUDriver::createDescriptorSetS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwDescriptorSet>((Handle<HwDescriptorSet>::HandleId) mNextFakeHandle++);
}

Handle<HwRenderPrimitive> WebGPUDriver::createRenderPrimitiveS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WGPURenderPrimitive>();
}

Handle<HwVertexBufferInfo> WebGPUDriver::createVertexBufferInfoS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WGPUVertexBufferInfo>();
}

Handle<HwTexture> WebGPUDriver::createTextureViewSwizzleS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwRenderTarget> WebGPUDriver::createDefaultRenderTargetS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return allocHandle<WGPURenderTarget>();
}

Handle<HwDescriptorSetLayout> WebGPUDriver::createDescriptorSetLayoutS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwDescriptorSetLayout>(
            (Handle<HwDescriptorSetLayout>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImageS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImage2S() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImagePlaneS() noexcept {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL();
#endif
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

void WebGPUDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sch, nativeWindow, flags);
#endif
    mNativeWindow = nativeWindow;
    assert_invariant(!mSwapChain);
    wgpu::Surface surface = mPlatform.createSurface(nativeWindow, flags);
    mAdapter = mPlatform.requestAdapter(surface);
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printAdapterDetails(mAdapter);
#endif
    mDevice = mPlatform.requestDevice(mAdapter);
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printDeviceDetails(mDevice);
#endif
    mQueue = mDevice.GetQueue();
    wgpu::Extent2D surfaceSize = mPlatform.getSurfaceExtent(mNativeWindow);
    mSwapChain = constructHandle<WebGPUSwapChain>(sch, std::move(surface), surfaceSize, mAdapter,
            mDevice, flags);
    assert_invariant(mSwapChain);
    FWGPU_LOGW << "WebGPU support is still essentially a no-op at this point in development (only "
                  "background components have been instantiated/selected, such as surface/screen, "
                  "graphics device/GPU, etc.), thus nothing is being drawn to the screen."
               << utils::io::endl;
#if !FWGPU_ENABLED(FWGPU_PRINT_SYSTEM) && !defined(NDEBUG)
    FWGPU_LOGI << "If the FILAMENT_BACKEND_DEBUG_FLAG variable were set with the " << utils::io::hex
               << FWGPU_PRINT_SYSTEM << utils::io::dec
               << " bit flag on during build time the application would print system details "
                  "about the selected graphics device, surface, etc. To see this try "
                  "rebuilding Filament with that flag, e.g. ./build.sh -x "
               << FWGPU_PRINT_SYSTEM << " ..." << utils::io::endl;
#endif
}

void WebGPUDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch, uint32_t width,
        uint32_t height, uint64_t flags) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sch, width, height, flags);
#endif
}

void WebGPUDriver::createVertexBufferInfoR(Handle<HwVertexBufferInfo> vbih, uint8_t bufferCount,
        uint8_t attributeCount, AttributeArray attributes) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(vbih, bufferCount, attributeCount, attributes);
#endif
}

void WebGPUDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint32_t vertexCount,
        Handle<HwVertexBufferInfo> vbih) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(vbh, vertexCount, vbih);
#endif
}

void WebGPUDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(ibh, elementType, indexCount, usage);
#endif
}

void WebGPUDriver::createBufferObjectR(Handle<HwBufferObject> boh, uint32_t byteCount,
        BufferObjectBinding bindingType, BufferUsage usage) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(boh, byteCount, bindingType, usage);
#endif
}

void WebGPUDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, target, levels, format, samples, w, h, depth, usage);
#endif
}

void WebGPUDriver::createTextureViewR(Handle<HwTexture> th, Handle<HwTexture> srch,
        uint8_t baseLevel, uint8_t levelCount) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, srch, baseLevel, levelCount);
#endif
}

void WebGPUDriver::createTextureViewSwizzleR(Handle<HwTexture> th, Handle<HwTexture> srch,
        backend::TextureSwizzle r, backend::TextureSwizzle g, backend::TextureSwizzle b,
        backend::TextureSwizzle a) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, srch, r, g, b, a);
#endif
}

void WebGPUDriver::createTextureExternalImage2R(Handle<HwTexture> th, backend::SamplerType target,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        Platform::ExternalImageHandleRef externalImage) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, target, format, width, height, usage, externalImage);
#endif
}

void WebGPUDriver::createTextureExternalImageR(Handle<HwTexture> th, backend::SamplerType target,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        void* externalImage) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, target, format, width, height, usage, externalImage);
#endif
}

void WebGPUDriver::createTextureExternalImagePlaneR(Handle<HwTexture> th,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        void* image, uint32_t plane) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, format, width, height, usage, image, plane);
#endif
}

void WebGPUDriver::importTextureR(Handle<HwTexture> th, intptr_t id, SamplerType target,
        uint8_t levels, TextureFormat format, uint8_t samples, uint32_t w, uint32_t h,
        uint32_t depth, TextureUsage usage) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, id, target, levels, format, samples, w, h, depth, usage);
#endif
}

void WebGPUDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, Handle<HwVertexBuffer> vbh,
        Handle<HwIndexBuffer> ibh, PrimitiveType pt) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(rph, vbh, ibh, pt);
#endif
}

void WebGPUDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(ph, program);
#endif
}

void WebGPUDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(rth, i);
#endif
    assert_invariant(!mDefaultRenderTarget);
    mDefaultRenderTarget = constructHandle<WGPURenderTarget>(rth);
    assert_invariant(mDefaultRenderTarget);
}

void WebGPUDriver::createRenderTargetR(Handle<HwRenderTarget> rth, TargetBufferFlags targets,
        uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount, MRT color,
        TargetBufferInfo depth, TargetBufferInfo stencil) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(rth, targets, width, height, samples, layerCount, color, depth, stencil);
#endif
}

void WebGPUDriver::createFenceR(Handle<HwFence> fh, int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(fh, i);
#endif
}

void WebGPUDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(tqh, i);
#endif
}

void WebGPUDriver::createDescriptorSetLayoutR(Handle<HwDescriptorSetLayout> dslh,
        backend::DescriptorSetLayout&& info) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(dslh, info);
#endif
}

void WebGPUDriver::createDescriptorSetR(Handle<HwDescriptorSet> dsh,
        Handle<HwDescriptorSetLayout> dslh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(dsh, dslh);
#endif
}

Handle<HwStream> WebGPUDriver::createStreamNative(void* nativeStream) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(nativeStream);
#endif
    return {};
}

Handle<HwStream> WebGPUDriver::createStreamAcquired() {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL();
#endif
    return {};
}

void WebGPUDriver::setAcquiredImage(Handle<HwStream> sh, void* image, const math::mat3f& transform,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sh, image, transform, handler, cb, userData);
#endif
}

void WebGPUDriver::registerBufferObjectStreams(Handle<HwBufferObject> boh,
        BufferObjectStreamDescriptor&& streams) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(boh, "streams");
#endif
}

void WebGPUDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sh, width, height);
#endif
}

int64_t WebGPUDriver::getStreamTimestamp(Handle<HwStream> sh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sh);
#endif
    return 0;
}

void WebGPUDriver::updateStreams(CommandStream* driver) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(driver);
#endif
}

void WebGPUDriver::destroyFence(Handle<HwFence> fh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(fh);
#endif
}

FenceStatus WebGPUDriver::getFenceStatus(Handle<HwFence> fh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(fh);
#endif
    return FenceStatus::CONDITION_SATISFIED;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool WebGPUDriver::isTextureFormatSupported(TextureFormat format) {
    return true;
}

bool WebGPUDriver::isTextureSwizzleSupported() {
    return true;
}

bool WebGPUDriver::isTextureFormatMipmappable(TextureFormat format) {
    return true;
}

bool WebGPUDriver::isRenderTargetFormatSupported(TextureFormat format) {
    return true;
}

bool WebGPUDriver::isFrameBufferFetchSupported() {
    return false;
}

bool WebGPUDriver::isFrameBufferFetchMultiSampleSupported() {
    return false; // TODO: add support for MS framebuffer_fetch
}

bool WebGPUDriver::isFrameTimeSupported() {
    return true;
}

bool WebGPUDriver::isAutoDepthResolveSupported() {
    return true;
}

bool WebGPUDriver::isSRGBSwapChainSupported() {
    return false;
}

bool WebGPUDriver::isProtectedContentSupported() {
    return false;
}

bool WebGPUDriver::isStereoSupported() {
    return false;
}

bool WebGPUDriver::isParallelShaderCompileSupported() {
    return false;
}

bool WebGPUDriver::isDepthStencilResolveSupported() {
    return true;
}

bool WebGPUDriver::isDepthStencilBlitSupported(TextureFormat format) {
    return true;
}

bool WebGPUDriver::isProtectedTexturesSupported() {
    return true;
}

bool WebGPUDriver::isDepthClampSupported() {
    return false;
}

bool WebGPUDriver::isWorkaroundNeeded(Workaround) {
    return false;
}

FeatureLevel WebGPUDriver::getFeatureLevel() {
    return FeatureLevel::FEATURE_LEVEL_1;
}

math::float2 WebGPUDriver::getClipSpaceParams() {
    return math::float2{ 1.0f, 0.0f };
}

uint8_t WebGPUDriver::getMaxDrawBuffers() {
    return MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;
}

size_t WebGPUDriver::getMaxUniformBufferSize() {
    return 16384u;
}

size_t WebGPUDriver::getMaxTextureSize(SamplerType target) {
    return 2048u;
}

size_t WebGPUDriver::getMaxArrayTextureLayers() {
    return 256u;
}

void WebGPUDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(ibh, p, byteOffset);
#endif
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::updateBufferObject(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(ibh, p, byteOffset);
#endif
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(ibh, p, byteOffset);
#endif
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::resetBufferObject(Handle<HwBufferObject> boh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(boh);
#endif
}

void WebGPUDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(vbh, index, boh);
#endif
    auto* vertexBuffer = handleCast<WGPUVertexBuffer>(vbh);
    auto* bufferObject = handleCast<WGPUBufferObject>(boh);
    assert_invariant(index < vertexBuffer->buffers.size());
    vertexBuffer->setBuffer(bufferObject, index);
}

void WebGPUDriver::update3DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, level, xoffset, yoffset, zoffset, width, height, depth, data);
#endif
    scheduleDestroy(std::move(data));
}

void WebGPUDriver::setupExternalImage(void* image) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(image);
#endif
}

TimerQueryResult WebGPUDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(tqh, elapsedTime);
#endif
    return TimerQueryResult::ERROR;
}

void WebGPUDriver::setupExternalImage2(Platform::ExternalImageHandleRef image) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(image);
#endif
}

void WebGPUDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th, sh);
#endif
}

void WebGPUDriver::generateMipmaps(Handle<HwTexture> th) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(th);
#endif
}

void WebGPUDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(priority, handler, callback, user);
#endif
    if (callback) {
        scheduleCallback(handler, user, callback);
    }
}

void WebGPUDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(rth, params);
#endif
    wgpu::CommandEncoderDescriptor commandEncoderDescriptor = {
        .label = "command_encoder"
    };
    mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
    assert_invariant(mCommandEncoder);
    // TODO: Remove this code once WebGPU pipeline is implemented
    static float red = 1.0f;
    if (red - 0.01 > 0) {
        red -= 0.01;
    } else {
        red = 1.0f;
    }
    assert_invariant(mTextureView);
    wgpu::RenderPassColorAttachment renderPassColorAttachment = {
        .view = mTextureView,
        // TODO: remove this code once WebGPU Pipeline is implemented with render targets, pipeline and buffers.
        .depthSlice = wgpu::kDepthSliceUndefined,
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        .clearValue = wgpu::Color{red, 0 , 0 , 1},
    };

    wgpu::RenderPassDescriptor renderPassDescriptor = {
        .colorAttachmentCount = 1,
        .colorAttachments = &renderPassColorAttachment,
        .depthStencilAttachment = nullptr,
        .timestampWrites = nullptr,
    };

    mRenderPassEncoder = mCommandEncoder.BeginRenderPass(&renderPassDescriptor);
    mRenderPassEncoder.SetViewport(params.viewport.left, params.viewport.bottom,
            params.viewport.width, params.viewport.height, params.depthRange.near, params.depthRange.far);
}

void WebGPUDriver::endRenderPass(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
    mRenderPassEncoder.End();
    mRenderPassEncoder = nullptr;
    wgpu::CommandBufferDescriptor commandBufferDescriptor {
        .label = "command_buffer",
    };
    mCommandBuffer = mCommandEncoder.Finish(&commandBufferDescriptor);
    assert_invariant(mCommandBuffer);
}

void WebGPUDriver::nextSubpass(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
}

void WebGPUDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(drawSch, readSch);
#endif
    ASSERT_PRECONDITION_NON_FATAL(drawSch == readSch,
            "WebGPU driver does not support distinct draw/read swap chains.");
    auto* swapChain = handleCast<WebGPUSwapChain>(drawSch);
    mSwapChain = swapChain;
    assert_invariant(mSwapChain);
    wgpu::Extent2D surfaceSize = mPlatform.getSurfaceExtent(mNativeWindow);
    mTextureView = mSwapChain->getCurrentSurfaceTextureView(surfaceSize);
    assert_invariant(mTextureView);
}

void WebGPUDriver::commit(Handle<HwSwapChain> sch) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(sch);
#endif
    mCommandEncoder = nullptr;
    mQueue.Submit(1, &mCommandBuffer);
    mCommandBuffer = nullptr;
    mTextureView = nullptr;
    assert_invariant(mSwapChain);
    mSwapChain->present();
}

void WebGPUDriver::setPushConstant(backend::ShaderStage stage, uint8_t index,
        backend::PushConstantVariant value) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(stage, index, value);
#endif
}

void WebGPUDriver::insertEventMarker(char const* string) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(string);
#endif
}

void WebGPUDriver::pushGroupMarker(char const* string) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(string);
#endif
}

void WebGPUDriver::popGroupMarker(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(i);
#endif
}

void WebGPUDriver::startCapture(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
}

void WebGPUDriver::stopCapture(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
}

void WebGPUDriver::readPixels(Handle<HwRenderTarget> src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(src, x, y, width, height, p);
#endif
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::readBufferSubData(backend::BufferObjectHandle boh,
        uint32_t offset, uint32_t size, backend::BufferDescriptor&& p) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(boh, offset, size, p);
#endif
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(buffers, dst, dstRect, src, srcRect, filter);
#endif
}

void WebGPUDriver::resolve(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(dst, srcLevel, srcLayer, src, dstLevel, dstLayer);
#endif
}

void WebGPUDriver::blit(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer, math::uint2 dstOrigin,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer, math::uint2 srcOrigin,
        math::uint2 size) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(dst, srcLevel, srcLayer, dstOrigin, src, dstLevel, dstLayer, srcOrigin,
            size);
#endif
}

void WebGPUDriver::bindPipeline(PipelineState const& pipelineState) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(pipelineState);
#endif
}

void WebGPUDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(rph);
#endif
}

void WebGPUDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(indexOffset, indexCount, instanceCount);
#endif
}

void WebGPUDriver::draw(PipelineState pipelineState, Handle<HwRenderPrimitive> rph,
        uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(pipelineState, rph, indexOffset, indexCount, instanceCount);
#endif
}

void WebGPUDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(program, workGroupCount);
#endif
}

void WebGPUDriver::scissor(
        Viewport scissor) {
}

void WebGPUDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(tqh);
#endif
}

void WebGPUDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(tqh);
#endif
}

void WebGPUDriver::resetState(int i) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(i);
#endif
}

void WebGPUDriver::updateDescriptorSetBuffer(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::BufferObjectHandle boh,
        uint32_t offset,
        uint32_t size) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(dsh, binding, boh, offset, size);
#endif
}

void WebGPUDriver::updateDescriptorSetTexture(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::TextureHandle th,
        SamplerParams params) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(dsh, binding, th, params);
#endif
}

void WebGPUDriver::bindDescriptorSet(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_set_t set,
        backend::DescriptorSetOffsetArray&& offsets) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
    FWGPU_PRINT_DRIVER_CALL(dsh, set, offsets);
#endif
}

void WebGPUDriver::setDebugTag(HandleBase::HandleId handleId, utils::CString tag) {
#if FWGPU_ENABLED(FWGPU_PRINT_DRIVER_CALLS)
//    FWGPU_PRINT_DRIVER_CALL(handleId, tag);
#endif
}

} // namespace filament
