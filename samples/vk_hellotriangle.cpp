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
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include "../samples/app/Config.h"
#include "../samples/app/FilamentApp.h"

#include <cmath>

using namespace filament;
using utils::Entity;
using utils::EntityManager;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity renderable;
};

struct Vertex {
    math::float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[3] = {
    {{1, 0}, 0xffff0000u},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

static constexpr uint8_t BAKED_COLOR_PACKAGE[] = {
    #include "generated/material/bakedColor.inc"
};

int main() {
    Config config;
    config.title = "hellotriangle";
    config.backend = Engine::Backend::VULKAN;

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        view->setClearColor({0.1, 0.125, 0.25, 1.0});
        view->setPostProcessingEnabled(false);
        view->setDepthPrepass(filament::View::DepthPrepass::DISABLED);
        static_assert(sizeof(Vertex) == 12, "Strange vertex size.");
        app.vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
        app.ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        app.mat = Material::Builder()
                .package((void*) BAKED_COLOR_PACKAGE, sizeof(BAKED_COLOR_PACKAGE))
                .build(*engine);
        app.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app.renderable);
        scene->addEntity(app.renderable);
        app.cam = engine->createCamera();
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        Fence::waitAndDestroy(engine->createFence());
        engine->destroy(app.renderable);
        engine->destroy(app.mat);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroy(app.cam);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        constexpr float ZOOM = 1.5f;
        const uint32_t w = view->getViewport().width;
        const uint32_t h = view->getViewport().height;
        const float aspect = (float) w / h;
        app.cam->setProjection(Camera::Projection::ORTHO,
            -aspect * ZOOM, aspect * ZOOM,
            -ZOOM, ZOOM, 0, 1);
        auto& tcm = engine->getTransformManager();
        tcm.setTransform(tcm.getInstance(app.renderable),
             math::mat4f::rotate(now, math::float3{0, 0, 1}));
    });

    FilamentApp::get().run(config, setup, cleanup);
}
