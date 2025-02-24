// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: loadInput
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

void ComputeCornerVertex4444( uint index, 
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
// Computes the tangent patch from the input bezier patch
//--------------------------------------------------------------------------------------
void ComputeTanPatch( const OutputPatch<BEZIER_CONTROL_POINT, 16> bezpatch, 
                      inout float3 vOut[16], 
                      in float fCWts[4], 
                      in float3 vCorner[4], 
                      in float3 vCornerLocal[4], 
                      in const uint cX, 
                      in const uint cY)
{
    float3 vQuad[3];
    float3 vQuadB[3];
    float3 vCubic[4];

    // boundary edges are really simple...
    vQuad[0] = vCornerLocal[0];
    vQuad[2] = vCornerLocal[1];
    vQuad[1] = 3.0f*(bezpatch[2*cX+0*cY].vPosition-bezpatch[1*cX+0*cY].vPosition);

    BezierRaise(vQuad,vCubic);
    vOut[1*cX + 0*cY] = vCubic[1];
    vOut[2*cX + 0*cY] = vCubic[2];

    vQuad[0] = vCornerLocal[2];
    vQuad[2] = vCornerLocal[3];
    vQuad[1] = 3.0f*(bezpatch[2*cX+3*cY].vPosition-bezpatch[1*cX+3*cY].vPosition);

    BezierRaise(vQuad,vCubic);
    vOut[1*cX + 3*cY] = vCubic[1];
    vOut[2*cX + 3*cY] = vCubic[2];

    // two internal edges - this is where work happens...
    float3 vA,vB,vC,vD,vE;
    float fC0,fC1;
    vQuad[1] = 3.0f*(bezpatch[2*cX+2*cY].vPosition-bezpatch[1*cX+2*cY].vPosition);
    // also do "second" scan line
    vQuadB[1] = 3.0f*(bezpatch[2*cX+1*cY].vPosition-bezpatch[1*cX+1*cY].vPosition);

    vD = 3.0f*(bezpatch[1*cX + 2*cY].vPosition - bezpatch[0*cX + 2*cY].vPosition);
    vE = 3.0f*(bezpatch[1*cX + 1*cY].vPosition - bezpatch[0*cX + 1*cY].vPosition); // used later...

    fC0 = fCWts[3];
    fC1 = fCWts[0];

    // sign flip
    vA = -vCorner[3];
    vB = 3.0f*(bezpatch[0*cX + 1*cY].vPosition - bezpatch[0*cX + 2*cY].vPosition);
    vC = -vCorner[0];

    vQuad[0] = 1.0f/3.0f*(2.0f*fC0*vB - fC1*vA) + vD;
    vQuadB[0] = 1.0f/3.0f*(fC0*vC - 2.0f*fC1*vB) + vE;

    // do end of strip - same as before, but stuff is switched around...
    vC = vCorner[2];
    vB = 3.0f*(bezpatch[3*cX + 2*cY].vPosition - bezpatch[3*cX + 1*cY].vPosition);
    vA = vCorner[1];

    vD = 3.0f*(bezpatch[2*cX + 1*cY].vPosition - bezpatch[3*cX + 1*cY].vPosition);
    vE = 3.0f*(bezpatch[2*cX + 2*cY].vPosition - bezpatch[3*cX + 2*cY].vPosition);
    
    fC0 = fCWts[1];
    fC1 = fCWts[2];
 
    vQuadB[2] = 1.0f/3.0f*(2.0f*fC0*vB - fC1*vA) + vD;
    vQuad[2] = 1.0f/3.0f*(fC0*vC - 2.0f*fC1*vB) + vE;

    vQuadB[2] *= -1.0f;
    vQuad[2] *= -1.0f;

    BezierRaise(vQuad,vCubic);

    vOut[0*cX + 2*cY] = vCubic[0];
    vOut[1*cX + 2*cY] = vCubic[1];
    vOut[2*cX + 2*cY] = vCubic[2];
    vOut[3*cX + 2*cY] = vCubic[3];

    BezierRaise(vQuadB,vCubic);

    vOut[0*cX + 1*cY] = vCubic[0];
    vOut[1*cX + 1*cY] = vCubic[1];
    vOut[2*cX + 1*cY] = vCubic[2];
    vOut[3*cX + 1*cY] = vCubic[3];
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

VS_CONTROL_POINT_OUTPUT main( VS_CONTROL_POINT_INPUT Input )
{
    VS_CONTROL_POINT_OUTPUT Output;
    
    float4 vInputPos = float4( Input.vPosition, 1 );
    float4 vWorldPos = float4( 0, 0, 0, 0 );
    
    vWorldPos += mul( vInputPos, g_mConstBoneWorld[ Input.vBones.x ] ) * Input.vWeights.x;
    vWorldPos += mul( vInputPos, g_mConstBoneWorld[ Input.vBones.y ] ) * Input.vWeights.y;
    vWorldPos += mul( vInputPos, g_mConstBoneWorld[ Input.vBones.z ] ) * Input.vWeights.z;
    vWorldPos += mul( vInputPos, g_mConstBoneWorld[ Input.vBones.w ] ) * Input.vWeights.w;
    
    float3 vWorldTan = float3( 0, 0, 0 );
    vWorldTan += mul( Input.vTangent, (float3x3)g_mConstBoneWorld[ Input.vBones.x ] ) * Input.vWeights.x;
    vWorldTan += mul( Input.vTangent, (float3x3)g_mConstBoneWorld[ Input.vBones.y ] ) * Input.vWeights.y;
    vWorldTan += mul( Input.vTangent, (float3x3)g_mConstBoneWorld[ Input.vBones.z ] ) * Input.vWeights.z;
    vWorldTan += mul( Input.vTangent, (float3x3)g_mConstBoneWorld[ Input.vBones.w ] ) * Input.vWeights.w;
    
    Output.vPosition = vWorldPos;
    Output.vUV = Input.vUV;
    Output.vTangent = vWorldTan;
    
    return Output;
}
