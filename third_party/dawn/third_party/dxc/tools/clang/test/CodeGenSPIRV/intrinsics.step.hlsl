// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float result;
  float2 result2;
  float3 result3;
  float4 result4;
  float2x3 result2x3;

// CHECK: {{%[0-9]+}} = OpExtInst %float [[glsl]] Step {{%[0-9]+}} {{%[0-9]+}}
  float a1,a2;
  result = step(a1,a2);

// CHECK: {{%[0-9]+}} = OpExtInst %float [[glsl]] Step {{%[0-9]+}} {{%[0-9]+}}
  float1 b1,b2;
  result = step(b1,b2);

// CHECK: {{%[0-9]+}} = OpExtInst %v3float [[glsl]] Step {{%[0-9]+}} {{%[0-9]+}}
  float3 c1,c2;
  result3 = step(c1,c2);

// CHECK: {{%[0-9]+}} = OpExtInst %float [[glsl]] Step {{%[0-9]+}} {{%[0-9]+}}
  float1x1 d1,d2;
  result = step(d1,d2);

// CHECK: {{%[0-9]+}} = OpExtInst %v2float [[glsl]] Step {{%[0-9]+}} {{%[0-9]+}}
  float1x2 e1,e2;
  result2 = step(e1,e2);

// CHECK: {{%[0-9]+}} = OpExtInst %v4float [[glsl]] Step {{%[0-9]+}} {{%[0-9]+}}
  float4x1 f1,f2;
  result4 = step(f1,f2);

// CHECK:      [[g1:%[0-9]+]] = OpLoad %mat2v3float %g1
// CHECK-NEXT: [[g2:%[0-9]+]] = OpLoad %mat2v3float %g2
// CHECK-NEXT: [[g1_row0:%[0-9]+]] = OpCompositeExtract %v3float [[g1]] 0
// CHECK-NEXT: [[g2_row0:%[0-9]+]] = OpCompositeExtract %v3float [[g2]] 0
// CHECK-NEXT: [[result_row0:%[0-9]+]] = OpExtInst %v3float [[glsl]] Step [[g1_row0]] [[g2_row0]]
// CHECK-NEXT: [[g1_row1:%[0-9]+]] = OpCompositeExtract %v3float [[g1]] 1
// CHECK-NEXT: [[g2_row1:%[0-9]+]] = OpCompositeExtract %v3float [[g2]] 1
// CHECK-NEXT: [[result_row1:%[0-9]+]] = OpExtInst %v3float [[glsl]] Step [[g1_row1]] [[g2_row1]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[result_row0]] [[result_row1]]
  float2x3 g1,g2;
  result2x3 = step(g1,g2);
}
