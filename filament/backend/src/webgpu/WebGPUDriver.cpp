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
#include "WebGPUConstants.h"
#include "WebGPUDescriptorSet.h"
#include "WebGPUDescriptorSetLayout.h"
#include "WebGPUFence.h"
#include "WebGPUIndexBuffer.h"
#include "WebGPUPipelineCache.h"
#include "WebGPUPipelineLayoutCache.h"
#include "WebGPUProgram.h"
#include "WebGPURenderPrimitive.h"
#include "WebGPURenderTarget.h"
#include "WebGPUStrings.h"
#include "WebGPUSwapChain.h"
#include "WebGPUTexture.h"
#include "WebGPUTextureHelpers.h"
#include "WebGPUVertexBuffer.h"
#include "WebGPUVertexBufferInfo.h"
#include "webgpu/utils/AsyncTaskCounter.h"
#include <backend/platforms/WebGPUPlatform.h>

#include "CommandStreamDispatcher.h"
#include "DriverBase.h"
#include "private/backend/Dispatcher.h"
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>
#include <private/backend/BackendUtils.h>

#include <math/mat3.h>
#include <utils/debug.h>
#include <utils/CString.h>
#include <utils/Hash.h>
#include <utils/Panic.h>
#include <utils/compiler.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <utility>

using namespace std::chrono_literals;

