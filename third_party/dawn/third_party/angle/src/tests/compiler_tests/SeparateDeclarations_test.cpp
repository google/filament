//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SeparateDeclarations.cpp:
//   Tests that compound declarations are rewritten to type declarations and variable declarations.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

namespace
{

class SeparateDeclarations : public MatchOutputCodeTest
{
  public:
    SeparateDeclarations() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions = {};
        defaultCompileOptions.validateAST      = true;
        setDefaultCompileOptions(defaultCompileOptions);
    }
};

class SeparateCompoundStructDeclarations : public MatchOutputCodeTest
{
  public:
    SeparateCompoundStructDeclarations() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions                   = {};
        defaultCompileOptions.validateAST                        = true;
        defaultCompileOptions.separateCompoundStructDeclarations = true;
        setDefaultCompileOptions(defaultCompileOptions);
    }
};

class SeparateStructFunctionDeclarations : public MatchOutputCodeTest
{
  public:
    SeparateStructFunctionDeclarations() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_ESSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions = {};
        defaultCompileOptions.validateAST      = true;
        setDefaultCompileOptions(defaultCompileOptions);
    }
};

TEST_F(SeparateDeclarations, Arrays)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
int a[1] = int[1](1), b[1] = int[1](2);
out vec4 o;
void main() {
    if (a[0] == b[0])
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
const mediump int sbbd = 1;
mediump int _ua[1] = int[1](sbbd);
const mediump int sbbe = 2;
mediump int _ub[1] = int[1](sbbe);
out highp vec4 _uo;
void main(){
  if ((_ua[0] == _ub[0]))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateDeclarations, StructNoChange)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
struct S { vec3 d; } a;
out vec4 o;
void main() {
    if (a.d == vec3(2))
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
struct _uS {
  highp vec3 _ud;
} _ua;
out highp vec4 _uo;
void main(){
  if ((_ua._ud == vec3(2.0, 2.0, 2.0)))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateDeclarations, Structs)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
struct S { vec3 d; } a, b;
out vec4 o;
void main() {
    if (a.d == b.d)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
struct _uS {
  highp vec3 _ud;
} _ua;
_uS _ub;
out highp vec4 _uo;
void main(){
  if ((_ua._ud == _ub._ud))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateDeclarations, AnonymousStructNoChange)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
struct { vec3 d; } a;
out vec4 o;
void main() {
    if (any(lessThan(a.d, vec3(2))))
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
struct {
  highp vec3 _ud;
} _ua;
out highp vec4 _uo;
void main(){
  if (any(lessThan(_ua._ud, vec3(2.0, 2.0, 2.0))))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateDeclarations, AnonymousStructs)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
struct { vec3 d; } a, b;
out vec4 o;
void main() {
    if (any(lessThan(a.d, b.d)))
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
struct sbbe {
  highp vec3 _ud;
} _ua;
sbbe _ub;
out highp vec4 _uo;
void main(){
  if (any(lessThan(_ua._ud, _ub._ud)))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundStructDeclarations, AnonymousStruct)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
struct { vec3 d; } a;
out vec4 o;
void main() {
    if (any(lessThan(a.d, vec3(2))))
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
struct sbbd {
  highp vec3 _ud;
};
sbbd _ua;
out highp vec4 _uo;
void main(){
  if (any(lessThan(_ua._ud, vec3(2.0, 2.0, 2.0))))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundStructDeclarations, AnonymousStructs)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
struct { vec3 d; } a, b;
out vec4 o;
void main() {
    if (any(lessThan(a.d, b.d)))
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
struct sbbe {
  highp vec3 _ud;
};
sbbe _ua;
sbbe _ub;
out highp vec4 _uo;
void main(){
  if (any(lessThan(_ua._ud, _ub._ud)))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundStructDeclarations, Struct)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
struct S { vec3 d; } a, b;
out vec4 o;
void main() {
    if (a.d == b.d)
        o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
struct _uS {
  highp vec3 _ud;
};
_uS _ua;
_uS _ub;
out highp vec4 _uo;
void main(){
  if ((_ua._ud == _ub._ud))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateCompoundStructDeclarations, ConstStruct)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
out vec4 o;
void main()
{
  const struct s2 {
    int i;
  } s22 = s2(8);
  const struct s1 {
    s2 ss;
    mat4 m;
  } s11 = s1(s22, mat4(5));
  if (s11.ss.i > int(o.x))
    o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
out highp vec4 _uo;
void main(){
  const mediump int sbc3 = 8;
  if ((sbc3 > int(_uo.x)))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

// Example of what the const struct does before evaluating constants.
TEST_F(SeparateCompoundStructDeclarations, ConstStructsCrossRef)
{
    const char kShader[]   = R"(#version 300 es
precision highp float;
out vec4 o;
void main()
{
  struct s2 {
    int i;
  } s22 = s2(8);
  struct s1 {
    s2 ss;
    mat4 m;
  } s11 = s1(s22, mat4(5));
  if (s11.ss.i > int(o.x))
    o = vec4(1);
})";
    const char kExpected[] = R"(#version 300 es
out highp vec4 _uo;
void main(){
  struct _us2 {
    mediump int _ui;
  };
  _us2 _us22 = _us2(8);
  struct _us1 {
    _us2 _uss;
    highp mat4 _um;
  };
  _us1 _us11 = _us1(_us22, mat4(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0));
  if ((_us11._uss._ui > int(_uo.x)))
  {
    (_uo = vec4(1.0, 1.0, 1.0, 1.0));
  }
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

// Test that struct name validation takes into account that intenal symbol namespace
// is different to user namespace. The test should be kept in sync so that struct sbbf is the same
// textual symbol as what the anonymous struct gets.
TEST_F(SeparateCompoundStructDeclarations, InternalSymbolNoCrash)
{
    const char kShader[] = R"(
precision highp float;
struct { vec4 e; } g;
struct sbbf { vec4 f; };
void main(){
  sbbf s;
  gl_FragColor = g.e + s.f;
})";
    compile(kShader);
    const char kExpected[] = R"(struct sbbf {
  highp vec4 _ue;
};
sbbf _ug;
struct _usbbf {
  highp vec4 _uf;
};
void main(){
  _usbbf _us;
  (gl_FragColor = (_ug._ue + _us._uf));
}
)";
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateStructFunctionDeclarations, StructInStruct)
{
    const char kShader[]   = R"(#version 300 es
struct S {
  int f;
};
struct S2 { S h; } o()
{
  return S2(S(1));
}
void main() {
  S2 s2 = o();
})";
    const char kExpected[] = R"(#version 300 es
struct _uS {
  mediump int _uf;
};
struct _uS2 {
  _uS _uh;
};
_uS2 _uo(){
  return _uS2(_uS(1));
}
void main(){
  _uS2 _us2 = _uo();
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

TEST_F(SeparateStructFunctionDeclarations, StructInAnonymousStruct)
{
    const char kShader[]   = R"(#version 300 es
struct S {
  int f;
};
struct { S h; } o();
void main() {
})";
    const char kExpected[] = R"(#version 300 es
void main(){
}
)";
    compile(kShader);
    EXPECT_EQ(kExpected, outputCode(SH_ESSL_OUTPUT));
}

}  // namespace
