// RUN: %dxc -T cs_6_0 -E mymain %s | FileCheck %s

// CHECK: error: entry type 'compute' from profile 'cs_6_0' conflicts with shader attribute type 'pixel' on entry function 'mymain'.


[shader("pixel")]
[numthreads(1, 0, 0)]
void mymain() {
  return;
}
