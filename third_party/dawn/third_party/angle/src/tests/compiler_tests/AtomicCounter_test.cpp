//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AtomicCounter_test.cpp:
//   Tests for validating ESSL 3.10 section 4.4.6.
//

#include "gtest/gtest.h"

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class AtomicCounterTest : public ShaderCompileTreeTest
{
  public:
    AtomicCounterTest() {}

  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->MaxAtomicCounterBindings = 8;
    }
};

// Test that layout qualifiers described in ESSL 3.10 section 4.4.6 can be successfully compiled,
// and the values of offset are properly assigned to counter variables.
TEST_F(AtomicCounterTest, BasicAtomicCounterDeclaration)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "layout(binding = 2) uniform atomic_uint b;\n"
        "layout(binding = 2, offset = 12) uniform atomic_uint c, d;\n"
        "layout(binding = 1, offset = 4) uniform atomic_uint e;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }

    std::vector<sh::ShaderVariable> counters = getUniforms();

    EXPECT_EQ(std::string("a"), counters[0].name);
    EXPECT_EQ(2, counters[0].binding);
    EXPECT_EQ(4, counters[0].offset);

    EXPECT_EQ(std::string("b"), counters[1].name);
    EXPECT_EQ(2, counters[1].binding);
    EXPECT_EQ(8, counters[1].offset);

    EXPECT_EQ(std::string("c"), counters[2].name);
    EXPECT_EQ(2, counters[2].binding);
    EXPECT_EQ(12, counters[2].offset);

    EXPECT_EQ(std::string("d"), counters[3].name);
    EXPECT_EQ(2, counters[3].binding);
    EXPECT_EQ(16, counters[3].offset);

    EXPECT_EQ(std::string("e"), counters[4].name);
    EXPECT_EQ(1, counters[4].binding);
    EXPECT_EQ(4, counters[4].offset);
}

// Test that ESSL 3.00 doesn't support atomic_uint.
TEST_F(AtomicCounterTest, InvalidShaderVersion)
{
    const std::string &source =
        "#version 300 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that any qualifier other than uniform leads to compile-time error.
TEST_F(AtomicCounterTest, InvalidQualifier)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) in atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that uniform must be specified for declaration.
TEST_F(AtomicCounterTest, UniformMustSpecifiedForDeclaration)
{
    const std::string &source =
        "#version 310 es\n"
        "atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that offset overlapping leads to compile-time error(ESSL 3.10 section 4.4.6).
TEST_F(AtomicCounterTest, BindingOffsetOverlapping)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "layout(binding = 2, offset = 6) uniform atomic_uint b;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test offset inheritance for multiple variables in one same declaration.
TEST_F(AtomicCounterTest, MultipleVariablesDeclaration)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a, b;\n"
        "layout(binding = 2, offset = 8) uniform atomic_uint c;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that subsequent declarations inherit the globally specified offset.
TEST_F(AtomicCounterTest, GlobalBindingOffsetOverlapping)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint;\n"
        "layout(binding = 2) uniform atomic_uint b;\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint c;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// The spec only demands offset unique and non-overlapping. So this should be allowed.
TEST_F(AtomicCounterTest, DeclarationSequenceWithDecrementalOffsetsSpecified)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint a;\n"
        "layout(binding = 2, offset = 0) uniform atomic_uint b;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(source))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that image format qualifiers are not allowed for atomic counters.
TEST_F(AtomicCounterTest, ImageFormatMustNotSpecified)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4, rgba32f) uniform atomic_uint a;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that global layout qualifiers must not use 'offset'.
TEST_F(AtomicCounterTest, OffsetMustNotSpecifiedForGlobalLayoutQualifier)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(offset = 4) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that offset overlapping leads to compile-time error (ESSL 3.10 section 4.4.6).
// Note that there is some vagueness in the spec when it comes to this test.
TEST_F(AtomicCounterTest, BindingOffsetOverlappingForArrays)
{
    const std::string &source =
        "#version 310 es\n"
        "layout(binding = 2, offset = 4) uniform atomic_uint[2] a;\n"
        "layout(binding = 2, offset = 8) uniform atomic_uint b;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(source))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
