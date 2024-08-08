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
        void SetPostProcessingEnabled(bool enabled);
        bool IsPostProcessingEnabled();

        void SetDitheringEnabled(bool enabled);
        bool IsDitheringEnabled();

        void SetBloomEnabled(bool enabled);
        bool IsBloomEnabled();

        void SetTaaEnabled(bool enabled);
        bool IsTaaEnabled();

        void SetFxaaEnabled(bool enabled);
        bool IsFxaaEnabled();

        void SetMsaaEnabled(bool enabled);
        bool IsMsaaEnabled();

        void SetMsaaCustomResolve(bool customResolve);
        bool IsMsaaCustomResolve();

        void SetSsaoEnabled(bool enabled);
        bool IsSsaoEnabled();

        void SetScreenSpaceReflectionsEnabled(bool enabled);
        bool IsScreenSpaceReflectionsEnabled();

        void SetGuardBandEnabled(bool enabled);
        bool IsGuardBandEnabled();

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

        void SetTaaUpscaling(bool upscaling);
        bool IsTaaUpscaling();

        void SetTaaHistoryReprojection(bool historyReprojection);
        bool IsTaaHistoryReprojection();

        void SetTaaFeedback(float feedback);
        float GetTaaFeedback();

        void SetTaaFilterHistory(bool filterHistory);
        bool IsTaaFilterHistory();

        void SetTaaFilterInput(bool filterInput);
        bool IsTaaFilterInput();

        void SetTaaFilterWidth(float filterWidth);
        float GetTaaFilterWidth();

        void SetTaaLodBias(float lodBias);
        float GetTaaLodBias();

        void SetTaaUseYCoCg(bool useYCoCg);
        bool IsTaaUseYCoCg();

        void SetTaaPreventFlickering(bool preventFlickering);
        bool IsTaaPreventFlickering();

        enum class JitterPattern : uint8_t {
            RGSS_X4,             //!  4-samples, rotated grid sampling
            UNIFORM_HELIX_X4,    //!  4-samples, uniform grid in helix sequence
            HALTON_23_X8,        //!  8-samples of halton 2,3
            HALTON_23_X16,       //! 16-samples of halton 2,3
            HALTON_23_X32        //! 32-samples of halton 2,3
        };
        void SetTaaJitterPattern(JitterPattern jitterPattern);
        JitterPattern GetTaaJitterPattern();

        enum class BoxClipping : uint8_t {
            ACCURATE,       //!< Accurate box clipping
            CLAMP,          //!< clamping
            NONE            //!< no rejections (use for debugging)
        };
        void SetTaaBoxClipping(BoxClipping boxClipping);
        BoxClipping GetTaaBoxClipping();

        enum class BoxType : uint8_t {
            AABB,           //!< use an AABB neighborhood
            VARIANCE,       //!< use the variance of the neighborhood (not recommended)
            AABB_VARIANCE   //!< use both AABB and variance
        };
        void SetTaaBoxType(BoxType boxType);
        BoxType GetTaaBoxType();

        void SetTaaVarianceGamma(float varianceGamma);
        float GetTaaVarianceGamma();

        void SetTaaSharpness(float sharpness);
        float GetTaaSharpness();

        void SetSsaoQuality(int quality);
        int GetSsaoQuality();

        void SetSsaoLowPassFilter(int lowPassFilter);
        int GetSsaoLowPassFilter();

        void SetSsaoBentNormals(bool bentNormals);
        bool IsSsaoBentNormals();

        void SetSsaoUpsampling(bool upsampling);
        bool IsSsaoUpsampling();

        void SetSsaoMinHorizonAngleRad(float minHorizonAngleRad);
        float GetSsaoMinHorizonAngleRad();

        void SetSsaoBilateralThreshold(float bilateralThreshold);
        float GetSsaoBilateralThreshold();

        void SetSsaoHalfResolution(bool halfResolution);
        bool IsSsaoHalfResolution();

        void SetSsaoSsctEnabled(bool enabled);
        bool IsSsaoSsctEnabled();

        void SetSsaoSsctLightConeRad(float lightConeRad);
        float GetSsaoSsctLightConeRad();

        void SetSsaoSsctShadowDistance(float shadowDistance);
        float GetSsaoSsctShadowDistance();

        void SetSsaoSsctContactDistanceMax(float contactDistanceMax);
        float GetSsaoSsctContactDistanceMax();

        void SetSsaoSsctIntensity(float intensity);
        float GetSsaoSsctIntensity();

        void SetSsaoSsctDepthBias(float depthBias);
        float GetSsaoSsctDepthBias();

        void SetSsaoSsctDepthSlopeBias(float depthSlopeBias);
        float GetSsaoSsctDepthSlopeBias();

        void SetSsaoSsctSampleCount(int sampleCount);
        int GetSsaoSsctSampleCount();

        void SetSsaoSsctLightDirection(const float lightDirection[3]);
        void GetSsaoSsctLightDirection(float lightDirection[3]);

        void SetScreenSpaceReflectionsThickness(float thickness);
        float GetScreenSpaceReflectionsThickness();

        void SetScreenSpaceReflectionsBias(float bias);
        float GetScreenSpaceReflectionsBias();

        void SetScreenSpaceReflectionsMaxDistance(float maxDistance);
        float GetScreenSpaceReflectionsMaxDistance();

        void SetScreenSpaceReflectionsStride(float stride);
        float GetScreenSpaceReflectionsStride();

        void SetDynamicResoultionEnabled(bool enabled);
        bool IsDynamicResoultionEnabled();

        void SetDynamicResoultionHomogeneousScaling(bool homogeneousScaling);
        bool IsDynamicResoultionHomogeneousScaling();
        
        void SetDynamicResoultionMinScale(float minScale);
        float GetDynamicResoultionMinScale();

        void SetDynamicResoultionMaxScale(float maxScale);
        float GetDynamicResoultionMaxScale();

        void SetDynamicResoultionQuality(int quality);
        int GetDynamicResoultionQuality();

        void SetDynamicResoultionSharpness(float sharpness);
        float GetDynamicResoultionSharpness();

        VZRESULT Render(const VID vidScene, const VID vidCam);
        VZRESULT Render(const VzBaseComp* scene, const VzBaseComp* camera) { return Render(scene->GetVID(), camera->GetVID()); };
    };
}
