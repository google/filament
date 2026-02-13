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

#include "webgpu/WebGPUTextureHelpers.h"

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

namespace test {

TEST(composeSwizzle, allUndefined) {
    const wgpu::TextureComponentSwizzle undefinedPrevious{
        .r = wgpu::ComponentSwizzle::Undefined,
        .g = wgpu::ComponentSwizzle::Undefined,
        .b = wgpu::ComponentSwizzle::Undefined,
        .a = wgpu::ComponentSwizzle::Undefined,
    };
    const wgpu::TextureComponentSwizzle undefinedNext{
        .r = wgpu::ComponentSwizzle::Undefined,
        .g = wgpu::ComponentSwizzle::Undefined,
        .b = wgpu::ComponentSwizzle::Undefined,
        .a = wgpu::ComponentSwizzle::Undefined,
    };
    const wgpu::TextureComponentSwizzle result{ filament::backend::composeSwizzle(undefinedPrevious,
            undefinedNext) };
    EXPECT_EQ(result.r, wgpu::ComponentSwizzle::Undefined);
    EXPECT_EQ(result.g, wgpu::ComponentSwizzle::Undefined);
    EXPECT_EQ(result.b, wgpu::ComponentSwizzle::Undefined);
    EXPECT_EQ(result.a, wgpu::ComponentSwizzle::Undefined);
}

TEST(composeSwizzle, previousUndefined) {
    const wgpu::TextureComponentSwizzle undefinedPrevious{
        .r = wgpu::ComponentSwizzle::Undefined,
        .g = wgpu::ComponentSwizzle::Undefined,
        .b = wgpu::ComponentSwizzle::Undefined,
        .a = wgpu::ComponentSwizzle::Undefined,
    };
    const wgpu::TextureComponentSwizzle definedNext{
        .r = wgpu::ComponentSwizzle::G,
        .g = wgpu::ComponentSwizzle::R,
        .b = wgpu::ComponentSwizzle::A,
        .a = wgpu::ComponentSwizzle::B,
    };
    const wgpu::TextureComponentSwizzle result{ filament::backend::composeSwizzle(undefinedPrevious,
            definedNext) };
    EXPECT_EQ(result.r, wgpu::ComponentSwizzle::G);
    EXPECT_EQ(result.g, wgpu::ComponentSwizzle::R);
    EXPECT_EQ(result.b, wgpu::ComponentSwizzle::A);
    EXPECT_EQ(result.a, wgpu::ComponentSwizzle::B);
}

TEST(composeSwizzle, nextUndefined) {
    const wgpu::TextureComponentSwizzle definedPrevious{
        .r = wgpu::ComponentSwizzle::G,
        .g = wgpu::ComponentSwizzle::R,
        .b = wgpu::ComponentSwizzle::A,
        .a = wgpu::ComponentSwizzle::B,
    };
    const wgpu::TextureComponentSwizzle undefinedNext{
        .r = wgpu::ComponentSwizzle::Undefined,
        .g = wgpu::ComponentSwizzle::Undefined,
        .b = wgpu::ComponentSwizzle::Undefined,
        .a = wgpu::ComponentSwizzle::Undefined,
    };
    const wgpu::TextureComponentSwizzle result{ filament::backend::composeSwizzle(definedPrevious,
            undefinedNext) };
    EXPECT_EQ(result.r, wgpu::ComponentSwizzle::G);
    EXPECT_EQ(result.g, wgpu::ComponentSwizzle::R);
    EXPECT_EQ(result.b, wgpu::ComponentSwizzle::A);
    EXPECT_EQ(result.a, wgpu::ComponentSwizzle::B);
}

TEST(composeSwizzle, bothDefinedNoSwizzling) {
    const wgpu::TextureComponentSwizzle previousOneToOneMapping{
        .r = wgpu::ComponentSwizzle::R,
        .g = wgpu::ComponentSwizzle::G,
        .b = wgpu::ComponentSwizzle::B,
        .a = wgpu::ComponentSwizzle::A,
    };
    const wgpu::TextureComponentSwizzle nextOneToOneMapping{
        .r = wgpu::ComponentSwizzle::R,
        .g = wgpu::ComponentSwizzle::G,
        .b = wgpu::ComponentSwizzle::B,
        .a = wgpu::ComponentSwizzle::A,
    };
    const wgpu::TextureComponentSwizzle result{ filament::backend::composeSwizzle(
            previousOneToOneMapping, nextOneToOneMapping) };
    EXPECT_EQ(result.r, wgpu::ComponentSwizzle::R);
    EXPECT_EQ(result.g, wgpu::ComponentSwizzle::G);
    EXPECT_EQ(result.b, wgpu::ComponentSwizzle::B);
    EXPECT_EQ(result.a, wgpu::ComponentSwizzle::A);
}

TEST(composeSwizzle, bothDefined) {
    const wgpu::TextureComponentSwizzle previous{
        .r = wgpu::ComponentSwizzle::G,
        .g = wgpu::ComponentSwizzle::R,
        .b = wgpu::ComponentSwizzle::A,
        .a = wgpu::ComponentSwizzle::B,
    };
    const wgpu::TextureComponentSwizzle next{
        .r = wgpu::ComponentSwizzle::A,
        .g = wgpu::ComponentSwizzle::B,
        .b = wgpu::ComponentSwizzle::G,
        .a = wgpu::ComponentSwizzle::R,
    };
    const wgpu::TextureComponentSwizzle result{ filament::backend::composeSwizzle(previous,
            next) };
    EXPECT_EQ(result.r, wgpu::ComponentSwizzle::B);
    EXPECT_EQ(result.g, wgpu::ComponentSwizzle::A);
    EXPECT_EQ(result.b, wgpu::ComponentSwizzle::R);
    EXPECT_EQ(result.a, wgpu::ComponentSwizzle::G);
}

TEST(composeSwizzle, bothPartiallyDefined) {
    const wgpu::TextureComponentSwizzle partiallyDefinedPrevious{
        .r = wgpu::ComponentSwizzle::Undefined,
        .g = wgpu::ComponentSwizzle::B,
        .b = wgpu::ComponentSwizzle::G,
        .a = wgpu::ComponentSwizzle::Undefined,
    };
    const wgpu::TextureComponentSwizzle partiallyDefinedNext{
        .r = wgpu::ComponentSwizzle::A,
        .g = wgpu::ComponentSwizzle::G,
        .b = wgpu::ComponentSwizzle::Undefined,
        .a = wgpu::ComponentSwizzle::R,
    };
    const wgpu::TextureComponentSwizzle result{ filament::backend::composeSwizzle(
            partiallyDefinedPrevious, partiallyDefinedNext) };
    EXPECT_EQ(result.r, wgpu::ComponentSwizzle::A);
    EXPECT_EQ(result.g, wgpu::ComponentSwizzle::B);
    EXPECT_EQ(result.b, wgpu::ComponentSwizzle::G);
    EXPECT_EQ(result.a, wgpu::ComponentSwizzle::R);
}

TEST(composeSwizzle, onesAndZeros) {
    const wgpu::TextureComponentSwizzle previousWithOnesAndZeros{
        .r = wgpu::ComponentSwizzle::Zero,
        .g = wgpu::ComponentSwizzle::R,
        .b = wgpu::ComponentSwizzle::One,
        .a = wgpu::ComponentSwizzle::B,
    };
    const wgpu::TextureComponentSwizzle nextWithOnesAndZeros{
        .r = wgpu::ComponentSwizzle::One,
        .g = wgpu::ComponentSwizzle::Zero,
        .b = wgpu::ComponentSwizzle::Undefined,
        .a = wgpu::ComponentSwizzle::R,
    };
    const wgpu::TextureComponentSwizzle result{ filament::backend::composeSwizzle(
            previousWithOnesAndZeros, nextWithOnesAndZeros) };
    EXPECT_EQ(result.r, wgpu::ComponentSwizzle::One);
    EXPECT_EQ(result.g, wgpu::ComponentSwizzle::Zero);
    EXPECT_EQ(result.b, wgpu::ComponentSwizzle::One);
    EXPECT_EQ(result.a, wgpu::ComponentSwizzle::Zero);
}

} // namespace test
