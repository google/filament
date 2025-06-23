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

#include "WebGPUBufferObject.h"
#include "WebGPUDescriptorSet.h"
#include "WebGPUDescriptorSetLayout.h"
#include "WebGPUFence.h"
#include "WebGPUIndexBuffer.h"
#include "WebGPUPipelineCreation.h"
#include "WebGPUProgram.h"
#include "WebGPURenderPrimitive.h"
#include "WebGPURenderTarget.h"
#include "WebGPUStrings.h"
#include "WebGPUSwapChain.h"
#include "WebGPUTexture.h"
#include "WebGPUVertexBuffer.h"
#include "WebGPUVertexBufferInfo.h"
#include <backend/platforms/WebGPUPlatform.h>

#include "CommandStreamDispatcher.h"
#include "DriverBase.h"
#include "private/backend/Dispatcher.h"
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>
#include <private/backend/BackendUtils.h>

#include <math/mat3.h>
#include <utils/CString.h>
#include <utils/Hash.h>
#include <utils/Panic.h>
#include <utils/compiler.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <utility>

using namespace std::chrono_literals;

namespace filament::backend {

Driver* WebGPUDriver::create(WebGPUPlatform& platform, const Platform::DriverConfig& driverConfig) noexcept {
    constexpr size_t defaultSize = FILAMENT_WEBGPU_HANDLE_ARENA_SIZE_IN_MB * 1024U * 1024U;
    Platform::DriverConfig validConfig {driverConfig};
    validConfig.handleArenaSize = std::max(driverConfig.handleArenaSize, defaultSize);
    return new WebGPUDriver(platform, validConfig);
}

WebGPUDriver::WebGPUDriver(WebGPUPlatform& platform,
        const Platform::DriverConfig& driverConfig) noexcept
    : mPlatform{ platform },
      mAdapter{ mPlatform.requestAdapter(nullptr) },
      mDevice{ mPlatform.requestDevice(mAdapter) },
      mQueue{ mDevice.GetQueue() },
      mRenderPassMipmapGenerator{ mDevice },
      mSpdComputePassMipmapGenerator{ mDevice },
      mHandleAllocator{ "Handles", driverConfig.handleArenaSize,
          driverConfig.disableHandleUseAfterFreeCheck, driverConfig.disableHeapHandleTags } {
    mDevice.GetLimits(&mDeviceLimits);
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
    mAdapter.GetInstance().ProcessEvents();
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
        destructHandle<WebGPURenderPrimitive>(rph);
    }
}

void WebGPUDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vertexBufferInfoHandle) {
    if (vertexBufferInfoHandle) {
        destructHandle<WebGPUVertexBufferInfo>(vertexBufferInfoHandle);
    }
}

void WebGPUDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vertexBufferHandle) {
    if (vertexBufferHandle) {
        destructHandle<WebGPUVertexBuffer>(vertexBufferHandle);
    }
}

void WebGPUDriver::destroyIndexBuffer(Handle<HwIndexBuffer> indexBufferHandle) {
    if (indexBufferHandle) {
        destructHandle<WebGPUIndexBuffer>(indexBufferHandle);
    }
}

void WebGPUDriver::destroyBufferObject(Handle<HwBufferObject> bufferObjectHandle) {
    if (bufferObjectHandle) {
        destructHandle<WebGPUBufferObject>(bufferObjectHandle);
    }
}

void WebGPUDriver::destroyTexture(Handle<HwTexture> textureHandle) {
    if (textureHandle) {
        destructHandle<WebGPUTexture>(textureHandle);
    }
}

void WebGPUDriver::destroyProgram(Handle<HwProgram> programHandle) {
    if (programHandle) {
        destructHandle<WebGPUProgram>(programHandle);
    }
}

void WebGPUDriver::destroyRenderTarget(Handle<HwRenderTarget> renderTargetHandle) {
    if (renderTargetHandle) {
        auto renderTarget = handleCast<WebGPURenderTarget>(renderTargetHandle);
        if (renderTarget == mDefaultRenderTarget) {
            mDefaultRenderTarget = nullptr;
        }
        if (renderTarget == mCurrentRenderTarget) {
            mCurrentRenderTarget = nullptr;
        }
        // WGPURenderTarget destructor is trivial.
        // The HwTexture handles stored within WGPURenderTarget (via MRT, TargetBufferInfo)
        // are not owned by WGPURenderTarget, so they are not destroyed here.
        // They are destroyed via WebGPUDriver::destroyTexture.
        destructHandle<WebGPURenderTarget>(renderTargetHandle);
    }
}

void WebGPUDriver::destroySwapChain(Handle<HwSwapChain> sch) {
    if (sch) {
        destructHandle<WebGPUSwapChain>(sch);
    }
    mSwapChain = nullptr;
}

void WebGPUDriver::destroyStream(Handle<HwStream> sh) {
    //TODO
}

void WebGPUDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
}

void WebGPUDriver::destroyDescriptorSetLayout(
        Handle<HwDescriptorSetLayout> descriptorSetLayoutHandle) {
    if (descriptorSetLayoutHandle) {
        destructHandle<WebGPUDescriptorSetLayout>(descriptorSetLayoutHandle);
    }
}

