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
#include <backend/PresentCallable.h>

#include <utils/compiler.h>
#include <utils/Allocator.h>
#include <utils/JobSystem.h>
#include <utils/Slice.h>

namespace filament {

namespace backend {
class Driver;
} // namespace backend

class View;

class FEngine;
class FView;
class ShadowMap;

/*
 * A concrete implementation of the Renderer Interface.
 */
class FRenderer : public Renderer {
    static constexpr size_t MAX_FRAMETIME_HISTORY = 32u;

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

    bool beginFrame(FSwapChain* swapChain, uint64_t vsyncSteadyClockTimeNano,
            backend::FrameFinishedCallback callback, void* user);
    void endFrame();

    void resetUserTime();

    void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    void readPixels(FRenderTarget* renderTarget,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    // Clean-up everything, this is typically called when the client calls Engine::destroyRenderer()
    void terminate(FEngine& engine);

    void setDisplayInfo(DisplayInfo const& info) noexcept {
        mDisplayInfo = info;
    }

    void setFrameRateOptions(FrameRateOptions const& options) noexcept {
        FrameRateOptions& frameRateOptions = mFrameRateOptions;
        frameRateOptions = options;

        // History can't be more than 32 frames (~0.5s)
        frameRateOptions.history = std::min(frameRateOptions.history,
                uint8_t(MAX_FRAMETIME_HISTORY));

        // History must at least be 3 frames
        frameRateOptions.history = std::max(frameRateOptions.history, uint8_t(3));

        frameRateOptions.interval = std::max(uint8_t(1), frameRateOptions.interval);

        // headroom can't be larger than frame time, or less than 0
        frameRateOptions.headRoomRatio = std::min(frameRateOptions.headRoomRatio, 1.0f);
        frameRateOptions.headRoomRatio = std::max(frameRateOptions.headRoomRatio, 0.0f);
    }

    void setClearOptions(const ClearOptions& options) {
        mClearOptions = options;
    }

private:
    friend class Renderer;
    using Command = RenderPass::Command;

    backend::Handle<backend::HwRenderTarget> getRenderTarget(FView& view) const noexcept;

    void readPixels(backend::Handle<backend::HwRenderTarget> renderTargetHandle,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);

    struct ColorPassConfig {
        Viewport vp;
        Viewport svp;
        math::float2 scale;
        backend::TextureFormat hdrFormat;
        uint8_t msaa;
        backend::TargetBufferFlags clearFlags;
        math::float4 clearColor = {};
        float refractionLodOffset;
        bool hasContactShadows;
    };

    struct ColorGradingConfig {
        bool asSubpass = false;
        bool translucent{};
        bool fxaa{};
        bool dithering{};
        backend::TextureFormat ldrFormat{};
    };

    FrameGraphId<FrameGraphTexture> colorPass(FrameGraph& fg, const char* name,
            FrameGraphTexture::Descriptor const& colorBufferDesc,
            ColorPassConfig const& config, ColorGradingConfig const& colorGradingConfig,
            RenderPass const& pass, FView const& view) const noexcept;

    FrameGraphId<FrameGraphTexture> refractionPass(FrameGraph& fg,
            ColorPassConfig config, ColorGradingConfig const& colorGradingConfig,
            RenderPass const& pass, FView const& view) const noexcept;

    void recordHighWatermark(size_t watermark) noexcept {
        mCommandsHighWatermark = std::max(mCommandsHighWatermark, watermark);
    }

    size_t getCommandsHighWatermark() const noexcept {
        return mCommandsHighWatermark * sizeof(RenderPass::Command);
    }

    backend::TextureFormat getHdrFormat(const View& view, bool translucent) const noexcept;
    backend::TextureFormat getLdrFormat(bool translucent) const noexcept;

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
    DisplayInfo mDisplayInfo;
    FrameRateOptions mFrameRateOptions;
    ClearOptions mClearOptions;
    backend::TargetBufferFlags mDiscardedFlags;
    backend::TargetBufferFlags mClearFlags;
    std::function<void()> mBeginFrameInternal;

    // per-frame arena for this Renderer
    LinearAllocatorArena& mPerRenderPassArena;
};

FILAMENT_UPCAST(Renderer)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RENDERER_H
