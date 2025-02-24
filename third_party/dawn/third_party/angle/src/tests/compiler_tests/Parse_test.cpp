//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Parse_test.cpp:
//   Test for parsing erroneous and correct GLSL input.
//

#include <memory>
#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/glsl/TranslatorESSL.h"
#include "gtest/gtest.h"

using namespace sh;

class ParseTest : public testing::Test
{
  public:
    ParseTest()
    {
        InitBuiltInResources(&mResources);
        mResources.FragmentPrecisionHigh = 1;
        mCompileOptions.intermediateTree = true;
    }

  protected:
    void TearDown() override { mTranslator.reset(); }

    testing::AssertionResult compile(const std::string &shaderString)
    {
        if (mTranslator == nullptr)
        {
            std::unique_ptr<TranslatorESSL> translator =
                std::make_unique<TranslatorESSL>(GL_FRAGMENT_SHADER, mShaderSpec);
            if (!translator->Init(mResources))
            {
                return testing::AssertionFailure() << "Failed to initialize translator";
            }
            mTranslator = std::move(translator);
        }

        const char *shaderStrings[] = {shaderString.c_str()};
        bool compilationSuccess     = mTranslator->compile(shaderStrings, 1, mCompileOptions);
        mInfoLog                    = mTranslator->getInfoSink().info.str();
        if (!compilationSuccess)
        {
            return testing::AssertionFailure() << "Shader compilation failed " << mInfoLog;
        }
        return testing::AssertionSuccess();
    }

    bool foundErrorInIntermediateTree() const { return foundInIntermediateTree("ERROR:"); }

    bool foundInIntermediateTree(const char *stringToFind) const
    {
        return mInfoLog.find(stringToFind) != std::string::npos;
    }
    std::string intermediateTree() const { return mInfoLog; }

    ShBuiltInResources mResources;
    ShCompileOptions mCompileOptions{};
    ShShaderSpec mShaderSpec = SH_WEBGL_SPEC;

  private:

    std::unique_ptr<TranslatorESSL> mTranslator;
    std::string mInfoLog;
};

TEST_F(ParseTest, UnsizedArrayConstructorNoCrash)
{
    const char kShader[] = R"(#version 310 es\n"
int A[];
int B[int[][](A)];)";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("constructing from an unsized array"));
}

TEST_F(ParseTest, UniformBlockNameReferenceConstructorNoCrash)
{
    const char kShader[] = R"(#version 300 es
precision mediump float;
out float o;
uniform a { float r; } UBOA;
void main() {
    o = float(UBOA);
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree(
        "interface block cannot be used as a constructor argument for this type"));
}

TEST_F(ParseTest, Precise320NoCrash)
{
    const char kShader[] = R"(#version 320 es
precision mediump float;
void main(){
    float t;
    precise t;
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("unsupported shader version"));
}

// Tests that layout(index=0) is parsed in es 100 shaders if an extension like
// EXT_shader_framebuffer_fetch is enabled, but this does not cause a crash.
TEST_F(ParseTest, ShaderFramebufferFetchLayoutIndexNoCrash)
{
    mResources.EXT_blend_func_extended      = 1;
    mResources.MaxDualSourceDrawBuffers     = 1;
    mResources.EXT_shader_framebuffer_fetch = 1;
    const char kShader[]                    = R"(
#extension GL_EXT_blend_func_extended: require
#extension GL_EXT_shader_framebuffer_fetch : require
layout(index=0)mediump vec4 c;
void main() { }
)";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'index' : invalid layout qualifier"));
}

TEST_F(ParseTest, Radians320NoCrash)
{
    const char kShader[] = R"(#version 320 es
precision mediump float;
vec4 s() { writeonly vec4 color; radians(color); return vec4(1); })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'writeonly' : Only allowed with shader storage blocks,"));
    EXPECT_TRUE(foundInIntermediateTree(
        "'radians' : wrong operand type - no operation 'radians' exists that"));
}

TEST_F(ParseTest, CoherentCoherentNoCrash)
{
    const char kShader[] = R"(#version 310 es
uniform highp coherent coherent readonly image2D image1;\n"
void main() {
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("coherent specified multiple times"));
}

