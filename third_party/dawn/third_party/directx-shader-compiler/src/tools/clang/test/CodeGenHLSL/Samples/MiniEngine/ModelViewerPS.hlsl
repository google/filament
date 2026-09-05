// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: textureLoad
// CHECK: dot3
// CHECK: Sqrt
// CHECK: FMax
// CHECK: Saturate
// CHECK: Log
// CHECK: Exp
// CHECK: sampleCmpLevelZero

//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//             Alex Nankervis
//

#include "ModelViewerRS.hlsli"

struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoord0 : texcoord0;
	float3 viewDir : texcoord1;
	float3 shadowCoord : texcoord2;
	float3 normal : normal;
	float3 tangent : tangent;
	float3 bitangent : bitangent;
};

Texture2D<float3> texDiffuse : register(t0);
Texture2D<float3> texSpecular : register(t1);
//Texture2D<float4> texEmissive : register(t2);
Texture2D<float3> texNormal : register(t3);
//Texture2D<float4> texLightmap : register(t4);
//Texture2D<float4> texReflection : register(t5);
Texture2D<float> texSSAO : register(t64);
Texture2D<float> texShadow : register(t65);

cbuffer PSConstants : register(b0)
{
	float3 SunDirection;
	float3 SunColor;
	float3 AmbientColor;
	uint _pad;
	float  ShadowTexelSize;
}

SamplerState sampler0 : register(s0);
SamplerComparisonState shadowSampler : register(s1);

// Apply fresnel to modulate the specular albedo
void FSchlick( inout float3 specular, inout float3 diffuse, float3 lightDir, float3 halfVec )
{
	float fresnel = pow(1.0 - saturate(dot(lightDir, halfVec)), 5.0);
	specular = lerp(specular, 1, fresnel);
	diffuse = lerp(diffuse, 0, fresnel);
}

float3 ApplyAmbientLight(
	float3 diffuse,		// Diffuse albedo
	float ao,			// Pre-computed ambient-occlusion
	float3 lightColor	// Radiance of ambient light
	)
{
	return ao * diffuse * lightColor;
}

float GetShadow( float3 ShadowCoord )
{
#ifdef SINGLE_SAMPLE
	float result = ShadowMap.SampleCmpLevelZero( ShadowSampler, ShadowCoord.xy, ShadowCoord.z );
#else
	const float Dilation = 2.0;
	float d1 = Dilation * ShadowTexelSize * 0.125;
	float d2 = Dilation * ShadowTexelSize * 0.875;
	float d3 = Dilation * ShadowTexelSize * 0.625;
	float d4 = Dilation * ShadowTexelSize * 0.375;
	float result = (
		2.0 * texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy, ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d2,  d1), ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d1, -d2), ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d2, -d1), ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d1,  d2), ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d4,  d3), ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2(-d3, -d4), ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d4, -d3), ShadowCoord.z ) +
		texShadow.SampleCmpLevelZero( shadowSampler, ShadowCoord.xy + float2( d3,  d4), ShadowCoord.z )
		) / 10.0;
#endif
	return result * result;
}

float3 ApplyDirectionalLight(
	float3 diffuseColor,	// Diffuse albedo
	float3 specularColor,	// Specular albedo
	float specularMask,		// Where is it shiny or dingy?
	float gloss,			// Specular power
	float3 normal,			// World-space normal
	float3 viewDir,			// World-space vector from eye to point
	float3 lightDir,		// World-space vector from point to light
	float3 lightColor,		// Radiance of directional light
	float3 shadowCoord		// Shadow coordinate (Shadow map UV & light-relative Z)
	)
{
	// normal and lightDir are assumed to be pre-normalized
	float nDotL = dot(normal, lightDir);
	if (nDotL <= 0)
		return 0;

	// viewDir is also assumed normalized
	float3 halfVec = normalize(lightDir - viewDir);
	float nDotH = max(0, dot(halfVec, normal));

	FSchlick( diffuseColor, specularColor, lightDir, halfVec );

	float specularFactor = specularMask * pow(nDotH, gloss) * (gloss + 2) / 8;

	float shadow = GetShadow(shadowCoord);

	return shadow * nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
}

void AntiAliasSpecular( inout float3 texNormal, inout float gloss )
{
	float norm = length(texNormal);
	texNormal /= norm;
	gloss = lerp(1, gloss, norm);
}


[RootSignature(ModelViewer_RootSig)]
float3 main(VSOutput vsOutput) : SV_Target0
{
	float3 diffuseAlbedo = texDiffuse.Sample(sampler0, vsOutput.texcoord0);
	float3 specularAlbedo = float3( 0.56, 0.56, 0.56 );//float3(1.0, 0.71, 0.29);
	float specularMask = texSpecular.Sample(sampler0, vsOutput.texcoord0).g;
	float3 normal = texNormal.Sample(sampler0, vsOutput.texcoord0) * 2.0 - 1.0;
	float gloss = 128.0;
	float ao = texSSAO[uint2(vsOutput.position.xy)];
	float3 viewDir = normalize(vsOutput.viewDir);

	AntiAliasSpecular(normal, gloss);

	float3x3 tbn = float3x3(normalize(vsOutput.tangent), normalize(vsOutput.bitangent), normalize(vsOutput.normal));
	normal = normalize(mul(normal, tbn));

	float3 ambientContribution = ApplyAmbientLight( diffuseAlbedo, ao, AmbientColor );

	float3 sunlightContribution = ApplyDirectionalLight( diffuseAlbedo, specularAlbedo, specularMask, gloss, normal, viewDir, SunDirection, SunColor, vsOutput.shadowCoord );

	return ambientContribution + sunlightContribution;
}
