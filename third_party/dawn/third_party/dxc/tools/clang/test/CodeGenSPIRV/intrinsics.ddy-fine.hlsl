// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability DerivativeControl

void main() {

  float  a;
  float4 b;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:   {{%[0-9]+}} = OpDPdyFine %float [[a]]
  float    da = ddy_fine(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:   {{%[0-9]+}} = OpDPdyFine %v4float [[b]]
  float4   db = ddy_fine(b);
}
