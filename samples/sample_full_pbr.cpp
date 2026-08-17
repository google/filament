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

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/MeshAssimp.h>

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <filamat/MaterialBuilder.h>

#include <utils/EntityManager.h>
#include <utils/getopt.h>
#include <utils/Path.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec3.h>

#include <stb_image.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace filament::math;
using namespace filament;
using namespace filamat;
using namespace utils;
static float g_meshScale = 1.0f;

constexpr int MAP_COUNT       = 7;
constexpr int MAP_COLOR       = 0;
constexpr int MAP_AO          = 1;
constexpr int MAP_ROUGHNESS   = 2;
constexpr int MAP_METALLIC    = 3;
constexpr int MAP_NORMAL      = 4;
constexpr int MAP_BENT_NORMAL = 5;
constexpr int MAP_HEIGHT      = 6;

struct PbrMap {
    const char* suffix;
    const char* parameterName;
    bool sRGB;
    Texture* texture;
};

#ifdef __ANDROID__
static const char* MODEL_FILE = "models/material_sphere/material_sphere.obj";
#else
static const char* MODEL_FILE = "assets/models/material_sphere/material_sphere.obj";
#endif

namespace {
struct App {
    std::vector<Path> filenames;
    std::map<utils::CString, MaterialInstance*> materialInstances;
    std::unique_ptr<MeshAssimp> meshSet;
    const Material* material = nullptr;
    Entity light;
    std::array<PbrMap, MAP_COUNT> maps = {
        PbrMap{ "color", "baseColorMap", true, nullptr },
        PbrMap{ "ao", "aoMap", false, nullptr },
        PbrMap{ "roughness", "roughnessMap", false, nullptr },
        PbrMap{ "metallic", "metallicMap", false, nullptr },
        PbrMap{ "normal", "normalMap", false, nullptr },
        PbrMap{ "bentNormal", "bentNormalMap", false, nullptr },
        PbrMap{ "height", "heightMap", false, nullptr },
    };
    SampleConfig config;
    struct PbrConfig {
        std::string materialDir;
        bool clearCoat = false;
        bool anisotropy = false;
    } pbrConfig;
    FilamentApp2* filamentApp = nullptr;
};
} // anonymous namespace


