// RUN: %dxc -T ps_6_1 -DFUNC=ddx %s        | FileCheck %s --check-prefixes CHECK,PRE69
// RUN: %dxc -T ps_6_1 -DFUNC=ddx_coarse %s | FileCheck %s --check-prefixes CHECK,PRE69
// RUN: %dxc -T ps_6_1 -DFUNC=ddx_fine %s   | FileCheck %s --check-prefixes CHECK,PRE69
// RUN: %dxc -T ps_6_1 -DFUNC=ddy %s        | FileCheck %s --check-prefixes CHECK,PRE69
// RUN: %dxc -T ps_6_1 -DFUNC=ddy_coarse %s | FileCheck %s --check-prefixes CHECK,PRE69
// RUN: %dxc -T ps_6_1 -DFUNC=ddy_fine %s   | FileCheck %s --check-prefixes CHECK,PRE69

// RUN: %dxc -T ps_6_9 -DFUNC=ddx %s        | FileCheck %s --check-prefixes CHECK,SM69
// RUN: %dxc -T ps_6_9 -DFUNC=ddx_coarse %s | FileCheck %s --check-prefixes CHECK,SM69
// RUN: %dxc -T ps_6_9 -DFUNC=ddx_fine %s   | FileCheck %s --check-prefixes CHECK,SM69
// RUN: %dxc -T ps_6_9 -DFUNC=ddy %s        | FileCheck %s --check-prefixes CHECK,SM69
// RUN: %dxc -T ps_6_9 -DFUNC=ddy_coarse %s | FileCheck %s --check-prefixes CHECK,SM69
// RUN: %dxc -T ps_6_9 -DFUNC=ddy_fine %s   | FileCheck %s --check-prefixes CHECK,SM69

// Make sure add(s) are not sunk into the conditional block.
// SM69: fadd fast <2 x float>
// PRE69: fadd fast float
// PRE69: fadd fast float
// CHECK: icmp sgt
// CHECK-NEXT: br i1

// Source for test of dxil-convergent pass.

float2 main(float2 a:A, float2 b:B, int c:C) : SV_Target {

  float2 coord = a + b;
  float2 res = 0;
  if (c > 2)
    res -= FUNC(coord);

  return res;

}
