// RUN: %dxc -E main -T cs_6_0 -O0 %s | FileCheck %s

// CHECK: groupId
// CHECK: threadIdInGroup
// CHECK: bufferLoad
// CHECK: textureLoad
// CHECK: textureGather
// CHECK: FirstbitLo
// CHECK: Saturate
// CHECK: sampleLevel
// CHECK: textureLoad
// CHECK: textureStore

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
// Author:  James Stanard 
//          Alex Nankervis
//          Julia Careaga
//

#include "ParticleUtility.hlsli"

//#define DEBUG_LOW_RES

#define ALPHA_THRESHOLD (252.0 / 255.0)

cbuffer CB : register(b0)
{
	float gDynamicResLevel;
	float gMipBias;
};

RWTexture2D<float3> g_OutputColorBuffer : register(u0);

StructuredBuffer<ParticleScreenData> g_VisibleParticles : register(t0);
ByteAddressBuffer g_HitMask : register(t1);
Texture2DArray<float4> g_TexArray : register(t2);
Texture2D<float3> g_InputColorBuffer : register(t3);
StructuredBuffer<uint> g_SortedParticles : register(t5);
#ifndef DISABLE_DEPTH_TESTS
Texture2D<float> g_InputDepthBuffer : register(t4);
StructuredBuffer<uint> g_DrawPackets : register(t6);
Texture2D<uint> g_TileDepthBounds : register(t8);
#else
StructuredBuffer<uint> g_DrawPackets : register(t7);
#endif

float4 SampleParticleColor( ParticleScreenData Particle, SamplerState Sampler, float2 UV, float LevelBias )
{
	float LOD = Particle.TextureLevel + LevelBias;

	float4 Color = g_TexArray.SampleLevel( Sampler, float3(UV, Particle.TextureIndex), LOD);

	// Multiply texture RGB with alpha.  Pre-multiplied alpha blending also permits additive blending.
	Color.rgb *= Color.a;

	return Color * Particle.Color;
}

void BlendPixel( inout float4 Dst, float4 Src, float Mask )
{
	Dst += Src * (1.0 - Dst.a) * Mask;
}

void BlendHighRes( inout float4x4 Quad, ParticleScreenData Particle, float2 PixelCoord, float4 Mask = 1 )
{
	float2 UV = (PixelCoord - Particle.Corner) * Particle.RcpSize;
	float2 dUV = 0.5 * gRcpBufferDim * Particle.RcpSize;
	float2 UV1 = UV - dUV;
	float2 UV2 = UV + dUV;

#if defined(DYNAMIC_RESOLUTION)
	// Use point sampling for high-res rendering because this implies we're not rendering
	// with the most detailed mip level anyway.
	SamplerState Sampler = gSampPointBorder;
	float LevelBias = gMipBias;
#else
	SamplerState Sampler = gSampLinearBorder;
	float LevelBias = 0.0;
#endif

	BlendPixel(Quad[0], SampleParticleColor(Particle, Sampler, float2(UV1.x, UV2.y), LevelBias), Mask.x);
	BlendPixel(Quad[1], SampleParticleColor(Particle, Sampler, float2(UV2.x, UV2.y), LevelBias), Mask.y);
	BlendPixel(Quad[2], SampleParticleColor(Particle, Sampler, float2(UV2.x, UV1.y), LevelBias), Mask.z);
	BlendPixel(Quad[3], SampleParticleColor(Particle, Sampler, float2(UV1.x, UV1.y), LevelBias), Mask.w);
}

void BlendLowRes( inout float4x4 Quad, ParticleScreenData Particle, float2 PixelCoord, float4 Mask = 1 )
{
	float2 UV = (PixelCoord - Particle.Corner) * Particle.RcpSize;
	float4 Color = SampleParticleColor(Particle, gSampLinearBorder, UV, 1.0);
#ifdef DEBUG_LOW_RES
	Color.g *= 0.5;
#endif
	BlendPixel(Quad[0], Color, Mask.x);
	BlendPixel(Quad[1], Color, Mask.y);
	BlendPixel(Quad[2], Color, Mask.z);
	BlendPixel(Quad[3], Color, Mask.w);
}

void WriteBlendedColor( uint2 ST, float4 Color )
{
	g_OutputColorBuffer[ST] = Color.rgb + g_InputColorBuffer[ST] * (1.0 - Color.a);
}

