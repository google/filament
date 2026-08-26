// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: FirstbitHi

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
//

#include "ParticleUtility.hlsli"

ByteAddressBuffer g_ActiveParticlesCount : register(t1);
RWByteAddressBuffer g_IndirectArgsBuffer : register(u0);

[RootSignature(Particle_RootSig)]
[numthreads(8, 1, 1)]
void main( uint GI : SV_GroupIndex )
{
	uint k = 1 << (GI + 11);

	uint VisibleParticles = g_ActiveParticlesCount.Load(4);
	uint NextPow2 = (1 << firstbithigh(VisibleParticles)) - 1;
	NextPow2 = (VisibleParticles + NextPow2) & ~NextPow2;
	NextPow2 = (NextPow2 + 2047) & ~2047;

	uint NumElements = k > NextPow2 ? 0 : (VisibleParticles + k - 1) & ~(k - 1);
	uint NumGroups = (GI == 0 ? NextPow2 : NumElements) / 2048;

	g_IndirectArgsBuffer.Store3(GI * 12, uint3(NumGroups, 1, 1));
}