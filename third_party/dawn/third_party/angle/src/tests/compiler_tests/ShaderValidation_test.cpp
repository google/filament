//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderValidation_test.cpp:
//   Tests that malformed shaders fail compilation, and that correct shaders pass compilation.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

// Tests that don't target a specific version of the API spec (sometimes there are minor
// differences). They choose the shader spec version with version directives.
class FragmentShaderValidationTest : public ShaderCompileTreeTest
{
  public:
    FragmentShaderValidationTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

// Tests that don't target a specific version of the API spec (sometimes there are minor
// differences). They choose the shader spec version with version directives.
class VertexShaderValidationTest : public ShaderCompileTreeTest
{
  public:
    VertexShaderValidationTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

class WebGL2FragmentShaderValidationTest : public ShaderCompileTreeTest
{
  public:
    WebGL2FragmentShaderValidationTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL2_SPEC; }
};

class WebGL1FragmentShaderValidationTest : public ShaderCompileTreeTest
{
  public:
    WebGL1FragmentShaderValidationTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL_SPEC; }
};

class ComputeShaderValidationTest : public ShaderCompileTreeTest
{
  public:
    ComputeShaderValidationTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_COMPUTE_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

class ComputeShaderEnforcePackingValidationTest : public ComputeShaderValidationTest
{
  public:
    ComputeShaderEnforcePackingValidationTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->MaxComputeUniformComponents = kMaxComputeUniformComponents;

        // We need both MaxFragmentUniformVectors and MaxFragmentUniformVectors smaller than
        // MaxComputeUniformComponents / 4.
        resources->MaxVertexUniformVectors   = 16;
        resources->MaxFragmentUniformVectors = 16;
    }

    void SetUp() override
    {
        mCompileOptions.enforcePackingRestrictions = true;
        ShaderCompileTreeTest::SetUp();
    }

    // It is unnecessary to use a very large MaxComputeUniformComponents in this test.
    static constexpr GLint kMaxComputeUniformComponents = 128;
};

class GeometryShaderValidationTest : public ShaderCompileTreeTest
{
  public:
    GeometryShaderValidationTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->EXT_geometry_shader = 1;
    }
    ::GLenum getShaderType() const override { return GL_GEOMETRY_SHADER_EXT; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
};

class FragmentShaderEXTGeometryShaderValidationTest : public FragmentShaderValidationTest
{
  public:
    FragmentShaderEXTGeometryShaderValidationTest() {}

  protected:
    void initResources(ShBuiltInResources *resources) override
    {
        resources->EXT_geometry_shader = 1;
    }
};

// This is a test for a bug that used to exist in ANGLE:
// Calling a function with all parameters missing should not succeed.
TEST_F(FragmentShaderValidationTest, FunctionParameterMismatch)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "void main() {\n"
        "   float ff = fun();\n"
        "   gl_FragColor = vec4(ff);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Functions can't be redeclared as variables in the same scope (ESSL 1.00 section 4.2.7)
TEST_F(FragmentShaderValidationTest, RedeclaringFunctionAsVariable)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "float fun;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Functions can't be redeclared as structs in the same scope (ESSL 1.00 section 4.2.7)
TEST_F(FragmentShaderValidationTest, RedeclaringFunctionAsStruct)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "struct fun { float a; };\n"
        "void main() {\n"
        "   gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Functions can't be redeclared with different qualifiers (ESSL 1.00 section 6.1.0)
TEST_F(FragmentShaderValidationTest, RedeclaringFunctionWithDifferentQualifiers)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(out float a);\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "void main() {\n"
        "   gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Assignment and equality are undefined for structures containing arrays (ESSL 1.00 section 5.7)
TEST_F(FragmentShaderValidationTest, CompareStructsContainingArrays)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct s { float a[3]; };\n"
        "void main() {\n"
        "   s a;\n"
        "   s b;\n"
        "   bool c = (a == b);\n"
        "   gl_FragColor = vec4(c ? 1.0 : 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Assignment and equality are undefined for structures containing arrays (ESSL 1.00 section 5.7)
TEST_F(FragmentShaderValidationTest, AssignStructsContainingArrays)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct s { float a[3]; };\n"
        "void main() {\n"
        "   s a;\n"
        "   s b;\n"
        "   b.a[0] = 0.0;\n"
        "   a = b;\n"
        "   gl_FragColor = vec4(a.a[0]);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Assignment and equality are undefined for structures containing samplers (ESSL 1.00 sections 5.7
// and 5.9)
TEST_F(FragmentShaderValidationTest, CompareStructsContainingSamplers)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct s { sampler2D foo; };\n"
        "uniform s a;\n"
        "uniform s b;\n"
        "void main() {\n"
        "   bool c = (a == b);\n"
        "   gl_FragColor = vec4(c ? 1.0 : 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Samplers are not allowed as l-values (ESSL 3.00 section 4.1.7), our interpretation is that this
