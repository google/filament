#ifndef VZRENDERPATH_H
#define VZRENDERPATH_H
#include "VzComponents.h"
#include "FIncludes.h"

#define CANVAS_INIT_W 16u
#define CANVAS_INIT_H 16u
#define CANVAS_INIT_DPI 96.f

namespace vzm
{
    enum class ToneMapping : uint8_t {
        LINEAR = 0,
        ACES_LEGACY = 1,
        ACES = 2,
        FILMIC = 3,
        AGX = 4,
        GENERIC = 5,
        PBR_NEUTRAL = 6,
        DISPLAY_RANGE = 7,
    };

    struct GenericToneMapperSettings {
        float contrast = 1.55f;
        float midGrayIn = 0.18f;
        float midGrayOut = 0.215f;
        float hdrMax = 10.0f;
        bool operator!=(const GenericToneMapperSettings& rhs) const { return !(rhs == *this); }
        bool operator==(const GenericToneMapperSettings& rhs) const;
    };

    struct AgxToneMapperSettings {
        AgxToneMapper::AgxLook look = AgxToneMapper::AgxLook::NONE;
        bool operator!=(const AgxToneMapperSettings& rhs) const { return !(rhs == *this); }
        bool operator==(const AgxToneMapperSettings& rhs) const;
    };

    struct ColorGradingSettings {
        // fields are ordered to avoid padding
        bool enabled = true;
        bool linkedCurves = false;
        bool luminanceScaling = false;
        bool gamutMapping = false;
        filament::ColorGrading::QualityLevel quality = filament::ColorGrading::QualityLevel::MEDIUM;
        ToneMapping toneMapping = ToneMapping::ACES_LEGACY;
        bool padding0{};
        AgxToneMapperSettings agxToneMapper;
        color::ColorSpace colorspace = Rec709-sRGB-D65;
        GenericToneMapperSettings genericToneMapper;
        math::float4 shadows{1.0f, 1.0f, 1.0f, 0.0f};
        math::float4 midtones{1.0f, 1.0f, 1.0f, 0.0f};
        math::float4 highlights{1.0f, 1.0f, 1.0f, 0.0f};
        math::float4 ranges{0.0f, 0.333f, 0.550f, 1.0f};
        math::float3 outRed{1.0f, 0.0f, 0.0f};
        math::float3 outGreen{0.0f, 1.0f, 0.0f};
        math::float3 outBlue{0.0f, 0.0f, 1.0f};
        math::float3 slope{1.0f};
        math::float3 offset{0.0f};
        math::float3 power{1.0f};
        math::float3 gamma{1.0f};
        math::float3 midPoint{1.0f};
        math::float3 scale{1.0f};
        float exposure = 0.0f;
        float nightAdaptation = 0.0f;
        float temperature = 0.0f;
        float tint = 0.0f;
        float contrast = 1.0f;
        float vibrance = 1.0f;
        float saturation = 1.0f;

        bool operator!=(const ColorGradingSettings& rhs) const { return !(rhs == *this); }
        bool operator==(const ColorGradingSettings& rhs) const;
    };

    struct DynamicLightingSettings {
        float zLightNear = 5;
        float zLightFar = 100;
    };

    struct FogSettings {
        Texture* fogColorTexture = nullptr;
    };

    // This defines fields in the same order as the setter methods in filament::View.
    struct ViewSettings {
        // standalone View settings
        AntiAliasing antiAliasing = AntiAliasing::FXAA;
        Dithering dithering = Dithering::TEMPORAL;
        ShadowType shadowType = ShadowType::PCF;
        bool postProcessingEnabled = true;

        // View Options (sorted)
        AmbientOcclusionOptions ssao;
        ScreenSpaceReflectionsOptions screenSpaceReflections;
        BloomOptions bloom;
        DepthOfFieldOptions dof;
        DynamicResolutionOptions dsr;
        FogOptions fog;
        MultiSampleAntiAliasingOptions msaa;
        RenderQuality renderQuality;
        TemporalAntiAliasingOptions taa;
        VignetteOptions vignette;
        VsmShadowOptions vsmShadowOptions;
        SoftShadowOptions softShadowOptions;
        GuardBandOptions guardBand;
        StereoscopicOptions stereoscopicOptions;

        // Custom View Options
        ColorGradingSettings colorGrading;
        DynamicLightingSettings dynamicLighting;
        FogSettings fogSettings;
    };

    struct VzCanvas
    {
    protected:
        uint32_t width_ = CANVAS_INIT_W;
        uint32_t height_ = CANVAS_INIT_H;
        float dpi_ = CANVAS_INIT_DPI;
        float scaling_ = 1; // custom DPI scaling factor (optional)
        void* nativeWindow_ = nullptr;
    public:
        VzCanvas() = default;
        ~VzCanvas() = default;
    };

    // note that renderPath involves 
    // 1. canvas (render targets), 2. camera, 3. scene
    class VzRenderPath : public VzCanvas
    {
    private:
        uint32_t prevWidth_ = 0;
        uint32_t prevHeight_ = 0;
        float prevDpi_ = 0;
        void* prevNativeWindow_ = nullptr;
        bool prevColorspaceConversionRequired_ = false;

        VzCamera* vzCam_ = nullptr;
        TimeStamp timeStamp_ = {};

        float targetFrameRate_ = 60.f;

        bool colorspaceConversionRequired_ = false;
        uint64_t colorSpace_ = 0; // swapchain color space

        // note "view" involves
        // 1. camera
        // 2. scene
        filament::View* view_ = nullptr;
        filament::SwapChain* swapChain_ = nullptr;
        filament::Renderer* renderer_ = nullptr;

        void resize();

    public:
        VzRenderPath();

        ~VzRenderPath();

        bool TryResizeRenderTargets();

        void SetFixedTimeUpdate(const float targetFPS);
        float GetFixedTimeUpdate() const;

        void GetCanvas(uint32_t* w, uint32_t* h, float* dpi, void** window);
        void SetCanvas(const uint32_t w, const uint32_t h, const float dpi, void* window = nullptr);
        filament::SwapChain* GetSwapChain();

        uint64_t FRAMECOUNT = 0;
        float deltaTime = 0;
        float deltaTimeAccumulator = 0;
        ViewSettings viewSettings;
        bool isDirty = true;

        filament::View* GetView();
        filament::Renderer* GetRenderer();

        void applySettings();
    };
}
#endif
