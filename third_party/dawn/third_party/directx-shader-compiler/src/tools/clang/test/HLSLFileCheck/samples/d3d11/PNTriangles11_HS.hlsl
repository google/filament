// RUN: %dxc -E main -T hs_6_0 %s | FileCheck %s

// CHECK: dot3
// CHECK: Rsqrt
// CHECK: dot3
// CHECK: storePatchConstant
// CHECK: main
// CHECK: outputControlPointID
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: PNTriangles11.hlsl
//
// These shaders implement the PN-Triangles tessellation technique
//
// Contributed by the AMD Developer Relations Team
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "AdaptiveTessellation.hlsli"


//--------------------------------------------------------------------------------------
// Constant buffer
//--------------------------------------------------------------------------------------

cbuffer cbPNTriangles : register( b0 )
{
    float4x4    g_f4x4World;                // World matrix for object
    float4x4    g_f4x4ViewProjection;       // View * Projection matrix
    float4x4    g_f4x4WorldViewProjection;  // World * View * Projection matrix
    float4      g_f4LightDir;               // Light direction vector
    float4      g_f4Eye;                    // Eye
    float4      g_f4ViewVector;             // View Vector
    float4      g_f4TessFactors;            // Tessellation factors ( x=Edge, y=Inside, z=MinDistance, w=Range )
    float4      g_f4ScreenParams;           // Screen resolution ( x=Current width, y=Current height )
    float4      g_f4GUIParams1;             // GUI params1 ( x=BackFace Epsilon, y=Silhouette Epsilon, z=Range scale, w=Edge size )
    float4      g_f4GUIParams2;             // GUI params2 ( x=Screen resolution scale, y=View Frustum Epsilon )
    float4      g_f4ViewFrustumPlanes[4];   // View frustum planes ( x=left, y=right, z=top, w=bottom )
}

// Some global lighting constants
static float4 g_f4MaterialDiffuseColor  = float4( 1.0f, 1.0f, 1.0f, 1.0f );
static float4 g_f4LightDiffuse          = float4( 1.0f, 1.0f, 1.0f, 1.0f );
static float4 g_f4MaterialAmbientColor  = float4( 0.2f, 0.2f, 0.2f, 1.0f );

// Some global epsilons for adaptive tessellation
static float g_fMaxScreenWidth = 2560.0f;
static float g_fMaxScreenHeight = 1600.0f;


//--------------------------------------------------------------------------------------
// Buffers, Textures and Samplers
//--------------------------------------------------------------------------------------

// Textures
Texture2D g_txDiffuse : register( t0 );

// Samplers
SamplerState g_SamplePoint  : register( s0 );
SamplerState g_SampleLinear : register( s1 );


//--------------------------------------------------------------------------------------
// Shader structures
//--------------------------------------------------------------------------------------

struct VS_RenderSceneInput
{
    float3 f3Position   : POSITION;  
    float3 f3Normal     : NORMAL;     
    float2 f2TexCoord   : TEXCOORD;
};

struct HS_Input
{
    float3 f3Position   : POSITION;
    float3 f3Normal     : NORMAL;
    float2 f2TexCoord   : TEXCOORD;
};

struct HS_ConstantOutput
{
    // Tess factor for the FF HW block
    float fTessFactor[3]    : SV_TessFactor;
    float fInsideTessFactor : SV_InsideTessFactor;
    
    // Geometry cubic generated control points
    float3 f3B210    : POSITION3;
    float3 f3B120    : POSITION4;
    float3 f3B021    : POSITION5;
    float3 f3B012    : POSITION6;
    float3 f3B102    : POSITION7;
    float3 f3B201    : POSITION8;
    float3 f3B111    : CENTER;
    
    // Normal quadratic generated control points
    float3 f3N110    : NORMAL3;      
    float3 f3N011    : NORMAL4;
    float3 f3N101    : NORMAL5;
};

struct HS_ControlPointOutput
{
    float3 f3Position    : POSITION;
    float3 f3Normal      : NORMAL;
    float2 f2TexCoord    : TEXCOORD;
};

struct DS_Output
{
    float4 f4Position   : SV_Position;
    float2 f2TexCoord   : TEXCOORD0;
    float4 f4Diffuse    : COLOR0;
};

struct PS_RenderSceneInput
{
    float4 f4Position   : SV_Position;
    float2 f2TexCoord   : TEXCOORD0;
    float4 f4Diffuse    : COLOR0;
};

