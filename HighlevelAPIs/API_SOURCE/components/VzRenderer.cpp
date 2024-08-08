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

#pragma region View
    void VzRenderer::SetPostProcessingEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.postProcessingEnabled = enabled;
        render_path->isDirty = true;
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
        render_path->isDirty = true;
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
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsBloomEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.bloom.enabled;
    }
    void VzRenderer::SetTaaEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.enabled = enabled;
        render_path->isDirty = true;
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
        render_path->isDirty = true;
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
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsMsaaEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.msaa.enabled;
    }
    void VzRenderer::SetMsaaCustomResolve(bool customResolve)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.msaa.customResolve = customResolve;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsMsaaCustomResolve()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.msaa.customResolve;
    }
    void VzRenderer::SetSsaoEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.enabled = enabled;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsSsaoEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.ssao.enabled;
    }
    void VzRenderer::SetScreenSpaceReflectionsEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.screenSpaceReflections.enabled = enabled;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsScreenSpaceReflectionsEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.screenSpaceReflections.enabled;
    }
    void VzRenderer::SetGuardBandEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.guardBand.enabled = enabled;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsGuardBandEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.guardBand.enabled;
    }
#pragma endregion

#pragma region Bloom Options
    void VzRenderer::SetBloomStrength(float strength)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.strength = std::clamp(strength, 0.0f, 1.0f);
        render_path->isDirty = true;
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
        render_path->isDirty = true;
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
        render_path->viewSettings.bloom.levels = (uint8_t) std::clamp(levels, 3, 11);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    int VzRenderer::GetBloomLevels()
    {
        COMP_RENDERPATH(render_path, (int) QualityLevel::LOW);
        return render_path->viewSettings.bloom.levels;
    }
    void VzRenderer::SetBloomQuality(int quality)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.bloom.quality = (QualityLevel) std::clamp(quality, 0, 3);
        render_path->isDirty = true;
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
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsBloomLensFlare()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.bloom.lensFlare;
    }
#pragma endregion

