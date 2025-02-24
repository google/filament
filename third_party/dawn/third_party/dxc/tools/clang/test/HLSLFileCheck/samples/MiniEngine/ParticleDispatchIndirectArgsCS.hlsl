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

#include "ParticleRS.hlsli"

ByteAddressBuffer g_ParticleInstance : register( t0 );
RWByteAddressBuffer g_NumThreadGroups : register( u1 );

[RootSignature(Particle_RootSig)]
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	g_NumThreadGroups.Store(0, ( g_ParticleInstance.Load(0) + 63) / 64);

}