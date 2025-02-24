//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WGSLOutput_test.cpp:
//   Tests for corect WGSL translations.
//

#include <regex>

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class WGSLVertexOutputTest : public MatchOutputCodeTest
{
  public:
    WGSLVertexOutputTest() : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_WGSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions = {};
        defaultCompileOptions.validateAST      = true;
        setDefaultCompileOptions(defaultCompileOptions);
    }
};

class WGSLOutputTest : public MatchOutputCodeTest
{
  public:
    WGSLOutputTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_WGSL_OUTPUT)
    {
        ShCompileOptions defaultCompileOptions = {};
        defaultCompileOptions.validateAST      = true;
        setDefaultCompileOptions(defaultCompileOptions);
    }
};

TEST_F(WGSLOutputTest, BasicTranslation)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision highp float;

        out vec4 outColor;

        struct Foo {
            float x;
            float y;
            vec3 multiArray[2][3];
            mat3 aMatrix;
        };

        vec4 doFoo(Foo foo, float zw);

        vec4 doFoo(Foo foo, float zw)
        {
            // foo.x = foo.y;
            return vec4(foo.x, foo.y, zw, zw);
        }

        Foo returnFoo(Foo foo) {
          return foo;
        }

        float returnFloat(float x) {
          return x;
        }

        float takeArgs(vec2 x, float y) {
          return y;
        }

        void main()
        {
            Foo foo;
            // Struct field accesses.
            foo.x = 2.0;
            foo.y = 2.0;
            // Complicated constUnion should be emitted correctly.
            foo.multiArray = vec3[][](
              vec3[](
                vec3(1.0, 2.0, 3.0),
                vec3(1.0, 2.0, 3.0),
                vec3(1.0, 2.0, 3.0)),
              vec3[](
                vec3(4.0, 5.0, 6.0),
                vec3(4.0, 5.0, 6.0),
                vec3(4.0, 5.0, 6.0)
              )
            );
            int arrIndex = 1;
            // Access an array index with a constant index.
            float f = foo.multiArray[0][1].x;
            // Access an array index with a non-const index, should clamp by default.
            float f2 = foo.multiArray[0][arrIndex].x;
            gl_FragDepth = f + f2;
            doFoo(returnFoo(foo), returnFloat(3.0));
            takeArgs(vec2(1.0, 2.0), foo.x);
            returnFloat(doFoo(foo, 7.0 + 9.0).x);
            outColor = vec4(0.0, 0.0, 0.0, 0.0);
        })";
    const std::string &outputString =
        R"(struct ANGLE_Output_Global {
  outColor : vec4<f32>,
  gl_FragDepth_ : f32,
};

var<private> ANGLE_output_global : ANGLE_Output_Global;

struct ANGLE_Output_Annotated {
  @location(@@@@@@) outColor : vec4<f32>,
  @builtin(frag_depth) gl_FragDepth_ : f32,
};


;

struct _uFoo
{
  _ux : f32,
  _uy : f32,
  _umultiArray : array<array<vec3<f32>, 3>, 2>,
  _uaMatrix : mat3x3<f32>,
};

fn _udoFoo(_ufoo : _uFoo, _uzw : f32) -> vec4<f32>;

fn _udoFoo(_ufoo : _uFoo, _uzw : f32) -> vec4<f32>
{
  return vec4<f32>((_ufoo)._ux, (_ufoo)._uy, _uzw, _uzw);
}

fn _ureturnFoo(_ufoo : _uFoo) -> _uFoo
{
  return _ufoo;
}

fn _ureturnFloat(_ux : f32) -> f32
{
  return _ux;
}

fn _utakeArgs(_ux : vec2<f32>, _uy : f32) -> f32
{
  return _uy;
}

