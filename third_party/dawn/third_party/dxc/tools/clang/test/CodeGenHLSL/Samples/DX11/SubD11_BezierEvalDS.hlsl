// RUN: %dxc -E main -T ds_6_0 %s | FileCheck %s

// CHECK: domainLocation
// CHECK: domainLocation
// CHECK: loadPatchConstant
// CHECK: Rsqrt
// CHECK: sampleLevel
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
// Bezier evaluation domain shader section
//--------------------------------------------------------------------------------------
struct DS_OUTPUT
{
    float3 vWorldPos        : POSITION;
    float3 vNormal			: NORMAL;
    float2 vUV				: TEXCOORD;
    float3 vTangent			: TANGENT;
    float3 vBiTangent		: BITANGENT;
    
    float4 vPosition		: SV_POSITION;
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
float3 EvaluateBezier( const OutputPatch<BEZIER_CONTROL_POINT, 16> bezpatch,
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

//--------------------------------------------------------------------------------------
float3 EvaluateBezierTan( const float3 bezpatch[16],
                          float4 BasisU,
                          float4 BasisV )
{
    float3 Value = float3(0,0,0);
    Value  = BasisV.x * ( bezpatch[0] * BasisU.x + bezpatch[1] * BasisU.y + bezpatch[2] * BasisU.z + bezpatch[3] * BasisU.w );
    Value += BasisV.y * ( bezpatch[4] * BasisU.x + bezpatch[5] * BasisU.y + bezpatch[6] * BasisU.z + bezpatch[7] * BasisU.w );
    Value += BasisV.z * ( bezpatch[8] * BasisU.x + bezpatch[9] * BasisU.y + bezpatch[10] * BasisU.z + bezpatch[11] * BasisU.w );
    Value += BasisV.w * ( bezpatch[12] * BasisU.x + bezpatch[13] * BasisU.y + bezpatch[14] * BasisU.z + bezpatch[15] * BasisU.w );
    
    return Value;
}

//--------------------------------------------------------------------------------------
// Compute a two full tangent patches from the Tangent corner data created in the
// HS constant data function.
//--------------------------------------------------------------------------------------
void CreatTangentPatches( in HS_CONSTANT_DATA_OUTPUT input, 
                        const OutputPatch<BEZIER_CONTROL_POINT, 16> bezpatch,
                        out float3 TanU[16], 
                        out float3 TanV[16] )
{    
    TanV[0]  = input.vTanVCorner[0];
    TanV[3]  = input.vTanVCorner[1];
    TanV[15] = input.vTanVCorner[2];
    TanV[12] = input.vTanVCorner[3];
    
    TanU[0]  = input.vTanUCorner[0];
    TanU[3]  = input.vTanUCorner[1];
    TanU[15] = input.vTanUCorner[2];
    TanU[12] = input.vTanUCorner[3];
    
    float fCWts[4];
    fCWts[0] = input.vCWts.x;
    fCWts[1] = input.vCWts.y;
    fCWts[2] = input.vCWts.z;
    fCWts[3] = input.vCWts.w;

    float3 vCorner[4];
    float3 vCornerLocal[4];
    
    vCorner[0] = TanV[0];
    vCorner[1] = TanV[3];
    vCorner[2] = TanV[15];
    vCorner[3] = TanV[12];
    vCornerLocal[0] = TanU[0];
    vCornerLocal[1] = TanU[3];
    vCornerLocal[2] = TanU[12];
    vCornerLocal[3] = TanU[15];

    ComputeTanPatch( bezpatch, TanU, fCWts, vCorner, vCornerLocal, 1, 4 );

    fCWts[3] = input.vCWts.y;
    fCWts[1] = input.vCWts.w;

    vCorner[0] = TanU[0];
    vCorner[3] = TanU[3];
    vCorner[2] = TanU[15];
    vCorner[1] = TanU[12];
    vCornerLocal[0] = TanV[0];
    vCornerLocal[1] = TanV[12];
    vCornerLocal[2] = TanV[3];
    vCornerLocal[3] = TanV[15];

    ComputeTanPatch( bezpatch, TanV, fCWts, vCorner, vCornerLocal, 4, 1 );
}

//--------------------------------------------------------------------------------------
// For each input UV (from the Tessellator), evaluate the Bezier patch at this position.
//--------------------------------------------------------------------------------------
[domain("quad")]
DS_OUTPUT main( HS_CONSTANT_DATA_OUTPUT input, 
                        float2 UV : SV_DomainLocation,
                        const OutputPatch<BEZIER_CONTROL_POINT, 16> bezpatch )
{
    float4 BasisU = BernsteinBasis( UV.x );
    float4 BasisV = BernsteinBasis( UV.y );
    
    float3 WorldPos = EvaluateBezier( bezpatch, BasisU, BasisV );
    
    float3 TanU[16];
    float3 TanV[16];
    CreatTangentPatches( input, bezpatch, TanU, TanV );
    float3 Tangent = EvaluateBezierTan( TanU, BasisU, BasisV );
    float3 BiTangent = EvaluateBezierTan( TanV, BasisU, BasisV );
    
    // To see what the patch looks like without using the tangent patches to fix the normals, uncomment this section
    /*
    float4 dBasisU = dBernsteinBasis( UV.x );
    float4 dBasisV = dBernsteinBasis( UV.y );
    Tangent = EvaluateBezier( bezpatch, dBasisU, BasisV );
    BiTangent = EvaluateBezier( bezpatch, BasisU, dBasisV );
    */
    
    float3 Norm = normalize( cross( Tangent, BiTangent ) );

    DS_OUTPUT Output;
    Output.vNormal = Norm;
    
    // Evalulate the tangent vectors through bilinear interpolation.
    // These tangents are the texture-space tangents.  They should not be confused with the parametric
    // tangents that we use to get the normals for the bicubic patch.
    float3 TextureTanU0 = input.vTangent[0];
    float3 TextureTanU1 = input.vTangent[1];
    float3 TextureTanU2 = input.vTangent[2];
    float3 TextureTanU3 = input.vTangent[3];
    
    float3 UVbottom = lerp( TextureTanU0, TextureTanU1, UV.x );
    float3 UVtop = lerp( TextureTanU3, TextureTanU2, UV.x );
    float3 Tan = lerp( UVbottom, UVtop, UV.y );

    Output.vTangent = Tan;

    // This is an optimization.  We assume that the UV mapping of the mesh will result in a "relatively" orthogonal
    // tangent basis.  If we assume this, then we can avoid fetching and bilerping the BiTangent along with the tangent.
    Output.vBiTangent = cross( Norm, Tan );

    // bilerp the texture coordinates    
    float2 tex0 = input.vUV[0];
    float2 tex1 = input.vUV[1];
    float2 tex2 = input.vUV[2];
    float2 tex3 = input.vUV[3];
        
    float2 bottom = lerp( tex0, tex1, UV.x );
    float2 top = lerp( tex3, tex2, UV.x );
    float2 TexUV = lerp( bottom, top, UV.y );
    Output.vUV = TexUV;
    
    if( g_fDisplacementHeight > 0 )
    {
        // On this sample displacement can go into or out of the mesh.  This is why we bias the heigh amount.
        float height = g_fDisplacementHeight * ( g_txHeight.SampleLevel( g_samPoint, TexUV, 0 ).a * 2 - 1 );
        float3 WorldPosMiddle = Norm * height;
        WorldPos += WorldPosMiddle;
    }
    
    Output.vPosition = mul( float4(WorldPos,1), g_mViewProjection );
    Output.vWorldPos = WorldPos;
    
    return Output;    
}
