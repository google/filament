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
#include <filament/MaterialInstance.h>
#include <filament/View.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <stddef.h>
#include <stdint.h>

#include <string>

namespace filament {
namespace viewer {

struct ColorGradingSettings;
struct DynamicLightingSettings;
struct MaterialSettings;
struct Settings;
struct ViewSettings;

using AmbientOcclusionOptions = filament::View::AmbientOcclusionOptions;
using AntiAliasing = filament::View::AntiAliasing;
using BloomOptions = filament::View::BloomOptions;
using DepthOfFieldOptions = filament::View::DepthOfFieldOptions;
using Dithering = filament::View::Dithering;
using FogOptions = filament::View::FogOptions;
using RenderQuality = filament::View::RenderQuality;
using ShadowType = filament::View::ShadowType;
using TemporalAntiAliasingOptions = filament::View::TemporalAntiAliasingOptions;
using ToneMapping = filament::ColorGrading::ToneMapping;
using VignetteOptions = filament::View::VignetteOptions;
using VsmShadowOptions = filament::View::VsmShadowOptions;

// Reads the given JSON blob and updates the corresponding fields in the given Settings object.
// - The given JSON blob need not specify all settings.
// - Returns true if successful.
// - This function writes warnings and error messages into the utils log.
bool readJson(const char* jsonChunk, size_t size, Settings* out);

// These functions push all editable property values to their respective Filament objects.
void applySettings(const ViewSettings& settings, View* dest);
void applySettings(const MaterialSettings& settings, MaterialInstance* dest);

// Creates a new ColorGrading object based on the given settings.
ColorGrading* createColorGrading(const ColorGradingSettings& settings, Engine* engine);

// Generates human-readable JSON strings from settings objects.
std::string writeJson(const AmbientOcclusionOptions& in);
std::string writeJson(const BloomOptions& in);
std::string writeJson(const ColorGradingSettings& in);
std::string writeJson(const DepthOfFieldOptions& in);
std::string writeJson(const DynamicLightingSettings& in);
std::string writeJson(const FogOptions& in);
std::string writeJson(const MaterialSettings& in);
std::string writeJson(const RenderQuality& in);
std::string writeJson(const Settings& in);
std::string writeJson(const TemporalAntiAliasingOptions& in);
std::string writeJson(const ViewSettings& in);
std::string writeJson(const VignetteOptions& in);

struct ColorGradingSettings {
    bool enabled = true;
    filament::ColorGrading::QualityLevel quality = filament::ColorGrading::QualityLevel::MEDIUM;
    ToneMapping toneMapping = ToneMapping::ACES_LEGACY;
    float temperature = 0;
    float tint = 0;
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
    uint8_t sampleCount = 1;
    AntiAliasing antiAliasing = AntiAliasing::FXAA;
    TemporalAntiAliasingOptions taa;
    ColorGradingSettings colorGrading;
    AmbientOcclusionOptions ssao;
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

struct Settings {
    ViewSettings view;
    MaterialSettings material;
};

} // namespace viewer
} // namespace filament

#endif // VIEWER_SETTINGS_H
