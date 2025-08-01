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
#include "common/configuration.h"

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

#include <filament/Engine.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <viewer/ViewerGui.h>

#include <camutils/Manipulator.h>

#include <getopt/getopt.h>

#include <utils/NameComponentManager.h>

#include <fstream>
#include <iostream>
#include <string>

#include <math/mat4.h>

#include "generated/resources/gltf_demo.h"
#include "materials/uberarchive.h"

using namespace filament;
using namespace filament::math;
using namespace filament::viewer;

using namespace filament::gltfio;
using namespace utils;

enum MaterialSource {
    JITSHADER,
    UBERSHADER,
};

struct App {
    Engine* engine;
    ViewerGui* viewer;
    Config config;
    AssetLoader* loader;
    FilamentAsset* asset = nullptr;
    NameComponentManager* names;
    MaterialProvider* materials;
    MaterialSource materialSource = UBERSHADER;
    ResourceLoader* resourceLoader = nullptr;
    gltfio::TextureProvider* stbDecoder = nullptr;
    gltfio::TextureProvider* ktxDecoder = nullptr;
    FilamentInstance* instance;
    bool enableMSAA = false;
};

static const char* DEFAULT_IBL = "assets/ibl/lightroom_14b";

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage("SHOWCASE renders the specified glTF file with minimal features.\nPrimarily "
                      "for early backend bringup.\n"
                      "Usage:\n"
                      "    SHOWCASE [options] <gltf path>\n"
                      "Options:\n"
                      "   --help, -h\n"
                      "       Prints this message\n\n"
                      "API_USAGE"
                      "   --ibl=<path to cmgen IBL>, -i <path>\n"
                      "       Override the built-in IBL\n\n"
                      "   --msaa, -m\n"
                      "       setMultiSampleAntiAliasingOptions\n\n");
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos;
            pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), samples::getBackendAPIArgumentsUsage());
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "hma:i:";
    static const struct option OPTIONS[] = { { "help", no_argument, nullptr, 'h' },
        { "api", required_argument, nullptr, 'a' },
        { "ibl", required_argument, nullptr, 'i' },
        { "msaa", no_argument, nullptr, 'm' },
        { nullptr, 0, nullptr, 0 } };
    int opt;
    int option_index = 0;
    // Make WebGPU the default for this test
    app->config.backend = Engine::Backend::WEBGPU;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                app->config.backend = samples::parseArgumentsForBackend(arg);
                break;
            case 'i':
                app->config.iblDirectory = arg;
                break;
            case 'm':
                app->enableMSAA = true;
                break;
        }
    }
    return optind;
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

int main(int argc, char** argv) {
    App app;

    app.config.title = "simple glTF";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;

    int optionIndex = handleCommandLineArguments(argc, argv, &app);
    utils::Path filename;
    int num_args = argc - optionIndex;
    if (num_args >= 1) {
        filename = argv[optionIndex];
        if (!filename.exists()) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
    }

    auto loadAsset = [&app](utils::Path filename) {
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
        app.asset = app.loader->createInstancedAsset(buffer.data(), buffer.size(),
                &(app.instance), 1);
        buffer.clear();
        buffer.shrink_to_fit();

        if (!app.asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }
    };

    auto loadResources = [&app](utils::Path filename) {
        // Load external textures and buffers.
        std::string gltfPath = filename.getAbsolutePath();
        ResourceConfiguration configuration;
        configuration.engine = app.engine;
        configuration.gltfPath = gltfPath.c_str();
        configuration.normalizeSkinningWeights = true;
        if (!app.resourceLoader) {
            app.resourceLoader = new gltfio::ResourceLoader(configuration);
            app.stbDecoder = createStbProvider(app.engine);
            app.ktxDecoder = createKtx2Provider(app.engine);
            app.resourceLoader->addTextureProvider("image/png", app.stbDecoder);
            app.resourceLoader->addTextureProvider("image/jpeg", app.stbDecoder);
            app.resourceLoader->addTextureProvider("image/ktx2", app.ktxDecoder);
        }

        if (!app.resourceLoader->asyncBeginLoad(app.asset)) {
            std::cerr << "Unable to start loading resources for " << filename << std::endl;
            exit(1);
        }

        auto ibl = FilamentApp::get().getIBL();
        if (ibl) {
            app.viewer->setIndirectLight(ibl->getIndirectLight(), ibl->getSphericalHarmonics());
        }
    };

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        if (app.enableMSAA) {
            // TODO Investigate why
            // Enabling MSAA fixes the "transmission" sample but breaks the others
            view->setMultiSampleAntiAliasingOptions({ .enabled = true});
        }
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());
        app.viewer = new ViewerGui(engine, scene, view);


        app.materials = (app.materialSource == JITSHADER)
                                ? createJitShaderProvider(engine, false /* optimize */,
                                          samples::getJitMaterialVariantFilter(app.config.backend))
                                : createUbershaderProvider(engine, UBERARCHIVE_DEFAULT_DATA,
                                          UBERARCHIVE_DEFAULT_SIZE);

        app.loader = AssetLoader::create({ engine, app.materials, app.names });
        if (filename.isEmpty()) {
            app.asset = app.loader->createInstancedAsset(GLTF_DEMO_DAMAGEDHELMET_DATA,
                    GLTF_DEMO_DAMAGEDHELMET_SIZE, &(app.instance), 1);
        } else {
            loadAsset(filename);
        }

        loadResources(filename);
        app.viewer->setAsset(app.asset, app.instance);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        app.loader->destroyAsset(app.asset);
        app.materials->destroyMaterials();

        delete app.viewer;
        delete app.materials;
        delete app.names;
        delete app.resourceLoader;
        delete app.stbDecoder;
        delete app.ktxDecoder;

        AssetLoader::destroy(&app.loader);
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        app.resourceLoader->asyncUpdateLoad();
        app.viewer->updateRootTransform();
        app.viewer->populateScene();

        app.viewer->applyAnimation(now, app.instance);
    };

    auto gui = [&app](Engine* engine, View* view) {};

    auto preRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {};

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.animate(animate);

    filamentApp.run(app.config, setup, cleanup, gui, preRender);

    return 0;
}
