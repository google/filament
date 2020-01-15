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

#include "app/Config.h"
#include "app/FilamentApp.h"
#include "app/IBL.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/RenderTarget.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>

#include <getopt/getopt.h>

#include <imgui.h>

#include <string>

#include "lucy_utils.h"

using namespace filament;
using namespace filament::math;
using namespace gltfio;
using namespace utils;

static constexpr uint32_t FBO_WIDTH = 1024;
static constexpr uint32_t FBO_HEIGHT = 1024;
static const uint32_t WINDOW_WIDTH = 800;
static const uint32_t WINDOW_HEIGHT = 800;

struct Framebuffer {
    Camera* camera = nullptr;
    View* view = nullptr;
    Scene* scene = nullptr;
    Texture* color = nullptr;
    Texture* depth = nullptr;
    RenderTarget* target = nullptr;
};

struct LucyApp {
    Config config;
    bool showQuads;
    bool showImgui;
    float iblIntensity = 15000;
    float iblRotation = M_PI * 1.5f;
    AssetLoader* loader;
    FilamentAsset* asset;
    MaterialProvider* materials;
    Entity rotationRoot;
    Entity hudQuads[4];
    Entity finalQuad;
    Entity hblurQuad;
    Entity vblurQuad;
    Framebuffer reflection;
    Framebuffer primary;
    Framebuffer hblur;
    Framebuffer vblur;
    Camera* finalCamera;
};

