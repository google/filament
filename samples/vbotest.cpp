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
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <filamentapp/FilamentApp.h>

#include <utils/EntityManager.h>

#include "generated/resources/resources.h"

using namespace filament;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    utils::Entity renderable;
};

static constexpr filament::math::float2 POSITIONS[] { {.5, 0}, {-.5, .5}, {-.5, -.5} };
static constexpr uint32_t COLORS[] { 0xffff0000u, 0xff00ff00u, 0xff0000ffu };
static constexpr uint16_t TRIANGLE_INDICES[] { 0, 1, 2 };

int main(int argc, char** argv) {
    Config config;
    config.title = "vbotest";

    // Aggregate positions and colors into a single buffer without interleaving.
    std::vector<uint8_t> vbo(sizeof(POSITIONS) + sizeof(COLORS));
    memcpy(vbo.data(), POSITIONS, sizeof(POSITIONS));
    memcpy(vbo.data() + sizeof(POSITIONS), COLORS, sizeof(COLORS));

    App app;
    auto setup = [&app, &vbo](Engine* engine, View* view, Scene* scene) {

        // Populate vertex buffer.
        app.vb = VertexBuffer::Builder().vertexCount(3).bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 8)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 24, 4)
                .normalized(VertexAttribute::COLOR).build(*engine);
        app.vb->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(vbo.data(), vbo.size(), 0));

        // Populate index buffer.
        app.ib = IndexBuffer::Builder().indexCount(3).bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, 0));

        // Construct material.
        app.mat = Material::Builder()
                .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE).build(*engine);

        // Construct renderable.
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .build(*engine, app.renderable = utils::EntityManager::get().create());
        scene->addEntity(app.renderable);

        // Replace the FilamentApp camera with identity.
        view->setCamera(app.cam = engine->createCamera());
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.renderable);
        engine->destroy(app.mat);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroy(app.cam);
    };

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
