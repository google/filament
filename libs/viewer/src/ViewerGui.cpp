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

#include <viewer/ViewerGui.h>

#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/LightManager.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <filagui/ImGuiHelper.h>

#include <utils/EntityManager.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>

#include <string>
#include <vector>

using namespace filagui;
using namespace filament::math;

namespace filament {
namespace viewer {

mat4f fitIntoUnitCube(const Aabb& bounds, float zoffset) {
    float3 minpt = bounds.min;
    float3 maxpt = bounds.max;
    float maxExtent;
    maxExtent = std::max(maxpt.x - minpt.x, maxpt.y - minpt.y);
    maxExtent = std::max(maxExtent, maxpt.z - minpt.z);
    float scaleFactor = 2.0f / maxExtent;
    float3 center = (minpt + maxpt) / 2.0f;
    center.z += zoffset / scaleFactor;
    return mat4f::scaling(float3(scaleFactor)) * mat4f::translation(-center);
}

static void computeRangePlot(Settings& settings, float* rangePlot) {
    float4& ranges = settings.view.colorGrading.ranges;
    ranges.y = clamp(ranges.y, ranges.x + 1e-5f, ranges.w - 1e-5f); // darks
    ranges.z = clamp(ranges.z, ranges.x + 1e-5f, ranges.w - 1e-5f); // lights

    for (size_t i = 0; i < 1024; i++) {
        float x = i / 1024.0f;
        float s = 1.0f - smoothstep(ranges.x, ranges.y, x);
        float h = smoothstep(ranges.z, ranges.w, x);
        rangePlot[i]        = s;
        rangePlot[1024 + i] = 1.0f - s - h;
        rangePlot[2048 + i] = h;
    }
}

static void rangePlotSeriesStart(int series) {
    switch (series) {
        case 0:
            ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.4f, 0.25f, 1.0f));
            break;
        case 1:
            ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.8f, 0.25f, 1.0f));
            break;
        case 2:
            ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.17f, 0.21f, 1.0f));
            break;
    }
}

static void rangePlotSeriesEnd(int series) {
    if (series < 3) {
        ImGui::PopStyleColor();
    }
}

static float getRangePlotValue(int series, void* data, int index) {
    return ((float*) data)[series * 1024 + index];
}

inline float3 curves(float3 v, float3 shadowGamma, float3 midPoint, float3 highlightScale) {
    float3 d = 1.0f / (pow(midPoint, shadowGamma - 1.0f));
    float3 dark = pow(v, shadowGamma) * d;
    float3 light = highlightScale * (v - midPoint) + midPoint;
    return float3{
            v.r <= midPoint.r ? dark.r : light.r,
            v.g <= midPoint.g ? dark.g : light.g,
            v.b <= midPoint.b ? dark.b : light.b,
    };
}

static void computeCurvePlot(Settings& settings, float* curvePlot) {
    const auto& colorGradingOptions = settings.view.colorGrading;
    for (size_t i = 0; i < 1024; i++) {
        float3 x{i / 1024.0f * 2.0f};
        float3 y = curves(x,
                colorGradingOptions.gamma,
                colorGradingOptions.midPoint,
                colorGradingOptions.scale);
        curvePlot[i]        = y.r;
        curvePlot[1024 + i] = y.g;
        curvePlot[2048 + i] = y.b;
    }
}

static void computeToneMapPlot(ColorGradingSettings& settings, float* plot) {
    float hdrMax = 10.0f;
    ToneMapper* mapper;
    switch (settings.toneMapping) {
        case ToneMapping::LINEAR:
            mapper = new LinearToneMapper;
            break;
        case ToneMapping::ACES_LEGACY:
            mapper = new ACESLegacyToneMapper;
            break;
        case ToneMapping::ACES:
            mapper = new ACESToneMapper;
            break;
        case ToneMapping::FILMIC:
            mapper = new FilmicToneMapper;
            break;
        case ToneMapping::AGX:
            mapper = new AgxToneMapper(settings.agxToneMapper.look);
            break;
        case ToneMapping::GENERIC:
            mapper = new GenericToneMapper(
                    settings.genericToneMapper.contrast,
                    settings.genericToneMapper.midGrayIn,
                    settings.genericToneMapper.midGrayOut,
                    settings.genericToneMapper.hdrMax
            );
            hdrMax = settings.genericToneMapper.hdrMax;
            break;
        case ToneMapping::PBR_NEUTRAL:
            mapper = new PBRNeutralToneMapper;
            break;
        case ToneMapping::DISPLAY_RANGE:
            mapper = new DisplayRangeToneMapper;
            break;
    }

    float a = std::log10(hdrMax * 1.5f / 1e-6f);
    for (size_t i = 0; i < 1024; i++) {
        float v = i;
        float x = 1e-6f * std::pow(10.0f, a * v / 1023.0f);
        plot[i] = (*mapper)(x).r;
    }

    delete mapper;
}

static void tooltipFloat(float value) {
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%.2f", value);
    }
}

static void pushSliderColors(float hue) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4) ImColor::HSV(hue, 0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4) ImColor::HSV(hue, 0.6f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4) ImColor::HSV(hue, 0.7f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4) ImColor::HSV(hue, 0.9f, 0.9f));
}

static void popSliderColors() { ImGui::PopStyleColor(4); }

