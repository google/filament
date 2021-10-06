/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef VIEWER_SETTINGS_H
#define VIEWER_SETTINGS_H

#include <filament/ColorGrading.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/MaterialInstance.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <utils/compiler.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <stddef.h>
#include <stdint.h>

#include <string>

namespace filament {

class Skybox;
class Renderer;

namespace viewer {

struct ColorGradingSettings;
struct DynamicLightingSettings;
struct MaterialSettings;
struct Settings;
struct ViewSettings;
struct LightSettings;
struct ViewerOptions;

enum class ToneMapping : uint8_t {
    LINEAR        = 0,
    ACES_LEGACY   = 1,
    ACES          = 2,
    FILMIC        = 3,
    GENERIC       = 4,
    DISPLAY_RANGE = 5,
};

using AmbientOcclusionOptions = filament::View::AmbientOcclusionOptions;
using AntiAliasing = filament::View::AntiAliasing;
using BloomOptions = filament::View::BloomOptions;
using DepthOfFieldOptions = filament::View::DepthOfFieldOptions;
using Dithering = filament::View::Dithering;
using FogOptions = filament::View::FogOptions;
using RenderQuality = filament::View::RenderQuality;
using ShadowType = filament::View::ShadowType;
using DynamicResolutionOptions = filament::View::DynamicResolutionOptions;
using MultiSampleAntiAliasingOptions = filament::View::MultiSampleAntiAliasingOptions;
using TemporalAntiAliasingOptions = filament::View::TemporalAntiAliasingOptions;
using VignetteOptions = filament::View::VignetteOptions;
using VsmShadowOptions = filament::View::VsmShadowOptions;
using LightManager = filament::LightManager;

// These functions push all editable property values to their respective Filament objects.
void applySettings(const ViewSettings& settings, View* dest);
void applySettings(const MaterialSettings& settings, MaterialInstance* dest);
void applySettings(const LightSettings& settings, IndirectLight* ibl, utils::Entity sunlight,
        utils::Entity* sceneLights, size_t sceneLightCount, LightManager* lm, Scene* scene);
void applySettings(const ViewerOptions& settings, Camera* camera, Skybox* skybox,
        Renderer* renderer);

// Creates a new ColorGrading object based on the given settings.
UTILS_PUBLIC
ColorGrading* createColorGrading(const ColorGradingSettings& settings, Engine* engine);

class UTILS_PUBLIC JsonSerializer {
public:
    JsonSerializer();
    ~JsonSerializer();

    // Writes a human-readable JSON string into an internal buffer and returns the result.
    const std::string& writeJson(const Settings& in);

    // Reads the given JSON blob and updates the corresponding fields in the given Settings object.
    // - The given JSON blob need not specify all settings.
    // - Returns true if successful.
    // - This function writes warnings and error messages into the utils log.
    bool readJson(const char* jsonChunk, size_t size, Settings* out);

private:
    class Context;
    Context* context;
};

struct GenericToneMapperSettings {
    float contrast = 1.585f;
    float shoulder = 0.5f;
    float midGrayIn = 0.18f;
    float midGrayOut = 0.268f;
    float hdrMax = 10.0f;
    bool operator!=(const GenericToneMapperSettings &rhs) const { return !(rhs == *this); }
    bool operator==(const GenericToneMapperSettings &rhs) const;
};

struct ColorGradingSettings {
    bool enabled = true;
    filament::ColorGrading::QualityLevel quality = filament::ColorGrading::QualityLevel::MEDIUM;
    ToneMapping toneMapping = ToneMapping::ACES_LEGACY;
    GenericToneMapperSettings genericToneMapper;
    bool luminanceScaling = false;
    bool gamutMapping = false;
    float exposure = 0.0f;
    float nightAdaptation = 0.0f;
    float temperature = 0.0f;
    float tint = 0.0f;
    math::float3 outRed{1.0f, 0.0f, 0.0f};
    math::float3 outGreen{0.0f, 1.0f, 0.0f};
    math::float3 outBlue{0.0f, 0.0f, 1.0f};
    math::float4 shadows{1.0f, 1.0f, 1.0f, 0.0f};
    math::float4 midtones{1.0f, 1.0f, 1.0f, 0.0f};
    math::float4 highlights{1.0f, 1.0f, 1.0f, 0.0f};
    math::float4 ranges{0.0f, 0.333f, 0.550f, 1.0f};
    float contrast = 1.0f;
    float vibrance = 1.0f;
    float saturation = 1.0f;
    math::float3 slope{1.0f};
    math::float3 offset{0.0f};
    math::float3 power{1.0f};
    math::float3 gamma{1.0f};
    math::float3 midPoint{1.0f};
    math::float3 scale{1.0f};
    bool linkedCurves = false;
    bool operator!=(const ColorGradingSettings &rhs) const { return !(rhs == *this); }
    bool operator==(const ColorGradingSettings &rhs) const;
};

struct DynamicLightingSettings {
    float zLightNear = 5;
    float zLightFar = 100;
};

// This defines fields in the same order as the setter methods in filament::View.
struct ViewSettings {
    AntiAliasing antiAliasing = AntiAliasing::FXAA;
    MultiSampleAntiAliasingOptions msaa;
    TemporalAntiAliasingOptions taa;
    ColorGradingSettings colorGrading;
    AmbientOcclusionOptions ssao;
    DynamicResolutionOptions dsr;
    BloomOptions bloom;
    FogOptions fog;
    DepthOfFieldOptions dof;
    VignetteOptions vignette;
    Dithering dithering = Dithering::TEMPORAL;
    RenderQuality renderQuality;
    DynamicLightingSettings dynamicLighting;
    ShadowType shadowType = ShadowType::PCF;
    VsmShadowOptions vsmShadowOptions;
    bool postProcessingEnabled = true;
};

template <typename T>
struct MaterialProperty { std::string name; T value; };

// This struct has a fixed size for simplicity. Each non-empty property name is an override.
struct MaterialSettings {
    static constexpr size_t MAX_COUNT = 4;
    MaterialProperty<float> scalar[MAX_COUNT];
    MaterialProperty<math::float3> float3[MAX_COUNT];
    MaterialProperty<math::float4> float4[MAX_COUNT];
};

struct LightSettings {
    bool enableShadows = true;
    bool enableSunlight = true;
    LightManager::ShadowOptions shadowOptions;
    float sunlightIntensity = 100000.0f;
    math::float3 sunlightDirection = {0.6, -1.0, -0.8};;
    math::float3 sunlightColor = filament::Color::toLinear<filament::ACCURATE>({ 0.98, 0.92, 0.89});
    float iblIntensity = 30000.0f;
    float iblRotation = 0.0f;
};

struct ViewerOptions {
    float cameraAperture = 16.0f;
    float cameraSpeed = 125.0f;
    float cameraISO = 100.0f;
    float groundShadowStrength = 0.75f;
    bool groundPlaneEnabled = false;
    bool skyboxEnabled = true;
    sRGBColor backgroundColor = { 0.0f };
    float cameraFocalLength = 28.0f;
    float cameraFocusDistance = 10.0f;
    bool autoScaleEnabled = true;
};

struct Settings {
    ViewSettings view;
    MaterialSettings material;
    LightSettings lighting;
    ViewerOptions viewer;
};

} // namespace viewer
} // namespace filament

#endif // VIEWER_SETTINGS_H
