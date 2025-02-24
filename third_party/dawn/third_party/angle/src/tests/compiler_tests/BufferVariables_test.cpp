//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferVariables_test.cpp:
//   Tests for buffer variables in GLSL ES 3.10 section 4.3.7.
//

#include "gtest/gtest.h"

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class BufferVariablesTest : public ShaderCompileTreeTest
{
  public:
    BufferVariablesTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->MaxShaderStorageBufferBindings = 8;
    }
};

class BufferVariablesMatchTest : public MatchOutputCodeTest
{
  public:
    BufferVariablesMatchTest() : MatchOutputCodeTest(GL_VERTEX_SHADER, SH_ESSL_OUTPUT)
    {
        getResources()->MaxShaderStorageBufferBindings = 8;
    }
};

// Test that the buffer qualifier described in GLSL ES 3.10 section 4.3.7 can be successfully
// compiled.
TEST_F(BufferVariablesTest, BasicShaderStorageBlockDeclaration)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    int b1;\n"
        "    buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that shader storage block layout qualifiers can be declared for global scope.
TEST_F(BufferVariablesTest, LayoutQualifiersDeclaredInGlobal)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(shared, column_major) buffer;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that shader storage block can be used with one or more memory qualifiers.
TEST_F(BufferVariablesTest, ShaderStorageBlockWithMemoryQualifier)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) writeonly buffer buf {\n"
        "    int b1;\n"
        "    buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that buffer variables can be used with one or more memory qualifiers.
TEST_F(BufferVariablesTest, BufferVariablesWithMemoryQualifier)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "    writeonly buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that it is a compile-time error to declare buffer variables at global scope (outside a
// block).
TEST_F(BufferVariablesTest, DeclareBufferVariableAtGlobal)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer int a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the buffer variable can't be opaque type.
TEST_F(BufferVariablesTest, BufferVariableWithOpaqueType)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    int b1;\n"
        "    atomic_uint b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that the uniform variable can't be in shader storage block.
TEST_F(BufferVariablesTest, UniformVariableInShaderStorageBlock)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    uniform int a;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that buffer qualifier is not supported in verson lower than GLSL ES 3.10.
TEST_F(BufferVariablesTest, BufferQualifierInESSL3)
{
    const std::string &source =
        "#version 300 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    int b1;\n"
        "    buffer int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that can't assign to a readonly buffer variable.
TEST_F(BufferVariablesTest, AssignToReadonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    b1 = 5;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that can't assign to a buffer variable declared within shader storage block with readonly.
TEST_F(BufferVariablesTest, AssignToBufferVariableWithinReadonlyBlock)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) readonly buffer buf {\n"
        "    int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    b1 = 5;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that can't assign to a readonly buffer variable through an instance name.
TEST_F(BufferVariablesTest, AssignToReadonlyBufferVariableByInstanceName)
{
    const std::string &source =
        R"(#version 310 es
        layout(binding = 3) buffer buf {
            readonly float f;
        } instanceBuffer;
        void main()
        {
            instanceBuffer.f += 0.2;
        })";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that can't assign to a readonly struct buffer variable.
TEST_F(BufferVariablesTest, AssignToReadonlyStructBufferVariable)
{
    const std::string &source =
        R"(#version 310 es
        struct S {
            float f;
        };
        layout(binding = 3) buffer buf {
            readonly S s;
        };
        void main()
        {
            s.f += 0.2;
        })";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that can't assign to a readonly struct buffer variable through an instance name.
TEST_F(BufferVariablesTest, AssignToReadonlyStructBufferVariableByInstanceName)
{
    const std::string &source =
        R"(#version 310 es
        struct S {
            float f;
        };
        layout(binding = 3) buffer buf {
            readonly S s;
        } instanceBuffer;
        void main()
        {
            instanceBuffer.s.f += 0.2;
        })";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that a readonly and writeonly buffer variable should neither read or write.
TEST_F(BufferVariablesTest, AccessReadonlyWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    b1 = 5;\n"
        "    int test = b1;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that accessing a writeonly buffer variable should be error.
TEST_F(BufferVariablesTest, AccessWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    int test = b1;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that accessing a buffer variable through an instance name is ok.
TEST_F(BufferVariablesTest, AccessReadonlyBufferVariableByInstanceName)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly float f;\n"
        "} instanceBuffer;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = instanceBuffer.f;\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that accessing a buffer variable through an instance name inherits the writeonly qualifier
// and generates errors.
TEST_F(BufferVariablesTest, AccessWriteonlyBufferVariableByInstanceName)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) writeonly buffer buf {\n"
        "    float f;\n"
        "} instanceBuffer;\n"
        "void main()\n"
        "{\n"
        "    float test = instanceBuffer.f;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as the argument of a unary operator should be error.
TEST_F(BufferVariablesTest, UnaryOperatorWithWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    ++b1;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable on the left-hand side of compound assignment should be error.
TEST_F(BufferVariablesTest, CompoundAssignmentToWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    b1 += 5;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as ternary op argument should be error.
TEST_F(BufferVariablesTest, TernarySelectionWithWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly bool b1;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    int test = b1 ? 1 : 0;\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as array constructor argument should be error.
TEST_F(BufferVariablesTest, ArrayConstructorWithWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly float f;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    float a[3] = float[3](f, f, f);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as structure constructor argument should be error.
TEST_F(BufferVariablesTest, StructureConstructorWithWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "struct S {\n"
        "    int a;\n"
        "};\n"
        "struct T {\n"
        "    S b;\n"
        "};\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly S c;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    T t = T(c);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as built-in function argument should be error.
TEST_F(BufferVariablesTest, BuildInFunctionWithWriteonlyBufferVariable)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly int a;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    int test = min(a, 1);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that readonly buffer variable as user-defined function in argument should be ok.
TEST_F(BufferVariablesTest, UserDefinedFunctionWithReadonlyBufferVariableInArgument)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly float f;\n"
        "};\n"
        "void foo(float a) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as user-defined function in argument should be error.
TEST_F(BufferVariablesTest, UserDefinedFunctionWithWriteonlyBufferVariableInArgument)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly float f;\n"
        "};\n"
        "void foo(float a) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that writeonly buffer variable as user-defined function out argument should be ok.
TEST_F(BufferVariablesTest, UserDefinedFunctionWithWriteonlyBufferVariableOutArgument)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    writeonly float f;\n"
        "};\n"
        "void foo(out float a) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that readonly buffer variable as user-defined function out argument should be error.
TEST_F(BufferVariablesTest, UserDefinedFunctionWithReadonlyBufferVariableOutArgument)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 3) buffer buf {\n"
        "    readonly float f;\n"
        "};\n"
        "void foo(out float a) {}\n"
        "void main()\n"
        "{\n"
        "    foo(f);\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that buffer qualifier can't modify a function parameter.
TEST_F(BufferVariablesTest, BufferQualifierOnFunctionParameter)
{
    const std::string &source =
        "#version 310 es\n"
        "void foo(buffer float a) {}\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that std430 qualifier is supported for shader storage blocks.
TEST_F(BufferVariablesTest, ShaderStorageBlockWithStd430)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(std430) buffer buf {\n"
        "    int b1;\n"
        "    int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that using std430 qualifier on a uniform block will fail to compile.
TEST_F(BufferVariablesTest, UniformBlockWithStd430)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(std430) uniform buf {\n"
        "    int b1;\n"
        "    int b2;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that indexing a runtime-sized array with a positive index compiles.
TEST_F(BufferVariablesTest, IndexRuntimeSizedArray)
{
    const std::string &source =
        R"(#version 310 es

        layout(std430) buffer buf
        {
            int arr[];
        };

        void main()
        {
            arr[100];
        })";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that indexing a runtime-sized array with a negative constant index does not compile.
TEST_F(BufferVariablesTest, IndexRuntimeSizedArrayWithNegativeIndex)
{
    const std::string &source =
        R"(#version 310 es

        layout(std430) buffer buf
        {
            int arr[];
        };

        void main()
        {
            arr[-1];
        })";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that only the last member of a buffer can be runtime-sized.
TEST_F(BufferVariablesTest, RuntimeSizedVariableInNotLastInBuffer)
{
    const std::string &source =
        R"(#version 310 es

        layout(std430) buffer buf
        {
            int arr[];
            int i;
        };

        void main()
        {
        })";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that memory qualifiers are output.
TEST_F(BufferVariablesMatchTest, MemoryQualifiers)
{
    const std::string &source =
        R"(#version 310 es

        layout(std430) coherent buffer buf
        {
            int defaultCoherent;
            coherent ivec2 specifiedCoherent;
            volatile ivec3 specifiedVolatile;
            restrict ivec4 specifiedRestrict;
            readonly float specifiedReadOnly;
            writeonly vec2 specifiedWriteOnly;
            volatile readonly vec3 specifiedMultiple;
        };

        void main()
        {
        })";
    compile(source);
    ASSERT_TRUE(foundInESSLCode("coherent highp int"));
    ASSERT_TRUE(foundInESSLCode("coherent highp ivec2"));
    ASSERT_TRUE(foundInESSLCode("coherent volatile highp ivec3"));
    ASSERT_TRUE(foundInESSLCode("coherent restrict highp ivec4"));
    ASSERT_TRUE(foundInESSLCode("readonly coherent highp float"));
    ASSERT_TRUE(foundInESSLCode("writeonly coherent highp vec2"));
    ASSERT_TRUE(foundInESSLCode("readonly coherent volatile highp vec3"));
}
