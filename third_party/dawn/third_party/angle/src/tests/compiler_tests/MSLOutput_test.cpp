//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MSLOutput_test.cpp:
//   Tests for MSL output.
//

#include <regex>
#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class MSLOutputTestBase : public MatchOutputCodeTest
{
  public:
    MSLOutputTestBase(GLenum shaderType) : MatchOutputCodeTest(shaderType, SH_MSL_METAL_OUTPUT)
    {
        setDefaultCompileOptions(defaultOptions());
    }
    static ShCompileOptions defaultOptions()
    {
        ShCompileOptions options = {};
        // Default options that are forced for MSL output.
        options.rescopeGlobalVariables             = true;
        options.simplifyLoopConditions             = true;
        options.initializeUninitializedLocals      = true;
        options.separateCompoundStructDeclarations = true;
        // The tests also test that validation succeeds. This should be also the
        // default forced option, but currently MSL backend does not generate
        // valid trees. Once validateAST is forced, move to above hunk.
        options.validateAST = true;
        return options;
    }
};

class MSLOutputTest : public MSLOutputTestBase
{
  public:
    MSLOutputTest() : MSLOutputTestBase(GL_FRAGMENT_SHADER) {}
};

class MSLVertexOutputTest : public MSLOutputTestBase
{
  public:
    MSLVertexOutputTest() : MSLOutputTestBase(GL_VERTEX_SHADER) {}
};

// Test that having dynamic indexing of a vector inside the right hand side of logical or doesn't
// trigger asserts in MSL output.
TEST_F(MSLOutputTest, DynamicIndexingOfVectorOnRightSideOfLogicalOr)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "uniform int u1;\n"
        "void main() {\n"
        "   bvec4 v = bvec4(true, true, true, false);\n"
        "   my_FragColor = vec4(v[u1 + 1] || v[u1]);\n"
        "}\n";
    compile(shaderString);
}

// Test that having an array constructor as a statement doesn't trigger an assert in MSL output.
TEST_F(MSLOutputTest, ArrayConstructorStatement)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        out vec4 outColor;
        void main()
        {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            float[1](outColor[1]++);
        })";
    compile(shaderString);
}

// Test an array of arrays constructor as a statement.
TEST_F(MSLOutputTest, ArrayOfArraysStatement)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        out vec4 outColor;
        void main()
        {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            float[2][2](float[2](outColor[1]++, 0.0), float[2](1.0, 2.0));
        })";
    compile(shaderString);
}

// Test dynamic indexing of a vector. This makes sure that helper functions added for dynamic
// indexing have correct data that subsequent traversal steps rely on.
TEST_F(MSLOutputTest, VectorDynamicIndexing)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        out vec4 outColor;
        uniform int i;
        void main()
        {
            vec4 foo = vec4(0.0, 0.0, 0.0, 1.0);
            foo[i] = foo[i + 1];
            outColor = foo;
        })";
    compile(shaderString);
}

// Test returning an array from a user-defined function. This makes sure that function symbols are
// changed consistently when the user-defined function is changed to have an array out parameter.
TEST_F(MSLOutputTest, ArrayReturnValue)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform float u;
        out vec4 outColor;

        float[2] getArray(float f)
        {
            return float[2](f, f + 1.0);
        }

        void main()
        {
            float[2] arr = getArray(u);
            outColor = vec4(arr[0], arr[1], 0.0, 1.0);
        })";
    compile(shaderString);
}

