/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "generated/resources/monkey.h"

#include <filameshio/MeshReader.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <filamat/MaterialBuilder.h>

#include <utils/EntityManager.h>
#include <utils/getopt.h>
#include <utils/Path.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec4.h>

#include <samples/SampleConfig.h>
#include <stb_image.h>

#include <iostream>
#include <map>
#include <vector>

using namespace filament::math;
using namespace filament;
using namespace filamesh;
using namespace filamat;
using namespace utils;

namespace {

float g_meshScale = 1.0f;

struct NormalConfig {
    utils::CString normalMap;
    utils::CString clearCoatNormalMap;
    utils::CString baseColorMap;
} g_normalConfig;

struct App {
    FilamentApp2* filamentApp;
    MeshReader::MaterialRegistry materialInstances;
    std::vector<MeshReader::Mesh> meshes;
    const Material* material;
    Entity light;
    Texture* normalMap = nullptr;
    Texture* clearCoatNormalMap = nullptr;
    Texture* baseColorMap = nullptr;
    SampleConfig config;
};

} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;
    utils::CString nm = config.getString("normal-map");
    if (!nm.empty()) {
        g_normalConfig.normalMap = nm;
    }
    utils::CString cnm = config.getString("clearcoat-normal-map");
    if (!cnm.empty()) {
        g_normalConfig.clearCoatNormalMap = cnm;
    }
    utils::CString bm = config.getString("basecolor-map");
    if (!bm.empty()) {
        g_normalConfig.baseColorMap = bm;
    }

    if (g_normalConfig.normalMap.empty()) {
        g_normalConfig.normalMap = "assets/models/monkey/normal.png";
    }
    if (g_normalConfig.baseColorMap.empty()) {
        g_normalConfig.baseColorMap = "assets/models/monkey/color.png";
    }
    g_meshScale = config.getFloat("scale", 1.0f);

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        if (app->baseColorMap) {
            engine->destroy(app->baseColorMap);
            app->baseColorMap = nullptr;
        }
        if (app->normalMap) {
            engine->destroy(app->normalMap);
            app->normalMap = nullptr;
        }
        if (app->clearCoatNormalMap) {
            engine->destroy(app->clearCoatNormalMap);
            app->clearCoatNormalMap = nullptr;
        }
        EntityManager& em = EntityManager::get();
        for (auto mesh: app->meshes) {
            if (mesh.vertexBuffer) {
                engine->destroy(mesh.vertexBuffer);
            }
            if (mesh.indexBuffer) {
                engine->destroy(mesh.indexBuffer);
            }
            if (mesh.renderable) {
                engine->destroy(mesh.renderable);
                em.destroy(mesh.renderable);
            }
        }
        app->meshes.clear();

        std::vector<filament::MaterialInstance*> materialList(
                app->materialInstances.numRegistered());
        app->materialInstances.getRegisteredMaterials(materialList.data());
        for (auto material: materialList) {
            if (material) {
                engine->destroy(material);
            }
        }
        app->materialInstances.unregisterAll();
        if (app->material) {
            engine->destroy(app->material);
            app->material = nullptr;
        }

        if (app->light) {
            engine->destroy(app->light);
            em.destroy(app->light);
            app->light = Entity{};
        }
    };

    auto setup = [app, loader](Engine* engine, View*, Scene* scene) {
        auto loadNormalMap = [loader](Engine* engine, Texture** normalMap,
                                     const utils::CString& path) {
            if (!path.empty()) {
                int w = 0, h = 0, n = 0;
                unsigned char* data = nullptr;
                auto buf = loader->load(path.c_str());
                if (!buf.empty()) {
                    data = stbi_load_from_memory(buf.data(), buf.size(), &w, &h, &n, 3);
                }
                if (data != nullptr) {
                    *normalMap = Texture::Builder()
                                         .width(uint32_t(w))
                                         .height(uint32_t(h))
                                         .levels(0xff)
                                         .format(Texture::InternalFormat::RGB8)
                                         .usage(Texture::Usage::DEFAULT |
                                                 Texture::Usage::GEN_MIPMAPPABLE)
                                         .build(*engine);
                    Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3),
                            Texture::Format::RGB, Texture::Type::UBYTE,
                            (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
                    (*normalMap)->setImage(*engine, 0, std::move(buffer));
                    (*normalMap)->generateMipmaps(*engine);
                } else {
                    std::cout << "The normal map " << path.c_str() << " could not be loaded"
                              << std::endl;
                }
            }
        };

        auto loadBaseColorMap = [app, loader](Engine* engine) {
            if (!g_normalConfig.baseColorMap.empty()) {
                int w = 0, h = 0, n = 0;
                unsigned char* data = nullptr;
                auto buf = loader->load(g_normalConfig.baseColorMap.c_str());
                if (!buf.empty()) {
                    data = stbi_load_from_memory(buf.data(), buf.size(), &w, &h, &n, 3);
                }
                if (data != nullptr) {
                    app->baseColorMap = Texture::Builder()
                                                .width(uint32_t(w))
                                                .height(uint32_t(h))
                                                .levels(0xff)
                                                .format(Texture::InternalFormat::SRGB8)
                                                .usage(Texture::Usage::DEFAULT |
                                                        Texture::Usage::GEN_MIPMAPPABLE)
                                                .build(*engine);
                    Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3),
                            Texture::Format::RGB, Texture::Type::UBYTE,
                            (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
                    app->baseColorMap->setImage(*engine, 0, std::move(buffer));
                    app->baseColorMap->generateMipmaps(*engine);
                } else {
                    std::cout << "The base color map " << g_normalConfig.baseColorMap.c_str()
                              << " could not be loaded" << std::endl;
                }
            }
        };

        loadNormalMap(engine, &app->normalMap, g_normalConfig.normalMap);
        loadNormalMap(engine, &app->clearCoatNormalMap, g_normalConfig.clearCoatNormalMap);
        loadBaseColorMap(engine);

        bool const hasNormalMap = app->normalMap != nullptr;
        bool const hasClearCoatNormalMap = app->clearCoatNormalMap != nullptr;
        bool const hasBaseColorMap = app->baseColorMap != nullptr;

        utils::CString shader = R"SHADER(
            void material(inout MaterialInputs material) {
        )SHADER";

        if (hasNormalMap) {
            shader += R"SHADER(
                material.normal = texture(materialParams_normalMap, getUV0()).xyz * 2.0 - 1.0;
            )SHADER";
        }

        if (hasClearCoatNormalMap) {
            shader += R"SHADER(
                material.clearCoatNormal =
                        texture(materialParams_clearCoatNormalMap, getUV0()).xyz * 2.0 - 1.0;
            )SHADER";
        }

        shader += R"SHADER(
            prepareMaterial(material);
        )SHADER";

        if (hasBaseColorMap) {
            shader += R"SHADER(
                material.baseColor.rgb = texture(materialParams_baseColorMap, getUV0()).rgb;
                material.metallic = 1.0;
                material.roughness = 0.6;
            )SHADER";
        } else {
            shader += R"SHADER(
                material.baseColor.rgb = float3(0.3, 0.0, 0.0);
                material.metallic = 0.0;
                material.roughness = 0.0;
            )SHADER";
        }

        if (hasClearCoatNormalMap) {
            shader += "    material.clearCoat = 1.0;\n";
        }

        shader += "}\n";

        MaterialBuilder::init();
        MaterialBuilder builder;
        builder.name("DefaultMaterial")
                .targetApi(MaterialBuilder::TargetApi::ALL)
                .platform(MaterialBuilder::Platform::ALL)
