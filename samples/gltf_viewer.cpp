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

#define GLTFIO_SIMPLEVIEWER_IMPLEMENTATION

#include "app/Config.h"
#include "app/FilamentApp.h"
#include "app/IBL.h"

#include <filament/Engine.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/SimpleViewer.h>

#include <getopt/getopt.h>

#include <utils/NameComponentManager.h>

#include <fstream>
#include <iostream>
#include <string>

#include "generated/resources/gltf_viewer.h"

using namespace filament;
using namespace gltfio;
using namespace utils;

struct App {
    Engine* engine;
    SimpleViewer* viewer;
    Config config;
    AssetLoader* loader;
    FilamentAsset* asset = nullptr;
    NameComponentManager* names;
    MaterialProvider* materials;
    MaterialSource materialSource = GENERATE_SHADERS;
    bool actualSize = false;
    gltfio::ResourceLoader* resourceLoader = nullptr;
};

static const char* DEFAULT_IBL = "venetian_crossroads_2k";

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
        "SHOWCASE renders the specified glTF file, or a built-in file if none is specified\n"
        "Usage:\n"
        "    SHOWCASE [options] <gltf path>\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
        "   --ibl=<path to cmgen IBL>, -i <path>\n"
        "       Override the built-in IBL\n\n"
        "   --actual-size, -s\n"
        "       Do not scale the model to fit into a unit cube\n\n"
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
    static constexpr const char* OPTSTR = "ha:i:us";
    static const struct option OPTIONS[] = {
        { "help",       no_argument,       nullptr, 'h' },
        { "api",        required_argument, nullptr, 'a' },
        { "ibl",        required_argument, nullptr, 'i' },
        { "ubershader", no_argument,       nullptr, 'u' },
        { "actual-size", no_argument,      nullptr, 's' },
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
            case 'i':
                app->config.iblDirectory = arg;
                break;
            case 'u':
                app->materialSource = LOAD_UBERSHADERS;
                break;
            case 's':
                app->actualSize = true;
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

    app.config.title = "Filament";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;

    int option_index = handleCommandLineArguments(argc, argv, &app);
    utils::Path filename;
    int num_args = argc - option_index;
    if (num_args >= 1) {
        filename = argv[option_index];
        if (!filename.exists()) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
        if (filename.isDirectory()) {
            auto files = filename.listContents();
            for (auto file : files) {
                if (file.getExtension() == "gltf" || file.getExtension() == "glb") {
                    filename = file;
                    break;
                }
            }
            if (filename.isDirectory()) {
                std::cerr << "no glTF file found in " << filename << std::endl;
                return 1;
            }
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
        std::ifstream in(filename.c_str(), std::ifstream::in);
        std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
        if (!in.read((char*) buffer.data(), contentSize)) {
            std::cerr << "Unable to read " << filename << std::endl;
            exit(1);
        }

        // Parse the glTF file and create Filament entities.
        if (filename.getExtension() == "glb") {
            app.asset = app.loader->createAssetFromBinary(buffer.data(), buffer.size());
        } else {
            app.asset = app.loader->createAssetFromJson(buffer.data(), buffer.size());
        }
        buffer.clear();
        buffer.shrink_to_fit();

        if (!app.asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }
    };

    auto loadResources = [&app] (utils::Path filename) {
        // Load external textures and buffers.
        ResourceConfiguration configuration;
        configuration.engine = app.engine;
        configuration.gltfPath = filename.getAbsolutePath();
        configuration.normalizeSkinningWeights = true;
        configuration.recomputeBoundingBoxes = false;
        if (!app.resourceLoader) {
            app.resourceLoader = new gltfio::ResourceLoader(configuration);
        }
        app.resourceLoader->asyncBeginLoad(app.asset);

        // Load animation data then free the source hierarchy.
        app.asset->getAnimator();
        app.asset->releaseSourceData();

        auto ibl = FilamentApp::get().getIBL();
        if (ibl) {
            app.viewer->setIndirectLight(ibl->getIndirectLight(), ibl->getSphericalHarmonics());
        }
    };

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());
        app.viewer = new SimpleViewer(engine, scene, view);
        app.materials = (app.materialSource == GENERATE_SHADERS) ?
                createMaterialGenerator(engine) : createUbershaderLoader(engine);
        app.loader = AssetLoader::create({engine, app.materials, app.names });
        if (filename.isEmpty()) {
            app.asset = app.loader->createAssetFromBinary(GLTF_VIEWER_DAMAGEDHELMET_DATA,
                    GLTF_VIEWER_DAMAGEDHELMET_SIZE);
        } else {
            loadAsset(filename);
        }

        loadResources(filename);

        app.viewer->setUiCallback([&app, scene] () {
            float progress = app.resourceLoader->asyncGetLoadProgress();
            if (progress < 1.0) {
                ImGui::ProgressBar(progress);
            }
            if (ImGui::CollapsingHeader("Stats")) {
                ImGui::Text("%zu entities in the asset", app.asset->getEntityCount());
                ImGui::Text("%zu renderables (excluding UI)", scene->getRenderableCount());
                ImGui::Text("%zu skipped frames", FilamentApp::get().getSkippedFrameCount());
            }
        });

        // Leave FXAA enabled but we also enable MSAA for a nice result. The wireframe looks
        // much better with MSAA enabled.
        view->setSampleCount(4);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        delete app.viewer;
        app.loader->destroyAsset(app.asset);
        app.materials->destroyMaterials();
        delete app.materials;
        AssetLoader::destroy(&app.loader);
        delete app.names;
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        app.resourceLoader->asyncUpdateLoad();

        // Add renderables to the scene as they become ready.
        app.viewer->populateScene(app.asset, !app.actualSize);

        app.viewer->applyAnimation(now);
    };

    auto gui = [&app](filament::Engine* engine, filament::View* view) {
        app.viewer->updateUserInterface();
        FilamentApp::get().setSidebarWidth(app.viewer->getSidebarWidth());
    };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.animate(animate);

    filamentApp.setDropHandler([&] (std::string path) {
        app.viewer->removeAsset();
        app.loader->destroyAsset(app.asset);
        loadAsset(path);
        loadResources(path);
    });

    filamentApp.run(app.config, setup, cleanup, gui);

    return 0;
}
