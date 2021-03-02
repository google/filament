/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

#include <filament/Camera.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>

#include <viewer/AutomationEngine.h>
#include <viewer/AutomationSpec.h>
#include <viewer/SimpleViewer.h>

#include <camutils/Manipulator.h>

#include <getopt/getopt.h>

#include <utils/NameComponentManager.h>

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/norm.h>

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>

#include <fstream>
#include <iostream>
#include <string>

#include "generated/resources/gltf_viewer.h"

using namespace filament;
using namespace filament::math;
using namespace filament::viewer;

using namespace gltfio;
using namespace utils;

struct App {
    Engine* engine;
    SimpleViewer* viewer;
    Config config;
    Camera* mainCamera;

    AssetLoader* assetLoader;
    FilamentAsset* asset = nullptr;
    NameComponentManager* names;

    MaterialProvider* materials;
    MaterialSource materialSource = GENERATE_SHADERS;

    gltfio::ResourceLoader* resourceLoader = nullptr;
    bool recomputeAabb = false;

    bool actualSize = false;

    struct ViewOptions {
        float cameraAperture = 16.0f;
        float cameraSpeed = 125.0f;
        float cameraISO = 100.0f;
        float groundShadowStrength = 0.75f;
        bool groundPlaneEnabled = false;
        bool skyboxEnabled = true;
        sRGBColor backgroundColor = { 0.0f };
    } viewOptions;

    struct Scene {
        Entity groundPlane;
        VertexBuffer* groundVertexBuffer;
        IndexBuffer* groundIndexBuffer;
        Material* groundMaterial;
    } scene;

    // zero-initialized so that the first time through is always dirty.
    ColorGradingSettings lastColorGradingOptions = { 0 };

    ColorGrading* colorGrading = nullptr;

    float rangePlot[1024 * 3];
    float curvePlot[1024 * 3];

    // 0 is the default "free camera". Additional cameras come from the gltf file.
    int currentCamera = 0;

    std::string messageBoxText;
    std::string settingsFile;
    std::string batchFile;

    AutomationSpec* automationSpec = nullptr;
    AutomationEngine* automationEngine = nullptr;
};

static const char* DEFAULT_IBL = "default_env";

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
        "SHOWCASE renders the specified glTF file, or a built-in file if none is specified\n"
        "Usage:\n"
        "    SHOWCASE [options] <gltf path>\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
        "   --batch=<path to JSON file or 'default'>, -b\n"
        "       Start automation using the given JSON spec, then quit the app\n\n"
        "   --headless, -e\n"
        "       Use a headless swapchain; ignored if --batch is not present\n\n"
        "   --ibl=<path to cmgen IBL>, -i <path>\n"
        "       Override the built-in IBL\n\n"
        "   --actual-size, -s\n"
        "       Do not scale the model to fit into a unit cube\n\n"
        "   --recompute-aabb, -r\n"
        "       Ignore the min/max attributes in the glTF file\n\n"
        "   --settings=<path to JSON file>, -t\n"
        "       Apply the settings in the given JSON file\n\n"
        "   --ubershader, -u\n"
        "       Enable ubershaders (improves load time, adds shader complexity)\n\n"
        "   --camera=<camera mode>, -c <camera mode>\n"
        "       Set the camera mode: orbit (default) or flight\n"
        "       Flight mode uses the following controls:\n"
        "           Click and drag the mouse to pan the camera\n"
        "           Use the scroll weel to adjust movement speed\n"
        "           W / S: forward / backward\n"
        "           A / D: left / right\n"
        "           E / Q: up / down\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:i:usc:rt:b:e";
    static const struct option OPTIONS[] = {
        { "help",         no_argument,       nullptr, 'h' },
        { "api",          required_argument, nullptr, 'a' },
        { "batch",        required_argument, nullptr, 'b' },
        { "headless",     no_argument,       nullptr, 'e' },
        { "ibl",          required_argument, nullptr, 'i' },
        { "ubershader",   no_argument,       nullptr, 'u' },
        { "actual-size",  no_argument,       nullptr, 's' },
        { "camera",       required_argument, nullptr, 'c' },
        { "recompute-aabb", no_argument,     nullptr, 'r' },
        { "settings",       required_argument, nullptr, 't' },
        { nullptr, 0, nullptr, 0 }
    };
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                if (arg == "opengl") {
                    app->config.backend = Engine::Backend::OPENGL;
                } else if (arg == "vulkan") {
                    app->config.backend = Engine::Backend::VULKAN;
                } else if (arg == "metal") {
                    app->config.backend = Engine::Backend::METAL;
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'.\n";
                }
                break;
            case 'c':
                if (arg == "flight") {
                    app->config.cameraMode = camutils::Mode::FREE_FLIGHT;
                } else if (arg == "orbit") {
                    app->config.cameraMode = camutils::Mode::ORBIT;
                } else {
                    std::cerr << "Unrecognized camera mode. Must be 'flight'|'orbit'.\n";
                }
                break;
            case 'e':
                app->config.headless = true;
                break;
            case 'i':
                app->config.iblDirectory = arg;
                break;
            case 'u':
                app->materialSource = LOAD_UBERSHADERS;
                break;
            case 's':
                app->actualSize = true;
                break;
            case 'r':
                app->recomputeAabb = true;
                break;
            case 't':
                app->settingsFile = arg;
                break;
            case 'b': {
                app->batchFile = arg;
                break;
            }
        }
    }
    if (app->config.headless && app->batchFile.empty()) {
        std::cerr << "--headless is allowed only when --batch is present." << std::endl;
        app->config.headless = false;
    }
    return optind;
}

