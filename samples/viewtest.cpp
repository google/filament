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

#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Renderer.h>

#include <filamentapp/FilamentApp.h>

#include <utils/EntityManager.h>

#include <getopt/getopt.h>

#include <iostream>
#include <string>

using namespace filament;

struct App {
    Config config;
    VertexBuffer* vb;
    IndexBuffer* ib;
    utils::Entity camera;
    Camera* cam;
    Skybox* skybox;
    utils::Entity renderable;
};

static const filament::math::float2 TRIANGLE_VERTICES[3] = { {1, 0}, {-0.5, 0.866}, {-0.5, -0.866} };
static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
            "SHOWCASE renders the specified glTF file, or a built-in file if none is specified\n"
            "Usage:\n"
            "    SHOWCASE [options] <gltf path>\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:f:i:usc:rt:b:ev";
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
                }
                break;
        }
    }
    return optind;
}

int main(int argc, char** argv) {
    App app;

    app.config.title = "viewtest";

    handleCommandLineArguments(argc, argv, &app);

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({ 0, 0, 1, 1 }).build(*engine);
        scene->setSkybox(app.skybox);
        view->setViewport({100, 100, 512, 512});
        app.vb = VertexBuffer::Builder()
                .vertexCount(3).bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 8)
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 24, nullptr));
        app.ib = IndexBuffer::Builder()
                .indexCount(3).bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
        app.renderable = utils::EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
                .build(*engine, app.renderable);
        scene->addEntity(app.renderable);
        app.camera = utils::EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable);
        engine->destroy(app.vb);
        engine->destroy(app.ib);

        engine->destroyCameraComponent(app.camera);
        utils::EntityManager::get().destroy(app.camera);
    };

    auto preRender = [](Engine*, View*, Scene*, Renderer* renderer) {
        renderer->setClearOptions({ .clear = true });
    };

    FilamentApp::get().run(app.config, setup, cleanup, {}, preRender);

    return 0;
}
