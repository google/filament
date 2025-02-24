// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure extern function don't need to flat is called.
// CHECK: call void @"\01?test{{[@$?.A-Za-z0-9_]+}}"

void test(float a, out float b);

float test2(float a) {
  float b;
  test(a, b);
  return b;
}
