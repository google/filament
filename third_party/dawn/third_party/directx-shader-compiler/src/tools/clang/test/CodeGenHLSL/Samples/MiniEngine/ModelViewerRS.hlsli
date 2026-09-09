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

#if USE_VERTEX_BUFFER

#define ModelViewer_RootSig \
	"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_PIXEL), " \
	"SRV(t0, visibility = SHADER_VISIBILITY_VERTEX), " \
	"DescriptorTable(SRV(t0, numDescriptors = 6), visibility = SHADER_VISIBILITY_PIXEL)," \
	"DescriptorTable(SRV(t64, numDescriptors = 3), visibility = SHADER_VISIBILITY_PIXEL)," \
	"RootConstants(b1, num32BitConstants = 1, visibility = SHADER_VISIBILITY_VERTEX), " \
	"StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
	"StaticSampler(s1, visibility = SHADER_VISIBILITY_PIXEL," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"comparisonFunc = COMPARISON_GREATER_EQUAL," \
		"filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)"

#elif USE_ROOT_BUFFER_SRV

#define ModelViewer_RootSig \
	"RootFlags(0), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_PIXEL), " \
	"SRV(t0, visibility = SHADER_VISIBILITY_VERTEX), " \
	"DescriptorTable(SRV(t0, numDescriptors = 6), visibility = SHADER_VISIBILITY_PIXEL)," \
	"DescriptorTable(SRV(t64, numDescriptors = 3), visibility = SHADER_VISIBILITY_PIXEL)," \
	"RootConstants(b1, num32BitConstants = 1, visibility = SHADER_VISIBILITY_VERTEX), " \
	"StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
	"StaticSampler(s1, visibility = SHADER_VISIBILITY_PIXEL," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"comparisonFunc = COMPARISON_GREATER_EQUAL," \
		"filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)"

#else

#define ModelViewer_RootSig \
	"RootFlags(0), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_PIXEL), " \
	"DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_VERTEX)," \
	"DescriptorTable(SRV(t0, numDescriptors = 6), visibility = SHADER_VISIBILITY_PIXEL)," \
	"DescriptorTable(SRV(t64, numDescriptors = 3), visibility = SHADER_VISIBILITY_PIXEL)," \
	"RootConstants(b1, num32BitConstants = 1, visibility = SHADER_VISIBILITY_VERTEX), " \
	"StaticSampler(s0, maxAnisotropy = 8, visibility = SHADER_VISIBILITY_PIXEL)," \
	"StaticSampler(s1, visibility = SHADER_VISIBILITY_PIXEL," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"comparisonFunc = COMPARISON_GREATER_EQUAL," \
		"filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)"

#endif