// RUN: %dxc -E main -T ps_6_2 %s | FileCheck %s

// Make sure frexp generate code pattern.
// CHECK:bitcast float {{.*}} to i32
// CHECK:and i32 {{.*}}, 2139095040
// CHECK:add {{.*}}, -1056964608
// CHECK:ashr {{.*}}, 23
// CHECK:sitofp
// CHECK:and i32 {{.*}}, 8388607
// CHECK:or i32 {{.*}}, 1056964608
// CHECK:fadd

float main(float a:A) : SV_Target {
  float b;
  float c = frexp ( a , b );

  return b+c;
}