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

#include <iostream>
#include <fstream>
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
    MaterialSource materialSource = JITSHADER;
    ResourceLoader* resourceLoader = nullptr;
    gltfio::TextureProvider* stbDecoder = nullptr;
    gltfio::TextureProvider* ktxDecoder = nullptr;
    int instanceToAnimate = -1;
    std::vector<FilamentInstance*> instances;
};

static const char* DEFAULT_IBL = "assets/ibl/lightroom_14b";

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
        "SHOWCASE renders the specified glTF file with instancing\n"
        "Usage:\n"
        "    SHOWCASE [options] <gltf path>\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
        "   --ibl=<path to cmgen IBL>, -i <path>\n"
        "       Override the built-in IBL\n\n"
        "   --num=<number of initial instances>, -n <num>\n"
        "       Number of instances to start with (defaults to 0)\n\n"
        "   --animate=<instance index>, -m <num>\n"
        "       Instance to animate (defaults to all instances)\n\n"
        "   --ubershader, -u\n"
        "       Enable ubershaders (improves load time, adds shader complexity)\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:i:un:m:";
    static const struct option OPTIONS[] = {
        { "help",         no_argument,       nullptr, 'h' },
        { "api",          required_argument, nullptr, 'a' },
        { "ibl",          required_argument, nullptr, 'i' },
        { "num",          required_argument, nullptr, 'n' },
        { "animate",      required_argument, nullptr, 'm' },
        { "ubershader",   no_argument,       nullptr, 'u' },
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
                }
                break;
            case 'm':
                app->instanceToAnimate = atoi(arg.c_str());
                break;
            case 'n':
                app->instances.resize(atoi(arg.c_str()));
                break;
            case 'i':
                app->config.iblDirectory = arg;
                break;
            case 'u':
                app->materialSource = UBERSHADER;
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

    app.config.title = "glTF Instancing";
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
                app.instances.data(), app.instances.size());
        buffer.clear();
        buffer.shrink_to_fit();

        if (!app.asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }
    };

    auto loadResources = [&app] (utils::Path filename) {
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

    auto arrangeIntoCircle = [&app]() {
        auto& tcm = app.engine->getTransformManager();
        auto extent = app.asset->getBoundingBox().extent();
        float max_extent = std::max(std::max(extent.x,  extent.y), extent.z);
        auto translation = mat4f::translation(float3(max_extent, 0, 0));
        for (size_t inst = 0; inst < app.instances.size(); ++inst) {
            FilamentInstance* instance = app.instances[inst];
            auto transformRoot = tcm.getInstance(instance->getRoot());
            float theta = inst * 2.0 * M_PI / app.instances.size();
            auto rotation = mat4f::rotation(theta, float3(0, 0, 1));
            tcm.setTransform(transformRoot, rotation * translation);
        }
    };

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());
        app.viewer = new ViewerGui(engine, scene, view);

        app.materials = (app.materialSource == JITSHADER) ? createJitShaderProvider(engine) :
                createUbershaderProvider(engine, UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);

        app.loader = AssetLoader::create({engine, app.materials, app.names });
        if (filename.isEmpty()) {
            app.asset = app.loader->createInstancedAsset(
                    GLTF_DEMO_DAMAGEDHELMET_DATA, GLTF_DEMO_DAMAGEDHELMET_SIZE,
                    app.instances.data(), app.instances.size());
        } else {
            loadAsset(filename);
        }

        FilamentInstance* instance = nullptr;
        if (app.instanceToAnimate > -1 && app.instanceToAnimate < app.instances.size()) {
            instance = app.instances[app.instanceToAnimate];
        }

        arrangeIntoCircle();
        loadResources(filename);
        app.viewer->setAsset(app.asset, instance);
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

    auto animate = [&app, arrangeIntoCircle](Engine* engine, View* view, double now) {
        app.resourceLoader->asyncUpdateLoad();
        app.viewer->updateRootTransform();
        app.viewer->populateScene();

        if (app.instanceToAnimate == -1) {
            for (FilamentInstance* instance : app.instances) {
                app.viewer->applyAnimation(now, instance);
            }
        } else {
            app.viewer->applyAnimation(now);
        }

        // Add a new instance every second until reaching 100 instances.
        static double previous = 0.0;
        if (now - previous > 1.0 && app.asset->getAssetInstanceCount() < 100) {
            FilamentInstance* instance = app.loader->createInstance(app.asset);

            // If the asset has variants, rotate through each variant.
            const size_t variantCount = instance->getMaterialVariantCount();
            if (variantCount > 1) {
                instance->applyMaterialVariant(app.instances.size() % variantCount);
            }

            app.instances.push_back(instance);
            arrangeIntoCircle();
            previous = now;
        }
    };

    auto gui = [&app](Engine* engine, View* view) { };

    auto preRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) { };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.animate(animate);

    filamentApp.run(app.config, setup, cleanup, gui, preRender);

    return 0;
}
