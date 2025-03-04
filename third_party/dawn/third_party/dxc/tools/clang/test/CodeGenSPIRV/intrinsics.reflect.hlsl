// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// HLSL reflect() only operates on vectors of floats.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float4 a1,a2,result;

// CHECK: {{%[0-9]+}} = OpExtInst %v4float [[glsl]] Reflect {{%[0-9]+}} {{%[0-9]+}}
  result = reflect(a1,a2);
}
