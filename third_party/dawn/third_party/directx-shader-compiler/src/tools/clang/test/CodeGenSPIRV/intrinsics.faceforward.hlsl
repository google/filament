// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// faceforward only takes vector of floats, and returns a vector of the same size.

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float3 n, i, ng, result;
  
// CHECK:       [[n:%[0-9]+]] = OpLoad %v3float %n
// CHECK-NEXT:  [[i:%[0-9]+]] = OpLoad %v3float %i
// CHECK-NEXT: [[ng:%[0-9]+]] = OpLoad %v3float %ng
// CHECK-NEXT: {{%[0-9]+}} = OpExtInst %v3float [[glsl]] FaceForward [[n]] [[i]] [[ng]]
  result = faceforward(n, i, ng);
}
