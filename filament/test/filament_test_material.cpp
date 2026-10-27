/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "DynamicSpecConstKey.h"
#include "filament_test_resources.h"
#include "MaterialParser.h"

#include "details/Engine.h"
#include "details/Material.h"
#include "details/View.h"

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>

#include <filamat/MaterialBuilder.h>

#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

using namespace filament;

namespace {

std::vector<std::unique_ptr<char[]>> churnHeapForMaterialParser() {
    std::vector<std::unique_ptr<char[]>> churn;
    churn.reserve(128);
    for (int i = 0; i < 128; ++i) {
        auto buf = std::make_unique<char[]>(296);
        std::memset(buf.get(), 0xAA, 296);
        churn.push_back(std::move(buf));
    }
    return churn;
}

#if defined(FILAMENT_SUPPORTS_MOTION_VECTORS)
// Returns the motion vectors variants that FEngine::compile() would prepare for this view.
std::vector<Variant> getMotionVectorsCompileVariants(View const* view, Material const* material,
        utils::tribool const skinning) {
    auto const variants = FEngine::getMaterialCompileVariants(downcast(view), downcast(material),
            /* shadowReceiver= */ utils::tribool(false), skinning);
    std::vector<Variant> result;
    for (auto const v : variants) {
        if (Variant::isMotionVectorsVariant(v)) {
            result.push_back(v);
        }
    }
    return result;
}

Material* buildUnlitMaterial(Engine& engine, BlendingMode const blending,
        UserVariantFilterMask const variantFilter) {
    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("UnlitMaterial");
    builder.shading(Shading::UNLIT);
    builder.blending(blending);
    builder.variantFilter(variantFilter);
    filamat::Package const package = builder.build(engine.getJobSystem());
    if (!package.isValid()) {
        return nullptr;
    }
    return Material::Builder().package(package.getData(), package.getSize()).build(engine);
}
#endif

// Parameters: (enable_program_cache feature flag, whether to duplicate the instance).
class SpecializedInstanceLifetimeTest : public testing::TestWithParam<std::tuple<bool, bool>> {};

} // anonymous namespace

TEST(MaterialTransformName, QuerySamplerWithTransform) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_STREQ(material->getParameterTransformName("sampler"), "transform");

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialTransformName, QueryMultipleSamplersWithTransforms) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_STREQ(material->getParameterTransformName("sampler"), "transform");
    EXPECT_STREQ(material->getParameterTransformName("videoTexture"), "videoTransform");

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialTransformName, QuerySamplerWithoutTransform) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);
    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_EQ(material->getParameterTransformName("sampler2"), nullptr);

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialTransformName, QueryMultipleSamplersWithoutTransforms) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    Material* material = Material::Builder()
                                 .package(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_DATA,
                                         FILAMENT_TEST_RESOURCES_TEST_MATERIAL_TRANSFORMNAME_SIZE)
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_EQ(material->getParameterTransformName("sampler1"), nullptr);
    EXPECT_EQ(material->getParameterTransformName("sampler3"), nullptr);

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, MaterialWithSourceMaterialSuccessfullyRetrieveSource) {
    // Need to set a specific backend to create a proper MaterialParser.
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.);
        }
    )");
    filamat::MaterialBuilder builder;
    builder.init();
    builder.materialSource(shaderCode);
    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_EQ(material->getSource(), shaderCode);

    engine->destroy(material);
    Engine::destroy(engine);
}


TEST(Material, MaterialWithoutSourceMaterialReturnsEmptySource) {
    // Need to set a specific backend to create a proper MaterialParser.
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);
    filamat::MaterialBuilder builder;
    builder.init();
    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    EXPECT_EQ(material->getSource(), "");

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, MaterialSettingValidApiLevelReturnsAnValidPackage) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.setApiLevel(filament::RELEASED_MATERIAL_API_LEVEL);
    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    builder.init();
    builder.setApiLevel(filament::UNSTABLE_MATERIAL_API_LEVEL);
    result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Engine::destroy(engine);
}

TEST(Material, MaterialSettingInvalidApiLevelReturnsAnInvalidPackage) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder builder;
    builder.init();
    // Set the API level higher than unstable, which is illegal.
    builder.setApiLevel(filament::UNSTABLE_MATERIAL_API_LEVEL + 1);
    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_FALSE(result.isValid());

    builder.init();
    // Set the API level lower than the minimum.
    builder.setApiLevel(0);
    result = builder.build(engine->getJobSystem());
    ASSERT_FALSE(result.isValid());

    Engine::destroy(engine);
}

TEST(MaterialInstanceTest, SetConstant) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.0);
        }
    )");

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("MaterialInstanceTest");
    builder.material(shaderCode.c_str());
    builder.constant("myFloat", filamat::MaterialBuilder::ConstantType::FLOAT, 1.0f);
    builder.constant("myInt", filamat::MaterialBuilder::ConstantType::INT, 2);
    builder.constant("myBool", filamat::MaterialBuilder::ConstantType::BOOL, false);

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Material* material = Material::Builder()
            .package(result.getData(), result.getSize())
            .build(*engine);
    ASSERT_NE(material, nullptr);

    MaterialInstance* instance = material->createInstance();
    ASSERT_NE(instance, nullptr);

    // Verify default values
    EXPECT_EQ(instance->getConstant<float>("myFloat"), 1.0f);
    EXPECT_EQ(instance->getConstant<int32_t>("myInt"), 2);
    EXPECT_EQ(instance->getConstant<bool>("myBool"), false);

    // Set new values
    instance->setConstant("myFloat", 3.0f);
    instance->setConstant("myInt", 4);
    instance->setConstant("myBool", true);

    // Verify new values
    EXPECT_EQ(instance->getConstant<float>("myFloat"), 3.0f);
    EXPECT_EQ(instance->getConstant<int32_t>("myInt"), 4);
    EXPECT_EQ(instance->getConstant<bool>("myBool"), true);

    engine->destroy(instance);
    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialInstanceTest, ParameterLargerThanFieldIsRejected) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    std::string shaderCode(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.0);
        }
    )");

    // Layout: "m" at bytes [0, 48), "f" at 48, "g" at 52, "a" at [64, 96).
    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("MaterialInstanceTest");
    builder.material(shaderCode.c_str());
    builder.parameter("m", filamat::MaterialBuilder::UniformType::MAT3);
    builder.parameter("f", filamat::MaterialBuilder::UniformType::FLOAT);
    builder.parameter("g", filamat::MaterialBuilder::UniformType::FLOAT);
    builder.parameter("a", 2, filamat::MaterialBuilder::UniformType::FLOAT);

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Material* material = Material::Builder()
            .package(result.getData(), result.getSize())
            .build(*engine);
    ASSERT_NE(material, nullptr);

    MaterialInstance* instance = material->createInstance();
    ASSERT_NE(instance, nullptr);

    instance->setParameter("m", math::mat3f{ 2.0f });
    EXPECT_EQ(instance->getParameter<math::mat3f>("m"), math::mat3f{ 2.0f });
    math::mat3f const mat[1] = { math::mat3f{ 3.0f } };
    instance->setParameter("m", mat, 1);
    instance->setParameter("f", 1.0f);
    EXPECT_EQ(instance->getParameter<float>("f"), 1.0f);
    float const one[1] = { 1.0f };
    instance->setParameter("f", one, 1);
    instance->setParameter("g", 3.0f);
    float const two[2] = { 1.0f, 2.0f };
    instance->setParameter("a", two, 2);

