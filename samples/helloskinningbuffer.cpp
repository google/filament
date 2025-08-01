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

#include "common/arguments.h"

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
#include <getopt/getopt.h>

#include <cmath>
#include <iostream>

#include "generated/resources/resources.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using namespace filament::math;

struct App {
    VertexBuffer *vb, *vb2;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable1;
    Entity renderable2;
    SkinningBuffer *sb;
};

struct Vertex {
    float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[3] = {
    {{1, 0}, 0xffff0000u},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static const uint16_t skinJoints[] = { 0, 1, 2, 3,
                                     0, 1, 2, 3,
                                     0, 1, 2, 3
};

static const float skinWeights[] = { 0.25f, 0.25f, 0.25f, 0.25f,
                                     0.25f, 0.25f, 0.25f, 0.25f,
                                     0.25f, 0.25f, 0.25f, 0.25f
};

static constexpr uint16_t TRIANGLE_INDICES[] = { 0, 1, 2, 3 };

mat4f transforms[] = {math::mat4f(1.f),
                      mat4f::translation(float3(1, 0, 0)),
                      mat4f::translation(float3(1, 1, 0)),
                      mat4f::translation(float3(0, 1, 0)),
                      mat4f::translation(float3(-1, 1, 0)),
                      mat4f::translation(float3(-1, 0, 0)),
                      mat4f::translation(float3(-1, -1, 0)),
                      mat4f::translation(float3(0, -1, 0)),
                      mat4f::translation(float3(1, -1, 0))};

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "SAMPLE is a command-line tool for testing Filament skinning buffers.\n"
            "Usage:\n"
            "    SAMPLE [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "API_USAGE"
    );
    const std::string from("SAMPLE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos; pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), samples::getBackendAPIArgumentsUsage());
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
                config->backend = samples::parseArgumentsForBackend(arg);
                break;
        }
    }

    return optind;
}

int main(int argc, char** argv) {
    Config config;
    config.title = "skinning buffer common for two renderables";

    handleCommandLineArgments(argc, argv, &config);

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);

        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);
        static_assert(sizeof(Vertex) == 12, "Strange vertex size.");
        app.vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(3)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .attribute(VertexAttribute::BONE_INDICES, 1, VertexBuffer::AttributeType::USHORT4, 0, 8)
                .attribute(VertexAttribute::BONE_WEIGHTS, 2, VertexBuffer::AttributeType::FLOAT4, 0, 16)
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
        app.vb->setBufferAt(*engine, 1,
                 VertexBuffer::BufferDescriptor(skinJoints, 24, nullptr));
        app.vb->setBufferAt(*engine, 2,
                VertexBuffer::BufferDescriptor(skinWeights, 48, nullptr));

        app.vb2 = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                .normalized(VertexAttribute::COLOR)
                .build(*engine);
        app.vb2->setBufferAt(*engine, 0,
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

        app.sb = SkinningBuffer::Builder()
            .boneCount(9)
            .initialize()
            .build(*engine);
        app.sb->setBones(*engine, transforms,9,0);

        app.renderable1 = EntityManager::get().create();
        app.renderable2 = EntityManager::get().create();

        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .enableSkinningBuffers(true)
                .skinning(app.sb, 9, 0)
                .build(*engine, app.renderable1);

        RenderableManager::Builder(2)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.mat->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .geometry(1, RenderableManager::PrimitiveType::TRIANGLES, app.vb2, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .enableSkinningBuffers(true)
                .skinning(app.sb, 9, 0)
                .build(*engine, app.renderable2);

        scene->addEntity(app.renderable1);
        scene->addEntity(app.renderable2);
        app.camera = utils::EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable1);
        engine->destroy(app.renderable2);
        engine->destroy(app.mat);
        engine->destroy(app.vb);
        engine->destroy(app.vb2);
        engine->destroy(app.ib);
        engine->destroy(app.sb);
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
        auto& tcm = engine->getTransformManager();

        // Transformation of both renderables
        tcm.setTransform(tcm.getInstance(app.renderable1),
                filament::math::mat4f::translation(filament::math::float3{ 0.5, 0, 0 }));
        tcm.setTransform(tcm.getInstance(app.renderable2),
                filament::math::mat4f::translation(filament::math::float3{ 0, 0.5, 0 }));

        auto& rm = engine->getRenderableManager();

        // Bone skinning animation
        float t = (float)(now - (int)now);
        float s = sin(t * f::PI * 2.f);
        float c = cos(t * f::PI * 2.f);

        mat4f translate[] = {mat4f::translation(float3(s, c, 0))};

        mat4f trans1of8[9] = {};
        for (size_t i = 0; i < 9; i++) {
            trans1of8[i] = filament::math::mat4f(1);
        }
        s *= 5;
        mat4f transA[] = {
            mat4f::translation(float3(s, 0, 0)),
            mat4f::translation(float3(s, s, 0)),
            mat4f::translation(float3(0, s, 0)),
            mat4f::translation(float3(-s, s, 0)),
            mat4f::translation(float3(-s, 0, 0)),
            mat4f::translation(float3(-s, -s, 0)),
            mat4f::translation(float3(0, -s, 0)),
            mat4f::translation(float3(s, -s, 0)),
            filament::math::mat4f(1)};
        size_t offset = ((size_t)now) % 8;
        trans1of8[offset] = transA[offset];

        // Set transformation of the first bone
        app.sb->setBones(*engine, translate, 1, 0);

        // Set transformation of the other bones, only 3 of them can be used, do to limitation
        app.sb->setBones(*engine,trans1of8, 8, 1);
    });

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
