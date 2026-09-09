// RUN: %dxc -E main -T cs_6_0 -O1 %s | FileCheck %s

// CHECK: bufferLoad
// CHECK: lshr
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
//

#include "ParticleUtility.hlsli"

ByteAddressBuffer g_FinalInstanceCounter : register( t0 );
RWByteAddressBuffer g_NumThreadGroups : register( u0 );
RWByteAddressBuffer g_DrawIndirectArgs : register ( u1 );

[RootSignature(Particle_RootSig)]
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint particleCount = g_FinalInstanceCounter.Load(0);
	g_NumThreadGroups.Store3(0, uint3((particleCount + 63) / 64, 1, 1));
	g_DrawIndirectArgs.Store(4, particleCount);
}