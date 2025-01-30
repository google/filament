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

#include "dawn/DawnDriver.h"
#include "CommandStreamDispatcher.h"

namespace filament::backend {

Driver* DawnDriver::create() {
    return new DawnDriver();
}

DawnDriver::DawnDriver() noexcept = default;

DawnDriver::~DawnDriver() noexcept = default;

Dispatcher DawnDriver::getDispatcher() const noexcept {
    return ConcreteDispatcher<DawnDriver>::make();
}

ShaderModel DawnDriver::getShaderModel() const noexcept {
#if defined(__ANDROID__) || defined(IOS) || defined(__EMSCRIPTEN__)
    return ShaderModel::MOBILE;
#else
    return ShaderModel::DESKTOP;
#endif
}

// explicit instantiation of the Dispatcher
template class ConcreteDispatcher<DawnDriver>;


void DawnDriver::terminate() {
}

void DawnDriver::tick(int) {
}

void DawnDriver::beginFrame(int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs, uint32_t frameId) {
}

void DawnDriver::setFrameScheduledCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, FrameScheduledCallback&& callback, uint64_t flags) {

}

void DawnDriver::setFrameCompletedCallback(Handle<HwSwapChain> sch,
        CallbackHandler* handler, utils::Invocable<void(void)>&& callback) {

}

void DawnDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void DawnDriver::endFrame(uint32_t frameId) {
}

void DawnDriver::flush(int) {
}

void DawnDriver::finish(int) {
}

void DawnDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
}

void DawnDriver::destroyVertexBufferInfo(Handle<HwVertexBufferInfo> vbih) {
}

void DawnDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
}

void DawnDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
}

void DawnDriver::destroyBufferObject(Handle<HwBufferObject> boh) {
}

void DawnDriver::destroyTexture(Handle<HwTexture> th) {
}

void DawnDriver::destroyProgram(Handle<HwProgram> ph) {
}

void DawnDriver::destroyRenderTarget(Handle<HwRenderTarget> rth) {
}

void DawnDriver::destroySwapChain(Handle<HwSwapChain> sch) {
}

void DawnDriver::destroyStream(Handle<HwStream> sh) {
}

void DawnDriver::destroyTimerQuery(Handle<HwTimerQuery> tqh) {
}

void DawnDriver::destroyDescriptorSetLayout(Handle<HwDescriptorSetLayout> tqh) {
}

void DawnDriver::destroyDescriptorSet(Handle<HwDescriptorSet> tqh) {
}

Handle<HwStream> DawnDriver::createStreamNative(void* nativeStream) {
    return {};
}

Handle<HwStream> DawnDriver::createStreamAcquired() {
    return {};
}

void DawnDriver::setAcquiredImage(Handle<HwStream> sh, void* image,
        CallbackHandler* handler, StreamCallback cb, void* userData) {
}

void DawnDriver::setStreamDimensions(Handle<HwStream> sh, uint32_t width, uint32_t height) {
}

int64_t DawnDriver::getStreamTimestamp(Handle<HwStream> sh) {
    return 0;
}

void DawnDriver::updateStreams(CommandStream* driver) {
}

void DawnDriver::destroyFence(Handle<HwFence> fh) {
}