static bool loadSettings(const char* filename, Settings* out) {
    auto contentSize = getFileSize(filename);
    if (contentSize <= 0) {
        return false;
    }
    std::ifstream in(filename, std::ifstream::binary | std::ifstream::in);
    std::vector<char> json(static_cast<unsigned long>(contentSize));
    if (!in.read(json.data(), contentSize)) {
        return false;
    }
    return readJson(json.data(), contentSize, out);
}

static void createGroundPlane(Engine* engine, Scene* scene, App& app) {
    auto& em = EntityManager::get();
    Material* shadowMaterial = Material::Builder()
            .package(GLTF_VIEWER_GROUNDSHADOW_DATA, GLTF_VIEWER_GROUNDSHADOW_SIZE)
            .build(*engine);
    shadowMaterial->setDefaultParameter("strength", app.viewOptions.groundShadowStrength);

    const static uint32_t indices[] = {
            0, 1, 2, 2, 3, 0
    };

    Aabb aabb = app.asset->getBoundingBox();
    if (!app.actualSize) {
        mat4f transform = fitIntoUnitCube(aabb);
        aabb = aabb.transform(transform);
    }

    float3 planeExtent{10.0f * aabb.extent().x, 0.0f, 10.0f * aabb.extent().z};

    const static float3 vertices[] = {
            { -planeExtent.x, 0, -planeExtent.z },
            { -planeExtent.x, 0,  planeExtent.z },
            {  planeExtent.x, 0,  planeExtent.z },
            {  planeExtent.x, 0, -planeExtent.z },
    };

    short4 tbn = packSnorm16(
            mat3f::packTangentFrame(
                    mat3f{
                            float3{ 1.0f, 0.0f, 0.0f },
                            float3{ 0.0f, 0.0f, 1.0f },
                            float3{ 0.0f, 1.0f, 0.0f }
                    }
            ).xyzw);

    const static short4 normals[] { tbn, tbn, tbn, tbn };

    VertexBuffer* vertexBuffer = VertexBuffer::Builder()
            .vertexCount(4)
            .bufferCount(2)
            .attribute(VertexAttribute::POSITION,
                    0, VertexBuffer::AttributeType::FLOAT3)
            .attribute(VertexAttribute::TANGENTS,
                    1, VertexBuffer::AttributeType::SHORT4)
            .normalized(VertexAttribute::TANGENTS)
            .build(*engine);

    vertexBuffer->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(
            vertices, vertexBuffer->getVertexCount() * sizeof(vertices[0])));
    vertexBuffer->setBufferAt(*engine, 1, VertexBuffer::BufferDescriptor(
            normals, vertexBuffer->getVertexCount() * sizeof(normals[0])));

    IndexBuffer* indexBuffer = IndexBuffer::Builder()
            .indexCount(6)
            .build(*engine);

    indexBuffer->setBuffer(*engine, IndexBuffer::BufferDescriptor(
            indices, indexBuffer->getIndexCount() * sizeof(uint32_t)));

    Entity groundPlane = em.create();
    RenderableManager::Builder(1)
            .boundingBox({
                { -planeExtent.x, 0, -planeExtent.z },
                { planeExtent.x, 1e-4f, planeExtent.z }
            })
            .material(0, shadowMaterial->getDefaultInstance())
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    vertexBuffer, indexBuffer, 0, 6)
            .culling(false)
            .receiveShadows(true)
            .castShadows(false)
            .build(*engine, groundPlane);

    scene->addEntity(groundPlane);

    auto& tcm = engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(groundPlane),
            mat4f::translation(float3{ 0, aabb.min.y, -4 }));

    auto& rcm = engine->getRenderableManager();
    auto instance = rcm.getInstance(groundPlane);
    rcm.setLayerMask(instance, 0xff, 0x00);

    app.scene.groundPlane = groundPlane;
    app.scene.groundVertexBuffer = vertexBuffer;
    app.scene.groundIndexBuffer = indexBuffer;
    app.scene.groundMaterial = shadowMaterial;
}