std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    if (config.customArgs.find("materialDir") != config.customArgs.end()) {
        app->pbrConfig.materialDir = config.customArgs.at("materialDir").c_str();
    }
    if (config.customArgs.find("clearCoat") != config.customArgs.end()) {
        app->pbrConfig.clearCoat = true;
    }
    if (config.customArgs.find("anisotropy") != config.customArgs.end()) {
        app->pbrConfig.anisotropy = true;
    }
    if (config.customArgs.find("filenames") != config.customArgs.end()) {
        std::string_view filenamesStr = config.customArgs.at("filenames").c_str();
        size_t pos = 0;
        while ((pos = std::string_view(filenamesStr).find('|')) != std::string_view::npos) {
            app->filenames.push_back(utils::Path(filenamesStr.substr(0, pos)));
            filenamesStr.remove_prefix(pos + 1);
        }
        if (!filenamesStr.empty()) {
            app->filenames.push_back(utils::Path(filenamesStr));
        }
    }

    auto loadTexture = [](Engine* engine, const std::string& filePath, Texture** map,
                               bool sRGB = true) -> bool {
        if (!filePath.empty()) {
            Path const path(filePath);
            if (path.exists()) {
                int w, h, n;
                unsigned char* data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, 3);
                if (data != nullptr) {
                    *map = Texture::Builder()
                                   .width(uint32_t(w))
                                   .height(uint32_t(h))
                                   .levels(0xff)
                                   .format(sRGB ? Texture::InternalFormat::SRGB8
                                                : Texture::InternalFormat::RGB8)
                                   .usage(Texture::Usage::DEFAULT | Texture::Usage::GEN_MIPMAPPABLE)
                                   .build(*engine);
                    Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3),
                            Texture::Format::RGB, Texture::Type::UBYTE,
                            [](void* buffer, size_t, void*) { stbi_image_free(buffer); });
                    (*map)->setImage(*engine, 0, std::move(buffer));
                    (*map)->generateMipmaps(*engine);
                    return true;
                } else {
                    std::cout << "The texture " << path.c_str() << " could not be loaded"
                              << std::endl;
                }
            }
        }
        return false;
    };

    auto setup = [app, loadTexture, loader](Engine* engine, View* view, Scene* scene) {
        Path const path(app->pbrConfig.materialDir);
        std::string const name(path.getName());

        view->setAmbientOcclusionOptions({ .radius = 0.01f,
            .bilateralThreshold = 0.005f,
            .quality = View::QualityLevel::ULTRA,
            .lowPassFilter = View::QualityLevel::MEDIUM,
            .upsampling = View::QualityLevel::HIGH,
            .enabled = true });

        bool hasUV = false;
        for (auto& map: app->maps) {
            if (!loadTexture(engine, path.concat(name + "_" + map.suffix + ".png"), &map.texture,
                        map.sRGB)) {
                if (!loadTexture(engine, path.concat(std::string(map.suffix) + ".png"),
                            &map.texture, map.sRGB)) {
                    std::cout << "The texture " << map.suffix << " does not exist" << std::endl;
                }
            }
            if (map.texture != nullptr) hasUV = true;
        }

        bool const hasBaseColorMap = app->maps[MAP_COLOR].texture != nullptr;
        bool const hasMetallicMap = app->maps[MAP_METALLIC].texture != nullptr;
        bool const hasRoughnessMap = app->maps[MAP_ROUGHNESS].texture != nullptr;
        bool const hasAOMap = app->maps[MAP_AO].texture != nullptr;
        bool const hasNormalMap = app->maps[MAP_NORMAL].texture != nullptr;
        bool const hasBentNormalMap = app->maps[MAP_BENT_NORMAL].texture != nullptr;
        bool const hasHeightMap = app->maps[MAP_HEIGHT].texture != nullptr;

        std::string shader = R"SHADER(
            void material(inout MaterialInputs material) {
        )SHADER";

        if (hasUV) {
            shader += R"SHADER(
                vec2 uv0 = getUV0();
            )SHADER";
        }

        if (hasHeightMap) {
            // Parallax Occlusion Mapping
            shader += R"SHADER(
                vec2 uvDx = dFdx(uv0);
                vec2 uvDy = dFdy(uv0);

                mat3 tangentFromWorld = transpose(getWorldTangentFrame());
                vec3 v = tangentFromWorld * getWorldViewVector();

                float minLayers = 8.0;
                float maxLayers = 48.0;
                float numLayers = mix(maxLayers, minLayers,
                        dot(getWorldGeometricNormalVector(), getWorldViewVector()));
                float heightScale = 0.05;

                float layerDepth = 1.0 / numLayers;
                float currLayerDepth = 0.0;

                vec2 deltaUV = v.xy * heightScale / (v.z * numLayers);
                vec2 currUV = uv0;
                float height = 1.0 - textureGrad(materialParams_heightMap, currUV, uvDx, uvDy).r;
                for (int i = 0; i < int(numLayers); i++) {
                    currLayerDepth += layerDepth;
                    currUV -= deltaUV;
                    height = 1.0 - textureGrad(materialParams_heightMap, currUV, uvDx, uvDy).r;
                    if (height < currLayerDepth) {
                        break;
                    }
                }

                vec2 prevUV = currUV + deltaUV;
                float nextDepth = height - currLayerDepth;
                float prevDepth = 1.0 - textureGrad(materialParams_heightMap, prevUV, uvDx, uvDy).r -
                        currLayerDepth + layerDepth;
                uv0 = mix(currUV, prevUV, nextDepth / (nextDepth - prevDepth));
            )SHADER";
        }

        if (hasNormalMap) {
            shader += R"SHADER(
                material.normal = texture(materialParams_normalMap, uv0).xyz * 2.0 - 1.0;
                material.normal.y *= -1.0;
            )SHADER";
        }
        if (hasBentNormalMap) {
            shader += R"SHADER(
                material.bentNormal = texture(materialParams_bentNormalMap, uv0).xyz * 2.0 - 1.0;
                material.bentNormal.y *= -1.0;
            )SHADER";
        }

        shader += R"SHADER(
            prepareMaterial(material);
        )SHADER";

        if (hasBaseColorMap) {
            shader += R"SHADER(
                material.baseColor.rgb = texture(materialParams_baseColorMap, uv0).rgb;
            )SHADER";
        } else {
            shader += R"SHADER(
                material.baseColor.rgb = float3(0.5);
            )SHADER";
        }
        if (hasMetallicMap) {
            shader += R"SHADER(
                material.metallic = texture(materialParams_metallicMap, uv0).r;
            )SHADER";
        } else {
            shader += R"SHADER(
                material.metallic = 0.0;
            )SHADER";
        }
        if (hasRoughnessMap) {
            shader += R"SHADER(
                material.roughness = texture(materialParams_roughnessMap, uv0).r;
            )SHADER";
        } else {
            shader += R"SHADER(
                material.roughness = 0.4;
            )SHADER";
        }
        if (hasAOMap) {
            shader += R"SHADER(
                material.ambientOcclusion = texture(materialParams_aoMap, uv0).r;
            )SHADER";
        } else {
            shader += R"SHADER(
                material.ambientOcclusion = 1.0;
            )SHADER";
        }

        if (app->pbrConfig.clearCoat) {
            shader += R"SHADER(
                material.clearCoat = 1.0;
            )SHADER";
        }
        if (app->pbrConfig.anisotropy) {
            shader += R"SHADER(
                material.anisotropy = 0.7;
            )SHADER";
        }
        shader += "}\n";

        MaterialBuilder::init();
        MaterialBuilder builder;
        builder.name("DefaultMaterial")
                .targetApi(MaterialBuilder::TargetApi::ALL)
                .platform(MaterialBuilder::Platform::ALL)