TEST_F(ParseTest, LargeArrayIndexNoCrash)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
int rr[~1U];
out int o;
void main() {
    o = rr[1];
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("Size of declared variable exceeds implementation-defined limit"));
}

// Tests that separating variable declaration of multiple instances of a anonymous structure
// rewrites the expression types for expressions that use the variables. At the time of writing
// the expression types were left referencing the original anonymous function.
TEST_F(ParseTest, SeparateAnonymousFunctionsRewritesExpressions)
{
    const char kShader[] = R"(
struct {
    mediump vec2 d;
} s0, s1;
void main() {
    s0 = s0;
    s1 = s1;
})";
    EXPECT_TRUE(compile(kShader));
    EXPECT_FALSE(foundInIntermediateTree("anonymous"));
}

// Tests that constant folding a division of a void variable does not crash during parsing.
TEST_F(ParseTest, ConstStructWithVoidAndDivNoCrash)
{
    const char kShader[] = R"(
const struct s { void i; } ss = s();
void main() {
    highp vec3 q = ss.i / ss.i;
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("illegal use of type 'void'"));
    EXPECT_TRUE(foundInIntermediateTree("constructor does not have any arguments"));
    EXPECT_TRUE(foundInIntermediateTree("operation with void operands"));
    EXPECT_TRUE(foundInIntermediateTree(
        "wrong operand types - no operation '/' exists that takes a left-hand operand of type "
        "'const void' and a right operand of type 'const void'"));
    EXPECT_TRUE(foundInIntermediateTree(
        "cannot convert from 'const void' to 'highp 3-component vector of float'"));
}

// Tests that division of void variable returns the same errors as division of constant
// void variable (see above).
TEST_F(ParseTest, StructWithVoidAndDivErrorCheck)
{
    const char kShader[] = R"(
struct s { void i; } ss = s();
void main() {
    highp vec3 q = ss.i / ss.i;
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("illegal use of type 'void'"));
    EXPECT_TRUE(foundInIntermediateTree("constructor does not have any arguments"));
    EXPECT_TRUE(foundInIntermediateTree("operation with void operands"));
    EXPECT_TRUE(foundInIntermediateTree(
        "wrong operand types - no operation '/' exists that takes a left-hand operand of type "
        "'void' and a right operand of type 'void'"));
    EXPECT_TRUE(foundInIntermediateTree(
        "cannot convert from 'void' to 'highp 3-component vector of float'"));
}

// Tests that usage of BuildIn struct type name does not crash during parsing.
TEST_F(ParseTest, BuildInStructTypeNameDeclarationNoCrash)
{
    mCompileOptions.validateAST = 1;
    const char kShader[]        = R"(
void main() {
gl_DepthRangeParameters testVariable;
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("reserved built-in name"));
}

TEST_F(ParseTest, BuildInStructTypeNameFunctionArgumentNoCrash)
{
    mCompileOptions.validateAST = 1;
    const char kShader[]        = R"(
void testFunction(gl_DepthRangeParameters testParam){}
void main() {
testFunction(gl_DepthRange);
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("reserved built-in name"));
}

TEST_F(ParseTest, BuildInStructTypeNameFunctionReturnValueNoCrash)
{
    mCompileOptions.validateAST = 1;
    const char kShader[]        = R"(
gl_DepthRangeParameters testFunction(){return gl_DepthRange;}
void main() {
testFunction();
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("reserved built-in name"));
}

// Tests that imod of const void variable does not crash during parsing.
TEST_F(ParseTest, ConstStructVoidAndImodAndNoCrash)
{
    const char kShader[] = R"(#version 310 es
const struct s { void i; } ss = s();
void main() {
    highp vec3 q = ss.i % ss.i;
})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("illegal use of type 'void'"));
    EXPECT_TRUE(foundInIntermediateTree("constructor does not have any arguments"));
    EXPECT_TRUE(foundInIntermediateTree("operation with void operands"));
    EXPECT_TRUE(foundInIntermediateTree(
        "wrong operand types - no operation '%' exists that takes a left-hand operand of type "
        "'const void' and a right operand of type 'const void'"));
    EXPECT_TRUE(foundInIntermediateTree(
        "cannot convert from 'const void' to 'highp 3-component vector of float'"));
}