#if GTEST_HAS_EXCEPTIONS
    // These stay inside the buffer but overrun the next field.
    EXPECT_THROW(instance->setParameter("f", math::float4{ 9.0f }), utils::PreconditionPanic);
    EXPECT_EQ(instance->getParameter<float>("g"), 3.0f);
    EXPECT_THROW(instance->setParameter("f", math::float2{}), utils::PreconditionPanic);
    EXPECT_THROW(instance->setParameter("f", math::mat3f{}), utils::PreconditionPanic);
    EXPECT_THROW(instance->setParameter("m", math::mat4f{}), utils::PreconditionPanic);
    EXPECT_THROW(instance->setParameter("f", two, 2), utils::PreconditionPanic);
    math::mat3f const mats[2] = {};
    EXPECT_THROW(instance->setParameter("m", mats, 2), utils::PreconditionPanic);
    EXPECT_THROW(instance->getParameter<math::float4>("f"), utils::PreconditionPanic);
    EXPECT_THROW(instance->getParameter<math::mat3f>("f"), utils::PreconditionPanic);
    EXPECT_THROW(instance->getParameter<math::mat4f>("m"), utils::PreconditionPanic);

    // This one runs past the end of the buffer.
    float const three[3] = { 1.0f, 2.0f, 3.0f };
    EXPECT_THROW(instance->setParameter("a", three, 3), utils::PreconditionPanic);
#endif

    engine->destroy(instance);
    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, CompileMaterialWithSkinningEnabled) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);

    filamat::MaterialBuilder builder;
    builder.init();

    builder.name("UnlitMaterial");
    builder.shading(Shading::UNLIT);

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    View* view = engine->createView();
    ASSERT_NE(view, nullptr);

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    auto variants = FEngine::getMaterialCompileVariants(
            downcast(view), downcast(material),
            /* shadowReceiver= */ utils::tribool(false),
            /* skinning= */ utils::tribool(true));

    for (auto const v : variants) {
        EXPECT_FALSE(filament::Variant::isShadowReceiverVariant(v));
        EXPECT_TRUE(v.hasSkinningOrMorphing());
    }

    engine->destroy(view);
    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, CompileLitMaterialWithShadowReceiverEnabled) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);

    filamat::MaterialBuilder builder;
    builder.init();

    builder.name("LitMaterial");
    builder.shading(Shading::LIT);

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    View* view = engine->createView();
    ASSERT_NE(view, nullptr);

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    auto variants = FEngine::getMaterialCompileVariants(
            downcast(view), downcast(material),
            /* shadowReceiver= */ utils::tribool(true),
            /* skinning= */ utils::tribool(false));

    // Verify that SRE is successfully generated for the lit material.
    bool hasShadowReceiver = false;
    for (auto const v : variants) {
        if (filament::Variant::isShadowReceiverVariant(v)) {
            hasShadowReceiver = true;
            break;
        }
    }
    EXPECT_TRUE(hasShadowReceiver);

    FEngine const& fengine = downcast(*engine);
    MaterialDefinition const& definition = downcast(material)->getDefinition();
    DynamicSpecConstKey dynamicLighting;
    dynamicLighting.setDynamicLighting(true);

    EXPECT_TRUE(definition.isValidProgram(
            Variant(Variant::S2D | Variant::SRE), DynamicSpecConstKey{},
            fengine.getShaderModel(), fengine.isStereoSupported()));
    EXPECT_TRUE(definition.isValidProgram(
            Variant(Variant::S2D | Variant::SRE), dynamicLighting,
            fengine.getShaderModel(), fengine.isStereoSupported()));
    EXPECT_TRUE(definition.isValidProgram(
            Variant(Variant::SPECIAL_SSR_VARIANT), DynamicSpecConstKey{},
            fengine.getShaderModel(), fengine.isStereoSupported()));

    engine->destroy(view);
    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialVariant, DynamicLightingSpecKeySupportsPunctualShadowReceivers) {
    DynamicSpecConstKey dynamicLighting;
    dynamicLighting.setDynamicLighting(true);

    constexpr Variant::type_t SHADOW_VARIANTS[] = {
            Variant::SRE,
            Variant::SRE | Variant::SKN,
            Variant::S2D | Variant::SRE,
            Variant::S2D | Variant::SRE | Variant::SKN,
            Variant::S2D | Variant::SRE | Variant::STE,
    };

    for (Variant::type_t const key : SHADOW_VARIANTS) {
        Variant const variant(key);
        EXPECT_FALSE(Variant::isSSRVariant(variant));
        EXPECT_TRUE(Variant::isShadowReceiverVariant(variant));
        EXPECT_TRUE(DynamicSpecConstKey::filterProgramSpecKey(
                variant, dynamicLighting, MaterialDomain::SURFACE, true).hasDynamicLighting());
    }

    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant(Variant::SPECIAL_SSR_VARIANT), dynamicLighting,
            MaterialDomain::SURFACE, true).hasDynamicLighting());
    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant(Variant::DEPTH_VARIANT), dynamicLighting,
            MaterialDomain::SURFACE, true).hasDynamicLighting());
    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant(Variant::S2D | Variant::SRE), dynamicLighting,
            MaterialDomain::SURFACE, false).hasDynamicLighting());
}