fn _umain()
{
  var _ufoo : _uFoo;
  ((_ufoo)._ux) = (2.0f);
  ((_ufoo)._uy) = (2.0f);
  ((_ufoo)._umultiArray) = (array<array<vec3<f32>, 3>, 2>(array<vec3<f32>, 3>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f)), array<vec3<f32>, 3>(vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(4.0f, 5.0f, 6.0f), vec3<f32>(4.0f, 5.0f, 6.0f))));
  var _uarrIndex : i32 = (1i);
  var _uf : f32 = (((((_ufoo)._umultiArray)[0i])[1i]).x);
  var _uf2 : f32 = (((((_ufoo)._umultiArray)[0i])[clamp((_uarrIndex), 0, 2)]).x);
  (ANGLE_output_global.gl_FragDepth_) = ((_uf) + (_uf2));
  _udoFoo(_ureturnFoo(_ufoo), _ureturnFloat(3.0f));
  _utakeArgs(vec2<f32>(1.0f, 2.0f), (_ufoo)._ux);
  _ureturnFloat((_udoFoo(_ufoo, 16.0f)).x);
  (ANGLE_output_global.outColor) = (vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f));
}
@fragment
fn wgslMain() -> ANGLE_Output_Annotated
{
  _umain();
  var ANGLE_output_annotated : ANGLE_Output_Annotated;
  ANGLE_output_annotated.outColor = ANGLE_output_global.outColor;
  ANGLE_output_annotated.gl_FragDepth_ = ANGLE_output_global.gl_FragDepth_;
  return ANGLE_output_annotated;
}
)";
    compile(shaderString);
    EXPECT_TRUE(foundInCode(outputString.c_str()));
}

TEST_F(WGSLOutputTest, ControlFlow)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        int ifElseDemo() {
          int x = 5;
          if (x == 5) {
            return 6;
          } else if (x == 6) {
            return 7;
          } else {
            return 8;
          }
        }

        void switchDemo() {
          int x = 5;
          switch (x) {
          case 5:
          case 6:
            discard;
          case 7: {
            return;
          }
          case 8:
          case 9:
            {
              x = 7;
            }
            return;
          default:
            return;
          }
        }

        void forLoopDemo() {
          for (int i = 0; i < 5; i++) {
            if (i == 4) {
              break;
            } else if (i == 5) {
              continue;
            }
          }
        }

        void whileLoopDemo() {
          int i = 0;
          while (i < 5) {
            i++;
          }

          do {
            i++;
          } while (i < 5);
        }

        void main()
        {
          ifElseDemo();
          switchDemo();
          forLoopDemo();
          whileLoopDemo();
        })";
    const std::string &outputString =
        R"(fn _uifElseDemo() -> i32
{
  var _ux : i32 = (5i);
  if ((_ux) == (5i))
  {
    return 6i;
  }
  else
  {
    if ((_ux) == (6i))
    {
      return 7i;
    }
    else
    {
      return 8i;
    }
  }
}

fn _uswitchDemo()
{
  var _ux : i32 = (5i);
  switch _ux
  {
    case 5i, 6i:
    {
      discard;
    }
    case 7i:
    {
      {
        return;
      }
    }
    case 8i, 9i:
    {
      {
        (_ux) = (7i);
      }
      return;
    }
    case default:
    {
      return;
    }
  }
}

fn _uforLoopDemo()
{
  for (var _ui : i32 = (0i); (_ui) < (5i); (_ui)++)
  {
    if ((_ui) == (4i))
    {
      break;
    }
    else
    {
      if ((_ui) == (5i))
      {
        continue;
      }
    }
  }
}

fn _uwhileLoopDemo()
{
  var _ui : i32 = (0i);
  while ((_ui) < (5i))
  {
    (_ui)++;
  }
  loop {
    {
      (_ui)++;
    }
    if (!((_ui) < (5i)) { break; }
  }
}

fn _umain()
{
  _uifElseDemo();
  _uswitchDemo();
  _uforLoopDemo();
  _uwhileLoopDemo();
}
@fragment
fn wgslMain()
{
  _umain();
}
)";
    compile(shaderString);
    EXPECT_TRUE(foundInCode(outputString.c_str()));
}

TEST_F(WGSLOutputTest, GLFragColorWithUniform)
{
    const std::string &shaderString =
        R"(
uniform mediump vec4 u_color;
void main(void)
{
    gl_FragColor = u_color;
})";
    const std::string &outputString =
        R"(struct ANGLE_Output_Global {
  gl_FragColor_ : vec4<f32>,
};