// extends to structs containing samplers. ESSL 1.00 spec is clearer about this.
TEST_F(FragmentShaderValidationTest, AssignStructsContainingSamplers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct s { sampler2D foo; };\n"
        "uniform s a;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   s b;\n"
        "   b = a;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// This is a regression test for a particular bug that was in ANGLE.
// It also verifies that ESSL3 functionality doesn't leak to ESSL1.
TEST_F(FragmentShaderValidationTest, ArrayWithNoSizeInInitializerList)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main() {\n"
        "   float a[2], b[];\n"
        "   gl_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Const variables need an initializer.
TEST_F(FragmentShaderValidationTest, ConstVarNotInitialized)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   const float a;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Const variables need an initializer. In ESSL1 const structs containing
// arrays are not allowed at all since it's impossible to initialize them.
// Even though this test is for ESSL3 the only thing that's critical for
// ESSL1 is the non-initialization check that's used for both language versions.
// Whether ESSL1 compilation generates the most helpful error messages is a
// secondary concern.
TEST_F(FragmentShaderValidationTest, ConstStructNotInitialized)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "   float a[3];\n"
        "};\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   const S b;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Const variables need an initializer. In ESSL1 const arrays are not allowed
// at all since it's impossible to initialize them.
// Even though this test is for ESSL3 the only thing that's critical for
// ESSL1 is the non-initialization check that's used for both language versions.
// Whether ESSL1 compilation generates the most helpful error messages is a
// secondary concern.
TEST_F(FragmentShaderValidationTest, ConstArrayNotInitialized)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   const float a[3];\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Block layout qualifiers can't be used on non-block uniforms (ESSL 3.00 section 4.3.8.3)
TEST_F(FragmentShaderValidationTest, BlockLayoutQualifierOnRegularUniform)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(packed) uniform mat2 x;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Block layout qualifiers can't be used on non-block uniforms (ESSL 3.00 section 4.3.8.3)
TEST_F(FragmentShaderValidationTest, BlockLayoutQualifierOnUniformWithEmptyDecl)
{
    // Yes, the comma in the declaration below is not a typo.
    // Empty declarations are allowed in GLSL.
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(packed) uniform mat2, x;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Arrays of arrays are not allowed (ESSL 3.00 section 4.1.9)
TEST_F(FragmentShaderValidationTest, ArraysOfArrays1)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[5] a[3];\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Arrays of arrays are not allowed (ESSL 3.00 section 4.1.9)
TEST_F(FragmentShaderValidationTest, ArraysOfArrays2)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[2] a, b[3];\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Arrays of arrays are not allowed (ESSL 3.00 section 4.1.9). Test this in a struct.
TEST_F(FragmentShaderValidationTest, ArraysOfArraysInStruct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct S {\n"
        "    float[2] foo[3];\n"
        "};\n"
        "void main() {\n"
        "    my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test invalid dimensionality of implicitly sized array constructor arguments.
TEST_F(FragmentShaderValidationTest,
       TooHighDimensionalityOfImplicitlySizedArrayOfArraysConstructorArguments)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    float[][] a = float[][](float[1][1](float[1](1.0)), float[1][1](float[1](2.0)));\n"
        "    my_FragColor = vec4(a[0][0]);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test invalid dimensionality of implicitly sized array constructor arguments.
TEST_F(FragmentShaderValidationTest,
       TooLowDimensionalityOfImplicitlySizedArrayOfArraysConstructorArguments)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    float[][][] a = float[][][](float[2](1.0, 2.0), float[2](3.0, 4.0));\n"
        "    my_FragColor = vec4(a[0][0][0]);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Implicitly sized arrays need to be initialized (ESSL 3.00 section 4.1.9)
TEST_F(FragmentShaderValidationTest, UninitializedImplicitArraySize)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[] a;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// An operator can only form a constant expression if all the operands are constant expressions
// - even operands of ternary operator that are never evaluated. (ESSL 3.00 section 4.3.3)
TEST_F(FragmentShaderValidationTest, TernaryOperatorNotConstantExpression)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform bool u;\n"
        "void main() {\n"
        "   const bool a = true ? true : u;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Ternary operator can operate on arrays (ESSL 3.00 section 5.7)
TEST_F(FragmentShaderValidationTest, TernaryOperatorOnArrays)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[1] a = float[1](0.0);\n"
        "   float[1] b = float[1](1.0);\n"
        "   float[1] c = true ? a : b;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Ternary operator can operate on structs (ESSL 3.00 section 5.7)
TEST_F(FragmentShaderValidationTest, TernaryOperatorOnStructs)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct S { float foo; };\n"
        "void main() {\n"
        "   S a = S(0.0);\n"
        "   S b = S(1.0);\n"
        "   S c = true ? a : b;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Array length() returns a constant signed integral expression (ESSL 3.00 section 4.1.9)
// Assigning it to unsigned should result in an error.
TEST_F(FragmentShaderValidationTest, AssignArrayLengthToUnsigned)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   int[1] arr;\n"
        "   uint l = arr.length();\n"
        "   my_FragColor = vec4(float(l));\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with a varying should be an error.
TEST_F(FragmentShaderValidationTest, AssignVaryingToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying float a;\n"
        "float b = a * 2.0;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 3.00 section 4.3)
// Initializing with an uniform should be an error.
TEST_F(FragmentShaderValidationTest, AssignUniformToGlobalESSL3)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform float a;\n"
        "float b = a * 2.0;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an uniform used to generate a warning on ESSL 1.00 because of legacy
// compatibility, but that causes dEQP to fail (which expects an error)
TEST_F(FragmentShaderValidationTest, AssignUniformToGlobalESSL1)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float a;\n"
        "float b = a * 2.0;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an user-defined function call should be an error.
TEST_F(FragmentShaderValidationTest, AssignFunctionCallToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float foo() { return 1.0; }\n"
        "float b = foo();\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an assignment to another global should be an error.
TEST_F(FragmentShaderValidationTest, AssignAssignmentToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float c = 1.0;\n"
        "float b = (c = 0.0);\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with incrementing another global should be an error.
TEST_F(FragmentShaderValidationTest, AssignIncrementToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float c = 1.0;\n"
        "float b = (c++);\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with a texture lookup function call should be an error.
TEST_F(FragmentShaderValidationTest, AssignTexture2DToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform mediump sampler2D s;\n"
        "float b = texture2D(s, vec2(0.5, 0.5)).x;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 3.00 section 4.3)
// Initializing with a non-constant global should be an error.
TEST_F(FragmentShaderValidationTest, AssignNonConstGlobalToGlobal)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "float a = 1.0;\n"
        "float b = a * 2.0;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 3.00 section 4.3)
// Initializing with a constant global should be fine.
TEST_F(FragmentShaderValidationTest, AssignConstGlobalToGlobal)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "const float a = 1.0;\n"
        "float b = a * 2.0;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(b);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Statically assigning to both gl_FragData and gl_FragColor is forbidden (ESSL 1.00 section 7.2)
TEST_F(FragmentShaderValidationTest, WriteBothFragDataAndFragColor)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void foo() {\n"
        "   gl_FragData[0].a++;\n"
        "}\n"
        "void main() {\n"
        "   gl_FragColor.x += 0.0;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Version directive must be on the first line (ESSL 3.00 section 3.3)
TEST_F(FragmentShaderValidationTest, VersionOnSecondLine)
{
    const std::string &shaderString =
        "\n"
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Layout qualifier can only appear in global scope (ESSL 3.00 section 4.3.8)
TEST_F(FragmentShaderValidationTest, LayoutQualifierInCondition)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    int i = 0;\n"
        "    for (int j = 0; layout(location = 0) bool b = false; ++j) {\n"
        "        ++i;\n"
        "    }\n"
        "    my_FragColor = u;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Layout qualifier can only appear where specified (ESSL 3.00 section 4.3.8)
TEST_F(FragmentShaderValidationTest, LayoutQualifierInFunctionReturnType)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "layout(location = 0) vec4 foo() {\n"
        "    return u;\n"
        "}\n"
        "void main() {\n"
        "    my_FragColor = foo();\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// If there is more than one output, the location must be specified for all outputs.
// (ESSL 3.00.04 section 4.3.8.2)
TEST_F(FragmentShaderValidationTest, TwoOutputsNoLayoutQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "out vec4 my_SecondaryFragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(1.0);\n"
        "    my_SecondaryFragColor = vec4(0.5);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// (ESSL 3.00.04 section 4.3.8.2)
TEST_F(FragmentShaderValidationTest, TwoOutputsFirstLayoutQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "layout(location = 0) out vec4 my_FragColor;\n"
        "out vec4 my_SecondaryFragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(1.0);\n"
        "    my_SecondaryFragColor = vec4(0.5);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// (ESSL 3.00.04 section 4.3.8.2)
TEST_F(FragmentShaderValidationTest, TwoOutputsSecondLayoutQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "layout(location = 0) out vec4 my_SecondaryFragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(1.0);\n"
        "    my_SecondaryFragColor = vec4(0.5);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Uniforms can be arrays (ESSL 3.00 section 4.3.5)
TEST_F(FragmentShaderValidationTest, UniformArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4[2] u;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = u[0];\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Fragment shader input variables cannot be arrays of structs (ESSL 3.00 section 4.3.4)
TEST_F(FragmentShaderValidationTest, FragmentInputArrayOfStructs)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "    vec4 foo;\n"
        "};\n"
        "in S i[2];\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = i[0].foo;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Vertex shader inputs can't be arrays (ESSL 3.00 section 4.3.4)
// This test is testing the case where the array brackets are after the variable name, so
// the arrayness isn't known when the type and qualifiers are initially parsed.
TEST_F(VertexShaderValidationTest, VertexShaderInputArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 i[2];\n"
        "void main() {\n"
        "    gl_Position = i[0];\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Vertex shader inputs can't be arrays (ESSL 3.00 section 4.3.4)
// This test is testing the case where the array brackets are after the type.
TEST_F(VertexShaderValidationTest, VertexShaderInputArrayType)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4[2] i;\n"
        "void main() {\n"
        "    gl_Position = i[0];\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Fragment shader inputs can't contain booleans (ESSL 3.00 section 4.3.4)
TEST_F(FragmentShaderValidationTest, FragmentShaderInputStructWithBool)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "    bool foo;\n"
        "};\n"
        "in S s;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Fragment shader inputs without a flat qualifier can't contain integers (ESSL 3.00 section 4.3.4)
TEST_F(FragmentShaderValidationTest, FragmentShaderInputStructWithInt)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "    int foo;\n"
        "};\n"
        "in S s;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Selecting a field of a vector that's the result of dynamic indexing a constant array should work.
TEST_F(FragmentShaderValidationTest, ShaderSelectingFieldOfVectorIndexedFromArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform int i;\n"
        "void main() {\n"
        "    float f = vec2[1](vec2(0.0, 0.1))[i].x;\n"
        "    my_FragColor = vec4(f);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Passing an array into a function and then passing a value from that array into another function
// should work. This is a regression test for a bug where the mangled name of a TType was not
// properly updated when determining the type resulting from array indexing.
TEST_F(FragmentShaderValidationTest, ArrayValueFromFunctionParameterAsParameter)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "float foo(float f) {\n"
        "   return f * 2.0;\n"
        "}\n"
        "float bar(float[2] f) {\n"
        "    return foo(f[0]);\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    float arr[2];\n"
        "    arr[0] = u;\n"
        "    gl_FragColor = vec4(bar(arr));\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that out-of-range integer literal generates an error in ESSL 3.00.
TEST_F(FragmentShaderValidationTest, OutOfRangeIntegerLiteral)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision highp int;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(0x100000000);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that vector field selection from a value taken from an array constructor is accepted as a
// constant expression.
TEST_F(FragmentShaderValidationTest, FieldSelectionFromVectorArrayConstructorIsConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const float f = vec2[1](vec2(0.0, 1.0))[0].x;\n"
        "    my_FragColor = vec4(f);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that structure field selection from a value taken from an array constructor is accepted as a
// constant expression.
TEST_F(FragmentShaderValidationTest, FieldSelectionFromStructArrayConstructorIsConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct S { float member; };\n"
        "void main()\n"
        "{\n"
        "    const float f = S[1](S(0.0))[0].member;\n"
        "    my_FragColor = vec4(f);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a reference to a const array is accepted as a constant expression.
TEST_F(FragmentShaderValidationTest, ArraySymbolIsConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const float[2] arr = float[2](0.0, 1.0);\n"
        "    const float f = arr[0];\n"
        "    my_FragColor = vec4(f);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that using an array constructor in a parameter to a built-in function is accepted as a
// constant expression.
TEST_F(FragmentShaderValidationTest, BuiltInFunctionAppliedToArrayConstructorIsConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const float f = sin(float[2](0.0, 1.0)[0]);\n"
        "    my_FragColor = vec4(f);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that using an array constructor in a parameter to a built-in function is accepted as a
// constant expression.
TEST_F(FragmentShaderValidationTest,
       BuiltInFunctionWithMultipleParametersAppliedToArrayConstructorIsConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const float f = pow(1.0, float[2](0.0, 1.0)[0]);\n"
        "    my_FragColor = vec4(f);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that using an array constructor in a parameter to a constructor is accepted as a constant
// expression.
TEST_F(FragmentShaderValidationTest,
       ConstructorWithMultipleParametersAppliedToArrayConstructorIsConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const vec2 f = vec2(1.0, float[2](0.0, 1.0)[0]);\n"
        "    my_FragColor = vec4(f.x);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that using an array constructor in an operand of the ternary selection operator is accepted
// as a constant expression.
TEST_F(FragmentShaderValidationTest, TernaryOperatorAppliedToArrayConstructorIsConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const float f = true ? float[2](0.0, 1.0)[0] : 1.0;\n"
        "    my_FragColor = vec4(f);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a ternary operator with one unevaluated non-constant operand is not a constant
// expression.
TEST_F(FragmentShaderValidationTest, TernaryOperatorNonConstantOperand)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main()\n"
        "{\n"
        "    const float f = true ? 1.0 : u;\n"
        "    gl_FragColor = vec4(f);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a sampler can't be used in constructor argument list
TEST_F(FragmentShaderValidationTest, SamplerInConstructorArguments)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform sampler2D s;\n"
        "void main()\n"
        "{\n"
        "    vec2 v = vec2(0.0, s);\n"
        "    gl_FragColor = vec4(v, 0.0, 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that void can't be used in constructor argument list
TEST_F(FragmentShaderValidationTest, VoidInConstructorArguments)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void foo() {}\n"
        "void main()\n"
        "{\n"
        "    vec2 v = vec2(0.0, foo());\n"
        "    gl_FragColor = vec4(v, 0.0, 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a shader passing a struct into a constructor of array of structs with 1 element works.
TEST_F(FragmentShaderValidationTest, SingleStructArrayConstructor)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform float u;\n"
        "struct S { float member; };\n"
        "void main()\n"
        "{\n"
        "    S[1] sarr = S[1](S(u));\n"
        "    my_FragColor = vec4(sarr[0].member);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a shader with empty constructor parameter list is not accepted.
TEST_F(FragmentShaderValidationTest, EmptyArrayConstructor)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform float u;\n"
        "const float[] f = f[]();\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that indexing fragment outputs with a non-constant expression is forbidden, even if ANGLE
// is able to constant fold the index expression. ESSL 3.00 section 4.3.6.
TEST_F(FragmentShaderValidationTest, DynamicallyIndexedFragmentOutput)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform int a;\n"
        "out vec4[2] my_FragData;\n"
        "void main()\n"
        "{\n"
        "    my_FragData[true ? 0 : a] = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that indexing a uniform buffer array with a non-constant expression is forbidden, even if
// ANGLE is able to constant fold the index expression. ESSL 3.00 section 4.3.7.
TEST_F(FragmentShaderValidationTest, DynamicallyIndexedUniformBuffer)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform int a;
        uniform B
        {
            vec4 f;
        }
        blocks[2];
        out vec4 my_FragColor;
        void main()
        {
            my_FragColor = blocks[true ? 0 : a].f;
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that indexing a storage buffer array with a non-constant expression is forbidden, even if
// ANGLE is able to constant fold the index expression. ESSL 3.10 section 4.3.9.
TEST_F(FragmentShaderValidationTest, DynamicallyIndexedStorageBuffer)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        uniform int a;
        layout(std140) buffer B
        {
            vec4 f;
        }
        blocks[2];
        out vec4 my_FragColor;
        void main()
        {
            my_FragColor = blocks[true ? 0 : a].f;
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that indexing a sampler array with a non-constant expression is forbidden, even if ANGLE is
// able to constant fold the index expression. ESSL 3.00 section 4.1.7.1.
TEST_F(FragmentShaderValidationTest, DynamicallyIndexedSampler)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform int a;
        uniform sampler2D s[2];
        out vec4 my_FragColor;
        void main()
        {
            my_FragColor = texture(s[true ? 0 : a], vec2(0));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that indexing an image array with a non-constant expression is forbidden, even if ANGLE is
// able to constant fold the index expression. ESSL 3.10 section 4.1.7.2.
TEST_F(FragmentShaderValidationTest, DynamicallyIndexedImage)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        uniform int a;
        layout(rgba32f) uniform highp readonly image2D image[2];
        out vec4 my_FragColor;
        void main()
        {
            my_FragColor = imageLoad(image[true ? 0 : a], ivec2(0));
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a shader that uses a struct definition in place of a struct constructor does not
// compile. See GLSL ES 1.00 section 5.4.3.
TEST_F(FragmentShaderValidationTest, StructConstructorWithStructDefinition)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    struct s { float f; } (0.0);\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that indexing gl_FragData with a non-constant expression is forbidden in WebGL 2.0, even
// when ANGLE is able to constant fold the index.
// WebGL 2.0 spec section 'GLSL ES 1.00 Fragment Shader Output'
TEST_F(WebGL2FragmentShaderValidationTest, IndexFragDataWithNonConstant)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    for (int i = 0; i < 2; ++i) {\n"
        "        gl_FragData[true ? 0 : i] = vec4(0.0);\n"
        "    }\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an uniform should generate a warning
// (we don't generate an error on ESSL 1.00 because of WebGL compatibility)
TEST_F(WebGL2FragmentShaderValidationTest, AssignUniformToGlobalESSL1)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float a;\n"
        "float b = a * 2.0;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        if (!hasWarning())
        {
            FAIL() << "Shader compilation succeeded without warnings, expecting warning:\n"
                   << mInfoLog;
        }
    }
    else
    {
        FAIL() << "Shader compilation failed, expecting success with warning:\n" << mInfoLog;
    }
}

// Test that deferring global variable init works with an empty main().
TEST_F(WebGL2FragmentShaderValidationTest, DeferGlobalVariableInitWithEmptyMain)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "float foo = u;\n"
        "void main() {}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a non-constant texture offset is not accepted for textureOffset.
// ESSL 3.00 section 8.8
TEST_F(FragmentShaderValidationTest, TextureOffsetNonConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform vec3 u_texCoord;\n"
        "uniform mediump sampler3D u_sampler;\n"
        "uniform int x;\n"
        "void main()\n"
        "{\n"
        "   my_FragColor = textureOffset(u_sampler, u_texCoord, ivec3(x, 3, -8));\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a non-constant texture offset is not accepted for textureProjOffset with bias.
// ESSL 3.00 section 8.8
TEST_F(FragmentShaderValidationTest, TextureProjOffsetNonConst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform vec4 u_texCoord;\n"
        "uniform mediump sampler3D u_sampler;\n"
        "uniform int x;\n"
        "void main()\n"
        "{\n"
        "   my_FragColor = textureProjOffset(u_sampler, u_texCoord, ivec3(x, 3, -8), 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that an out-of-range texture offset is not accepted.
// GLES 3.0.4 section 3.8.10 specifies that out-of-range offset has undefined behavior.
TEST_F(FragmentShaderValidationTest, TextureLodOffsetOutOfRange)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform vec3 u_texCoord;\n"
        "uniform mediump sampler3D u_sampler;\n"
        "void main()\n"
        "{\n"
        "   my_FragColor = textureLodOffset(u_sampler, u_texCoord, 0.0, ivec3(0, 0, 8));\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that default precision qualifier for uint is not accepted.
// ESSL 3.00.4 section 4.5.4: Only allowed for float, int and sampler types.
TEST_F(FragmentShaderValidationTest, DefaultPrecisionUint)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision mediump uint;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "   my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that sampler3D needs to be precision qualified.
// ESSL 3.00.4 section 4.5.4: New ESSL 3.00 sampler types don't have predefined precision.
TEST_F(FragmentShaderValidationTest, NoPrecisionSampler3D)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler3D s;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "   my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using a non-constant expression in a for loop initializer is forbidden in WebGL 1.0,
// even when ANGLE is able to constant fold the initializer.
// ESSL 1.00 Appendix A.
TEST_F(WebGL1FragmentShaderValidationTest, NonConstantLoopIndex)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform int u;\n"
        "void main()\n"
        "{\n"
        "    for (int i = (true ? 1 : u); i < 5; ++i) {\n"
        "        gl_FragColor = vec4(0.0);\n"
        "    }\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an uniform should generate a warning
// (we don't generate an error on ESSL 1.00 because of WebGL compatibility)
TEST_F(WebGL1FragmentShaderValidationTest, AssignUniformToGlobalESSL1)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float a;\n"
        "float b = a * 2.0;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        if (!hasWarning())
        {
            FAIL() << "Shader compilation succeeded without warnings, expecting warning:\n"
                   << mInfoLog;
        }
    }
    else
    {
        FAIL() << "Shader compilation failed, expecting success with warning:\n" << mInfoLog;
    }
}

// Test that deferring global variable init works with an empty main().
TEST_F(WebGL1FragmentShaderValidationTest, DeferGlobalVariableInitWithEmptyMain)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "float foo = u;\n"
        "void main() {}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Check that indices that are not integers are rejected.
// The check should be done even if ESSL 1.00 Appendix A limitations are not applied.
TEST_F(FragmentShaderValidationTest, NonIntegerIndex)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    float f[3];\n"
        "    const float i = 2.0;\n"
        "    gl_FragColor = vec4(f[i]);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// ESSL1 shaders with a duplicate function prototype should be rejected.
// ESSL 1.00.17 section 4.2.7.
TEST_F(FragmentShaderValidationTest, DuplicatePrototypeESSL1)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void foo();\n"
        "void foo();\n"
        "void foo() {}\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// ESSL3 shaders with a duplicate function prototype should be allowed.
// ESSL 3.00.4 section 4.2.3.
TEST_F(FragmentShaderValidationTest, DuplicatePrototypeESSL3)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void foo();\n"
        "void foo();\n"
        "void foo() {}\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Shaders with a local function prototype should be rejected.
// ESSL 3.00.4 section 4.2.4.
TEST_F(FragmentShaderValidationTest, LocalFunctionPrototype)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    void foo();\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// ESSL 3.00 fragment shaders can not use #pragma STDGL invariant(all).
// ESSL 3.00.4 section 4.6.1. Does not apply to other versions of ESSL.
TEST_F(FragmentShaderValidationTest, ESSL300FragmentInvariantAll)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#pragma STDGL invariant(all)\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Built-in functions can be overloaded in ESSL 1.00.
TEST_F(FragmentShaderValidationTest, ESSL100BuiltInFunctionOverload)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "int sin(int x)\n"
        "{\n"
        "    return int(sin(float(x)));\n"
        "}\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = vec4(sin(1));"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Built-in functions can not be overloaded in ESSL 3.00.
TEST_F(FragmentShaderValidationTest, ESSL300BuiltInFunctionOverload)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "int sin(int x)\n"
        "{\n"
        "    return int(sin(float(x)));\n"
        "}\n"
        "void main()\n"
        "{\n"
        "   my_FragColor = vec4(sin(1));"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Multiplying a 4x2 matrix with a 4x2 matrix should not work.
TEST_F(FragmentShaderValidationTest, CompoundMultiplyMatrixIdenticalNonSquareDimensions)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "   mat4x2 foo;\n"
        "   foo *= mat4x2(4.0);\n"
        "   my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Multiplying a matrix with 2 columns and 4 rows with a 2x2 matrix should work.
TEST_F(FragmentShaderValidationTest, CompoundMultiplyMatrixValidNonSquareDimensions)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "   mat2x4 foo;\n"
        "   foo *= mat2x2(4.0);\n"
        "   my_FragColor = vec4(0.0);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Covers a bug where we would set the incorrect result size on an out-of-bounds vector swizzle.
TEST_F(FragmentShaderValidationTest, OutOfBoundsVectorSwizzle)
{
    const std::string &shaderString =
        "void main() {\n"
        "   vec2(0).qq;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Covers a bug where strange preprocessor defines could trigger asserts.
TEST_F(FragmentShaderValidationTest, DefineWithSemicolon)
{
    const std::string &shaderString =
        "#define Def; highp\n"
        "uniform Def vec2 a;\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Covers a bug in our parsing of malformed shift preprocessor expressions.
TEST_F(FragmentShaderValidationTest, LineDirectiveUndefinedShift)
{
    const std::string &shaderString = "#line x << y";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Covers a bug in our parsing of malformed shift preprocessor expressions.
TEST_F(FragmentShaderValidationTest, LineDirectiveNegativeShift)
{
    const std::string &shaderString = "#line x << -1";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// gl_MaxImageUnits is only available in ES 3.1 shaders.
TEST_F(FragmentShaderValidationTest, MaxImageUnitsInES3Shader)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 myOutput;"
        "void main() {\n"
        "   float ff = float(gl_MaxImageUnits);\n"
        "   myOutput = vec4(ff);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// struct += struct is an invalid operation.
TEST_F(FragmentShaderValidationTest, StructCompoundAssignStruct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 myOutput;\n"
        "struct S { float foo; };\n"
        "void main() {\n"
        "   S a, b;\n"
        "   a += b;\n"
        "   myOutput = vec4(0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// struct == different struct is an invalid operation.
TEST_F(FragmentShaderValidationTest, StructEqDifferentStruct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 myOutput;\n"
        "struct S { float foo; };\n"
        "struct S2 { float foobar; };\n"
        "void main() {\n"
        "   S a;\n"
        "   S2 b;\n"
        "   a == b;\n"
        "   myOutput = vec4(0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Compute shaders are not supported in versions lower than 310.
TEST_F(ComputeShaderValidationTest, Version100)
{
    const std::string &shaderString =
        R"(void main()
        {
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Compute shaders are not supported in versions lower than 310.
TEST_F(ComputeShaderValidationTest, Version300)
{
    const std::string &shaderString =
        R"(#version 300 es
        void main()
        {
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Compute shaders should have work group size specified. However, it is not a compile time error
// to not have the size specified, but rather a link time one.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, NoWorkGroupSizeSpecified)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that workgroup size declaration doesn't accept variable declaration.
TEST_F(ComputeShaderValidationTest, NoVariableDeclrationAfterWorkGroupSize)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        layout(local_size_x = 1) in vec4 x;
        void main()
        {
        })";
    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Work group size is less than 1. It should be at least 1.
// GLSL ES 3.10 Revision 4, 7.1.3 Compute Shader Special Variables
// The spec is not clear whether having a local size qualifier equal zero
// is correct.
// TODO (mradev): Ask people from Khronos to clarify the spec.
TEST_F(ComputeShaderValidationTest, WorkGroupSizeTooSmallXdimension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 0) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Work group size is correct for the x and y dimensions, but not for the z dimension.
// GLSL ES 3.10 Revision 4, 7.1.3 Compute Shader Special Variables
TEST_F(ComputeShaderValidationTest, WorkGroupSizeTooSmallZDimension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 4, local_size_y = 6, local_size_z = 0) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Work group size is bigger than the minimum in the x dimension.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, WorkGroupSizeTooBigXDimension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 9989899) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Work group size is bigger than the minimum in the y dimension.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, WorkGroupSizeTooBigYDimension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, local_size_y = 9989899) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Work group size is definitely bigger than the minimum in the z dimension.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, WorkGroupSizeTooBigZDimension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, local_size_y = 5, local_size_z = 9989899) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Work group size specified through macro expansion.
TEST_F(ComputeShaderValidationTest, WorkGroupSizeMacro)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#define MYDEF(x) x"
        "layout(local_size_x = MYDEF(127)) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Work group size specified as an unsigned integer.
TEST_F(ComputeShaderValidationTest, WorkGroupSizeUnsignedInteger)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 123u) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Work group size specified in hexadecimal.
TEST_F(ComputeShaderValidationTest, WorkGroupSizeHexadecimal)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 0x3A) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// local_size_x is -1 in hexadecimal format.
// -1 is used as unspecified value in the TLayoutQualifier structure.
TEST_F(ComputeShaderValidationTest, WorkGroupSizeMinusOneHexadecimal)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 0xFFFFFFFF) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Work group size specified in octal.
TEST_F(ComputeShaderValidationTest, WorkGroupSizeOctal)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 013) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Work group size is negative. It is specified in hexadecimal.
TEST_F(ComputeShaderValidationTest, WorkGroupSizeNegativeHexadecimal)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 0xFFFFFFEC) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Multiple work group layout qualifiers with differing values.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, DifferingLayoutQualifiers)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, local_size_x = 6) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Multiple work group input variables with differing local size values.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, MultipleInputVariablesDifferingLocalSize)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, local_size_y = 6) in;\n"
        "layout(local_size_x = 5, local_size_y = 7) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Multiple work group input variables with differing local size values.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, MultipleInputVariablesDifferingLocalSize2)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5) in;\n"
        "layout(local_size_x = 5, local_size_y = 7) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Multiple work group input variables with the same local size values. It should compile.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, MultipleInputVariablesSameLocalSize)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, local_size_y = 6) in;\n"
        "layout(local_size_x = 5, local_size_y = 6) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Multiple work group input variables with the same local size values. It should compile.
// Since the default value is 1, it should compile.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, MultipleInputVariablesSameLocalSize2)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5) in;\n"
        "layout(local_size_x = 5, local_size_y = 1) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Multiple work group input variables with the same local size values. It should compile.
// Since the default value is 1, it should compile.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, MultipleInputVariablesSameLocalSize3)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, local_size_y = 1) in;\n"
        "layout(local_size_x = 5) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Specifying row_major qualifier in a work group size layout.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, RowMajorInComputeInputLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, row_major) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// local size layout can be used only with compute input variables
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, UniformComputeInputLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5) uniform;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// local size layout can be used only with compute input variables
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, UniformBufferComputeInputLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5) uniform SomeBuffer { vec4 something; };\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// local size layout can be used only with compute input variables
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, StructComputeInputLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5) struct SomeBuffer { vec4 something; };\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// local size layout can be used only with compute input variables
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, StructBodyComputeInputLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "struct S {\n"
        "   layout(local_size_x = 12) vec4 foo;\n"
        "};\n"
        "void main()"
        "{"
        "}";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// local size layout can be used only with compute input variables
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, TypeComputeInputLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5) vec4;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid use of the out storage qualifier in a compute shader.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, InvalidOutStorageQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 15) in;\n"
        "out vec4 myOutput;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid use of the out storage qualifier in a compute shader.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, InvalidOutStorageQualifier2)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 15) in;\n"
        "out myOutput;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid use of the in storage qualifier. Can be only used to describe the local block size.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, InvalidInStorageQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 15) in;\n"
        "in vec4 myInput;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid use of the in storage qualifier. Can be only used to describe the local block size.
// The test checks a different part of the GLSL grammar than what InvalidInStorageQualifier checks.
// GLSL ES 3.10 Revision 4, 4.4.1.1 Compute Shader Inputs
TEST_F(ComputeShaderValidationTest, InvalidInStorageQualifier2)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 15) in;\n"
        "in myInput;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The local_size layout qualifier is only available in compute shaders.
TEST_F(VertexShaderValidationTest, InvalidUseOfLocalSizeX)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 15) in vec4 myInput;\n"
        "out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The local_size layout qualifier is only available in compute shaders.
TEST_F(FragmentShaderValidationTest, InvalidUseOfLocalSizeX)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 15) in vec4 myInput;\n"
        "out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The local_size layout qualifier is only available in compute shaders.
TEST_F(GeometryShaderValidationTest, InvalidUseOfLocalSizeX)
{
    const std::string &shaderString1 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points, local_size_x = 15) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString2 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (invocations = 2, local_size_x = 15) in;
        layout (points, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString3 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points, local_size_x = 15, max_vertices = 2) out;
        void main()
        {
        })";

    const std::string &shaderString4 =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 2, local_size_x = 15) out;
        void main()
        {
        })";
    if (compile(shaderString1) || compile(shaderString2) || compile(shaderString3) ||
        compile(shaderString4))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is a compile time error to use the gl_WorkGroupSize constant if
// the local size has not been declared yet.
// GLSL ES 3.10 Revision 4, 7.1.3 Compute Shader Special Variables
TEST_F(ComputeShaderValidationTest, InvalidUsageOfWorkGroupSize)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "void main()\n"
        "{\n"
        "   uvec3 WorkGroupSize = gl_WorkGroupSize;\n"
        "}\n"
        "layout(local_size_x = 12) in;\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The test covers the compute shader built-in variables and constants.
TEST_F(ComputeShaderValidationTest, CorrectUsageOfComputeBuiltins)
{
    const std::string &shaderString =
        R"(#version 310 es
        layout(local_size_x=4, local_size_y=3, local_size_z=2) in;
        layout(rgba32ui) uniform highp writeonly uimage2D imageOut;
        void main()
        {
            uvec3 temp1 = gl_NumWorkGroups;
            uvec3 temp2 = gl_WorkGroupSize;
            uvec3 temp3 = gl_WorkGroupID;
            uvec3 temp4 = gl_LocalInvocationID;
            uvec3 temp5 = gl_GlobalInvocationID;
            uint  temp6 = gl_LocalInvocationIndex;
            imageStore(imageOut, ivec2(0), uvec4(temp1 + temp2 + temp3 + temp4 + temp5, temp6));
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// It is illegal to write to a special variable.
TEST_F(ComputeShaderValidationTest, SpecialVariableNumWorkGroups)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   gl_NumWorkGroups = uvec3(1); \n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is illegal to write to a special variable.
TEST_F(ComputeShaderValidationTest, SpecialVariableWorkGroupID)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   gl_WorkGroupID = uvec3(1); \n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is illegal to write to a special variable.
TEST_F(ComputeShaderValidationTest, SpecialVariableLocalInvocationID)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   gl_LocalInvocationID = uvec3(1); \n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is illegal to write to a special variable.
TEST_F(ComputeShaderValidationTest, SpecialVariableGlobalInvocationID)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   gl_GlobalInvocationID = uvec3(1); \n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is illegal to write to a special variable.
TEST_F(ComputeShaderValidationTest, SpecialVariableLocalInvocationIndex)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   gl_LocalInvocationIndex = 1; \n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is illegal to write to a special variable.
TEST_F(ComputeShaderValidationTest, SpecialVariableWorkGroupSize)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   gl_WorkGroupSize = uvec3(1); \n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is illegal to apply an unary operator to a sampler.
TEST_F(FragmentShaderValidationTest, SamplerUnaryOperator)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform sampler2D s;\n"
        "void main()\n"
        "{\n"
        "   -s;\n"
        "   gl_FragColor = vec4(0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invariant cannot be used with a work group size declaration.
TEST_F(ComputeShaderValidationTest, InvariantBlockSize)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "invariant layout(local_size_x = 15) in;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invariant cannot be used with a non-output variable in ESSL3.
TEST_F(FragmentShaderValidationTest, InvariantNonOuput)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "invariant int value;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invariant cannot be used with a non-output variable in ESSL3.
// ESSL 3.00.6 section 4.8: This applies even if the declaration is empty.
TEST_F(FragmentShaderValidationTest, InvariantNonOuputEmptyDeclaration)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "invariant in float;\n"
        "void main() {}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invariant declaration should follow the following format "invariant <out variable name>".
// Test having an incorrect qualifier in the invariant declaration.
TEST_F(FragmentShaderValidationTest, InvariantDeclarationWithStorageQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 foo;\n"
        "invariant centroid foo;\n"
        "void main() {\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invariant declaration should follow the following format "invariant <out variable name>".
// Test having an incorrect precision qualifier in the invariant declaration.
TEST_F(FragmentShaderValidationTest, InvariantDeclarationWithPrecisionQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 foo;\n"
        "invariant highp foo;\n"
        "void main() {\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invariant declaration should follow the following format "invariant <out variable name>".
// Test having an incorrect layout qualifier in the invariant declaration.
TEST_F(FragmentShaderValidationTest, InvariantDeclarationWithLayoutQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 foo;\n"
        "invariant layout(location=0) foo;\n"
        "void main() {\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Variable declaration with both invariant and layout qualifiers is not valid in the formal grammar
// provided in the ESSL 3.00 spec. ESSL 3.10 starts allowing this combination, but ESSL 3.00 should
// still disallow it.
TEST_F(FragmentShaderValidationTest, VariableDeclarationWithInvariantAndLayoutQualifierESSL300)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "invariant layout(location = 0) out vec4 my_FragColor;\n"
        "void main() {\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Bit shift with a rhs value > 31 has an undefined result in the GLSL spec. Detecting an undefined
// result at compile time should not generate an error either way.
// ESSL 3.00.6 section 5.9.
TEST_F(FragmentShaderValidationTest, ShiftBy32)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        out uint my_out;
        void main() {
           my_out = 1u << 32u;
        })";
    if (compile(shaderString))
    {
        if (!hasWarning())
        {
            FAIL() << "Shader compilation succeeded without warnings, expecting warning:\n"
                   << mInfoLog;
        }
    }
    else
    {
        FAIL() << "Shader compilation failed, expecting success with warning:\n" << mInfoLog;
    }
}

// Bit shift with a rhs value < 0 has an undefined result in the GLSL spec. Detecting an undefined
// result at compile time should not generate an error either way.
// ESSL 3.00.6 section 5.9.
TEST_F(FragmentShaderValidationTest, ShiftByNegative)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        out uint my_out;
        void main() {
           my_out = 1u << (-1);
        })";
    if (compile(shaderString))
    {
        if (!hasWarning())
        {
            FAIL() << "Shader compilation succeeded without warnings, expecting warning:\n"
                   << mInfoLog;
        }
    }
    else
    {
        FAIL() << "Shader compilation failed, expecting success with warning:\n" << mInfoLog;
    }
}

// Test that pruning empty declarations from loop init expression works.
TEST_F(FragmentShaderValidationTest, EmptyDeclarationAsLoopInit)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i = 0;\n"
        "    for (int; i < 3; i++)\n"
        "    {\n"
        "        my_FragColor = vec4(i);\n"
        "    }\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}
// r32f, r32i, r32ui do not require either the writeonly or readonly memory qualifiers.
// GLSL ES 3.10, Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, ImageR32FNoMemoryQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "in vec4 myInput;\n"
        "layout(r32f) uniform image2D myImage;\n"
        "void main() {\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Images which do not have r32f, r32i or r32ui as internal format, must have readonly or writeonly
// specified.
// GLSL ES 3.10, Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, ImageRGBA32FWithIncorrectMemoryQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "in vec4 myInput;\n"
        "layout(rgba32f) uniform image2D myImage;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is a compile-time error to call imageStore when the image is qualified as readonly.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, StoreInReadOnlyImage)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "in vec4 myInput;\n"
        "layout(r32f) uniform readonly image2D myImage;\n"
        "void main() {\n"
        "   imageStore(myImage, ivec2(0), vec4(1.0));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is a compile-time error to call imageLoad when the image is qualified as writeonly.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, LoadFromWriteOnlyImage)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "in vec4 myInput;\n"
        "layout(r32f) uniform writeonly image2D myImage;\n"
        "void main() {\n"
        "   imageLoad(myImage, ivec2(0));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is a compile-time error to call imageStore when the image is qualified as readonly.
// Test to make sure this is validated correctly for images in arrays.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, StoreInReadOnlyImageArray)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "in vec4 myInput;\n"
        "layout(r32f) uniform readonly image2D myImage[2];\n"
        "void main() {\n"
        "   imageStore(myImage[0], ivec2(0), vec4(1.0));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It is a compile-time error to call imageStore when the image is qualified as readonly.
// Test to make sure that checking this doesn't crash when validating an image in a struct.
// Image in a struct in itself isn't accepted by the parser, but error recovery still results in
// an image in the struct.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, StoreInReadOnlyImageInStruct)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "in vec4 myInput;\n"
        "uniform struct S {\n"
        "    layout(r32f) readonly image2D myImage;\n"
        "} s;\n"
        "void main() {\n"
        "   imageStore(s.myImage, ivec2(0), vec4(1.0));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// A valid declaration and usage of an image3D.
TEST_F(FragmentShaderValidationTest, ValidImage3D)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image3D;\n"
        "in vec4 myInput;\n"
        "layout(rgba32f) uniform readonly image3D myImage;\n"
        "void main() {\n"
        "   imageLoad(myImage, ivec3(0));\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// A valid declaration and usage of an imageCube.
TEST_F(FragmentShaderValidationTest, ValidImageCube)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump imageCube;\n"
        "in vec4 myInput;\n"
        "layout(rgba32f) uniform readonly imageCube myImage;\n"
        "void main() {\n"
        "   imageLoad(myImage, ivec3(0));\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// A valid declaration and usage of an image2DArray.
TEST_F(FragmentShaderValidationTest, ValidImage2DArray)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2DArray;\n"
        "in vec4 myInput;\n"
        "layout(rgba32f) uniform readonly image2DArray myImage;\n"
        "void main() {\n"
        "   imageLoad(myImage, ivec3(0));\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Images cannot be l-values.
// GLSL ES 3.10 Revision 4, 4.1.7 Opaque Types
TEST_F(FragmentShaderValidationTest, ImageLValueFunctionDefinitionInOut)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "void myFunc(inout image2D someImage) {}\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Cannot assign to images.
// GLSL ES 3.10 Revision 4, 4.1.7 Opaque Types
TEST_F(FragmentShaderValidationTest, ImageAssignment)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(rgba32f) uniform readonly image2D myImage;\n"
        "layout(rgba32f) uniform readonly image2D myImage2;\n"
        "void main() {\n"
        "   myImage = myImage2;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Passing an image qualifier to a function should not be able to discard the readonly qualifier.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, ReadOnlyQualifierMissingInFunctionArgument)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(rgba32f) uniform readonly image2D myImage;\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Passing an image qualifier to a function should not be able to discard the readonly qualifier.
// Test with an image from an array.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, ReadOnlyQualifierMissingInFunctionArgumentArray)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(rgba32f) uniform readonly image2D myImage[2];\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage[0]);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Passing an image qualifier to a function should not be able to discard the readonly qualifier.
// Test that validation doesn't crash on this for an image in a struct.
// Image in a struct in itself isn't accepted by the parser, but error recovery still results in
// an image in the struct.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, ReadOnlyQualifierMissingInFunctionArgumentStruct)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "uniform struct S {\n"
        "    layout(r32f) readonly image2D myImage;\n"
        "} s;\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(s.myImage);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Passing an image qualifier to a function should not be able to discard the writeonly qualifier.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, WriteOnlyQualifierMissingInFunctionArgument)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(rgba32f) uniform writeonly image2D myImage;\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Passing an image parameter as an argument to another function should not be able to discard the
// writeonly qualifier.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, DiscardWriteonlyInFunctionBody)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(rgba32f) uniform writeonly image2D myImage;\n"
        "void myFunc1(in image2D someImage) {}\n"
        "void myFunc2(in writeonly image2D someImage) { myFunc1(someImage); }\n"
        "void main() {\n"
        "   myFunc2(myImage);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The memory qualifiers for the image declaration and function argument match and the test should
// pass.
TEST_F(FragmentShaderValidationTest, CorrectImageMemoryQualifierSpecified)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(r32f) uniform image2D myImage;\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// The test adds additional qualifiers to the argument in the function header.
// This is correct since no memory qualifiers are discarded upon the function call.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, CorrectImageMemoryQualifierSpecified2)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(r32f) uniform image2D myImage;\n"
        "void myFunc(in readonly writeonly image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Images are not allowed in structs.
// GLSL ES 3.10 Revision 4, 4.1.8 Structures
TEST_F(FragmentShaderValidationTest, ImageInStruct)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "struct myStruct { layout(r32f) image2D myImage; };\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Images are not allowed in interface blocks.
// GLSL ES 3.10 Revision 4, 4.3.9 Interface Blocks
TEST_F(FragmentShaderValidationTest, ImageInInterfaceBlock)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "uniform myBlock { layout(r32f) image2D myImage; };\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Readonly used with an interface block.
TEST_F(FragmentShaderValidationTest, ReadonlyWithInterfaceBlock)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "uniform readonly myBlock { float something; };\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Readonly used with an invariant.
TEST_F(FragmentShaderValidationTest, ReadonlyWithInvariant)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 something;\n"
        "invariant readonly something;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Readonly used with a member of a structure.
TEST_F(FragmentShaderValidationTest, ReadonlyWithStructMember)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 something;\n"
        "struct MyStruct { readonly float myMember; };\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It should not be possible to use an internal format layout qualifier with an interface block.
TEST_F(FragmentShaderValidationTest, ImageInternalFormatWithInterfaceBlock)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 something;\n"
        "layout(rgba32f) uniform MyStruct { float myMember; };\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It should not be possible to use an internal format layout qualifier with a uniform without a
// type.
TEST_F(FragmentShaderValidationTest, ImageInternalFormatInGlobalLayoutQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 something;\n"
        "layout(rgba32f) uniform;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// ESSL 1.00 section 4.1.7.
// Samplers are not allowed as operands for most operations. Test this for ternary operator.
TEST_F(FragmentShaderValidationTest, SamplerAsTernaryOperand)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform bool u;\n"
        "uniform sampler2D s1;\n"
        "uniform sampler2D s2;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(u ? s1 : s2, vec2(0, 0));\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// ESSL 1.00.17 section 4.5.2.
// ESSL 3.00.6 section 4.5.3.
// Precision must be specified for floats. Test this with a declaration with no qualifiers.
TEST_F(FragmentShaderValidationTest, FloatDeclarationNoQualifiersNoPrecision)
{
    const std::string &shaderString =
        "vec4 foo = vec4(0.0);\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = foo;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Precision must be specified for floats. Test this with a function argument no qualifiers.
TEST_F(FragmentShaderValidationTest, FloatDeclarationNoQualifiersNoPrecisionFunctionArg)
{
    const std::string &shaderString = R"(
int c(float x)
{
    return int(x);
}
void main()
{
    c(5.0);
})";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check compiler doesn't crash on incorrect unsized array declarations.
TEST_F(FragmentShaderValidationTest, IncorrectUnsizedArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "float foo[] = 0.0;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    foo[0] = 1.0;\n"
        "    my_FragColor = vec4(foo[0]);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check compiler doesn't crash when a bvec is on the right hand side of a logical operator.
// ESSL 3.00.6 section 5.9.
TEST_F(FragmentShaderValidationTest, LogicalOpRHSIsBVec)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "void main()\n"
        "{\n"
        "    bool b;\n"
        "    bvec3 b3;\n"
        "    b && b3;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Check compiler doesn't crash when there's an unsized array constructor with no parameters.
// ESSL 3.00.6 section 4.1.9: Array size must be greater than zero.
TEST_F(FragmentShaderValidationTest, UnsizedArrayConstructorNoParameters)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "void main()\n"
        "{\n"
        "    int[]();\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Passing an image parameter as an argument to another function should not be able to discard the
// coherent qualifier.
TEST_F(FragmentShaderValidationTest, CoherentQualifierMissingInFunctionArgument)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(r32f) uniform coherent image2D myImage;\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Passing an image parameter as an argument to another function should not be able to discard the
// volatile qualifier.
TEST_F(FragmentShaderValidationTest, VolatileQualifierMissingInFunctionArgument)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(r32f) uniform volatile image2D myImage;\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The restrict qualifier can be discarded from a function argument.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, RestrictQualifierDiscardedInFunctionArgument)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(r32f) uniform restrict image2D myImage;\n"
        "void myFunc(in image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Function image arguments can be overqualified.
// GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
TEST_F(FragmentShaderValidationTest, OverqualifyingImageParameter)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(r32f) uniform image2D myImage;\n"
        "void myFunc(in coherent volatile image2D someImage) {}\n"
        "void main() {\n"
        "   myFunc(myImage);\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that work group size can be used to size arrays.
// GLSL ES 3.10.4 section 7.1.3 Compute Shader Special Variables
TEST_F(ComputeShaderValidationTest, WorkGroupSizeAsArraySize)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, local_size_y = 3, local_size_z = 1) in;\n"
        "void main()\n"
        "{\n"
        "    int[gl_WorkGroupSize.x] a = int[5](0, 0, 0, 0, 0);\n"
        "    int[gl_WorkGroupSize.y] b = int[3](0, 0, 0);\n"
        "    int[gl_WorkGroupSize.z] c = int[1](0);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Shared memory variables cannot be used inside a vertex shader.
// GLSL ES 3.10 Revision 4, 4.3.8 Shared Variables
TEST_F(VertexShaderValidationTest, VertexShaderSharedMemory)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 i;\n"
        "shared float myShared[10];\n"
        "void main() {\n"
        "    gl_Position = i;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Shared memory variables cannot be used inside a fragment shader.
// GLSL ES 3.10 Revision 4, 4.3.8 Shared Variables
TEST_F(FragmentShaderValidationTest, FragmentShaderSharedMemory)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "shared float myShared[10];\n"
        "out vec4 color;\n"
        "void main() {\n"
        "   color = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Shared memory cannot be combined with any other storage qualifier.
TEST_F(ComputeShaderValidationTest, UniformSharedMemory)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "uniform shared float myShared[100];\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Correct usage of shared memory variables.
TEST_F(ComputeShaderValidationTest, CorrectUsageOfSharedMemory)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "shared float myShared[100];\n"
        "void main() {\n"
        "   myShared[gl_LocalInvocationID.x] = 1.0;\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Shared memory variables cannot be initialized.
// GLSL ES 3.10 Revision 4, 4.3.8 Shared Variables
TEST_F(ComputeShaderValidationTest, SharedVariableInitialization)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "shared int myShared = 0;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Local variables cannot be qualified as shared.
// GLSL ES 3.10 Revision 4, 4.3 Storage Qualifiers
TEST_F(ComputeShaderValidationTest, SharedMemoryInFunctionBody)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "void func() {\n"
        "   shared int myShared;\n"
        "}\n"
        "void main() {\n"
        "   func();\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Struct members cannot be qualified as shared.
TEST_F(ComputeShaderValidationTest, SharedMemoryInStruct)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "struct MyStruct {\n"
        "   shared int myShared;\n"
        "};\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Interface block members cannot be qualified as shared.
TEST_F(ComputeShaderValidationTest, SharedMemoryInInterfaceBlock)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "uniform Myblock {\n"
        "   shared int myShared;\n"
        "};\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The shared qualifier cannot be used with any other qualifier.
TEST_F(ComputeShaderValidationTest, SharedWithInvariant)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "invariant shared int myShared;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The shared qualifier cannot be used with any other qualifier.
TEST_F(ComputeShaderValidationTest, SharedWithMemoryQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "readonly shared int myShared;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The shared qualifier cannot be used with any other qualifier.
TEST_F(ComputeShaderValidationTest, SharedGlobalLayoutDeclaration)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 5) in;\n"
        "layout(row_major) shared mat4;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Declaring a function with the same name as a built-in from a higher ESSL version should not cause
// a redeclaration error.
TEST_F(FragmentShaderValidationTest, BuiltinESSL31FunctionDeclaredInESSL30Shader)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "void imageSize() {}\n"
        "void main() {\n"
        "   imageSize();\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Attempting to declare num_views without enabling OVR_multiview.
TEST_F(VertexShaderValidationTest, InvalidNumViews)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout (num_views = 2) in;\n"
        "void main() {\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// memoryBarrierShared is only available in a compute shader.
// GLSL ES 3.10 Revision 4, 8.15 Shader Memory Control Functions
TEST_F(FragmentShaderValidationTest, InvalidUseOfMemoryBarrierShared)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "void main() {\n"
        "    memoryBarrierShared();\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// groupMemoryBarrier is only available in a compute shader.
// GLSL ES 3.10 Revision 4, 8.15 Shader Memory Control Functions
TEST_F(FragmentShaderValidationTest, InvalidUseOfGroupMemoryBarrier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "void main() {\n"
        "    groupMemoryBarrier();\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// barrier can be used in a compute shader.
// GLSL ES 3.10 Revision 4, 8.14 Shader Invocation Control Functions
TEST_F(ComputeShaderValidationTest, ValidUseOfBarrier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 15) in;\n"
        "void main() {\n"
        "   barrier();\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success " << mInfoLog;
    }
}

// memoryBarrierImage() can be used in all GLSL ES 3.10 shaders.
// GLSL ES 3.10 Revision 4, 8.15 Shader Memory Control Functions
TEST_F(FragmentShaderValidationTest, ValidUseOfMemoryBarrierImageInFragmentShader)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision highp image2D;\n"
        "layout(r32f) uniform image2D myImage;\n"
        "void main() {\n"
        "    imageStore(myImage, ivec2(0), vec4(1.0));\n"
        "    memoryBarrierImage();\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success " << mInfoLog;
    }
}

// checks that gsampler2DMS is not supported in version lower than 310
TEST_F(FragmentShaderValidationTest, Sampler2DMSInESSL300Shader)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform highp sampler2DMS s;\n"
        "void main() {}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Declare main() with incorrect parameters.
// ESSL 3.00.6 section 6.1 Function Definitions.
TEST_F(FragmentShaderValidationTest, InvalidMainPrototypeParameters)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "void main(int a);\n"
        "void main() {}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Regression test for a crash in the empty constructor of unsized array
// of a structure with non-basic fields fields. Test with "void".
TEST_F(FragmentShaderValidationTest, VoidFieldStructUnsizedArrayEmptyConstructor)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "struct S {void a;};"
        "void main() {S s[] = S[]();}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Regression test for a crash in the empty constructor of unsized array
// of a structure with non-basic fields fields. Test with something other than "void".
TEST_F(FragmentShaderValidationTest, SamplerFieldStructUnsizedArrayEmptyConstructor)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "struct S {sampler2D a;};"
        "void main() {S s[] = S[]();}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Checks that odd array initialization syntax is an error, and does not produce
// an ASSERT failure.
TEST_F(VertexShaderValidationTest, InvalidArrayConstruction)
{
    const std::string &shaderString =
        "struct S { mediump float i; mediump int ggb; };\n"
        "void main() {\n"
        "  S s[2];\n"
        "  s = S[](s.x, 0.0);\n"
        "  gl_Position = vec4(1, 0, 0, 1);\n"
        "}";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Correct usage of image binding layout qualifier.
TEST_F(ComputeShaderValidationTest, CorrectImageBindingLayoutQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump image2D;\n"
        "layout(local_size_x = 5) in;\n"
        "layout(binding = 1, rgba32f) writeonly uniform image2D myImage;\n"
        "void main()\n"
        "{\n"
        "   imageStore(myImage, ivec2(gl_LocalInvocationID.xy), vec4(1.0));\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success " << mInfoLog;
    }
}

// Incorrect use of "binding" on a global layout qualifier.
TEST_F(ComputeShaderValidationTest, IncorrectGlobalBindingLayoutQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(local_size_x = 5, binding = 0) in;\n"
        "void main() {}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Incorrect use of "binding" on a struct field layout qualifier.
TEST_F(ComputeShaderValidationTest, IncorrectStructFieldBindingLayoutQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(local_size_x = 1) in;\n"
        "struct S\n"
        "{\n"
        "  layout(binding = 0) float f;\n"
        "};\n"
        "void main() {}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Variable binding layout qualifier is set to a negative value. 0xffffffff wraps around to -1
// according to the integer parsing rules.
TEST_F(FragmentShaderValidationTest, ImageBindingUnitNegative)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(rgba32f, binding = 0xffffffff) writeonly uniform mediump image2D myImage;\n"
        "out vec4 outFrag;\n"
        "void main()\n"
        "{\n"
        "   outFrag = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Image binding layout qualifier value is greater than the maximum image binding.
TEST_F(FragmentShaderValidationTest, ImageBindingUnitTooBig)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(rgba32f, binding = 9999) writeonly uniform mediump image2D myImage;\n"
        "out vec4 outFrag;\n"
        "void main()\n"
        "{\n"
        "   outFrag = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Uniform variable binding is set on a non-opaque type.
TEST_F(FragmentShaderValidationTest, NonOpaqueUniformBinding)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(binding = 0) uniform float myFloat;\n"
        "out vec4 outFrag;\n"
        "void main()\n"
        "{\n"
        "   outFrag = vec4(myFloat);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Uniform variable binding is set on a sampler type.
// ESSL 3.10 section 4.4.5 Opaque Uniform Layout Qualifiers.
TEST_F(FragmentShaderValidationTest, SamplerUniformBinding)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(binding = 0) uniform mediump sampler2D mySampler;\n"
        "out vec4 outFrag;\n"
        "void main()\n"
        "{\n"
        "   outFrag = vec4(0.0);\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success " << mInfoLog;
    }
}

// Uniform variable binding is set on a sampler type in an ESSL 3.00 shader.
// The binding layout qualifier was added in ESSL 3.10, so this is incorrect.
TEST_F(FragmentShaderValidationTest, SamplerUniformBindingESSL300)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(binding = 0) uniform mediump sampler2D mySampler;\n"
        "out vec4 outFrag;\n"
        "void main()\n"
        "{\n"
        "   outFrag = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Attempting to construct a struct containing a void array should fail without asserting.
TEST_F(FragmentShaderValidationTest, ConstructStructContainingVoidArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 outFrag;\n"
        "struct S\n"
        "{\n"
        "    void A[1];\n"
        "} s = S();\n"
        "void main()\n"
        "{\n"
        "    outFrag = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Uniforms can't have location in ESSL 3.00.
// Test this with an empty declaration (ESSL 3.00.6 section 4.8: The combinations of qualifiers that
// cause compile-time or link-time errors are the same whether or not the declaration is empty).
TEST_F(FragmentShaderValidationTest, UniformLocationEmptyDeclaration)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location=0) uniform float;\n"
        "void main() {}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test function parameters of opaque type can't be l-value too.
TEST_F(FragmentShaderValidationTest, OpaqueParameterCanNotBeLValue)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "uniform sampler2D s;\n"
        "void foo(sampler2D as) {\n"
        "    as = s;\n"
        "}\n"
        "void main() {}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test samplers must not be operands in expressions, except for array indexing, structure field
// selection and parentheses(ESSL 3.00 Secion 4.1.7).
TEST_F(FragmentShaderValidationTest, InvalidExpressionForSamplerOperands)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform sampler2D s;\n"
        "uniform sampler2D s2;\n"
        "void main() {\n"
        "    s + s2;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test interface blocks as invalid operands to a binary expression.
TEST_F(FragmentShaderValidationTest, InvalidInterfaceBlockBinaryExpression)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform U\n"
        "{\n"
        "    int foo; \n"
        "} u;\n"
        "void main()\n"
        "{\n"
        "    u + u;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test interface block as an invalid operand to an unary expression.
TEST_F(FragmentShaderValidationTest, InvalidInterfaceBlockUnaryExpression)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform U\n"
        "{\n"
        "    int foo; \n"
        "} u;\n"
        "void main()\n"
        "{\n"
        "    +u;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test interface block as an invalid operand to a ternary expression.
// Note that the spec is not very explicit on this, but it makes sense to forbid this.
TEST_F(FragmentShaderValidationTest, InvalidInterfaceBlockTernaryExpression)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "uniform U\n"
        "{\n"
        "    int foo; \n"
        "} u;\n"
        "void main()\n"
        "{\n"
        "    true ? u : u;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that "buffer" and "shared" are valid identifiers in version lower than GLSL ES 3.10.
TEST_F(FragmentShaderValidationTest, BufferAndSharedAsIdentifierOnES3)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        out vec4 my_out;
        void main()
        {
            int buffer = 1;
            int shared = 2;
            my_out = vec4(buffer + shared);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a struct can not be used as a constructor argument for a scalar.
TEST_F(FragmentShaderValidationTest, StructAsBoolConstructorArgument)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct my_struct\n"
        "{\n"
        "    float f;\n"
        "};\n"
        "my_struct a = my_struct(1.0);\n"
        "void main(void)\n"
        "{\n"
        "    bool test = bool(a);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a compute shader can be compiled with MAX_COMPUTE_UNIFORM_COMPONENTS uniform
// components.
TEST_F(ComputeShaderEnforcePackingValidationTest, MaxComputeUniformComponents)
{
    GLint uniformVectorCount = kMaxComputeUniformComponents / 4;

    std::ostringstream ostream;
    ostream << "#version 310 es\n"
               "layout(local_size_x = 1) in;\n";

    for (GLint i = 0; i < uniformVectorCount; ++i)
    {
        ostream << "uniform vec4 u_value" << i << ";\n";
    }

    ostream << "void main()\n"
               "{\n";

    for (GLint i = 0; i < uniformVectorCount; ++i)
    {
        ostream << "    vec4 v" << i << " = u_value" << i << ";\n";
    }

    ostream << "}\n";

    if (!compile(ostream.str()))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a function can't be declared with a name starting with "gl_". Note that it's important
// that the function is not being called.
TEST_F(FragmentShaderValidationTest, FunctionDeclaredWithReservedName)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void gl_();\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a function can't be defined with a name starting with "gl_". Note that it's important
// that the function is not being called.
TEST_F(FragmentShaderValidationTest, FunctionDefinedWithReservedName)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void gl_()\n"
        "{\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that ops with mismatching operand types are disallowed and don't result in an assert.
// This makes sure that constant folding doesn't fetch invalid union values in case operand types
// mismatch.
TEST_F(FragmentShaderValidationTest, InvalidOpsWithConstantOperandsDontAssert)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    float f1 = 0.5 / 2;\n"
        "    float f2 = true + 0.5;\n"
        "    float f3 = float[2](0.0, 1.0)[1.0];\n"
        "    float f4 = float[2](0.0, 1.0)[true];\n"
        "    float f5 = true ? 1.0 : 0;\n"
        "    float f6 = 1.0 ? 1.0 : 2.0;\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that case labels with invalid types don't assert
TEST_F(FragmentShaderValidationTest, CaseLabelsWithInvalidTypesDontAssert)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform int i;\n"
        "void main()\n"
        "{\n"
        "    float f = 0.0;\n"
        "    switch (i)\n"
        "    {\n"
        "        case 0u:\n"
        "            f = 0.0;\n"
        "        case true:\n"
        "            f = 1.0;\n"
        "        case 2.0:\n"
        "            f = 2.0;\n"
        "    }\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using an array as an index is not allowed.
TEST_F(FragmentShaderValidationTest, ArrayAsIndex)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    int i[2] = int[2](0, 1);\n"
        "    float f[2] = float[2](2.0, 3.0);\n"
        "    my_FragColor = vec4(f[i]);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using an array as an array size is not allowed.
TEST_F(FragmentShaderValidationTest, ArrayAsArraySize)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    const int i[2] = int[2](1, 2);\n"
        "    float f[i];\n"
        "    my_FragColor = vec4(f[0]);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The input primitive layout qualifier is only available in geometry shaders.
TEST_F(VertexShaderValidationTest, InvalidUseOfInputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(points) in vec4 myInput;\n"
        "out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The input primitive layout qualifier is only available in geometry shaders.
TEST_F(FragmentShaderValidationTest, InvalidUseOfInputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(points) in vec4 myInput;\n"
        "out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The input primitive layout qualifier is only available in geometry shaders.
TEST_F(ComputeShaderValidationTest, InvalidUseOfInputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(points, local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   uvec3 WorkGroupSize = gl_WorkGroupSize;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The output primitive layout qualifier is only available in geometry shaders.
TEST_F(VertexShaderValidationTest, InvalidUseOfOutputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 myInput;\n"
        "layout(points) out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The output primitive layout qualifier is only available in geometry shaders.
TEST_F(FragmentShaderValidationTest, InvalidUseOfOutputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 myInput;\n"
        "layout(points) out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The 'invocations' layout qualifier is only available in geometry shaders.
TEST_F(VertexShaderValidationTest, InvalidUseOfInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout (invocations = 3) in vec4 myInput;\n"
        "out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The 'invocations' layout qualifier is only available in geometry shaders.
TEST_F(FragmentShaderValidationTest, InvalidUseOfInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout (invocations = 3) in vec4 myInput;\n"
        "out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The 'invocations' layout qualifier is only available in geometry shaders.
TEST_F(ComputeShaderValidationTest, InvalidUseOfInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(invocations = 3, local_size_x = 12) in;\n"
        "void main()\n"
        "{\n"
        "   uvec3 WorkGroupSize = gl_WorkGroupSize;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The 'max_vertices' layout qualifier is only available in geometry shaders.
TEST_F(VertexShaderValidationTest, InvalidUseOfMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 myInput;\n"
        "layout(max_vertices = 3) out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The 'max_vertices' layout qualifier is only available in geometry shaders.
TEST_F(FragmentShaderValidationTest, InvalidUseOfMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in vec4 myInput;\n"
        "layout(max_vertices = 3) out vec4 myOutput;\n"
        "void main() {\n"
        "   myOutput = myInput;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using the same variable name twice in function parameters fails without crashing.
TEST_F(FragmentShaderValidationTest, RedefinedParamInFunctionHeader)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void foo(int a, float a)\n"
        "{\n"
        "    return;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using gl_ViewportIndex is not allowed in an ESSL 3.10 shader.
TEST_F(VertexShaderValidationTest, ViewportIndexInESSL310)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(gl_ViewportIndex);\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that gl_PrimitiveID is valid in fragment shader with 'GL_EXT_geometry_shader' declared.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest, PrimitiveIDWithExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        precision mediump float;
        layout(location = 0) out mediump vec4 fragColor;
        void main(void)
        {
            vec4 data = vec4(0.1, 0.2, 0.3, 0.4);
            float value = data[gl_PrimitiveID % 4];
            fragColor = vec4(value, 0, 0, 1);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that gl_PrimitiveID is invalid in fragment shader without 'GL_EXT_geometry_shader' declared.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest, PrimitiveIDWithoutExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        layout(location = 0) out mediump vec4 fragColor;
        void main(void)
        {
            vec4 data = vec4(0.1, 0.2, 0.3, 0.4);
            float value = data[gl_PrimitiveID % 4];
            fragColor = vec4(value, 0, 0, 1);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that gl_PrimitiveID cannot be l-value in fragment shader.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest, AssignValueToPrimitiveID)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        precision mediump float;
        layout(location = 0) out mediump vec4 fragColor;
        void main(void)
        {
            gl_PrimitiveID = 1;
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that gl_Layer is valid in fragment shader with 'GL_EXT_geometry_shader' declared.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest, LayerWithExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        precision mediump float;
        layout(location = 0) out mediump vec4 fragColor;
        void main(void)
        {
            vec4 data = vec4(0.1, 0.2, 0.3, 0.4);
            float value = data[gl_Layer % 4];
            fragColor = vec4(value, 0, 0, 1);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that gl_Layer is invalid in fragment shader without 'GL_EXT_geometry_shader' declared.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest, LayerWithoutExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        layout(location = 0) out mediump vec4 fragColor;
        void main(void)
        {
            vec4 data = vec4(0.1, 0.2, 0.3, 0.4);
            float value = data[gl_Layer % 4];
            fragColor = vec4(value, 0, 0, 1);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that gl_Layer cannot be l-value in fragment shader.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest, AssignValueToLayer)
{
    const std::string &shaderString =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        precision mediump float;
        layout(location = 0) out mediump vec4 fragColor;
        void main(void)
        {
            gl_Layer = 1;
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that all built-in constants defined in GL_EXT_geometry_shader can be used in fragment shader
// with 'GL_EXT_geometry_shader' declared.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest, GeometryShaderBuiltInConstants)
{
    const std::string &kShaderHeader =
        R"(#version 310 es
        #extension GL_EXT_geometry_shader : require
        precision mediump float;
        layout(location = 0) out mediump vec4 fragColor;
        void main(void)
        {
            int val = )";

    const std::array<std::string, 9> kGeometryShaderBuiltinConstants = {{
        "gl_MaxGeometryInputComponents",
        "gl_MaxGeometryOutputComponents",
        "gl_MaxGeometryImageUniforms",
        "gl_MaxGeometryTextureImageUnits",
        "gl_MaxGeometryOutputVertices",
        "gl_MaxGeometryTotalOutputComponents",
        "gl_MaxGeometryUniformComponents",
        "gl_MaxGeometryAtomicCounters",
        "gl_MaxGeometryAtomicCounterBuffers",
    }};

    const std::string &kShaderTail =
        R"(;
            fragColor = vec4(val, 0, 0, 1);
        })";

    for (const std::string &kGSBuiltinConstant : kGeometryShaderBuiltinConstants)
    {
        std::ostringstream ostream;
        ostream << kShaderHeader << kGSBuiltinConstant << kShaderTail;
        if (!compile(ostream.str()))
        {
            FAIL() << "Shader compilation failed, expecting success: \n" << mInfoLog;
        }
    }
}

// Test that any built-in constants defined in GL_EXT_geometry_shader cannot be used in fragment
// shader without 'GL_EXT_geometry_shader' declared.
TEST_F(FragmentShaderEXTGeometryShaderValidationTest,
       GeometryShaderBuiltInConstantsWithoutExtension)
{
    const std::string &kShaderHeader =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout(location = 0) out mediump vec4 fragColor;\n"
        "void main(void)\n"
        "{\n"
        "    int val = ";

    const std::array<std::string, 9> kGeometryShaderBuiltinConstants = {{
        "gl_MaxGeometryInputComponents",
        "gl_MaxGeometryOutputComponents",
        "gl_MaxGeometryImageUniforms",
        "gl_MaxGeometryTextureImageUnits",
        "gl_MaxGeometryOutputVertices",
        "gl_MaxGeometryTotalOutputComponents",
        "gl_MaxGeometryUniformComponents",
        "gl_MaxGeometryAtomicCounters",
        "gl_MaxGeometryAtomicCounterBuffers",
    }};

    const std::string &kShaderTail =
        ";\n"
        "    fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    for (const std::string &kGSBuiltinConstant : kGeometryShaderBuiltinConstants)
    {
        std::ostringstream ostream;
        ostream << kShaderHeader << kGSBuiltinConstant << kShaderTail;
        if (compile(ostream.str()))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Test that declaring and using an interface block with 'const' qualifier is not allowed.
TEST_F(VertexShaderValidationTest, InterfaceBlockUsingConstQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "const block\n"
        "{\n"
        "    vec2 value;\n"
        "} ConstBlock[2];\n"
        "void main()\n"
        "{\n"
        "    int i = 0;\n"
        "    vec2 value1 = ConstBlock[i].value;\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using shader io blocks without declaration of GL_EXT_shader_io_block is not allowed.
TEST_F(VertexShaderValidationTest, IOBlockWithoutExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        out block
        {
            vec2 value;
        } VSOutput[2];
        void main()
        {
            int i = 0;
            vec2 value1 = VSOutput[i].value;
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using shader io blocks without declaration of GL_EXT_shader_io_block is not allowed.
TEST_F(FragmentShaderValidationTest, IOBlockWithoutExtension)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        in block
        {
            vec4 i_color;
        } FSInput[2];
        out vec4 o_color;
        void main()
        {
            int i = 0;
            o_color = FSInput[i].i_color;
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a shader input with 'flat' qualifier cannot be used as l-value.
TEST_F(FragmentShaderValidationTest, AssignValueToFlatIn)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "flat in float value;\n"
        "out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "    value = 1.0;\n"
        "    o_color = vec4(1.0, 0.0, 0.0, 1.0);"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a shader input with 'smooth' qualifier cannot be used as l-value.
TEST_F(FragmentShaderValidationTest, AssignValueToSmoothIn)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "smooth in float value;\n"
        "out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "    value = 1.0;\n"
        "    o_color = vec4(1.0, 0.0, 0.0, 1.0);"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a shader input with 'centroid' qualifier cannot be used as l-value.
TEST_F(FragmentShaderValidationTest, AssignValueToCentroidIn)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "centroid in float value;\n"
        "out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "    value = 1.0;\n"
        "    o_color = vec4(1.0, 0.0, 0.0, 1.0);"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that shader compilation fails if the component argument is dynamic.
TEST_F(FragmentShaderValidationTest, DynamicComponentTextureGather)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump sampler2D;\n"
        "uniform sampler2D tex;\n"
        "out vec4 o_color;\n"
        "uniform int uComp;\n"
        "void main()\n"
        "{\n"
        "    o_color = textureGather(tex, vec2(0), uComp);"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that shader compilation fails if the component argument to textureGather has a negative
// value.
TEST_F(FragmentShaderValidationTest, TextureGatherNegativeComponent)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump sampler2D;\n"
        "uniform sampler2D tex;\n"
        "out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "    o_color = textureGather(tex, vec2(0), -1);"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that shader compilation fails if the component argument to textureGather has a value greater
// than 3.
TEST_F(FragmentShaderValidationTest, TextureGatherTooGreatComponent)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump sampler2D;\n"
        "uniform sampler2D tex;\n"
        "out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "    o_color = textureGather(tex, vec2(0), 4);"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that shader compilation fails if the offset is less than the minimum value.
TEST_F(FragmentShaderValidationTest, TextureGatherTooGreatOffset)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "precision mediump sampler2D;\n"
        "uniform sampler2D tex;\n"
        "out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "    o_color = textureGatherOffset(tex, vec2(0), ivec2(-100), 2);"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that it isn't allowed to use 'location' layout qualifier on GLSL ES 3.0 vertex shader
// outputs.
TEST_F(VertexShaderValidationTest, UseLocationOnVertexOutES30)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "in vec4 v1;\n"
        "layout (location = 1) out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using 'location' layout qualifier on vertex shader outputs is legal in GLSL ES 3.1
// shaders.
TEST_F(VertexShaderValidationTest, UseLocationOnVertexOutES31)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "in vec4 v1;\n"
        "layout (location = 1) out vec4 o_color1;\n"
        "layout (location = 2) out vec4 o_color2;\n"
        "out vec3 v3;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it isn't allowed to use 'location' layout qualifier on GLSL ES 3.0 fragment shader
// inputs.
TEST_F(FragmentShaderValidationTest, UseLocationOnFragmentInES30)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout (location = 0) in vec4 v_color1;\n"
        "layout (location = 0) out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that using 'location' layout qualifier on fragment shader inputs is legal in GLSL ES 3.1
// shaders.
TEST_F(FragmentShaderValidationTest, UseLocationOnFragmentInES31)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "layout (location = 0) in mat4 v_mat;\n"
        "layout (location = 4) in vec4 v_color1;\n"
        "in vec2 v_color2;\n"
        "layout (location = 0) out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that declaring outputs of a vertex shader with same location causes a compile error.
TEST_F(VertexShaderValidationTest, DeclareSameLocationOnVertexOut)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "in float i_value;\n"
        "layout (location = 1) out vec4 o_color1;\n"
        "layout (location = 1) out vec4 o_color2;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that declaring inputs of a fragment shader with same location causes a compile error.
TEST_F(FragmentShaderValidationTest, DeclareSameLocationOnFragmentIn)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in float i_value;\n"
        "layout (location = 1) in vec4 i_color1;\n"
        "layout (location = 1) in vec4 i_color2;\n"
        "layout (location = 0) out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the location of an element of an array conflicting with other output varyings in a
// vertex shader causes a compile error.
TEST_F(VertexShaderValidationTest, LocationConflictsnOnArrayElement)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "in float i_value;\n"
        "layout (location = 0) out vec4 o_color1[3];\n"
        "layout (location = 1) out vec4 o_color2;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the location of an element of a matrix conflicting with other output varyings in a
// vertex shader causes a compile error.
TEST_F(VertexShaderValidationTest, LocationConflictsOnMatrixElement)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "in float i_value;\n"
        "layout (location = 0) out mat4 o_mvp;\n"
        "layout (location = 2) out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the location of an element of a struct conflicting with other output varyings in a
// vertex shader causes a compile error.
TEST_F(VertexShaderValidationTest, LocationConflictsOnStructElement)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "in float i_value;\n"
        "struct S\n"
        "{\n"
        "    float value1;\n"
        "    vec3 value2;\n"
        "};\n"
        "layout (location = 0) out S s_in;"
        "layout (location = 1) out vec4 o_color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that declaring inputs of a vertex shader with a location larger than GL_MAX_VERTEX_ATTRIBS
// causes a compile error.
TEST_F(VertexShaderValidationTest, AttributeLocationOutOfRange)
{
    // Assumes 1000 >= GL_MAX_VERTEX_ATTRIBS.
    // Current OpenGL and Direct3D implementations support up to 32.

    const std::string &shaderString =
        "#version 300 es\n"
        "layout (location = 1000) in float i_value;\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a block can follow the final case in a switch statement.
// GLSL ES 3.00.5 section 6 and the grammar suggest that an empty block is a statement.
TEST_F(FragmentShaderValidationTest, SwitchFinalCaseHasEmptyBlock)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision mediump float;
        uniform int i;
        void main()
        {
            switch (i)
            {
                case 0:
                    break;
                default:
                    {}
            }
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that an empty declaration can follow the final case in a switch statement.
TEST_F(FragmentShaderValidationTest, SwitchFinalCaseHasEmptyDeclaration)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision mediump float;
        uniform int i;
        void main()
        {
            switch (i)
            {
                case 0:
                    break;
                default:
                    float;
            }
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// The final case in a switch statement can't be empty in ESSL 3.10 either. This is the intent of
// the spec though public spec in early 2018 didn't reflect this yet.
TEST_F(FragmentShaderValidationTest, SwitchFinalCaseEmptyESSL310)
{
    const std::string &shaderString =
        R"(#version 310 es

        precision mediump float;
        uniform int i;
        void main()
        {
            switch (i)
            {
                case 0:
                    break;
                default:
            }
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that fragment shader cannot declare unsized inputs.
TEST_F(FragmentShaderValidationTest, UnsizedInputs)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "precision mediump float;\n"
        "in float i_value[];\n"
        "void main()\n"
        "{\n"
        "}\n";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that unsized struct members are not allowed.
TEST_F(FragmentShaderValidationTest, UnsizedStructMember)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision highp float;
        out vec4 color;

        struct S
        {
            int[] foo;
        };

        void main()
        {
            color = vec4(1.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that unsized parameters without a name are not allowed.
// GLSL ES 3.10 section 6.1 Function Definitions.
TEST_F(FragmentShaderValidationTest, UnsizedNamelessParameter)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision highp float;
        out vec4 color;

        void foo(int[]);

        void main()
        {
            color = vec4(1.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that partially unsized array of arrays constructor sizes are validated.
TEST_F(FragmentShaderValidationTest, PartiallyUnsizedArrayOfArraysConstructor)
{
    const std::string &shaderString =
        R"(#version 310 es

        precision highp float;
        out vec4 color;

        void main()
        {
            int a[][] = int[2][](int[1](1));
            color = vec4(a[0][0]);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that duplicate field names in a struct declarator list are validated.
TEST_F(FragmentShaderValidationTest, DuplicateFieldNamesInStructDeclaratorList)
{
    const std::string &shaderString =
        R"(precision mediump float;

        struct S {
            float f, f;
        };

        void main()
        {
            gl_FragColor = vec4(1.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that an empty statement is not allowed in switch before the first case.
TEST_F(FragmentShaderValidationTest, EmptyStatementInSwitchBeforeFirstCase)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision mediump float;
        uniform int u_zero;
        out vec4 my_FragColor;

        void main()
        {
            switch(u_zero)
            {
                    ;
                case 0:
                    my_FragColor = vec4(0.0);
                default:
                    my_FragColor = vec4(1.0);
            }
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a nameless struct definition is not allowed as a function parameter type.
// ESSL 3.00.6 section 12.10. ESSL 3.10 January 2016 section 13.10.
TEST_F(FragmentShaderValidationTest, NamelessStructDefinitionAsParameterType)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        float foo(struct { float field; } f)
        {
            return f.field;
        }

        void main()
        {
            my_FragColor = vec4(0, 1, 0, 1);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a named struct definition is not allowed as a function parameter type.
// ESSL 3.00.6 section 12.10. ESSL 3.10 January 2016 section 13.10.
TEST_F(FragmentShaderValidationTest, NamedStructDefinitionAsParameterType)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        float foo(struct S { float field; } f)
        {
            return f.field;
        }

        void main()
        {
            my_FragColor = vec4(0, 1, 0, 1);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a named struct definition is not allowed as a function parameter type.
// ESSL 3.00.6 section 12.10. ESSL 3.10 January 2016 section 13.10.
TEST_F(FragmentShaderValidationTest, StructDefinitionAsTypeOfParameterWithoutName)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        float foo(struct S { float field; } /* no parameter name */)
        {
            return 1.0;
        }

        void main()
        {
            my_FragColor = vec4(0, 1, 0, 1);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that an unsized const array doesn't assert.
TEST_F(FragmentShaderValidationTest, UnsizedConstArray)
{
    const std::string &shaderString =
        R"(#version 300 es

        void main()
        {
            const int t[];
            t[0];
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the value passed to the mem argument of an atomic memory function can be a shared
// variable.
TEST_F(ComputeShaderValidationTest, AtomicAddWithSharedVariable)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(local_size_x = 5) in;
        shared uint myShared;

        void main() {
            atomicAdd(myShared, 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is acceptable to pass an element of an array to the mem argument of an atomic memory
// function, as long as the underlying array is a buffer or shared variable.
TEST_F(ComputeShaderValidationTest, AtomicAddWithSharedVariableArray)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(local_size_x = 5) in;
        shared uint myShared[2];

        void main() {
            atomicAdd(myShared[0], 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is acceptable to pass a single component of a vector to the mem argument of an
// atomic memory function, as long as the underlying vector is a buffer or shared variable.
TEST_F(ComputeShaderValidationTest, AtomicAddWithSharedVariableVector)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(local_size_x = 5) in;
        shared uvec4 myShared;

        void main() {
            atomicAdd(myShared[0], 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that the value passed to the mem argument of an atomic memory function can be a buffer
// variable.
TEST_F(FragmentShaderValidationTest, AtomicAddWithBufferVariable)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(std140) buffer bufferName1{
            uint u1;
        };

        void main()
        {
            atomicAdd(u1, 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is acceptable to pass an element of an array to the mem argument of an atomic memory
// function, as long as the underlying array is a buffer or shared variable.
TEST_F(FragmentShaderValidationTest, AtomicAddWithBufferVariableArrayElement)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(std140) buffer bufferName1{
            uint u1[2];
        };

        void main()
        {
            atomicAdd(u1[0], 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is acceptable to pass a member of a shader storage block instance to the mem
// argument of an atomic memory function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithBufferVariableInBlockInstance)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(std140) buffer bufferName{
            uint u1;
        } instanceName;

        void main()
        {
            atomicAdd(instanceName.u1, 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is acceptable to pass a member of a shader storage block instance array to the mem
// argument of an atomic memory function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithBufferVariableInBlockInstanceArray)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(std140) buffer bufferName{
            uint u1;
        } instanceName[1];

        void main()
        {
            atomicAdd(instanceName[0].u1, 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is acceptable to pass an element of an array  of a shader storage block instance to
// the mem argument of an atomic memory function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithElementOfArrayInBlockInstance)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(std140) buffer blockName {
            uint data[2];
        } instanceName;

        void main()
        {
            atomicAdd(instanceName.data[0], 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is not allowed to pass an atomic counter variable to the mem argument of an atomic
// memory function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithAtomicCounter)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(binding = 0, offset = 4) uniform atomic_uint ac;

        void main()
        {
            atomicAdd(ac, 2u);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that it is not allowed to pass an element of an atomic counter array to the mem argument of
// an atomic memory function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithAtomicCounterArray)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(binding = 0, offset = 4) uniform atomic_uint ac[2];

        void main()
        {
            atomicAdd(ac[0], 2u);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that it is not allowed to pass a local uint value to the mem argument of an atomic memory
// function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithNonStorageVariable)
{
    const std::string &shaderString =
        R"(#version 310 es

        void main()
        {
            uint test = 1u;
            atomicAdd(test, 2u);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that it is acceptable to pass a swizzle of a member of a shader storage block to the mem
// argument of an atomic memory function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithSwizzle)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(std140) buffer bufferName{
            uvec4 u1[2];
        } instanceName[3];

        void main()
        {
            atomicAdd(instanceName[2].u1[1].y, 2u);
        })";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is not allowed to pass an expression that does not constitute of indexing, field
// selection or swizzle to the mem argument of an atomic memory function.
TEST_F(FragmentShaderValidationTest, AtomicAddWithNonIndexNonSwizzleExpression)
{
    const std::string &shaderString =
        R"(#version 310 es

        layout(std140) buffer bufferName{
            uint u1[2];
        } instanceName[3];

        void main()
        {
            atomicAdd(instanceName[2].u1[1] + 1u, 2u);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that negative indexing of a matrix doesn't result in an assert.
TEST_F(FragmentShaderValidationTest, MatrixNegativeIndex)
{
    const std::string &shaderString =
        R"(
        precision mediump float;

        void main()
        {
            gl_FragColor = mat4(1.0)[-1];
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions. Test with assigning a ternary
// expression that ANGLE can fold.
TEST_F(FragmentShaderValidationTest, AssignConstantFoldedFromNonConstantTernaryToGlobal)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float u;
        float f = true ? 1.0 : u;

        out vec4 my_FragColor;

        void main()
        {
           my_FragColor = vec4(f);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions. Test with assigning a ternary
// expression that ANGLE can fold.
TEST_F(FragmentShaderValidationTest,
       AssignConstantArrayVariableFoldedFromNonConstantTernaryToGlobal)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float u[2];
        const float c[2] = float[2](1.0, 2.0);
        float f[2] = true ? c : u;

        out vec4 my_FragColor;

        void main()
        {
           my_FragColor = vec4(f[0], f[1], 0.0, 1.0);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test going past the struct nesting limit while simultaneously using invalid nested struct
// definitions. This makes sure that the code generating an error message about going past the
// struct nesting limit does not access the name of a nameless struct definition.
TEST_F(WebGL1FragmentShaderValidationTest, StructNestingLimitWithNestedStructDefinitions)
{
    const std::string &shaderString =
        R"(
        precision mediump float;

        struct
        {
            struct
            {
                struct
                {
                    struct
                    {
                        struct
                        {
                            struct
                            {
                                float f;
                            } s5;
                        } s4;
                    } s3;
                } s2;
            } s1;
        } s0;

        void main(void)
        {
            gl_FragColor = vec4(s0.s1.s2.s3.s4.s5.f);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the result of a sequence operator is not a constant-expression.
// ESSL 3.00 section 12.43.
TEST_F(FragmentShaderValidationTest, CommaReturnsNonConstant)
{
    const std::string &shaderString =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        void main(void)
        {
            const int i = (0, 0);
            my_FragColor = vec4(i);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the result of indexing into an array constructor with some non-constant arguments is
// not a constant expression.
TEST_F(FragmentShaderValidationTest,
       IndexingIntoArrayConstructorWithNonConstantArgumentsIsNotConstantExpression)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision highp float;
        uniform float u;
        out float my_FragColor;
        void main()
        {
            const float f = float[2](u, 1.0)[1];
            my_FragColor = f;
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the type of an initializer of a constant variable needs to match.
TEST_F(FragmentShaderValidationTest, ConstantInitializerTypeMismatch)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        const float f = 0;

        void main()
        {
            gl_FragColor = vec4(f);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that redeclaring a built-in is an error in ESSL 1.00. ESSL 1.00.17 section 4.2.6 disallows
// "redefinition" of built-ins - it's not very explicit about redeclaring them, but we treat this as
// an error. The redeclaration cannot serve any purpose since it can't be accompanied by a
// definition.
TEST_F(FragmentShaderValidationTest, RedeclaringBuiltIn)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        float sin(float x);

        void main()
        {
            gl_FragColor = vec4(0.0);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Redefining a built-in that is not available in the current shader stage is assumed to be not an
// error. Test with redefining groupMemoryBarrier() in fragment shader. The built-in
// groupMemoryBarrier() is only available in compute shaders.
TEST_F(FragmentShaderValidationTest, RedeclaringBuiltInFromAnotherShaderStage)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        out vec4 my_FragColor;
        float groupMemoryBarrier() { return 1.0; }

        void main()
        {
            my_FragColor = vec4(groupMemoryBarrier());
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that standard derivative functions that are in core ESSL 3.00 compile successfully.
TEST_F(FragmentShaderValidationTest, ESSL300StandardDerivatives)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        in vec4 iv;
        out vec4 my_FragColor;

        void main()
        {
            vec4 v4 = vec4(0.0);
            v4 += fwidth(iv);
            v4 += dFdx(iv);
            v4 += dFdy(iv);
            my_FragColor = v4;
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that vertex shader built-in gl_Position is not accessible in fragment shader.
TEST_F(FragmentShaderValidationTest, GlPosition)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        in vec4 iv;
        out vec4 my_FragColor;

        void main()
        {
            gl_Position = iv;
            my_FragColor = iv;
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that compute shader built-in gl_LocalInvocationID is not accessible in fragment shader.
TEST_F(FragmentShaderValidationTest, GlLocalInvocationID)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        out vec3 my_FragColor;

        void main()
        {
            my_FragColor = vec3(gl_LocalInvocationID);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that fragment shader built-in gl_FragCoord is not accessible in vertex shader.
TEST_F(VertexShaderValidationTest, GlFragCoord)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        void main()
        {
            gl_Position = vec4(gl_FragCoord);
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a long sequence of repeated swizzling on an l-value does not cause a stack overflow.
TEST_F(VertexShaderValidationTest, LValueRepeatedSwizzle)
{
    std::stringstream shaderString;
    shaderString << R"(#version 300 es
        precision mediump float;

        uniform vec2 u;

        void main()
        {
            vec2 f;
            f)";
    for (int i = 0; i < 1000; ++i)
    {
        shaderString << ".yx.yx";
    }
    shaderString << R"( = vec2(0.0);
        })";

    if (!compile(shaderString.str()))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that swizzling that contains duplicate components can't form an l-value, even if it is
// swizzled again so that the final result does not contain duplicate components.
TEST_F(VertexShaderValidationTest, LValueSwizzleDuplicateComponents)
{

    const std::string &shaderString = R"(#version 300 es
        precision mediump float;

        void main()
        {
            vec2 f;
            (f.xxyy).xz = vec2(0.0);
        })";

    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a fragment shader with nested if statements without braces compiles successfully.
TEST_F(FragmentShaderValidationTest, HandleIfInnerIfStatementAlwaysTriviallyPruned)
{
    const std::string &shaderString =
        R"(precision mediump float;
        void main()
        {
            if (true)
                if (false)
                    gl_FragColor = vec4(0.0);
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a fragment shader with an if statement nested in a loop without braces compiles
// successfully.
TEST_F(FragmentShaderValidationTest, HandleLoopInnerIfStatementAlwaysTriviallyPruned)
{
    const std::string &shaderString =
        R"(precision mediump float;
        void main()
        {
            while (false)
                if (false)
                    gl_FragColor = vec4(0.0);
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that declaring both gl_FragColor and gl_FragData invariant is not an error. The GLSL ES 1.00
// spec only disallows writing to both of them. ANGLE extends this validation to also cover reads,
// but it makes sense not to treat declaring them both invariant as an error.
TEST_F(FragmentShaderValidationTest, DeclareBothBuiltInFragmentOutputsInvariant)
{
    const std::string &shaderString =
        R"(
        invariant gl_FragColor;
        invariant gl_FragData;
        precision mediump float;
        void main()
        {
            gl_FragColor = vec4(0.0);
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that a case cannot be placed inside a block nested inside a switch statement. GLSL ES 3.10
// section 6.2.
TEST_F(FragmentShaderValidationTest, CaseInsideBlock)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform int u;
        out vec4 my_FragColor;
        void main()
        {
            switch (u)
            {
                case 1:
                {
                    case 0:
                        my_FragColor = vec4(0.0);
                }
                default:
                    my_FragColor = vec4(1.0);
            }
        })";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test using a value from a constant array as a case label.
TEST_F(FragmentShaderValidationTest, ValueFromConstantArrayAsCaseLabel)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform int u;
        const int[3] arr = int[3](2, 1, 0);
        out vec4 my_FragColor;
        void main()
        {
            switch (u)
            {
                case arr[1]:
                    my_FragColor = vec4(0.0);
                case 2:
                case 0:
                default:
                    my_FragColor = vec4(1.0);
            }
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test using a value from a constant array as a fragment output index.
TEST_F(FragmentShaderValidationTest, ValueFromConstantArrayAsFragmentOutputIndex)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform int u;
        const int[3] arr = int[3](4, 1, 0);
        out vec4 my_FragData[2];
        void main()
        {
            my_FragData[arr[1]] = vec4(0.0);
            my_FragData[arr[2]] = vec4(0.0);
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test using a value from a constant array as an array size.
TEST_F(FragmentShaderValidationTest, ValueFromConstantArrayAsArraySize)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform int u;
        const int[3] arr = int[3](0, 2, 0);
        const int[arr[1]] arr2 = int[2](2, 1);
        out vec4 my_FragColor;
        void main()
        {
            my_FragColor = vec4(arr2[1]);
        })";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that an invalid struct with void fields doesn't crash or assert when used in a comma
// operator. This is a regression test.
TEST_F(FragmentShaderValidationTest, InvalidStructWithVoidFieldsInComma)
{
    // The struct needed the two fields for the bug to repro.
    const std::string &shaderString =
        R"(#version 300 es
precision highp float;

struct T { void a[8], c; };

void main() {
    0.0, T();
})";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that layout(early_fragment_tests) in; is valid in fragment shader
TEST_F(FragmentShaderValidationTest, ValidEarlyFragmentTests)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision mediump float;
        layout(early_fragment_tests) in;
        out vec4 color;
        void main()
        {
            color = vec4(0.0);
        })";
    if (!compile(kShaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that layout(early_fragment_tests=x) in; is invalid
TEST_F(FragmentShaderValidationTest, InvalidValueForEarlyFragmentTests)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision mediump float;
        layout(early_fragment_tests=1) in;
        out vec4 color;
        void main()
        {
            color = vec4(0.0);
        })";
    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that layout(early_fragment_tests) in varying; is invalid
TEST_F(FragmentShaderValidationTest, InvalidEarlyFragmentTestsOnVariableDecl)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        precision mediump float;
        layout(early_fragment_tests) in vec4 v;
        out vec4 color;
        void main()
        {
            color = v;
        })";
    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that layout(early_fragment_tests) in; is invalid in vertex shader
TEST_F(VertexShaderValidationTest, InvalidEarlyFragmentTests)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        layout(early_fragment_tests) in;
        void main()
        {
            gl_Position = vec4(0.0);
        })";
    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that layout(early_fragment_tests) in; is invalid in compute shader
TEST_F(ComputeShaderValidationTest, InvalidEarlyFragmentTests)
{
    constexpr char kShaderString[] =
        R"(#version 310 es
        layout(local_size_x = 1) in;
        layout(early_fragment_tests) in;
        void main() {})";
    if (compile(kShaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that layout(x) in; only accepts x=early_fragment_tests.
TEST_F(FragmentShaderValidationTest, NothingButEarlyFragmentTestsWithInWithoutVariableDecl)
{
    const char *noValueQualifiers[] = {
        "shared",      "packed",
        "std140",      "std430",
        "row_major",   "col_major",
        "location",    "yuv",
        "rgba32f",     "rgba16f",
        "r32f",        "rgba8",
        "rgba8_snorm", "rgba32i",
        "rgba16i",     "rgba8i",
        "r32i",        "rgba32ui",
        "rgba16ui",    "rgba8ui",
        "r32ui",       "points",
        "lines",       "lines_adjacency",
        "triangles",   "triangles_adjacency",
        "line_strip",  "triangle_strip",
    };

    const char *withValueQualifiers[] = {
        "location",     "binding",   "offset",      "local_size_x", "local_size_y",
        "local_size_z", "num_views", "invocations", "max_vertices", "index",
    };

    constexpr char kShaderStringPre[] =
        R"(#version 310 es
        precision mediump float;
        layout()";
    constexpr char kShaderStringPost[] =
        R"() in;
        out vec4 color;
        void main()
        {
            color = vec4(0.0);
        })";

    // Make sure the method of constructing shaders is valid.
    const std::string validShaderString =
        kShaderStringPre + std::string("early_fragment_tests") + kShaderStringPost;
    if (!compile(validShaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }

    for (size_t i = 0; i < ArraySize(noValueQualifiers); ++i)
    {
        const std::string shaderString =
            kShaderStringPre + std::string(noValueQualifiers[i]) + kShaderStringPost;

        if (compile(shaderString))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }

    for (size_t i = 0; i < ArraySize(withValueQualifiers); ++i)
    {
        const std::string shaderString =
            kShaderStringPre + std::string(withValueQualifiers[i]) + "=1" + kShaderStringPost;

        if (compile(shaderString))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}