void WriteBlendedQuad( uint2 ST, float4x4 Quad )
{
	WriteBlendedColor(ST + uint2(0, 0), Quad[3]);
	WriteBlendedColor(ST + uint2(1, 0), Quad[2]);
	WriteBlendedColor(ST + uint2(1, 1), Quad[1]);
	WriteBlendedColor(ST + uint2(0, 1), Quad[0]);
}

float4x4 RenderParticles( uint2 TileCoord, uint2 ST, uint NumParticles, uint HitMaskStart, uint BinStart )
{
#ifndef DISABLE_DEPTH_TESTS
	const uint TileNearZ = g_TileDepthBounds[TileCoord] << 18;
	float4 Depths = g_InputDepthBuffer.Gather(gSampPointClamp, (ST + 1) * gRcpBufferDim);
#endif

	// VGPR
	float4x4 Quad = 0.0;
	const float2 PixelCoord = (ST + 1) * gRcpBufferDim;

	uint BlendedParticles = 0;

	while (BlendedParticles < NumParticles)
	{
		for (uint ParticleMask = g_HitMask.Load(HitMaskStart); ParticleMask != 0; ++BlendedParticles)
		{
			// Get the next bit and then clear it
			uint SubIdx = firstbitlow(ParticleMask);
			ParticleMask ^= 1 << SubIdx;

			// Get global particle index from sorted buffer and then load the particle
			uint SortKey = g_SortedParticles[BinStart + SubIdx];
			uint ParticleIdx = SortKey & 0x3FFFF;
			ParticleScreenData Particle = g_VisibleParticles[ParticleIdx];

#if defined(DYNAMIC_RESOLUTION)
			bool DoFullRes = (Particle.TextureLevel > gDynamicResLevel);
#elif defined(LOW_RESOLUTION)
			static const bool DoFullRes = false;
#else
			static const bool DoFullRes = true;
#endif

			if (DoFullRes)
			{
#ifndef DISABLE_DEPTH_TESTS
				if (SortKey > TileNearZ)
				{
					float4 DepthMask = saturate(1000.0 * (Depths - Particle.Depth));
					BlendHighRes(Quad, Particle, PixelCoord, DepthMask);
				}
				else
#endif
				{
					BlendHighRes(Quad, Particle, PixelCoord);
				}
			}
			else
			{
#ifndef DISABLE_DEPTH_TESTS
				if (SortKey > TileNearZ)
				{
					float4 DepthMask = saturate(1000.0 * (Depths - Particle.Depth));
					BlendLowRes(Quad, Particle, PixelCoord, DepthMask);
				}
				else
#endif
				{
					BlendLowRes(Quad, Particle, PixelCoord);
				}
			}

			//if (all(float4(Quad[0].a, Quad[1].a, Quad[2].a, Quad[3].a) > ALPHA_THRESHOLD))
			//{
			//	Quad[0].a = Quad[1].a = Quad[2].a = Quad[3].a = 1.0;
			//	return Quad;
			//}

		} // for

		// Every outer loop iteration traverses 32 entries in the sorted particle list
		HitMaskStart += 4;
		BinStart += 32;

	} // while

	return Quad;
}

[RootSignature(Particle_RootSig)]
[numthreads(TILE_SIZE / 2, TILE_SIZE / 2, 1)]
void main( uint3 Gid : SV_GroupID, uint  GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID )
{
	const uint DrawPacket = g_DrawPackets[Gid.x];
	uint2 TileCoord = uint2(DrawPacket >> 16, DrawPacket >> 24) & 0xFF;
	const uint ParticleCount = DrawPacket & 0xFFFF;

	const uint HitMaskSizeInBytes = MAX_PARTICLES_PER_BIN / 8;
	const uint TileIndex = TileCoord.x + TileCoord.y * gTileRowPitch;
	const uint HitMaskStart = TileIndex * HitMaskSizeInBytes;
	const uint2 BinCoord = TileCoord / uint2(TILES_PER_BIN_X, TILES_PER_BIN_Y);
	const uint BinIndex = BinCoord.x + BinCoord.y * gBinsPerRow;
	const uint BinStart = BinIndex * MAX_PARTICLES_PER_BIN;

	const uint2 ST = TileCoord * TILE_SIZE + 2 * GTid.xy;

	float4x4 Quad = RenderParticles( TileCoord, ST, ParticleCount, HitMaskStart, BinStart );

	WriteBlendedQuad(ST, Quad);
}
