// RUN: %dxc -T lib_6_3 -DTYPE=float -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -T lib_6_3 -DTYPE=float2x2 -Od %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -T lib_6_3 -DTYPE=float2x2 -Od -Zpr %s | FileCheck %s -check-prefixes=CHECK
// RUN: %dxc -T lib_6_3 -DTYPE=float2x2 -DTYPEMOD=row_major -Od -Zpr %s | FileCheck %s -check-prefixes=CHECK
// These need to copy:
// RUN: %dxc -T lib_6_3 -DTYPE=float2x2 -DTYPEMOD=row_major -Od %s | FileCheck %s -check-prefixes=CHECK,COPY
// RUN: %dxc -T lib_6_3 -DTYPE=float2x2 -DTYPEMOD=column_major -Od -Zpr %s | FileCheck %s -check-prefixes=CHECK,COPY

// Make sure array is not copied unless matrix orientation changed
// CHECK: alloca
// COPY: alloca
// CHECK-NOT: alloca
// CHECK: ret

#ifndef TYPEMOD
#define TYPEMOD
#endif

TYPE lookup(in TYPE arr[16], int index) {
  return arr[index];
}

int idx;

[shader("vertex")]
TYPE main() : OUT {
  TYPEMOD TYPE arr[16];
  for (int i = 0; i < 16; i++) {
    arr[i] = (TYPE)i;
  }
  return lookup(arr, idx);
}