// Test that writing parameters without a name doesn't assert.
TEST_F(MSLOutputTest, ParameterWithNoName)
{
    const std::string &shaderString =
        R"(precision mediump float;

        uniform vec4 v;

        vec4 s(vec4)
        {
            return v;
        }
        void main()
        {
            gl_FragColor = s(v);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, Macro)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        #define FOO vec4

        out vec4 outColor;

        void main()
        {
            outColor = FOO(1.0, 2.0, 3.0, 4.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, UniformSimple)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;
        uniform float x;

        void main()
        {
            outColor = vec4(x, x, x, x);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, FragmentOutSimple)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        void main()
        {
            outColor = vec4(1.0, 2.0, 3.0, 4.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, FragmentOutIndirect1)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        void foo()
        {
            outColor = vec4(1.0, 2.0, 3.0, 4.0);
        }

        void bar()
        {
            foo();
        }

        void main()
        {
            bar();
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, FragmentOutIndirect2)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        void foo();

        void bar()
        {
            foo();
        }

        void foo()
        {
            outColor = vec4(1.0, 2.0, 3.0, 4.0);
        }

        void main()
        {
            bar();
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, FragmentOutIndirect3)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        float foo(float x, float y)
        {
            outColor = vec4(x, y, 3.0, 4.0);
            return 7.0;
        }

        float bar(float x)
        {
            return foo(x, 2.0);
        }

        float baz()
        {
            return 13.0;
        }

        float identity(float x)
        {
            return x;
        }

        void main()
        {
            identity(bar(baz()));
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, VertexInOut)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        in float in0;
        out float out0;
        void main()
        {
            out0 = in0;
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, SymbolSharing)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        struct Foo {
            float x;
            float y;
        };

        void doFoo(Foo foo, float zw);

        void doFoo(Foo foo, float zw)
        {
            foo.x = foo.y;
            outColor = vec4(foo.x, foo.y, zw, zw);
        }

        void main()
        {
            Foo foo;
            foo.x = 2.0;
            foo.y = 2.0;
            doFoo(foo, 3.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, StructDecl)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out float out0;

        struct Foo {
            float value;
        };

        void main()
        {
            Foo foo;
            out0 = foo.value;
        }
        )";
    compile(shaderString);
}

TEST_F(MSLOutputTest, Structs)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        struct Foo {
            float value;
        };

        out vec4 out0;

        struct Bar {
            Foo foo;
        };

        void go();

        uniform UniInstance {
            Bar bar;
            float instance;
        } uniInstance;

        uniform UniGlobal {
            Foo foo;
            float global;
        };

        void main()
        {
            go();
        }

        struct Baz {
            Bar bar;
        } baz;

        void go()
        {
            out0.x = baz.bar.foo.value;
            out0.y = global;
            out0.z = uniInstance.instance;
            out0.w = 0.0;
        }

        )";
    compile(shaderString);
}

TEST_F(MSLOutputTest, KeywordConflict)
{
    const std::string &shaderString =
        R"(#version 300 es
            precision highp float;

        struct fragment {
            float kernel;
        } device;

        struct Foo {
            fragment frag;
        } foo;

        out float vertex;
        float kernel;

        float stage_in(float x)
        {
            return x;
        }

        void metal(float metal, float fragment);
        void metal(float metal, float fragment)
        {
            vertex = metal * fragment * foo.frag.kernel;
        }

        void main()
        {
            metal(stage_in(stage_in(kernel * device.kernel)), foo.frag.kernel);
        })";
    compile(shaderString);
}

TEST_F(MSLVertexOutputTest, Vertex)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        void main()
        {
            gl_Position = vec4(1.0,1.0,1.0,1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLVertexOutputTest, LastReturn)
{
    const std::string &shaderString =
        R"(#version 300 es
        in highp vec4 a_position;
        in highp vec4 a_coords;
        out highp vec4 v_color;

        void main (void)
        {
            gl_Position = a_position;
            v_color = vec4(a_coords.xyz, 1.0);
            return;
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, LastReturn)
{
    const std::string &shaderString =
        R"(#version 300 es
        in mediump vec4 v_coords;
        layout(location = 0) out mediump vec4 o_color;

        void main (void)
        {
            o_color = vec4(v_coords.xyz, 1.0);
            return;
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, FragColor)
{
    const std::string &shaderString = R"(
        void main ()
        {
            gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, MatrixIn)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        in mat4 mat;
        out float out0;

        void main()
        {
            out0 = mat[0][0];
        }
        )";
    compile(shaderString);
}

TEST_F(MSLOutputTest, WhileTrue)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            while (true)
            {
                break;
            }
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, ForTrue)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            for (;true;)
            {
                break;
            }
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, ForEmpty)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            for (;;)
            {
                break;
            }
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, ForComplex)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            for (int i = 0, j = 2; i < j; ++i) {
                if (i == 0) continue;
                if (i == 42) break;
                my_FragColor.x += float(i);
            }
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, ForSymbol)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            bool cond = true;
            for (;cond;)
            {
                my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                cond = false;
            }
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, DoWhileSymbol)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            bool cond = false;
            do
            {
                my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            } while (cond);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, AnonymousStruct)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        struct { vec4 v; } anonStruct;
        void main() {
            anonStruct.v = vec4(0.0,1.0,0.0,1.0);
            gl_FragColor = anonStruct.v;
        })";
    compile(shaderString);
    // TODO(anglebug.com/42264909): This success condition is expected to fail now.
    // When WebKit build is able to run the tests, this should be changed to something else.
    //    ASSERT_TRUE(foundInCode(SH_MSL_METAL_OUTPUT, "__unnamed"));
}

