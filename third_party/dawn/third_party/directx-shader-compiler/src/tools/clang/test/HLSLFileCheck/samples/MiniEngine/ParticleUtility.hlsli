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
// Author(s):  Julia Careaga
//             James Stanard 
//

#include "ParticleRS.hlsli"

#define MAX_PARTICLES_PER_BIN 1024
#define BIN_SIZE_X 128
#define BIN_SIZE_Y 64
#define TILE_SIZE 16

#define TILES_PER_BIN_X (BIN_SIZE_X / TILE_SIZE)
#define TILES_PER_BIN_Y (BIN_SIZE_Y / TILE_SIZE)
#define TILES_PER_BIN (TILES_PER_BIN_X * TILES_PER_BIN_Y)

#define MaxTextureSize 64

SamplerState gSampLinearBorder : register(s0);
SamplerState gSampPointBorder : register(s1);
SamplerState gSampPointClamp : register(s2);

cbuffer CBChangesPerView : register(b1)
{
	float4x4 gInvView;
	float4x4 gViewProj;

	float gVertCotangent;
	float gAspectRatio;
	float gRcpFarZ;
	float gInvertZ;

	float2 gBufferDim;
	float2 gRcpBufferDim;

	uint gBinsPerRow;
	uint gTileRowPitch;
	uint gTilesPerRow;
	uint gTilesPerCol;
};

struct ParticleVertex
{
	float3 Position;
	float4 Color;
	float Size;
	uint TextureID;
};

// Intentionally left unpacked to allow scalar register loads and ops
struct ParticleScreenData
{
	float2 Corner;		// Top-left location
	float2 RcpSize;		// 1/width, 1/height
	float4 Color;
	float Depth;
	float TextureIndex;
	float TextureLevel;
	uint Bounds;
};

uint InsertZeroBit( uint Value, uint BitIdx )
{
	uint Mask = BitIdx - 1;
	return (Value & ~Mask) << 1 | (Value & Mask);
}
