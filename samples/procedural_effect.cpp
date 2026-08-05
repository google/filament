/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "generated/resources/resources.h"

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/getopt.h>
#include <utils/Path.h>

#include <stb_image.h>

#include <iostream>
#include <string>

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

struct App {
    FilamentApp2* filamentApp = nullptr;
    SampleConfig config;
    VertexBuffer* vb = nullptr;
    Material* mat = nullptr;
    MaterialInstance* matInstance = nullptr;
    Skybox* skybox = nullptr;
    Entity renderable;
    Entity camera;
    Camera* cam = nullptr;
    double startTime = 0.0;
};

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;
    auto setup = [app](Engine* engine, View* view, Scene* scene) {
        app->mat =
                Material::Builder()
                        .package(RESOURCES_PROCEDURALEFFECT_DATA, RESOURCES_PROCEDURALEFFECT_SIZE)
                        .build(*engine);
        app->matInstance = app->mat->createInstance();

        // Attribute-less VertexBuffer: no attributes, no buffer slots.
        // The vertex count tells the draw call how many vertices to emit; the
        // vertex shader generates positions and UVs from getVertexIndex().
        app->vb = VertexBuffer::Builder().vertexCount(6).bufferCount(0).build(*engine);

        app->renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                // Use the non-indexed geometry overload that omits the IndexBuffer parameter
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app->vb)
                .material(0, app->matInstance)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app->renderable);
        scene->addEntity(app->renderable);

        app->skybox = Skybox::Builder().color({ 0.1f, 0.125f, 0.25f, 1.0f }).build(*engine);
        scene->setSkybox(app->skybox);
        view->setPostProcessingEnabled(false);

        app->camera = EntityManager::get().create();
        app->cam = engine->createCamera(app->camera);
        view->setCamera(app->cam);
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        engine->destroy(app->skybox);
        engine->destroy(app->renderable);
        engine->destroy(app->matInstance);
        engine->destroy(app->mat);
        engine->destroy(app->vb);
        engine->destroyCameraComponent(app->camera);
        EntityManager::get().destroy(app->camera);
    };


    auto fApp = FilamentApp2::Builder()
                        .displayManager(dm)
                        .title(config.title)
                        .backend(config.backend)
                        .setup(setup)
                        .cleanup(cleanup)
                        .animation([app](Engine*, View* view, double now) {
                            const uint32_t w = view->getViewport().width;
                            const uint32_t h = view->getViewport().height;
                            const float aspect = float(w) / float(h);
                            app->cam->setProjection(Camera::Projection::ORTHO, -aspect, aspect,
                                    -1.0f, 1.0f, 0.0f, 1.0f);

                            if (app->startTime == 0.0) {
                                app->startTime = now;
                            }

                            double elapsed = now - app->startTime;
                            app->matInstance->setParameter("time", (float) elapsed);
                        })
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "procedural_effect";
    int optind = samples::handleCommandLineArguments(argc, argv, &config);
    auto dm = samples::getDisplayManager(config);

    auto app = createSampleApp(config, dm.get(), nullptr);
    app->run();
    return 0;
}
#endif
