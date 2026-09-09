// RUN: %dxc -E main -T vs_6_0 -I %S/Inputs %s | FileCheck %s

// CHECK: SV_VertexID

// RUN: %dxc %s /Tvs_6_0 /Zi /Qembed_debug -I %S/Inputs /Fo %t.TextVS.cso

// RUN: %dxc %S/Inputs/smoke.hlsl /D "semantic = SV_Position" /T vs_6_0 /Zi /Qembed_debug /DDX12 /Fo %t.test.vs.cso

// RUN: %dxc %t.test.vs.cso /dumpbin /verifyrootsignature %t.TextVS.cso


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

#include "TextRS.hlsli"

cbuffer cbFontParams : register( b0 )
{
	float2 Scale;			// Scale and offset for transforming coordinates
	float2 Offset;
	float2 InvTexDim;		// Normalizes texture coordinates
	float TextSize;			// Height of text in destination pixels
	float TextScale;		// TextSize / FontHeight
	float DstBorder;		// Extra space around a glyph measured in screen space coordinates
	uint SrcBorder;			// Extra spacing around glyphs to avoid sampling neighboring glyphs
}

struct VS_INPUT
{
	float2 ScreenPos : POSITION;	// Upper-left position in screen pixel coordinates
	uint4  Glyph : TEXCOORD;		// X, Y, Width, Height in texel space
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;	// Upper-left and lower-right coordinates in clip space
	float2 Tex : TEXCOORD0;		// Upper-left and lower-right normalized UVs
};

[RootSignature(Text_RootSig)]
VS_OUTPUT main( VS_INPUT input, uint VertID : SV_VertexID )
{
	const float2 xy0 = input.ScreenPos - DstBorder;
	const float2 xy1 = input.ScreenPos + DstBorder + float2(TextScale * input.Glyph.z, TextSize);
	const uint2 uv0 = input.Glyph.xy - SrcBorder;
	const uint2 uv1 = input.Glyph.xy + SrcBorder + input.Glyph.zw;

	float2 uv = float2( VertID & 1, (VertID >> 1) & 1 );

	VS_OUTPUT output;
	output.Pos = float4( lerp(xy0, xy1, uv) * Scale + Offset, 0, 1 );
	output.Tex = lerp(uv0, uv1, uv) * InvTexDim;
	return output;
}
