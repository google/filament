// RUN: %dxc -T ps_6_0 -E main -Zi %s -spirv -fspv-debug=vulkan-with-source | FileCheck %s

// CHECK:      [[main:%[0-9]+]] = OpString
// CHECK-SAME: shader.debug.opsource.include.hlsl
// CHECK:      [[file1:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opsource.include-file.hlsli
// CHECK:      [[file1string:%[0-9]+]] = OpString "#define UBER_TYPE(x) x ## Type
// CHECK:      [[mainstring:%[0-9]+]] = OpString "// RUN: %dxc -T ps_6_0 -E main -Zi %s -spirv -fspv-debug=vulkan-with-source | FileCheck %s

// CHECK:      DebugSource [[file1]] [[file1string]] 
// CHECK:      DebugSource [[main]] [[mainstring]] 

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
