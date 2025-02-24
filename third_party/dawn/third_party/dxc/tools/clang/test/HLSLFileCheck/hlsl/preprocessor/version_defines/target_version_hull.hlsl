// RUN: %dxc -O0 -T hs_6_2 %s | FileCheck %s
// CHECK: fadd
// CHECK: fadd
// CHECK: fadd

struct PSSceneIn
{
    float4 pos  : SV_Position;
    float2 tex  : TEXCOORD0;
    float3 norm : NORMAL;

uint   RTIndex      : SV_RenderTargetArrayIndex;
};


struct HSPerVertexData
{
    PSSceneIn v;
};

struct HSPerPatchData
{
	float	edges[ 3 ]	: SV_TessFactor;
	float	inside		: SV_InsideTessFactor;
};

cbuffer cb : register(b0) {
  float foo;
};

HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points,  OutputPatch<HSPerVertexData, 3> outp )
{
    HSPerPatchData d;

    d.edges[ 0 ] = 1;
    d.edges[ 1 ] = 1;
    d.edges[ 2 ] = 1;
    d.inside = 1;

    return d;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HSPerPatchFunc")]
[outputcontrolpoints(3)]
HSPerVertexData main( const uint id : SV_OutputControlPointID,
                      const InputPatch< PSSceneIn, 3 > points )
{
    HSPerVertexData v;
    v.v = points[ id ];

    float x = foo;

#if defined(__SHADER_TARGET_STAGE) && __SHADER_TARGET_STAGE == __SHADER_STAGE_HULL
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MAJOR) && __SHADER_TARGET_MAJOR == 6
    x += 1;
#else
    x -= 1;
#endif
#if defined(__SHADER_TARGET_MINOR) && __SHADER_TARGET_MINOR == 2
    x += 1;
#else
    x -= 1;
#endif

    v.v.pos.x = x;

    return v;
}


