//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VaryingPacking_unittest.cpp:
//   Tests for ANGLE's internal varying packing algorithm.
//

#include <gtest/gtest.h>
// 'None' is defined as 'struct None {};' in
// third_party/googletest/src/googletest/include/gtest/internal/gtest-type-util.h.
// But 'None' is also define as a numberic constant 0L in <X11/X.h>.
// So we need to include gtest first to avoid such conflict.

#include "libANGLE/Program.h"
#include "libANGLE/VaryingPacking.h"

using namespace gl;

namespace
{

class VaryingPackingTest : public ::testing::TestWithParam<GLuint>
{
  protected:
    VaryingPackingTest() {}

    bool testVaryingPacking(GLint maxVaryings,
                            PackMode packMode,
                            const std::vector<sh::ShaderVariable> &shVaryings)
    {
        ProgramMergedVaryings mergedVaryings;
        for (const sh::ShaderVariable &shVarying : shVaryings)
        {
            ProgramVaryingRef ref;
            ref.frontShader      = &shVarying;
            ref.backShader       = &shVarying;
            ref.frontShaderStage = ShaderType::Vertex;
            ref.backShaderStage  = ShaderType::Fragment;
            mergedVaryings.push_back(ref);
        }

        InfoLog infoLog;
        std::vector<std::string> transformFeedbackVaryings;

        VaryingPacking varyingPacking;
        return varyingPacking.collectAndPackUserVaryings(
            infoLog, maxVaryings, packMode, ShaderType::Vertex, ShaderType::Fragment,
            mergedVaryings, transformFeedbackVaryings, false);
    }

    // Uses the "relaxed" ANGLE packing mode.
    bool packVaryings(GLint maxVaryings, const std::vector<sh::ShaderVariable> &shVaryings)
    {
        return testVaryingPacking(maxVaryings, PackMode::ANGLE_RELAXED, shVaryings);
    }

    // Uses the stricter WebGL style packing rules.
    bool packVaryingsStrict(GLint maxVaryings, const std::vector<sh::ShaderVariable> &shVaryings)
    {
        return testVaryingPacking(maxVaryings, PackMode::WEBGL_STRICT, shVaryings);
    }

    const int kMaxVaryings = GetParam();
};

std::vector<sh::ShaderVariable> MakeVaryings(GLenum type, size_t count, size_t arraySize)
{
    std::vector<sh::ShaderVariable> varyings;

    for (size_t index = 0; index < count; ++index)
    {
        std::stringstream strstr;
        strstr << type << index;

        sh::ShaderVariable varying;
        varying.type       = type;
        varying.precision  = GL_MEDIUM_FLOAT;
        varying.name       = strstr.str();
        varying.mappedName = strstr.str();
        if (arraySize > 0)
        {
            varying.arraySizes.push_back(static_cast<unsigned int>(arraySize));
        }
        varying.staticUse     = true;
        varying.interpolation = sh::INTERPOLATION_FLAT;
        varying.isInvariant   = false;

        varyings.push_back(varying);
    }

    return varyings;
}

void AddVaryings(std::vector<sh::ShaderVariable> *varyings,
                 GLenum type,
                 size_t count,
                 size_t arraySize)
{
    const auto &newVaryings = MakeVaryings(type, count, arraySize);
    varyings->insert(varyings->end(), newVaryings.begin(), newVaryings.end());
}

// Test that a single varying can't overflow the packing.
TEST_P(VaryingPackingTest, OneVaryingLargerThanMax)
{
    ASSERT_FALSE(packVaryings(1, MakeVaryings(GL_FLOAT_MAT4, 1, 0)));
}

// This will overflow the available varying space.
TEST_P(VaryingPackingTest, MaxPlusOneVaryingVec3)
{
    ASSERT_FALSE(packVaryings(kMaxVaryings, MakeVaryings(GL_FLOAT_VEC3, kMaxVaryings + 1, 0)));
}

// This will overflow the available varying space.
TEST_P(VaryingPackingTest, MaxPlusOneVaryingVec3Array)
{
    ASSERT_FALSE(packVaryings(kMaxVaryings, MakeVaryings(GL_FLOAT_VEC3, kMaxVaryings / 2 + 1, 2)));
}

// This will overflow the available varying space.
TEST_P(VaryingPackingTest, MaxVaryingVec3AndOneVec2)
{
    std::vector<sh::ShaderVariable> varyings = MakeVaryings(GL_FLOAT_VEC3, kMaxVaryings, 0);
    AddVaryings(&varyings, GL_FLOAT_VEC2, 1, 0);
    ASSERT_FALSE(packVaryings(kMaxVaryings, varyings));
}

// This should work since two vec2s are packed in a single register.
TEST_P(VaryingPackingTest, MaxPlusOneVaryingVec2)
{
    ASSERT_TRUE(packVaryings(kMaxVaryings, MakeVaryings(GL_FLOAT_VEC2, kMaxVaryings + 1, 0)));
}

// Same for this one as above.
TEST_P(VaryingPackingTest, TwiceMaxVaryingVec2)
{
    ASSERT_TRUE(packVaryings(kMaxVaryings, MakeVaryings(GL_FLOAT_VEC2, kMaxVaryings * 2, 0)));
}

// This should not work since it overflows available varying space.
TEST_P(VaryingPackingTest, TooManyVaryingVec2)
{
    ASSERT_FALSE(packVaryings(kMaxVaryings, MakeVaryings(GL_FLOAT_VEC2, kMaxVaryings * 2 + 1, 0)));
}

// This should work according to the example GL packing rules - the float varyings are slotted
// into the end of the vec3 varying arrays.
TEST_P(VaryingPackingTest, MaxVaryingVec3ArrayAndFloatArrays)
{
    std::vector<sh::ShaderVariable> varyings = MakeVaryings(GL_FLOAT_VEC3, kMaxVaryings / 2, 2);
    AddVaryings(&varyings, GL_FLOAT, kMaxVaryings / 2, 2);
    ASSERT_TRUE(packVaryings(kMaxVaryings, varyings));
}

// This should not work - it has one too many float arrays.
TEST_P(VaryingPackingTest, MaxVaryingVec3ArrayAndMaxPlusOneFloatArray)
{
    std::vector<sh::ShaderVariable> varyings = MakeVaryings(GL_FLOAT_VEC3, kMaxVaryings / 2, 2);
    AddVaryings(&varyings, GL_FLOAT, kMaxVaryings / 2 + 1, 2);
    ASSERT_FALSE(packVaryings(kMaxVaryings, varyings));
}

// WebGL should fail to pack max+1 vec2 arrays, unlike our more relaxed packing.
TEST_P(VaryingPackingTest, MaxPlusOneMat2VaryingsFailsWebGL)
{
    auto varyings = MakeVaryings(GL_FLOAT_MAT2, kMaxVaryings / 2 + 1, 0);
    ASSERT_FALSE(packVaryingsStrict(kMaxVaryings, varyings));
}

// Makes separate tests for different values of kMaxVaryings.
INSTANTIATE_TEST_SUITE_P(, VaryingPackingTest, ::testing::Values(1, 4, 8));

}  // anonymous namespace
