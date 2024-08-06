#pragma once
#include "../VizComponentAPIs.h"

namespace vzm
{
    __dojostruct VzRenderer : VzBaseComp
    {
        VzRenderer(const VID vid, const std::string& originFrom)
            : VzBaseComp(vid, originFrom, "VzRenderer") {}
        void SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window = nullptr);
        void GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window = nullptr);

        void SetViewport(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h);
        void GetViewport(uint32_t* x, uint32_t* y, uint32_t* w, uint32_t* h);

        void SetVisibleLayerMask(const uint8_t layerBits, const uint8_t maskBits);

        // setters and getters of rendering options

        // Post-processing
        void SetPostProcessingEnabled(bool enabled);
        bool IsPostProcessingEnabled();

        // Dithering
        void SetDitheringEnabled(bool enabled);
        bool IsDitheringEnabled();

        // Bloom
        void SetBloomEnabled(bool enabled);
        bool IsBloomEnabled();

        void SetBloomStrength(float strength);
        float GetBloomStrength();

        void SetBloomThreshold(bool threshold);
        bool IsBloomThreshold();

        void SetBloomLevels(int levels);
        int GetBloomLevels();

        void SetBloomQuality(int quality);
        int GetBloomQuality();

        void SetBloomLensFlare(bool lensFlare);
        bool IsBloomLensFlare();

        // TAA
        void SetTaaEnabled(bool enabled);
        bool IsTaaEnabled();

        // FXAA
        void SetFxaaEnabled(bool enabled);
        bool IsFxaaEnabled();

        // MSAA 4x
        void SetMsaaEnabled(bool enabled);
        bool IsMsaaEnabled();

        // SSAO
        void SetSsaoEnabled(bool enabled);
        bool IsSsaoEnabled();

        // Screen-space reflections
        void SetScreenSpaceReflectionEnabled(bool enabled);
        bool IsScreenSpaceReflectionEnabled();

        // Screen-space Guard Band
        void SetGuardBandEnabled(bool enabled);
        bool IsGuardBandEnabled();

        VZRESULT Render(const VID vidScene, const VID vidCam);
        VZRESULT Render(const VzBaseComp* scene, const VzBaseComp* camera) { return Render(scene->GetVID(), camera->GetVID()); };
    };
}