TEST(MaterialVariant, DirectionalLightingSpecKeyExcludesDepthAndSsr) {
    // Variant::DIR used to carry directional lighting, and Variant::filterVariant() cleared it on
    // depth and SSR variants. Now that the bit lives in the specialization key, the same exclusion
    // has to hold there instead.
    DynamicSpecConstKey directionalLighting;
    directionalLighting.setDirectionalLighting(true);

    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant(Variant::SPECIAL_SSR_VARIANT), directionalLighting,
            MaterialDomain::SURFACE, true).hasDirectionalLighting());
    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant(Variant::DEPTH_VARIANT), directionalLighting,
            MaterialDomain::SURFACE, true).hasDirectionalLighting());
    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant(Variant::S2D | Variant::SRE), directionalLighting,
            MaterialDomain::SURFACE, false).hasDirectionalLighting());
    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant{}, directionalLighting,
            MaterialDomain::POST_PROCESS, true).hasDirectionalLighting());

    // An ordinary lit surface variant keeps it, otherwise the assertions above prove nothing.
    EXPECT_TRUE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant(Variant::S2D | Variant::SRE), directionalLighting,
            MaterialDomain::SURFACE, true).hasDirectionalLighting());
}

TEST(MaterialVariant, ExtraDirectionalLightsRequireDirectionalLighting) {
    DynamicSpecConstKey extraOnly;
    extraOnly.setExtraDirectionalLights(true);

    EXPECT_FALSE(DynamicSpecConstKey::isValidProgramSpecKey(
            Variant{}, extraOnly, MaterialDomain::SURFACE, true));

    DynamicSpecConstKey const filtered = DynamicSpecConstKey::filterProgramSpecKey(
            Variant{}, extraOnly, MaterialDomain::SURFACE, true);
    EXPECT_FALSE(filtered.hasExtraDirectionalLights());

    DynamicSpecConstKey directionalAndExtra;
    directionalAndExtra.setDirectionalLighting(true);
    directionalAndExtra.setExtraDirectionalLights(true);
    EXPECT_TRUE(DynamicSpecConstKey::isValidProgramSpecKey(
            Variant{}, directionalAndExtra, MaterialDomain::SURFACE, true));

    DynamicSpecConstKey const retained = DynamicSpecConstKey::filterProgramSpecKey(
            Variant{}, directionalAndExtra, MaterialDomain::SURFACE, true);
    EXPECT_TRUE(retained.hasDirectionalLighting());
    EXPECT_TRUE(retained.hasExtraDirectionalLights());

    auto const validKeys = DynamicSpecConstKey::getValidKeys(
            Variant{}, MaterialDomain::SURFACE, true);
    EXPECT_EQ(validKeys.size, 12);
    for (auto const key : validKeys) {
        EXPECT_FALSE(key.hasExtraDirectionalLights() && !key.hasDirectionalLighting());
    }
}

TEST(MaterialVariant, FogSpecKeyIsOnlyValidForColorSurfaceVariants) {
    DynamicSpecConstKey fog;
    fog.setFog(true);

    // Fog applies to both lit and unlit surface materials.
    for (bool const isLit : { false, true }) {
        EXPECT_TRUE(DynamicSpecConstKey::isValidProgramSpecKey(
                Variant{}, fog, MaterialDomain::SURFACE, isLit));
        EXPECT_TRUE(DynamicSpecConstKey::isValidProgramSpecKey(
                Variant(Variant::SRE | Variant::SKN), fog, MaterialDomain::SURFACE, isLit));
        EXPECT_TRUE(DynamicSpecConstKey::filterProgramSpecKey(
                Variant{}, fog, MaterialDomain::SURFACE, isLit).hasFog());
    }

    // ...but never to depth, SSR or post-process programs.
    for (Variant::type_t const key : { Variant::DEPTH_VARIANT,
                 Variant::type_t(Variant::DEP | Variant::PCK), Variant::SPECIAL_SSR_VARIANT }) {
        EXPECT_FALSE(DynamicSpecConstKey::isValidProgramSpecKey(
                Variant(key), fog, MaterialDomain::SURFACE, true)) << +key;
        EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
                Variant(key), fog, MaterialDomain::SURFACE, true).hasFog()) << +key;
    }
    EXPECT_FALSE(DynamicSpecConstKey::isValidProgramSpecKey(
            Variant{}, fog, MaterialDomain::POST_PROCESS, false));
    EXPECT_FALSE(DynamicSpecConstKey::filterProgramSpecKey(
            Variant{}, fog, MaterialDomain::POST_PROCESS, false).hasFog());

    // The FOG user variant filter removes the fog specialization.
    EXPECT_FALSE(DynamicSpecConstKey::filterUserVariant(
            fog, UserVariantFilterMask(UserVariantFilterBit::FOG)).hasFog());
    EXPECT_TRUE(DynamicSpecConstKey::filterUserVariant(
            fog, UserVariantFilterMask(UserVariantFilterBit::SSR)).hasFog());

    // Unlit surface materials only have the fog specialization.
    auto const unlitKeys = DynamicSpecConstKey::getValidKeys(
            Variant{}, MaterialDomain::SURFACE, false);
    EXPECT_EQ(unlitKeys.size, 2);
}

