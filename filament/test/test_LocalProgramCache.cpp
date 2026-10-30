/*
 * Copyright (C) 2026 The Android Open Source Project
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
#include "LocalProgramCache.h"
#include "MaterialDefinition.h"
#include "MaterialParser.h"
#include "ProgramSlotMap.h"

#include "details/Material.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/Variant.h>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialChunkType.h>
#include <filament/MaterialEnums.h>
#include <filament/RenderableManager.h>

#include <filamat/MaterialBuilder.h>

#include <filaflat/ChunkContainer.h>

#include <utils/EntityManager.h>
#include <utils/Slice.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace filament;

namespace {

using Program = std::pair<Variant, DynamicSpecConstKey>;

// What MaterialDefinition::acquirePrograms() can store.
std::vector<Program> getCacheablePrograms(MaterialDomain const domain, bool const isLit) {
    std::vector<Program> programs;
    auto const add = [&](utils::Slice<const Variant> const variants) {
        for (Variant const variant : variants) {
            for (DynamicSpecConstKey const specKey :
                    DynamicSpecConstKey::getValidKeys(variant, domain, isLit)) {
                Program const program{ variant, specKey };
                if (std::find(programs.begin(), programs.end(), program) == programs.end()) {
                    programs.push_back(program);
                }
            }
        }
    };
    add(MaterialDefinition::getVariantsFor(domain, isLit));
    add(MaterialDefinition::getDepthVariantsFor(domain));
    return programs;
}

// Every (variant, specKey) the renderer can ask a material of this kind for, derived from the
// filtering the callers apply rather than from the variant lists the slot map is built from:
// - RenderPass::setupColorCommand(): Variant::filterVariant() then filterProgramSpecKey().
// - RenderPass depth commands: depth variants, filterProgramSpecKey() as if lit.
// - PostProcessManager: post-process variants with specKey 0.
std::vector<Program> getRequestablePrograms(MaterialDomain const domain, bool const isLit) {
    std::vector<Program> programs;
    if (domain == MaterialDomain::POST_PROCESS) {
        for (Variant::type_t v = 0; v < POST_PROCESS_VARIANT_COUNT; ++v) {
            for (DynamicSpecConstKey::type_t k = 0; k < DYNAMIC_SPEC_CONST_KEY_COUNT; ++k) {
                programs.emplace_back(Variant{ v }, DynamicSpecConstKey::filterProgramSpecKey(
                        Variant{ v }, DynamicSpecConstKey{ k }, domain, isLit));
            }
        }
        return programs;
    }
    for (std::size_t v = 0; v < VARIANT_COUNT; ++v) {
        Variant const raw{ Variant::type_t(v) };
        if (!Variant::isValid(raw)) {
            continue;
        }
        Variant const variant = Variant::filterVariant(raw, isLit);
        for (DynamicSpecConstKey::type_t k = 0; k < DYNAMIC_SPEC_CONST_KEY_COUNT; ++k) {
            DynamicSpecConstKey const key{ k };
            programs.emplace_back(variant,
                    DynamicSpecConstKey::filterProgramSpecKey(variant, key, domain, isLit));
            if (Variant::isValidDepthVariant(variant)) {
                programs.emplace_back(variant,
                        DynamicSpecConstKey::filterProgramSpecKey(variant, key, domain, true));
            }
        }
    }
    return programs;
}

void mapWithoutSlot(MaterialDomain const domain, bool const isLit, Variant::type_t const variantKey,
        DynamicSpecConstKey::type_t const specKey) noexcept {
    (void) ProgramSlotMap::get(domain, isLit).map(Variant{ variantKey },
            DynamicSpecConstKey{ specKey });
}

// Compiles a genuine post-process material, i.e. the kind of asset an application can load at
// runtime from a package it did not author.
filamat::Package buildPostProcessMaterial(Engine& engine) {
    static constexpr std::string_view shaderCode = R"(
        void postProcess(inout PostProcessInputs postProcess) {
            postProcess.color = float4(1.0);
        }
    )";
    filamat::MaterialBuilder builder;
    builder.init();
    builder.name("test_post_process")
            .materialDomain(filamat::MaterialBuilder::MaterialDomain::POST_PROCESS)
            .materialSource(shaderCode);
    return builder.build(downcast(engine).getJobSystem());
}
}

TEST(ProgramSlotMap, EveryProgramHasItsOwnSlot) {
    // The cache is dense: each cacheable program has a distinct slot and no slot is unused.
    for (auto const [domain, isLit] : { std::pair{ MaterialDomain::SURFACE, true },
                 std::pair{ MaterialDomain::SURFACE, false },
                 std::pair{ MaterialDomain::POST_PROCESS, false } }) {
        ProgramSlotMap const& slotMap = ProgramSlotMap::get(domain, isLit);
        std::vector<Program> const programs = getCacheablePrograms(domain, isLit);
        std::vector<bool> used(slotMap.size(), false);
        for (auto const& [variant, specKey] : programs) {
            ASSERT_TRUE(slotMap.hasSlot(variant, specKey))
                    << "variant=" << +variant.key << " specKey=" << specKey.key;
            uint32_t const slot = slotMap.map(variant, specKey);
            ASSERT_LT(slot, slotMap.size());
            EXPECT_FALSE(used[slot]) << "variant=" << +variant.key << " specKey=" << specKey.key;
            used[slot] = true;
        }
        EXPECT_EQ(programs.size(), slotMap.size()) << "lit=" << isLit;
    }
}

TEST(ProgramSlotMap, EveryRequestableProgramHasASlot) {
    for (auto const [domain, isLit] : { std::pair{ MaterialDomain::SURFACE, true },
                 std::pair{ MaterialDomain::SURFACE, false },
                 std::pair{ MaterialDomain::POST_PROCESS, false } }) {
        ProgramSlotMap const& slotMap = ProgramSlotMap::get(domain, isLit);
        for (auto const& [variant, specKey] : getRequestablePrograms(domain, isLit)) {
            EXPECT_TRUE(slotMap.hasSlot(variant, specKey))
                    << "domain=" << int(domain) << " lit=" << isLit
                    << " variant=" << +variant.key << " specKey=" << specKey.key;
        }
    }
}

TEST(ProgramSlotMap, HasTheExpectedSize) {
    // Update when the variants or the specialization keys change:
    //   lit:   12 standard variants x 12 keys + 2 SSR + 14 depth = 160
    //   unlit:  4 standard variants x  2 keys + 2 SSR + 14 depth =  24
    EXPECT_EQ(ProgramSlotMap::get(MaterialDomain::SURFACE, true).size(), 160u);
    EXPECT_EQ(ProgramSlotMap::get(MaterialDomain::SURFACE, false).size(), 24u);
    EXPECT_EQ(ProgramSlotMap::get(MaterialDomain::POST_PROCESS, false).size(),
            POST_PROCESS_VARIANT_COUNT);
    EXPECT_EQ(ProgramSlotMap::get(MaterialDomain::COMPUTE, false).size(), 1u);
}

TEST(ProgramSlotMap, IsSharedByMaterialsOfTheSameKind) {
    EXPECT_EQ(&ProgramSlotMap::get(MaterialDomain::SURFACE, true),
            &ProgramSlotMap::get(MaterialDomain::SURFACE, true));
    EXPECT_NE(&ProgramSlotMap::get(MaterialDomain::SURFACE, true),
            &ProgramSlotMap::get(MaterialDomain::SURFACE, false));
}

TEST(ProgramSlotMap, DepthIgnoresSpecialization) {
    // Depth variants are decoupled from the dynamic specialization space, so every specialization
    // key must collapse onto the depth variant's own slot (i.e. as if specKey were 0).
    for (bool const isLit : { true, false }) {
        ProgramSlotMap const& slotMap = ProgramSlotMap::get(MaterialDomain::SURFACE, isLit);
        for (Variant const variant : VariantUtils::getDepthVariants()) {
            uint32_t const slot = slotMap.map(variant, DynamicSpecConstKey{ 0 });
            for (uint16_t key = 0; key < DYNAMIC_SPEC_CONST_KEY_COUNT; ++key) {
                EXPECT_EQ(slotMap.map(variant, DynamicSpecConstKey{ key }), slot)
                        << "variant=" << +variant.key << " specKey=" << key;
            }
        }
    }
}

TEST(ProgramSlotMap, HasNoSlotForProgramsTheMaterialCantHave) {
    ProgramSlotMap const& lit = ProgramSlotMap::get(MaterialDomain::SURFACE, true);
    ProgramSlotMap const& unlit = ProgramSlotMap::get(MaterialDomain::SURFACE, false);
    ProgramSlotMap const& postProcess =
            ProgramSlotMap::get(MaterialDomain::POST_PROCESS, false);

    // Out of the variant space.
    EXPECT_FALSE(lit.hasSlot(Variant{ Variant::type_t(VARIANT_COUNT) }, DynamicSpecConstKey{}));
    EXPECT_FALSE(lit.hasSlot(Variant{ 0xFF }, DynamicSpecConstKey{}));
    EXPECT_FALSE(postProcess.hasSlot(
            Variant{ Variant::type_t(POST_PROCESS_VARIANT_COUNT) }, DynamicSpecConstKey{}));
    // Surface variants on a post-process material.
    EXPECT_FALSE(postProcess.hasSlot(Variant{ Variant::DEP }, DynamicSpecConstKey{}));
    // Specialization keys the material can't use.
    EXPECT_FALSE(postProcess.hasSlot(Variant{}, DynamicSpecConstKey{ DynamicSpecConstKey::FOG }));
    EXPECT_FALSE(unlit.hasSlot(Variant{},
            DynamicSpecConstKey{ DynamicSpecConstKey::DYNAMIC_LIGHTING }));
    EXPECT_FALSE(lit.hasSlot(Variant{},
            DynamicSpecConstKey{ DynamicSpecConstKey::EXTRA_DIRECTIONAL_LIGHTS }));
}

TEST(ProgramSlotMapDeathTest, RejectsProgramsWithoutSlot) {
    EXPECT_DEATH(mapWithoutSlot(MaterialDomain::SURFACE, true,
            Variant::type_t(VARIANT_COUNT), 0), "");
    EXPECT_DEATH(mapWithoutSlot(MaterialDomain::POST_PROCESS, false,
            Variant::type_t(POST_PROCESS_VARIANT_COUNT), 0), "");
    EXPECT_DEATH(mapWithoutSlot(MaterialDomain::POST_PROCESS, false, Variant::DEP, 0), "");
    EXPECT_DEATH(mapWithoutSlot(MaterialDomain::SURFACE, false, 0,
            DynamicSpecConstKey::DYNAMIC_LIGHTING), "");
}

TEST(MaterialDomainValidation, RejectsInvalidAndMissingDomain) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);
    for (int value : { -1, 255 }) {
        std::vector<uint8_t> bytes(FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA,
                FILAMENT_TEST_RESOURCES_TEST_MATERIAL_DATA +
                        FILAMENT_TEST_RESOURCES_TEST_MATERIAL_SIZE);
        filaflat::ChunkContainer chunks(bytes.data(), bytes.size());
        ASSERT_TRUE(chunks.parse());
        auto const range = chunks.getChunkRange(filamat::ChunkType::MaterialDomain);
        ASSERT_NE(range.first, nullptr);
        ASSERT_EQ(range.second - range.first, 1);
        std::size_t const offset = range.first - bytes.data();
        if (value < 0) {
            // A chunk has a uint64_t type and uint32_t payload size before its payload.
            bytes.erase(bytes.begin() + offset - 12, bytes.begin() + offset + 1);
        } else {
            bytes[offset] = static_cast<uint8_t>(value);
        }
        Material* material = Material::Builder().package(bytes.data(), bytes.size()).build(*engine);
        EXPECT_EQ(material, nullptr);
        if (material) {
            engine->destroy(material);
        }
    }
    Engine::destroy(engine);
}

TEST(LocalProgramCacheRegressionDeathTest, SurfaceVariantOnPostProcessMaterialIsRejected) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::Package const package = buildPostProcessMaterial(*engine);
    ASSERT_TRUE(package.isValid());

    // 1. The asset loads. Rejecting post-process materials outright would break
    //    IBLPrefilterLibrary, so this is by design.
    Material* material =
            Material::Builder().package(package.getData(), package.getSize()).build(*engine);
    ASSERT_NE(material, nullptr);
    ASSERT_EQ(material->getMaterialDomain(), MaterialDomain::POST_PROCESS);

    // 2. Nothing stops that material from being attached to a renderable: RenderableManager
    //    only gates on feature level. This is what puts a post-process material under the
    //    control of the surface render path.
    utils::Entity const entity = utils::EntityManager::get().create();
    RenderableManager::Builder(1).boundingBox({{ 0, 0, 0 }, { 1, 1, 1 }}).build(*engine, entity);
    RenderableManager& rcm = engine->getRenderableManager();
    auto const renderable = rcm.getInstance(entity);
    ASSERT_TRUE(renderable.isValid());
    rcm.setMaterialInstanceAt(renderable, 0, material->getDefaultInstance());
    EXPECT_EQ(rcm.getMaterialInstanceAt(renderable, 0), material->getDefaultInstance());

    // 3. Meanwhile the program cache only has slots for the post-process variants.
    LocalProgramCache const& programs = downcast(material)->getPrograms();
    ProgramSlotMap const& slotMap =
            ProgramSlotMap::get(MaterialDomain::POST_PROCESS, false);
    ASSERT_EQ(programs.getPrograms().size(), slotMap.size());

    // 4. RenderPass::setupColorCommand() builds a surface variant for a renderable and
    //    RenderPass::instanceify() feeds it to prepareProgram(). A lit renderable that receives
    //    shadows is about as ordinary as it gets, and it has no slot in the cache.
    Variant variant{};
    variant.key = Variant::SRE;
    ASSERT_FALSE(slotMap.hasSlot(variant, DynamicSpecConstKey{ 0 }))
            << "SRE must not have a slot in the post-process cache, otherwise this test proves "
               "nothing; pick a surface-only bit if the variant layout changes";

    // The Engine owns worker threads, and the default "fast" style forks without exec, which is
    // unsafe there. "threadsafe" re-executes the binary instead.
    std::string const previousStyle = GTEST_FLAG_GET(death_test_style);
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(programs.prepareProgram(downcast(engine)->getDriverApi(), variant,
                         DynamicSpecConstKey{ 0 }, backend::CompilerPriorityQueue::HIGH),
            "utils::PostconditionPanic");
    GTEST_FLAG_SET(death_test_style, previousStyle);

    engine->destroy(entity);
    utils::EntityManager::get().destroy(entity);
    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialDomainValidation, UserVariantFilterIsInertForPostProcessMaterials) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::Package const package = buildPostProcessMaterial(*engine);
    ASSERT_TRUE(package.isValid());
    Material* material =
            Material::Builder().package(package.getData(), package.getSize()).build(*engine);
    ASSERT_NE(material, nullptr);

    MaterialDefinition const& definition = downcast(material)->getDefinition();
    ASSERT_EQ(definition.materialDomain, MaterialDomain::POST_PROCESS);

    // A post-process variant key is a plain PostProcessVariant index, not a bitfield of surface
    // variant bits, so the user variant filter must leave it completely untouched.
    //
    // This matters because callers compare the filtered key against the original to decide
    // whether to skip a variant (see FMaterialInstance::compile). If a filter mask happens to
    // clear a bit that a post-process key uses as part of its index, that variant silently
    // disappears from precompilation with no diagnostic.
    //
    // Asserting over every mask rather than a specific one keeps this test independent of the
    // surface variant bit layout, which is exactly what shifts when a variant bit is added or
    // removed.
    for (auto const variant: definition.getVariants()) {
        for (uint32_t mask = 0; mask <= uint32_t(UserVariantFilterBit::ALL); ++mask) {
            EXPECT_EQ(definition.filterUserVariant(variant, UserVariantFilterMask(mask)).key,
                    variant.key)
                    << "post-process variant " << +variant.key
                    << " was altered by user variant filter mask " << mask;
        }
    }

    engine->destroy(material);
    Engine::destroy(engine);
}

TEST(MaterialDomainValidation, FeatureLevel0IncludesBothPostProcessVariants) {
    Engine* engine = Engine::create(Engine::Backend::NOOP);
    ASSERT_NE(engine, nullptr);

    filamat::MaterialBuilder::init();
    filamat::MaterialBuilder builder;
    builder.materialDomain(filamat::MaterialBuilder::MaterialDomain::POST_PROCESS)
            .featureLevel(backend::FeatureLevel::FEATURE_LEVEL_0)
            .platform(filamat::MaterialBuilder::Platform::MOBILE)
            .targetApi(filamat::MaterialBuilder::TargetApi::OPENGL)
            .materialSource(R"(
                void postProcess(inout PostProcessInputs postProcess) {
                    postProcess.color = float4(1.0);
                }
            )");
    filamat::Package const package = builder.build(downcast(engine)->getJobSystem());
    filamat::MaterialBuilder::shutdown();
    Engine::destroy(engine);
    ASSERT_TRUE(package.isValid());

    // Inspect ESSL1 specifically: a valid package can still be missing its translucent shaders
    // if the FL0 surface filter mistakes the post-process index for the DIR bit.
    MaterialParser parser({ backend::ShaderLanguage::ESSL1 },
            package.getData(), package.getSize());
    ASSERT_EQ(parser.parse(), MaterialParser::ParseResult::SUCCESS);
    for (auto const variant: { PostProcessVariant::OPAQUE, PostProcessVariant::TRANSLUCENT }) {
        for (auto const stage: { backend::ShaderStage::VERTEX, backend::ShaderStage::FRAGMENT }) {
            EXPECT_TRUE(parser.hasShader(backend::ShaderModel::MOBILE,
                    Variant(static_cast<Variant::type_t>(variant)), stage))
                    << "Missing ESSL1 shader for post-process variant " << int(variant)
                    << ", stage " << int(stage);
        }
    }
}