static void computeRangePlot(App& app, float* rangePlot) {
    float4& ranges = app.viewer->getSettings().view.colorGrading.ranges;
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

static void computeCurvePlot(App& app, float* curvePlot) {
    const auto& colorGradingOptions = app.viewer->getSettings().view.colorGrading;
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

static void colorGradingUI(App& app) {
    const static ImVec2 verticalSliderSize(18.0f, 160.0f);
    const static ImVec2 plotLinesSize(260.0f, 160.0f);
    const static ImVec2 plotLinesWideSize(350.0f, 120.0f);

    if (ImGui::CollapsingHeader("Color grading")) {
        ColorGradingSettings& colorGrading = app.viewer->getSettings().view.colorGrading;

        ImGui::Indent();
        ImGui::Checkbox("Enabled##colorGrading", &colorGrading.enabled);

        int quality = (int) colorGrading.quality;
        ImGui::Combo("Quality##colorGradingQuality", &quality, "Low\0Medium\0High\0Ultra\0\0");
        colorGrading.quality = (decltype(colorGrading.quality)) quality;

        int toneMapping = (int) colorGrading.toneMapping;
        ImGui::Combo("Tone-mapping", &toneMapping,
                "Linear\0ACES (legacy)\0ACES\0Filmic\0Uchimura\0Reinhard\0Display Range\0\0");
        colorGrading.toneMapping = (decltype(colorGrading.toneMapping)) toneMapping;

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
            computeRangePlot(app, app.rangePlot);
            ImGuiExt::PlotLinesSeries("", 3,
                    rangePlotSeriesStart, getRangePlotValue, rangePlotSeriesEnd,
                    app.rangePlot, 1024, 0, "", 0.0f, 1.0f, plotLinesWideSize);
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

            computeCurvePlot(app, app.curvePlot);

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
                ImGui::PlotLines("", app.curvePlot, 1024, 0, "Red", 0.0f, 2.0f, plotLinesSize);
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
                ImGui::PlotLines("", app.curvePlot + 1024, 1024, 0, "Green", 0.0f, 2.0f, plotLinesSize);
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
                ImGui::PlotLines("", app.curvePlot + 2048, 1024, 0, "Blue", 0.0f, 2.0f, plotLinesSize);
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
                ImGui::PlotLines("", app.curvePlot, 1024, 0, "RGB", 0.0f, 2.0f, plotLinesSize);
                ImGui::PopStyleColor();
            }
        }
        ImGui::Unindent();
    }
}

