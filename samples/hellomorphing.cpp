/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/SkinningBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <cmath>
#include <iostream>

#include "generated/resources/resources.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using namespace filament::math;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable;
    MorphTargetBuffer *mt1;
};

struct Vertex {
    float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[3] = {
    {{1, 0}, 0xffff0000u}, // blue one (ABGR)
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u}, // green one
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu}, // red one
};

static const float3 targets_pos1[9] = {
    {-2, 0, 0},{0, 2, 0},{1, 0, 0}, // 1st position for 1st, 2nd and 3rd point of the first primitive
    {1, 1, 0},{-1, 0, 0},{-1, 0, 0}, // 2nd ...
    {0, 0, 0},{0, 0, 0},{0, 0, 0} // no position change
};

static const float3 targets_pos2[9] = {
    {0, 2, 0},{-2, 0, 0},{1, 0, 0}, // 1st position for 1st, 2nd and 3rd point of the second primitive
    {-1, 0, 0},{1, 1, 0},{-1, 0, 0}, // position of th 3rd point is same for both morph targets
    {0, 0, 0},{0, 0, 0}, {0, 0, 0}
};

static const short4 targets_tan[9] = {
  {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},
  {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},
  {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "SAMPLE is a command-line tool for testing Filament skinning buffers.\n"
            "Usage:\n"
            "    SAMPLE [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
    );
    const std::string from("SAMPLE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArgments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
            { "help",         no_argument,       nullptr, 'h' },
            { "api",          required_argument, nullptr, 'a' },
            { nullptr, 0, nullptr, 0 }  // termination of the option list
    };
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(optarg != nullptr ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                if (arg == "opengl") {
                    config->backend = Engine::Backend::OPENGL;
                } else if (arg == "vulkan") {
                    config->backend = Engine::Backend::VULKAN;
                } else if (arg == "metal") {
                    config->backend = Engine::Backend::METAL;
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'."
                              << std::endl;
                }
                break;
        }
    }

    return optind;
}

int main(int argc, char** argv) {
    Config config;
    config.title = "helloMorphing";

    handleCommandLineArgments(argc, argv, &config);

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);

        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);
        static_assert(sizeof(Vertex) == 12, "Strange vertex size.");
        app.vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
        app.ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        app.mat = Material::Builder()
                .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
                .build(*engine);

        app.mt1 = MorphTargetBuffer::Builder()
            .vertexCount(9 * 2)
            .count(3)
            .build(*engine);

        app.mt1->setPositionsAt(*engine,0, targets_pos1, 3, 0);
        app.mt1->setPositionsAt(*engine,1, targets_pos1+3, 3, 0);
        app.mt1->setPositionsAt(*engine,2, targets_pos1+6, 3, 0);
        app.mt1->setTangentsAt(*engine,0, targets_tan, 3, 0);
        app.mt1->setTangentsAt(*engine,1, targets_tan+3, 3, 0);
        app.mt1->setTangentsAt(*engine,2, targets_tan+6, 3, 0);

        app.mt1->setPositionsAt(*engine,0, targets_pos2, 3, 9);
        app.mt1->setPositionsAt(*engine,1, targets_pos2+3, 3, 9);
        app.mt1->setPositionsAt(*engine,2, targets_pos2+6, 3, 9);
        app.mt1->setTangentsAt(*engine,0, targets_tan, 3, 9);
        app.mt1->setTangentsAt(*engine,1, targets_tan+3, 3, 9);
        app.mt1->setTangentsAt(*engine,2, targets_tan+6, 3, 9);

        app.renderable = EntityManager::get().create();

        RenderableManager::Builder(2)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.mat->getDefaultInstance())
                .material(1, app.mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .geometry(1, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .morphing(app.mt1)
                .morphing(0, 0, 0)
                .morphing(0, 1, 9)
                .build(*engine, app.renderable);

        scene->addEntity(app.renderable);
        app.camera = utils::EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable);
        engine->destroy(app.mat);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroy(app.mt1);
        engine->destroyCameraComponent(app.camera);
        utils::EntityManager::get().destroy(app.camera);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        constexpr float ZOOM = 1.5f;
        const uint32_t w = view->getViewport().width;
        const uint32_t h = view->getViewport().height;
        const float aspect = (float) w / h;
        app.cam->setProjection(Camera::Projection::ORTHO,
            -aspect * ZOOM, aspect * ZOOM,
            -ZOOM, ZOOM, 0, 1);

        auto& rm = engine->getRenderableManager();
        // morphTarget/blendshapes animation defined for all primitives
        float z = (float)(sin(now)/2.f + 0.5f);
        float weights[] = {1 - z, z/2, z/2};
        // set global weights of all morph targets
        rm.setMorphWeights(rm.getInstance(app.renderable), weights, 3, 0);
    });

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