void WebGPUDriver::destroyDescriptorSet(Handle<HwDescriptorSet> descriptorSetHandle) {
    auto bindGroup = handleCast<WebGPUDescriptorSet>(descriptorSetHandle);
    assert_invariant(bindGroup);
    if (bindGroup->getBindGroup() != nullptr) {
        for (size_t i = 0; i < MAX_DESCRIPTOR_SET_COUNT; i++) {
            if (mCurrentDescriptorSets[i].bindGroup != nullptr &&
                    bindGroup->getBindGroup().Get() == mCurrentDescriptorSets[i].bindGroup.Get()) {
                // Clear this from our current entries
                mCurrentDescriptorSets[i].bindGroup = nullptr;
                mCurrentDescriptorSets[i].offsets.clear();
                mCurrentDescriptorSets[i].offsetCount = 0;
            }
        }
    }
    if (descriptorSetHandle) {
        destructHandle<WebGPUDescriptorSet>(descriptorSetHandle);
    }
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainS() noexcept {
    return allocHandle<WebGPUSwapChain>();
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainHeadlessS() noexcept {
    return Handle<HwSwapChain>((Handle<HwSwapChain>::HandleId) mNextFakeHandle++);
}

Handle<HwTexture> WebGPUDriver::createTextureS() noexcept {
    return allocHandle<WebGPUTexture>();
}

Handle<HwTexture> WebGPUDriver::importTextureS() noexcept { return allocHandle<WebGPUTexture>(); }

Handle<HwProgram> WebGPUDriver::createProgramS() noexcept {
    return allocHandle<WebGPUProgram>();
}

Handle<HwFence> WebGPUDriver::createFenceS() noexcept {
    // The handle must be constructed here, as a synchronous call to get the status
    // might happen before createFenceR is executed.
    return allocAndConstructHandle<WebGPUFence, HwFence>();
}

Handle<HwTimerQuery> WebGPUDriver::createTimerQueryS() noexcept {
    return Handle<HwTimerQuery>((Handle<HwTimerQuery>::HandleId) mNextFakeHandle++);
}

Handle<HwIndexBuffer> WebGPUDriver::createIndexBufferS() noexcept {
    return allocHandle<HwIndexBuffer>();
}

Handle<HwTexture> WebGPUDriver::createTextureViewS() noexcept {
    return allocHandle<WebGPUTexture>();
}

Handle<HwBufferObject> WebGPUDriver::createBufferObjectS() noexcept {
    return allocHandle<WebGPUBufferObject>();
}

Handle<HwRenderTarget> WebGPUDriver::createRenderTargetS() noexcept {
    return allocHandle<WebGPURenderTarget>();
}

Handle<HwVertexBuffer> WebGPUDriver::createVertexBufferS() noexcept {
    return allocHandle<WebGPUVertexBuffer>();
}

Handle<HwDescriptorSet> WebGPUDriver::createDescriptorSetS() noexcept {
    return allocHandle<WebGPUDescriptorSet>();
}

Handle<HwRenderPrimitive> WebGPUDriver::createRenderPrimitiveS() noexcept {
    return allocHandle<WebGPURenderPrimitive>();
}

Handle<HwVertexBufferInfo> WebGPUDriver::createVertexBufferInfoS() noexcept {
    return allocHandle<WebGPUVertexBufferInfo>();
}

Handle<HwTexture> WebGPUDriver::createTextureViewSwizzleS() noexcept {
    return allocHandle<WebGPUTexture>();
}

Handle<HwRenderTarget> WebGPUDriver::createDefaultRenderTargetS() noexcept {
    return allocHandle<WebGPURenderTarget>();
}

Handle<HwDescriptorSetLayout> WebGPUDriver::createDescriptorSetLayoutS() noexcept {
    return allocHandle<WebGPUDescriptorSetLayout>();
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImageS() noexcept {
    return allocHandle<WebGPUTexture>();
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImage2S() noexcept {
    return allocHandle<WebGPUTexture>();
}

Handle<HwTexture> WebGPUDriver::createTextureExternalImagePlaneS() noexcept {
    return allocHandle<WebGPUTexture>();
}

void WebGPUDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    mNativeWindow = nativeWindow;
    assert_invariant(!mSwapChain);
    wgpu::Surface surface = mPlatform.createSurface(nativeWindow, flags);

    wgpu::Extent2D surfaceSize = mPlatform.getSurfaceExtent(mNativeWindow);
    mSwapChain = constructHandle<WebGPUSwapChain>(sch, std::move(surface), surfaceSize, mAdapter,
            mDevice, flags);
    assert_invariant(mSwapChain);

#if !FWGPU_ENABLED(FWGPU_PRINT_SYSTEM) && !defined(NDEBUG)
    char printSystemHex[16];
    snprintf(printSystemHex, sizeof(printSystemHex), "%#x", FWGPU_PRINT_SYSTEM);
    FWGPU_LOGI << "If the FILAMENT_BACKEND_DEBUG_FLAG variable were set with the " << printSystemHex
               << " bit flag on during build time the application would print system details "
                  "about the selected graphics device, surface, etc. To see this try "
                  "rebuilding Filament with that flag, e.g. ./build.sh -x "
               << FWGPU_PRINT_SYSTEM << " ...";
#endif
}

void WebGPUDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch, uint32_t width,
        uint32_t height, uint64_t flags) {}

void WebGPUDriver::createVertexBufferInfoR(Handle<HwVertexBufferInfo> vertexBufferInfoHandle,
        const uint8_t bufferCount, const uint8_t attributeCount, const AttributeArray attributes) {
    constructHandle<WebGPUVertexBufferInfo>(vertexBufferInfoHandle, bufferCount, attributeCount,
            attributes, mDeviceLimits);
}

void WebGPUDriver::createVertexBufferR(Handle<HwVertexBuffer> vertexBufferHandle,
        const uint32_t vertexCount, Handle<HwVertexBufferInfo> vertexBufferInfoHandle) {
    const auto vertexBufferInfo = handleCast<WebGPUVertexBufferInfo>(vertexBufferInfoHandle);
    constructHandle<WebGPUVertexBuffer>(vertexBufferHandle, vertexCount,
            vertexBufferInfo->bufferCount, vertexBufferInfoHandle);
}

void WebGPUDriver::createIndexBufferR(Handle<HwIndexBuffer> indexBufferHandle,
        const ElementType elementType, const uint32_t indexCount, const BufferUsage usage) {
    const auto elementSize = static_cast<uint8_t>(getElementTypeSize(elementType));
    constructHandle<WebGPUIndexBuffer>(indexBufferHandle, mDevice, elementSize, indexCount);
}

void WebGPUDriver::createBufferObjectR(Handle<HwBufferObject> bufferObjectHandle,
        const uint32_t byteCount, const BufferObjectBinding bindingType, const BufferUsage usage) {
    constructHandle<WebGPUBufferObject>(bufferObjectHandle, mDevice, bindingType, byteCount);
}

void WebGPUDriver::createTextureR(Handle<HwTexture> textureHandle, const SamplerType target,
        const uint8_t levels, const TextureFormat format, const uint8_t samples,
        const uint32_t width, const uint32_t height, const uint32_t depth,
        const TextureUsage usage) {
    constructHandle<WebGPUTexture>(textureHandle, target, levels, format, samples, width, height,
            depth, usage, mDevice);
}

void WebGPUDriver::createTextureViewR(Handle<HwTexture> textureHandle,
        Handle<HwTexture> sourceTextureHandle, const uint8_t baseLevel, const uint8_t levelCount) {
    auto source = handleCast<WebGPUTexture>(sourceTextureHandle);

    constructHandle<WebGPUTexture>(textureHandle, source, baseLevel, levelCount);
}

void WebGPUDriver::createTextureViewSwizzleR(Handle<HwTexture> textureHandle,
        Handle<HwTexture> sourceTextureHandle, const backend::TextureSwizzle r,
        const backend::TextureSwizzle g, const backend::TextureSwizzle b,
        const backend::TextureSwizzle a) {
    PANIC_POSTCONDITION("Swizzle WebGPU Texture is not supported");
}

void WebGPUDriver::createTextureExternalImage2R(Handle<HwTexture> textureHandle,
        const backend::SamplerType target, const backend::TextureFormat format,
        const uint32_t width, const uint32_t height, const backend::TextureUsage usage,
        Platform::ExternalImageHandleRef externalImage) {
    PANIC_POSTCONDITION("External WebGPU Texture is not supported");
}

void WebGPUDriver::createTextureExternalImageR(Handle<HwTexture> textureHandle,
        const backend::SamplerType target, const backend::TextureFormat format,
        const uint32_t width, const uint32_t height, const backend::TextureUsage usage,
        void* externalImage) {
    PANIC_POSTCONDITION("External WebGPU Texture is not supported");
}

void WebGPUDriver::createTextureExternalImagePlaneR(Handle<HwTexture> textureHandle,
        const backend::TextureFormat format, const uint32_t width, const uint32_t height,
        const backend::TextureUsage usage, void* image, const uint32_t plane) {
    PANIC_POSTCONDITION("External WebGPU Texture is not supported");
}

void WebGPUDriver::importTextureR(Handle<HwTexture> textureHandle, const intptr_t id,
        const SamplerType target, const uint8_t levels, const TextureFormat format,
        const uint8_t samples, const uint32_t width, const uint32_t height, const uint32_t depth,
        const TextureUsage usage) {
    PANIC_POSTCONDITION("Import WebGPU Texture is not supported");
}

void WebGPUDriver::createRenderPrimitiveR(Handle<HwRenderPrimitive> renderPrimitiveHandle,
        Handle<HwVertexBuffer> vertexBufferHandle, Handle<HwIndexBuffer> indexBufferHandle,
        const PrimitiveType primitiveType) {
    assert_invariant(mDevice);
    const auto renderPrimitive = constructHandle<WebGPURenderPrimitive>(renderPrimitiveHandle);
    const auto vertexBuffer = handleCast<WebGPUVertexBuffer>(vertexBufferHandle);
    const auto indexBuffer = handleCast<WebGPUIndexBuffer>(indexBufferHandle);
    renderPrimitive->vertexBuffer = vertexBuffer;
    renderPrimitive->indexBuffer = indexBuffer;
    renderPrimitive->type = primitiveType;
}

void WebGPUDriver::createProgramR(Handle<HwProgram> programHandle, Program&& program) {
    constructHandle<WebGPUProgram>(programHandle, mDevice, program);
}

void WebGPUDriver::createDefaultRenderTargetR(Handle<HwRenderTarget> renderTargetHandle,
        const int /* dummy */) {
    assert_invariant(!mDefaultRenderTarget);
    mDefaultRenderTarget = constructHandle<WebGPURenderTarget>(renderTargetHandle);
    assert_invariant(mDefaultRenderTarget);
}

void WebGPUDriver::createRenderTargetR(Handle<HwRenderTarget> renderTargetHandle,
        const TargetBufferFlags targetFlags, const uint32_t width, const uint32_t height,
        const uint8_t samples, const uint8_t layerCount, const MRT color,
        const TargetBufferInfo depth, const TargetBufferInfo stencil) {
    constructHandle<WebGPURenderTarget>(renderTargetHandle, width, height, samples, layerCount,
            color, depth, stencil, targetFlags);
}

void WebGPUDriver::createFenceR(Handle<HwFence> fenceHandle, const int /* dummy */) {
    // the handle was already constructed in createFenceS
    const auto fence = handleCast<WebGPUFence>(fenceHandle);
    assert_invariant(mQueue);
    fence->addMarkerToQueueState(mQueue);
}

void WebGPUDriver::createTimerQueryR(Handle<HwTimerQuery> tqh, int) {}

void WebGPUDriver::createDescriptorSetLayoutR(
        Handle<HwDescriptorSetLayout> descriptorSetLayoutHandle,
        backend::DescriptorSetLayout&& info) {
    constructHandle<WebGPUDescriptorSetLayout>(descriptorSetLayoutHandle, std::move(info), mDevice);
}

void WebGPUDriver::createDescriptorSetR(Handle<HwDescriptorSet> descriptorSetHandle,
        Handle<HwDescriptorSetLayout> descriptorSetLayoutHandle) {
    auto layout = handleCast<WebGPUDescriptorSetLayout>(descriptorSetLayoutHandle);
    constructHandle<WebGPUDescriptorSet>(descriptorSetHandle, layout->getLayout(),
            layout->getBindGroupEntries());
}

Handle<HwStream> WebGPUDriver::createStreamNative(void* nativeStream) {
    return {
        //todo
    };
}

Handle<HwStream> WebGPUDriver::createStreamAcquired() {
    return {
        //todo
    };
}

void WebGPUDriver::setAcquiredImage(Handle<HwStream> sh, void* image, const math::mat3f& transform,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
    //todo
}

void WebGPUDriver::registerBufferObjectStreams(Handle<HwBufferObject> bufferObjectHandle,
        BufferObjectStreamDescriptor&& streams) {
    //todo
}

void WebGPUDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
    //todo
}

int64_t WebGPUDriver::getStreamTimestamp(Handle<HwStream> sh) {
    //todo
    return 0;
}

void WebGPUDriver::updateStreams(CommandStream* driver) {
    //todo
}

void WebGPUDriver::destroyFence(Handle<HwFence> fenceHandle) {
    if (fenceHandle) {
        destructHandle<WebGPUFence>(fenceHandle);
    }
}

FenceStatus WebGPUDriver::getFenceStatus(Handle<HwFence> fenceHandle) {
    const auto fence = handleCast<WebGPUFence>(fenceHandle);
    if (!fence) {
        return FenceStatus::ERROR;
    }
    return fence->getStatus();
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool WebGPUDriver::isTextureFormatSupported(const TextureFormat format) {
    return WebGPUTexture::fToWGPUTextureFormat(format) != wgpu::TextureFormat::Undefined;
}

bool WebGPUDriver::isTextureSwizzleSupported() {
    return false;
}

bool WebGPUDriver::isTextureFormatMipmappable(const TextureFormat format) {
    return WebGPUTexture::supportsMultipleMipLevelsViaStorageBinding(
            WebGPUTexture::fToWGPUTextureFormat(format));
}

bool WebGPUDriver::isRenderTargetFormatSupported(const TextureFormat format) {
    //todo
    return true;
}

bool WebGPUDriver::isFrameBufferFetchSupported() {
    //todo
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

bool WebGPUDriver::isDepthStencilBlitSupported(const TextureFormat format) {
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
    //todo
    return FeatureLevel::FEATURE_LEVEL_1;
}

math::float2 WebGPUDriver::getClipSpaceParams() {
    return math::float2{ 1.0f, 0.0f };
}

uint8_t WebGPUDriver::getMaxDrawBuffers() {
    return MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;
}

size_t WebGPUDriver::getMaxUniformBufferSize() {
    return mDeviceLimits.maxUniformBufferBindingSize;
}

size_t WebGPUDriver::getMaxTextureSize(const SamplerType target) {
    size_t result = 2048u;
    switch (target) {
        case SamplerType::SAMPLER_2D:
        case SamplerType::SAMPLER_2D_ARRAY:
        case SamplerType::SAMPLER_EXTERNAL:
        case SamplerType::SAMPLER_CUBEMAP:
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            result = mDeviceLimits.maxTextureDimension2D;
            break;
        case SamplerType::SAMPLER_3D:
            result = mDeviceLimits.maxTextureDimension3D;
            break;
    }
    return result;
}

size_t WebGPUDriver::getMaxArrayTextureLayers() {
    return mDeviceLimits.maxTextureArrayLayers;
}

void WebGPUDriver::updateIndexBuffer(Handle<HwIndexBuffer> indexBufferHandle,
        BufferDescriptor&& bufferDescriptor, const uint32_t byteOffset) {
    handleCast<WebGPUIndexBuffer>(indexBufferHandle)
            ->updateGPUBuffer(bufferDescriptor, byteOffset, mQueue);
    scheduleDestroy(std::move(bufferDescriptor));
}

void WebGPUDriver::updateBufferObject(Handle<HwBufferObject> bufferObjectHandle,
        BufferDescriptor&& bufferDescriptor, const uint32_t byteOffset) {
    handleCast<WebGPUBufferObject>(bufferObjectHandle)
            ->updateGPUBuffer(bufferDescriptor, byteOffset, mQueue);
    scheduleDestroy(std::move(bufferDescriptor));
}

void WebGPUDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> bufferObjectHandle,
        BufferDescriptor&& bufferDescriptor, const uint32_t byteOffset) {
    handleCast<WebGPUBufferObject>(bufferObjectHandle)
            ->updateGPUBuffer(bufferDescriptor, byteOffset, mQueue);
    scheduleDestroy(std::move(bufferDescriptor));
}

void WebGPUDriver::resetBufferObject(Handle<HwBufferObject> bufferObjectHandle) {
    // Is there something that needs to be done here? Vulkan has left it unimplemented.
}

void WebGPUDriver::setVertexBufferObject(Handle<HwVertexBuffer> vertexBufferHandle,
        const uint32_t index, Handle<HwBufferObject> bufferObjectHandle) {
    const auto vertexBuffer = handleCast<WebGPUVertexBuffer>(vertexBufferHandle);
    const auto bufferObject = handleCast<WebGPUBufferObject>(bufferObjectHandle);
    assert_invariant(index < vertexBuffer->getBuffers().size());
    assert_invariant(bufferObject->getBuffer().GetUsage() & wgpu::BufferUsage::Vertex);
    vertexBuffer->getBuffers()[index] = bufferObject->getBuffer();
}

void WebGPUDriver::update3DImage(Handle<HwTexture> textureHandle, const uint32_t level,
        const uint32_t xoffset, const uint32_t yoffset, const uint32_t zoffset,
        const uint32_t width, const uint32_t height, const uint32_t depth,
        PixelBufferDescriptor&& pixelBufferDescriptor) {
    PixelBufferDescriptor* data = &pixelBufferDescriptor;
    PixelBufferDescriptor reshapedData;
    if (reshape(pixelBufferDescriptor, reshapedData)) {
        data = &reshapedData;
    }
    auto texture = handleCast<WebGPUTexture>(textureHandle);

    // TODO: Writing to a depth texture is illegal and errors. I'm not sure why Filament is trying
    // to do so, but early returning is working?
    if(texture->getAspect() == wgpu::TextureAspect::DepthOnly){
        scheduleDestroy(std::move(pixelBufferDescriptor));
        return;
    }
    size_t blockWidth = texture->getBlockWidth();
    size_t blockHeight = texture->getBlockHeight();
    // WebGPU specification requires that for compressed textures, the x and y offsets
    // must be a multiple of the compressed texture format's block width and height.
    // See: https://www.w3.org/TR/webgpu/#abstract-opdef-validating-gputexelcopytextureinfo
    if (blockWidth > 1 || blockHeight > 1) {
        FILAMENT_CHECK_PRECONDITION(xoffset % blockWidth == 0)
                << "xoffset must be aligned to blockwidth, but offset is " << blockWidth
                << "and offset is " << xoffset;
        FILAMENT_CHECK_PRECONDITION(yoffset % blockHeight == 0)
                << "yoffset must be aligned to blockHeight, but offset is " << blockHeight
                << "and offset is " << yoffset;
    }

    auto copyInfo = wgpu::TexelCopyTextureInfo{
        .texture = texture->getTexture(),
        .mipLevel = level,
        .origin = { .x = xoffset, .y = yoffset, .z = zoffset },
        .aspect = texture->getAspect() };
    uint32_t bytesPerRow = static_cast<uint32_t>(
            PixelBufferDescriptor::computePixelSize(data->format, data->type) * width);
    auto extent = wgpu::Extent3D{ .width = width, .height = height, .depthOrArrayLayers = depth };

    const uint8_t* dataBuff = static_cast<const uint8_t*>(data->buffer);
    size_t dataSize = data->size;
    std::unique_ptr<uint8_t[]> paddedBuffer;

    if (bytesPerRow % 256 != 0) {
        uint32_t padding = 256 - (bytesPerRow % 256);
        uint32_t paddedBytesPerRow = bytesPerRow + padding;

        size_t paddedBufferSize = static_cast<size_t>(paddedBytesPerRow) * height * depth;
        paddedBuffer = std::make_unique<uint8_t[]>(paddedBufferSize);
        uint8_t* dest = paddedBuffer.get();

        for (uint32_t z = 0; z < depth; ++z) {
            for (uint32_t y = 0; y < height; ++y) {
                std::memcpy(dest, dataBuff, bytesPerRow);
                dest += paddedBytesPerRow;
                dataBuff += bytesPerRow;
            }
        }
        dataBuff = paddedBuffer.get();
        dataSize = paddedBufferSize;
        bytesPerRow = paddedBytesPerRow;
    }

    auto layout = wgpu::TexelCopyBufferLayout{ .bytesPerRow = bytesPerRow, .rowsPerImage = height };

    mQueue.WriteTexture(&copyInfo, dataBuff, dataSize, &layout, &extent);
    scheduleDestroy(std::move(pixelBufferDescriptor));
}

void WebGPUDriver::setupExternalImage(void* image) {
    //todo
}

TimerQueryResult WebGPUDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    return TimerQueryResult::ERROR;
}

void WebGPUDriver::setupExternalImage2(Platform::ExternalImageHandleRef image) {
    //todo
}

void WebGPUDriver::setExternalStream(Handle<HwTexture> textureHandle,
        Handle<HwStream> streamHandle) {
    // todo
}

void WebGPUDriver::generateMipmaps(Handle<HwTexture> textureHandle) {
    auto texture = handleCast<WebGPUTexture>(textureHandle);
    assert_invariant(texture);
    wgpu::Texture wgpuTexture = texture->getTexture();
    assert_invariant(wgpuTexture);

    const uint32_t totalMipLevels = wgpuTexture.GetMipLevelCount();
    if (totalMipLevels <= 1) {
        return; // nothing to do
    }

    const auto usage = wgpuTexture.GetUsage();
    FILAMENT_CHECK_PRECONDITION(usage & wgpu::TextureUsage::TextureBinding)
            << "Texture for mipmap generation must have TextureBinding usage.";

    switch (texture->getMipmapGenerationStrategy()) {
        case WebGPUTexture::MipmapGenerationStrategy::RENDER_PASS:
            FILAMENT_CHECK_PRECONDITION(usage & wgpu::TextureUsage::RenderAttachment)
                    << "Texture for mipmap generation via render pass blit must have "
                       "RenderAttachment usage.";
            mRenderPassMipmapGenerator.generateMipmaps(mQueue, texture->getTexture());
            break;
        case WebGPUTexture::MipmapGenerationStrategy::NONE:
            FWGPU_LOGW << "Trying to generate mipmaps for a texture we already know we cannot do "
                          "such generation. Texture sample type "
                       << to_string(texture->target) << " WebGPU texture format "
                       << webGPUTextureFormatToString(texture->getViewFormat()) << " samples "
                       << static_cast<uint32_t>(texture->samples)
                       << ". Going to try using the SPD_COMPUTE_PASS approach.";
            UTILS_FALLTHROUGH;
        case WebGPUTexture::MipmapGenerationStrategy::SPD_COMPUTE_PASS:
            FILAMENT_CHECK_PRECONDITION(usage & wgpu::TextureUsage::StorageBinding)
                    << "Texture for mipmap generation must have StorageBinding usage "
                       "(as a compute pass strategy was selected for use). "
                       "supportsMultipleMipLevels for WebGPU texture format "
                    << webGPUTextureFormatToString(wgpuTexture.GetFormat()) << " is "
                    << WebGPUTexture::supportsMultipleMipLevelsViaStorageBinding(
                               wgpuTexture.GetFormat());
            // We will record all passes into a single command encoder.
            wgpu::CommandEncoderDescriptor encoderDesc = {};
            encoderDesc.label = "Mipmap Command Encoder";
            wgpu::CommandEncoder encoder = mDevice.CreateCommandEncoder(&encoderDesc);

            spd::SPDPassConfig spdConfig = { .filter = spd::SPDFilter::Average,
                .targetTexture = wgpuTexture,
                .numMips = totalMipLevels,
                .halfPrecision = false,
                .sourceMipLevel = 0 };

            mSpdComputePassMipmapGenerator.Generate(encoder, wgpuTexture, spdConfig);

            // Finish the encoder and submit all the passes at once.
            wgpu::CommandBufferDescriptor cmdBufferDesc = {};
            cmdBufferDesc.label = "Mipmap Command Buffer";
            wgpu::CommandBuffer commandBuffer = encoder.Finish(&cmdBufferDesc);
            mQueue.Submit(1, &commandBuffer);
            break;
    }
}

void WebGPUDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        scheduleCallback(handler, user, callback);
    }
}

