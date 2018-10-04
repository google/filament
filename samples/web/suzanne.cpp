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

#include "filamesh.h"
#include "filaweb.h"

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <math/vec3.h>

using namespace filament;
using namespace math;
using namespace std;
using namespace utils;

using MagFilter = TextureSampler::MagFilter;
using WrapMode = TextureSampler::WrapMode;
using Format = Texture::InternalFormat;

struct SuzanneApp {
    Filamesh filamesh;
    Material* mat;
    MaterialInstance* mi;
    Camera* cam;
    Entity sun;
    Entity ptlight[4];
};

static constexpr uint8_t MATERIAL_LIT_PACKAGE[] = {
    #include "generated/material/texturedLit.inc"
};

static SuzanneApp app;

static Texture* setTextureParameter(Engine& engine, filaweb::Asset& asset, string name, bool linear,
        TextureSampler const &sampler) {
    const auto& info = asset.texture->getInfo();
    const uint32_t nmips = asset.texture->getNumMipLevels();

    // This little structure tracks how many miplevels have been uploaded to the GPU so that we can
    // free the CPU memory at the right time.
    struct Uploader {
        filaweb::Asset* asset;
        uint32_t refcount;
    };

    Uploader* uploader = new Uploader {&asset, nmips};

    const auto destroy = [](void* buffer, size_t size, void* user) {
        auto uploader = (Uploader*) user;
        if (--uploader->refcount == 0) {
            uploader->asset->texture.reset();
            delete uploader;
        }
    };

    uint8_t* data;
    uint32_t nbytes;

    // Compressed textures in KTX always have a glFormat of 0.
    if (info.glFormat == 0) {
        assert(info.pixelWidth == info.pixelHeight);
        driver::CompressedPixelDataType datatype = filaweb::toPixelDataType(info.glInternalFormat);
        driver::TextureFormat texformat = filaweb::toTextureFormat(info.glInternalFormat);

        auto texture = Texture::Builder()
                .width(info.pixelWidth)
                .height(info.pixelHeight)
                .levels(nmips)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(texformat)
                .build(engine);

        for (uint32_t level = 0; level < nmips; ++level) {
            asset.texture->getBlob({level}, &data, &nbytes);
            Texture::PixelBufferDescriptor pb(data, nbytes, datatype, nbytes, destroy, uploader);
            texture->setImage(engine, level, std::move(pb));
        }
        app.mi->setParameter(name.c_str(), texture, sampler);
        return texture;
    }

    Texture::Format format;
    switch (info.glTypeSize) {
        case 1: format = Texture::Format::R; break;
        case 2: format = Texture::Format::RG; break;
        case 3: format = Texture::Format::RGB; break;
        case 4: format = Texture::Format::RGBA; break;
    }

    Format internalFormat;
    switch (info.glTypeSize) {
        case 1: internalFormat = Format::R8; break;
        case 2: internalFormat = Format::RG8; break;
        case 3: internalFormat = linear ? Format::RGB8 : Format::SRGB8; break;
        case 4: internalFormat = linear ? Format::RGBA8 : Format::SRGB8_A8; break;
    }

    auto texture = Texture::Builder()
            .width(info.pixelWidth)
            .height(info.pixelHeight)
            .levels(nmips)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(internalFormat)
            .build(engine);

    for (uint32_t level = 0; level < nmips; ++level) {
        asset.texture->getBlob({level}, &data, &nbytes);
        Texture::PixelBufferDescriptor pb(data, nbytes, format, Texture::Type::UBYTE,
                destroy, uploader);
        texture->setImage(engine, level, std::move(pb));
    }

    app.mi->setParameter(name.c_str(), texture, sampler);
    return texture;
}

