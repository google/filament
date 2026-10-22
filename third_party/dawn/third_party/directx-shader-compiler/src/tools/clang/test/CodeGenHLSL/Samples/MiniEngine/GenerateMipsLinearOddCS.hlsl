// RUN: %dxc -E main -T cs_6_0 -HV 2018 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: threadId
// CHECK: sampleLevel
// CHECK: sampleLevel
// CHECK: sampleLevel
// CHECK: sampleLevel
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier

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

#define NON_POWER_OF_TWO 3
#include "GenerateMipsCS.hlsli"
