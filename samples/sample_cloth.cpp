/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/MeshAssimp.h>

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
#include <memory>
#include <vector>

using namespace filament::math;
using namespace filament;
using namespace filamat;
using namespace utils;

namespace {

float g_meshScale = 1.0f;

struct App {
    std::vector<Path> filenames;
    std::map<utils::CString, MaterialInstance*> materialInstances;
    std::unique_ptr<MeshAssimp> meshSet;
    const Material* material = nullptr;
    Entity light;
    std::map<utils::CString, Texture*> maps;
    SampleConfig config;
    FilamentApp2* filamentApp = nullptr;
};

static const char* MODEL_FILE = "assets/models/monkey/monkey.obj";
static const char* TEXTURE_NORMAL = "assets/models/monkey/normal.png";
static const char* TEXTURE_BASECOLOR = "assets/models/monkey/color.png";
static const char* TEXTURE_ROUGHNESS = "assets/models/monkey/roughness.png";
static const char* IBL_FOLDER = "assets/ibl/lightroom_14b";

} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    if (config.iblDirectory.empty()) {
        config.iblDirectory = utils::CString(IBL_FOLDER);
    }
    auto app = std::make_shared<App>();
    app->config = config;
    g_meshScale = config.getFloat("scale", 1.0f);

    for (const auto& filename: config.positionalArgs) {
        app->filenames.push_back(utils::Path(filename.c_str()));
    }

    auto loadMap = [app, loader](Engine* engine, const char* name, bool sRGB = true) -> Texture* {
        int w = 0, h = 0, n = 0;
        unsigned char* data = nullptr;
        auto buf = loader->load(name);
        if (!buf.empty()) {
            data = stbi_load_from_memory(buf.data(), buf.size(), &w, &h, &n, 3);
        }
        if (data != nullptr) {
            Texture* map = Texture::Builder()
                                   .width(uint32_t(w))
                                   .height(uint32_t(h))
                                   .levels(0xff)
                                   .format(sRGB ? Texture::InternalFormat::SRGB8
                                                : Texture::InternalFormat::RGB8)
                                   .usage(Texture::Usage::DEFAULT | Texture::Usage::GEN_MIPMAPPABLE)
                                   .build(*engine);
            Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3), Texture::Format::RGB,
                    Texture::Type::UBYTE,
                    [](void* buffer, size_t, void*) { stbi_image_free(buffer); });
            map->setImage(*engine, 0, std::move(buffer));
            map->generateMipmaps(*engine);
            app->maps[utils::CString(name)] = map;
            return map;
        } else {
            std::cout << "The map " << name << " could not be loaded" << std::endl;
        }
        return nullptr;
    };

    auto setup = [app, loadMap, loader](Engine* engine, View* view, Scene* scene) {
        Texture* normal =
                loader->exists(Path("normal.png")) ? loadMap(engine, "normal.png", false) : nullptr;
        if (!normal) {
            normal = loadMap(engine, TEXTURE_NORMAL, false);
        }

        Texture* basecolor = loader->exists(Path("basecolor.png"))
                                     ? loadMap(engine, "basecolor.png", true)
                                     : nullptr;
        if (!basecolor) {
            basecolor = loadMap(engine, TEXTURE_BASECOLOR, true);
        }

        Texture* roughness = loader->exists(Path("roughness.png"))
                                     ? loadMap(engine, "roughness.png", false)
                                     : nullptr;
        if (!roughness) {
            roughness = loadMap(engine, TEXTURE_ROUGHNESS, false);
        }

        if (!basecolor || !normal || !roughness) {
            std::cout << "Need basecolor.png, normal.png and roughness.png" << std::endl;
            return;
        }

        MaterialBuilder::init();
        MaterialBuilder builder;
        builder.name("DefaultMaterial")
                .targetApi(MaterialBuilder::TargetApi::ALL)
                .platform(MaterialBuilder::Platform::ALL)
#ifndef NDEBUG
                .optimization(MaterialBuilderBase::Optimization::NONE)
#endif
                .require(VertexAttribute::UV0)
                .parameter("normalMap", MaterialBuilder::SamplerType::SAMPLER_2D)
                .parameter("basecolorMap", MaterialBuilder::SamplerType::SAMPLER_2D)
                .parameter("roughnessMap", MaterialBuilder::SamplerType::SAMPLER_2D)
                .material(R"SHADER(
                    void material(inout MaterialInputs material) {
                        vec2 uv = getUV0() * 2.0;
                        material.normal = texture(materialParams_normalMap, uv).xyz * 2.0 - 1.0;
                        prepareMaterial(material);

                        vec3 baseColor = texture(materialParams_basecolorMap, uv).rgb;
                        float luma = dot(baseColor, vec3(0.2126, 0.7152, 0.0722));

                        material.baseColor.rgb = baseColor;
                        material.roughness = texture(materialParams_roughnessMap, uv).r;
                        material.sheenColor = vec3(luma) * 0.5;
                    }
                )SHADER")
                .shading(Shading::CLOTH);

        Package pkg = builder.build(engine->getJobSystem());

        app->material = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
        if (app->material) {
            const utils::CString defaultMaterialName("DefaultMaterial");
            MaterialInstance* defaultMaterialInstance = app->material->createInstance();
            app->materialInstances[defaultMaterialName] = defaultMaterialInstance;

            TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                    TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
            sampler.setAnisotropy(8.0f);
            defaultMaterialInstance->setParameter("normalMap", normal, sampler);
            defaultMaterialInstance->setParameter("basecolorMap", basecolor, sampler);
            defaultMaterialInstance->setParameter("roughnessMap", roughness, sampler);
        }

        app->meshSet = std::make_unique<MeshAssimp>(*engine, loader);
        for (const auto& filename: app->filenames) {
            app->meshSet->addFromFile(filename, app->materialInstances, true);
        }
        if (app->meshSet->getRenderables().empty()) {
            app->meshSet->addFromFile(MODEL_FILE, app->materialInstances, true);
        }

        auto& tcm = engine->getTransformManager();
        if (!app->meshSet->getRenderables().empty()) {
            auto ei = tcm.getInstance(app->meshSet->getRenderables()[0]);
            tcm.setTransform(ei, mat4f{ mat3f(g_meshScale), float3(0.0f, 0.0f, -4.0f) } *
                                         tcm.getWorldTransform(ei));
        }
        for (auto renderable: app->meshSet->getRenderables()) {
            scene->addEntity(renderable);
        }

        app->light = EntityManager::get().create();
        LightManager::Builder(LightManager::Type::DIRECTIONAL)
                .color(Color::toLinear<ACCURATE>({ 0.98f, 0.92f, 0.89f }))
                .intensity(110000)
                .direction({ 0.6, -1, -0.8 })
                .build(*engine, app->light);
        scene->addEntity(app->light);
    };

    auto cleanup = [app](Engine* engine, View* view, Scene* scene) {
        app->meshSet.reset();
        for (auto& map: app->maps) {
            if (map.second) {
                engine->destroy(map.second);
                map.second = nullptr;
            }
        }
        for (auto& item: app->materialInstances) {
            if (item.second) {
                engine->destroy(item.second);
            }
        }
        app->materialInstances.clear();
        if (app->material) {
            engine->destroy(app->material);
            app->material = nullptr;
        }
        if (app->light) {
            engine->destroy(app->light);
            EntityManager::get().destroy(app->light);
            app->light = Entity{};
        }
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
        samples::Parameter::makeFloat("scale", 's', "Applies uniform scale", 1.0f),
    };
}

#ifndef __ANDROID__
int main(int argc, char* argv[]) {
    SampleConfig config;
    samples::CommandLineSpecification spec = {
        .sampleDescription = "SAMPLE_CLOTH demonstrates cloth shading in Filament.",
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

    config.title = "Cloth shading";

    auto app = createSampleApp(config, dm.get(), loader.get());
    app->run();

    return 0;
}
#endif