FenceStatus DawnDriver::getFenceStatus(Handle<HwFence> fh) {
    return FenceStatus::CONDITION_SATISFIED;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool DawnDriver::isTextureFormatSupported(TextureFormat format) {
    return true;
}

bool DawnDriver::isTextureSwizzleSupported() {
    return true;
}

bool DawnDriver::isTextureFormatMipmappable(TextureFormat format) {
    return true;
}

bool DawnDriver::isRenderTargetFormatSupported(TextureFormat format) {
    return true;
}

bool DawnDriver::isFrameBufferFetchSupported() {
    return false;
}

bool DawnDriver::isFrameBufferFetchMultiSampleSupported() {
    return false; // TODO: add support for MS framebuffer_fetch
}

bool DawnDriver::isFrameTimeSupported() {
    return true;
}

bool DawnDriver::isAutoDepthResolveSupported() {
    return true;
}

bool DawnDriver::isSRGBSwapChainSupported() {
    return false;
}

bool DawnDriver::isProtectedContentSupported() {
    return false;
}

bool DawnDriver::isStereoSupported() {
    return false;
}

bool DawnDriver::isParallelShaderCompileSupported() {
    return false;
}

bool DawnDriver::isDepthStencilResolveSupported() {
    return true;
}

bool DawnDriver::isDepthStencilBlitSupported(TextureFormat format) {
    return true;
}

bool DawnDriver::isProtectedTexturesSupported() {
    return true;
}

bool DawnDriver::isDepthClampSupported() {
    return false;
}

bool DawnDriver::isWorkaroundNeeded(Workaround) {
    return false;
}

FeatureLevel DawnDriver::getFeatureLevel() {
    return FeatureLevel::FEATURE_LEVEL_1;
}

math::float2 DawnDriver::getClipSpaceParams() {
    return math::float2{ 1.0f, 0.0f };
}

uint8_t DawnDriver::getMaxDrawBuffers() {
    return MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;
}

size_t DawnDriver::getMaxUniformBufferSize() {
    return 16384u;
}

void DawnDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void DawnDriver::updateBufferObject(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void DawnDriver::updateBufferObjectUnsynchronized(Handle<HwBufferObject> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void DawnDriver::resetBufferObject(Handle<HwBufferObject> boh) {
}

void DawnDriver::setVertexBufferObject(Handle<HwVertexBuffer> vbh, uint32_t index,
        Handle<HwBufferObject> boh) {
}

void DawnDriver::update3DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& data) {
    scheduleDestroy(std::move(data));
}

void DawnDriver::setupExternalImage(void* image) {
}

TimerQueryResult DawnDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    return TimerQueryResult::ERROR;
}

void DawnDriver::setExternalImage(Handle<HwTexture> th, void* image) {
}

void DawnDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, uint32_t plane) {
}

void DawnDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

void DawnDriver::generateMipmaps(Handle<HwTexture> th) { }

void DawnDriver::compilePrograms(CompilerPriorityQueue priority,
        CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
    if (callback) {
        scheduleCallback(handler, user, callback);
    }
}

void DawnDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
}

void DawnDriver::endRenderPass(int) {
}

void DawnDriver::nextSubpass(int) {
}

void DawnDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
}

void DawnDriver::commit(Handle<HwSwapChain> sch) {
}

void DawnDriver::setPushConstant(backend::ShaderStage stage, uint8_t index,
        backend::PushConstantVariant value) {
}

void DawnDriver::insertEventMarker(char const* string) {
}

void DawnDriver::pushGroupMarker(char const* string) {
}

void DawnDriver::popGroupMarker(int) {
}

void DawnDriver::startCapture(int) {
}

void DawnDriver::stopCapture(int) {
}

void DawnDriver::readPixels(Handle<HwRenderTarget> src,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void DawnDriver::readBufferSubData(backend::BufferObjectHandle boh,
        uint32_t offset, uint32_t size, backend::BufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void DawnDriver::blitDEPRECATED(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, Viewport dstRect,
        Handle<HwRenderTarget> src, Viewport srcRect,
        SamplerMagFilter filter) {
}

void DawnDriver::resolve(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer) {
}

void DawnDriver::blit(
        Handle<HwTexture> dst, uint8_t srcLevel, uint8_t srcLayer, math::uint2 dstOrigin,
        Handle<HwTexture> src, uint8_t dstLevel, uint8_t dstLayer, math::uint2 srcOrigin,
        math::uint2 size) {
}

void DawnDriver::bindPipeline(PipelineState const& pipelineState) {
}

void DawnDriver::bindRenderPrimitive(Handle<HwRenderPrimitive> rph) {
}

void DawnDriver::draw2(uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
}

void DawnDriver::draw(PipelineState pipelineState, Handle<HwRenderPrimitive> rph,
        uint32_t indexOffset, uint32_t indexCount, uint32_t instanceCount) {
}

void DawnDriver::dispatchCompute(Handle<HwProgram> program, math::uint3 workGroupCount) {
}

void DawnDriver::scissor(
        Viewport scissor) {
}

void DawnDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
}

void DawnDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
}

void DawnDriver::resetState(int) {
}

void DawnDriver::updateDescriptorSetBuffer(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::BufferObjectHandle boh,
        uint32_t offset,
        uint32_t size) {
}

void DawnDriver::updateDescriptorSetTexture(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_binding_t binding,
        backend::TextureHandle th,
        SamplerParams params) {
}

void DawnDriver::bindDescriptorSet(
        backend::DescriptorSetHandle dsh,
        backend::descriptor_set_t set,
        backend::DescriptorSetOffsetArray&& offsets) {
}

void DawnDriver::setDebugTag(HandleBase::HandleId handleId, utils::CString tag) {
}

} // namespace filament