TEST_F(MSLOutputTest, GlobalRescopingSimple)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should rescope uf into main

        float uf;
        out vec4 my_FragColor;

        void main()
        {
            uf += 1.0f;
            my_FragColor = vec4(uf, 0.0, 0.0, 1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, GlobalRescopingNoRescope)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should not rescope any variable

        float uf;
        out vec4 my_FragColor;
        void modifyGlobal()
        {
            uf = 1.0f;
        }
        void main()
        {
            modifyGlobal();
            my_FragColor = vec4(uf, 0.0, 0.0, 1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, GlobalRescopingInitializer)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should rescope uf into main

        float uf = 1.0f;
        out vec4 my_FragColor;

        void main()
        {
            uf += 1.0;
            my_FragColor = vec4(uf, 0.0, 0.0, 1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, GlobalRescopingInitializerNoRescope)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should not rescope any variable

        float uf = 1.0f;
        out vec4 my_FragColor;

        void modifyGlobal()
        {
            uf =+ 1.0f;
        }
        void main()
        {
            modifyGlobal();
            my_FragColor = vec4(uf, 0.0, 0.0, 1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, GlobalRescopingNestedFunction)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should rescope a info modifyGlobal

        float a = 1.0f;
        float uf = 1.0f;
        out vec4 my_FragColor;

        void modifyGlobal()
        {
            uf =+ a;
        }
        void main()
        {
            modifyGlobal();
            my_FragColor = vec4(uf, 0.0, 0.0, 1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, GlobalRescopingMultipleUses)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should rescope uf into main

        float uf = 1.0f;
        out vec4 my_FragColor;

        void main()
        {
            uf =+ 1.0;
            if (uf > 0.0)
            {
                uf =- 0.5;
            }
            my_FragColor = vec4(uf, 0.0, 0.0, 1.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, GlobalRescopingGloballyReferencedVar)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should rescope uf into main

        const float a = 1.0f;
        float uf = a;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(uf, 0.0, a, 0.0);
        })";
    compile(shaderString);
}

TEST_F(MSLOutputTest, GlobalRescopingDeclarationAfterFunction)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        // Should rescope c and b into main

        float a = 1.0f;
        float c = 1.0f;
        out vec4 my_FragColor;

        void modifyGlobal()
        {
            a =+ 1.0f;
        }

        float b = 1.0f;

        void main()
        {
            modifyGlobal();
            my_FragColor = vec4(a, b, c, 0.0);
        }

        )";
    compile(shaderString);
}

TEST_F(MSLOutputTest, ReusedOutVarName)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        out vec4 my_FragColor;

        void funcWith1Out(
        out float outC) {
            outC = 1.0;
        }

        void funcWith4Outs(
        out float outA,
        out float outB,
        out float outC,
        out float outD) {
            outA = 1.0;
            outB = 1.0;
            outD = 1.0;
        }


        void main()
        {
            funcWith1Out(my_FragColor.g);
            funcWith4Outs(my_FragColor.r, my_FragColor.g, my_FragColor.b, my_FragColor.a);
        }

        )";
    compile(shaderString);
}

// Test that for loops without body do not crash. At the time of writing, constant hoisting would
// traverse such ASTs and crash when loop bodies were not present.
TEST_F(MSLOutputTest, RemovedForBodyNoCrash)
{
    const char kShader[] = R"(#version 310 es
void main() {
    for(;;)if(2==0);
})";
    compile(kShader);
}

// Test that accessing array element of array of anonymous struct instances does not fail
// validation.
TEST_F(MSLOutputTest, AnonymousStructArrayValidationNoCrash)
{
    const char kShader[] = R"(
precision mediump float;
void main() {
    struct { vec4 field; } s1[1];
    gl_FragColor = s1[0].field;
})";
    compile(kShader);
}

// Tests that rewriting varyings for per-element element access does not cause crash.
// At the time of writing a_ would be confused with a due to matrixes being flattened
// for fragment inputs, and the new variables would be given semantic names separated
// with _. This would cause confusion because semantic naming would filter underscores.
TEST_F(MSLOutputTest, VaryingRewriteUnderscoreNoCrash)
{
    const char kShader[] = R"(precision mediump float;
varying mat2 a_;
varying mat3 a;
void main(){
    gl_FragColor = vec4(a_) + vec4(a);
})";
    compile(kShader);
}

// Tests that rewriting varyings for per-element element access does not cause crash.
// Test for a clash between a[0] and a_0. Both could be clashing at a_0.
TEST_F(MSLVertexOutputTest, VaryingRewriteUnderscoreNoCrash)
{
    const char kShader[] = R"(precision mediump float;
varying mat2 a_0;
varying mat3 a[1];
void main(){
    a_0 = mat2(0,1,2,3);
    a[0] = mat3(0,1,2,3,4,5,6,7,8);
    gl_Position = vec4(1);
})";
    compile(kShader);
}

// Tests that rewriting varyings for per-element element access does not cause crash.
// ES3 variant.
// Test for a clash between a[0] and a_0. Both could be clashing at a_0.
TEST_F(MSLVertexOutputTest, VaryingRewriteUnderscoreNoCrash2)
{
    const char kShader[] = R"(#version 300 es
precision mediump float;
out mat2 a_0;
out mat3 a[1];
void main(){
    a_0 = mat2(0,1,2,3);
    a[0] = mat3(0,1,2,3,4,5,6,7,8);
})";
    compile(kShader);
}

