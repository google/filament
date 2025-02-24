//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// NV_draw_buffers_test.cpp:
//   Test for NV_draw_buffers setting
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class NVDrawBuffersTest : public MatchOutputCodeTest
{
  public:
    NVDrawBuffersTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT)
    {
        ShBuiltInResources *resources = getResources();
        resources->MaxDrawBuffers     = 8;
        resources->EXT_draw_buffers   = 1;
        resources->NV_draw_buffers    = 1;
    }
};

TEST_F(NVDrawBuffersTest, NVDrawBuffers)
{
    const std::string &shaderString =
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragData[0] = vec4(1.0);\n"
        "   gl_FragData[1] = vec4(0.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("GL_NV_draw_buffers"));
    ASSERT_FALSE(foundInCode("GL_EXT_draw_buffers"));
}
