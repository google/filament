//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_frag_depth_test.cpp:
//   Test for EXT_frag_depth
//

#include "tests/test_utils/ShaderExtensionTest.h"

using EXTFragDepthTest = sh::ShaderExtensionTest;

namespace
{
const char EXTPragma[] = "#extension GL_EXT_frag_depth : require\n";

// Shader setting gl_FragDepthEXT
const char ESSL100_FragDepthShader[] =

    R"(
    precision mediump float;

    void main()
    {
        gl_FragDepthEXT = 1.0;
    })";

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTFragDepthTest, CompileFailsWithoutExtension)
{
    mResources.EXT_frag_depth = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTFragDepthTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTFragDepthTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTFragDepthTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_FragDepthShader)));

}  // anonymous namespace
