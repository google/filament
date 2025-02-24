//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerPerfTest:
//   Performance test for the shader translator. The test initializes the compiler once and then
//   compiles the same shader repeatedly. There are different variations of the tests using
//   different shaders.
//

#include "ANGLEPerfTest.h"

#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/InitializeGlobals.h"
#include "compiler/translator/PoolAlloc.h"

namespace
{

const char *kSimpleESSL100FragSource = R"(
precision mediump float;
void main()
{
    gl_FragColor = vec4(0, 1, 0, 1);
}
)";

const char *kSimpleESSL100Id = "SimpleESSL100";

const char *kSimpleESSL300FragSource = R"(#version 300 es
precision highp float;
out vec4 outColor;
void main()
{
    outColor = vec4(0, 1, 0, 1);
}
)";

const char *kSimpleESSL300Id = "SimpleESSL300";

const char *kRealWorldESSL100FragSource = R"(precision highp float;
precision highp sampler2D;
precision highp int;
varying vec2 vPixelCoords; // in pixels
uniform int uCircleCount;
uniform sampler2D uCircleParameters;
uniform sampler2D uBrushTex;
void main(void)
{
    float destAlpha = 0.0;
    for (int i = 0; i < 32; ++i)
    {
        vec4 parameterColor = texture2D(uCircleParameters,vec2(0.25, (float(i) + 0.5) / 32.0));
        vec2 center = parameterColor.xy;
        float circleRadius = parameterColor.z;
        float circleFlowAlpha = parameterColor.w;
        vec4 parameterColor2 = texture2D(uCircleParameters,vec2(0.75, (float(i) + 0.5) / 32.0));
        float circleRotation = parameterColor2.x;
        vec2 centerDiff = vPixelCoords - center;
        float radius = max(circleRadius, 0.5);
        float flowAlpha = (circleRadius < 0.5) ? circleFlowAlpha * circleRadius * circleRadius * 4.0: circleFlowAlpha;
        float antialiasMult = clamp((radius + 1.0 - length(centerDiff)) * 0.5, 0.0, 1.0);
        mat2 texRotation = mat2(cos(circleRotation), -sin(circleRotation), sin(circleRotation), cos(circleRotation));
        vec2 texCoords = texRotation * centerDiff / radius * 0.5 + 0.5;
        float texValue = texture2D(uBrushTex, texCoords).r;
        float circleAlpha = flowAlpha * antialiasMult * texValue;
        if (i < uCircleCount)
        {
            destAlpha = clamp(circleAlpha + (1.0 - circleAlpha) * destAlpha, 0.0, 1.0);
        }
    }
    gl_FragColor = vec4(0.0, 0.0, 0.0, destAlpha);
})";

const char *kRealWorldESSL100Id = "RealWorldESSL100";

// This shader is intended to trigger many AST transformations, particularly on the HLSL backend.
const char *kTrickyESSL300FragSource = R"(#version 300 es
precision highp float;
precision highp sampler2D;
precision highp isampler2D;
precision highp int;

float globalF;

uniform ivec4 uivec;
uniform int ui;

struct SS
{
    int iField;
    float fField;
    vec2 f2Field;
    sampler2D sField;
    isampler2D isField;
};
uniform SS us;

out vec4 my_FragColor;

float[3] sideEffectArray()
{
    globalF += 1.0;
    return float[3](globalF, globalF * 2.0, globalF * 3.0);
}

// This struct is unused and can be pruned.
struct SUnused
{
    vec2 fField;
};

void main()
{
    struct S2
    {
        float fField;
    } s2;
    vec4 foo = vec4(ui);
    mat4 fooM = mat4(foo.x);

    // Some unused variables that can be pruned.
    float fUnused, fUnused2;
    ivec4 iUnused, iUnused2;

    globalF = us.fField;
    s2.fField = us.fField;

    float[3] fa = sideEffectArray();

    globalF -= us.fField;
    if (fa == sideEffectArray())
    {
        globalF += us.fField * sin(2.0);
    }

    // Switch with fall-through.
    switch (ui)
    {
      case 0:
        // Sequence operator and matrix and vector dynamic indexing.
        (globalF += 1.0, fooM[ui][ui] += fooM[ui - 1][uivec[ui] + 1]);
      case 1:
        // Built-in emulation.
        foo[3] = tanh(foo[1]);
      default:
        // Sequence operator and length of an array expression with side effects.
        foo[2] += (globalF -= 1.0, float((sideEffectArray()).length() * 2));
    }
    int i = 0;
    do
    {
        s2.fField = us.fField * us.f2Field.x;
        // Sequence operator and short-circuiting operator with side effects on the right hand side.
    } while ((++i, i < int(us.fField) && ++i <= ui || ++i < ui * 2 - 3));
    // Samplers in structures and integer texture sampling.
    foo += texture(us.sField, us.f2Field) + intBitsToFloat(texture(us.isField, us.f2Field + 4.0));
    my_FragColor = foo * s2.fField * globalF + fooM[ui];
})";

const char *kTrickyESSL300Id = "TrickyESSL300";

constexpr int kNumIterationsPerStep = 4;

struct CompilerParameters
{
    CompilerParameters() { output = SH_HLSL_4_1_OUTPUT; }

    CompilerParameters(ShShaderOutput output) : output(output) {}

    const char *str() const
    {
        switch (output)
        {
            case SH_HLSL_4_1_OUTPUT:
                return "HLSL_4_1";
            case SH_GLSL_450_CORE_OUTPUT:
                return "GLSL_4_50";
            case SH_ESSL_OUTPUT:
                return "ESSL";
            default:
                UNREACHABLE();
                return "unk";
        }
    }

