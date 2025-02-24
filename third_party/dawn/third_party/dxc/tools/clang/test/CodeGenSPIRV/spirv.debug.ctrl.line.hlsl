// RUN: %dxc -T ps_6_1 -E main -fspv-target-env=vulkan1.1 -fspv-debug=line -fcgl  %s -spirv | FileCheck %s

// Have file path
// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.ctrl.line.hlsl
// CHECK:      OpSource HLSL 610 [[file]]
// Have source code
// CHECK:      float4 main(uint val
// Have line
// CHECK:      OpLine

float4 main(uint val : A) : SV_Target {
  uint a = reversebits(val);
  return a;
}
