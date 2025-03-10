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

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include "webgpu/WebGPUDriver.h"
#include "CommandStreamDispatcher.h"

#include <stdint.h>

namespace filament::backend {

Driver* WebGPUDriver::create() {
    return new WebGPUDriver();
}

WebGPUDriver::WebGPUDriver() noexcept = default;

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
    return ShaderLanguage::ESSL3;
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