var<private> ANGLE_output_global : ANGLE_Output_Global;

struct ANGLE_Output_Annotated {
  @location(0) gl_FragColor_ : vec4<f32>,
};

struct ANGLE_DefaultUniformBlock {
  u_color : vec4<f32>,
};

@group(0) @binding(1) var<uniform> ANGLE_defaultUniformBlock : ANGLE_DefaultUniformBlock;

;

fn _umain()
{
  (ANGLE_output_global.gl_FragColor_) = (ANGLE_defaultUniformBlock.u_color);
}
@fragment
fn wgslMain() -> ANGLE_Output_Annotated
{
  _umain();
  var ANGLE_output_annotated : ANGLE_Output_Annotated;
  ANGLE_output_annotated.gl_FragColor_ = ANGLE_output_global.gl_FragColor_;
  return ANGLE_output_annotated;
}
)";
    compile(shaderString);
    EXPECT_TRUE(foundInCode(outputString.c_str()));
}

TEST_F(WGSLOutputTest, UniformsWithNestedStructs)
{
    const std::string &shaderString =
        R"(#version 300 es
precision mediump float;
struct NestedUniforms {
    float x;
};
struct Uniforms {
    NestedUniforms a;
    float b;
    float c;
    float[5] d;
    float e;
    vec3 f[7];
    float[5] g;
};
uniform Uniforms unis;
out vec4 fragColor;
void main() {
    float[5] dCopy = unis.d;
    fragColor = vec4(unis.a.x, unis.b, unis.c, dCopy[1]);
    fragColor += vec4(unis.d[2], unis.e, unis.f[0][2], (unis.e > 0.5 ? unis.d : unis.g)[1]);
})";
    const std::string &outputString =
        R"(struct ANGLE_Output_Global {
  fragColor : vec4<f32>,
};

var<private> ANGLE_output_global : ANGLE_Output_Global;

struct ANGLE_Output_Annotated {
  @location(@@@@@@) fragColor : vec4<f32>,
};

struct ANGLE_DefaultUniformBlock {
  unis : _uUniforms,
};

@group(0) @binding(1) var<uniform> ANGLE_defaultUniformBlock : ANGLE_DefaultUniformBlock;

struct ANGLE_wrapped_float
{
  @align(16) elem : f32
};
fn ANGLE_Convert_Array5_ANGLE_wrapped_float_ElementsTo_float_Elements(wrappedArr : array<ANGLE_wrapped_float, 5>) -> array<f32, 5>
{
  var retVal : array<f32, 5>;
  for (var i : u32 = 0; i < 5; i++) {;
    retVal[i] = wrappedArr[i].elem;
  }
  return retVal;
}
struct _uNestedUniforms
{
  @align(16) _ux : f32,
};

struct _uUniforms
{
  @align(16) _ua : _uNestedUniforms,
  @align(16) _ub : f32,
  _uc : f32,
  @align(16) _ud : array<ANGLE_wrapped_float, 5>,
  _ue : f32,
  @align(16) _uf : array<vec3<f32>, 7>,
  @align(16) _ug : array<ANGLE_wrapped_float, 5>,
};

;
;

fn _umain()
{
  var _udCopy : array<f32, 5> = (ANGLE_Convert_Array5_ANGLE_wrapped_float_ElementsTo_float_Elements((ANGLE_defaultUniformBlock.unis)._ud));
  (ANGLE_output_global.fragColor) = (vec4<f32>(((ANGLE_defaultUniformBlock.unis)._ua)._ux, (ANGLE_defaultUniformBlock.unis)._ub, (ANGLE_defaultUniformBlock.unis)._uc, (_udCopy)[1i]));
  (ANGLE_output_global.fragColor) += (vec4<f32>((ANGLE_defaultUniformBlock.unis)._ud[2i].elem, (ANGLE_defaultUniformBlock.unis)._ue, (((ANGLE_defaultUniformBlock.unis)._uf)[0i])[2i], (select((ANGLE_Convert_Array5_ANGLE_wrapped_float_ElementsTo_float_Elements((ANGLE_defaultUniformBlock.unis)._ug)), (ANGLE_Convert_Array5_ANGLE_wrapped_float_ElementsTo_float_Elements((ANGLE_defaultUniformBlock.unis)._ud)), (((ANGLE_defaultUniformBlock.unis)._ue) > (0.5f))))[1i]));
}
@fragment
fn wgslMain() -> ANGLE_Output_Annotated
{
  _umain();
  var ANGLE_output_annotated : ANGLE_Output_Annotated;
  ANGLE_output_annotated.fragColor = ANGLE_output_global.fragColor;
  return ANGLE_output_annotated;
}
)";
    compile(shaderString);
    EXPECT_TRUE(foundInCode(outputString.c_str()));
}

