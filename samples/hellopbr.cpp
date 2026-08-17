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

#include "common/arguments.h"
#include "common/SampleConfig.h"

#include "generated/resources/monkey.h"
#include "generated/resources/resources.h"

#include <filameshio/MeshReader.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/getopt.h>

#include <iostream>
#include <string>// for printing usage/help

using namespace filament;
using namespace filamesh;
using namespace filament::math;

using Backend = Engine::Backend;

namespace {
struct App {
    FilamentApp2* filamentApp;
    SampleConfig config;
    utils::Entity light;
    Material* material;
    MaterialInstance* materialInstance;
    MeshReader::Mesh mesh;
    mat4f transform;
};
} // anonymous namespace


#ifdef __ANDROID__
static const char* IBL_FOLDER = "lightroom_14b.hdr";
#else
static const char* IBL_FOLDER = "assets/ibl/lightroom_14b";
#endif

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    auto setup = [app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();

        // Instantiate material.
        app->material = Material::Builder()
                                .package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE)
                                .build(*engine);
        auto mi = app->materialInstance = app->material->createInstance();
        mi->setParameter("baseColor", RgbType::LINEAR, float3{0.8});
        mi->setParameter("metallic", 1.0f);
        mi->setParameter("roughness", 0.4f);
        mi->setParameter("reflectance", 0.5f);

        // Add geometry into the scene.
        app->mesh = MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, MONKEY_SUZANNE_SIZE,
                nullptr, nullptr, mi);
        auto ti = tcm.getInstance(app->mesh.renderable);
        app->transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        rcm.setCastShadows(rcm.getInstance(app->mesh.renderable), false);
        scene->addEntity(app->mesh.renderable);

        // Add light sources into the scene.
        app->light = em.create();
        LightManager::Builder(LightManager::Type::SUN)
                .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                .intensity(110000)
                .direction({ 0.7, -1, -0.8 })
                .sunAngularRadius(1.9f)
                .castShadows(false)
                .build(*engine, app->light);
        scene->addEntity(app->light);
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        engine->destroy(app->light);
        engine->destroy(app->mesh.renderable);
        engine->destroy(app->materialInstance);
        engine->destroy(app->material);
    };

    auto fApp = FilamentApp2::Builder()
                        .displayManager(dm)
                        .assetLoader(loader)
                        .title(app->config.title)
                        .iblDirectory(app->config.iblDirectory.empty() ? utils::CString(IBL_FOLDER) : app->config.iblDirectory)
                        .backend(app->config.backend)
                        .setup(setup)
                        .cleanup(cleanup)
                        .animation([app](Engine* engine, View* view, double now) {
                            auto& tcm = engine->getTransformManager();
                            auto ti = tcm.getInstance(app->mesh.renderable);
                            tcm.setTransform(ti,
                                    app->transform * mat4f::rotation(now, float3{ 0, 1, 0 }));
                        })
                        .build();

    app->filamentApp = fApp.get();
    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "hellopbr";
    config.iblDirectory = utils::CString((FilamentApp2::getRootAssetsPath() + IBL_FOLDER).c_str());
    int optind = samples::handleCommandLineArguments(argc, argv, &config);
    auto dm = samples::getDisplayManager(config);

    auto app = createSampleApp(config, dm.get(), nullptr);
    app->run();

    return 0;
}
#endif
