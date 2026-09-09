// RUN: %dxc -T lib_6_6 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure calls with empty struct params are well-behaved

// CHECK: define float @"\01?test2{{[@$?.A-Za-z0-9_]+}}"(%struct.T* nocapture readnone %t)
// CHECK-NOT:memcpy
// CHECK-NOT:load
// CHECK-NOT:store
// CHECK-DAG: call float @"\01?test{{[@$?.A-Za-z0-9_]+}}"(%struct.T*


struct T {
};

float test(T t);

float test2(T t): SV_Target {
  return test(t);
}
