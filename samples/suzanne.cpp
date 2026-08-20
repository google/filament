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

#include "common/arguments.h"
#include "common/SampleConfig.h"

#include "generated/resources/monkey.h"
#include "generated/resources/resources.h"

#include <ktxreader/Ktx2Reader.h>

#include <filameshio/MeshReader.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/IBL.h>

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
#include <utils/getopt.h>
#include <utils/Log.h>
#include <utils/Path.h>

#include <stb_image.h>

#include <iostream>

using namespace filament;
using namespace ktxreader;
using namespace filament::math;

namespace {

struct App {
    FilamentApp2* filamentApp;
    SampleConfig config;
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

constexpr const char* IBL_FOLDER = "assets/ibl/lightroom_14b";

Texture* loadNormalMap(Engine* engine, const uint8_t* normals, size_t nbytes) {
    int w, h, n;
    unsigned char* data = stbi_load_from_memory(normals, nbytes, &w, &h, &n, 3);
    Texture* normalMap = Texture::Builder()
            .width(uint32_t(w))
            .height(uint32_t(h))
            .levels(0xff)
            .format(Texture::InternalFormat::RGB8)
            .usage(Texture::Usage::DEFAULT | Texture::Usage::GEN_MIPMAPPABLE)
            .build(*engine);
    Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3),
            Texture::Format::RGB, Texture::Type::UBYTE,
            (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
    normalMap->setImage(*engine, 0, std::move(buffer));
    normalMap->generateMipmaps(*engine);
    return normalMap;
}

} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    auto setup = [app](Engine* engine, View* view, Scene* scene) {
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

        app->albedo = reader.load(MONKEY_ALBEDO_DATA, MONKEY_ALBEDO_SIZE, sRGB);
        app->ao = reader.load(MONKEY_AO_DATA, MONKEY_AO_SIZE, LINEAR);
        app->metallic = reader.load(MONKEY_METALLIC_DATA, MONKEY_METALLIC_SIZE, LINEAR);
        app->roughness = reader.load(MONKEY_ROUGHNESS_DATA, MONKEY_ROUGHNESS_SIZE, LINEAR);

#if !defined(NDEBUG)
        using namespace utils;
        slog.i << "Resolved format for albedo: " << app->albedo->getFormat() << io::endl;
        slog.i << "Resolved format for ambient occlusion: " << app->ao->getFormat() << io::endl;
        slog.i << "Resolved format for metallic: " << app->metallic->getFormat() << io::endl;
        slog.i << "Resolved format for roughness: " << app->roughness->getFormat() << io::endl;
#endif

        app->normal = loadNormalMap(engine, MONKEY_NORMAL_DATA, MONKEY_NORMAL_SIZE);
        TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                TextureSampler::MagFilter::LINEAR);

        // Instantiate material.
        app->material = Material::Builder()
                                .package(RESOURCES_TEXTUREDLIT_DATA, RESOURCES_TEXTUREDLIT_SIZE)
                                .build(*engine);
        app->materialInstance = app->material->createInstance();
        app->materialInstance->setParameter("albedo", app->albedo, sampler);
        app->materialInstance->setParameter("ao", app->ao, sampler);
        app->materialInstance->setParameter("metallic", app->metallic, sampler);
        app->materialInstance->setParameter("normal", app->normal, sampler);
        app->materialInstance->setParameter("roughness", app->roughness, sampler);

        auto ibl = app->filamentApp->getIBL()->getIndirectLight();
        ibl->setIntensity(100000);
        ibl->setRotation(mat3f::rotation(0.5f, float3{ 0, 1, 0 }));

        // Add geometry into the scene.
        app->mesh = filamesh::MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA,
                MONKEY_SUZANNE_SIZE, nullptr, nullptr, app->materialInstance);
        auto ti = tcm.getInstance(app->mesh.renderable);
        app->transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        rcm.setCastShadows(rcm.getInstance(app->mesh.renderable), false);
        scene->addEntity(app->mesh.renderable);
        tcm.setTransform(ti, app->transform);
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        engine->destroy(app->mesh.renderable);
        engine->destroy(app->materialInstance);
        engine->destroy(app->material);
        engine->destroy(app->albedo);
        engine->destroy(app->normal);
        engine->destroy(app->roughness);
        engine->destroy(app->metallic);
        engine->destroy(app->ao);
    };

    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .build();

    app->filamentApp = fApp.get();
    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "suzanne";
    config.iblDirectory = utils::CString((FilamentApp2::getRootAssetsPath() + IBL_FOLDER).c_str());

    int optind = samples::handleCommandLineArguments(argc, argv, &config);
    auto dm = samples::getDisplayManager(config);

    auto fApp = createSampleApp(config, dm.get(), nullptr);
    fApp->run();

    return 0;
}
#endif
