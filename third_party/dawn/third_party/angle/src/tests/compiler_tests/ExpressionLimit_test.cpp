//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include <sstream>
#include <string>
#include <vector>
#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"

class ExpressionLimitTest : public testing::Test
{
  protected:
    static const int kMaxExpressionComplexity = 16;
    static const int kMaxCallStackDepth       = 16;
    static const int kMaxFunctionParameters   = 16;

    virtual void SetUp()
    {
        memset(&resources, 0, sizeof(resources));

        GenerateResources(&resources);
    }

    // Set up the per compile resources
    static void GenerateResources(ShBuiltInResources *res)
    {
        sh::InitBuiltInResources(res);

        res->MaxVertexAttribs             = 8;
        res->MaxVertexUniformVectors      = 128;
        res->MaxVaryingVectors            = 8;
        res->MaxVertexTextureImageUnits   = 0;
        res->MaxCombinedTextureImageUnits = 8;
        res->MaxTextureImageUnits         = 8;
        res->MaxFragmentUniformVectors    = 16;
        res->MaxDrawBuffers               = 1;

        res->OES_standard_derivatives = 0;
        res->OES_EGL_image_external   = 0;

        res->MaxExpressionComplexity = kMaxExpressionComplexity;
        res->MaxCallStackDepth       = kMaxCallStackDepth;
        res->MaxFunctionParameters   = kMaxFunctionParameters;
    }

    static void GenerateLongExpression(int length, std::stringstream *ss)
    {
        for (int ii = 0; ii < length; ++ii)
        {
            *ss << "+ vec4(" << ii << ")";
        }
    }

    static std::string GenerateShaderWithLongExpression(int length)
    {
        static const char *shaderStart =
            R"(precision mediump float;
            uniform vec4 u_color;
            void main()
            {
               gl_FragColor = u_color
        )";

        std::stringstream ss;
        ss << shaderStart;
        GenerateLongExpression(length, &ss);
        ss << "; }";

        return ss.str();
    }

    static std::string GenerateShaderWithUnusedLongExpression(int length)
    {
        static const char *shaderStart =
            R"(precision mediump float;
            uniform vec4 u_color;
            void main()
            {
               gl_FragColor = u_color;
            }
            vec4 someFunction() {
              return u_color
        )";

        std::stringstream ss;

        ss << shaderStart;
        GenerateLongExpression(length, &ss);
        ss << "; }";

        return ss.str();
    }

    static void GenerateDeepFunctionStack(int length, std::stringstream *ss)
    {
        static const char *shaderStart =
            R"(precision mediump float;
            uniform vec4 u_color;
            vec4 function0()  {
              return u_color;
            }
        )";

        *ss << shaderStart;
        for (int ii = 0; ii < length; ++ii)
        {
            *ss << "vec4 function" << (ii + 1) << "() {\n"
                << "  return function" << ii << "();\n"
                << "}\n";
        }
    }

    static std::string GenerateShaderWithDeepFunctionStack(int length)
    {
        std::stringstream ss;

        GenerateDeepFunctionStack(length, &ss);

        ss << "void main() {\n" << "  gl_FragColor = function" << length << "();\n" << "}";

        return ss.str();
    }

    static std::string GenerateShaderWithUnusedDeepFunctionStack(int length)
    {
        std::stringstream ss;

        GenerateDeepFunctionStack(length, &ss);

        ss << "void main() {\n" << "  gl_FragColor = vec4(0,0,0,0);\n" << "}";

        return ss.str();
    }

    static std::string GenerateShaderWithFunctionParameters(int parameters)
    {
        std::stringstream ss;

        ss << "precision mediump float;\n" << "\n" << "float foo(";
        for (int i = 0; i < parameters; ++i)
        {
            ss << "float f" << i;
            if (i + 1 < parameters)
            {
                ss << ", ";
            }
        }
        ss << ")\n"
           << "{\n"
           << "    return f0;\n"
           << "}\n"
           << "\n"
           << "void main()\n"
           << "{\n"
           << "    gl_FragColor = vec4(0,0,0,0);\n"
           << "}";

        return ss.str();
    }

    static std::string GenerateShaderWithNestingInsideSwitch(int nesting)
    {
        std::stringstream shaderString;
        shaderString <<
            R"(#version 300 es
            uniform int u;

            void main()
            {
                int x;
                switch (u)
                {
                    case 0:
                        x = x)";
        for (int i = 0; i < nesting; ++i)
        {
            shaderString << " + x";
        }
        shaderString <<
            R"(;
                }  // switch (u)
            })";
        return shaderString.str();
    }

    static std::string GenerateShaderWithNestingInsideGlobalInitializer(int nesting)
    {
        std::stringstream shaderString;
        shaderString <<
            R"(uniform int u;
            int x = u)";

        for (int i = 0; i < nesting; ++i)
        {
            shaderString << " + u";
        }
        shaderString << R"(;
            void main()
            {
                gl_FragColor = vec4(0.0);
            })";
        return shaderString.str();
    }

    // Compiles a shader and if there's an error checks for a specific
    // substring in the error log. This way we know the error is specific
    // to the issue we are testing.
    bool CheckShaderCompilation(ShHandle compiler,
                                const char *source,
                                const ShCompileOptions &compileOptions,
                                const char *expected_error)
    {
        bool success = sh::Compile(compiler, &source, 1, compileOptions) != 0;
        if (success)
        {
            success = !expected_error;
        }
        else
        {
            std::string log = sh::GetInfoLog(compiler);
            if (expected_error)
                success = log.find(expected_error) != std::string::npos;

            EXPECT_TRUE(success) << log << "\n----shader----\n" << source;
        }
        return success;
    }

    ShBuiltInResources resources;
};