TEST_F(ParseTest, HugeUnsizedMultidimensionalArrayConstructorNoCrash)
{
    mCompileOptions.limitExpressionComplexity = true;
    std::ostringstream shader;
    shader << R"(#version 310 es
int E=int)";
    for (int i = 0; i < 10000; ++i)
    {
        shader << "[]";
    }
    shader << "()";
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("array has too many dimensions"));
}

TEST_F(ParseTest, HugeMultidimensionalArrayConstructorNoCrash)
{
    mCompileOptions.limitExpressionComplexity = true;
    std::ostringstream shader;
    shader << R"(#version 310 es
int E=int)";
    for (int i = 0; i < 10000; ++i)
    {
        shader << "[1]";
    }

    for (int i = 0; i < 10000; ++i)
    {
        shader << "(2)";
    }
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("array has too many dimensions"));
}

TEST_F(ParseTest, DeeplyNestedWhileStatementsNoCrash)
{
    mShaderSpec = SH_WEBGL2_SPEC;
    std::ostringstream shader;
    shader << R"(#version 300 es
void main() {
)";
    for (int i = 0; i < 1700; ++i)
    {
        shader << " while(true)";
    }
    shader << "; }";
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("statement is too deeply nested"));
}

TEST_F(ParseTest, DeeplyNestedForStatementsNoCrash)
{
    mShaderSpec = SH_WEBGL2_SPEC;
    std::ostringstream shader;
    shader << R"(#version 300 es
void main() {
)";
    for (int i = 0; i < 1700; ++i)
    {
        shader << " for(int i = 0; i < 10; i++)";
    }
    shader << "; }";
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("statement is too deeply nested"));
}

TEST_F(ParseTest, DeeplyNestedDoWhileStatementsNoCrash)
{
    mShaderSpec = SH_WEBGL2_SPEC;
    std::ostringstream shader;
    shader << R"(#version 300 es
void main() {
)";
    for (int i = 0; i < 1700; ++i)
    {
        shader << " do {";
    }
    for (int i = 0; i < 1700; ++i)
    {
        shader << "} while(true);";
    }
    shader << "}";
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("statement is too deeply nested"));
}

TEST_F(ParseTest, DeeplyNestedSwitchStatementsNoCrash)
{
    mShaderSpec = SH_WEBGL2_SPEC;
    std::ostringstream shader;
    shader << R"(#version 300 es
void main() {
)";
    for (int i = 0; i < 1700; ++i)
    {
        shader << " switch(1) { default: int i=0;";
    }
    for (int i = 0; i < 1700; ++i)
    {
        shader << "}";
    }
    shader << "}";
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("statement is too deeply nested"));
}

TEST_F(ParseTest, ManyChainedUnaryExpressionsNoCrash)
{
    mCompileOptions.limitExpressionComplexity = true;
    mShaderSpec                               = SH_WEBGL2_SPEC;
    std::ostringstream shader;
    shader << R"(#version 300 es
precision mediump float;
void main() {
  int iterations=0;)";
    for (int i = 0; i < 6000; ++i)
    {
        shader << "~";
    }
    shader << R"(++iterations;
}
)";
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("Expression too complex"));
}

TEST_F(ParseTest, ManyChainedAssignmentsNoCrash)
{
    mCompileOptions.limitExpressionComplexity = true;
    mShaderSpec                               = SH_WEBGL2_SPEC;
    std::ostringstream shader;
    shader << R"(#version 300 es
void main() {
    int c = 0;
)";
    for (int i = 0; i < 3750; ++i)
    {
        shader << "c=\n";
    }
    shader << "c+1; }";
    EXPECT_FALSE(compile(shader.str()));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("Expression too complex"));
}

