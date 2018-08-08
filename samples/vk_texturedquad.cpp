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
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/Path.h>

#include "../samples/app/Config.h"
#include "../samples/app/FilamentApp.h"

#include <stb_image.h>

#include <cmath>
#include <stdint.h>

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    MaterialInstance* matInstance;
    Camera* cam;
    Texture* tex;
    Entity renderable;
};

struct Vertex {
    math::float2 position;
    math::float2 uv;
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

static constexpr uint8_t BAKED_TEXTURE_PACKAGE[] = {
    #include "generated/material/bakedTexture.inc"
};

int main() {
    Config config;
    config.title = "texturedquad";
    config.backend = Engine::Backend::VULKAN;

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {

        // Load texture
        Path path = Path::getCurrentExecutable().getParent() +
                "textures/Moss_01/Moss_01_Color.png";
        if (!path.exists()) {
            std::cerr << "The texture " << path << " does not exist" << std::endl;
            exit(1);
        }
        int w, h, n;
        unsigned char* data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, 4);
        if (data == nullptr) {
            std::cerr << "The texture " << path << " could not be loaded" << std::endl;
            exit(1);
        }
        std::cout << "Loaded texture: " << w << "x" << h << std::endl;
        Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 4),
                Texture::Format::RGBA, Texture::Type::UBYTE,
                (driver::BufferDescriptor::Callback) &stbi_image_free);
        app.tex = Texture::Builder()
                .width(uint32_t(w))
                .height(uint32_t(h))
                .levels(1)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(driver::TextureFormat::RGBA8)
                .build(*engine);
                app.tex->setImage(*engine, 0, std::move(buffer));
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);

        // Set up view
        view->setClearColor({0.1, 0.125, 0.25, 1.0});
        view->setPostProcessingEnabled(false);
        app.cam = engine->createCamera();
        view->setCamera(app.cam);

        // Create quad renderable
        static_assert(sizeof(Vertex) == 16, "Strange vertex size.");
        app.vb = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 16)
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 16)
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(QUAD_VERTICES, 64, nullptr));
        app.ib = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(QUAD_INDICES, 12, nullptr));
        app.mat = Material::Builder()
                .package((void*) BAKED_TEXTURE_PACKAGE, sizeof(BAKED_TEXTURE_PACKAGE))
                .build(*engine);
        app.matInstance = app.mat->createInstance();
        app.matInstance->setParameter("albedo", app.tex, sampler);
        app.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.matInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 6)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app.renderable);
        scene->addEntity(app.renderable);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        Fence::waitAndDestroy(engine->createFence());
        engine->destroy(app.renderable);
        engine->destroy(app.matInstance);
        engine->destroy(app.mat);
        engine->destroy(app.tex);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroy(app.cam);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        const float zoom = 2.0 + 2.0 * sin(now);
        const uint32_t w = view->getViewport().width;
        const uint32_t h = view->getViewport().height;
        const float aspect = (float) w / h;
        app.cam->setProjection(Camera::Projection::ORTHO,
                -aspect * zoom, aspect * zoom,
                -zoom, zoom, -1, 1);
    });

    FilamentApp::get().run(config, setup, cleanup);
}
