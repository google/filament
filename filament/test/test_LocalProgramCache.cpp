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

#include "filament_test_resources.h"
#include "LocalProgramCache.h"

#include "details/Material.h"

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialChunkType.h>
#include <filament/RenderableManager.h>

#include <filamat/MaterialBuilder.h>

#include <filaflat/ChunkContainer.h>

#include <utils/EntityManager.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

using namespace filament;

namespace {
constexpr std::size_t SURFACE_SIZE = 1u << (VARIANT_BITS + DYNAMIC_SPEC_CONST_KEY_BITS);
constexpr std::size_t POST_PROCESS_SIZE =
        1u << (POST_PROCESS_VARIANT_BITS + DYNAMIC_SPEC_CONST_KEY_BITS);

void mapInvalidKey(uint8_t variantKey, std::size_t size) noexcept {
    Variant variant{};
    variant.key = variantKey;
    (void) LocalProgramCache::mapCacheEntryKey(variant, DynamicSpecConstKey{ 0 }, size);
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

TEST(LocalProgramCache, SurfaceBoundary) {
    Variant variant{};
    variant.key = 0x7f;
    EXPECT_EQ(LocalProgramCache::mapCacheEntryKey(
                      variant, DynamicSpecConstKey{ 7 }, SURFACE_SIZE), 1023u);
}

TEST(LocalProgramCache, PostProcessBoundaries) {
    Variant variant{};
    EXPECT_EQ(LocalProgramCache::mapCacheEntryKey(
                      variant, DynamicSpecConstKey{ 0 }, POST_PROCESS_SIZE), 0u);
    variant.key = 1;
    EXPECT_EQ(LocalProgramCache::mapCacheEntryKey(
                      variant, DynamicSpecConstKey{ 0 }, POST_PROCESS_SIZE), 8u);
}

TEST(LocalProgramCacheDeathTest, RejectsOverflow) {
    EXPECT_DEATH(mapInvalidKey(0x80, SURFACE_SIZE), "");
    EXPECT_DEATH(mapInvalidKey(2, POST_PROCESS_SIZE), "");
    EXPECT_DEATH(mapInvalidKey(Variant::DEP, POST_PROCESS_SIZE), "");
}

TEST(LocalProgramCacheDeathTest, UsesActualCapacity) {
    EXPECT_DEATH(mapInvalidKey(0, 0), "");
    EXPECT_DEATH(mapInvalidKey(1, 8), "");
}

TEST(LocalProgramCache, DepthIgnoresSpecialization) {
    Variant variant{};
    variant.key = Variant::DEP;
    ASSERT_TRUE(Variant::isValidDepthVariant(variant));
    for (uint16_t key = 0; key < DYNAMIC_SPEC_CONST_KEY_COUNT; ++key) {
        EXPECT_EQ(LocalProgramCache::mapCacheEntryKey(
                          variant, DynamicSpecConstKey{ key }, SURFACE_SIZE), 128u);
    }
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

    // 3. Meanwhile the program cache was sized for the post-process variant space only.
    LocalProgramCache const& programs = downcast(material)->getPrograms();
    ASSERT_EQ(programs.getPrograms().size(), POST_PROCESS_SIZE);

    // 4. RenderPass::setupColorCommand() builds a surface variant for a renderable and
    //    RenderPass::instanceify() feeds it to prepareProgram(). A lit renderable that receives
    //    shadows is about as ordinary as it gets, and it already lands outside the cache.
    Variant variant{};
    variant.key = Variant::DIR | Variant::SRE;
    ASSERT_GE(std::size_t{ variant.key } << DYNAMIC_SPEC_CONST_KEY_BITS, POST_PROCESS_SIZE)
            << "this variant must actually be out of bounds, otherwise the test proves nothing";

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
