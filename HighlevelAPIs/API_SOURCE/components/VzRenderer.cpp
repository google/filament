#include "VzRenderer.h"
#include "../VzRenderPath.h"
#include "../VzEngineApp.h"
#include "VzAsset.h"
#include "../FIncludes.h"

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    void VzRenderer::SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window)
    {
        COMP_RENDERPATH(render_path, );

        std::vector<RendererVID> render_path_vids;
        gEngineApp.GetRenderPathVids(render_path_vids);
        if (window) 
        {
            for (size_t i = 0, n = render_path_vids.size(); i < n; ++i)
            {
                VID vid_i = render_path_vids[i];
                VzRenderPath* render_path_i = gEngineApp.GetRenderPath(vid_i);
                if (render_path_i != render_path)
                {
                    void* window_i = nullptr;
                    uint32_t w_i, h_i;
                    float dpi_i;
                    render_path_i->GetCanvas(&w_i, &h_i, &dpi_i, &window_i);
                    if (window_i == window)
                    {
                        std::string name_i = gEngineApp.GetVzComponent<VzRenderer>(vid_i)->GetName();
                        render_path_i->SetCanvas(w_i, h_i, dpi_i, nullptr);
                        backlog::post("another renderer (" + name_i + ") has the same window handle, so force to set nullptr!", backlog::LogLevel::Warning);
                    }
                }
            }
        }

        render_path->SetCanvas(w, h, dpi, window);
        UpdateTimeStamp();
    }
    void VzRenderer::GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window)
    {
        COMP_RENDERPATH(render_path, );
        render_path->GetCanvas(w, h, dpi, window);
    }
    void VzRenderer::SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits)
    {
        COMP_RENDERPATH(render_path, );
        View* view = render_path->GetView();
        view->setVisibleLayers(layerBits, maskBits);
        UpdateTimeStamp();
    }
    void VzRenderer::SetPostProcessingEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.postProcessingEnabled = enabled;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsPostProcessingEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.postProcessingEnabled;
    }
    void VzRenderer::SetDitheringEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.dithering = enabled ? Dithering::TEMPORAL : Dithering::NONE;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsDitheringEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.dithering == Dithering::TEMPORAL;
    }
    void VzRenderer::SetBloomEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.enabled = enabled;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsBloomEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.bloom.enabled;
    }
    void VzRenderer::SetBloomStrength(float strength)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.strength = strength;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetBloomStrength()
    {
        COMP_RENDERPATH(render_path, 0);
        return render_path->viewSettings.bloom.strength;
    }
    void VzRenderer::SetBloomThreshold(bool threshold)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.threshold = threshold;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsBloomThreshold()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.bloom.threshold;
    }
    void VzRenderer::SetBloomLevels(int levels)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.levels = levels;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    int VzRenderer::GetBloomLevels()
    {
        COMP_RENDERPATH(render_path, 0);
        return render_path->viewSettings.bloom.levels;
    }
    void VzRenderer::SetBloomQuality(int quality)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.quality = (QualityLevel) quality;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    int VzRenderer::GetBloomQuality()
    {
        COMP_RENDERPATH(render_path, 0);
        return (int) render_path->viewSettings.bloom.quality;
    }
    void VzRenderer::SetBloomLensFlare(bool lensFlare)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.lensFlare = lensFlare;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsBloomLensFlare()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.bloom.lensFlare;
    }
    void VzRenderer::SetTaaEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.enabled = enabled;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsTaaEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.taa.enabled;
    }
    void VzRenderer::SetFxaaEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.antiAliasing = enabled ? AntiAliasing::FXAA : AntiAliasing::NONE;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsFxaaEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.antiAliasing == AntiAliasing::FXAA;
    }
    void VzRenderer::SetMsaaEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.msaa.enabled = enabled;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsMsaaEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.msaa.enabled;
    }
    void VzRenderer::SetSsaoEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.enabled = enabled;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsSsaoEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.ssao.enabled;
    }
    void VzRenderer::SetScreenSpaceReflectionEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.screenSpaceReflections.enabled = enabled;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsScreenSpaceReflectionEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.screenSpaceReflections.enabled;
    }
    void VzRenderer::SetGuardBandEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.guardBand.enabled = enabled;
        render_path->isViewSettingsDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsGuardBandEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.guardBand.enabled;
    }
    VZRESULT VzRenderer::Render(const VID vidScene, const VID vidCam)
    {
        VzRenderPath* render_path = gEngineApp.GetRenderPath(GetVID());
        if (render_path == nullptr)
        {
            backlog::post("invalid render path", backlog::LogLevel::Error);
            return VZ_FAIL;
        }

        View* view = render_path->GetView();
        Scene* scene = gEngineApp.GetScene(vidScene);
        Camera * camera = gEngine->getCameraComponent(utils::Entity::import(vidCam));
        if (view == nullptr || scene == nullptr || camera == nullptr)
        {
            backlog::post("renderer has nullptr : " + std::string(view == nullptr ? "view " : "")
                + std::string(scene == nullptr ? "scene " : "") + std::string(camera == nullptr ? "camera" : "")
                , backlog::LogLevel::Error);
            return VZ_FAIL;
        }
        render_path->TryResizeRenderTargets();
        view->setScene(scene);
        view->setCamera(camera);
        //view->setVisibleLayers(0x4, 0x4);
        //SceneVID vid_scene = gEngineApp.GetSceneVidBelongTo(vidCam);
        //assert(vid_scene != INVALID_VID);

        if (!UTILS_HAS_THREADING)
        {
            gEngine->execute();
        }

        VzCameraRes* cam_res = gEngineApp.GetCameraRes(vidCam);
        cam_res->UpdateCameraWithCM(render_path->deltaTime);

        if (cam_res->FRAMECOUNT == 0)
        {
            cam_res->timer = std::chrono::high_resolution_clock::now();
        }

        // fixed time update
        if (0)
        {
            render_path->deltaTimeAccumulator += render_path->deltaTime;
            if (render_path->deltaTimeAccumulator > 10)
            {
                // application probably lost control, fixed update would take too long
                render_path->deltaTimeAccumulator = 0;
            }

            const float targetFrameRateInv = 1.0f / render_path->GetFixedTimeUpdate();
            while (render_path->deltaTimeAccumulator >= targetFrameRateInv)
            {
                //renderer->FixedUpdate();
                render_path->deltaTimeAccumulator -= targetFrameRateInv;
            }
        }

        // Update the cube distortion matrix used for frustum visualization.
        const Camera* lightmapCamera = view->getDirectionalShadowCamera();
        if (lightmapCamera) {
            VzSceneRes* scene_res = gEngineApp.GetSceneRes(vidScene);
            Cube* lightmapCube = scene_res->GetLightmapCube();
            lightmapCube->mapFrustum(*gEngine, lightmapCamera);
        }
        Cube* cameraCube = cam_res->GetCameraCube();
        if (cameraCube) {
            cameraCube->mapFrustum(*gEngine, camera);
        }

        ResourceLoader* resource_loader = gEngineApp.GetGltfResourceLoader();
        if (resource_loader)
            resource_loader->asyncUpdateLoad();

        std::unordered_map<AssetVID, std::unique_ptr<VzAssetRes>>& assetResMap = *gEngineApp.GetAssetResMap();

        for (auto& it : assetResMap)
        {
            VzAssetRes* asset_res = it.second.get();
            VzAsset* v_asset = gEngineApp.GetVzComponent<VzAsset>(it.first);
            assert(v_asset);
            vzm::VzAsset::Animator* animator = v_asset->GetAnimator();
            if (animator->IsPlayScene(vidScene))
            {
                animator->UpdateAnimation();
            }
        }

        Renderer* renderer = render_path->GetRenderer();

        auto& tcm = gEngine->getTransformManager();
        scene->forEach([&tcm](Entity ett) {
            VID vid = ett.getId();
            VzSceneComp* comp = gEngineApp.GetVzComponent<VzSceneComp>(vid);
            if (comp && comp->IsMatrixAutoUpdate())
            {
                comp->UpdateMatrix();
            }
        });

        // setup
        //if (preRender) {
        //    preRender(mEngine, window->mViews[0]->getView(), mScene, renderer);
        //}

        //if (mReconfigureCameras) {
        //    window->configureCamerasForWindow();
        //    mReconfigureCameras = false;
        //}

        //if (config.splitView) {
        //    if (!window->mOrthoView->getView()->hasCamera()) {
        //        Camera const* debugDirectionalShadowCamera =
        //            window->mMainView->getView()->getDirectionalShadowCamera();
        //        if (debugDirectionalShadowCamera) {
        //            window->mOrthoView->setCamera(
        //                const_cast<Camera*>(debugDirectionalShadowCamera));
        //        }
        //    }
        //}

        render_path->applyViewSettings();

        filament::SwapChain* sc = render_path->GetSwapChain();
        if (renderer->beginFrame(sc)) {
            renderer->render(view);
            renderer->endFrame();
        }

        if (gEngine->getBackend() == Backend::OPENGL)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        TimeStamp timer2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timer2 - cam_res->timer);
        cam_res->timer = timer2;
        render_path->deltaTime = (float)time_span.count();
        render_path->FRAMECOUNT++;

        return VZ_OK;
    }
}