void WebGPUDriver::beginRenderPass(Handle<HwRenderTarget> renderTargetHandle,
        RenderPassParams const& params) {
    assert_invariant(mCommandEncoder);
    auto renderTarget = handleCast<WebGPURenderTarget>(renderTargetHandle);

    wgpu::TextureView defaultColorView = nullptr;
    wgpu::TextureView defaultDepthStencilView = nullptr;
    wgpu::TextureFormat defaultDepthStencilFormat = wgpu::TextureFormat::Undefined;

    std::array<wgpu::TextureView, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT> customColorViews{};
    uint32_t customColorViewCount = 0;

    wgpu::TextureView customDepthStencilView = nullptr;
    wgpu::TextureFormat customDepthStencilFormat = wgpu::TextureFormat::Undefined;

    mCurrentRenderTarget = renderTarget;
    if (renderTarget->isDefaultRenderTarget()) {
        assert_invariant(mSwapChain && mTextureView);
        defaultColorView = mTextureView;
        defaultDepthStencilView = mSwapChain->getDepthTextureView();
        defaultDepthStencilFormat = mSwapChain->getDepthFormat();

        if (any(renderTarget->getTargetFlags() & TargetBufferFlags::STENCIL) &&
            !(defaultDepthStencilFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
              defaultDepthStencilFormat == wgpu::TextureFormat::Depth32FloatStencil8 ||
              defaultDepthStencilFormat == wgpu::TextureFormat::Stencil8)) {
            FILAMENT_CHECK_POSTCONDITION(false) << "Default render target requested stencil, but swap chain's depth format "
                                                << (uint32_t)defaultDepthStencilFormat << " does not have a stencil aspect.";
        }

        if (any(renderTarget->getTargetFlags() & TargetBufferFlags::DEPTH) &&
            !(defaultDepthStencilFormat == wgpu::TextureFormat::Depth16Unorm ||
              defaultDepthStencilFormat == wgpu::TextureFormat::Depth32Float ||
              defaultDepthStencilFormat == wgpu::TextureFormat::Depth24Plus ||
              defaultDepthStencilFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
              defaultDepthStencilFormat == wgpu::TextureFormat::Depth32FloatStencil8)) {
            FILAMENT_CHECK_POSTCONDITION(false) << "Default render target requested depth, but swap chain's depth format "
                                                << (uint32_t)defaultDepthStencilFormat << " does not have a depth aspect.";
        }
    } else {
        // Resolve views for custom render target
        const auto& colorInfos = renderTarget->getColorAttachmentInfos();
        for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
            if (colorInfos[i].handle) {
                auto colorTexture = handleCast<WebGPUTexture>(colorInfos[i].handle);
                if (colorTexture) {
                    FILAMENT_CHECK_POSTCONDITION(
                            colorInfos[i].layer < renderTarget->getLayerCount())
                            << "Color attachment " << i << " requests layer " << colorInfos[i].layer
                            << " but render target has only " << renderTarget->getLayerCount()
                            << ".";
                    const uint8_t mipLevel = colorInfos[i].level;
                    const uint32_t arrayLayer = colorInfos[i].layer;
                    customColorViews[customColorViewCount++] =
                            colorTexture->getOrMakeTextureView(mipLevel, arrayLayer);
                }
            }
        }
        const auto& depthInfo = renderTarget->getDepthAttachmentInfo();
        const auto& stencilInfo = renderTarget->getStencilAttachmentInfo();

        Handle<HwTexture> depthStencilSourceHandle = {};
        uint8_t depthStencilMipLevel = 0;
        uint32_t depthStencilArrayLayer = 0;

        if (depthInfo.handle && stencilInfo.handle) {
            auto const depthTexture{handleCast<WebGPUTexture>(depthInfo.handle)->getTexture()};
            auto const stencilTexture{handleCast<WebGPUTexture>(stencilInfo.handle)->getTexture()};
            FILAMENT_CHECK_POSTCONDITION(depthTexture.Get() == stencilTexture.Get())
                    << "Filament must reference the same resource if both depth and stencil "
                       "textures are present";
        }

        if (depthInfo.handle) {
            depthStencilSourceHandle = depthInfo.handle;
            depthStencilMipLevel = depthInfo.level;
            depthStencilArrayLayer = depthInfo.layer;
        } else if (stencilInfo.handle) {
            depthStencilSourceHandle = stencilInfo.handle;
            depthStencilMipLevel = stencilInfo.level;
            depthStencilArrayLayer = stencilInfo.layer;
        }

        if (depthStencilSourceHandle) {
            auto dsTexture = handleCast<WebGPUTexture>(depthStencilSourceHandle);
            if (dsTexture) {
                customDepthStencilView = dsTexture->getOrMakeTextureView(depthStencilMipLevel, depthStencilArrayLayer);
                customDepthStencilFormat = dsTexture->getViewFormat();

                if (any(renderTarget->getTargetFlags() & TargetBufferFlags::STENCIL) &&
                        !(customDepthStencilFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                                customDepthStencilFormat ==
                                        wgpu::TextureFormat::Depth32FloatStencil8 ||
                                customDepthStencilFormat ==
                                        wgpu::TextureFormat::Stencil8)) {
                    FILAMENT_CHECK_POSTCONDITION(false)
                            << "Custom render target requested stencil, but the provided texture"
                               "format number"
                            << (uint32_t) customDepthStencilFormat
                            << " does not have a stencil aspect.";
                }
                if (any(renderTarget->getTargetFlags() & TargetBufferFlags::DEPTH) &&
                        !(customDepthStencilFormat == wgpu::TextureFormat::Depth16Unorm ||
                                customDepthStencilFormat == wgpu::TextureFormat::Depth32Float ||
                                customDepthStencilFormat ==
                                        wgpu::TextureFormat::Depth24Plus ||
                                customDepthStencilFormat ==
                                        wgpu::TextureFormat::Depth24PlusStencil8 ||
                                customDepthStencilFormat ==
                                        wgpu::TextureFormat::Depth32FloatStencil8)) {
                    FILAMENT_CHECK_POSTCONDITION(false) << "Custom render target requested depth, "
                                                           "but the provided texture format number"
                                                        << (uint32_t) customDepthStencilFormat
                                                        << " does not have a depth aspect.";
                }
            }
        }
    }
    wgpu::RenderPassDescriptor renderPassDescriptor{};
    renderTarget->setUpRenderPassAttachments(renderPassDescriptor,
            params,
            defaultColorView,
            defaultDepthStencilView,
            customColorViews.data(),
            customColorViewCount,
            customDepthStencilView);

    mRenderPassEncoder = mCommandEncoder.BeginRenderPass(&renderPassDescriptor);

    // Ensure viewport dimensions are not 0
    FILAMENT_CHECK_POSTCONDITION(params.viewport.width > 0) << "viewport width is 0?";
    FILAMENT_CHECK_POSTCONDITION(params.viewport.height > 0) << "viewport height is 0?";

    mRenderPassEncoder.SetViewport(
            static_cast<float>(params.viewport.left),
            static_cast<float>(params.viewport.bottom),
            static_cast<float>(params.viewport.width),
            static_cast<float>(params.viewport.height),
            params.depthRange.near,
            params.depthRange.far);
}

