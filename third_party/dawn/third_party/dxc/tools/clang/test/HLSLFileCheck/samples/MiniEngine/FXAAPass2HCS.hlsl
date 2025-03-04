// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: textureLoad
// CHECK: FAbs
// CHECK: FMin
// CHECK: FMax
// CHECK: sampleLevel
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
//

#include "FXAAPass2CS.hlsli"
