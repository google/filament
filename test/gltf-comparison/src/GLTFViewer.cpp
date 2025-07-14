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

#include "GLTFViewer.h"

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
#include <utils/NameComponentManager.h>

#include <gltfio/TextureProvider.h>

//#include "generated/resources/gltf.h"
#include "materials/uberarchive.h"

#include <iostream>
#include <fstream>

using namespace filament;
using utils::Entity;
using utils::EntityManager;


void GLTFViewer::runApp(std::function<void(filament::Engine*, filament::View*, filament::Scene*,
                    filament::Renderer*)> postRender) {
    FilamentApp::get().animate(
            [this](Engine* engine, View* view, double now) { animate(engine, view, now); });

    // TODO: Remove
    mApp.config.backend = filament::Engine::Backend::METAL;

    FilamentApp::get().run(
            mApp.config,
            [this](Engine* engine, View* view, Scene* scene) { setup(engine, view, scene); },
            [this](Engine* engine, View* view, Scene* scene) { cleanup(engine, view, scene); },
            FilamentApp::ImGuiCallback(), FilamentApp::PreRenderCallback(), std::move(postRender));
}

void GLTFViewer::setup(Engine* engine, filament::View* view, filament::Scene* scene) {
    if (mApp.enableMSAA) {
        // TODO Investigate why
        // Enabling MSAA fixes the "transmission" sample but breaks the others
        view->setMultiSampleAntiAliasingOptions({ .enabled = true});
    }
    mApp.engine = engine;
    mApp.names = new utils::NameComponentManager(EntityManager::get());
    mApp.viewer = new viewer::ViewerGui(engine, scene, view);

    // TODO: Support JIT
    mApp.materials = //(mApp.materialSource == JITSHADER)
                     //? createJitShaderProvider(engine, false /* optimize */,
                     // samples::getJitMaterialVariantFilter(mApp.config.backend))
                     //:
            filament::gltfio::createUbershaderProvider(engine, UBERARCHIVE_DEFAULT_DATA,
                    UBERARCHIVE_DEFAULT_SIZE);

    mApp.loader = gltfio::AssetLoader::create({ engine, mApp.materials, mApp.names });

    loadAsset(mFilename);
    loadResources(mFilename);
    mApp.viewer->setAsset(mApp.asset, mApp.instance);
}

void GLTFViewer::cleanup(Engine* engine, filament::View* view, filament::Scene* scene) {
    mApp.loader->destroyAsset(mApp.asset);
    mApp.materials->destroyMaterials();

    delete mApp.materials;
    delete mApp.names;
    delete mApp.viewer;
    delete mApp.resourceLoader;
    delete mApp.stbDecoder;
    delete mApp.ktxDecoder;

    gltfio::AssetLoader::destroy(&mApp.loader);
}

void GLTFViewer::animate(filament::Engine* engine, filament::View* view, double now) {
      mApp.resourceLoader->asyncUpdateLoad();

    mApp.viewer->updateRootTransform();
    mApp.viewer->populateScene();

    mApp.viewer->applyAnimation(now, mApp.instance);
}

void GLTFViewer::setFilename(std::string filename) {
    mFilename = std::move(filename);
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

void GLTFViewer::loadAsset(utils::Path filename) {
    // Peek at the file size to allow pre-allocation.
    long contentSize = static_cast<long>(getFileSize(filename.c_str()));
    if (contentSize <= 0) {
        std::cerr << "Unable to open " << filename << std::endl;
        exit(1);
    }

    // Consume the glTF file.
    std::ifstream in(filename.c_str(), std::ifstream::binary | std::ifstream::in);
    std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
    if (!in.read((char*) buffer.data(), contentSize)) {
        std::cerr << "Unable to read " << filename << std::endl;
        exit(1);
    }

    // Parse the glTF file and create Filament entities.
    mApp.asset = mApp.loader->createInstancedAsset(buffer.data(), buffer.size(),
            &(mApp.instance), 1);
    buffer.clear();
    buffer.shrink_to_fit();

    if (!mApp.asset) {
        std::cerr << "Unable to parse " << filename << std::endl;
        exit(1);
    }
}

void GLTFViewer::loadResources(utils::Path filename) {
    // Load external textures and buffers.
    std::string gltfPath = filename.getAbsolutePath();
    gltfio::ResourceConfiguration configuration;
    configuration.engine = mApp.engine;
    configuration.gltfPath = gltfPath.c_str();
    configuration.normalizeSkinningWeights = true;
    if (!mApp.resourceLoader) {
        mApp.resourceLoader = new gltfio::ResourceLoader(configuration);
        mApp.stbDecoder = gltfio::createStbProvider(mApp.engine);
        mApp.ktxDecoder = gltfio::createKtx2Provider(mApp.engine);
        mApp.resourceLoader->addTextureProvider("image/png", mApp.stbDecoder);
        mApp.resourceLoader->addTextureProvider("image/jpeg", mApp.stbDecoder);
        mApp.resourceLoader->addTextureProvider("image/ktx2", mApp.ktxDecoder);
    }

    if (!mApp.resourceLoader->asyncBeginLoad(mApp.asset)) {
        std::cerr << "Unable to start loading resources for " << filename << std::endl;
        exit(1);
    }

    auto ibl = FilamentApp::get().getIBL();
    if (ibl) {
        mApp.viewer->setIndirectLight(ibl->getIndirectLight(), ibl->getSphericalHarmonics());
    }
}
