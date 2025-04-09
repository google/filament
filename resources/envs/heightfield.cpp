/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/JobSystem.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

#include <filamentapp/FilamentApp.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include <math/mat4.h>
#include <math/norm.h>

#include <cmath>
#include <cstdint>
#include <iostream>

#include <imgui.h>

#include "generated/resources/resources.h"

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

struct App {
    Skybox* skybox = nullptr;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    Material* mat = nullptr;
    MaterialInstance* matInstance = nullptr;

    Texture* r8Tex = nullptr;
    Texture* floatTex = nullptr;
    Texture* rgbTex = nullptr;

    Entity renderable;
};

struct Vertex {
    filament::math::float3 position;
    filament::math::float2 uv;
};

enum NoiseType {
    PERLIN = 0,
    RIDGE,
    FBM,
    TURBULENCE
};

struct Params {
    float lacunarity = 2.0f;
    float gain = 0.5f;
    int octaves = 2;
    float speed = 1.0f;
    int noiseType = 0;
    bool updateSubRegion = false;
    int textureType = 0;
    int currentTextureType = -1;
    bool addPadding = false;
};
static Params g_params;

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
            "HEIGHTFIELD is a command-line tool for testing Filament texture updates.\n"
            "Usage:\n"
            "    HEIGHTFIELD [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
    );
    const std::string from("HEIGHTFIELD");
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

template<typename T>
T packFloat(float f);

template <>
float packFloat(float f) { return f; }

template <>
uint8_t packFloat(float f) { return packUnorm8(f); }

template <>
filament::math::byte3 packFloat(float f) { return filament::math::byte3 {packUnorm8(f), 0, 0}; }

template<typename T>
void populateTextureWithPerlin(Texture* texture, Engine& engine, float time, Params& params,
        size_t xoffset, size_t yoffset, size_t dimension, size_t bufferPadding) {
    using namespace utils;

    // The bufferPadding parameter adds some padding to the left and top of the pixel buffer. This
    // tests that the backend respects PixelBufferDescriptor's left and top parameters.

    assert(bufferPadding < dimension);
    const size_t dimensionWithPadding = dimension + bufferPadding;
    const size_t imageBufferSize = dimensionWithPadding * dimensionWithPadding * sizeof(T);
    T* imageData = (T*) malloc(imageBufferSize);

    JobSystem* js = &engine.getJobSystem();

    typedef float (*NoiseFunc)(float, float, float, float, float, int);

    NoiseFunc noiseGen = nullptr;
    switch (params.noiseType) {
        case NoiseType::PERLIN:
              noiseGen = [](float x, float y, float z, float, float, int) {
                return stb_perlin_noise3(x, y, z, 0, 0, 0);
              };
              break;
        case NoiseType::RIDGE:
              noiseGen = [](float x, float y, float z, float lacunarity, float gain, int octaves) {
                return stb_perlin_ridge_noise3(x, y, z, lacunarity, gain, 1.0f, octaves);
              };
              break;
        case NoiseType::FBM:
              noiseGen = stb_perlin_fbm_noise3;
              break;
        case NoiseType::TURBULENCE:
              noiseGen = stb_perlin_turbulence_noise3;
              break;
    }

    auto work = [imageData, dimension, bufferPadding, time, params, noiseGen](uint32_t startPixel,
            uint32_t pixelCount) {
        for (uint32_t p = startPixel; p < startPixel + pixelCount; p++) {
            const size_t r = p / dimension;
            const size_t c = p % dimension;
            float x = (float) r / dimension;
            float y = (float) c / dimension;
            float noise = noiseGen(x, y, time, params.lacunarity, params.gain, params.octaves);
            const size_t pixelIndex = (bufferPadding + r) * (dimension + bufferPadding) +
                    (bufferPadding + c);
            imageData[pixelIndex] = packFloat<T>(noise);
        }
    };

    auto job = jobs::parallel_for(*js, nullptr, 0, dimension * dimension, std::cref(work),
            jobs::CountSplitter<64, 32>());
    js->runAndWait(job);

    Texture::PixelBufferDescriptor::PixelDataFormat format {};
    Texture::PixelBufferDescriptor::PixelDataType type {};

    if (texture->getFormat() == Texture::InternalFormat::R8) {
        format = Texture::Format::R;
        type = Texture::Type::UBYTE;
    } else if (texture->getFormat() == Texture::InternalFormat::R32F) {
        format = Texture::Format::R;
        type = Texture::Type::FLOAT;
    } else if (texture->getFormat() == Texture::InternalFormat::RGB8) {
        format = Texture::Format::RGB;
        type = Texture::Type::UBYTE;
    }

    Texture::PixelBufferDescriptor pixelBuffer(imageData, imageBufferSize,
            format, type, 1, bufferPadding, bufferPadding, dimensionWithPadding,
            [](void* buffer, size_t, void*) {
        free(buffer);
    });
    texture->setImage(engine, 0, xoffset, yoffset, dimension, dimension, std::move(pixelBuffer));
}

