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

#include "generated/resources/resources.h"

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Camera.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <samples/SampleConfig.h>

using namespace filament;

namespace {

struct App {
    FilamentApp2* filamentApp;
    SampleConfig config;
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    utils::Entity camera;
    utils::Entity renderable;
};

constexpr filament::math::float2 POSITIONS[] { {.5, 0}, {-.5, .5}, {-.5, -.5} };
constexpr uint32_t COLORS[] { 0xffff0000u, 0xff00ff00u, 0xff0000ffu };
constexpr uint16_t TRIANGLE_INDICES[] { 0, 1, 2 };
constexpr float ZOOM = 1.5f;

void setCameraProjection(App* app, View* view) {
    const uint32_t w = view->getViewport().width;
    const uint32_t h = view->getViewport().height;
    const float aspect = (float) w / h;
    app->cam->setProjection(Camera::Projection::ORTHO,
        -aspect * ZOOM, aspect * ZOOM,
        -ZOOM, ZOOM, 0, 1);
}

} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    // Aggregate positions and colors into a single buffer without interleaving.
    std::vector<uint8_t> vbo(sizeof(POSITIONS) + sizeof(COLORS));
    memcpy(vbo.data(), POSITIONS, sizeof(POSITIONS));
    memcpy(vbo.data() + sizeof(POSITIONS), COLORS, sizeof(COLORS));

    auto setup = [app, vbo](Engine* engine, View* view, Scene* scene) {
        // Populate vertex buffer.
        app->vb = VertexBuffer::Builder().vertexCount(3).bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 8)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 24, 4)
                .normalized(VertexAttribute::COLOR).build(*engine);
        app->vb->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(vbo.data(), vbo.size(), 0));

        // Populate index buffer.
        app->ib = IndexBuffer::Builder().indexCount(3).bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app->ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, 0));

        // Construct material.
        app->mat = Material::Builder()
                .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE).build(*engine);

        // Construct renderable.
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app->mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app->vb, app->ib, 0, 3)
                .build(*engine, app->renderable = utils::EntityManager::get().create());
        scene->addEntity(app->renderable);

        // Replace the FilamentApp camera with identity.
        app->camera = utils::EntityManager::get().create();
        view->setCamera(app->cam = engine->createCamera(app->camera));
        setCameraProjection(app.get(), view);
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        engine->destroy(app->renderable);
        engine->destroy(app->mat);
        engine->destroy(app->vb);
        engine->destroy(app->ib);
        engine->destroyCameraComponent(app->camera);
        utils::EntityManager::get().destroy(app->camera);
    };

    auto preRender = [app](Engine*, View*, Scene*, Renderer* renderer) {
        renderer->setClearOptions({ .clear = true });
    };

    auto resize = [app](Engine* engine, View* view) {
        setCameraProjection(app.get(), view);
    };

    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .preRender(preRender)
                        .resize(resize)
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

samples::SampleParameters createAppParameters() { return {}; }

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "vbotest";
    samples::handleCommandLineArguments(argc, argv, &config,
            { .parameters = createAppParameters() });

    auto dm = samples::getDisplayManager(config);
    auto fApp = createSampleApp(config, dm.get(), nullptr);
    fApp->run();

    return 0;
}
#endif
