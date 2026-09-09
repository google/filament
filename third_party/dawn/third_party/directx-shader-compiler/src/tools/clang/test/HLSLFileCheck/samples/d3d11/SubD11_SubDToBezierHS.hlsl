// RUN: %dxc -E main -T hs_6_0 -O0 %s | FileCheck %s

// CHECK: primitiveID
// CHECK: storePatchConstant
// CHECK: main
// These values are not used other than being stored into array allocas, which we now remove.
// xCHECK: primitiveID
// xCHECK: bufferLoad
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: SubD11.hlsl
//
// This file contains functions to convert from a Catmull-Clark subdivision
// representation to a bicubic patch representation.
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// A sample extraordinary SubD quad is represented by the following diagram:
//
//                        15              Valences:
//                       /  \               Vertex 0: 5
//                      /    14             Vertex 1: 4
//          17---------16   /  \            Vertex 2: 5
//          | \         |  /    \           Vertex 3: 3
//          |  \        | /      13
//          |   \       |/      /         Prefixes:
//          |    3------2------12           Vertex 0: 9
//          |    |      |      |            Vertex 1: 12
//          |    |      |      |            Vertex 2: 16
//          4----0------1------11           Vertex 3: 18
//         /    /|      |      |
//        /    / |      |      |
//       5    /  8------9------10
//        \  /  /
//         6   /
//          \ /
//           7
//
// Where the quad bounded by vertices 0,1,2,3 represents the actual subd surface of interest
// The 1-ring neighborhood of the quad is represented by vertices 4 through 17.  The counter-
// clockwise winding of this 1-ring neighborhood is important, especially when it comes to compute
// the corner vertices of the bicubic patch that we will use to approximate the subd quad (0,1,2,3).
// 
// The resulting bicubic patch fits within the subd quad (0,1,2,3) and has the following control
// point layout:
//
//     12--13--14--15
//      8---9--10--11
//      4---5---6---7
//      0---1---2---3
//
// The inner 4 control points of the bicubic patch are a combination of only the vertices (0,1,2,3)
// of the subd quad.  However, the corner control points for the bicubic patch (0,3,15,12) are actually
// a much more complex weighting of the subd patch and the 1-ring neighborhood.  In the example above
// the bicubic control point 0 is actually a weighted combination of subd points 0,1,2,3 and 1-ring
// neighborhood points 17, 4, 5, 6, 7, 8, and 9.  We can see that the 1-ring neighbor hood is simply
// walked from the prefix value from the previous corner (corner 3 in this case) to the prefix 
// prefix value for the current corner.  We add one more vertex on either side of the prefix values
// and we have all the data necessary to calculate the value for the corner points.
//
// The edge control points of the bicubic patch (1,2,13,14,4,8,7,11) are also combinations of their 
// neighbors, but fortunately each one is only a combination of 6 values and no walk is required.
//--------------------------------------------------------------------------------------

#define MOD4(x) ((x)&3)
#ifndef MAX_POINTS
#define MAX_POINTS 32
#endif
#define MAX_BONE_MATRICES 80
                        
//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D       g_txHeight : register( t0 );           // Height and Bump texture
Texture2D       g_txDiffuse : register( t1 );          // Diffuse texture
Texture2D       g_txSpecular : register( t2 );         // Specular texture

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState g_samLinear : register( s0 );
SamplerState g_samPoint : register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbTangentStencilConstants : register( b0 )
{
    float g_TanM[1024]; // Tangent patch stencils precomputed by the application
    float g_fCi[16];    // Valence coefficients precomputed by the application
};

cbuffer cbPerMesh : register( b1 )
{
    matrix g_mConstBoneWorld[MAX_BONE_MATRICES];
};

cbuffer cbPerFrame : register( b2 )
{
    matrix g_mViewProjection;
    float3 g_vCameraPosWorld;
    float  g_fTessellationFactor;
    float  g_fDisplacementHeight;
    float3 g_vSolidColor;
};

cbuffer cbPerSubset : register( b3 )
{
    int g_iPatchStartIndex;
}

//--------------------------------------------------------------------------------------
Buffer<uint4>  g_ValencePrefixBuffer : register( t0 );

//--------------------------------------------------------------------------------------
struct VS_CONTROL_POINT_OUTPUT
{
    float3 vPosition		: WORLDPOS;
    float2 vUV				: TEXCOORD0;
    float3 vTangent			: TANGENT;
};

