// RUN: %dxc -T ps_6_3 %s | FileCheck %s
StructuredBuffer<int> buf[]: register(t2);
// CHECK: @dx.break.cond = internal constant

// Cannonical example. Expected to keep the block in loop
// Verify this function loads the global
// CHECK: load i32
// CHECK-SAME: @dx.break.cond
// CHECK: icmp eq i32

// CHECK: call i32 @dx.op.waveActiveOp.i32

// These verify the first break block keeps the conditional
// CHECK: call %dx.types.Handle @dx.op.createHandle
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK: add
// CHECK: br i1

// These verify the second break block doesn't
// CHECK: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad
// CHECK: add
// CHICK-NOT: br i1

int main(int a : A, int b : B) : SV_Target
{
  int res = 0;

  // Loop with wave op
  for (;;) {
      int u = WaveActiveSum(a);
      if (a == u) {
          res += buf[b][u];
          break;
        }
    }

  // Loop with non-wave op with same signature as previous wave op
  // Without prototype manipulation, this will share an intermediate
  // op call with the previous and get the wave attribute.
  for (;;) {
      int u = abs(a--);
      if (a == u) {
          res += buf[b][u];
          break;
        }
    }

  return res;
}
