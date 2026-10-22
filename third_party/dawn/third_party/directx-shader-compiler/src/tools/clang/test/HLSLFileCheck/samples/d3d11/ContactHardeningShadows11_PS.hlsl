// RUN: %dxc -E main -T ps_6_0 -O0 -HV 2018 %s | FileCheck %s

// CHECK: Round_ni
// CHECK: textureGather
// CHECK: dot4
// CHECK: dot4
// CHECK: textureGatherCmp
// CHECK: Saturate
// CHECK: sample
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: ContactHardeningShadows11.hlsl
//
// These shaders demonstrate the use of the DX11 sm5 instructions
// for fast high quality contact hardening shadows
//
// Contributed by AMD Developer Relations Team
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//--------------------------------------------------------------------------------------

cbuffer cbConstants : register( b0 )
{
    float4x4 g_f4x4WorldViewProjection;        // World * View * Projection matrix
    float4x4 g_f4x4WorldViewProjLight;        // World * ViewLight * Projection Light matrix
    float4   g_vShadowMapDimensions;
    float4   g_f4LightDir;
    float    g_fSunWidth;
    float3   g_f3Padding;
}

//======================================================================================
// Textures and Samplers
//======================================================================================

// Textures
Texture2D         g_txScene     : register( t0 );
Texture2D<float>  g_txShadowMap : register( t1 );

// Samplers
SamplerState                g_SamplePoint       : register( s0 );
SamplerState                g_SampleLinear      : register( s1 );
SamplerComparisonState      g_SamplePointCmp    : register( s2 );

//======================================================================================
// Vertex & Pixel shader structures
//======================================================================================

struct VS_RenderSceneInput
{
    float3 f3Position    : POSITION;  
    float3 f3Normal      : NORMAL;     
    float2 f2TexCoord    : TEXTURE0;
};

struct PS_RenderSceneInput
{
    float4 f4Position   : SV_Position;
    float4 f4Diffuse    : COLOR0; 
    float2 f2TexCoord   : TEXTURE0;
    float4 f4SMC        : TEXTURE1;
};

struct PS_RenderOutput
{
    float4 f4Color    : SV_Target0;
};

#define FILTER_SIZE    11
#define FS  FILTER_SIZE
#define FS2 ( FILTER_SIZE / 2 )

// 4 control matrices for a dynamic cubic bezier filter weights matrix

static const float C3[11][11] = 
                 { { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 }, 
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   { 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0 },
                   };

static const float C2[11][11] = 
                 { { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 }, 
                   { 0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,1.0,1.0,1.0,1.0,1.0,1.0,1.0,0.2,0.0 },
                   { 0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   };

static const float C1[11][11] = 
                 { { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 }, 
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,1.0,1.0,1.0,1.0,1.0,0.2,0.0,0.0 },
                   { 0.0,0.0,0.2,0.2,0.2,0.2,0.2,0.2,0.2,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   };

static const float C0[11][11] = 
                 { { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 }, 
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.8,0.8,0.8,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.8,1.0,0.8,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.8,0.8,0.8,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0 },
                   };

// compute dynamic weight at a certain row, column of the matrix
float Fw( int r, int c, float fL )
{
    return (1.0-fL)*(1.0-fL)*(1.0-fL) * C0[r][c] +
           fL*fL*fL * C3[r][c] +
           3.0f * (1.0-fL)*(1.0-fL)*fL * C1[r][c]+
           3.0f * fL*fL*(1.0-fL) * C2[r][c];
} 

#define BLOCKER_FILTER_SIZE    11
#define BFS  BLOCKER_FILTER_SIZE
#define BFS2 ( BLOCKER_FILTER_SIZE / 2 )

#define SUN_WIDTH g_fSunWidth
   
