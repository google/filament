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
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable;
};

struct Vertex {
    filament::math::float3 position;
    filament::math::float3 normal;
};
std::vector<Vertex> vertices;
std::vector<uint16_t>indices;

void CreateSphere(float radius) {
    constexpr float PI = 3.141592654;
    constexpr float PI_2 = 3.141592654 / 2;

    // default LOD = 100x100 mesh grid size
    unsigned int n_rows = 100;
    unsigned int n_cols = 100;
    unsigned int n_verts = (n_rows + 1) * (n_cols + 1);
    unsigned int n_tris = n_rows * n_cols * 2;


    vertices.reserve(n_verts);
    indices.reserve(n_tris * 3);

    for (unsigned int col = 0; col <= n_cols; ++col) {
        for (unsigned int row = 0; row <= n_rows; ++row) {
            // unscaled uv coordinates ~ [0, 1]
            float u = static_cast<float>(col) / n_cols;
            float v = static_cast<float>(row) / n_rows;

            float theta = PI * v - PI_2;  // ~ [-PI/2, PI/2], latitude from south to north pole
            float phi = PI * 2 * u;       // ~ [0, 2PI], longitude around the equator circle

            float x = cos(phi) * cos(theta);
            float y = sin(theta);
            float z = sin(phi) * cos(theta) * (-1);

            // for a unit sphere centered at the origin, normal = position
            // binormal is normal rotated by 90 degrees along the latitude (+theta)
            theta += PI_2;
            float r = cos(phi) * cos(theta);
            float s = sin(theta);
            float t = sin(phi) * cos(theta) * (-1);

            Vertex vertex{};
            vertex.position = filament::math::float3(x, y, z) * radius;
            vertex.normal = filament::math::float3(x, y, z);
            vertices.push_back(vertex);
        }
    }

    for (unsigned int col = 0; col < n_cols; ++col) {
        for (unsigned int row = 0; row < n_rows; ++row) {
            auto index = col * (n_rows + 1);

            // counter-clockwise winding order
            indices.push_back(index + row + 1);
            indices.push_back(index + row);
            indices.push_back(index + row + 1 + n_rows);

            // counter-clockwise winding order
            indices.push_back(index + row + 1 + n_rows + 1);
            indices.push_back(index + row + 1);
            indices.push_back(index + row + 1 + n_rows);
        }
    }


}


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
            }
            else if (arg == "vulkan") {
                app->config.backend = Engine::Backend::VULKAN;
            }
            else if (arg == "metal") {
                app->config.backend = Engine::Backend::METAL;
            }
            else {
                std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'.\n";
                exit(1);
            }
            break;
        }
    }
    return optind;
}

int main(int argc, char** argv) {
    App app{};
    app.config.title = "hellosphere";
    app.config.featureLevel = backend::FeatureLevel::FEATURE_LEVEL_0;
    handleCommandLineArguments(argc, argv, &app);
    CreateSphere(1.0);
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*engine);
        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);

        app.vb = VertexBuffer::Builder()
            .vertexCount(vertices.size())
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, 24)
            .attribute(VertexAttribute::CUSTOM0, 0, VertexBuffer::AttributeType::FLOAT3, 12, 24)
            .build(*engine);
        app.vb->setBufferAt(*engine, 0,
            VertexBuffer::BufferDescriptor(vertices.data(), vertices.size() * 24, nullptr));
        app.ib = IndexBuffer::Builder()
            .indexCount(indices.size())
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*engine);
        app.ib->setBuffer(*engine,
            IndexBuffer::BufferDescriptor(indices.data(), indices.size() * 2, nullptr));
        app.mat = Material::Builder()
            .package(RESOURCES_NORMALCOLOR_DATA, RESOURCES_NORMALCOLOR_SIZE)
            .build(*engine);
        app.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .material(0, app.mat->getDefaultInstance())
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, indices.size())
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
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
        engine->destroyCameraComponent(app.camera);
        utils::EntityManager::get().destroy(app.camera);
        };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        constexpr float ZOOM = 1.5f;
        const uint32_t w = view->getViewport().width;
        const uint32_t h = view->getViewport().height;
        const float aspect = (float)w / h;
        app.cam->setProjection(Camera::Projection::ORTHO,
            -aspect * ZOOM, aspect * ZOOM,
            -ZOOM, ZOOM, 0, 1);
        auto& tcm = engine->getTransformManager();
        tcm.setTransform(tcm.getInstance(app.renderable),
            filament::math::mat4f::rotation(now, filament::math::float3{ 0, 0, 1 }));
        });

    FilamentApp::get().run(app.config, setup, cleanup);

    return 0;
}
