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

#include <iostream>
#include <fstream>

#include <viewer/SimpleViewer.h>
#include <viewer/CustomFileDialogs.h>

#include <filament/RenderableManager.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/LightManager.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <utils/Path.h>

#include <filagui/ImGuiHelper.h>

#include <utils/EntityManager.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>

#include <cctype>
#include <string>
#include <vector>

#include <viewer/json.hpp>

#include "stb.h"
#include "stb_image.h"

using namespace filagui;
using namespace filament::math;

std::string g_ArtRootPathStr {};

namespace filament {
namespace viewer {

// Taken from MeshAssimp.cpp
static void loadTexture(Engine* engine, const std::string& filePath, Texture** map,
    bool sRGB, bool hasAlpha, const std::string& artRootPathStr) {

    std::cout << "Loading texture \"" << filePath << "\" relative to \"" << artRootPathStr << "\"" << std::endl;

    if (!filePath.empty()) {
        utils::Path path(filePath);
        utils::Path artRootPath(artRootPathStr);
        if (path.isAbsolute() && !artRootPath.isEmpty()) {
            std::cout << "\tTexture path is absolute. Making it relative to '" << artRootPathStr << "'" << std::endl;
            path.makeRelativeTo(artRootPath);
        }
        path = artRootPath + path;

        std::cout << "\tResolved path: " << path.getPath() << std::endl;

        if (path.exists()) {
            int w, h, n;
            int numChannels = hasAlpha ? 4 : 3;

            Texture::InternalFormat inputFormat;
            if (sRGB) {
                inputFormat = hasAlpha ? Texture::InternalFormat::SRGB8_A8 : Texture::InternalFormat::SRGB8;
            }
            else {
                inputFormat = hasAlpha ? Texture::InternalFormat::RGBA8 : Texture::InternalFormat::RGB8;
            }

            Texture::Format outputFormat = hasAlpha ? Texture::Format::RGBA : Texture::Format::RGB;

            uint8_t* data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, numChannels);
            if (data != nullptr) {
                *map = Texture::Builder()
                    .width(uint32_t(w))
                    .height(uint32_t(h))
                    .levels(0xff)
                    .format(inputFormat)
                    .build(*engine);

                Texture::PixelBufferDescriptor buffer(data,
                    size_t(w * h * numChannels),
                    outputFormat,
                    Texture::Type::UBYTE,
                    (Texture::PixelBufferDescriptor::Callback)&stbi_image_free);

                (*map)->setImage(*engine, 0, std::move(buffer));
                (*map)->generateMipmaps(*engine);
            }
            else {
                std::cout << "The texture " << path << " could not be loaded" << std::endl;
            }
        }
        else {
            std::cout << "The texture " << path << " does not exist" << std::endl;
        }
    }
}

filament::math::mat4f fitIntoUnitCube(const filament::Aabb& bounds, float zoffset) {
    using namespace filament::math;
    auto minpt = bounds.min;
    auto maxpt = bounds.max;
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
        case ToneMapping::GENERIC:
            mapper = new GenericToneMapper(
                    settings.genericToneMapper.contrast,
                    settings.genericToneMapper.shoulder,
                    settings.genericToneMapper.midGrayIn,
                    settings.genericToneMapper.midGrayOut,
                    settings.genericToneMapper.hdrMax
            );
            hdrMax = settings.genericToneMapper.hdrMax;
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

        int toneMapping = (int) colorGrading.toneMapping;
        ImGui::Combo("Tone-mapping", &toneMapping,
                "Linear\0ACES (legacy)\0ACES\0Filmic\0Generic\0Display Range\0\0");
        colorGrading.toneMapping = (decltype(colorGrading.toneMapping)) toneMapping;
        if (colorGrading.toneMapping == ToneMapping::GENERIC) {
            if (ImGui::CollapsingHeader("Tonemap parameters")) {
                GenericToneMapperSettings& generic = colorGrading.genericToneMapper;
                ImGui::SliderFloat("Contrast##genericToneMapper", &generic.contrast, 1e-5f, 3.0f);
                ImGui::SliderFloat("Shoulder##genericToneMapper", &generic.shoulder, 0.0f, 1.0f);
                ImGui::SliderFloat("Mid-gray in##genericToneMapper", &generic.midGrayIn, 0.0f, 1.0f);
                ImGui::SliderFloat("Mid-gray out##genericToneMapper", &generic.midGrayOut, 0.0f, 1.0f);
                ImGui::SliderFloat("HDR max", &generic.hdrMax, 1.0f, 64.0f);
            }
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

SimpleViewer::SimpleViewer(filament::Engine* engine, filament::Scene* scene, filament::View* view,
        int sidebarWidth) :
        mEngine(engine), mScene(scene), mView(view),
        mSunlight(utils::EntityManager::get().create()),
        mSidebarWidth(sidebarWidth) {

    mSettings.view.shadowType = ShadowType::PCF;
    mSettings.view.vsmShadowOptions.anisotropy = 0;
    mSettings.view.dithering = Dithering::TEMPORAL;
    mSettings.view.antiAliasing = AntiAliasing::FXAA;
    mSettings.view.sampleCount = 4;
    mSettings.view.ssao.enabled = true;
    mSettings.view.bloom.enabled = true;

    using namespace filament;
    LightManager::Builder(LightManager::Type::SUN)
        .color(mSettings.lighting.sunlightColor)
        .intensity(mSettings.lighting.sunlightIntensity)
        .direction(normalize(mSettings.lighting.sunlightDirection))
        .castShadows(true)
        .sunAngularRadius(1.0f)
        .sunHaloSize(2.0f)
        .sunHaloFalloff(80.0f)
        .build(*engine, mSunlight);
    if (mSettings.lighting.enableSunlight) {
        mScene->addEntity(mSunlight);
    }
    view->setAmbientOcclusionOptions({ .upsampling = View::QualityLevel::HIGH });
}

// Add a dummy entity with our general shaders so that its IBL and whatnot will be set properly
void SimpleViewer::generateDummyMaterial() {
    struct Vertex {
        filament::math::float2 position;
        uint32_t color;
    };
    static const Vertex TRIANGLE_VERTICES[3] = {
        {{1, 0}, 0xffff0000u},
        {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
        {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
    };

    static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

    const Material* currentMaterial = mEngine->getShaprMaterial(0);
    const_cast<MaterialInstance*>(currentMaterial->getDefaultInstance())->setParameter("baseColor", filament::math::float4{ 1.0f, 0.0f, 0.0f, 1.0f });

    mDummyVB = VertexBuffer::Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
        .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
        .normalized(VertexAttribute::COLOR)
        .build(*mEngine);
    mDummyVB->setBufferAt(*mEngine, 0,
        VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
    mDummyIB = IndexBuffer::Builder()
        .indexCount(3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*mEngine);
    mDummyIB->setBuffer(*mEngine,
        IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
    mDummyEntity = utils::EntityManager::get().create();
    RenderableManager::Builder(1)
        .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
        .material(0, currentMaterial->getDefaultInstance())
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, mDummyVB, mDummyIB, 0, 3)
        .culling(false)
        .receiveShadows(true)
        .castShadows(true)
        .build(*mEngine, mDummyEntity);
    mScene->addEntity(mDummyEntity);
}

SimpleViewer::~SimpleViewer() {
    for (auto* mi : mMaterialInstances) {
        if (mi != nullptr) {
            mEngine->destroy(mi);
        }
    }
    for (auto textureEntry : mTextures) {
        mEngine->destroy(textureEntry.second);
    }
    mEngine->destroy(mSunlight);
    delete mImGuiHelper;
}

void SimpleViewer::populateScene(FilamentAsset* asset,  FilamentInstance* instanceToAnimate) {
    if (mAsset != asset) {
        removeAsset();
        mAsset = asset;
        mCurrentCamera = 0;
        if (!asset) {
            mAnimator = nullptr;
            return;
        }
        mAnimator = instanceToAnimate ? instanceToAnimate->getAnimator() : asset->getAnimator();
        updateRootTransform();
        mScene->addEntities(asset->getLightEntities(), asset->getLightEntityCount());
    }

    auto& tcm = mEngine->getRenderableManager();

    static constexpr int kNumAvailable = 128;
    utils::Entity renderables[kNumAvailable];
    while (size_t numWritten = mAsset->popRenderables(renderables, kNumAvailable)) {
        for (size_t i = 0; i < numWritten; i++) {
            auto ri = tcm.getInstance(renderables[i]);
            tcm.setScreenSpaceContactShadows(ri, true);
        }
        mScene->addEntities(renderables, numWritten);
    }
}

void SimpleViewer::removeAsset() {
    if (mAsset) {
        mScene->removeEntities(mAsset->getEntities(), mAsset->getEntityCount());
    }
    mAsset = nullptr;
    mAnimator = nullptr;
}

void SimpleViewer::setIndirectLight(filament::IndirectLight* ibl,
        filament::math::float3 const* sh3) {
    using namespace filament::math;

    mIbl = ibl;
    mSh3 = sh3;

    mSettings.view.fog.color = sh3[0];
    mIndirectLight = ibl;
}

void SimpleViewer::setSunlightFromIbl() {
    using namespace filament::math;
    if (mIbl && mSh3) {
        float3 d = filament::IndirectLight::getDirectionEstimate(mSh3);
        float4 c = filament::IndirectLight::getColorEstimate(mSh3, d);
        if (!std::isnan(d.x * d.y * d.z)) {
            mSettings.lighting.sunlightDirection = mIbl->getRotation() * d;
            mSettings.lighting.sunlightColor = c.rgb;
            mSettings.lighting.sunlightIntensity = c[3] * mIbl->getIntensity();
            updateIndirectLight();
        }
    }
}

void SimpleViewer::updateRootTransform() {
    if (mAsset == nullptr) {
        return;
    }
    auto& tcm = mEngine->getTransformManager();
    auto root = tcm.getInstance(mAsset->getRoot());
    filament::math::mat4f transform;
    if (mSettings.viewer.autoScaleEnabled) {
        transform = fitIntoUnitCube(mAsset->getBoundingBox(), 4);
    }
    tcm.setTransform(root, transform);
}

void SimpleViewer::updateIndirectLight() {
    using namespace filament::math;
    if (mIndirectLight) {
        mIndirectLight->setIntensity(mSettings.lighting.iblIntensity);
        mIndirectLight->setRotation(
            mat3f::rotation(mSettings.lighting.iblRotation, float3{ 0, 0, 1 }) *
            mat3f::rotation((float)M_PI_2, float3{ 1, 0, 0 })
        );
    }
    if (mScene->getSkybox()) {
        mScene->getSkybox()->setIntensity(mSettings.lighting.skyIntensity);
        mScene->getSkybox()->setType(mSettings.lighting.skyboxType);
    }
}

void SimpleViewer::applyAnimation(double currentTime) {
    if (!mAnimator) {
        return;
    }
    static double startTime = 0;
    const size_t numAnimations = mAnimator->getAnimationCount();
    if (mResetAnimation) {
        startTime = currentTime;
        for (size_t i = 0; i < numAnimations; i++) {
            mAnimator->applyAnimation(i, 0);
        }
        mResetAnimation = false;
    }
    if (numAnimations > 0 && mCurrentAnimation > 0) {
        mAnimator->applyAnimation(mCurrentAnimation - 1, currentTime - startTime);
    }
    mAnimator->updateBoneMatrices();
}

void SimpleViewer::renderUserInterface(float timeStepInSeconds, View* guiView, float pixelRatio) {
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

void SimpleViewer::mouseEvent(float mouseX, float mouseY, bool mouseButton, float mouseWheelY, bool control) {
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

void SimpleViewer::keyDownEvent(int keyCode) {
    if (mImGuiHelper && keyCode < IM_ARRAYSIZE(ImGui::GetIO().KeysDown)) {
        ImGui::GetIO().KeysDown[keyCode] = true;
    }
}

void SimpleViewer::keyUpEvent(int keyCode) {
    if (mImGuiHelper && keyCode < IM_ARRAYSIZE(ImGui::GetIO().KeysDown)) {
        ImGui::GetIO().KeysDown[keyCode] = false;
    }
}

void SimpleViewer::keyPressEvent(int charCode) {
    if (mImGuiHelper) {
        ImGui::GetIO().AddInputCharacter(charCode);
    }
}

void SimpleViewer::saveTweaksToFile(TweakableMaterial* tweaks, const char* filePath) {
    nlohmann::json js = tweaks->toJson();
    std::string filePathStr = filePath;
    if (filePathStr.rfind(".json") != (filePathStr.size() - 5)) {
        filePathStr += ".json";
    }
    mLastSavedFileName = filePathStr;
    std::fstream outFile(filePathStr, std::ios::binary | std::ios::out);
    outFile << js << std::endl;
    outFile.close();
}


void SimpleViewer::loadTweaksFromFile(const std::string& entityName, const std::string& filePath) {
    auto entityMaterial = mTweakedMaterials.find(entityName);
    if (entityMaterial == mTweakedMaterials.end()) {
        std::cout << "Cannot quick load material '" << filePath << "' for entity '" << entityName << "'" << std::endl;
        return;
    }
    TweakableMaterial& tweaks = entityMaterial->second;

    std::fstream inFile(filePath, std::ios::binary | std::ios::in);
    nlohmann::json js{};
    inFile >> js;
    inFile.close();
    tweaks.fromJson(js);

    // fixup legacy transparent thin material
    if (tweaks.mShaderType == TweakableMaterial::MaterialType::TransparentThin) tweaks.mShaderType = TweakableMaterial::MaterialType::TransparentSolid;
}

void SimpleViewer::changeElementVisibility(utils::Entity entity, int elementIndex, bool newVisibility) {
    // The children of the root are the below-Node level nodes in the graph - indexing refers to them by artist request
    auto& tm = mEngine->getTransformManager();
    auto& rm = mEngine->getRenderableManager();

    auto rinstance = rm.getInstance(entity);
    auto tinstance = tm.getInstance(entity);

    auto getNthChild = [&](utils::Entity& parentEntity, int N = 0) {
        auto tparentEntity = tm.getInstance(parentEntity);
        utils::Entity nthChild;
        if (N < tm.getChildCount(tparentEntity)) {
            tm.getChildren(tparentEntity, &nthChild, N);
        }
        return nthChild;
    };

    auto hasChild = [&](utils::Entity& parentEntity) {
        auto tparentEntity = tm.getInstance(parentEntity);
        return tm.getChildCount(tparentEntity) != 0;
    };

    if (hasChild(entity)) {
        auto tparentOfRoi = tm.getInstance(entity);
        std::vector<utils::Entity> children(tm.getChildCount(tparentOfRoi));
        tm.getChildren(tinstance, children.data(), children.size());

        if (elementIndex < children.size()) {
            changeAllVisibility(children[elementIndex], newVisibility);
        }
    }
}

void SimpleViewer::changeAllVisibility(utils::Entity entity, bool changeToVisible) {
    auto& tm = mEngine->getTransformManager();
    if (!changeToVisible) {
        mScene->remove(entity);
    }
    else {
        mScene->addEntity(entity);
    }

    auto tinstance = tm.getInstance(entity);

    std::vector<utils::Entity> children(tm.getChildCount(tinstance));

    tm.getChildren(tinstance, children.data(), children.size());
    for (auto ce : children) {
        changeAllVisibility(ce, changeToVisible);
    }
};

void SimpleViewer::updateUserInterface() {
    using namespace filament;

    static char fileDialogPath[1024];
    static const char* materialFileFilter = "JSON\0*.JSON\0";

    auto& tm = mEngine->getTransformManager();
    auto& rm = mEngine->getRenderableManager();
    auto& lm = mEngine->getLightManager();

    // Show a common set of UI widgets for all renderables.
    auto renderableTreeItem = [this, &rm](utils::Entity entity) {
        bool rvis = mScene->hasEntity(entity);
        ImGui::Checkbox("visible", &rvis);
        if ( rvis ) {
            mScene->addEntity(entity);
        } else {
            mScene->remove(entity);
        }

        auto instance = rm.getInstance(entity);
        bool scaster = rm.isShadowCaster(instance);
        ImGui::Checkbox("casts shadows", &scaster);
        rm.setCastShadows(instance, scaster);
        size_t numPrims = rm.getPrimitiveCount(instance);
        for (size_t prim = 0; prim < numPrims; ++prim) {
            // check if we have already assigned a custom material tweak to this entity
            const char* entityName = mAsset->getName(entity);
            auto entityMaterial = mTweakedMaterials.find(entityName);
            if (entityMaterial == mTweakedMaterials.end()) {
                // Only allow adding a new material, nothing else
                if (ImGui::Button("Add custom material")) {                
                    mTweakedMaterials.emplace(std::pair<std::string, TweakableMaterial>(entityName, TweakableMaterial{}));
                    filament::MaterialInstance* newInstance = mEngine->getShaprMaterial(0)->createInstance();
                    mMaterialInstances.push_back(newInstance);
                    rm.setMaterialInstanceAt(instance, prim, newInstance);
                }
            } else {
                // As soon as they have assigned a material to it, we display its settings all the time
                if (ImGui::CollapsingHeader("Custom persistent material tweaks")) {
                    TweakableMaterial& tweaks = entityMaterial->second;

                    if (ImGui::Button("Load material")) {

                        if (SD_OpenFileDialog(fileDialogPath, materialFileFilter)) {
                            TweakableMaterial::MaterialType prevType = tweaks.mShaderType;

                            loadTweaksFromFile(entityName, fileDialogPath);

                            mLastSavedEntityName = entityName;
                            mLastSavedFileName = fileDialogPath;

                            if (tweaks.mShaderType != prevType) {
                                filament::MaterialInstance* currentInstance = rm.getMaterialInstanceAt(instance, prim);
                                for (auto& mat : mMaterialInstances) if (mat == currentInstance) mat = nullptr;
                                mEngine->destroy(currentInstance);

                                filament::MaterialInstance* newInstance = mEngine->getShaprMaterial(tweaks.mShaderType)->createInstance();
                                mMaterialInstances.push_back(newInstance);
                                rm.setMaterialInstanceAt(instance, prim, newInstance);
                            }
                        }

                    } else {
                        ImGui::SameLine();
                        if (ImGui::Button("Save material")) {
                            if (SD_SaveFileDialog(fileDialogPath, materialFileFilter)) {
                                mLastSavedEntityName = entityName;
                                saveTweaksToFile(&tweaks, fileDialogPath);
                            }
                        } else {
                            ImGui::SameLine();
                            if (ImGui::Button("Reset")) {
                                TweakableMaterial::MaterialType prevType = tweaks.mShaderType;
                                tweaks.resetWithType(prevType);
                            }

                            std::function<void( TweakableMaterial::MaterialType materialType)> changeMaterialTypeTo;
                            changeMaterialTypeTo = [&]( TweakableMaterial::MaterialType materialType) {
                                if (tweaks.mShaderType != materialType) {
                                    filament::MaterialInstance* currentInstance = rm.getMaterialInstanceAt(instance, prim);
                                    for (auto& mat : mMaterialInstances) if (mat == currentInstance) mat = nullptr;
                                    mEngine->destroy(currentInstance);

                                    filament::MaterialInstance* newInstance = mEngine->getShaprMaterial(materialType)->createInstance();
                                    mMaterialInstances.push_back(newInstance);
                                    rm.setMaterialInstanceAt(instance, prim, newInstance);
                                }
                                tweaks.mShaderType = materialType;
                            };

                            if (ImGui::RadioButton("Opaque", tweaks.mShaderType == TweakableMaterial::MaterialType::Opaque)) {
                                changeMaterialTypeTo(TweakableMaterial::MaterialType::Opaque);
                            }
                            ImGui::SameLine();
                            if (ImGui::RadioButton("Transparent solid", tweaks.mShaderType == TweakableMaterial::MaterialType::TransparentSolid)) {
                                changeMaterialTypeTo(TweakableMaterial::MaterialType::TransparentSolid);
                            }
                            ImGui::SameLine();
                            if (ImGui::RadioButton("Cloth", tweaks.mShaderType == TweakableMaterial::MaterialType::Cloth)) {
                                changeMaterialTypeTo(TweakableMaterial::MaterialType::Cloth);
                            }
                            ImGui::SameLine();
                            if (ImGui::RadioButton("Subsurface", tweaks.mShaderType == TweakableMaterial::MaterialType::Subsurface)) {
                                changeMaterialTypeTo(TweakableMaterial::MaterialType::Subsurface);
                            }
                        }
                    }

                    tweaks.drawUI();

                    auto checkAndFixPathRelative([](auto& path) {
                        utils::Path asPath(path);
                        if (asPath.isAbsolute()) {
                            std::string newFilePath = asPath.makeRelativeTo(g_ArtRootPathStr).c_str();
                            //std::cout << "\t\tMaking path relative: " << path << std::endl;
                            //std::cout << "\t\tMade path relative: " << newFilePath << std::endl;
                            path = newFilePath;
                        }
                    });

                    // Load the requested textures
                    TweakableMaterial::RequestedTexture currentRequestedTexture = tweaks.nextRequestedTexture();
                    while (currentRequestedTexture.filename != "") {
                        checkAndFixPathRelative(currentRequestedTexture.filename);
                        std::string keyName = currentRequestedTexture.filename;
                        auto textureEntry = mTextures.find(keyName);
                        if (textureEntry == mTextures.end() ) {
                            mTextures[keyName] = nullptr;
                            loadTexture(mEngine, currentRequestedTexture.filename, &mTextures[keyName], currentRequestedTexture.isSrgb, currentRequestedTexture.isAlpha, mSettings.viewer.artRootPath);
                        } else if (currentRequestedTexture.doRequestReload) {
                            if (mTextures[keyName] != nullptr) mEngine->destroy(mTextures[keyName]);
                            loadTexture(mEngine, currentRequestedTexture.filename, &mTextures[keyName], currentRequestedTexture.isSrgb, currentRequestedTexture.isAlpha, mSettings.viewer.artRootPath);
                        }

                        currentRequestedTexture = tweaks.nextRequestedTexture();
                    }
                }
            }

            const auto& matInstance = rm.getMaterialInstanceAt(instance, prim);
            const char* mname = matInstance->getName();
            const auto* mat = matInstance->getMaterial();
            
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

    std::function<void(utils::Entity)> applyMaterialSettingsInTree;
    applyMaterialSettingsInTree = [&](utils::Entity entity) {
        auto tinstance = tm.getInstance(entity);
        auto rinstance = rm.getInstance(entity);
        auto linstance = lm.getInstance(entity);
        intptr_t treeNodeId = 1 + entity.getId();

        std::vector<utils::Entity> children(tm.getChildCount(tinstance));
        if (rinstance) {
            const char* entityName = mAsset->getName(entity);
            auto instance = rm.getInstance(entity);
            size_t numPrims = rm.getPrimitiveCount(instance);

            auto entityMaterial = mTweakedMaterials.find(entityName);
            if (entityMaterial != mTweakedMaterials.end()) {
                // These attributes are only present in the Shapr general material
                const auto& tweaks = entityMaterial->second;
                for (size_t prim = 0; prim < numPrims; ++prim) {
                    const auto& matInstance = rm.getMaterialInstanceAt(instance, prim);

                    auto setTextureIfPresent ([&](bool useTexture, const auto& filename, const std::string& propertyName) {
                        std::string useTextureName = "use" + propertyName + "Texture";
                        std::string samplerName = propertyName + "Texture";

                        useTextureName[3] = std::toupper(useTextureName[3]);

                        auto textureEntry = mTextures.find(filename.asString());
                        if (useTexture && textureEntry != mTextures.end() && textureEntry->second != nullptr) {
                            matInstance->setParameter(useTextureName.c_str(), 1);
                            matInstance->setParameter(samplerName.c_str(), textureEntry->second, trilinSampler);
                        }
                        else {
                            matInstance->setParameter(useTextureName.c_str(), 0);
                        }

                    });

                    matInstance->setParameter("useWard", (tweaks.mUseWard) ? 1 : 0 );

                    setTextureIfPresent(tweaks.mBaseColor.isFile, tweaks.mBaseColor.filename, "baseColor");

                    matInstance->setParameter("normalScale", tweaks.mNormalIntensity.value);
                    setTextureIfPresent(tweaks.mNormal.isFile, tweaks.mNormal.filename, "normal");
                    matInstance->setParameter("roughnessScale", tweaks.mRoughnessScale.value);
                    setTextureIfPresent(tweaks.mRoughness.isFile, tweaks.mRoughness.filename, "roughness");
                    if (tweaks.mShaderType == TweakableMaterial::MaterialType::Opaque || tweaks.mShaderType == TweakableMaterial::MaterialType::Cloth || tweaks.mShaderType == TweakableMaterial::MaterialType::Subsurface) {
                        matInstance->setParameter("occlusionIntensity", tweaks.mOcclusionIntensity.value);
                        setTextureIfPresent(tweaks.mOcclusion.isFile, tweaks.mOcclusion.filename, "occlusion");
                        matInstance->setParameter("occlusion", tweaks.mOcclusion.value);
                    }

                    matInstance->setParameter("clearCoatNormalScale", tweaks.mClearCoatNormalIntensity.value);
                    setTextureIfPresent(tweaks.mClearCoatNormal.isFile, tweaks.mClearCoatNormal.filename, "clearCoatNormal");
                    setTextureIfPresent(tweaks.mClearCoatRoughness.isFile, tweaks.mClearCoatRoughness.filename, "clearCoatRoughness");

                    matInstance->setParameter("useBumpTexture", 0);

                    matInstance->setParameter("textureScaler", math::float4(tweaks.mBaseTextureScale, tweaks.mNormalTextureScale, tweaks.mClearCoatTextureScale, tweaks.mRefractiveTextureScale));
                    matInstance->setParameter("specularIntensity", tweaks.mSpecularIntensity.value);
                    math::float4 gammaBaseColor{}; 
                    gammaBaseColor.r = std::pow(tweaks.mBaseColor.value.r, 2.22f); 
                    gammaBaseColor.g = std::pow(tweaks.mBaseColor.value.g, 2.22f);
                    gammaBaseColor.b = std::pow(tweaks.mBaseColor.value.b, 2.22f);
                    gammaBaseColor.a = tweaks.mBaseColor.value.a;
                    matInstance->setParameter("baseColor", gammaBaseColor);
                    matInstance->setParameter("roughness", tweaks.mRoughness.value);
                    matInstance->setParameter("clearCoat", tweaks.mClearCoat.value);
                    matInstance->setParameter("clearCoatRoughness", tweaks.mClearCoatRoughness.value);
                    
                    if (tweaks.mShaderType != TweakableMaterial::MaterialType::Cloth) {
                        setTextureIfPresent(tweaks.mMetallic.isFile, tweaks.mMetallic.filename, "metallic");
                        matInstance->setParameter("metallic", tweaks.mMetallic.value);
                    }

                    if (tweaks.mShaderType == TweakableMaterial::MaterialType::Cloth) {
                        if (tweaks.mSheenColor.useDerivedQuantity) {
                            matInstance->setParameter("doDeriveSheenColor", 1);
                        } else {
                            matInstance->setParameter("sheenColor", tweaks.mSheenColor.value);
                            matInstance->setParameter("doDeriveSheenColor", 0);
                        }
                        matInstance->setParameter("subsurfaceColor", tweaks.mSubsurfaceColor.value);
                    } else if (tweaks.mShaderType == TweakableMaterial::MaterialType::Subsurface) {
                        matInstance->setParameter("thickness", tweaks.mThickness.value);
                        setTextureIfPresent(tweaks.mThickness.isFile, tweaks.mThickness.filename, "thickness");
                        matInstance->setParameter("subsurfaceColor", tweaks.mSubsurfaceColor.value);
                        matInstance->setParameter("subsurfacePower", tweaks.mSubsurfacePower.value);
                    }

                    if (tweaks.mShaderType == TweakableMaterial::MaterialType::Opaque) {
                        // Transparent materials do not expose anisotropy and sheen, these are not present in their UBOs
                        matInstance->setParameter("anisotropy", tweaks.mAnisotropy.value);
                        matInstance->setParameter("anisotropyDirection", normalize(tweaks.mAnisotropyDirection.value));

                        if (tweaks.mSheenColor.useDerivedQuantity) {
                            matInstance->setParameter("doDeriveSheenColor", 1);
                        }
                        else {
                            matInstance->setParameter("sheenColor", tweaks.mSheenColor.value);
                            matInstance->setParameter("doDeriveSheenColor", 0);
                        }
                        setTextureIfPresent(tweaks.mSheenRoughness.isFile, tweaks.mSheenRoughness.filename, "sheenRoughness");
                        matInstance->setParameter("sheenRoughness", tweaks.mSheenRoughness.value);
                    } else if (tweaks.mShaderType == TweakableMaterial::MaterialType::TransparentSolid) {
                        // Only transparent materials have the properties below
                        if (tweaks.mSheenColor.useDerivedQuantity) {
                            matInstance->setParameter("doDeriveAbsorption", 1);
                        }
                        else {
                            matInstance->setParameter("absorption", tweaks.mAbsorption.value);
                            matInstance->setParameter("doDeriveAbsorption", 0);
                        }
                        
                        setTextureIfPresent(tweaks.mTransmission.isFile, tweaks.mTransmission.filename, "transmission");
                        setTextureIfPresent(tweaks.mThickness.isFile, tweaks.mThickness.filename, "thickness");

                        matInstance->setParameter("iorScale", tweaks.mIorScale.value);
                        matInstance->setParameter("ior", tweaks.mIor.value);
                        matInstance->setParameter("transmission", tweaks.mTransmission.value);
                        matInstance->setParameter("thickness", tweaks.mThickness.value);
                        matInstance->setParameter("maxThickness", tweaks.mMaxThickness.value);
                    }
                }
            }
        }
        tm.getChildren(tinstance, children.data(), children.size());
        for (auto ce : children) {
            applyMaterialSettingsInTree(ce);
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
            ImGui::Checkbox("Flare", &mSettings.view.bloom.lensFlare);

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

        bool msaa = mSettings.view.sampleCount != 1;
        ImGui::Checkbox("MSAA 4x", &msaa);
        enableMsaa(msaa);

        ImGui::Checkbox("SSAO", &mSettings.view.ssao.enabled);
        if (ImGui::CollapsingHeader("SSAO Options")) {
            auto& ssao = mSettings.view.ssao;

            int quality = (int) ssao.quality;
            int lowpass = (int) ssao.lowPassFilter;
            bool upsampling = ssao.upsampling != View::QualityLevel::LOW;

            ImGui::SliderFloat("SSAO power (contrast)", &ssao.power, 0.0f, 8.0f);
            ImGui::SliderFloat("SSAO intensity", &ssao.intensity, 0.0f, 2.0f);
            ImGui::SliderFloat("SSAO radius (meters)", &ssao.radius, 0.0f, 10.0f);
            ImGui::SliderFloat("SSAO bias (meters)", &ssao.bias, 0.0f, 0.1f, "%.6f");

            static int ssaoRes = 2;
            ImGui::RadioButton("SSAO half resolution", &ssaoRes, 1);
            ImGui::RadioButton("SSAO fullresolution", &ssaoRes, 2);
            ssao.resolution = ssaoRes * 0.5f;

            ImGui::SliderInt("Quality", &quality, 0, 3);
            ImGui::SliderInt("Low Pass", &lowpass, 0, 2);
            ImGui::Checkbox("Bent Normals", &ssao.bentNormals);
            ImGui::Checkbox("High quality upsampling", &upsampling);
            ImGui::SliderFloat("Min Horizon angle", &ssao.minHorizonAngleRad, 0.0f, (float)M_PI_4);
            ImGui::SliderFloat("Bilateral Threshold", &ssao.bilateralThreshold, 0.0f, 0.1f);
            bool halfRes = ssao.resolution == 1.0f ? false : true;
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
        ImGui::Unindent();
    }

    auto& light = mSettings.lighting;
    if (ImGui::CollapsingHeader("Light")) {
        ImGui::Indent();
        if (ImGui::CollapsingHeader("Skybox")) {
            ImGui::SliderFloat("Intensity", &light.skyIntensity, 0.0f, 100000.0f);
        }

        if (ImGui::CollapsingHeader("Indirect light")) {
            ImGui::SliderFloat("IBL intensity", &light.iblIntensity, 0.0f, 100000.0f);
            ImGui::SliderAngle("IBL rotation", &light.iblRotation);
        }
        if (ImGui::CollapsingHeader("Sunlight")) {
            ImGui::Checkbox("Enable sunlight", &light.enableSunlight);
            ImGui::SliderFloat("Sun intensity", &light.sunlightIntensity, 1000.0, 150000.0f);
            ImGuiExt::DirectionWidget("Sun direction", light.sunlightDirection.v);
            ImGui::ColorEdit3("Sun color", light.sunlightColor.v);
            if (ImGui::Button("Reset Sunlight from IBL")) setSunlightFromIbl();
        }
        if (ImGui::CollapsingHeader("All lights")) {
            ImGui::Checkbox("Enable shadows", &light.enableShadows);
            int mapSize = light.shadowOptions.mapSize;
            ImGui::SliderInt("Shadow map size", &mapSize, 32, 4096);
            light.shadowOptions.mapSize = mapSize;


            bool enableVsm = mSettings.view.shadowType == ShadowType::VSM;
            ImGui::Checkbox("Enable VSM", &enableVsm);
            mSettings.view.shadowType = enableVsm ? ShadowType::VSM : ShadowType::PCF;

            char label[32];
            snprintf(label, 32, "%d", 1 << mVsmMsaaSamplesLog2);
            ImGui::SliderInt("VSM MSAA samples", &mVsmMsaaSamplesLog2, 0, 3, label);
            light.shadowOptions.vsm.msaaSamples = static_cast<uint8_t>(1u << mVsmMsaaSamplesLog2);

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
        ImGui::Indent();
        ImGui::Checkbox("Enable fog", &mSettings.view.fog.enabled);
        ImGui::SliderFloat("Start", &mSettings.view.fog.distance, 0.0f, 100.0f);
        ImGui::SliderFloat("Density", &mSettings.view.fog.density, 0.0f, 1.0f);
        ImGui::SliderFloat("Height", &mSettings.view.fog.height, 0.0f, 100.0f);
        ImGui::SliderFloat("Height falloff", &mSettings.view.fog.heightFalloff, 0.0f, 10.0f);
        ImGui::SliderFloat("Scattering start", &mSettings.view.fog.inScatteringStart, 0.0f, 100.0f);
        ImGui::SliderFloat("Scattering size", &mSettings.view.fog.inScatteringSize, 0.1f, 100.0f);
        ImGui::Checkbox("Color from IBL", &mSettings.view.fog.fogColorFromIbl);
        ImGui::ColorPicker3("Color", mSettings.view.fog.color.v);
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Scene")) {
        ImGui::Indent();

        ImGui::Checkbox("Scale to unit cube", &mSettings.viewer.autoScaleEnabled);
        updateRootTransform();

        int skyboxType = (int)mSettings.lighting.skyboxType;
        ImGui::Combo("Skybox type", &skyboxType,
            "Solid color\0Gradient\0Environment\0Checkerboard\0\0");
        mSettings.lighting.skyboxType = (decltype(mSettings.lighting.skyboxType))skyboxType;
        ImGui::ColorEdit3("Background color", &mSettings.viewer.backgroundColor.r);

        // We do not yet support ground shadow in remote mode (i.e. when mAsset is null)
        if (mAsset) {
            ImGui::Checkbox("Ground shadow", &mSettings.viewer.groundPlaneEnabled);
            ImGui::Indent();
            ImGui::SliderFloat("Strength", &mSettings.viewer.groundShadowStrength, 0.0f, 1.0f);
            ImGui::Unindent();
        }

        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::Indent();

        ImGui::SliderFloat("Focal length (mm)", &mSettings.viewer.cameraFocalLength,
                16.0f, 90.0f);

        bool dofMedian = mSettings.view.dof.filter == View::DepthOfFieldOptions::Filter::MEDIAN;
        int dofRingCount = mSettings.view.dof.fastGatherRingCount;
        int dofMaxCoC = mSettings.view.dof.maxForegroundCOC;
        if (!dofRingCount) dofRingCount = 5;
        if (!dofMaxCoC) dofMaxCoC = 32;

        ImGui::SliderFloat("Aperture", &mSettings.viewer.cameraAperture, 1.0f, 32.0f);
        ImGui::SliderFloat("Speed (1/s)", &mSettings.viewer.cameraSpeed, 1000.0f, 1.0f);
        ImGui::SliderFloat("ISO", &mSettings.viewer.cameraISO, 25.0f, 6400.0f);
        ImGui::Checkbox("DoF", &mSettings.view.dof.enabled);
        ImGui::SliderFloat("Focus distance", &mSettings.viewer.cameraFocusDistance, 0.0f, 30.0f);
        ImGui::SliderFloat("Blur scale", &mSettings.view.dof.cocScale, 0.1f, 10.0f);
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

        if (ImGui::CollapsingHeader("Vignette")) {
            ImGui::Checkbox("Enabled##vignetteEnabled", &mSettings.view.vignette.enabled);
            ImGui::SliderFloat("Mid point", &mSettings.view.vignette.midPoint, 0.0f, 1.0f);
            ImGui::SliderFloat("Roundness", &mSettings.view.vignette.roundness, 0.0f, 1.0f);
            ImGui::SliderFloat("Feather", &mSettings.view.vignette.feather, 0.0f, 1.0f);
            ImGui::ColorEdit3("Color##vignetteColor", &mSettings.view.vignette.color.r);
        }

        // We do not yet support camera selection in the remote UI. To support this feature, we
        // would need to send a message from DebugServer to the WebSockets client.
        if (mAsset != nullptr) {

            const utils::Entity* cameras = mAsset->getCameraEntities();
            const size_t cameraCount = mAsset->getCameraEntityCount();

            std::vector<std::string> names;
            names.reserve(cameraCount + 1);
            names.push_back("Free camera");
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
            for (size_t i = 0; i < names.size(); i++) {
                cstrings.push_back(names[i].c_str());
            }

            ImGui::ListBox("Cameras", &mCurrentCamera, cstrings.data(), cstrings.size());
        }

        ImGui::Unindent();
    }

    colorGradingUI(mSettings, mRangePlot, mCurvePlot, mToneMapPlot);

    // At this point, all View settings have been modified,
    //  so we can now push them into the Filament View.
    applySettings(mSettings.view, mView);

    if (light.enableSunlight) {
        mScene->addEntity(mSunlight);
        auto sun = lm.getInstance(mSunlight);
        lm.setIntensity(sun, light.sunlightIntensity);
        lm.setDirection(sun, normalize(light.sunlightDirection));
        lm.setColor(sun, light.sunlightColor);
        lm.setShadowCaster(sun, light.enableShadows);
        lm.setShadowOptions(sun, light.shadowOptions);
    } else {
        mScene->remove(mSunlight);
    }

    lm.forEachComponent([this, &lm, &light](utils::Entity e, LightManager::Instance ci) {
        lm.setShadowOptions(ci, light.shadowOptions);
        lm.setShadowCaster(ci, light.enableShadows);
    });

    if (mAsset != nullptr) {
        if (ImGui::CollapsingHeader("Hierarchy")) {
            if (ImGui::Button("Make all visible")) {
                changeAllVisibility(mAsset->getRoot(), true);
                for (bool& isVisible : mVisibility) isVisible = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Make all invisible")) {
                changeAllVisibility(mAsset->getRoot(), false);
                for (bool& isVisible : mVisibility) isVisible = false;
            }
            ImGui::Indent();
            ImGui::Checkbox("Show bounds", &mEnableWireframe);
            treeNode(mAsset->getRoot());
            ImGui::Unindent();
        }

        applyMaterialSettingsInTree(mAsset->getRoot());

        // We do not yet support animation selection in the remote UI. To support this feature, we
        // would need to send a message from DebugServer to the WebSockets client.
        if (mAnimator && mAnimator->getAnimationCount() > 0 &&
                ImGui::CollapsingHeader("Animation")) {
            ImGui::Indent();
            int selectedAnimation = mCurrentAnimation;
            ImGui::RadioButton("Disable", &selectedAnimation, 0);
            for (size_t i = 0, count = mAnimator->getAnimationCount(); i < count; ++i) {
                std::string label = mAnimator->getAnimationName(i);
                if (label.empty()) {
                    label = "Unnamed " + std::to_string(i);
                }
                ImGui::RadioButton(label.c_str(), &selectedAnimation, i + 1);
            }
            if (selectedAnimation != mCurrentAnimation) {
                mCurrentAnimation = selectedAnimation;
                mResetAnimation = true;
            }
            ImGui::Unindent();
        }

        if (mEnableWireframe) {
            mScene->addEntity(mAsset->getWireframe());
        } else {
            mScene->remove(mAsset->getWireframe());
        }
    }

    {
        // Camera movement speed setting UI
        ImGui::Separator();
        if (ImGui::SliderFloat("Cam move speed", &mSettings.viewer.cameraMovementSpeed, 1.0f, 100.0f)) {
            updateCameraMovementSpeed();
        }
    }

    {
        // Art root directory display
        ImGui::Separator();
        ImGui::Text("Art root: %s", mSettings.viewer.artRootPath.empty() ? "<none>" : mSettings.viewer.artRootPath.c_str());
        char tempArtRootPath[1024];
        if (ImGui::Button("Set art root") && mDoSaveSettings && SD_OpenFolderDialog(&tempArtRootPath[0])) {
            utils::Path tempPath = tempArtRootPath;
            if (tempPath.exists()) {
                auto sanitizePath = [](std::string& path) {
                    for (int i = 0; i < path.length(); ++i) {
                        if (path[i] == '\\') path[i] = '/';
                    }
                };

                mSettings.viewer.artRootPath = tempPath.c_str();
                sanitizePath(mSettings.viewer.artRootPath);
                g_ArtRootPathStr = tempArtRootPath;
                mDoSaveSettings();
            }
            else {
                // error
            }
        }
    }

    {
        // Hotkey-related feedback UI
        ImGui::Separator();
        ImGui::Text("Entity material to save: %s", mLastSavedEntityName.empty() ? "<none>" : mLastSavedEntityName.c_str());
        ImGui::Text("Save filename: %s", mLastSavedFileName.empty() ? "<none>" : mLastSavedFileName.c_str());
    }
    mSidebarWidth = ImGui::GetWindowWidth();
    ImGui::End();

    updateIndirectLight();
}

// This loads the last saved (or loaded) material from file, to its entity
void SimpleViewer::quickLoad() {
    loadTweaksFromFile(mLastSavedEntityName, mLastSavedFileName);
}

void SimpleViewer::undoLastModification() {
    // TODO implement
}

void SimpleViewer::updateCameraMovementSpeed() {
    if (mCameraMovementSpeedUpdateCallback) {
        mCameraMovementSpeedUpdateCallback(mSettings.viewer.cameraMovementSpeed);
    } else {
        std::cout << "Oops! No mCameraMovementSpeedUpdateCallback set!" << std::endl;
    }
}

void SimpleViewer::setCameraMovementSpeedUpdateCallback(std::function<void(float)>&& callback) {
    mCameraMovementSpeedUpdateCallback = std::move(callback);
}

void SimpleViewer::updateCameraSpeedOnUI(float value) {
    mSettings.viewer.cameraMovementSpeed = value;
}


} // namespace viewer
} // namespace filament
