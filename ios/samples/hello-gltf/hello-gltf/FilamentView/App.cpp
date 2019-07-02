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

#include "App.h"

#include <filament/LightManager.h>
#include <filament/TransformManager.h>

#include <image/KtxUtility.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/ResourceLoader.h>

#include <fstream>
#include <sstream>

// This file is generated via the "Run Script" build phase and contains the IBL and skybox
// textures this app uses.
#include "resources.h"

App::App(void* nativeLayer, uint32_t width, uint32_t height, const utils::Path& resourcePath)
        : nativeLayer(nativeLayer), width(width), height(height), resourcePath(resourcePath) {
    setupFilament();
    setupIbl();
    setupMesh();
    setupView();
}

void App::render() {
    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
}

void App::pan(float deltaX, float deltaY) {
    cameraManipulator.rotate(filament::math::double2(deltaX, -deltaY), 10);
}

App::~App() {
    engine->destroy(app.indirectLight);
    engine->destroy(app.iblTexture);
    engine->destroy(app.skyboxTexture);
    engine->destroy(app.skybox);
    app.assetLoader->destroyAsset(app.asset);
    app.materialProvider->destroyMaterials();
    delete app.materialProvider;
    gltfio::AssetLoader::destroy(&app.assetLoader);

    engine->destroy(renderer);
    engine->destroy(scene);
    engine->destroy(view);
    engine->destroy(camera);
    engine->destroy(swapChain);
    engine->destroy(&engine);
}

void App::setupFilament() {
#if FILAMENT_APP_USE_OPENGL
    engine = Engine::create(filament::Engine::Backend::OPENGL);
#elif FILAMENT_APP_USE_METAL
    engine = Engine::create(filament::Engine::Backend::METAL);
#endif
    swapChain = engine->createSwapChain(nativeLayer);
    renderer = engine->createRenderer();
    scene = engine->createScene();
    camera = engine->createCamera();
    cameraManipulator.setCamera(camera);
    cameraManipulator.setViewport(width, height);
    cameraManipulator.lookAt(filament::math::double3(0, 0, 3), filament::math::double3(0, 0, 0));
    camera->setProjection(60, (float) width / height, 0.1, 10);
}

void App::setupIbl() {
    image::KtxBundle* iblBundle = new image::KtxBundle(RESOURCES_VENETIAN_CROSSROADS_IBL_DATA,
                                                       RESOURCES_VENETIAN_CROSSROADS_IBL_SIZE);
    float3 harmonics[9];
    iblBundle->getSphericalHarmonics(harmonics);
    app.iblTexture = image::KtxUtility::createTexture(engine, iblBundle, false);

    image::KtxBundle* skyboxBundle = new image::KtxBundle(RESOURCES_VENETIAN_CROSSROADS_SKYBOX_DATA,
                                                          RESOURCES_VENETIAN_CROSSROADS_SKYBOX_SIZE);
    app.skyboxTexture = image::KtxUtility::createTexture(engine, skyboxBundle, false);

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
        .direction({0.0, -1.0, 0.0})
        .build(*engine, app.sun);
    scene->addEntity(app.sun);
}

void App::setupMesh() {
    app.materialProvider = gltfio::createUbershaderLoader(engine);
    app.assetLoader = gltfio::AssetLoader::create({engine, app.materialProvider, nullptr});

    // Load the glTF file.
    std::ifstream in(resourcePath.concat(utils::Path("DamagedHelmet.glb")), std::ifstream::in);
    in.seekg(0, std::ios::end);
    std::ifstream::pos_type size = in.tellg();
    in.seekg(0);
    std::vector<uint8_t> buffer(size);
    if (!in.read((char*) buffer.data(), size)) {
        std::cerr << "Unable to read scene.gltf" << std::endl;
        exit(1);
    }
    app.asset = app.assetLoader->createAssetFromBinary(buffer.data(), static_cast<uint32_t>(size));

    gltfio::ResourceLoader({
        .engine = engine,
        .normalizeSkinningWeights = true,
        .recomputeBoundingBoxes = false
    }).loadResources(app.asset);

    scene->addEntities(app.asset->getEntities(), app.asset->getEntityCount());
}

void App::setupView() {
    view = engine->createView();
    view->setDepthPrepass(filament::View::DepthPrepass::DISABLED);
    view->setScene(scene);
    view->setCamera(camera);
    view->setViewport(Viewport(0, 0, width, height));

    // Even FXAA anti-aliasing is overkill on iOS retina screens. This saves a few ms.
    view->setAntiAliasing(View::AntiAliasing::NONE);
}
