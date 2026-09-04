/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "common/arguments.h"

#include "generated/resources/gltf_demo.h"

#include "materials/uberarchive.h"

#include <viewer/ViewerGui.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/DesktopAssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/IBL.h>

#include <filament/Engine.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <camutils/Manipulator.h>

#include <utils/getopt.h>
#include <utils/NameComponentManager.h>

#include <math/mat4.h>

#include <samples/SampleConfig.h>

#include <fstream>
#include <iostream>

using namespace filament;
using namespace filament::math;
using namespace filament::viewer;

using namespace filament::gltfio;
using namespace utils;

namespace {

enum MaterialSource {
    JITSHADER,
    UBERSHADER,
};

struct App {
    FilamentApp2* filamentApp = nullptr;
    Engine* engine;
    ViewerGui* viewer;
    SampleConfig config;
    AssetLoader* assetLoader;
    FilamentAsset* asset = nullptr;
    NameComponentManager* names;
    MaterialProvider* materials;
    MaterialSource materialSource = JITSHADER;
    ResourceLoader* resourceLoader = nullptr;
    gltfio::TextureProvider* stbDecoder = nullptr;
    gltfio::TextureProvider* ktxDecoder = nullptr;
    gltfio::TextureProvider* webpDecoder = nullptr;
    int instanceToAnimate = -1;
    std::vector<FilamentInstance*> instances;
};

const char* DEFAULT_IBL = "assets/ibl/lightroom_14b";

std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* appLoader) {
    if (config.iblDirectory.empty()) {
        config.iblDirectory =
                utils::CString((FilamentApp2::getRootAssetsPath() + DEFAULT_IBL).c_str());
    }
    auto app = std::make_shared<App>();
    app->config = config;
    app->instanceToAnimate = config.getInt("animate", -1);
    app->instances.resize(config.getInt("num", 1));
    if (config.getBool("ubershader")) {
        app->materialSource = UBERSHADER;
    }

    auto loadAsset = [app, appLoader]() {
        utils::Path filename(!app->config.positionalArgs.empty() ?
                app->config.positionalArgs[0].c_str_safe() : "");
        std::vector<uint8_t> buffer = appLoader->load(filename);
        if (buffer.empty()) {
            std::cerr << "Unable to open " << filename << std::endl;
            exit(1);
        }

        // Parse the glTF file and create Filament entities.
        app->asset = app->assetLoader->createInstancedAsset(buffer.data(), buffer.size(),
                app->instances.data(), app->instances.size());
        buffer.clear();
        buffer.shrink_to_fit();

        if (!app->asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }
    };

    auto loadResources = [app, appLoader]() {
        utils::Path filename(!app->config.positionalArgs.empty() ?
                app->config.positionalArgs[0].c_str_safe() : "");
        // Load external textures and buffers.
        utils::CString gltfPath = utils::CString(filename.getAbsolutePath().c_str());
        ResourceConfiguration configuration;
        configuration.engine = app->engine;
        configuration.gltfPath = gltfPath.c_str();
        configuration.normalizeSkinningWeights = true;
        if (!app->resourceLoader) {
            app->resourceLoader = new gltfio::ResourceLoader(configuration);
            app->stbDecoder = createStbProvider(app->engine);
            app->ktxDecoder = createKtx2Provider(app->engine);
            app->resourceLoader->addTextureProvider("image/png", app->stbDecoder);
            app->resourceLoader->addTextureProvider("image/jpeg", app->stbDecoder);
            app->resourceLoader->addTextureProvider("image/ktx2", app->ktxDecoder);
            if (isWebpSupported()) {
                app->webpDecoder = createWebpProvider(app->engine);
                app->resourceLoader->addTextureProvider("image/webp", app->webpDecoder);
            }
            else {
                app->webpDecoder = nullptr;
            }
        }

        // We explicitly fetch the raw bytes and push them into ResourceLoader via addResourceData.
        // This pre-populates the cache and completely bypasses ResourceLoader's internal disk I/O,
        // which is required for Android compatibility (where assets are packed in the APK).
        for (size_t i = 0, c = app->asset->getResourceUriCount(); i < c; i++) {
            const char* uri = app->asset->getResourceUris()[i];
            utils::Path uriPath = filename.getParent() + uri;
            std::vector<uint8_t> buffer = appLoader->load(uriPath);
            if (!buffer.empty()) {
                auto* b = new std::vector<uint8_t>(std::move(buffer));
                gltfio::ResourceLoader::BufferDescriptor desc(
                    b->data(), b->size(),
                    [](void*, size_t, void* user) {
                        delete static_cast<std::vector<uint8_t>*>(user);
                    },
                    b
                );
                app->resourceLoader->addResourceData(uri, std::move(desc));
            }
        }

        if (!app->resourceLoader->asyncBeginLoad(app->asset)) {
            std::cerr << "Unable to start loading resources for " << filename << std::endl;
            exit(1);
        }

        auto ibl = app->filamentApp->getIBL();
        if (ibl) {
            app->viewer->setIndirectLight(ibl->getIndirectLight(), ibl->getSphericalHarmonics());
        }
    };

    auto arrangeIntoCircle = [app]() {
        auto& tcm = app->engine->getTransformManager();
        auto extent = app->asset->getBoundingBox().extent();
        float max_extent = std::max(std::max(extent.x,  extent.y), extent.z);
        auto translation = mat4f::translation(float3(max_extent, 0, 0));
        for (size_t inst = 0; inst < app->instances.size(); ++inst) {
            FilamentInstance* instance = app->instances[inst];
            auto transformRoot = tcm.getInstance(instance->getRoot());
            float theta = inst * 2.0 * M_PI / app->instances.size();
            auto rotation = mat4f::rotation(theta, float3(0, 0, 1));
            tcm.setTransform(transformRoot, rotation * translation);
        }
    };

    auto setup = [app, loadAsset, loadResources, arrangeIntoCircle, appLoader](Engine* engine,
                         View* view, Scene* scene) {
        app->engine = engine;
        app->names = new NameComponentManager(EntityManager::get());
        app->viewer = new ViewerGui(engine, scene, view);

        app->materials = (app->materialSource == JITSHADER)
                                 ? createJitShaderProvider(engine, false /* optimize */, {})
                                 : createUbershaderProvider(engine, UBERARCHIVE_DEFAULT_DATA,
                                           UBERARCHIVE_DEFAULT_SIZE);

        app->assetLoader = AssetLoader::create({ engine, app->materials, app->names });
        utils::Path filename(!app->config.positionalArgs.empty() ?
                app->config.positionalArgs[0].c_str_safe() : "");
        if (filename.isEmpty()) {
            app->asset = app->assetLoader->createInstancedAsset(GLTF_DEMO_DAMAGEDHELMET_DATA,
                    GLTF_DEMO_DAMAGEDHELMET_SIZE, app->instances.data(), app->instances.size());
        } else {
            loadAsset();
        }

        FilamentInstance* instance = nullptr;
        if (app->instanceToAnimate > -1 && app->instanceToAnimate < app->instances.size()) {
            instance = app->instances[app->instanceToAnimate];
        }

        arrangeIntoCircle();
        loadResources();
        app->viewer->setAsset(app->asset, instance);
    };

    auto cleanup = [app, appLoader](Engine* engine, View*, Scene*) {
        app->assetLoader->destroyAsset(app->asset);
        app->materials->destroyMaterials();

        delete app->viewer;
        delete app->materials;
        delete app->names;
        delete app->resourceLoader;
        delete app->stbDecoder;
        delete app->ktxDecoder;
        delete app->webpDecoder;
        AssetLoader::destroy(&app->assetLoader);
    };

    auto animate = [app, arrangeIntoCircle](Engine* engine, View* view, double now) {
        app->resourceLoader->asyncUpdateLoad();
        app->viewer->updateRootTransform();
        app->viewer->populateScene();

        app->names->gc();
        app->assetLoader->gc();

        if (app->instanceToAnimate == -1) {
            for (FilamentInstance* instance: app->instances) {
                app->viewer->applyAnimation(now, instance);
            }
        } else {
            app->viewer->applyAnimation(now);
        }

        // Add a new instance every second until reaching 100 instances.
        static double previous = 0.0;
        if (now - previous > 1.0 && app->asset->getAssetInstanceCount() < 100) {
            FilamentInstance* instance = app->assetLoader->createInstance(app->asset);

            // If the asset has variants, rotate through each variant.
            const size_t variantCount = instance->getMaterialVariantCount();
            if (variantCount > 1) {
                instance->applyMaterialVariant(app->instances.size() % variantCount);
            }

            app->instances.push_back(instance);
            arrangeIntoCircle();
            previous = now;
        }
    };

    auto gui = [app](Engine* engine, View* view) {};

    auto preRender = [app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {};

    auto fApp = samples::getBuilder(config, dm, appLoader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .imgui(gui)
                        .preRender(preRender)
                        .animation(animate)
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

samples::SampleParameters createAppParameters() {
    return {
        samples::Parameter::makeInt("num", 'n', "Number of instances to create", 1, 1),
        samples::Parameter::makeInt("animate", 'm', "Index of instance to animate (-1 for all)",
                -1, -1),
        samples::Parameter::makeBool("ubershader", 'u', "Enable ubershader", false),
    };
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "glTF Instancing";
    config.iblDirectory = utils::CString((FilamentApp2::getRootAssetsPath() + DEFAULT_IBL).c_str());

    samples::CommandLineSpecification spec = {
        .parameters = createAppParameters(),
    };
    samples::handleCommandLineArguments(argc, argv, &config, spec);
    auto dm = samples::getDisplayManager(config);
    if (!config.positionalArgs.empty()) {
        utils::Path filename(config.positionalArgs[0].c_str_safe());
        if (!filename.exists()) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
    }

    auto loader = new filament::app::DesktopAssetLoader();
    auto fApp = createSampleApp(config, dm.get(), loader);
    if (fApp) {
        fApp->run();
    }
    delete loader;

    return 0;
}
#endif
