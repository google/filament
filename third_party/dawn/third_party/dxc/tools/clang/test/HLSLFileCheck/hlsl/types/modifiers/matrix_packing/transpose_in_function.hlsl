// RUN: %dxc /T vs_6_0 /E main  %s | FileCheck %s

// Regression test for a bug where the transpose isn't performed,
// or is performed twice, when wrapped in its own function.

typedef row_major int2x2 rmi2x2;
rmi2x2 DoTranspose(rmi2x2 mat) { return transpose(mat); }
int4 main() : OUT
{
  rmi2x2 mat = DoTranspose(rmi2x2(11, 12, 21, 22));
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 11)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 21)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 12)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 22)
  return int4(mat._11, mat._12, mat._21, mat._22);
}