// RUN: %dxc -E main -T cs_6_0 -O0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferUpdateCounter
// CHECK: bufferLoad
// CHECK: bufferStore

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
// Author:  Julia Careaga
//          James Stanard 
//

#include "ParticleUpdateCommon.hlsli"
#include "ParticleUtility.hlsli"

StructuredBuffer< ParticleSpawnData > g_ResetData : register( t0 );
RWStructuredBuffer< ParticleMotion > g_OutputBuffer : register( u2 );

[RootSignature(Particle_RootSig)]
[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = g_OutputBuffer.IncrementCounter();
	if (index >= MaxParticles)
		return;
	
	uint ResetDataIndex = RandIndex[DTid.x].x; 
	ParticleSpawnData rd  = g_ResetData[ResetDataIndex];
		
	float3 emitterVelocity = EmitPosW - LastEmitPosW; 
	float3 randDir = rd.Velocity.x * EmitRightW + rd.Velocity.y * EmitUpW + rd.Velocity.z * EmitDirW;
	float3 newVelocity = emitterVelocity * EmitterVelocitySensitivity + randDir;
	float3 adjustedPosition = EmitPosW - emitterVelocity * rd.Random + rd.SpreadOffset;

	ParticleMotion newParticle;
	newParticle.Position = adjustedPosition;
	newParticle.Rotation = 0.0;
	newParticle.Velocity = newVelocity + EmitDirW * EmitSpeed; 
	newParticle.Mass = rd.Mass; 
	newParticle.Age = 0.0;
	newParticle.ResetDataIndex = ResetDataIndex; 
	g_OutputBuffer[index] = newParticle;
}
