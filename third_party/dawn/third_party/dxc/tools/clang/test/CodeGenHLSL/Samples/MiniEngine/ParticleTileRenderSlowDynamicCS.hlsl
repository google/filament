// RUN: %dxc -E main -T cs_6_0 -O0 %s | FileCheck %s

// CHECK: groupId
// CHECK: threadIdInGroup
// CHECK: bufferLoad
// CHECK: textureLoad
// CHECK: textureGather
// CHECK: FirstbitLo
// CHECK: bufferLoad
// CHECK: Saturate
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
#include "ParticleTileRenderCS.hlsl"