// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Check that matrix pre-increment/decrement can be chained.

int2 main() : OUT
{
  int1x1 variable = 10;
  int1x1 result = --(++(++(++variable)));
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 12)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 12)
  return int2(variable._11, result._11);
}