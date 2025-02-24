// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: Sqrt
// CHECK: FMin
// CHECK: sampleLevel
// CHECK: sampleLevel
// CHECK: Round_pi
// CHECK: sampleLevel
// CHECK: sampleLevel
// CHECK: sampleLevel



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

#define TEMPORAL_UPSAMPLE
#include "MotionBlurFinalPassCS.hlsl"