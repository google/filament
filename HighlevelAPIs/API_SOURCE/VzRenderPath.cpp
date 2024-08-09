#include "VzRenderPath.h"
#include "VzEngineApp.h"

using namespace vzm;
extern Engine* gEngine;
extern VzEngineApp gEngineApp;

namespace vzm
{
    VzRenderPath::VzRenderPath()
    {
        assert(gEngine && "native engine is not initialized!");
        view_ = gEngine->createView();
        renderer_ = gEngine->createRenderer();
        swapChain_ = gEngine->createSwapChain(width_, height_);

        //for (int i = 0; i < 8; i++)
        //view_->setStencilBufferEnabled(false);
    }

    VzRenderPath::~VzRenderPath()
    {
        if (gEngine)
        {
            if (renderer_)
                gEngine->destroy(renderer_);
            if (view_)
                gEngine->destroy(view_);
            if (swapChain_)
                gEngine->destroy(swapChain_);
        }
    }

    void VzRenderPath::resize()
    {
        auto resizeJob = [&]()
            {
                gEngine->destroy(swapChain_);
                if (nativeWindow_ == nullptr)
                {
                    swapChain_ = gEngine->createSwapChain(width_, height_);
                }
                else
                {
                    swapChain_ = gEngine->createSwapChain(
                        nativeWindow_, filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);

                    // dummy calls?
                    // this code causes async error 
                    // "state->elapsed.store(int64_t(TimerQueryResult::ERROR), std::memory_order_relaxed);"
                    //renderer_->beginFrame(swapChain_);
                    //renderer_->endFrame();
                }
            };

        //utils::JobSystem::Job* parent = js.createJob();
        //js.run(jobs::createJob(js, parent, resizeJob));
        //js.runAndWait(parent);
        resizeJob();
    }
    
    bool VzRenderPath::TryResizeRenderTargets()
    {
        if (gEngine == nullptr)
            return false;

        colorspaceConversionRequired_ = colorSpace_ != SWAP_CHAIN_CONFIG_SRGB_COLORSPACE;

        bool requireUpdateRenderTarget = prevWidth_ != width_ || prevHeight_ != height_ || prevDpi_ != dpi_
            || prevColorspaceConversionRequired_ != colorspaceConversionRequired_;
        if (!requireUpdateRenderTarget)
            return false;

        resize(); // how to handle rendertarget textures??

        prevWidth_ = width_;
        prevHeight_ = height_;
        prevDpi_ = dpi_;
        prevNativeWindow_ = nativeWindow_;
        prevColorspaceConversionRequired_ = colorspaceConversionRequired_;
        return true;
    }

    void VzRenderPath::SetFixedTimeUpdate(const float targetFPS)
    {
        targetFrameRate_ = targetFPS;
        timeStamp_ = std::chrono::high_resolution_clock::now();
    }
    float VzRenderPath::GetFixedTimeUpdate() const
    {
        return targetFrameRate_;
    }

    void VzRenderPath::GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window)
    {
        if (w) *w = width_;
        if (h) *h = height_;
        if (dpi) *dpi = dpi_;
        if (window) *window = nativeWindow_;
    }
    void VzRenderPath::SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window)
    {
        // the resize is called during the rendering (pre-processing)
        width_ = w;
        height_ = h;
        this->dpi_ = dpi;
        nativeWindow_ = window;

        view_->setViewport(filament::Viewport(0, 0, w, h));
        timeStamp_ = std::chrono::high_resolution_clock::now();
    }
    filament::SwapChain* VzRenderPath::GetSwapChain()
    {
        return swapChain_;
    }

    filament::View* VzRenderPath::GetView() { return view_; }

    filament::Renderer* VzRenderPath::GetRenderer() { return renderer_; }

    void VzRenderPath::applySettings() {
        if (any(dirtyFlags & DirtyFlags::ANTI_ALIASING))
            view_->setAntiAliasing(viewSettings.antiAliasing);

        if (any(dirtyFlags & DirtyFlags::TAA))
            view_->setTemporalAntiAliasingOptions(viewSettings.taa);

        if (any(dirtyFlags & DirtyFlags::MSAA))
            view_->setMultiSampleAntiAliasingOptions(viewSettings.msaa);

        if (any(dirtyFlags & DirtyFlags::DSR))
            view_->setDynamicResolutionOptions(viewSettings.dsr);

        if (any(dirtyFlags & DirtyFlags::SSAO))
            view_->setAmbientOcclusionOptions(viewSettings.ssao);

        if (any(dirtyFlags & DirtyFlags::SCREEN_SPACE_REFLECTIONS))
            view_->setScreenSpaceReflectionsOptions(viewSettings.screenSpaceReflections);

        if (any(dirtyFlags & DirtyFlags::BLOOM))
            view_->setBloomOptions(viewSettings.bloom);

        if (any(dirtyFlags & DirtyFlags::FOG))
            view_->setFogOptions(viewSettings.fog);

        if (any(dirtyFlags & DirtyFlags::DOF))
            view_->setDepthOfFieldOptions(viewSettings.dof);

        if (any(dirtyFlags & DirtyFlags::VIGNETTE))
            view_->setVignetteOptions(viewSettings.vignette);

        if (any(dirtyFlags & DirtyFlags::DITHERING))
            view_->setDithering(viewSettings.dithering);

        if (any(dirtyFlags & DirtyFlags::RENDER_QUALITY))
            view_->setRenderQuality(viewSettings.renderQuality);

        if (any(dirtyFlags & DirtyFlags::DYNAMIC_LIGHTING))
            view_->setDynamicLightingOptions(viewSettings.dynamicLighting.zLightNear,
                                             viewSettings.dynamicLighting.zLightFar);

        if (any(dirtyFlags & DirtyFlags::SHADOW_TYPE))
            view_->setShadowType(viewSettings.shadowType);

        if (any(dirtyFlags & DirtyFlags::VSM_SHADOW_OPTIONS))
            view_->setVsmShadowOptions(viewSettings.vsmShadowOptions);

        if (any(dirtyFlags & DirtyFlags::SOFT_SHADOW_OPTIONS))
            view_->setSoftShadowOptions(viewSettings.softShadowOptions);

        if (any(dirtyFlags & DirtyFlags::GUARD_BAND))
            view_->setGuardBandOptions(viewSettings.guardBand);

        if (any(dirtyFlags & DirtyFlags::STEREOSCOPIC_OPTIONS))
            view_->setStereoscopicOptions(viewSettings.stereoscopicOptions);

        if (any(dirtyFlags & DirtyFlags::POST_PROCESSING_ENABLED))
            view_->setPostProcessingEnabled(viewSettings.postProcessingEnabled);

        dirtyFlags = DirtyFlags::NONE;
    }
}
