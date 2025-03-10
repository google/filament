// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: bufferLoad
// CHECK: storeOutput


//--------------------------------------------------------------------------------------
// File: Render.hlsl
//
// The shaders for rendering tessellated mesh and base mesh
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
    row_major matrix    g_mWorldViewProjection    : packoffset( c0 );
}

// The tessellated vertex structure
struct TessedVertex
{
    uint BaseTriID;         // Which triangle of the base mesh this tessellated vertex belongs to?
    float2 bc;              // Barycentric coordinates with regard to the base triangle
};
Buffer<float4>                  g_base_vb_buffer : register(t0);  // Base mesh vertex buffer
StructuredBuffer<TessedVertex>  g_TessedVertices : register(t1);  // Tessellated mesh vertex buffer

float4 bary_centric(float4 v1, float4 v2, float4 v3, float2 bc)
{
    return (1 - bc.x - bc.y) * v1 + bc.x * v2 + bc.y * v3;
}

float4 main( uint vertid : SV_VertexID ) : SV_POSITION
{
    TessedVertex input = g_TessedVertices[vertid];
    
    // Get the positions of the three vertices of the base triangle
    float4 v[3];
    [unroll]
    for (int i = 0; i < 3; ++ i)
    {
        uint vert_id = input.BaseTriID * 3 + i;
        v[i] = g_base_vb_buffer[vert_id];
    }

    // Calculate the position of this tessellated vertex from barycentric coordinates and then project it
    return mul(bary_centric(v[0], v[1], v[2], input.bc), g_mWorldViewProjection);
}