// Tests that rewriting varyings for per-element element access does not cause crash.
// Test for a clash between a_[0] and a._0. Both could be clashing at a__0.
TEST_F(MSLVertexOutputTest, VaryingRewriteUnderscoreNoCrash3)
{
    const char kShader[] = R"(#version 300 es
precision mediump float;
out mat3 a_[1];
struct s {
    mat2 _0;
};
out s a;
void main(){
    a._0 = mat2(0,1,2,3);
    a_[0] = mat3(0,1,2,3,4,5,6,7,8);
})";
    compile(kShader);
}

// Tests that rewriting attributes for per-element element access does not cause crash.
// At the time of writing a_ would be confused with a due to matrixes being flattened
// for fragment inputs, and the new variables would be given semantic names separated
// with _. This would cause confusion because semantic naming would filter underscores.
TEST_F(MSLVertexOutputTest, AttributeRewriteUnderscoreNoCrash)
{
    const char kShader[] = R"(precision mediump float;
attribute mat2 a_;
attribute mat3 a;
void main(){
    gl_Position = vec4(a_) + vec4(a);
})";
    compile(kShader);
}

// Test that emulated clip distance varying passes AST validation
TEST_F(MSLVertexOutputTest, ClipDistanceVarying)
{
    getResources()->ANGLE_clip_cull_distance = 1;
    const char kShader[]                     = R"(#version 300 es
#extension GL_ANGLE_clip_cull_distance:require
void main(){gl_ClipDistance[0];})";
    compile(kShader);
}

TEST_F(MSLVertexOutputTest, VertexIDIvecNoCrash)
{
    const char kShader[] = R"(#version 300 es
void main(){ivec2 xy=ivec2((+gl_VertexID));gl_Position=vec4((xy), 0,1);})";
    compile(kShader);
}

TEST_F(MSLVertexOutputTest, StructEqualityNoCrash)
{
    const char kShader[] = R"(#version 300 es
struct S{mediump vec2 i;};S a,b;void main(){if (a==b){}})";
    compile(kShader);
}

TEST_F(MSLOutputTest, StructAndVarDeclarationNoCrash)
{
    const char kShader[] = R"(#version 300 es
void main(){struct S{mediump vec4 v;};S a;a=a,1;})";
    compile(kShader);
}

TEST_F(MSLOutputTest, StructAndVarDeclarationSeparationNoCrash)
{
    const char kShader[] = R"(#version 300 es
void main(){struct S{mediump vec4 v;}a;a=a,1;})";
    compile(kShader);
}

TEST_F(MSLOutputTest, StructAndVarDeclarationSeparationNoCrash2)
{
    const char kShader[] = R"(#version 300 es
void main(){struct S{mediump vec4 v;}a,b;a=b,1;})";
    compile(kShader);
}

TEST_F(MSLOutputTest, StructAndVarDeclarationSeparationNoCrash3)
{
    const char kShader[] = R"(#version 300 es
 void main(){struct S1{mediump vec4 v;}l;struct S2{S1 s1;}s2;s2=s2,l=l,1;})";
    compile(kShader);
}

TEST_F(MSLOutputTest, MultisampleInterpolationNoCrash)
{
    getResources()->OES_shader_multisample_interpolation = 1;
    const char kShader[]                                 = R"(#version 300 es
#extension GL_OES_shader_multisample_interpolation : require
precision highp float;
in float i; out vec4 c; void main() { c = vec4(interpolateAtOffset(i, vec2(i))); })";
    compile(kShader);
}

TEST_F(MSLVertexOutputTest, ClipCullDistanceNoCrash)
{
    getResources()->ANGLE_clip_cull_distance = 1;
    const char kShader[]                     = R"(#version 300 es
#extension GL_ANGLE_clip_cull_distance : require
void main() { gl_Position = vec4(0.0, 0.0, 0.0, 1.0); gl_ClipDistance[1] = 1.0;})";
    compile(kShader);
}

TEST_F(MSLOutputTest, UnnamedOutParameterNoCrash)
{
    const char kShader[] = R"(void f(out int){}void main(){int a;f(a);})";
    compile(kShader);
}

TEST_F(MSLOutputTest, ExplicitBoolCastsNoCrash)
{
    ShCompileOptions options     = defaultOptions();
    options.addExplicitBoolCasts = 1;
    const char kShader[]         = R"(
precision mediump float;
void main(){vec2 c;bvec2 U=bvec2(c.xx);if (U.x) gl_FragColor = vec4(1);})";
    compile(kShader, options);
}
