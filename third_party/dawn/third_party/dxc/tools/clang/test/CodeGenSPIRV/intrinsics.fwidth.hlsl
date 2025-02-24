// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {

  float    a;
  float2   b;
  float2x3 c;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:   {{%[0-9]+}} = OpFwidth %float [[a]]
  float    fwa = fwidth(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v2float %b
// CHECK-NEXT:   {{%[0-9]+}} = OpFwidth %v2float [[b]]
  float2   fwb = fwidth(b);

// CHECK:        [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:  [[c0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT: [[dc0:%[0-9]+]] = OpFwidth %v3float [[c0]]
// CHECK-NEXT:  [[c1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT: [[dc1:%[0-9]+]] = OpFwidth %v3float [[c1]]
// CHECK-NEXT:     {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[dc0]] [[dc1]]
  float2x3 fwc = fwidth(c);
}
