/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <gtest/gtest.h>

#include "../src/metal/MetalContext.h"

namespace test {

TEST(MetalDynamicOffsets, none) {
    filament::backend::MetalDynamicOffsets dynamicOffsets;
    const auto [count, offsets] = dynamicOffsets.getOffsets();
    EXPECT_EQ(count, 0u);
}

TEST(MetalDynamicOffsets, basic) {
    filament::backend::MetalDynamicOffsets dynamicOffsets;
    {
        const auto [count, offsets] = dynamicOffsets.getOffsets();
        EXPECT_EQ(count, 0u);
    }

    {
        uint32_t o[2] = { 1, 2 };
        dynamicOffsets.setOffsets(0, o, 2);
        const auto [count, offsets] = dynamicOffsets.getOffsets();
        EXPECT_EQ(count, 2);
        EXPECT_EQ(offsets[0], 1);
        EXPECT_EQ(offsets[1], 2);
    }

    {
        uint32_t o[3] = { 3, 4, 5 };
        dynamicOffsets.setOffsets(1, o, 3);
        const auto [count, offsets] = dynamicOffsets.getOffsets();
        EXPECT_EQ(count, 5);
        EXPECT_EQ(offsets[0], 1);
        EXPECT_EQ(offsets[1], 2);
        EXPECT_EQ(offsets[2], 3);
        EXPECT_EQ(offsets[3], 4);
        EXPECT_EQ(offsets[4], 5);
    }

    // skip descriptor set index 2

    {
        uint32_t o[1] = { 6 };
        dynamicOffsets.setOffsets(3, o, 1);
        const auto [count, offsets] = dynamicOffsets.getOffsets();
        EXPECT_EQ(count, 6);
        EXPECT_EQ(offsets[0], 1);
        EXPECT_EQ(offsets[1], 2);
        EXPECT_EQ(offsets[2], 3);
        EXPECT_EQ(offsets[3], 4);
        EXPECT_EQ(offsets[4], 5);
        EXPECT_EQ(offsets[5], 6);
    }
}

TEST(MetalDynamicOffsets, outOfOrder) {
    filament::backend::MetalDynamicOffsets dynamicOffsets;
    uint32_t o1[2] = { 2, 3 };
    dynamicOffsets.setOffsets(1, o1, 2);
    uint32_t o2[2] = { 0, 1 };
    dynamicOffsets.setOffsets(0, o2, 2);
    uint32_t o3[2] = { 4, 5 };
    dynamicOffsets.setOffsets(2, o3, 2);

    const auto [count, offsets] = dynamicOffsets.getOffsets();
    EXPECT_EQ(count, 6);
    EXPECT_EQ(offsets[0], 0);
    EXPECT_EQ(offsets[1], 1);
    EXPECT_EQ(offsets[2], 2);
    EXPECT_EQ(offsets[3], 3);
    EXPECT_EQ(offsets[4], 4);
    EXPECT_EQ(offsets[5], 5);
};

TEST(MetalDynamicOffsets, removal) {
    filament::backend::MetalDynamicOffsets dynamicOffsets;
    uint32_t o1[2] = { 2, 3 };
    dynamicOffsets.setOffsets(1, o1, 2);
    uint32_t o2[2] = { 0, 1 };
    dynamicOffsets.setOffsets(0, o2, 2);
    uint32_t o3[2] = { 4, 5 };
    dynamicOffsets.setOffsets(2, o3, 2);
    dynamicOffsets.setOffsets(1, nullptr, 0);

    const auto [count, offsets] = dynamicOffsets.getOffsets();
    EXPECT_EQ(count, 4);
    EXPECT_EQ(offsets[0], 0);
    EXPECT_EQ(offsets[1], 1);
    EXPECT_EQ(offsets[2], 4);
    EXPECT_EQ(offsets[3], 5);
};

TEST(MetalDynamicOffsets, resize) {
    filament::backend::MetalDynamicOffsets dynamicOffsets;
    uint32_t o1[2] = { 2, 3 };
    dynamicOffsets.setOffsets(1, o1, 2);
    uint32_t o2[2] = { 0, 1 };
    dynamicOffsets.setOffsets(0, o2, 2);
    uint32_t o3[2] = { 6, 7 };
    dynamicOffsets.setOffsets(2, o3, 2);
    uint32_t o4[4] = { 2, 3, 4, 5 };
    dynamicOffsets.setOffsets(1, o4, 4);

    const auto [count, offsets] = dynamicOffsets.getOffsets();
    EXPECT_EQ(count, 8);
    EXPECT_EQ(offsets[0], 0);
    EXPECT_EQ(offsets[1], 1);
    EXPECT_EQ(offsets[2], 2);
    EXPECT_EQ(offsets[3], 3);
    EXPECT_EQ(offsets[4], 4);
    EXPECT_EQ(offsets[5], 5);
    EXPECT_EQ(offsets[6], 6);
    EXPECT_EQ(offsets[7], 7);
};

TEST(MetalDynamicOffsets, dirty) {
    filament::backend::MetalDynamicOffsets dynamicOffsets;
    EXPECT_FALSE(dynamicOffsets.isDirty());

    uint32_t o1[2] = { 2, 3 };
    dynamicOffsets.setOffsets(1, o1, 2);
    EXPECT_TRUE(dynamicOffsets.isDirty());

    dynamicOffsets.setDirty(false);
    EXPECT_FALSE(dynamicOffsets.isDirty());

    // Setting the same offsets should not mark the offsets as dirty
    dynamicOffsets.setOffsets(1, o1, 2);
    EXPECT_FALSE(dynamicOffsets.isDirty());

    // Resizing the offsets should mark the offsets as dirty
    uint32_t o2[3] = { 4, 5, 6 };
    dynamicOffsets.setOffsets(1, o2, 3);
    EXPECT_TRUE(dynamicOffsets.isDirty());
};

};

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