// Test that comma expression referring to an uniform block member through instance-name is not an
// error.
TEST_F(ParseTest, UniformBlockWorks)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
         uniform B { uint e; } b;
         void main() { b.e; })";

    EXPECT_TRUE(compile(kShader));

    const char kShader2[] = R"(#version 300 es
         uniform B { uint e; } b;
         mediump float f() { return .0; }
         void main() { f(), b.e; })";
    EXPECT_TRUE(compile(kShader2));

    const char kShader3[] = R"(#version 300 es
         uniform B { uint e; };
         void main() { e; })";
    EXPECT_TRUE(compile(kShader3));

    const char kShader4[] = R"(#version 300 es
        uniform B { uint e; };
        mediump float f() { return .0; }
        void main() { f(), e; })";
    EXPECT_TRUE(compile(kShader4));

    const char kShader5[] = R"(#version 300 es
        uniform B { uint e; } b[3];
        void main() { b[0].e; })";
    EXPECT_TRUE(compile(kShader5));

    const char kShader6[] = R"(#version 300 es
        uniform B { uint e; } b[3];
        mediump float f() { return .0; }
        void main() { f(), b[0].e; })";
    EXPECT_TRUE(compile(kShader6));
}

// Test that comma expression referring to an uniform block instance-name is an error.
TEST_F(ParseTest, UniformBlockInstanceNameReferenceIsError)
{
    mShaderSpec = SH_WEBGL2_SPEC;

    const char kShader[] = R"(#version 300 es
         precision mediump float;
         uniform B { uint e; } b;
         void main() { b; })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("expression statement is not allowed for interface blocks"));

    const char kShader2[] = R"(#version 300 es
         uniform B { uint e; } b;
         mediump float f() { return .0; }
         void main() { f(), b; })";
    EXPECT_FALSE(compile(kShader2));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("',' : sequence operator is not allowed for interface blocks"));
}

// Test that expression statements resulting in a uniform block instance-name as an array subscript
// is an error.
TEST_F(ParseTest, UniformBlockInstanceNameReferenceSubscriptIsError)
{
    mShaderSpec = SH_WEBGL2_SPEC;

    const char kShader[] = R"(#version 300 es
         precision mediump float;
         uniform B { uint e; } b[3];
         void main() { b[0]; })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("expression statement is not allowed for interface blocks"));
    const char kShader2[] = R"(#version 300 es
         uniform B { uint e; } b[3];
         mediump float f() { return .0; }
         void main() { f(), b[0]; })";
    EXPECT_FALSE(compile(kShader2));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("',' : sequence operator is not allowed for interface blocks"));
}

// Test that expressions using binary operations on a uniform block instance-name is an error.
TEST_F(ParseTest, UniformBlockInstanceNameOpIsError)
{
    mShaderSpec = SH_WEBGL2_SPEC;

    const char kShader[] = R"(#version 300 es
        precision mediump float;
        uniform B { uint e; } b;
        void main() { b = b; })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'assign' : l-value required (can't modify a uniform \"b\")"));
    EXPECT_TRUE(foundInIntermediateTree("'=' : Invalid operation for interface blocks"));
    EXPECT_TRUE(foundInIntermediateTree(
        "'assign' : cannot convert from 'uniform interface block' to 'uniform interface block'"));

    const char kShader2[] = R"(#version 300 es
        uniform B { uint e; } b;
        void main() { b == b; })";
    EXPECT_FALSE(compile(kShader2));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'==' : Invalid operation for interface blocks"));
    EXPECT_TRUE(foundInIntermediateTree(
        "'==' : wrong operand types - no operation '==' exists that takes a left-hand operand of "
        "type 'uniform interface block' and a right operand of type 'uniform interface block' (or "
        "there is no acceptable conversion)"));
    const char kShader3[] = R"(#version 300 es
         uniform B { uint e; } b;
         void main() { b.e > 33u ? b : b; })";
    EXPECT_FALSE(compile(kShader3));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'?:' : ternary operator is not allowed for interface blocks"));
}

TEST_F(ParseTest, UniformBlockReferenceIsError)
{
    const char kShader[] = R"(#version 300 es
        uniform B { uint e; } b;
        void main() { B; })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'B' : variable expected"));

    const char kShader2[] = R"(#version 300 es
        uniform B { uint e; };
        mediump float f() { return .0; }
        void main() { f(), B; })";
    EXPECT_FALSE(compile(kShader2));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'B' : variable expected"));
}

// Tests that referring to functions is a parse error.
TEST_F(ParseTest, FunctionReferenceIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
        mediump float f() { return .0; }
        void main() { f; })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'f' : variable expected"));

    const char kShader2[] = R"(#version 300 es
        mediump float f() { return .0; }
        void main() { f(), f; })";
    EXPECT_FALSE(compile(kShader2));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'f' : variable expected"));
}

