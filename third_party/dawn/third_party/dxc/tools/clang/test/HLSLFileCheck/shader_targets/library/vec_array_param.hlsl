// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// Make sure vector array param works.
// CHECK:%[[Cast:.+]] = bitcast [9 x float]* %{{.+}} to [3 x <3 x float>]*
// CHECK:call float @"\01?foo{{[@$?.A-Za-z0-9_]+}}"([3 x <3 x float>]* %[[Cast]]

float foo(float3 a[3]);

export
float bar(float3 a[3]) {
  return foo(a);
}
