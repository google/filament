// RUN: %dxc -T ps_6_0 -E main -Zi %s -spirv | FileCheck %s

// CHECK:      [[main:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opsource.include.hlsl
// CHECK:      [[file1:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opsource.include-file.hlsli
// CHECK-NEXT: OpSource HLSL 600 [[main]] "// RUN: %dxc -T ps_6_0 -E main -Zi %s -spirv | FileCheck %s
// CHECK:      OpSource HLSL 600 [[file1]] "#define UBER_TYPE(x) x ## Type

// CHECK:      [[type:%[0-9]+]] = OpTypeFunction %void
// CHECK:      OpLine [[main]] 22 1
// CHECK-NEXT: %main = OpFunction %void None [[type]]

#include "spirv.debug.opsource.include-file.hlsli"

struct ColorType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(UBER_TYPE(Color) input) : SV_TARGET
{
    return input.color;
}