// Tests that referring to builtin functions is a parse error.
// Shows discrepancy where the error message is unexpected, user defined functions have
// better error message.
TEST_F(ParseTest, BuiltinFunctionReferenceIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
        void main() { sin; })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'sin' : undeclared identifier"));

    const char kShader2[] = R"(#version 300 es
        void main() { sin(3.0), sin; })";
    EXPECT_FALSE(compile(kShader2));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'sin' : undeclared identifier"));
}

// Tests that unsized array parameters fail.
TEST_F(ParseTest, UnsizedArrayParameterIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
int f(int a[], int i) {
    return i;
}
void main() { }
)";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'a' : function parameter array must specify a size"));
}

// Tests that unsized array parameters fail.
TEST_F(ParseTest, UnsizedArrayParameterIsError2)
{
    mShaderSpec          = SH_GLES3_1_SPEC;
    const char kShader[] = R"(#version 310 es
int f(int []a[1], int i) {
    return i;
}
void main() { }
)";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'a' : function parameter array must specify a size"));
}

// Tests that unnamed, unsized array parameters fail with same error message as named ones.
TEST_F(ParseTest, UnnamedUnsizedArrayParameterIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
int f(int[], int i) {
    return i;
}
void main() { }
)";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'' : function parameter array must specify a size"));
}

// Tests that different array notatinos [1]a[2], a[1][2] etc work in parameters.
TEST_F(ParseTest, ArrayParameterVariants)
{
    mShaderSpec          = SH_GLES3_1_SPEC;
    const char kShader[] = R"(#version 310 es
int f(int[1][2] a) {
    return a[0][0];
}
int g(int a[1][2]) {
    return a[0][0];
}
int h(int[1]a[2]) {
    return a[0][0];
}
void main() {
    int[1][2] a;
    int b[1][2];
    int x1 = f(a);
    int x2 = f(b);
    int x3 = g(a);
    int x4 = g(b);

    int[1] c[2];
    int d[2][1];
    int y1 = h(c);
    int y2 = h(d);
}
)";
    EXPECT_TRUE(compile(kShader));
}

// Tests that parameters parse the [1]a[2] notation in correct order.
TEST_F(ParseTest, ArrayParameterVariantsMismatchIsError2)
{
    mShaderSpec          = SH_GLES3_1_SPEC;
    const char kShader[] = R"(#version 310 es
int f(int[1]a[2]) {
    return a[0][0];
}
void main() {
    int[1][2] a;
    int x = f(a);
}
)";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'f' : no matching overloaded function found"));
}

// Tests that specifying a struct in a function parameter is a parse error.
TEST_F(ParseTest, StructSpecificationFunctionParameterIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
precision highp float;
float f(struct S {float f;} a) {
    return a.f;
}
void main() { })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'a' : Function parameter type cannot be a structure definition"));
}

// Tests that specifying a struct in a function parameter is the same parse error as with named one.
TEST_F(ParseTest, StructSpecificationUnnamedFunctionParameterIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
precision highp float;
float f(struct S {float f;}) {
    return a.f;
}
void main() { })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'' : Function parameter type cannot be a structure definition"));
}

// Tests that specifying a struct in a function parameter is the same parse error as with named one.
TEST_F(ParseTest, UnnamedStructSpecificationUnnamedFunctionParameterIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
precision highp float;
float f(struct {float f;}) {
    return a.f;
}
void main() { })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'' : Function parameter type cannot be a structure definition"));
}

// Tests that specifying a struct in a function parameter is the same parse error as with named one.
TEST_F(ParseTest, UnnamedStructSpecificationFunctionParameterIsError)
{
    mShaderSpec          = SH_WEBGL2_SPEC;
    const char kShader[] = R"(#version 300 es
precision highp float;
float f(struct {float f;} d) {
    return a.f;
}
void main() { })";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'d' : Function parameter type cannot be a structure definition"));
}

TEST_F(ParseTest, SeparateStructStructSpecificationFunctionNoCrash)
{
    mCompileOptions.validateAST = 1;
    const char kShader[] =
        R"(struct S{int f;};struct S2{S h;} o() { return S2(S(1)); } void main(){ S2 s2 = o(); })";
    EXPECT_TRUE(compile(kShader));
}