struct BEZIER_CONTROL_POINT
{
    float3 vPosition	: BEZIERPOS;
};

struct PS_INPUT
{
    float3 vWorldPos        : POSITION;
    float3 vNormal			: NORMAL;
    float2 vUV				: TEXCOORD;
    float3 vTangent			: TANGENT;
    float3 vBiTangent		: BITANGENT;
};

//--------------------------------------------------------------------------------------
// SubD to Bezier helper functions
//--------------------------------------------------------------------------------------
// Helps with getting tangent stencils from the g_TanM constant array
#define TANM(a,v) ( g_TanM[ Val[v]*64 + (a) ] )

//--------------------------------------------------------------------------------------
float3 ComputeInteriorVertex( uint index, 
                              uint Val[4], 
                              const in InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip )
{
    switch( index )
    {
    case 0:
        return (ip[0].vPosition*Val[0] + ip[1].vPosition*2 +      ip[2].vPosition +        ip[3].vPosition*2)      / (5+Val[0]);
    case 1:
        return (ip[0].vPosition*2 +      ip[1].vPosition*Val[1] + ip[2].vPosition*2 +      ip[3].vPosition)        / (5+Val[1]);
    case 2:
        return (ip[0].vPosition +        ip[1].vPosition*2 +      ip[2].vPosition*Val[2] + ip[3].vPosition*2)      / (5+Val[2]);
    case 3:
        return (ip[0].vPosition*2 +      ip[1].vPosition +        ip[2].vPosition*2 +      ip[3].vPosition*Val[3]) / (5+Val[3]);
    }
    
    return float3(0,0,0);
}

//--------------------------------------------------------------------------------------
// Computes the corner vertices of the output UV patch.  The corner vertices are
// a weighted combination of all points that are "connected" to that corner by an edge.
// The interior 4 points of the original subd quad are easy to get.  The points in the
// 1-ring neighborhood around the interior quad are not.
//
// Because the valence of that corner could be any number between 3 and 16, we need to
// walk around the subd patch vertices connected to that point.  This is there the
// Pref (prefix) values come into play.  Each corner has a prefix value that is the index
// of the last value around the 1-ring neighborhood that should be used in calculating
// the coefficient of that corner.  The walk goes from the prefix value of the previous
// corner to the prefix value of the current corner.
//--------------------------------------------------------------------------------------
void ComputeCornerVertex( uint index, 
                          out float3 CornerB, // Corner for the Bezier patch
                          out float3 CornerU, // Corner for the tangent patch
                          out float3 CornerV, // Corner for the bitangent patch
                          const in InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, 
                          const in uint Val[4], 
                          const in uint Pref[4] )
{
    const float fOWt = 1;
    const float fEWt = 4;

    // Figure out where to start the walk by using the previous corner's prefix value
    uint PrefIm1 = 0;
    uint uStart = 4;
    if( index )
    {
        PrefIm1 = Pref[index-1];
        uStart = PrefIm1;
    }
    
    // Setup the walk indices
    uint uTIndexStart = 2 - (index&1);
    uint uTIndex = uTIndexStart;

    // Calculate the N*N weight for the final value
    CornerB = (Val[index]*Val[index])*ip[index].vPosition; // n^2 part

    // Zero out the corners
    CornerU = float4(0,0,0,0);
    CornerV = float4(0,0,0,0);
    
    const uint uV = Val[index]  + ( ( index & 1 ) ? 1 : -1 );
        
    // Start the walk with the uStart prefix (the prefix of the corner before us)
    CornerB += ip[uStart].vPosition * fEWt;
    CornerU += ip[uStart].vPosition * TANM( uTIndex * 2, index );
    CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex + uV ) % Val[index] ) * 2, index);

    // Gather all vertices between the previous corner's prefix and our own prefix
    // We'll do two at a time, since they always come in twos
    while(uStart < Pref[index]-1) 
    {
        ++uStart;
        CornerB += ip[uStart].vPosition * fOWt;
        CornerU += ip[uStart].vPosition * TANM( uTIndex * 2 + 1, index );
        CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex + uV ) % Val[index] ) * 2 + 1, index );

        ++uTIndex;
        ++uStart;
        CornerB += ip[uStart].vPosition * fEWt;
        CornerU += ip[uStart].vPosition * TANM( ( uTIndex % Val[index] ) * 2, index );
        CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex+uV)%Val[index]) * 2, index );
    }
    ++uStart;

    // Add in the last guy and make sure to wrap to the beginning if we're the last corner
    if (index == 3)
        uStart = 4; 
    CornerB += ip[uStart].vPosition * fOWt;
    CornerU += ip[uStart].vPosition * TANM( ( uTIndex % Val[index] ) * 2 + 1, index );
    CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex + uV ) % Val[index] ) * 2 + 1, index );

    // Add in the guy before the prefix as well
    if (index)
        uStart = PrefIm1-1;
    else
        uStart = Pref[3]-1;
    uTIndex = uTIndexStart-1;

    CornerB += ip[uStart].vPosition * fOWt;
    CornerU += ip[uStart].vPosition * TANM( ( uTIndex % Val[index] ) * 2 + 1, index );
    CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex + uV ) % Val[index] ) * 2 + 1, index );

    // We're done with the walk now.  Now we need to add the contributions of the original subd quad.
    CornerB += ip[MOD4(index+1)].vPosition * fEWt;
    CornerB += ip[MOD4(index+2)].vPosition * fOWt;
    CornerB += ip[MOD4(index+3)].vPosition * fEWt;
    
    uTIndex = 0 + (index&1)*(Val[index]-1);
    uStart = MOD4(index+1);
    CornerU += ip[uStart].vPosition * TANM( ( uTIndex % Val[index] ) * 2, index );
    CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex + uV ) % Val[index] ) * 2, index );
    
    uStart = MOD4(index+2);
    CornerU += ip[uStart].vPosition * TANM( ( uTIndex % Val[index] ) * 2 + 1, index );
    CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex + uV ) % Val[index] ) * 2 + 1, index );

    uStart = MOD4(index+3);
    uTIndex = (uTIndex+1)%Val[index];

    CornerU += ip[uStart].vPosition * TANM( ( uTIndex % Val[index] ) * 2, index );
    CornerV += ip[uStart].vPosition * TANM( ( ( uTIndex + uV ) % Val[index] ) * 2, index );

    // Normalize the corner weights
    CornerB *= 1.0f / ( Val[index] * Val[index] + 5 * Val[index] ); // normalize

    // fixup signs from directional derivatives...
    if( !((index - 1) & 2) ) // 1 and 2
        CornerU *= -1;

    if( index >= 2 ) // 2 and 3
        CornerV *= -1;
}


