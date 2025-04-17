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

#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>

#include <filameshio/MeshReader.h>

#include <ktxreader/Ktx2Reader.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

#include <getopt/getopt.h>

#include <utils/Path.h>

#include <stb_image.h>

#include <iostream>

#include "generated/resources/resources.h"
#include "generated/resources/monkey.h"

using namespace filament;
using namespace ktxreader;
using namespace filament::math;

struct App {
    Material* material;
    MaterialInstance* materialInstance;
    filamesh::MeshReader::Mesh mesh;
    mat4f transform;
    Texture* albedo;
    Texture* normal;
    Texture* roughness;
    Texture* metallic;
    Texture* ao;
};

static const char* IBL_FOLDER = "assets/ibl/lightroom_14b";

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
            "SHOWCASE renders a Suzanne model with compressed textures.\n"
            "Usage:\n"
            "    SHOWCASE [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --api, -a\n"
            "       Specify the backend API: opengl (default), vulkan, or metal\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], Config* config) {
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
            { "help",         no_argument,       nullptr, 'h' },
            { "api",          required_argument, nullptr, 'a' },
            { nullptr, 0, nullptr, 0 }
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
                    config->backend = Engine::Backend::OPENGL;
                } else if (arg == "vulkan") {
                    config->backend = Engine::Backend::VULKAN;
                } else if (arg == "metal") {
                    config->backend = Engine::Backend::METAL;
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'.\n";
                }
                break;
        }
    }
    return optind;
}

static Texture* loadNormalMap(Engine* engine, const uint8_t* normals, size_t nbytes) {
    int w, h, n;
    unsigned char* data = stbi_load_from_memory(normals, nbytes, &w, &h, &n, 3);
    Texture* normalMap = Texture::Builder()
            .width(uint32_t(w))
            .height(uint32_t(h))
            .levels(0xff)
            .format(Texture::InternalFormat::RGB8)
            .build(*engine);
    Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3),
            Texture::Format::RGB, Texture::Type::UBYTE,
            (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
    normalMap->setImage(*engine, 0, std::move(buffer));
    normalMap->generateMipmaps(*engine);
    return normalMap;
}

int main(int argc, char** argv) {
    Config config;
    config.title = "suzanne";
    config.iblDirectory = FilamentApp::getRootAssetsPath() + IBL_FOLDER;

    handleCommandLineArguments(argc, argv, &config);

    App app;
    auto setup = [config, &app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();

        Ktx2Reader reader(*engine);

        reader.requestFormat(Texture::InternalFormat::DXT3_SRGBA);
        reader.requestFormat(Texture::InternalFormat::DXT3_RGBA);

        // Uncompressed formats are lower priority, so they get added last.
        reader.requestFormat(Texture::InternalFormat::SRGB8_A8);
        reader.requestFormat(Texture::InternalFormat::RGBA8);

        constexpr auto sRGB = Ktx2Reader::TransferFunction::sRGB;
        constexpr auto LINEAR = Ktx2Reader::TransferFunction::LINEAR;

        app.albedo = reader.load(MONKEY_ALBEDO_DATA, MONKEY_ALBEDO_SIZE, sRGB);
        app.ao = reader.load(MONKEY_AO_DATA, MONKEY_AO_SIZE, LINEAR);
        app.metallic = reader.load(MONKEY_METALLIC_DATA, MONKEY_METALLIC_SIZE, LINEAR);
        app.roughness = reader.load(MONKEY_ROUGHNESS_DATA, MONKEY_ROUGHNESS_SIZE, LINEAR);

#if !defined(NDEBUG)
        using namespace utils;
        slog.i << "Resolved format for albedo: " << app.albedo->getFormat() << io::endl;
        slog.i << "Resolved format for ambient occlusion: " << app.ao->getFormat() << io::endl;
        slog.i << "Resolved format for metallic: " << app.metallic->getFormat() << io::endl;
        slog.i << "Resolved format for roughness: " << app.roughness->getFormat() << io::endl;
#endif

        app.normal = loadNormalMap(engine, MONKEY_NORMAL_DATA, MONKEY_NORMAL_SIZE);
        TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                TextureSampler::MagFilter::LINEAR);

        // Instantiate material.
        app.material = Material::Builder()
                .package(RESOURCES_TEXTUREDLIT_DATA, RESOURCES_TEXTUREDLIT_SIZE).build(*engine);
        app.materialInstance = app.material->createInstance();
        app.materialInstance->setParameter("albedo", app.albedo, sampler);
        app.materialInstance->setParameter("ao", app.ao, sampler);
        app.materialInstance->setParameter("metallic", app.metallic, sampler);
        app.materialInstance->setParameter("normal", app.normal, sampler);
        app.materialInstance->setParameter("roughness", app.roughness, sampler);

        auto ibl = FilamentApp::get().getIBL()->getIndirectLight();
        ibl->setIntensity(100000);
        ibl->setRotation(mat3f::rotation(0.5f, float3{ 0, 1, 0 }));

        // Add geometry into the scene.
        app.mesh = filamesh::MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA, nullptr,
                nullptr, app.materialInstance);
        auto ti = tcm.getInstance(app.mesh.renderable);
        app.transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        rcm.setCastShadows(rcm.getInstance(app.mesh.renderable), false);
        scene->addEntity(app.mesh.renderable);
        tcm.setTransform(ti, app.transform);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.mesh.renderable);
        engine->destroy(app.materialInstance);
        engine->destroy(app.material);
        engine->destroy(app.albedo);
        engine->destroy(app.normal);
        engine->destroy(app.roughness);
        engine->destroy(app.metallic);
        engine->destroy(app.ao);
    };

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
