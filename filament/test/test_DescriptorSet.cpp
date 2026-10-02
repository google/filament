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

#include "MockDriver.h"

#include "HwDescriptorSetLayoutFactory.h"
#include "ds/DescriptorSet.h"
#include "ds/DescriptorSetLayout.h"

#include <private/backend/CommandBufferQueue.h>
#include <private/backend/CommandStream.h>

#include <utils/Panic.h>
#include <utils/StaticString.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>

namespace {

using namespace filament;
using namespace backend;

/**
 * Descriptor bindings can originate from the material binary (see FMaterial::getSamplerBinding())
 * and are not cross-checked against the material's DescriptorSetLayout. The per-binding storage
 * of both DescriptorSetLayout (mDescriptorTypes) and DescriptorSet (mDescriptors) is only sized
 * for the bindings the layout actually declares, so an out-of-range binding used to be an
 * out-of-bounds read (heap-buffer-overflow under ASan). These tests make sure such a binding is
 * detected instead of being used to index those arrays.
 */
class DescriptorSetBoundsTest : public ::testing::Test {
protected:
    static constexpr size_t MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
    static constexpr size_t COMMAND_BUFFERS_SIZE = 3 * MIN_COMMAND_BUFFERS_SIZE;

    // Guaranteed to be outside of the 1-entry layouts built below.
    static constexpr descriptor_binding_t OUT_OF_RANGE_BINDING = 60;

    DescriptorSetBoundsTest()
            : mCommandBufferQueue(MIN_COMMAND_BUFFERS_SIZE, COMMAND_BUFFERS_SIZE, false),
              mCommandStream(mMockDriver, mCommandBufferQueue.getCircularBuffer()),
              mDriverApi(mCommandStream) {}

    // Builds a descriptor set layout with a single descriptor at binding 0.
    filament::DescriptorSetLayout buildLayout(DescriptorType const type) {
        DescriptorSetLayoutDescriptor desc{};
        desc.type = type;
        desc.stageFlags = ShaderStageFlags::FRAGMENT;
        desc.binding = 0;
        desc.count = 1;

        backend::DescriptorSetLayout dsl;
        dsl.descriptors.reserve(1);
        dsl.descriptors.push_back(desc);
        return filament::DescriptorSetLayout(mFactory, mDriverApi, dsl);
    }

    MockDriver mMockDriver;
    CommandBufferQueue mCommandBufferQueue;
    CommandStream mCommandStream;
    DriverApi& mDriverApi;
    HwDescriptorSetLayoutFactory mFactory;
};

TEST_F(DescriptorSetBoundsTest, OutOfRangeBindingIsNotInTheLayout) {
    filament::DescriptorSetLayout layout = buildLayout(DescriptorType::SAMPLER_2D_FLOAT);
    ASSERT_EQ(layout.getMaxDescriptorBinding(), 0u);
    EXPECT_FALSE(layout.hasDescriptor(OUT_OF_RANGE_BINDING));

    // Reading the type of an out-of-range binding must not read out of bounds.
    EXPECT_EQ(layout.getDescriptorType(OUT_OF_RANGE_BINDING), DescriptorType::UNIFORM_BUFFER);

    layout.terminate(mFactory, mDriverApi);
}

TEST_F(DescriptorSetBoundsTest, InRangeBindingIsStillAccepted) {
    filament::DescriptorSetLayout layout = buildLayout(DescriptorType::UNIFORM_BUFFER);
    ASSERT_TRUE(layout.hasDescriptor(0));

    utils::StaticString const name = "test";
    DescriptorSet set{ name, layout };
    EXPECT_NO_THROW(set.setBuffer(layout, 0, {}, 0, 0));

    layout.terminate(mFactory, mDriverApi);
}

TEST_F(DescriptorSetBoundsTest, SetBufferWithOutOfRangeBindingDoesNotReadOutOfBounds) {
    filament::DescriptorSetLayout layout = buildLayout(DescriptorType::UNIFORM_BUFFER);
    utils::StaticString const name = "test";
    DescriptorSet set{ name, layout };

    // Would read mDescriptors[60] (heap-buffer-overflow) before the fix.
#if GTEST_HAS_EXCEPTIONS
    EXPECT_THROW(set.setBuffer(layout, OUT_OF_RANGE_BINDING, {}, 0, 0), utils::PreconditionPanic);
#else
    EXPECT_DEATH(set.setBuffer(layout, OUT_OF_RANGE_BINDING, {}, 0, 0), "");
#endif

    layout.terminate(mFactory, mDriverApi);
}

TEST_F(DescriptorSetBoundsTest, SetSamplerWithOutOfRangeBindingDoesNotReadOutOfBounds) {
    filament::DescriptorSetLayout layout = buildLayout(DescriptorType::SAMPLER_2D_FLOAT);
    utils::StaticString const name = "test";
    DescriptorSet set{ name, layout };

    // Would read mDescriptorTypes[60] and mDescriptors[60] (heap-buffer-overflows) before the fix.
#if GTEST_HAS_EXCEPTIONS
    EXPECT_THROW(set.setSampler(layout, OUT_OF_RANGE_BINDING, {}, {}), utils::PreconditionPanic);
#else
    EXPECT_DEATH(set.setSampler(layout, OUT_OF_RANGE_BINDING, {}, {}), "");
#endif

    layout.terminate(mFactory, mDriverApi);
}

} // anonymous namespace
