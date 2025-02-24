//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PruneEmptyDeclarations_test.cpp:
//   Tests for pruning empty declarations.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class PruneEmptyDeclarationsTest : public MatchOutputCodeTest
{
  public:
    PruneEmptyDeclarationsTest()
        : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_GLSL_COMPATIBILITY_OUTPUT)
    {}
};

TEST_F(PruneEmptyDeclarationsTest, EmptyDeclarationStartsDeclaratorList)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main()\n"
        "{\n"
        "   float, f;\n"
        "   gl_Position = vec4(u * f);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("float _uf"));
    ASSERT_TRUE(notFoundInCode("float, _uf"));
    ASSERT_TRUE(notFoundInCode("float, f"));
    ASSERT_TRUE(notFoundInCode("float _u, _uf"));
}

TEST_F(PruneEmptyDeclarationsTest, EmptyStructDeclarationWithQualifiers)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "const struct S { float f; };\n"
        "uniform S s;"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(s.f);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("struct _uS"));
    ASSERT_TRUE(foundInCode("uniform _uS"));
    ASSERT_TRUE(notFoundInCode("const struct _uS"));
}

}  // namespace
