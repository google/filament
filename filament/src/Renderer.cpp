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

#include <filament/Renderer.h>

#include "ResourceAllocator.h"

#include "details/Engine.h"
#include "details/Renderer.h"
#include "details/View.h"

#include <utils/FixedCapacityVector.h>

#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament {

using namespace math;
using namespace backend;

Engine* Renderer::getEngine() noexcept {
    return &downcast(this)->getEngine();
}

void Renderer::render(View const* view) {
    downcast(this)->render(downcast(view));
}

void Renderer::setPresentationTime(int64_t const monotonic_clock_ns) {
    downcast(this)->setPresentationTime(monotonic_clock_ns);
}

void Renderer::skipFrame(uint64_t const vsyncSteadyClockTimeNano) {
    downcast(this)->skipFrame(vsyncSteadyClockTimeNano);
}

bool Renderer::beginFrame(SwapChain* swapChain, uint64_t const vsyncSteadyClockTimeNano) {
    return downcast(this)->beginFrame(downcast(swapChain), vsyncSteadyClockTimeNano);
}

void Renderer::copyFrame(SwapChain* dstSwapChain, filament::Viewport const& dstViewport,
        filament::Viewport const& srcViewport, CopyFrameFlag const flags) {
    downcast(this)->copyFrame(downcast(dstSwapChain), dstViewport, srcViewport, flags);
}

void Renderer::readPixels(uint32_t const xoffset, uint32_t const yoffset, uint32_t const width, uint32_t const height,
        PixelBufferDescriptor&& buffer) {
    downcast(this)->readPixels(xoffset, yoffset, width, height, std::move(buffer));
}

void Renderer::readPixels(RenderTarget* renderTarget,
        uint32_t const xoffset, uint32_t const yoffset, uint32_t const width, uint32_t const height,
        PixelBufferDescriptor&& buffer) {
    downcast(this)->readPixels(downcast(renderTarget),
            xoffset, yoffset, width, height, std::move(buffer));
}

void Renderer::endFrame() {
    downcast(this)->endFrame();
}

double Renderer::getUserTime() const {
    return downcast(this)->getUserTime();
}

void Renderer::resetUserTime() {
    downcast(this)->resetUserTime();
}

void Renderer::setDisplayInfo(const DisplayInfo& info) noexcept {
    downcast(this)->setDisplayInfo(info);
}

void Renderer::setFrameRateOptions(FrameRateOptions const& options) noexcept {
    downcast(this)->setFrameRateOptions(options);
}

void Renderer::setClearOptions(const ClearOptions& options) {
    downcast(this)->setClearOptions(options);
}

Renderer::ClearOptions const& Renderer::getClearOptions() const noexcept {
    return downcast(this)->getClearOptions();
}

void Renderer::renderStandaloneView(View const* view) {
    downcast(this)->renderStandaloneView(downcast(view));
}

void Renderer::setVsyncTime(uint64_t const steadyClockTimeNano) noexcept {
    downcast(this)->setVsyncTime(steadyClockTimeNano);
}

utils::FixedCapacityVector<Renderer::FrameInfo> Renderer::getFrameInfoHistory(size_t const historySize) const noexcept {
    return downcast(this)->getFrameInfoHistory(historySize);
}

size_t Renderer::getMaxFrameHistorySize() const noexcept {
    return downcast(this)->getMaxFrameHistorySize();
}

} // namespace filament
