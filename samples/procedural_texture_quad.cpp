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

#include <samples/SampleConfig.h>
#include <stb_image.h>

#include <iostream>

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

namespace {
struct App {
    FilamentApp2* filamentApp;
    SampleConfig config;
    VertexBuffer* vb = nullptr;
    Material* mat = nullptr;
    MaterialInstance* matInstance = nullptr;
    Texture* tex = nullptr;
    Skybox* skybox = nullptr;
    Entity renderable;
    Entity camera;
    Camera* cam = nullptr;
};
} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;
    auto setup = [app](Engine* engine, View* view, Scene* scene) {
        Path path = FilamentApp2::getRootAssetsPath() + "textures/Moss_01/Moss_01_Color.png";
        if (!path.exists()) {
            std::cerr << "The texture " << path.c_str() << " does not exist" << std::endl;
            exit(1);
        }
        int w, h, n;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);
        if (data == nullptr) {
            std::cerr << "The texture " << path.c_str() << " could not be loaded" << std::endl;
            exit(1);
        }
        std::cout << "Loaded texture: " << w << "x" << h << std::endl;
        Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 4),
                Texture::Format::RGBA, Texture::Type::UBYTE,
                (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
        app->tex = Texture::Builder()
                           .width(uint32_t(w))
                           .height(uint32_t(h))
                           .levels(1)
                           .sampler(Texture::Sampler::SAMPLER_2D)
                           .format(Texture::InternalFormat::RGBA8)
                           .build(*engine);
        app->tex->setImage(*engine, 0, std::move(buffer));

        app->mat = Material::Builder()
                           .package(RESOURCES_PROCEDURALTEXTUREQUAD_DATA,
                                   RESOURCES_PROCEDURALTEXTUREQUAD_SIZE)
                           .build(*engine);
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
        app->matInstance = app->mat->createInstance();
        app->matInstance->setParameter("albedo", app->tex, sampler);

        // Attribute-less VertexBuffer: no attributes, no buffer slots.
        // The vertex count tells the draw call how many vertices to emit; the
        // vertex shader generates positions and UVs from getVertexIndex().
        app->vb = VertexBuffer::Builder().vertexCount(6).bufferCount(0).build(*engine);

        app->renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({ { -0.5f, -0.5f, -0.01f }, { 0.5f, 0.5f, 0.01f } })
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
        engine->destroy(app->tex);
        engine->destroy(app->vb);
        engine->destroyCameraComponent(app->camera);
        EntityManager::get().destroy(app->camera);
    };


    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .animation([app](Engine*, View* view, double) {
                            const uint32_t w = view->getViewport().width;
                            const uint32_t h = view->getViewport().height;
                            const float aspect = float(w) / float(h);
                            app->cam->setProjection(Camera::Projection::ORTHO, -aspect, aspect,
                                    -1.0f, 1.0f, 0.0f, 1.0f);
                        })
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

samples::SampleParameters createAppParameters() { return {}; }

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "procedural_texture_quad";
    samples::handleCommandLineArguments(argc, argv, &config,
            { .parameters = createAppParameters() });
    auto dm = samples::getDisplayManager(config);

    auto app = createSampleApp(config, dm.get(), nullptr);
    app->run();
    return 0;
}
#endif