static void gui(Engine*, View*) {
    auto& params = g_params;
    ImGui::Begin("Parameters");
    {
        if (ImGui::CollapsingHeader("Noise", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Combo("Noise type", &g_params.noiseType,
                    "perlin\0ridge\0fbm\0turbulence\0\0");
            if (g_params.noiseType != NoiseType::PERLIN) {
                ImGui::SliderFloat("Lacunarity", &params.lacunarity, 0.0f, 5.0f);
                ImGui::SliderFloat("Gain", &params.gain, 0.0f, 1.0f);
                ImGui::SliderInt("Octaves", &params.octaves, 1, 10);
            }
        }
        ImGui::SliderFloat("Speed", &params.speed, 0.0f, 5.0f);
        ImGui::Checkbox("Update subregion", &params.updateSubRegion);
        ImGui::Checkbox("Add pixel buffer padding", &params.addPadding);
        ImGui::Combo("Texture format", &g_params.textureType,
                "R8\0R32F\0RGB8\0\0");
    }
    ImGui::End();
}

int main(int argc, char** argv) {
    Config config;
    config.title = "Heightfield";

    handleCommandLineArgments(argc, argv, &config);

    const size_t textureSize = 512;

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {

        // Create heightfield textures for each format.
        app.r8Tex = Texture::Builder()
                .width(uint32_t(textureSize))
                .height(uint32_t(textureSize))
                .levels(1)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(Texture::InternalFormat::R8)
                .build(*engine);
        app.floatTex = Texture::Builder()
                .width(uint32_t(textureSize))
                .height(uint32_t(textureSize))
                .levels(1)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(Texture::InternalFormat::R32F)
                .build(*engine);
        app.rgbTex = Texture::Builder()
                .width(uint32_t(textureSize))
                .height(uint32_t(textureSize))
                .levels(1)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(Texture::InternalFormat::RGB8)
                .build(*engine);

        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);

        // Set up view
        app.skybox = Skybox::Builder().color({0.03f, 0.04f, 0.36f, 1.0f}).build(*engine);
        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);

        // Generate heightfield vertices.
        constexpr size_t VERTICES_SIZE = 256;
        constexpr size_t vertexCount = VERTICES_SIZE * VERTICES_SIZE;
        static Vertex vertices[vertexCount] = {};
        Vertex* vertex = vertices;

        constexpr size_t indexCount = (VERTICES_SIZE - 1) * (VERTICES_SIZE - 1) * 2 * 3;
        static uint16_t indices[indexCount] = {};

        for (size_t r = 0; r < VERTICES_SIZE; r++) {
            for (size_t c = 0; c < VERTICES_SIZE; c++) {
                const float x = (float) c / (VERTICES_SIZE - 1) * 2.0f - 1.0f;
                const float z = (float) r / (VERTICES_SIZE - 1) * 2.0f - 1.0f;
                vertex->position = {x, 0.0f, z};
                vertex->uv = {(float) c / (VERTICES_SIZE - 1), (float) r / (VERTICES_SIZE - 1)};
                vertex++;
            }
        }

        // Generate heightfield indices.
        size_t i = 0;
        size_t q = 0;
        const size_t indicesPerRow = VERTICES_SIZE;
        for (size_t r = 0; r < VERTICES_SIZE - 1; r++) {
            for (size_t c = 0; c < VERTICES_SIZE - 1; c++) {
                indices[i + 0] = q;
                indices[i + 1] = q + 1;
                indices[i + 2] = q + indicesPerRow;

                indices[i + 3] = q + 1 + indicesPerRow;
                indices[i + 4] = q + indicesPerRow;
                indices[i + 5] = q + 1;

                i += 6;
                q++;
            }
            q++;
        }

        // Create heightfield renderable.
        static_assert(sizeof(Vertex) == 20, "Strange vertex size.");
        app.vb = VertexBuffer::Builder()
                .vertexCount(vertexCount)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0,
                        sizeof(Vertex))
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 12,
                        sizeof(Vertex))
                .build(*engine);
        app.vb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(vertices, sizeof(Vertex) * vertexCount, nullptr));

        app.ib = IndexBuffer::Builder()
                .indexCount(indexCount)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.ib->setBuffer(*engine,
                IndexBuffer::BufferDescriptor(indices, sizeof(uint16_t) * indexCount, nullptr));
        app.mat = Material::Builder()
                .package(RESOURCES_HEIGHTFIELD_DATA, RESOURCES_HEIGHTFIELD_SIZE)
                .build(*engine);
        app.matInstance = app.mat->createInstance();
        app.matInstance->setParameter("height", app.r8Tex, sampler);
        app.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, app.matInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0,
                        indexCount)
                .culling(false)
                .receiveShadows(true)
                .castShadows(true)
                .build(*engine, app.renderable);
        scene->addEntity(app.renderable);

        auto& tcm = engine->getTransformManager();
        auto ti = tcm.getInstance(app.renderable);
        tcm.setTransform(ti, mat4f{ mat3f(1.0f), float3(0.0f, 0.0f, -4.0f) });
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable);
        engine->destroy(app.matInstance);
        engine->destroy(app.mat);
        engine->destroy(app.r8Tex);
        engine->destroy(app.floatTex);
        engine->destroy(app.rgbTex);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        const size_t offset = g_params.updateSubRegion ? 64 : 0;
        const size_t dimension = g_params.updateSubRegion ? textureSize / 2 : textureSize;
        const size_t padding = g_params.addPadding ? 32 : 0;
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);

        Texture* textureUpdate = nullptr;

        if (g_params.textureType != g_params.currentTextureType) {
            if (g_params.textureType == 0) {
                textureUpdate = app.r8Tex;
            } else if (g_params.textureType == 1) {
                textureUpdate = app.floatTex;
            } else if (g_params.textureType == 2) {
                textureUpdate = app.rgbTex;
            }

            g_params.currentTextureType = g_params.textureType;
        }

        if (textureUpdate) {
            app.matInstance->setParameter("height", textureUpdate, sampler);
        }

        const auto n = static_cast<float>(now);
        if (g_params.textureType == 0) {
            populateTextureWithPerlin<uint8_t>(app.r8Tex, *engine, n * g_params.speed, g_params,
                    offset, offset, dimension, padding);
        } else if (g_params.textureType == 1) {
            populateTextureWithPerlin<float>(app.floatTex, *engine, n * g_params.speed, g_params,
                    offset, offset, dimension, padding);
        } else if (g_params.textureType == 2) {
            populateTextureWithPerlin<filament::math::byte3>(app.rgbTex, *engine,
                    n * g_params.speed, g_params, offset, offset, dimension, padding);
        }
    });

    FilamentApp::get().run(config, setup, cleanup, gui);

    return 0;
}
