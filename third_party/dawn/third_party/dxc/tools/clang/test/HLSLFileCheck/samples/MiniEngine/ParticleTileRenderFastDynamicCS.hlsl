// RUN: %dxc -E main -T cs_6_0 -O0 %s | FileCheck %s

// CHECK: groupId
// CHECK: threadIdInGroup
// CHECK: bufferLoad
// CHECK: FirstbitLo
// CHECK: sampleLevel
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

#define DYNAMIC_RESOLUTION
#define DISABLE_DEPTH_TESTS
#include "ParticleTileRenderCS.hlsl"