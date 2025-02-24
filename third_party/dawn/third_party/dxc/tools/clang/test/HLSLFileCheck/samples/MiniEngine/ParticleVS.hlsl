// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: bufferLoad

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
// Author(s):   James Stanard

#include "ParticleUpdateCommon.hlsli"
#include "ParticleUtility.hlsli"

StructuredBuffer<ParticleVertex> g_VertexBuffer : register( t0 );
StructuredBuffer<uint> g_IndexBuffer : register( t3 );

[RootSignature(Particle_RootSig)]
ParticleVertexOutput main( uint BillboardVertex : SV_VertexID, uint InstanceId : SV_InstanceID )
{
	ParticleVertex In = g_VertexBuffer[ g_IndexBuffer[InstanceId] & 0x3FFFF ];
	ParticleVertexOutput Out;

	Out.TexCoord = float2((BillboardVertex >> 1) & 1, BillboardVertex & 1);
	Out.Color = In.Color;
	Out.TexID = In.TextureID;

	float2 Corner = lerp( float2(-1, 1), float2(1, -1), Out.TexCoord );
	float3 Position = mul( (float3x3)gInvView, float3(Corner * In.Size, 0) ) + In.Position;

	Out.Pos = mul( gViewProj, float4(Position, 1) );
	Out.LinearZ = Out.Pos.w * gRcpFarZ;

	return Out;
}
