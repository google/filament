// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure struct param used as out arg works.

// CHECK: call void @"\01?getT{{[@$?.A-Za-z0-9_]+}}"

struct T {
  float a;
  int   b;
};

T getT();

T getT2() {
  return getT();
}
