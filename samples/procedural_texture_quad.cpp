/*
 * Copyright (C) 2026 The Android Open Source Project
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
#include "common/SampleConfig.h"

#include "generated/resources/resources.h"

#include <filamentapp/FilamentApp2.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/getopt.h>
#include <utils/Path.h>

#include <stb_image.h>

#include <iostream>
#include <string>

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

struct App {
    std::unique_ptr<FilamentApp2> filamentApp;
    VertexBuffer* vb = nullptr;
    Material* mat = nullptr;
    MaterialInstance* matInstance = nullptr;
    Texture* tex = nullptr;
    Skybox* skybox = nullptr;
    Entity renderable;
    Entity camera;
    Camera* cam = nullptr;
};

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
            "PROCEDURAL_TEXTURE_QUAD renders a textured quad whose geometry is generated\n"
            "entirely in the vertex shader using getVertexIndex(); no vertex or index buffer.\n"
            "Usage:\n"
            "    PROCEDURAL_TEXTURE_QUAD [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "API_USAGE"
    );
    const std::string from("PROCEDURAL_TEXTURE_QUAD");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos; pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), samples::getBackendAPIArgumentsUsage());
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], SampleConfig& config) {
    static constexpr const char* OPTSTR = "ha:";
    static const utils::getopt::option OPTIONS[] = {
        { "help", utils::getopt::no_argument,       nullptr, 'h' },
        { "api",  utils::getopt::required_argument, nullptr, 'a' },
        { nullptr, 0,                               nullptr, 0 }
    };
    int opt;
    int option_index = 0;
    while ((opt = utils::getopt::getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(utils::getopt::optarg ? utils::getopt::optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                config.backend = samples::parseArgumentsForBackend(arg);
                break;
        }
    }
    return utils::getopt::optind;
}

int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "procedural_texture_quad";
    handleCommandLineArguments(argc, argv, config);

    App app;
    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        Path path = FilamentApp2::getRootAssetsPath() + "textures/Moss_01/Moss_01_Color.png";
        if (!path.exists()) {
            std::cerr << "The texture " << path << " does not exist" << std::endl;
            exit(1);
        }
        int w, h, n;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 4);
        if (data == nullptr) {
            std::cerr << "The texture " << path << " could not be loaded" << std::endl;
            exit(1);
        }
        std::cout << "Loaded texture: " << w << "x" << h << std::endl;
        Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 4),
                Texture::Format::RGBA, Texture::Type::UBYTE,
                (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
        app.tex = Texture::Builder()
                .width(uint32_t(w))
                .height(uint32_t(h))
                .levels(1)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(Texture::InternalFormat::RGBA8)
                .build(*engine);
        app.tex->setImage(*engine, 0, std::move(buffer));

        app.mat = Material::Builder()
                .package(RESOURCES_PROCEDURALTEXTUREQUAD_DATA, RESOURCES_PROCEDURALTEXTUREQUAD_SIZE)
                .build(*engine);
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
        app.matInstance = app.mat->createInstance();
        app.matInstance->setParameter("albedo", app.tex, sampler);

        // Attribute-less VertexBuffer: no attributes, no buffer slots.
        // The vertex count tells the draw call how many vertices to emit; the
        // vertex shader generates positions and UVs from getVertexIndex().
        app.vb = VertexBuffer::Builder()
                .vertexCount(6)
                .bufferCount(0)
                .build(*engine);

        app.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -0.5f, -0.5f, -0.01f }, { 0.5f, 0.5f, 0.01f }})
                // Use the non-indexed geometry overload that omits the IndexBuffer parameter
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb)
                .material(0, app.matInstance)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, app.renderable);
        scene->addEntity(app.renderable);

        app.skybox = Skybox::Builder().color({0.1f, 0.125f, 0.25f, 1.0f}).build(*engine);
        scene->setSkybox(app.skybox);
        view->setPostProcessingEnabled(false);

        app.camera = EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        view->setCamera(app.cam);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.skybox);
        engine->destroy(app.renderable);
        engine->destroy(app.matInstance);
        engine->destroy(app.mat);
        engine->destroy(app.tex);
        engine->destroy(app.vb);
        engine->destroyCameraComponent(app.camera);
        EntityManager::get().destroy(app.camera);
    };


    app.filamentApp = FilamentApp2::Builder()
                              .title(config.title)
                              .backend(config.backend)
                              .setup(setup)
                              .cleanup(cleanup)
                              .animation([&app](Engine*, View* view, double) {
                                  const uint32_t w = view->getViewport().width;
                                  const uint32_t h = view->getViewport().height;
                                  const float aspect = float(w) / float(h);
                                  app.cam->setProjection(Camera::Projection::ORTHO, -aspect, aspect,
                                          -1.0f, 1.0f, 0.0f, 1.0f);
                              })
                              .build();
    app.filamentApp->run();
    return 0;
}
