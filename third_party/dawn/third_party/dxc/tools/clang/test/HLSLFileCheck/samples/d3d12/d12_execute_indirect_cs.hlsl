// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: bufferUpdateCounter

//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#define threadBlockSize 128

struct ConstantBufferData
{
	float4 velocity;
	float4 offset;
	float4 color;
	float4x4 projection;
	float4 padding[9];
};

struct IndirectCommand
{
	uint2 cbvAddress;
	uint4 drawArguments;
};

cbuffer RootConstants : register(b0)
{
	float xOffset;		// Half the width of the triangles.
	float zOffset;		// The z offset for the triangle vertices.
	float cullOffset;	// The culling plane offset in homogenous space.
	float commandCount;	// The number of commands to be processed.
};

StructuredBuffer<ConstantBufferData> cbv				: register(t0);	// SRV: Wrapped constant buffers
StructuredBuffer<IndirectCommand> inputCommands			: register(t1);	// SRV: Indirect commands
AppendStructuredBuffer<IndirectCommand> outputCommands	: register(u0);	// UAV: Processed indirect commands

[numthreads(threadBlockSize, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
	// Each thread of the CS operates on one of the indirect commands.
	uint index = (groupId.x * threadBlockSize) + groupIndex;

	// Don't attempt to access commands that don't exist if more threads are allocated
	// than commands.
	if (index < commandCount)
	{
		// Project the left and right bounds of the triangle into homogenous space.
		float4 left = float4(-xOffset, 0.0f, zOffset, 1.0f) + cbv[index].offset;
		left = mul(left, cbv[index].projection);
		left /= left.w;

		float4 right = float4(xOffset, 0.0f, zOffset, 1.0f) + cbv[index].offset;
		right = mul(right, cbv[index].projection);
		right /= right.w;

		// Only draw triangles that are within the culling space.
		if (-cullOffset < right.x && left.x < cullOffset)
		{
			outputCommands.Append(inputCommands[index]);
		}
	}
}