static LinearColor inverseTonemapSRGB(sRGBColor x) {
    return (x * -0.155) / (x - 1.019);
}

int main(int argc, char** argv) {
    App app;

    app.config.title = "Filament";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;

    int optionIndex = handleCommandLineArguments(argc, argv, &app);

    utils::Path filename;
    int num_args = argc - optionIndex;
    if (num_args >= 1) {
        filename = argv[optionIndex];
        if (!filename.exists()) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
        if (filename.isDirectory()) {
            auto files = filename.listContents();
            for (auto file : files) {
                if (file.getExtension() == "gltf" || file.getExtension() == "glb") {
                    filename = file;
                    break;
                }
            }
            if (filename.isDirectory()) {
                std::cerr << "no glTF file found in " << filename << std::endl;
                return 1;
            }
        }
    }

    auto loadAsset = [&app](utils::Path filename) {
        // Peek at the file size to allow pre-allocation.
        long contentSize = static_cast<long>(getFileSize(filename.c_str()));
        if (contentSize <= 0) {
            std::cerr << "Unable to open " << filename << std::endl;
            exit(1);
        }

        // Consume the glTF file.
        std::ifstream in(filename.c_str(), std::ifstream::binary | std::ifstream::in);
        std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
        if (!in.read((char*) buffer.data(), contentSize)) {
            std::cerr << "Unable to read " << filename << std::endl;
            exit(1);
        }

        // Parse the glTF file and create Filament entities.
        if (filename.getExtension() == "glb") {
            app.asset = app.assetLoader->createAssetFromBinary(buffer.data(), buffer.size());
        } else {
            app.asset = app.assetLoader->createAssetFromJson(buffer.data(), buffer.size());
        }
        buffer.clear();
        buffer.shrink_to_fit();

        if (!app.asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }
    };

    auto loadResources = [&app] (utils::Path filename) {
        // Load external textures and buffers.
        std::string gltfPath = filename.getAbsolutePath();
        ResourceConfiguration configuration = {};
        configuration.engine = app.engine;
        configuration.gltfPath = gltfPath.c_str();
        configuration.recomputeBoundingBoxes = app.recomputeAabb;
        configuration.normalizeSkinningWeights = true;
        if (!app.resourceLoader) {
            app.resourceLoader = new gltfio::ResourceLoader(configuration);
        }
        app.resourceLoader->asyncBeginLoad(app.asset);

        // Load animation data then free the source hierarchy.
        app.asset->getAnimator();
        app.asset->releaseSourceData();

        auto ibl = FilamentApp::get().getIBL();
        if (ibl) {
            app.viewer->setIndirectLight(ibl->getIndirectLight(), ibl->getSphericalHarmonics());
        }
    };

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());
        app.viewer = new SimpleViewer(engine, scene, view, 410);

        const bool batchMode = !app.batchFile.empty();

        // First check if a custom automation spec has been provided. If it fails to load, the app
        // must be closed since it could be invoked from a script.
        if (batchMode && app.batchFile != "default") {
            auto size = getFileSize(app.batchFile.c_str());
            if (size > 0) {
                std::ifstream in(app.batchFile, std::ifstream::binary | std::ifstream::in);
                std::vector<char> json(static_cast<unsigned long>(size));
                in.read(json.data(), size);
                app.automationSpec = AutomationSpec::generate(json.data(), size);
                if (!app.automationSpec) {
                    std::cerr << "Unable to parse automation spec: " << app.batchFile << std::endl;
                    exit(1);
                }
            } else {
                std::cerr << "Unable to load automation spec: " << app.batchFile << std::endl;
                exit(1);
            }
        }

        // If no custom spec has been provided, or if in interactive mode, load the default spec.
        if (!app.automationSpec) {
            app.automationSpec = AutomationSpec::generateDefaultTestCases();
        }

        app.automationEngine = new AutomationEngine(app.automationSpec, &app.viewer->getSettings());

        if (batchMode) {
            app.automationEngine->startBatchMode();
            auto options = app.automationEngine->getOptions();
            options.sleepDuration = 0.0;
            options.exportScreenshots = true;
            options.exportSettings = true;
            app.automationEngine->setOptions(options);
            app.viewer->stopAnimation();
        }

        if (app.settingsFile.size() > 0) {
            bool success = loadSettings(app.settingsFile.c_str(), &app.viewer->getSettings());
            if (success) {
                std::cout << "Loaded settings from " << app.settingsFile << std::endl;
            } else {
                std::cerr << "Failed to load settings from " << app.settingsFile << std::endl;
            }
        }

        app.materials = (app.materialSource == GENERATE_SHADERS) ?
                createMaterialGenerator(engine) : createUbershaderLoader(engine);
        app.assetLoader = AssetLoader::create({engine, app.materials, app.names });
        app.mainCamera = &view->getCamera();
        if (filename.isEmpty()) {
            app.asset = app.assetLoader->createAssetFromBinary(
                    GLTF_VIEWER_DAMAGEDHELMET_DATA,
                    GLTF_VIEWER_DAMAGEDHELMET_SIZE);
        } else {
            loadAsset(filename);
        }

        loadResources(filename);

        createGroundPlane(engine, scene, app);

        app.viewer->setUiCallback([&app, scene, view, engine] () {
            auto& automation = *app.automationEngine;

            float progress = app.resourceLoader->asyncGetLoadProgress();
            if (progress < 1.0) {
                ImGui::ProgressBar(progress);
            } else {
                // The model is now fully loaded, so let automation know.
                automation.signalBatchMode();
            }

            // The screenshots do not include the UI, but we auto-open the Automation UI group
            // when in batch mode. This is useful when a human is observing progress.
            const int flags = automation.isBatchModeEnabled() ? ImGuiTreeNodeFlags_DefaultOpen : 0;

            if (ImGui::CollapsingHeader("Automation", flags)) {
                ImGui::Indent();

                const ImVec4 yellow(1.0f,1.0f,0.0f,1.0f);
                if (automation.isRunning()) {
                    ImGui::TextColored(yellow, "Test case %zu / %zu",
                            automation.currentTest(), automation.testCount());
                } else {
                    ImGui::TextColored(yellow, "%zu test cases", automation.testCount());
                }

                auto options = automation.getOptions();

                ImGui::PushItemWidth(150);
                ImGui::SliderFloat("Sleep (seconds)", &options.sleepDuration, 0.0, 5.0);
                ImGui::PopItemWidth();

                // Hide the tooltip during automation to avoid photobombing the screenshot.
                if (ImGui::IsItemHovered() && !automation.isRunning()) {
                    ImGui::SetTooltip("Specifies the amount of time to sleep between test cases.");
                }

                ImGui::Checkbox("Export screenshot for each test", &options.exportScreenshots);
                ImGui::Checkbox("Export settings JSON for each test", &options.exportSettings);

                automation.setOptions(options);

                if (automation.isRunning()) {
                    if (ImGui::Button("Stop batch test")) {
                        automation.stopRunning();
                    }
                } else if (ImGui::Button("Run batch test")) {
                    automation.startRunning();
                }

                if (ImGui::Button("Export view settings")) {
                    automation.exportSettings(app.viewer->getSettings(), "settings.json");
                    app.messageBoxText = automation.getStatusMessage();
                    ImGui::OpenPopup("MessageBox");
                }
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Stats")) {
                ImGui::Indent();
                ImGui::Text("%zu entities in the asset", app.asset->getEntityCount());
                ImGui::Text("%zu renderables (excluding UI)", scene->getRenderableCount());
                ImGui::Text("%zu skipped frames", FilamentApp::get().getSkippedFrameCount());
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Scene")) {
                ImGui::Indent();
                ImGui::Checkbox("Show skybox", &app.viewOptions.skyboxEnabled);
                ImGui::ColorEdit3("Background color", &app.viewOptions.backgroundColor.r);
                ImGui::Checkbox("Ground shadow", &app.viewOptions.groundPlaneEnabled);
                ImGui::Indent();
                ImGui::SliderFloat("Strength", &app.viewOptions.groundShadowStrength, 0.0f, 1.0f);
                ImGui::Unindent();
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Camera")) {
                ViewSettings& settings = app.viewer->getSettings().view;

                ImGui::Indent();
                ImGui::SliderFloat("Focal length (mm)", &FilamentApp::get().getCameraFocalLength(), 16.0f, 90.0f);
                ImGui::SliderFloat("Aperture", &app.viewOptions.cameraAperture, 1.0f, 32.0f);
                ImGui::SliderFloat("Speed (1/s)", &app.viewOptions.cameraSpeed, 1000.0f, 1.0f);
                ImGui::SliderFloat("ISO", &app.viewOptions.cameraISO, 25.0f, 6400.0f);
                ImGui::Checkbox("DoF", &settings.dof.enabled);
                ImGui::SliderFloat("Focus distance", &settings.dof.focusDistance, 0.0f, 30.0f);
                ImGui::SliderFloat("Blur scale", &settings.dof.cocScale, 0.1f, 10.0f);

                if (ImGui::CollapsingHeader("Vignette")) {
                    ImGui::Checkbox("Enabled##vignetteEnabled", &settings.vignette.enabled);
                    ImGui::SliderFloat("Mid point", &settings.vignette.midPoint, 0.0f, 1.0f);
                    ImGui::SliderFloat("Roundness", &settings.vignette.roundness, 0.0f, 1.0f);
                    ImGui::SliderFloat("Feather", &settings.vignette.feather, 0.0f, 1.0f);
                    ImGui::ColorEdit3("Color##vignetteColor", &settings.vignette.color.r);
                }

                const utils::Entity* cameras = app.asset->getCameraEntities();
                const size_t cameraCount = app.asset->getCameraEntityCount();

                std::vector<std::string> names;
                names.reserve(cameraCount + 1);
                names.push_back("Free camera");
                int c = 0;
                for (size_t i = 0; i < cameraCount; i++) {
                    const char* n = app.asset->getName(cameras[i]);
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

                ImGui::ListBox("Cameras", &app.currentCamera, cstrings.data(), cstrings.size());
                ImGui::Unindent();
            }

            colorGradingUI(app);

            if (ImGui::CollapsingHeader("Debug")) {
                if (ImGui::Button("Capture frame")) {
                    auto& debug = engine->getDebugRegistry();
                    bool* captureFrame =
                        debug.getPropertyAddress<bool>("d.renderer.doFrameCapture");
                    *captureFrame = true;
                }
            }

            if (ImGui::BeginPopupModal("MessageBox", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%s", app.messageBoxText.c_str());
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        });
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        app.automationEngine->terminate();
        app.resourceLoader->asyncCancelLoad();
        app.assetLoader->destroyAsset(app.asset);
        app.materials->destroyMaterials();

        engine->destroy(app.scene.groundPlane);
        engine->destroy(app.scene.groundVertexBuffer);
        engine->destroy(app.scene.groundIndexBuffer);
        engine->destroy(app.scene.groundMaterial);
        engine->destroy(app.colorGrading);

        delete app.viewer;
        delete app.materials;
        delete app.names;

        AssetLoader::destroy(&app.assetLoader);
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        app.resourceLoader->asyncUpdateLoad();

        // Add renderables to the scene as they become ready.
        app.viewer->populateScene(app.asset, !app.actualSize);

        app.viewer->applyAnimation(now);
    };

    auto resize = [&app](Engine* engine, View* view) {
        Camera& camera = view->getCamera();
        if (&camera == app.mainCamera) {
            // Don't adjut the aspect ratio of the main camera, this is done inside of
            // FilamentApp.cpp
            return;
        }
        const Viewport& vp = view->getViewport();
        double aspectRatio = (double) vp.width / vp.height;
        camera.setScaling(double4 {1.0 / aspectRatio, 1.0, 1.0, 1.0});
    };

    auto gui = [&app](Engine* engine, View* view) {
        app.viewer->updateUserInterface();

        FilamentApp::get().setSidebarWidth(app.viewer->getSidebarWidth());
    };

    auto preRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        auto& rcm = engine->getRenderableManager();
        auto instance = rcm.getInstance(app.scene.groundPlane);
        rcm.setLayerMask(instance,
                0xff, app.viewOptions.groundPlaneEnabled ? 0xff : 0x00);

        const size_t cameraCount = app.asset->getCameraEntityCount();
        view->setCamera(app.mainCamera);
        if (app.currentCamera > 0) {
            const int gltfCamera = app.currentCamera - 1;
            if (gltfCamera < cameraCount) {
                const utils::Entity* cameras = app.asset->getCameraEntities();
                Camera* c = engine->getCameraComponent(cameras[gltfCamera]);
                assert(c);
                view->setCamera(c);

                // Override the aspect ratio in the glTF file and adjust the aspect ratio of this
                // camera to the viewport.
                const Viewport& vp = view->getViewport();
                double aspectRatio = (double) vp.width / vp.height;
                c->setScaling(double4 {1.0 / aspectRatio, 1.0, 1.0, 1.0});
            } else {
                // gltfCamera is out of bounds. Reset camera selection to main camera.
                app.currentCamera = 0;
            }
        }

        Camera& camera = view->getCamera();
        camera.setExposure(
                app.viewOptions.cameraAperture,
                1.0f / app.viewOptions.cameraSpeed,
                app.viewOptions.cameraISO);

        app.scene.groundMaterial->setDefaultParameter(
                "strength", app.viewOptions.groundShadowStrength);

        auto ibl = FilamentApp::get().getIBL();
        if (ibl) {
            ibl->getSkybox()->setLayerMask(0xff, app.viewOptions.skyboxEnabled ? 0xff : 0x00);
        }

        // we have to clear because the side-bar doesn't have a background, we cannot use
        // a skybox on the ui scene, because the ui view is always full screen.
        renderer->setClearOptions({
                .clearColor = { inverseTonemapSRGB(app.viewOptions.backgroundColor), 1.0f },
                .clear = true
        });

        ColorGradingSettings& options = app.viewer->getSettings().view.colorGrading;
        if (options.enabled) {
            if (options != app.lastColorGradingOptions) {
                ColorGrading *colorGrading = createColorGrading(options, engine);
                engine->destroy(app.colorGrading);
                app.colorGrading = colorGrading;
                app.lastColorGradingOptions = options;
            }
            view->setColorGrading(app.colorGrading);
        } else {
            view->setColorGrading(nullptr);
        }
    };

    auto postRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        if (app.automationEngine->shouldClose()) {
            FilamentApp::get().close();
            return;
        }
        Settings* settings = &app.viewer->getSettings();
        MaterialInstance* const* materials = app.asset->getMaterialInstances();
        size_t materialCount = app.asset->getMaterialInstanceCount();
        app.automationEngine->tick(view, materials, materialCount, renderer,
                ImGui::GetIO().DeltaTime);
    };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.animate(animate);
    filamentApp.resize(resize);

    filamentApp.setDropHandler([&] (std::string path) {
        app.resourceLoader->asyncCancelLoad();
        app.viewer->removeAsset();
        app.assetLoader->destroyAsset(app.asset);
        loadAsset(path);
        loadResources(path);
    });

    filamentApp.run(app.config, setup, cleanup, gui, preRender, postRender);

    return 0;
}
