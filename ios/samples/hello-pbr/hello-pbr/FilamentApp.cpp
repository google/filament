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

#include <image/KtxUtility.h>

#include <sstream>

// This file is generated via the "Run Script" build phase and contains the mesh, material, and IBL
// textures this app uses.
#include "resources.h"

using namespace filament;
using namespace filamesh;

void FilamentApp::initialize() {
#if FILAMENT_APP_USE_OPENGL
    engine = Engine::create(filament::Engine::Backend::OPENGL);
#elif FILAMENT_APP_USE_METAL
    engine = Engine::create(filament::Engine::Backend::METAL);
#endif
    swapChain = engine->createSwapChain(nativeLayer);
    renderer = engine->createRenderer();
    scene = engine->createScene();
    camera = engine->createCamera();

    filaView = engine->createView();

    image::KtxBundle* iblBundle = new image::KtxBundle(RESOURCES_VENETIAN_CROSSROADS_2K_IBL_DATA,
            RESOURCES_VENETIAN_CROSSROADS_2K_IBL_SIZE);
    filament::math::float3 harmonics[9];
    iblBundle->getSphericalHarmonics(harmonics);
    app.iblTexture = image::ktx::createTexture(engine, iblBundle, false);

    image::KtxBundle* skyboxBundle = new image::KtxBundle(RESOURCES_VENETIAN_CROSSROADS_2K_SKYBOX_DATA,
            RESOURCES_VENETIAN_CROSSROADS_2K_SKYBOX_SIZE);
    app.skyboxTexture = image::ktx::createTexture(engine, skyboxBundle, false);

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
    filaView->setViewport(Viewport(0, 0, width, height));

    const uint32_t w = filaView->getViewport().width;
    const uint32_t h = filaView->getViewport().height;
    const float aspect = (float) w / h;
    cameraManipulator.setCamera(camera);
    cameraManipulator.setViewport(w, h);
    cameraManipulator.lookAt(filament::math::double3(0, 0, 3), filament::math::double3(0, 0, 0));
    camera->setProjection(60, aspect, 0.1, 10);
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
    engine->destroy(camera);
    engine->destroy(swapChain);
    engine->destroy(&engine);
}
