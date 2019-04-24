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

#include "FilamentApp.h"

#include <filament/LightManager.h>
#include <filament/TransformManager.h>

#include <filameshio/MeshReader.h>
#include <image/KtxUtility.h>

#include <sstream>

// This file is generated via the "Run Script" build phase and contains the mesh, materials, and IBL
// textures this app uses.
#include "resources.h"

using namespace filamesh;

static constexpr float OBJECT_SCALE = 0.02f;

FilamentApp::FilamentApp(void* nativeLayer, uint32_t width, uint32_t height)
        : nativeLayer(nativeLayer), width(width), height(height) {
    setupFilament();
    setupCameraFeedTriangle();
    setupIbl();
    setupMaterial();
    setupMesh();
    setupView();
}

void FilamentApp::render(const FilamentArFrame& frame) {
    app.cameraFeedTriangle->setCameraFeedTexture(frame.cameraImage);
    app.cameraFeedTriangle->setCameraFeedTransform(frame.cameraTextureTransform);
    camera->setModelMatrix(frame.view);
    camera->setCustomProjection(frame.projection, 0.01, 10);

    // Rotate the mesh a little bit each frame.
    meshRotation *= quatf::fromAxisAngle(float3{1.0f, 0.5f, 0.2f}, 0.05);

    // Update the mesh's transform.
    auto& tcm = engine->getTransformManager();
    auto i = tcm.getInstance(app.renderable);
    tcm.setTransform(i,
                     meshTransform *
                     mat4f(meshRotation) *
                     mat4f::scaling(OBJECT_SCALE));

    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
}

void FilamentApp::setObjectTransform(const mat4f& transform) {
    meshTransform = transform;
}

FilamentApp::~FilamentApp() {
    delete app.cameraFeedTriangle;

    engine->destroy(app.materialInstance);
    engine->destroy(app.mat);
    engine->destroy(app.indirectLight);
    engine->destroy(app.iblTexture);
    engine->destroy(app.renderable);

    engine->destroy(renderer);
    engine->destroy(scene);
    engine->destroy(view);
    engine->destroy(camera);
    engine->destroy(swapChain);
    engine->destroy(&engine);
}

void FilamentApp::setupFilament() {
    engine = Engine::create(filament::Engine::Backend::METAL);
    swapChain = engine->createSwapChain(nativeLayer);
    renderer = engine->createRenderer();
    scene = engine->createScene();
    camera = engine->createCamera();
    camera->setProjection(60, (float) width / height, 0.1, 10);
}

void FilamentApp::setupIbl() {
    image::KtxBundle* iblBundle = new image::KtxBundle(RESOURCES_VENETIAN_CROSSROADS_IBL_DATA,
                                                       RESOURCES_VENETIAN_CROSSROADS_IBL_SIZE);
    float3 harmonics[9];
    parseSphereHarmonics(iblBundle->getMetadata("sh"), harmonics);
    app.iblTexture = image::KtxUtility::createTexture(engine, iblBundle, false, true);

    app.indirectLight = IndirectLight::Builder()
        .reflections(app.iblTexture)
        .irradiance(3, harmonics)
        .intensity(30000)
        .build(*engine);
    scene->setIndirectLight(app.indirectLight);

    app.sun = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
        .castShadows(true)
        .direction({0.0, -1.0, 0.0})
        .build(*engine, app.sun);
    scene->addEntity(app.sun);
}

void FilamentApp::setupMaterial() {
    app.mat = Material::Builder()
        .package(RESOURCES_CLEAR_COAT_DATA, RESOURCES_CLEAR_COAT_SIZE)
        .build(*engine);
    app.materialInstance = app.mat->createInstance();
}

void FilamentApp::setupMesh() {
    MeshReader::Mesh mesh = MeshReader::loadMeshFromBuffer(engine, RESOURCES_CUBE_DATA,
            nullptr, nullptr, app.materialInstance);
    app.materialInstance->setParameter("baseColor", RgbType::sRGB, {0.71f, 0.0f, 0.0f});

    app.renderable = mesh.renderable;
    scene->addEntity(app.renderable);

    // Position the mesh 2 units down the Z axis.
    auto& tcm = engine->getTransformManager();
    tcm.create(app.renderable);
    auto i = tcm.getInstance(app.renderable);
    tcm.setTransform(i,
            mat4f::translation(float3{0.0f, 0.0f, -2.0f}) *
            mat4f::scaling(OBJECT_SCALE));
}

void FilamentApp::setupView() {
    view = engine->createView();
    view->setDepthPrepass(filament::View::DepthPrepass::DISABLED);
    view->setScene(scene);
    view->setCamera(camera);
    view->setViewport(Viewport(0, 0, width, height));
}

void FilamentApp::setupCameraFeedTriangle() {
    app.cameraFeedTriangle = new FullScreenTriangle(engine);
    scene->addEntity(app.cameraFeedTriangle->getEntity());
}

void FilamentApp::parseSphereHarmonics(const char* str, float3 harmonics[9]) {
    std::istringstream iss(str);
    std::string read;
    for (int i = 0; i < 9; i++) {
        float3 harmonic;
        iss >> read;
        harmonic.x = std::stof(read);
        iss >> read;
        harmonic.y = std::stof(read);
        iss >> read;
        harmonic.z = std::stof(read);
        harmonics[i] = std::move(harmonic);
    }
}