constexpr char kExpressionTooComplex[] = "Expression too complex";
constexpr char kCallStackTooDeep[]     = "Call stack too deep";
constexpr char kHasRecursion[]         = "Recursive function call in the following call chain";
constexpr char kTooManyParameters[]    = "Function has too many parameters";
constexpr char kTooComplexSwitch[]     = "too complex expressions inside a switch statement";
constexpr char kGlobalVariableInit[] = "global variable initializers must be constant expressions";
constexpr char kTooManyFields[]      = "Too many fields in the struct";

TEST_F(ExpressionLimitTest, ExpressionComplexity)
{
    ShShaderSpec spec       = SH_WEBGL_SPEC;
    ShShaderOutput output   = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions          = {};
    compileOptions.limitExpressionComplexity = true;

    // Test expression under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithLongExpression(kMaxExpressionComplexity - 10).c_str(),
        compileOptions, nullptr));
    // Test expression over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithLongExpression(kMaxExpressionComplexity + 10).c_str(),
        compileOptions, kExpressionTooComplex));
    // Test expression over the limit without a limit does not fail.
    compileOptions.limitExpressionComplexity = false;
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithLongExpression(kMaxExpressionComplexity + 10).c_str(),
        compileOptions, nullptr));
    sh::Destruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, UnusedExpressionComplexity)
{
    ShShaderSpec spec       = SH_WEBGL_SPEC;
    ShShaderOutput output   = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions          = {};
    compileOptions.limitExpressionComplexity = true;

    // Test expression under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedLongExpression(kMaxExpressionComplexity - 10).c_str(),
        compileOptions, nullptr));
    // Test expression over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedLongExpression(kMaxExpressionComplexity + 10).c_str(),
        compileOptions, kExpressionTooComplex));
    // Test expression over the limit without a limit does not fail.
    compileOptions.limitExpressionComplexity = false;
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler,
        GenerateShaderWithUnusedLongExpression(kMaxExpressionComplexity + 10).c_str(),
        compileOptions, nullptr));
    sh::Destruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, CallStackDepth)
{
    ShShaderSpec spec       = SH_WEBGL_SPEC;
    ShShaderOutput output   = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions    = {};
    compileOptions.limitCallStackDepth = true;

    // Test call stack under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithDeepFunctionStack(kMaxCallStackDepth - 10).c_str(),
        compileOptions, nullptr));
    // Test call stack over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithDeepFunctionStack(kMaxCallStackDepth + 10).c_str(),
        compileOptions, kCallStackTooDeep));
    // Test call stack over the limit without limit does not fail.
    compileOptions.limitCallStackDepth = false;
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithDeepFunctionStack(kMaxCallStackDepth + 10).c_str(),
        compileOptions, nullptr));
    sh::Destruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, UnusedCallStackDepth)
{
    ShShaderSpec spec       = SH_WEBGL_SPEC;
    ShShaderOutput output   = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions    = {};
    compileOptions.limitCallStackDepth = true;

    // Test call stack under the limit passes.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithUnusedDeepFunctionStack(kMaxCallStackDepth - 10).c_str(),
        compileOptions, nullptr));
    // Test call stack over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithUnusedDeepFunctionStack(kMaxCallStackDepth + 10).c_str(),
        compileOptions, kCallStackTooDeep));
    // Test call stack over the limit without limit does not fail.
    compileOptions.limitCallStackDepth = false;
    EXPECT_TRUE(CheckShaderCompilation(
        vertexCompiler, GenerateShaderWithUnusedDeepFunctionStack(kMaxCallStackDepth + 10).c_str(),
        compileOptions, nullptr));
    sh::Destruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, Recursion)
{
    ShShaderSpec spec       = SH_WEBGL_SPEC;
    ShShaderOutput output   = SH_ESSL_OUTPUT;
    ShHandle vertexCompiler = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions = {};

    static const char *shaderWithRecursion0 =
        R"(precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return someFunc();
        }

        void main() {
            gl_FragColor = u_color * someFunc();
        }
    )";

    static const char *shaderWithRecursion1 =
        R"(precision mediump float;
        uniform vec4 u_color;

        vec4 someFunc();

        vec4 someFunc1()  {
            return someFunc();
        }

        vec4 someFunc()  {
            return someFunc1();
        }

        void main() {
            gl_FragColor = u_color * someFunc();
        }
    )";

    static const char *shaderWithRecursion2 =
        R"(precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            if (u_color.x > 0.5) {
                return someFunc();
            } else {
                return vec4(1);
            }
        }

        void main() {
            gl_FragColor = someFunc();
        }
    )";

    static const char *shaderWithRecursion3 =
        R"(precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            if (u_color.x > 0.5) {
                return vec4(1);
            } else {
                return someFunc();
            }
        }

        void main() {
            gl_FragColor = someFunc();
        }
    )";

    static const char *shaderWithRecursion4 =
        R"(precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return (u_color.x > 0.5) ? vec4(1) : someFunc();
        }

        void main() {
            gl_FragColor = someFunc();
        }
    )";

    static const char *shaderWithRecursion5 =
        R"(precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return (u_color.x > 0.5) ? someFunc() : vec4(1);
        }

        void main() {
            gl_FragColor = someFunc();
        }
    )";

    static const char *shaderWithRecursion6 =
        R"(precision mediump float;
        uniform vec4 u_color;
        vec4 someFunc()  {
            return someFunc();
        }

        void main() {
            gl_FragColor = u_color;
        }
    )";

    static const char *shaderWithNoRecursion =
        R"(precision mediump float;
        uniform vec4 u_color;

        vec3 rgb(int r, int g, int b) {
            return vec3(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0);
        }

        void main() {
            vec3 hairColor0 = rgb(151, 200, 234);
            vec3 faceColor2 = rgb(183, 148, 133);
            gl_FragColor = u_color + vec4(hairColor0 + faceColor2, 0);
        }
    )";

    static const char *shaderWithRecursion7 =
        R"(precision mediump float;
        uniform vec4 u_color;

        vec4 function2() {
            return u_color;
        }

        vec4 function1() {
            vec4 a = function2();
            vec4 b = function1();
            return a + b;
        }

        void main() {
            gl_FragColor = function1();
        }
    )";

    static const char *shaderWithRecursion8 =
        R"(precision mediump float;
        uniform vec4 u_color;

        vec4 function1();

        vec4 function3() {
            return function1();
        }

        vec4 function2() {
            return function3();
        }

        vec4 function1() {
            return function2();
        }

        void main() {
            gl_FragColor = function1();
        }
    )";

    // Check simple recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion0, compileOptions,
                                       kHasRecursion));
    // Check simple recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion1, compileOptions,
                                       kHasRecursion));
    // Check if recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion2, compileOptions,
                                       kHasRecursion));
    // Check if recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion3, compileOptions,
                                       kHasRecursion));
    // Check ternary recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion4, compileOptions,
                                       kHasRecursion));
    // Check ternary recursions fails.
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion5, compileOptions,
                                       kHasRecursion));

    // Check some more forms of recursion
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion6, compileOptions,
                                       kHasRecursion));
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion7, compileOptions,
                                       kHasRecursion));
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion8, compileOptions,
                                       kHasRecursion));
    // Check unused recursions fails if limiting call stack
    // since we check all paths.
    compileOptions.limitCallStackDepth = true;
    EXPECT_TRUE(CheckShaderCompilation(vertexCompiler, shaderWithRecursion6, compileOptions,
                                       kHasRecursion));

    // Check unused recursions passes.
    EXPECT_TRUE(
        CheckShaderCompilation(vertexCompiler, shaderWithNoRecursion, compileOptions, nullptr));
    // Check unused recursions passes if limiting call stack.
    EXPECT_TRUE(
        CheckShaderCompilation(vertexCompiler, shaderWithNoRecursion, compileOptions, nullptr));
    sh::Destruct(vertexCompiler);
}

