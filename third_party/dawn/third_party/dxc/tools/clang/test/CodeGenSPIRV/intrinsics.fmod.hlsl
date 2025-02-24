// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
  float    a1, a2, fmod_a;
  float4   b1, b2, fmod_b;
  float2x3 c1, c2, fmod_c;

// CHECK:      [[a1:%[0-9]+]] = OpLoad %float %a1
// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %float %a2
// CHECK-NEXT:    {{%[0-9]+}} = OpFRem %float [[a1]] [[a2]]
  fmod_a = fmod(a1, a2);

// CHECK:      [[b1:%[0-9]+]] = OpLoad %v4float %b1
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %v4float %b2
// CHECK-NEXT:    {{%[0-9]+}} = OpFRem %v4float [[b1]] [[b2]]
  fmod_b = fmod(b1, b2);

// CHECK:               [[c1:%[0-9]+]] = OpLoad %mat2v3float %c1
// CHECK-NEXT:          [[c2:%[0-9]+]] = OpLoad %mat2v3float %c2
// CHECK-NEXT:     [[c1_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 0
// CHECK-NEXT:     [[c2_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 0
// CHECK-NEXT: [[fmod_c_row0:%[0-9]+]] = OpFRem %v3float [[c1_row0]] [[c2_row0]]
// CHECK-NEXT:     [[c1_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 1
// CHECK-NEXT:     [[c2_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 1
// CHECK-NEXT: [[fmod_c_row1:%[0-9]+]] = OpFRem %v3float [[c1_row1]] [[c2_row1]]
// CHECK-NEXT:             {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[fmod_c_row0]] [[fmod_c_row1]]
  fmod_c = fmod(c1, c2);
}
