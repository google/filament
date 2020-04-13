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

#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <filamentapp/FilamentApp.h>

#include <utils/EntityManager.h>

using namespace filament;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Camera* cam;
    Skybox* skybox;
    utils::Entity renderable;
};

static const filament::math::float2 TRIANGLE_VERTICES[3] = { {1, 0}, {-0.5, 0.866}, {-0.5, -0.866} };
static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

int main(int argc, char** argv) {
    Config config;
    config.title = "viewtest";

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({ 0, 0, 1, 1 }).build(*engine);
        scene->setSkybox(app.skybox);
        view->setViewport({100, 100, 512, 512});
        app.vb = VertexBuffer::Builder()
                .vertexCount(3).bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 8)
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 24, nullptr));
        app.ib = IndexBuffer::Builder()
                .indexCount(3).bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        app.renderable = utils::EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .build(*engine, app.renderable);
        scene->addEntity(app.renderable);
        app.cam = engine->createCamera();
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroy(app.cam);
    };

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
