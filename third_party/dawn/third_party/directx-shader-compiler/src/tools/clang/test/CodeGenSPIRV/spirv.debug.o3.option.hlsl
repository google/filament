// RUN: %dxc -E main -T vs_6_0 -fspv-target-env=vulkan1.1 -Zi -O3  %s -spirv | FileCheck %s

// This test ensures that the debug info generation does not cause
// crash when we enable spirv-opt with it.

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.o3.option.hlsl

struct VSOUT {
  float4 pos   : SV_POSITION;
  float4 color : COLOR;
};

// TODO: After correctly propagate the line information in spirv-opt,
//       add checking OpLine instructions.
VSOUT main(float4 pos   : POSITION,
           float4 color : COLOR)
{
  VSOUT foo;
  foo.pos = pos;
  foo.color = color;
  return foo;
}