static void setup(LucyApp& app, Engine* engine, View* finalView, Scene* finalScene) {

    app.reflection.color = Texture::Builder()
        .width(FBO_WIDTH).height(FBO_HEIGHT).levels(1)
        .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
        .format(Texture::InternalFormat::RGBA8).build(*engine);
    app.reflection.depth = Texture::Builder()
        .width(FBO_WIDTH).height(FBO_HEIGHT).levels(1)
        .usage(Texture::Usage::DEPTH_ATTACHMENT)
        .format(Texture::InternalFormat::DEPTH24).build(*engine);
    app.reflection.target = RenderTarget::Builder()
        .texture(RenderTarget::COLOR, app.reflection.color)
        .texture(RenderTarget::DEPTH, app.reflection.depth)
        .build(*engine);

    app.primary.color = Texture::Builder()
        .width(FBO_WIDTH).height(FBO_HEIGHT).levels(1)
        .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
        .format(Texture::InternalFormat::RGBA16F).build(*engine);
    app.primary.depth = Texture::Builder()
        .width(FBO_WIDTH).height(FBO_HEIGHT).levels(1)
        .usage(Texture::Usage::DEPTH_ATTACHMENT)
        .format(Texture::InternalFormat::DEPTH24).build(*engine);
    app.primary.target = RenderTarget::Builder()
        .texture(RenderTarget::COLOR, app.primary.color)
        .texture(RenderTarget::DEPTH, app.primary.depth)
        .build(*engine);

    app.hblur.color = Texture::Builder()
        .width(FBO_WIDTH).height(FBO_HEIGHT).levels(1)
        .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
        .format(Texture::InternalFormat::RGBA16F).build(*engine);
    app.hblur.target = RenderTarget::Builder()
        .texture(RenderTarget::COLOR, app.hblur.color)
        .build(*engine);

    app.vblur.color = Texture::Builder()
        .width(FBO_WIDTH).height(FBO_HEIGHT).levels(1)
        .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
        .format(Texture::InternalFormat::RGBA16F).build(*engine);
    app.vblur.target = RenderTarget::Builder()
        .texture(RenderTarget::COLOR, app.vblur.color)
        .build(*engine);

    // Create lights.
    auto* ibl = FilamentApp::get().getIBL();
    ibl->getIndirectLight()->setIntensity(app.iblIntensity);
    ibl->getIndirectLight()->setRotation(mat3f::rotation(app.iblRotation, float3{ 0, 1, 0 }));
    ibl->getSkybox()->setLayerMask(0xff, 0xff);

    auto sunlight = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
        .color(Color::toLinear<ACCURATE>({0.98, 0.92, 0.89}))
        .intensity(50000)
        .direction(normalize(float3 {0.6, -1.0, -0.8}))
        .castShadows(true)
        .sunAngularRadius(1.9)
        .sunHaloSize(10.0)
        .sunHaloFalloff(80.0)
        .build(*engine, sunlight);

    // Create filament entities for the embedded glTF model.
    app.materials = createMaterialGenerator(engine);
    app.loader = AssetLoader::create({engine, app.materials });
    app.asset = app.loader->createAssetFromBinary(RESOURCE_ARGS(LUCY));
    gltfio::ResourceLoader({ .engine = engine }).loadResources(app.asset);
    app.asset->releaseSourceData();

    // Tweak the model materials.
    auto begin = app.asset->getMaterialInstances();
    auto end = begin + app.asset->getMaterialInstanceCount();
    for (auto iter = begin; iter != end; ++iter) {
        (*iter)->setParameter("roughnessFactor", 0.25f);
        (*iter)->setParameter("baseColorFactor", float4 {1.0, 0.6, 0.5, 1.0});
    }

    // Transform the model to fit into the unit cube, then parent it under a rotation node.
    auto& tcm = engine->getTransformManager();
    auto root = tcm.getInstance(app.asset->getRoot());
    mat4f transform = LucyUtils::fitIntoUnitCube(app.asset->getBoundingBox());
    tcm.setTransform(root, transform);
    app.rotationRoot = EntityManager::get().create();
    tcm.create(app.rotationRoot);
    tcm.setParent(root, tcm.getInstance(app.rotationRoot));

    // Create the podium.
    Entity disk = LucyUtils::createDisk(engine, app.reflection.color);
    mat4f diskTransform = mat4f::scaling(float3 {1.5f, 1.5, 1.0f});
    diskTransform = mat4f::rotation(-M_PI / 2, float3 {1, 0, 0}) * diskTransform;
    diskTransform = mat4f::translation(float3 {0, -1, 0}) * diskTransform;
    tcm.create(disk, tcm.getInstance(app.rotationRoot), diskTransform);

    // Create full-screen quads.
    app.finalQuad = LucyUtils::createQuad(engine, app.primary.color, LucyUtils::MIX,
            app.vblur.color);
    app.hblurQuad = LucyUtils::createQuad(engine, app.primary.color, LucyUtils::HBLUR);
    app.vblurQuad = LucyUtils::createQuad(engine, app.hblur.color, LucyUtils::VBLUR);

    mat4f quadScale = mat4f::scaling(float3 {FBO_WIDTH, FBO_HEIGHT, 1});
    tcm.setTransform(tcm.getInstance(app.hblurQuad), quadScale);
    tcm.setTransform(tcm.getInstance(app.vblurQuad), quadScale);

    app.reflection.scene = engine->createScene();
    app.reflection.scene->setIndirectLight(ibl->getIndirectLight());
    app.reflection.scene->addEntities(app.asset->getEntities(), app.asset->getEntityCount());
    app.reflection.scene->addEntity(sunlight);

    app.primary.scene = engine->createScene();
    app.primary.scene->addEntity(disk);
    app.primary.scene->addEntities(app.asset->getEntities(), app.asset->getEntityCount());
    app.primary.scene->addEntity(sunlight);
    app.primary.scene->setSkybox(ibl->getSkybox());
    app.primary.scene->setIndirectLight(ibl->getIndirectLight());

    app.hblur.scene = engine->createScene();
    app.hblur.scene->addEntity(app.hblurQuad);
    app.vblur.scene = engine->createScene();
    app.vblur.scene->addEntity(app.vblurQuad);

    app.reflection.camera = engine->createCamera();
    app.reflection.view = engine->createView();
    app.reflection.view->setName("reflection");
    app.reflection.view->setRenderTarget(app.reflection.target);
    app.reflection.view->setClearColor({0, 0, 0, 1});
    app.reflection.view->setClearTargets(true, true, false);
    app.reflection.view->setViewport(Viewport(0, 0, FBO_WIDTH, FBO_HEIGHT));
    app.reflection.view->setScene(app.reflection.scene);
    app.reflection.view->setCamera(app.reflection.camera);
    app.reflection.view->setToneMapping(View::ToneMapping::LINEAR);
    app.reflection.view->setDithering(View::Dithering::NONE);
    FilamentApp::get().addOffscreenView(app.reflection.view);

    app.primary.camera = engine->createCamera();
    app.primary.view = engine->createView();
    app.primary.view->setName("primary");
    app.primary.view->setRenderTarget(app.primary.target);
    app.primary.view->setViewport(Viewport(0, 0, FBO_WIDTH, FBO_HEIGHT));
    app.primary.view->setScene(app.primary.scene);
    app.primary.view->setCamera(app.primary.camera);
    app.primary.view->setToneMapping(View::ToneMapping::LINEAR);
    app.primary.view->setDithering(View::Dithering::NONE);
    FilamentApp::get().addOffscreenView(app.primary.view);

    app.hblur.camera = engine->createCamera();
    app.hblur.view = engine->createView();
    app.hblur.view->setName("hblur");
    app.hblur.view->setRenderTarget(app.hblur.target);
    app.hblur.view->setViewport(Viewport(0, 0, FBO_WIDTH, FBO_HEIGHT));
    app.hblur.view->setScene(app.hblur.scene);
    app.hblur.view->setCamera(app.hblur.camera);
    app.hblur.view->setPostProcessingEnabled(false);
    FilamentApp::get().addOffscreenView(app.hblur.view);
    app.hblur.camera->setProjection(Camera::Projection::ORTHO,
            0.0, FBO_WIDTH, FBO_HEIGHT, 0.0, 0.0, 1.0);

    app.vblur.camera = engine->createCamera();
    app.vblur.view = engine->createView();
    app.vblur.view->setName("vblur");
    app.vblur.view->setRenderTarget(app.vblur.target);
    app.vblur.view->setViewport(Viewport(0, 0, FBO_WIDTH, FBO_HEIGHT));
    app.vblur.view->setScene(app.vblur.scene);
    app.vblur.view->setCamera(app.vblur.camera);
    app.vblur.view->setPostProcessingEnabled(false);
    FilamentApp::get().addOffscreenView(app.vblur.view);
    app.vblur.camera->setProjection(Camera::Projection::ORTHO,
            0.0, FBO_WIDTH, FBO_HEIGHT, 0.0, 0.0, 1.0);

    app.finalCamera = engine->createCamera();
    finalScene->addEntity(app.finalQuad);
    finalScene->setSkybox(nullptr);
    finalScene->setIndirectLight(nullptr);
    finalView->setSampleCount(1);
    finalView->setCamera(app.finalCamera);
};

