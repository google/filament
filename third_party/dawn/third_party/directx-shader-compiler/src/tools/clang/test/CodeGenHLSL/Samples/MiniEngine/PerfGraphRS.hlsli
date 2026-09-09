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

#define PerfGraph_RootSig \
	"RootFlags(0), " \
	"CBV(b0)," \
	"DescriptorTable(UAV(u0, numDescriptors = 2))," \
	"SRV(t0, visibility = SHADER_VISIBILITY_VERTEX)," \
	"RootConstants(b1, num32BitConstants = 3)"
