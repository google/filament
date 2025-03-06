// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadIdInGroup
// CHECK: threadId
// CHECK: textureGather
// CHECK: barrier
// CHECK: Saturate
// CHECK: FMax
// CHECK: FMin

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


#include "AoRenderCS.hlsli"