TEST(Material, SsrFilteredLitMaterialContainsPunctualShadowPrograms) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("LitMaterialWithoutSsr");
    builder.shading(Shading::LIT);
    builder.variantFilter(uint32_t(UserVariantFilterBit::SSR));

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    FEngine const& fengine = downcast(*engine);
    MaterialDefinition const& definition = downcast(material)->getDefinition();
    DynamicSpecConstKey dynamicLighting;
    dynamicLighting.setDynamicLighting(true);
    DynamicSpecConstKey fog;
    fog.setFog(true);
    DynamicSpecConstKey dynamicLightingAndFog = dynamicLighting;
    dynamicLightingAndFog.setFog(true);
    constexpr Variant::type_t SHADOW_VARIANTS[] = {
            Variant::SRE,
            Variant::SRE | Variant::SKN,
            Variant::S2D | Variant::SRE,
            Variant::S2D | Variant::SRE | Variant::SKN,
    };

    for (Variant::type_t const key : SHADOW_VARIANTS) {
        for (DynamicSpecConstKey const specKey :
                { DynamicSpecConstKey{}, dynamicLighting, fog, dynamicLightingAndFog }) {
            EXPECT_TRUE(definition.isValidProgram(
                    Variant(key), specKey, fengine.getShaderModel(),
                    fengine.isStereoSupported()))
                    << "variant=" << +key << " specKey=" << specKey.key;
        }
    }
    EXPECT_FALSE(definition.isValidProgram(
            Variant(Variant::SPECIAL_SSR_VARIANT), DynamicSpecConstKey{},
            fengine.getShaderModel(), fengine.isStereoSupported()));

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, FogFilteredMaterialRejectsFogSpecKey) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("LitMaterialWithoutFog");
    builder.shading(Shading::LIT);
    builder.variantFilter(uint32_t(UserVariantFilterBit::FOG));

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    FEngine const& fengine = downcast(*engine);
    MaterialDefinition const& definition = downcast(material)->getDefinition();
    DynamicSpecConstKey dynamicLighting;
    dynamicLighting.setDynamicLighting(true);
    DynamicSpecConstKey fog;
    fog.setFog(true);
    DynamicSpecConstKey dynamicLightingAndFog = dynamicLighting;
    dynamicLightingAndFog.setFog(true);

    for (Variant::type_t const key : { Variant::type_t(0), Variant::SRE,
                 Variant::type_t(Variant::S2D | Variant::SRE) }) {
        EXPECT_TRUE(definition.isValidProgram(Variant(key), DynamicSpecConstKey{},
                fengine.getShaderModel(), fengine.isStereoSupported())) << +key;
        EXPECT_TRUE(definition.isValidProgram(Variant(key), dynamicLighting,
                fengine.getShaderModel(), fengine.isStereoSupported())) << +key;
        // The material has no fog code; the fog specialization would be a duplicate program.
        EXPECT_FALSE(definition.isValidProgram(Variant(key), fog,
                fengine.getShaderModel(), fengine.isStereoSupported())) << +key;
        EXPECT_FALSE(definition.isValidProgram(Variant(key), dynamicLightingAndFog,
                fengine.getShaderModel(), fengine.isStereoSupported())) << +key;
    }

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, SsrFilteredShadowMultiplierContainsPunctualShadowPrograms) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("ShadowMultiplierWithoutSsr");
    builder.shading(Shading::UNLIT);
    builder.shadowMultiplier(true);
    builder.variantFilter(uint32_t(UserVariantFilterBit::SSR));

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    FEngine const& fengine = downcast(*engine);
    MaterialDefinition const& definition = downcast(material)->getDefinition();
    DynamicSpecConstKey dynamicLighting;
    dynamicLighting.setDynamicLighting(true);

    EXPECT_TRUE(definition.isValidProgram(
            Variant(Variant::SRE), dynamicLighting,
            fengine.getShaderModel(), fengine.isStereoSupported()));
    EXPECT_TRUE(definition.isValidProgram(
            Variant(Variant::S2D | Variant::SRE), dynamicLighting,
            fengine.getShaderModel(), fengine.isStereoSupported()));
    EXPECT_FALSE(definition.isValidProgram(
            Variant(Variant::SPECIAL_SSR_VARIANT), DynamicSpecConstKey{},
            fengine.getShaderModel(), fengine.isStereoSupported()));

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, CompileUnlitMaterialShadowMultiplierWithShadowReceiverEnabled) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);

    filamat::MaterialBuilder builder;
    builder.init();

    builder.name("UnlitMaterial");
    builder.shading(Shading::UNLIT);
    // This is necessary for the shadow receiver variant to be generated.
    builder.shadowMultiplier(true);

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    View* view = engine->createView();
    ASSERT_NE(view, nullptr);

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    auto variants = FEngine::getMaterialCompileVariants(
            downcast(view), downcast(material),
            /* shadowReceiver= */ utils::tribool(true),
            /* skinning= */ utils::tribool(false));

    // Verify that SRE is successfully generated for the lit material.
    bool hasShadowReceiver = false;
    for (auto const v : variants) {
        if (filament::Variant::isShadowReceiverVariant(v)) {
            hasShadowReceiver = true;
            break;
        }
    }
    EXPECT_TRUE(hasShadowReceiver);

    engine->destroy(view);
    engine->destroy(material);
    Engine::destroy(engine);
}

