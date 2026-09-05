// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Rsqrt
// CHECK: dot3
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: SimpleBezier11.hlsl
//
// This sample shows an simple implementation of the DirectX 11 Hardware Tessellator
// for rendering a Bezier Patch.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// This allows us to compile the shader with a #define to choose
// the different partition modes for the hull shader.
// See the hull shader: [partitioning(BEZIER_HS_PARTITION)]
// This sample demonstrates "integer", "fractional_even", and "fractional_odd"
#ifndef BEZIER_HS_PARTITION
#define BEZIER_HS_PARTITION "integer"
#endif // BEZIER_HS_PARTITION

// The input patch size.  In this sample, it is 16 control points.
// This value should match the call to IASetPrimitiveTopology()
#define INPUT_PATCH_SIZE 16

// The output patch size.  In this sample, it is also 16 control points.
#define OUTPUT_PATCH_SIZE 16

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame : register( b0 )
{
    matrix g_mViewProjection;
    float3 g_vCameraPosWorld;
    float  g_fTessellationFactor;
};

//--------------------------------------------------------------------------------------
// Vertex shader section
//--------------------------------------------------------------------------------------
struct VS_CONTROL_POINT_INPUT
{
    float3 vPosition        : POSITION;
};

struct VS_CONTROL_POINT_OUTPUT
{
    float3 vPosition        : POSITION;
};

// This simple vertex shader passes the control points straight through to the
// hull shader.  In a more complex scene, you might transform the control points
// or perform skinning at this step.

// The input to the vertex shader comes from the vertex buffer.

// The output from the vertex shader will go into the hull shader.

VS_CONTROL_POINT_OUTPUT BezierVS( VS_CONTROL_POINT_INPUT Input )
{
    VS_CONTROL_POINT_OUTPUT Output;

    Output.vPosition = Input.vPosition;

    return Output;
}

//--------------------------------------------------------------------------------------
// Constant data function for the BezierHS.  This is executed once per patch.
//--------------------------------------------------------------------------------------
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4]             : SV_TessFactor;
    float Inside[2]            : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
    float3 vPosition           : BEZIERPOS;
};

// This constant hull shader is executed once per patch.  For the simple Mobius strip
// model, it will be executed 4 times.  In this sample, we set the tessellation factor
// via SV_TessFactor and SV_InsideTessFactor for each patch.  In a more complex scene,
// you might calculate a variable tessellation factor based on the camera's distance.

HS_CONSTANT_DATA_OUTPUT BezierConstantHS( InputPatch<VS_CONTROL_POINT_OUTPUT, INPUT_PATCH_SIZE> ip,
                                          uint PatchID : SV_PrimitiveID )
{    
    HS_CONSTANT_DATA_OUTPUT Output;

    float TessAmount = g_fTessellationFactor;

    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount;
    Output.Inside[0] = Output.Inside[1] = TessAmount;

    return Output;
}

// The hull shader is called once per output control point, which is specified with
// outputcontrolpoints.  For this sample, we take the control points from the vertex
// shader and pass them directly off to the domain shader.  In a more complex scene,
// you might perform a basis conversion from the input control points into a Bezier
// patch, such as the SubD11 Sample.

// The input to the hull shader comes from the vertex shader

// The output from the hull shader will go to the domain shader.
// The tessellation factor, topology, and partition mode will go to the fixed function
// tessellator stage to calculate the UVW and domain points.

[domain("quad")]
[partitioning(BEZIER_HS_PARTITION)]
[outputtopology("triangle_cw")]
[outputcontrolpoints(OUTPUT_PATCH_SIZE)]
[patchconstantfunc("BezierConstantHS")]
HS_OUTPUT BezierHS( InputPatch<VS_CONTROL_POINT_OUTPUT, INPUT_PATCH_SIZE> p, 
                    uint i : SV_OutputControlPointID,
                    uint PatchID : SV_PrimitiveID )
{
    HS_OUTPUT Output;
    Output.vPosition = p[i].vPosition;
    return Output;
}

//--------------------------------------------------------------------------------------
// Bezier evaluation domain shader section
//--------------------------------------------------------------------------------------
struct DS_OUTPUT
{
    float4 vPosition        : SV_POSITION;
    float3 vWorldPos        : WORLDPOS;
    float3 vNormal            : NORMAL;
};