namespace filament::backend {

namespace {

void setDefaultTargetFlags(WebGPURenderTarget& defaultRenderTarget,
        WebGPUSwapChain const& swapChain) {
    const wgpu::TextureFormat depthFormat{ swapChain.getDepthFormat() };
    TargetBufferFlags newTargetFlags{ filament::backend::TargetBufferFlags::NONE };

    // Color is always assumed to be present in the default render target.
    newTargetFlags |= filament::backend::TargetBufferFlags::COLOR;
    if (depthFormat != wgpu::TextureFormat::Undefined) {
        if (hasDepth(depthFormat)) {
            newTargetFlags |= filament::backend::TargetBufferFlags::DEPTH;
        }
        if (hasStencil(depthFormat)) {
            newTargetFlags |= filament::backend::TargetBufferFlags::STENCIL;
        }
    }
    defaultRenderTarget.setTargetFlags(newTargetFlags);
}

} // namespace

// The WebGPUDriver is the main entry point for the WebGPU backend. It is responsible for
// creating and managing all WebGPU resources, and for submitting commands to the GPU.
// It implements the Driver interface, which is the abstraction layer used by Filament's
// renderer. The driver uses a handle-based system to manage resources, and it operates
// asynchronously, with `create...S` methods for synchronous handle allocation and
// `create...R` methods for resource creation on the backend thread.
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
      mPipelineLayoutCache{ mDevice },
      mPipelineCache{ mDevice },
      mRenderPassMipmapGenerator{ mDevice },
      mSpdComputePassMipmapGenerator{ mDevice },
      mBlitter{ mDevice },
      mHandleAllocator{ "Handles", driverConfig.handleArenaSize,
          driverConfig.disableHandleUseAfterFreeCheck, driverConfig.disableHeapHandleTags }{
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

void WebGPUDriver::endFrame(const uint32_t /* frameId */) {
    mPipelineLayoutCache.onFrameEnd();
    mPipelineCache.onFrameEnd();

    // Clear the currently bound descriptor sets at the end of the frame.
    for (size_t i = 0; i < MAX_DESCRIPTOR_SET_COUNT; i++) {
        mCurrentDescriptorSets[i] = {};
    }
}

// If a command encoder is in flight then the encoder is finished and submitted to the GPU queue.
// A new command encoder is created fresh after it.
// If a command encoder is not in flight (nullptr), then this is a noop.
void WebGPUDriver::flush(int /* dummy */) {
    if (mCommandEncoder == nullptr) {
        return;
    }
    // Submit the commands recorded so far.
    assert_invariant(mRenderPassEncoder == nullptr);
    wgpu::CommandBufferDescriptor commandBufferDescriptor{
        .label = "frame_command_after_flush",
    };
    mCommandBuffer = mCommandEncoder.Finish(&commandBufferDescriptor);
    assert_invariant(mCommandBuffer);
    mQueue.Submit(1, &mCommandBuffer);
    mCommandBuffer = nullptr;
    // create a new command buffer encoder to continue recording the next command for frame...
    wgpu::CommandEncoderDescriptor commandEncoderDescriptor = {
        .label = "frame_command_after_flush"
    };
    mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
    assert_invariant(mCommandEncoder);
}

// Submits the currently recorded commands and waits for them to complete on the GPU.
// This is a synchronous operation and should be used sparingly.
void WebGPUDriver::finish(int /* dummy */) {
    mDevice.Tick();
    mAdapter.GetInstance().ProcessEvents();
    flush();
    // Wait for all previously submitted work to finish.
    std::mutex syncPoint;
    std::condition_variable syncCondition;
    bool done = false;
    mQueue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowSpontaneous,
            [&syncPoint, &syncCondition, &done](wgpu::QueueWorkDoneStatus status, wgpu::StringView message) {
                if (status != wgpu::QueueWorkDoneStatus::Success)
                {
                    FWGPU_LOGW << "WebGPUDriver::finish: QueueWorkDoneStatus was not successful. " << message;
                }
                assert_invariant(status == wgpu::QueueWorkDoneStatus::Success);
                std::unique_lock<std::mutex> lock(syncPoint);
                done = true;
                syncCondition.notify_one();
            });
    std::unique_lock<std::mutex> lock(syncPoint);
    syncCondition.wait(lock, [&done] { return done; });
    mReadPixelMapsCounter.waitForAllToFinish();
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
        // The HwTexture handles stored within WGPURenderTarget are not owned by it,
        // so they are not destroyed here. They are destroyed via WebGPUDriver::destroyTexture.
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

// ------------------------------------------------------------------------------------------------
// Synchronous handle creation methods (S-methods)
//
// These methods are called from the frontend thread to allocate a handle for a backend resource.
// They are lightweight and do not perform any GPU operations. The actual resource creation
// happens in the corresponding R-method on the backend thread.
// ------------------------------------------------------------------------------------------------

Handle<HwSwapChain> WebGPUDriver::createSwapChainS() noexcept {
    return allocHandle<WebGPUSwapChain>();
}

Handle<HwSwapChain> WebGPUDriver::createSwapChainHeadlessS() noexcept {
    return allocHandle<WebGPUSwapChain>();
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

// ------------------------------------------------------------------------------------------------
// Asynchronous resource creation methods (R-methods)
//
// These methods are called on the backend thread to create the actual GPU resources
// associated with a handle that was previously allocated by an S-method.
// ------------------------------------------------------------------------------------------------

void WebGPUDriver::createSwapChainR(Handle<HwSwapChain> sch, void* nativeWindow, uint64_t flags) {
    mNativeWindow = nativeWindow;
    wgpu::Surface surface = mPlatform.createSurface(nativeWindow, flags);

    wgpu::Extent2D extent = mPlatform.getSurfaceExtent(mNativeWindow);
    mSwapChain = constructHandle<WebGPUSwapChain>(sch, std::move(surface), extent, mAdapter,
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
    if (mDefaultRenderTarget) {
        setDefaultTargetFlags(*mDefaultRenderTarget, *mSwapChain);
    }
}

void WebGPUDriver::createSwapChainHeadlessR(Handle<HwSwapChain> sch, uint32_t width,
        uint32_t height, uint64_t flags) {
     wgpu::Extent2D extent = { .width = width, .height = height };
     mSwapChain = constructHandle<WebGPUSwapChain>(sch, extent, mAdapter,
            mDevice, flags);
     assert_invariant(mSwapChain);
     if (mDefaultRenderTarget) {
        setDefaultTargetFlags(*mDefaultRenderTarget, *mSwapChain);
    }
}

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

    if (!isTextureSwizzleSupported()) {
        FWGPU_LOGW << "WebGPUDriver::createTextureViewSwizzleR called while texture swizzling is "
                      "not supported by the device/driver";
    }

    auto sourceTexture{ handleCast<WebGPUTexture>(sourceTextureHandle) };
    assert_invariant(sourceTexture);

    wgpu::TextureComponentSwizzle swizzle
    {
        .r = toWGPUComponentSwizzle(r),
        .g = toWGPUComponentSwizzle(g),
        .b = toWGPUComponentSwizzle(b),
        .a = toWGPUComponentSwizzle(a),
    };

    wgpu::TextureComponentSwizzleDescriptor swizzleDesc {};
    swizzleDesc.swizzle = swizzle;

    const wgpu::TextureViewDescriptor viewDesc {
        .nextInChain = &swizzleDesc,
        .label = "swizzled_texture_view",
        .format = sourceTexture->getTexture().GetFormat(),
        .dimension = sourceTexture->getViewDimension(),
        .baseMipLevel = 0,
        .mipLevelCount = sourceTexture->getTexture().GetMipLevelCount(),
        .baseArrayLayer = 0,
        .arrayLayerCount = sourceTexture->getTexture().GetDepthOrArrayLayers(),
    };

    wgpu::TextureView swizzledView{ sourceTexture->getTexture().CreateView(&viewDesc) };
    FILAMENT_CHECK_POSTCONDITION(swizzledView) << "Failed to create swizzled Texture view";
    constructHandle<WebGPUTexture>(textureHandle, sourceTexture, swizzledView);
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
    if (mSwapChain) {
        setDefaultTargetFlags(*mDefaultRenderTarget, *mSwapChain);
    }
}

void WebGPUDriver::createRenderTargetR(Handle<HwRenderTarget> renderTargetHandle,
        const TargetBufferFlags targetFlags, const uint32_t width, const uint32_t height,
        const uint8_t samples, const uint8_t layerCount, const MRT color,
        const TargetBufferInfo depth, const TargetBufferInfo stencil) {
    constructHandle<WebGPURenderTarget>(
            renderTargetHandle, width, height, samples, layerCount, color, depth, stencil,
            targetFlags,
            [&](const Handle<HwTexture> textureHandle) {
                return handleCast<WebGPUTexture>(textureHandle);
            },
            mDevice);
}

void WebGPUDriver::createFenceR(Handle<HwFence> fenceHandle, const int /* dummy */) {
    // The handle is constructed synchronously in createFenceS.
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

bool WebGPUDriver::isTextureFormatSupported(const TextureFormat format) {
    return toWGPUTextureFormat(format) != wgpu::TextureFormat::Undefined;
}

bool WebGPUDriver::isTextureSwizzleSupported() {
    return mDevice.HasFeature(wgpu::FeatureName::TextureComponentSwizzle);
}

bool WebGPUDriver::isTextureFormatMipmappable(const TextureFormat format) {
    const wgpu::TextureFormat webGpuFormat{ toWGPUTextureFormat(format) };
    // passing 2D and sampleCount 1 to only check the format
    const WebGPURenderPassMipmapGenerator::FormatCompatibility renderPassCompatibility{
        WebGPURenderPassMipmapGenerator::getCompatibilityFor(webGpuFormat,
                wgpu::TextureDimension::e2D, 1)
    };

    if (renderPassCompatibility.compatible) {
        return true;
    }

    return WebGPUTexture::supportsMultipleMipLevelsViaStorageBinding(webGpuFormat);
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
    return false;
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

    // If the max sampler counts do not meet FeatureLevel2 standards, then this is an FeatureLevel1
    // device.
    const auto& featureLevel2 = FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_2];
    if (mDeviceLimits.maxSamplersPerShaderStage < featureLevel2.MAX_VERTEX_SAMPLER_COUNT ||
            mDeviceLimits.maxSamplersPerShaderStage < featureLevel2.MAX_FRAGMENT_SAMPLER_COUNT) {
        return FeatureLevel::FEATURE_LEVEL_1;
    }

    // If the max sampler counts do not meet FeatureLevel3 standards, then this is an FeatureLevel2
    // device.
    const auto& featureLevel3 = FEATURE_LEVEL_CAPS[+FeatureLevel::FEATURE_LEVEL_3];
    if (mDeviceLimits.maxSamplersPerShaderStage < featureLevel3.MAX_VERTEX_SAMPLER_COUNT ||
            mDeviceLimits.maxSamplersPerShaderStage < featureLevel3.MAX_FRAGMENT_SAMPLER_COUNT) {
        return FeatureLevel::FEATURE_LEVEL_2;
    }
    return FeatureLevel::FEATURE_LEVEL_3;
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
    // make sure command elements (draws, etc.) prior to the buffer update are processed before the
    // update on the GPU, otherwise the expected data may not be available at the time certain
    // draw calls are made.
    flush();
    handleCast<WebGPUIndexBuffer>(indexBufferHandle)
            ->updateGPUBuffer(bufferDescriptor, byteOffset, mQueue);
    scheduleDestroy(std::move(bufferDescriptor));
}

void WebGPUDriver::updateBufferObject(Handle<HwBufferObject> bufferObjectHandle,
        BufferDescriptor&& bufferDescriptor, const uint32_t byteOffset) {
    // make sure command elements (draws, etc.) prior to the buffer update are processed before the
    // update on the GPU, otherwise the expected data may not be available at the time certain
    // draw calls are made.
    flush();
    handleCast<WebGPUBufferObject>(bufferObjectHandle)
            ->updateGPUBuffer(bufferDescriptor, byteOffset, mQueue);
    scheduleDestroy(std::move(bufferDescriptor));
}

void WebGPUDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> bufferObjectHandle,
        BufferDescriptor&& bufferDescriptor, const uint32_t byteOffset) {
    // make sure command elements (draws, etc.) prior to the buffer update are processed before the
    // update on the GPU, otherwise the expected data may not be available at the time certain
    // draw calls are made.
    flush();
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

// Updates a 3D texture region with pixel data from a buffer.
// This function handles various pixel formats and performs a blit operation if the source
// format needs to be converted to the destination format. For compressed textures, it ensures
// that the offsets are aligned to the block dimensions.
void WebGPUDriver::update3DImage(Handle<HwTexture> textureHandle, const uint32_t level,
        const uint32_t xoffset, const uint32_t yoffset, const uint32_t zoffset,
        const uint32_t width, const uint32_t height, const uint32_t depth,
        PixelBufferDescriptor&& pixelBufferDescriptor) {
    // one way or another this function writes texture(s), thus any commands (draw calls etc.) should
    // get submitted prior to these updates so that subsequent commands/draws run with the
    // image/texture(s) updated as expected.
    flush();
    PixelBufferDescriptor* inputData{ &pixelBufferDescriptor };
    PixelBufferDescriptor reshapedData;
    if (reshape(pixelBufferDescriptor, reshapedData)) {
        inputData = &reshapedData;
    }
    const auto texture{ handleCast<WebGPUTexture>(textureHandle) };
    FILAMENT_CHECK_PRECONDITION((texture->getTexture().GetWidth() + xoffset) >= width)
            << "Blitting requires the destination region to have enough width ("
            << texture->getTexture().GetWidth() << ") to accommodate the requested " << width
            << " width to write/blit (accounting for xoffset, which is " << xoffset << ").";
    FILAMENT_CHECK_PRECONDITION((texture->getTexture().GetHeight() + yoffset) >= height)
            << "Blitting requires the destination region to have enough height ("
            << texture->getTexture().GetHeight() << ") to accommodate the requested " << height
            << " height to write/blit (accounting for yoffset, which is " << yoffset << ").";
    FILAMENT_CHECK_PRECONDITION((texture->getTexture().GetDepthOrArrayLayers() + zoffset) >= depth)
            << "Blitting requires the destination region to have enough depth/arrayLayers ("
            << texture->getTexture().GetDepthOrArrayLayers() << ") to accommodate the requested "
            << depth << " depth to write/blit (accounting for zoffset, which is " << zoffset << ").";

    // TODO: Writing to a depth texture is illegal and errors. I'm not sure why Filament is trying
    // to do so, but early returning is working?
    if (texture->getAspect() == wgpu::TextureAspect::DepthOnly) {
        scheduleDestroy(std::move(pixelBufferDescriptor));
        return;
    }
    const wgpu::TextureFormat inputPixelFormat{ toWebGPUFormat(inputData->format,
            inputData->type) };
    const wgpu::TextureFormat outputLinearFormat{ toLinearFormat(
            texture->getTexture().GetFormat()) };

    const bool doBlit{ conversionNecessary(inputPixelFormat, outputLinearFormat, inputData->type) };
#if FWGPU_ENABLED(FWGPU_DEBUG_UPDATE_IMAGE)
    if (texture->width > 1000 && texture->height > 500) {
        FWGPU_LOGD << "Update3DImage(..., level=" << level << ", xoffset=" << xoffset
                   << ", yoffset=" << yoffset << ", zoffset=" << zoffset << ", width=" << width
                   << ", height=" << height << ", depth=" << depth << ",  ...):";
        FWGPU_LOGD << "     PixelBufferDescriptor format (input): " << toString(inputData->format);
        FWGPU_LOGD << "     PixelBufferDescriptor type (input):   " << toString(inputData->type);
        FWGPU_LOGD << "     Pixel WebGPUFormat (input):           "
                   << webGPUTextureFormatToString(inputPixelFormat);
        FWGPU_LOGD << "     Texture View format (output):         "
                   << webGPUTextureFormatToString(texture->getViewFormat());
        FWGPU_LOGD << "     Texture format (output):              "
                   << webGPUTextureFormatToString(texture->getTexture().GetFormat());
        FWGPU_LOGD << "     Linear Texture format (output):       "
                   << webGPUTextureFormatToString(outputLinearFormat);
        FWGPU_LOGD << "     Conversion Necessary:                 " << conversionNecessary;
        FWGPU_LOGD << "     Do Blit:                              " << doBlit;
    }
#endif
    FILAMENT_CHECK_PRECONDITION(inputData->type == PixelDataType::COMPRESSED ||
                                inputPixelFormat != wgpu::TextureFormat::Undefined)
            << "Failed to determine uncompressed input pixel format for WebGPU. Pixel format "
            << toString(inputData->format) << " type " << toString(inputData->type);
    const size_t blockWidth{ texture->getBlockWidth() };
    const size_t blockHeight{ texture->getBlockHeight() };
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
    const auto extent{
        wgpu::Extent3D{ .width = width, .height = height, .depthOrArrayLayers = depth }
    };
    const uint32_t bytesPerRow{ static_cast<uint32_t>(
            PixelBufferDescriptor::computePixelSize(inputData->format, inputData->type) * width) };
    const uint8_t* dataBuff{ static_cast<const uint8_t*>(inputData->buffer) };
    const size_t dataSize{ inputData->size };
    const auto layout{ wgpu::TexelCopyBufferLayout{
        .bytesPerRow = bytesPerRow,
        .rowsPerImage = height,
    } };
    if (doBlit) {
        const wgpu::TextureDescriptor stagingTextureDescriptor{
            .label = "blit_staging_input_texture",
            .usage = texture->getTexture().GetUsage(),
            .dimension = texture->getTexture().GetDimension(),
            .size = extent,
            .format = inputPixelFormat,
            .mipLevelCount = 1,
            .sampleCount = texture->getTexture().GetSampleCount(),
            .viewFormatCount = 0,
            .viewFormats = nullptr,
        };
        const wgpu::Texture stagingTexture{ mDevice.CreateTexture(&stagingTextureDescriptor) };
        FILAMENT_CHECK_POSTCONDITION(stagingTexture)
                << "Failed to create staging input texture for blit?";
        const auto copyInfo{ wgpu::TexelCopyTextureInfo{
            .texture = stagingTexture,
            .mipLevel = 0,
            .origin = { .x = 0, .y = 0, .z = 0 },
            .aspect = texture->getAspect(),
        } };
        mQueue.WriteTexture(&copyInfo, dataBuff, dataSize, &layout, &extent);
        bool reusedCommandEncoder{ true };
        if (!mCommandEncoder) {
            reusedCommandEncoder = false;
            const wgpu::CommandEncoderDescriptor commandEncoderDescriptor{
                .label = "blit_command",
            };
            mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
            FILAMENT_CHECK_POSTCONDITION(mCommandEncoder)
                    << "Failed to create command encoder for blit?";
        }
        WebGPUBlitter::BlitArgs blitArgs{
            .source = {
                .texture = stagingTexture,
                .origin = { .x = 0, .y = 0 },
                .extent = {.width = width, .height = height},
                .mipLevel = 0,
            },
            .destination = {
                .texture = texture->getTexture(),
                .origin = {.x=xoffset,.y=yoffset},
                .extent = {.width = width, .height = height},
                .mipLevel = level,
            },
            .filter = SamplerMagFilter::NEAREST,
        };
        for (uint32_t layerIndex{ 0 }; layerIndex < depth; ++layerIndex) {
            blitArgs.source.layerOrDepth = layerIndex;
            blitArgs.destination.layerOrDepth = layerIndex + zoffset;
            mBlitter.blit(mQueue, mCommandEncoder, blitArgs);
        }
        if (!reusedCommandEncoder) {
            // TODO consider removing encoder finish + submit once we have test coverage and can
            //      safely verify doing so will not result in strange errors. We don't think this
            //      submit is necessary, as the encoder should get finished and the command
            //      submitted in commit(), finish(), and/or flush() in this frame
            //      or the next anyway. And, the blit bits will be encoded before other things.
            //      Thus, not submitting here _seems_ ok. Nonetheless, doing the submit here likely
            //      represents unnecessary inefficiency and higher bandwidth
            //      between the CPU and GPU, but we are worried about functional correctness at this
            //      stage.
            const wgpu::CommandBufferDescriptor commandBufferDescriptor{
                .label = "blit_command_buffer",
            };
            const wgpu::CommandBuffer blitCommand{ mCommandEncoder.Finish(
                    &commandBufferDescriptor) };
            FILAMENT_CHECK_POSTCONDITION(blitCommand)
                    << "Failed to create command buffer for blit?";
            mQueue.Submit(1, &blitCommand);
            mCommandEncoder = nullptr;
        }
    } else {
        // Direct copy without a blit.
        const auto copyInfo { wgpu::TexelCopyTextureInfo{
            .texture = texture->getTexture(),
            .mipLevel = level,
            .origin = { .x = xoffset, .y = yoffset, .z = zoffset, },
            .aspect = texture->getAspect(),
        }};
        mQueue.WriteTexture(&copyInfo, dataBuff, dataSize, &layout, &extent);
    }
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
    if (UTILS_UNLIKELY(totalMipLevels <= 1)) {
        return; // nothing to do
    }

    // make sure command elements (draws, etc.) prior to the texture update are processed before the
    // update on the GPU.
    // this ensures subsequent draw calls are run after the mipmaps have been generated as expected
    flush();

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
            // TODO consider reusing the existing command encoder or creating it in this function
            //      if it doesn't already exist and removing encoder finish + submit here once
            //      we have test coverage and can safely verify doing so will not result in
            //      strange errors. We don't think this submit is necessary, as the encoder
            //      should get finished and the command submitted in
            //      commit(), finish(), and/or flush() in this frame or the next anyway.
            //      And, the mipmap generation bits will be encoded before other things.
            //      Thus, not submitting here _seems_ ok. Nonetheless, doing the submit here likely
            //      represents unnecessary inefficiency and higher bandwidth
            //      between the CPU and GPU, but we are worried about functional correctness at this
            //      stage.
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

// Begins a render pass. This sets up the render pass descriptor with the appropriate
// color and depth/stencil attachments, based on whether the target is the default
// swap chain render target or a custom one. It also handles MSAA by setting up
// resolve targets if necessary.
void WebGPUDriver::beginRenderPass(Handle<HwRenderTarget> renderTargetHandle,
        RenderPassParams const& params) {
    if (!mCommandEncoder) {
        wgpu::CommandEncoderDescriptor commandEncoderDescriptor{ .label = "frame_command_encoder" };
        mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
    }
    assert_invariant(mCommandEncoder);
    auto renderTarget = handleCast<WebGPURenderTarget>(renderTargetHandle);

    wgpu::TextureView defaultColorView = nullptr;
    wgpu::TextureView defaultDepthStencilView = nullptr;
    wgpu::TextureFormat defaultDepthStencilFormat = wgpu::TextureFormat::Undefined;

    const bool msaaSidecarsRequired{ renderTarget->getSamples() > 1 &&
                                     renderTarget->getSampleCountPerAttachment() <= 1 };

    std::array<wgpu::TextureView, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT> customColorViews{};
    std::array<wgpu::TextureView, customColorViews.size()> customColorMsaaSidecarViews{};
    uint32_t customColorViewCount = 0;

    wgpu::TextureView customDepthStencilView = nullptr;
    wgpu::TextureView customDepthStencilMsaaSidecarTextureView = nullptr;
    wgpu::TextureFormat customDepthStencilFormat = wgpu::TextureFormat::Undefined;

    mCurrentRenderTarget = renderTarget;
    if (renderTarget->isDefaultRenderTarget()) {
        assert_invariant(mSwapChain && mTextureView);
        defaultColorView = mTextureView;
        defaultDepthStencilView = mSwapChain->getDepthTextureView();
        defaultDepthStencilFormat = mSwapChain->getDepthFormat();

        if (any(renderTarget->getTargetFlags() & TargetBufferFlags::STENCIL) &&
                !(hasStencil(defaultDepthStencilFormat))) {
            FILAMENT_CHECK_POSTCONDITION(false)
                    << "Default render target requested stencil, but swap chain's depth format "
                    << (uint32_t) defaultDepthStencilFormat << " does not have a stencil aspect.";
        }

        if (any(renderTarget->getTargetFlags() & TargetBufferFlags::DEPTH) &&
                !(hasDepth(defaultDepthStencilFormat))) {
            FILAMENT_CHECK_POSTCONDITION(false)
                    << "Default render target requested depth, but swap chain's depth format "
                    << (uint32_t) defaultDepthStencilFormat << " does not have a depth aspect.";
        }
    } else {
        // Resolve views for custom render target
        const auto& colorInfos = renderTarget->getColorAttachmentInfos();
        for (int i = 0; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
            if (colorInfos[i].handle) {
                auto colorTexture = handleCast<WebGPUTexture>(colorInfos[i].handle);
                if (colorTexture) {
                    const uint8_t mipLevel = colorInfos[i].level;
                    const uint32_t arrayLayer = colorInfos[i].layer;
                    customColorViews[customColorViewCount] =
                            colorTexture->makeAttachmentTextureView(mipLevel, arrayLayer, renderTarget->getLayerCount());
                    if (msaaSidecarsRequired) {
                        const wgpu::TextureView msaaSidecarView{
                            colorTexture->makeMsaaSidecarTextureViewIfTextureSidecarExists(
                                    renderTarget->getSamples(), mipLevel, arrayLayer)
                        };
                        FILAMENT_CHECK_POSTCONDITION(msaaSidecarView)
                                << "Could not get a required MSAA sidecar texture view for color "
                                << customColorViewCount << "?";
                        customColorMsaaSidecarViews[customColorViewCount] = msaaSidecarView;
                    }
                    customColorViewCount++;
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
                customDepthStencilView = dsTexture->makeAttachmentTextureView(depthStencilMipLevel,
                        depthStencilArrayLayer, renderTarget->getLayerCount());
                if (msaaSidecarsRequired) {
                    customDepthStencilMsaaSidecarTextureView =
                            dsTexture->makeMsaaSidecarTextureViewIfTextureSidecarExists(
                                    renderTarget->getSamples(), depthStencilMipLevel,
                                    depthStencilArrayLayer);
                    FILAMENT_CHECK_POSTCONDITION(customDepthStencilMsaaSidecarTextureView)
                            << "Could not get a required MSAA sidecar texture view for "
                               "depth/stencil?";
                }
                customDepthStencilFormat = dsTexture->getViewFormat();

                if (any(renderTarget->getTargetFlags() & TargetBufferFlags::STENCIL) &&
                        !(hasStencil(customDepthStencilFormat))) {
                    FILAMENT_CHECK_POSTCONDITION(false)
                            << "Custom render target requested stencil, but the provided texture"
                               "format number"
                            << (uint32_t) customDepthStencilFormat
                            << " does not have a stencil aspect.";
                }
                if (any(renderTarget->getTargetFlags() & TargetBufferFlags::DEPTH) &&
                        !(hasDepth(customDepthStencilFormat))) {
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
            customColorMsaaSidecarViews.data(),
            customColorViewCount,
            customDepthStencilView,
            customDepthStencilMsaaSidecarTextureView);

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

    if (mSwapChain->isHeadless()) {
        mTextureView = mSwapChain->getNextTextureView();
    } else {
        mTextureView = mSwapChain->getNextTextureView(mPlatform.getSurfaceExtent(mNativeWindow));
    }

    assert_invariant(mTextureView);

    if (!mCommandEncoder) {
        wgpu::CommandEncoderDescriptor commandEncoderDescriptor = { .label = "frame_command_encoder" };
        mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
    }
    assert_invariant(mCommandEncoder);
}

void WebGPUDriver::commit(Handle<HwSwapChain> sch) {
    if (UTILS_VERY_UNLIKELY(!mCommandEncoder)) {
        // nothing to submit
        if (UTILS_LIKELY(mSwapChain)) {
            mSwapChain->present();
        }
        return;
    }
    wgpu::CommandBufferDescriptor commandBufferDescriptor{
        .label = "frame_command",
    };
    mCommandBuffer = mCommandEncoder.Finish(&commandBufferDescriptor);
    assert_invariant(mCommandBuffer);
    mCommandEncoder = nullptr;
    mQueue.Submit(1, &mCommandBuffer);
    mCommandBuffer = nullptr;
    static bool firstRender = true;
    // For the first frame rendered, we need to make sure the work is done before presenting or we
    // get a purple flash
    if (UTILS_UNLIKELY(firstRender)) {
        auto f = mQueue.OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly,
                [](wgpu::QueueWorkDoneStatus status, wgpu::StringView message) {
                    // This lambda does nothing, but the signature must match.
                    if (status != wgpu::QueueWorkDoneStatus::Success){
                        FWGPU_LOGE << "Waiting for first frame work to finish resulted in an error";
                    }
                });
        const wgpu::Instance instance = mAdapter.GetInstance();
        auto wStatus = instance.WaitAny(f,
                std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count());
        if (wStatus != wgpu::WaitStatus::Success) {
            FWGPU_LOGW << "Waiting for first frame work to finish resulted in an error"
                       << static_cast<uint32_t>(wStatus);
        }
        firstRender = false;
    }
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

// Reads a block of pixels from a render target into a buffer.
// This function is asynchronous. It copies the pixel data to a staging buffer on the GPU,
// and then maps the buffer for reading on the CPU. The provided callback is invoked when the
// data is ready. This function also handles the 256-byte row alignment requirement for
// buffer-to-texture copies in WebGPU.
void WebGPUDriver::readPixels(Handle<HwRenderTarget> sourceRenderTargetHandle, const uint32_t x,
        const uint32_t y, const uint32_t width, const uint32_t height,
        PixelBufferDescriptor&& pixelBufferDescriptor) {
    const auto srcTarget{ handleCast<WebGPURenderTarget>(sourceRenderTargetHandle) };
    assert_invariant(srcTarget);

    wgpu::Texture srcTexture{ nullptr };
    if (srcTarget->isDefaultRenderTarget()) {
        assert_invariant(mSwapChain);
        srcTexture = mSwapChain->getCurrentTexture();
    } else {
        // Handle custom render targets. Read from the first color attachment.
        const auto& colorAttachmentInfos{ srcTarget->getColorAttachmentInfos() };
        // TODO we are currently assuming the first attachment is the desired texture.
        if (colorAttachmentInfos[0].handle) {
            auto texture = handleCast<WebGPUTexture>(colorAttachmentInfos[0].handle);
            if (texture) {
                srcTexture = texture->getTexture();
            }
        }
    }

    if (UTILS_UNLIKELY(!srcTexture)) {
        FWGPU_LOGE << "readPixels: Could not find a valid source texture for the render target.";
        scheduleDestroy(std::move(pixelBufferDescriptor));
        return;
    }
    const uint32_t srcWidth {srcTexture.GetWidth()};
    const uint32_t srcHeight{srcTexture.GetHeight()};

    // Clamp read region to texture bounds
    if (UTILS_UNLIKELY(x >= srcWidth || y >= srcHeight)) {
        scheduleDestroy(std::move(pixelBufferDescriptor));
        return;
    }
    auto actualWidth{ std::min(width, srcWidth - x) };
    auto actualHeight{ std::min(height, srcHeight - y)};
    if (UTILS_UNLIKELY(actualWidth == 0 || actualHeight == 0)) {
        scheduleDestroy(std::move(pixelBufferDescriptor));
        return;
    }

    if (!mCommandEncoder) {
        const wgpu::CommandEncoderDescriptor commandEncoderDescriptor{
            .label = "read_pixels_command",
        };
        mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
        FILAMENT_CHECK_POSTCONDITION(mCommandEncoder)
                << "Failed to create command encoder for readPixels?";
    }

    const wgpu::TextureFormat srcFormat{srcTexture.GetFormat()};
    const wgpu::TextureFormat dstFormat{toWebGPUFormat(pixelBufferDescriptor.format,
                                                       pixelBufferDescriptor.type)};
    wgpu::Texture textureToReadFrom{srcTexture};
    wgpu::Texture stagingTexture{ nullptr };
    uint32_t readX{x};
    uint32_t readY{y};

    // If the source format is different from the destination (e.g. BGRA vs RGBA),
    // we need to perform a conversion using an intermediate blit.
    if (conversionNecessary(srcFormat, dstFormat, pixelBufferDescriptor.type)) {
        const wgpu::TextureDescriptor stagingDescriptor{
                .label = "readpixels_staging_texture",
                .usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment,
                .dimension = wgpu::TextureDimension::e2D, // are we sure we can assume this? thus far we have
                .size = {
                        .width = actualWidth,
                        .height = actualHeight,
                        .depthOrArrayLayers = 1,
                },
                .format = dstFormat,
                .mipLevelCount = 1,
                .sampleCount = srcTexture.GetSampleCount(),
        };
        stagingTexture = mDevice.CreateTexture(&stagingDescriptor);
        assert_invariant(stagingTexture);
        textureToReadFrom = stagingTexture;
        const WebGPUBlitter::BlitArgs blitArgs{
                .source = {
                        .texture = srcTexture,
                        .origin = {.x = x, .y = y,},
                        .extent = {
                                .width = actualWidth,
                                .height = actualHeight,
                        },
                },
                .destination = {
                        .texture = stagingTexture,
                        .origin = {.x = 0, .y = 0},
                        .extent = {
                                .width = actualWidth,
                                .height = actualHeight,
                        },
                },
                .filter = SamplerMagFilter::NEAREST, // Use NEAREST for a 1:1 copy
        };
        mBlitter.blit(mQueue, mCommandEncoder, blitArgs);

        // The subsequent read will be from the top-left of the new intermediate texture.
        readX = 0;
        readY = 0;
    }

    // Create a staging buffer to copy the texture to. WebGPU requires 256 byte alignment for buffer-to-texture copies.
    const size_t bytesPerPixel{ PixelBufferDescriptor::computePixelSize(
            pixelBufferDescriptor.format, pixelBufferDescriptor.type) };
    const size_t unpaddedBytesPerRow{ actualWidth * bytesPerPixel };
    const size_t alignment{ 256 };
    const size_t paddedBytesPerRow = { (unpaddedBytesPerRow + alignment - 1) & ~(alignment - 1) };

    const size_t bufferSize{ paddedBytesPerRow * actualHeight };
    const wgpu::BufferDescriptor bufferDesc{
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
        .size = bufferSize,
    };

    wgpu::Buffer stagingBuffer{ mDevice.CreateBuffer(&bufferDesc) };
    assert_invariant(stagingBuffer);

    // WebGPU's texture coordinates for copies are top-left, but Filament's y-coordinate is
    // bottom-left. We must flip the y-coordinate relative to the texture we are reading from.
    const uint32_t textureHeight{ textureToReadFrom.GetHeight() };
    const uint32_t flippedY{ textureHeight - readY - actualHeight };

    const wgpu::TexelCopyTextureInfo source{
            .texture = textureToReadFrom, // Read from the original or intermediate texture
            .mipLevel = 0,
            .origin = {.x = readX, .y = flippedY, .z = 0,},
    };
    const wgpu::TexelCopyBufferInfo destination{
            .layout = {
                    .offset = 0,
                    .bytesPerRow = static_cast<uint32_t>(paddedBytesPerRow),
                    .rowsPerImage = actualHeight,
            },
            .buffer = stagingBuffer
    };
    const wgpu::Extent3D copySize{
        .width = actualWidth,
        .height = actualHeight,
        .depthOrArrayLayers = 1,
    };
    mCommandEncoder.CopyTextureToBuffer(&source, &destination, &copySize);
    mCommandBuffer = mCommandEncoder.Finish();
    assert_invariant(mCommandBuffer);
    mCommandEncoder = nullptr;
    mQueue.Submit(1, &mCommandBuffer);
    mCommandBuffer = nullptr;

    // Map the buffer to read the data
    struct UserData final {
        PixelBufferDescriptor pixelBufferDescriptor;
        wgpu::Buffer buffer;
        size_t unpaddedBytesPerRow;
        size_t paddedBytesPerRow;
        uint32_t height;
        WebGPUDriver *driver;
    };
    auto userData = std::make_unique<UserData>(UserData{
            .pixelBufferDescriptor = std::move(pixelBufferDescriptor),
            .buffer = std::move(stagingBuffer),
            .unpaddedBytesPerRow = unpaddedBytesPerRow,
            .paddedBytesPerRow = paddedBytesPerRow,
            .height = actualHeight,
            .driver = this,
    });

    mReadPixelMapsCounter.startTask();
    userData->buffer.MapAsync(
            wgpu::MapMode::Read, 0, bufferSize, wgpu::CallbackMode::AllowSpontaneous,
            [](wgpu::MapAsyncStatus status, const char* message, UserData* userdata) {
                std::unique_ptr<UserData> data(static_cast<UserData*>(userdata));
                if (UTILS_LIKELY(status == wgpu::MapAsyncStatus::Success)) {
                    const char *src {static_cast<const char *>(
                            data->buffer.GetConstMappedRange(0, data->buffer.GetSize()))};
                    char* dst{ static_cast<char*>(data->pixelBufferDescriptor.buffer) };

                    // If padding was added for alignment, we need to copy row by row.
                    if (data->paddedBytesPerRow == data->unpaddedBytesPerRow) {
                        memcpy(dst, src, data->unpaddedBytesPerRow * data->height);
                    } else {
                        for (uint32_t i{ 0 }; i < data->height; ++i) {
                            memcpy(dst + i * data->unpaddedBytesPerRow,
                                    src + i * data->paddedBytesPerRow, data->unpaddedBytesPerRow);
                        }
                    }
                    data->buffer.Unmap();
                } else {
                    FWGPU_LOGE << "Failed to map staging buffer for readPixels: " << message;
                }
                data->driver->scheduleDestroy(std::move(data->pixelBufferDescriptor));
                data->driver->mReadPixelMapsCounter.finishTask();
            },
            userData.release());
}

void WebGPUDriver::readBufferSubData(Handle<HwBufferObject> bufferObjectHandle,
        const uint32_t offset, const uint32_t size, backend::BufferDescriptor&& bufferDescriptor) {
    // todo
    scheduleDestroy(std::move(bufferDescriptor));
}

void WebGPUDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> destinationRenderTargetHandle, const Viewport destinationViewport,
        Handle<HwRenderTarget> sourceRenderTargetHandle, const Viewport sourceViewport,
        const SamplerMagFilter filter) {

    auto const sourceTarget{ handleCast<WebGPURenderTarget>(sourceRenderTargetHandle) };
    auto const destinationTarget{ handleCast<WebGPURenderTarget>(destinationRenderTargetHandle) };
    assert_invariant(sourceTarget && destinationTarget);

    FILAMENT_CHECK_PRECONDITION(buffers == TargetBufferFlags::COLOR0)
            << "blitDEPRECATED only supports COLOR0";

    FILAMENT_CHECK_PRECONDITION(sourceViewport.left >= 0 && sourceViewport.bottom >= 0 &&
                                destinationViewport.left >= 0 && destinationViewport.bottom >= 0)
            << "Source and destination viewports must be positive.";

    // We always blit from/to the COLOR0 attachment.
    auto sourceAttachment {sourceTarget->getColorAttachmentInfos()[0]};
    Handle<HwTexture> sourceTextureHandle{ sourceAttachment.handle };
    auto destinationAttachment {destinationTarget->getColorAttachmentInfos()[0]};
    Handle<HwTexture> destinationTextureHandle{
        destinationAttachment.handle
    };

    if (UTILS_UNLIKELY(!sourceTextureHandle || !destinationTextureHandle)) {
        FWGPU_LOGE << "blitDEPRECATED could not find a valid color attachment to blit.";
        return;
    }

    // WebGPU's texture coordinates are top-left, while Filament's are bottom-left.
    // We need to flip the y-coordinate for both source and destination.
    auto const sourceTexture{ handleCast<WebGPUTexture>(sourceTextureHandle) };
    auto const destinationTexture{ handleCast<WebGPUTexture>(destinationTextureHandle) };

    const uint32_t sourceTextureHeight{ sourceTexture->height };
    const uint32_t flippedSourceY{ sourceTextureHeight - sourceViewport.bottom -
                                   sourceViewport.height };

    const uint32_t destinationTextureHeight{ destinationTexture->height };
    const uint32_t flippedDestinationY{ destinationTextureHeight - destinationViewport.bottom -
                                        destinationViewport.height };

    const wgpu::Origin2D sourceOrigin{ static_cast<uint32_t>(sourceViewport.left), flippedSourceY };
    const wgpu::Origin2D destinationOrigin{ static_cast<uint32_t>(destinationViewport.left),
        flippedDestinationY };

    const wgpu::Extent2D sourceSize{ static_cast<uint32_t>(sourceViewport.width),
        static_cast<uint32_t>(sourceViewport.height) };
    const wgpu::Extent2D destinationSize{ static_cast<uint32_t>(destinationViewport.width),
        static_cast<uint32_t>(destinationViewport.height) };
    
    bool reusedCommandEncoder{ true };
    if (mCommandEncoder) {
        flush();
    } else {
        reusedCommandEncoder = false;
        const wgpu::CommandEncoderDescriptor desc{ .label = "blit_deprecated_command" };
        mCommandEncoder = mDevice.CreateCommandEncoder(&desc);
    }

    const WebGPUBlitter::BlitArgs blitArgs{
        .source = { .texture = sourceTexture->getTexture(),
                    .aspect = sourceTexture->getAspect(),
                    .origin = sourceOrigin,
                    .extent = sourceSize,
                    .mipLevel = sourceAttachment.level,
                    .layerOrDepth = sourceAttachment.layer,
        },
        .destination = { .texture = destinationTexture->getTexture(),
                         .aspect = destinationTexture->getAspect(),
                         .origin = destinationOrigin,
                         .extent = destinationSize,
                         .mipLevel = destinationAttachment.level,
                         .layerOrDepth = destinationAttachment.layer,
        },
        .filter = filter,
    };
    mBlitter.blit(mQueue, mCommandEncoder, blitArgs);

    if (!reusedCommandEncoder) {
        const wgpu::CommandBufferDescriptor desc{ .label = "blit_deprecated_command_buffer" };
        const wgpu::CommandBuffer blitCommand{ mCommandEncoder.Finish(&desc) };
        mQueue.Submit(1, &blitCommand);
        mCommandEncoder = nullptr;
    }
}

void WebGPUDriver::resolve(Handle<HwTexture> destinationTextureHandle, const uint8_t sourceLevel,
        const uint8_t sourceLayer, Handle<HwTexture> sourceTextureHandle,
        const uint8_t destinationLevel, const uint8_t destinationLayer) {
    if (!mCommandEncoder) {
        const wgpu::CommandEncoderDescriptor encoderDescriptor{
            .label = "command_created_with_resolve",
        };
        mCommandEncoder = mDevice.CreateCommandEncoder(&encoderDescriptor);
    }
    FILAMENT_CHECK_PRECONDITION(mRenderPassEncoder == nullptr)
            << "Resolve cannot be called during an existing render pass";
    const auto sourceTexture{ handleCast<WebGPUTexture>(sourceTextureHandle) };
    const auto destinationTexture{ handleCast<WebGPUTexture>(destinationTextureHandle) };
    assert_invariant(sourceTexture);
    assert_invariant(destinationTexture);
    const WebGPUMsaaTextureResolver::ResolveRequest request{
        .commandEncoder = mCommandEncoder,
        .viewFormat = sourceTexture->getViewFormat(),
        .source = {
            .texture = sourceTexture->getTexture(),
            .viewDimension = sourceTexture->getViewDimension(),
            .mipLevel = sourceLevel,
            .layer = sourceLayer,
            .aspect = sourceTexture->getAspect(),
        },
        .destination = {
            .texture = destinationTexture->getTexture(),
            .viewDimension = destinationTexture->getViewDimension(),
            .mipLevel = destinationLevel,
            .layer = destinationLayer,
            .aspect = destinationTexture->getAspect(),
        },
    };
    mMsaaTextureResolver.resolve(request);
}

void WebGPUDriver::blit(Handle<HwTexture> destinationTextureHandle, const uint8_t sourceLevel,
        const uint8_t sourceLayer, const math::uint2 destinationOrigin,
        Handle<HwTexture> sourceTextureHandle, const uint8_t destinationLevel,
        const uint8_t destinationLayer, const math::uint2 sourceOrigin, const math::uint2 size) {
    bool reusedCommandEncoder{ true };
    if (mCommandEncoder) {
        // make sure command elements (draws, etc.) prior to this blit are processed before the blit
        // on the GPU
        flush();
    } else {
        // a command encoder is not in flight, so create one for this blit itself
        reusedCommandEncoder = false;
        const wgpu::CommandEncoderDescriptor commandEncoderDescriptor{
            .label = "blit_command",
        };
        mCommandEncoder = mDevice.CreateCommandEncoder(&commandEncoderDescriptor);
        FILAMENT_CHECK_POSTCONDITION(mCommandEncoder)
                << "Failed to create command encoder for blit?";
    }
    const auto sourceTexture{ handleCast<WebGPUTexture>(sourceTextureHandle) };
    const auto destinationTexture{ handleCast<WebGPUTexture>(destinationTextureHandle) };
    const WebGPUBlitter::BlitArgs blitArgs{
        .source = {
            .texture = sourceTexture->getTexture(),
            .aspect = sourceTexture->getAspect(),
            .origin = {.x = sourceOrigin.x, .y=sourceOrigin.y},
            .extent = {.width=size.x, .height =size.y},
            .mipLevel = sourceLevel,
            .layerOrDepth = sourceLayer,
        },
        .destination = {
            .texture = destinationTexture->getTexture(),
            .aspect = destinationTexture->getAspect(),
            .origin = {.x = destinationOrigin.x, .y=destinationOrigin.y},
            .extent = {.width=size.x, .height =size.y},
            .mipLevel = destinationLevel,
            .layerOrDepth = destinationLayer,
        },
        .filter = SamplerMagFilter::NEAREST,
    };
    mBlitter.blit(mQueue, mCommandEncoder, blitArgs);
    if (!reusedCommandEncoder) {
        // TODO consider removing encoder finish + submit once we have test coverage and can
        //      safely verify doing so will not result in strange errors. We don't think this
        //      submit is necessary, as the encoder should get finished and the command submitted in
        //      commit(), finish(), and/or flush() in this frame or the next anyway.
        //      And, the blit bits will be encoded before other things.
        //      Thus, not submitting here _seems_ ok. Nonetheless, doing the submit here likely
        //      represents unnecessary inefficiency and higher bandwidth
        //      between the CPU and GPU, but we are worried about functional correctness at this
        //      stage.
        const wgpu::CommandBufferDescriptor commandBufferDescriptor{
            .label = "blit_command_buffer",
        };
        const wgpu::CommandBuffer blitCommand{ mCommandEncoder.Finish(&commandBufferDescriptor) };
        FILAMENT_CHECK_POSTCONDITION(blitCommand) << "Failed to create command buffer for blit?";
        mQueue.Submit(1, &blitCommand);
        mCommandEncoder = nullptr;
    }
}

void WebGPUDriver::bindPipeline(PipelineState const& pipelineState) {
    assert_invariant(mRenderPassEncoder);
    const auto program{ handleCast<WebGPUProgram>(pipelineState.program) };
    assert_invariant(program);
    WebGPURenderTarget const* renderTarget{ mCurrentRenderTarget };
    assert_invariant(renderTarget);
    assert_invariant(program->computeShaderModule == nullptr &&
                     "WebGPU backend does not (yet) support compute pipelines.");
    FILAMENT_CHECK_POSTCONDITION(program->vertexShaderModule)
            << "WebGPU backend requires a vertex shader module for a render pipeline";
    const auto vertexBufferInfo{ handleCast<WebGPUVertexBufferInfo>(
            pipelineState.vertexBufferInfo) };
    assert_invariant(vertexBufferInfo);
    std::array<wgpu::BindGroupLayout, MAX_DESCRIPTOR_SET_COUNT> bindGroupLayouts{};
    assert_invariant(bindGroupLayouts.size() >= pipelineState.pipelineLayout.setLayout.size());
    size_t bindGroupLayoutCount{ 0 };
    for (size_t i{ 0 }; i < bindGroupLayouts.size(); i++) {
        const auto handle{ pipelineState.pipelineLayout.setLayout[i] };
        if (!handle) {
            continue;
        }
        bindGroupLayouts[bindGroupLayoutCount++] =
                handleCast<WebGPUDescriptorSetLayout>(handle)->getLayout();
    }
    const WebGPUPipelineLayoutCache::PipelineLayoutRequest pipelineLayoutRequest{
        .label = program->name,
        .bindGroupLayouts = bindGroupLayouts,
        .bindGroupLayoutCount = bindGroupLayoutCount,
    };
    wgpu::PipelineLayout const& layout{ mPipelineLayoutCache.getOrCreatePipelineLayout(
            pipelineLayoutRequest) };
    uint8_t colorFormatCount{ 0 };
    std::array<wgpu::TextureFormat, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT> colorFormats{
        wgpu::TextureFormat::Undefined
    };
    wgpu::TextureFormat depthStencilFormat{ wgpu::TextureFormat::Undefined };
    if (renderTarget->isDefaultRenderTarget()) {
        colorFormatCount = 1;
        colorFormats[0] = mSwapChain->getColorFormat();
        depthStencilFormat = mSwapChain->getDepthFormat();
    } else {
        MRT const& mrtColorAttachments{ mCurrentRenderTarget->getColorAttachmentInfos() };
        for (size_t i{ 0 }; i < MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT; ++i) {
            if (mrtColorAttachments[i].handle) {
                const auto colorTexture{ handleCast<WebGPUTexture>(mrtColorAttachments[i].handle) };
                if (colorTexture) {
                    colorFormats[colorFormatCount++] = colorTexture->getTexture().GetFormat();
                }
            }
        }
        // custom render target depth/stencil...
        const auto& depthInfo = mCurrentRenderTarget->getDepthAttachmentInfo();
        const auto& stencilInfo = mCurrentRenderTarget->getStencilAttachmentInfo();
        Handle<HwTexture> depthStencilHandle{};
        if (depthInfo.handle) {
            depthStencilHandle = depthInfo.handle;
        } else if (stencilInfo.handle) {
            depthStencilHandle = stencilInfo.handle;
        }
        if (depthStencilHandle) {
            const auto depthStencilTexture{ handleCast<WebGPUTexture>(depthStencilHandle) };
            if (depthStencilTexture) {
                depthStencilFormat = depthStencilTexture->getTexture().GetFormat();
            }
        }
    }
    const WebGPUPipelineCache::RenderPipelineRequest pipelineRequest{
        .label = program->name,
        .vertexShaderModule = program->vertexShaderModule,
        .fragmentShaderModule = program->fragmentShaderModule,
        .vertexBufferSlots = vertexBufferInfo->getWebGPUSlotBindingInfos(),
        .vertexBufferLayouts = vertexBufferInfo->getVertexBufferLayouts(),
        .pipelineLayout = layout,
        .primitiveType = pipelineState.primitiveType,
        .rasterState = pipelineState.rasterState,
        .stencilState = pipelineState.stencilState,
        .polygonOffset = pipelineState.polygonOffset,
        .targetRenderFlags = renderTarget->getTargetFlags(),
        .multisampleCount = renderTarget->getSamples(),
        .depthStencilFormat = depthStencilFormat,
        .colorFormatCount = colorFormatCount,
        .colorFormats = colorFormats.data(),
    };
    wgpu::RenderPipeline const& pipeline{ mPipelineCache.getOrCreateRenderPipeline(
            pipelineRequest) };
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
    // Bind groups are deferred until the actual draw call.
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
        // Dawn will cache duplicate samplers, so we don't strictly need to maintain a cache here.
        const auto sampler = makeSampler(params);
        wgpu::BindGroupEntry tEntry{
            .binding = static_cast<uint32_t>(binding * 2),
            .textureView = texture->getDefaultTextureView() };
        bindGroup->addEntry(tEntry.binding, std::move(tEntry));

        wgpu::BindGroupEntry sEntry{
            .binding = static_cast<uint32_t>(binding * 2 + 1),
            .sampler = sampler };
        bindGroup->addEntry(sEntry.binding, std::move(sEntry));
#if FWGPU_ENABLED(FWGPU_DEBUG_BIND_GROUPS)
        FWGPU_LOGD << "updateDescriptorSetTexture:";
        FWGPU_LOGD << "  wgpu::BindGroupLayout handle: " << bindGroup->getLayout().Get();
        FWGPU_LOGD << "  texture: binding:" << tEntry.binding
                   << " wgpu handle:" << texture->getTexture().Get()
                   << " " << webGPUTextureToString(texture->getTexture())
                   << " " << webGPUPrintableToString(texture->getAspect());
        FWGPU_LOGD << "  sampler: binding:" << sEntry.binding
                   << " filterMag:" << filamentSamplerMagFilterToString(params.filterMag)
                   << " filterMin:" << filamentSamplerMinFilterToString(params.filterMin)
                   << " wrapS:" << filamentSamplerWrapModeToString(params.wrapS)
                   << " wrapT:" << filamentSamplerWrapModeToString(params.wrapT)
                   << " wrapR:" << filamentSamplerWrapModeToString(params.wrapR)
                   << " anisotropyLog2:" << +params.anisotropyLog2
                   << " compareMode:" << filamentSamplerCompareModeToString(params.compareMode)
                   << " compareFunc:" << filamentSamplerCompareFuncToString(params.compareFunc);
#endif
    }
}

void WebGPUDriver::bindDescriptorSet(Handle<HwDescriptorSet> descriptorSetHandle,
        const backend::descriptor_set_t setIndex, backend::DescriptorSetOffsetArray&& offsets) {
    assert_invariant(setIndex < MAX_DESCRIPTOR_SET_COUNT);

    // An empty handle unbinds the descriptor set.
    if (!descriptorSetHandle) {
        mCurrentDescriptorSets[setIndex] = {};
        return;
    }
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

} // namespace filament::backend
