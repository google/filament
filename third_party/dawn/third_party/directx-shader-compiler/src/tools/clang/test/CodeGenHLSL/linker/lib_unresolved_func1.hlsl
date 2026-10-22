// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK-DAG: define float @"\01?lib1_fn{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: declare float @"\01?external_fn{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: declare float @"\01?external_fn1{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: define float @"\01?call_lib2{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: declare float @"\01?lib2_fn{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-NOT: @"\01?unused_fn1

float external_fn();
float external_fn1();
float lib2_fn();
float unused_fn1();

float lib1_fn() {
  if (false)
    return unused_fn1();
  return 11.0 * external_fn() * external_fn1();
}

float call_lib2() {
  return lib2_fn();
}
