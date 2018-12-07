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
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <filameshio/MeshReader.h>

#include <image/KtxBundle.h>
#include <image/KtxUtility.h>

#include "app/Config.h"
#include "app/FilamentApp.h"
#include "app/IBL.h"

#include "generated/resources/resources.h"
#include "generated/resources/textures.h"

using namespace filament;
using namespace image;
using namespace math;

struct App {
    Material* material;
    MaterialInstance* materialInstance;
    MeshReader::Mesh mesh;
    mat4f transform;
    Texture* albedo;
    Texture* normal;
    Texture* roughness;
    Texture* metallic;
    Texture* ao;
};

static const char* IBL_FOLDER = "envs/venetian_crossroads";

int main(int argc, char** argv) {
    Config config;
    config.title = "suzanne";
    // config.backend = Backend::VULKAN;
    config.iblDirectory = FilamentApp::getRootPath() + IBL_FOLDER;

    App app;
    auto setup = [config, &app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();

        // Create textures. The KTX bundles are freed by KtxUtility.
        auto albedo = new image::KtxBundle(TEXTURES_ALBEDO_S3TC_DATA, TEXTURES_ALBEDO_S3TC_SIZE);
        auto ao = new image::KtxBundle(TEXTURES_AO_DATA, TEXTURES_AO_SIZE);
        auto metallic = new image::KtxBundle(TEXTURES_METALLIC_DATA, TEXTURES_METALLIC_SIZE);
        auto normal = new image::KtxBundle(TEXTURES_NORMAL_DATA, TEXTURES_NORMAL_SIZE);
        auto roughness = new image::KtxBundle(TEXTURES_ROUGHNESS_DATA, TEXTURES_ROUGHNESS_SIZE);
        app.albedo = KtxUtility::createTexture(engine, albedo, true, false);
        app.ao = KtxUtility::createTexture(engine, ao, false, false);
        app.metallic = KtxUtility::createTexture(engine, metallic, false, false);
        app.normal = KtxUtility::createTexture(engine, normal, false, false);
        app.roughness = KtxUtility::createTexture(engine, roughness, false, false);
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
        ibl->setRotation(mat3f::rotate(0.5f, float3{ 0, 1, 0 }));

        // Add geometry into the scene.
        app.mesh = MeshReader::loadMeshFromBuffer(engine, RESOURCES_SUZANNE_DATA, nullptr, nullptr,
                app.materialInstance);
        auto ti = tcm.getInstance(app.mesh.renderable);
        app.transform = mat4f{ mat3f(1), float3(0, 0, -4) } * tcm.getWorldTransform(ti);
        rcm.setCastShadows(rcm.getInstance(app.mesh.renderable), false);
        scene->addEntity(app.mesh.renderable);
        tcm.setTransform(ti, app.transform);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        Fence::waitAndDestroy(engine->createFence());
        engine->destroy(app.materialInstance);
        engine->destroy(app.mesh.renderable);
        engine->destroy(app.material);
        engine->destroy(app.albedo);
        engine->destroy(app.normal);
        engine->destroy(app.roughness);
        engine->destroy(app.metallic);
        engine->destroy(app.ao);
    };

    FilamentApp::get().run(config, setup, cleanup);
}
