/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <filameshio/MeshReader.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <getopt/getopt.h>

#include <imgui.h>

#include <filagui/ImGuiExtensions.h>

#include <iostream>

#include "generated/resources/resources.h"
#include "generated/resources/monkey.h"

using namespace filament;
using namespace filamesh;
using namespace filament::math;

using Backend = Engine::Backend;

struct App {
    Config config;
    Material* material;
    MaterialInstance* materialInstance;
    MeshReader::Mesh mesh;
    mat4f transform;
    Skybox* skybox;
    utils::Entity areaLight;
    sRGBColor areaLightColor = {1.0f, 1.0f, 1.0f};
    float areaLightIntensity = 200000.0f;
    math::float3 areaLightPosition = {0.0f, 1.5f, -3.5f};
    math::float3 areaLightDirection = {0.0f, -1.0f, -0.0f};
    float areaLightWidth = 1.0f;
    float areaLightHeight = 1.0f;
};

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
            "AREALIGHT is a command-line tool for testing the filament engine\n"
            "Usage:\n"
            "    AREALIGHT [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl, vulkan, or metal\n"
    );
    const std::string from("AREALIGHT");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
            { "help", no_argument,       nullptr, 'h' },
            { "api",  required_argument, nullptr, 'a' },
            { nullptr, 0,                nullptr, 0 }
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
                    exit(1);
                }
                break;
        }
    }
    return optind;
}

int main(int argc, char** argv) {
    App app;
    app.config.title = "arealight";
    handleCommandLineArguments(argc, argv, &app);

    auto setup = [config=app.config, &app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();

        // Instantiate material.
        app.material = Material::Builder()
            .package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE).build(*engine);
        auto mi = app.materialInstance = app.material->createInstance();
        mi->setParameter("baseColor", RgbType::LINEAR, float3{0.8});
        mi->setParameter("metallic", 1.0f);
        mi->setParameter("roughness", 0.4f);
        mi->setParameter("reflectance", 0.5f);

        // Add geometry into the scene.
        app.mesh = MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, nullptr, nullptr, mi);
        auto ti = tcm.getInstance(app.mesh.renderable);
        app.transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        rcm.setCastShadows(rcm.getInstance(app.mesh.renderable), false);
        scene->addEntity(app.mesh.renderable);

        // Add light sources into the scene.
        app.areaLight = em.create();
        LightManager::Builder(LightManager::Type::AREA)
                .color(Color::toLinear<ACCURATE>(app.areaLightColor))
                .intensity(app.areaLightIntensity)
                .position(app.areaLightPosition)
                .direction(app.areaLightDirection)
                .width(app.areaLightWidth)
                .height(app.areaLightWidth)
                .castShadows(false)
                .falloff(10.0f)
                .build(*engine, app.areaLight);
        scene->addEntity(app.areaLight);

        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
        scene->setSkybox(app.skybox);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.areaLight);
        engine->destroy(app.materialInstance);
        engine->destroy(app.mesh.renderable);
        engine->destroy(app.mesh.vertexBuffer);
        engine->destroy(app.mesh.indexBuffer);
        engine->destroy(app.material);
        engine->destroy(app.skybox);
    };

    auto gui = [&app](Engine* engine, View*) {
        ImGui::SliderFloat3("Position", &app.areaLightPosition.x, -5.0f, 5.0f);
        ImGui::ColorEdit3("Color##areaLight", &app.areaLightColor.r);
        ImGui::SliderFloat("Lumens", &app.areaLightIntensity, 0.0, 1000000.f);
        ImGui::SliderFloat("Width", &app.areaLightWidth, 0.0f, 10.0f);
        ImGui::SliderFloat("Height", &app.areaLightHeight, 0.0f, 10.0f);
        ImGuiExt::DirectionWidget("Direction", app.areaLightDirection.v);

        auto& lcm = engine->getLightManager();
        auto areaLightInstance = lcm.getInstance(app.areaLight);
        lcm.setColor(areaLightInstance, app.areaLightColor);
        lcm.setIntensity(areaLightInstance, app.areaLightIntensity);
        lcm.setPosition(areaLightInstance, app.areaLightPosition);
        lcm.setDirection(areaLightInstance, app.areaLightDirection);
        lcm.setWidth(areaLightInstance, app.areaLightWidth);
        lcm.setHeight(areaLightInstance, app.areaLightHeight);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        auto& tcm = engine->getTransformManager();
        auto ti = tcm.getInstance(app.mesh.renderable);
        tcm.setTransform(ti, app.transform * mat4f::rotation(now, float3{ 0, 1, 0 }));
    });

    FilamentApp::get().run(app.config, setup, cleanup, gui);

    return 0;
}
