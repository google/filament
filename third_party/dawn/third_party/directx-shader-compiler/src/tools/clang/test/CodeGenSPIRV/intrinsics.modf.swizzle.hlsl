// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
// CHECK:      [[swizzle:%[0-9]+]] = OpLoad %v3int %v3i
// CHECK-NEXT:  [[vector:%[0-9]+]] = OpVectorShuffle %v3int [[swizzle]] {{%[0-9]+}} 3 4 2
// CHECK-NEXT:                    OpStore %v3i [[vector]]
  float2 v2f;
  int3 v3i = 0;
  modf(v2f, v3i.xy);
}
