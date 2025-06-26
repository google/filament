/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

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

#include "ImageExpectations.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;

struct App {
    Config config;
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable;
};

struct Vertex {
    filament::math::float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[3] = {
        {{1, 0}, 0xffff0000u},
        {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
        {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

const std::string material = R"(material {
    name : BakedColor,
    requires : [
        color
    ],
    shadingModel : unlit,
    culling : none,
    featureLevel : 0
}

fragment {
    void material(inout MaterialInputs material) {
        prepareMaterial(material);
        material.baseColor = getColor();
    }
}
)";

class CompareGLTFTest : public testing::Test {
public:
    test::ImageExpectations mExpectations;

    void postRender(Engine*, View* view, Scene*, Renderer* renderer);
};

void CompareGLTFTest::postRender(Engine*, View* view, Scene*, Renderer* renderer) {
    EXPECT_IMAGE(renderer, mExpectations, test::ScreenshotParams(512, 512, "GLTF"));

    FilamentApp::get().close();
}

TEST_F(CompareGLTFTest, Compare) {
    App app;

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
      app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
      scene->setSkybox(app.skybox);
      view->setPostProcessingEnabled(false);
      static_assert(sizeof(Vertex) == 12, "Strange vertex size.");
      /*app.vb = VertexBuffer::Builder()
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
              .package(material.c_str(), material.size())
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
      scene->addEntity(app.renderable);*/
      app.camera = utils::EntityManager::get().create();
      app.cam = engine->createCamera(app.camera);
      view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
      engine->destroy(app.skybox);
      //engine->destroy(app.renderable);
      //engine->destroy(app.mat);
      //engine->destroy(app.vb);
      //engine->destroy(app.ib);
      engine->destroyCameraComponent(app.camera);
      utils::EntityManager::get().destroy(app.camera);
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
              filament::math::mat4f::rotation(now, filament::math::float3{ 0, 0, 1 }));
    });

    app.config.backend = filament::Engine::Backend::METAL;

    FilamentApp::get().run(app.config, setup, cleanup, FilamentApp::ImGuiCallback(),
            FilamentApp::PreRenderCallback(),
            [this](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
                postRender(engine, view, scene, renderer);
            });
}