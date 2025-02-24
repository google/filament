// RUN: %dxc -E main -T vs_6_0 -Zi -O2 -fcgl  %s -spirv | FileCheck %s

// This test ensures that the debug info generation does not cause
// crash when we enable spirv-opt with it.

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.o2.option.hlsl

struct VSOUT {
  float4 pos   : SV_POSITION;
  float4 color : COLOR;
};

// CHECK:      OpLine [[file]] 15 1
VSOUT main(float4 pos   : POSITION,
           float4 color : COLOR)
{
  VSOUT foo;
  foo.pos = pos;
  foo.color = color;
  return foo;
}
