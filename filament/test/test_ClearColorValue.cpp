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

#include <gtest/gtest.h>

#include <backend/DriverEnums.h>

#include <math/vec4.h>

#include <cstdint>
#include <limits>

using namespace filament::backend;
using namespace filament::math;

TEST(ClearColorValue, DefaultIsAutoZero) {
    // A default-constructed ClearColorValue defers the type family to the backend so that
    // `params.clearColor = {}` does not silently force FLOAT onto an integer-format attachment.
    constexpr ClearColorValue v;
    EXPECT_EQ(v.type, ClearColorValue::Type::AUTO);
    EXPECT_EQ(v.color[0], 0.0);
    EXPECT_EQ(v.color[1], 0.0);
    EXPECT_EQ(v.color[2], 0.0);
    EXPECT_EQ(v.color[3], 0.0);
}

TEST(ClearColorValue, ImplicitFromFloat4) {
    ClearColorValue v = float4{ 0.25f, 0.5f, 0.75f, 1.0f };
    EXPECT_EQ(v.type, ClearColorValue::Type::FLOAT);
    EXPECT_FLOAT_EQ(static_cast<float>(v.color[0]), 0.25f);
    EXPECT_FLOAT_EQ(static_cast<float>(v.color[1]), 0.5f);
    EXPECT_FLOAT_EQ(static_cast<float>(v.color[2]), 0.75f);
    EXPECT_FLOAT_EQ(static_cast<float>(v.color[3]), 1.0f);
}

TEST(ClearColorValue, ImplicitFromInt4PreservesFullRange) {
    constexpr int32_t lo = std::numeric_limits<int32_t>::min();
    constexpr int32_t hi = std::numeric_limits<int32_t>::max();
    ClearColorValue v = int4{ lo, hi, -1, 0 };
    EXPECT_EQ(v.type, ClearColorValue::Type::INT);
    EXPECT_EQ(static_cast<int32_t>(v.color[0]), lo);
    EXPECT_EQ(static_cast<int32_t>(v.color[1]), hi);
    EXPECT_EQ(static_cast<int32_t>(v.color[2]), -1);
    EXPECT_EQ(static_cast<int32_t>(v.color[3]), 0);
}

TEST(ClearColorValue, ImplicitFromUInt4PreservesFullRange) {
    constexpr uint32_t hi = std::numeric_limits<uint32_t>::max();
    ClearColorValue v = uint4{ 0u, 1u, 0xCAFEBABEu, hi };
    EXPECT_EQ(v.type, ClearColorValue::Type::UINT);
    EXPECT_EQ(static_cast<uint32_t>(v.color[0]), 0u);
    EXPECT_EQ(static_cast<uint32_t>(v.color[1]), 1u);
    EXPECT_EQ(static_cast<uint32_t>(v.color[2]), 0xCAFEBABEu);
    EXPECT_EQ(static_cast<uint32_t>(v.color[3]), hi);
}

TEST(ClearColorValue, Double4WithExplicitTypeKeepsType) {
    // Used by the Renderer::ClearOptions adapter, which forwards a double4 plus a Type tag.
    double4 const values{ 1.0, 2.0, 3.0, 4.0 };

    ClearColorValue const asAuto(values, ClearColorValue::Type::AUTO);
    EXPECT_EQ(asAuto.type, ClearColorValue::Type::AUTO);
    EXPECT_EQ(asAuto.color[2], 3.0);

    ClearColorValue const asUInt(values, ClearColorValue::Type::UINT);
    EXPECT_EQ(asUInt.type, ClearColorValue::Type::UINT);

    // Default Type for the double4 ctor is AUTO: a double4 alone carries no type-family info,
    // so the backend resolves against the attachment format.
    ClearColorValue const defaulted{ values };
    EXPECT_EQ(defaulted.type, ClearColorValue::Type::AUTO);
}

TEST(ClearColorValue, AutoSurvivesCopy) {
    // The adapter at filament/src/details/Renderer.cpp builds a ClearColorValue with AUTO and
    // hands it down through frame-graph descriptors before it reaches the backend. AUTO must not
    // be silently rewritten by any of those constructors / assignments.
    ClearColorValue src(double4{ 0.0, 0.0, 0.0, 0.0 }, ClearColorValue::Type::AUTO);
    ClearColorValue copy = src;
    EXPECT_EQ(copy.type, ClearColorValue::Type::AUTO);

    ClearColorValue assigned;
    assigned = src;
    EXPECT_EQ(assigned.type, ClearColorValue::Type::AUTO);
}
