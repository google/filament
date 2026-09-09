// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'cross' function can only operate on float3 vectors.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float3 a,b,c;

// CHECK:      [[a:%[0-9]+]] = OpLoad %v3float %a
// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %v3float %b
// CHECK-NEXT: [[c:%[0-9]+]] = OpExtInst %v3float [[glsl]] Cross [[a]] [[b]]
  c = cross(a,b);
}