#pragma region TAA Options
    void VzRenderer::SetTaaUpscaling(bool upscaling)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.upscaling = upscaling;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsTaaUpscaling()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.taa.upscaling;
    }
    void VzRenderer::SetTaaHistoryReprojection(bool historyReprojection)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.historyReprojection = historyReprojection;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsTaaHistoryReprojection()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.taa.historyReprojection;
    }
    void VzRenderer::SetTaaFeedback(float feedback)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.feedback = std::clamp(feedback, 0.0f, 1.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetTaaFeedback()
    {
        COMP_RENDERPATH(render_path, 0);
        return render_path->viewSettings.taa.feedback;
    }
    void VzRenderer::SetTaaFilterHistory(bool filterHistory)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.filterHistory = filterHistory;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsTaaFilterHistory()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.taa.filterHistory;
    }
    void VzRenderer::SetTaaFilterInput(bool filterInput)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.filterInput = filterInput;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsTaaFilterInput()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.taa.filterInput;
    }
    void VzRenderer::SetTaaFilterWidth(float filterWidth)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.filterWidth = std::clamp(filterWidth, 0.2f, 2.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetTaaFilterWidth()
    {
        COMP_RENDERPATH(render_path, 0);
        return render_path->viewSettings.taa.filterWidth;
    }
    void VzRenderer::SetTaaLodBias(float lodBias)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.lodBias = std::clamp(lodBias, -8.0f, 0.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetTaaLodBias()
    {
        COMP_RENDERPATH(render_path, 0);
        return render_path->viewSettings.taa.lodBias;
    }
    void VzRenderer::SetTaaUseYCoCg(bool useYCoCg)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.useYCoCg = useYCoCg;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsTaaUseYCoCg()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.taa.useYCoCg;
    }
    void VzRenderer::SetTaaPreventFlickering(bool preventFlickering)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.preventFlickering = preventFlickering;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsTaaPreventFlickering()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.taa.preventFlickering;
    }
    void VzRenderer::SetTaaJitterPattern(JitterPattern pattern)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.jitterPattern = (TemporalAntiAliasingOptions::JitterPattern) pattern;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    VzRenderer::JitterPattern VzRenderer::GetTaaJitterPattern()
    {
        COMP_RENDERPATH(render_path, JitterPattern::HALTON_23_X16);
        return (JitterPattern) render_path->viewSettings.taa.jitterPattern;
    }
    void VzRenderer::SetTaaBoxClipping(BoxClipping boxClipping)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.boxClipping = (TemporalAntiAliasingOptions::BoxClipping) boxClipping;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    VzRenderer::BoxClipping VzRenderer::GetTaaBoxClipping()
    {
        COMP_RENDERPATH(render_path, BoxClipping::ACCURATE);
        return (BoxClipping) render_path->viewSettings.taa.boxClipping;
    }
    void VzRenderer::SetTaaBoxType(BoxType boxType)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.boxType = (TemporalAntiAliasingOptions::BoxType) boxType;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    VzRenderer::BoxType VzRenderer::GetTaaBoxType()
    {
        COMP_RENDERPATH(render_path, BoxType::AABB);
        return (BoxType) render_path->viewSettings.taa.boxType;
    }
    void VzRenderer::SetTaaVarianceGamma(float varianceGamma)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.varianceGamma = std::clamp(varianceGamma, 0.75f, 1.25f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetTaaVarianceGamma()
    {
        COMP_RENDERPATH(render_path, 0);
        return render_path->viewSettings.taa.varianceGamma;
    }
    void VzRenderer::SetTaaSharpness(float sharpness)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.taa.sharpness = std::clamp(sharpness, 0.0f, 1.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetTaaSharpness()
    {
        COMP_RENDERPATH(render_path, 0);
        return render_path->viewSettings.taa.sharpness;
    }
#pragma endregion

#pragma region SSAO Options
    void VzRenderer::SetSsaoQuality(int quality)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.quality = (QualityLevel) std::clamp(quality, 0, 3);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    int VzRenderer::GetSsaoQuality()
    {
        COMP_RENDERPATH(render_path, (int) QualityLevel::LOW);
        return (int) render_path->viewSettings.ssao.quality;
    }
    void VzRenderer::SetSsaoLowPassFilter(int lowPassFilter)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.lowPassFilter = (QualityLevel) std::clamp(lowPassFilter, 0, 2);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    int VzRenderer::GetSsaoLowPassFilter()
    {
        COMP_RENDERPATH(render_path, (int) QualityLevel::MEDIUM);
        return (int) render_path->viewSettings.ssao.lowPassFilter;
    }
    void VzRenderer::SetSsaoBentNormals(bool bentNormals)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.bentNormals = bentNormals;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsSsaoBentNormals()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.ssao.bentNormals;
    }
    void VzRenderer::SetSsaoUpsampling(bool upsampling)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.upsampling = upsampling ? QualityLevel::HIGH : QualityLevel::LOW;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsSsaoUpsampling()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.ssao.upsampling != QualityLevel::LOW;
    }
    void VzRenderer::SetSsaoMinHorizonAngleRad(float minHorizonAngleRad)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.minHorizonAngleRad = std::clamp(minHorizonAngleRad, 0.0f, VZ_PIDIV4);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoMinHorizonAngleRad()
    {
        COMP_RENDERPATH(render_path, 0.0f);
        return render_path->viewSettings.ssao.minHorizonAngleRad;
    }
    void VzRenderer::SetSsaoBilateralThreshold(float bilateralThreshold)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.bilateralThreshold = std::clamp(bilateralThreshold, 0.0f, 0.1f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoBilateralThreshold()
    {
        COMP_RENDERPATH(render_path, 0.05f);
        return render_path->viewSettings.ssao.bilateralThreshold;
    }
    void VzRenderer::SetSsaoHalfResolution(bool halfResolution)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.resolution = halfResolution ? 0.5f : 1.0f;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsSsaoHalfResolution()
    {
        COMP_RENDERPATH(render_path, true);
        return render_path->viewSettings.ssao.resolution != 1.0f;
    }

#pragma region Dominant Light Shadows (experimental)
    void VzRenderer::SetSsaoSsctEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.enabled = enabled;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsSsaoSsctEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.ssao.ssct.enabled;
    }
    void VzRenderer::SetSsaoSsctLightConeRad(float lightConeRad)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.lightConeRad = std::clamp(lightConeRad, 0.0f, VZ_PIDIV2);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoSsctLightConeRad()
    {
        COMP_RENDERPATH(render_path, 1.0f);
        return render_path->viewSettings.ssao.ssct.lightConeRad;
    }
    void VzRenderer::SetSsaoSsctShadowDistance(float shadowDistance)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.shadowDistance = std::clamp(shadowDistance, 0.0f, 10.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoSsctShadowDistance()
    {
        COMP_RENDERPATH(render_path, 0.3f);
        return render_path->viewSettings.ssao.ssct.shadowDistance;
    }
    void VzRenderer::SetSsaoSsctContactDistanceMax(float contactDistanceMax)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.contactDistanceMax = std::clamp(contactDistanceMax, 0.0f, 10.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoSsctContactDistanceMax()
    {
        COMP_RENDERPATH(render_path, 1.0f);
        return render_path->viewSettings.ssao.ssct.contactDistanceMax;
    }
    void VzRenderer::SetSsaoSsctIntensity(float intensity)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.intensity = std::clamp(intensity, 0.0f, 10.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoSsctIntensity()
    {
        COMP_RENDERPATH(render_path, 0.8f);
        return render_path->viewSettings.ssao.ssct.intensity;
    }
    void VzRenderer::SetSsaoSsctDepthBias(float depthBias)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.depthBias = std::clamp(depthBias, 0.0f, 1.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoSsctDepthBias()
    {
        COMP_RENDERPATH(render_path, 0.01f);
        return render_path->viewSettings.ssao.ssct.depthBias;
    }
    void VzRenderer::SetSsaoSsctDepthSlopeBias(float depthSlopeBias)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.depthSlopeBias = std::clamp(depthSlopeBias, 0.0f, 1.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetSsaoSsctDepthSlopeBias()
    {
        COMP_RENDERPATH(render_path, 0.01f);
        return render_path->viewSettings.ssao.ssct.depthSlopeBias;
    }
    void VzRenderer::SetSsaoSsctSampleCount(int sampleCount)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.sampleCount = (uint8_t) std::clamp(sampleCount, 1, 32);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    int VzRenderer::GetSsaoSsctSampleCount()
    {
        COMP_RENDERPATH(render_path, 4);
        return render_path->viewSettings.ssao.ssct.sampleCount;
    }
    void VzRenderer::SetSsaoSsctLightDirection(const float lightDirection[3])
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.ssao.ssct.lightDirection.x = lightDirection[0];
        render_path->viewSettings.ssao.ssct.lightDirection.y = lightDirection[1];
        render_path->viewSettings.ssao.ssct.lightDirection.z = lightDirection[2];
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    void VzRenderer::GetSsaoSsctLightDirection(float lightDirection[3])
    {
        COMP_RENDERPATH(render_path, );
        lightDirection[0] = render_path->viewSettings.ssao.ssct.lightDirection[0];
        lightDirection[1] = render_path->viewSettings.ssao.ssct.lightDirection[1];
        lightDirection[2] = render_path->viewSettings.ssao.ssct.lightDirection[2];
    }
#pragma endregion

#pragma endregion

#pragma region Screen-space reflections Options
    void VzRenderer::SetScreenSpaceReflectionsThickness(float thickness)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.screenSpaceReflections.thickness = std::clamp(thickness, 0.001f, 0.2f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetScreenSpaceReflectionsThickness()
    {
        COMP_RENDERPATH(render_path, 0.1f);
        return render_path->viewSettings.screenSpaceReflections.thickness;
    }
    void VzRenderer::SetScreenSpaceReflectionsBias(float bias)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.screenSpaceReflections.bias = std::clamp(bias, 0.001f, 0.5f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetScreenSpaceReflectionsBias()
    {
        COMP_RENDERPATH(render_path, 0.01f);
        return render_path->viewSettings.screenSpaceReflections.bias;
    }
    void VzRenderer::SetScreenSpaceReflectionsMaxDistance(float maxDistance)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.screenSpaceReflections.maxDistance = std::clamp(maxDistance, 0.1f, 10.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetScreenSpaceReflectionsMaxDistance()
    {
        COMP_RENDERPATH(render_path, 3.0f);
        return render_path->viewSettings.screenSpaceReflections.maxDistance;
    }
    void VzRenderer::SetScreenSpaceReflectionsStride(float stride)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.screenSpaceReflections.stride = std::clamp(stride, 0.1f, 10.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetScreenSpaceReflectionsStride()
    {
        COMP_RENDERPATH(render_path, 2.0f);
        return render_path->viewSettings.screenSpaceReflections.stride;
    }
#pragma endregion

#pragma region Dynamic Resolution
    void VzRenderer::SetDynamicResoultionEnabled(bool enabled)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.dsr.enabled = enabled;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsDynamicResoultionEnabled()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.dsr.enabled;
    }
    void VzRenderer::SetDynamicResoultionHomogeneousScaling(bool homogeneousScaling)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.dsr.homogeneousScaling = homogeneousScaling;
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    bool VzRenderer::IsDynamicResoultionHomogeneousScaling()
    {
        COMP_RENDERPATH(render_path, false);
        return render_path->viewSettings.dsr.homogeneousScaling;
    }
    void VzRenderer::SetDynamicResoultionMinScale(float minScale)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.dsr.minScale = std::clamp(minScale, 0.25f, 1.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetDynamicResoultionMinScale()
    {
        COMP_RENDERPATH(render_path, 0.5f);
        return render_path->viewSettings.dsr.minScale.x;
    }
    void VzRenderer::SetDynamicResoultionMaxScale(float maxScale)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.dsr.maxScale = std::clamp(maxScale, 0.25f, 1.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetDynamicResoultionMaxScale()
    {
        COMP_RENDERPATH(render_path, 1.0f);
        return render_path->viewSettings.dsr.maxScale.x;
    }
    void VzRenderer::SetDynamicResoultionQuality(int quality)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.dsr.quality = (QualityLevel) std::clamp(quality, 0, 3);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    int VzRenderer::GetDynamicResoultionQuality()
    {
        COMP_RENDERPATH(render_path, 0);
        return (int) render_path->viewSettings.dsr.quality;
    }
    void VzRenderer::SetDynamicResoultionSharpness(float sharpness)
    {
        COMP_RENDERPATH(render_path, );
        render_path->viewSettings.dsr.sharpness = std::clamp(sharpness, 0.0f, 1.0f);
        render_path->isDirty = true;
        UpdateTimeStamp();
    }
    float VzRenderer::GetDynamicResoultionSharpness()
    {
        COMP_RENDERPATH(render_path, 0.9f);
        return render_path->viewSettings.dsr.sharpness;
    }
#pragma endregion

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

        render_path->applySettings();

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
