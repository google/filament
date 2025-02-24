//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FragDepth_test.cpp:
//   Test for GLES SL 3.0 gl_FragDepth variable implementation.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"

namespace
{
const char ESSLVersion100[] = "#version 100\n";
const char ESSLVersion300[] = "#version 300 es\n";
const char EXTFDPragma[]    = "#extension GL_EXT_frag_depth : require\n";
}  // namespace

class FragDepthTest : public testing::TestWithParam<bool>
{
  protected:
    void SetUp() override
    {
        sh::InitBuiltInResources(&mResources);
        mCompiler                 = nullptr;
        mResources.EXT_frag_depth = GetParam();
    }

    void TearDown() override { DestroyCompiler(); }
    void DestroyCompiler()
    {
        if (mCompiler)
        {
            sh::Destruct(mCompiler);
            mCompiler = nullptr;
        }
    }

    void InitializeCompiler()
    {
        DestroyCompiler();
        mCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_GLES3_SPEC,
                                          SH_GLSL_COMPATIBILITY_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }

    testing::AssertionResult TestShaderCompile(const char *version,
                                               const char *pragma,
                                               const char *shader)
    {
        const char *shaderStrings[] = {version, pragma, shader};
        bool success                = sh::Compile(mCompiler, shaderStrings, 3, {});
        if (success)
        {
            return ::testing::AssertionSuccess() << "Compilation success";
        }
        return ::testing::AssertionFailure() << sh::GetInfoLog(mCompiler);
    }

  protected:
    ShBuiltInResources mResources;
    ShHandle mCompiler;
};

// The GLES SL 3.0 built-in variable gl_FragDepth fails to compile with GLES SL 1.0.
TEST_P(FragDepthTest, CompileFailsESSL100)
{
    static const char shaderString[] =
        "precision mediump float;\n"
        "void main() { \n"
        "    gl_FragDepth = 1.0;\n"
        "}\n";

    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(ESSLVersion100, "", shaderString));
    EXPECT_FALSE(TestShaderCompile("", "", shaderString));
    EXPECT_FALSE(TestShaderCompile("", EXTFDPragma, shaderString));
}

// The GLES SL 3.0 built-in variable gl_FragDepth compiles with GLES SL 3.0.
TEST_P(FragDepthTest, CompileSucceedsESSL300)
{
    static const char shaderString[] =
        "precision mediump float;\n"
        "void main() { \n"
        "    gl_FragDepth = 1.0;\n"
        "}\n";
    InitializeCompiler();
    EXPECT_TRUE(TestShaderCompile(ESSLVersion300, "", shaderString));
}

// Using #extension GL_EXT_frag_depth in GLSL ES 3.0 shader fails to compile.
TEST_P(FragDepthTest, ExtensionFDFailsESSL300)
{
    static const char shaderString[] =
        "precision mediump float;\n"
        "out vec4 fragColor;\n"
        "void main() { \n"
        "    fragColor = vec4(1.0);\n"
        "}\n";
    InitializeCompiler();
    EXPECT_FALSE(TestShaderCompile(ESSLVersion300, EXTFDPragma, shaderString));
}

// The tests should pass regardless whether the EXT_frag_depth is on or not.
INSTANTIATE_TEST_SUITE_P(FragDepthTests, FragDepthTest, testing::Values(false, true));