static float3 reflect(float3 i, float3 n) { return i - 2.0f * dot(n, i) * n; };

static void animate(LucyApp& app, Engine* engine, View* finalView, double now) {
    // Rotate the embedded mesh. (use now = -0.5 for a nice screenshot)
    const mat4f xform = mat4f::rotation(now, float3 {0, 1, 0});
    auto& tcm = engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(app.rotationRoot), xform);

    // Adjust the projection in case the window has been resized.
    const auto viewport = finalView->getViewport();
    const float aspect = float(viewport.width) / viewport.height;
    app.primary.camera->setProjection(30, aspect, 1.0, 10.0);
    app.reflection.camera->setProjection(30, aspect, 1.0, 10.0);
    app.finalCamera->setProjection(Camera::Projection::ORTHO,
            0.0, viewport.width, viewport.height, 0.0, 0.0, 1.0);

    // Adjust the size of the final quad in case the window has been resized.
    mat4f finalQuadSize = mat4f::scaling(float3 {viewport.width, viewport.height, 1});
    tcm.setTransform(tcm.getInstance(app.finalQuad), finalQuadSize);

    // Primary camera. The target point will be moved along the gazing direction to aid reflection.
    const float3 eye = {0.0f, 1.5f, 4.0f};
    float3 target = {0, 0, 0};

    // Find where the central camera ray intersects the podium.
    const float3 gaze = normalize(target - eye);
    const float planeD = -1;
    const float3 planeN = {0, 1, 0};
    const float t = dot(planeN * planeD - eye, planeN) / dot(gaze, planeN);
    target = eye + t * gaze;

    // Set up the primary camera.
    const float3 up = cross(gaze, float3 {-1, 0, 0});
    app.primary.camera->lookAt(eye, target, up);

    // Find the location of the reflected camera.
    const float eyedist = length(target - eye);
    const float3 newgaze = reflect(gaze, planeN);
    const float3 neweye = target - eyedist * newgaze;
    const float3 newup = cross(newgaze, float3 {-1, 0, 0});
    app.reflection.camera->lookAt(neweye, neweye + newgaze, newup);
};

