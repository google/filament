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
#include "common/SampleConfig.h"

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

#include <stb_image.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace filament::math;
using namespace filament;
using namespace filamesh;
using namespace filamat;
using namespace utils;
static float g_meshScale = 1.0f;

static std::vector<Path> g_filenames;

static struct NormalConfig {
    std::string normalMap;
    std::string clearCoatNormalMap;
    std::string baseColorMap;
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

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        engine->destroy(app->normalMap);
        engine->destroy(app->clearCoatNormalMap);
        std::vector<filament::MaterialInstance*> materialList(
                app->materialInstances.numRegistered());
        app->materialInstances.getRegisteredMaterials(materialList.data());
        for (auto material: materialList) {
            engine->destroy(material);
        }
        app->materialInstances.unregisterAll();
        engine->destroy(app->material);
        EntityManager& em = EntityManager::get();
        for (auto mesh: app->meshes) {
            engine->destroy(mesh.vertexBuffer);
            engine->destroy(mesh.indexBuffer);
            engine->destroy(mesh.renderable);
            em.destroy(mesh.renderable);
        }
        engine->destroy(app->light);
        em.destroy(app->light);
    };

    auto setup = [app](Engine* engine, View*, Scene* scene) {
        auto loadNormalMap = [](Engine* engine, Texture** normalMap, const std::string& path) {
            if (!path.empty()) {
                Path p(path);
                if (p.exists()) {
                    int w, h, n;
                    unsigned char* data = stbi_load(p.getAbsolutePath().c_str(), &w, &h, &n, 3);
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
                        std::cout << "The normal map " << p << " could not be loaded" << std::endl;
                    }
                } else {
                    std::cout << "The normal map " << p << " does not exist" << std::endl;
                }
            }
        };

        auto loadBaseColorMap = [app](Engine* engine) {
            if (!g_normalConfig.baseColorMap.empty()) {
                Path path(g_normalConfig.baseColorMap);
                if (path.exists()) {
                    int w, h, n;
                    unsigned char* data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, 3);
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
                        std::cout << "The base color map " << path.c_str() << " could not be loaded"
                                  << std::endl;
                    }
                } else {
                    std::cout << "The base color map " << path.c_str() << " does not exist"
                              << std::endl;
                }
            }
        };

        loadNormalMap(engine, &app->normalMap, g_normalConfig.normalMap);
        loadNormalMap(engine, &app->clearCoatNormalMap, g_normalConfig.clearCoatNormalMap);
        loadBaseColorMap(engine);

        bool hasNormalMap = app->normalMap != nullptr;
        bool hasClearCoatNormalMap = app->clearCoatNormalMap != nullptr;
        bool hasBaseColorMap = app->baseColorMap != nullptr;

        std::string shader = R"SHADER(
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

        auto& tcm = engine->getTransformManager();
        for (const auto& filename: g_filenames) {
            MeshReader::Mesh mesh =
                    MeshReader::loadMeshFromFile(engine, filename, app->materialInstances);
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

    auto fApp = FilamentApp2::Builder()
                        .displayManager(dm)
                        .title(app->config.title)
                        .setup(setup)
                        .cleanup(cleanup)
                        .imgui(nullptr)
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char* argv[]) {
    SampleConfig config;
    static constexpr const char* CUSTOM_OPTSTR = "n:c:b:s:";
    static const utils::getopt::option CUSTOM_OPTIONS[] = {
        { "normal-map", utils::getopt::required_argument, nullptr, 'n' },
        { "clearcoat-normal-map", utils::getopt::required_argument, nullptr, 'c' },
        { "basecolor-map", utils::getopt::required_argument, nullptr, 'b' },
        { "scale", utils::getopt::required_argument, nullptr, 's' },
        { nullptr, 0, nullptr, 0 },
    };
    auto customHandler = [](int opt, const utils::CString& arg) -> bool {
        switch (opt) {
            case 'n':
                g_normalConfig.normalMap = arg.c_str();
                return true;
            case 'c':
                g_normalConfig.clearCoatNormalMap = arg.c_str();
                return true;
            case 'b':
                g_normalConfig.baseColorMap = arg.c_str();
                return true;
            case 's':
                g_meshScale = std::stof(arg.c_str());
                return true;
        }
        return false;
    };
    samples::CommandLineSpecification spec = {
        .sampleDescription = "SAMPLE_NORMAL_MAP tests normal mapping and clearcoat normal mapping.",
        .positionalArgsDescription = "<mesh files (.obj, .fbx)>",
        .requiredPositionalArgCount = 1,
        .customOptionsHelp = "   --normal-map=<path>, -n <path>\n"
                             "       Path to normal map texture\n\n"
                             "   --clearcoat-normal-map=<path>, -c <path>\n"
                             "       Path to clearcoat normal map texture\n\n"
                             "   --basecolor-map=<path>, -b <path>\n"
                             "       Path to base color texture\n",
        .customHandler = customHandler,
        .customOptStr = CUSTOM_OPTSTR,
        .customOptions = CUSTOM_OPTIONS,
    };

    int optind = samples::handleCommandLineArguments(argc, argv, &config, spec);
    auto dm = samples::getDisplayManager(config);

    for (int i = optind; i < argc; i++) {
        utils::Path filename = argv[i];
        if (!filename.exists()) {
            std::cerr << "file " << argv[i] << " not found!" << std::endl;
            return 1;
        }
        g_filenames.push_back(filename);
    }

    config.title = "Normal Mapping";
    auto app = createSampleApp(config, dm.get(), nullptr);
    app->run();

    return 0;
}
#endif
