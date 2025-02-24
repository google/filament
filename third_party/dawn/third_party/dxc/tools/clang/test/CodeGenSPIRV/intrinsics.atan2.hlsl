// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'atan' function can only operate on float, vector of float, and matrix of floats.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float    a1, a2, atan2a;
  float4   b1, b2, atan2b;
  float2x3 c1, c2, atan2c;
  
// CHECK:          [[a1:%[0-9]+]] = OpLoad %float %a1
// CHECK-NEXT:     [[a2:%[0-9]+]] = OpLoad %float %a2
// CHECK-NEXT: [[atan2a:%[0-9]+]] = OpExtInst %float [[glsl]] Atan2 [[a1]] [[a2]]
// CHECK-NEXT:                   OpStore %atan2a [[atan2a]]
  atan2a = atan2(a1, a2);

// CHECK:          [[b1:%[0-9]+]] = OpLoad %v4float %b1
// CHECK-NEXT:     [[b2:%[0-9]+]] = OpLoad %v4float %b2
// CHECK-NEXT: [[atan2b:%[0-9]+]] = OpExtInst %v4float [[glsl]] Atan2 [[b1]] [[b2]]
// CHECK-NEXT:                   OpStore %atan2b [[atan2b]]
  atan2b = atan2(b1, b2);

// CHECK:               [[c1:%[0-9]+]] = OpLoad %mat2v3float %c1
// CHECK-NEXT:          [[c2:%[0-9]+]] = OpLoad %mat2v3float %c2
// CHECK-NEXT:     [[c1_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 0
// CHECK-NEXT:     [[c2_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 0
// CHECK-NEXT: [[atan2c_row0:%[0-9]+]] = OpExtInst %v3float [[glsl]] Atan2 [[c1_row0]] [[c2_row0]]
// CHECK-NEXT:     [[c1_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 1
// CHECK-NEXT:     [[c2_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 1
// CHECK-NEXT: [[atan2c_row1:%[0-9]+]] = OpExtInst %v3float [[glsl]] Atan2 [[c1_row1]] [[c2_row1]]
// CHECK-NEXT:      [[atan2c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[atan2c_row0]] [[atan2c_row1]]
// CHECK-NEXT:                        OpStore %atan2c [[atan2c]]
  atan2c = atan2(c1, c2);
}