//--------------------------------------------------------------------------------------
// Computes the edge vertices of the output bicubic patch.  The edge vertices
// (1,2,4,7,8,11,13,14) are a weighted (by valence) combination of 6 interior and 1-ring
// neighborhood points.  However, we don't have to do the walk on this one since we
// don't need all of the neighbor points attached to this vertex.
//--------------------------------------------------------------------------------------
float3 ComputeEdgeVertex( in uint index /* 0-7 */, 
                          const in InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, 
                          const in uint Val[4], 
                          const in uint Pref[4] )
{
    float val1 = 2 * Val[0] + 10;
    float val2 = 2 * Val[1] + 10;
    float val13 = 2 * Val[3] + 10;
    float val14 = 2 * Val[2] + 10;
    float val4 = val1;
    float val8 = val13;
    float val7 = val2;
    float val11 = val14;
    
    float3 vRetVal = float3(0,0,0);
    switch( index )
    {
    // Horizontal
    case 0:
        vRetVal = (Val[0]*2*ip[0].vPosition + 4*ip[1].vPosition + ip[2].vPosition + ip[3].vPosition*2 +
              2*ip[Pref[0]-1].vPosition + ip[Pref[0]].vPosition) / val1;
        break;
    case 1:
        vRetVal = (4*ip[0].vPosition + Val[1]*2*ip[1].vPosition + ip[2].vPosition*2 + ip[3].vPosition +
              ip[Pref[0]-1].vPosition + 2*ip[Pref[0]].vPosition) / val2;
        break;
    case 2:
        vRetVal = (2*ip[0].vPosition + ip[1].vPosition + 4*ip[2].vPosition + ip[3].vPosition*2*Val[3] +
               2*ip[Pref[2]].vPosition + ip[Pref[2]-1].vPosition) / val13;
        break;
    case 3:
        vRetVal = (ip[0].vPosition + 2*ip[1].vPosition + Val[2]*2*ip[2].vPosition + ip[3].vPosition*4 +
               ip[Pref[2]].vPosition + 2*ip[Pref[2]-1].vPosition) / val14;
        break;
    // Vertical
    case 4:
        vRetVal = (Val[0]*2*ip[0].vPosition + 2*ip[1].vPosition + ip[2].vPosition + ip[3].vPosition*4 +
              2*ip[4].vPosition + ip[Pref[3]-1].vPosition) / val4;
        break;
    case 5:
        vRetVal = (4*ip[0].vPosition + ip[1].vPosition + 2*ip[2].vPosition + ip[3].vPosition*2*Val[3] +
              ip[4].vPosition + 2*ip[Pref[3]-1].vPosition) / val8;
        break;
    case 6:
        vRetVal = (2*ip[0].vPosition + Val[1]*2*ip[1].vPosition + 4*ip[2].vPosition + ip[3].vPosition +
              2*ip[Pref[1]-1].vPosition + ip[Pref[1]].vPosition) / val7;
        break;
    case 7:
        vRetVal = (ip[0].vPosition + 4*ip[1].vPosition + Val[2]*2*ip[2].vPosition + 2*ip[3].vPosition +
               ip[Pref[1]-1].vPosition + 2*ip[Pref[1]].vPosition) / val11;
        break;
    }
        
    return vRetVal;
}

