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

#include <getopt/getopt.h>

#include <iostream>

#include "generated/resources/resources.h"
#include "generated/resources/monkey.h"

using namespace filament;
using namespace filamesh;
using namespace filament::math;

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

    Texture* offscreenColorTexture = nullptr;
    Texture* offscreenDepthTexture = nullptr;
    RenderTarget* offscreenRenderTarget = nullptr;
    View* offscreenView = nullptr;
    Scene* offscreenScene = nullptr;
    Camera* offscreenCamera = nullptr;

    enum class ReflectionMode {
        RENDERABLES,
        CAMERA,
    };

    ReflectionMode mode = ReflectionMode::CAMERA;
    Config config;

    utils::Entity quadEntity;
    VertexBuffer* quadVb = nullptr;
    IndexBuffer* quadIb = nullptr;
    Material* quadMaterial = nullptr;
    MaterialInstance* quadMatInstance = nullptr;

    float3 quadCenter;
    float3 quadNormal;
};

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

static void setReflectionMode(App& app, App::ReflectionMode mode) {
    switch (mode) {
    case App::ReflectionMode::RENDERABLES:
        app.offscreenScene->addEntity(app.reflectedMonkey);
        app.offscreenScene->remove(app.monkeyMesh.renderable);
        app.offscreenView->setFrontFaceWindingInverted(false);
        break;
    case App::ReflectionMode::CAMERA:
        app.offscreenScene->addEntity(app.monkeyMesh.renderable);
        app.offscreenScene->remove(app.reflectedMonkey);
        app.offscreenView->setFrontFaceWindingInverted(true);
        break;
    }
    app.mode = mode;
}

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
        "SHOWCASE renders suzanne with planar reflection\n"
        "Usage:\n"
        "    SHOWCASE [options]\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: opengl (default), vulkan, or metal\n"
        "   --mode, -m\n"
        "       Specify the reflection mode: camera (default), or renderables\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:m:";
    static const struct option OPTIONS[] = {
        { "help", no_argument,       nullptr, 'h' },
        { "api",  required_argument, nullptr, 'a' },
        { "mode", required_argument, nullptr, 'm' },
        { nullptr, 0, nullptr, 0 }
    };
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                if (arg == "opengl") {
                    app->config.backend = Engine::Backend::OPENGL;
                } else if (arg == "vulkan") {
                    app->config.backend = Engine::Backend::VULKAN;
                } else if (arg == "metal") {
                    app->config.backend = Engine::Backend::METAL;
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'.\n";
                    exit(1);
                }
                break;
            case 'm':
                if (arg == "camera") {
                    app->mode = App::ReflectionMode::CAMERA;
                } else if (arg == "renderables") {
                    app->mode = App::ReflectionMode::RENDERABLES;
                } else {
                    std::cerr << "Unrecognized mode. Must be 'camera'|'renderables'.\n";
                    exit(1);
                }
                break;
        }
    }
    return optind;
}

int main(int argc, char** argv) {
    App app{};
    app.config.title = "rendertarget";
    handleCommandLineArguments(argc, argv, &app);

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();
        auto vp = view->getViewport();

        // Instantiate offscreen render target.
        app.offscreenView = engine->createView();
        app.offscreenScene = engine->createScene();
        app.offscreenView->setScene(app.offscreenScene);
        app.offscreenView->setPostProcessingEnabled(false);
        app.offscreenColorTexture = Texture::Builder()
            .width(vp.width).height(vp.height).levels(1)
            .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
            .format(Texture::InternalFormat::RGBA8).build(*engine);
        app.offscreenDepthTexture = Texture::Builder()
            .width(vp.width).height(vp.height).levels(1)
            .usage(Texture::Usage::DEPTH_ATTACHMENT)
            .format(Texture::InternalFormat::DEPTH32F).build(*engine);
        app.offscreenRenderTarget = RenderTarget::Builder()
            .texture(RenderTarget::AttachmentPoint::COLOR, app.offscreenColorTexture)
            .texture(RenderTarget::AttachmentPoint::DEPTH, app.offscreenDepthTexture)
            .build(*engine);
        app.offscreenView->setRenderTarget(app.offscreenRenderTarget);
        app.offscreenView->setViewport({0, 0, vp.width, vp.height});
        app.offscreenCamera = engine->createCamera(em.create());
        app.offscreenView->setCamera(app.offscreenCamera);
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
        app.quadMatInstance->setParameter("albedo", app.offscreenColorTexture, sampler);
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

        // Create a reflected monkey, which is used only for App::ReflectionMode::RENDERABLES.
        app.reflectedMonkey = em.create();
        RenderableManager::Builder(1)
                .boundingBox({{ -2, -2, -2 }, { 2, 2, 2 }})
                .material(0, mi)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.monkeyMesh.vertexBuffer, app.monkeyMesh.indexBuffer)
                .receiveShadows(true)
                .castShadows(false)
                .build(*engine, app.reflectedMonkey);
        setReflectionMode(app, app.mode);

        // Add light source to both scenes.
        // NOTE: this is slightly wrong when the reflection mode is RENDERABLES.
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

        auto& em = utils::EntityManager::get();
        auto camera = app.offscreenCamera->getEntity();
        engine->destroyCameraComponent(camera);
        em.destroy(camera);

        engine->destroy(app.reflectedMonkey);
        engine->destroy(app.lightEntity);
        engine->destroy(app.quadEntity);
        engine->destroy(app.monkeyMesh.renderable);
        engine->destroy(app.monkeyMesh.vertexBuffer);
        engine->destroy(app.monkeyMesh.indexBuffer);
        engine->destroy(app.meshMatInstance);
        engine->destroy(app.meshMaterial);
        engine->destroy(app.offscreenColorTexture);
        engine->destroy(app.offscreenDepthTexture);
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

        // Animate the monkey by spinning and sliding back and forth along Z.
        auto ti = tcm.getInstance(app.monkeyMesh.renderable);
        mat4f xlate = mat4f::translation(float3(0, 0, 0.5 + sin(now)));
        mat4f xform =  app.transform * xlate * mat4f::rotation(now, float3{ 0, 1, 0 });
        tcm.setTransform(ti, xform);

        // Generate a reflection matrix from the plane equation Ax + By + Cz + D = 0.
        const float3 planeNormal = app.quadNormal;
        const float4 planeEquation(planeNormal, -dot(planeNormal, app.quadCenter));
        const mat4f reflection = reflectionMatrix(planeEquation);

        // Apply the reflection matrix to either the renderable or the camera, depending on mode.
        Camera const& camera = view->getCamera();
        const auto model = camera.getModelMatrix();
        const auto renderingProjection = camera.getProjectionMatrix();
        const auto cullingProjection = camera.getCullingProjectionMatrix();
        app.offscreenCamera->setCustomProjection(renderingProjection, cullingProjection,
                camera.getNear(), camera.getCullingFar());
        switch (app.mode) {
            case App::ReflectionMode::RENDERABLES:
                tcm.setTransform(tcm.getInstance(app.reflectedMonkey), reflection * xform);
                app.offscreenCamera->setModelMatrix(model);
                break;
            case App::ReflectionMode::CAMERA:
                app.offscreenCamera->setModelMatrix(reflection * model);
                break;
        }
    });

    FilamentApp::get().run(app.config, setup, cleanup, FilamentApp::ImGuiCallback(), preRender);

    return 0;
}
