/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <string>
#include <map>
#include <vector>

#include <getopt/getopt.h>

#include <utils/Path.h>

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec4.h>

#include "app/Config.h"
#include "app/FilamentApp.h"
#include "app/MeshAssimp.h"

#include <filamat/MaterialBuilder.h>

#include <utils/EntityManager.h>

using namespace math;
using namespace filament;
using namespace filamat;
using namespace utils;

static std::vector<Path> g_filenames;

static std::map<std::string, MaterialInstance*> g_materialInstances;
std::unique_ptr<MeshAssimp> g_meshSet;
static const Material* g_material;
static Entity g_light;

static Config g_config;

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "SAMPLE_POSITION_OFFSET is an example of setting the position offset\n"
            "Usage:\n"
            "    SAMPLE_POSITION_OFFSET [options] <mesh files (.obj, .fbx, COLLADA)>\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --split-view, -v\n"
            "       Splits the window into 4 views\n\n"
            "   --scale=[number], -s [number]\n"
            "       Applies uniform scale\n\n"
    );
    const std::string from("SAMPLE_POSITION_OFFSET");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArgments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "hvs:";
    static const struct option OPTIONS[] = {
            { "help",             no_argument,       0, 'h' },
            { "split-view",       no_argument,       0, 'v' },
            { "scale",            required_argument, 0, 's' },
            { 0, 0, 0, 0 }  // termination of the option list
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
            case 's':
                try {
                    config->scale = std::stof(arg);
                } catch (std::invalid_argument& e) {
                    // keep scale of 1.0
                } catch (std::out_of_range& e) {
                    // keep scale of 1.0
                }
                break;
            case 'v':
                config->splitView = true;
                break;
        }
    }

    return optind;
}

static void cleanup(Engine* engine, View* view, Scene* scene) {
    for (auto material : g_materialInstances) {
        engine->destroy(material.second);
    }
    engine->destroy(g_material);
    g_meshSet.reset(nullptr);
    engine->destroy(g_light);
    EntityManager& em = EntityManager::get();
    em.destroy(g_light);
}

static void setup(Engine* engine, View* view, Scene* scene) {
    g_meshSet.reset(new MeshAssimp(*engine));

    Package pkg = MaterialBuilder()
            .name("PositionOffset")
            .set(Property::BASE_COLOR)
            .set(Property::ROUGHNESS)
            .material(R"SHADER(
                void material(inout MaterialInputs material) {
                    prepareMaterial(material);
                    material.baseColor = float4(0.8, 0.8, 0.8, 1.0);
                    material.roughness = 0.3;
                }
            )SHADER")
            .materialVertex(R"SHADER(
                void materialVertex(inout MaterialVertexInputs material) {
                    material.worldPosition.x += sin(getPosition().y * 3.0 + frameUniforms.time * 5.0) * 0.1;
                }
            )SHADER")
            .build();
    g_material = Material::Builder().package(pkg.getData(), pkg.getSize())
            .build(*engine);

    MaterialInstance* positionOffset = g_material->createInstance();
    g_materialInstances["PositionOffset"] = positionOffset;

    for (auto& filename : g_filenames) {
        g_meshSet->addFromFile(filename, g_materialInstances);
    }

    auto& tcm = engine->getTransformManager();
    auto ei = tcm.getInstance(g_meshSet->getRenderables()[0]);
    tcm.setTransform(ei, mat4f{ mat3f(g_config.scale), float3(0.0f, 0.0f, -4.0f) } *
            tcm.getWorldTransform(ei));

    auto& rcm = engine->getRenderableManager();
    for (auto renderable : g_meshSet->getRenderables()) {
        auto ri = rcm.getInstance(renderable);
        auto blendingMode = rcm.getMaterialInstanceAt(ri, 0)->getMaterial()->getBlendingMode();
        rcm.setCastShadows(ri,
                blendingMode == BlendingMode::OPAQUE || blendingMode == BlendingMode::MASKED);
        scene->addEntity(renderable);
    }

    g_light = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::DIRECTIONAL)
            .color(Color::toLinear<ACCURATE>({0.98f, 0.92f, 0.89f}))
            .intensity(110000)
            .direction({ 0.7, -1, -0.8 })
            .castShadows(true)
            .build(*engine, g_light);
    scene->addEntity(g_light);
}

int main(int argc, char* argv[]) {
    int option_index = handleCommandLineArgments(argc, argv, &g_config);
    int num_args = argc - option_index;
    if (num_args < 1) {
        printUsage(argv[0]);
        return 1;
    }

    for (int i = option_index; i < argc; i++) {
        utils::Path filename = argv[i];
        if (!filename.exists()) {
            std::cerr << "file " << argv[option_index] << " not found!" << std::endl;
            return 1;
        }
        g_filenames.push_back(filename);
    }

    g_config.title = "Mesh Overdraw";
    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.run(g_config, setup, cleanup);

    return 0;
}