#if defined(FILAMENT_SUPPORTS_MOTION_VECTORS)
TEST(Material, CompileVariantsIncludeMotionVectorsWhenEnabled) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("LitMaterial");
    builder.shading(Shading::LIT);

    filamat::Package result = builder.build(engine->getJobSystem());
    ASSERT_TRUE(result.isValid());

    View* view = engine->createView();
    ASSERT_NE(view, nullptr);

    Material* material = Material::Builder()
                                 .package(result.getData(), result.getSize())
                                 .build(*engine);
    ASSERT_NE(material, nullptr);

    FEngine const& fengine = downcast(*engine);
    FView const* const fview = downcast(view);
    Variant const motionVectors{ Variant::SPECIAL_MOT_VARIANT };
    Variant const skinnedMotionVectors{ Variant::SPECIAL_MOT_VARIANT | Variant::SKN };

    // Motion vectors are disabled by default.
    EXPECT_FALSE(view->getMotionVectorsOptions().enabled);
    EXPECT_FALSE(fview->evaluateMotionVectorsActive(fengine));
    EXPECT_TRUE(getMotionVectorsCompileVariants(view, material, utils::tribool{}).empty());

    View::MotionVectorsOptions options;
    options.enabled = true;
    view->setMotionVectorsOptions(options);
    EXPECT_TRUE(view->getMotionVectorsOptions().enabled);
    EXPECT_TRUE(fview->evaluateMotionVectorsActive(fengine));
    // The per-frame state is only latched by prepare().
    EXPECT_FALSE(fview->isMotionVectorsActive());

    auto variants = getMotionVectorsCompileVariants(view, material, utils::tribool(false));
    ASSERT_EQ(variants.size(), 1u);
    EXPECT_EQ(variants[0], motionVectors);

    variants = getMotionVectorsCompileVariants(view, material, utils::tribool(true));
    ASSERT_EQ(variants.size(), 1u);
    EXPECT_EQ(variants[0], skinnedMotionVectors);

    variants = getMotionVectorsCompileVariants(view, material, utils::tribool{});
    ASSERT_EQ(variants.size(), 2u);
    EXPECT_EQ(variants[0], motionVectors);
    EXPECT_EQ(variants[1], skinnedMotionVectors);

    // Worst case: 4 color + 2 depth + 2 motion vectors variants.
    auto const allVariants = FEngine::getMaterialCompileVariants(fview, downcast(material),
            /* shadowReceiver= */ utils::tribool{}, /* skinning= */ utils::tribool{});
    EXPECT_EQ(allVariants.size(), 8u);

    // Standard materials omit motion vectors shaders from their package and render them using the
    // default material's programs.
    MaterialDefinition const& definition = downcast(material)->getDefinition();
    EXPECT_FALSE(definition.hasCustomDepthShader);
    for (Variant const variant : { motionVectors, skinnedMotionVectors }) {
        EXPECT_FALSE(definition.isValidProgram(variant, DynamicSpecConstKey{},
                fengine.getShaderModel(), fengine.isStereoSupported()));
        EXPECT_TRUE(downcast(material)->isValidProgram(variant, DynamicSpecConstKey{},
                fengine.getShaderModel(), fengine.isStereoSupported()));
    }
    EXPECT_TRUE(downcast(material)->hasMotionVectorsVariant());
    ASSERT_NE(fengine.getDefaultMaterial(), nullptr);
    EXPECT_TRUE(fengine.getDefaultMaterial()->hasMotionVectorsVariant());

    // Motion vectors are only consumed by post-processing.
    view->setPostProcessingEnabled(false);
    EXPECT_FALSE(fview->evaluateMotionVectorsActive(fengine));
    EXPECT_TRUE(getMotionVectorsCompileVariants(view, material, utils::tribool{}).empty());

    engine->destroy(view);
    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, HasMotionVectorsVariantFollowsRenderedPrograms) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    constexpr auto motionVectorsFilter = uint32_t(UserVariantFilterBit::MOTION_VECTORS);

    // MASKED materials have their own depth programs, including the motion vectors ones.
    Material* masked = buildUnlitMaterial(*engine, BlendingMode::MASKED, 0);
    ASSERT_NE(masked, nullptr);
    EXPECT_TRUE(downcast(masked)->getDefinition().hasCustomDepthShader);
    EXPECT_TRUE(downcast(masked)->hasMotionVectorsVariant());

    // ...unless they're filtered out, in which case the motion vectors pass must draw them
    // depth-only.
    Material* maskedFiltered =
            buildUnlitMaterial(*engine, BlendingMode::MASKED, motionVectorsFilter);
    ASSERT_NE(maskedFiltered, nullptr);
    EXPECT_FALSE(downcast(maskedFiltered)->hasMotionVectorsVariant());

    // OPAQUE materials use the default material's depth programs, so the filter only shrinks
    // the package.
    Material* opaqueFiltered =
            buildUnlitMaterial(*engine, BlendingMode::OPAQUE, motionVectorsFilter);
    ASSERT_NE(opaqueFiltered, nullptr);
    EXPECT_FALSE(downcast(opaqueFiltered)->getDefinition().hasCustomDepthShader);
    EXPECT_TRUE(downcast(opaqueFiltered)->hasMotionVectorsVariant());

    engine->destroy(masked);
    engine->destroy(maskedFiltered);
    engine->destroy(opaqueFiltered);
    Engine::destroy(engine);
}

TEST(Material, MotionVectorsCompileVariantsFollowRenderedPrograms) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    FEngine const& fengine = downcast(*engine);
    auto const sm = fengine.getShaderModel();
    bool const stereo = fengine.isStereoSupported();
    constexpr auto motionVectorsFilter = uint32_t(UserVariantFilterBit::MOTION_VECTORS);

    View* view = engine->createView();
    ASSERT_NE(view, nullptr);
    View::MotionVectorsOptions options;
    options.enabled = true;
    view->setMotionVectorsOptions(options);

    // A material with its own motion vectors programs gets both warm-up variants.
    Material* masked = buildUnlitMaterial(*engine, BlendingMode::MASKED, 0);
    ASSERT_NE(masked, nullptr);
    EXPECT_EQ(getMotionVectorsCompileVariants(view, masked, utils::tribool{}).size(), 2u);

    // A material without them is always drawn depth-only in the motion vectors pass: no warm-up.
    Material* maskedFiltered =
            buildUnlitMaterial(*engine, BlendingMode::MASKED, motionVectorsFilter);
    ASSERT_NE(maskedFiltered, nullptr);
    EXPECT_TRUE(getMotionVectorsCompileVariants(view, maskedFiltered, utils::tribool{}).empty());
    EXPECT_FALSE(downcast(maskedFiltered)->isValidProgram(Variant{ Variant::SPECIAL_MOT_VARIANT },
            DynamicSpecConstKey{}, sm, stereo));

    // A material that renders them with the default material's programs gets both warm-up
    // variants, which are valid programs for it although its package doesn't have them, so that
    // Material::compile() prepares them.
    Material* opaqueFiltered =
            buildUnlitMaterial(*engine, BlendingMode::OPAQUE, motionVectorsFilter);
    ASSERT_NE(opaqueFiltered, nullptr);
    auto const variants = getMotionVectorsCompileVariants(view, opaqueFiltered, utils::tribool{});
    ASSERT_EQ(variants.size(), 2u);
    for (Variant const variant : variants) {
        EXPECT_FALSE(downcast(opaqueFiltered)->getDefinition().isValidProgram(variant,
                DynamicSpecConstKey{}, sm, stereo));
        EXPECT_TRUE(downcast(opaqueFiltered)->isValidProgram(variant, DynamicSpecConstKey{}, sm,
                stereo));
    }

    // Translucent materials keep them, since their instances can write depth.
    Material* translucent = buildUnlitMaterial(*engine, BlendingMode::TRANSPARENT, 0);
    ASSERT_NE(translucent, nullptr);
    EXPECT_EQ(getMotionVectorsCompileVariants(view, translucent, utils::tribool{}).size(), 2u);

    engine->destroy(view);
    engine->destroy(masked);
    engine->destroy(maskedFiltered);
    engine->destroy(opaqueFiltered);
    engine->destroy(translucent);
    Engine::destroy(engine);
}