#ifndef NDEBUG
                .optimization(MaterialBuilderBase::Optimization::NONE)
                .generateDebugInfo(true)
#endif
                .material(shader.c_str())
                .multiBounceAmbientOcclusion(true)
                .specularAmbientOcclusion(MaterialBuilder::SpecularAmbientOcclusion::BENT_NORMALS)
                .shading(Shading::LIT);

        if (hasUV) {
            builder.require(VertexAttribute::UV0);
        }

        for (auto& map: app->maps) {
            if (map.texture != nullptr) {
                builder.parameter(map.parameterName, MaterialBuilder::SamplerType::SAMPLER_2D);
            }
        }

        Package const pkg = builder.build(engine->getJobSystem());

        app->material = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
        if (app->material) {
            app->materialInstances["DefaultMaterial"] = app->material->createInstance();

            TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                    TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
            sampler.setAnisotropy(8.0f);

            for (auto& map: app->maps) {
                if (map.texture != nullptr) {
                    app->materialInstances["DefaultMaterial"]->setParameter(map.parameterName,
                            map.texture, sampler);
                }
            }
        }

        app->meshSet = std::make_unique<MeshAssimp>(*engine);
        for (auto& filename: app->filenames) {
            app->meshSet->addFromFile(filename, app->materialInstances, true);
        }
        if (app->meshSet->getRenderables().empty()) {
            if (loader) {
                auto modelBuffer = loader->load(MODEL_FILE);
                if (!modelBuffer.empty()) {
                    app->meshSet->addFromMemory(modelBuffer.data(), modelBuffer.size(),
                            utils::Path(MODEL_FILE), app->materialInstances, true);
                }
            }
            if (app->meshSet->getRenderables().empty()) {
                app->meshSet->addFromFile(FilamentApp2::getRootAssetsPath() + MODEL_FILE,
                        app->materialInstances, true);
            }
        }

        auto& rcm = engine->getRenderableManager();
        auto& tcm = engine->getTransformManager();
        for (auto renderable: app->meshSet->getRenderables()) {
            if (rcm.hasComponent(renderable)) {
                auto ti = tcm.getInstance(renderable);
                tcm.setTransform(ti, mat4f{ mat3f(g_meshScale), float3(0.0f, 0.0f, -4.0f) } *
                                             tcm.getWorldTransform(ti));
                rcm.setReceiveShadows(rcm.getInstance(renderable), true);
                rcm.setCastShadows(rcm.getInstance(renderable), true);
                scene->addEntity(renderable);
            }
        }

        app->light = EntityManager::get().create();
        LightManager::Builder(LightManager::Type::SUN)
                .color(Color::toLinear<ACCURATE>({ 0.98f, 0.92f, 0.89f }))
                .intensity(110000)
                .direction({ 0.6, -1, -0.8 })
                .castShadows(true)
                .build(*engine, app->light);
        scene->addEntity(app->light);
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        app->meshSet.reset();
        for (auto& item: app->materialInstances) {
            auto materialInstance = item.second;
            if (materialInstance) {
                engine->destroy(materialInstance);
            }
        }
        app->materialInstances.clear();
        if (app->material) {
            engine->destroy(app->material);
            app->material = nullptr;
        }
        for (auto& map: app->maps) {
            if (map.texture) {
                engine->destroy(map.texture);
                map.texture = nullptr;
            }
        }
        if (app->light) {
            engine->destroy(app->light);
            EntityManager::get().destroy(app->light);
            app->light = Entity{};
        }
    };

    auto preRender = [app](filament::Engine*, filament::View*, filament::Scene*,
                             filament::Renderer* renderer) {
        // Without an IBL, we must clear the swapchain to black before each frame.
        renderer->setClearOptions(
                { .clearColor = { 0.5f, 0.5f, 0.5f, 1.0f }, .clear = !app->filamentApp->getIBL() });
    };

    auto fApp = FilamentApp2::Builder()
                        .title(app->config.title)
                        .backend(app->config.backend)
                        .featureLevel(app->config.featureLevel)
                        .displayManager(dm)
                        .assetLoader(loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .preRender(preRender)
                        .build();
    app->filamentApp = fApp.get();
    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char* argv[]) {
    SampleConfig config;
    static constexpr const char* CUSTOM_OPTSTR = "m:cAs:";
    static const utils::getopt::option CUSTOM_OPTIONS[] = {
        { "material-dir", utils::getopt::required_argument, nullptr, 'm' },
        { "clear-coat", utils::getopt::no_argument, nullptr, 'c' },
        { "anisotropy", utils::getopt::no_argument, nullptr, 'A' },
        { "scale", utils::getopt::required_argument, nullptr, 's' },
        { nullptr, 0, nullptr, 0 },
    };
    auto customHandler = [&config](int opt, const utils::CString& arg) -> bool {
        switch (opt) {
            case 'm':
                config.customArgs["materialDir"] = arg;
                return true;
            case 'c':
                config.customArgs["clearCoat"] = utils::CString("true");
                return true;
            case 'A':
                config.customArgs["anisotropy"] = utils::CString("true");
                return true;
            case 's':
                g_meshScale = std::stof(arg.c_str());
                return true;
        }
        return false;
    };
    samples::CommandLineSpecification spec = {
        .sampleDescription = "SAMPLE_FULL_PBR demonstrates physically based rendering in Filament.",
        .positionalArgsDescription = "<mesh files (.obj, .fbx)>",
        .requiredPositionalArgCount = 1,
        .customOptionsHelp = "   --material-dir=<path>, -m <path>\n"
                             "       Directory containing custom materials\n\n"
                             "   --clear-coat, -c\n"
                             "       Enable clear coat\n\n"
                             "   --anisotropy, -A\n"
                             "       Enable anisotropy\n",
        .customHandler = customHandler,
        .customOptStr = CUSTOM_OPTSTR,
        .customOptions = CUSTOM_OPTIONS,
    };

    int optind = samples::handleCommandLineArguments(argc, argv, &config, spec);
    auto dm = samples::getDisplayManager(config);

    utils::CString filenames;
    for (int i = optind; i < argc; i++) {
        utils::Path const filename = argv[i];
        if (!filename.exists()) {
            std::cerr << "file " << argv[i] << " not found!" << std::endl;
            return 1;
        }
        if (i > optind) filenames += "|";
        filenames += argv[i];
    }
    config.customArgs["filenames"] = utils::CString(filenames.c_str());

    config.title = "PBR";

    auto app = createSampleApp(config, dm.get(), nullptr);
    app->run();

    return 0;
}
#endif
