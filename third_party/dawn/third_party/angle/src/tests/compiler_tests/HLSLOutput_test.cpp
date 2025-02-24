//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HLSLOutput_test.cpp:
//   Tests for HLSL output.
//

#include <regex>
#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class HLSLOutputTest : public MatchOutputCodeTest
{
  public:
    HLSLOutputTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, SH_HLSL_4_1_OUTPUT) {}
};

class HLSL30VertexOutputTest : public MatchOutputCodeTest
{
  public:
    HLSL30VertexOutputTest() : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_HLSL_3_0_OUTPUT) {}
};

class HLSL41VertexOutputTest : public MatchOutputCodeTest
{
  public:
    HLSL41VertexOutputTest() : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_HLSL_4_1_OUTPUT) {}
};

// Test that having dynamic indexing of a vector inside the right hand side of logical or doesn't
// trigger asserts in HLSL output.
TEST_F(HLSLOutputTest, DynamicIndexingOfVectorOnRightSideOfLogicalOr)
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

// Test that rewriting else blocks in a function that returns a struct doesn't use the struct name
// without a prefix.
TEST_F(HLSL30VertexOutputTest, RewriteElseBlockReturningStruct)
{
    const std::string &shaderString =
        "struct foo\n"
        "{\n"
        "    float member;\n"
        "};\n"
        "uniform bool b;\n"
        "foo getFoo()\n"
        "{\n"
        "    if (b)\n"
        "    {\n"
        "        return foo(0.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        return foo(1.0);\n"
        "    }\n"
        "}\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(getFoo().member);\n"
        "}\n";
    compile(shaderString);
    EXPECT_TRUE(foundInCode("_foo"));
    EXPECT_FALSE(foundInCode("(foo)"));
    EXPECT_FALSE(foundInCode(" foo"));
}

// Regression test for RemoveDynamicIndexing transformation producing invalid AST, based on fuzzer
// test.
TEST_F(HLSL41VertexOutputTest, RemoveDynamicingIndexIndexPrecisionBug)
{
    const char shaderString[] = R"(void main()
{
    mat3 tmp;
    vec3 res = vec3(0);
    for (int i = 0; res += 0., ivec3(0)[i], ivec3(tmp)[i], i < 0;);
})";
    compile(shaderString);
}

// Test that having an array constructor as a statement doesn't trigger an assert in HLSL output.
// This test has a constant array constructor statement.
TEST_F(HLSLOutputTest, ConstArrayConstructorStatement)
{
    const std::string &shaderString =
        R"(#version 300 es
        void main()
        {
            int[1](0);
        })";
    compile(shaderString);
}

// Test that having an array constructor as a statement doesn't trigger an assert in HLSL output.
TEST_F(HLSLOutputTest, ArrayConstructorStatement)
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
TEST_F(HLSLOutputTest, ArrayOfArraysStatement)
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
TEST_F(HLSLOutputTest, VectorDynamicIndexing)
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
TEST_F(HLSLOutputTest, ArrayReturnValue)
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
TEST_F(HLSLOutputTest, ParameterWithNoName)
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

// Test that array dimensions are written out correctly.
TEST_F(HLSLOutputTest, Array)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;

        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            float arr[2];
            for (int i = 0; i < 2; ++i) {
                arr[i] = uf * 2.0;
                my_FragColor.x += arr[i];
            }
        })";
    compile(shaderString);
    EXPECT_TRUE(foundInCodeRegex(std::regex("_arr(\\d)*\\[2\\]")));
}

// Test that initializing array with previously declared array will not be overwritten
TEST_F(HLSLOutputTest, SameNameArray)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        out vec4 my_FragColor;

        void main()
        {
          float arr[2] = float[2](1.0, 1.0);
          {
            float arr[2] = arr;
            my_FragColor = vec4(0.0, arr[0], 0.0, arr[1]);
          }
        })";
    compile(shaderString);
    // There should be two different arr defined, e.g. _arr1000 and _arr1001
    // Use Workaround for now.
    // Once the build team fixes libc++ we could use the following one line solution instead.
    // EXPECT_TRUE(foundInCodeRegex(std::regex("_arr(\\d*)\\[2\\](.|\\r|\\n)*_arr(?!\\1)\\d*\\[2\\]")));
    std::smatch m;
    EXPECT_TRUE(foundInCodeRegex(std::regex("_arr(\\d)*\\[2\\]"), &m));
    EXPECT_TRUE(m.size() == 2);
    EXPECT_TRUE(m[0].str() != m[1].str());
}

// Test that passing a non-struct member of a std140 structure to a function won't trigger the
// struct mapping.
TEST_F(HLSLOutputTest, NonStructMemberAsFunctionArgument)
{
    constexpr char shaderString[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;

struct InstancingData
{
    vec4 data;
};

layout(std140) uniform InstanceBlock
{
    InstancingData instances[8];
};

void main()
{
    int index = int(gl_FragCoord.x);
    float result = dot(instances[index].data, vec4(1.0, 1.0, 1.0, 1.0));
    my_FragColor = vec4(result, 0.0, 0.0, 1.0);
})";

    compile(shaderString);
    EXPECT_FALSE(foundInCode("map_instances"));
}
