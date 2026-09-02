// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Minimum-precision data types
// CHECK: fptrunc float
// CHECK: to half

min16float cast(float a) {
  return a;
}

float main(float a : A) : SV_Target
{
  return cast(a);
}