static void colorGradingUI(Settings& settings, float* rangePlot, float* curvePlot, float* toneMapPlot) {
    const static ImVec2 verticalSliderSize(18.0f, 160.0f);
    const static ImVec2 plotLinesSize(0.0f, 160.0f);
    const static ImVec2 plotLinesWideSize(0.0f, 120.0f);

    if (ImGui::CollapsingHeader("Color grading")) {
        ColorGradingSettings& colorGrading = settings.view.colorGrading;

        ImGui::Indent();
        ImGui::Checkbox("Enabled##colorGrading", &colorGrading.enabled);

        int quality = (int) colorGrading.quality;
        ImGui::Combo("Quality##colorGradingQuality", &quality, "Low\0Medium\0High\0Ultra\0\0");
        colorGrading.quality = (decltype(colorGrading.quality)) quality;

        int colorspace = (colorGrading.colorspace == Rec709-Linear-D65) ? 0 : 1;
        ImGui::Combo("Output color space", &colorspace, "Rec709-Linear-D65\0Rec709-sRGB-D65\0\0");
        colorGrading.colorspace = (colorspace == 0) ? Rec709-Linear-D65 : Rec709-sRGB-D65;

        int toneMapping = (int) colorGrading.toneMapping;
        ImGui::Combo("Tone-mapping", &toneMapping,
                "Linear\0ACES (legacy)\0ACES\0Filmic\0AgX\0Generic\0PBR Neutral\0Display Range\0\0");
        colorGrading.toneMapping = (decltype(colorGrading.toneMapping)) toneMapping;
        if (colorGrading.toneMapping == ToneMapping::GENERIC) {
            if (ImGui::CollapsingHeader("Tonemap parameters")) {
                GenericToneMapperSettings& generic = colorGrading.genericToneMapper;
                ImGui::SliderFloat("Contrast##genericToneMapper", &generic.contrast, 1e-5f, 3.0f);
                ImGui::SliderFloat("Mid-gray in##genericToneMapper", &generic.midGrayIn, 0.0f, 1.0f);
                ImGui::SliderFloat("Mid-gray out##genericToneMapper", &generic.midGrayOut, 0.0f, 1.0f);
                ImGui::SliderFloat("HDR max", &generic.hdrMax, 1.0f, 64.0f);
            }
        }
        if (colorGrading.toneMapping == ToneMapping::AGX) {
            int agxLook = (int) colorGrading.agxToneMapper.look;
            ImGui::Combo("AgX Look", &agxLook, "None\0Punchy\0Golden\0\0");
            colorGrading.agxToneMapper.look = (decltype(colorGrading.agxToneMapper.look)) agxLook;
        }

        computeToneMapPlot(colorGrading, toneMapPlot);

        ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.17f, 0.21f, 0.9f));
        ImGui::PlotLines("", toneMapPlot, 1024, 0, "Tone map", 0.0f, 1.05f, ImVec2(0, 160));
        ImGui::PopStyleColor();

        ImGui::Checkbox("Luminance scaling", &colorGrading.luminanceScaling);
        ImGui::Checkbox("Gamut mapping", &colorGrading.gamutMapping);

        ImGui::SliderFloat("Exposure", &colorGrading.exposure, -10.0f, 10.0f);
        ImGui::SliderFloat("Night adaptation", &colorGrading.nightAdaptation, 0.0f, 1.0f);

        if (ImGui::CollapsingHeader("White balance")) {
            int temperature = colorGrading.temperature * 100.0f;
            int tint = colorGrading.tint * 100.0f;
            ImGui::SliderInt("Temperature", &temperature, -100, 100);
            ImGui::SliderInt("Tint", &tint, -100, 100);
            colorGrading.temperature = temperature / 100.0f;
            colorGrading.tint = tint / 100.0f;
        }

        if (ImGui::CollapsingHeader("Channel mixer")) {
            pushSliderColors(0.0f / 7.0f);
            ImGui::VSliderFloat("##outRed.r", verticalSliderSize, &colorGrading.outRed.r, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outRed.r);
            ImGui::SameLine();
            ImGui::VSliderFloat("##outRed.g", verticalSliderSize, &colorGrading.outRed.g, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outRed.g);
            ImGui::SameLine();
            ImGui::VSliderFloat("##outRed.b", verticalSliderSize, &colorGrading.outRed.b, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outRed.b);
            ImGui::SameLine(0.0f, 18.0f);
            popSliderColors();

            pushSliderColors(2.0f / 7.0f);
            ImGui::VSliderFloat("##outGreen.r", verticalSliderSize, &colorGrading.outGreen.r, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outGreen.r);
            ImGui::SameLine();
            ImGui::VSliderFloat("##outGreen.g", verticalSliderSize, &colorGrading.outGreen.g, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outGreen.g);
            ImGui::SameLine();
            ImGui::VSliderFloat("##outGreen.b", verticalSliderSize, &colorGrading.outGreen.b, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outGreen.b);
            ImGui::SameLine(0.0f, 18.0f);
            popSliderColors();

            pushSliderColors(4.0f / 7.0f);
            ImGui::VSliderFloat("##outBlue.r", verticalSliderSize, &colorGrading.outBlue.r, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outBlue.r);
            ImGui::SameLine();
            ImGui::VSliderFloat("##outBlue.g", verticalSliderSize, &colorGrading.outBlue.g, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outBlue.g);
            ImGui::SameLine();
            ImGui::VSliderFloat("##outBlue.b", verticalSliderSize, &colorGrading.outBlue.b, -2.0f, 2.0f, "");
            tooltipFloat(colorGrading.outBlue.b);
            popSliderColors();
        }
        if (ImGui::CollapsingHeader("Tonal ranges")) {
            ImGui::ColorEdit3("Shadows", &colorGrading.shadows.x);
            ImGui::SliderFloat("Weight##shadowsWeight", &colorGrading.shadows.w, -2.0f, 2.0f);
            ImGui::ColorEdit3("Mid-tones", &colorGrading.midtones.x);
            ImGui::SliderFloat("Weight##midTonesWeight", &colorGrading.midtones.w, -2.0f, 2.0f);
            ImGui::ColorEdit3("Highlights", &colorGrading.highlights.x);
            ImGui::SliderFloat("Weight##highlightsWeight", &colorGrading.highlights.w, -2.0f, 2.0f);
            ImGui::SliderFloat4("Ranges", &colorGrading.ranges.x, 0.0f, 1.0f);
            computeRangePlot(settings, rangePlot);
            ImGuiExt::PlotLinesSeries("", 3,
                    rangePlotSeriesStart, getRangePlotValue, rangePlotSeriesEnd,
                    rangePlot, 1024, 0, "", 0.0f, 1.0f, plotLinesWideSize);
        }
        if (ImGui::CollapsingHeader("Color decision list")) {
            ImGui::SliderFloat3("Slope", &colorGrading.slope.x, 0.0f, 2.0f);
            ImGui::SliderFloat3("Offset", &colorGrading.offset.x, -0.5f, 0.5f);
            ImGui::SliderFloat3("Power", &colorGrading.power.x, 0.0f, 2.0f);
        }
        if (ImGui::CollapsingHeader("Adjustments")) {
            ImGui::SliderFloat("Contrast", &colorGrading.contrast, 0.0f, 2.0f);
            ImGui::SliderFloat("Vibrance", &colorGrading.vibrance, 0.0f, 2.0f);
            ImGui::SliderFloat("Saturation", &colorGrading.saturation, 0.0f, 2.0f);
        }
        if (ImGui::CollapsingHeader("Curves")) {
            ImGui::Checkbox("Linked curves", &colorGrading.linkedCurves);

            computeCurvePlot(settings, curvePlot);

            if (!colorGrading.linkedCurves) {
                pushSliderColors(0.0f / 7.0f);
                ImGui::VSliderFloat("##curveGamma.r", verticalSliderSize, &colorGrading.gamma.r, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.gamma.r);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveMid.r", verticalSliderSize, &colorGrading.midPoint.r, 0.0f, 2.0f, "");
                tooltipFloat(colorGrading.midPoint.r);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveScale.r", verticalSliderSize, &colorGrading.scale.r, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.scale.r);
                ImGui::SameLine(0.0f, 18.0f);
                popSliderColors();

                ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.0f, 0.7f, 0.8f));
                ImGui::PlotLines("", curvePlot, 1024, 0, "Red", 0.0f, 2.0f, plotLinesSize);
                ImGui::PopStyleColor();

                pushSliderColors(2.0f / 7.0f);
                ImGui::VSliderFloat("##curveGamma.g", verticalSliderSize, &colorGrading.gamma.g, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.gamma.g);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveMid.g", verticalSliderSize, &colorGrading.midPoint.g, 0.0f, 2.0f, "");
                tooltipFloat(colorGrading.midPoint.g);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveScale.g", verticalSliderSize, &colorGrading.scale.g, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.scale.g);
                ImGui::SameLine(0.0f, 18.0f);
                popSliderColors();

                ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.3f, 0.7f, 0.8f));
                ImGui::PlotLines("", curvePlot + 1024, 1024, 0, "Green", 0.0f, 2.0f, plotLinesSize);
                ImGui::PopStyleColor();

                pushSliderColors(4.0f / 7.0f);
                ImGui::VSliderFloat("##curveGamma.b", verticalSliderSize, &colorGrading.gamma.b, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.gamma.b);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveMid.b", verticalSliderSize, &colorGrading.midPoint.b, 0.0f, 2.0f, "");
                tooltipFloat(colorGrading.midPoint.b);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveScale.b", verticalSliderSize, &colorGrading.scale.b, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.scale.b);
                ImGui::SameLine(0.0f, 18.0f);
                popSliderColors();

                ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.6f, 0.7f, 0.8f));
                ImGui::PlotLines("", curvePlot + 2048, 1024, 0, "Blue", 0.0f, 2.0f, plotLinesSize);
                ImGui::PopStyleColor();
            } else {
                ImGui::VSliderFloat("##curveGamma", verticalSliderSize, &colorGrading.gamma.r, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.gamma.r);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveMid", verticalSliderSize, &colorGrading.midPoint.r, 0.0f, 2.0f, "");
                tooltipFloat(colorGrading.midPoint.r);
                ImGui::SameLine();
                ImGui::VSliderFloat("##curveScale", verticalSliderSize, &colorGrading.scale.r, 0.0f, 4.0f, "");
                tooltipFloat(colorGrading.scale.r);
                ImGui::SameLine(0.0f, 18.0f);

                colorGrading.gamma = float3{colorGrading.gamma.r};
                colorGrading.midPoint = float3{colorGrading.midPoint.r};
                colorGrading.scale = float3{colorGrading.scale.r};

                ImGui::PushStyleColor(ImGuiCol_PlotLines, (ImVec4) ImColor::HSV(0.17f, 0.21f, 0.9f));
                ImGui::PlotLines("", curvePlot, 1024, 0, "RGB", 0.0f, 2.0f, plotLinesSize);
                ImGui::PopStyleColor();
            }
        }
        ImGui::Unindent();
    }
}