// Test showing that prototypes get the function definition variable names.
// An example where parser loses information.
TEST_F(ParseTest, VariableNamesInPrototypesUnnamedOut)
{
    const char kShader[]   = R"(
precision highp float;
void f(out float, out float);
void main()
{
    gl_FragColor = vec4(0.5);
    f(gl_FragColor.r, gl_FragColor.g);
}
void f(out float r, out float)
{
    r = 1.0;
}
)";
    const char kExpected[] = R"(0:2: Code block
0:3:   Function Prototype: 'f' (symbol id 3001) (void)
0:3:     parameter: 'r' (symbol id 3006) (out highp float)
0:3:     parameter: '' (symbol id 3007) (out highp float)
0:4:   Function Definition:
0:4:     Function Prototype: 'main' (symbol id 3004) (void)
0:5:     Code block
0:6:       move second child to first child (mediump 4-component vector of float)
0:6:         gl_FragColor (symbol id 2230) (FragColor mediump 4-component vector of float)
0:6:         Constant union (const mediump 4-component vector of float)
0:6:           0.5 (const float)
0:6:           0.5 (const float)
0:6:           0.5 (const float)
0:6:           0.5 (const float)
0:7:       Call a function: 'f' (symbol id 3001) (void)
0:7:         vector swizzle (x) (mediump float)
0:7:           gl_FragColor (symbol id 2230) (FragColor mediump 4-component vector of float)
0:7:         vector swizzle (y) (mediump float)
0:7:           gl_FragColor (symbol id 2230) (FragColor mediump 4-component vector of float)
0:9:   Function Definition:
0:9:     Function Prototype: 'f' (symbol id 3001) (void)
0:9:       parameter: 'r' (symbol id 3006) (out highp float)
0:9:       parameter: '' (symbol id 3007) (out highp float)
0:10:     Code block
0:11:       move second child to first child (highp float)
0:11:         'r' (symbol id 3006) (out highp float)
0:11:         Constant union (const highp float)
0:11:           1.0 (const float)
)";
    compile(kShader);
    EXPECT_EQ(kExpected, intermediateTree());
}

TEST_F(ParseTest, ConstInSamplerParamNoCrash)
{
    mCompileOptions.validateAST = 1;
    const char kShader[]        = R"(void n(const in sampler2D){2;} void main(){})";
    EXPECT_TRUE(compile(kShader));
}

TEST_F(ParseTest, ConstSamplerParamNoCrash)
{
    mCompileOptions.validateAST = 1;
    const char kShader[]        = R"(void n(const sampler2D){2;} void main(){})";
    EXPECT_TRUE(compile(kShader));
}

TEST_F(ParseTest, InConstSamplerParamIsError)
{
    mCompileOptions.validateAST = 1;
    const char kShader[]        = R"(void n(in const sampler2D){2;} void main(){})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(foundInIntermediateTree("'const' : invalid parameter qualifier"));
}

TEST_F(ParseTest, UniformBlockInstanceUnsizedArrayIsError)
{
    const char kShader[] = R"(#version 300 es
precision mediump float;out vec4 o;uniform a{float r;}u[];void main(){o=vec4(u[0].r+u[1].r+u[1].r);})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'u' : implicitly sized arrays only allowed for tessellation "
                                "shaders or geometry shader inputs"));
}

TEST_F(ParseTest, InputBlockInstanceUnsizedArrayIsError)
{
    const char kShader[] = R"(#version 300 es
precision mediump float;out vec4 o;in a{float r;}i[];void main(){o=vec4(i[0].r+i[1].r+i[1].r);})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'i' : implicitly sized arrays only allowed for tessellation "
                                "shaders or geometry shader inputs"));
}

TEST_F(ParseTest, OutputBlockInstanceUnsizedArrayIsError)
{
    const char kShader[] = R"(#version 300 es
precision mediump float;out a{float r;}o[];void main(){o[0].r=1.0; o[1].r=2.0;})";
    EXPECT_FALSE(compile(kShader));
    EXPECT_TRUE(foundErrorInIntermediateTree());
    EXPECT_TRUE(
        foundInIntermediateTree("'o' : implicitly sized arrays only allowed for tessellation "
                                "shaders or geometry shader inputs"));
}
