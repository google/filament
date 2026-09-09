// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK-DAG: define float @"\01?lib2_fn{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: declare float @"\01?external_fn{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: declare float @"\01?external_fn2{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: define float @"\01?call_lib1{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-DAG: declare float @"\01?lib1_fn{{[@$?.A-Za-z0-9_]+}}"()
// CHECK-NOT: @"\01?unused_fn2

float external_fn();
float external_fn2();
float lib1_fn();
float unused_fn2();

float lib2_fn() {
  if (false)
    return unused_fn2();
  return 22.0 * external_fn() * external_fn2();
}

float call_lib1() {
  return lib1_fn();
}
