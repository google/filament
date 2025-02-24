//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateGLFragColorBroadcast_test.cpp:
//   Tests for gl_FragColor broadcast behavior emulation.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

const int kMaxDrawBuffers = 2;

class EmulateGLFragColorBroadcastTest : public MatchOutputCodeTest
{
  public:
    EmulateGLFragColorBroadcastTest()
        : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_GLSL_COMPATIBILITY_OUTPUT)
    {
        getResources()->MaxDrawBuffers   = kMaxDrawBuffers;
        getResources()->EXT_draw_buffers = 1;
    }
};

// Verifies that without explicitly enabling GL_EXT_draw_buffers extension
// in the shader, no broadcast emulation.
TEST_F(EmulateGLFragColorBroadcastTest, FragColorNoBroadcast)
{
    const std::string shaderString =
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(1, 0, 0, 0);\n"
        "}\n";
    compile(shaderString);
    EXPECT_TRUE(foundInCode("gl_FragColor"));
    EXPECT_FALSE(foundInCode("gl_FragData[0]"));
    EXPECT_FALSE(foundInCode("gl_FragData[1]"));
}

// Verifies that with explicitly enabling GL_EXT_draw_buffers extension
// in the shader, broadcast is emualted by replacing gl_FragColor with gl_FragData.
TEST_F(EmulateGLFragColorBroadcastTest, FragColorBroadcast)
{
    const std::string shaderString =
        "#extension GL_EXT_draw_buffers : require\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(1, 0, 0, 0);\n"
        "}\n";
    compile(shaderString);
    EXPECT_FALSE(foundInCode("gl_FragColor"));
    EXPECT_TRUE(foundInCode("gl_FragData[0]"));
    EXPECT_TRUE(foundInCode("gl_FragData[1]"));
}

// Verifies that with explicitly enabling GL_EXT_draw_buffers extension
// in the shader with an empty main(), anothing happens.
TEST_F(EmulateGLFragColorBroadcastTest, EmptyMain)
{
    const std::string shaderString =
        "#extension GL_EXT_draw_buffers : require\n"
        "void main()\n"
        "{\n"
        "}\n";
    compile(shaderString);
    EXPECT_FALSE(foundInCode("gl_FragColor"));
    EXPECT_FALSE(foundInCode("gl_FragData[0]"));
    EXPECT_FALSE(foundInCode("gl_FragData[1]"));
}

}  // namespace