#ifndef NDEBUG
                .optimization(MaterialBuilderBase::Optimization::NONE)
#endif
                .material(shader.c_str())
                .specularAntiAliasing(true)
                .shading(Shading::LIT);

        if (hasNormalMap) {
            builder.require(VertexAttribute::UV0)
                    .parameter("normalMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        }

        if (hasClearCoatNormalMap) {
            builder.require(VertexAttribute::UV0)
                    .parameter("clearCoatNormalMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        }

        if (hasBaseColorMap) {
            builder.require(VertexAttribute::UV0)
                    .parameter("baseColorMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        }

        Package pkg = builder.build(engine->getJobSystem());

        app->material = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
        if (app->material) {
            const utils::CString defaultMaterialName("DefaultMaterial");
            app->materialInstances.registerMaterialInstance(defaultMaterialName,
                    app->material->createInstance());

            TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                    TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
            sampler.setAnisotropy(8.0f);

            if (hasNormalMap) {
                app->materialInstances.getMaterialInstance(defaultMaterialName)
                        ->setParameter("normalMap", app->normalMap, sampler);
            }
            if (hasClearCoatNormalMap) {
                app->materialInstances.getMaterialInstance(defaultMaterialName)
                        ->setParameter("clearCoatNormalMap", app->clearCoatNormalMap, sampler);
            }
            if (hasBaseColorMap) {
                app->materialInstances.getMaterialInstance(defaultMaterialName)
                        ->setParameter("baseColorMap", app->baseColorMap, sampler);
            }
        }

        std::vector<utils::Path> filenames;
        for (const auto& fname : app->config.positionalArgs) {
            filenames.push_back(utils::Path(fname.c_str_safe()));
        }
        auto& tcm = engine->getTransformManager();
        for (const auto& filename: filenames) {
            MeshReader::Mesh mesh;
            auto buf = loader->load(filename);
            if (!buf.empty()) {
                mesh = MeshReader::loadMeshFromBuffer(engine, buf.data(), buf.size(), nullptr,
                        nullptr, app->materialInstances);
            }
            if (mesh.renderable) {
                auto ei = tcm.getInstance(mesh.renderable);
                tcm.setTransform(ei, mat4f{ mat3f(g_meshScale), float3(0.0f, 0.0f, -4.0f) } *
                                             tcm.getWorldTransform(ei));
                scene->addEntity(mesh.renderable);
                app->meshes.push_back(mesh);
            }
        }
        if (app->meshes.empty()) {
            MeshReader::Mesh mesh = MeshReader::loadMeshFromBuffer(engine, MONKEY_SUZANNE_DATA,
                    MONKEY_SUZANNE_SIZE, nullptr, nullptr, app->materialInstances);
            if (mesh.renderable) {
                auto ei = tcm.getInstance(mesh.renderable);
                tcm.setTransform(ei, mat4f{ mat3f(g_meshScale), float3(0.0f, 0.0f, -4.0f) } *
                                             tcm.getWorldTransform(ei));
                scene->addEntity(mesh.renderable);
                app->meshes.push_back(mesh);
            }
        }

        app->light = EntityManager::get().create();
        LightManager::Builder(LightManager::Type::DIRECTIONAL)
                .color(Color::toLinear<ACCURATE>({ 0.98f, 0.92f, 0.89f }))
                .intensity(110000)
                .direction({ 0.6, -1, -0.8 })
                .build(*engine, app->light);
        scene->addEntity(app->light);
    };

    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .imgui(nullptr)
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

samples::SampleParameters createAppParameters() {
    return {
        samples::Parameter::makeString("normal-map", 'n', "Path to normal map texture", ""),
        samples::Parameter::makeString("clearcoat-normal-map", 'C',
                "Path to clearcoat normal map texture", ""),
        samples::Parameter::makeString("basecolor-map", 'b', "Path to base color texture", ""),
        samples::Parameter::makeFloat("scale", 's', "Applies uniform scale", 1.0f),
    };
}

#ifndef __ANDROID__
int main(int argc, char* argv[]) {
    SampleConfig config;
    samples::CommandLineSpecification spec = {
        .sampleDescription = "SAMPLE_NORMAL_MAP tests normal mapping and clearcoat normal mapping.",
        .positionalArgsDescription = { "[mesh files (.obj, .fbx)]" },
        .requiredPositionalArgCount = 0,
        .parameters = createAppParameters(),
    };

    samples::handleCommandLineArguments(argc, argv, &config, spec);
    auto dm = samples::getDisplayManager(config);
    auto loader = samples::getAssetLoader(config);

    for (const auto& fname : config.positionalArgs) {
        utils::Path filename(fname.c_str_safe());
        if (!loader->exists(filename)) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
    }

    config.title = "Normal Mapping";
    auto app = createSampleApp(config, dm.get(), loader.get());
    app->run();

    return 0;
}
#endif