TEST_F(ExpressionLimitTest, FunctionParameterCount)
{
    ShShaderSpec spec     = SH_WEBGL_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle compiler     = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions          = {};
    compileOptions.limitExpressionComplexity = true;

    // Test parameters under the limit succeeds.
    EXPECT_TRUE(CheckShaderCompilation(
        compiler, GenerateShaderWithFunctionParameters(kMaxFunctionParameters).c_str(),
        compileOptions, nullptr));
    // Test parameters over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        compiler, GenerateShaderWithFunctionParameters(kMaxFunctionParameters + 1).c_str(),
        compileOptions, kTooManyParameters));
    // Test parameters over the limit without limit does not fail.
    compileOptions.limitExpressionComplexity = false;
    EXPECT_TRUE(CheckShaderCompilation(
        compiler, GenerateShaderWithFunctionParameters(kMaxFunctionParameters + 1).c_str(),
        compileOptions, nullptr));
    sh::Destruct(compiler);
}

TEST_F(ExpressionLimitTest, NestingInsideSwitch)
{
    ShShaderSpec spec     = SH_WEBGL2_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle compiler     = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions          = {};
    compileOptions.limitExpressionComplexity = true;

    // Test nesting over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        compiler, GenerateShaderWithNestingInsideSwitch(kMaxExpressionComplexity + 1).c_str(),
        compileOptions, kExpressionTooComplex));
    // Test that nesting way over the limit doesn't cause stack overflow but is handled
    // gracefully.
    EXPECT_TRUE(CheckShaderCompilation(compiler,
                                       GenerateShaderWithNestingInsideSwitch(5000).c_str(),
                                       compileOptions, kTooComplexSwitch));
    // Test nesting over the limit without limit does not fail.
    compileOptions.limitExpressionComplexity = false;
    EXPECT_TRUE(CheckShaderCompilation(
        compiler, GenerateShaderWithNestingInsideSwitch(kMaxExpressionComplexity + 1).c_str(),
        compileOptions, nullptr));
    sh::Destruct(compiler);
}

