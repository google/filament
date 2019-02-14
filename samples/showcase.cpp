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

// TODO: trim this list of includes

#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <filameshio/MeshReader.h>

#include <gltfio/AnimationHelper.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/BindingHelper.h>

#include "app/Config.h"
#include "app/FilamentApp.h"
#include "app/IBL.h"

#include <getopt/getopt.h>

#include <fstream>
#include <iomanip>
#include <iostream>

#include "generated/resources/resources.h"
#include "generated/resources/textures.h"

using namespace filament;
using namespace filamesh;
using namespace gltfio;
using namespace math;
using namespace utils;

struct App {
    Config config;
    AssetLoader* loader;
    FilamentAsset* asset;
    AnimationHelper* animation;
    bool shadowPlane = false;
};

static const char* DEFAULT_IBL = "envs/venetian_crossroads";

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "SHOWCASE renders the specified glTF file, or a built-in file if none is specified\n"
            "Usage:\n"
            "    SHOWCASE [options] <gltf file>\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl (default) or vulkan\n\n"
            "   --ibl=<path to cmgen IBL>, -i <path>\n"
            "       Override the built-in IBL\n\n"
            "   --shadow-plane, -p\n"
            "       Enable shadow plane\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArgments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:pi:";
    static const struct option OPTIONS[] = {
            { "help",       no_argument,       nullptr, 'h' },
            { "api",        required_argument, nullptr, 'a' },
            { "ibl",        required_argument, nullptr, 'i' },
            { "shadow-plane", no_argument,     nullptr, 'p' },
            { nullptr, 0, nullptr, 0 }  // termination of the option list
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
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'." << std::endl;
                }
                break;
            case 'i':
                app->config.iblDirectory = arg;
                break;
            case 'p':
                app->shadowPlane = true;
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

    app.config.title = "showcase";
    app.config.iblDirectory = FilamentApp::getRootPath() + DEFAULT_IBL;

    int option_index = handleCommandLineArgments(argc, argv, &app);
    utils::Path filename;
    int num_args = argc - option_index;
    if (num_args >= 1) {
        filename = argv[option_index];
        if (!filename.exists()) {
            std::cerr << "file " << argv[option_index] << " not found!" << std::endl;
            return 1;
        }
    }

    auto setup = [&app, filename](Engine* engine, View* view, Scene* scene) {
        FilamentApp::get().getIBL()->getIndirectLight()->setIntensity(100000);
        app.loader = AssetLoader::create(engine);
        if (filename.isEmpty()) {
            return;
        }
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
        app.asset = app.loader->createAssetFromJson(buffer.data(), buffer.size());
        if (!app.asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }
        buffer.clear();
        buffer.shrink_to_fit();

        // Compute the scale required to fit the model's bounding box into [-1, +1]
        std::cout << "\nAsset min: " << app.asset->getBoundingBox().min << std::endl;
        std::cout << "Asset max: " << app.asset->getBoundingBox().max << std::endl << std::endl;
        auto minpt = app.asset->getBoundingBox().min;
        auto maxpt = app.asset->getBoundingBox().max;
        float maxExtent = 0;
        maxExtent = std::max(maxpt.x - minpt.x, maxpt.y - minpt.y);
        maxExtent = std::max(maxExtent, maxpt.z - minpt.z);
        float scaleFactor = 2.0f / maxExtent;
        float3 center = (minpt + maxpt) / 2.0f;
        center.z += 4.0f / scaleFactor;
        auto& tcm = engine->getTransformManager();
        auto root = tcm.getInstance(app.asset->getRoot());
        tcm.setTransform(root, mat4f::scale(float3(scaleFactor)) * mat4f::translate(-center));

        // Load external textures and buffers.
        utils::Path assetFolder = filename.getParent();
        gltfio::BindingHelper(engine, assetFolder.c_str()).loadResources(app.asset);

        // Load animation data.
        app.animation = new AnimationHelper(app.asset);
        app.asset->releaseSourceData();

        // Add renderables and lights. This also adds transform-only entities that get ignored.
        scene->addEntities(app.asset->getEntities(), app.asset->getEntityCount());
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        Fence::waitAndDestroy(engine->createFence());
        app.loader->destroyAsset(app.asset);
        app.loader->destroyMaterials();
        AssetLoader::destroy(&app.loader);
    };

    auto animate = [&app](Engine* engine, View*, double now) {
        if (app.animation->getAnimationCount() > 0) {
            app.animation->applyAnimation(0, now);
        }
    };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.run(app.config, setup, cleanup);
    filamentApp.animate(animate);

    return 0;
}