//--------------------------------------------------------------------------------------
float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4( invT * invT * invT,
                   3.0f * t * invT * invT,
                   3.0f * t * t * invT,
                   t * t * t );
}

//--------------------------------------------------------------------------------------
float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4( -3 * invT * invT,
                   3 * invT * invT - 6 * t * invT,
                   6 * t * invT - 3 * t * t,
                   3 * t * t );
}

//--------------------------------------------------------------------------------------
float3 EvaluateBezier( const OutputPatch<HS_OUTPUT, OUTPUT_PATCH_SIZE> bezpatch,
                       float4 BasisU,
                       float4 BasisV )
{
    float3 Value = float3(0,0,0);
    Value  = BasisV.x * ( bezpatch[0].vPosition * BasisU.x + bezpatch[1].vPosition * BasisU.y + bezpatch[2].vPosition * BasisU.z + bezpatch[3].vPosition * BasisU.w );
    Value += BasisV.y * ( bezpatch[4].vPosition * BasisU.x + bezpatch[5].vPosition * BasisU.y + bezpatch[6].vPosition * BasisU.z + bezpatch[7].vPosition * BasisU.w );
    Value += BasisV.z * ( bezpatch[8].vPosition * BasisU.x + bezpatch[9].vPosition * BasisU.y + bezpatch[10].vPosition * BasisU.z + bezpatch[11].vPosition * BasisU.w );
    Value += BasisV.w * ( bezpatch[12].vPosition * BasisU.x + bezpatch[13].vPosition * BasisU.y + bezpatch[14].vPosition * BasisU.z + bezpatch[15].vPosition * BasisU.w );

    return Value;
}

// The domain shader is run once per vertex and calculates the final vertex's position
// and attributes.  It receives the UVW from the fixed function tessellator and the
// control point outputs from the hull shader.  Since we are using the DirectX 11
// Tessellation pipeline, it is the domain shader's responsibility to calculate the
// final SV_POSITION for each vertex.  In this sample, we evaluate the vertex's
// position using a Bernstein polynomial and the normal is calculated as the cross
// product of the U and V derivatives.

// The input SV_DomainLocation to the domain shader comes from fixed function
// tessellator.  And the OutputPatch comes from the hull shader.  From these, you
// must calculate the final vertex position, color, texcoords, and other attributes.

// The output from the domain shader will be a vertex that will go to the video card's
// rasterization pipeline and get drawn to the screen.

[domain("quad")]
DS_OUTPUT BezierDS( HS_CONSTANT_DATA_OUTPUT input, 
                    float2 UV : SV_DomainLocation,
                    const OutputPatch<HS_OUTPUT, OUTPUT_PATCH_SIZE> bezpatch )
{
    float4 BasisU = BernsteinBasis( UV.x );
    float4 BasisV = BernsteinBasis( UV.y );
    float4 dBasisU = dBernsteinBasis( UV.x );
    float4 dBasisV = dBernsteinBasis( UV.y );

    float3 WorldPos = EvaluateBezier( bezpatch, BasisU, BasisV );
    float3 Tangent = EvaluateBezier( bezpatch, dBasisU, BasisV );
    float3 BiTangent = EvaluateBezier( bezpatch, BasisU, dBasisV );
    float3 Norm = normalize( cross( Tangent, BiTangent ) );

    DS_OUTPUT Output;
    Output.vPosition = mul( float4(WorldPos,1), g_mViewProjection );
    Output.vWorldPos = WorldPos;
    Output.vNormal = Norm;

    return Output;    
}

//--------------------------------------------------------------------------------------
// Smooth shading pixel shader section
//--------------------------------------------------------------------------------------

// The pixel shader works the same as it would in a normal graphics pipeline.
// In this sample, it performs very simple N dot L lighting.

float4 main( DS_OUTPUT Input ) : SV_TARGET
{
    float3 N = normalize(Input.vNormal);
    float3 L = normalize(Input.vWorldPos - g_vCameraPosWorld);
    return abs(dot(N, L)) * float4(1, 0, 0, 1);
}
