// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float min, max, val;
  float4 min4, max4, val4;
  float2x3 min2x3, max2x3, val2x3;

// CHECK:      [[min:%[0-9]+]] = OpLoad %float %min
// CHECK-NEXT: [[max:%[0-9]+]] = OpLoad %float %max
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT:     {{%[0-9]+}} = OpExtInst %float [[glsl]] SmoothStep [[min]] [[max]] [[val]]
  float       ss = smoothstep(min, max, val);

// CHECK:      [[min4:%[0-9]+]] = OpLoad %v4float %min4
// CHECK-NEXT: [[max4:%[0-9]+]] = OpLoad %v4float %max4
// CHECK-NEXT: [[val4:%[0-9]+]] = OpLoad %v4float %val4
// CHECK-NEXT:      {{%[0-9]+}} = OpExtInst %v4float [[glsl]] SmoothStep [[min4]] [[max4]] [[val4]]
  float4     ss4 = smoothstep(min4, max4, val4);

// CHECK:      [[min2x3:%[0-9]+]] = OpLoad %mat2v3float %min2x3
// CHECK-NEXT: [[max2x3:%[0-9]+]] = OpLoad %mat2v3float %max2x3
// CHECK-NEXT: [[val2x3:%[0-9]+]] = OpLoad %mat2v3float %val2x3
// CHECK-NEXT: [[min_r0:%[0-9]+]] = OpCompositeExtract %v3float [[min2x3]] 0
// CHECK-NEXT: [[max_r0:%[0-9]+]] = OpCompositeExtract %v3float [[max2x3]] 0
// CHECK-NEXT: [[val_r0:%[0-9]+]] = OpCompositeExtract %v3float [[val2x3]] 0
// CHECK-NEXT:  [[ss_r0:%[0-9]+]] = OpExtInst %v3float [[glsl]] SmoothStep [[min_r0]] [[max_r0]] [[val_r0]]
// CHECK-NEXT: [[min_r1:%[0-9]+]] = OpCompositeExtract %v3float [[min2x3]] 1
// CHECK-NEXT: [[max_r1:%[0-9]+]] = OpCompositeExtract %v3float [[max2x3]] 1
// CHECK-NEXT: [[val_r1:%[0-9]+]] = OpCompositeExtract %v3float [[val2x3]] 1
// CHECK-NEXT:  [[ss_r1:%[0-9]+]] = OpExtInst %v3float [[glsl]] SmoothStep [[min_r1]] [[max_r1]] [[val_r1]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[ss_r0]] [[ss_r1]]
  float2x3 ss2x3 = smoothstep(min2x3, max2x3, val2x3);
}
