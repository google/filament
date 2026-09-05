// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"
// CHECK: [[v4f0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK: [[v4f1:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
// CHECK: [[v3f0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK: [[v3f1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1

void main() {
  float    a, sata;
  float4   b, satb;
  float2x3 c, satc;
  float1   d;
  float1x1 e;

// CHECK:         [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[sata:%[0-9]+]] = OpExtInst %float [[glsl]] FClamp [[a]] %float_0 %float_1
// CHECK-NEXT:                 OpStore %sata [[sata]]
  sata = saturate(a);

// CHECK:         [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT: [[satb:%[0-9]+]] = OpExtInst %v4float [[glsl]] FClamp [[b]] [[v4f0]] [[v4f1]]
// CHECK-NEXT:                 OpStore %satb [[satb]]
  satb = saturate(b);

// CHECK:      [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT: [[row0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT: [[sat0:%[0-9]+]] = OpExtInst %v3float [[glsl]] FClamp [[row0]] [[v3f0]] [[v3f1]]
// CHECK-NEXT: [[row1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT: [[sat1:%[0-9]+]] = OpExtInst %v3float [[glsl]] FClamp [[row1]] [[v3f0]] [[v3f1]]
// CHECK-NEXT: [[satc:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[sat0]] [[sat1]]
// CHECK-NEXT: OpStore %satc [[satc]]
  satc = saturate(c);

// CHECK:      [[d:%[0-9]+]] = OpLoad %float %d
// CHECK-NEXT: [[a_0:%[0-9]+]] = OpExtInst %float [[glsl]] FClamp [[d]] %float_0 %float_1
// CHECK-NEXT:              OpStore %a [[a_0]]
  a = saturate(d);

// CHECK:      [[e:%[0-9]+]] = OpLoad %float %e
// CHECK-NEXT: [[a_1:%[0-9]+]] = OpExtInst %float [[glsl]] FClamp [[e]] %float_0 %float_1
// CHECK-NEXT:              OpStore %a [[a_1]]
  a = saturate(e);
}