//--------------------------------------------------------------------------------------
// Helper function
//--------------------------------------------------------------------------------------
void BezierRaise(inout float3 pQ[3], out float3 pC[4])
{
    pC[0] = pQ[0];
    pC[3] = pQ[2];

    for( int i=1; i<3; i++ ) 
    {
        pC[i] = ( 1.0f / 3.0f ) * ( pQ[i - 1] * i + ( 3.0f - i ) * pQ[i] );
    }
}


//--------------------------------------------------------------------------------------
// Skinning vertex shader Section
//--------------------------------------------------------------------------------------
struct VS_CONTROL_POINT_INPUT
{
    float3 vPosition		: POSITION;
    float2 vUV				: TEXCOORD0;
    float3 vTangent			: TANGENT;
    uint4  vBones			: BONES;
    float4 vWeights			: WEIGHTS;
};



//--------------------------------------------------------------------------------------
// SubD to Bezier hull shader Section
//--------------------------------------------------------------------------------------
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4]			: SV_TessFactor;
    float Inside[2]			: SV_InsideTessFactor;
    
    float3 vTangent[4]		: TANGENT;
    float2 vUV[4]			: TEXCOORD;
    float3 vTanUCorner[4]	: TANUCORNER;
    float3 vTanVCorner[4]	: TANVCORNER;
    float4 vCWts			: TANWEIGHTS;
};

//--------------------------------------------------------------------------------------
// Load per-patch valence and prefix data
//--------------------------------------------------------------------------------------
void LoadValenceAndPrefixData( in uint PatchID, out uint Val[4], out uint Prefixes[4] )
{
    PatchID += g_iPatchStartIndex;
    uint4 ValPack = g_ValencePrefixBuffer.Load( PatchID * 2 );
    uint4 PrefPack = g_ValencePrefixBuffer.Load( PatchID * 2 + 1 );
    
    Val[0] = ValPack.x;
    Val[1] = ValPack.y;
    Val[2] = ValPack.z;
    Val[3] = ValPack.w;
    
    Prefixes[0] = PrefPack.x;
    Prefixes[1] = PrefPack.y;
    Prefixes[2] = PrefPack.z;
    Prefixes[3] = PrefPack.w;
}


//--------------------------------------------------------------------------------------
// Constant data function for the SubDToBezierHS.  This is executed once per patch.
//--------------------------------------------------------------------------------------
HS_CONSTANT_DATA_OUTPUT SubDToBezierConstantsHS( InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip,
                                                 uint PatchID : SV_PrimitiveID )
{	
    HS_CONSTANT_DATA_OUTPUT Output;
    
    float TessAmount = g_fTessellationFactor;

    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount;
    Output.Inside[0] = Output.Inside[1] = TessAmount;
    
    Output.vTangent[0] = ip[0].vTangent;
    Output.vTangent[1] = ip[1].vTangent;
    Output.vTangent[2] = ip[2].vTangent;
    Output.vTangent[3] = ip[3].vTangent;
    
    Output.vUV[0] = ip[0].vUV;
    Output.vUV[1] = ip[1].vUV;
    Output.vUV[2] = ip[2].vUV;
    Output.vUV[3] = ip[3].vUV;
    
    // Compute part of our tangent patch here
    uint Val[4];
    uint Prefixes[4];
    LoadValenceAndPrefixData( PatchID, Val, Prefixes );

    [unroll]
    for( int i=0; i<4; i++ )
    {
        float3 CornerB, CornerU, CornerV;
        ComputeCornerVertex( i, CornerB, CornerU, CornerV, ip, Val, Prefixes );
        Output.vTanUCorner[i] = CornerU;
        Output.vTanVCorner[i] = CornerV;
    }
    
    float fCWts[4];
    Output.vCWts.x = g_fCi[ Val[0]-3 ];
    Output.vCWts.y = g_fCi[ Val[1]-3 ];
    Output.vCWts.z = g_fCi[ Val[2]-3 ];
    Output.vCWts.w = g_fCi[ Val[3]-3 ];
    
    return Output;
}

