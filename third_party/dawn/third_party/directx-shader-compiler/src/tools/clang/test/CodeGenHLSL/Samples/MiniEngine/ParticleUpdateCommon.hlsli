// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved
//
// Developed by Minigraph
//
// Author(s):   Julia Careaga
//              James Stanard
//

cbuffer EmissionProperties : register(b2)
{	
	float3 LastEmitPosW;
	float EmitSpeed;
	float3 EmitPosW;
	float FloorHeight;
	float3 EmitDirW;
	float Restitution;
	float3 EmitRightW;
	float EmitterVelocitySensitivity;
	float3 EmitUpW;
	uint MaxParticles;
	float3 Gravity;
	uint TextureID;
	float3 EmissiveColor;
	float pad;
	uint4 RandIndex[64];
};

struct ParticleSpawnData
{
	float AgeRate;
	float RotationSpeed; 
	float StartSize;
	float EndSize;
	float3 Velocity;
	float Mass;
	float3 SpreadOffset;
	float Random;
	float4 StartColor;
	float4 EndColor;
};

struct ParticleMotion
{
	float3 Position;
	float Mass; 
	float3 Velocity;
	float Age;
	float Rotation;
	uint ResetDataIndex;
};

struct ParticleVertexOutput
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	nointerpolation uint TexID : TEXCOORD1;
	nointerpolation float4 Color : TEXCOORD2;
	nointerpolation float LinearZ : TEXCOORD3;
};