void WebGPUDriver::endRenderPass(int /* dummy */) {
    mRenderPassEncoder.End();
    mRenderPassEncoder = nullptr;
}

void WebGPUDriver::nextSubpass(int) {
    //todo
}

void WebGPUDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
    ASSERT_PRECONDITION_NON_FATAL(drawSch == readSch,
            "WebGPU driver does not support distinct draw/read swap chains.");
    auto swapChain = handleCast<WebGPUSwapChain>(drawSch);
    mSwapChain = swapChain;
    assert_invariant(mSwapChain);
    wgpu::Extent2D surfaceSize = mPlatform.getSurfaceExtent(mNativeWindow);
    mTextureView = mSwapChain->getCurrentSurfaceTextureView(surfaceSize);
    assert_invariant(mTextureView);

    assert_invariant(mDefaultRenderTarget);

    wgpu::TextureFormat depthFormat = mSwapChain->getDepthFormat();
    TargetBufferFlags newTargetFlags = filament::backend::TargetBufferFlags::NONE;

    //Assuming Color and Depth are always present.
    newTargetFlags |= filament::backend::TargetBufferFlags::COLOR;
    if (depthFormat != wgpu::TextureFormat::Undefined) {
        newTargetFlags |= filament::backend::TargetBufferFlags::DEPTH;

        if (depthFormat == wgpu::TextureFormat::Depth24PlusStencil8 ||
                depthFormat == wgpu::TextureFormat::Depth32FloatStencil8) {
            newTargetFlags |= filament::backend::TargetBufferFlags::STENCIL;
        }
    }
    mDefaultRenderTarget->setTargetFlags(newTargetFlags);

    wgpu::CommandEncoderDescriptor commandEncoderDescriptor = { .label = "frame_command_encoder" };
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

    static bool firstRender = true;
    // For the first frame rendered, we need to make sure the work is done before presenting or we
    // get a purple flash
    if (firstRender) {
        auto f = mQueue.OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                [=](wgpu::QueueWorkDoneStatus) {});
        const wgpu::Instance instance = mAdapter.GetInstance();
        auto wStatus = instance.WaitAny(f,
                std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count());
        if (wStatus != wgpu::WaitStatus::Success) {
            FWGPU_LOGW << "Waiting for first frame work to finish resulted in an error"
                       << static_cast<uint32_t>(wStatus);
        }
        firstRender = false;
    }
    mCommandBuffer = nullptr;
    mTextureView = nullptr;
    assert_invariant(mSwapChain);
    mSwapChain->present();
}

