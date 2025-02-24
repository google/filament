//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// InitializeUninitializedVariables_test.cpp: Tests InitializeUninitializedVariables pass.
//

#include "common/angleutils.h"

#include "tests/test_utils/compiler_test.h"

namespace sh
{

namespace
{

class InitializeUninitializedVariables : public MatchOutputCodeTest
{
  public:
    InitializeUninitializedVariables() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT)
    {
        ShCompileOptions options{};
        options.intermediateTree              = true;
        options.initializeUninitializedLocals = true;
        options.validateAST                   = true;
        setDefaultCompileOptions(options);
    }
};

// Tests that when unnamed variables must be initialized, the variables get internal names.
TEST_F(InitializeUninitializedVariables, VariableNamesInPrototypesUnnamedOut)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
out vec4 o;
void f(out float, out float);
void main()
{
    o = vec4(0.5);
    f(o.r, o.g);
}
void f(out float r, out float)
{
    r = 1.0;
}
)";
    const char kExpected[] = R"(#version 300 es
out highp vec4 _uo;
void _uf(out highp float _ur, out highp float sbc2);
void main(){
  (_uo = vec4(0.5, 0.5, 0.5, 0.5));
  _uf(_uo.x, _uo.y);
}
void _uf(out highp float _ur, out highp float sbc2){
  (_ur = 0.0);
  (sbc2 = 0.0);
  (_ur = 1.0);
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

// Tests that when unnamed variables must be initialized, the variables get internal names.
TEST_F(InitializeUninitializedVariables, VariableNamesInPrototypesUnnamedOut2)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
out vec4 o;
void f(out float, out float);
void g(out float a, out float b)
{
    f(a, b);
}
void main()
{
    o = vec4(0.5);
    g(o.r, o.g);
}
void f(out float r, out float)
{
    r = 1.0;
}
)";
    const char kExpected[] = R"(#version 300 es
out highp vec4 _uo;
void _uf(out highp float _ur, out highp float sbc5);
void _ug(out highp float _ua, out highp float _ub){
  (_ua = 0.0);
  (_ub = 0.0);
  _uf(_ua, _ub);
}
void main(){
  (_uo = vec4(0.5, 0.5, 0.5, 0.5));
  _ug(_uo.x, _uo.y);
}
void _uf(out highp float _ur, out highp float sbc5){
  (_ur = 0.0);
  (sbc5 = 0.0);
  (_ur = 1.0);
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

// Tests that when unnamed variables must be initialized, the variables get internal names.
// Tests the case where local z is initialized with a function f, when f must be rewritten.
TEST_F(InitializeUninitializedVariables, VariableNamesInPrototypesUnnamedOut3)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
out vec4 o;
float f(out float r, out float)
{
    r = 1.0;
    return 3.0;
}
void main()
{
    o = vec4(0.5);
    float z = f(o.r, o.g);
}
)";
    const char kExpected[] = R"(#version 300 es
out highp vec4 _uo;
highp float _uf(out highp float _ur, out highp float sbc0){
  (_ur = 0.0);
  (sbc0 = 0.0);
  (_ur = 1.0);
  return 3.0;
}
void main(){
  (_uo = vec4(0.5, 0.5, 0.5, 0.5));
  highp float _uz = _uf(_uo.x, _uo.y);
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

}  // namespace

}  // namespace sh
