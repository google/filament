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
#include <utils/Hash.h>
#include "webgpu/WebGPUDriver.h"

#include "WebGPUPipelineCreation.h"
#include "WebGPUSwapChain.h"
#include <backend/platforms/WebGPUPlatform.h>

#include "CommandStreamDispatcher.h"
#include "DriverBase.h"
#include "private/backend/Dispatcher.h"
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>

#include <math/mat3.h>
#include <utils/CString.h>
#include <utils/Panic.h>
#include <utils/ostream.h>

#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
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
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printInstanceDetails(mPlatform.getInstance());
#endif
    mAdapter = mPlatform.requestAdapter(nullptr);
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printAdapterDetails(mAdapter);
#endif
    mDevice = mPlatform.requestDevice(mAdapter);
#if FWGPU_ENABLED(FWGPU_PRINT_SYSTEM)
    printDeviceDetails(mDevice);
#endif
    mQueue = mDevice.GetQueue();
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
    mDevice.Tick();
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

void WebGPUDriver::finish(int /* dummy */) {
    if (mCommandEncoder != nullptr) {
        // submit the command buffer thus far...
        assert_invariant(mRenderPassEncoder == nullptr);
        wgpu::CommandBufferDescriptor commandBufferDescriptor{
            .label = "command_buffer",
        };
        mCommandBuffer = mCommandEncoder.Finish(&commandBufferDescriptor);
        assert_invariant(mCommandBuffer);
        mQueue.Submit(1, &mCommandBuffer);
        mCommandBuffer = nullptr;
        // create a new command buffer encoder to continue recording the next command for frame...
        wgpu::CommandEncoderDescriptor commandEncoderDescriptor = { .label = "command_encoder" };
        mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
        assert_invariant(mCommandEncoder);
    }
}

void WebGPUDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
   if (rph) {
        destructHandle<WGPURenderPrimitive>(rph);
    }
}

void WebGPUDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
   if (vbih) {
        destructHandle<WGPUVertexBufferInfo>(vbih);
    }
}

void WebGPUDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
    if (vbh) {
        destructHandle<WGPUVertexBuffer>(vbh);
    }
}

void WebGPUDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
    if (ibh) {
        destructHandle<WGPUIndexBuffer>(ibh);
    }
}

void WebGPUDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
    if (boh) {
        destructHandle<WGPUBufferObject>(boh);
    }
}

void WebGPUDriver::destroyTexture(Handle<HwTexture> th) {
}

void WebGPUDriver::destroyProgram(Handle<HwProgram> ph) {
    if (ph) {
        destructHandle<WGPUProgram>(ph);
    }
}

void WebGPUDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
}

void WebGPUDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    if (sch) {
        destructHandle<WebGPUSwapChain>(sch);
    }
    mSwapChain = nullptr;
}

void WebGPUDriver::destroyStream(Handle<HwStream> sh) {
}

void WebGPUDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
}

void WebGPUDriver::destroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> tqh) {
    if (tqh) {
        destructHandle<WebGPUDescriptorSetLayout>(tqh);
    }
}