TEST(Material, MotionVectorsOmittedWhenSharedOrFragmentStandard) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);
    FEngine& fengine = downcast(*engine);
    FEngine::DriverApi& driver = fengine.getDriverApi();
    auto const sm = fengine.getShaderModel();
    bool const stereo = fengine.isStereoSupported();

    Variant const mot{ Variant::SPECIAL_MOT_VARIANT };
    Variant const skinnedMot{ Variant::SPECIAL_MOT_VARIANT | Variant::SKN };

    // 1. A material with custom varyings and a vertex shader that only modifies UVs/custom
    // varyings (not vertex position) has hasCustomDepthShader == false and omits both MOT VS and
    // MOT FS from its package, sharing the default material's programs.
    {
        filamat::MaterialBuilder builder;
        builder.init();
        builder.name("VaryingOnlyVertexMaterial")
                .shading(Shading::UNLIT)
                .variable(filamat::MaterialBuilder::Variable::CUSTOM0, "localPos")
                .materialVertex(R"(
                    void materialVertex(inout MaterialVertexInputs material) {
                        material.localPos = getPosition();
                    }
                )")
                .material(R"(
                    void material(inout MaterialInputs material) {
                        prepareMaterial(material);
                        material.baseColor = variable_localPos;
                    }
                )");
        filamat::Package const pkg = builder.build(engine->getJobSystem());
        ASSERT_TRUE(pkg.isValid());
        Material* mat = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
        ASSERT_NE(mat, nullptr);
        FMaterial* fmat = downcast(mat);
        MaterialParser const& parser = fmat->getDefinition().getMaterialParser();

        EXPECT_FALSE(fmat->getDefinition().hasCustomDepthShader);
        EXPECT_FALSE(parser.hasShader(sm, mot, backend::ShaderStage::VERTEX));
        EXPECT_FALSE(parser.hasShader(sm, skinnedMot, backend::ShaderStage::VERTEX));
        EXPECT_FALSE(parser.hasShader(sm, mot, backend::ShaderStage::FRAGMENT));
        EXPECT_TRUE(fmat->isSharedVariant(mot));
        EXPECT_TRUE(fmat->isSharedVariant(skinnedMot));
        EXPECT_TRUE(fmat->hasMotionVectorsVariant());

        engine->destroy(mat);
    }

    // 2. A material whose vertex shader modifies worldPosition but whose fragment shader is
    // standard (OPAQUE, no discard or gl_FragDepth) has hasCustomDepthShader == true, emits its
    // custom MOT vertex shaders, omits the MOT fragment shader, and links against the default
    // material's MOT fragment shader.
    {
        filamat::MaterialBuilder builder;
        builder.init();
        builder.name("PositionDisplacingVertexMaterial")
                .shading(Shading::UNLIT)
                .materialVertex(R"(
                    void materialVertex(inout MaterialVertexInputs material) {
                        material.worldPosition.xyz += vec3(0.0, 1.0, 0.0);
                    }
                )");
        filamat::Package const pkg = builder.build(engine->getJobSystem());
        ASSERT_TRUE(pkg.isValid());
        Material* mat = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
        ASSERT_NE(mat, nullptr);
        FMaterial* fmat = downcast(mat);
        MaterialParser const& parser = fmat->getDefinition().getMaterialParser();

        EXPECT_TRUE(fmat->getDefinition().hasCustomDepthShader);
        EXPECT_TRUE(parser.hasShader(sm, mot, backend::ShaderStage::VERTEX));
        EXPECT_TRUE(parser.hasShader(sm, skinnedMot, backend::ShaderStage::VERTEX));
        EXPECT_FALSE(parser.hasShader(sm, mot, backend::ShaderStage::FRAGMENT));
        EXPECT_FALSE(fmat->isSharedVariant(mot));
        EXPECT_TRUE(fmat->getDefinition().isValidProgram(mot, DynamicSpecConstKey{}, sm, stereo));
        EXPECT_TRUE(fmat->getDefinition().isValidProgram(skinnedMot, DynamicSpecConstKey{}, sm,
                stereo));
        EXPECT_TRUE(fmat->hasMotionVectorsVariant());
        EXPECT_TRUE(bool(fmat->getPrograms().prepareProgram(
                driver, mot, DynamicSpecConstKey{}, backend::CompilerPriorityQueue::HIGH)));
        EXPECT_TRUE(bool(fmat->getPrograms().prepareProgram(
                driver, skinnedMot, DynamicSpecConstKey{}, backend::CompilerPriorityQueue::HIGH)));

        engine->destroy(mat);
    }

    // 3. A material whose fragment shader calls discard in OPAQUE mode has a custom depth fragment
    // shader, so its package retains both MOT VS and MOT FS.
    {
        filamat::MaterialBuilder builder;
        builder.init();
        builder.name("CustomDepthDiscardMaterial")
                .shading(Shading::UNLIT)
                .material(R"(
                    void material(inout MaterialInputs material) {
                        prepareMaterial(material);
                        if (gl_FragCoord.x < 0.0) {
                            discard;
                        }
                    }
                )");
        filamat::Package const pkg = builder.build(engine->getJobSystem());
        ASSERT_TRUE(pkg.isValid());
        Material* mat = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
        ASSERT_NE(mat, nullptr);
        FMaterial* fmat = downcast(mat);
        MaterialParser const& parser = fmat->getDefinition().getMaterialParser();

        EXPECT_TRUE(fmat->getDefinition().hasCustomDepthShader);
        EXPECT_TRUE(parser.hasShader(sm, mot, backend::ShaderStage::VERTEX));
        EXPECT_TRUE(parser.hasShader(sm, skinnedMot, backend::ShaderStage::VERTEX));
        EXPECT_TRUE(parser.hasShader(sm, mot, backend::ShaderStage::FRAGMENT));
        EXPECT_TRUE(fmat->hasMotionVectorsVariant());

        engine->destroy(mat);
    }

    Engine::destroy(engine);
}
#endif

