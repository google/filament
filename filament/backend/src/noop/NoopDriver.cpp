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

namespace filament {

using namespace backend;

Driver* NoopDriver::create() {
    return new NoopDriver();
}

NoopDriver::NoopDriver() noexcept : DriverBase(new ConcreteDispatcher<NoopDriver>()) {
}

NoopDriver::~NoopDriver() noexcept = default;

backend::ShaderModel NoopDriver::getShaderModel() const noexcept {
#if defined(ANDROID) || defined(IOS)
    return ShaderModel::GL_ES_30;
#else
    return ShaderModel::GL_CORE_41;
#endif
}

// explicit instantiation of the Dispatcher
template class backend::ConcreteDispatcher<NoopDriver>;


void NoopDriver::terminate() {
}

void NoopDriver::tick(int) {
}

void NoopDriver::beginFrame(int64_t monotonic_clock_ns, uint32_t frameId,
        backend::FrameFinishedCallback, void*) {
}

void NoopDriver::setPresentationTime(int64_t monotonic_clock_ns) {
}

void NoopDriver::endFrame(uint32_t frameId) {
}

void NoopDriver::flush(int) {
}

void NoopDriver::finish(int) {
}

void NoopDriver::destroyUniformBuffer(Handle<HwUniformBuffer> ubh) {
}

void NoopDriver::destroyRenderPrimitive(Handle<HwRenderPrimitive> rph) {
}

void NoopDriver::destroyVertexBuffer(Handle<HwVertexBuffer> vbh) {
}

void NoopDriver::destroyIndexBuffer(Handle<HwIndexBuffer> ibh) {
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

void NoopDriver::destroySync(Handle<HwSync> fh) {
}

Handle<HwStream> NoopDriver::createStreamNative(void* nativeStream) {
    return {};
}

Handle<HwStream> NoopDriver::createStreamAcquired() {
    return {};
}

void NoopDriver::setAcquiredImage(Handle<HwStream> sh, void* image, backend::StreamCallback cb,
        void* userData) {
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

FenceStatus NoopDriver::wait(Handle<HwFence> fh, uint64_t timeout) {
    return FenceStatus::CONDITION_SATISFIED;
}

// We create all textures using VK_IMAGE_TILING_OPTIMAL, so our definition of "supported" is that
// the GPU supports the given texture format with non-zero optimal tiling features.
bool NoopDriver::isTextureFormatSupported(TextureFormat format) {
    return true;
}

bool NoopDriver::isTextureFormatMipmappable(backend::TextureFormat format) {
    return true;
}

bool NoopDriver::isRenderTargetFormatSupported(TextureFormat format) {
    return true;
}

bool NoopDriver::isFrameTimeSupported() {
    return true;
}

void NoopDriver::updateVertexBuffer(Handle<HwVertexBuffer> vbh, size_t index,
        BufferDescriptor&& p, uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::updateIndexBuffer(Handle<HwIndexBuffer> ibh, BufferDescriptor&& p,
        uint32_t byteOffset) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::update2DImage(Handle<HwTexture> th,
        uint32_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& data) {
    scheduleDestroy(std::move(data));
}

void NoopDriver::updateCubeImage(Handle<HwTexture> th, uint32_t level,
        PixelBufferDescriptor&& data, FaceOffsets faceOffsets) {
    scheduleDestroy(std::move(data));
}

void NoopDriver::setupExternalImage(void* image) {
}

void NoopDriver::cancelExternalImage(void* image) {
}

bool NoopDriver::getTimerQueryValue(Handle<HwTimerQuery> tqh, uint64_t* elapsedTime) {
    return false;
}

SyncStatus NoopDriver::getSyncStatus(Handle<HwSync> sh) {
    return SyncStatus::SIGNALED;
}

void NoopDriver::setExternalImage(Handle<HwTexture> th, void* image) {
}

void NoopDriver::setExternalImagePlane(Handle<HwTexture> th, void* image, size_t plane) {
}

void NoopDriver::setExternalStream(Handle<HwTexture> th, Handle<HwStream> sh) {
}

void NoopDriver::generateMipmaps(Handle<HwTexture> th) { }

bool NoopDriver::canGenerateMipmaps() {
    return true;
}

void NoopDriver::loadUniformBuffer(Handle<HwUniformBuffer> ubh, BufferDescriptor&& data) {
    scheduleDestroy(std::move(data));
}

void NoopDriver::updateSamplerGroup(Handle<HwSamplerGroup> sbh,
        SamplerGroup&& samplerGroup) {
}

void NoopDriver::beginRenderPass(Handle<HwRenderTarget> rth, const RenderPassParams& params) {
}

void NoopDriver::endRenderPass(int) {
}

void NoopDriver::setRenderPrimitiveBuffer(Handle<HwRenderPrimitive> rph,
        Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh,
        uint32_t enabledAttributes) {
}

void NoopDriver::setRenderPrimitiveRange(Handle<HwRenderPrimitive> rph,
        PrimitiveType pt, uint32_t offset,
        uint32_t minIndex, uint32_t maxIndex, uint32_t count) {
}

void NoopDriver::makeCurrent(Handle<HwSwapChain> drawSch, Handle<HwSwapChain> readSch) {
}

void NoopDriver::commit(Handle<HwSwapChain> sch) {
}

void NoopDriver::bindUniformBuffer(size_t index, Handle<HwUniformBuffer> ubh) {
}

void NoopDriver::bindUniformBufferRange(size_t index, Handle<HwUniformBuffer> ubh,
        size_t offset, size_t size) {
}

void NoopDriver::bindSamplers(size_t index, Handle<HwSamplerGroup> sbh) {
}

void NoopDriver::insertEventMarker(char const* string, size_t len) {
}

void NoopDriver::pushGroupMarker(char const* string,  size_t len) {
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

void NoopDriver::readStreamPixels(Handle<HwStream> sh, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height, PixelBufferDescriptor&& p) {
    scheduleDestroy(std::move(p));
}

void NoopDriver::blit(TargetBufferFlags buffers,
        Handle<HwRenderTarget> dst, backend::Viewport dstRect,
        Handle<HwRenderTarget> src, backend::Viewport srcRect,
        SamplerMagFilter filter) {
}

void NoopDriver::draw(PipelineState pipelineState, Handle<HwRenderPrimitive> rph) {
}

void NoopDriver::beginTimerQuery(Handle<HwTimerQuery> tqh) {
}

void NoopDriver::endTimerQuery(Handle<HwTimerQuery> tqh) {
}

} // namespace filament
