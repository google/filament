//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramParameterTest: validate parameters of ProgramParameter

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class ProgramParameterTest : public ANGLETest<>
{
  protected:
    ProgramParameterTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class ProgramParameterTestES31 : public ProgramParameterTest
{
  protected:
    ProgramParameterTestES31() : ProgramParameterTest() {}
};

// If es version < 3.1, PROGRAM_SEPARABLE is not supported.
TEST_P(ProgramParameterTest, ValidatePname)
{
    GLuint program = glCreateProgram();
    ASSERT_NE(program, 0u);

    glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        ASSERT_GL_ERROR(GL_INVALID_ENUM);
    }
    else
    {
        ASSERT_GL_NO_ERROR();
    }

    glDeleteProgram(program);
}

// Validate parameters for ProgramParameter when pname is PROGRAM_SEPARABLE.
TEST_P(ProgramParameterTestES31, ValidateParameters)
{
    GLuint program = glCreateProgram();
    ASSERT_NE(program, 0u);

    glProgramParameteri(0, GL_PROGRAM_SEPARABLE, GL_TRUE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, 2);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glDeleteProgram(program);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramParameterTest);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(ProgramParameterTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramParameterTestES31);
ANGLE_INSTANTIATE_TEST_ES31(ProgramParameterTestES31);
}  // namespace
