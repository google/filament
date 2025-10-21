/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "common/arguments.h"

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <filameshio/MeshReader.h>

#include <getopt/getopt.h>

#include <imgui.h>

#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "generated/resources/monkey.h"
#include "generated/resources/resources.h"

using namespace filament;
using namespace filament::math;
using namespace utils;
using namespace filamesh;

struct App {
    struct UiState {
        int objectCountSlider = 1000;
        float updateRatio = 0.5f;
        bool animate = true;
    } ui;

    int currentObjectCount = 0;
    int desiredObjectCount = 1000;
    uint32_t frameCounter = 0;

    Material* litMaterial = nullptr;
    MeshReader::Mesh suzanneTemplate;
    Entity light;

    std::vector<MaterialInstance*> materialInstances;
    std::vector<Entity> renderables;
};

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "MATERIALINSTANCESTRESS renders a lot of suzanne models and changes their material "
            "properties periodically.\n"
            "Usage:\n"
            "    MATERIALINSTANCESTRESS [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "API_USAGE");
    const std::string from("MATERIALINSTANCESTRESS");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos; pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), samples::getBackendAPIArgumentsUsage());
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
            { "help", no_argument, nullptr, 'h' },
            { "api",  required_argument, nullptr, 'a' },
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
                config->backend = samples::parseArgumentsForBackend(arg);
                break;
        }
    }
    return optind;
}

static void clearScene(Engine* engine, Scene* scene, App& app) {
    EntityManager& em = EntityManager::get();

    for (Entity e : app.renderables) {
        engine->destroy(e);
        em.destroy(e);
    }
    app.renderables.clear();

    for (const MaterialInstance* mi : app.materialInstances) {
        engine->destroy(mi);
    }
    app.materialInstances.clear();

    app.currentObjectCount = 0;
}

static void createSceneObjects(Engine* engine, Scene* scene, App& app) {
    TransformManager& tcm = engine->getTransformManager();
    EntityManager& em = EntityManager::get();

    const int gridDim = static_cast<int>(sqrt(app.desiredObjectCount));
    const float spacing = 2.5f;

    if (app.desiredObjectCount == 0) {
        app.currentObjectCount = 0;
        return;
    }

    app.renderables.reserve(app.desiredObjectCount);
    app.materialInstances.reserve(app.desiredObjectCount);

    for (int i = 0; i < app.desiredObjectCount; ++i) {
        const int x = i % gridDim;
        const int y = i / gridDim;

        float3 position = {
            (x - gridDim / 2.0f) * spacing,
            (y - gridDim / 2.0f) * spacing,
            -20.0f
        };

        MaterialInstance* mi = app.litMaterial->createInstance();
        mi->setParameter("baseColor", RgbType::sRGB, float3{1.0f});
        mi->setParameter("metallic", 0.0f);
        mi->setParameter("roughness", 0.4f);
        app.materialInstances.push_back(mi);

        Entity renderable = em.create();
        RenderableManager::Builder(1)
                .boundingBox({{-1, -1, -1}, {1, 1, 1}})
                .material(0, mi)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                        app.suzanneTemplate.vertexBuffer, app.suzanneTemplate.indexBuffer)
                .culling(true)
                .castShadows(false)
                .build(*engine, renderable);

        scene->addEntity(renderable);
        app.renderables.push_back(renderable);

        tcm.setTransform(tcm.getInstance(renderable), mat4f::translation(position));
    }
    app.currentObjectCount = app.desiredObjectCount;
}


int main(int argc, char** argv) {
    Config config;
    config.title = "Material Instances Stress Test";
    config.iblDirectory = FilamentApp::getRootAssetsPath() + "assets/ibl/lightroom_14b";
    handleCommandLineArguments(argc, argv, &config);

    App app;

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.desiredObjectCount = app.ui.objectCountSlider;
        app.suzanneTemplate = MeshReader::loadMeshFromBuffer(engine,
                MONKEY_SUZANNE_DATA, nullptr, nullptr, nullptr);

        app.litMaterial = Material::Builder()
                .package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE)
                .build(*engine);

        createSceneObjects(engine, scene, app);

        app.light = EntityManager::get().create();
        LightManager::Builder(LightManager::Type::SUN)
                .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                .intensity(110000)
                .direction({0.7, -1, -0.8})
                .sunAngularRadius(1.9f)
                .castShadows(false)
                .build(*engine, app.light);
        scene->addEntity(app.light);
    };

    auto cleanup = [&app](Engine* engine, View* view, Scene* scene) {
        clearScene(engine, scene, app);

        engine->destroy(app.light);
        EntityManager& em = EntityManager::get();
        em.destroy(app.light);

        engine->destroy(app.litMaterial);
        engine->destroy(app.suzanneTemplate.renderable);
        engine->destroy(app.suzanneTemplate.vertexBuffer);
        engine->destroy(app.suzanneTemplate.indexBuffer);
    };

    auto gui = [&app](Engine* engine, View* view) {
        ImGui::Begin("Material Instance Stress Test Controls");
        ImGui::Text("Objects: %d", app.currentObjectCount);
        ImGui::SliderInt("Object Count", &app.ui.objectCountSlider, 100, 10000);

        if (ImGui::Button("Apply")) {
            app.desiredObjectCount = app.ui.objectCountSlider;
        }
        ImGui::SliderFloat("Update Ratio / Frame", &app.ui.updateRatio, 0.0f, 1.0f);
        ImGui::Checkbox("Animate", &app.ui.animate);
        ImGui::End();
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        if (app.currentObjectCount != app.desiredObjectCount) {
            clearScene(engine, view->getScene(), app);
            engine->flushAndWait();
            createSceneObjects(engine, view->getScene(), app);
        }

        if (!app.ui.animate || app.currentObjectCount == 0) {
            return;
        }

        const int updateCount = static_cast<int>(app.currentObjectCount * app.ui.updateRatio);
        const uint32_t frame = app.frameCounter++;
        for (int i = 0; i < updateCount; ++i) {
            const int instanceIndex = (frame * i) % app.currentObjectCount;
            MaterialInstance* mi = app.materialInstances[instanceIndex];
            const int gridDim = static_cast<int>(sqrt(app.desiredObjectCount));

            const float x = static_cast<float>(instanceIndex % gridDim);
            const float y = static_cast<float>(instanceIndex / gridDim);

            const float r = 0.5f + 0.5f * sin(now + (x + y) * 0.2f);
            const float g = 0.5f + 0.5f * cos(now + (x - y) * 0.2f);
            const float b = 0.5f + 0.5f * sin(now * 0.7f + y * 0.2f);
            mi->setParameter("baseColor", RgbType::sRGB, float3{r, g, b});
        }
    };

    FilamentApp::get().animate(animate);
    FilamentApp::get().run(config, setup, cleanup, gui);

    return 0;
}
