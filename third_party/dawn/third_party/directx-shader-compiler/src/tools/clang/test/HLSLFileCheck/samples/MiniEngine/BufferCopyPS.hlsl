// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: textureLoad

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

#include "PresentRS.hlsli"

Texture2D ColorTex : register(t0);

[RootSignature(Present_RootSig)]
float4 main( float4 position : SV_Position ) : SV_Target0
{
	return ColorTex[(int2)position.xy];
}