static void drawHud(LucyApp& app, Engine* engine, View* hudView) {
    Scene* hudScene = hudView->getScene();
    if (!app.hudQuads[0] && app.showQuads) {
        auto& tcm = engine->getTransformManager();
        app.hudQuads[0] = LucyUtils::createQuad(engine, app.reflection.color, LucyUtils::BLIT);
        app.hudQuads[1] = LucyUtils::createQuad(engine, app.primary.color, LucyUtils::BLIT);
        app.hudQuads[2] = LucyUtils::createQuad(engine, app.hblur.color, LucyUtils::BLIT);
        app.hudQuads[3] = LucyUtils::createQuad(engine, app.vblur.color, LucyUtils::BLIT);
        hudScene->addEntity(app.hudQuads[0]);
        hudScene->addEntity(app.hudQuads[1]);
        hudScene->addEntity(app.hudQuads[2]);
        hudScene->addEntity(app.hudQuads[3]);
        mat4f scale = mat4f::scaling(float3 {100, 100, 1});
        tcm.create(app.hudQuads[0], {}, mat4f::translation(float3 {10, 10, 0}) * scale);
        tcm.create(app.hudQuads[1], {}, mat4f::translation(float3 {10, 120, 0}) * scale);
        tcm.create(app.hudQuads[2], {}, mat4f::translation(float3 {10, 230, 0}) * scale);
        tcm.create(app.hudQuads[3], {}, mat4f::translation(float3 {10, 340, 0}) * scale);
    }
    if (app.showImgui) {
        ImGui::SetNextWindowSize(ImVec2(305, 90));
        ImGui::Begin("Parameters");
        ImGui::SliderFloat("ibl", &app.iblIntensity, 0.0f, 50000.0f);
        ImGui::SliderAngle("ibl rotation", &app.iblRotation);
        ImGui::End();
        auto* ibl = FilamentApp::get().getIBL();
        ibl->getIndirectLight()->setIntensity(app.iblIntensity);
        ibl->getIndirectLight()->setRotation(mat3f::rotation(app.iblRotation, float3{ 0, 1, 0 }));
    }
}

static void cleanup(LucyApp& app, Engine* engine) {
    engine->destroy(app.hudQuads[0]);
    engine->destroy(app.hudQuads[1]);
    engine->destroy(app.hudQuads[2]);
    engine->destroy(app.hudQuads[3]);

    engine->destroy(app.hblurQuad);
    engine->destroy(app.finalQuad);

    engine->destroy(app.hblur.camera);
    engine->destroy(app.hblur.scene);
    engine->destroy(app.hblur.view);
    engine->destroy(app.hblur.color);
    engine->destroy(app.hblur.target);

    engine->destroy(app.vblur.camera);
    engine->destroy(app.vblur.scene);
    engine->destroy(app.vblur.view);
    engine->destroy(app.vblur.color);
    engine->destroy(app.vblur.target);

    engine->destroy(app.reflection.camera);
    engine->destroy(app.reflection.scene);
    engine->destroy(app.reflection.view);
    engine->destroy(app.reflection.color);
    engine->destroy(app.reflection.depth);
    engine->destroy(app.reflection.target);

    engine->destroy(app.primary.camera);
    engine->destroy(app.primary.scene);
    engine->destroy(app.primary.view);
    engine->destroy(app.primary.color);
    engine->destroy(app.primary.depth);
    engine->destroy(app.primary.target);

    engine->destroy(app.finalCamera);

    app.loader->destroyAsset(app.asset);
    app.materials->destroyMaterials();
    delete app.materials;
    AssetLoader::destroy(&app.loader);
};

int main(int argc, char** argv) {
    static const char* DEFAULT_IBL = "venetian_crossroads_2k";

    LucyApp app;
    app.config.title = "Lucy";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;
    app.config.resizeable = false;
    app.showQuads = true;
    app.showImgui = true;

    // Handle arguments.
    static constexpr const char* OPTSTR = "i:";
    static const struct option OPTIONS[] = {
        { "ibl", required_argument, nullptr, 'i' },
        { nullptr, 0, nullptr, 0 }
    };
    int opt, option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            case 'i': app.config.iblDirectory = arg; break;
        }
    }

    // Configure the animation callback.
    FilamentApp::get().animate(
            [&app](Engine* engine, View* view, double now) { animate(app, engine, view, now); });

    // Start the app.
    FilamentApp::get().run(app.config,
            [&app](Engine* engine, View* view, Scene* scene) { setup(app, engine, view, scene); },
            [&app](Engine* engine, View*, Scene*) { cleanup(app, engine); },
            [&app](Engine* engine, View* view) { drawHud(app, engine, view); },
            FilamentApp::PreRenderCallback(),
            FilamentApp::PostRenderCallback(),
            WINDOW_WIDTH, WINDOW_HEIGHT);

    return 0;
}
