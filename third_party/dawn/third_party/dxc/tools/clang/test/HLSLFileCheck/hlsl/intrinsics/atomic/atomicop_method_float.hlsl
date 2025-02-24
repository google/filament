// RUN: %dxc -T ps_6_6 %s | FileCheck %s

RWByteAddressBuffer res;

int main( float a : A) : SV_Target
{
  // Test some disallowed atomic binop intrinsics with floats as both args
  // Since the destination value is a raw buffer, only the provided value can determine the overload
  // Since casts are allowed for the existing methods, these will result in i32 variants.
  // Make sure they are not f32 variants

  uint ix = 0;
  int b;

  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  res.InterlockedAdd(ix, a);
  res.InterlockedMin(ix, a);
  res.InterlockedMax(ix, a);
  res.InterlockedAnd(ix, a);
  res.InterlockedOr(ix, a);
  res.InterlockedXor(ix, a);

  // Try the same with an integer second arg to make sure they still fail

  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  res.InterlockedAdd(ix, a, b);
  res.InterlockedMin(ix, a, b);
  res.InterlockedMax(ix, a, b);
  res.InterlockedAnd(ix, a, b);
  res.InterlockedOr(ix, a, b);
  res.InterlockedXor(ix, a, b);

  // CHECK-NOT: dx.op.atomicBinOp.f32
  return b;
}

