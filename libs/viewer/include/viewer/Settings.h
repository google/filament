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

#include <stddef.h>
#include <stdint.h>

#include <string>

#include <filament/ColorGrading.h>
#include <filament/View.h>

namespace filament {
namespace viewer {

struct ColorGradingSettings;
struct DynamicLightingSettings;
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

// Reads the given JSON blob and updates the corresponding fields in the given Settings object.
// - The given JSON blob need not specify all settings.
// - Returns true if successful.
// - This function writes warnings and error messages into the utils log.
bool readJson(const char* jsonChunk, size_t size, Settings* out);

// Generates human-readable JSON strings from settings objects.
std::string writeJson(const AmbientOcclusionOptions& out);
std::string writeJson(const BloomOptions& out);
std::string writeJson(const ColorGradingSettings& out);
std::string writeJson(const DepthOfFieldOptions& out);
std::string writeJson(const DynamicLightingSettings& out);
std::string writeJson(const FogOptions& out);
std::string writeJson(const RenderQuality& out);
std::string writeJson(const Settings& out);
std::string writeJson(const TemporalAntiAliasingOptions& out);
std::string writeJson(const ViewSettings& out);
std::string writeJson(const VignetteOptions& out);

struct ColorGradingSettings {
    bool enabled = true;
    filament::ColorGrading::QualityLevel quality = filament::ColorGrading::QualityLevel::MEDIUM;
    ToneMapping toneMapping = ToneMapping::ACES_LEGACY;
    int temperature = 0;
    int tint = 0;
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
    bool postProcessingEnabled = true;
};

struct Settings {
    ViewSettings view;
};

} // namespace viewer
} // namespace filament

#endif // VIEWER_SETTINGS_H