void WebGPUDriver::setPushConstant(backend::ShaderStage stage, uint8_t index,
        backend::PushConstantVariant value) {
    //todo
}

void WebGPUDriver::insertEventMarker(char const* string) {
    //todo
}

void WebGPUDriver::pushGroupMarker(char const* string) {
    //todo
}

void WebGPUDriver::popGroupMarker(int) {
    //todo
}

void WebGPUDriver::startCapture(int) {
    //todo
}

void WebGPUDriver::stopCapture(int) {
    //todo
}

void WebGPUDriver::readPixels(Handle<HwRenderTarget> sourceRenderTargetHandle, const uint32_t x,
        const uint32_t y, const uint32_t width, const uint32_t height,
        PixelBufferDescriptor&& pixelBufferDescriptor) {
    // todo
    scheduleDestroy(std::move(pixelBufferDescriptor));
}

void WebGPUDriver::readBufferSubData(Handle<HwBufferObject> bufferObjectHandle,
        const uint32_t offset, const uint32_t size, backend::BufferDescriptor&& bufferDescriptor) {
    // todo
    scheduleDestroy(std::move(bufferDescriptor));
}

void WebGPUDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> destinationRenderTargetHandle, const Viewport destinationViewport,
        Handle<HwRenderTarget> sourceRenderTargetHandle, const Viewport sourceViewport,
        const SamplerMagFilter filter) {}