TEST(Material, FeatureLevel0PackageHasNoEssl1MotionVectorsShaders) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("FeatureLevel0Material")
            .shading(Shading::UNLIT)
            .featureLevel(backend::FeatureLevel::FEATURE_LEVEL_0)
            .isDefaultMaterial(true)
            .platform(filamat::MaterialBuilder::Platform::MOBILE)
            .targetApi(filamat::MaterialBuilder::TargetApi::OPENGL);
    filamat::Package const package = builder.build(engine->getJobSystem());
    Engine::destroy(engine);
    ASSERT_TRUE(package.isValid());

    using backend::ShaderStage;
    constexpr backend::ShaderModel sm = backend::ShaderModel::MOBILE;
    Variant const depth{ Variant::DEP };
    Variant const motionVectors{ Variant::SPECIAL_MOT_VARIANT };
    Variant const skinnedMotionVectors{ Variant::SPECIAL_MOT_VARIANT | Variant::SKN };

    // The ESSL3 shaders are used at feature level 1 and above and include motion vectors.
    MaterialParser essl3({ backend::ShaderLanguage::ESSL3 }, package.getData(), package.getSize());
    ASSERT_EQ(essl3.parse(), MaterialParser::ParseResult::SUCCESS);
    EXPECT_TRUE(essl3.hasShader(sm, motionVectors, ShaderStage::VERTEX));
    EXPECT_TRUE(essl3.hasShader(sm, skinnedMotionVectors, ShaderStage::VERTEX));
    EXPECT_TRUE(essl3.hasShader(sm, motionVectors, ShaderStage::FRAGMENT));

    // The ESSL1 shaders are only used at feature level 0, which doesn't support motion vectors.
    MaterialParser essl1({ backend::ShaderLanguage::ESSL1 }, package.getData(), package.getSize());
    ASSERT_EQ(essl1.parse(), MaterialParser::ParseResult::SUCCESS);
    EXPECT_TRUE(essl1.hasShader(sm, depth, ShaderStage::VERTEX));
    EXPECT_TRUE(essl1.hasShader(sm, depth, ShaderStage::FRAGMENT));
    EXPECT_FALSE(essl1.hasShader(sm, motionVectors, ShaderStage::VERTEX));
    EXPECT_FALSE(essl1.hasShader(sm, skinnedMotionVectors, ShaderStage::VERTEX));
    EXPECT_FALSE(essl1.hasShader(sm, motionVectors, ShaderStage::FRAGMENT));
}

TEST(Material, MaterialCacheDestroyTriggersUAF) {
    Engine::Config config;
    config.materialCacheCapacity = 4;

    Engine* engine = Engine::Builder().backend(Engine::Backend::NOOP).config(&config).build();
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("MaterialCacheLruHitPathA");
    builder.shading(Shading::UNLIT);
    filamat::Package pkg = builder.build(engine->getJobSystem());
    ASSERT_TRUE(pkg.isValid());

    // Step 1: Initial Build (Factory Path) -> mDefinitions stores {key1{&Parser_1} -> Def_1}
    Material* mat1 = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(mat1, nullptr);

    // Step 2: Destroy -> Refcount drops to 0, Def_1 moves to LRU cache with key1{&Parser_1}
    engine->destroy(mat1);

    // Step 3: Rebuild with identical bytes -> LRU cache hits, pops Def_1.
    Material* mat2 = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(mat2, nullptr);

    auto churn = churnHeapForMaterialParser();

    // Step 4 (Path A): Destroy mat2 directly.
    // Calls mDefinitions.release(key_r{&Parser_1}), triggering mMap.find(key_r) which compares
    // *Parser_1 == *dangling_Parser_2 (Heap UAF read / FILAMENT_CHECK_PRECONDITION abort).
    engine->destroy(mat2);

    Engine::destroy(engine);
}

TEST(Material, MaterialCacheThirdBuildTriggersUAFAndCorruption) {
    Engine::Config config;
    config.materialCacheCapacity = 4;

    Engine* engine = Engine::Builder().backend(Engine::Backend::NOOP).config(&config).build();
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("MaterialCacheLruHitPathB");
    builder.shading(Shading::UNLIT);
    filamat::Package pkg = builder.build(engine->getJobSystem());
    ASSERT_TRUE(pkg.isValid());

    // Step 1: Initial Build (Factory Path)
    Material* mat1 = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(mat1, nullptr);

    // Step 2: Destroy (LRU Population)
    engine->destroy(mat1);

    // Step 3: Rebuild (LRU Hit)
    Material* mat2 = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(mat2, nullptr);

    auto churn = churnHeapForMaterialParser();

    // Step 4 (Path B): Build a third material from the same package bytes.
    // mDefinitions.acquire calls mMap.find(key3{&Parser_3}), comparing *Parser_3 ==
    // *dangling_Parser_2.
    Material* mat3 = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(mat3, nullptr);
    EXPECT_EQ(&downcast(mat2)->getDefinition(), &downcast(mat3)->getDefinition());

    engine->destroy(mat2);
    engine->destroy(mat3);

    Engine::destroy(engine);
}

TEST(Material, ProgramCacheLruEvictUAF) {
    Engine::Config config;
    config.programCacheCapacity = 1;

    Engine* engine = Engine::Builder()
                             .backend(Engine::Backend::NOOP)
                             .config(&config)
                             .feature("engine.enable_program_cache", true)
                             .build();
    ASSERT_NE(engine, nullptr);

    std::string shaderCode1(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )");
    filamat::MaterialBuilder builder1;
    builder1.init();
    builder1.name("Material1");
    builder1.material(shaderCode1.c_str());
    builder1.constant("myFloat", filamat::MaterialBuilder::ConstantType::FLOAT, 1.0f);
    filamat::Package pkg1 = builder1.build(engine->getJobSystem());
    ASSERT_TRUE(pkg1.isValid());

    std::string shaderCode2(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
    )");
    filamat::MaterialBuilder builder2;
    builder2.init();
    builder2.name("MaterialB");
    builder2.material(shaderCode2.c_str());
    builder2.constant("myInt", filamat::MaterialBuilder::ConstantType::INT, 42);
    filamat::Package pkg2 = builder2.build(engine->getJobSystem());
    ASSERT_TRUE(pkg2.isValid());

    // 1. Build, prepare program, and destroy Material A: populates LRU cache with compiled program
    // and releases LocalProgramCache's InternPool ref.
    Material* mat1 = Material::Builder().package(pkg1.getData(), pkg1.getSize()).build(*engine);
    ASSERT_NE(mat1, nullptr);
    downcast(mat1)->getPrograms().prepareProgram(downcast(*engine).getDriverApi(), Variant{ 0 },
            DynamicSpecConstKey{ 0 }, backend::CompilerPriorityQueue::HIGH);
    engine->destroy(mat1);

    // 2. Build, prepare program, and destroy Material B: exceeds LRU capacity (1), evicting
    // Material A's program from LRU. Eviction hashes Material A's key. Without pinning, this
    // triggers a heap-use-after-free in Slice::hash().
    Material* mat2 = Material::Builder().package(pkg2.getData(), pkg2.getSize()).build(*engine);
    ASSERT_NE(mat2, nullptr);
    downcast(mat2)->getPrograms().prepareProgram(downcast(*engine).getDriverApi(), Variant{ 0 },
            DynamicSpecConstKey{ 0 }, backend::CompilerPriorityQueue::HIGH);
    engine->destroy(mat2);

    Engine::destroy(engine);
}

