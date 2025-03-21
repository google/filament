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
#include <sstream>
#include <string_view>
#include <utility>
#include <variant>

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
    printLimit("maxInterStageShaderComponents", limits.maxInterStageShaderComponents);
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
        FWGPU_LOGI << "  compatibility mode: " << bool(adapterInfo.compatibilityMode)
                   << utils::io::endl;
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
    wgpu::SupportedLimits supportedLimits{};
    if (!adapter.GetLimits(&supportedLimits)) {
        FWGPU_LOGW << "Failed to get WebGPU adapter supported limits" << utils::io::endl;
    } else {
        FWGPU_LOGI << "WebGPU adapter supported limits:" << utils::io::endl;
        printLimits(supportedLimits.limits);
    }
}
#endif

#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
void printSurfaceCapabilitiesDetails(wgpu::SurfaceCapabilities const& capabilities) {
    std::stringstream usages_stream{};
    usages_stream << capabilities.usages;
    FWGPU_LOGI << "WebGPU surface capabilities:" << utils::io::endl;
    FWGPU_LOGI << "  surface usages: " << usages_stream.str().data() << utils::io::endl;
    FWGPU_LOGI << "  surface formats (" << capabilities.formatCount << "):" << utils::io::endl;
    if (capabilities.formatCount > 0 && capabilities.formats != nullptr) {
        std::for_each(capabilities.formats, capabilities.formats + capabilities.formatCount,
                [](wgpu::TextureFormat const format) {
                    std::stringstream format_stream{};
                    format_stream << format;
                    FWGPU_LOGI << "    " << format_stream.str().data() << utils::io::endl;
                });
    }
    FWGPU_LOGI << "  surface present modes (" << capabilities.presentModeCount
               << "):" << utils::io::endl;
    if (capabilities.presentModeCount > 0 && capabilities.presentModes != nullptr) {
        std::for_each(capabilities.presentModes,
                capabilities.presentModes + capabilities.presentModeCount,
                [](wgpu::PresentMode const presentMode) {
                    std::stringstream present_mode_stream{};
                    present_mode_stream << presentMode;
                    FWGPU_LOGI << "    " << present_mode_stream.str().data() << utils::io::endl;
                });
    }
    FWGPU_LOGI << "  surface alpha modes (" << capabilities.alphaModeCount
               << "):" << utils::io::endl;
    if (capabilities.alphaModeCount > 0 && capabilities.alphaModes != nullptr) {
        std::for_each(capabilities.alphaModes,
                capabilities.alphaModes + capabilities.alphaModeCount,
                [](wgpu::CompositeAlphaMode const alphaMode) {
                    std::stringstream alpha_mode_stream{};
                    alpha_mode_stream << alphaMode;
                    FWGPU_LOGI << "    " << alpha_mode_stream.str().data() << utils::io::endl;
                });
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
    wgpu::SupportedLimits supportedLimits{};
    if (!device.GetLimits(&supportedLimits)) {
        FWGPU_LOGW << "Failed to get WebGPU supported device limits" << utils::io::endl;
    } else {
        FWGPU_LOGI << "WebGPU device supported limits:" << utils::io::endl;
        printLimits(supportedLimits.limits);
    }
}
#endif

}// namespace

Driver* WebGPUDriver::create(WebGPUPlatform& platform) noexcept {
    return new WebGPUDriver(platform);
}

WebGPUDriver::WebGPUDriver(WebGPUPlatform& platform) noexcept
    : mPlatform(platform) {
  #if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printInstanceDetails(mPlatform.getInstance());

#endif
}

WebGPUDriver::~WebGPUDriver() noexcept = default;

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
}

void WebGPUDriver::tick(int) {
}

void WebGPUDriver::beginFrame(int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs, uint32_t frameId) {
}

void WebGPUDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, FrameScheduledCallback&& callback, uint64_t flags) {

}

void WebGPUDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, utils::Invocable<void(void)>&& callback) {

}

void WebGPUDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void WebGPUDriver::endFrame(uint32_t frameId) {
}

