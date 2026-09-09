// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: threadId
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
//

#include "SSAORS.hlsli"

Texture2D<float> DS4x : register(t0);
RWTexture2D<float> DS8x : register(u0);
RWTexture2DArray<float> DS8xAtlas : register(u1);
RWTexture2D<float> DS16x : register(u2);
RWTexture2DArray<float> DS16xAtlas : register(u3);

cbuffer ConstantBuffer_x : register(b0)
{
	float2 InvSourceDimension;
}

[RootSignature(SSAO_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	float m1 = DS4x[DTid.xy << 1];

	uint2 st = DTid.xy;
	uint2 stAtlas = st >> 2;
	uint stSlice = (st.x & 3) | ((st.y & 3) << 2);
	DS8x[st] = m1;
	DS8xAtlas[uint3(stAtlas, stSlice)] = m1;

	if ((GI & 011) == 0)
	{
		uint2 st = DTid.xy >> 1;
		uint2 stAtlas = st >> 2;
		uint stSlice = (st.x & 3) | ((st.y & 3) << 2);
		DS16x[st] = m1;
		DS16xAtlas[uint3(stAtlas, stSlice)] = m1;
	}
}
