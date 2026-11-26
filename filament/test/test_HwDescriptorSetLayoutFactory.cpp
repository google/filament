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

#include "HwDescriptorSetLayoutFactory.h"

#include "details/Engine.h"

#include <filament/Engine.h>

#include <private/backend/DriverApi.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <gtest/gtest.h>

#include <initializer_list>

#include <stddef.h>
#include <stdint.h>

namespace {

using namespace filament;
using namespace filament::backend;

backend::DescriptorSetLayout makeSamplerLayout(
        std::initializer_list<descriptor_binding_t> bindings) {
    backend::DescriptorSetLayout dsl;
    dsl.descriptors.reserve(bindings.size());
    for (descriptor_binding_t const binding : bindings) {
        dsl.descriptors.push_back({
            .type = DescriptorType::SAMPLER_2D_FLOAT,
            .stageFlags = ShaderStageFlags::FRAGMENT,
            .binding = binding,
            .flags = DescriptorFlags::NONE,
            .count = 0,
        });
    }
    return dsl;
}

class HwDescriptorSetLayoutFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        mEngine = Engine::Builder().backend(Backend::NOOP).build();
        ASSERT_NE(mEngine, nullptr);
    }

    void TearDown() override {
        Engine::destroy(&mEngine);
    }

    DriverApi& driverApi() const {
        return downcast(mEngine)->getDriverApi();
    }

    Engine* mEngine = nullptr;
};

TEST(DescriptorSetLayoutDescriptorTest, EqualityChecksAllFields) {
    DescriptorSetLayoutDescriptor const base{
        .type = DescriptorType::SAMPLER_2D_FLOAT,
        .stageFlags = ShaderStageFlags::FRAGMENT,
        .binding = 2,
        .flags = DescriptorFlags::NONE,
        .count = 1,
    };

    EXPECT_EQ(base, base);

    auto diffType = base;
    diffType.type = DescriptorType::UNIFORM_BUFFER;
    EXPECT_FALSE(base == diffType);

    auto diffStage = base;
    diffStage.stageFlags = ShaderStageFlags::VERTEX;
    EXPECT_FALSE(base == diffStage);

    auto diffBinding = base;
    diffBinding.binding = 63;
    EXPECT_FALSE(base == diffBinding);

    auto diffFlags = base;
    diffFlags.flags = DescriptorFlags::DYNAMIC_OFFSET;
    EXPECT_FALSE(base == diffFlags);

    auto diffCount = base;
    diffCount.count = 2;
    EXPECT_FALSE(base == diffCount);
}

TEST_F(HwDescriptorSetLayoutFactoryTest, DistinctBindingsAreNotAliased) {
    HwDescriptorSetLayoutFactory factory;
    DriverApi& driver = driverApi();

    auto const handleA = factory.create(driver, makeSamplerLayout({ 0, 1, 6 }));
    auto const handleB = factory.create(driver, makeSamplerLayout({ 0, 1, 63 }));
    EXPECT_NE(handleA.getId(), handleB.getId());

    // Identical bindings in a different input order still deduplicate to the same handle.
    auto const handleADup = factory.create(driver, makeSamplerLayout({ 6, 0, 1 }));
    EXPECT_EQ(handleA.getId(), handleADup.getId());

    factory.destroy(driver, handleADup);
    factory.destroy(driver, handleB);
    factory.destroy(driver, handleA);
    factory.terminate(driver);
    mEngine->flushAndWait();
}

TEST_F(HwDescriptorSetLayoutFactoryTest, HashCollisionWithDifferentBindingsDoesNotAlias) {
    // These two sorted layouts have identical (type, stageFlags, flags, count) and produce the
    // exact same 32-bit murmurSlow hash, but differ in their binding indices.
    backend::DescriptorSetLayout const layoutA = makeSamplerLayout({ 0, 1, 38, 40, 53 });
    backend::DescriptorSetLayout const layoutB = makeSamplerLayout({ 0, 3, 7, 13, 51 });

    HwDescriptorSetLayoutFactory::Parameters const paramsA{ layoutA };
    HwDescriptorSetLayoutFactory::Parameters const paramsB{ layoutB };
    ASSERT_EQ(paramsA.hash(), paramsB.hash());
    EXPECT_FALSE(paramsA == paramsB);

    HwDescriptorSetLayoutFactory factory;
    DriverApi& driver = driverApi();
    auto const handleA = factory.create(driver, layoutA);
    auto const handleB = factory.create(driver, layoutB);
    EXPECT_NE(handleA.getId(), handleB.getId());

    factory.destroy(driver, handleB);
    factory.destroy(driver, handleA);
    factory.terminate(driver);
    mEngine->flushAndWait();
}

TEST_F(HwDescriptorSetLayoutFactoryTest, EmptyLayoutCreatesAndDeduplicates) {
    HwDescriptorSetLayoutFactory factory;
    DriverApi& driver = driverApi();

    auto const handleA = factory.create(driver, backend::DescriptorSetLayout{});
    auto const handleB = factory.create(driver, backend::DescriptorSetLayout{});
    EXPECT_EQ(handleA.getId(), handleB.getId());

    factory.destroy(driver, handleB);
    factory.destroy(driver, handleA);
    factory.terminate(driver);
    mEngine->flushAndWait();
}

} // namespace
