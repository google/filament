// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// A simple test to ensure that an unconditional break
// followed by additional statements is handled correctly.

// make sure the cos is included and the dot isn't
//CHECK: dx.op.unary
//CHECK-NOT: dx.op.dot

float4 main(float4 a:A, int b:B) :SV_Target
{
  while(b) {
    b--;
    a += cos(a);
    break;
    a += dot(a,a);
  }
  return a;
}