ViewerGui::ViewerGui(filament::Engine* engine, filament::Scene* scene, filament::View* view,
        int sidebarWidth) :
        mEngine(engine), mScene(scene), mView(view),
        mSunlight(utils::EntityManager::get().create()),
        mSidebarWidth(sidebarWidth) {

    mSettings.view.shadowType = ShadowType::PCF;
    mSettings.view.vsmShadowOptions.anisotropy = 0;
    mSettings.view.dithering = Dithering::TEMPORAL;
    mSettings.view.antiAliasing = AntiAliasing::FXAA;
    mSettings.view.msaa = { .enabled = true, .sampleCount = 4 };
    mSettings.view.ssao.enabled = true;
    mSettings.view.bloom.enabled = true;

    DebugRegistry& debug = mEngine->getDebugRegistry();
    *debug.getPropertyAddress<bool>("d.stereo.combine_multiview_images") = true;

    using namespace filament;
    LightManager::Builder(LightManager::Type::SUN)
        .color(mSettings.lighting.sunlightColor)
        .intensity(mSettings.lighting.sunlightIntensity)
        .direction(normalize(mSettings.lighting.sunlightDirection))
        .castShadows(true)
        .sunAngularRadius(mSettings.lighting.sunlightAngularRadius)
        .sunHaloSize(mSettings.lighting.sunlightHaloSize)
        .sunHaloFalloff(mSettings.lighting.sunlightHaloFalloff)
        .build(*engine, mSunlight);
    if (mSettings.lighting.enableSunlight) {
        mScene->addEntity(mSunlight);
    }
    view->setAmbientOcclusionOptions({ .upsampling = View::QualityLevel::HIGH });
}

ViewerGui::~ViewerGui() {
    mEngine->destroy(mSunlight);
    delete mImGuiHelper;
}

void ViewerGui::setAsset(FilamentAsset* asset, FilamentInstance* instance) {
    if (mInstance != instance || mAsset != asset) {
        removeAsset();

        // We keep a non-const reference to the asset for popRenderables and getWireframe.
        mAsset = asset;
        mInstance = instance;

        mVisibleScenes.reset();
        mVisibleScenes.set(0);
        mCurrentCamera = 0;
        if (!mAsset) {
            mAsset = nullptr;
            mInstance = nullptr;
            return;
        }
        updateRootTransform();
        mScene->addEntities(asset->getLightEntities(), asset->getLightEntityCount());
        auto& rcm = mEngine->getRenderableManager();
        for (size_t i = 0, n = asset->getRenderableEntityCount(); i < n; i++) {
            auto ri = rcm.getInstance(asset->getRenderableEntities()[i]);
            rcm.setScreenSpaceContactShadows(ri, true);
        }
    }
}

void ViewerGui::populateScene() {
    static constexpr int kNumAvailable = 128;
    utils::Entity renderables[kNumAvailable];
    while (size_t numWritten = mAsset->popRenderables(renderables, kNumAvailable)) {
        mAsset->addEntitiesToScene(*mScene, renderables, numWritten, mVisibleScenes);
    }
}

void ViewerGui::removeAsset() {
    if (!isRemoteMode()) {
        mScene->removeEntities(mAsset->getEntities(), mAsset->getEntityCount());
        mAsset = nullptr;
    }
}

UTILS_NOINLINE
static bool notequal(float a, float b) noexcept {
    return a != b;
}

// we do this to circumvent -ffast-math ignoring NaNs
static bool is_not_a_number(float v) noexcept {
    return notequal(v, v);
}

void ViewerGui::setIndirectLight(filament::IndirectLight* ibl,
        filament::math::float3 const* sh3) {
    using namespace filament::math;
    mSettings.view.fog.color = sh3[0];
    mIndirectLight = ibl;
    if (ibl) {
        float3 const d = filament::IndirectLight::getDirectionEstimate(sh3);
        float4 const c = filament::IndirectLight::getColorEstimate(sh3, d);
        bool const dIsValid = std::none_of(std::begin(d.v), std::end(d.v), is_not_a_number);
        bool const cIsValid = std::none_of(std::begin(c.v), std::end(c.v), is_not_a_number);
        if (dIsValid && cIsValid) {
            mSettings.lighting.sunlightDirection = d;
            mSettings.lighting.sunlightColor = c.rgb;
            mSettings.lighting.sunlightIntensity = c[3] * ibl->getIntensity();
        }
    }
}

void ViewerGui::updateRootTransform() {
    if (isRemoteMode()) {
        return;
    }
    auto& tcm = mEngine->getTransformManager();
    auto root = tcm.getInstance(mAsset->getRoot());
    filament::math::mat4f transform;
    if (mSettings.viewer.autoScaleEnabled) {
        FilamentInstance* instance = mAsset->getInstance();
        Aabb aabb = instance ? instance->getBoundingBox() : mAsset->getBoundingBox();
        transform = fitIntoUnitCube(aabb, 4);
    }
    tcm.setTransform(root, transform);
}