//--------------------------------------------------------------------------------------
// HS for SubDToBezier.  This outputcontrolpoints(16) specifies that we will produce
// 16 control points.  Therefore this function will be invoked 16x, one for each output
// control point.
//
// !! PERFORMANCE NOTE: This hull shader is written for maximum readability, and its
// performance is not expected to be optimal on D3D11 hardware.  The switch statement
// below that determines the codepath for each patch control point generates sub-optimal
// code for parallel execution on the GPU.  A future implementation of this hull shader
// will combine the 16 codepaths and 3 variants (corner, edge, interior) into one shared
// codepath; this change is expected to increase performance at the expense of readability.
//--------------------------------------------------------------------------------------
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("SubDToBezierConstantsHS")]
BEZIER_CONTROL_POINT main( InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> p, 
                                     uint i : SV_OutputControlPointID,
                                     uint PatchID : SV_PrimitiveID )
{
    // Valences and prefixes are loaded from a buffer
    uint Val[4];
    uint Prefixes[4];
    LoadValenceAndPrefixData( PatchID, Val, Prefixes );
    
    float3 CornerB = float3(0,0,0);
    float3 CornerU = float3(0,0,0);
    float3 CornerV = float3(0,0,0);
    
    BEZIER_CONTROL_POINT Output;
    Output.vPosition = float3(0,0,0);
    /*
    // !! PERFORMANCE NOTE: As mentioned above, this switch statement generates
    // inefficient code for the sake of readability.
    switch( i )
    {
    // Interior vertices
    case 5:
        Output.vPosition = ComputeInteriorVertex( 0, Val, p );
        break;
    case 6:
        Output.vPosition = ComputeInteriorVertex( 1, Val, p );
        break;
    case 10:
        Output.vPosition = ComputeInteriorVertex( 2, Val, p );
        break;
    case 9:
        Output.vPosition = ComputeInteriorVertex( 3, Val, p );
        break;
        
    // Corner vertices
    case 0:
        ComputeCornerVertex( 0, CornerB, CornerU, CornerV, p, Val, Prefixes );
        Output.vPosition = CornerB;
        break;
    case 3:
        ComputeCornerVertex( 1, CornerB, CornerU, CornerV, p, Val, Prefixes );
        Output.vPosition = CornerB;
        break;
    case 15:
        ComputeCornerVertex( 2, CornerB, CornerU, CornerV, p, Val, Prefixes );
        Output.vPosition = CornerB;
        break;
    case 12:
        ComputeCornerVertex( 3, CornerB, CornerU, CornerV, p, Val, Prefixes );
        Output.vPosition = CornerB;
        break;
        
    // Edge vertices
    case 1:
        Output.vPosition = ComputeEdgeVertex( 0, p, Val, Prefixes );
        break;
    case 2:
        Output.vPosition = ComputeEdgeVertex( 1, p, Val, Prefixes );
        break;
    case 13:
        Output.vPosition = ComputeEdgeVertex( 2, p, Val, Prefixes );
        break;
    case 14:
        Output.vPosition = ComputeEdgeVertex( 3, p, Val, Prefixes );
        break;
    case 4:
        Output.vPosition = ComputeEdgeVertex( 4, p, Val, Prefixes );
        break;
    case 8:
        Output.vPosition = ComputeEdgeVertex( 5, p, Val, Prefixes );
        break;
    case 7:
        Output.vPosition = ComputeEdgeVertex( 6, p, Val, Prefixes );
        break;
    case 11:
        Output.vPosition = ComputeEdgeVertex( 7, p, Val, Prefixes );
        break;
    }
    */
    return Output;
}
