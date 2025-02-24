// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: DerivCoarseX
// CHECK: DerivCoarseY
// CHECK: sampleGrad
// CHECK: FMax
// CHECK: FMin
// CHECK: Log
// CHECK: Exp
// CHECK: Saturate
// CHECK: dot3
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: RenderCascadeScene.hlsl
//
// This is the main shader file.  This shader is compiled with several different flags 
// to provide different customizations based on user controls.
// 
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// This flag enables the shadow to blend between cascades.  This is most useful when the 
// the shadow maps are small and artifact can be seen between the various cascade layers.
#ifndef BLEND_BETWEEN_CASCADE_LAYERS_FLAG
#define BLEND_BETWEEN_CASCADE_LAYERS_FLAG 0
#endif

// There are two methods for selecting the proper cascade a fragment lies in.  Interval selection
// compares the depth of the fragment against the frustum's depth partition.
// Map based selection compares the texture coordinates against the acutal cascade maps.
// Map based selection gives better coverage.  
// Interval based selection is easier to extend and understand.
#ifndef SELECT_CASCADE_BY_INTERVAL_FLAG
#define SELECT_CASCADE_BY_INTERVAL_FLAG 0
#endif

// The number of cascades 
#ifndef CASCADE_COUNT_FLAG
#define CASCADE_COUNT_FLAG 3
#endif


// Most titles will find that 3-4 cascades with 
// BLEND_BETWEEN_CASCADE_LAYERS_FLAG, is good for lower end PCs.

cbuffer cbAllShadowData : register( b0 )
{
    matrix          m_mWorldViewProjection;
    matrix          m_mWorld;
    matrix          m_mWorldView;
    matrix          m_mShadow;
    float4          m_vCascadeOffset[8];
    float4          m_vCascadeScale[8];
    int             m_nCascadeLevels; // Number of Cascades
    int             m_iVisualizeCascades; // 1 is to visualize the cascades in different colors. 0 is to just draw the scene

    // For Map based selection scheme, this keeps the pixels inside of the the valid range.
    // When there is no boarder, these values are 0 and 1 respectivley.
    float           m_fMinBorderPadding;     
    float           m_fMaxBorderPadding;
                                          
    float           m_fCascadeBlendArea; // Amount to overlap when blending between cascades.
    float           m_fTexelSize; // Padding variables exist because CBs must be a multiple of 16 bytes.
    float           m_fNativeTexelSizeInX;
    float4          m_fCascadeFrustumsEyeSpaceDepthsData[2];  // The values along Z that seperate the cascades.
    // This code creates an array based pointer that points towards the vectorized input data.
    // This is the only way to index arbitrary arrays of data.
    // If the array is used at run time, the compiler will generate code that uses logic to index the correct component.

    static float    m_fCascadeFrustumsEyeSpaceDepths[8] = (float[8])m_fCascadeFrustumsEyeSpaceDepthsData;
    
    float3          m_vLightDir;
    float           m_fPaddingCB4;

};



//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D           g_txDiffuse             : register( t0 );
Texture2DArray      g_txShadow              : register( t5 );

SamplerState g_samLinear                    : register( s0 );
SamplerState g_samShadow                    : register( s5 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 vPosition                        : POSITION;
    float3 vNormal                          : NORMAL;
    float2 vTexcoord                        : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 vNormal                          : NORMAL;
    float2 vTexcoord                        : COLOR0;
    float4 vTexShadow						: TEXCOORD1;
    float4 vPosition                        : SV_POSITION;
    float4 vInterpPos                       : TEXCOORD2;
    float  vDepth                           : TEXCOORD3;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
    VS_OUTPUT Output;

    Output.vPosition = mul( Input.vPosition, m_mWorldViewProjection );
    Output.vNormal = mul( Input.vNormal, (float3x3)m_mWorld );
    Output.vTexcoord = Input.vTexcoord;
    Output.vInterpPos = Input.vPosition;   
    Output.vDepth = mul( Input.vPosition, m_mWorldView ).z ; 
       
    // Transform the shadow texture coordinates for all the cascades.
    Output.vTexShadow = mul( Input.vPosition, m_mShadow );
        
    return Output;
}



static const float4 vCascadeColorsMultiplier[8] = 
{
    float4 ( 1.5f, 0.0f, 0.0f, 1.0f ),
    float4 ( 0.0f, 1.5f, 0.0f, 1.0f ),
    float4 ( 0.0f, 0.0f, 5.5f, 1.0f ),
    float4 ( 1.5f, 0.0f, 5.5f, 1.0f ),
    float4 ( 1.5f, 1.5f, 0.0f, 1.0f ),
    float4 ( 1.0f, 1.0f, 1.0f, 1.0f ),
    float4 ( 0.0f, 1.0f, 5.5f, 1.0f ),
    float4 ( 0.5f, 3.5f, 0.75f, 1.0f )
};


