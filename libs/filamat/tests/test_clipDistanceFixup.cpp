/*
* Copyright (C) 2023 The Android Open Source Project
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

#include "SpirvFixup.h"

TEST(ClipDistanceFixup, NoReplacement) {
    std::string disassembly =
            "       OpDecorate %gl_PerVertex Block\n"
            "       %void = OpTypeVoid\n"
            "       %3 = OpTypeFunction %void\n";
    std::string expected =
            "       OpDecorate %gl_PerVertex Block\n"
            "       %void = OpTypeVoid\n"
            "       %3 = OpTypeFunction %void\n";
    EXPECT_EQ(filamat::fixupClipDistance(disassembly), false);
    EXPECT_EQ(disassembly, expected);
}

TEST(ClipDistanceFixup, BasicReplacement) {
    std::string disassembly =
            "       OpDecorate %gl_PerVertex Block\n"
            "       OpDecorate %filament_gl_ClipDistance Location 100\n"
            "       %void = OpTypeVoid\n"
            "       %3 = OpTypeFunction %void\n";
    std::string expected =
            "       OpDecorate %gl_PerVertex Block\n"
            "       OpDecorate %filament_gl_ClipDistance BuiltIn ClipDistance\n"
            "       %void = OpTypeVoid\n"
            "       %3 = OpTypeFunction %void\n";
    EXPECT_EQ(filamat::fixupClipDistance(disassembly), true);
    EXPECT_EQ(disassembly, expected);
}

TEST(ClipDistanceFixup, NoNewline) {
    std::string disassembly =
            "       OpDecorate %gl_PerVertex Block\n"
            "       OpDecorate %filament_gl_ClipDistance Location 100";
    std::string expected =
            "       OpDecorate %gl_PerVertex Block\n"
            "       OpDecorate %filament_gl_ClipDistance BuiltIn ClipDistance";
    EXPECT_EQ(filamat::fixupClipDistance(disassembly), true);
    EXPECT_EQ(disassembly, expected);
}
