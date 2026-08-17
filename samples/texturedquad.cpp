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

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
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

#include <math/vec2.h>

#include "generated/resources/resources.h"

#include <stb_image.h>

#include <cmath>
#include <iostream>
#include <memory>
#include <string>

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

namespace {
struct App {
    FilamentApp2* filamentApp = nullptr;
    SampleConfig config;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    Material* mat = nullptr;
    MaterialInstance* matInstance = nullptr;
    Camera* cam = nullptr;
    Entity camera;
    Skybox* skybox = nullptr;
    Texture* tex = nullptr;
    Entity renderable;
};
} // anonymous namespace


struct Vertex {
    filament::math::float2 position;
    filament::math::float2 uv;
};

static const Vertex QUAD_VERTICES[4] = {
    {{-1, -1}, {0, 0}},
    {{ 1, -1}, {1, 0}},
    {{-1,  1}, {0, 1}},
    {{ 1,  1}, {1, 1}},
};

static constexpr uint16_t QUAD_INDICES[6] = {
    0, 1, 2,
    3, 2, 1,
};

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    auto setup = [app, loader](Engine* engine, View* view, Scene* scene) {
        // Load texture
        int w = 0, h = 0, n = 0;
        unsigned char* data = nullptr;
        if (loader) {
            auto buf = loader->load("textures/Moss_01/Moss_01_Color.png");
            if (buf.empty()) {
                buf = loader->load("Moss_01/Moss_01_Color.png");
            }
            if (!buf.empty()) {
                data = stbi_load_from_memory(buf.data(), buf.size(), &w, &h, &n, 4);
            }
        }
        if (!data) {
            Path path = FilamentApp2::getRootAssetsPath() + "textures/Moss_01/Moss_01_Color.png";
            if (!path.exists()) {
                path = Path("textures/Moss_01/Moss_01_Color.png");
            }
            if (path.exists()) {
                data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, 4);
            }
        }
        if (data == nullptr) {
            std::cerr << "The texture could not be loaded" << std::endl;
            return;
        }
        std::cout << "Loaded texture: " << w << "x" << h << std::endl;
        Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 4),
                Texture::Format::RGBA, Texture::Type::UBYTE,
                [](void* buffer, size_t, void*) { stbi_image_free(buffer); });
        app->tex = Texture::Builder()
                           .width(uint32_t(w))
                           .height(uint32_t(h))
                           .levels(1)
                           .sampler(Texture::Sampler::SAMPLER_2D)
                           .format(Texture::InternalFormat::RGBA8)
                           .build(*engine);
        app->tex->setImage(*engine, 0, std::move(buffer));
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);

        // Set up view
        app->skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*engine);
        scene->setSkybox(app->skybox);

        view->setPostProcessingEnabled(false);
        app->camera = utils::EntityManager::get().create();
        app->cam = engine->createCamera(app->camera);
        view->setCamera(app->cam);

        // Create quad renderable
        static_assert(sizeof(Vertex) == 16, "Strange vertex size.");
        app->vb = VertexBuffer::Builder()
                          .vertexCount(4)
                          .bufferCount(1)
                          .attribute(VertexAttribute::POSITION, 0,
                                  VertexBuffer::AttributeType::FLOAT2, 0, 16)
                          .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
                                  8, 16)
                          .build(*engine);
        app->vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(QUAD_VERTICES, 64, nullptr));
        app->ib = IndexBuffer::Builder()
                          .indexCount(6)
                          .bufferType(IndexBuffer::IndexType::USHORT)
                          .build(*engine);
        app->ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(QUAD_INDICES, 12, nullptr));
        app->mat = Material::Builder()
                           .package(RESOURCES_BAKEDTEXTURE_DATA, RESOURCES_BAKEDTEXTURE_SIZE)
                           .build(*engine);
        app->matInstance = app->mat->createInstance();
        app->matInstance->setParameter("albedo", app->tex, sampler);
        app->renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
                .material(0, app->matInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app->vb, app->ib, 0, 6)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app->renderable);
        scene->addEntity(app->renderable);
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        if (app->skybox) engine->destroy(app->skybox);
        if (app->renderable) {
            engine->destroy(app->renderable);
            EntityManager::get().destroy(app->renderable);
        }
        if (app->matInstance) engine->destroy(app->matInstance);
        if (app->mat) engine->destroy(app->mat);
        if (app->tex) engine->destroy(app->tex);
        if (app->vb) engine->destroy(app->vb);
        if (app->ib) engine->destroy(app->ib);

        if (app->cam) {
            engine->destroyCameraComponent(app->camera);
            utils::EntityManager::get().destroy(app->camera);
        }
    };


    auto fApp = FilamentApp2::Builder()
                        .displayManager(dm)
                        .assetLoader(loader)
                        .title(app->config.title)
                        .backend(app->config.backend)
                        .featureLevel(app->config.featureLevel)
                        .setup(setup)
                        .cleanup(cleanup)
                        .animation([app](Engine* engine, View* view, double now) {
                            if (!app->cam) return;
                            const float zoom = 2.0f + 2.0f * (float) std::sin(now);
                            const uint32_t w = view->getViewport().width;
                            const uint32_t h = view->getViewport().height;
                            const float aspect = (float) w / h;
                            app->cam->setProjection(Camera::Projection::ORTHO, -aspect * zoom,
                                    aspect * zoom, -zoom, zoom, -1, 1);
                        })
                        .build();

    app->filamentApp = fApp.get();

    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "texturedquad";
    int optind = samples::handleCommandLineArguments(argc, argv, &config);
    auto dm = samples::getDisplayManager(config);

    auto fApp = createSampleApp(config, dm.get(), nullptr);
    fApp->run();

    return 0;
}
#endif