void ComputeCoordinatesTransform( in int iCascadeIndex,
                                  in float4 InterpolatedPosition, 
                                  in out float4 vShadowTexCoord,
                                  in out float4 vShadowTexCoordViewSpace ) 
{
    // Now that we know the correct map, we can transform the world space position of the current fragment                
    if( SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        vShadowTexCoord = vShadowTexCoordViewSpace * m_vCascadeScale[iCascadeIndex];
        vShadowTexCoord += m_vCascadeOffset[iCascadeIndex];
    }  
    vShadowTexCoord.w = vShadowTexCoord.z; // We put the z value in w so that we can index the texture array with Z.
    vShadowTexCoord.z = iCascadeIndex;
    
} 

//--------------------------------------------------------------------------------------
// Use PCF to sample the depth map and return a percent lit value.
//--------------------------------------------------------------------------------------
void CalculateVarianceShadow ( in float4 vShadowTexCoord, in float4 vShadowMapTextureCoordViewSpace, int iCascade, out float fPercentLit ) 
{
    fPercentLit = 0.0f;
    // This loop could be unrolled, and texture immediate offsets could be used if the kernel size were fixed.
    // This would be a performance improvment.
	        
    float2 mapDepth = 0;


    // In orderto pull the derivative out of divergent flow control we calculate the 
    // derivative off of the view space coordinates an then scale the deriviative.
    
    float3 vShadowTexCoordDDX = 
		ddx(vShadowMapTextureCoordViewSpace );
    vShadowTexCoordDDX *= m_vCascadeScale[iCascade].xyz; 
    float3 vShadowTexCoordDDY = 
		ddy(vShadowMapTextureCoordViewSpace );
    vShadowTexCoordDDY *= m_vCascadeScale[iCascade].xyz; 
    
    mapDepth += g_txShadow.SampleGrad( g_samShadow, vShadowTexCoord.xyz, 
									   vShadowTexCoordDDX,
									   vShadowTexCoordDDY);
    // The sample instruction uses gradients for some filters.
		        
    float  fAvgZ  = mapDepth.x; // Filtered z
    float  fAvgZ2 = mapDepth.y; // Filtered z-squared
    
    if ( vShadowTexCoord.w <= fAvgZ ) // We put the z value in w so that we can index the texture array with Z.
    {
        fPercentLit = 1;
	}
	else 
	{
	    float variance = ( fAvgZ2 ) - ( fAvgZ * fAvgZ );
        variance       = min( 1.0f, max( 0.0f, variance + 0.00001f ) );
    
        float mean     = fAvgZ;
        float d        = vShadowTexCoord.w - mean; // We put the z value in w so that we can index the texture array with Z.
        float p_max    = variance / ( variance + d*d );

        // To combat light-bleeding, experiment with raising p_max to some power
        // (Try values from 0.1 to 100.0, if you like.)
        fPercentLit = pow( p_max, 4 );
	    
	}
    
}

//--------------------------------------------------------------------------------------
// Calculate amount to blend between two cascades and the band where blending will occure.
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForInterval ( in int iNextCascadeIndex, 
                                       in out float fPixelDepth, 
                                       in out float fCurrentPixelsBlendBandLocation,
                                       out float fBlendBetweenCascadesAmount
                                       ) 
{

    // We need to calculate the band of the current shadow map where it will fade into the next cascade.
    // We can then early out of the expensive PCF for loop.
    // 
    float fBlendInterval = m_fCascadeFrustumsEyeSpaceDepths[ iNextCascadeIndex - 1 ];
    if( iNextCascadeIndex > 1 ) 
    {
        fPixelDepth -= m_fCascadeFrustumsEyeSpaceDepths[ iNextCascadeIndex-2 ];
        fBlendInterval -= m_fCascadeFrustumsEyeSpaceDepths[ iNextCascadeIndex-2 ];
    } 
    // The current pixel's blend band location will be used to determine when we need to blend and by how much.
    fCurrentPixelsBlendBandLocation = fPixelDepth / fBlendInterval;
    fCurrentPixelsBlendBandLocation = 1.0f - fCurrentPixelsBlendBandLocation;
    // The fBlendBetweenCascadesAmount is our location in the blend band.
    fBlendBetweenCascadesAmount = fCurrentPixelsBlendBandLocation / m_fCascadeBlendArea;
}


