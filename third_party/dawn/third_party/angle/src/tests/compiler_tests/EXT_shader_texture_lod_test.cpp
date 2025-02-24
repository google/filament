//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EXT_shader_texture_lod_test.cpp:
//   Test for EXT_shader_texture_lod
//

#include "tests/test_utils/ShaderExtensionTest.h"

using EXTShaderTextureLodTest = sh::ShaderExtensionTest;

namespace
{
const char EXTPragma[] = "#extension GL_EXT_shader_texture_lod : require\n";

// Shader calling texture2DLodEXT()
const char ESSL100_TextureLodShader[] =
    R"(
    precision mediump float;
    varying vec2 texCoord0v;
    uniform float lod;
    uniform sampler2D tex;
    void main()
    {
        vec4 color = texture2DLodEXT(tex, texCoord0v, lod);
    })";

// Extension flag is required to compile properly. Expect failure when it is
// not present.
TEST_P(EXTShaderTextureLodTest, CompileFailsWithoutExtension)
{
    mResources.EXT_shader_texture_lod = 0;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(EXTPragma));
}

// Extension directive is required to compile properly. Expect failure when
// it is not present.
TEST_P(EXTShaderTextureLodTest, CompileFailsWithExtensionWithoutPragma)
{
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(""));
}

// With extension flag and extension directive, compiling succeeds.
// Also test that the extension directive state is reset correctly.
TEST_P(EXTShaderTextureLodTest, CompileSucceedsWithExtensionAndPragma)
{
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
    // Test reset functionality.
    EXPECT_FALSE(TestShaderCompile(""));
    EXPECT_TRUE(TestShaderCompile(EXTPragma));
}

// The SL #version 100 shaders that are correct work similarly
// in both GL2 and GL3, with and without the version string.
INSTANTIATE_TEST_SUITE_P(CorrectESSL100Shaders,
                         EXTShaderTextureLodTest,
                         Combine(Values(SH_GLES2_SPEC),
                                 Values(sh::ESSLVersion100),
                                 Values(ESSL100_TextureLodShader)));

}  // anonymous namespace
