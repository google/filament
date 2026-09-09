// RUN: %dxc -E main -T ps_6_0 -O0 -HV 2018 %s | FileCheck %s

// CHECK: Frc
// CHECK: IMax
// CHECK: UMin
// CHECK: textureLoad
// CHECK: Log
// CHECK: Exp


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

#define GAMMA_SPACE
#include "BicubicUpsamplePS.hlsl"
