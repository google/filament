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

#include "details/Engine.h"
#include "details/Material.h"
#include "details/View.h"

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>

#include <filamat/MaterialBuilder.h>

#include <gtest/gtest.h>

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
            Variant::SRE | Variant::FOG,
            Variant::SRE | Variant::SKN,
            Variant::SRE | Variant::DIR,
            Variant::S2D | Variant::SRE,
            Variant::S2D | Variant::SRE | Variant::FOG,
            Variant::S2D | Variant::SRE | Variant::SKN,
            Variant::S2D | Variant::SRE | Variant::DIR,
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
    EXPECT_EQ(validKeys.size, 6);
    for (auto const key : validKeys) {
        EXPECT_FALSE(key.hasExtraDirectionalLights() && !key.hasDirectionalLighting());
    }
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
    constexpr Variant::type_t SHADOW_VARIANTS[] = {
            Variant::SRE,
            Variant::SRE | Variant::FOG,
            Variant::SRE | Variant::SKN,
            Variant::S2D | Variant::SRE,
            Variant::S2D | Variant::SRE | Variant::FOG,
            Variant::S2D | Variant::SRE | Variant::SKN,
    };

    for (Variant::type_t const key : SHADOW_VARIANTS) {
        EXPECT_TRUE(definition.isValidProgram(
                Variant(key), DynamicSpecConstKey{}, fengine.getShaderModel(),
                fengine.isStereoSupported()));
        EXPECT_TRUE(definition.isValidProgram(
                Variant(key), dynamicLighting, fengine.getShaderModel(),
                fengine.isStereoSupported()));
    }
    EXPECT_FALSE(definition.isValidProgram(
            Variant(Variant::SPECIAL_SSR_VARIANT), DynamicSpecConstKey{},
            fengine.getShaderModel(), fengine.isStereoSupported()));

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
