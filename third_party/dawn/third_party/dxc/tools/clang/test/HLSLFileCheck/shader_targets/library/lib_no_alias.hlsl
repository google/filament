// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure out param has no-alias.
// CHECK: float @"\01?test{{[@$?.A-Za-z0-9_]+}}"(float %a, %struct.T* noalias nocapture %t, %class.matrix.float.2.2* noalias nocapture dereferenceable(16) %m, float %b)

struct T {
  float a;
  int b;
};

float test(float a, out T t, out float2x2 m, float b) {
  t.a = 1;
  t.b = 6;
  m = 3;
  return a + t.a - t.b - b;
}