void ViewerGui::sceneSelectionUI() {
    // Build a list of checkboxes, one for each glTF scene.
    bool changed = false;
    for (size_t i = 0, n = mAsset->getSceneCount(); i < n; ++i) {
        bool isVisible = mVisibleScenes.test(i);
        const char* name = mAsset->getSceneName(i);
        if (name) {
            changed = ImGui::Checkbox(name, &isVisible) || changed;
        } else {
            char label[16];
            snprintf(label, 16, "Scene %zu", i);
            changed = ImGui::Checkbox(label, &isVisible) || changed;
        }
        if (isVisible) {
            mVisibleScenes.set(i);
        } else {
            mVisibleScenes.unset(i);
        }
    }
    // If any checkboxes have been toggled, rebuild the scene list.
    if (changed) {
        const utils::Entity* entities = mAsset->getRenderableEntities();
        const size_t entityCount = mAsset->getRenderableEntityCount();
        mScene->removeEntities(entities, entityCount);
        mAsset->addEntitiesToScene(*mScene, entities, entityCount, mVisibleScenes);
    }
}

void ViewerGui::applyAnimation(double currentTime, FilamentInstance* instance) {
    instance = instance ? instance : mInstance;
    assert_invariant(!isRemoteMode());
    if (instance == nullptr) {
        return;
    }
    Animator& animator = *instance->getAnimator();
    const size_t animationCount = animator.getAnimationCount();
    if (mResetAnimation) {
        mPreviousStartTime = mCurrentStartTime;
        mCurrentStartTime = currentTime;
        mResetAnimation = false;
    }
    const double elapsedSeconds = currentTime - mCurrentStartTime;
    if (animationCount > 0 && mCurrentAnimation >= 0) {
        if (mCurrentAnimation == animationCount) {
            for (size_t i = 0; i < animationCount; i++) {
                animator.applyAnimation(i, elapsedSeconds);
            }
        } else {
            animator.applyAnimation(mCurrentAnimation, elapsedSeconds);
        }
        if (elapsedSeconds < mCrossFadeDuration && mPreviousAnimation >= 0 && mPreviousAnimation != animationCount) {
            const double previousSeconds = currentTime - mPreviousStartTime;
            const float lerpFactor = elapsedSeconds / mCrossFadeDuration;
            animator.applyCrossFade(mPreviousAnimation, previousSeconds, lerpFactor);
        }
    }
    if (mShowingRestPose) {
        animator.resetBoneMatrices();
    } else {
        animator.updateBoneMatrices();
    }
}

void ViewerGui::renderUserInterface(float timeStepInSeconds, View* guiView, float pixelRatio) {
    if (mImGuiHelper == nullptr) {
        mImGuiHelper = new ImGuiHelper(mEngine, guiView, "");

        auto& io = ImGui::GetIO();

        // The following table uses normal ANSI codes, which is consistent with the keyCode that
        // comes from a web "keydown" event.
        io.KeyMap[ImGuiKey_Tab] = 9;
        io.KeyMap[ImGuiKey_LeftArrow] = 37;
        io.KeyMap[ImGuiKey_RightArrow] = 39;
        io.KeyMap[ImGuiKey_UpArrow] = 38;
        io.KeyMap[ImGuiKey_DownArrow] = 40;
        io.KeyMap[ImGuiKey_Home] = 36;
        io.KeyMap[ImGuiKey_End] = 35;
        io.KeyMap[ImGuiKey_Delete] = 46;
        io.KeyMap[ImGuiKey_Backspace] = 8;
        io.KeyMap[ImGuiKey_Enter] = 13;
        io.KeyMap[ImGuiKey_Escape] = 27;
        io.KeyMap[ImGuiKey_A] = 65;
        io.KeyMap[ImGuiKey_C] = 67;
        io.KeyMap[ImGuiKey_V] = 86;
        io.KeyMap[ImGuiKey_X] = 88;
        io.KeyMap[ImGuiKey_Y] = 89;
        io.KeyMap[ImGuiKey_Z] = 90;

        // TODO: this is not the best way to handle high DPI in ImGui, but it is fine when using the
        // proggy font. Users need to refresh their window when dragging between displays with
        // different pixel ratios.
        io.FontGlobalScale = pixelRatio;
        ImGui::GetStyle().ScaleAllSizes(pixelRatio);
    }

    const auto size = guiView->getViewport();
    mImGuiHelper->setDisplaySize(size.width, size.height, 1, 1);

    mImGuiHelper->render(timeStepInSeconds, [this](Engine*, View*) {
        this->updateUserInterface();
    });
}

void ViewerGui::mouseEvent(float mouseX, float mouseY, bool mouseButton, float mouseWheelY,
        bool control) {
    if (mImGuiHelper) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x = mouseX;
        io.MousePos.y = mouseY;
        io.MouseWheel += mouseWheelY;
        io.MouseDown[0] = mouseButton != 0;
        io.MouseDown[1] = false;
        io.MouseDown[2] = false;
        io.KeyCtrl = control;
    }
}

void ViewerGui::keyDownEvent(int keyCode) {
    if (mImGuiHelper && keyCode < IM_ARRAYSIZE(ImGui::GetIO().KeysDown)) {
        ImGui::GetIO().KeysDown[keyCode] = true;
    }
}

void ViewerGui::keyUpEvent(int keyCode) {
    if (mImGuiHelper && keyCode < IM_ARRAYSIZE(ImGui::GetIO().KeysDown)) {
        ImGui::GetIO().KeysDown[keyCode] = false;
    }
}

void ViewerGui::keyPressEvent(int charCode) {
    if (mImGuiHelper) {
        ImGui::GetIO().AddInputCharacter(charCode);
    }
}

