/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <image/ImageSampler.h>
#include <image/LinearImage.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <cmath>

#include "generated/resources/resources.h"

using namespace filament;
using namespace filament::math;

using utils::Entity;
using utils::EntityManager;

using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;
using AttributeType = VertexBuffer::AttributeType;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    MaterialInstance* matInstance;
    Camera* cam;
    Skybox* skybox;
    Texture* tex;
    Entity renderable;
};

struct Vertex {
    ::float2 position;
    uint32_t color;
};

#define NUM_POINTS 100
#define TEXTURE_SIZE 128
#define MAX_POINT_SIZE 128.0f
#define MIN_POINT_SIZE 12.0f

void createSplatTexture(App& app, Engine* engine) {

    // To generate a Gaussian splat, create a single-channel 3x3 texture with a bright pixel in
    // its center, then magnify it using a Gaussian filter kernel.
    static image::LinearImage splat(3, 3, 1);
    splat.getPixelRef(1, 1)[0] = 0.25f;
    splat = image::resampleImage(splat, TEXTURE_SIZE, TEXTURE_SIZE, image::Filter::GAUSSIAN_SCALARS);

    Texture::PixelBufferDescriptor buffer(splat.getPixelRef(),
            size_t(TEXTURE_SIZE * TEXTURE_SIZE * sizeof(float)),
            Texture::Format::R, Texture::Type::FLOAT);

    app.tex = Texture::Builder()
            .width(TEXTURE_SIZE).height(TEXTURE_SIZE).levels(1)
            .sampler(Texture::Sampler::SAMPLER_2D).format(Texture::InternalFormat::R32F)
            .build(*engine);

    app.tex->setImage(*engine, 0, std::move(buffer));
}

void setup(App& app, Engine* engine, View* view, Scene* scene) {

    createSplatTexture(app, engine);

    static Vertex kVertices[NUM_POINTS];
    static float kPointSizes[NUM_POINTS];
    static uint16_t kIndices[NUM_POINTS];

    constexpr float dtheta = M_PI * 2 / NUM_POINTS;
    constexpr float dsize = MAX_POINT_SIZE / NUM_POINTS;
    constexpr float dcolor = 256.0f / NUM_POINTS;

    for (int i = 0; i < NUM_POINTS; i++) {
        const float theta = dtheta * i;
        const uint32_t c = dcolor * i;
        kVertices[i].position.x = cos(theta);
        kVertices[i].position.y = sin(theta);
        kVertices[i].color = 0xff000000u | c | (c << 8u) | (c << 16u);
        kPointSizes[i] = MIN_POINT_SIZE + dsize * i;
        kIndices[i] = i;
    }

    app.vb = VertexBuffer::Builder()
            .vertexCount(NUM_POINTS)
            .bufferCount(2)
            .attribute(VertexAttribute::POSITION, 0, AttributeType::FLOAT2, 0, sizeof(Vertex))
            .attribute(VertexAttribute::COLOR, 0, AttributeType::UBYTE4, sizeof(float2), sizeof(Vertex))
            .normalized(VertexAttribute::COLOR)
            .attribute(VertexAttribute::CUSTOM0, 1, AttributeType::FLOAT, 0, sizeof(float))
            .build(*engine);

    app.vb->setBufferAt(*engine, 0,
            VertexBuffer::BufferDescriptor(kVertices, NUM_POINTS * sizeof(Vertex), nullptr));

    app.vb->setBufferAt(*engine, 1,
            VertexBuffer::BufferDescriptor(kPointSizes, NUM_POINTS * sizeof(float), nullptr));

    app.ib = IndexBuffer::Builder()
            .indexCount(NUM_POINTS)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*engine);

    app.ib->setBuffer(*engine,
            IndexBuffer::BufferDescriptor(kIndices, NUM_POINTS * sizeof(uint16_t), nullptr));

    app.mat = Material::Builder()
            .package(RESOURCES_POINTSPRITES_DATA, RESOURCES_POINTSPRITES_SIZE)
            .build(*engine);

    app.renderable = EntityManager::get().create();

    app.matInstance = app.mat->createInstance();
    app.matInstance->setParameter("fade", app.tex, TextureSampler(MinFilter::LINEAR, MagFilter::LINEAR));

    RenderableManager::Builder(1)
            .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
            .material(0, app.matInstance)
            .geometry(0, RenderableManager::PrimitiveType::POINTS, app.vb, app.ib, 0, NUM_POINTS)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .build(*engine, app.renderable);

    scene->addEntity(app.renderable);
    app.cam = engine->createCamera();
    view->setCamera(app.cam);

    app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
    scene->setSkybox(app.skybox);
};

void cleanup(App& app, Engine* engine) {
    engine->destroy(app.skybox);
    engine->destroy(app.renderable);
    engine->destroy(app.matInstance);
    engine->destroy(app.mat);
    engine->destroy(app.vb);
    engine->destroy(app.ib);
    engine->destroy(app.cam);
}

void animate(App& app, Engine* engine, View* view, double now) {
    constexpr float ZOOM = 1.5f;
    const uint32_t w = view->getViewport().width;
    const uint32_t h = view->getViewport().height;
    const float aspect = (float) w / h;
    app.cam->setProjection(Camera::Projection::ORTHO,
            -aspect * ZOOM, aspect * ZOOM, -ZOOM, ZOOM, 0, 1);
    auto& tcm = engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(app.renderable), mat4f::rotation(now, float3{ 0, 0, 1 }));
}

int main(int argc, char** argv) {
    Config config;
    config.title = "point_sprites";

    App app;
    FilamentApp::get().animate([&app](Engine* e, View* v, double now) { animate(app, e, v, now); });
    FilamentApp::get().run(config,
            [&app](Engine* engine, View* view, Scene* scene) { setup(app, engine, view, scene); },
            [&app](Engine* engine, View*, Scene*) { cleanup(app, engine); });

    return 0;
}