void setup(Engine* engine, View* view, Scene* scene) {

    // Create material.
    app.mat = Material::Builder()
            .package((void*) MATERIAL_LIT_PACKAGE, sizeof(MATERIAL_LIT_PACKAGE))
            .build(*engine);
    app.mi = app.mat->createInstance();
    app.mi->setParameter("clearCoat", 0.0f);

    // Move raw asset data from JavaScript to C++ static storage. Their held data will be freed via
    // BufferDescriptor callbacks after Filament creates the corresponding GPU objects.
    static auto mesh = filaweb::getRawFile("mesh");
    static auto albedo = filaweb::getTexture("albedo");
    static auto metallic = filaweb::getTexture("metallic");
    static auto roughness = filaweb::getTexture("roughness");
    static auto normal = filaweb::getTexture("normal");
    static auto ao = filaweb::getTexture("ao");

    // Create mesh.
    printf("%s: %d bytes\n", "mesh", mesh.rawSize);
    const uint8_t* mdata = mesh.rawData.get();
    const auto destructor = [](void* buffer, size_t size, void* user) {
        auto asset = (filaweb::Asset*) user;
        asset->rawData.reset();
    };
    app.filamesh = decodeMesh(*engine, mdata, 0, app.mi, destructor, &mesh);
    scene->addEntity(app.filamesh->renderable);

    // Create textures.
    TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR, MagFilter::LINEAR,
            WrapMode::REPEAT);
    auto setTexture = [engine, sampler] (filaweb::Asset& asset, const char* name, bool linear) {
        setTextureParameter(*engine, asset, name, linear, sampler);
    };
    setTexture(albedo, "albedo", false);
    setTexture(metallic, "metallic", false);
    setTexture(roughness, "roughness", false);
    setTexture(normal, "normal", true);
    setTexture(ao, "ao", false);

    // Create the sun.
    auto& em = EntityManager::get();
    app.sun = em.create();
    LightManager::Builder(LightManager::Type::SUN)
            .color(Color::toLinear<ACCURATE>({ 0.98f, 0.92f, 0.89f }))
            .intensity(110000)
            .direction({ 0.7, -1, -0.8 })
            .sunAngularRadius(1.2f)
            .castShadows(true)
            .build(*engine, app.sun);
    scene->addEntity(app.sun);

    // Create point lights.
    em.create(4, app.ptlight);
    LightManager::Builder(LightManager::Type::POINT)
            .color(Color::toLinear<ACCURATE>({0.98f, 0.92f, 0.89f}))
            .intensity(LightManager::EFFICIENCY_LED, 300.0f)
            .position({0.0f, -0.2f, -3.0f})
            .falloff(4.0f)
            .build(*engine, app.ptlight[0]);
    LightManager::Builder(LightManager::Type::POINT)
            .color(Color::toLinear<ACCURATE>({0.98f, 0.12f, 0.19f}))
            .intensity(LightManager::EFFICIENCY_LED, 200.0f)
            .position({0.6f, 0.6f, -3.2f})
            .falloff(2.0f)
            .build(*engine, app.ptlight[1]);
    LightManager::Builder(LightManager::Type::POINT)
            .color(Color::toLinear<ACCURATE>({0.18f, 0.12f, 0.89f}))
            .intensity(LightManager::EFFICIENCY_LED, 200.0f)
            .position({-0.6f, 0.6f, -3.2f})
            .falloff(2.0f)
            .build(*engine, app.ptlight[2]);
    LightManager::Builder(LightManager::Type::POINT)
            .color(Color::toLinear<ACCURATE>({0.88f, 0.82f, 0.29f}))
            .intensity(LightManager::EFFICIENCY_LED, 200.0f)
            .position({0.0f, 1.5f, -3.5f})
            .falloff(2.0f)
            .build(*engine, app.ptlight[3]);
    scene->addEntity(app.ptlight[0]);
    scene->addEntity(app.ptlight[1]);
    scene->addEntity(app.ptlight[2]);
    scene->addEntity(app.ptlight[3]);

    // Create skybox and image-based light source.
    auto skylight = filaweb::getSkyLight(*engine, "syferfontein_18d_clear_2k");
    scene->setIndirectLight(skylight.indirectLight);
    scene->setSkybox(skylight.skybox);
    skylight.indirectLight->setIntensity(100000);

    app.cam = engine->createCamera();
    app.cam->setExposure(16.0f, 1 / 125.0f, 100.0f);
    app.cam->lookAt(float3{0}, float3{0, 0, -4});
    view->setCamera(app.cam);
    view->setClearColor({0.1, 0.125, 0.25, 1.0});
};

void animate(Engine* engine, View* view, double now) {
    using Fov = Camera::Fov;
    const uint32_t width = view->getViewport().width;
    const uint32_t height = view->getViewport().height;
    double ratio = double(width) / height;
    app.cam->setProjection(45.0, ratio, 0.1, 50.0, ratio < 1 ? Fov::HORIZONTAL : Fov::VERTICAL);
    auto& tcm = engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(app.filamesh->renderable),
        mat4f{mat3f{1.0}, float3{0.0f, 0.0f, -4.0f}} *
        mat4f::rotate(now, math::float3{0, 1, 0}));
};

// This is called only after the JavaScript layer has created a WebGL 2.0 context and all assets
// have been downloaded.
extern "C" void launch() {
    filaweb::Application::get()->run(setup, animate);
}

// The main() entry point is implicitly called after JIT compilation, but potentially before the
// WebGL context has been created or assets have finished loading.
int main() { }

