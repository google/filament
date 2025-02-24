//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlFragDataNotModified_test.cpp:
//   Test that the properties of built-in gl_FragData are not modified when a shader is compiled
//   multiple times.
//

#include "tests/test_utils/ShaderCompileTreeTest.h"

namespace
{

class GlFragDataNotModifiedTest : public sh::ShaderCompileTreeTest
{
  public:
    GlFragDataNotModifiedTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->MaxDrawBuffers   = 4;
        resources->EXT_draw_buffers = 1;
    }

    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES2_SPEC; }
};

// Test a bug where we could modify the value of a builtin variable.
TEST_F(GlFragDataNotModifiedTest, BuiltinRewritingBug)
{
    const std::string &shaderString =
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "    gl_FragData[gl_MaxDrawBuffers] = vec4(0.0);\n"
        "}";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure\n";
    }
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure\n";
    }
}

}  // anonymous namespace
