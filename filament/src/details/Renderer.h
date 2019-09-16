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

#ifndef TNT_FILAMENT_DETAILS_RENDERER_H
#define TNT_FILAMENT_DETAILS_RENDERER_H

#include "upcast.h"

#include "FrameInfo.h"
#include "RenderPass.h"

#include "details/Allocators.h"
#include "details/FrameSkipper.h"
#include "details/SwapChain.h"

#include "private/backend/DriverApiForward.h"

#include <filament/Renderer.h>
#include <filament/View.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/JobSystem.h>
#include <utils/Slice.h>

namespace filament {

namespace backend {
class Driver;
} // namespace backend

class View;

namespace details {

class FEngine;
class FView;
class ShadowMap;

/*
 * A concrete implementation of the Renderer Interface.
 */
class FRenderer : public Renderer {
public:
    explicit FRenderer(FEngine& engine);
    ~FRenderer() noexcept;

    void init() noexcept;

    FEngine& getEngine() const noexcept { return mEngine; }

    math::float4 getShaderUserTime() const { return mShaderUserTime; }

    // do all the work here!
    void render(FView const* view);
    void renderJob(ArenaScope& arena, FView& view);

    void copyFrame(FSwapChain* dstSwapChain, Viewport const& dstViewport,
            Viewport const& srcViewport, CopyFrameFlag flags);

    bool beginFrame(FSwapChain* swapChain);
    void endFrame();

    void resetUserTime();

    void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    // Clean-up everything, this is typically called when the client calls Engine::destroyRenderer()
    void terminate(FEngine& engine);

private:
    friend class Renderer;
    using Command = RenderPass::Command;

    backend::Handle<backend::HwRenderTarget> getRenderTarget(FView& view) const noexcept;

    RenderPass::CommandTypeFlags getCommandType(View::DepthPrepass prepass) const noexcept;

    void recordHighWatermark(size_t watermark) noexcept {
        mCommandsHighWatermark = std::max(mCommandsHighWatermark, watermark);
    }

    size_t getCommandsHighWatermark() const noexcept {
        return mCommandsHighWatermark * sizeof(RenderPass::Command);
    }

    backend::TextureFormat getHdrFormat(const View& view) const noexcept;
    backend::TextureFormat getLdrFormat() const noexcept;

    using clock = std::chrono::steady_clock;
    using Epoch = clock::time_point;
    using duration = clock::duration;

    Epoch getUserEpoch() const { return mUserEpoch; }
    duration getUserTime() const noexcept {
        return clock::now() - getUserEpoch();
    }

    // keep a reference to our engine
    FEngine& mEngine;
    FrameSkipper mFrameSkipper;
    backend::Handle<backend::HwRenderTarget> mRenderTarget;
    FSwapChain* mSwapChain = nullptr;
    size_t mCommandsHighWatermark = 0;
    uint32_t mFrameId = 0;
    FrameInfoManager mFrameInfoManager;
    backend::TextureFormat mHdrTranslucent{};
    backend::TextureFormat mHdrQualityMedium{};
    backend::TextureFormat mHdrQualityHigh{};
    bool mIsRGB8Supported : 1;
    Epoch mUserEpoch;
    math::float4 mShaderUserTime{};

    // per-frame arena for this Renderer
    LinearAllocatorArena& mPerRenderPassArena;

#if EXTRA_TIMING_INFO
    Series<float> mRendering;
    Series<float> mPostProcess;
#endif
};

FILAMENT_UPCAST(Renderer)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RENDERER_H