void WebGPUDriver::resolve(Handle<HwTexture> destinationTextureHandle, const uint8_t sourceLevel,
        const uint8_t sourceLayer, Handle<HwTexture> sourceTextureHandle,
        const uint8_t destinationLevel, const uint8_t destinationLayer) {
    // todo
}

void WebGPUDriver::blit(Handle<HwTexture> destinationTextureHandle, const uint8_t sourceLevel,
        const uint8_t sourceLayer, const math::uint2 destinationOrigin,
        Handle<HwTexture> sourceTextureHandle, const uint8_t destinationLevel,
        const uint8_t destinationLayer, const math::uint2 sourceOrigin, const math::uint2 size) {
    // todo
}

void WebGPUDriver::bindPipeline(PipelineState const& pipelineState) {
    // TODO Investigate implications of this hash more closely. Vulkan has a whole class
    // VulkanPipelineCache to handle this, may be missing nuance
    static auto pipleineStateHasher = utils::hash::MurmurHashFn<filament::backend::PipelineState>();
    auto hash = pipleineStateHasher(pipelineState);
    if (mPipelineMap.find(hash) != mPipelineMap.end()) {
        mRenderPassEncoder.SetPipeline(mPipelineMap[hash]);
        return;
    }
    const auto program = handleCast<WebGPUProgram>(pipelineState.program);
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
    const auto vertexBufferInfo =
            handleCast<WebGPUVertexBufferInfo>(pipelineState.vertexBufferInfo);
    assert_invariant(vertexBufferInfo);

    std::vector<wgpu::TextureFormat> pipelineColorFormats;
    wgpu::TextureFormat pipelineDepthStencilFormat = wgpu::TextureFormat::Undefined;
    uint8_t pipelineSamples = 1;
    bool const requestedDepth = any(mCurrentRenderTarget->getTargetFlags() & TargetBufferFlags::DEPTH);
    bool const requestedStencil = any(mCurrentRenderTarget->getTargetFlags() & TargetBufferFlags::STENCIL);
    pipelineSamples = mCurrentRenderTarget->getSamples();

    if (mCurrentRenderTarget->isDefaultRenderTarget()) {
        pipelineColorFormats.push_back(mSwapChain->getColorFormat());
        pipelineDepthStencilFormat = mSwapChain->getDepthFormat();
    } else {
        const auto& mrtColorAttachments = mCurrentRenderTarget->getColorAttachmentInfos();
        for (size_t i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
            if (mrtColorAttachments[i].handle) {
                const auto colorTexture = handleCast<WebGPUTexture>(mrtColorAttachments[i].handle);
                if (colorTexture) {
                    pipelineColorFormats.push_back(colorTexture->getTexture().GetFormat());
                }
            }
        }

        const auto& depthInfo = mCurrentRenderTarget->getDepthAttachmentInfo();
        const auto& stencilInfo = mCurrentRenderTarget->getStencilAttachmentInfo();
        Handle<HwTexture> depthStencilHandle = {};
        if (depthInfo.handle) {
            depthStencilHandle = depthInfo.handle;
        } else if (stencilInfo.handle) {
             depthStencilHandle = stencilInfo.handle;
        }

        if (depthStencilHandle) {
            const auto dsTexture = handleCast<WebGPUTexture>(depthStencilHandle);
            if (dsTexture) {
                pipelineDepthStencilFormat = dsTexture->getTexture().GetFormat();
            }
        }
    }

    // TODO: We expected this to be a sane check, however it complains when running shadowtest.
    //if (program->fragmentShaderModule != nullptr) {
    //    FILAMENT_CHECK_POSTCONDITION(!pipelineColorFormats.empty())
    //            << "Render pipeline with fragment shader must have at least one color target "
    //               "format.";
    //}
    wgpu::RenderPipeline pipeline = createWebGPURenderPipeline(mDevice, *program, *vertexBufferInfo,
            layout, pipelineState.rasterState, pipelineState.stencilState,
            pipelineState.polygonOffset, pipelineState.primitiveType, pipelineColorFormats,
            pipelineDepthStencilFormat, pipelineSamples, requestedDepth, requestedStencil);
    assert_invariant(pipeline);
    mPipelineMap[hash] = pipeline;
    mRenderPassEncoder.SetPipeline(pipeline);
}

void WebGPUDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> renderPrimitiveHandle) {
    const auto renderPrimitive = handleCast<WebGPURenderPrimitive>(renderPrimitiveHandle);
    const auto vertexBufferInfo = handleCast<WebGPUVertexBufferInfo>(
            renderPrimitive->vertexBuffer->getVertexBufferInfoHandle());
    for (size_t slotIndex = 0; slotIndex < vertexBufferInfo->getVertexBufferLayoutCount();
            slotIndex++) {
        WebGPUVertexBufferInfo::WebGPUSlotBindingInfo const& bindingInfo =
                vertexBufferInfo->getWebGPUSlotBindingInfos()[slotIndex];
        mRenderPassEncoder.SetVertexBuffer(slotIndex,
                renderPrimitive->vertexBuffer->getBuffers()[bindingInfo.sourceBufferIndex],
                bindingInfo.bufferOffset);
    }
    mRenderPassEncoder.SetIndexBuffer(renderPrimitive->indexBuffer->getBuffer(),
            renderPrimitive->indexBuffer->getIndexFormat());
}

void WebGPUDriver::draw2(const uint32_t indexOffset, const uint32_t indexCount,
        const uint32_t instanceCount) {
    // We defer actually binding until we actually draw
    for (size_t i = 0; i < MAX_DESCRIPTOR_SET_COUNT; i++) {
        auto& binding = mCurrentDescriptorSets[i];
        if (binding.bindGroup) {
            mRenderPassEncoder.SetBindGroup(i, binding.bindGroup, binding.offsetCount,
                    binding.offsets.data());
        }
    }

    mRenderPassEncoder.DrawIndexed(indexCount, instanceCount, indexOffset, 0, 0);
}