TEST_F(WGSLOutputTest, UniformsWithMatCx2)
{
    const std::string &shaderString =
        R"(#version 300 es
precision mediump float;
struct Uniforms {
    mat2 a;
    mat3x2 b;
    mat4x2 c;

    mat2[2] aArr;
    mat3x2[2] bArr;
    mat4x2[2] cArr;
};
uniform Uniforms unis;
out vec4 fragColor;
void main() {
  mat2 a = unis.a;
  mat3x2 b = unis.b;
  mat4x2 c = unis.c;

  mat2[2] aArr = unis.aArr;
  mat3x2[2] bArr = unis.bArr;
  mat4x2[2] cArr = unis.cArr;

  mat2 aIndexed = unis.aArr[1];
  mat3x2 bIndexed = unis.bArr[1];
  mat4x2 cIndexed = unis.cArr[1];

  fragColor = vec4(a[0][0], b[0][0], c[0][0], 1.0);
  fragColor += vec4(aArr[0][0][0], bArr[0][0][0], cArr[0][0][0], 1.0);
  fragColor += vec4(aIndexed[0][0], bIndexed[0][0], cIndexed[0][0], 1.0);
})";
    const std::string &outputString =
        R"(struct ANGLE_Output_Global {
  fragColor : vec4<f32>,
};

var<private> ANGLE_output_global : ANGLE_Output_Global;

struct ANGLE_Output_Annotated {
  @location(@@@@@@) fragColor : vec4<f32>,
};

struct ANGLE_DefaultUniformBlock {
  unis : _uUniforms,
};

@group(0) @binding(1) var<uniform> ANGLE_defaultUniformBlock : ANGLE_DefaultUniformBlock;

struct ANGLE_wrapped_vec2
{
  @align(16) elem : vec2<f32>
};
fn ANGLE_Convert_Mat2x2(mangledMatrix : array<ANGLE_wrapped_vec2, 2>) -> mat2x2<f32>
{
  var retVal : mat2x2<f32>;
  retVal = mat2x2<f32>(mangledMatrix[0].elem, mangledMatrix[1].elem);
  return retVal;
}
fn ANGLE_Convert_Array2_Mat2x2(mangledMatrix : array<array<ANGLE_wrapped_vec2, 2>, 2>) -> array<mat2x2<f32>, 2>
{
  var retVal : array<mat2x2<f32>, 2>;
  for (var i : u32 = 0; i < 2; i++) {;
    retVal[i] = mat2x2<f32>(mangledMatrix[i][0].elem, mangledMatrix[i][1].elem);
  }
  return retVal;
}
fn ANGLE_Convert_Mat3x2(mangledMatrix : array<ANGLE_wrapped_vec2, 3>) -> mat3x2<f32>
{
  var retVal : mat3x2<f32>;
  retVal = mat3x2<f32>(mangledMatrix[0].elem, mangledMatrix[1].elem, mangledMatrix[2].elem);
  return retVal;
}
fn ANGLE_Convert_Array2_Mat3x2(mangledMatrix : array<array<ANGLE_wrapped_vec2, 3>, 2>) -> array<mat3x2<f32>, 2>
{
  var retVal : array<mat3x2<f32>, 2>;
  for (var i : u32 = 0; i < 2; i++) {;
    retVal[i] = mat3x2<f32>(mangledMatrix[i][0].elem, mangledMatrix[i][1].elem, mangledMatrix[i][2].elem);
  }
  return retVal;
}
fn ANGLE_Convert_Mat4x2(mangledMatrix : array<ANGLE_wrapped_vec2, 4>) -> mat4x2<f32>
{
  var retVal : mat4x2<f32>;
  retVal = mat4x2<f32>(mangledMatrix[0].elem, mangledMatrix[1].elem, mangledMatrix[2].elem, mangledMatrix[3].elem);
  return retVal;
}
fn ANGLE_Convert_Array2_Mat4x2(mangledMatrix : array<array<ANGLE_wrapped_vec2, 4>, 2>) -> array<mat4x2<f32>, 2>
{
  var retVal : array<mat4x2<f32>, 2>;
  for (var i : u32 = 0; i < 2; i++) {;
    retVal[i] = mat4x2<f32>(mangledMatrix[i][0].elem, mangledMatrix[i][1].elem, mangledMatrix[i][2].elem, mangledMatrix[i][3].elem);
  }
  return retVal;
}
struct _uUniforms
{
  @align(16) _ua : array<ANGLE_wrapped_vec2, 2>,
  @align(16) _ub : array<ANGLE_wrapped_vec2, 3>,
  @align(16) _uc : array<ANGLE_wrapped_vec2, 4>,
  @align(16) _uaArr : array<array<ANGLE_wrapped_vec2, 2>, 2>,
  @align(16) _ubArr : array<array<ANGLE_wrapped_vec2, 3>, 2>,
  @align(16) _ucArr : array<array<ANGLE_wrapped_vec2, 4>, 2>,
};

