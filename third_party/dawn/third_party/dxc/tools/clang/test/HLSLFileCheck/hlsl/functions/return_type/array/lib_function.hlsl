// RUN: %dxc -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// Tests that non-entry point functions can return arrays in library profile.

// CHECK: define void
// CHECK-SAME: getA
// CHECK-SAME: ([2 x i32]* noalias nocapture sret %[[result:.*]])
export int getA() [2]
{
  // CHECK: %[[gep1:.*]] = getelementptr inbounds [2 x i32], [2 x i32]* %[[result]], i32 0, i32 0
  // CHECK: store i32 1, i32* %[[gep1]]
  // CHECK: %[[gep2:.*]] = getelementptr inbounds [2 x i32], [2 x i32]* %[[result]], i32 0, i32 1
  // CHECK: store i32 2, i32* %[[gep2]]
  int a[2] = { 1, 2 };
  return a;
}
// CHECK: ret void
