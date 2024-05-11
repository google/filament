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
#include <filament/Viewport.h>

#include <ktxreader/Ktx1Reader.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>
#include <gltfio/materials/uberarchive.h>

#include <fstream>
#include <iostream>
#include <sstream>

// This file is generated via the "Run Script" build phase and contains the IBL and skybox
// textures this app uses.
#include "resources.h"

using namespace ktxreader;

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
    filament::gltfio::AssetLoader::destroy(&app.assetLoader);

    engine->destroy(renderer);
    engine->destroy(scene);
    engine->destroy(view);
    Entity c = camera->getEntity();
    engine->destroyCameraComponent(c);
    EntityManager::get().destroy(c);
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
    Entity c = EntityManager::get().create();
    camera = engine->createCamera(c);
    cameraManipulator.setCamera(camera);
    cameraManipulator.setViewport(width, height);
    cameraManipulator.lookAt(filament::math::double3(0, 0, 3), filament::math::double3(0, 0, 0));
    camera->setProjection(60, (float) width / height, 0.1, 10);
}

void App::setupIbl() {
    image::Ktx1Bundle* iblBundle = new image::Ktx1Bundle(RESOURCES_VENETIAN_CROSSROADS_2K_IBL_DATA,
                                                       RESOURCES_VENETIAN_CROSSROADS_2K_IBL_SIZE);
    float3 harmonics[9];
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
        .direction({0.0, -1.0, 0.0})
        .build(*engine, app.sun);
    scene->addEntity(app.sun);
}

void App::setupMesh() {
    app.materialProvider = filament::gltfio::createUbershaderProvider(engine,
            UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);
    app.assetLoader = filament::gltfio::AssetLoader::create({engine, app.materialProvider, nullptr});

    // Load the glTF file.
    std::ifstream in(resourcePath.concat(utils::Path("DamagedHelmet.glb")), std::ifstream::in);
    in.seekg(0, std::ios::end);
    std::ifstream::pos_type size = in.tellg();
    in.seekg(0);
    std::vector<uint8_t> buffer(size);
    if (!in.read((char*) buffer.data(), size)) {
        std::cerr << "Unable to read glTF" << std::endl;
        exit(1);
    }
    app.asset = app.assetLoader->createAsset(buffer.data(), static_cast<uint32_t>(size));

    auto resourceLoader = new filament::gltfio::ResourceLoader({
        .engine = engine,
        .normalizeSkinningWeights = true
    });
    auto stbDecoder = filament::gltfio::createStbProvider(engine);
    auto ktxDecoder = filament::gltfio::createKtx2Provider(engine);

    resourceLoader->addTextureProvider("image/png", stbDecoder);
    resourceLoader->addTextureProvider("image/jpeg", stbDecoder);
    resourceLoader->addTextureProvider("image/ktx2", ktxDecoder);
    resourceLoader->loadResources(app.asset);

    delete resourceLoader;
    delete stbDecoder;
    delete ktxDecoder;

    scene->addEntities(app.asset->getEntities(), app.asset->getEntityCount());
}

void App::setupView() {
    view = engine->createView();
    view->setScene(scene);
    view->setCamera(camera);
    view->setViewport(Viewport(0, 0, width, height));

    // Even FXAA anti-aliasing is overkill on iOS retina screens. This saves a few ms.
    view->setAntiAliasing(View::AntiAliasing::NONE);
}
