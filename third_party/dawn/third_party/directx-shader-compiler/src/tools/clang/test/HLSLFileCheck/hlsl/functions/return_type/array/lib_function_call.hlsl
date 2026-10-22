// RUN: %dxc -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// Tests that we don't drop an array result when calling an external function returning an array.

// CHECK: call void
// CHECK-SAME: getA
// CHECK-SAME: ([2 x i32]* nonnull sret %[[tmp:.*]])
// CHECK: %[[arrayidx:.*]] = getelementptr inbounds [2 x i32], [2 x i32]* %[[tmp]], i32 0, i32 0
// CHECK: %[[val:.*]] = load i32, i32* %[[arrayidx]]
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %[[val]])

export int getA() [2];

[shader("vertex")]
int main() : OUT {
  return getA()[0];
}
