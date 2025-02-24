//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Precise_test.cpp:
//   Test that precise produces the right number of NoContraction decorations in the generated
//   SPIR-V.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "common/spirv/spirv_instruction_parser_autogen.h"
#include "gtest/gtest.h"

namespace spirv = angle::spirv;

namespace
{
class PreciseTest : public testing::TestWithParam<bool>
{
  public:
    void SetUp() override
    {
        std::map<ShShaderOutput, std::string> shaderOutputList = {
            {SH_SPIRV_VULKAN_OUTPUT, "SH_SPIRV_VULKAN_OUTPUT"}};

        Initialize(shaderOutputList);
    }

    void TearDown() override
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            DestroyCompiler(shaderOutputType.first);
        }
    }

    void Initialize(std::map<ShShaderOutput, std::string> &shaderOutputList)
    {
        mShaderOutputList = std::move(shaderOutputList);

        for (auto shaderOutputType : mShaderOutputList)
        {
            sh::InitBuiltInResources(&mResourceList[shaderOutputType.first]);
            mCompilerList[shaderOutputType.first] = nullptr;
        }
    }

    void DestroyCompiler(ShShaderOutput shaderOutputType)
    {
        if (mCompilerList[shaderOutputType])
        {
            sh::Destruct(mCompilerList[shaderOutputType]);
            mCompilerList[shaderOutputType] = nullptr;
        }
    }

    void InitializeCompiler()
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            InitializeCompiler(shaderOutputType.first);
        }
    }

    void InitializeCompiler(ShShaderOutput shaderOutputType)
    {
        DestroyCompiler(shaderOutputType);

        mCompilerList[shaderOutputType] = sh::ConstructCompiler(
            GL_VERTEX_SHADER, SH_GLES3_2_SPEC, shaderOutputType, &mResourceList[shaderOutputType]);
        ASSERT_TRUE(mCompilerList[shaderOutputType] != nullptr)
            << "Compiler for " << mShaderOutputList[shaderOutputType]
            << " could not be constructed.";
    }

    testing::AssertionResult TestShaderCompile(ShShaderOutput shaderOutputType,
                                               const char *shaderSource)
    {
        const char *shaderStrings[] = {shaderSource};

        ShCompileOptions options = {};
        options.objectCode       = true;

        bool success = sh::Compile(mCompilerList[shaderOutputType], shaderStrings, 1, options);
        if (success)
        {
            return ::testing::AssertionSuccess()
                   << "Compilation success(" << mShaderOutputList[shaderOutputType] << ")";
        }
        return ::testing::AssertionFailure() << sh::GetInfoLog(mCompilerList[shaderOutputType]);
    }

    void TestShaderCompile(const char *shaderSource, size_t expectedNoContractionDecorationCount)
    {
        for (auto shaderOutputType : mShaderOutputList)
        {
            EXPECT_TRUE(TestShaderCompile(shaderOutputType.first, shaderSource));

            const spirv::Blob &blob =
                sh::GetObjectBinaryBlob(mCompilerList[shaderOutputType.first]);
            ValidateDecorations(blob, expectedNoContractionDecorationCount);
        }
    }

    void ValidateDecorations(const spirv::Blob &blob, size_t expectedNoContractionDecorationCount);

  private:
    std::map<ShShaderOutput, std::string> mShaderOutputList;
    std::map<ShShaderOutput, ShHandle> mCompilerList;
    std::map<ShShaderOutput, ShBuiltInResources> mResourceList;
};

