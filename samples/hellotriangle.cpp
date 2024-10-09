/*
 * Copyright (C) 2018 The Android Open Source Project
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
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <getopt/getopt.h>

#include <cmath>
#include <iostream>


#include "generated/resources/resources.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;

struct App {
    Config config;
    VertexBuffer* vb;
    VertexBuffer* vb2;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable;
    Entity r2;
};

struct Vertex {
    filament::math::float3 position;
    uint32_t color;
};

float const z = 5;

static const Vertex TRIANGLE_VERTICES[3] = {
    {{1, 0, z}, 0xffff0000u},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3), z}, 0xff00ff00u},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3), z}, 0xff0000ffu},
};

static Vertex T2[3] = {
        TRIANGLE_VERTICES[0],
        TRIANGLE_VERTICES[1],
        TRIANGLE_VERTICES[2],
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
            "HELLOTRIANGLE renders a spinning colored triangle\n"
            "Usage:\n"
            "    HELLOTRIANGLE [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl, vulkan, or metal\n"
    );
    const std::string from("HELLOTRIANGLE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
            { "help", no_argument,       nullptr, 'h' },
            { "api",  required_argument, nullptr, 'a' },
            { nullptr, 0,                nullptr, 0 }
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
                    exit(1);
                }
                break;
        }
    }
    return optind;
}

int main(int argc, char** argv) {
    T2[0].position.z = z - 10;
    T2[1].position.z = z - 10;
    T2[2].position.z = z - 10;

    T2[0].color = 0xFF0000FF;
    T2[1].color = 0xFF0000FF;
    T2[2].color = 0xFF0000FF;

    App app{};
    app.config.title = "hellotriangle";
    app.config.featureLevel = backend::FeatureLevel::FEATURE_LEVEL_0;
    handleCommandLineArguments(argc, argv, &app);

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        view->setPostProcessingEnabled(false);
        view->setStencilBufferEnabled(true);

        
//        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
//        scene->setSkybox(app.skybox);
        static_assert(sizeof(Vertex) == 16, "Strange vertex size.");
        auto builder = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, 16)
                .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 12, 16)
                .normalized(VertexAttribute::COLOR);
        app.vb = builder.build(*engine);
        app.vb2 = builder.build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 48, nullptr));
        app.vb2->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(T2, 48, nullptr));
        app.ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        app.mat = Material::Builder()
                .package(RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
                .build(*engine);
        app.renderable = EntityManager::get().create();
        app.r2 = EntityManager::get().create();

        auto inst1 = app.mat->createInstance();
        inst1->setDepthWrite(false);        
        inst1->setDepthFunc(MaterialInstance::DepthFunc::A);        
        inst1->setDepthCulling(false);
        
        inst1->setStencilWrite(true);
        inst1->setStencilCompareFunction(MaterialInstance::StencilCompareFunc::E);
        inst1->setStencilOpDepthStencilPass(MaterialInstance::StencilOperation::REPLACE);
        inst1->setStencilReferenceValue(6);

        auto inst2 = app.mat->createInstance();
        inst2->setDepthWrite(false);
        inst2->setDepthFunc(MaterialInstance::DepthFunc::A);
        inst2->setDepthCulling(false);
        
        inst2->setStencilWrite(true);
        inst2->setStencilCompareFunction(MaterialInstance::StencilCompareFunc::L);
        inst2->setStencilOpDepthStencilPass(MaterialInstance::StencilOperation::REPLACE);
        inst2->setStencilReferenceValue(6);


        auto& renderableMan = engine->getRenderableManager();

        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, inst1)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app.renderable);


        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, inst2)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb2, app.ib, 0, 3)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app.r2);

        scene->addEntity(app.renderable);


        auto r1inst = renderableMan.getInstance(app.renderable);
        renderableMan.setPriority(r1inst, 7);

        scene->addEntity(app.r2);        
        auto r2inst = renderableMan.getInstance(app.r2);
        renderableMan.setPriority(r2inst, 6);

        app.camera = utils::EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
//        engine->destroy(app.skybox);
        engine->destroy(app.renderable);
        engine->destroy(app.mat);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroyCameraComponent(app.camera);
        utils::EntityManager::get().destroy(app.camera);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        constexpr float ZOOM = 1.5;
        const uint32_t w = view->getViewport().width;
        const uint32_t h = view->getViewport().height;
        const float aspect = (float) w / h;
        app.cam->setProjection(Camera::Projection::ORTHO,
            -aspect * ZOOM, aspect * ZOOM,
            -ZOOM, ZOOM, -100, 100);
        auto& tcm = engine->getTransformManager();
        tcm.setTransform(tcm.getInstance(app.renderable),
                filament::math::mat4f::rotation(now, filament::math::float3{ 0, 0, 1 }));
    });


    auto preRender = [](Engine*, View* view, Scene*, Renderer* renderer) {
        renderer->setClearOptions({ .clearStencil = 0u, .clear = true,  });
    };

    FilamentApp::get().run(app.config, setup, cleanup, {}, preRender);

    return 0;
}
