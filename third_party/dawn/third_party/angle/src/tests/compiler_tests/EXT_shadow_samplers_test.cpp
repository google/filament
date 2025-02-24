//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_shadow_samplers_test.cpp:
//   Test for EXT_shadow_samplers
//

#include "tests/test_utils/ShaderExtensionTest.h"

using EXTShadowSamplersTest = sh::ShaderExtensionTest;

namespace
{
const char EXTPragma[] = "#extension GL_EXT_shadow_samplers : require\n";

// Shader calling shadow2DEXT()
const char ESSL100_ShadowSamplersShader[] =
    R"(
    precision mediump float;
    varying vec3 texCoord0v;
    uniform sampler2DShadow tex;
    void main()
    {
        float color = shadow2DEXT(tex, texCoord0v);
    })";

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTShadowSamplersTest, CompileFailsWithoutExtension)
{
    mResources.EXT_shadow_samplers = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTShadowSamplersTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_shadow_samplers = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTShadowSamplersTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_shadow_samplers = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTShadowSamplersTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_ShadowSamplersShader)));

}  // anonymous namespace