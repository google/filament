// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadIdInGroup
// CHECK: threadId
// CHECK: UMax
// CHECK: textureGather
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: FMax
// CHECK: FMin
// CHECK: FAbs
// CHECK: Saturate
// CHECK: bufferUpdateCounter
// CHECK: bufferStore
// CHECK: bufferUpdateCounter
// CHECK: bufferStore

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

#define USE_LUMA_INPUT_BUFFER
#include "FXAAPass1CS.hlsli"