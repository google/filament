// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: threadIdInGroup
// CHECK: threadId
// CHECK: textureGather
// CHECK: barrier
// CHECK: FAbs
// CHECK: dot4

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

#define COMBINE_LOWER_RESOLUTIONS
#define BLEND_WITH_HIGHER_RESOLUTION

#include "AoBlurAndUpsampleCS.hlsli"