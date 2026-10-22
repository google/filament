// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: FMin
// CHECK: dot3
// CHECK: bufferUpdateCounter
// CHECK: bufferUpdateCounter

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
//          Julia Careaga
//

#include "ParticleUpdateCommon.hlsli"
#include "ParticleUtility.hlsli"

cbuffer CB : register(b0)
{
	float gElapsedTime;
};

StructuredBuffer< ParticleSpawnData > g_ResetData : register( t0 );
StructuredBuffer< ParticleMotion > g_InputBuffer : register( t1 );
RWStructuredBuffer< ParticleVertex > g_VertexBuffer : register( u0 );
RWStructuredBuffer< ParticleMotion > g_OutputBuffer : register( u2 );

[RootSignature(Particle_RootSig)]
[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x >= MaxParticles)
		return;

	ParticleMotion ParticleState = g_InputBuffer[ DTid.x ];
	ParticleSpawnData rd = g_ResetData[ ParticleState.ResetDataIndex ];

	// Update age.  If normalized age exceeds 1, the particle does not renew its lease on life.
	ParticleState.Age += gElapsedTime * rd.AgeRate;
	if (ParticleState.Age >= 1.0)
		return;

	// Update position.  Compute two deltas to support rebounding off the ground plane.
	float StepSize = (ParticleState.Position.y > 0.0 && ParticleState.Velocity.y < 0.0) ?
		min(gElapsedTime, ParticleState.Position.y / -ParticleState.Velocity.y) : gElapsedTime;

	ParticleState.Position += ParticleState.Velocity * StepSize;
	ParticleState.Velocity += Gravity * ParticleState.Mass * StepSize;

	// Rebound off the ground if we didn't consume all of the elapsed time
	StepSize = gElapsedTime - StepSize;
	if (StepSize > 0.0)
	{
		ParticleState.Velocity = reflect(ParticleState.Velocity, float3(0, 1, 0)) * Restitution;
		ParticleState.Position += ParticleState.Velocity * StepSize;
		ParticleState.Velocity += Gravity * ParticleState.Mass * StepSize;
	}

	// The spawn dispatch will be simultaneously adding particles as well.  It's possible to overflow.
	uint index = g_OutputBuffer.IncrementCounter();	
	if (index >= MaxParticles)
		return;

	g_OutputBuffer[index] = ParticleState;

	//
	// Generate a sprite vertex
	//

	ParticleVertex Sprite;

	Sprite.Position = ParticleState.Position;
	Sprite.TextureID = TextureID;

	// Update size and color
	Sprite.Size = lerp(rd.StartSize, rd.EndSize, ParticleState.Age);
	Sprite.Color = lerp(rd.StartColor, rd.EndColor, ParticleState.Age);

	// ...Originally from Reflex...
	// Use a trinomial to smoothly fade in a particle at birth and fade it out at death.
	Sprite.Color *= ParticleState.Age * (1.0 - ParticleState.Age) * (1.0 - ParticleState.Age) * 6.7;

	g_VertexBuffer[ g_VertexBuffer.IncrementCounter() ] = Sprite;
}