//--------------------------------------------------------------------------------------
// Calculate amount to blend between two cascades and the band where blending will occure.
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForMap ( in float4 vShadowMapTextureCoord, 
                                  in out float fCurrentPixelsBlendBandLocation,
                                  out float fBlendBetweenCascadesAmount ) 
{
    // Calcaulte the blend band for the map based selection.
    float2 distanceToOne = float2 ( 1.0f - vShadowMapTextureCoord.x, 1.0f - vShadowMapTextureCoord.y );
    fCurrentPixelsBlendBandLocation = min( vShadowMapTextureCoord.x, vShadowMapTextureCoord.y );
    float fCurrentPixelsBlendBandLocation2 = min( distanceToOne.x, distanceToOne.y );
    fCurrentPixelsBlendBandLocation = 
        min( fCurrentPixelsBlendBandLocation, fCurrentPixelsBlendBandLocation2 );
    fBlendBetweenCascadesAmount = fCurrentPixelsBlendBandLocation / m_fCascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// Calculate the shadow based on several options and rende the scene.
//--------------------------------------------------------------------------------------

float4 main( VS_OUTPUT Input ) : SV_TARGET
{
    float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );
    
    
    float4 vShadowMapTextureCoordViewSpace = 0.0f;
    float4 vShadowMapTextureCoord = 0.0f;
    float4 vShadowMapTextureCoord_blend = 0.0f;
    
    float4 vVisualizeCascadeColor = float4(0.0f,0.0f,0.0f,1.0f);
    
    float fPercentLit = 0.0f;
    float fPercentLit_blend = 0.0f;

    int iCascadeFound = 0;
    int iCurrentCascadeIndex=1;
    int iNextCascadeIndex = 0;

    float fCurrentPixelDepth;

    // The interval based selection technique compares the pixel's depth against the frustum's cascade divisions.
    fCurrentPixelDepth = Input.vDepth;
    
    // This for loop is not necessary when the frustum is uniformaly divided and interval based selection is used.
    // In this case fCurrentPixelDepth could be used as an array lookup into the correct frustum. 
    vShadowMapTextureCoordViewSpace = Input.vTexShadow;
    
    
    if( SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        iCurrentCascadeIndex = 0;
        if (CASCADE_COUNT_FLAG > 1 ) 
        {
            float4 vCurrentPixelDepth = Input.vDepth;
            float4 fComparison = ( vCurrentPixelDepth > m_fCascadeFrustumsEyeSpaceDepthsData[0]);
            float4 fComparison2 = ( vCurrentPixelDepth > m_fCascadeFrustumsEyeSpaceDepthsData[1]);
            float fIndex = dot( 
                            float4( CASCADE_COUNT_FLAG > 0,
                                    CASCADE_COUNT_FLAG > 1, 
                                    CASCADE_COUNT_FLAG > 2, 
                                    CASCADE_COUNT_FLAG > 3)
                            , fComparison )
                         + dot( 
                            float4(
                                    CASCADE_COUNT_FLAG > 4,
                                    CASCADE_COUNT_FLAG > 5,
                                    CASCADE_COUNT_FLAG > 6,
                                    CASCADE_COUNT_FLAG > 7)
                            , fComparison2 ) ;
                                    
            fIndex = min( fIndex, CASCADE_COUNT_FLAG - 1 );
            iCurrentCascadeIndex = (int)fIndex;
        }
    }
    
    if ( !SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        iCurrentCascadeIndex = 0;
        if ( CASCADE_COUNT_FLAG == 1 ) 
        {
            vShadowMapTextureCoord = vShadowMapTextureCoordViewSpace * m_vCascadeScale[0];
            vShadowMapTextureCoord += m_vCascadeOffset[0];
        }
        if ( CASCADE_COUNT_FLAG > 1 ) {
            for( int iCascadeIndex = 0; iCascadeIndex < CASCADE_COUNT_FLAG && iCascadeFound == 0; ++iCascadeIndex ) 
            {
                vShadowMapTextureCoord = vShadowMapTextureCoordViewSpace * m_vCascadeScale[iCascadeIndex];
                vShadowMapTextureCoord += m_vCascadeOffset[iCascadeIndex];

                if ( min( vShadowMapTextureCoord.x, vShadowMapTextureCoord.y ) > m_fMinBorderPadding
                  && max( vShadowMapTextureCoord.x, vShadowMapTextureCoord.y ) < m_fMaxBorderPadding )
                { 
                    iCurrentCascadeIndex = iCascadeIndex;   
                    iCascadeFound = 1; 
                }
            }
        }
    }    
    // Found the correct map.
    vVisualizeCascadeColor = vCascadeColorsMultiplier[iCurrentCascadeIndex];
    
    ComputeCoordinatesTransform( iCurrentCascadeIndex, Input.vInterpPos, vShadowMapTextureCoord, vShadowMapTextureCoordViewSpace  );    
                                             
    if( BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1 ) 
    {
        // Repeat text coord calculations for the next cascade. 
        // The next cascade index is used for blurring between maps.
        iNextCascadeIndex = min ( CASCADE_COUNT_FLAG - 1, iCurrentCascadeIndex + 1 ); 
        if( !SELECT_CASCADE_BY_INTERVAL_FLAG ) 
        {
            vShadowMapTextureCoord_blend = vShadowMapTextureCoordViewSpace * m_vCascadeScale[iNextCascadeIndex];
            vShadowMapTextureCoord_blend += m_vCascadeOffset[iNextCascadeIndex];
        }
        ComputeCoordinatesTransform( iNextCascadeIndex, Input.vInterpPos, vShadowMapTextureCoord_blend, vShadowMapTextureCoordViewSpace );  
    }            
    float fBlendBetweenCascadesAmount = 1.0f;
    float fCurrentPixelsBlendBandLocation = 1.0f;
    
    if( SELECT_CASCADE_BY_INTERVAL_FLAG ) 
    {
        if( CASCADE_COUNT_FLAG > 1 && BLEND_BETWEEN_CASCADE_LAYERS_FLAG ) 
        {
            CalculateBlendAmountForInterval ( iNextCascadeIndex, fCurrentPixelDepth, 
                fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount );
            
        }   
    }
    else 
    {
        if( CASCADE_COUNT_FLAG > 1 && BLEND_BETWEEN_CASCADE_LAYERS_FLAG ) 
        {
            CalculateBlendAmountForMap ( vShadowMapTextureCoord, 
                fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount );
        }   
    }
    
    // Because the Z coordinate specifies the texture array,
    // the derivative will be 0 when there is no divergence
    //float fDivergence = abs( ddy( vShadowMapTextureCoord.z ) ) +  abs( ddx( vShadowMapTextureCoord.z ) );
    CalculateVarianceShadow ( vShadowMapTextureCoord, vShadowMapTextureCoordViewSpace, 
								iCurrentCascadeIndex, fPercentLit);
								
    // We repeat the calcuation for the next cascade layer, when blending between maps.
    if( BLEND_BETWEEN_CASCADE_LAYERS_FLAG  && CASCADE_COUNT_FLAG > 1 ) 
    {
        if( fCurrentPixelsBlendBandLocation < m_fCascadeBlendArea ) 
        {  // the current pixel is within the blend band.

			// Because the Z coordinate species the texture array,
			// the derivative will be 0 when there is no divergence
			float fDivergence = abs( ddy( vShadowMapTextureCoord_blend.z ) ) +  
				abs( ddx( vShadowMapTextureCoord_blend.z) );
            CalculateVarianceShadow ( vShadowMapTextureCoord_blend, vShadowMapTextureCoordViewSpace, 
										iNextCascadeIndex, fPercentLit_blend );

            // Blend the two calculated shadows by the blend amount.
            fPercentLit = lerp( fPercentLit_blend, fPercentLit, fBlendBetweenCascadesAmount ); 

        }   
    }    
  
    if( !m_iVisualizeCascades ) vVisualizeCascadeColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );
    
    float3 vLightDir1 = float3( -1.0f, 1.0f, -1.0f ); 
    float3 vLightDir2 = float3( 1.0f, 1.0f, -1.0f ); 
    float3 vLightDir3 = float3( 0.0f, -1.0f, 0.0f );
    float3 vLightDir4 = float3( 1.0f, 1.0f, 1.0f );     
    // Some ambient-like lighting.
    float fLighting = 
                      saturate( dot( vLightDir1 , Input.vNormal ) )*0.05f +
                      saturate( dot( vLightDir2 , Input.vNormal ) )*0.05f +
                      saturate( dot( vLightDir3 , Input.vNormal ) )*0.05f +
                      saturate( dot( vLightDir4 , Input.vNormal ) )*0.05f ;
    
    float4 vShadowLighting = fLighting * 0.5f;
    fLighting += saturate( dot( m_vLightDir , Input.vNormal ) );
    fLighting = lerp( vShadowLighting, fLighting, fPercentLit );
    
    return fLighting * vVisualizeCascadeColor * vDiffuse;

}

