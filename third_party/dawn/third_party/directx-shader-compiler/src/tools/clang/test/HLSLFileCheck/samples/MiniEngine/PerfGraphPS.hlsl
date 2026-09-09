// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: loadInput
// CHECK: storeOutput

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

#include "PerfGraphRS.hlsli"

struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 col : COLOR;
};

[RootSignature(PerfGraph_RootSig)]
float4 main( VSOutput input ) : SV_TARGET
{
	return float4(input.col, 0.75);
}