    ShShaderOutput output;
};

bool IsPlatformAvailable(const CompilerParameters &param)
{
    switch (param.output)
    {
        case SH_HLSL_4_1_OUTPUT:
        case SH_HLSL_3_0_OUTPUT:
        {
            angle::PoolAllocator allocator;
            InitializePoolIndex();
            allocator.push();
            SetGlobalPoolAllocator(&allocator);
            ShHandle translator =
                sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL2_SPEC, param.output);
            bool success = translator != nullptr;
            SetGlobalPoolAllocator(nullptr);
            allocator.pop();
            FreePoolIndex();
            if (!success)
            {
                return false;
            }
            break;
        }
        default:
            break;
    }
    return true;
}

struct CompilerPerfParameters final : public CompilerParameters
{
    CompilerPerfParameters(ShShaderOutput output,
                           const char *shaderSource,
                           const char *shaderSourceId)
        : CompilerParameters(output), shaderSource(shaderSource)
    {
        testId = shaderSourceId;
        testId += "_";
        testId += CompilerParameters::str();
    }

    const char *shaderSource;
    std::string testId;
};

std::ostream &operator<<(std::ostream &stream, const CompilerPerfParameters &p)
{
    stream << p.testId;
    return stream;
}

class CompilerPerfTest : public ANGLEPerfTest,
                         public ::testing::WithParamInterface<CompilerPerfParameters>
{
  public:
    CompilerPerfTest();

    void step() override;

    void SetUp() override;
    void TearDown() override;

  protected:
    void setTestShader(const char *str) { mTestShader = str; }

  private:
    const char *mTestShader;

    ShBuiltInResources mResources;
    angle::PoolAllocator mAllocator;
    sh::TCompiler *mTranslator;
};

CompilerPerfTest::CompilerPerfTest()
    : ANGLEPerfTest("CompilerPerf", "", GetParam().testId, kNumIterationsPerStep)
{}

void CompilerPerfTest::SetUp()
{
    ANGLEPerfTest::SetUp();

    InitializePoolIndex();
    mAllocator.push();
    SetGlobalPoolAllocator(&mAllocator);

    const auto &params = GetParam();

    mTranslator = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL2_SPEC, params.output);
    sh::InitBuiltInResources(&mResources);
    mResources.FragmentPrecisionHigh = true;
    if (!mTranslator->Init(mResources))
    {
        SafeDelete(mTranslator);
    }

    setTestShader(params.shaderSource);
}

void CompilerPerfTest::TearDown()
{
    SafeDelete(mTranslator);

    SetGlobalPoolAllocator(nullptr);
    mAllocator.pop();

    FreePoolIndex();

    ANGLEPerfTest::TearDown();
}

void CompilerPerfTest::step()
{
    const char *shaderStrings[] = {mTestShader};

    ShCompileOptions compileOptions              = {};
    compileOptions.objectCode                    = true;
    compileOptions.initializeUninitializedLocals = true;
    compileOptions.initOutputVariables           = true;

#if !defined(NDEBUG)
    // Make sure that compilation succeeds and print the info log if it doesn't in debug mode.
    if (!mTranslator->compile(shaderStrings, 1, compileOptions))
    {
        std::cout << "Compiling perf test shader failed with log:\n"
                  << mTranslator->getInfoSink().info.c_str();
    }
#endif

    for (unsigned int iteration = 0; iteration < kNumIterationsPerStep; ++iteration)
    {
        mTranslator->compile(shaderStrings, 1, compileOptions);
    }
}

TEST_P(CompilerPerfTest, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(
    CompilerPerfTest,
    CompilerPerfParameters(SH_HLSL_4_1_OUTPUT, kSimpleESSL100FragSource, kSimpleESSL100Id),
    CompilerPerfParameters(SH_HLSL_4_1_OUTPUT, kSimpleESSL300FragSource, kSimpleESSL300Id),
    CompilerPerfParameters(SH_HLSL_4_1_OUTPUT, kRealWorldESSL100FragSource, kRealWorldESSL100Id),
    CompilerPerfParameters(SH_HLSL_4_1_OUTPUT, kTrickyESSL300FragSource, kTrickyESSL300Id),
    CompilerPerfParameters(SH_GLSL_450_CORE_OUTPUT, kSimpleESSL100FragSource, kSimpleESSL100Id),
    CompilerPerfParameters(SH_GLSL_450_CORE_OUTPUT, kSimpleESSL300FragSource, kSimpleESSL300Id),
    CompilerPerfParameters(SH_GLSL_450_CORE_OUTPUT,
                           kRealWorldESSL100FragSource,
                           kRealWorldESSL100Id),
    CompilerPerfParameters(SH_GLSL_450_CORE_OUTPUT, kTrickyESSL300FragSource, kTrickyESSL300Id),
    CompilerPerfParameters(SH_ESSL_OUTPUT, kSimpleESSL100FragSource, kSimpleESSL100Id),
    CompilerPerfParameters(SH_ESSL_OUTPUT, kSimpleESSL300FragSource, kSimpleESSL300Id),
    CompilerPerfParameters(SH_ESSL_OUTPUT, kRealWorldESSL100FragSource, kRealWorldESSL100Id),
    CompilerPerfParameters(SH_ESSL_OUTPUT, kTrickyESSL300FragSource, kTrickyESSL300Id));

}  // anonymous namespace