TEST_F(ExpressionLimitTest, NestingInsideGlobalInitializer)
{
    ShShaderSpec spec     = SH_WEBGL_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle compiler     = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions          = {};
    compileOptions.limitExpressionComplexity = true;

    // Test nesting over the limit fails.
    EXPECT_TRUE(CheckShaderCompilation(
        compiler,
        GenerateShaderWithNestingInsideGlobalInitializer(kMaxExpressionComplexity + 1).c_str(),
        compileOptions, kExpressionTooComplex));
    // Test that nesting way over the limit doesn't cause stack overflow but is handled
    // gracefully.
    EXPECT_TRUE(CheckShaderCompilation(
        compiler, GenerateShaderWithNestingInsideGlobalInitializer(5000).c_str(), compileOptions,
        kGlobalVariableInit));
    // Test nesting over the limit without limit does not fail.
    compileOptions.limitExpressionComplexity = false;
    EXPECT_TRUE(CheckShaderCompilation(
        compiler,
        GenerateShaderWithNestingInsideGlobalInitializer(kMaxExpressionComplexity + 1).c_str(),
        compileOptions, nullptr));
    sh::Destruct(compiler);
}

TEST_F(ExpressionLimitTest, TooManyStructFields)
{
    ShShaderSpec spec     = SH_WEBGL2_SPEC;
    ShShaderOutput output = SH_ESSL_OUTPUT;
    ShHandle compiler     = sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &resources);
    ShCompileOptions compileOptions = {};

    std::ostringstream fs;
    fs << R"(#version 300 es
precision highp float;
struct TooManyFields
{
)";
    for (uint32_t i = 0; i < (1 << 16); ++i)
    {
        fs << "    float field" << i << ";\n";
    }
    fs << R"(};
uniform B { TooManyFields s; };
out vec4 color;
void main() {
    color = vec4(s.field0, 0.0, 0.0, 1.0);
})";

    EXPECT_TRUE(CheckShaderCompilation(compiler, fs.str().c_str(), compileOptions, kTooManyFields));
    sh::Destruct(compiler);
}
