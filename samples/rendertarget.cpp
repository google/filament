/*
 * Copyright (C) 2021 The Android Open Source Project
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
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/RenderTarget.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <filameshio/MeshReader.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include "generated/resources/resources.h"
#include "generated/resources/monkey.h"

using namespace filament;
using namespace filamesh;
using namespace filament::math;

static mat4f reflectionMatrix(float4 plane) {
    mat4f m;
    m[0][0] = -2 * plane.x * plane.x + 1;
    m[0][1] = -2 * plane.x * plane.y;
    m[0][2] = -2 * plane.x * plane.z;
    m[0][3] = -2 * plane.x * plane.w;
    m[1][0] = -2 * plane.x * plane.y;
    m[1][1] = -2 * plane.y * plane.y + 1;
    m[1][2] = -2 * plane.y * plane.z;
    m[1][3] = -2 * plane.y * plane.w;
    m[2][0] = -2 * plane.z * plane.x;
    m[2][1] = -2 * plane.z * plane.y;
    m[2][2] = -2 * plane.z * plane.z + 1;
    m[2][3] = -2 * plane.z * plane.w;
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
    return transpose(m);
}

struct Vertex {
    float3 position;
    float2 uv;
};

struct App {
    utils::Entity lightEntity;
    Material* meshMaterial;
    MaterialInstance* meshMatInstance;
    MeshReader::Mesh monkeyMesh;
    utils::Entity reflectedMonkey;
    mat4f transform;

    Texture* offscreenTexture = nullptr;
    RenderTarget* offscreenRenderTarget = nullptr;
    View* offscreenView = nullptr;
    Scene* offscreenScene = nullptr;

    utils::Entity quadEntity;
    VertexBuffer* quadVb = nullptr;
    IndexBuffer* quadIb = nullptr;
    Material* quadMaterial = nullptr;
    MaterialInstance* quadMatInstance = nullptr;
    ColorGrading* colorGrading = nullptr;

    float3 quadCenter;
    float3 quadNormal;
};

int main(int argc, char** argv) {
    Config config;
    config.title = "rendertarget";

    App app;
    auto setup = [config, &app](Engine* engine, View* view, Scene* scene) {

        // Filmic tone mapping is invertible in GLSL using inverseTonemapSRGB().
        app.colorGrading = ColorGrading::Builder().toneMapping(ColorGrading::ToneMapping::FILMIC).build(*engine);
        view->setColorGrading(app.colorGrading);

        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();
        auto vp = view->getViewport();

        // Instantiate offscreen render target.
        app.offscreenView = engine->createView();
        app.offscreenScene = engine->createScene();
        app.offscreenView->setScene(app.offscreenScene);
        app.offscreenView->setColorGrading(app.colorGrading);
        app.offscreenTexture = Texture::Builder()
            .width(vp.width).height(vp.height).levels(1)
            .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
            .format(Texture::InternalFormat::RGBA8).build(*engine);
        app.offscreenRenderTarget = RenderTarget::Builder()
            .texture(RenderTarget::COLOR, app.offscreenTexture)
            .build(*engine);
        app.offscreenView->setRenderTarget(app.offscreenRenderTarget);
        app.offscreenView->setViewport({0, 0, vp.width, vp.height});
        app.offscreenView->setCamera(&view->getCamera());
        FilamentApp::get().addOffscreenView(app.offscreenView);

        // Position and orient the mirror in an interesting way.
        float3 c = app.quadCenter = {-2, 0, -5};
        float3 n = app.quadNormal = normalize(float3 {1, 0, 2});
        float3 u = normalize(cross(app.quadNormal, float3(0, 1, 0)));
        float3 v = cross(n, u);
        u = 1.5 * u;
        v = 1.5 * v;
        static Vertex kQuadVertices[4] = { {{}, {1, 0}}, {{}, {0, 0}}, {{}, {1, 1}}, {{}, {0, 1}} };
        kQuadVertices[0].position = c - u - v;
        kQuadVertices[1].position = c + u - v;
        kQuadVertices[2].position = c - u + v;
        kQuadVertices[3].position = c + u + v;

        // Create quad vertex buffer.
        static_assert(sizeof(Vertex) == 20, "Strange vertex size.");
        app.quadVb = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, 20)
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 12, 20)
                .build(*engine);
        app.quadVb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(kQuadVertices, 80, nullptr));

        // Create quad index buffer.
        static constexpr uint16_t kQuadIndices[6] = { 0, 1, 2, 3, 2, 1 };
        app.quadIb = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.quadIb->setBuffer(*engine, IndexBuffer::BufferDescriptor(kQuadIndices, 12, nullptr));

        // Create quad material and renderable.
        app.quadMaterial = Material::Builder()
                .package(RESOURCES_MIRROR_DATA, RESOURCES_MIRROR_SIZE)
                .build(*engine);
        app.quadMatInstance = app.quadMaterial->createInstance();
        TextureSampler sampler(TextureSampler::MinFilter::LINEAR, TextureSampler::MagFilter::LINEAR);
        app.quadMatInstance->setParameter("albedo", app.offscreenTexture, sampler);
        app.quadEntity = em.create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.quadMatInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.quadVb, app.quadIb, 0, 6)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app.quadEntity);
        scene->addEntity(app.quadEntity);

        // Instantiate mesh material.
        app.meshMaterial = Material::Builder()
            .package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE).build(*engine);
        auto mi = app.meshMatInstance = app.meshMaterial->createInstance();
        mi->setParameter("baseColor", RgbType::LINEAR, {0.8, 1.0, 1.0});
        mi->setParameter("metallic", 0.0f);
        mi->setParameter("roughness", 0.4f);
        mi->setParameter("reflectance", 0.5f);

        // Add monkey into the scene.
        app.monkeyMesh = MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, nullptr, nullptr, mi);
        auto ti = tcm.getInstance(app.monkeyMesh.renderable);
        app.transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        rcm.setCastShadows(rcm.getInstance(app.monkeyMesh.renderable), false);
        scene->addEntity(app.monkeyMesh.renderable);

        // Add reflected monkey into the offscreen scene.
        app.reflectedMonkey = em.create();
        RenderableManager::Builder(1)
                .boundingBox({{ -2, -2, -2 }, { 2, 2, 2 }})
                .material(0, mi)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.monkeyMesh.vertexBuffer, app.monkeyMesh.indexBuffer)
                .receiveShadows(true)
                .castShadows(false)
                .build(*engine, app.reflectedMonkey);
        app.offscreenScene->addEntity(app.reflectedMonkey);

        // Add light source to both scenes.
        // NOTE: the reflected scene should use a reflected light dir.
        app.lightEntity = em.create();
        LightManager::Builder(LightManager::Type::SUN)
                .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                .intensity(110000)
                .direction({ 0.7, -1, -0.8 })
                .sunAngularRadius(1.9f)
                .castShadows(false)
                .build(*engine, app.lightEntity);
        scene->addEntity(app.lightEntity);
        app.offscreenScene->addEntity(app.lightEntity);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.colorGrading);
        engine->destroy(app.reflectedMonkey);
        engine->destroy(app.lightEntity);
        engine->destroy(app.quadEntity);
        engine->destroy(app.meshMatInstance);
        engine->destroy(app.meshMaterial);
        engine->destroy(app.monkeyMesh.renderable);
        engine->destroy(app.monkeyMesh.vertexBuffer);
        engine->destroy(app.monkeyMesh.indexBuffer);
        engine->destroy(app.offscreenTexture);
        engine->destroy(app.offscreenRenderTarget);
        engine->destroy(app.offscreenScene);
        engine->destroy(app.offscreenView);
        engine->destroy(app.quadVb);
        engine->destroy(app.quadIb);
        engine->destroy(app.quadMatInstance);
        engine->destroy(app.quadMaterial);
    };

    auto preRender = [&app](Engine*, View*, Scene*, Renderer* renderer) {
        renderer->setClearOptions({.clearColor = {0.1,0.2,0.4,1.0}, .clear = true});
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        auto& tcm = engine->getTransformManager();

        // Rotate the monkey and slide her along Z.
        auto ti = tcm.getInstance(app.monkeyMesh.renderable);
        mat4f xlate = mat4f::translation(float3(0, 0, 0.5 + sin(now)));
        mat4f xform =  app.transform * xlate * mat4f::rotation(now, float3{ 0, 1, 0 });
        tcm.setTransform(ti, xform);

        // Transform the monkey to obtain the reflected monkey.
        ti = tcm.getInstance(app.reflectedMonkey);
        const float3 planeNormal = app.quadNormal;
        const float4 planeEqn(planeNormal, -dot(planeNormal, app.quadCenter));
        tcm.setTransform(ti, reflectionMatrix(planeEqn) * xform);
    });

    FilamentApp::get().run(config, setup, cleanup, FilamentApp::ImGuiCallback(), preRender);

    return 0;
}
