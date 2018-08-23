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
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include "../samples/app/Config.h"
#include "../samples/app/FilamentApp.h"
#include "../samples/app/MeshAssimp.h"

using namespace filament;
using namespace math;
using Backend = Engine::Backend;

struct App {
    utils::Entity light;
    std::map<std::string, MaterialInstance*> materials;
    MeshAssimp* meshes;
    mat4f transform;
};

static const char* MODEL_FILE = "assets/models/monkey/monkey.obj";
static const char* IBL_FOLDER = "envs/office";

int main(int argc, char** argv) {
    Config config;
    config.title = "hellopbr";
    config.backend = Backend::VULKAN;
    config.iblDirectory = FilamentApp::getRootPath() + IBL_FOLDER;

    App app;
    auto setup = [config, &app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();

        // Add geometry into the scene.
        app.meshes = new MeshAssimp(*engine);
        app.meshes->addFromFile(FilamentApp::getRootPath() + MODEL_FILE, app.materials);
        auto ti = tcm.getInstance(app.meshes->getRenderables()[0]);
        app.transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        for (auto renderable : app.meshes->getRenderables()) {
            if (rcm.hasComponent(renderable)) {
                rcm.setCastShadows(rcm.getInstance(renderable), false);
                scene->addEntity(renderable);
            }
        }

        // Enable the metallic surface property to observe reflections.
        for (auto& pair : app.materials) {
            pair.second->setParameter("metallic", 1.0f);
        }

        // Add light sources into the scene.
        app.light = em.create();
        LightManager::Builder(LightManager::Type::SUN)
                .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                .intensity(110000)
                .direction({ 0.7, -1, -0.8 })
                .sunAngularRadius(1.9f)
                .castShadows(false)
                .build(*engine, app.light);
        scene->addEntity(app.light);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        Fence::waitAndDestroy(engine->createFence());
        engine->destroy(app.light);
        for (auto& item : app.materials) {
            engine->destroy(item.second);
        }
        delete app.meshes;
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        auto& tcm = engine->getTransformManager();
        auto ti = tcm.getInstance(app.meshes->getRenderables()[0]);
        tcm.setTransform(ti, app.transform * mat4f::rotate(now, float3{0, 1, 0}));
    });

    FilamentApp::get().run(config, setup, cleanup);
}
