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
        int objectCountSlider = 100;
        float updateRatio = 0.5f;
        bool animate = true;
        int deltaCount = 5;
    } ui;

    int currentObjectCount = 0;
    int desiredObjectCount = 1000;
    uint32_t frameCounter = 0;
    float spacing = 2.5f;
    float distanceToCamera = -20.0f;
    int gridDim = 1;

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

static void removeObjects(Engine* engine, Scene* scene, App& app, int count) {
    if (count <= 0) return;

    EntityManager& em = EntityManager::get();
    const int removeCount = std::min(count, app.currentObjectCount);

    for (int i = 0; i < removeCount; ++i) {
        Entity renderable = app.renderables.back();
        scene->remove(renderable);
        engine->destroy(renderable);
        em.destroy(renderable);
        app.renderables.pop_back();

        MaterialInstance* mi = app.materialInstances.back();
        engine->destroy(mi);
        app.materialInstances.pop_back();
    }

    app.currentObjectCount = app.renderables.size();
}

static void clearScene(Engine* engine, Scene* scene, App& app) {
    removeObjects(engine, scene, app, app.currentObjectCount);
}

static void addObjects(Engine* engine, Scene* scene, App& app, int count) {
    if (count <= 0) return;

    TransformManager& tcm = engine->getTransformManager();
    EntityManager& em = EntityManager::get();

    int const oldTotal = app.currentObjectCount;
    int const newTotal = oldTotal + count;

    app.renderables.reserve(newTotal);
    app.materialInstances.reserve(newTotal);

    for (int i = oldTotal; i < newTotal; ++i) {
        int const x = i % app.gridDim;
        int const y = i / app.gridDim;

        float3 const position = {
            (x - app.gridDim / 2.0f) * app.spacing,
            (y - app.gridDim / 2.0f) * app.spacing,
            app.distanceToCamera
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

    app.currentObjectCount = newTotal;
}

static void createSceneObjects(Engine* engine, Scene* scene, App& app) {
    app.gridDim = static_cast<int>(ceil(sqrt(app.desiredObjectCount)));
    addObjects(engine, scene, app, std::max(0, app.desiredObjectCount - app.currentObjectCount));
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
        ImGui::SliderInt("Object Count", &app.ui.objectCountSlider, 1, 1000);

        if (ImGui::Button("Apply")) {
            app.desiredObjectCount = app.ui.objectCountSlider;
        }

        ImGui::Separator();
        ImGui::InputInt("Delta Count", &app.ui.deltaCount);
        if (ImGui::Button("Add Objects")) {
            app.desiredObjectCount += app.ui.deltaCount;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove Objects")) {
            app.desiredObjectCount = std::max(0, app.desiredObjectCount - app.ui.deltaCount);
        }
        ImGui::Separator();

        ImGui::SliderFloat("Update Ratio / Frame", &app.ui.updateRatio, 0.0f, 1.0f);
        ImGui::Checkbox("Animate", &app.ui.animate);
        ImGui::End();
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        if (app.currentObjectCount != app.desiredObjectCount) {
            if (app.desiredObjectCount == app.ui.objectCountSlider) {
                clearScene(engine, view->getScene(), app);
                engine->flushAndWait();
                createSceneObjects(engine, view->getScene(), app);
            } else {
                if (app.desiredObjectCount > app.currentObjectCount) {
                    const int toAdd = app.desiredObjectCount - app.currentObjectCount;
                    addObjects(engine, view->getScene(), app, toAdd);
                } else {
                    const int toRemove = app.currentObjectCount - app.desiredObjectCount;
                    removeObjects(engine, view->getScene(), app, toRemove);
                }
            }
            app.ui.objectCountSlider = app.currentObjectCount;
        }

        if (!app.ui.animate || app.currentObjectCount == 0) {
            return;
        }

        const int updateCount = static_cast<int>(app.currentObjectCount * app.ui.updateRatio);
        const uint32_t frame = app.frameCounter++;
        for (int i = 0; i < updateCount; ++i) {
            const int instanceIndex = (frame * i) % app.currentObjectCount;
            MaterialInstance* mi = app.materialInstances[instanceIndex];

            const float x = static_cast<float>(instanceIndex % app.gridDim);
            const float y = static_cast<float>(instanceIndex / app.gridDim);

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
