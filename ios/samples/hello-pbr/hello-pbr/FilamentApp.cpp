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

#include <filament/Material.h>
#include <filament/Viewport.h>

#include <filameshio/MeshReader.h>

#include <ktxreader/Ktx1Reader.h>

#include <sstream>

// This file is generated via the "Run Script" build phase and contains the mesh, material, and IBL
// textures this app uses.
#include "resources.h"

using namespace ktxreader;

using namespace filament;
using namespace filamesh;

const double kFov = 60.0;
const double kNearPlane = 0.1;
const double kFarPlane = 10.0;

void FilamentApp::initialize() {
#if FILAMENT_APP_USE_OPENGL
    engine = Engine::create(filament::Engine::Backend::OPENGL);
#elif FILAMENT_APP_USE_METAL
    engine = Engine::create(filament::Engine::Backend::METAL);
#endif
    swapChain = engine->createSwapChain(nativeLayer);
    renderer = engine->createRenderer();
    scene = engine->createScene();
    Entity c = EntityManager::get().create();
    camera = engine->createCamera(c);

    filaView = engine->createView();

    image::Ktx1Bundle* iblBundle = new image::Ktx1Bundle(RESOURCES_VENETIAN_CROSSROADS_2K_IBL_DATA,
            RESOURCES_VENETIAN_CROSSROADS_2K_IBL_SIZE);
    filament::math::float3 harmonics[9];
    iblBundle->getSphericalHarmonics(harmonics);
    app.iblTexture = Ktx1Reader::createTexture(engine, iblBundle, false);

    image::Ktx1Bundle* skyboxBundle = new image::Ktx1Bundle(RESOURCES_VENETIAN_CROSSROADS_2K_SKYBOX_DATA,
            RESOURCES_VENETIAN_CROSSROADS_2K_SKYBOX_SIZE);
    app.skyboxTexture = Ktx1Reader::createTexture(engine, skyboxBundle, false);

    app.skybox = Skybox::Builder()
        .environment(app.skyboxTexture)
        .build(*engine);

    app.indirectLight = IndirectLight::Builder()
        .reflections(app.iblTexture)
        .irradiance(3, harmonics)
        .intensity(30000)
        .build(*engine);
    scene->setIndirectLight(app.indirectLight);
    scene->setSkybox(app.skybox);

    app.sun = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
        .castShadows(true)
        // The direction is calibrated to match the IBL's sun.
        .direction({0.548267, -0.473983, -0.689016})
        .build(*engine, app.sun);
    scene->addEntity(app.sun);

    app.mat = Material::Builder()
        .package(RESOURCES_CLEAR_COAT_DATA, RESOURCES_CLEAR_COAT_SIZE)
        .build(*engine);

    app.materialInstance = app.mat->createInstance();
    MeshReader::Mesh mesh = MeshReader::loadMeshFromBuffer(engine, RESOURCES_MATERIAL_SPHERE_DATA,
            nullptr, nullptr, app.materialInstance);
    app.materialInstance->setParameter("baseColor", RgbType::sRGB, {0.71f, 0.0f, 0.0f});

    app.renderable = mesh.renderable;
    scene->addEntity(app.renderable);
    auto& rcm = engine->getRenderableManager();
    rcm.setCastShadows(rcm.getInstance(app.renderable), true);

    filaView->setScene(scene);
    filaView->setCamera(camera);

    cameraManipulator.setCamera(camera);
    cameraManipulator.lookAt(filament::math::double3(0, 0, 3), filament::math::double3(0, 0, 0));
}

void FilamentApp::render() {
    if (renderer->beginFrame(swapChain)) {
        renderer->render(filaView);
        renderer->endFrame();
    }
}

void FilamentApp::pan(float deltaX, float deltaY) {
    cameraManipulator.rotate(filament::math::double2(deltaX, -deltaY), 10);
}

void FilamentApp::updateViewportAndCameraProjection(uint32_t width, uint32_t height,
        float scaleFactor) {
    if (!filaView || !camera) {
        return;
    }

    cameraManipulator.setViewport(width, height);

    const uint32_t scaledWidth = width * scaleFactor;
    const uint32_t scaledHeight = height * scaleFactor;
    filaView->setViewport({0, 0, scaledWidth, scaledHeight});

    const double aspect = (double)width / height;
    camera->setProjection(kFov, aspect, kNearPlane, kFarPlane);
}

FilamentApp::~FilamentApp() {
    engine->destroy(app.materialInstance);
    engine->destroy(app.mat);
    engine->destroy(app.indirectLight);
    engine->destroy(app.iblTexture);
    engine->destroy(app.skyboxTexture);
    engine->destroy(app.skybox);
    engine->destroy(app.renderable);
    engine->destroy(app.sun);

    engine->destroy(renderer);
    engine->destroy(scene);
    engine->destroy(filaView);
    Entity c = camera->getEntity();
    engine->destroyCameraComponent(c);
    EntityManager::get().destroy(c);
    engine->destroy(swapChain);
    engine->destroy(&engine);
}
