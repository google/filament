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

#include "generated/resources/resources.h"

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
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <cmath>

using namespace filament;
using utils::Entity;
using utils::EntityManager;

namespace {
struct App {
    SampleConfig config;
    FilamentApp2* filamentApp;
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable;
};
} // anonymous namespace


struct Vertex {
    filament::math::float2 position;
    uint32_t color;
};

static Vertex TRIANGLE_VERTICES[3] = {
    {{1, 0}, 0xffff0000u},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;
    auto setup = [app](Engine* engine, View* view, Scene* scene) {
        app->skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*engine);
        scene->setSkybox(app->skybox);
        view->setPostProcessingEnabled(false);
        app->vb = VertexBuffer::Builder()
                          .vertexCount(3)
                          .bufferCount(1)
                          .attribute(VertexAttribute::POSITION, 0,
                                  VertexBuffer::AttributeType::FLOAT2, 0, 12)
                          .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4,
                                  8, 12)
                          .normalized(VertexAttribute::COLOR)
                          .build(*engine);
        app->vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
        app->ib = IndexBuffer::Builder()
                          .indexCount(3)
                          .bufferType(IndexBuffer::IndexType::USHORT)
                          .build(*engine);
        app->ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        app->mat = Material::Builder()
                           .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
                           .build(*engine);
        app->renderable = EntityManager::get().create();
        scene->addEntity(app->renderable);
        app->camera = utils::EntityManager::get().create();
        app->cam = engine->createCamera(app->camera);
        view->setCamera(app->cam);
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        engine->destroy(app->skybox);
        engine->destroy(app->renderable);
        engine->destroy(app->mat);
        engine->destroy(app->vb);
        engine->destroy(app->ib);

        engine->destroyCameraComponent(app->camera);
        utils::EntityManager::get().destroy(app->camera);
    };


    auto fApp = FilamentApp2::Builder()
                        .displayManager(dm)
                        .title(app->config.title)
                        .backend(app->config.backend)
                        .setup(setup)
                        .cleanup(cleanup)
                        .animation([app](Engine* engine, View* view, double now) {

#if 0
        engine->destroy(app->vb);
        auto vb = app->vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
#else
                            auto vb = app->vb;
#endif

                            void* verts = malloc(36);
                            TRIANGLE_VERTICES[0].position.y = sin(now * 4);
                            memcpy(verts, TRIANGLE_VERTICES, 36);
                            vb->setBufferAt(*engine, 0,
                                    VertexBuffer::BufferDescriptor(verts, 36,
                                            (VertexBuffer::BufferDescriptor::Callback) free));

                            auto& rcm = engine->getRenderableManager();
                            rcm.destroy(app->renderable);
                            RenderableManager::Builder(1)
                                    .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
                                    .material(0, app->mat->getDefaultInstance())
                                    .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                                            app->vb, app->ib, 0, 3)
                                    .culling(false)
                                    .receiveShadows(false)
                                    .castShadows(false)
                                    .build(*engine, app->renderable);

                            constexpr float ZOOM = 1.5f;
                            const uint32_t w = view->getViewport().width;
                            const uint32_t h = view->getViewport().height;
                            const float aspect = (float) w / h;
                            app->cam->setProjection(Camera::Projection::ORTHO, -aspect * ZOOM,
                                    aspect * ZOOM, -ZOOM, ZOOM, 0, 1);
                            auto& tcm = engine->getTransformManager();
                            tcm.setTransform(tcm.getInstance(app->renderable),
                                    filament::math::mat4f::rotation(now,
                                            filament::math::float3{ 0, 0, 1 }));
                        })
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "animation";
    samples::handleCommandLineArguments(argc, argv, &config);
    auto dm = samples::getDisplayManager(config);
    auto fApp = createSampleApp(config, dm.get(), nullptr);
    fApp->run();
    return 0;
}
#endif
