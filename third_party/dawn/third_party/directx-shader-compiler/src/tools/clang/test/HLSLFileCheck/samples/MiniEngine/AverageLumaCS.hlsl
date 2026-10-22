// RUN: %dxc -E main -T cs_6_0 -O0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: threadId
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: addrspace(3)


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

#include "PostEffectsRS.hlsli"

Texture2D<float> InputBuf : register( t0 );
RWStructuredBuffer<float> Result : register( u0 );

groupshared float buffer[64];

[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	float sumThisThread = InputBuf[DTid.xy];
	buffer[GI] = sumThisThread;
	GroupMemoryBarrierWithGroupSync();

	sumThisThread += buffer[GI + 32];
	buffer[GI] = sumThisThread;
	GroupMemoryBarrierWithGroupSync();

	sumThisThread += buffer[GI + 16];
	buffer[GI] = sumThisThread;
	GroupMemoryBarrierWithGroupSync();

	sumThisThread += buffer[GI + 8];
	buffer[GI] = sumThisThread;
	GroupMemoryBarrierWithGroupSync();

	sumThisThread += buffer[GI + 4];
	buffer[GI] = sumThisThread;
	GroupMemoryBarrierWithGroupSync();

	sumThisThread += buffer[GI + 2];
	buffer[GI] = sumThisThread;
	GroupMemoryBarrierWithGroupSync();

	sumThisThread += buffer[GI + 1];

	if (GI == 0)
		Result[Gid.x + Gid.y * 5] = sumThisThread / 64.0f;
}
