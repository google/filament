/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include <filament/MaterialInstance.h>
#include <filament/MorphTargetBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <vector>
#include <iostream>

#include "generated/resources/resources.h"

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;
using utils::Path;

struct Vertex {
    float3 position;
    float2 uv;
};

struct App {
    Config config;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    Material* mat = nullptr;
    MaterialInstance* mi = nullptr;
    Texture* baseColorMap = nullptr;
    Texture* uvMorphTexture = nullptr;
    Camera* cam = nullptr;
    Entity camera;
    Skybox* skybox = nullptr;
    Entity renderable;
    MorphTargetBuffer* mtb = nullptr;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    std::vector<float2> uvMorphData;
};

static constexpr int GRID_SIZE = 20;
static constexpr float GRID_SCALE = 2.0f;

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
            "HELLOUVMORPHING renders a quad with uv morphing\n"
            "Usage:\n"
            "    HELLOUVMORPHING [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "API_USAGE"
    );
    const std::string from("HELLOUVMORPHING");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos; pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), samples::getBackendAPIArgumentsUsage());
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
                app->config.backend = samples::parseArgumentsForBackend(arg);
                break;
        }
    }
    return optind;
}

int main(int argc, char** argv) {
    App app{};
    app.config.title = "hellouvmorphing";
    handleCommandLineArguments(argc, argv, &app);

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);

        app.vertices.reserve(GRID_SIZE * GRID_SIZE);
        app.indices.reserve((GRID_SIZE - 1) * (GRID_SIZE - 1) * 6);

        // Grid vertices
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
                float u = (float) x / (GRID_SIZE - 1);
                float v = (float) y / (GRID_SIZE - 1);
                float px = (u * 2.0f - 1.0f) * GRID_SCALE;
                float py = (v * 2.0f - 1.0f) * GRID_SCALE;
                app.vertices.push_back({ { px, py, 0.0f }, { u, v } });
            }
        }

        // Grid triangles
        for (int y = 0; y < GRID_SIZE - 1; ++y) {
            for (int x = 0; x < GRID_SIZE - 1; ++x) {
                uint16_t tl = y * GRID_SIZE + x;
                uint16_t tr = tl + 1;
                uint16_t bl = (y + 1) * GRID_SIZE + x;
                uint16_t br = bl + 1;

                // CCW Triangles
                app.indices.push_back(tl);
                app.indices.push_back(tr);
                app.indices.push_back(bl);

                app.indices.push_back(tr);
                app.indices.push_back(br);
                app.indices.push_back(bl);
            }
        }

        app.vb = VertexBuffer::Builder()
                         .vertexCount(app.vertices.size())
                         .bufferCount(1)
                         .attribute(VertexAttribute::POSITION, 0,
                                 VertexBuffer::AttributeType::FLOAT3, 0, sizeof(Vertex))
                         .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
                                 sizeof(float3), sizeof(Vertex))
                         .build(*engine);

        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(
                        app.vertices.data(), app.vertices.size() * sizeof(Vertex),
                        [](void* buffer, size_t size, void* user) {}, nullptr));

        app.ib = IndexBuffer::Builder()
                         .indexCount(app.indices.size())
                         .bufferType(IndexBuffer::IndexType::USHORT)
                         .build(*engine);

        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(
                        app.indices.data(), app.indices.size() * sizeof(uint16_t),
                        [](void* buffer, size_t size, void* user) {}, nullptr));

        // Create a 4x1 "rainbow" texture with Mipmaps
        static const uint32_t rainbow[] = {0xff0000ff, 0xff00ff00, 0xffff0000, 0xff00ffff};
        app.baseColorMap = Texture::Builder()
                                   .width(4)
                                   .height(1)
                                   .levels(0xff) // Auto levels
                                   .format(Texture::InternalFormat::RGBA8)
                                   .sampler(Texture::Sampler::SAMPLER_2D)
                                   .build(*engine);

        Texture::PixelBufferDescriptor pbd(rainbow, sizeof(rainbow),
                Texture::Format::RGBA, Texture::Type::UBYTE);
        app.baseColorMap->setImage(*engine, 0, std::move(pbd));

        // Generate swirl deltas
        const float2 center = { 0.5f, 0.5f };
        app.uvMorphData.reserve(app.vertices.size());

        for (const auto& v: app.vertices) {
            float2 uv = v.uv;
            float2 toCenter = uv - center;
            float dist = length(toCenter);

            float angle = (1.0f - dist) * 3.14159f * 4.0f;
            float s = sin(angle);
            float c = cos(angle);

            float2 rotated;
            rotated.x = toCenter.x * c - toCenter.y * s;
            rotated.y = toCenter.x * s + toCenter.y * c;

            float2 newUV = rotated + center;
            app.uvMorphData.push_back(newUV - uv);
        }

        size_t width = app.vertices.size();
        size_t height = 1;

        app.uvMorphTexture = Texture::Builder()
                                     .width(width)
                                     .height(height)
                                     .depth(1)
                                     .levels(1)
                                     .format(Texture::InternalFormat::RG32F)
                                     .sampler(Texture::Sampler::SAMPLER_2D_ARRAY)
                                     .build(*engine);

        Texture::PixelBufferDescriptor uvDeltaDesc(
                app.uvMorphData.data(), app.uvMorphData.size() * sizeof(float2),
                Texture::Format::RG, Texture::Type::FLOAT,
                [](void* buffer, size_t size, void* user) {}, nullptr);

        app.uvMorphTexture->setImage(*engine, 0, 0, 0, 0, width, height, 1, std::move(uvDeltaDesc));

        app.mat = Material::Builder()
                .package(RESOURCES_UVMORPH_DATA, RESOURCES_UVMORPH_SIZE)
                .build(*engine);
        app.mi = app.mat->createInstance();

        TextureSampler colorSampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
        app.mi->setParameter("baseColor", app.baseColorMap, colorSampler);

        TextureSampler dataSampler(TextureSampler::MinFilter::NEAREST, TextureSampler::MagFilter::NEAREST);
        app.mi->setParameter("uv0_morph", app.uvMorphTexture, dataSampler);

        app.mtb = MorphTargetBuffer::Builder()
                          .vertexCount(app.vertices.size())
                          .count(1)
                          .withPositions(true)
                          .enableCustomMorphing(true)
                          .build(*engine);

        app.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({ { -GRID_SCALE, -GRID_SCALE, -1 }, { GRID_SCALE, GRID_SCALE, 1 } })
                .material(0, app.mi)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib)
                .culling(false)
                .morphing(app.mtb)
                .build(*engine, app.renderable);
        scene->addEntity(app.renderable);

        app.camera = utils::EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable);
        engine->destroy(app.mi);
        engine->destroy(app.mat);
        engine->destroy(app.baseColorMap);
        engine->destroy(app.uvMorphTexture);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
        engine->destroy(app.mtb);
        engine->destroyCameraComponent(app.camera);
        EntityManager::get().destroy(app.camera);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        constexpr float ZOOM = 2.5f;
        const uint32_t w = view->getViewport().width;
        const uint32_t h = view->getViewport().height;
        const float aspect = (float)w / h;
        app.cam->setProjection(Camera::Projection::ORTHO, -aspect * ZOOM, aspect * ZOOM, -ZOOM, ZOOM, 0, 1);

        auto& rm = engine->getRenderableManager();
        float weight = (sin(now * 2.0) + 1.0f) * 0.5f;
        rm.setMorphWeights(rm.getInstance(app.renderable), &weight, 1);
    });

    FilamentApp::get().run(app.config, setup, cleanup);

    return 0;
}
