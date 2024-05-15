/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "noop/NoopDriver.h"
#include "CommandStreamDispatcher.h"

namespace filament::backend {

Driver* NoopDriver::create() {
    return new NoopDriver();
}

NoopDriver::NoopDriver() noexcept = default;

NoopDriver::~NoopDriver() noexcept = default;

Dispatcher NoopDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<NoopDriver>::make();
}

ShaderModel NoopDriver::getShaderModel() const noexcept {
#if defined(__ANDROID__) || defined(IOS) || defined(__EMSCRIPTEN__)
    return ShaderModel::MOBILE;
#else
    return ShaderModel::DESKTOP;
#endif
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<NoopDriver>;


void NoopDriver::terminate() {
}

void NoopDriver::tick(int) {
}

void NoopDriver::beginFrame(int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs, uint32_t frameId) {
}

void NoopDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, FrameScheduledCallback&& callback) {

}

void NoopDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, utils::Invocable<void(void)>&& callback) {

}

void NoopDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void NoopDriver::endFrame(uint32_t frameId) {
}

void NoopDriver::flush(int) {
}

void NoopDriver::finish(int) {
}

void NoopDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
}

void NoopDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
}

void NoopDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
}

void NoopDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
}

void NoopDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
}

void NoopDriver::destroyTexture(Handle<HwTexture> th) {
}

void NoopDriver::destroyProgram(Handle<HwProgram> ph) {
}

void NoopDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
}

void NoopDriver::destroySamplerGroup(Handle<HwSamplerGroup> sbh) {
}

void NoopDriver::destroySwapChain(Handle<HwSwapChain> sch) {
}

void NoopDriver::destroyStream(Handle<HwStream> sh) {
}

void NoopDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
}

Handle<HwStream> NoopDriver::createStreamNative(void* nativeStream) {
    return {};
}

Handle<HwStream> NoopDriver::createStreamAcquired() {
    return {};
}

void NoopDriver::setAcquiredImage(Handle<HwStream> sh, void* image,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
}

void NoopDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
}

int64_t NoopDriver::getStreamTimestamp(Handle<HwStream> sh) {
    return 0;
}

void NoopDriver::updateStreams(CommandStream* driver) {
}

void NoopDriver::destroyFence(Handle<HwFence> fh) {
}

FenceStatus NoopDriver::getFenceStatus(Handle<HwFence> fh) {
    return FenceStatus::CONDITION_SATISFIED;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool NoopDriver::isTextureFormatSupported(TextureFormat format) {
    return true;
}

bool NoopDriver::isTextureSwizzleSupported() {
    return true;
}

bool NoopDriver::isTextureFormatMipmappable(TextureFormat format) {
    return true;
}

bool NoopDriver::isRenderTargetFormatSupported(TextureFormat format) {
    return true;
}

bool NoopDriver::isFrameBufferFetchSupported() {
    return false;
}

bool NoopDriver::isFrameBufferFetchMultiSampleSupported() {
    return false; // TODO: add support for MS framebuffer_fetch
}

bool NoopDriver::isFrameTimeSupported() {
    return true;
}

bool NoopDriver::isAutoDepthResolveSupported() {
    return true;
}

bool NoopDriver::isSRGBSwapChainSupported() {
    return false;
}

bool NoopDriver::isProtectedContentSupported() {
    return false;
}

bool NoopDriver::isStereoSupported(backend::StereoscopicType) {
    return false;
}

bool NoopDriver::isParallelShaderCompileSupported() {
    return false;
}

bool NoopDriver::isDepthStencilResolveSupported() {
    return true;
}

bool NoopDriver::isDepthStencilBlitSupported(TextureFormat format) {
    return true;
}

bool NoopDriver::isProtectedTexturesSupported() {
    return true;
}

bool NoopDriver::isWorkaroundNeeded(Workaround) {
    return false;
}

FeatureLevel NoopDriver::getFeatureLevel() {
    return FeatureLevel::FEATURE_LEVEL_1;
}

math::float2 NoopDriver::getClipSpaceParams() {
    return math::float2{ 1.0f, 0.0f };
}

uint8_t NoopDriver::getMaxDrawBuffers() {
    return MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;
}

size_t NoopDriver::getMaxUniformBufferSize() {
    return 16384u;
}

void NoopDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::updateBufferObject(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::resetBufferObject(Handle<HwBufferObject> boh) {
}

void NoopDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
}

void NoopDriver::setMinMaxLevels(Handle<HwTexture> th, uint32_t minLevel, uint32_t maxLevel) {
}

void NoopDriver::update3DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    scheduleDestroy(std::move(data));
}

void NoopDriver::setupExternalImage(void* image) {
}

TimerQueryResult NoopDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    return TimerQueryResult::ERROR;
}

void NoopDriver::setExternalImage(Handle<HwTexture> th, void* image) {
}

void NoopDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
}

void NoopDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

void NoopDriver::generateMipmaps(Handle<HwTexture> th) { }

void NoopDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        BufferDescriptor&& data) {
    scheduleDestroy(std::move(data));
}

void NoopDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        scheduleCallback(handler, user, callback);
    }
}

void NoopDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
}

void NoopDriver::endRenderPass(int) {
}

void NoopDriver::nextSubpass(int) {
}

void NoopDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
}

void NoopDriver::commit(Handle<HwSwapChain> sch) {
}

void NoopDriver::bindUniformBuffer(uint32_t index, Handle<HwBufferObject> ubh) {
}

void NoopDriver::bindBufferRange(BufferObjectBinding bindingType, uint32_t index,
        Handle<HwBufferObject> ubh, uint32_t offset, uint32_t size) {
}

void NoopDriver::unbindBuffer(BufferObjectBinding bindingType, uint32_t index) {
}

void NoopDriver::bindSamplers(uint32_t index, Handle<HwSamplerGroup> sbh) {
}

void NoopDriver::setPushConstant(backend::ShaderStage stage, uint8_t index,
        backend::PushConstantVariant value) {
}

void NoopDriver::insertEventMarker(char const* string, uint32_t len) {
}

void NoopDriver::pushGroupMarker(char const* string,  uint32_t len) {
}

void NoopDriver::popGroupMarker(int) {
}

void NoopDriver::startCapture(int) {
}

void NoopDriver::stopCapture(int) {
}

void NoopDriver::readPixels(Handle<HwRenderTarget> src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::readBufferSubData(backend::BufferObjectHandle boh,
        uint32_t offset, uint32_t size, backend::BufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
}

void NoopDriver::resolve(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer) {
}

void NoopDriver::blit(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer, math::uint2 dstOrigin,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer, math::uint2 srcOrigin,
        math::uint2 size) {
}

void NoopDriver::bindPipeline(PipelineState const& pipelineState) {
}

void NoopDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
}

void NoopDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
}

void NoopDriver::draw(PipelineState pipelineState, Handle<HwRenderPrimitive> rph,
        uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
}

void NoopDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
}

void NoopDriver::scissor(
        Viewport scissor) {
}

void NoopDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
}

void NoopDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
}

void NoopDriver::resetState(int) {
}

} // namespace filament
