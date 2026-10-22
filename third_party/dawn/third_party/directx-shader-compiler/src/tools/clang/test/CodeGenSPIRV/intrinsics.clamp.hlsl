// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {

// CHECK:      [[int_x:%[0-9]+]] = OpLoad %int %int_x
// CHECK-NEXT: [[int_min:%[0-9]+]] = OpLoad %int %int_min
// CHECK-NEXT: [[int_max:%[0-9]+]] = OpLoad %int %int_max
// CHECK-NEXT: {{%[0-9]+}} = OpExtInst %int [[glsl]] SClamp [[int_x]] [[int_min]] [[int_max]]
  int int_x, int_min, int_max;
  int int_result = clamp(int_x, int_min, int_max);

// CHECK: {{%[0-9]+}} = OpExtInst %v2uint [[glsl]] UClamp {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}}
  uint2 uint2_x, uint2_min, uint2_max;
  uint2 uint2_result = clamp(uint2_x, uint2_min, uint2_max);

// CHECK: {{%[0-9]+}} = OpExtInst %v3float [[glsl]] FClamp {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}}
  float3 float3_x, float3_min, float3_max;
  float3 float3_result = clamp(float3_x, float3_min, float3_max);

// CHECK:      [[mat_x:%[0-9]+]] = OpLoad %mat4v4float %float4x4_x
// CHECK-NEXT: [[mat_min:%[0-9]+]] = OpLoad %mat4v4float %float4x4_min
// CHECK-NEXT: [[mat_max:%[0-9]+]] = OpLoad %mat4v4float %float4x4_max
// CHECK-NEXT: [[x_row0:%[0-9]+]] = OpCompositeExtract %v4float [[mat_x]] 0
// CHECK-NEXT: [[min_row0:%[0-9]+]] = OpCompositeExtract %v4float [[mat_min]] 0
// CHECK-NEXT: [[max_row0:%[0-9]+]] = OpCompositeExtract %v4float [[mat_max]] 0
// CHECK-NEXT: [[clamp_row0:%[0-9]+]] = OpExtInst %v4float [[glsl]] FClamp [[x_row0]] [[min_row0]] [[max_row0]]
// CHECK-NEXT: [[x_row1:%[0-9]+]] = OpCompositeExtract %v4float [[mat_x]] 1
// CHECK-NEXT: [[min_row1:%[0-9]+]] = OpCompositeExtract %v4float [[mat_min]] 1
// CHECK-NEXT: [[max_row1:%[0-9]+]] = OpCompositeExtract %v4float [[mat_max]] 1
// CHECK-NEXT: [[clamp_row1:%[0-9]+]] = OpExtInst %v4float [[glsl]] FClamp [[x_row1]] [[min_row1]] [[max_row1]]
// CHECK-NEXT: [[x_row2:%[0-9]+]] = OpCompositeExtract %v4float [[mat_x]] 2
// CHECK-NEXT: [[min_row2:%[0-9]+]] = OpCompositeExtract %v4float [[mat_min]] 2
// CHECK-NEXT: [[max_row2:%[0-9]+]] = OpCompositeExtract %v4float [[mat_max]] 2
// CHECK-NEXT: [[clamp_row2:%[0-9]+]] = OpExtInst %v4float [[glsl]] FClamp [[x_row2]] [[min_row2]] [[max_row2]]
// CHECK-NEXT: [[x_row3:%[0-9]+]] = OpCompositeExtract %v4float [[mat_x]] 3
// CHECK-NEXT: [[min_row3:%[0-9]+]] = OpCompositeExtract %v4float [[mat_min]] 3
// CHECK-NEXT: [[max_row3:%[0-9]+]] = OpCompositeExtract %v4float [[mat_max]] 3
// CHECK-NEXT: [[clamp_row3:%[0-9]+]] = OpExtInst %v4float [[glsl]] FClamp [[x_row3]] [[min_row3]] [[max_row3]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %mat4v4float [[clamp_row0]] [[clamp_row1]] [[clamp_row2]] [[clamp_row3]]
  float4x4 float4x4_x, float4x4_min, float4x4_max;
  float4x4 float4x4_result = clamp(float4x4_x, float4x4_min, float4x4_max);
}