void WebGPUDriver::flush(int) {
}

void WebGPUDriver::finish(int) {
}

void WebGPUDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
}

void WebGPUDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
}

void WebGPUDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
}

void WebGPUDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
}

void WebGPUDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
}

void WebGPUDriver::destroyTexture(Handle<HwTexture> th) {
}

void WebGPUDriver::destroyProgram(Handle<HwProgram> ph) {
}

void WebGPUDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
}

void WebGPUDriver::destroySwapChain(Handle<HwSwapChain> sch) {
}

void WebGPUDriver::destroyStream(Handle<HwStream> sh) {
}

void WebGPUDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
}

void WebGPUDriver::destroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> tqh) {
}

void WebGPUDriver::destroyDescriptorSet(Handle<HwDescriptorSet> tqh) {
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainS() noexcept {
    return Handle<HwSwapChain>((Handle<HwSwapChain>::HandleId) mNextFakeHandle++);
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainHeadlessS() noexcept {
    return Handle<HwSwapChain>((Handle<HwSwapChain>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureS() noexcept {
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::importTextureS() noexcept {
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwProgram> WebGPUDriver::createProgramS() noexcept {
    return Handle<HwProgram>((Handle<HwProgram>::HandleId) mNextFakeHandle++);
}

Handle<HwFence> WebGPUDriver::createFenceS() noexcept {
    return Handle<HwFence>((Handle<HwFence>::HandleId) mNextFakeHandle++);
}

Handle<HwTimerQuery> WebGPUDriver::createTimerQueryS() noexcept {
    return Handle<HwTimerQuery>((Handle<HwTimerQuery>::HandleId) mNextFakeHandle++);
}

Handle<HwIndexBuffer> WebGPUDriver::createIndexBufferS() noexcept {
    return Handle<HwIndexBuffer>((Handle<HwIndexBuffer>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureViewS() noexcept {
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwBufferObject> WebGPUDriver::createBufferObjectS() noexcept {
    return Handle<HwBufferObject>((Handle<HwBufferObject>::HandleId) mNextFakeHandle++);
}

Handle<HwRenderTarget> WebGPUDriver::createRenderTargetS() noexcept {
    return Handle<HwRenderTarget>((Handle<HwRenderTarget>::HandleId) mNextFakeHandle++);
}

Handle<HwVertexBuffer> WebGPUDriver::createVertexBufferS() noexcept {
    return Handle<HwVertexBuffer>((Handle<HwVertexBuffer>::HandleId) mNextFakeHandle++);
}

Handle<HwDescriptorSet> WebGPUDriver::createDescriptorSetS() noexcept {
    return Handle<HwDescriptorSet>((Handle<HwDescriptorSet>::HandleId) mNextFakeHandle++);
}

Handle<HwRenderPrimitive> WebGPUDriver::createRenderPrimitiveS() noexcept {
    return Handle<HwRenderPrimitive>((Handle<HwRenderPrimitive>::HandleId) mNextFakeHandle++);
}

Handle<HwVertexBufferInfo> WebGPUDriver::createVertexBufferInfoS() noexcept {
    return Handle<HwVertexBufferInfo>((Handle<HwVertexBufferInfo>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureViewSwizzleS() noexcept {
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwRenderTarget> WebGPUDriver::createDefaultRenderTargetS() noexcept {
    return Handle<HwRenderTarget>((Handle<HwRenderTarget>::HandleId) mNextFakeHandle++);
}

Handle<HwDescriptorSetLayout> WebGPUDriver::createDescriptorSetLayoutS() noexcept {
    return Handle<HwDescriptorSetLayout>(
            (Handle<HwDescriptorSetLayout>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImageS() noexcept {
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImage2S() noexcept {
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImagePlaneS() noexcept {
    return Handle<HwTexture>((Handle<HwTexture>::HandleId) mNextFakeHandle++);
}

void WebGPUDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    mSurface= mPlatform.createSurface(nativeWindow, flags);
    mAdapter = mPlatform.requestAdapter(mSurface);
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printAdapterDetails(mAdapter);
    wgpu::SurfaceCapabilities surfaceCapabilities{};
    if (!mSurface.GetCapabilities(mAdapter, &surfaceCapabilities)) {
        FWGPU_LOGW << "Failed to get WebGPU surface capabilities" << utils::io::endl;
    } else {
        printSurfaceCapabilitiesDetails(surfaceCapabilities);
    }
#endif
    mDevice = mPlatform.requestDevice(mAdapter);
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printDeviceDetails(mDevice);
#endif
    mQueue = mDevice.GetQueue();

    // Configure the surface ... Should probably move creation to the Surface call and configuration, resize, headless can live there too, similar to driver?
    wgpu::SurfaceCapabilities surfaceCapabilities{};
    if (!mSurface.GetCapabilities(mAdapter, &surfaceCapabilities)) {
        FWGPU_LOGW << "Failed to get WebGPU surface capabilities" << utils::io::endl;
    } else {
        //printSurfaceCapabilitiesDetails(surfaceCapabilities);
    }
    wgpu::SurfaceConfiguration surfaceConfig = {};
    surfaceConfig.device = mDevice;
    surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
    //TODO: get size from nativeWindow ,or hook into queue to get it from a texture
    surfaceConfig.width = 2048;
    surfaceConfig.height = 1280;
    //Should Probably make sure these formats and modes are ideal?
    surfaceConfig.format = surfaceCapabilities.formats[0];
    surfaceConfig.presentMode = surfaceCapabilities.presentModes[0];
    surfaceConfig.alphaMode = surfaceCapabilities.alphaModes[0];
    mSurface.Configure(&surfaceConfig);
    FWGPU_LOGW << "Successfully configured surface" << utils::io::endl;

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
        uint32_t height, uint64_t flags) {}

void WebGPUDriver::createVertexBufferInfoR(Handle<HwVertexBufferInfo> vbih, uint8_t bufferCount,
        uint8_t attributeCount, AttributeArray attributes) {}

void WebGPUDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint32_t vertexCount,
        Handle<HwVertexBufferInfo> vbih) {}

void WebGPUDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage) {}

void WebGPUDriver::createBufferObjectR(Handle<HwBufferObject> boh, uint32_t byteCount,
        BufferObjectBinding bindingType, BufferUsage usage) {}

void WebGPUDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {}

void WebGPUDriver::createTextureViewR(Handle<HwTexture> th, Handle<HwTexture> srch,
        uint8_t baseLevel, uint8_t levelCount) {}

void WebGPUDriver::createTextureViewSwizzleR(Handle<HwTexture> th, Handle<HwTexture> srch,
        backend::TextureSwizzle r, backend::TextureSwizzle g, backend::TextureSwizzle b,
        backend::TextureSwizzle a) {}

void WebGPUDriver::createTextureExternalImage2R(Handle<HwTexture> th, backend::SamplerType target,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        Platform::ExternalImageHandleRef externalImage) {}

void WebGPUDriver::createTextureExternalImageR(Handle<HwTexture> th, backend::SamplerType target,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        void* externalImage) {}

void WebGPUDriver::createTextureExternalImagePlaneR(Handle<HwTexture> th,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        void* image, uint32_t plane) {}

void WebGPUDriver::importTextureR(Handle<HwTexture> th, intptr_t id, SamplerType target,
        uint8_t levels, TextureFormat format, uint8_t samples, uint32_t w, uint32_t h,
        uint32_t depth, TextureUsage usage) {}

void WebGPUDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, Handle<HwVertexBuffer> vbh,
        Handle<HwIndexBuffer> ibh, PrimitiveType pt) {}

void WebGPUDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {}

void WebGPUDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int) {}

void WebGPUDriver::createRenderTargetR(Handle<HwRenderTarget> rth, TargetBufferFlags targets,
        uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount, MRT color,
        TargetBufferInfo depth, TargetBufferInfo stencil) {}

void WebGPUDriver::createFenceR(Handle<HwFence> fh, int) {}

void WebGPUDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {}

void WebGPUDriver::createDescriptorSetLayoutR(Handle<HwDescriptorSetLayout> dslh,
        backend::DescriptorSetLayout&& info) {}

void WebGPUDriver::createDescriptorSetR(Handle<HwDescriptorSet> dsh,
        Handle<HwDescriptorSetLayout> dslh) {}

Handle<HwStream> WebGPUDriver::createStreamNative(void* nativeStream) {
    return {};
}

Handle<HwStream> WebGPUDriver::createStreamAcquired() {
    return {};
}

void WebGPUDriver::setAcquiredImage(Handle<HwStream> sh, void* image, const math::mat3f& transform,
        CallbackHandler* handler, StreamCallback cb, void* userData) {}

void WebGPUDriver::registerBufferObjectStreams(Handle<HwBufferObject> boh,
        BufferObjectStreamDescriptor&& streams) {}

void WebGPUDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
}

int64_t WebGPUDriver::getStreamTimestamp(Handle<HwStream> sh) {
    return 0;
}

void WebGPUDriver::updateStreams(CommandStream* driver) {
}

void WebGPUDriver::destroyFence(Handle<HwFence> fh) {
}

FenceStatus WebGPUDriver::getFenceStatus(Handle<HwFence> fh) {
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
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::updateBufferObject(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::resetBufferObject(Handle<HwBufferObject> boh) {
}

void WebGPUDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
}

void WebGPUDriver::update3DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    scheduleDestroy(std::move(data));
}

void WebGPUDriver::setupExternalImage(void* image) {
}

TimerQueryResult WebGPUDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    return TimerQueryResult::ERROR;
}

void WebGPUDriver::setupExternalImage2(Platform::ExternalImageHandleRef image) {
}

void WebGPUDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

void WebGPUDriver::generateMipmaps(Handle<HwTexture> th) { }

void WebGPUDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        scheduleCallback(handler, user, callback);
    }
}

void WebGPUDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
}

void WebGPUDriver::endRenderPass(int) {
}

void WebGPUDriver::nextSubpass(int) {
}

void WebGPUDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
}

void WebGPUDriver::commit(Handle<HwSwapChain> sch) {
}

void WebGPUDriver::setPushConstant(backend::ShaderStage stage, uint8_t index,
        backend::PushConstantVariant value) {
}

void WebGPUDriver::insertEventMarker(char const* string) {
}

void WebGPUDriver::pushGroupMarker(char const* string) {
}

void WebGPUDriver::popGroupMarker(int) {
}

void WebGPUDriver::startCapture(int) {
}

void WebGPUDriver::stopCapture(int) {
}

void WebGPUDriver::readPixels(Handle<HwRenderTarget> src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::readBufferSubData(backend::BufferObjectHandle boh,
        uint32_t offset, uint32_t size, backend::BufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
}

void WebGPUDriver::resolve(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer) {
}

void WebGPUDriver::blit(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer, math::uint2 dstOrigin,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer, math::uint2 srcOrigin,
        math::uint2 size) {
}

void WebGPUDriver::bindPipeline(PipelineState const& pipelineState) {
}

void WebGPUDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
}

void WebGPUDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
}

void WebGPUDriver::draw(PipelineState pipelineState, Handle<HwRenderPrimitive> rph,
        uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
}

void WebGPUDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
}

void WebGPUDriver::scissor(
        Viewport scissor) {
}

void WebGPUDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
}

void WebGPUDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
}

void WebGPUDriver::resetState(int) {
}

void WebGPUDriver::updateDescriptorSetBuffer(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::BufferObjectHandle boh,
        uint32_t offset,
        uint32_t size) {
}

void WebGPUDriver::updateDescriptorSetTexture(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::TextureHandle th,
        SamplerParams params) {
}

void WebGPUDriver::bindDescriptorSet(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_set_t set,
        backend::DescriptorSetOffsetArray&& offsets) {
}

void WebGPUDriver::setDebugTag(HandleBase::HandleId handleId, utils::CString tag) {
}

} // namespace filament
