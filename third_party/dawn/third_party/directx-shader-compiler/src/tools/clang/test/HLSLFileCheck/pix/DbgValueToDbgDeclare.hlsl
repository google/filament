// RUN: %dxc -EFlowControlPS -Tps_6_0 /O3 /Zi %s                                          | %FileCheck %s -check-prefixes=VEC,VEC-BUG 
// RUN: %dxc -EFlowControlPS -Tps_6_0 /O3 /Zi %s | %opt -S -dxil-dbg-value-to-dbg-declare | %FileCheck %s -check-prefixes=VEC,VEC-CHK
// RUN: %dxc -EGeometryPS    -Tps_6_0 /O3 /Zi %s                                          | %FileCheck %s -check-prefixes=RES,RES-BUG
// RUN: %dxc -EGeometryPS    -Tps_6_0 /O3 /Zi %s | %opt -S -dxil-dbg-value-to-dbg-declare | %FileCheck %s -check-prefixes=RES,RES-CHK

// These tests are designed to exercise the dbg.value to dbg.declare conversion
// pass' handling of known issues with dxcompiler's emission of debug info.
// When those bugs are fixed we should probably still test these corner cases
// but we won't be able to use dxc during the test.
struct VS_OUTPUT_ENV
{
    float4 Pos        : SV_Position;
    float2 Tex        : TEXCOORD0;
};

struct VS_OUTPUT_PosAt1
{
    float2 Tex0        : TEXCOORD0;
    float4 Pos         : SV_Position;
    float2 Tex1        : TEXCOORD1;
};


struct VS_OUTPUT_GEO
{
    float4 Pos        : SV_Position;
    float2 Tex0        : TEXCOORD0;
    float2 Tex1        : TEXCOORD1;
};

Texture2D g_txDiffuse : register(t0);
Texture2D g_txBump: register(t1);
Texture2D g_txEnv: register(t2);

SamplerState g_samStage0 : register(s0);
SamplerState g_samStage1 : register(s1);

cbuffer cbEveryFrame : register(b0)
{
    int i32;
    float f32;
};

/***************************************************
 * Test for dxcompiler bug workaround:             *
 * vector in dbg.value                             *
 ***************************************************/
// VEC-LABLE: sw.bb:
// VEC-BUG:       @llvm.dbg.value(metadata <4 x float>
// VEC-CHK:       store float 1.000000e+00, float* %9
// VEC-CHK:       store float 0.000000e+00, float* %10
// VEC-CHK:       store float 1.000000e+00, float* %11
// VEC-CHK:       store float 1.000000e+00, float* %12
float4 Vectorize(float f)
{
    float4 ret;

    if (f < 1024) // testbreakpoint0
        ret.x = f;
    else
        ret.x = f + 100;


    if (f < 512)
        ret.y = f;
    else
        ret.y = f + 10;


    if (f < 2048)
        ret.z = f;
    else
        ret.z = f + 1000;


    if (f < 4096)
        ret.w = f + f;
    else
        ret.w = f + 1;

    return ret;
}

float4 FlowControlPS(VS_OUTPUT_ENV input) : SV_Target
{
    float4 ret = { f32,0,0,1 }; // FirstExecutableLine
    switch (i32)
    {
    case 0:
        ret = float4(1, 0, 1, 1);
        break;
    case 32:
        ret = Vectorize(f32);
        break;
    }
    
    return ret;
}

/***************************************************
 * Test for dxcompiler bug workaround:             *
 * dx.types.ResRet.f32 in dbg.value                *
 ***************************************************/
// RES:           %[[S:[0-9]+]] = call %dx.types.ResRet.f32 @dx.op.sample.f32
// RES-BUG:       @llvm.dbg.value(metadata %dx.types.ResRet.f32
// RES-CHK-DAG:   %[[X:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[S]], 0
// RES-CHK-DAG:   %[[Y:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[S]], 1
// RES-CHK-DAG:   %[[Z:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[S]], 2
// RES-CHK-DAG:   %[[W:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[S]], 3
// RES-CHK-DAG:   store float %[[X]]
// RES-CHK-DAG:   store float %[[Y]]
// RES-CHK-DAG:   store float %[[Z]]
// RES-CHK-DAG:   store float %[[W]]
float4 GeometryPS( VS_OUTPUT_GEO input ) : SV_Target
{
    float4 Diffuse    = g_txDiffuse.Sample(g_samStage0, input.Tex1);
    float2 Bump       = g_txBump.Sample(g_samStage1, input.Tex1).xy;
    float2 BumpOffset;

    BumpOffset.x = Bump.x * 0.5f;
    BumpOffset.y = Bump.y * 0.5f;

    float2 EnvTexCrd = input.Tex0 + BumpOffset;
    float4 Env = g_txEnv.Sample(g_samStage1, EnvTexCrd);

    return Diffuse + 0.25 * Env;
}
