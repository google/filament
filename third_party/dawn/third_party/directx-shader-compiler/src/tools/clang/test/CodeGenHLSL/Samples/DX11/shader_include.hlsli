    //--------------------------------------------------------------------------------------
// File: shader_include.hlsl
//
// Include file for common shader definitions and functions.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------                  
#define ADD_SPECULAR 0
                                          
//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture2D g_baseTexture : register( t0 );    // Base color texture
Texture2D g_nmhTexture :  register( t1 );    // Normal map and height map texture pair


//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState g_samLinear : register( s0 );
SamplerState g_samPoint  : register( s1 );


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbMain : register( b0 )
{
	matrix g_mWorld;                            // World matrix
	matrix g_mView;                             // View matrix
	matrix g_mProjection;                       // Projection matrix
    matrix g_mWorldViewProjection;              // WVP matrix
    matrix g_mViewProjection;                   // VP matrix
    matrix g_mInvView;                          // Inverse of view matrix
    float4 g_vScreenResolution;                 // Screen resolution
    
    float4 g_vMeshColor;                        // Mesh color
    float4 g_vTessellationFactor;               // Edge, inside, minimum tessellation factor and 1/desired triangle size
    float4 g_vDetailTessellationHeightScale;    // Height scale for detail tessellation of grid surface
    float4 g_vGridSize;                         // Grid size
    
    float4 g_vDebugColorMultiply;               // Debug colors
    float4 g_vDebugColorAdd;                    // Debug colors
    
    float4 g_vFrustumPlaneEquation[4];          // View frustum plane equations
};

cbuffer cbMaterial : register( b1 )
{
	float4 	 g_materialAmbientColor;          	// Material's ambient color
	float4 	 g_materialDiffuseColor;          	// Material's diffuse color
	float4 	 g_materialSpecularColor;         	// Material's specular color
	float4 	 g_fSpecularExponent;             	// Material's specular exponent

	float4 	 g_LightPosition;                 	// Light's position in world space
	float4 	 g_LightDiffuse;                  	// Light's diffuse color
	float4 	 g_LightAmbient;                  	// Light's ambient color

	float4 	 g_vEye;					    	// Camera's location
	float4 	 g_fBaseTextureRepeat;		    	// The tiling factor for base and normal map textures
	float4 	 g_fPOMHeightMapScale;		    	// Describes the useful range of values for the height field
	
	float4   g_fShadowSoftening;		    	// Blurring factor for the soft shadows computation
	
	int      g_nMinSamples;				    	// The minimum number of samples for sampling the height field profile
	int      g_nMaxSamples;				    	// The maximum number of samples for sampling the height field profile
};

//--------------------------------------------------------------------------------------
// Function:    ComputeIllumination
// 
// Description: Computes phong illumination for the given pixel using its attribute 
//              textures and a light vector.
//--------------------------------------------------------------------------------------
float4 ComputeIllumination( float2 texCoord, float3 vLightTS, float3 vViewTS )
{
   // Sample the normal from the normal map for the given texture sample:
   float3 vNormalTS = normalize( g_nmhTexture.Sample(g_samLinear, texCoord) * 2.0 - 1.0 );
   
   // Sample base map
   float4 cBaseColor = g_baseTexture.Sample( g_samLinear, texCoord );
   
   // Compute diffuse color component:
   float4 cDiffuse = saturate( dot( vNormalTS, vLightTS ) ) * g_materialDiffuseColor;
   
   // Compute the specular component if desired:  
   float4 cSpecular = 0;

#if ADD_SPECULAR==1
   
   float3 vReflectionTS = normalize( 2 * dot( vViewTS, vNormalTS ) * vNormalTS - vViewTS );
   float fRdotL = saturate( dot( vReflectionTS, vLightTS ) );
   cSpecular = pow( fRdotL, g_fSpecularExponent.x ) * g_materialSpecularColor;

#endif
   
   // Composite the final color:
   float4 cFinalColor = ( g_materialAmbientColor + cDiffuse ) * cBaseColor + cSpecular; 
   
   return cFinalColor;  
}  