;
;

fn _umain()
{
  var _ua : mat2x2<f32> = (ANGLE_Convert_Mat2x2((ANGLE_defaultUniformBlock.unis)._ua));
  var _ub : mat3x2<f32> = (ANGLE_Convert_Mat3x2((ANGLE_defaultUniformBlock.unis)._ub));
  var _uc : mat4x2<f32> = (ANGLE_Convert_Mat4x2((ANGLE_defaultUniformBlock.unis)._uc));
  var _uaArr : array<mat2x2<f32>, 2> = (ANGLE_Convert_Array2_Mat2x2((ANGLE_defaultUniformBlock.unis)._uaArr));
  var _ubArr : array<mat3x2<f32>, 2> = (ANGLE_Convert_Array2_Mat3x2((ANGLE_defaultUniformBlock.unis)._ubArr));
  var _ucArr : array<mat4x2<f32>, 2> = (ANGLE_Convert_Array2_Mat4x2((ANGLE_defaultUniformBlock.unis)._ucArr));
  var _uaIndexed : mat2x2<f32> = (ANGLE_Convert_Mat2x2((ANGLE_defaultUniformBlock.unis)._uaArr[1i]));
  var _ubIndexed : mat3x2<f32> = (ANGLE_Convert_Mat3x2((ANGLE_defaultUniformBlock.unis)._ubArr[1i]));
  var _ucIndexed : mat4x2<f32> = (ANGLE_Convert_Mat4x2((ANGLE_defaultUniformBlock.unis)._ucArr[1i]));
  (ANGLE_output_global.fragColor) = (vec4<f32>(((_ua)[0i])[0i], ((_ub)[0i])[0i], ((_uc)[0i])[0i], 1.0f));
  (ANGLE_output_global.fragColor) += (vec4<f32>((((_uaArr)[0i])[0i])[0i], (((_ubArr)[0i])[0i])[0i], (((_ucArr)[0i])[0i])[0i], 1.0f));
  (ANGLE_output_global.fragColor) += (vec4<f32>(((_uaIndexed)[0i])[0i], ((_ubIndexed)[0i])[0i], ((_ucIndexed)[0i])[0i], 1.0f));
}
@fragment
fn wgslMain() -> ANGLE_Output_Annotated
{
  _umain();
  var ANGLE_output_annotated : ANGLE_Output_Annotated;
  ANGLE_output_annotated.fragColor = ANGLE_output_global.fragColor;
  return ANGLE_output_annotated;
}
)";
    compile(shaderString);
    EXPECT_TRUE(foundInCode(outputString.c_str()));
}