// Parse the SPIR-V and verify that there are as many NoContraction decorations as expected.
void PreciseTest::ValidateDecorations(const spirv::Blob &blob,
                                      size_t expectedNoContractionDecorationCount)
{
    size_t currentWord                  = spirv::kHeaderIndexInstructions;
    size_t noContractionDecorationCount = 0;

    while (currentWord < blob.size())
    {
        uint32_t wordCount;
        spv::Op opCode;
        const uint32_t *instruction = &blob[currentWord];
        spirv::GetInstructionOpAndLength(instruction, &opCode, &wordCount);

        currentWord += wordCount;

        // Early out when the decorations section is visited.
        if (opCode == spv::OpTypeVoid || opCode == spv::OpTypeInt || opCode == spv::OpTypeFloat ||
            opCode == spv::OpTypeBool)
        {
            break;
        }

        if (opCode == spv::OpMemberDecorate)
        {
            spirv::IdRef type;
            spirv::LiteralInteger member;
            spv::Decoration decoration;
            spirv::ParseMemberDecorate(instruction, &type, &member, &decoration, nullptr);

            // NoContraction should be applied to arithmetic instructions, and should not be seen on
            // block members.
            EXPECT_NE(decoration, spv::DecorationNoContraction);
        }
        else if (opCode == spv::OpDecorate)
        {
            spirv::IdRef target;
            spv::Decoration decoration;
            spirv::ParseDecorate(instruction, &target, &decoration, nullptr);

            if (decoration == spv::DecorationNoContraction)
            {
                ++noContractionDecorationCount;
            }
        }
    }

    EXPECT_EQ(noContractionDecorationCount, expectedNoContractionDecorationCount);
}

// Test that precise on a local variable works.
TEST_F(PreciseTest, LocalVariable)
{
    constexpr char kVS[] = R"(#version 320 es

uniform float u;

void main()
{
    float f1 = u, f2 = u;   // f1 is precise, but f2 isn't.

    f1 += 1.0;              // NoContraction
    f2 += 1.0;

    float f3 = f1 * f1;     // NoContraction
    f3 /= 2.0;              // NoContraction

    int i1 = int(f3);       // i1 is precise
    ++i1;                   // NoContraction
    --i1;                   // NoContraction
    i1++;                   // NoContraction
    i1--;                   // NoContraction

    int i2 = i1 % 3;
    f2 -= float(i2);

    precise float p = float(i1) / 1.5;  // NoContraction

    gl_Position = vec4(p, f2, 0, 0);
})";

    InitializeCompiler();
    TestShaderCompile(kVS, 8);
}

// Test that precise on gl_Position works.
TEST_F(PreciseTest, Position)
{
    constexpr char kVS[] = R"(#version 320 es

uniform float u;

out float o;

precise gl_Position;

void main()
{
    mat4 m1 = mat4(u);      // m1 is precise, even if not all components are used to determine the
                            // gl_Position.
    vec4 v1 = vec4(u);      // v1 is precise

    vec4 v2 = m1 * v1;
    v1 *= m1;               // NoContraction
    m1 *= m1;               // NoContraction
    m1 *= u;                // NoContraction
    v1 *= u;                // NoContraction

    float f1 = dot(v1, v1);
    float f2 = dot(v1, v1); // NoContraction

    gl_Position = vec4(m1[0][0], v1[0], f2, 0);
    o = f1;
})";

    InitializeCompiler();
    TestShaderCompile(kVS, 5);
}

// Test that precise on function parameters and return value works.
TEST_F(PreciseTest, Functions)
{
    constexpr char kVS[] = R"(#version 320 es

uniform float u;

struct S1
{
    float f;
    int i;
};

precise float f1(S1 s, out int io)
{
    float f = s.f;          // f is precise
    f *= float(s.i);        // NoContraction

    io = s.i;
    ++io;

    return s.f / f;         // NoContraction
}

void f2(S1 s, precise out S1 so)
{
    float f = s.f;          // f is precise
    f /= float(s.i);        // NoContraction

    int i = s.i;            // i is precise
    ++i;                    // NoContraction

    so = S1(f, i);
}

void main()
{
    precise S1 s1;
    S1 s2;

    int i;
    float f = f1(s1, i);    // f1's return value being precise doesn't affect f

    f2(s1, s2);             // f2's out parameter being precise doesn't affect s2

    i /= 2;
    f *= 2.0;
    s2.f += float(s2.i);

    gl_Position = vec4(s1.f);
})";

    InitializeCompiler();
    TestShaderCompile(kVS, 4);
}

}  // anonymous namespace