void WebGPUDriver::destroyDescriptorSet(Handle<HwDescriptorSet> tqh) {
    if (tqh) {
        destructHandle<WebGPUDescriptorSet>(tqh);
    }
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainS() noexcept {
    return allocHandle<WebGPUSwapChain>();
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainHeadlessS() noexcept {
    return Handle<HwSwapChain>((Handle<HwSwapChain>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureS() noexcept {
    return allocHandle<WGPUTexture>();
}

Handle<HwTexture> WebGPUDriver::importTextureS() noexcept { return allocHandle<WGPUTexture>(); }

Handle<HwProgram> WebGPUDriver::createProgramS() noexcept {
    return allocHandle<WGPUProgram>();
}

Handle<HwFence> WebGPUDriver::createFenceS() noexcept {
    return Handle<HwFence>((Handle<HwFence>::HandleId) mNextFakeHandle++);
}

Handle<HwTimerQuery> WebGPUDriver::createTimerQueryS() noexcept {
    return Handle<HwTimerQuery>((Handle<HwTimerQuery>::HandleId) mNextFakeHandle++);
}

Handle<HwIndexBuffer> WebGPUDriver::createIndexBufferS() noexcept {
    return allocHandle<HwIndexBuffer>();
}

Handle<HwTexture> WebGPUDriver::createTextureViewS() noexcept {
    return allocHandle<WGPUTexture>();
}

Handle<HwBufferObject> WebGPUDriver::createBufferObjectS() noexcept {
    return allocHandle<WGPUBufferObject>();
}

Handle<HwRenderTarget> WebGPUDriver::createRenderTargetS() noexcept {
    return allocHandle<WGPURenderTarget>();
}

Handle<HwVertexBuffer> WebGPUDriver::createVertexBufferS() noexcept {
    return allocHandle<WGPUVertexBuffer>();
}

Handle<HwDescriptorSet> WebGPUDriver::createDescriptorSetS() noexcept {
    return allocHandle<WebGPUDescriptorSet>();
}

Handle<HwRenderPrimitive> WebGPUDriver::createRenderPrimitiveS() noexcept {
    return allocHandle<WGPURenderPrimitive>();
}

Handle<HwVertexBufferInfo> WebGPUDriver::createVertexBufferInfoS() noexcept {
    return allocHandle<WGPUVertexBufferInfo>();
}

Handle<HwTexture> WebGPUDriver::createTextureViewSwizzleS() noexcept {
    return allocHandle<WGPUTexture>();
}

Handle<HwRenderTarget> WebGPUDriver::createDefaultRenderTargetS() noexcept {
    return allocHandle<WGPURenderTarget>();
}

Handle<HwDescriptorSetLayout> WebGPUDriver::createDescriptorSetLayoutS() noexcept {
    return allocHandle<WebGPUDescriptorSetLayout>();
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImageS() noexcept {
    return allocHandle<WGPUTexture>();
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImage2S() noexcept {
    return allocHandle<WGPUTexture>();
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImagePlaneS() noexcept {
    return allocHandle<WGPUTexture>();
}

void WebGPUDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    mNativeWindow = nativeWindow;
    assert_invariant(!mSwapChain);
    wgpu::Surface surface = mPlatform.createSurface(nativeWindow, flags);

    wgpu::Extent2D surfaceSize = mPlatform.getSurfaceExtent(mNativeWindow);
    mSwapChain = constructHandle<WebGPUSwapChain>(sch, std::move(surface), surfaceSize, mAdapter,
            mDevice, flags);
    assert_invariant(mSwapChain);
    WebGPUDescriptorSet::initializeDummyResourcesIfNotAlready(mDevice,
            mSwapChain->getColorFormat());
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
        uint8_t attributeCount, AttributeArray attributes) {
    constructHandle<WGPUVertexBufferInfo>(vbih, bufferCount, attributeCount, attributes);
}

void WebGPUDriver::createVertexBufferR(Handle<HwVertexBuffer> vbh, uint32_t vertexCount,
        Handle<HwVertexBufferInfo> vbih) {
    auto* vertexBufferInfo = handleCast<WGPUVertexBufferInfo>(vbih);
    constructHandle<WGPUVertexBuffer>(vbh, mDevice, vertexCount, vertexBufferInfo->bufferCount,
            vbih);
}

void WebGPUDriver::createIndexBufferR(Handle<HwIndexBuffer> ibh, ElementType elementType,
        uint32_t indexCount, BufferUsage usage) {
    auto elementSize = static_cast<uint8_t>(getElementTypeSize(elementType));
    constructHandle<WGPUIndexBuffer>(ibh, mDevice, elementSize, indexCount);
}

void WebGPUDriver::createBufferObjectR(Handle<HwBufferObject> boh, uint32_t byteCount,
        BufferObjectBinding bindingType, BufferUsage usage) {
    constructHandle<WGPUBufferObject>(boh, mDevice, bindingType, byteCount);
}

void WebGPUDriver::createTextureR(Handle<HwTexture> th, SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t w, uint32_t h, uint32_t depth,
        TextureUsage usage) {
    constructHandle<WGPUTexture>(th, target, levels, format, samples, w, h, depth, usage, mDevice);
}

void WebGPUDriver::createTextureViewR(Handle<HwTexture> th, Handle<HwTexture> srch,
        uint8_t baseLevel, uint8_t levelCount) {
    auto source = handleCast<WGPUTexture>(srch);

    constructHandle<WGPUTexture>(th, source, baseLevel, levelCount);
}

void WebGPUDriver::createTextureViewSwizzleR(Handle<HwTexture> th, Handle<HwTexture> srch,
        backend::TextureSwizzle r, backend::TextureSwizzle g, backend::TextureSwizzle b,
        backend::TextureSwizzle a) {
    PANIC_POSTCONDITION("Swizzle WebGPU Texture is not supported");
}

void WebGPUDriver::createTextureExternalImage2R(Handle<HwTexture> th, backend::SamplerType target,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        Platform::ExternalImageHandleRef externalImage) {
    PANIC_POSTCONDITION("External WebGPU Texture is not supported");
}

void WebGPUDriver::createTextureExternalImageR(Handle<HwTexture> th, backend::SamplerType target,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        void* externalImage) {
    PANIC_POSTCONDITION("External WebGPU Texture is not supported");
}

void WebGPUDriver::createTextureExternalImagePlaneR(Handle<HwTexture> th,
        backend::TextureFormat format, uint32_t width, uint32_t height, backend::TextureUsage usage,
        void* image, uint32_t plane) {
    PANIC_POSTCONDITION("External WebGPU Texture is not supported");
}

void WebGPUDriver::importTextureR(Handle<HwTexture> th, intptr_t id, SamplerType target,
        uint8_t levels, TextureFormat format, uint8_t samples, uint32_t w, uint32_t h,
        uint32_t depth, TextureUsage usage) {
    PANIC_POSTCONDITION("Import WebGPU Texture is not supported");
}

void WebGPUDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> rph, Handle<HwVertexBuffer> vbh,
        Handle<HwIndexBuffer> ibh, PrimitiveType pt) {
    assert_invariant(mDevice);

    auto* renderPrimitive = constructHandle<WGPURenderPrimitive>(rph);
    auto* vertexBuffer = handleCast<WGPUVertexBuffer>(vbh);
    auto* indexBuffer = handleCast<WGPUIndexBuffer>(ibh);
    renderPrimitive->vertexBuffer = vertexBuffer;
    renderPrimitive->indexBuffer = indexBuffer;
    renderPrimitive->type = pt;
}

void WebGPUDriver::createProgramR(Handle<HwProgram> ph, Program&& program) {
    constructHandle<WGPUProgram>(ph, mDevice, program);
}

void WebGPUDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> rth, int) {
    assert_invariant(!mDefaultRenderTarget);
    mDefaultRenderTarget = constructHandle<WGPURenderTarget>(rth);
    assert_invariant(mDefaultRenderTarget);
}

void WebGPUDriver::createRenderTargetR(Handle<HwRenderTarget> rth, TargetBufferFlags targets,
        uint32_t width, uint32_t height, uint8_t samples, uint8_t layerCount, MRT color,
        TargetBufferInfo depth, TargetBufferInfo stencil) {}

void WebGPUDriver::createFenceR(Handle<HwFence> fh, int) {}

void WebGPUDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {}

void WebGPUDriver::createDescriptorSetLayoutR(Handle<HwDescriptorSetLayout> dslh,
        backend::DescriptorSetLayout&& info) {
    constructHandle<WebGPUDescriptorSetLayout>(dslh, std::move(info), mDevice);
}

void WebGPUDriver::createDescriptorSetR(Handle<HwDescriptorSet> dsh,
        Handle<HwDescriptorSetLayout> dslh) {
    auto layout = handleCast<WebGPUDescriptorSetLayout>(dslh);
    constructHandle<WebGPUDescriptorSet>(dsh, layout->getLayout(), layout->getBindGroupEntries());
}

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
    return WGPUTexture::fToWGPUTextureFormat(format) != wgpu::TextureFormat::Undefined;
}

bool WebGPUDriver::isTextureSwizzleSupported() {
    return false;
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
    handleCast<WGPUIndexBuffer>(ibh)->updateGPUBuffer(p, byteOffset, mQueue);
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::updateBufferObject(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    handleCast<WGPUBufferObject>(ibh)->updateGPUBuffer(p, byteOffset, mQueue);
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> ibh,
        BufferDescriptor&& p, uint32_t byteOffset) {
    handleCast<WGPUBufferObject>(ibh)->updateGPUBuffer(p, byteOffset, mQueue);
    scheduleDestroy(std::move(p));
}

void WebGPUDriver::resetBufferObject(Handle<HwBufferObject> boh) {
    // Is there something that needs to be done here? Vulkan has left it unimplemented.
}

void WebGPUDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
    auto* vertexBuffer = handleCast<WGPUVertexBuffer>(vbh);
    auto* bufferObject = handleCast<WGPUBufferObject>(boh);
    assert_invariant(index < vertexBuffer->buffers.size());
    assert_invariant(bufferObject->getBuffer().GetUsage() & wgpu::BufferUsage::Vertex);
    vertexBuffer->buffers[index] = bufferObject->getBuffer();
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

void WebGPUDriver::beginRenderPass(Handle<HwRenderTarget> rth, RenderPassParams const& params) {
    assert_invariant(mCommandEncoder);

    auto* renderTarget = handleCast<WGPURenderTarget>(rth);
    // if (renderTarget == mDefaultRenderTarget) {
    //     FWGPU_LOGW << "Default render target"
    //                << utils::io::endl;
    // } else {
    //     FWGPU_LOGW << "Non Default render target"
    //                << utils::io::endl;
    // }
    wgpu::RenderPassDescriptor renderPassDescriptor2;
    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment{
        .view = mSwapChain->getDepthTextureView(),
        .depthLoadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::DEPTH),
        .depthStoreOp = WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::DEPTH),
        .depthClearValue = static_cast<float>(params.clearDepth),
        .depthReadOnly = (params.readOnlyDepthStencil & RenderPassParams::READONLY_DEPTH) > 0,
        .stencilLoadOp = WGPURenderTarget::getLoadOperation(params, TargetBufferFlags::STENCIL),
        .stencilStoreOp = WGPURenderTarget::getStoreOperation(params, TargetBufferFlags::STENCIL),
        .stencilClearValue = params.clearStencil,
        .stencilReadOnly = (params.readOnlyDepthStencil & RenderPassParams::READONLY_STENCIL) > 0
    };
    renderTarget->setUpRenderPassAttachments(renderPassDescriptor2, mTextureView, params);
    renderPassDescriptor2.depthStencilAttachment = &depthStencilAttachment;
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

    mRenderPassEncoder = mCommandEncoder.BeginRenderPass(&renderPassDescriptor2);
    mRenderPassEncoder.SetViewport(params.viewport.left, params.viewport.bottom,
            params.viewport.width, params.viewport.height, params.depthRange.near, params.depthRange.far);
}

void WebGPUDriver::endRenderPass(int /* dummy */) {
    mRenderPassEncoder.End();
    mRenderPassEncoder = nullptr;
}

void WebGPUDriver::nextSubpass(int) {
}

void WebGPUDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
    ASSERT_PRECONDITION_NON_FATAL(drawSch == readSch,
            "WebGPU driver does not support distinct draw/read swap chains.");
    auto* swapChain = handleCast<WebGPUSwapChain>(drawSch);
    mSwapChain = swapChain;
    assert_invariant(mSwapChain);
    wgpu::Extent2D surfaceSize = mPlatform.getSurfaceExtent(mNativeWindow);
    mTextureView = mSwapChain->getCurrentSurfaceTextureView(surfaceSize);
    assert_invariant(mTextureView);
    wgpu::CommandEncoderDescriptor commandEncoderDescriptor = {
        .label = "command_encoder"
    };
    mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
    assert_invariant(mCommandEncoder);
}

void WebGPUDriver::commit(Handle<HwSwapChain> sch) {
    wgpu::CommandBufferDescriptor commandBufferDescriptor{
        .label = "command_buffer",
    };
    mCommandBuffer = mCommandEncoder.Finish(&commandBufferDescriptor);
    assert_invariant(mCommandBuffer);
    mCommandEncoder = nullptr;
    mQueue.Submit(1, &mCommandBuffer);
    mCommandBuffer = nullptr;
    mTextureView = nullptr;
    assert_invariant(mSwapChain);
    mSwapChain->present();
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

void WebGPUDriver::readBufferSubData(Handle<HwBufferObject> boh,
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
    // TODO Investigate implications of this hash more closely. Vulkan has a whole class
    // VulkanPipelineCache to handle this, may be missing nuance
    static auto pipleineStateHasher = utils::hash::MurmurHashFn<filament::backend::PipelineState>();
    auto hash = pipleineStateHasher(pipelineState);
    if(mPipelineMap.find(hash) != mPipelineMap.end()){
        mRenderPassEncoder.SetPipeline(mPipelineMap[hash]);
        return;
    }
    const auto* program = handleCast<WGPUProgram>(pipelineState.program);
    assert_invariant(program);
    assert_invariant(program->computeShaderModule == nullptr &&
                     "WebGPU backend does not (yet) support compute pipelines.");
    FILAMENT_CHECK_POSTCONDITION(program->vertexShaderModule)
            << "WebGPU backend requires a vertex shader module for a render pipeline";
    std::array<wgpu::BindGroupLayout, MAX_DESCRIPTOR_SET_COUNT> bindGroupLayouts{};
    assert_invariant(bindGroupLayouts.size() >= pipelineState.pipelineLayout.setLayout.size());
    size_t bindGroupLayoutCount = 0;
    for (size_t i = 0; i < bindGroupLayouts.size(); i++) {
        const auto handle = pipelineState.pipelineLayout.setLayout[bindGroupLayoutCount];
        if (handle.getId() == HandleBase::nullid) {
            continue;
        }
        bindGroupLayouts[bindGroupLayoutCount++] =
                handleCast<WebGPUDescriptorSetLayout>(handle)->getLayout();
    }
    std::stringstream layoutLabelStream;
    layoutLabelStream << program->name.c_str() << " layout";
    const auto layoutLabel = layoutLabelStream.str();
    const wgpu::PipelineLayoutDescriptor layoutDescriptor{
        .label = wgpu::StringView(layoutLabel),
        .bindGroupLayoutCount = bindGroupLayoutCount,
        .bindGroupLayouts = bindGroupLayouts.data()
        // TODO investigate immediateDataRangeByteSize
    };
    const wgpu::PipelineLayout layout = mDevice.CreatePipelineLayout(&layoutDescriptor);
    FILAMENT_CHECK_POSTCONDITION(layout)
            << "Failed to create wgpu::PipelineLayout for render pipeline for "
            << layoutDescriptor.label;
    auto const* vertexBufferInfo = handleCast<WGPUVertexBufferInfo>(pipelineState.vertexBufferInfo);
    assert_invariant(vertexBufferInfo);
    const wgpu::RenderPipeline pipeline = createWebGPURenderPipeline(mDevice, *program,
            *vertexBufferInfo, layout, pipelineState.rasterState, pipelineState.stencilState,
            pipelineState.polygonOffset, pipelineState.primitiveType, mSwapChain->getColorFormat(),
            mSwapChain->getDepthFormat());
    mPipelineMap[hash] = pipeline;
    mRenderPassEncoder.SetPipeline(pipeline);
}

void WebGPUDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
    auto* renderPrimitive = handleCast<WGPURenderPrimitive>(rph);

    // This *must* match the WGPUVertexBufferInfo that was bound in bindPipeline(). But we want
    // to allow to call this before bindPipeline(), so the validation can only happen in draw()
    auto vbi = handleCast<WGPUVertexBufferInfo>(renderPrimitive->vertexBuffer->vbih);
    assert_invariant(
            vbi->getVertexBufferLayoutSize() == renderPrimitive->vertexBuffer->buffers.size());
    for (uint32_t i = 0; i < vbi->getVertexBufferLayoutSize(); i++) {
        mRenderPassEncoder.SetVertexBuffer(i, renderPrimitive->vertexBuffer->buffers[i]);
    }

    mRenderPassEncoder.SetIndexBuffer(renderPrimitive->indexBuffer->getBuffer(),
            renderPrimitive->indexBuffer->indexFormat);
}

void WebGPUDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
    mRenderPassEncoder.DrawIndexed(indexCount, instanceCount, indexOffset, 0, 0);
}

void WebGPUDriver::draw(PipelineState, Handle<HwRenderPrimitive>, uint32_t indexOffset,
        uint32_t indexCount, uint32_t instanceCount) {
    draw2(indexOffset, indexCount, instanceCount);
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

void WebGPUDriver::updateDescriptorSetBuffer(Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding, Handle<HwBufferObject> boh, uint32_t offset,
        uint32_t size) {
    auto bindGroup = handleCast<WebGPUDescriptorSet>(dsh);
    auto buffer = handleCast<WGPUBufferObject>(boh);
    if (!bindGroup->getIsLocked()) {
        // TODO making assumptions that size and offset mean the same thing here.
        wgpu::BindGroupEntry entry{ .binding = static_cast<uint32_t>(binding * 2),
            .buffer = buffer->getBuffer(),
            .offset = offset,
            .size = size };
        bindGroup->addEntry(entry.binding, std::move(entry));
    }
}

void WebGPUDriver::updateDescriptorSetTexture(Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding, Handle<HwTexture> th, SamplerParams params) {
    auto bindGroup = handleCast<WebGPUDescriptorSet>(dsh);
    auto texture = handleCast<WGPUTexture>(th);

    if (!bindGroup->getIsLocked()) {
        // Dawn will cache duplicate samplers, so we don't strictly need to maintain a cache.
        //  Making a cache might save us minor perf by reducing param translation
        auto sampler = makeSampler(params);
        // TODO making assumptions that size and offset mean the same thing here.
        wgpu::BindGroupEntry tEntry{ .binding = static_cast<uint32_t>(binding * 2),
            .textureView = texture->getTexView() };
        bindGroup->addEntry(tEntry.binding, std::move(tEntry));

        wgpu::BindGroupEntry sEntry{ .binding = static_cast<uint32_t>(binding * 2 + 1),
            .sampler = sampler };
        bindGroup->addEntry(sEntry.binding, std::move(sEntry));
    }
}

void WebGPUDriver::bindDescriptorSet(Handle<HwDescriptorSet> dsh,
        backend::descriptor_set_t setIndex, backend::DescriptorSetOffsetArray&& offsets) {
    const auto bindGroup = handleCast<WebGPUDescriptorSet>(dsh);
    const auto wbg = bindGroup->lockAndReturn(mDevice);
    assert_invariant(mRenderPassEncoder);
    const size_t dynamicOffsetCount = bindGroup->countEntitiesWithDynamicOffsets();
    mRenderPassEncoder.SetBindGroup(setIndex, wbg, dynamicOffsetCount, offsets.data());
}

void WebGPUDriver::setDebugTag(HandleBase::HandleId handleId, utils::CString tag) {
}
wgpu::Sampler WebGPUDriver::makeSampler(SamplerParams const& params) {
    wgpu::SamplerDescriptor desc;

    desc.label = "TODO";
    desc.addressModeU = fWrapModeToWAddressMode(params.wrapS);
    desc.addressModeV = fWrapModeToWAddressMode(params.wrapR);
    desc.addressModeW = fWrapModeToWAddressMode(params.wrapT);
    switch (params.filterMag) {
        case SamplerMagFilter::NEAREST: {
            desc.magFilter = wgpu::FilterMode::Nearest;
            break;
        }
        case SamplerMagFilter::LINEAR: {
            desc.magFilter = wgpu::FilterMode::Linear;
            break;
        }
    }
    switch (params.filterMin) {
        case SamplerMinFilter::NEAREST: {
            desc.minFilter = wgpu::FilterMode::Nearest;
            // Metal Driver uses an explicit not-mipmapped value webgpu lacks. Nearest should
            // suffice
            desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
            break;
        }
        case SamplerMinFilter::LINEAR: {
            desc.minFilter = wgpu::FilterMode::Linear;
            // Metal Driver uses an explicit not-mipmapped value webgpu lacks. Nearest should
            // suffice

            desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
            break;
        }
        case SamplerMinFilter::NEAREST_MIPMAP_NEAREST: {
            desc.minFilter = wgpu::FilterMode::Nearest;
            desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
            break;
        }
        case SamplerMinFilter::LINEAR_MIPMAP_NEAREST: {
            desc.minFilter = wgpu::FilterMode::Linear;
            desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;

            break;
        }
        case SamplerMinFilter::NEAREST_MIPMAP_LINEAR: {
            desc.minFilter = wgpu::FilterMode::Nearest;
            desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;

            break;
        }
        case SamplerMinFilter::LINEAR_MIPMAP_LINEAR: {
            desc.minFilter = wgpu::FilterMode::Linear;
            desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
            break;
        }
    }
    switch (params.compareFunc) {
        case SamplerCompareFunc::LE: {
            desc.compare = wgpu::CompareFunction::LessEqual;
            break;
        }
        case SamplerCompareFunc::GE: {
            desc.compare = wgpu::CompareFunction::GreaterEqual;
            break;
        }
        case SamplerCompareFunc::L: {
            desc.compare = wgpu::CompareFunction::Less;
            break;
        }
        case SamplerCompareFunc::G: {
            desc.compare = wgpu::CompareFunction::Greater;
            break;
        }
        case SamplerCompareFunc::E: {
            desc.compare = wgpu::CompareFunction::Equal;
            break;
        }
        case SamplerCompareFunc::NE: {
            desc.compare = wgpu::CompareFunction::NotEqual;
            break;
        }
        case SamplerCompareFunc::A: {
            desc.compare = wgpu::CompareFunction::Always;
            break;
        }
        case SamplerCompareFunc::N: {
            desc.compare = wgpu::CompareFunction::Never;
            break;
        }
    }

    desc.maxAnisotropy = 1u << params.anisotropyLog2;


    // Unused: Filament's compareMode, WGPU lodMinClamp/lodMaxClamp

    //TODO Once we can properly map to descriptorsetlayout use the sampler.
    return mDevice.CreateSampler(/*&desc*/);
}
wgpu::AddressMode WebGPUDriver::fWrapModeToWAddressMode(const SamplerWrapMode& fWrapMode) {
    switch (fWrapMode) {
        case SamplerWrapMode::CLAMP_TO_EDGE: {
            return wgpu::AddressMode::ClampToEdge;
            break;
        }
        case SamplerWrapMode::REPEAT: {
            return wgpu::AddressMode::Repeat;
            break;
        }
        case SamplerWrapMode::MIRRORED_REPEAT: {
            return wgpu::AddressMode::MirrorRepeat;
            break;
        }
    }
    return wgpu::AddressMode::Undefined;
}


} // namespace filament