struct PS_RenderOutput
{
    float4 f4Color      : SV_Target0;
};

//--------------------------------------------------------------------------------------
// This hull shader passes the tessellation factors through to the HW tessellator, 
// and the 10 (geometry), 6 (normal) control points of the PN-triangular patch to the domain shader
//--------------------------------------------------------------------------------------
HS_ConstantOutput HS_PNTrianglesConstant( InputPatch<HS_Input, 3> I )
{
    HS_ConstantOutput O = (HS_ConstantOutput)0;
    bool bViewFrustumCull = false;
    bool bBackFaceCull = false;
    float fEdgeDot[3];
    
    #ifdef USE_VIEW_FRUSTUM_CULLING

        // Perform view frustum culling test
        bViewFrustumCull = ViewFrustumCull( I[0].f3Position, I[1].f3Position, I[2].f3Position, g_f4ViewFrustumPlanes, g_f4GUIParams2.y );
                    
    #endif

    #ifdef USE_BACK_FACE_CULLING

        // Perform back face culling test
        
        // Aquire patch edge dot product between patch edge normal and view vector 
        fEdgeDot[0] = GetEdgeDotProduct( I[2].f3Normal, I[0].f3Normal, g_f4ViewVector.xyz );
        fEdgeDot[1] = GetEdgeDotProduct( I[0].f3Normal, I[1].f3Normal, g_f4ViewVector.xyz );
        fEdgeDot[2] = GetEdgeDotProduct( I[1].f3Normal, I[2].f3Normal, g_f4ViewVector.xyz );

        // If all 3 fail the test then back face cull
        bBackFaceCull = BackFaceCull( fEdgeDot[0], fEdgeDot[1], fEdgeDot[2], g_f4GUIParams1.x );

    #endif

    // Skip the rest of the function if culling
    if( !bViewFrustumCull && !bBackFaceCull )
    {
        // Use the tessellation factors as defined in constant space 
        O.fTessFactor[0] = O.fTessFactor[1] = O.fTessFactor[2] = g_f4TessFactors.x;
        float fAdaptiveScaleFactor;
                
        #if defined( USE_SCREEN_SPACE_ADAPTIVE_TESSELLATION )

            // Get the screen space position of each control point, so we can compute the 
            // desired tess factor based upon an ideal primitive size
            float2 f2EdgeScreenPosition0 = GetScreenSpacePosition( I[0].f3Position, g_f4x4ViewProjection,  g_f4ScreenParams.x,  g_f4ScreenParams.y );
            float2 f2EdgeScreenPosition1 = GetScreenSpacePosition( I[1].f3Position, g_f4x4ViewProjection,  g_f4ScreenParams.x,  g_f4ScreenParams.y );
            float2 f2EdgeScreenPosition2 = GetScreenSpacePosition( I[2].f3Position, g_f4x4ViewProjection,  g_f4ScreenParams.x,  g_f4ScreenParams.y );
            // Edge 0
            fAdaptiveScaleFactor = GetScreenSpaceAdaptiveScaleFactor( f2EdgeScreenPosition2, f2EdgeScreenPosition0, g_f4TessFactors.x, g_f4GUIParams1.w );
            O.fTessFactor[0] = lerp( 1.0f, O.fTessFactor[0], fAdaptiveScaleFactor ); 
            // Edge 1
            fAdaptiveScaleFactor = GetScreenSpaceAdaptiveScaleFactor( f2EdgeScreenPosition0, f2EdgeScreenPosition1, g_f4TessFactors.x, g_f4GUIParams1.w );
            O.fTessFactor[1] = lerp( 1.0f, O.fTessFactor[1], fAdaptiveScaleFactor ); 
            // Edge 2
            fAdaptiveScaleFactor = GetScreenSpaceAdaptiveScaleFactor( f2EdgeScreenPosition1, f2EdgeScreenPosition2, g_f4TessFactors.x, g_f4GUIParams1.w );
            O.fTessFactor[2] = lerp( 1.0f, O.fTessFactor[2], fAdaptiveScaleFactor ); 

        #else
        
            #if defined( USE_DISTANCE_ADAPTIVE_TESSELLATION )
        
                // Perform distance adaptive tessellation per edge
                // Edge 0
                fAdaptiveScaleFactor = GetDistanceAdaptiveScaleFactor(    g_f4Eye.xyz, I[2].f3Position, I[0].f3Position, g_f4TessFactors.z, g_f4TessFactors.w * g_f4GUIParams1.z );
                O.fTessFactor[0] = lerp( 1.0f, O.fTessFactor[0], fAdaptiveScaleFactor ); 
                // Edge 1
                fAdaptiveScaleFactor = GetDistanceAdaptiveScaleFactor(    g_f4Eye.xyz, I[0].f3Position, I[1].f3Position, g_f4TessFactors.z, g_f4TessFactors.w * g_f4GUIParams1.z );
                O.fTessFactor[1] = lerp( 1.0f, O.fTessFactor[1], fAdaptiveScaleFactor ); 
                // Edge 2
                fAdaptiveScaleFactor = GetDistanceAdaptiveScaleFactor(    g_f4Eye.xyz, I[1].f3Position, I[2].f3Position, g_f4TessFactors.z, g_f4TessFactors.w * g_f4GUIParams1.z );
                O.fTessFactor[2] = lerp( 1.0f, O.fTessFactor[2], fAdaptiveScaleFactor ); 
            
            #endif

            #if defined( USE_SCREEN_RESOLUTION_ADAPTIVE_TESSELLATION )

                // Use screen resolution as a global scaling factor
                // Edge 0
                fAdaptiveScaleFactor = GetScreenResolutionAdaptiveScaleFactor( g_f4ScreenParams.x, g_f4ScreenParams.y, g_fMaxScreenWidth * g_f4GUIParams2.x, g_fMaxScreenHeight * g_f4GUIParams2.x );
                O.fTessFactor[0] = lerp( 1.0f, O.fTessFactor[0], fAdaptiveScaleFactor ); 
                // Edge 1
                fAdaptiveScaleFactor = GetScreenResolutionAdaptiveScaleFactor( g_f4ScreenParams.x, g_f4ScreenParams.y, g_fMaxScreenWidth * g_f4GUIParams2.x, g_fMaxScreenHeight * g_f4GUIParams2.x );
                O.fTessFactor[1] = lerp( 1.0f, O.fTessFactor[1], fAdaptiveScaleFactor ); 
                // Edge 2
                fAdaptiveScaleFactor = GetScreenResolutionAdaptiveScaleFactor( g_f4ScreenParams.x, g_f4ScreenParams.y, g_fMaxScreenWidth * g_f4GUIParams2.x, g_fMaxScreenHeight * g_f4GUIParams2.x );
                O.fTessFactor[2] = lerp( 1.0f, O.fTessFactor[2], fAdaptiveScaleFactor ); 

            #endif

        #endif

        #ifdef USE_ORIENTATION_ADAPTIVE_TESSELLATION

            #ifndef USE_BACK_FACE_CULLING

                // If back face culling is not used, then aquire patch edge dot product
                // between patch edge normal and view vector 
                fEdgeDot[0] = GetEdgeDotProduct( I[2].f3Normal, I[0].f3Normal, g_f4ViewVector.xyz );
                fEdgeDot[1] = GetEdgeDotProduct( I[0].f3Normal, I[1].f3Normal, g_f4ViewVector.xyz );
                fEdgeDot[2] = GetEdgeDotProduct( I[1].f3Normal, I[2].f3Normal, g_f4ViewVector.xyz );    

            #endif

            // Scale the tessellation factors based on patch orientation with respect to the viewing
            // vector
            // Edge 0
            fAdaptiveScaleFactor = GetOrientationAdaptiveScaleFactor( fEdgeDot[0], g_f4GUIParams1.y );
            float fTessFactor0 = lerp( 1.0f, g_f4TessFactors.x, fAdaptiveScaleFactor ); 
            // Edge 1
            fAdaptiveScaleFactor = GetOrientationAdaptiveScaleFactor( fEdgeDot[1], g_f4GUIParams1.y );
            float fTessFactor1 = lerp( 1.0f, g_f4TessFactors.x, fAdaptiveScaleFactor ); 
            // Edge 2
            fAdaptiveScaleFactor = GetOrientationAdaptiveScaleFactor( fEdgeDot[2], g_f4GUIParams1.y );
            float fTessFactor2 = lerp( 1.0f, g_f4TessFactors.x, fAdaptiveScaleFactor ); 

            #if defined( USE_SCREEN_SPACE_ADAPTIVE_TESSELLATION ) || defined( USE_DISTANCE_ADAPTIVE_TESSELLATION )

                O.fTessFactor[0] = ( O.fTessFactor[0] + fTessFactor0 ) / 2.0f;    
                O.fTessFactor[1] = ( O.fTessFactor[1] + fTessFactor1 ) / 2.0f;    
                O.fTessFactor[2] = ( O.fTessFactor[2] + fTessFactor2 ) / 2.0f;    

            #else
            
                O.fTessFactor[0] = fTessFactor0;    
                O.fTessFactor[1] = fTessFactor1;    
                O.fTessFactor[2] = fTessFactor2;    

            #endif
                                            
        #endif
        
        // Now setup the PNTriangle control points...

        // Assign Positions
        float3 f3B003 = I[0].f3Position;
        float3 f3B030 = I[1].f3Position;
        float3 f3B300 = I[2].f3Position;
        // And Normals
        float3 f3N002 = I[0].f3Normal;
        float3 f3N020 = I[1].f3Normal;
        float3 f3N200 = I[2].f3Normal;
            
        // Compute the cubic geometry control points
        // Edge control points
        O.f3B210 = ( ( 2.0f * f3B003 ) + f3B030 - ( dot( ( f3B030 - f3B003 ), f3N002 ) * f3N002 ) ) / 3.0f;
        O.f3B120 = ( ( 2.0f * f3B030 ) + f3B003 - ( dot( ( f3B003 - f3B030 ), f3N020 ) * f3N020 ) ) / 3.0f;
        O.f3B021 = ( ( 2.0f * f3B030 ) + f3B300 - ( dot( ( f3B300 - f3B030 ), f3N020 ) * f3N020 ) ) / 3.0f;
        O.f3B012 = ( ( 2.0f * f3B300 ) + f3B030 - ( dot( ( f3B030 - f3B300 ), f3N200 ) * f3N200 ) ) / 3.0f;
        O.f3B102 = ( ( 2.0f * f3B300 ) + f3B003 - ( dot( ( f3B003 - f3B300 ), f3N200 ) * f3N200 ) ) / 3.0f;
        O.f3B201 = ( ( 2.0f * f3B003 ) + f3B300 - ( dot( ( f3B300 - f3B003 ), f3N002 ) * f3N002 ) ) / 3.0f;
        // Center control point
        float3 f3E = ( O.f3B210 + O.f3B120 + O.f3B021 + O.f3B012 + O.f3B102 + O.f3B201 ) / 6.0f;
        float3 f3V = ( f3B003 + f3B030 + f3B300 ) / 3.0f;
        O.f3B111 = f3E + ( ( f3E - f3V ) / 2.0f );
        
        // Compute the quadratic normal control points, and rotate into world space
        float fV12 = 2.0f * dot( f3B030 - f3B003, f3N002 + f3N020 ) / dot( f3B030 - f3B003, f3B030 - f3B003 );
        O.f3N110 = normalize( f3N002 + f3N020 - fV12 * ( f3B030 - f3B003 ) );
        float fV23 = 2.0f * dot( f3B300 - f3B030, f3N020 + f3N200 ) / dot( f3B300 - f3B030, f3B300 - f3B030 );
        O.f3N011 = normalize( f3N020 + f3N200 - fV23 * ( f3B300 - f3B030 ) );
        float fV31 = 2.0f * dot( f3B003 - f3B300, f3N200 + f3N002 ) / dot( f3B003 - f3B300, f3B003 - f3B300 );
        O.f3N101 = normalize( f3N200 + f3N002 - fV31 * ( f3B003 - f3B300 ) );
    }
    else
    {
        // Cull the patch
        O.fTessFactor[0] = 0.0f;
        O.fTessFactor[1] = 0.0f;
        O.fTessFactor[2] = 0.0f;
    }

    // Inside tess factor is just the average of the edge factors
    O.fInsideTessFactor = ( O.fTessFactor[0] + O.fTessFactor[1] + O.fTessFactor[2] ) / 3.0f;
               
    return O;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[patchconstantfunc("HS_PNTrianglesConstant")]
[outputcontrolpoints(3)]
[maxtessfactor(9)]
HS_ControlPointOutput main( InputPatch<HS_Input, 3> I, uint uCPID : SV_OutputControlPointID )
{
    HS_ControlPointOutput O = (HS_ControlPointOutput)0;

    // Just pass through inputs = fast pass through mode triggered
    O.f3Position = I[uCPID].f3Position;
    O.f3Normal = I[uCPID].f3Normal;
    O.f2TexCoord = I[uCPID].f2TexCoord;
    
    return O;
}



//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