void WebGPUDriver::draw(PipelineState pipelineState,
        Handle<HwRenderPrimitive> renderPrimitiveHandle, const uint32_t indexOffset,
        const uint32_t indexCount, const uint32_t instanceCount) {
    WebGPURenderPrimitive const* const renderPrimitive =
            handleCast<WebGPURenderPrimitive>(renderPrimitiveHandle);
    pipelineState.primitiveType = renderPrimitive->type;
    pipelineState.vertexBufferInfo = renderPrimitive->vertexBuffer->getVertexBufferInfoHandle();
    bindPipeline(pipelineState);
    bindRenderPrimitive(renderPrimitiveHandle);
    draw2(indexOffset, indexCount, instanceCount);
}

void WebGPUDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
    //todo
}

void WebGPUDriver::scissor(
        Viewport scissor) {
    //todo
}

void WebGPUDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
}

void WebGPUDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
}

void WebGPUDriver::resetState(int) {
    //todo
}

void WebGPUDriver::updateDescriptorSetBuffer(Handle<HwDescriptorSet> descriptorSetHandle,
        const backend::descriptor_binding_t binding, Handle<HwBufferObject> bufferObjectHandle,
        const uint32_t offset, const uint32_t size) {
    const auto bindGroup = handleCast<WebGPUDescriptorSet>(descriptorSetHandle);
    const auto buffer = handleCast<WebGPUBufferObject>(bufferObjectHandle);
    if (!bindGroup->getIsLocked()) {
        // TODO making assumptions that size and offset mean the same thing here.
        FILAMENT_CHECK_PRECONDITION(offset % mDeviceLimits.minUniformBufferOffsetAlignment == 0)
                << "Binding offset must be multiple of "
                << mDeviceLimits.minUniformBufferOffsetAlignment << "But requested offset is "
                << offset;
        wgpu::BindGroupEntry entry{
            .binding = static_cast<uint32_t>(binding * 2),
            .buffer = buffer->getBuffer(),
            .offset = offset,
            .size = size };
        bindGroup->addEntry(entry.binding, std::move(entry));
    }
}

void WebGPUDriver::updateDescriptorSetTexture(Handle<HwDescriptorSet> descriptorSetHandle,
        const backend::descriptor_binding_t binding, Handle<HwTexture> textureHandle,
        const SamplerParams params) {
    auto bindGroup = handleCast<WebGPUDescriptorSet>(descriptorSetHandle);
    auto texture = handleCast<WebGPUTexture>(textureHandle);

    if (!bindGroup->getIsLocked()) {
        // Dawn will cache duplicate samplers, so we don't strictly need to maintain a cache.
        //  Making a cache might save us minor perf by reducing param translation
        const auto sampler = makeSampler(params);
        // TODO making assumptions that size and offset mean the same thing here.
        wgpu::BindGroupEntry tEntry{
            .binding = static_cast<uint32_t>(binding * 2),
            .textureView = texture->getDefaultTextureView() };
        bindGroup->addEntry(tEntry.binding, std::move(tEntry));

        wgpu::BindGroupEntry sEntry{
            .binding = static_cast<uint32_t>(binding * 2 + 1),
            .sampler = sampler };
        bindGroup->addEntry(sEntry.binding, std::move(sEntry));
    }
}

void WebGPUDriver::bindDescriptorSet(Handle<HwDescriptorSet> descriptorSetHandle,
        const backend::descriptor_set_t setIndex, backend::DescriptorSetOffsetArray&& offsets) {
    assert_invariant(setIndex < MAX_DESCRIPTOR_SET_COUNT);
    const auto bindGroup = handleCast<WebGPUDescriptorSet>(descriptorSetHandle);
    const auto wbg = bindGroup->lockAndReturn(mDevice);

    mCurrentDescriptorSets[setIndex] = {
        .bindGroup = wbg,
        .offsetCount = bindGroup->getEntitiesWithDynamicOffsetsCount(),
        .offsets = std::move(offsets) };
}

void WebGPUDriver::setDebugTag(HandleBase::HandleId handleId, utils::CString tag) {
    //todo
}

wgpu::Sampler WebGPUDriver::makeSampler(SamplerParams const& params) {
    wgpu::SamplerDescriptor desc{};

    desc.label = "TODO";
    desc.addressModeU = fWrapModeToWAddressMode(params.wrapS);
    desc.addressModeV = fWrapModeToWAddressMode(params.wrapT);
    desc.addressModeW = fWrapModeToWAddressMode(params.wrapR);

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
            desc.mipmapFilter = wgpu::MipmapFilterMode::Undefined;
            break;
        }
        case SamplerMinFilter::LINEAR: {
            desc.minFilter = wgpu::FilterMode::Linear;
            desc.mipmapFilter = wgpu::MipmapFilterMode::Undefined;
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
    if (params.compareMode == SamplerCompareMode::COMPARE_TO_TEXTURE) {
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
    }

    desc.maxAnisotropy = 1u << params.anisotropyLog2;

    // Unused: WGPU lodMinClamp/lodMaxClamp unnecessary

    return mDevice.CreateSampler(&desc);
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