void ViewerGui::updateUserInterface() {
    using namespace filament;

    auto& tm = mEngine->getTransformManager();
    auto& rm = mEngine->getRenderableManager();
    auto& lm = mEngine->getLightManager();

    // Show a common set of UI widgets for all renderables.
    auto renderableTreeItem = [this, &rm](utils::Entity entity) {
        bool rvis = mScene->hasEntity(entity);
        ImGui::Checkbox("visible", &rvis);
        if (rvis) {
            mScene->addEntity(entity);
        } else {
            mScene->remove(entity);
        }
        auto instance = rm.getInstance(entity);
        bool scaster = rm.isShadowCaster(instance);
        bool sreceiver = rm.isShadowReceiver(instance);
        ImGui::Checkbox("casts shadows", &scaster);
        rm.setCastShadows(instance, scaster);
        ImGui::Checkbox("receives shadows", &sreceiver);
        rm.setReceiveShadows(instance, sreceiver);
        auto numMorphTargets = rm.getMorphTargetCount(instance);
        if (numMorphTargets > 0) {
            bool selected = entity == mCurrentMorphingEntity;
            ImGui::Checkbox("morphing", &selected);
            if (selected) {
                mCurrentMorphingEntity = entity;
                mMorphWeights.resize(numMorphTargets, 0.0f);
            } else {
                mCurrentMorphingEntity = utils::Entity();
            }
        }
        size_t numPrims = rm.getPrimitiveCount(instance);
        for (size_t prim = 0; prim < numPrims; ++prim) {
            const char* mname = rm.getMaterialInstanceAt(instance, prim)->getName();
            if (mname) {
                ImGui::Text("prim %zu: material %s", prim, mname);
            } else {
                ImGui::Text("prim %zu: (unnamed material)", prim);
            }
        }
    };

    auto lightTreeItem = [this, &lm](utils::Entity entity) {
        bool lvis = mScene->hasEntity(entity);
        ImGui::Checkbox("visible", &lvis);

        if (lvis) {
            mScene->addEntity(entity);
        } else {
            mScene->remove(entity);
        }

        auto instance = lm.getInstance(entity);
        bool lcaster = lm.isShadowCaster(instance);
        ImGui::Checkbox("shadow caster", &lcaster);
        lm.setShadowCaster(instance, lcaster);
    };

    // Declare a std::function for tree nodes, it's an easy way to make a recursive lambda.
    std::function<void(utils::Entity)> treeNode;

    treeNode = [&](utils::Entity entity) {
        auto tinstance = tm.getInstance(entity);
        auto rinstance = rm.getInstance(entity);
        auto linstance = lm.getInstance(entity);
        intptr_t treeNodeId = 1 + entity.getId();

        const char* name = mAsset->getName(entity);
        auto getLabel = [&name, &rinstance, &linstance]() {
            if (name) {
                return name;
            }
            if (rinstance) {
                return "Mesh";
            }
            if (linstance) {
                return "Light";
            }
            return "Node";
        };
        const char* label = getLabel();

        ImGuiTreeNodeFlags flags = 0; // rinstance ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
        std::vector<utils::Entity> children(tm.getChildCount(tinstance));
        if (ImGui::TreeNodeEx((const void*) treeNodeId, flags, "%s", label)) {
            if (rinstance) {
                renderableTreeItem(entity);
            }
            if (linstance) {
                lightTreeItem(entity);
            }
            tm.getChildren(tinstance, children.data(), children.size());
            for (auto ce : children) {
                treeNode(ce);
            }
            ImGui::TreePop();
        }
    };

    // Disable rounding and draw a fixed-height ImGui window that looks like a sidebar.
    ImGui::GetStyle().WindowRounding = 0;
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;
    ImGui::SetNextWindowSize(ImVec2(mSidebarWidth, height), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(20, height), ImVec2(width, height));

    ImGui::Begin("Filament", nullptr, ImGuiWindowFlags_NoTitleBar);
    if (mCustomUI) {
        mCustomUI();
    }

    DebugRegistry& debug = mEngine->getDebugRegistry();

    if (ImGui::CollapsingHeader("View")) {
        ImGui::Indent();

        ImGui::Checkbox("Post-processing", &mSettings.view.postProcessingEnabled);
        ImGui::Indent();
            bool dither = mSettings.view.dithering == Dithering::TEMPORAL;
            ImGui::Checkbox("Dithering", &dither);
            enableDithering(dither);
            ImGui::Checkbox("Bloom", &mSettings.view.bloom.enabled);
            ImGui::Checkbox("TAA", &mSettings.view.taa.enabled);
            // this clutters the UI and isn't that useful (except when working on TAA)
            //ImGui::Indent();
            //ImGui::SliderFloat("feedback", &mSettings.view.taa.feedback, 0.0f, 1.0f);
            //ImGui::SliderFloat("filter", &mSettings.view.taa.filterWidth, 0.0f, 2.0f);
            //ImGui::Unindent();

            bool fxaa = mSettings.view.antiAliasing == AntiAliasing::FXAA;
            ImGui::Checkbox("FXAA", &fxaa);
            enableFxaa(fxaa);
        ImGui::Unindent();

        ImGui::Checkbox("MSAA 4x", &mSettings.view.msaa.enabled);
        ImGui::Indent();
            ImGui::Checkbox("Custom resolve", &mSettings.view.msaa.customResolve);
        ImGui::Unindent();

        ImGui::Checkbox("SSAO", &mSettings.view.ssao.enabled);

        ImGui::Checkbox("Screen-space reflections", &mSettings.view.screenSpaceReflections.enabled);
        ImGui::Unindent();

        ImGui::Checkbox("Screen-space Guard Band", &mSettings.view.guardBand.enabled);
    }

    if (ImGui::CollapsingHeader("Bloom Options")) {
        ImGui::SliderFloat("Strength", &mSettings.view.bloom.strength, 0.0f, 1.0f);
        ImGui::Checkbox("Threshold", &mSettings.view.bloom.threshold);

        int levels = mSettings.view.bloom.levels;
        ImGui::SliderInt("Levels", &levels, 3, 11);
        mSettings.view.bloom.levels = levels;

        int quality = (int) mSettings.view.bloom.quality;
        ImGui::SliderInt("Bloom Quality", &quality, 0, 3);
        mSettings.view.bloom.quality = (View::QualityLevel) quality;

        ImGui::Checkbox("Lens Flare", &mSettings.view.bloom.lensFlare);
    }

    if (ImGui::CollapsingHeader("TAA Options")) {
        ImGui::Checkbox("Upscaling", &mSettings.view.taa.upscaling);
        ImGui::Checkbox("History Reprojection", &mSettings.view.taa.historyReprojection);
        ImGui::SliderFloat("Feedback", &mSettings.view.taa.feedback, 0.0f, 1.0f);
        ImGui::Checkbox("Filter History", &mSettings.view.taa.filterHistory);
        ImGui::Checkbox("Filter Input", &mSettings.view.taa.filterInput);
        ImGui::SliderFloat("FilterWidth", &mSettings.view.taa.filterWidth, 0.2f, 2.0f);
        ImGui::SliderFloat("LOD bias", &mSettings.view.taa.lodBias, -8.0f, 0.0f);
        ImGui::Checkbox("Use YCoCg", &mSettings.view.taa.useYCoCg);
        ImGui::Checkbox("Prevent Flickering", &mSettings.view.taa.preventFlickering);
        int jitterSequence = (int)mSettings.view.taa.jitterPattern;
        int boxClipping = (int)mSettings.view.taa.boxClipping;
        int boxType = (int)mSettings.view.taa.boxType;
        ImGui::Combo("Jitter Pattern", &jitterSequence, "RGSS x4\0Uniform Helix x4\0Halton x8\0Halton x16\0Halton x32\0\0");
        ImGui::Combo("Box Clipping", &boxClipping, "Accurate\0Clamp\0None\0\0");
        ImGui::Combo("Box Type", &boxType, "AABB\0Variance\0Both\0\0");
        ImGui::SliderFloat("Variance Gamma", &mSettings.view.taa.varianceGamma, 0.75f, 1.25f);
        ImGui::SliderFloat("RCAS", &mSettings.view.taa.sharpness, 0.0f, 1.0f);
        mSettings.view.taa.boxClipping = (TemporalAntiAliasingOptions::BoxClipping)boxClipping;
        mSettings.view.taa.boxType = (TemporalAntiAliasingOptions::BoxType)boxType;
        mSettings.view.taa.jitterPattern = (TemporalAntiAliasingOptions::JitterPattern)jitterSequence;
    }

    if (ImGui::CollapsingHeader("SSAO Options")) {
        auto& ssao = mSettings.view.ssao;

        int quality = (int) ssao.quality;
        int lowpass = (int) ssao.lowPassFilter;
        bool upsampling = ssao.upsampling != View::QualityLevel::LOW;

        bool halfRes = ssao.resolution != 1.0f;
        ImGui::SliderInt("Quality", &quality, 0, 3);
        ImGui::SliderInt("Low Pass", &lowpass, 0, 2);
        ImGui::Checkbox("Bent Normals", &ssao.bentNormals);
        ImGui::Checkbox("High quality upsampling", &upsampling);
        ImGui::SliderFloat("Min Horizon angle", &ssao.minHorizonAngleRad, 0.0f, (float)M_PI_4);
        ImGui::SliderFloat("Bilateral Threshold", &ssao.bilateralThreshold, 0.0f, 0.1f);
        ImGui::Checkbox("Half resolution", &halfRes);
        ssao.resolution = halfRes ? 0.5f : 1.0f;

        ssao.upsampling = upsampling ? View::QualityLevel::HIGH : View::QualityLevel::LOW;
        ssao.lowPassFilter = (View::QualityLevel) lowpass;
        ssao.quality = (View::QualityLevel) quality;

        if (ImGui::CollapsingHeader("Dominant Light Shadows (experimental)")) {
            int sampleCount = ssao.ssct.sampleCount;
            ImGui::Checkbox("Enabled##dls", &ssao.ssct.enabled);
            ImGui::SliderFloat("Cone angle", &ssao.ssct.lightConeRad, 0.0f, (float)M_PI_2);
            ImGui::SliderFloat("Shadow Distance", &ssao.ssct.shadowDistance, 0.0f, 10.0f);
            ImGui::SliderFloat("Contact dist max", &ssao.ssct.contactDistanceMax, 0.0f, 100.0f);
            ImGui::SliderFloat("Intensity##dls", &ssao.ssct.intensity, 0.0f, 10.0f);
            ImGui::SliderFloat("Depth bias", &ssao.ssct.depthBias, 0.0f, 1.0f);
            ImGui::SliderFloat("Depth slope bias", &ssao.ssct.depthSlopeBias, 0.0f, 1.0f);
            ImGui::SliderInt("Sample Count", &sampleCount, 1, 32);
            ImGuiExt::DirectionWidget("Direction##dls", ssao.ssct.lightDirection.v);
            ssao.ssct.sampleCount = sampleCount;
        }
    }

    if (ImGui::CollapsingHeader("Screen-space reflections Options")) {
        auto& ssrefl = mSettings.view.screenSpaceReflections;
        ImGui::SliderFloat("Ray thickness", &ssrefl.thickness, 0.001f, 0.2f);
        ImGui::SliderFloat("Bias", &ssrefl.bias, 0.001f, 0.5f);
        ImGui::SliderFloat("Max distance", &ssrefl.maxDistance, 0.1, 10.0f);
        ImGui::SliderFloat("Stride", &ssrefl.stride, 1.0, 10.0f);
    }

    if (ImGui::CollapsingHeader("Dynamic Resolution")) {
        auto& dsr = mSettings.view.dsr;
        int quality = (int)dsr.quality;
        ImGui::Checkbox("enabled", &dsr.enabled);
        ImGui::Checkbox("homogeneous", &dsr.homogeneousScaling);
        ImGui::SliderFloat("min. scale", &dsr.minScale.x, 0.25f, 1.0f);
        ImGui::SliderFloat("max. scale", &dsr.maxScale.x, 0.25f, 1.0f);
        ImGui::SliderInt("quality", &quality, 0, 3);
        ImGui::SliderFloat("sharpness", &dsr.sharpness, 0.0f, 1.0f);
        dsr.minScale.x = std::min(dsr.minScale.x, dsr.maxScale.x);
        dsr.minScale.y = dsr.minScale.x;
        dsr.maxScale.y = dsr.maxScale.x;
        dsr.quality = (QualityLevel)quality;
    }

    auto& light = mSettings.lighting;
    if (ImGui::CollapsingHeader("Light")) {
        ImGui::Indent();
        if (ImGui::CollapsingHeader("Indirect light")) {
            ImGui::SliderFloat("IBL intensity", &light.iblIntensity, 0.0f, 100000.0f);
            ImGui::SliderAngle("IBL rotation", &light.iblRotation);
        }
        if (ImGui::CollapsingHeader("Sunlight")) {
            ImGui::Checkbox("Enable sunlight", &light.enableSunlight);
            ImGui::SliderFloat("Sun intensity", &light.sunlightIntensity, 0.0f, 150000.0f);
            ImGui::SliderFloat("Halo size", &light.sunlightHaloSize, 1.01f, 40.0f);
            ImGui::SliderFloat("Halo falloff", &light.sunlightHaloFalloff, 4.0f, 1024.0f);
            ImGui::SliderFloat("Sun radius", &light.sunlightAngularRadius, 0.1f, 10.0f);
            ImGuiExt::DirectionWidget("Sun direction", light.sunlightDirection.v);
            ImGui::SliderFloat("Shadow Far", &light.shadowOptions.shadowFar, 0.0f,
                    mSettings.viewer.cameraFar);

            if (ImGui::CollapsingHeader("Shadow direction")) {
                float3 shadowDirection = light.shadowOptions.transform * light.sunlightDirection;
                ImGuiExt::DirectionWidget("Shadow direction", shadowDirection.v);
                light.shadowOptions.transform = normalize(quatf{
                        cross(light.sunlightDirection, shadowDirection),
                        sqrt(length2(light.sunlightDirection) * length2(shadowDirection))
                        + dot(light.sunlightDirection, shadowDirection)
                });
            }
        }
        if (ImGui::CollapsingHeader("Shadows")) {
            ImGui::Checkbox("Enable shadows", &light.enableShadows);
            int mapSize = light.shadowOptions.mapSize;
            ImGui::SliderInt("Shadow map size", &mapSize, 32, 1024);
            light.shadowOptions.mapSize = mapSize;
            ImGui::Checkbox("Stable Shadows", &light.shadowOptions.stable);
            ImGui::Checkbox("Enable LiSPSM", &light.shadowOptions.lispsm);

            int shadowType = (int)mSettings.view.shadowType;
            ImGui::Combo("Shadow type", &shadowType, "PCF\0VSM\0DPCF\0PCSS\0PCFd\0\0");
            mSettings.view.shadowType = (ShadowType)shadowType;

            if (mSettings.view.shadowType == ShadowType::VSM) {
                ImGui::Checkbox("High precision", &mSettings.view.vsmShadowOptions.highPrecision);
                ImGui::Checkbox("ELVSM", &mSettings.lighting.shadowOptions.vsm.elvsm);
                char label[32];
                snprintf(label, 32, "%d", 1 << mVsmMsaaSamplesLog2);
                ImGui::SliderInt("VSM MSAA samples", &mVsmMsaaSamplesLog2, 0, 3, label);
                mSettings.view.vsmShadowOptions.msaaSamples =
                        static_cast<uint8_t>(1u << mVsmMsaaSamplesLog2);

                int vsmAnisotropy = mSettings.view.vsmShadowOptions.anisotropy;
                snprintf(label, 32, "%d", 1 << vsmAnisotropy);
                ImGui::SliderInt("VSM anisotropy", &vsmAnisotropy, 0, 3, label);
                mSettings.view.vsmShadowOptions.anisotropy = vsmAnisotropy;
                ImGui::Checkbox("VSM mipmapping", &mSettings.view.vsmShadowOptions.mipmapping);
                ImGui::SliderFloat("VSM blur", &light.shadowOptions.vsm.blurWidth, 0.0f, 125.0f);

                // These are not very useful in practice (defaults are good), but we keep them here for debugging
                //ImGui::SliderFloat("VSM exponent", &mSettings.view.vsmShadowOptions.exponent, 0.0, 6.0f);
                //ImGui::SliderFloat("VSM Light bleed", &mSettings.view.vsmShadowOptions.lightBleedReduction, 0.0, 1.0f);
                //ImGui::SliderFloat("VSM min variance scale", &mSettings.view.vsmShadowOptions.minVarianceScale, 0.0, 10.0f);
            } else if (mSettings.view.shadowType == ShadowType::DPCF || mSettings.view.shadowType == ShadowType::PCSS) {
                ImGui::SliderFloat("Penumbra scale", &light.softShadowOptions.penumbraScale, 0.0f, 100.0f);
                ImGui::SliderFloat("Penumbra Ratio scale", &light.softShadowOptions.penumbraRatioScale, 1.0f, 100.0f);
            }

            int shadowCascades = light.shadowOptions.shadowCascades;
            ImGui::SliderInt("Cascades", &shadowCascades, 1, 4);
            ImGui::Checkbox("Debug cascades",
                    debug.getPropertyAddress<bool>("d.shadowmap.visualize_cascades"));
            ImGui::Checkbox("Enable contact shadows", &light.shadowOptions.screenSpaceContactShadows);
            ImGui::SliderFloat("Split pos 0", &light.shadowOptions.cascadeSplitPositions[0], 0.0f, 1.0f);
            ImGui::SliderFloat("Split pos 1", &light.shadowOptions.cascadeSplitPositions[1], 0.0f, 1.0f);
            ImGui::SliderFloat("Split pos 2", &light.shadowOptions.cascadeSplitPositions[2], 0.0f, 1.0f);
            light.shadowOptions.shadowCascades = shadowCascades;
        }
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Fog")) {
        int fogColorSource = 0;
        if (mSettings.view.fog.skyColor) {
            fogColorSource = 2;
        } else if (mSettings.view.fog.fogColorFromIbl) {
            fogColorSource = 1;
        }

        bool excludeSkybox = !std::isinf(mSettings.view.fog.cutOffDistance);
        ImGui::Indent();
        ImGui::Checkbox("Enable large-scale fog", &mSettings.view.fog.enabled);
        ImGui::SliderFloat("Start [m]", &mSettings.view.fog.distance, 0.0f, 100.0f);
        ImGui::SliderFloat("Extinction [1/m]", &mSettings.view.fog.density, 0.0f, 1.0f);
        ImGui::SliderFloat("Floor [m]", &mSettings.view.fog.height, 0.0f, 100.0f);
        ImGui::SliderFloat("Height falloff [1/m]", &mSettings.view.fog.heightFalloff, 0.0f, 4.0f);
        ImGui::SliderFloat("Sun Scattering start [m]", &mSettings.view.fog.inScatteringStart, 0.0f, 100.0f);
        ImGui::SliderFloat("Sun Scattering size", &mSettings.view.fog.inScatteringSize, 0.1f, 100.0f);
        ImGui::Checkbox("Exclude Skybox", &excludeSkybox);
        ImGui::Combo("Color##fogColor", &fogColorSource, "Constant\0IBL\0Skybox\0\0");
        ImGui::ColorPicker3("Color", mSettings.view.fog.color.v);
        ImGui::Unindent();
        mSettings.view.fog.cutOffDistance =
                excludeSkybox ? 1e6f : std::numeric_limits<float>::infinity();
        switch (fogColorSource) {
            case 0:
                mSettings.view.fog.skyColor = nullptr;
                mSettings.view.fog.fogColorFromIbl = false;
                break;
            case 1:
                mSettings.view.fog.skyColor = nullptr;
                mSettings.view.fog.fogColorFromIbl = true;
                break;
            case 2:
                mSettings.view.fog.skyColor = mSettings.view.fogSettings.fogColorTexture;
                mSettings.view.fog.fogColorFromIbl = false;
                break;
        }
    }

    if (ImGui::CollapsingHeader("Scene")) {
        ImGui::Indent();

        if (ImGui::Checkbox("Scale to unit cube", &mSettings.viewer.autoScaleEnabled)) {
            updateRootTransform();
        }

        ImGui::Checkbox("Automatic instancing", &mSettings.viewer.autoInstancingEnabled);

        ImGui::Checkbox("Show skybox", &mSettings.viewer.skyboxEnabled);
        ImGui::ColorEdit3("Background color", &mSettings.viewer.backgroundColor.r);

        // We do not yet support ground shadow or scene selection in remote mode.
        if (!isRemoteMode()) {
            ImGui::Checkbox("Ground shadow", &mSettings.viewer.groundPlaneEnabled);
            ImGui::Indent();
            ImGui::SliderFloat("Strength", &mSettings.viewer.groundShadowStrength, 0.0f, 1.0f);
            ImGui::Unindent();

            if (mAsset->getSceneCount() > 1) {
                ImGui::Separator();
                sceneSelectionUI();
            }
        }

        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Indent();

        ImGui::SliderFloat("Focal length (mm)", &mSettings.viewer.cameraFocalLength, 16.0f, 90.0f);
        ImGui::SliderFloat("Aperture", &mSettings.viewer.cameraAperture, 1.0f, 32.0f);
        ImGui::SliderFloat("Speed (1/s)", &mSettings.viewer.cameraSpeed, 1000.0f, 1.0f);
        ImGui::SliderFloat("ISO", &mSettings.viewer.cameraISO, 25.0f, 6400.0f);
        ImGui::SliderFloat("Near", &mSettings.viewer.cameraNear, 0.001f, 1.0f);
        ImGui::SliderFloat("Far", &mSettings.viewer.cameraFar, 1.0f, 10000.0f);

        if (ImGui::CollapsingHeader("DoF")) {
            bool dofMedian = mSettings.view.dof.filter == View::DepthOfFieldOptions::Filter::MEDIAN;
            int dofRingCount = mSettings.view.dof.fastGatherRingCount;
            int dofMaxCoC = mSettings.view.dof.maxForegroundCOC;
            if (!dofRingCount) dofRingCount = 5;
            if (!dofMaxCoC) dofMaxCoC = 32;
            ImGui::Checkbox("Enabled##dofEnabled", &mSettings.view.dof.enabled);
            ImGui::SliderFloat("Focus distance", &mSettings.viewer.cameraFocusDistance, 0.0f, 30.0f);
            ImGui::SliderFloat("Blur scale", &mSettings.view.dof.cocScale, 0.1f, 10.0f);
            ImGui::SliderFloat("CoC aspect-ratio", &mSettings.view.dof.cocAspectRatio, 0.25f, 4.0f);
            ImGui::SliderInt("Ring count", &dofRingCount, 1, 17);
            ImGui::SliderInt("Max CoC", &dofMaxCoC, 1, 32);
            ImGui::Checkbox("Native Resolution", &mSettings.view.dof.nativeResolution);
            ImGui::Checkbox("Median Filter", &dofMedian);
            mSettings.view.dof.filter = dofMedian ?
                                        View::DepthOfFieldOptions::Filter::MEDIAN :
                                        View::DepthOfFieldOptions::Filter::NONE;
            mSettings.view.dof.backgroundRingCount = dofRingCount;
            mSettings.view.dof.foregroundRingCount = dofRingCount;
            mSettings.view.dof.fastGatherRingCount = dofRingCount;
            mSettings.view.dof.maxForegroundCOC = dofMaxCoC;
            mSettings.view.dof.maxBackgroundCOC = dofMaxCoC;
        }

        if (ImGui::CollapsingHeader("Vignette")) {
            ImGui::Checkbox("Enabled##vignetteEnabled", &mSettings.view.vignette.enabled);
            ImGui::SliderFloat("Mid point", &mSettings.view.vignette.midPoint, 0.0f, 1.0f);
            ImGui::SliderFloat("Roundness", &mSettings.view.vignette.roundness, 0.0f, 1.0f);
            ImGui::SliderFloat("Feather", &mSettings.view.vignette.feather, 0.0f, 1.0f);
            ImGui::ColorEdit3("Color##vignetteColor", &mSettings.view.vignette.color.r);
        }

        // We do not yet support camera selection in the remote UI. To support this feature, we
        // would need to send a message from DebugServer to the WebSockets client.
        if (!isRemoteMode()) {

            const utils::Entity* cameras = mAsset->getCameraEntities();
            const size_t cameraCount = mAsset->getCameraEntityCount();

            std::vector<std::string> names;
            names.reserve(cameraCount + 1);
            names.emplace_back("Free camera");
            int c = 0;
            for (size_t i = 0; i < cameraCount; i++) {
                const char* n = mAsset->getName(cameras[i]);
                if (n) {
                    names.emplace_back(n);
                } else {
                    char buf[32];
                    sprintf(buf, "Unnamed camera %d", c++);
                    names.emplace_back(buf);
                }
            }

            std::vector<const char*> cstrings;
            cstrings.reserve(names.size());
            for (const auto & name : names) {
                cstrings.push_back(name.c_str());
            }

            ImGui::ListBox("Cameras", &mCurrentCamera, cstrings.data(), cstrings.size());
        }

#if defined(FILAMENT_SAMPLES_STEREO_TYPE_INSTANCED)                                                \
        || defined(FILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
        ImGui::Checkbox("Stereo mode", &mSettings.view.stereoscopicOptions.enabled);
#if defined(FILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
        ImGui::Indent();
        ImGui::Checkbox("Combine Multiview Images",
                debug.getPropertyAddress<bool>("d.stereo.combine_multiview_images"));
        ImGui::Unindent();
#endif
#endif
        ImGui::SliderFloat("Ocular distance", &mSettings.viewer.cameraEyeOcularDistance, 0.0f,
                1.0f);

        float toeInDegrees = mSettings.viewer.cameraEyeToeIn / f::PI * 180.0f;
        ImGui::SliderFloat("Toe in", &toeInDegrees, 0.0f, 30.0, "%.3f°");
        mSettings.viewer.cameraEyeToeIn = toeInDegrees / 180.0f * f::PI;

        ImGui::Unindent();
    }

    colorGradingUI(mSettings, mRangePlot, mCurvePlot, mToneMapPlot);

    // At this point, all View settings have been modified,
    //  so we can now push them into the Filament View.
    applySettings(mEngine, mSettings.view, mView);

    auto lights = utils::FixedCapacityVector<utils::Entity>::with_capacity(mScene->getEntityCount());
    mScene->forEach([&](utils::Entity entity) {
        if (lm.hasComponent(entity)) {
            lights.push_back(entity);
        }
    });

    applySettings(mEngine, mSettings.lighting, mIndirectLight, mSunlight,
            lights.data(), lights.size(), &lm, mScene, mView);

    // TODO(prideout): add support for hierarchy, animation and variant selection in remote mode. To
    // support these features, we will need to send a message (list of strings) from DebugServer to
    // the WebSockets client.
    if (!isRemoteMode()) {
        if (ImGui::CollapsingHeader("Hierarchy")) {
            ImGui::Indent();
            ImGui::Checkbox("Show bounds", &mEnableWireframe);
            treeNode(mAsset->getRoot());
            ImGui::Unindent();
        }

        if (mInstance->getMaterialVariantCount() > 0 && ImGui::CollapsingHeader("Variants")) {
            ImGui::Indent();
            int selectedVariant = mCurrentVariant;
            for (size_t i = 0, count = mInstance->getMaterialVariantCount(); i < count; ++i) {
                const char* label = mInstance->getMaterialVariantName(i);
                ImGui::RadioButton(label, &selectedVariant, i);
            }
            if (selectedVariant != mCurrentVariant) {
                mCurrentVariant = selectedVariant;
                mInstance->applyMaterialVariant(mCurrentVariant);
            }
            ImGui::Unindent();
        }

        Animator& animator = *mInstance->getAnimator();
        const size_t animationCount = animator.getAnimationCount();
        if (animationCount > 0 && ImGui::CollapsingHeader("Animation")) {
            ImGui::Indent();
            int selectedAnimation = mCurrentAnimation;
            ImGui::RadioButton("Disable", &selectedAnimation, -1);
            ImGui::RadioButton("Apply all animations", &selectedAnimation, animationCount);
            ImGui::SliderFloat("Cross fade", &mCrossFadeDuration, 0.0f, 2.0f,
                    "%4.2f seconds", ImGuiSliderFlags_AlwaysClamp);
            for (size_t i = 0; i < animationCount; ++i) {
                std::string label = animator.getAnimationName(i);
                if (label.empty()) {
                    label = "Unnamed " + std::to_string(i);
                }
                ImGui::RadioButton(label.c_str(), &selectedAnimation, i);
            }
            if (selectedAnimation != mCurrentAnimation) {
                mPreviousAnimation = mCurrentAnimation;
                mCurrentAnimation = selectedAnimation;
                mResetAnimation = true;
            }
            ImGui::Checkbox("Show rest pose", &mShowingRestPose);
            ImGui::Unindent();
        }

        if (mCurrentMorphingEntity && ImGui::CollapsingHeader("Morphing")) {
            const bool isAnimating = mCurrentAnimation > 0 && animator.getAnimationCount() > 0;
            if (isAnimating) {
                ImGui::BeginDisabled();
            }
            for (int i = 0; i != mMorphWeights.size(); ++i) {
                const char* name = mAsset->getMorphTargetNameAt(mCurrentMorphingEntity, i);
                std::string label = name ? name : "Unnamed target " + std::to_string(i);
                ImGui::SliderFloat(label.c_str(), &mMorphWeights[i], 0.0f, 1.0);
            }
            if (isAnimating) {
                ImGui::EndDisabled();
            }
            if (!isAnimating) {
                auto instance = rm.getInstance(mCurrentMorphingEntity);
                rm.setMorphWeights(instance, mMorphWeights.data(), mMorphWeights.size());
            }
        }

        if (mEnableWireframe) {
            mScene->addEntity(mAsset->getWireframe());
        } else {
            mScene->remove(mAsset->getWireframe());
        }
    }

    mSidebarWidth = ImGui::GetWindowWidth();
    ImGui::End();
}

} // namespace viewer
} // namespace filament
