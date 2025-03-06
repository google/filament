// RUN: %dxc -E main -T ps_6_0 | FileCheck %s

// CHECK: Sqrt
// CHECK: float 4.000000e+00
// CHECK: fadd
// CHECK: 2.000000e+00
// CHECK: fmul
// CHECK: 2.000000e+00
// CHECK: fsub
// CHECK: -1.000000e+00

float4 main(float4 a : A) : SV_TARGET {
// refract(i, n, eta):
  //  d = dot(i,n);
  //  t = 1 - eta * eta * ( 1 - d*d);
  //  cond = t >= 1;
  //  r = eta * i - (eta * d + sqrt(t)) * n;
  //  return cond ? r : 0;
// -> d = dot(1, 2) = 2
//      t = 1 - ( 1 - 4) = 4
//      cond = 4 >= 1 is true
//      r = 1 - ( 2 + sqrt(4)) * 2 = 1 - (2 + sqrt(4)) * 2
//      return r;

// reflect(i, n): = i - 2 * n * dot(i, n) -> 1- 4*2 = -7
// dot(2,3) -> 6
// faceforward(n,i,ng): -nx sign(dot(i, ng))  -> -1

// 1 - (2 + sqrt(4)) * 2 + -7 + 6 + -1 = -1 (2 + sqrt(4)) * 2
// = -1 - (2 + sqrt(4)) * 2
  return refract(1,2,1) + reflect(1,2) + dot(2,3) + faceforward(1,1,1);
}
