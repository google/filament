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

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <cmath>
#include <iostream>

#include <getopt/getopt.h>

#include "generated/resources/resources.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using namespace filament::math;

struct App {
    VertexBuffer* vb;
    VertexBuffer* vb2;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable;
};

struct VertexWithBones {
    float2 position;
    uint32_t color;
    filament::math::ushort4 joints;
    filament::math::float4 weighs;
};

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "SAMPLE is a command-line tool for testing Filament skinning.\n"
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


static const VertexWithBones TRIANGLE_VERTICES_WITHBONES[6] = {
    {{1, 0}, 0xffff0000u, {0,1,0,0}, {1.0f,0.f,0.f,0.f}},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u, {0,1,0,0}, {0.f,1.f,0.f,0.f}},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu,{0,1,0,0}, {0.5f,0.5f,0.f,0.f}},
    {{1, -1}, 0xffffff00u, {0,2,0,0}, {0.0f,1.f,0.f,0.f}},
    {{-cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ffffu, {0,1,0,0}, {0.f,1.f,0.f,0.f}},
    {{-cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xffff00ffu,{0,1,0,0}, {0.f,0.f,0.5f,0.5f}},
};

static constexpr uint16_t TRIANGLE_INDICES[6] = { 0, 1, 2, 3};

mat4f transforms[] = {mat4f(1),
                      mat4f::translation(float3(1, 0, 0)),
                      mat4f::translation(float3(1, 1, 0)),
                      mat4f::translation(float3(0, 1, 0))};

int main(int argc, char** argv) {
    Config config;
    config.title = "hello skinning";
    config.vulkanGPUHint = "0";
    config.backend = filament::Engine::Backend::OPENGL;

    handleCommandLineArgments(argc, argv, &config);

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);

        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);
        static_assert(sizeof(VertexWithBones) == 36, "Strange vertex size.");
        app.vb = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 36)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 36)
                .normalized(VertexAttribute::COLOR)
                .attribute(VertexAttribute::BONE_INDICES, 0, VertexBuffer::AttributeType::USHORT4, 12, 36)
                .attribute(VertexAttribute::BONE_WEIGHTS, 0, VertexBuffer::AttributeType::FLOAT4, 20, 36)
                .build(*engine);
        app.vb2 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 36)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 36)
                .normalized(VertexAttribute::COLOR)
                .attribute(VertexAttribute::BONE_INDICES, 0, VertexBuffer::AttributeType::USHORT4, 12, 36)
                .attribute(VertexAttribute::BONE_WEIGHTS, 0, VertexBuffer::AttributeType::FLOAT4, 20, 36)
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES_WITHBONES, 154, nullptr));
        app.vb2->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES_WITHBONES + 3, 108, nullptr));
        app.ib = IndexBuffer::Builder()
                .indexCount(4)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 8, nullptr));
        app.mat = Material::Builder()
                .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
                .build(*engine);

        app.renderable = EntityManager::get().create();

        RenderableManager::Builder(2)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.mat->getDefaultInstance())
                .material(1, app.mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLE_STRIP, app.vb, app.ib, 0, 4)
                .geometry(1, RenderableManager::PrimitiveType::TRIANGLES, app.vb2, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .skinning(4, transforms)
                .enableSkinningBuffers(false)
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
        engine->destroy(app.vb2);
        engine->destroy(app.ib);
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

        // Bone skinning animation
        float tr = (float)(sin(now));
        mat4f trans[] = {filament::math::mat4f::translation(filament::math::float3{tr, 0, 0}),
                         filament::math::mat4f::translation(filament::math::float3{-1, tr, 0}),
                         filament::math::mat4f(1.f)};
        rm.setBones(rm.getInstance(app.renderable), trans, 3, 0);


    });

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