TEST_P(SpecializedInstanceLifetimeTest, DestroyInstanceKeepsMaterialDefinition) {
    auto const [enableProgramCache, duplicateInstance] = GetParam();

    Engine* engine = Engine::Builder()
                             .backend(Engine::Backend::NOOP)
                             .feature("engine.enable_program_cache", enableProgramCache)
                             .build();
    ASSERT_NE(engine, nullptr);
    FEngine::DriverApi& driver = downcast(engine)->getDriverApi();

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("SpecializedInstanceLifetime");
    builder.material(R"(
        void material(inout MaterialInputs material) {
            prepareMaterial(material);
            material.baseColor = vec4(1.0);
        }
    )");
    builder.constant("probe", filamat::MaterialBuilder::ConstantType::BOOL, false);
    filamat::Package pkg = builder.build(engine->getJobSystem());
    ASSERT_TRUE(pkg.isValid());

    Material* material = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(material, nullptr);

    // prepareProgram() flushes the pending constants, which initializes the instance's own
    // program cache (this is what rendering a frame does).
    MaterialInstance* instance = material->createInstance();
    instance->setConstant("probe", true);
    downcast(instance)->prepareProgram(driver, Variant{ 0 }, DynamicSpecConstKey{ 0 },
            backend::CompilerPriorityQueue::HIGH);

    if (duplicateInstance) {
        MaterialInstance* copy = MaterialInstance::duplicate(instance);
        ASSERT_NE(copy, nullptr);
        EXPECT_TRUE(copy->getConstant<bool>("probe"));
        engine->destroy(copy);
    }
    engine->destroy(instance);

    // The material still holds its MaterialDefinition, so building the same package again must
    // share it rather than create a new one.
    Material* sameMaterial =
            Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(sameMaterial, nullptr);
    EXPECT_EQ(&downcast(sameMaterial)->getDefinition(), &downcast(material)->getDefinition());

    engine->destroy(sameMaterial);
    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(Material, RejectMismatchedDescriptorBindingsAndLayout) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("DescriptorMismatchTest");
    builder.shading(Shading::UNLIT);
    builder.parameter("tex", filamat::MaterialBuilder::SamplerType::SAMPLER_2D,
            filamat::MaterialBuilder::SamplerFormat::FLOAT);
    filamat::Package const pkg = builder.build(engine->getJobSystem());
    ASSERT_TRUE(pkg.isValid());

    Material* validMaterial =
            Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    ASSERT_NE(validMaterial, nullptr);
    engine->destroy(validMaterial);

    auto const getChunkSpan = [](std::vector<uint8_t>& bytes, filamat::ChunkType const type)
            -> std::pair<uint8_t*, uint8_t*> {
        filaflat::ChunkContainer cc(bytes.data(), bytes.size());
        if (!cc.parse() || !cc.hasChunk(type)) {
            return { nullptr, nullptr };
        }
        auto const [start, end] = cc.getChunkRange(type);
        return { bytes.data() + (start - bytes.data()), bytes.data() + (end - bytes.data()) };
    };

    // Case 1 (b/557281826 PoC): MAT_DSLI has 1 descriptor (binding 0), while MAT_DBDI declares
    // a sampler at binding 63.
    {
        std::vector<uint8_t> mutated(pkg.getData(), pkg.getData() + pkg.getSize());
        auto const [dsliStart, dsliEnd] =
                getChunkSpan(mutated, filamat::MaterialDescriptorSetLayoutInfo);
        auto const [dbdiStart, dbdiEnd] =
                getChunkSpan(mutated, filamat::MaterialDescriptorBindingsInfo);
        ASSERT_NE(dsliStart, nullptr);
        ASSERT_NE(dbdiStart, nullptr);
        dsliStart[0] = 1; // shrink layout to 1 descriptor (binding 0 only)
        dbdiEnd[-1] = 63; // move sampler binding in MAT_DBDI to 63
        EXPECT_EQ(Material::Builder().package(mutated.data(), mutated.size()).build(*engine),
                nullptr);
    }

    // Case 2: MAT_DBDI declares a descriptor type that disagrees with MAT_DSLI for the same
    // binding.
    {
        std::vector<uint8_t> mutated(pkg.getData(), pkg.getData() + pkg.getSize());
        auto const [dbdiStart, dbdiEnd] =
                getChunkSpan(mutated, filamat::MaterialDescriptorBindingsInfo);
        ASSERT_NE(dbdiStart, nullptr);
        dbdiEnd[-2] = uint8_t(backend::DescriptorType::SAMPLER_3D_FLOAT);
        EXPECT_EQ(Material::Builder().package(mutated.data(), mutated.size()).build(*engine),
                nullptr);
    }

    // Case 3: MAT_DBDI fails unflatten (duplicate binding 0).
    {
        std::vector<uint8_t> mutated(pkg.getData(), pkg.getData() + pkg.getSize());
        auto const [dbdiStart, dbdiEnd] =
                getChunkSpan(mutated, filamat::MaterialDescriptorBindingsInfo);
        ASSERT_NE(dbdiStart, nullptr);
        dbdiEnd[-1] = 0; // collides with UBO at binding 0
        EXPECT_EQ(Material::Builder().package(mutated.data(), mutated.size()).build(*engine),
                nullptr);
    }

    Engine::destroy(engine);
}

INSTANTIATE_TEST_SUITE_P(MaterialInstanceTest, SpecializedInstanceLifetimeTest,
        testing::Combine(testing::Bool(), testing::Bool()),
        [](testing::TestParamInfo<std::tuple<bool, bool>> const& info) {
            return std::string(std::get<0>(info.param) ? "ProgramCache" : "NoProgramCache") +
                   (std::get<1>(info.param) ? "_Duplicated" : "_Single");
        });
