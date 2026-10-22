// RUN: %dxc -E main -T cs_6_0 -HV 2018 %s | FileCheck %s

// CHECK: threadId
// CHECK: textureLoad
// CHECK: sampleLevel
// CHECK: dot3
// CHECK: Exp
// CHECK: Log

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

#define PRESERVE_HUE 1
#include "ToneMapCS.hlsl"
