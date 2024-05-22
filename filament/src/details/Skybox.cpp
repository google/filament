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

#include "details/Skybox.h"

#include "details/Engine.h"
#include "details/IndirectLight.h"
#include "details/Material.h"
#include "details/Texture.h"
#include "details/VertexBuffer.h"

#include "FilamentAPI-impl.h"

#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/TextureSampler.h>
#include <filament/Skybox.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>

#include <math/vec4.h>

#include <stdint.h>


#include "generated/resources/materials.h"

using namespace filament::math;
namespace filament {

struct Skybox::BuilderDetails {
    Texture* mEnvironmentMap = nullptr;
    float4 mColor{0, 0, 0, 1};
    float mIntensity = FIndirectLight::DEFAULT_INTENSITY;
    bool mShowSun = false;
};

using BuilderType = Skybox;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;


Skybox::Builder& Skybox::Builder::environment(Texture* cubemap) noexcept {
    mImpl->mEnvironmentMap = cubemap;
    return *this;
}

Skybox::Builder& Skybox::Builder::intensity(float envIntensity) noexcept {
    mImpl->mIntensity = envIntensity;
    return *this;
}

Skybox::Builder& Skybox::Builder::color(math::float4 color) noexcept {
    mImpl->mColor = color;
    return *this;
}

Skybox::Builder& Skybox::Builder::showSun(bool show) noexcept {
    mImpl->mShowSun = show;
    return *this;
}

Skybox* Skybox::Builder::build(Engine& engine) {
    FTexture* cubemap = downcast(mImpl->mEnvironmentMap);

    FILAMENT_CHECK_PRECONDITION(!cubemap || cubemap->isCubemap())
            << "environment maps must be a cubemap";

    return downcast(engine).createSkybox(*this);
}

// ------------------------------------------------------------------------------------------------

FSkybox::FSkybox(FEngine& engine, const Builder& builder) noexcept
        : mSkyboxTexture(downcast(builder->mEnvironmentMap)),
          mRenderableManager(engine.getRenderableManager()),
          mIntensity(builder->mIntensity) {

    FMaterial const* material = engine.getSkyboxMaterial();
    mSkyboxMaterialInstance = material->createInstance("Skybox");

    TextureSampler const sampler(TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
    auto *pInstance = static_cast<MaterialInstance*>(mSkyboxMaterialInstance);
    FTexture const* texture = mSkyboxTexture ? mSkyboxTexture : engine.getDummyCubemap();
    pInstance->setParameter("skybox", texture, sampler);
    pInstance->setParameter("showSun", builder->mShowSun);
    pInstance->setParameter("constantColor", mSkyboxTexture == nullptr);
    pInstance->setParameter("color", builder->mColor);

    mSkybox = engine.getEntityManager().create();

    RenderableManager::Builder(1)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    engine.getFullScreenVertexBuffer(),
                    engine.getFullScreenIndexBuffer())
            .material(0, mSkyboxMaterialInstance)
            .castShadows(false)
            .receiveShadows(false)
            .priority(0x7)
            .culling(false)
            .build(engine, mSkybox);
}

FMaterial const* FSkybox::createMaterial(FEngine& engine) {
    Material::Builder builder;
#ifdef FILAMENT_ENABLE_FEATURE_LEVEL_0
    if (UTILS_UNLIKELY(engine.getActiveFeatureLevel() == Engine::FeatureLevel::FEATURE_LEVEL_0)) {
        builder.package(MATERIALS_SKYBOX_FL0_DATA, MATERIALS_SKYBOX_FL0_SIZE);
    } else
#endif
    {
        switch (engine.getConfig().stereoscopicType) {
            case Engine::StereoscopicType::NONE:
            case Engine::StereoscopicType::INSTANCED:
                builder.package(MATERIALS_SKYBOX_DATA, MATERIALS_SKYBOX_SIZE);
                break;
            case Engine::StereoscopicType::MULTIVIEW:
#ifdef FILAMENT_ENABLE_MULTIVIEW
                builder.package(MATERIALS_SKYBOX_MULTIVIEW_DATA, MATERIALS_SKYBOX_MULTIVIEW_SIZE);
#else
                PANIC_POSTCONDITION("Multiview is enabled in the Engine, but this build has not "
                                    "been compiled for multiview.");
#endif
                break;
        }
    }
    auto material = builder.build(engine);
    return downcast(material);
}

void FSkybox::terminate(FEngine& engine) noexcept {
    // use Engine::destroy because FEngine::destroy is inlined
    Engine& e = engine;
    e.destroy(mSkyboxMaterialInstance);
    e.destroy(mSkybox);

    engine.getEntityManager().destroy(mSkybox);

    mSkyboxMaterialInstance = nullptr;
    mSkybox = {};
}

void FSkybox::setLayerMask(uint8_t select, uint8_t values) noexcept {
    auto& rcm = mRenderableManager;
    rcm.setLayerMask(rcm.getInstance(mSkybox), select, values);
    // we keep a checked version
    mLayerMask = (mLayerMask & ~select) | (values & select);
}

void FSkybox::setColor(math::float4 color) noexcept {
    mSkyboxMaterialInstance->setParameter("color", color);
}

} // namespace filament
