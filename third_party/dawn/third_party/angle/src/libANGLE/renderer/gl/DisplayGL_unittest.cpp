//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "tests/angle_unittests_utils.h"

namespace rx
{

extern std::string SanitizeRendererString(std::string rendererString);
extern std::string SanitizeVersionString(std::string versionString,
                                         bool isES,
                                         bool includeFullVersion);

namespace testing
{

namespace
{

TEST(DisplayGLTest, SanitizeRendererStringIntel)
{
    std::string testString      = "Mesa DRI Intel(R) HD Graphics 4000 (IVB GT2)";
    std::string testExpectation = "Mesa DRI Intel(R) HD Graphics 4000 (IVB GT2)";
    EXPECT_EQ(SanitizeRendererString(testString), testExpectation);
}

TEST(DisplayGLTest, SanitizeRendererStringLLVMPipe)
{
    std::string testString      = "llvmpipe (LLVM 11.0.0, 256 bits)";
    std::string testExpectation = "llvmpipe (LLVM 11.0.0, 256 bits)";
    EXPECT_EQ(SanitizeRendererString(testString), testExpectation);
}

TEST(DisplayGLTest, SanitizeRendererStringRadeonVega)
{
    std::string testString      = "Radeon RX Vega";
    std::string testExpectation = "Radeon RX Vega";
    EXPECT_EQ(SanitizeRendererString(testString), testExpectation);
}

TEST(DisplayGLTest, SanitizeRendererStringRadeonTM)
{
    std::string testString =
        "AMD Radeon (TM) RX 460 Graphics (POLARIS11, DRM 3.35.0, 5.4.0-65-generic, LLVM 11.0.0)";
    std::string testExpectation = "AMD Radeon (TM) RX 460 Graphics (POLARIS11)";
    EXPECT_EQ(SanitizeRendererString(testString), testExpectation);
}

TEST(DisplayGLTest, SanitizeRendererStringRadeonWithoutGeneration)
{
    std::string testString      = "AMD Radeon RX 5700 (DRM 3.35.0, 5.4.0-65-generic, LLVM 11.0.0)";
    std::string testExpectation = "AMD Radeon RX 5700";
    EXPECT_EQ(SanitizeRendererString(testString), testExpectation);
}

TEST(DisplayGLTest, SanitizeVersionStringOpenGLMissing)
{
    std::string testString      = "4.6.0 NVIDIA 391.76";
    std::string testExpectation = "OpenGL 4.6.0 NVIDIA 391.76";
    EXPECT_EQ(SanitizeVersionString(testString, false, true), testExpectation);
}

// Note: OpenGL renderers with this prefix don't actually seem to be present in the wild
TEST(DisplayGLTest, SanitizeVersionStringOpenGLPresent)
{
    std::string testString      = "OpenGL 4.5.0 - Build 22.20.16.4749";
    std::string testExpectation = "OpenGL 4.5.0 - Build 22.20.16.4749";
    EXPECT_EQ(SanitizeVersionString(testString, false, true), testExpectation);
}

TEST(DisplayGLTest, SanitizeVersionStringOpenGLESMissing)
{
    std::string testString      = "4.6.0 NVIDIA 419.67";
    std::string testExpectation = "OpenGL ES 4.6.0 NVIDIA 419.67";
    EXPECT_EQ(SanitizeVersionString(testString, true, true), testExpectation);
}

TEST(DisplayGLTest, SanitizeVersionStringOpenGLESPresent)
{
    std::string testString      = "OpenGL ES 3.2 v1.r12p0-04rel0.44f2946824bb8739781564bffe2110c9";
    std::string testExpectation = "OpenGL ES 3.2 v1.r12p0-04rel0.44f2946824bb8739781564bffe2110c9";
    EXPECT_EQ(SanitizeVersionString(testString, true, true), testExpectation);
}

TEST(DisplayGLTest, SanitizeVersionStringOpenGLMissingLimited)
{
    std::string testString      = "4.6.0 NVIDIA 391.76";
    std::string testExpectation = "OpenGL 4.6.0";
    EXPECT_EQ(SanitizeVersionString(testString, false, false), testExpectation);
}

// Note: OpenGL renderers with this prefix don't actually seem to be present in the wild
TEST(DisplayGLTest, SanitizeVersionStringOpenGLPresentLimited)
{
    std::string testString      = "OpenGL 4.5.0 - Build 22.20.16.4749";
    std::string testExpectation = "OpenGL 4.5.0";
    EXPECT_EQ(SanitizeVersionString(testString, false, false), testExpectation);
}

TEST(DisplayGLTest, SanitizeVersionStringOpenGLESMissingLimited)
{
    std::string testString      = "4.6.0 NVIDIA 419.67";
    std::string testExpectation = "OpenGL ES 4.6.0";
    EXPECT_EQ(SanitizeVersionString(testString, true, false), testExpectation);
}

TEST(DisplayGLTest, SanitizeVersionStringOpenGLESPresentLimited)
{
    std::string testString      = "OpenGL ES 3.2 v1.r12p0-04rel0.44f2946824bb8739781564bffe2110c9";
    std::string testExpectation = "OpenGL ES 3.2";
    EXPECT_EQ(SanitizeVersionString(testString, true, false), testExpectation);
}

}  // anonymous namespace

}  // namespace testing

}  // namespace rx
