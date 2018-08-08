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
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <math/norm.h>

#include "../samples/app/Config.h"
#include "../samples/app/FilamentApp.h"
#include "../samples/app/MeshAssimp.h"

using namespace filament;
using namespace math;
using Backend = Engine::Backend;
using TargetApi = MeshAssimp::TargetApi;

struct GroundPlane {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    utils::Entity renderable;
};

struct App {
    utils::Entity light;
    std::map<std::string, MaterialInstance*> materials;
    MeshAssimp* meshes;
    mat4f transform;
    GroundPlane plane;
};

static const char* MODEL_FILE = "samples/assets/models/monkey/monkey.obj";
static const char* IBL_FOLDER = "../samples/envs/office";

static constexpr bool ENABLE_SHADOWS = true;
static constexpr uint8_t GROUND_SHADOW_PACKAGE[] = {
    #include "generated/material/groundShadow.inc"
};

static GroundPlane createGroundPlane(Engine* engine);

static const Config config {
    .title = "shadowtest",
    .iblDirectory = IBL_FOLDER,
    .scale = 1,
    .splitView = false,
    .backend = Backend::VULKAN,
};

int main(int argc, char** argv) {
    App app;

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();

        // Add geometry into the scene.
        TargetApi api = config.backend == Backend::VULKAN ? TargetApi::VULKAN : TargetApi::OPENGL;
        app.meshes = new MeshAssimp(*engine, api, MeshAssimp::Platform::DESKTOP);
        app.meshes->addFromFile(MODEL_FILE, app.materials);
        auto ti = tcm.getInstance(app.meshes->getRenderables()[0]);
        app.transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        for (auto renderable : app.meshes->getRenderables()) {
            auto instance = rcm.getInstance(renderable);
            if (rcm.hasComponent(renderable)) {
                rcm.setCastShadows(instance, ENABLE_SHADOWS);
                rcm.setReceiveShadows(instance, false);
                scene->addEntity(renderable);
            }
        }

        // Add light sources into the scene.
        app.light = em.create();
        LightManager::Builder(LightManager::Type::SUN)
            .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
            .intensity(110000)
            .direction({ 0.7, -1, -0.8 })
            .sunAngularRadius(1.9f)
            .castShadows(ENABLE_SHADOWS)
            .build(*engine, app.light);
        scene->addEntity(app.light);

        // Hide skybox and add ground plane.
        scene->setSkybox(nullptr);
        view->setClearColor({0.5f,0.75f,1.0f,1.0f});
        app.plane = createGroundPlane(engine);
        scene->addEntity(app.plane.renderable);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        Fence::waitAndDestroy(engine->createFence());
        engine->destroy(app.plane.renderable);
        engine->destroy(app.plane.mat);
        engine->destroy(app.plane.vb);
        engine->destroy(app.plane.ib);
        engine->destroy(app.light);
        for (auto& item : app.materials) {
            engine->destroy(item.second);
        }
        delete app.meshes;
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        auto& tcm = engine->getTransformManager();
        auto ti = tcm.getInstance(app.meshes->getRenderables()[0]);
        tcm.setTransform(ti, app.transform * mat4f::rotate(now, float3{0, 1, 0}));
    });

    FilamentApp::get().run(config, setup, cleanup);
}

static GroundPlane createGroundPlane(Engine* engine) {
    Material* shadowMaterial = Material::Builder()
        .package((void*) GROUND_SHADOW_PACKAGE, sizeof(GROUND_SHADOW_PACKAGE))
        .build(*engine);

    const static uint32_t indices[] {
        0, 1, 2, 2, 3, 0
    };
    const static float3 vertices[] {
        { -10, 0, -10 },
        { -10, 0,  10 },
        {  10, 0,  10 },
        {  10, 0, -10 },
    };
    short4 tbn = packSnorm16(normalize(positive(mat3f{
        float3{1.0f, 0.0f, 0.0f}, float3{0.0f, 0.0f, 1.0f}, float3{0.0f, 1.0f, 0.0f}
    }.toQuaternion())).xyzw);
    const static short4 normals[] { tbn, tbn, tbn, tbn };
    VertexBuffer* vertexBuffer = VertexBuffer::Builder()
        .vertexCount(4)
        .bufferCount(2)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
        .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
        .normalized(VertexAttribute::TANGENTS)
        .build(*engine);
    vertexBuffer->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(
            vertices, vertexBuffer->getVertexCount() * sizeof(vertices[0])));
    vertexBuffer->setBufferAt(*engine, 1, VertexBuffer::BufferDescriptor(
            normals, vertexBuffer->getVertexCount() * sizeof(normals[0])));
    IndexBuffer* indexBuffer = IndexBuffer::Builder().indexCount(6).build(*engine);
    indexBuffer->setBuffer(*engine, IndexBuffer::BufferDescriptor(
            indices, indexBuffer->getIndexCount() * sizeof(uint32_t)));

    auto& em = utils::EntityManager::get();
    utils::Entity renderable = em.create();
    RenderableManager::Builder(1)
        .boundingBox({{ 0, 0, 0 }, { 10, 1e-4f, 10 }})
        .material(0, shadowMaterial->getDefaultInstance())
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0, 6)
        .culling(false)
        .receiveShadows(ENABLE_SHADOWS)
        .castShadows(false)
        .build(*engine, renderable);

    auto& tcm = engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(renderable), mat4f::translate(float4{0, -1, -4, 1}));
    return {
        .vb = vertexBuffer,
        .ib = indexBuffer,
        .mat = shadowMaterial,
        .renderable = renderable,
    };
}