//======================================================================================
// This shader computes the contact hardening shadow filter
//======================================================================================
float shadow( float3 tc )
{
    float  s   = 0.0f;
    float2 stc = ( g_vShadowMapDimensions.xy * tc.xy ) + float2( 0.5, 0.5 );
    float2 tcs = floor( stc );
    float2 fc;
    int    row;
    int    col;
    float  w = 0.0;
    float  avgBlockerDepth = 0;
    float  blockerCount = 0;
    float  fRatio;
    float4 v1[ FS2 + 1 ];
    float2 v0[ FS2 + 1 ];
    float2 off;

    fc     = stc - tcs;
    tc.xy  = tc - ( fc * g_vShadowMapDimensions.zw );

    // find number of blockers and sum up blocker depth
    for( row = -BFS2; row <= BFS2; row += 2 )
    {
        for( col = -BFS2; col <= BFS2; col += 2 )
        {
            float4 d4 = g_txShadowMap.GatherRed( g_SamplePoint, tc.xy, int2( col, row ) );
            float4 b4  = ( tc.zzzz <= d4 ) ? (0.0).xxxx : (1.0).xxxx;   

            blockerCount += dot( b4, (1.0).xxxx );
            avgBlockerDepth += dot( d4, b4 );
        }
    }

    // compute ratio using formulas from PCSS
    if( blockerCount > 0.0 )
    {
        avgBlockerDepth /= blockerCount;
        fRatio = saturate( ( ( tc.z - avgBlockerDepth ) * SUN_WIDTH ) / avgBlockerDepth );
        fRatio *= fRatio;
    }
    else
    {
        fRatio = 0.0; 
    }

    // sum up weights of dynamic filter matrix
    for( row = 0; row < FS; ++row )
    {
       for( col = 0; col < FS; ++col )
       {
          w += Fw(row,col,fRatio);
       }
    }

    // filter shadow map samples using the dynamic weights
    [unroll(FILTER_SIZE)]for( row = -FS2; row <= FS2; row += 2 )
    {
        for( col = -FS2; col <= FS2; col += 2 )
        {
            v1[(col+FS2)/2] = g_txShadowMap.GatherCmpRed( g_SamplePointCmp, tc.xy, tc.z, 
                                                          int2( col, row ) );
          
            if( col == -FS2 )
            {
                s += ( 1 - fc.y ) * ( v1[0].w * ( Fw(row+FS2,0,fRatio) - 
                                      Fw(row+FS2,0,fRatio) * fc.x ) + v1[0].z * 
                                    ( fc.x * ( Fw(row+FS2,0,fRatio) - 
                                      Fw(row+FS2,1,fRatio) ) +  
                                      Fw(row+FS2,1,fRatio) ) );
                s += (     fc.y ) * ( v1[0].x * ( Fw(row+FS2,0,fRatio) - 
                                      Fw(row+FS2,0,fRatio) * fc.x ) + 
                                      v1[0].y * ( fc.x * ( Fw(row+FS2,0,fRatio) - 
                                      Fw(row+FS2,1,fRatio) ) +  
                                      Fw(row+FS2,1,fRatio) ) );
                if( row > -FS2 )
                {
                    s += ( 1 - fc.y ) * ( v0[0].x * ( Fw(row+FS2-1,0,fRatio) - 
                                          Fw(row+FS2-1,0,fRatio) * fc.x ) + v0[0].y * 
                                        ( fc.x * ( Fw(row+FS2-1,0,fRatio) - 
                                          Fw(row+FS2-1,1,fRatio) ) +  
                                          Fw(row+FS2-1,1,fRatio) ) );
                    s += (     fc.y ) * ( v1[0].w * ( Fw(row+FS2-1,0,fRatio) - 
                                          Fw(row+FS2-1,0,fRatio) * fc.x ) + v1[0].z * 
                                        ( fc.x * ( Fw(row+FS2-1,0,fRatio) - 
                                          Fw(row+FS2-1,1,fRatio) ) +  
                                          Fw(row+FS2-1,1,fRatio) ) );
                }
            }
            else if( col == FS2 )
            {
                s += ( 1 - fc.y ) * ( v1[FS2].w * ( fc.x * ( Fw(row+FS2,FS-2,fRatio) - 
                                      Fw(row+FS2,FS-1,fRatio) ) + 
                                      Fw(row+FS2,FS-1,fRatio) ) + v1[FS2].z * fc.x * 
                                      Fw(row+FS2,FS-1,fRatio) );
                s += (     fc.y ) * ( v1[FS2].x * ( fc.x * ( Fw(row+FS2,FS-2,fRatio) - 
                                      Fw(row+FS2,FS-1,fRatio) ) + 
                                      Fw(row+FS2,FS-1,fRatio) ) + v1[FS2].y * fc.x * 
                                      Fw(row+FS2,FS-1,fRatio) );
                if( row > -FS2 )
                {
                    s += ( 1 - fc.y ) * ( v0[FS2].x * ( fc.x * 
                                        ( Fw(row+FS2-1,FS-2,fRatio) - 
                                          Fw(row+FS2-1,FS-1,fRatio) ) + 
                                          Fw(row+FS2-1,FS-1,fRatio) ) + 
                                          v0[FS2].y * fc.x * Fw(row+FS2-1,FS-1,fRatio) );
                    s += (     fc.y ) * ( v1[FS2].w * ( fc.x * 
                                        ( Fw(row+FS2-1,FS-2,fRatio) - 
                                          Fw(row+FS2-1,FS-1,fRatio) ) + 
                                          Fw(row+FS2-1,FS-1,fRatio) ) + 
                                          v1[FS2].z * fc.x * Fw(row+FS2-1,FS-1,fRatio) );
                }
            }
            else
            {
                s += ( 1 - fc.y ) * ( v1[(col+FS2)/2].w * ( fc.x * 
                                    ( Fw(row+FS2,col+FS2-1,fRatio) - 
                                      Fw(row+FS2,col+FS2+0,fRatio) ) + 
                                      Fw(row+FS2,col+FS2+0,fRatio) ) +
                                      v1[(col+FS2)/2].z * ( fc.x * 
                                    ( Fw(row+FS2,col+FS2-0,fRatio) - 
                                      Fw(row+FS2,col+FS2+1,fRatio) ) + 
                                      Fw(row+FS2,col+FS2+1,fRatio) ) );
                s += (     fc.y ) * ( v1[(col+FS2)/2].x * ( fc.x * 
                                    ( Fw(row+FS2,col+FS2-1,fRatio) - 
                                      Fw(row+FS2,col+FS2+0,fRatio) ) + 
                                      Fw(row+FS2,col+FS2+0,fRatio) ) +
                                      v1[(col+FS2)/2].y * ( fc.x * 
                                    ( Fw(row+FS2,col+FS2-0,fRatio) - 
                                      Fw(row+FS2,col+FS2+1,fRatio) ) + 
                                      Fw(row+FS2,col+FS2+1,fRatio) ) );
                if( row > -FS2 )
                {
                    s += ( 1 - fc.y ) * ( v0[(col+FS2)/2].x * ( fc.x * 
                                        ( Fw(row+FS2-1,col+FS2-1,fRatio) - 
                                          Fw(row+FS2-1,col+FS2+0,fRatio) ) + 
                                          Fw(row+FS2-1,col+FS2+0,fRatio) ) +
                                          v0[(col+FS2)/2].y * ( fc.x * 
                                        ( Fw(row+FS2-1,col+FS2-0,fRatio) - 
                                          Fw(row+FS2-1,col+FS2+1,fRatio) ) + 
                                          Fw(row+FS2-1,col+FS2+1,fRatio) ) );
                    s += (     fc.y ) * ( v1[(col+FS2)/2].w * ( fc.x * 
                                        ( Fw(row+FS2-1,col+FS2-1,fRatio) - 
                                          Fw(row+FS2-1,col+FS2+0,fRatio) ) + 
                                          Fw(row+FS2-1,col+FS2+0,fRatio) ) +
                                          v1[(col+FS2)/2].z * ( fc.x * 
                                        ( Fw(row+FS2-1,col+FS2-0,fRatio) - 
                                          Fw(row+FS2-1,col+FS2+1,fRatio) ) + 
                                          Fw(row+FS2-1,col+FS2+1,fRatio) ) );
                }
            }
            
            if( row != FS2 )
            {
                v0[(col+FS2)/2] = v1[(col+FS2)/2].xy;
            }
        }
    }

    return s/w;
}

//====================================================================================== 
// This shader outputs the pixel's color by passing through the lit 
// diffuse material color and by evaluating the shadow function
//====================================================================================== 
PS_RenderOutput main( PS_RenderSceneInput I )
{
    PS_RenderOutput O;
    float3 LSp  = I.f4SMC.xyz / I.f4SMC.w;
    
    //transform from RT space to texture space.
    float2 ShadowTexC = 0.5 * LSp.xy + float2( 0.5, 0.5 );
    ShadowTexC.y = 1.0f - ShadowTexC.y;
    
    float3 f3TC = float3( ShadowTexC, LSp.z - 0.005f );
    float fShadow = shadow( f3TC );

    O.f4Color =  ( saturate( float4( 0.3, 0.3, 0.3, 0.0 ) + 
                 ( fShadow * I.f4Diffuse ) ) * g_txScene.Sample( g_SampleLinear, 
                                                                 I.f2TexCoord ) );
    
    return O;
}

//====================================================================================== 
// EOF
//====================================================================================== 
